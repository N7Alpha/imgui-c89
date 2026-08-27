#!/usr/bin/env python3
"""Build and measure a reachable embedded Dear ImGui C89 application."""

from __future__ import annotations

import json
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path


CORE_STEMS = ("imgui", "imgui_draw", "imgui_tables", "imgui_widgets")
SIZE_FLAGS = (
    "-Oz", "-flto", "-ffunction-sections", "-fdata-sections",
    "-fvisibility=hidden", "-fno-vectorize", "-fno-slp-vectorize",
    "-fno-unwind-tables",
    "-fno-asynchronous-unwind-tables", "-fomit-frame-pointer",
    "-fno-stack-protector", "-mllvm", "-enable-machine-outliner=always",
    "-mllvm", "-ir-outliner", "-mllvm", "-jump-threading-threshold=0",
    "-mllvm", "-inline-threshold=6",
    "-fno-math-errno", "-fno-trapping-math", "-fno-signed-zeros",
)


def capture(arguments: list[str]) -> str:
    return subprocess.run(
        arguments, check=True, text=True, stdout=subprocess.PIPE
    ).stdout.strip()


def run(arguments: list[str]) -> None:
    print("+", " ".join(arguments), flush=True)
    subprocess.run(arguments, check=True)


def dependency_flags(profile: dict[str, object]) -> list[str]:
    result: list[str] = []
    for package in profile.get("pkg_config_packages", []):
        flags = shlex.split(capture(["pkg-config", "--cflags", str(package)]))
        index = 0
        while index < len(flags):
            flag = flags[index]
            if flag == "-I" and index + 1 < len(flags):
                result.extend(("-isystem", flags[index + 1]))
                index += 2
            elif flag.startswith("-I"):
                result.extend(("-isystem", flag[2:]))
                index += 1
            else:
                result.append(flag)
                index += 1
    return result


def darwin_sections(executable: Path) -> dict[str, int]:
    output = capture(["size", "-m", str(executable)])
    sections: dict[str, int] = {}
    for line in output.splitlines():
        match = re.match(r"\s*Section (__[^:]+): (\d+)", line)
        if match:
            name = match.group(1)
            sections[name] = sections.get(name, 0) + int(match.group(2))
    if "__text" not in sections:
        raise RuntimeError("could not find Mach-O __text section")
    return sections


