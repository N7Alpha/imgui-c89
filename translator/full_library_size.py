#!/usr/bin/env python3
"""Measure the complete latest-release core with like-for-like size flags."""

from __future__ import annotations

import json
import os
import subprocess
from pathlib import Path


CORE_STEMS = ("imgui", "imgui_draw", "imgui_tables", "imgui_widgets")
COMMON_SIZE_FLAGS = (
    "-Os",
    "-fno-exceptions",
    "-fno-unwind-tables",
    "-fno-asynchronous-unwind-tables",
)


def run(arguments: list[str]) -> None:
    print("+", " ".join(arguments), flush=True)
    subprocess.run(arguments, check=True)


def capture(arguments: list[str]) -> str:
    return subprocess.run(
        arguments, check=True, text=True, stdout=subprocess.PIPE
    ).stdout.strip()


def section_bytes(path: Path, size_tool: str) -> int:
    lines = capture([size_tool, str(path)]).splitlines()
    if len(lines) < 2:
        raise RuntimeError(f"cannot parse size output for {path}")
    fields = lines[-1].split()
    if fields[-1] in {str(path), path.name}:
        fields.pop()
    return int(fields[-2])


def archive(ar: str, path: Path, objects: list[Path]) -> None:
    if path.exists():
        path.unlink()
    run([ar, "rcs", str(path), *(str(item) for item in objects)])


def object_row(path: Path, size_tool: str) -> dict[str, int | str]:
    return {
        "name": path.name,
        "file_bytes": path.stat().st_size,
        "section_bytes": section_bytes(path, size_tool),
    }


def build_cpp(
    root: Path, upstream: Path, output: Path, cxx: str, ar: str,
    size_tool: str,
) -> dict[str, object]:
    output.mkdir(parents=True, exist_ok=True)
    objects: list[Path] = []
    for stem in CORE_STEMS:
        obj = output / f"{stem}.o"
        run([
            cxx, "-std=c++11", "-pedantic-errors", *COMMON_SIZE_FLAGS,
            "-I", str(upstream), "-c", str(upstream / f"{stem}.cpp"),
            "-o", str(obj),
        ])
        objects.append(obj)
    library = output / "libimgui.a"
    archive(ar, library, objects)
    rows = [object_row(item, size_tool) for item in objects]
    return {
        "kind": "upstream C++",
        "objects": rows,
        "object_file_bytes": sum(int(row["file_bytes"]) for row in rows),
        "section_bytes": sum(int(row["section_bytes"]) for row in rows),
        "archive_bytes": library.stat().st_size,
    }


def build_c89(
    root: Path, upstream: Path, generated: Path, output: Path,
    cc: str, cxx: str, ar: str, size_tool: str, label: str,
) -> dict[str, object]:
    output.mkdir(parents=True, exist_ok=True)
    core_objects: list[Path] = []
    for stem in CORE_STEMS:
        obj = output / f"{stem}.o"
        run([
            cc, "-std=c89", "-pedantic-errors", *COMMON_SIZE_FLAGS, "-w",
            "-I", str(generated), "-c", str(generated / f"{stem}.c"),
            "-o", str(obj),
        ])
        core_objects.append(obj)
    api_object = output / "imgui_c89_api.o"
    run([
        cc, "-std=c89", "-pedantic-errors", *COMMON_SIZE_FLAGS, "-w",
        "-I", str(generated), "-c", str(generated / "imgui_c89_api.c"),
        "-o", str(api_object),
    ])
    wrapper_object = output / "imgui_translated_wrapper.o"
    run([
        cxx, "-std=c++11", "-pedantic-errors", *COMMON_SIZE_FLAGS,
        "-Wno-return-type-c-linkage", "-I", str(upstream),
        "-I", str(generated), "-c",
        str(generated / "imgui_translated_wrapper.cpp"),
        "-o", str(wrapper_object),
    ])
    core_library = output / "libimgui_c89_core.a"
    c_api_library = output / "libimgui_c89.a"
    cpp_facade_library = output / "libimgui_c89_cpp_facade.a"
    archive(ar, core_library, core_objects)
    archive(ar, c_api_library, [*core_objects, api_object])
    archive(ar, cpp_facade_library, [*core_objects, wrapper_object])
    rows = [object_row(item, size_tool) for item in core_objects]
    api = object_row(api_object, size_tool)
    wrapper = object_row(wrapper_object, size_tool)
    return {
        "kind": label,
        "objects": rows,
        "object_file_bytes": sum(int(row["file_bytes"]) for row in rows),
        "section_bytes": sum(int(row["section_bytes"]) for row in rows),
        "archive_bytes": core_library.stat().st_size,
        "c_api": {
            "adapter": api,
            "archive_bytes": c_api_library.stat().st_size,
            "section_bytes": int(api["section_bytes"])
            + sum(int(row["section_bytes"]) for row in rows),
        },
        "cpp_facade": {
            "adapter": wrapper,
            "archive_bytes": cpp_facade_library.stat().st_size,
            "section_bytes": int(wrapper["section_bytes"])
            + sum(int(row["section_bytes"]) for row in rows),
        },
    }


