## 1. Install dependencies

```shell
sudo apt install cmake ninja-build
sudo apt install clang
```

## 2. Build the original version without any modification.

Parameterize the cmake command (Release Build).

```shell
cmake -G Ninja -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;lld" -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_INSTALL_PREFIX=~/toolchain/llvm14-orig -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ../llvm
```

## 3. Debug my optimized version (Release Build)

Parameterize the cmake command (Debug Build)

```shell
$ cmake -G Ninja -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;lld" -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_INSTALL_PREFIX=~/toolchain/llvm14-opti-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ../llvm

# Enrich debug information, including variable names, into the generarated binaries (clang/llvm).
$ bash debug-cfi.sh
```

## 4. Release my optimized version (Release Build)
```shell
cmake -G Ninja -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;lld" -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_INSTALL_PREFIX=~/toolchain/llvm14-opti -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ../llvm
```

## 5. How to use my optimized version
```shell
Please kindly check the "./benchmark/SPEC_CPU2006v1.2/llvm-vcfi-opt.cfg" file.
```
