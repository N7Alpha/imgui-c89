#!/usr/bin/env python3
"""Compile generated C with GCC and prove its record ABI matches Clang's IR."""

from __future__ import annotations

import argparse
import json
import os
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "py"))
from imgui_translator.emit import Emitter  # noqa: E402


def run(arguments: list[str]) -> None:
    print("+", " ".join(arguments), flush=True)
    subprocess.run(arguments, check=True)


def capture(arguments: list[str]) -> str:
    return subprocess.run(
        arguments, check=True, text=True, stdout=subprocess.PIPE
    ).stdout.strip()


def system_dependency_flags(flags: list[str]) -> list[str]:
    result: list[str] = []
    index = 0
    while index < len(flags):
        flag = flags[index]
        if flag == "-I" and index + 1 < len(flags):
            result.extend(["-isystem", flags[index + 1]])
            index += 2
        elif flag.startswith("-I"):
            result.extend(["-isystem", flag[2:]])
            index += 1
        else:
            result.append(flag)
            index += 1
    return result


def gcc() -> str:
    requested = os.environ.get("PORTABILITY_CC")
    if requested:
        return requested
    for candidate in ("gcc-15", "gcc-14", "gcc-13", "gcc"):
        found = shutil.which(candidate)
        if found:
            return found
    raise SystemExit("GCC is required (set PORTABILITY_CC if it is not on PATH)")


def layout_source(ir: dict, emitter: Emitter) -> str:
    lines = [
        '#include "imgui_c89_internal.h"',
        "#include <stddef.h>",
        "",
    ]
    assertion = 0

    def check(expression: str) -> None:
        nonlocal assertion
        lines.append(
            f"typedef char imgui_c89_layout_assert_{assertion}"
            f"[({expression}) ? 1 : -1];"
        )
        assertion += 1

    emitted: set[str] = set()
    for record in sorted(emitter.records.values(), key=lambda item: item["qualified_name"]):
        size_bits = record.get("size_bits")
        align_bits = record.get("align_bits")
        if not size_bits or not align_bits:
            continue
        name = emitter.record_names_by_id[record["id"]]
        if name in emitted:
            continue
        emitted.add(name)
        check(f"sizeof({name}) * 8 == {size_bits}")
        probe = f"imgui_c89_align_probe_{assertion}"
        lines.append(f"struct {probe} {{ char byte; {name} value; }};")
        check(f"offsetof(struct {probe}, value) * 8 == {align_bits}")
        for field in record.get("fields", []):
            if "bit_width" in field or field.get("offset_bits") is None:
                continue
            field_name = emitter.field_names[field["id"]]
            check(
                f"offsetof({name}, {field_name}) * 8 == {field['offset_bits']}"
            )
    lines.extend([
        "",
        "int main(void)",
        "{",
        "    return 0;",
        "}",
        "",
    ])
    print(f"layout gate: {len(emitted)} records, {assertion} assertions")
    return "\n".join(lines)


def test_profile(
    root: Path, profile_name: str, cc: str, architecture_flags: list[str]
) -> None:
    profile = json.loads(
        (root / "translator/profiles" / f"{profile_name}.json").read_text()
    )
    directory = root / "build/translator" / profile_name
    ir_path = directory / "program.ir.json"
    if not ir_path.exists():
        run([sys.executable, str(root / "translator/build_baseline.py"),
             "--profile", profile_name])
    generated = directory / "generated-portability"
    run([sys.executable, str(root / "translator/translate.py"),
         str(ir_path), str(generated)])
    dependency_cflags: list[str] = []
    dependency_libs: list[str] = []
    for package in profile.get("pkg_config_packages", []):
        dependency_cflags.extend(shlex.split(capture([
            "pkg-config", "--cflags", package,
        ])))
        dependency_libs.extend(shlex.split(capture([
            "pkg-config", "--libs", package,
        ])))
    dependency_cflags = system_dependency_flags(dependency_cflags)
    common_c = [
        cc, *architecture_flags, "-std=c90", "-pedantic-errors", "-w",
        "-I", str(generated),
        *dependency_cflags,
    ]
    units = json.loads((generated / "translation_units.json").read_text())
    objects: list[Path] = []
    for unit in units:
        source = generated / unit["source"]
        output = source.with_suffix(".gcc.o")
        run(common_c + ["-c", str(source), "-o", str(output)])
        objects.append(output)

    native_api_object = generated / "imgui_c89_api.gcc.o"
    run(common_c + [
        "-c", str(generated / "imgui_c89_api.c"),
        "-o", str(native_api_object),
    ])

    ir = json.loads(ir_path.read_text())
    layout = generated / "layout_abi.c"
    layout.write_text(layout_source(ir, Emitter(ir)), encoding="utf-8")
    layout_executable = generated / "layout_abi"
    run(common_c + [str(layout), "-o", str(layout_executable)])
    run([str(layout_executable)])

    # The normal facade is compiled by a C++ compiler and linked against the
    # GCC-produced objects.  This catches calling-convention and mangling
    # mistakes in addition to the compile-time record-layout proof above.
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    revision = profile.get("upstream_revision", profile_name)
    upstream = root / "build/upstream" / lock[revision]["commit"]
    cxx = os.environ.get("CXX", "c++")
    facade_standard = profile.get("facade_language", profile["language"])
    common_cpp = [
        cxx, *architecture_flags, f"-std={facade_standard}",
        "-pedantic-errors", "-Wall",
        "-I", str(upstream), "-I", str(upstream / "backends"),
        *dependency_cflags,
    ]
    wrapper = generated / "imgui_translated_wrapper.gcc-abi.o"
    smoke = generated / "baseline_smoke.gcc-abi.o"
    run(common_cpp + [
        "-Wno-return-type-c-linkage", "-I", str(generated), "-c",
        str(generated / "imgui_translated_wrapper.cpp"), "-o", str(wrapper),
    ])
    backend_units = profile.get("backend_translation_units", [])
    have_null = int("backends/imgui_impl_null.cpp" in backend_units)
    have_sdl = int("backends/imgui_impl_sdl3.cpp" in backend_units)
    run(common_cpp + [
        f"-DIMGUI_TRANSLATED_HAVE_NULL_BACKEND={have_null}",
        f"-DIMGUI_TRANSLATED_HAVE_SDL3_BACKEND={have_sdl}", "-c",
        str(root / "translator/fixtures/baseline_smoke.cpp"), "-o", str(smoke),
    ])
    executable = generated / "gcc_abi_smoke"
    run([
        cxx, *architecture_flags, str(smoke), str(wrapper),
        *(str(item) for item in objects),
        "-lm", *dependency_libs, "-o", str(executable),
    ])
    run([str(executable)])
    native_smoke = generated / "native_api_gcc_smoke"
    run(common_c + [
        str(root / "translator/fixtures/native_api_smoke.c"),
        str(native_api_object), *(str(item) for item in objects),
        "-lm", *dependency_libs, "-o", str(native_smoke),
    ])
    run([str(native_smoke)])
    print(
        f"portability {profile_name}: PASS ({len(objects)} GCC C90 units, "
        "Clang-IR layout, C++ ABI smoke)", flush=True,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", action="append", dest="profiles")
    parser.add_argument(
        "--m32", action="store_true",
        help="compile, link, and execute the entire gate as ILP32",
    )
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    cc = gcc()
    print("GCC portability compiler:", capture([cc, "--version"]).splitlines()[0])
    architecture_flags = ["-m32"] if args.m32 else []
    for profile in args.profiles or ["baseline", "docking"]:
        test_profile(root, profile, cc, architecture_flags)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
