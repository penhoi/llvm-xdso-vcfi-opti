#!/usr/bin/env python3
import argparse
import sys
import shutil
import os
import r2pipe
import graphviz
import json
from collections import deque

def check_prerequisites():
    """检查所需的命令行工具 r2 (radare2) 和 dot (graphviz) 是否存在。"""
    print("--- 正在检查依赖工具 ---")
    if not shutil.which("r2"):
        print("错误: 'r2' (radare2) 命令未找到。", file=sys.stderr)
        sys.exit(1)
    if not shutil.which("dot"):
        print("错误: 'dot' (graphviz) 命令未找到。", file=sys.stderr)
        sys.exit(1)
    print("✓ Radare2 和 Graphviz 已找到。")

def find_longest_path_by_instructions(blocks: list, entry_offset: int):
    """
    在CFG中寻找从入口到RET指令的、包含最多指令的路径。
    返回 路径长度（指令数）, 块数量, 和 构成路径的块偏移列表。
    """
    if not blocks:
        return 0, 0, []

    # 1. 构建图结构，并识别所有包含'ret'指令的终点块
    adj = {block['offset']: [] for block in blocks}
    in_degree = {block['offset']: 0 for block in blocks}
    block_map = {block['offset']: block for block in blocks}
    ret_nodes = set()

    for block in blocks:
        offset = block['offset']
        # 检查块内是否有ret指令
        if 'ops' in block:
            for op in block['ops']:
                if op['type'] == 'ret':
                    ret_nodes.add(offset)
                    break # 找到一个即可
        
        successors = block.get('jump', []) + block.get('fail', [])
        for succ_offset in successors:
            if succ_offset in adj:
                adj[offset].append(succ_offset)
                in_degree[succ_offset] += 1

    if not ret_nodes:
        print("  ✗ 警告: 在函数中未找到任何 'ret' 指令块。", file=sys.stderr)
        return 0, 0, []

    # 2. 拓扑排序 (Kahn's algorithm)
    queue = deque([offset for offset, degree in in_degree.items() if degree == 0])
    topo_order = []
    while queue:
        u = queue.popleft()
        topo_order.append(u)
        for v in adj.get(u, []):
            in_degree[v] -= 1
            if in_degree[v] == 0:
                queue.append(v)
    
    # 3. 计算最长路径（按指令数加权）
    # dist现在存储的是指令总数
    dist = {offset: -1 for offset in block_map}
    parent = {offset: None for offset in block_map}
    
    # 初始化入口节点的距离为其自身的指令数
    dist[entry_offset] = len(block_map[entry_offset].get('ops', []))

    for u_offset in topo_order:
        if dist[u_offset] == -1:
            continue
        
        for v_offset in adj.get(u_offset, []):
            v_instruction_count = len(block_map[v_offset].get('ops', []))
            if dist[u_offset] + v_instruction_count > dist[v_offset]:
                dist[v_offset] = dist[u_offset] + v_instruction_count
                parent[v_offset] = u_offset

    # 4. 在所有合法的RET终点中，找到路径最长的那个
    end_node_offset = max(ret_nodes, key=lambda offset: dist.get(offset, -1))
    
    max_instructions = dist[end_node_offset]
    if max_instructions == -1:
        print("  ✗ 警告: 无法从入口到达任何 'ret' 指令块。", file=sys.stderr)
        return 0, 0, []

    # 5. 回溯以重建路径
    path = []
    curr = end_node_offset
    while curr is not None:
        path.append(curr)
        curr = parent[curr]
    
    path.reverse()
    return max_instructions, len(path), path

def highlight_path_in_dot(dot_data: str, path_offsets: list) -> str:
    """修改dot数据，高亮显示最长路径上的节点和边。"""
    if not path_offsets: return dot_data
    path_nodes = {f"_{hex(offset)}" for offset in path_offsets}
    path_edges = set()
    for i in range(len(path_offsets) - 1):
        u, v = path_offsets[i], path_offsets[i+1]
        path_edges.add(f"_{hex(u)} -> _{hex(v)}")

    import re
    modified_lines = []
    for line in dot_data.splitlines():
        node_match = re.match(r'\s*(_0x[0-9a-f]+)\s*\[', line)
        if node_match and node_match.group(1) in path_nodes:
            line = line.rstrip()[:-1] + ', color="red", penwidth=3.0, style=filled, fillcolor="#ffdddd"];'
        
        edge_match = re.search(r'(_0x[0-9a-f]+)\s*->\s*(_0x[0-9a-f]+)', line)
        if edge_match:
            edge_str = f"_{edge_match.group(1)} -> _{edge_match.group(2)}"
            if edge_str in path_edges:
                line = line.rstrip()[:-1] + ' [color="red", penwidth=2.5];'
            
        modified_lines.append(line)
    return "\n".join(modified_lines)


