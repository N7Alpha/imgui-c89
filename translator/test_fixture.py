#!/usr/bin/env python3
"""Build and verify the first semantic-extraction/lowering fixture."""

from __future__ import annotations

import copy
import filecmp
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path


def run(*arguments: str, cwd: Path | None = None) -> None:
    print("+", " ".join(arguments))
    subprocess.run(arguments, cwd=cwd, check=True)


def check_handwritten_effective_callgraph(root: Path, extracted: dict) -> None:
    """A replaced body must not retain constructor edges from its C++ AST."""
    sys.path.insert(0, str(root / "translator/py"))
    from imgui_translator.emit import Emitter

    probe = copy.deepcopy(extracted)
    external_hook = "fixture:external-hook"
    probe["functions"].append({
        "id": external_hook,
        "name": "ExternalHook",
        "qualified_name": "Fixture::ExternalHook",
        "return_type": "void",
        "parameters": [],
        "definition": False,
        "dependent": False,
        "method": False,
        "static": False,
        "const": False,
        "variadic": False,
    })

    def definition(name: str, parameter_types: list[str] | None = None) -> str:
        matches = {
            function["id"]
            for function in probe["functions"]
            if function.get("body")
            and function.get("qualified_name") == name
            and (
                parameter_types is None
                or [
                    parameter.get("type")
                    for parameter in function.get("parameters", [])
                ] == parameter_types
            )
        }
        if len(matches) != 1:
            raise RuntimeError(f"effective-callgraph fixture cannot find {name}")
        return next(iter(matches))

    constructor = definition("ImGui::Vec2::Vec2", ["float", "float"])
    direct_consumer = definition("ImGui::DiscardedReference")
    value_consumer = definition("ImGui::TemporarySum")
    probe["translation_units"] = [
        {"function_definitions": [constructor]},
        {"function_definitions": [direct_consumer, value_consumer]},
    ]
    emitter = Emitter(probe)
    if constructor in emitter.internal_functions:
        raise RuntimeError("raw cross-TU constructor unexpectedly became local")
    if value_consumer not in emitter.constructor_value_consumers[constructor]:
        raise RuntimeError("temporary constructor adapter was not detected")

    emitter.handwritten_function_dependencies[direct_consumer] = set()
    emitter.handwritten_function_dependencies[value_consumer] = set()
    emitter.handwritten_constructor_value_dependencies[direct_consumer] = set()
    emitter.handwritten_constructor_value_dependencies[value_consumer] = set()
    _, _, internal = emitter.analyze_function_linkage()
    value_consumers, _ = emitter.analyze_constructor_helper_usage()
    if constructor not in internal:
        raise RuntimeError(
            "discarded handwritten constructor edges still force cross-TU linkage"
        )
    if value_consumer in value_consumers[constructor]:
        raise RuntimeError(
            "discarded handwritten body still retains a constructor adapter"
        )
    # Test-engine overlays can deliberately call declaration-only C++ bridge
    # hooks. They are dependencies, but not translated linkage vertices.
    emitter.handwritten_function_dependencies[direct_consumer] = {
        external_hook
    }
    emitter.analyze_function_linkage()


def check_handwritten_groups(root: Path, extracted: dict) -> None:
    """Shared handwritten mappings merge without weakening body guards."""
    sys.path.insert(0, str(root / "translator/py"))
    from imgui_translator.emit import Emitter, TranslationError

    probe = copy.deepcopy(extracted)
    function = next(
        item for item in probe["functions"]
        if item.get("qualified_name") == "ImGui::DiscardedReference"
        and item.get("body")
    )
    body_hash = Emitter.function_body_sha256(function)
    emitter = Emitter(probe)
    emitter.handwritten_groups = {
        "fixture": {
            "template": "shared.c.in",
            "constants": {"@SHARED@": "SharedValue"},
            "snippets": {"@GROUP@": "group"},
        }
    }
    emitter.handwritten_functions = {
        "ImGui::DiscardedReference": {
            "group": "fixture",
            "parameter_types": [],
            "body_sha256": body_hash,
            "fragment": "discarded",
            "snippets": {"@LOCAL@": "local"},
        }
    }
    specification = emitter.handwritten_specification(function)
    if specification is None:
        raise RuntimeError("handwritten group did not match guarded function")
    if specification["template"] != "shared.c.in":
        raise RuntimeError("handwritten group template was not inherited")
    if specification["snippets"] != {"@GROUP@": "group", "@LOCAL@": "local"}:
        raise RuntimeError("handwritten group mappings did not merge")

    emitter.handwritten_functions["ImGui::DiscardedReference"]["group"] = "missing"
    try:
        emitter.handwritten_specification(function)
    except TranslationError:
        pass
    else:
        raise RuntimeError("unknown handwritten group was accepted")


