#!/usr/bin/env python3
"""Run full extraction, C89 lowering, compilation, and facade smoke by release."""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

from check_baseline import assert_same_outputs
from merge_ir import merge


CORE_SOURCES = [
    "imgui.cpp",
    "imgui_draw.cpp",
    "imgui_tables.cpp",
    "imgui_widgets.cpp",
    "imgui_demo.cpp",
]


def capture(arguments: list[str]) -> str:
    return subprocess.run(
        arguments, check=True, text=True, stdout=subprocess.PIPE
    ).stdout.strip()


def run(arguments: list[str]) -> None:
    print("+", " ".join(arguments), flush=True)
    subprocess.run(arguments, check=True)


def standard_for(family: str) -> str:
    return "c++98" if tuple(map(int, family.split("."))) < (1, 87) else "c++11"


def check_revision(
    root: Path,
    selected: dict[str, str],
    extractor: Path,
    resource: str,
    sdk: str,
) -> dict[str, object]:
    family = selected["family"]
    standard = standard_for(family)
    checkout = root / "build/upstream-matrix" / family
    output = root / "build/translator/matrix" / family
    generated = output / "generated"
    output.mkdir(parents=True, exist_ok=True)
    run([
        sys.executable, str(root / "translator/fetch_upstream.py"),
        "--revision", family, "--destination", str(checkout),
    ])

    ir_paths: list[Path] = []
    for source_name in CORE_SOURCES:
        source = checkout / source_name
        ir_path = output / (source_name.replace(".", "_") + ".ir.json")
        run([
            str(extractor), "--output", str(ir_path),
            "--source-root", str(checkout), str(source), "--",
            f"-std={standard}", f"-I{checkout}",
            "-resource-dir", resource, "-isysroot", sdk,
            "-Wno-nontrivial-memcall",
        ])
        ir_paths.append(ir_path)

    program = output / "program.ir.json"
    program.write_text(
        json.dumps(merge(ir_paths), indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    run([
        sys.executable, str(root / "translator/translate.py"),
        str(program), str(generated),
    ])
    with tempfile.TemporaryDirectory(
        prefix="generated-check-", dir=output
    ) as temporary:
        generated_check = Path(temporary)
        run([
            sys.executable, str(root / "translator/translate.py"),
            str(program), str(generated_check),
        ])
        assert_same_outputs(generated, generated_check)

    cc = os.environ.get("CC", "cc")
    cxx = os.environ.get("CXX", "c++")
    units = json.loads(
        (generated / "translation_units.json").read_text(encoding="utf-8")
    )
    objects: list[Path] = []
    for unit in units:
        source = generated / unit["source"]
        obj = source.with_suffix(".o")
        run([
            cc, "-std=c89", "-pedantic-errors", "-Wall",
            "-Wno-overlength-strings", "-I", str(generated),
            "-c", str(source), "-o", str(obj),
        ])
        objects.append(obj)

    cpp_flags = [
        cxx, f"-std={standard}", "-pedantic-errors", "-Wall",
        "-Wno-variadic-macros", "-Wno-c++11-long-long",
        "-I", str(checkout), "-I", str(checkout / "backends"),
    ]
    smoke_defines = [
        "-DIMGUI_TRANSLATED_HAVE_DEMO=1",
        "-DIMGUI_TRANSLATED_HAVE_NULL_BACKEND=0",
        "-DIMGUI_TRANSLATED_HAVE_SDL3_BACKEND=0",
    ]
    native_executable = output / "native_smoke"
    run(cpp_flags + [
        *smoke_defines,
        str(root / "translator/fixtures/baseline_smoke.cpp"),
        *(str(checkout / source) for source in CORE_SOURCES),
        "-lm", "-o", str(native_executable),
    ])
    run([str(native_executable)])
    wrapper = generated / "imgui_translated_wrapper.o"
    run(cpp_flags + [
        "-Wno-return-type-c-linkage", "-I", str(generated), "-c",
        str(generated / "imgui_translated_wrapper.cpp"), "-o", str(wrapper),
    ])
    smoke = generated / "matrix_smoke.o"
    run(cpp_flags + [
        *smoke_defines, "-c",
        str(root / "translator/fixtures/baseline_smoke.cpp"),
        "-o", str(smoke),
    ])
    executable = generated / "matrix_smoke"
    run([
        cxx, str(smoke), str(wrapper),
        *(str(item) for item in objects), "-lm", "-o", str(executable),
    ])
    run([str(executable)])

    ir = json.loads(program.read_text(encoding="utf-8"))
    return {
        "family": family,
        "tag": selected["tag"],
        "commit": selected["commit"],
        "source_standard": standard,
        "records": len(ir["records"]),
        "functions": len({item["id"] for item in ir["functions"]}),
        "c89_translation_units": len(objects),
        "generated_from_source": True,
        "deterministic_generation": True,
        "strict_c89_compile_passed": True,
        "demo_linked": True,
        "demo_exercised": True,
        "native_smoke_passed": True,
        "cpp_facade_smoke_passed": True,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--family", action="append",
        help="check only this locked family (repeatable; default is all)",
    )
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    build = root / "build/translator"
    extractor = build / "imgui-clang-extract"
    if not extractor.exists():
        raise RuntimeError("run `make translator-check` first")
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    selected_rows = lock["compatibility_matrix"]
    if args.family:
        requested = set(args.family)
        selected_rows = [
            row for row in selected_rows if row["family"] in requested
        ]
        found = {row["family"] for row in selected_rows}
        if found != requested:
            parser.error(f"unknown locked families: {sorted(requested - found)}")

    llvm_prefix = Path(os.environ.get("LLVM_PREFIX", "/opt/homebrew/opt/llvm"))
    resource = capture([str(llvm_prefix / "bin/clang"), "-print-resource-dir"])
    sdk = capture(["xcrun", "--show-sdk-path"])
    summary = []
    for selected in selected_rows:
        row = check_revision(root, selected, extractor, resource, sdk)
        summary.append(row)
        print(json.dumps(row, sort_keys=True), flush=True)

    output_root = build / "matrix"
    output_root.mkdir(parents=True, exist_ok=True)
    (output_root / "summary.json").write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    markdown = [
        "# Ten-family Dear ImGui translation gate",
        "",
        "Each locked family is extracted from upstream source, generated "
        "twice and compared byte-for-byte, compiled as five independent "
        "strict-C89 units, and run through the source-identical demo-enabled "
        "smoke in both native C++ and exact C++-facade/C89 modes.",
        "",
        "| Family | Tag | C++ standard | C89 units | Native | C89 facade |",
        "|---|---|---|---:|---:|---:|",
    ]
    markdown.extend(
        f"| {row['family']} | {row['tag']} | {row['source_standard']} | "
        f"{row['c89_translation_units']} | PASS | PASS |"
        for row in summary
    )
    markdown.extend(["", f"All {len(summary)} families passed.", ""])
    (output_root / "summary.md").write_text(
        "\n".join(markdown), encoding="utf-8", newline="\n"
    )
    print(f"{len(summary)}-family full translation matrix: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
