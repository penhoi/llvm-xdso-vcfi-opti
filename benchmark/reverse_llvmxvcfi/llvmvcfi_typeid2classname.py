#!/usr/bin/env python3
import subprocess
import re
import os
import argparse
import shutil
import sys
from typing import List, Dict, Optional

# --- C++ 代码模板 ---
CPP_TEMPLATE = """
#include <cstdio>

class {class_name}
{{
public:
    virtual int execute(int a) {{ 
        // 使用一个易于被优化的函数体，防止编译器产生不必要的代码
        return a * 2; 
    }}
    virtual ~{class_name}() {{}}
}};

// 使用C风格的导出函数，防止函数名被mangled
extern "C" {class_name}* createObject_{class_name}()
{{
    return new {class_name}();
}}
"""

# --- 编译命令 ---
# 使用了您提供的精确命令
COMPILE_COMMAND = [
    "clang++",
    "-fuse-ld=lld",
    "-flto",
    "-fvisibility=default",
    "-fsanitize=cfi-vcall",
    "-fsanitize-cfi-cross-dso",
    "-fPIC",
    "-shared",
    "-o", "temp.so", "temp.cpp"
]

def check_prerequisites():
    """检查所需命令行工具是否存在"""
    tools = ["clang++", "nm", "c++filt", "objdump"]
    for tool in tools:
        if not shutil.which(tool):
            print(f"错误: 必需的工具 '{tool}' 未在您的PATH中找到。", file=sys.stderr)
            print("请安装Clang 14工具链和binutils。", file=sys.stderr)
            sys.exit(1)
    print("✓ 所有必需的工具都已找到。")

def extract_class_names(so_file_path: str) -> List[str]:
    """
    从一个.so文件中提取所有唯一的C++类名。
    它通过查找typeinfo符号（_ZTS）并进行demangle来工作。
    """
    print(f"\n[步骤 1/3] 正在从 '{so_file_path}' 中提取类名...")
    class_names = set()
    
    # 管道命令: nm <file> | grep _ZTS | c++filt
    try:
        # 1. 使用 nm 查找所有类型信息符号 (_ZTS)
        nm_process = subprocess.Popen(["nm", "-D", so_file_path], stdout=subprocess.PIPE)
        
        # 2. 过滤出包含 _ZTS 的行
        grep_process = subprocess.Popen(["grep", "_ZTS"], stdin=nm_process.stdout, stdout=subprocess.PIPE)
        nm_process.stdout.close()

        # 3. 使用 c++filt 将符号 demangle
        filt_process = subprocess.Popen(["c++filt"], stdin=grep_process.stdout, stdout=subprocess.PIPE, text=True)
        grep_process.stdout.close()

        stdout, _ = filt_process.communicate()

        # 正则表达式，从 'typeinfo name for MyNamespace::MyClass' 中提取 'MyNamespace::MyClass'
        # 支持命名空间和模板（尽管模板可能不完全可靠）
        regex = re.compile(r"typeinfo name for (.*)")
        for line in stdout.strip().split('\n'):
            match = regex.search(line)
            if match:
                # 移除可能存在的' (Mangled name: ...)'部分
                full_name = match.group(1).split("'")[0].strip()
                class_names.add(full_name)

    except FileNotFoundError:
        print(f"错误: 无法运行 'nm' 或 'c++filt'。请确保binutils已安装。", file=sys.stderr)
        return []
    except subprocess.CalledProcessError as e:
        print(f"执行命令时出错: {e}", file=sys.stderr)
        return []

    print(f"  > 找到了 {len(class_names)} 个唯一的类名。")
    return sorted(list(class_names))

def get_hash_for_class(class_name: str) -> Optional[str]:
    """
    为一个类名编译模板，并从生成的.so文件中提取CFI哈希值。
    """
    temp_cpp_file = "temp.cpp"
    temp_so_file = "temp.so"

    # 步骤 2: 将类名放入C++模板并创建文件
    cpp_code = CPP_TEMPLATE.format(class_name=class_name)
    with open(temp_cpp_file, "w") as f:
        f.write(cpp_code)

    try:
        # 步骤 3: 编译文件
        result = subprocess.run(COMPILE_COMMAND, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"  ✗ 编译 '{class_name}' 失败:\n{result.stderr}", file=sys.stderr)
            return None

        # 步骤 4: 反汇编并提取哈希
        objdump_output = subprocess.check_output(["objdump", "-d", temp_so_file], text=True)
        
        # 找到 __cfi_check 函数的开始位置
        cfi_check_label = "<__cfi_check>:"
        start_index = objdump_output.find(cfi_check_label)
        
        if start_index == -1:
            print(f"  ✗ 无法在为 '{class_name}' 生成的so中找到 '__cfi_check' 函数。", file=sys.stderr)
            return None
        
        # 从该位置开始搜索第一个 movabs 指令
        search_area = objdump_output[start_index:]
        
        # 正则表达式来匹配 'movabs $0x...,%rax' 并捕获十六进制值
        match = re.search(r"movabs\s+\$(0x[0-9a-fA-F]+),%rax", search_area)
        
        if match:
            return match.group(1)
        else:
            print(f"  ✗ 无法在 '__cfi_check' 后找到 'movabs $...,%rax' 指令 (类: '{class_name}')。", file=sys.stderr)
            return None

    except Exception as e:
        print(f"处理 '{class_name}' 时发生意外错误: {e}", file=sys.stderr)
        return None
    finally:
        # 步骤 5 (清理): 确保临时文件被删除
        if os.path.exists(temp_cpp_file):
            os.remove(temp_cpp_file)
        if os.path.exists(temp_so_file):
            os.remove(temp_so_file)

def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description="自动从.so文件提取类名，并通过编译和反汇编来生成其CFI哈希值。"
    )
    parser.add_argument("so_file", help="要从中提取类名的源二进制.so文件路径")
    args = parser.parse_args()

    if not os.path.exists(args.so_file):
        print(f"错误: 文件 '{args.so_file}' 不存在。", file=sys.stderr)
        sys.exit(1)

    check_prerequisites()
    
    class_names = extract_class_names(args.so_file)
    if not class_names:
        print("未能提取到任何类名，程序退出。")
        sys.exit(1)
        
    print("\n[步骤 2/3] 开始为每个类名生成并提取哈希值...")
    
    results: Dict[str, str] = {}
    total = len(class_names)
    for i, name in enumerate(class_names):
        print(f"  ({i+1}/{total}) 正在处理: {name} ...")
        hash_value = get_hash_for_class(name)
        if hash_value:
            print(f"    ✓ 成功! 哈希值: {hash_value}")
            results[name] = hash_value
        else:
            results[name] = "Error"
            
    print("\n[步骤 3/3] 所有任务完成！")
    print("\n" + "="*40)
    print(" 类名 -> 哈希值 映射关系")
    print("="*40)
    for name, hash_val in results.items():
        print(f'"{name}": "{hash_val}"')
    print("="*40)


if __name__ == "__main__":
    main()
