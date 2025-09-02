# Source Code Artifacts for [llvm-xdso-vcfi-opti]

This repository contains the source code and evaluation artifacts for the paper, "xvcfiopt". It includes our modifications to the CLANG/LLVM compiler, benchmarks, and various tools developed during our research.

## Repository Contents

This repository is organized into the following key directories:

1. **CLANG/LLVM-14 Modifications (`/llvm-project`)**
   - This directory contains our modified version of the CLANG/LLVM-14 source code.
   - The changes implementing the techniques described in our paper are clearly commented and can be reviewed by comparing this branch to the official LLVM-14 release.
2. **Micro-benchmarks (`/benchmarks`)**
   - This directory contains the micro-benchmark programs we developed to evaluate the performance of our system.
   - Each benchmark is in its own subdirectory and includes the source code and scripts to build and run it.
3. **Hacking Tools for llvm-vcfi (`/tools`)**
   - This directory includes a collection of scripts and tools for analyzing and hacking the implementation details of llvm-vcfi.
   - These tools were used during our research to understand and verify the low-level behavior of the control-flow integrity mechanisms.
4. **Prototypes and Tests (`/prototypes`)**
   - This directory contains various prototypes and a suite of testing programs.
   - These tests were created to verify the functional correctness of our modifications to the compiler and the overall system.

## Getting Started

### Prerequisites

To build and use the artifacts in this repository, you will need a compatible build environment. Please refer to the official LLVM documentation for the required dependencies.

*(You may want to add specific dependencies here, e.g., cmake, ninja, python3, etc.)*

### Building the Compiler

1. Navigate to the modified LLVM project directory:

   ```
   cd llvm-project
   ```

2. Follow the standard LLVM build process. For a typical release build, you can use the following commands:

   ```
   mkdir build && cd build
   #checking the build-llvm.md file
   ninja
   ```

### Running Benchmarks and Tests

Instructions for compiling and running the benchmarks and correctness tests are provided in the respective `/benchmarks` and `/prototypes` directories. Please refer to the `README.md` file within each directory for detailed steps.

## Contact

For any questions, issues with the source code, or discussions about the paper, please feel free to reach out.

**YUAN Pinghai**: `pinghaiyuan@gmail.com`
