#!/usr/bin/env python3
"""Extract and merge the pinned baseline one translation unit at a time."""

from __future__ import annotations

import argparse
import json
import os
import shlex
import subprocess
import sys
from pathlib import Path

from merge_ir import merge


def run(arguments: list[str]) -> None:
    print("+", " ".join(arguments), flush=True)
    subprocess.run(arguments, check=True)


def capture(arguments: list[str]) -> str:
    return subprocess.run(arguments, check=True, text=True,
                          stdout=subprocess.PIPE).stdout.strip()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", default="baseline")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    profile_path = root / "translator/profiles" / f"{args.profile}.json"
    profile = json.loads(profile_path.read_text())
    revision = profile.get("upstream_revision", args.profile)
    commit = lock[revision]["commit"]
    upstream = root / "build/upstream" / commit
    test_engine = (
        root / "build/upstream-test-engine" / lock["test_engine"]["commit"]
    )
    extractor = root / "build/translator/imgui-clang-extract"
    output = root / "build/translator" / args.profile
    llvm_prefix = Path(os.environ.get("LLVM_PREFIX", "/opt/homebrew/opt/llvm"))
    resource = capture([str(llvm_prefix / "bin/clang"), "-print-resource-dir"])
    platform_arguments: list[str] = []
    if sys.platform == "darwin":
        platform_arguments = [
            "-isysroot", capture(["xcrun", "--show-sdk-path"]),
        ]
    if not extractor.exists():
        raise SystemExit("extractor missing; run `make translator-check` first")
    if not upstream.exists():
        run([sys.executable, str(root / "translator/fetch_upstream.py"),
             "--revision", revision])
    if profile.get("needs_test_engine") and not test_engine.exists():
        run([sys.executable, str(root / "translator/fetch_upstream.py"),
             "--revision", "test_engine"])

    include_values = {
        "root": str(root),
        "upstream": str(upstream),
        "test_engine": str(test_engine),
    }
    extra_includes = [
        value.format(**include_values)
        for value in profile.get("include_directories", [])
    ]

    source_names = list(profile["core_translation_units"])
    source_names.extend(profile.get("compatibility_translation_units", []))
    source_names.extend(profile.get("backend_translation_units", []))
    dependency_cflags: list[str] = []
    for package in profile.get("pkg_config_packages", []):
        dependency_cflags.extend(shlex.split(capture([
            "pkg-config", "--cflags", package,
        ])))
    ir_paths: list[Path] = []
    output.mkdir(parents=True, exist_ok=True)
    for source_name in source_names:
        source = upstream / source_name
        ir_path = output / (source_name.replace("/", "_").replace(".", "_") + ".ir.json")
        arguments = [
            str(extractor), "--output", str(ir_path),
            "--source-root", str(upstream), str(source), "--",
            "-std=" + profile["language"], "-I" + str(upstream),
            "-I" + str(upstream / "backends"),
            "-resource-dir", resource, *platform_arguments,
            "-Wno-nontrivial-memcall",
        ]
        arguments.extend("-I" + value for value in extra_includes)
        arguments.extend(profile.get("compiler_flags", []))
        arguments.extend("-D" + value for value in profile.get("defines", []))
        arguments.extend("-U" + value for value in profile.get("undefines", []))
        arguments.extend(dependency_cflags)
        run(arguments)
        ir_paths.append(ir_path)

    merged_path = output / "program.ir.json"
    program = merge(ir_paths)
    program["c_includes"] = profile.get("c_includes", [])
    program["c_type_replacements"] = profile.get("c_type_replacements", {})
    translator_options = dict(profile.get("translator_options", {}))
    patch_name = profile.get("translator_patch")
    if patch_name:
        patch_path = (profile_path.parent / patch_name).resolve()
        patch = json.loads(patch_path.read_text(encoding="utf-8"))
        if patch.get("format_version") != 1:
            raise RuntimeError(
                f"unsupported translator patch format in {patch_path}"
            )
        patch_options = patch.get("translator_options")
        if not isinstance(patch_options, dict):
            raise RuntimeError(
                f"translator patch has no options object: {patch_path}"
            )
        duplicates = sorted(translator_options.keys() & patch_options.keys())
        if duplicates:
            raise RuntimeError(
                "profile and translator patch both set: "
                + ", ".join(duplicates)
            )
        translator_options.update(patch_options)
        program["translator_patch"] = {
            "format_version": patch["format_version"],
            "name": patch.get("name", patch_path.stem),
            "path": str(patch_path.relative_to(root)),
        }
    program["translator_options"] = translator_options
    merged_path.write_text(
        json.dumps(program, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(f"merged {len(ir_paths)} translation units -> {merged_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