def check_table_pool_direct_initialization(root: Path) -> None:
    """The table-pool lowering is direct, readable, and fails closed."""
    sys.path.insert(0, str(root / "translator/py"))
    from imgui_translator.emit import Emitter, TranslationError

    this = {"kind": "CXXThisExpr"}
    table_constructor = {
        "id": "table-ctor",
        "qualified_name": "ImGuiTable::ImGuiTable",
        "constructor": True,
        "parameters": [],
        "initializers": [],
        "body": {
            "kind": "CompoundStmt",
            "statements": [
                {
                    "kind": "CallExpr",
                    "callee_name": "memset",
                    "arguments": [
                        this,
                        {"kind": "IntegerLiteral", "value": "0"},
                        {
                            "kind": "UnaryExprOrTypeTraitExpr",
                            "operator": "sizeof",
                            "argument": {
                                "kind": "UnaryOperator",
                                "opcode": "*",
                                "operand": this,
                            },
                        },
                    ],
                },
                {
                    "kind": "BinaryOperator",
                    "opcode": "=",
                    "lhs": {
                        "kind": "MemberExpr",
                        "name": "LastFrameActive",
                        "base": this,
                    },
                    "rhs": {
                        "kind": "UnaryOperator",
                        "opcode": "-",
                        "operand": {"kind": "IntegerLiteral", "value": "1"},
                    },
                },
            ],
        },
    }
    add = {
        "id": "table-add",
        "qualified_name": "ImPool<ImGuiTable>::Add",
        "body": {
            "kind": "CompoundStmt",
            "statements": [
                {"kind": "DeclStmt"},
                {"kind": "IfStmt"},
                {
                    "kind": "ExprWithCleanups",
                    "operand": {
                        "kind": "CXXNewExpr",
                        "array": False,
                        "allocated_type": "ImGuiTable",
                        "initializer": {
                            "kind": "CXXConstructExpr",
                            "constructor": "table-ctor",
                            "arguments": [],
                        },
                    },
                },
                {
                    "kind": "UnaryOperator",
                    "opcode": "++",
                    "operand": {
                        "kind": "MemberExpr",
                        "name": "AliveCount",
                        "base": this,
                    },
                },
                {"kind": "ReturnStmt"},
            ],
        },
    }
    emitter = Emitter.__new__(Emitter)
    emitter.compact_impool = True
    emitter.flatten_table_pool = False
    emitter.c_type = lambda spelling: spelling
    emitter.functions = {
        table_constructor["id"]: table_constructor,
        add["id"]: add,
    }
    body = emitter.compact_impool_body(add)
    if body is None or "    memset(item, 0, sizeof(*item));" not in body:
        raise RuntimeError("table pool Add did not lower to direct initialization")
    if "    item->LastFrameActive = -1;" not in body:
        raise RuntimeError("table pool Add lost its nonzero table default")

    changed = copy.deepcopy(table_constructor)
    changed["body"]["statements"][1]["lhs"]["name"] = "ChangedDefault"
    emitter.functions[changed["id"]] = changed
    try:
        emitter.compact_impool_body(add)
    except TranslationError:
        pass
    else:
        raise RuntimeError("table pool lowering accepted changed defaults")


