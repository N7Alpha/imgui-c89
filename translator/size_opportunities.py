#!/usr/bin/env python3
"""Build a systematic code/data reuse report for idiomatic Dear ImGui C89.

The compiler merge and outlining variants are discovery oracles.  They are
measured against a single combined-bitcode control and are never substituted
for the idiomatic, separately compiled objects by this script.
"""

from __future__ import annotations

import csv
import hashlib
import io
import json
import os
import re
import shutil
import subprocess
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable


CORE_STEMS = ("imgui", "imgui_draw", "imgui_tables", "imgui_widgets")
SIZE_FLAGS = (
    "-Os",
    "-fno-exceptions",
    "-fno-unwind-tables",
    "-fno-asynchronous-unwind-tables",
)
VARIANT_DIRECTORIES = {
    "upstream C++": "cpp",
    "literal translated C89": "c89-literal",
    "idiomatic C89": "c89-idiomatic",
}
IDENTITY_KEYS = {
    "id", "decl", "member", "parent", "location", "value_category",
    "canonical_type", "source_range",
}


def tool(environment: str, preferred: str, fallback: str) -> str:
    configured = os.environ.get(environment)
    if configured:
        return configured
    if Path(preferred).exists():
        return preferred
    found = shutil.which(fallback)
    if found:
        return found
    raise SystemExit(f"required tool not found: {fallback}")


def run(arguments: list[str]) -> None:
    print("+", " ".join(arguments), flush=True)
    subprocess.run(arguments, check=True)


def capture(arguments: list[str]) -> str:
    return subprocess.run(
        arguments, check=True, text=True, stdout=subprocess.PIPE,
    ).stdout