def markdown(report: dict[str, object]) -> str:
    variants = report["variants"]
    cpp, baseline, idiomatic = variants  # type: ignore[misc]
    core_saved = int(baseline["section_bytes"]) - int(idiomatic["section_bytes"])
    archive_saved = int(baseline["archive_bytes"]) - int(idiomatic["archive_bytes"])
    lines = [
        "# Idiomatic C89 Dear ImGui full-library size",
        "",
        f"Upstream `{report['upstream_tag']}` at `{report['upstream_commit']}`; "
        f"compiler `{report['compiler']}`. All objects use `{' '.join(COMMON_SIZE_FLAGS)}`. "
        "The four core translation units are the like-for-like implementation comparison; "
        "demo and backends are excluded.",
        "",
        "| Variant | Core archive bytes | Core object bytes | Loaded section bytes |",
        "|---|---:|---:|---:|",
    ]
    for variant in variants:  # type: ignore[union-attr]
        lines.append(
            f"| {variant['kind']} | {variant['archive_bytes']:,} | "
            f"{variant['object_file_bytes']:,} | {variant['section_bytes']:,} |"
        )
    lines.extend([
        "",
        f"Overlay saving versus literal C89: **{archive_saved:,} archive bytes** and "
        f"**{core_saved:,} loaded section bytes** "
        f"({100.0 * core_saved / int(baseline['section_bytes']):.2f}%).",
        "",
        "| Optional compatibility surface | Literal C89 archive | Idiomatic C89 archive | Literal sections | Idiomatic sections |",
        "|---|---:|---:|---:|---:|",
        f"| Exact C API + optional scope helpers | {baseline['c_api']['archive_bytes']:,} | "
        f"{idiomatic['c_api']['archive_bytes']:,} | {baseline['c_api']['section_bytes']:,} | "
        f"{idiomatic['c_api']['section_bytes']:,} |",
        f"| Exact C++ facade + core | {baseline['cpp_facade']['archive_bytes']:,} | "
        f"{idiomatic['cpp_facade']['archive_bytes']:,} | {baseline['cpp_facade']['section_bytes']:,} | "
        f"{idiomatic['cpp_facade']['section_bytes']:,} |",
        "",
        "The exact C API lives in the core definitions. The small C helper object contains only "
        "the three optional normalized Begin/BeginChild scope helpers. The C++ facade is reported "
        "separately because C-only applications do not ship it.",
        "",
    ])
    return "\n".join(lines)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    release = lock["latest_release"]
    upstream = root / "build/upstream" / release["commit"]
    generated_root = root / "build/translator"
    baseline = generated_root / "latest_release/generated-a"
    idiomatic = generated_root / "idiomatic_c89/generated-a"
    for path in (upstream, baseline, idiomatic):
        if not path.exists():
            raise SystemExit(f"missing prerequisite: {path}")
    cc = os.environ.get("CC", "/opt/homebrew/opt/llvm/bin/clang")
    cxx = os.environ.get("CXX", "/opt/homebrew/opt/llvm/bin/clang++")
    ar = os.environ.get("AR", "ar")
    default_size = Path("/opt/homebrew/opt/llvm/bin/llvm-size")
    size_tool = os.environ.get(
        "SIZE", str(default_size) if default_size.exists() else "size"
    )
    output = root / "build/full-library-size"
    variants = [
        build_cpp(root, upstream, output / "cpp", cxx, ar, size_tool),
        build_c89(
            root, upstream, baseline, output / "c89-literal", cc, cxx,
            ar, size_tool, "literal translated C89",
        ),
        build_c89(
            root, upstream, idiomatic, output / "c89-idiomatic", cc, cxx,
            ar, size_tool, "idiomatic C89",
        ),
    ]
    report: dict[str, object] = {
        "upstream_tag": release["tag"],
        "upstream_commit": release["commit"],
        "compiler": capture([cc, "--version"]).splitlines()[0],
        "common_flags": list(COMMON_SIZE_FLAGS),
        "variants": variants,
    }
    if int(variants[2]["section_bytes"]) >= int(variants[1]["section_bytes"]):
        raise RuntimeError("idiomatic overlay did not reduce C89 section bytes")
    output.mkdir(parents=True, exist_ok=True)
    (output / "report.json").write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    text = markdown(report)
    (output / "report.md").write_text(text, encoding="utf-8")
    print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
