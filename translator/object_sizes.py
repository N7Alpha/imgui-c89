#!/usr/bin/env python3
"""Compile native and translated units with -Os and report object sizes."""

from __future__ import annotations

import argparse
import json
import os
import shlex
import subprocess
from pathlib import Path

C89_SIZE_FLAGS = [
    "-Os",
    # C has no exception unwinding. These tables are useful to some profilers
    # and backtrace implementations, but cost tens of kilobytes in Dear ImGui
    # even after all dead code is stripped.
    "-fno-unwind-tables",
    "-fno-asynchronous-unwind-tables",
]


def capture(arguments: list[str]) -> str:
    return subprocess.run(
        arguments, check=True, text=True, stdout=subprocess.PIPE
    ).stdout.strip()


def run(arguments: list[str], label: str) -> None:
    print(f"+ {label}", flush=True)
    subprocess.run(arguments, check=True)


def section_bytes(path: Path, size_tool: str) -> int:
    lines = capture([size_tool, str(path)]).splitlines()
    if len(lines) < 2:
        raise RuntimeError(f"cannot parse size output for {path}")
    fields = lines[-1].split()
    # Darwin's size table ends in `dec hex`; GNU/Berkeley tables use the
    # same final pair (with an optional filename after it).
    if fields[-1] == str(path) or fields[-1] == path.name:
        fields.pop()
    return int(fields[-2])


def profile_sizes(root: Path, profile_name: str, cc: str, cxx: str,
                  size_tool: str) -> dict[str, object]:
    profile = json.loads(
        (root / "translator/profiles" / f"{profile_name}.json").read_text()
    )
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    revision = profile.get("upstream_revision", profile_name)
    upstream = root / "build/upstream" / lock[revision]["commit"]
    generated = root / "build/translator" / profile_name / "generated-a"
    manifest_path = generated / "translation_units.json"
    if not manifest_path.exists():
        raise SystemExit(
            f"generated {profile_name} missing; build that profile first"
        )
    units = json.loads(manifest_path.read_text())
    generation_manifest = json.loads(
        (generated / "manifest.json").read_text()
    )
    inputs = (
        profile["core_translation_units"]
        + profile.get("compatibility_translation_units", [])
        + profile.get("backend_translation_units", [])
    )
    if len(inputs) != len(units):
        raise RuntimeError(f"{profile_name}: source/manifest length mismatch")

    dependency_flags: list[str] = []
    for package in profile.get("pkg_config_packages", []):
        dependency_flags.extend(shlex.split(capture([
            "pkg-config", "--cflags", package,
        ])))
    define_flags = ["-D" + value for value in profile.get("defines", [])]
    undefine_flags = ["-U" + value for value in profile.get("undefines", [])]
    output = root / "build/object-sizes" / profile_name
    native_dir = output / "native"
    translated_dir = output / "translated"
    wrapper_dir = output / "wrapper"
    native_dir.mkdir(parents=True, exist_ok=True)
    translated_dir.mkdir(parents=True, exist_ok=True)
    wrapper_dir.mkdir(parents=True, exist_ok=True)

    rows: list[dict[str, object]] = []
    for source_name, unit in zip(inputs, units):
        label = Path(source_name).name
        native_object = native_dir / (Path(source_name).stem + ".o")
        translated_source = generated / unit["source"]
        translated_object = translated_dir / translated_source.with_suffix(".o").name
        run([
            cxx, "-std=" + profile["language"], "-Os",
            "-I", str(upstream), "-I", str(upstream / "backends"),
            *define_flags, *undefine_flags, *dependency_flags,
            "-c", str(upstream / source_name), "-o", str(native_object),
        ], f"{profile_name}: native -Os {label}")
        run([
            cc, "-std=c89", "-pedantic-errors", *C89_SIZE_FLAGS, "-w",
            "-I", str(generated), *dependency_flags,
            "-c", str(translated_source), "-o", str(translated_object),
        ], f"{profile_name}: C89 -Os {translated_source.name}")
        native_file = native_object.stat().st_size
        translated_file = translated_object.stat().st_size
        rows.append({
            "translation_unit": label,
            "native_file_bytes": native_file,
            "translated_file_bytes": translated_file,
            "file_delta_bytes": translated_file - native_file,
            "native_section_bytes": section_bytes(native_object, size_tool),
            "translated_section_bytes": section_bytes(
                translated_object, size_tool
            ),
        })
        rows[-1]["section_delta_bytes"] = (
            int(rows[-1]["translated_section_bytes"])
            - int(rows[-1]["native_section_bytes"])
        )

    wrapper_object = wrapper_dir / "imgui_translated_wrapper.o"
    run([
        cxx, "-std=" + profile.get("facade_language", profile["language"]),
        "-Os", "-Wno-return-type-c-linkage",
        "-I", str(upstream), "-I", str(upstream / "backends"),
        "-I", str(generated), *define_flags, *undefine_flags,
        *dependency_flags, "-c",
        str(generated / "imgui_translated_wrapper.cpp"),
        "-o", str(wrapper_object),
    ], f"{profile_name}: generated C++ wrapper -Os (excluded)")

    return {
        "profile": profile_name,
        "native_compiler_flags": ["-Os"],
        "translated_compiler_flags": C89_SIZE_FLAGS,
        "internal_function_count": generation_manifest.get(
            "internal_function_count", 0
        ),
        "native_c_function_count": generation_manifest.get(
            "native_c_function_count", 0
        ),
        "constructor_helper_count": (
            generation_manifest.get("constructor_value_helper_count", 0)
            + generation_manifest.get("constructor_at_helper_count", 0)
        ),
        "inline_constructor_value_helper_count": generation_manifest.get(
            "inline_constructor_value_helper_count", 0
        ),
        "units": rows,
        "native_total_file_bytes": sum(
            int(row["native_file_bytes"]) for row in rows
        ),
        "translated_total_file_bytes": sum(
            int(row["translated_file_bytes"]) for row in rows
        ),
        "native_total_section_bytes": sum(
            int(row["native_section_bytes"]) for row in rows
        ),
        "translated_total_section_bytes": sum(
            int(row["translated_section_bytes"]) for row in rows
        ),
        "wrapper_excluded": {
            "file_bytes": wrapper_object.stat().st_size,
            "section_bytes": section_bytes(wrapper_object, size_tool),
        },
    }