def check_table_pool_test_engine_fingerprint(root: Path) -> None:
    """Accept only the known Test Engine assertion-instrumented pool body."""
    sys.path.insert(0, str(root / "translator/py"))
    from imgui_translator.emit import Emitter, TranslationError

    expected = Emitter.table_pool_fingerprint_variants()
    key = ("GetIndex", ("const ImGuiTable *",))
    engine_hash = (
        "1087de96a2483f5891e1795751b1fa56cf71c8c626019a3753e822a9c12b1e19"
    )
    if expected.get(key, set()) != {
        engine_hash,
        "b55ba7fdc5908b3033e057cd4119baabb381146be5e9826103d5fc1f9f00fa50",
    }:
        raise RuntimeError("table pool GetIndex fingerprint variants drifted")

    functions = {}
    for index, (operation, variants) in enumerate(expected.items()):
        name, parameters = operation
        identifier = f"pool-{index}"
        functions[identifier] = {
            "id": identifier,
            "name": name,
            "qualified_name": f"ImPool<ImGuiTable>::{name}",
            "parameters": [{"type": spelling} for spelling in parameters],
            "fixture_hash": engine_hash if operation == key else next(iter(variants)),
        }
    functions["table-fini"] = {
        "id": "table-fini",
        "qualified_name": "ImGuiTable::~ImGuiTable",
        "fixture_hash": (
            "cf849c76c22272bfb8a8595e5bbce62c9302e3aab4720149dc08c19066bd21b2"
        ),
    }
    emitter = Emitter.__new__(Emitter)
    emitter.functions = functions
    emitter.function_body_sha256 = lambda function: function["fixture_hash"]
    emitter.validate_table_pool_fingerprints()

    get_index = next(
        function for function in functions.values()
        if function.get("qualified_name") == "ImPool<ImGuiTable>::GetIndex"
    )
    get_index["fixture_hash"] = "0" * 64
    try:
        emitter.validate_table_pool_fingerprints()
    except TranslationError as error:
        if "GetIndex('const ImGuiTable *',)" not in str(error):
            raise RuntimeError("fingerprint failure named the wrong method")
    else:
        raise RuntimeError("table pool guard accepted unknown Test Engine body")


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    build = root / "build/translator"
    fixture = root / "translator/fixtures/basic"
    unsupported_fixture = root / "translator/fixtures/unsupported"
    first = build / "fixtures/basic/generated-a"
    second = build / "fixtures/basic/generated-b"
    ir = build / "fixtures/basic/ir.json"
    llvm_prefix = os.environ.get("LLVM_PREFIX", "/opt/homebrew/opt/llvm")
    cmake_prefix = os.environ.get("CMAKE_PREFIX_PATH", llvm_prefix)

    run(
        "cmake", "-S", str(root / "translator"), "-B", str(build),
        "-G", "Ninja", f"-DCMAKE_PREFIX_PATH={cmake_prefix}",
        "-DCMAKE_BUILD_TYPE=Debug",
    )
    run("cmake", "--build", str(build), "--target", "imgui-clang-extract")
    ir.parent.mkdir(parents=True, exist_ok=True)
    run(
        str(build / "imgui-clang-extract"), "--output", str(ir),
        str(fixture / "input.cpp"), "--", "-std=c++11",
    )
    extracted = json.loads(ir.read_text())
    macro_names = {item["name"] for item in extracted.get("macros", [])}
    if "IMGUI_FIXTURE_DEFAULT_SCALE" not in macro_names:
        raise RuntimeError("project macro definitions were not captured in IR")
    check_handwritten_effective_callgraph(root, extracted)
    check_handwritten_groups(root, extracted)
    check_table_pool_direct_initialization(root)
    check_table_pool_test_engine_fingerprint(root)
    for directory in (first, second):
        if directory.exists():
            shutil.rmtree(directory)
        run(sys.executable, str(root / "translator/translate.py"), str(ir), str(directory))

    comparison = filecmp.dircmp(first, second)
    if comparison.left_only or comparison.right_only or comparison.diff_files or comparison.funny_files:
        raise RuntimeError("generation is not byte-identical")

    cc = os.environ.get("CC", "cc")
    cxx = os.environ.get("CXX", "c++")
    obj = first / "imgui_translated.o"
    run(
        cc, "-std=c89", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
        "-I", str(first), "-c", str(first / "imgui_translated.c"), "-o", str(obj),
    )
    reference = build / "fixtures/basic/reference"
    candidate = build / "fixtures/basic/candidate"
    run(
        cxx, "-std=c++98", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
        str(fixture / "reference_main.cpp"), "-o", str(reference),
    )
    run(
        cxx, "-std=c++98", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
        "-I", str(first), str(fixture / "candidate_main.cpp"), str(obj), "-o", str(candidate),
    )
    run(str(reference))
    run(str(candidate))

    unsupported_ir = build / "fixtures/unsupported/ir.json"
    unsupported_output = build / "fixtures/unsupported/generated"
    unsupported_ir.parent.mkdir(parents=True, exist_ok=True)
    run(
        str(build / "imgui-clang-extract"), "--output", str(unsupported_ir),
        str(unsupported_fixture / "input.cpp"), "--", "-std=c++11",
    )
    failure = subprocess.run(
        [sys.executable, str(root / "translator/translate.py"),
         str(unsupported_ir), str(unsupported_output)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if failure.returncode == 0 or "unsupported statement" not in failure.stderr:
        raise RuntimeError(
            "unsupported constructs must fail with a source-located diagnostic; "
            f"got exit={failure.returncode}, stderr={failure.stderr!r}"
        )
    print("translator fixture: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
