#!/bin/bash

# 目标目录
DEST_DIR="../patch"

# 创建目标目录（如果不存在）
mkdir -p "$DEST_DIR"

# 要复制的文件列表
FILES=(
    "clang/lib/CodeGen/CGClass.cpp"
    "clang/lib/CodeGen/CGExpr.cpp"
    "clang/lib/CodeGen/CodeGenFunction.h"
    "clang/lib/Driver/ToolChains/CommonArgs.cpp"
    "compiler-rt/cmake/config-ix.cmake"
    "compiler-rt/lib/CMakeLists.txt"
    "compiler-rt/lib/cfi/cfi.cpp"
    "compiler-rt/lib/xvcfiopt/CMakeLists.txt"
    "compiler-rt/lib/xvcfiopt/cache_init.inc"
    "compiler-rt/lib/xvcfiopt/cfi_xdso_cache.cpp"
    "compiler-rt/lib/xvcfiopt/generate_cache_init.py"
)

# 逐个复制文件
for file in "${FILES[@]}"; do
    # 创建目标文件所在的目录
    mkdir -p "$DEST_DIR/$(dirname "$file")"
    # 复制文件
    if cp "$file" "$DEST_DIR/$file"; then
        echo "已复制: $file"
    else
        echo "复制失败: $file"
    fi
done

echo "所有文件处理完成"

