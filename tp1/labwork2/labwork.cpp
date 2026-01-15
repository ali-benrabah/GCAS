#include <otawa/otawa.h>
#include <otawa/ipet.h>

using namespace elm;
using namespace otawa;

class TimeBuilder: public BBProcessor {
public:
    static p::declare reg;
    TimeBuilder(): BBProcessor(reg) { }

protected:

    void processBB(WorkSpace *ws, CFG *cfg, Block *b) override {
        if(!b->isBasic())
            return;
        BasicBlock *bb = b->toBasic();
        
        int time = 0;
        for(auto i: *bb) {
            if(i->isMul())
                time += 4;
            else if(i->isLoad())
                time += 5;
            else if(i->isStore())
                time += 2;
            else if(i->isControl() || i->isConditional())
                time += 2;
            else
                time += 1;
        } 
        
        ipet::TIME(bb) = time;
    }
};

p::declare TimeBuilder::reg = p::init("TimeBuilder", Version(1, 0, 0))
    .require(COLLECTED_CFG_FEATURE)
    .provide(ipet::BB_TIME_FEATURE);


class FlashAnalysis: public CFGProcessor {
public:
    static p::declare reg;
    FlashAnalysis(): CFGProcessor(reg) { }

protected:

    void processAll(WorkSpace *ws) override {
        Vector<Block *> todo;

        if(otawa::INVOLVED_CFGS(ws)->count() > 0) {
            Block *entry = otawa::ENTRY_CFG(ws)->entry();
            OUT(entry) = TOP;
            todo.push(entry);
        }

        while(todo) {
            Block *b = todo.pop();
            
            Address in_state = input(b);
            Address out_state = in_state;
            
            if(b->isBasic()) {
                for(auto i: *b->toBasic())
                    out_state = update(out_state, i);
            }
            
            if(out_state != OUT(b)) {
                OUT(b) = out_state;
                for(auto e: b->outEdges())
                    if(!todo.contains(e->sink()))
                        todo.push(e->sink());
            }
        }

        
        const otawa::CFGCollection *cfgs = otawa::INVOLVED_CFGS(ws);
        for(int k = 0; k < cfgs->count(); k++) {
            CFG *cfg = cfgs->get(k);

            for(auto b: *cfg) {
                if(!b->isBasic()) continue;
                
                BasicBlock *bb = b->toBasic();
                Address state = input(bb); 
                int penalty = 0;

                for(auto i: *bb) {
                    Address page = flashBlock(i);
                    if(state == TOP || state != page) {
                        penalty += cost;
                    }
                    state = page;
                }

                if(penalty > 0)
                    ipet::TIME(bb) = ipet::TIME(bb) + penalty;
            }
        }
    }

    void processCFG(WorkSpace *ws, CFG *g) override {
    }

private:

    Address join(Address a1, Address a2) {
        if(a1 == BOT) return a2;
        if(a2 == BOT) return a1;
        if(a1 == a2) return a1;
        return TOP;
    }

    Address update(Address s, Inst *i) {
        return flashBlock(i);
    }

    Address input(Block *v) {
        Address res = BOT;
        for(auto e: v->inEdges())
            res = join(res, OUT(e->source()));
        return res;
    }

    Address flashBlock(Inst *i) {
        return i->address().mask(mask);
    }

    static p::id<Address> OUT;
    static const Address TOP, BOT;
    static const t::uint32 mask = ~63; // masque pour page de 64 octetss
    static const ot::time cost = 20;   // 20 cycles
};

p::declare FlashAnalysis::reg = p::init("FlashAnalysis", Version(1, 0, 0))
    .require(COLLECTED_CFG_FEATURE)
    .require(ipet::BB_TIME_FEATURE);

p::id<Address> FlashAnalysis::OUT("", BOT);
const Address FlashAnalysis::TOP(-1, -2);
const Address FlashAnalysis::BOT;


int main(int argc, char **argv) {

    if(argc != 2) {
        cerr << "SYNTAX: " << argv[0] << " ELF_FILE\n";
        return 1;
    }

    try {
        PropList props;
        VERBOSE(props) = true;

        WorkSpace *ws = MANAGER.load(argv[1], props);

        ws->run<TimeBuilder>(props);
        ws->run<FlashAnalysis>(props);

        ws->require(ipet::WCET_FEATURE, props);

        cout << "WCET = " << *ipet::WCET(ws) << io::endl;

    }
    catch(elm::Exception& e) {
        cerr << "ERROR: " << e.message() << io::endl;
        return 2;
    }

    return 0;
}