def write_csv(path: Path, fieldnames: list[str], rows: Iterable[dict[str, Any]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def bloaty_rows(
    bloaty: str, path: Path, datasource: str, base: Path | None = None,
) -> list[dict[str, str]]:
    arguments = [bloaty, "-d", datasource, "-n", "0", "--csv", str(path)]
    if base is not None:
        arguments.extend(["--", str(base)])
    return list(csv.DictReader(io.StringIO(capture(arguments))))


def section_bytes(path: Path, size_tool: str) -> int:
    lines = capture([size_tool, str(path)]).splitlines()
    if len(lines) < 2:
        raise RuntimeError(f"cannot parse size output for {path}")
    fields = lines[-1].split()
    if fields[-1] in {str(path), path.name}:
        fields.pop()
    return int(fields[-2])


def function_symbol_names(objdump: str, path: Path) -> set[str]:
    names: set[str] = set()
    for line in capture([objdump, "--syms", str(path)]).splitlines():
        fields = line.split()
        if "F" in fields and "__TEXT,__text" in fields and fields:
            names.add(fields[-1])
    return names


def strip_generated_hash(name: str) -> str:
    return re.sub(r"__[0-9a-f]{10}(?:\.[0-9]+)?$", "", name.lstrip("_"))


def template_family(name: str) -> str:
    base = strip_generated_hash(name)
    for prefix in ("ImVector_", "ImPool_", "ImChunkStream_"):
        if not base.startswith(prefix) or "__" not in base:
            continue
        if re.search(r"__operator_*$", base):
            method = "operator"
        else:
            method = base.rsplit("__", 1)[-1]
        if method.startswith("dtor_"):
            method = "~" + method[len("dtor_"):]
        elif method.startswith("operator_"):
            method = "operator"
        return prefix[:-1] + "<T>::" + method
    return ""


def compact_names(value: str, limit: int = 3) -> str:
    names = value.split(";")
    shown = ", ".join(f"`{name}`" for name in names[:limit])
    if len(names) > limit:
        shown += f", +{len(names) - limit} more"
    return shown


def bloaty_inventory(
    root: Path, output: Path, bloaty: str, objdump: str,
    size_report: dict[str, Any],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], list[dict[str, Any]]]:
    section_rows: list[dict[str, Any]] = []
    function_rows: list[dict[str, Any]] = []
    delta_rows: list[dict[str, Any]] = []
    size_root = root / "build/full-library-size"
    variants = {item["kind"]: item for item in size_report["variants"]}
    for variant, directory in VARIANT_DIRECTORIES.items():
        if variant not in variants:
            raise RuntimeError(f"size report is missing {variant}")
        for stem in CORE_STEMS:
            obj = size_root / directory / f"{stem}.o"
            for row in bloaty_rows(bloaty, obj, "sections"):
                section_rows.append({
                    "variant": variant,
                    "object": obj.name,
                    "section": row["sections"],
                    "vm_bytes": int(row["vmsize"]),
                    "file_bytes": int(row["filesize"]),
                })
            functions = function_symbol_names(objdump, obj)
            for row in bloaty_rows(bloaty, obj, "symbols"):
                symbol = row["symbols"]
                if symbol not in functions:
                    continue
                function_rows.append({
                    "variant": variant,
                    "object": obj.name,
                    "symbol": symbol,
                    "family": template_family(symbol),
                    "vm_bytes": int(row["vmsize"]),
                    "file_bytes": int(row["filesize"]),
                })

    for stem in CORE_STEMS:
        canonical = size_root / "c89-idiomatic" / f"{stem}.o"
        literal = size_root / "c89-literal" / f"{stem}.o"
        for datasource in ("sections", "symbols"):
            for row in bloaty_rows(bloaty, canonical, datasource, literal):
                delta_rows.append({
                    "object": canonical.name,
                    "datasource": datasource,
                    "name": row[datasource],
                    "vm_delta_bytes": int(row["vmsize"]),
                    "file_delta_bytes": int(row["filesize"]),
                })

    write_csv(output / "bloaty_sections.csv", [
        "variant", "object", "section", "vm_bytes", "file_bytes",
    ], section_rows)
    write_csv(output / "function_sizes.csv", [
        "variant", "object", "symbol", "family", "vm_bytes", "file_bytes",
    ], function_rows)
    write_csv(output / "bloaty_idiomatic_vs_literal.csv", [
        "object", "datasource", "name", "vm_delta_bytes", "file_delta_bytes",
    ], delta_rows)
    return section_rows, function_rows, delta_rows


def template_family_rows(function_rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in function_rows:
        if row["variant"] == "idiomatic C89" and row["family"]:
            groups[row["family"]].append(row)
    result: list[dict[str, Any]] = []
    for family, rows in groups.items():
        if len(rows) < 2:
            continue
        sizes = [int(item["vm_bytes"]) for item in rows]
        result.append({
            "family": family,
            "instances": len(rows),
            "total_vm_bytes": sum(sizes),
            "largest_vm_bytes": max(sizes),
            "theoretical_shared_vm_bytes": sum(sizes) - max(sizes),
            "symbols": ";".join(
                f"{item['object']}:{item['symbol']}" for item in rows
            ),
        })
    return sorted(
        result, key=lambda item: int(item["theoretical_shared_vm_bytes"]),
        reverse=True,
    )


def disassembly_fingerprints(
    objdump: str, objects: list[Path], function_sizes: dict[tuple[str, str], int],
) -> list[dict[str, Any]]:
    label = re.compile(r"^[0-9a-f]+ <([^>]+)>:$")
    instruction = re.compile(r"^\s*[0-9a-f]+:\s+([0-9a-f]{8})\s")
    relocation = re.compile(r"ARM64_RELOC_[A-Z0-9_]+\s+(\S+)")
    bodies: list[dict[str, Any]] = []
    for obj in objects:
        current = ""
        raw: list[str] = []
        aware: list[str] = []

        def finish() -> None:
            if not current or not raw:
                return
            size = function_sizes.get((obj.name, current), 0)
            if size < 8:
                return
            bodies.append({
                "object": obj.name,
                "symbol": current,
                "vm_bytes": size,
                "raw_hash": hashlib.sha256("|".join(raw).encode()).hexdigest(),
                "aware_hash": hashlib.sha256("|".join(aware).encode()).hexdigest(),
            })

        for line in capture([objdump, "-dr", str(obj)]).splitlines():
            match = label.match(line)
            if match:
                finish()
                current = match.group(1)
                raw = []
                aware = []
                continue
            match = instruction.match(line)
            if match and current:
                raw.append(match.group(1))
                aware.append(match.group(1))
                continue
            match = relocation.search(line)
            if match and current:
                aware.append("reloc:" + match.group(1))
        finish()

    result: list[dict[str, Any]] = []
    for kind, key in (("relocation-aware", "aware_hash"), ("raw-bytes", "raw_hash")):
        groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
        for body in bodies:
            groups[str(body[key])].append(body)
        for fingerprint, rows in groups.items():
            if len(rows) < 2:
                continue
            sizes = [int(item["vm_bytes"]) for item in rows]
            result.append({
                "kind": kind,
                "fingerprint": fingerprint,
                "instances": len(rows),
                "total_vm_bytes": sum(sizes),
                "duplicate_vm_bytes": sum(sizes) - max(sizes),
                "symbols": ";".join(
                    f"{item['object']}:{item['symbol']}" for item in rows
                ),
            })
    return sorted(
        result, key=lambda item: int(item["duplicate_vm_bytes"]), reverse=True,
    )


def normalized_template_name(value: str) -> str:
    result = value
    for template in ("ImVector", "ImPool", "ImChunkStream"):
        result = re.sub(rf"{template}<[^<>]*(?:<[^<>]*>[^<>]*)*>",
                        template + "<T>", result)
    return result


def ir_clone_rows(root: Path) -> list[dict[str, Any]]:
    ir_root = root / "build/translator/idiomatic_c89"
    grouped: dict[tuple[str, str], list[dict[str, Any]]] = defaultdict(list)
    for stem in CORE_STEMS:
        ir_path = ir_root / f"{stem}_cpp.ir.json"
        document = json.loads(ir_path.read_text(encoding="utf-8"))
        declarations = {
            item.get("id", ""): item.get("qualified_name", item.get("name", ""))
            for item in document.get("functions", [])
        }

        def canonical(node: Any, mode: str) -> Any:
            if isinstance(node, list):
                return [canonical(item, mode) for item in node]
            if not isinstance(node, dict):
                return node
            kind = node.get("kind", "")
            result: dict[str, Any] = {}
            for key in sorted(node):
                if key in IDENTITY_KEYS:
                    continue
                value = node[key]
                if key in {"callee", "constructor"} and isinstance(value, str):
                    value = declarations.get(value, value)
                    if mode != "exact":
                        value = normalized_template_name(value)
                if mode != "exact" and key == "type":
                    continue
                if (mode != "exact" and key == "name"
                        and kind in {"DeclRefExpr", "ParmVarDecl", "VarDecl"}):
                    value = "$local"
                if (mode == "literal-normalized" and key == "value"
                        and (kind.endswith("Literal") or kind == "GNUNullExpr")):
                    value = "$literal"
                result[key] = canonical(value, mode)
            return result

        def count_nodes(node: Any) -> int:
            if isinstance(node, dict):
                return 1 + sum(count_nodes(value) for value in node.values())
            if isinstance(node, list):
                return sum(count_nodes(value) for value in node)
            return 0

        for function in document.get("functions", []):
            body = function.get("body")
            if not body:
                continue
            name = function.get("qualified_name", function.get("name", ""))
            nodes = count_nodes(body)
            for mode in ("exact", "type-normalized", "literal-normalized"):
                encoded = json.dumps(
                    canonical(body, mode), sort_keys=True, separators=(",", ":"),
                ).encode()
                fingerprint = hashlib.sha256(encoded).hexdigest()
                grouped[(mode, fingerprint)].append({
                    "function": name,
                    "translation_unit": stem,
                    "nodes": nodes,
                })
        del document

    result: list[dict[str, Any]] = []
    for (mode, fingerprint), rows in grouped.items():
        if len(rows) < 2:
            continue
        nodes = [int(item["nodes"]) for item in rows]
        result.append({
            "kind": mode,
            "fingerprint": fingerprint,
            "instances": len(rows),
            "total_nodes": sum(nodes),
            "duplicate_nodes": sum(nodes) - max(nodes),
            "functions": ";".join(
                f"{item['translation_unit']}:{item['function']}" for item in rows
            ),
        })
    return sorted(
        result, key=lambda item: int(item["duplicate_nodes"]), reverse=True,
    )


def compile_llvm_oracles(
    root: Path, output: Path, cc: str, llvm_link: str, opt: str,
    size_tool: str, bloaty: str, objdump: str,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    llvm_output = output / "llvm"
    llvm_output.mkdir(parents=True, exist_ok=True)
    generated = root / "build/translator/idiomatic_c89/generated-a"
    bitcode: list[Path] = []
    for stem in CORE_STEMS:
        target = llvm_output / f"{stem}.bc"
        run([
            cc, "-std=c89", *SIZE_FLAGS, "-w", "-I", str(generated),
            "-emit-llvm", "-c", str(generated / f"{stem}.c"), "-o", str(target),
        ])
        bitcode.append(target)
    control = llvm_output / "control.bc"
    run([llvm_link, *(str(item) for item in bitcode), "-o", str(control)])
    passes = {
        "control": None,
        "constmerge": "constmerge",
        "mergefunc": "mergefunc",
        "iroutliner": "iroutliner",
        "all_ir": "constmerge,mergefunc,iroutliner",
    }
    scenario_bitcode: dict[str, Path] = {"control": control}
    for name, pipeline in passes.items():
        if pipeline is None:
            continue
        target = llvm_output / f"{name}.bc"
        run([opt, f"-passes={pipeline}", str(control), "-o", str(target)])
        scenario_bitcode[name] = target

    scenario_objects: dict[str, Path] = {}
    for name, source in scenario_bitcode.items():
        target = llvm_output / f"{name}.o"
        run([cc, *SIZE_FLAGS, "-c", str(source), "-o", str(target)])
        scenario_objects[name] = target
    for name, source in {
        "machine_outliner": control,
        "all_ir_machine_outliner": scenario_bitcode["all_ir"],
    }.items():
        target = llvm_output / f"{name}.o"
        run([
            cc, *SIZE_FLAGS, "-mllvm", "-enable-machine-outliner=always",
            "-c", str(source), "-o", str(target),
        ])
        scenario_objects[name] = target

    control_sections = section_bytes(scenario_objects["control"], size_tool)
    control_file = scenario_objects["control"].stat().st_size
    oracle_rows: list[dict[str, Any]] = []
    for name, obj in scenario_objects.items():
        sections = bloaty_rows(bloaty, obj, "sections")
        text = sum(
            int(item["vmsize"]) for item in sections
            if item["sections"].endswith("__text")
        )
        symbols = bloaty_rows(bloaty, obj, "symbols")
        loaded = section_bytes(obj, size_tool)
        oracle_rows.append({
            "scenario": name,
            "file_bytes": obj.stat().st_size,
            "loaded_section_bytes": loaded,
            "text_bytes": text,
            "symbol_count": len(symbols),
            "file_delta_bytes": obj.stat().st_size - control_file,
            "loaded_delta_bytes": loaded - control_sections,
        })

    machine = scenario_objects["machine_outliner"]
    symbol_sizes = {
        item["symbols"]: int(item["vmsize"])
        for item in bloaty_rows(bloaty, machine, "symbols")
        if item["symbols"].startswith("_OUTLINED_FUNCTION_")
    }
    calls: dict[str, list[str]] = defaultdict(list)
    current = ""
    label = re.compile(r"^[0-9a-f]+ <([^>]+)>:$")
    target = re.compile(r"ARM64_RELOC_BRANCH26\s+(_OUTLINED_FUNCTION_\d+)")
    for line in capture([objdump, "-dr", "--symbolize-operands", str(machine)]).splitlines():
        match = label.match(line)
        if match:
            current = match.group(1)
            continue
        match = target.search(line)
        if match and current and not current.startswith("_OUTLINED_FUNCTION_"):
            calls[match.group(1)].append(current)
    call_rows: list[dict[str, Any]] = []
    for target_name, callers in calls.items():
        size = symbol_sizes.get(target_name, 0)
        count = len(callers)
        call_rows.append({
            "outlined_function": target_name,
            "thunk_vm_bytes": size,
            "call_count": count,
            "distinct_callers": len(set(callers)),
            "reuse_score": max(0, count - 1) * max(0, size - 4),
            "callers": ";".join(sorted(set(callers))),
        })
    call_rows.sort(key=lambda item: int(item["reuse_score"]), reverse=True)
    return oracle_rows, call_rows


def markdown(
    release: dict[str, str], size_report: dict[str, Any],
    sections: list[dict[str, Any]], families: list[dict[str, Any]],
    machine_clones: list[dict[str, Any]], ir_clones: list[dict[str, Any]],
    oracles: list[dict[str, Any]], outliner_calls: list[dict[str, Any]],
    bloaty_version: str,
) -> str:
    variants = {item["kind"]: item for item in size_report["variants"]}
    literal = variants["literal translated C89"]
    canonical = variants["idiomatic C89"]
    literal_objects = {item["name"]: item for item in literal["objects"]}
    canonical_objects = {item["name"]: item for item in canonical["objects"]}
    section_totals: Counter[str] = Counter()
    for row in sections:
        if row["variant"] == "idiomatic C89":
            section_totals[str(row["section"])] += int(row["file_bytes"])
    control = next(item for item in oracles if item["scenario"] == "control")
    lines = [
        "# Idiomatic C89 size-opportunity report", "",
        f"Dear ImGui `{release['tag']}` at `{release['commit']}`. "
        f"Bloaty `{bloaty_version}` and LLVM size/reuse oracles use "
        f"`{' '.join(SIZE_FLAGS)}`.", "",
        "Compiler transforms in this report are diagnostic upper bounds against a "
        "single combined-bitcode control. They do not alter the idiomatic build.", "",
        "## Current idiomatic result", "",
        "| Build | Archive bytes | Object bytes | Loaded bytes |", "|---|---:|---:|---:|",
        f"| Literal C89 | {literal['archive_bytes']:,} | {literal['object_file_bytes']:,} | {literal['section_bytes']:,} |",
        f"| Idiomatic C89 | {canonical['archive_bytes']:,} | {canonical['object_file_bytes']:,} | {canonical['section_bytes']:,} |",
        "", "### Per-translation-unit reduction", "",
        "| Object | Literal file | Idiomatic file | File delta | Literal loaded | Idiomatic loaded | Loaded delta |",
        "|---|---:|---:|---:|---:|---:|---:|",
    ]
    for name in sorted(canonical_objects):
        before = literal_objects[name]
        after = canonical_objects[name]
        lines.append(
            f"| `{name}` | {before['file_bytes']:,} | {after['file_bytes']:,} | "
            f"{int(after['file_bytes']) - int(before['file_bytes']):+,} | "
            f"{before['section_bytes']:,} | {after['section_bytes']:,} | "
            f"{int(after['section_bytes']) - int(before['section_bytes']):+,} |"
        )
    lines.extend([
        "", "## LLVM reuse upper bounds", "",
        "| Scenario | Loaded bytes | Delta vs control | File bytes | File delta | Text bytes | Symbols |",
        "|---|---:|---:|---:|---:|---:|---:|",
    ])
    order = {
        "control": 0, "constmerge": 1, "mergefunc": 2,
        "iroutliner": 3, "all_ir": 4, "machine_outliner": 5,
        "all_ir_machine_outliner": 6,
    }
    for row in sorted(oracles, key=lambda item: order[item["scenario"]]):
        lines.append(
            f"| `{row['scenario']}` | {row['loaded_section_bytes']:,} | "
            f"{row['loaded_delta_bytes']:+,} | {row['file_bytes']:,} | "
            f"{row['file_delta_bytes']:+,} | {row['text_bytes']:,} | "
            f"{row['symbol_count']:,} |"
        )
    machine = next(item for item in oracles if item["scenario"] == "machine_outliner")
    lines.extend([
        "",
        f"The aggressive ARM64 machine outliner finds "
        f"**{-int(machine['loaded_delta_bytes']):,} loaded bytes** of repeated "
        f"instruction sequences, but changes the combined object by "
        f"**{int(machine['file_delta_bytes']):+,} file bytes** and emits "
        f"{sum(1 for item in outliner_calls):,} called outline thunks. It is a "
        "strong discovery oracle, not an idiomatic-build compiler flag.", "",
        "## Largest idiomatic Mach-O file categories", "",
        "| Bloaty category | File bytes |", "|---|---:|",
    ])
    for name, value in section_totals.most_common(12):
        lines.append(f"| `{name}` | {value:,} |")
    lines.extend([
        "", "## Largest remaining template families", "",
        "Theoretical shared bytes are a ranking heuristic: total family VM bytes "
        "minus its largest member, before helper-call and semantic costs.", "",
        "| Family | Instances | VM bytes | Heuristic shared bytes |",
        "|---|---:|---:|---:|",
    ])
    for row in families[:20]:
        lines.append(
            f"| `{row['family']}` | {row['instances']} | {row['total_vm_bytes']:,} | "
            f"{row['theoretical_shared_vm_bytes']:,} |"
        )
    aware = [item for item in machine_clones if item["kind"] == "relocation-aware"]
    raw = [item for item in machine_clones if item["kind"] == "raw-bytes"]
    lines.extend([
        "", "## Clone inventories", "",
        f"Relocation-aware exact machine-code groups: **{len(aware):,}** "
        f"({sum(int(item['duplicate_vm_bytes']) for item in aware):,} duplicate VM bytes).",
        f"Raw-byte groups ignoring relocation targets: **{len(raw):,}** "
        f"({sum(int(item['duplicate_vm_bytes']) for item in raw):,} potential parameterization bytes).",
        f"Extracted-IR clone groups: **{len(ir_clones):,}** across exact, "
        "type-normalized, and literal-normalized fingerprints.", "",
        "### Largest relocation-aware exact function clones", "",
        "These are the safest whole-function factoring leads, though a shared "
        "helper still pays call and argument-setup costs.", "",
        "| Instances | Duplicate VM bytes | Examples |", "|---:|---:|---|",
    ])
    for row in aware[:10]:
        lines.append(
            f"| {row['instances']} | {row['duplicate_vm_bytes']:,} | "
            f"{compact_names(str(row['symbols']))} |"
        )
    normalized_ir = [
        item for item in ir_clones if item["kind"] == "type-normalized"
    ]
    lines.extend([
        "", "### Largest type-normalized source-shape clones", "",
        "This view catches boilerplate before code generation. Large template "
        "groups can already be folded by the idiomatic emitter, so this is a "
        "lead list rather than an estimate of remaining object bytes.", "",
        "| Instances | Duplicate IR nodes | Examples |", "|---:|---:|---|",
    ])
    for row in normalized_ir[:10]:
        lines.append(
            f"| {row['instances']} | {row['duplicate_nodes']:,} | "
            f"{compact_names(str(row['functions']))} |"
        )
    lines.extend([
        "",
        "## Highest-scoring machine-outliner clusters", "",
        "| Thunk | Bytes | Calls | Callers | Reuse score |", "|---|---:|---:|---:|---:|",
    ])
    for row in outliner_calls[:20]:
        lines.append(
            f"| `{row['outlined_function']}` | {row['thunk_vm_bytes']} | "
            f"{row['call_count']} | {row['distinct_callers']} | {row['reuse_score']:,} |"
        )
    lines.extend([
        "", "## Artifacts", "",
        "- `bloaty_sections.csv`: VM/file category attribution per object and build.",
        "- `bloaty_idiomatic_vs_literal.csv`: section and symbol deltas.",
        "- `function_sizes.csv`: Bloaty function sizes with template-family labels.",
        "- `template_families.csv`: ranked template-instantiation families.",
        "- `machine_code_clones.csv`: relocation-aware and raw-byte clone groups.",
        "- `ir_clones.csv`: normalized extracted-IR body fingerprints.",
        "- `llvm_upper_bounds.csv`: compiler reuse-oracle measurements.",
        "- `machine_outliner_calls.csv`: outlined sequences ranked by reuse score.",
        "", f"Combined-bitcode control: {control['loaded_section_bytes']:,} loaded bytes.", "",
    ])
    return "\n".join(lines)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    output = root / "build/size-opportunities"
    output.mkdir(parents=True, exist_ok=True)
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    release = lock["latest_release"]
    size_report_path = root / "build/full-library-size/report.json"
    if not size_report_path.exists():
        raise SystemExit("missing idiomatic size report; run make translator-idiomatic-c89-size")
    size_report = json.loads(size_report_path.read_text())
    if (size_report.get("upstream_commit") != release["commit"]
            or "idiomatic C89" not in {item["kind"] for item in size_report["variants"]}):
        raise SystemExit("idiomatic size report is stale; run make translator-idiomatic-c89-size")

    cc = tool("CC", "/opt/homebrew/opt/llvm/bin/clang", "clang")
    llvm_link = tool("LLVM_LINK", "/opt/homebrew/opt/llvm/bin/llvm-link", "llvm-link")
    opt = tool("OPT", "/opt/homebrew/opt/llvm/bin/opt", "opt")
    objdump = tool("OBJDUMP", "/opt/homebrew/opt/llvm/bin/llvm-objdump", "llvm-objdump")
    size_tool = tool("SIZE", "/opt/homebrew/opt/llvm/bin/llvm-size", "size")
    bloaty = tool("BLOATY", "/opt/homebrew/bin/bloaty", "bloaty")
    bloaty_version = capture([bloaty, "--version"]).strip().split()[-1]

    sections, functions, deltas = bloaty_inventory(
        root, output, bloaty, objdump, size_report,
    )
    families = template_family_rows(functions)
    write_csv(output / "template_families.csv", [
        "family", "instances", "total_vm_bytes", "largest_vm_bytes",
        "theoretical_shared_vm_bytes", "symbols",
    ], families)

    canonical_objects = [
        root / "build/full-library-size/c89-idiomatic" / f"{stem}.o"
        for stem in CORE_STEMS
    ]
    canonical_sizes = {
        (item["object"], item["symbol"]): int(item["vm_bytes"])
        for item in functions if item["variant"] == "idiomatic C89"
    }
    machine_clones = disassembly_fingerprints(
        objdump, canonical_objects, canonical_sizes,
    )
    write_csv(output / "machine_code_clones.csv", [
        "kind", "fingerprint", "instances", "total_vm_bytes",
        "duplicate_vm_bytes", "symbols",
    ], machine_clones)

    ir_clones = ir_clone_rows(root)
    write_csv(output / "ir_clones.csv", [
        "kind", "fingerprint", "instances", "total_nodes",
        "duplicate_nodes", "functions",
    ], ir_clones)

    oracles, outliner_calls = compile_llvm_oracles(
        root, output, cc, llvm_link, opt, size_tool, bloaty, objdump,
    )
    write_csv(output / "llvm_upper_bounds.csv", [
        "scenario", "file_bytes", "loaded_section_bytes", "text_bytes",
        "symbol_count", "file_delta_bytes", "loaded_delta_bytes",
    ], oracles)
    write_csv(output / "machine_outliner_calls.csv", [
        "outlined_function", "thunk_vm_bytes", "call_count",
        "distinct_callers", "reuse_score", "callers",
    ], outliner_calls)

    report: dict[str, Any] = {
        "upstream_tag": release["tag"],
        "upstream_commit": release["commit"],
        "flags": list(SIZE_FLAGS),
        "bloaty_version": bloaty_version,
        "idiomatic_size_report": size_report,
        "bloaty_sections": sections,
        "bloaty_deltas": deltas,
        "template_families": families,
        "machine_code_clones": machine_clones,
        "ir_clones": ir_clones,
        "llvm_upper_bounds": oracles,
        "machine_outliner_calls": outliner_calls,
    }
    (output / "report.json").write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8",
    )
    text = markdown(
        release, size_report, sections, families, machine_clones, ir_clones,
        oracles, outliner_calls, bloaty_version,
    )
    (output / "report.md").write_text(text, encoding="utf-8")
    print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