def profile_size(
    root: Path,
    profile_name: str,
    cc: str,
    *,
    generated: Path | None = None,
    output_name: str | None = None,
    extra_size_flags: tuple[str, ...] = (),
) -> dict[str, object]:
    profile = json.loads(
        (root / "translator/profiles" / f"{profile_name}.json").read_text()
    )
    if generated is None:
        generated = root / "build/translator" / profile_name / "generated-a"
    if not (generated / "imgui_c89_api.c").exists():
        raise SystemExit(
            f"generated {profile_name} missing; build the profile first"
        )
    output = root / "build/embedded-size" / (output_name or profile_name)
    output.mkdir(parents=True, exist_ok=True)
    dependencies = dependency_flags(profile)
    define_flags = ["-D" + value for value in profile.get("defines", [])]
    undefine_flags = ["-U" + value for value in profile.get("undefines", [])]
    common = [
        cc, "-std=c89", "-pedantic-errors", *SIZE_FLAGS,
        *extra_size_flags, "-w",
        *define_flags, *undefine_flags, "-I", str(generated), *dependencies,
    ]
    objects: list[Path] = []
    for stem in (*CORE_STEMS, "imgui_c89_api"):
        source = generated / f"{stem}.c"
        object_path = output / f"{stem}.o"
        run([*common, "-c", str(source), "-o", str(object_path)])
        objects.append(object_path)

    executable = output / "embedded_smoke"
    map_path = output / "embedded_smoke.map"
    linker_flags = (
        ["-Wl,-dead_strip", f"-Wl,-map,{map_path}"]
        if sys.platform == "darwin"
        else ["-Wl,--gc-sections", f"-Wl,-Map,{map_path}"]
    )
    run([
        *common, str(root / "translator/fixtures/embedded_smoke.c"),
        *(str(path) for path in objects), *linker_flags, "-lm",
        "-o", str(executable),
    ])
    run([str(executable)])
    if sys.platform != "darwin":
        raise SystemExit("embedded size section parsing currently requires Mach-O")
    sections = darwin_sections(executable)
    manifest_path = generated / "manifest.json"
    manifest = (
        json.loads(manifest_path.read_text(encoding="utf-8"))
        if manifest_path.exists() else {}
    )
    compact_truetype_only = manifest.get(
        "compact_truetype_only",
        profile.get("translator_options", {}).get(
            "compact_truetype_only", False
        ),
    )
    return {
        "profile": profile_name,
        "compiler_flags": [*SIZE_FLAGS, *extra_size_flags],
        "code_bytes": sections["__text"],
        "section_bytes": sum(sections.values()),
        "physical_file_bytes": executable.stat().st_size,
        "sections": sections,
        "runtime_ttf": True,
        "font_outline_formats": (
            ["TrueType glyf"]
            if compact_truetype_only
            else ["TrueType glyf", "OpenType CFF/Type2"]
        ),
        "widget_families_removed": 0,
        "map": str(map_path.relative_to(root)),
    }


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    cc = os.environ.get("CC", "cc")
    extra_size_flags = tuple(
        shlex.split(os.environ.get("IMGUI_C89_EXTRA_SIZE_FLAGS", ""))
    )
    results = [
        profile_size(root, name, cc, extra_size_flags=extra_size_flags)
        for name in ("baseline", "embedded", "embedded_compact")
    ]
    baseline, embedded, compact = results
    code_delta = int(embedded["code_bytes"]) - int(baseline["code_bytes"])
    section_delta = int(embedded["section_bytes"]) - int(baseline["section_bytes"])
    report = "\n".join((
        "# Embedded Dear ImGui reachable-size report",
        "",
        "The fixture creates a context, loads `third_party/ProggyClean.ttf` through Dear ImGui's runtime TTF parser/rasterizer, renders one window and text, and destroys the context. This is not a pre-rendered atlas. No widget family is removed. All profiles use strict C89, `-Oz`, LTO, LLVM IR and machine outlining, function/data sections, hidden visibility, no unwind tables, and linker dead stripping.",
        "",
        "| Profile | Instruction bytes | Code + data sections | Executable file |",
        "|---|---:|---:|---:|",
        f"| literal baseline | {baseline['code_bytes']:,} | {baseline['section_bytes']:,} | {baseline['physical_file_bytes']:,} |",
        f"| embedded | {embedded['code_bytes']:,} | {embedded['section_bytes']:,} | {embedded['physical_file_bytes']:,} |",
        f"| embedded compact | {compact['code_bytes']:,} | {compact['section_bytes']:,} | {compact['physical_file_bytes']:,} |",
        "",
        f"Embedded delta: {code_delta:+,} instruction bytes and {section_delta:+,} total section bytes.",
        f"Compact profile margin below 100 KiB: {102400 - int(compact['section_bytes']):+,} code-and-data bytes.",
        "",
        "The embedded profile removes assertions and requires the application to supply a TTF instead of linking Dear ImGui's compressed built-in fonts. Lossless compact lowerings coalesce zero-initialization in selected core constructors, promote immutable local tables to static data, encode the cursor atlas at two bits per pixel symbol, store key-name pointers as 16-bit string offsets, use a 16-entry nibble CRC table, and lower typed ImVector growth operations to a shared element-size-driven C89 runtime. The compact profile requires the host to provide the optional open-in-shell platform callback instead of linking Dear ImGui's desktop fork/exec default. It retains context hooks, malformed-scope recovery machinery, and the navigation core needed by differential-tested programmatic focus while making directional/gamepad/window-switching navigation, INI persistence, and interactive debug tools selectable modules. It retains runtime TrueType glyf parsing/rasterization, removes the unreachable CFF-only cubic tessellator, leaves CFF/Type2 support in the full profile, and turns recoverable user errors into one-instruction debug traps without format strings. Every widget implementation, the native C API, and the exact C++ facade are still generated. The literal profile retains full behavior and remains the oracle. Only code reachable from this fixture is present in the measured executable.",
        "",
    ))
    output = root / "build/embedded-size"
    (output / "report.json").write_text(
        json.dumps(results, indent=2, sort_keys=True) + "\n"
    )
    (output / "report.md").write_text(report)
    if int(compact["section_bytes"]) >= 100 * 1024:
        raise RuntimeError(
            "embedded compact profile exceeds 100 KiB code+data: "
            f"{compact['section_bytes']} bytes"
        )
    print(report, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
