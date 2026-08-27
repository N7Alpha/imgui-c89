#!/usr/bin/env python3
"""Run a real SDL3/SDL_GPU multi-viewport lifecycle natively and translated."""

from __future__ import annotations

import json
import shlex
import subprocess
from pathlib import Path


def run(arguments: list[str]) -> None:
    print("+", " ".join(arguments), flush=True)
    subprocess.run(arguments, check=True)


def capture(arguments: list[str]) -> str:
    return subprocess.run(
        arguments, check=True, text=True, stdout=subprocess.PIPE
    ).stdout.strip()


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    profile_name = "docking"
    profile = json.loads((root / "translator/profiles/docking.json").read_text())
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    upstream = root / "build/upstream" / lock["docking"]["commit"]
    generated = root / "build/translator/docking/generated-a"
    if not (generated / "imgui_impl_sdlgpu3.c").exists():
        run(["python3", str(root / "translator/build_baseline.py"),
             "--profile", profile_name])
        run(["python3", str(root / "translator/check_baseline.py"),
             "--profile", profile_name])
    cflags = shlex.split(capture(["pkg-config", "--cflags", "sdl3"]))
    libs = shlex.split(capture(["pkg-config", "--libs", "sdl3"]))
    output = root / "build/translator/docking/viewport-sdlgpu"
    output.mkdir(parents=True, exist_ok=True)
    fixture = root / "translator/fixtures/sdlgpu_viewport.cpp"
    cxx_common = [
        "c++", "-std=c++11", "-pedantic-errors", "-Wall", "-Wextra",
        "-I", str(upstream), "-I", str(upstream / "backends"), *cflags,
    ]

    native_objects: list[Path] = []
    for source_name in (
        "imgui.cpp", "imgui_draw.cpp", "imgui_tables.cpp", "imgui_widgets.cpp",
        "backends/imgui_impl_sdl3.cpp", "backends/imgui_impl_sdlgpu3.cpp",
    ):
        output_object = output / (Path(source_name).stem + ".native.o")
        run(cxx_common + [
            "-c", str(upstream / source_name), "-o", str(output_object),
        ])
        native_objects.append(output_object)
    native_fixture = output / "fixture.native.o"
    run(cxx_common + ["-c", str(fixture), "-o", str(native_fixture)])
    native = output / "native"
    run(["c++", str(native_fixture), *(str(item) for item in native_objects),
         *libs, "-lm", "-o", str(native)])
    native_result = capture([str(native)])
    print("native:", native_result)

    translated_objects: list[Path] = []
    for unit in json.loads((generated / "translation_units.json").read_text()):
        source = generated / unit["source"]
        output_object = output / (source.stem + ".translated.o")
        run([
            "cc", "-std=c89", "-pedantic-errors", "-w", "-I", str(generated),
            *cflags, "-c", str(source), "-o", str(output_object),
        ])
        translated_objects.append(output_object)
    wrapper = output / "wrapper.translated.o"
    run(cxx_common + [
        "-Wno-return-type-c-linkage", "-I", str(generated), "-c",
        str(generated / "imgui_translated_wrapper.cpp"), "-o", str(wrapper),
    ])
    translated_fixture = output / "fixture.translated.o"
    run(cxx_common + ["-c", str(fixture), "-o", str(translated_fixture)])
    translated = output / "translated"
    run(["c++", str(translated_fixture), str(wrapper),
         *(str(item) for item in translated_objects), *libs, "-lm",
         "-o", str(translated)])
    translated_result = capture([str(translated)])
    print("translated:", translated_result)
    if translated_result != native_result:
        raise RuntimeError(
            f"native/translated viewport results differ: "
            f"{native_result!r} != {translated_result!r}"
        )
    print("real SDL_GPU multi-viewport differential: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