def analyze_and_generate_cfg(binary_path: str, function_name: str, output_pdf_path: str, asm_output_dir: str):
    """主分析函数。"""
    print(f"\n--- 正在分析二进制文件: {binary_path} ---")
    r2_func_name = f"sym.{function_name}"
    os.makedirs(asm_output_dir, exist_ok=True)
    r2 = None
    
    try:
        r2 = r2pipe.open(binary_path, flags=['-2'])
        print("[1/6] 运行二进制深度分析 (aaaa)...")
        r2.cmd('aaaa')
        
        target_func = next((f for f in r2.cmdj('aflj') if f['name'] == r2_func_name), None)
        if not target_func:
            print(f"错误: 找不到函数 '{function_name}'.", file=sys.stderr)
            return

        print(f"[2/6] 定位到函数 '{function_name}'...")
        r2.cmd(f"s {target_func['offset']}")
        
        print(f"[3/6] 提取并保存 '{function_name}' 的反汇编代码...")
        disassembly = r2.cmd('pdf')
        asm_filename = os.path.join(asm_output_dir, f"{os.path.basename(binary_path)}_{function_name}.asm")
        with open(asm_filename, 'w', encoding='utf-8') as f:
            f.write(disassembly)
        print(f"  ✓ 反汇编代码已保存至: {asm_filename}")
        
        print(f"[4/6] 提取CFG的JSON结构...")
        graph_json = r2.cmdj('agj')
        if not graph_json:
            print("错误: 无法提取CFG的JSON数据。", file=sys.stderr)
            return

        print("[5/6] 计算最长路径 (按指令数)...")
        entry_offset = target_func['offset']
        instr_len, block_len, path_offsets = find_longest_path_by_instructions(graph_json, entry_offset)
        
        if instr_len > 0:
            print(f"  ✓ 最长路径已找到！ 长度: {instr_len} 条指令 ({block_len} 个基本块)。")
            path_hex = [hex(p) for p in path_offsets]
            print(f"  ✓ 路径 (按块地址): {' -> '.join(path_hex)}")
        else:
            print("  ✗ 未能计算出有效的最长路径。")

        print("[6/6] 生成高亮显示的控制流图...")
        dot_data = r2.cmd('agfd')
        if not dot_data or "digraph" not in dot_data:
            print("错误: 无法生成图数据。", file=sys.stderr)
            return
            
        highlighted_dot = highlight_path_in_dot(dot_data, path_offsets)

    finally:
        if r2: r2.quit()

    try:
        source = graphviz.Source(highlighted_dot)
        output_filename_base = os.path.splitext(output_pdf_path)[0]
        source.render(output_filename_base, view=False, cleanup=True, format='pdf')
        print(f"\n✓ 成功! 高亮的控制流图已保存至: {output_filename_base}.pdf")
        
    except Exception as e:
        print(f"Graphviz渲染出错: {e}", file=sys.stderr)

def main():
    parser = argparse.ArgumentParser(description="为二进制文件中的函数生成带最长路径高亮的CFG。")
    # ... (其余主函数代码与上一版相同)
    parser.add_argument("binary_file", help="要分析的二进制文件路径")
    parser.add_argument("-f", "--function", default="__cfi_check", help="目标函数名 (默认: __cfi_check)")
    parser.add_argument("-o", "--output", default="cfg_output.pdf", help="输出的PDF文件名 (默认: cfg_output.pdf)")
    parser.add_argument("--asm-dir", default="asm_output", help="保存反汇编代码的目录 (默认: asm_output)")
    args = parser.parse_args()

    check_prerequisites()
    analyze_and_generate_cfg(args.binary_file, args.function, args.output, args.asm_dir)

if __name__ == "__main__":
    main()
