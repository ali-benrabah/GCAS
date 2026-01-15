# WCET Analysis with OTAWA Framework

**Master 2 SECIL** - *University Paul Sabatier (Toulouse)*

## 📋 Overview
This repository contains laboratory works focused on **Worst-Case Execution Time (WCET)** analysis using the **OTAWA framework**. It covers both the usage of analysis tools and the development of custom static analysis passes in C++.

## 📂 Project Structure

### Labwork 1: WCET Calculation & Optimization
* **Tools:** `owcet`, `mkff`, `oRange`.
* **Tasks:** Manual loop bounding (`.ff` scripts), complex control flow handling, and code optimization (loop unrolling) to meet real-time constraints.

### Labwork 2: Custom C++ Analysis
* **TimeBuilder:** C++ implementation of a Basic Block timing model (pipeline simulation).
* **FlashAnalysis:** Implementation of an **Abstract Interpretation** algorithm (Fixpoint) to simulate a Flash Fetch Buffer and calculate cache miss penalties (20 cycles).

## 🛠 Build & Run (Labwork 2)
```bash
cd labwork2
cmake .
make
./labwork test/bubble.elf
