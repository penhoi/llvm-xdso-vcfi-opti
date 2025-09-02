#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script takes a single C++ class name as input, embeds it into a template,
compiles it with Clang's CFI instrumentation, and extracts the resulting
CFI type ID (hash) from the compiled shared object.
"""

import subprocess
import re
import os
import argparse
import shutil
import sys
from typing import Optional

# --- C++ Code Template ---
# A simple C++ class template to generate the type info.
CPP_TEMPLATE = """
#include <cstdio>

class {class_name}
{{
public:
    virtual int execute(int a) {{
        // A simple virtual function to ensure a vtable is created.
        return a * 2;
    }}
    virtual ~{class_name}() {{}}
}};

// C-style export to prevent function name mangling.
extern "C" {class_name}* createObject_{class_name}()
{{
    return new {class_name}();
}}
"""

# --- Compile Command ---
# As per your request, using the specified Clang flags.
# Added -fPIC which is necessary for creating a -shared library.
COMPILE_COMMAND = [
    "clang++",
    "-shared",
    "-fuse-ld=lld",
    "-flto",
    "-fvisibility=default",
    "-fsanitize=cfi-vcall",
    "-fsanitize-cfi-cross-dso",
    "-fPIC",
    "-o", "temp.so", "temp.cpp",
    "-ldl"
]

def check_prerequisites():
    """Checks if required command-line tools (clang, objdump) are available."""
    tools = ["clang++", "objdump"]
    for tool in tools:
        if not shutil.which(tool):
            print(f"Error: Required tool '{tool}' not found in your PATH.", file=sys.stderr)
            print("Please ensure Clang/LLVM toolchain is installed and in your PATH.", file=sys.stderr)
            sys.exit(1)
    print("✓ All required tools found.")

def get_typeid_for_class(class_name: str) -> Optional[str]:
    """
    Compiles a template for a given class name and extracts its CFI type ID hash
    from the generated .so file.

    This function implements steps 2 and 3 of your request.
    """
    temp_cpp_file = "temp.cpp"
    temp_so_file = "temp.so"

    # Step 1: Insert class name into the C++ template and write to a file.
    print(f"\n[Step 1/3] Generating C++ source for class '{class_name}'...")
    cpp_code = CPP_TEMPLATE.format(class_name=class_name)
    with open(temp_cpp_file, "w") as f:
        f.write(cpp_code)
    print("  > Done.")

    try:
        # Step 2: Compile the C++ file into a shared object.
        print(f"[Step 2/3] Compiling '{temp_cpp_file}' into '{temp_so_file}'...")
        result = subprocess.run(COMPILE_COMMAND, capture_output=True, text=True, check=False)
        if result.returncode != 0:
            print(f"  ✗ Compilation failed for '{class_name}':\n{result.stderr}", file=sys.stderr)
            return None
        print("  > Done.")

        # Step 3: Disassemble and extract the type ID hash.
        print(f"[Step 3/3] Disassembling and searching for the type ID in __cfi_check...")
        objdump_output = subprocess.check_output(["objdump", "-d", temp_so_file], text=True)

        # Find the start of the __cfi_check function.
        cfi_check_label = "<__cfi_check>:"
        start_index = objdump_output.find(cfi_check_label)

        if start_index == -1:
            print(f"  ✗ Could not find '__cfi_check' function for class '{class_name}'.", file=sys.stderr)
            print("  > This can happen if the compiler optimized away the virtual call.", file=sys.stderr)
            return None

        # Search for the first 'movabs' instruction after the function label.
        search_area = objdump_output[start_index:]
        # This regex captures the hexadecimal value from 'movabs $0x...,%rax'.
        match = re.search(r"movabs\s+\$(0x[0-9a-fA-F]+),%rax", search_area)

        if match:
            type_id = match.group(1)
            print(f"  > Found type ID: {type_id}")
            return type_id
        else:
            print(f"  ✗ Could not find 'movabs $...,%rax' instruction after '__cfi_check' for class '{class_name}'.", file=sys.stderr)
            return None

    except Exception as e:
        print(f"An unexpected error occurred while processing '{class_name}': {e}", file=sys.stderr)
        return None
    finally:
        # Cleanup: ensure temporary files are deleted.
        if os.path.exists(temp_cpp_file):
            os.remove(temp_cpp_file)
        if os.path.exists(temp_so_file):
            os.remove(temp_so_file)

def main():
    """Main function to parse arguments and drive the process."""
    parser = argparse.ArgumentParser(
        description="Generates the CFI type ID (hash) for a given C++ class name."
    )
    parser.add_argument("class_name", help="The C++ class name (e.g., 'MyClass' or 'MyNamespace::MyClass')")
    args = parser.parse_args()

    check_prerequisites()

    class_name = args.class_name
    type_id = get_typeid_for_class(class_name)

    # Step 4: Print the final result.
    print("\n" + "="*40)
    print("      CFI Type ID Generation Result")
    print("="*40)
    if type_id:
        print(f"  Class Name: \"{class_name}\"")
        print(f"  Type ID   : \"{type_id}\"")
        print("\nSuccess!")
    else:
        print(f"  Failed to generate Type ID for \"{class_name}\".")
        print("  Please check the error messages above for details.")
        sys.exit(1)
    print("="*40)

if __name__ == "__main__":
    main()