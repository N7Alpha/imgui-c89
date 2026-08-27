#!/usr/bin/env python3
"""Generate, compile, link, and exercise the pinned literal baseline."""

from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path


def run(arguments: list[str]) -> None:
    print("+", " ".join(arguments), flush=True)
    subprocess.run(arguments, check=True)


def capture(arguments: list[str]) -> str:
    return subprocess.run(
        arguments, check=True, text=True, stdout=subprocess.PIPE
    ).stdout.strip()


def system_dependency_flags(flags: list[str]) -> list[str]:
    """Treat dependency headers as system headers for strict-C89 checking."""
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


def assert_same_outputs(left: Path, right: Path) -> None:
    """Compare translator-owned outputs, ignoring later build artifacts."""
    left_units = json.loads((left / "translation_units.json").read_text())
    right_units = json.loads((right / "translation_units.json").read_text())
    if left_units != right_units:
        raise RuntimeError("non-deterministic translation-unit manifest")
    names = {
        "imgui_translated.c", "imgui_c89.h", "imgui_c89_internal.h",
        "imgui_translated.h",
        "imgui_translated.hpp",
        "imgui_translated_wrapper.cpp", "imgui_c89_api.h", "imgui_c89_api.c",
        "manifest.json",
        "translation_units.json",
        *(item["source"] for item in left_units),
    }
    different = [
        name for name in sorted(names)
        if not (left / name).exists() or not (right / name).exists()
        or (left / name).read_bytes() != (right / name).read_bytes()
    ]
    if different:
        raise RuntimeError(f"non-deterministic generated files: {different}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", default="baseline")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    baseline = root / "build/translator" / args.profile
    ir = baseline / "program.ir.json"
    if not ir.exists():
        raise SystemExit("baseline IR missing; run `python3 translator/build_baseline.py`")

    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    profile = json.loads(
        (root / "translator/profiles" / f"{args.profile}.json").read_text()
    )
    revision = profile.get("upstream_revision", args.profile)
    upstream = root / "build/upstream" / lock[revision]["commit"]
    generated_a = baseline / "generated-a"
    generated_b = baseline / "generated-b"
    translator = root / "translator/translate.py"
    cc = os.environ.get("CC", "cc")
    cxx = os.environ.get("CXX", "c++")
    facade_standard = profile.get("facade_language", profile["language"])
    define_flags = ["-D" + value for value in profile.get("defines", [])]
    undefine_flags = ["-U" + value for value in profile.get("undefines", [])]
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

    run([sys.executable, str(translator), str(ir), str(generated_a)])
    run([sys.executable, str(translator), str(ir), str(generated_b)])
    assert_same_outputs(generated_a, generated_b)

    native_source_text = (generated_a / "imgui_c89_api.c").read_text()
    forbidden_context_adapters = [
        token for token in (
            "imgui_c89_api_push_context",
            "imgui_c89_api_pop_context",
            "SetCurrentContext",
        )
        if token in native_source_text
    ]
    if forbidden_context_adapters:
        raise RuntimeError(
            "native C API still swaps implicit context: "
            + ", ".join(forbidden_context_adapters)
        )
    manifest = json.loads((generated_a / "manifest.json").read_text())
    if manifest.get("exact_c_function_count") != (
        manifest.get("native_c_function_count", 0) + 2
    ):
        raise RuntimeError("maintained exact-C surface lost context accessors")
    wrapper_text = (
        generated_a / "imgui_translated_wrapper.cpp"
    ).read_text(encoding="utf-8")
    if '#include "imgui_c89.h"' not in wrapper_text:
        raise RuntimeError("exact C++ facade no longer consumes imgui_c89.h")
    facade_hash_symbols = sorted(set(re.findall(
        r"\b[A-Za-z_][A-Za-z0-9_]*__[0-9a-f]{10}\b", wrapper_text
    )))
    if facade_hash_symbols:
        raise RuntimeError(
            "exact C++ facade exposes identity-hashed C symbols: "
            + ", ".join(facade_hash_symbols[:5])
        )
    public_header_text = (
        generated_a / "imgui_c89.h"
    ).read_text(encoding="utf-8")
    internal_header_text = (
        generated_a / "imgui_c89_internal.h"
    ).read_text(encoding="utf-8")
    if '#include "imgui_c89.h"' not in internal_header_text:
        raise RuntimeError("private C header no longer consumes imgui_c89.h")
    if '#include "imgui_c89_internal.h"' not in (
        generated_a / "imgui_translated.c"
    ).read_text(encoding="utf-8"):
        raise RuntimeError("translated C source bypasses its private header")
    enum_hashes = []
    for text in (public_header_text, internal_header_text):
        enum_hashes.extend(re.findall(
            r"^\s*([A-Za-z_][A-Za-z0-9_]*__[0-9a-f]{10})\s*=",
            text,
            flags=re.MULTILINE,
        ))
    if enum_hashes:
        raise RuntimeError(
            "enum constants still use identity hashes: "
            + ", ".join(sorted(set(enum_hashes))[:5])
        )
    ir_data = json.loads(ir.read_text(encoding="utf-8"))
    if manifest.get("split_public_header"):
        public_hashes = sorted(set(re.findall(
            r"__[0-9a-f]{8,10}\b", public_header_text
        )))
        if public_hashes:
            raise RuntimeError(
                "public C89 header still exposes identity hashes: "
                + ", ".join(public_hashes[:5])
            )
        forbidden_public_declarations = [
            token for token in (
                "struct ImGuiContext {",
                "struct ImGuiWindow {",
                "imgui_c89_debugtrap(",
                "imgui_c89_vector_reserve(",
                "imgui_c89_enable_cff_module(",
            )
            if token in public_header_text
        ]
        if forbidden_public_declarations:
            raise RuntimeError(
                "implementation declarations leaked into imgui_c89.h: "
                + ", ".join(forbidden_public_declarations)
            )
        public_api_names = set(re.findall(
            r"\b(imgui_(?!c89_)[a-z][a-z0-9_]*)\s*\(",
            public_header_text,
        ))
        if len(public_api_names) != manifest.get("exact_c_function_count"):
            raise RuntimeError(
                "public C89 function surface diverged from exact API "
                f"({len(public_api_names)} != "
                f"{manifest.get('exact_c_function_count')})"
            )
        if manifest.get("c_header_bytes") != len(public_header_text.encode()):
            raise RuntimeError("public C89 header byte count is stale")
        if manifest.get("c_internal_header_bytes") != len(
            internal_header_text.encode()
        ):
            raise RuntimeError("private C89 header byte count is stale")
        total_records = sum(
            bool(record.get("definition")) and not record.get("dependent")
            for record in ir_data.get("records", [])
        )
        public_records = manifest.get("public_complete_record_count", 0)
        private_records = manifest.get("private_complete_record_count", 0)
        if public_records <= 0 or public_records + private_records != total_records:
            raise RuntimeError("public/private record partition is incomplete")
        if public_records >= total_records:
            raise RuntimeError("public header failed to isolate private records")
        if "struct ImGuiContext {" not in internal_header_text:
            raise RuntimeError("private header lost ImGuiContext definition")
    public_constant_items = {
        constant["id"]: constant["name"]
        for enum in ir_data.get("enums", [])
        if Path(enum.get("location", {}).get("file", "")).name == "imgui.h"
        for constant in enum.get("constants", [])
    }
    private_constant_items = {
        constant["id"]: constant["name"]
        for enum in ir_data.get("enums", [])
        if Path(enum.get("location", {}).get("file", "")).name != "imgui.h"
        for constant in enum.get("constants", [])
    }
    public_constants = set(public_constant_items.values())
    private_constants = set(private_constant_items.values())
    missing_public = sorted(
        name for name in public_constants
        if re.search(rf"^\s*{re.escape(name)}\s*=", public_header_text,
                     flags=re.MULTILINE) is None
    )
    leaked_private = sorted(
        name for name in private_constants - public_constants
        if re.search(rf"^\s*{re.escape(name)}\s*=", public_header_text,
                     flags=re.MULTILINE) is not None
    )
    if missing_public:
        raise RuntimeError(
            "public enum constants missing from imgui_c89.h: "
            + ", ".join(missing_public[:5])
        )
    if leaked_private:
        raise RuntimeError(
            "private enum constants leaked into imgui_c89.h: "
            + ", ".join(leaked_private[:5])
        )
    if manifest.get("public_enum_constant_count") != len(public_constant_items):
        raise RuntimeError("public enum constant manifest count changed")
    if manifest.get("private_enum_constant_count") != len(private_constant_items):
        raise RuntimeError("private enum constant manifest count changed")
    threaded_count = manifest.get("context_threaded_function_count", 0)
    if manifest.get("compact_global_context"):
        explicit_count = manifest.get(
            "explicit_context_threaded_function_count", 0
        )
        if explicit_count <= 0:
            raise RuntimeError(
                "compact global-context profile lost its proven threaded flow"
            )
        expected_bindings = manifest["native_conditional_scope_count"]
        actual_bindings = native_source_text.count("    GImGui = ctx;\n")
        if actual_bindings != expected_bindings:
            raise RuntimeError(
                "compact global-context API did not bind every non-lifecycle "
                f"entry point ({actual_bindings} != {expected_bindings})"
            )
    elif threaded_count <= 0:
        raise RuntimeError("Dear ImGui profile generated no threaded context flow")
    if manifest.get("native_conditional_scope_count") != 3:
        raise RuntimeError(
            "native C API did not adapt Begin and both BeginChild overloads"
        )
    native_header_text = (generated_a / "imgui_c89_api.h").read_text()
    expected_scope_declarations = (
        "imgui_scope imgui_begin_scope(",
        "imgui_scope imgui_begin_child_id_scope(",
        "imgui_scope imgui_begin_child_string_scope(",
    )
    if not all(token in native_header_text for token in expected_scope_declarations):
        raise RuntimeError("native conditional-scope declarations are incomplete")
    if native_source_text.count("return IMGUI_SCOPE_INACTIVE;") != 3:
        raise RuntimeError("native conditional-scope adapters are incomplete")

    units = json.loads(
        (generated_a / "translation_units.json").read_text(encoding="utf-8")
    )
    expected = [
        Path(name).stem + ".c"
        for name in (
            profile["core_translation_units"]
            + profile.get("compatibility_translation_units", [])
            + profile.get("backend_translation_units", [])
        )
    ]
    actual = [item["source"] for item in units]
    if actual != expected:
        raise RuntimeError(f"translation-unit mismatch: expected {expected}, got {actual}")

    objects: list[Path] = []
    for item in units:
        source = generated_a / item["source"]
        output = source.with_suffix(".o")
        run([
            cc, "-std=c89", "-pedantic-errors", "-Wall",
            "-Wno-overlength-strings", "-I", str(generated_a),
            *dependency_cflags,
            "-c", str(source), "-o", str(output),
        ])
        objects.append(output)

    native_api_source = generated_a / "imgui_c89_api.c"
    native_api_object = generated_a / "imgui_c89_api.o"
    run([
        cc, "-std=c89", "-pedantic-errors", "-Wall",
        "-Wno-overlength-strings", "-I", str(generated_a),
        *dependency_cflags, "-c", str(native_api_source),
        "-o", str(native_api_object),
    ])

    common_cpp = [
        cxx, f"-std={facade_standard}", "-pedantic-errors", "-Wall",
        "-I", str(upstream), "-I", str(upstream / "backends"),
        *define_flags, *undefine_flags, *dependency_cflags,
    ]
    compatibility_units = profile.get("compatibility_translation_units", [])
    backend_units = profile.get("backend_translation_units", [])
    have_demo = "imgui_demo.cpp" in compatibility_units
    have_null = "backends/imgui_impl_null.cpp" in backend_units
    have_sdl = (
        "backends/imgui_impl_sdl3.cpp" in backend_units
        and "backends/imgui_impl_sdlrenderer3.cpp" in backend_units
    )
    smoke_feature_flags = [
        f"-DIMGUI_TRANSLATED_HAVE_DEMO={int(have_demo)}",
        f"-DIMGUI_TRANSLATED_HAVE_NULL_BACKEND={int(have_null)}",
        f"-DIMGUI_TRANSLATED_HAVE_SDL3_BACKEND={int(have_sdl)}",
    ]
    wrapper_object = generated_a / "imgui_translated_wrapper.o"
    run(common_cpp + [
        "-Wno-return-type-c-linkage", "-I", str(generated_a), "-c",
        str(generated_a / "imgui_translated_wrapper.cpp"),
        "-o", str(wrapper_object),
    ])
    smoke_object = generated_a / "baseline_smoke.o"
    run(common_cpp + [
        *smoke_feature_flags,
        "-c", str(root / "translator/fixtures/baseline_smoke.cpp"),
        "-o", str(smoke_object),
    ])
    executable = generated_a / "baseline_smoke"
    run([
        cxx, str(smoke_object), str(wrapper_object),
        *(str(item) for item in objects), "-lm", *dependency_libs,
        "-o", str(executable),
    ])
    run([str(executable)])
    native_smoke = generated_a / "native_api_smoke"
    run([
        cc, "-std=c89", "-pedantic-errors", "-Wall",
        "-Wno-overlength-strings", "-I", str(generated_a),
        *define_flags, *undefine_flags, *dependency_cflags,
        str(root / "translator/fixtures/native_api_smoke.c"),
        str(native_api_object), *(str(item) for item in objects),
        "-lm", *dependency_libs, "-o", str(native_smoke),
    ])
    run([str(native_smoke)])
    if manifest.get("trapped_call_count", 0) > 0:
        trap_smoke = generated_a / "embedded_trap_smoke"
        run([
            cc, "-std=c89", "-pedantic-errors", "-Wall",
            "-Wno-overlength-strings", "-I", str(generated_a),
            *define_flags, *undefine_flags, *dependency_cflags,
            str(root / "translator/fixtures/embedded_trap_smoke.c"),
            str(native_api_object), *(str(item) for item in objects),
            "-lm", *dependency_libs, "-o", str(trap_smoke),
        ])
        print("+", trap_smoke, "(expect debug trap)", flush=True)
        trapped = subprocess.run([str(trap_smoke)], check=False)
        if trapped.returncode >= 0:
            raise RuntimeError(
                "compact panic smoke did not terminate through a signal"
            )
    if manifest.get("compact_assert_traps"):
        assert_smoke = generated_a / "embedded_assert_smoke"
        run([
            cc, "-std=c89", "-pedantic-errors", "-Wall",
            "-Wno-overlength-strings", "-I", str(generated_a),
            *define_flags, *undefine_flags, *dependency_cflags,
            str(root / "translator/fixtures/embedded_assert_smoke.c"),
            str(native_api_object), *(str(item) for item in objects),
            "-lm", *dependency_libs, "-o", str(assert_smoke),
        ])
        print("+", assert_smoke, "(expect assertion trap)", flush=True)
        assertion_trapped = subprocess.run([str(assert_smoke)], check=False)
        if assertion_trapped.returncode >= 0:
            raise RuntimeError(
                "compact assertion smoke did not terminate through a signal"
            )
    native_count = manifest["native_c_function_count"]
    scope_count = manifest["native_conditional_scope_count"]
    threaded_count = manifest["context_threaded_function_count"]
    if manifest.get("compact_global_context"):
        threaded_count = manifest["explicit_context_threaded_function_count"]
    fixed_count = manifest["context_fixed_signature_count"]
    enabled_smoke_parts = ["core frame"]
    if have_demo:
        enabled_smoke_parts.append("demo")
    if have_null:
        enabled_smoke_parts.append("null backend")
    if have_sdl:
        enabled_smoke_parts.append("SDL3 render")
    print(
        f"literal {args.profile}: PASS ({len(objects)} independent C89 units, "
        f"exact {facade_standard} facade, "
        f"{'/'.join(enabled_smoke_parts)}, "
        f"{native_count}-function exact C89 API/{scope_count} optional scope "
        f"helpers, {threaded_count} context-threaded "
        f"functions/{fixed_count} fixed signatures)",
        flush=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
