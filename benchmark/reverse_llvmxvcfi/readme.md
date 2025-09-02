# A Binary Analysis and CFI Toolset

This repository contains a collection of Python scripts for binary analysis, with a focus on Clang's Control-Flow Integrity (CFI) and Control-Flow Graph (CFG) generation. These tools help in reverse engineering, security analysis, and understanding program structure.

------
## Prerequisites

Before using these scripts, please ensure you have the following dependencies installed on your system.

### System Tools


- **Clang/LLVM Toolchain**: Required for compiling C++ code with CFI instrumentation (`clang++`).
- **Binutils**: Provides essential binary tools like `nm` and `objdump`.
- **Radare2**: A powerful reverse engineering framework used for disassembly and analysis.
- **Graphviz**: A graph visualization tool used to render CFGs (`dot`).


### Python Libraries


You can install the required Python libraries using pip:

```
pip install r2pipe graphviz pydot
```

------



## Scripts Overview


### 1. llvmvcfi_typeid2classname.py

This script is a reverse lookup tool that converts a CFI **type ID** (hash) back to its original C++ **class name**.

- **How it Works**: It operates by first extracting all possible C++ class names from a target shared library (`.so`). It then recompiles each class name individually to generate its corresponding type ID, effectively building a temporary map. Finally, it uses this map to find the class name that matches the user-provided type ID.
- **Usage**: `python3 llvmvcfi_typeid2classname.py <path_to_shared_library>`
- **Example**: `python3 llvmvcfi_typeid2classname.py ./libproject.so`


### 2. llvmvcfi_classname2typeid.py

This script generates the CFI **type ID** for a given C++ **class name**. This is useful for predicting the hash that Clang's V-CFI will generate for a specific class.

- **How it Works**: The script embeds the provided class name into a C++ code template. It then compiles this template into a temporary shared library using Clang with the necessary CFI flags (`-fsanitize=cfi-vcall`). Finally, it disassembles the binary's `__cfi_check` function to extract the generated type ID hash.
- **Usage**: `python3 llvmvcfi_classname2typeid.py "<ClassName>"`
- **Example**: `python3 llvmvcfi_classname2typeid.py "MyClass"`


### 3. generate_function_cfg.py

This script visualizes the control flow of a function by generating its **Control-Flow Graph (CFG)**.

- **How it Works**: It leverages the **Radare2** framework to disassemble the target binary and analyze the specified function. It identifies all basic blocks and the jumps/branches between them. This structural information is then passed to the **Graphviz** library to render a PDF image of the graph.
- **Usage**: `python3 generate_function_cfg.py <path_to_binary> <function_name>`
- **Example**: `python3 generate_function_cfg.py /bin/ls main`


### 4. analyze_cficheck.py

This is a more advanced script that not only generates a function's CFG but also analyzes it to find the **longest possible execution path**.

- **How it Works**: First, it generates the CFG using the same Radare2 and Graphviz process as the script above. In addition, it builds an in-memory representation of the graph and performs a **Depth-First Search (DFS)** to discover all possible paths from the function's entry point to any block ending in a `ret` instruction. It then identifies and prints the longest of these paths.
- **Usage**: `python3 analyze_cficheck.py <path_to_binary> <function_name>`
- **Example**: `python3 analyze_cficheck.py ./my_app __cfi_check`