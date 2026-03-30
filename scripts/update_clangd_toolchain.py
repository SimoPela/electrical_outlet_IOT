# Régénère dans .clangd : (1) includes GCC/newlib Xtensa, (2) lib/*/include (PlatformIO).
# Appelé par PlatformIO après chaque build.

import os
import re
from pathlib import Path
from typing import Optional

Import("env")


def _toolchain_block(home: Path) -> Optional[str]:
    tc = home / ".platformio/packages/toolchain-xtensa-esp-elf"
    gcc_root = tc / "lib/gcc/xtensa-esp-elf"
    if not gcc_root.is_dir():
        print("clangd: toolchain toolchain-xtensa-esp-elf introuvable")
        return None

    candidates = [p for p in gcc_root.iterdir() if p.is_dir() and (p / "include").is_dir()]
    if not candidates:
        print("clangd: aucune version GCC Xtensa sous lib/gcc/xtensa-esp-elf")
        return None

    ver_dir = sorted(candidates, key=lambda p: p.name)[-1]
    inc = ver_dir / "include"
    inc_fixed = ver_dir / "include-fixed"
    newlib = tc / "xtensa-esp-elf/include"

    lines = []
    for p in (inc, inc_fixed, newlib):
        if p.is_dir():
            lines.extend(["    - -isystem", f"    - {p}"])

    block = (
        "    # CLANGD_PIO_TOOLCHAIN_BEGIN\n"
        + "\n".join(lines)
        + "\n    # CLANGD_PIO_TOOLCHAIN_END\n"
    )
    print(f"clangd: toolchain Xtensa {ver_dir.name}")
    return block


def _lib_includes_block(proj: Path) -> str:
    lib = proj / "lib"
    lines = []
    if lib.is_dir():
        for inc in sorted(lib.glob("*/include")):
            if inc.is_dir():
                p = inc.resolve()
                lines.extend(["    - -I", f"    - {p}"])

    body = "\n".join(lines)
    if body:
        body = body + "\n"
    return (
        "    # CLANGD_LIB_INCLUDES_BEGIN\n"
        + body
        + "    # CLANGD_LIB_INCLUDES_END\n"
    )


def patch_clangd():
    proj = Path(env["PROJECT_DIR"])
    clangd = proj / ".clangd"
    if not clangd.is_file():
        return

    text = clangd.read_text(encoding="utf-8")

    home = Path(os.path.expanduser("~"))
    tb = _toolchain_block(home)
    if tb is not None:
        pat_tc = re.compile(
            r"    # CLANGD_PIO_TOOLCHAIN_BEGIN\n.*?    # CLANGD_PIO_TOOLCHAIN_END\n",
            re.DOTALL,
        )
        if not pat_tc.search(text):
            print("clangd: marqueurs CLANGD_PIO_TOOLCHAIN_* absents dans .clangd")
        else:
            text = pat_tc.sub(tb, text, count=1)

    lb = _lib_includes_block(proj)
    pat_lib = re.compile(
        r"    # CLANGD_LIB_INCLUDES_BEGIN\n.*?    # CLANGD_LIB_INCLUDES_END\n",
        re.DOTALL,
    )
    if not pat_lib.search(text):
        print("clangd: marqueurs CLANGD_LIB_INCLUDES_* absents dans .clangd")
    else:
        text = pat_lib.sub(lb, text, count=1)
        n = len(list((proj / "lib").glob("*/include"))) if (proj / "lib").is_dir() else 0
        print(f"clangd: lib — {n} dossier(s) include")

    clangd.write_text(text, encoding="utf-8")
    print("clangd: .clangd mis à jour")


patch_clangd()
