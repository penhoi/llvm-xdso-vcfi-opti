#!/bin/bash

# --- Configuration for LLVM-XDSO-VCFI Build ---
export MIXVCALL="YES"

# Set the toolchain root directory
export CLANG_ROOT=$HOME/toolchain/llvm14-orig/bin

# Set the specific compiler binaries
export PROJECT_CXX="${CLANG_ROOT}/clang++"
export PROJECT_CC="${CLANG_ROOT}/clang"

# Define and export the specific CXXFLAGS for this build
export CXXFLAGS="-O2 -fuse-ld=lld -flto -fvisibility=default -fsanitize=cfi-vcall -fsanitize-cfi-cross-dso"


# Prepend the toolchain to the PATH
export PATH="${CLANG_ROOT}:${PATH}"

echo "================================================="
echo "Starting LLVM-XDSO-VCFI Build..."
echo "CXX: ${PROJECT_CXX}"
echo "CXXFLAGS: ${CXXFLAGS}"
echo "================================================="

# Call the common Makefile, passing along any script arguments (like 'all' or 'clean')
make -f common.mk "$@"