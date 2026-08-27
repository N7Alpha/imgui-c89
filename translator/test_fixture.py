#!/usr/bin/env python3
"""Build and verify the first semantic-extraction/lowering fixture."""

from __future__ import annotations

import filecmp
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path


def run(*arguments: str, cwd: Path | None = None) -> None:
    print("+", " ".join(arguments))
    subprocess.run(arguments, cwd=cwd, check=True)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    build = root / "build/translator"
    fixture = root / "translator/fixtures/basic"
    unsupported_fixture = root / "translator/fixtures/unsupported"
    first = build / "fixtures/basic/generated-a"
    second = build / "fixtures/basic/generated-b"
    ir = build / "fixtures/basic/ir.json"
    llvm_prefix = os.environ.get("LLVM_PREFIX", "/opt/homebrew/opt/llvm")
    cmake_prefix = os.environ.get("CMAKE_PREFIX_PATH", llvm_prefix)

    run(
        "cmake", "-S", str(root / "translator"), "-B", str(build),
        "-G", "Ninja", f"-DCMAKE_PREFIX_PATH={cmake_prefix}",
        "-DCMAKE_BUILD_TYPE=Debug",
    )
    run("cmake", "--build", str(build), "--target", "imgui-clang-extract")
    ir.parent.mkdir(parents=True, exist_ok=True)
    run(
        str(build / "imgui-clang-extract"), "--output", str(ir),
        str(fixture / "input.cpp"), "--", "-std=c++11",
    )
    extracted = json.loads(ir.read_text())
    macro_names = {item["name"] for item in extracted.get("macros", [])}
    if "IMGUI_FIXTURE_DEFAULT_SCALE" not in macro_names:
        raise RuntimeError("project macro definitions were not captured in IR")
    for directory in (first, second):
        if directory.exists():
            shutil.rmtree(directory)
        run(sys.executable, str(root / "translator/translate.py"), str(ir), str(directory))

    comparison = filecmp.dircmp(first, second)
    if comparison.left_only or comparison.right_only or comparison.diff_files or comparison.funny_files:
        raise RuntimeError("generation is not byte-identical")

    cc = os.environ.get("CC", "cc")
    cxx = os.environ.get("CXX", "c++")
    obj = first / "imgui_translated.o"
    run(
        cc, "-std=c89", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
        "-I", str(first), "-c", str(first / "imgui_translated.c"), "-o", str(obj),
    )
    reference = build / "fixtures/basic/reference"
    candidate = build / "fixtures/basic/candidate"
    run(
        cxx, "-std=c++98", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
        str(fixture / "reference_main.cpp"), "-o", str(reference),
    )
    run(
        cxx, "-std=c++98", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
        "-I", str(first), str(fixture / "candidate_main.cpp"), str(obj), "-o", str(candidate),
    )
    run(str(reference))
    run(str(candidate))

    unsupported_ir = build / "fixtures/unsupported/ir.json"
    unsupported_output = build / "fixtures/unsupported/generated"
    unsupported_ir.parent.mkdir(parents=True, exist_ok=True)
    run(
        str(build / "imgui-clang-extract"), "--output", str(unsupported_ir),
        str(unsupported_fixture / "input.cpp"), "--", "-std=c++11",
    )
    failure = subprocess.run(
        [sys.executable, str(root / "translator/translate.py"),
         str(unsupported_ir), str(unsupported_output)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if failure.returncode == 0 or "unsupported statement" not in failure.stderr:
        raise RuntimeError(
            "unsupported constructs must fail with a source-located diagnostic; "
            f"got exit={failure.returncode}, stderr={failure.stderr!r}"
        )
    print("translator fixture: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
