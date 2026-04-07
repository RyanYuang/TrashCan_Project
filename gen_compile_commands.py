#!/usr/bin/env python3
"""
从 STM32CubeIDE 生成的 Debug/subdir.mk 中提取编译命令，
生成 clangd 所需的 compile_commands.json。

用法：在项目根目录运行 python3 gen_compile_commands.py
"""

import os
import re
import json
import glob

PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
DEBUG_DIR = os.path.join(PROJECT_ROOT, "Debug")
OUTPUT = os.path.join(PROJECT_ROOT, "compile_commands.json")


def find_subdir_mks():
    return glob.glob(os.path.join(DEBUG_DIR, "**", "subdir.mk"), recursive=True)


def parse_subdir_mk(mk_path):
    """解析 subdir.mk，提取 .c 文件到编译命令的映射"""
    with open(mk_path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    entries = []

    # 查找形如 "xxx/%.o ... : ../xxx/%.c ...\n\tarm-none-eabi-gcc ..." 的规则块
    # 多行合并（续行用 \）
    content_merged = re.sub(r"\\\n\s*", " ", content)

    # 匹配规则行：目标.o ... : 前缀/%.c ...
    rule_pattern = re.compile(
        r"([\w/%.]+\.o[^:]*):([^\n]+)\n\t(arm-none-eabi-gcc[^\n]+)"
    )

    for m in rule_pattern.finditer(content_merged):
        prerequisite = m.group(2).strip()
        command_template = m.group(3).strip()

        # 提取源文件目录前缀（如 ../Core/Src/%.c -> ../Core/Src/）
        src_match = re.search(r"\.\./([^\s%]+)/%.c", prerequisite)
        if not src_match:
            continue
        src_rel_dir = src_match.group(1)  # 例如 Core/Src
        src_abs_dir = os.path.join(PROJECT_ROOT, src_rel_dir)

        # 在同一个 subdir.mk 中找 C_SRCS 列出的具体文件
        c_srcs_match = re.findall(r"\.\./(" + re.escape(src_rel_dir) + r"/[\w\-\.]+\.c)", content_merged)

        for rel_src in c_srcs_match:
            abs_src = os.path.normpath(os.path.join(PROJECT_ROOT, rel_src))
            if not os.path.exists(abs_src):
                continue

            # 把 "$<" 替换为实际源文件路径
            cmd = command_template.replace('"$<"', f'"{abs_src}"')
            # 把 "$@" / 输出目标替换为占位符
            stem = os.path.splitext(os.path.basename(rel_src))[0]
            cmd = cmd.replace('"$@"', f'"{stem}.o"')
            # 处理 -MF"$(@:%.o=%.d)" 等
            cmd = re.sub(r'-MF"\$\([^)]+\)"', '', cmd)
            cmd = re.sub(r'-MT"\$@"', '', cmd)
            # 相对路径 -I../xxx 转绝对路径（在 Debug/ 中执行时）
            def abs_include(m2):
                flag, path = m2.group(1), m2.group(2)
                if path.startswith("/"):
                    return f'{flag}"{path}"'
                abs_p = os.path.normpath(os.path.join(DEBUG_DIR, path))
                return f'{flag}"{abs_p}"'
            cmd = re.sub(r'(-I)\"(\.\.[^"]+)\"', abs_include, cmd)

            entries.append({
                "directory": DEBUG_DIR,
                "command": cmd,
                "file": abs_src,
            })

    return entries


def main():
    all_entries = []
    mk_files = find_subdir_mks()
    print(f"找到 {len(mk_files)} 个 subdir.mk 文件")

    for mk in sorted(mk_files):
        entries = parse_subdir_mk(mk)
        print(f"  {os.path.relpath(mk, PROJECT_ROOT)}: {len(entries)} 个文件")
        all_entries.extend(entries)

    with open(OUTPUT, "w", encoding="utf-8") as f:
        json.dump(all_entries, f, indent=2, ensure_ascii=False)

    print(f"\n✓ 生成 compile_commands.json，共 {len(all_entries)} 条记录")
    print(f"  路径: {OUTPUT}")


if __name__ == "__main__":
    main()
