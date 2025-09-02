#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Finds a specific function (like __cfi_check), saves its disassembly and CFG,
and analyzes its longest execution path based on a specific algorithm.
"""

import sys
import os
import argparse
import shutil
import r2pipe
import pydot
import graphviz
from typing import List, Dict

#
# Part 1: Longest Path Analysis Logic
# --------------------------------------------------------------------


def find_all_paths_dfs(
    adj_list: Dict[str, List[str]], start_node: str, end_node: str, path: List[str]
) -> List[List[str]]:
    """Uses DFS to recursively find all paths from a start to an end node."""
    path = path + [start_node]
    if start_node == end_node:
        return [path]
    if start_node not in adj_list:
        return []
    all_paths = []
    for neighbor in adj_list.get(start_node, []):
        if neighbor not in path:  # Prevent cycles
            new_paths = find_all_paths_dfs(adj_list, neighbor, end_node, path)
            for new_path in new_paths:
                all_paths.append(new_path)
    return all_paths


def are_paths_different(path1: List[str], path2: List[str], min_diff: int) -> bool:
    """Checks if two paths differ by at least a minimum number of nodes."""
    # Use set symmetric difference to find nodes not present in both paths.
    diff = set(path1) ^ set(path2)
    return len(diff) >= min_diff


def print_single_path(path: List[str], title: str):
    """A helper function to format and print a single path."""
    print("\n" + "=" * 40)
    print(f"{title} Found")
    print("=" * 40)
    print(f"Path Length (nodes): {len(path)}")
    print("Path Details:")
    print(" -> ".join(f'"{hex(int(node))}"' for node in path))
    print("=" * 40)


def analyze_longest_path(dot_data: str):
    """
    Parses DOT graph data and finds two representative paths based on the user's
    specified algorithm.
    """
    print("\n--- Longest Path Analysis ---")
    try:
        graphs = pydot.graph_from_dot_data(dot_data)
        graph = graphs[0]
        nodes = graph.get_nodes()

        if not nodes:
            print("Error: No nodes found in the parsed graph.", file=sys.stderr)
            return

        start_node = next(
            (
                n.get_name()
                for n in nodes
                if n.get_name() and n.get_name().lower() not in ["node", "graph"]
            ),
            None,
        )

        if start_node is None:
            print("Error: Could not determine a valid start node.", file=sys.stderr)
            return

        end_nodes = [
            n.get_name() for n in nodes if "ret" in n.get_attributes().get("label", "")
        ]

        if not end_nodes:
            print(
                "Warning: No 'ret' instruction found. Cannot determine an end node for the path.",
                file=sys.stderr,
            )
            return

        print(f"Start Node: {hex(int(start_node))}")
        print(
            f"End Node(s) containing 'ret': {', '.join(hex(int(en)) for en in end_nodes)}"
        )

        adj_list = {node.get_name(): [] for node in nodes if node.get_name()}
        for edge in graph.get_edges():
            src = edge.get_source()
            dst = edge.get_destination()
            if src in adj_list:
                adj_list[src].append(dst)

        all_possible_paths = []
        for end_node in end_nodes:
            paths = find_all_paths_dfs(adj_list, start_node, end_node, path=[])
            all_possible_paths.extend(paths)

        if not all_possible_paths:
            print(
                "\nError: No executable path found from start to an end node.",
                file=sys.stderr,
            )
            return

        # --- IMPLEMENTATION OF THE NEW ALGORITHM ---

        # 1. Sort all unique paths by their length (longest first).
        unique_paths = [list(p) for p in set(tuple(p) for p in all_possible_paths)]
        unique_paths.sort(key=len, reverse=True)

        if not unique_paths:
            # This case should be rare if all_possible_paths was not empty, but it is safe to check.
            return

        # 2. Mark the first longest path as P0.
        P0 = unique_paths[0]
        Pi = None

        # 3. Iterate through the other paths to find the first suitable Pi.
        for candidate_path in unique_paths[1:]:
            # 4. If Pi has at least two different nodes with P0, end the search.
            if are_paths_different(P0, candidate_path, 3):
                Pi = candidate_path
                break  # Path found, stop searching.

        # Finally, print the results.
        print_single_path(P0, "Longest Path (P0)")

        if Pi:
            print_single_path(Pi, "Alternative Path (Pi)")
        else:
            print(
                "\nInfo: No other path with at least 2 different nodes could be found."
            )

    except Exception as e:
        print(f"Error: An error occurred during path analysis: {e}", file=sys.stderr)


#
# Part 2: CFG Generation Logic (Modified)
# --------------------------------------------------------------------


def generate_and_analyze_cfg(binary_path: str, function_name: str):
    """
    Generates and analyzes the CFG for a given function in a binary.
    """
    print(f"--- CFG Generation for '{function_name}' ---")

    try:
        r2 = r2pipe.open(binary_path)
        r2.cmd("aaa")  # Run initial analysis

        # --- MODIFICATION START ---
        # Instead of seeking directly, find the symbol's address first.
        print(f"Info: Searching for symbol '{function_name}'...")
        addr_str = r2.cmd(f"?v sym.{function_name}").strip()

        if not addr_str or addr_str == "0x0":
            print(
                f"Error: Symbol '{function_name}' not found in the binary.",
                file=sys.stderr,
            )
            return

        print(f"Success: Symbol '{function_name}' found at address {addr_str}.")

        # Explicitly define a function at this address so analysis commands work.
        print(f"Info: Defining function at {addr_str}...")
        r2.cmd(f"af @ {addr_str}")
        # --- MODIFICATION END ---

        # Now, seek to the newly defined function.
        r2.cmd(f"s {addr_str}")

        # Save the disassembly
        disassembly = r2.cmd("pD $FS")  # $FS is radare2's function size variable
        disasm_filename = f"{function_name}_disassembly.txt"
        with open(disasm_filename, "w") as f:
            f.write(disassembly)
        print(f"Info: Disassembly saved to '{disasm_filename}'.")

        # Generate CFG graph object
        graph_data = r2.cmdj("agj")
        if not graph_data or not graph_data[0]["blocks"]:
            print(
                f"Warning: Could not generate graph data for '{function_name}'.",
                file=sys.stderr,
            )
            return

        dot = graphviz.Digraph(name=f"CFG for {function_name}")
        dot.attr("node", shape="box", fontname="monospace")
        dot.attr(bgcolor="transparent")

        for block in graph_data[0]["blocks"]:
            addr = block["offset"]
            ops = block.get("ops", [])
            asm_lines = [f"0x{op['offset']:x}: {op['opcode']}" for op in ops]
            block_label = f"loc_{addr:x}\n" + "\\l".join(asm_lines) + "\\l"
            dot.node(str(addr), label=block_label)

            src_addr = str(addr)
            if "jump" in block:
                dot.edge(src_addr, str(block["jump"]), label="True", color="green")
            if "fail" in block:
                is_unconditional = "jump" not in block
                label = " " if is_unconditional else "False"
                color = "black" if is_unconditional else "red"
                dot.edge(src_addr, str(block["fail"]), label=label, color=color)

        # Apply the longest-path algorithm
        dot_source_data = dot.source
        analyze_longest_path(dot_source_data)

        # Save the CFG file as PDF
        cfg_filename = f"{function_name}_cfg"
        dot.render(cfg_filename, format="pdf", view=False, cleanup=True)
        print(f"\nInfo: CFG graph saved to '{cfg_filename}.pdf'.")

    except Exception as e:
        print(f"Error: An error occurred during CFG generation: {e}", file=sys.stderr)
    finally:
        if "r2" in locals() and r2:
            r2.quit()


def main():
    parser = argparse.ArgumentParser(
        description="Generate a CFG for a function and find its longest path.",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument("binary_file", help="Path to the binary file.")
    parser.add_argument(
        "function_name",
        help="Name of the function/symbol to analyze (e.g., __cfi_check).",
    )
    args = parser.parse_args()

    # ... (The rest of the main function is identical)
    if not os.path.exists(args.binary_file):
        print(f"Error: Binary file '{args.binary_file}' not found.", file=sys.stderr)
        sys.exit(1)
    print("Info: Checking for prerequisites...")
    if not shutil.which("r2") or not shutil.which("dot"):
        print(
            "Error: Ensure 'radare2' and 'graphviz' are installed and in your PATH.",
            file=sys.stderr,
        )
        sys.exit(1)
    print("Success: Prerequisites found.")
    generate_and_analyze_cfg(args.binary_file, args.function_name)
    print("\nProcess completed!")


if __name__ == "__main__":
    main()
