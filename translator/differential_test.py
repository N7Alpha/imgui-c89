#!/usr/bin/env python3
"""Compile one C++ scenario against native and translated Dear ImGui and diff."""

from __future__ import annotations

import argparse
import difflib
import hashlib
import json
import os
import shlex
import subprocess
from pathlib import Path


def run(arguments: list[str], **kwargs: object) -> subprocess.CompletedProcess[str]:
    print("+", " ".join(arguments), flush=True)
    return subprocess.run(arguments, check=True, text=True, **kwargs)


def capture(arguments: list[str]) -> list[str]:
    result = subprocess.run(
        arguments, check=True, text=True, stdout=subprocess.PIPE
    )
    return shlex.split(result.stdout.strip())


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", default="baseline")
    parser.add_argument("--generated", type=Path)
    parser.add_argument("--output-name")
    parser.add_argument("--font", type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    profile = json.loads(
        (root / "translator/profiles" / f"{args.profile}.json").read_text()
    )
    revision = profile.get("upstream_revision", args.profile)
    upstream = root / "build/upstream" / lock[revision]["commit"]
    generated = (
        args.generated.resolve()
        if args.generated is not None
        else root / "build/translator" / args.profile / "generated-a"
    )
    output_name = args.output_name or (
        "differential" if args.profile == "baseline" else
        "differential-" + args.profile
    )
    output = root / "build/translator" / output_name
    harness = root / "translator/fixtures/differential_main.cpp"
    output.mkdir(parents=True, exist_ok=True)
    if not (generated / "translation_units.json").exists():
        raise SystemExit(
            f"generated {args.profile} missing; run `make translator-{args.profile}`"
        )
    cxx = os.environ.get("CXX", "c++")
    cc = os.environ.get("CC", "cc")
    define_flags = ["-D" + value for value in profile.get("defines", [])]
    undefine_flags = ["-U" + value for value in profile.get("undefines", [])]
    software_object = output / "imgui_c89_software.o"
    run([
        cc, "-std=c89", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
        "-I", str(root / "include"), "-c",
        str(root / "src/imgui_c89_software.c"), "-o", str(software_object),
    ])
    common = [
        cxx, "-std=" + profile.get("facade_language", profile["language"]),
        "-pedantic-errors", "-Wall", "-Wextra",
        "-Werror", "-Wno-return-type-c-linkage", "-I", str(upstream),
        *define_flags, *undefine_flags,
        "-I", str(root / "include"), str(harness),
    ]
    native = output / "native"
    run(common + [
        str(upstream / "imgui.cpp"), str(upstream / "imgui_draw.cpp"),
        str(upstream / "imgui_tables.cpp"),
        str(upstream / "imgui_widgets.cpp"),
        str(upstream / "imgui_demo.cpp"), str(software_object), "-lm", "-o",
        str(native),
    ])

    units = json.loads((generated / "translation_units.json").read_text())
    translated_cflags = shlex.split(
        os.environ.get("IMGUI_C89_TRANSLATED_CFLAGS", "")
    )
    translated_ldflags = shlex.split(
        os.environ.get("IMGUI_C89_TRANSLATED_LDFLAGS", "")
    )
    if translated_cflags:
        dependency_cflags = capture(["pkg-config", "--cflags", "sdl3"])
        objects = []
        for unit in units:
            source = generated / unit["source"]
            object_path = output / (source.stem + "-translated.o")
            run([
                cc, "-std=c89", "-pedantic-errors", "-w",
                *translated_cflags, *define_flags, *undefine_flags,
                "-I", str(generated), *dependency_cflags,
                "-c", str(source), "-o", str(object_path),
            ])
            objects.append(str(object_path))
    else:
        objects = [str((generated / unit["source"]).with_suffix(".o"))
                   for unit in units]
    sdl_flags = capture(["pkg-config", "--libs", "sdl3"])
    translated = output / "translated"
    run(common + [
        "-I", str(upstream / "backends"), "-I", str(generated),
        str(generated / "imgui_translated_wrapper.o"), *objects,
        str(software_object), "-lm", *sdl_flags, *translated_ldflags,
        "-o", str(translated),
    ])

    native_pixels = output / "native.rgba"
    translated_pixels = output / "translated.rgba"
    native_repeat_pixels = output / "native-repeat.rgba"
    translated_repeat_pixels = output / "translated-repeat.rgba"
    font_arguments = [str(args.font.resolve())] if args.font else []
    native_result = run(
        [str(native), str(native_pixels), *font_arguments],
        cwd=root, stdout=subprocess.PIPE,
    )
    translated_result = run(
        [str(translated), str(translated_pixels), *font_arguments], cwd=root,
        stdout=subprocess.PIPE,
    )
    native_repeat = run(
        [str(native), str(native_repeat_pixels), *font_arguments], cwd=root,
        stdout=subprocess.PIPE,
    )
    translated_repeat = run(
        [str(translated), str(translated_repeat_pixels), *font_arguments], cwd=root,
        stdout=subprocess.PIPE,
    )
    if native_result.stdout != native_repeat.stdout:
        raise SystemExit("native differential trace is non-deterministic")
    if translated_result.stdout != translated_repeat.stdout:
        raise SystemExit("translated differential trace is non-deterministic")
    native_pixel_bytes = native_pixels.read_bytes()
    translated_pixel_bytes = translated_pixels.read_bytes()
    if native_pixel_bytes != native_repeat_pixels.read_bytes():
        raise SystemExit("native framebuffer stream is non-deterministic")
    if translated_pixel_bytes != translated_repeat_pixels.read_bytes():
        raise SystemExit("translated framebuffer stream is non-deterministic")
    if native_pixel_bytes != translated_pixel_bytes:
        if len(native_pixel_bytes) != len(translated_pixel_bytes):
            raise SystemExit(
                "native and translated framebuffer stream sizes differ: "
                f"{len(native_pixel_bytes)} != {len(translated_pixel_bytes)}"
            )
        difference_index = next(
            index for index, (left, right) in enumerate(zip(
                native_pixel_bytes, translated_pixel_bytes
            )) if left != right
        )
        frame_size = 640 * 480 * 4
        render_index, byte_in_frame = divmod(difference_index, frame_size)
        pixel_index, channel = divmod(byte_in_frame, 4)
        y, x = divmod(pixel_index, 640)
        raise SystemExit(
            "native and translated framebuffers differ at "
            f"frame={render_index // 2} mode="
            f"{'atlas' if render_index % 2 == 0 else 'flat'} "
            f"x={x} y={y} channel={channel}"
        )
    native_trace = output / "native.trace"
    translated_trace = output / "translated.trace"
    native_trace.write_text(native_result.stdout, encoding="utf-8", newline="\n")
    translated_trace.write_text(
        translated_result.stdout, encoding="utf-8", newline="\n"
    )
    if native_result.stdout != translated_result.stdout:
        difference = "".join(difflib.unified_diff(
            native_result.stdout.splitlines(keepends=True),
            translated_result.stdout.splitlines(keepends=True),
            fromfile=str(native_trace), tofile=str(translated_trace), n=3,
        ))
        (output / "difference.diff").write_text(
            difference, encoding="utf-8", newline="\n"
        )
        print(difference[:12000])
        raise SystemExit("native and translated behavioral traces differ")
    trace_bytes = native_result.stdout.encode("utf-8")
    summary = {
        "bytes": len(trace_bytes),
        "canonical_records": len(native_result.stdout.splitlines()),
        "frames": len(native_pixel_bytes) // (640 * 480 * 4 * 2),
        "native_equals_translated": True,
        "pixel_bytes": len(native_pixel_bytes),
        "pixel_sha256": hashlib.sha256(native_pixel_bytes).hexdigest(),
        "repeatable": True,
        "profile": args.profile,
        "sha256": hashlib.sha256(trace_bytes).hexdigest(),
    }
    (output / "summary.json").write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n",
        encoding="utf-8", newline="\n",
    )
    print(
        f"differential {args.profile}: PASS "
        f"({summary['canonical_records']} canonical "
        f"state/draw/resource/pixel records, sha256={summary['sha256']})",
        flush=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
