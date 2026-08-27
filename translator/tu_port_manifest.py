#!/usr/bin/env python3
"""Inventory one upstream translation unit for complete handwritten ownership."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path


def body_hash(function: dict) -> str:
    encoded = json.dumps(
        function.get("body", {}), sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return hashlib.sha256(encoded).hexdigest()


def walk(value: object):
    if isinstance(value, dict):
        yield value
        for child in value.values():
            yield from walk(child)
    elif isinstance(value, list):
        for child in value:
            yield from walk(child)


def symbol_sizes(object_path: Path) -> dict[str, dict[str, int]]:
    result = subprocess.run(
        [
            "bloaty", "-d", "symbols", "-n", "0", "--domain=vm",
            "--csv", str(object_path),
        ],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
    )
    rows: dict[str, dict[str, int]] = {}
    for row in csv.DictReader(result.stdout.splitlines()):
        name = row["symbols"]
        if name.startswith("_"):
            name = name[1:]
        rows[name] = {
            "loaded_bytes": int(row["vmsize"]),
            "file_bytes": int(row["filesize"]),
        }
    return rows


def source_sections(source: Path) -> list[tuple[int, str]]:
    sections = [(1, "preamble")]
    for line_number, line in enumerate(
        source.read_text(encoding="utf-8").splitlines(), 1
    ):
        marker = "[SECTION]"
        if marker in line:
            sections.append((line_number, line.split(marker, 1)[1].strip(" -/")))
    return sections


def section_for(line: int, sections: list[tuple[int, str]]) -> str:
    result = sections[0][1]
    for section_line, section in sections:
        if section_line > line:
            break
        result = section
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", default="idiomatic_c89")
    parser.add_argument("--source", default="imgui_tables.cpp")
    parser.add_argument("--object", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    sys.path.insert(0, str(root / "translator/py"))
    from imgui_translator.emit import Emitter

    profile_dir = root / "build/translator" / args.profile
    ir_path = profile_dir / "program.ir.json"
    ir = json.loads(ir_path.read_text(encoding="utf-8"))
    emitter = Emitter(ir)
    suffix = "/" + args.source
    functions = [
        item for item in ir.get("functions", [])
        if item.get("definition")
        and str(item.get("location", {}).get("file", "")).endswith(suffix)
    ]
    functions.sort(key=lambda item: (item["location"]["line"], item["id"]))
    if not functions:
        raise SystemExit(f"no definitions found for {args.source}")

    source = Path(functions[0]["location"]["file"])
    lines = source.read_text(encoding="utf-8").splitlines()
    sections = source_sections(source)
    local_ids = {item["id"] for item in functions}
    declarations = {
        item["id"]: item for item in ir.get("functions", [])
    }
    handwritten = set(emitter.handwritten_functions)
    inbound_callers: dict[str, set[str]] = {identifier: set() for identifier in local_ids}
    for caller in ir.get("functions", []):
        if not caller.get("definition") or caller.get("id") in local_ids:
            continue
        for node in walk(caller.get("body", {})):
            callee = node.get("callee")
            if callee in local_ids:
                inbound_callers[callee].add(caller.get("qualified_name", ""))
    object_path = args.object or (
        root / "build/full-library-size/c89-patched"
        / args.source.replace(".cpp", ".o")
    )
    sizes = symbol_sizes(object_path)

    rows = []
    for index, function in enumerate(functions):
        start = int(function["location"]["line"])
        end = (
            int(functions[index + 1]["location"]["line"]) - 1
            if index + 1 < len(functions) else len(lines)
        )
        callees = {
            node["callee"]
            for node in walk(function.get("body", {}))
            if "callee" in node and node["callee"] in declarations
        }
        internal = sorted({
            declarations[identifier].get("qualified_name", "")
            for identifier in callees if identifier in local_ids
        })
        external = sorted({
            declarations[identifier].get("qualified_name", "")
            for identifier in callees if identifier not in local_ids
        })
        symbol = emitter.function_names[function["id"]]
        measured = sizes.get(symbol, {"loaded_bytes": 0, "file_bytes": 0})
        rows.append({
            "qualified_name": function["qualified_name"],
            "c_symbol": symbol,
            "start_line": start,
            "approx_end_line": end,
            "approx_source_lines": end - start + 1,
            "section": section_for(start, sections),
            "return_type": function["return_type"],
            "parameter_types": [
                parameter["type"] for parameter in function.get("parameters", [])
            ],
            "body_sha256": body_hash(function),
            "handwritten": function["qualified_name"] in handwritten,
            "loaded_bytes": measured["loaded_bytes"],
            "file_bytes": measured["file_bytes"],
            "internal_callees": internal,
            "external_callees": external,
            "inbound_callers": sorted(inbound_callers[function["id"]]),
            "exact_c_boundary": function["id"] in emitter.exact_c_names,
        })

    globals_ = [
        item for item in ir.get("globals", [])
        if item.get("definition")
        and str(item.get("location", {}).get("file", "")).endswith(suffix)
    ]
    type_counts: dict[str, int] = {}
    for function in functions:
        candidates = [function.get("return_type", "")]
        candidates.extend(
            parameter.get("type", "")
            for parameter in function.get("parameters", [])
        )
        for node in walk(function.get("body", {})):
            if isinstance(node.get("type"), str):
                candidates.append(node["type"])
            for declaration in node.get("declarations", []):
                if isinstance(declaration.get("type"), str):
                    candidates.append(declaration["type"])
        for spelling in candidates:
            base = re.sub(r"\b(const|volatile)\b", "", spelling)
            base = re.sub(r"[&*\[\]]", " ", base)
            base = re.sub(r"\s+", " ", base).strip()
            if base:
                type_counts[base] = type_counts.get(base, 0) + 1

    report = {
        "schema_version": 1,
        "profile": args.profile,
        "source": str(source),
        "source_lines": len(lines),
        "object": str(object_path),
        "object_file_bytes": object_path.stat().st_size,
        "function_count": len(rows),
        "handwritten_count": sum(int(row["handwritten"]) for row in rows),
        "source_derived_count": sum(not row["handwritten"] for row in rows),
        "mapped_function_loaded_bytes": sum(row["loaded_bytes"] for row in rows),
        "global_count": len(globals_),
        "globals": sorted(item["qualified_name"] for item in globals_),
        "types": [
            {"type": name, "occurrences": count}
            for name, count in sorted(
                type_counts.items(), key=lambda item: (-item[1], item[0])
            )
        ],
        "functions": rows,
    }
    output = args.output or root / "build/tu-port" / args.source.removesuffix(".cpp")
    output.mkdir(parents=True, exist_ok=True)
    (output / "manifest.json").write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )

    ranked = sorted(
        rows, key=lambda row: (row["loaded_bytes"], row["approx_source_lines"]),
        reverse=True,
    )
    markdown = [
        f"# `{args.source}` handwritten-port manifest",
        "",
        f"- Upstream source: {len(lines):,} lines",
        f"- Upstream-defined functions: {len(rows)}",
        f"- Already handwritten: {report['handwritten_count']}",
        f"- Remaining source-derived bodies: {report['source_derived_count']}",
        f"- Current object: {object_path.stat().st_size:,} bytes",
        f"- Mapped function loaded bytes: {report['mapped_function_loaded_bytes']:,}",
        f"- TU-defined globals: {len(globals_)}",
        f"- Exact/cross-TU function boundaries: "
        f"{sum(int(bool(row['exact_c_boundary'] or row['inbound_callers'])) for row in rows)}",
        "",
        "| Function | Section | Approx. lines | Loaded bytes | State |",
        "|---|---|---:|---:|---|",
    ]
    for row in ranked:
        markdown.append(
            f"| `{row['qualified_name']}` | {row['section']} | "
            f"{row['approx_source_lines']:,} | {row['loaded_bytes']:,} | "
            f"{'handwritten' if row['handwritten'] else 'source-derived'} |"
        )
    markdown.append("")
    (output / "manifest.md").write_text("\n".join(markdown), encoding="utf-8")
    print("\n".join(markdown[:9]))
    print(f"Reports: {output / 'manifest.json'}, {output / 'manifest.md'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