def markdown(results: list[dict[str, object]]) -> str:
    lines = [
        "# Dear ImGui `-Os` object-size comparison",
        "",
        "Physical `.o` byte counts include Mach-O symbols and relocations; ",
        "section bytes come from `size`. The generated C++ facade is shown ",
        "separately and is excluded from every translated-C total.",
        "The C89 size build uses `-Os -fno-unwind-tables ",
        "-fno-asynchronous-unwind-tables`; omit the latter two flags when ",
        "platform backtraces or profilers require unwind metadata.",
        "",
    ]
    for result in results:
        lines.extend([
            f"## {result['profile']}", "",
            "| Translation unit | Native C++ `.o` | Translated C89 `.o` | Delta | Native sections | C89 sections |",
            "|---|---:|---:|---:|---:|---:|",
        ])
        for row in result["units"]:  # type: ignore[union-attr]
            lines.append(
                f"| {row['translation_unit']} | {row['native_file_bytes']:,} | "
                f"{row['translated_file_bytes']:,} | {row['file_delta_bytes']:+,} | "
                f"{row['native_section_bytes']:,} | {row['translated_section_bytes']:,} |"
            )
        file_delta = (
            int(result["translated_total_file_bytes"])
            - int(result["native_total_file_bytes"])
        )
        section_delta = (
            int(result["translated_total_section_bytes"])
            - int(result["native_total_section_bytes"])
        )
        file_percent = 100.0 * file_delta / int(
            result["native_total_file_bytes"]
        )
        section_percent = 100.0 * section_delta / int(
            result["native_total_section_bytes"]
        )
        positive_sections = sorted(
            (
                row for row in result["units"]  # type: ignore[union-attr]
                if int(row["section_delta_bytes"]) > 0
            ),
            key=lambda row: int(row["section_delta_bytes"]),
            reverse=True,
        )
        lines.extend([
            f"| **Total (wrapper excluded)** | **{result['native_total_file_bytes']:,}** | **{result['translated_total_file_bytes']:,}** | **{file_delta:+,}** | **{result['native_total_section_bytes']:,}** | **{result['translated_total_section_bytes']:,}** |",
            "",
            f"Excluded generated C++ facade: {result['wrapper_excluded']['file_bytes']:,} `.o` bytes, {result['wrapper_excluded']['section_bytes']:,} section bytes.",  # type: ignore[index]
            f"Translator linkage analysis made {result['internal_function_count']:,} functions translation-unit-local; the native C API exports {result['native_c_function_count']:,} non-template entry points.",
            f"Demand-driven lowering retained {result['constructor_helper_count']:,} constructor adapters.",
            f"Of those value adapters, {result['inline_constructor_value_helper_count']:,} header-defined field-only constructors are cloned translation-unit-locally to preserve C++ inline semantics.",
            f"Physical-byte delta (C89 - native): {file_delta:+,} ({file_percent:+.2f}%).",
            f"Section-byte delta (C89 - native): {section_delta:+,} ({section_percent:+.2f}%).",
            "Largest remaining positive section deltas: "
            + ", ".join(
                f"{row['translation_unit']} {row['section_delta_bytes']:+,}"
                for row in positive_sections[:3]
            )
            + ".",
            "",
        ])
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("profiles", nargs="*", default=["baseline", "docking"])
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    cc = os.environ.get("CC", "cc")
    cxx = os.environ.get("CXX", "c++")
    size_tool = os.environ.get("SIZE", "size")
    results = [
        profile_sizes(root, name, cc, cxx, size_tool)
        for name in args.profiles
    ]
    output = root / "build/object-sizes"
    output.mkdir(parents=True, exist_ok=True)
    (output / "report.json").write_text(
        json.dumps(results, indent=2, sort_keys=True) + "\n"
    )
    report = markdown(results)
    (output / "report.md").write_text(report)
    print(report, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
