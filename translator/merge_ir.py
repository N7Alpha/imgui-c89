#!/usr/bin/env python3
"""Merge per-translation-unit extractor output into one semantic program IR.

Clang deliberately parses each source file independently.  The translator
keeps that boundary in the merged IR so generated C can also be compiled one
file at a time, while resolving calls against definitions in other units.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def _prefer_definition(previous: dict[str, Any] | None,
                       candidate: dict[str, Any]) -> dict[str, Any]:
    if previous is None or candidate.get("definition"):
        return candidate
    return previous


def merge(paths: list[Path]) -> dict[str, Any]:
    records: dict[str, dict[str, Any]] = {}
    enums: dict[str, dict[str, Any]] = {}
    functions: dict[str, dict[str, Any]] = {}
    typedefs: dict[str, dict[str, Any]] = {}
    globals_: dict[str, dict[str, Any]] = {}
    declarations: dict[str, list[dict[str, Any]]] = {}
    macros: dict[tuple[str, str, int], dict[str, Any]] = {}
    units: list[dict[str, Any]] = []
    clang_versions: set[str] = set()

    for path in paths:
        document = json.loads(path.read_text(encoding="utf-8"))
        if document.get("schema_version") != 1:
            raise ValueError(f"{path}: unsupported schema version")
        clang_versions.add(document.get("clang_version", ""))
        definition_ids: list[str] = []
        global_definition_ids: list[str] = []
        source_files: set[str] = set()

        for item in document.get("records", []):
            records[item["id"]] = _prefer_definition(records.get(item["id"]), item)
        for item in document.get("enums", []):
            previous = enums.get(item["id"])
            if previous is None:
                enums[item["id"]] = item
            else:
                constants = {
                    constant["id"]: constant
                    for constant in previous.get("constants", [])
                }
                for constant in item.get("constants", []):
                    constants.setdefault(constant["id"], constant)
                previous["constants"] = [
                    constants[key] for key in sorted(constants)
                ]
        for item in document.get("functions", []):
            identifier = item["id"]
            declarations.setdefault(identifier, []).append(item)
            functions[identifier] = _prefer_definition(functions.get(identifier), item)
            if item.get("definition"):
                definition_ids.append(identifier)
                file_name = item.get("location", {}).get("file")
                if file_name:
                    source_files.add(file_name)
        for item in document.get("typedefs", []):
            typedefs.setdefault(item["id"], item)
        for item in document.get("globals", []):
            globals_[item["id"]] = _prefer_definition(
                globals_.get(item["id"]), item
            )
            if item.get("definition"):
                global_definition_ids.append(item["id"])
        for item in document.get("macros", []):
            location = item.get("location", {})
            key = (item.get("name", ""), location.get("file", ""),
                   int(location.get("line", 0)))
            macros.setdefault(key, item)

        units.append({
            "name": path.stem.removesuffix(".ir"),
            "ir_path": str(path),
            "source_files": sorted(source_files),
            "function_definitions": sorted(set(definition_ids)),
            "global_definitions": sorted(set(global_definition_ids)),
        })

    # Preserve every declaration so default arguments from headers are not
    # lost when the selected representative is an out-of-line definition.
    merged_functions: list[dict[str, Any]] = []
    for identifier in sorted(functions):
        merged_functions.extend(declarations[identifier])

    return {
        "schema_version": 1,
        "clang_version": ", ".join(sorted(clang_versions)),
        "records": [records[key] for key in sorted(records)],
        "enums": [enums[key] for key in sorted(enums)],
        "functions": merged_functions,
        "typedefs": [typedefs[key] for key in sorted(typedefs)],
        "globals": [globals_[key] for key in sorted(globals_)],
        "macros": [macros[key] for key in sorted(macros)],
        "translation_units": units,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("output", type=Path)
    parser.add_argument("inputs", nargs="+", type=Path)
    args = parser.parse_args()
    result = merge(args.inputs)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n",
                           encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
