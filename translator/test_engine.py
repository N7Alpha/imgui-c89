#!/usr/bin/env python3
"""Build and run the pinned official Dear ImGui Test Engine control pair."""

from __future__ import annotations

import argparse
import json
import os
import platform
import re
import subprocess
import sys
from pathlib import Path

from check_baseline import assert_same_outputs


ANSI = re.compile(r"\x1b\[[0-9;]*m")
RESULT = re.compile(r"\((\d+)/(\d+) tests passed\)")


def run(arguments: list[str], label: str) -> None:
    print(f"+ {label}", flush=True)
    subprocess.run(arguments, check=True)


def run_suite(executable: Path, cwd: Path, log: Path, label: str,
              extra_arguments: list[str] | None = None,
              test_filter: str = "tests") -> tuple[int, int]:
    print(f"+ {label}", flush=True)
    result = subprocess.run(
        [str(executable), "-nogui", "-nopause", "-v2", "-ve4",
         *(extra_arguments or []), test_filter],
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    log.write_text(result.stdout, encoding="utf-8")
    clean = ANSI.sub("", result.stdout)
    matches = RESULT.findall(clean)
    if result.returncode or not matches:
        print("\n".join(clean.splitlines()[-200:]), file=sys.stderr)
        raise RuntimeError(f"{label} failed; see {log}")
    passed, total = map(int, matches[-1])
    if passed != total:
        print("\n".join(clean.splitlines()[-200:]), file=sys.stderr)
        raise RuntimeError(f"{label}: {passed}/{total}; see {log}")
    print(f"  PASS {passed}/{total} (full log: {log})", flush=True)
    return passed, total


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", default="test_engine")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    profile = json.loads(
        (root / "translator/profiles" / f"{args.profile}.json").read_text()
    )
    revision = profile.get("upstream_revision", "baseline")
    upstream = root / "build/upstream" / lock[revision]["commit"]
    test_engine = (
        root / "build/upstream-test-engine" / lock["test_engine"]["commit"]
    )
    profile_dir = root / "build/translator" / args.profile
    ir = profile_dir / "program.ir.json"
    if not ir.exists():
        raise SystemExit(
            "test-engine IR missing; run `python3 translator/build_baseline.py "
            f"--profile {args.profile}`"
        )

    translator = root / "translator/translate.py"
    generated_a = profile_dir / "generated-a"
    generated_b = profile_dir / "generated-b"
    run([sys.executable, str(translator), str(ir), str(generated_a)],
        "generate test-enabled C89 core (pass 1)")
    run([sys.executable, str(translator), str(ir), str(generated_b)],
        "generate test-enabled C89 core (determinism pass)")
    assert_same_outputs(generated_a, generated_b)

    cc = os.environ.get("CC", "cc")
    cxx = os.environ.get("CXX", "c++")
    define_flags = [
        "-DIMGUI_TEST_ENGINE_ENABLE_IMPLOT=0",
        '-DIMGUI_USER_CONFIG="imgui_test_suite/imgui_test_suite_imconfig.h"',
    ]
    include_flags = [
        "-I" + str(test_engine),
        "-I" + str(test_engine / "imgui_test_engine"),
        "-I" + str(test_engine / "imgui_test_suite"),
        "-I" + str(test_engine / "shared"),
        "-I" + str(upstream),
        "-I" + str(upstream / "backends"),
    ]
    cpp_flags = ["-std=" + profile["language"], "-O1", "-g", "-pthread",
                 *define_flags, *include_flags]
    link_flags = ["-pthread"]
    if platform.system() == "Darwin":
        link_flags.extend(["-framework", "CoreFoundation"])
    elif platform.system() == "Linux":
        link_flags.append("-ldl")

    engine_sources = sorted((test_engine / "imgui_test_engine").glob("*.cpp"))
    suite_sources = [test_engine / "imgui_test_suite/imgui_test_suite.cpp"]
    suite_sources.extend(sorted(
        (test_engine / "imgui_test_suite").glob("imgui_tests_*.cpp")
    ))
    harness_sources = engine_sources + suite_sources + [
        test_engine / "shared/imgui_app.cpp"
    ]
    core_sources = [
        upstream / name for name in (
            "imgui.cpp", "imgui_draw.cpp", "imgui_tables.cpp",
            "imgui_widgets.cpp", "imgui_demo.cpp",
        )
    ]

    output_root = root / "build/test-engine" / args.profile
    native_dir = output_root / "native"
    translated_dir = output_root / "translated"
    native_dir.mkdir(parents=True, exist_ok=True)
    translated_dir.mkdir(parents=True, exist_ok=True)
    native_exe = native_dir / "imgui_test_suite"
    run([
        cxx, *cpp_flags, *(str(item) for item in harness_sources + core_sources),
        *link_flags, "-o", str(native_exe),
    ], f"compile native Test Engine control ({len(harness_sources + core_sources)} C++ files)")

    units = json.loads((generated_a / "translation_units.json").read_text())
    c_objects: list[Path] = []
    c_warnings = [
        "-Wall", "-Wno-overlength-strings", "-Wno-parentheses-equality",
        "-Wno-unused-value", "-Wno-unused-function", "-Wno-unused-variable",
        "-Wno-unused-but-set-variable", "-Wno-unused-label",
    ]
    for item in units:
        source = generated_a / item["source"]
        obj = source.with_suffix(".o")
        run([
            cc, "-std=c89", "-pedantic-errors", *c_warnings,
            "-I" + str(generated_a), "-c", str(source), "-o", str(obj),
        ], f"compile strict-C89 {source.name}")
        c_objects.append(obj)

    wrapper_obj = generated_a / "imgui_translated_wrapper.o"
    run([
        cxx, *cpp_flags, "-I" + str(generated_a),
        "-Wno-return-type-c-linkage", "-c",
        str(generated_a / "imgui_translated_wrapper.cpp"),
        "-o", str(wrapper_obj),
    ], "compile generated exact C++ facade and callback bridges")
    translated_exe = translated_dir / "imgui_test_suite"
    run([
        cxx, *cpp_flags, *(str(item) for item in harness_sources),
        str(wrapper_obj), *(str(item) for item in c_objects),
        *link_flags, "-o", str(translated_exe),
    ], f"link translated Test Engine ({len(c_objects)} strict-C89 core units)")

    native_result = run_suite(
        native_exe, test_engine, output_root / "native.log",
        "run native official regression suite",
    )
    translated_result = run_suite(
        translated_exe, test_engine, output_root / "translated.log",
        "run translated official regression suite",
    )
    if native_result != translated_result:
        raise RuntimeError(
            f"native/translated test counts differ: "
            f"{native_result} vs {translated_result}"
        )
    viewport_result: tuple[int, int] | None = None
    if profile.get("test_viewports"):
        native_viewport = run_suite(
            native_exe, test_engine, output_root / "native-viewport.log",
            "run native official viewport-mock suite", ["-viewport-mock"],
            "viewport",
        )
        translated_viewport = run_suite(
            translated_exe, test_engine,
            output_root / "translated-viewport.log",
            "run translated official viewport-mock suite", ["-viewport-mock"],
            "viewport",
        )
        if native_viewport != translated_viewport:
            raise RuntimeError(
                "native/translated viewport test counts differ: "
                f"{native_viewport} vs {translated_viewport}"
            )
        viewport_result = translated_viewport
    print(
        f"Dear ImGui Test Engine: PASS ({translated_result[0]}/"
        f"{translated_result[1]} native and translated"
        + (f", viewport mock {viewport_result[0]}/{viewport_result[1]}"
           if viewport_result else "") + ")",
        flush=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
