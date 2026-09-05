#!/usr/bin/env python3
"""一键生成并打印 sum 程序的调用树。

流程: 编译 sum.c -> gdb 运行 gdb_trace.py 生成调用栈轨迹 (gdb_trace.txt)
      -> 解析轨迹中每条 "new path" (栈顶到栈底的函数链) -> 组装成树打印。

用法: python3 analyze_trace.py
"""

import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
TRACE = HERE / "gdb_trace.txt"

# gdb 无法把 _dl_start_user 的栈帧接到 ld.so 的 _start 上(中间不是标准调用),
# 手动把这条父边补上
FIXUPS = {
    ("_dl_start_user", "ld"): ("_start", "ld"),
}

LABEL_MAP = {"main": "a.out", "ld": "ld.so", "lib": "libc"}

# 叶子节点的补充说明
LEAF_NOTES = {
    "main": "return 0",
    "__GI__exit": "exit_group 系统调用, 进程消失",
}

PATH_RE = re.compile(r"^===== new path: (.*) =====\s*$")
FRAME_RE = re.compile(r"^#\d+\s+")
FRAME_ADDR_RE = re.compile(r"^#\d+\s+(0x[0-9a-f]+)\s+in\s+")
FRAME_FILE_RE = re.compile(r"\bat\s+(\S+):\d+")


def addr_class(addr):
    """地址属于哪个二进制: main / ld / lib (本机布局的启发式)"""
    if addr is None:
        return "lib"
    if 0x555555550000 <= addr <= 0x555555560000:  # PIE 主程序
        return "main"
    if 0x7FFFF7F90000 <= addr <= 0x7FFFF8000000:  # ld.so 装载在地址空间顶部
        return "ld"
    return "lib"


def frame_class(addr, file):
    """帧属于哪个二进制: 有地址看地址, 否则按源码路径兜底
    (ld.so 的源码都在 ./elf/ 下)"""
    if addr is not None:
        return addr_class(addr)
    if file is not None:
        if file.startswith("./elf/"):
            return "ld"
        if "sum.c" in file:
            return "main"
    return "lib"


def run_gdb():
    print("[1/3] 编译 sum.c ...")
    subprocess.run(["gcc", "-g", "-O0", "sum.c", "-o", "sum"], cwd=HERE, check=True)
    print("[2/3] gdb 追踪进入/退出路径 ...")
    with open(TRACE, "w", encoding="utf-8") as out:
        ret = subprocess.run(
            ["gdb", "-batch", "-q", "-x", "gdb_trace.py", "./sum"],
            cwd=HERE,
            stdout=out,
            stderr=subprocess.STDOUT,
            check=True,
        )
    (HERE / "sum").unlink(missing_ok=True)
    if ret.returncode != 0:
        print(
            f"warning: gdb 退出码 {ret.returncode}, 详见 {TRACE.name}", file=sys.stderr
        )


def parse_sections():
    """解析 gdb_trace.txt 中每个 new path 段落。

    返回 [(names, frames), ...], names 是栈顶到栈底的函数名,
    frames 是各帧的 (地址, 源码文件) (栈顶到栈底, 缺省处为 None), 与 names 对齐。
    """
    lines = TRACE.read_text(encoding="utf-8").splitlines()
    sections = []
    i = 0
    while i < len(lines):
        m = PATH_RE.match(lines[i])
        if not m:
            i += 1
            continue
        names = tuple(s for s in m.group(1).split(" <- ") if s)
        i += 1
        frames = []
        while i < len(lines):
            line = lines[i]
            if not FRAME_RE.match(line):
                break
            am = FRAME_ADDR_RE.match(line)
            fm = FRAME_FILE_RE.search(line)
            frames.append(
                (int(am.group(1), 16) if am else None, fm.group(1) if fm else None)
            )
            i += 1
            # 跳过 gdb 换行折叠产生的续行 (缩进行)
            while (
                i < len(lines)
                and lines[i].startswith((" ", "\t"))
                and not FRAME_RE.match(lines[i])
            ):
                i += 1
        sections.append((names, frames))
    return sections


class Node:
    __slots__ = ("children", "cls", "name")

    def __init__(self, name, cls):
        self.name = name
        self.cls = cls
        self.children = {}  # (name, cls) -> Node


def build_tree(sections):
    """把每条栈路径(栈顶到栈底)倒过来插入树, 根是调用链最底层"""
    root = Node("", "")
    for names, frames in sections:
        chain = list(reversed(names))
        stripped = 0
        while chain and chain[0] == "??":  # 去掉内核初始栈的垃圾帧
            chain.pop(0)
            stripped += 1
        if not chain:
            continue
        keys = []
        for j, name in enumerate(chain):
            # 链上第 j 个对应原 names 的倒数第 stripped+j+1 个
            fi = len(names) - 1 - stripped - j
            addr, file = frames[fi] if fi < len(frames) else (None, None)
            keys.append((name, frame_class(addr, file)))
        if keys[0] in FIXUPS:
            keys.insert(0, FIXUPS[keys[0]])
        node = root
        for key in keys:
            node = node.children.setdefault(key, Node(*key))
    return root


def duplicate_names(root):
    """同名但来自不同二进制的函数名集合 (需要加标注)"""
    by_name = {}
    stack = list(root.children.values())
    while stack:
        n = stack.pop()
        by_name.setdefault(n.name, set()).add(n.cls)
        stack.extend(n.children.values())
    return {name for name, classes in by_name.items() if len(classes) > 1}


def render(root, dup_names):
    lines = []

    def walk(node, prefix, is_last, on_path):
        if node in on_path:
            lines.append(prefix + ("└── " if is_last else "├── ") + node.name + " ↺")
            return
        label = (
            f" ⟨{LABEL_MAP[node.cls]}⟩"
            if node.name in dup_names and node.cls in LABEL_MAP
            else ""
        )
        lines.append(prefix + ("└── " if is_last else "├── ") + node.name + label)
        on_path.add(node)
        children = list(node.children.values())
        for idx, child in enumerate(children):
            walk(
                child,
                prefix + ("    " if is_last else "│   "),
                idx == len(children) - 1,
                on_path,
            )
        on_path.discard(node)
        if node.name in LEAF_NOTES and not children:
            lines[-1] += f"   ← {LEAF_NOTES[node.name]}"

    for idx, child in enumerate(root.children.values()):
        walk(child, "", idx == len(root.children) - 1, set())
    return "\n".join(lines)


def main():
    run_gdb()
    print("[3/3] 解析并打印调用树 ...\n")
    sections = parse_sections()
    if not sections:
        print("没有解析到任何 new path, 请检查 gdb_trace.txt", file=sys.stderr)
        return 1
    tree = build_tree(sections)
    dup = duplicate_names(tree)
    print(render(tree, dup))
    print(f"\n({len(sections)} 条路径, 完整轨迹见 {TRACE.name})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
