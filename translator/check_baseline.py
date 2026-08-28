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


def assert_idiomatic_table_style(
    root: Path, generated: Path, units: list[dict], ir_data: dict
) -> None:
    """Lock the maintained table TU's private namespace and C style."""
    options = ir_data.get("translator_options", {})
    internal_namespace = options.get("internal_namespace", "imgui_i_")
    checked_names = [
        "imgui_c89.h",
        "imgui_c89_internal.h",
        "imgui_c89_api.h",
        "imgui_c89_api.c",
        "imgui_translated.c",
        *(item["source"] for item in units),
    ]
    if internal_namespace != "imgui_i_":
        leaks = [
            name for name in checked_names
            if re.search(r"\bimgui_i_", (
                generated / name
            ).read_text(encoding="utf-8"))
        ]
        if leaks:
            raise RuntimeError(
                "legacy imgui_i_ private namespace leaked into: "
                + ", ".join(leaks)
            )

    if not options.get("format_table_source"):
        return
    sys.path.insert(0, str(root / "translator/py"))
    from imgui_translator.emit import format_table_c89_source

    table_path = generated / "imgui_tables.c"
    table_source = table_path.read_text(encoding="utf-8")
    if format_table_c89_source(table_source) != table_source:
        raise RuntimeError(
            "imgui_tables.c is not in the maintained K&R/braced C89 style"
        )
    print("idiomatic table style: PASS (imgui__ namespace, K&R, braced controls)")


def assert_flattened_table_spans(
    generated: Path,
    units: list[dict],
    ir_data: dict,
    cc: str,
    cxx: str,
    facade_standard: str,
    upstream: Path,
    define_flags: list[str],
    undefine_flags: list[str],
    dependency_cflags: list[str],
) -> None:
    """Prove the table span lowering is complete and layout-compatible."""
    if not ir_data.get("translator_options", {}).get("flatten_table_spans"):
        return

    checked_names = [
        "imgui_c89.h",
        "imgui_c89_internal.h",
        "imgui_c89_api.h",
        "imgui_c89_api.c",
        "imgui_translated.h",
        "imgui_translated.c",
        *(item["source"] for item in units),
    ]
    leaks: list[str] = []
    for name in checked_names:
        text = (generated / name).read_text(encoding="utf-8")
        tokens = [
            token for token in ("ImSpan_", "ImSpanAllocator") if token in text
        ]
        if tokens:
            leaks.append(f"{name} ({', '.join(tokens)})")
    if leaks:
        raise RuntimeError(
            "flattened table spans leaked into canonical C output: "
            + ", ".join(leaks)
        )

    internal = (generated / "imgui_c89_internal.h").read_text(encoding="utf-8")

    def record_body(name: str) -> str:
        match = re.search(
            rf"struct {re.escape(name)}\s*\{{(.*?)\n\}};",
            internal,
            flags=re.DOTALL,
        )
        if match is None:
            raise RuntimeError(f"flattened table span owner {name} is missing")
        return match.group(1)

    expected_fields = {
        "ImGuiTable": (
            "ImGuiTableColumn * Columns;",
            "ImGuiTableColumn * ColumnsEnd;",
            "ImGuiTableColumnIdx * DisplayOrderToIndex;",
            "ImGuiTableColumnIdx * DisplayOrderToIndexEnd;",
            "ImGuiTableCellData * RowCellData;",
            "ImGuiTableCellData * RowCellDataEnd;",
        ),
        "ImGuiTableTempData": (
            "ImGuiTableColumn * OldColumnsData;",
            "ImGuiTableColumn * OldColumnsDataEnd;",
        ),
    }
    for record, declarations in expected_fields.items():
        body = record_body(record)
        missing = [declaration for declaration in declarations if declaration not in body]
        if missing:
            raise RuntimeError(
                f"flattened {record} pointer fields are incomplete: "
                + ", ".join(missing)
            )

    c_probe = generated / "table_span_layout.c"
    cpp_probe = generated / "table_span_layout.cpp"
    c_probe.write_text(
        '''#include "imgui_c89_internal.h"
#include <stddef.h>
#include <stdio.h>

#define PRINT_LAYOUT(key, value) printf(key "=%lu\\n", (unsigned long)(value))

int main(void)
{
    PRINT_LAYOUT("ImGuiTable.size", sizeof(ImGuiTable));
    PRINT_LAYOUT("ImGuiTable.Columns", offsetof(ImGuiTable, Columns));
    PRINT_LAYOUT("ImGuiTable.ColumnsEnd", offsetof(ImGuiTable, ColumnsEnd));
    PRINT_LAYOUT("ImGuiTable.DisplayOrderToIndex", offsetof(ImGuiTable, DisplayOrderToIndex));
    PRINT_LAYOUT("ImGuiTable.DisplayOrderToIndexEnd", offsetof(ImGuiTable, DisplayOrderToIndexEnd));
    PRINT_LAYOUT("ImGuiTable.RowCellData", offsetof(ImGuiTable, RowCellData));
    PRINT_LAYOUT("ImGuiTable.RowCellDataEnd", offsetof(ImGuiTable, RowCellDataEnd));
    PRINT_LAYOUT("ImGuiTableTempData.size", sizeof(ImGuiTableTempData));
    PRINT_LAYOUT("ImGuiTableTempData.OldColumnsData", offsetof(ImGuiTableTempData, OldColumnsData));
    PRINT_LAYOUT("ImGuiTableTempData.OldColumnsDataEnd", offsetof(ImGuiTableTempData, OldColumnsDataEnd));
    return 0;
}
''',
        encoding="utf-8",
    )
    cpp_probe.write_text(
        '''#include "imgui.h"
#include "imgui_internal.h"
#include <stddef.h>
#include <stdio.h>

#define PRINT_LAYOUT(key, value) printf(key "=%lu\\n", (unsigned long)(value))

int main()
{
    PRINT_LAYOUT("ImGuiTable.size", sizeof(ImGuiTable));
    PRINT_LAYOUT("ImGuiTable.Columns", offsetof(ImGuiTable, Columns) + offsetof(ImSpan<ImGuiTableColumn>, Data));
    PRINT_LAYOUT("ImGuiTable.ColumnsEnd", offsetof(ImGuiTable, Columns) + offsetof(ImSpan<ImGuiTableColumn>, DataEnd));
    PRINT_LAYOUT("ImGuiTable.DisplayOrderToIndex", offsetof(ImGuiTable, DisplayOrderToIndex) + offsetof(ImSpan<ImGuiTableColumnIdx>, Data));
    PRINT_LAYOUT("ImGuiTable.DisplayOrderToIndexEnd", offsetof(ImGuiTable, DisplayOrderToIndex) + offsetof(ImSpan<ImGuiTableColumnIdx>, DataEnd));
    PRINT_LAYOUT("ImGuiTable.RowCellData", offsetof(ImGuiTable, RowCellData) + offsetof(ImSpan<ImGuiTableCellData>, Data));
    PRINT_LAYOUT("ImGuiTable.RowCellDataEnd", offsetof(ImGuiTable, RowCellData) + offsetof(ImSpan<ImGuiTableCellData>, DataEnd));
    PRINT_LAYOUT("ImGuiTableTempData.size", sizeof(ImGuiTableTempData));
    PRINT_LAYOUT("ImGuiTableTempData.OldColumnsData", offsetof(ImGuiTableTempData, OldColumnsData) + offsetof(ImSpan<ImGuiTableColumn>, Data));
    PRINT_LAYOUT("ImGuiTableTempData.OldColumnsDataEnd", offsetof(ImGuiTableTempData, OldColumnsData) + offsetof(ImSpan<ImGuiTableColumn>, DataEnd));
    return 0;
}
''',
        encoding="utf-8",
    )
    c_executable = generated / "table_span_layout_c"
    cpp_executable = generated / "table_span_layout_cpp"
    run([
        cc, "-std=c89", "-pedantic-errors", "-Wall",
        "-Wno-overlength-strings", "-I", str(generated),
        *define_flags, *undefine_flags, *dependency_cflags,
        str(c_probe), "-o", str(c_executable),
    ])
    run([
        cxx, f"-std={facade_standard}", "-pedantic-errors", "-Wall",
        "-I", str(upstream), "-I", str(upstream / "backends"),
        *define_flags, *undefine_flags, *dependency_cflags,
        str(cpp_probe), "-o", str(cpp_executable),
    ])
    c_layout = capture([str(c_executable)])
    cpp_layout = capture([str(cpp_executable)])
    if c_layout != cpp_layout:
        raise RuntimeError(
            "flattened table span ABI differs from upstream C++:\n"
            f"C89:\n{c_layout}\nC++:\n{cpp_layout}"
        )
    print("flattened table spans: PASS (no C span shell, C/C++ ABI match)")


def assert_flattened_table_vectors(
    generated: Path,
    units: list[dict],
    ir_data: dict,
    cc: str,
    cxx: str,
    facade_standard: str,
    upstream: Path,
    define_flags: list[str],
    undefine_flags: list[str],
    dependency_cflags: list[str],
) -> None:
    """Prove table-owned vector shells were lowered without changing ABI."""
    if not ir_data.get("translator_options", {}).get("flatten_table_vectors"):
        return

    checked_names = [
        "imgui_c89.h",
        "imgui_c89_internal.h",
        "imgui_c89_api.h",
        "imgui_c89_api.c",
        "imgui_translated.h",
        "imgui_translated.c",
        *(item["source"] for item in units),
    ]
    specializations = (
        "ImVector_ImGuiTableInstanceData",
        "ImVector_ImGuiTableColumnSortSpecs",
        "ImVector_ImGuiTableHeaderData",
        "ImVector_ImGuiTableReconcileColumnData",
        "ImVector_ImGuiTableTempData",
    )
    leaks: list[str] = []
    for name in checked_names:
        text = (generated / name).read_text(encoding="utf-8")
        tokens = [token for token in specializations if token in text]
        if tokens:
            leaks.append(f"{name} ({', '.join(tokens)})")
    if leaks:
        raise RuntimeError(
            "flattened table vectors leaked into canonical C output: "
            + ", ".join(leaks)
        )

    internal = (generated / "imgui_c89_internal.h").read_text(encoding="utf-8")

    def record_body(name: str) -> str:
        match = re.search(
            rf"struct {re.escape(name)}\s*\{{(.*?)\n\}};",
            internal,
            flags=re.DOTALL,
        )
        if match is None:
            raise RuntimeError(f"flattened table vector owner {name} is missing")
        return match.group(1)

    expected_fields = {
        "ImGuiTable": (
            "int InstanceDataExtraSize;",
            "int InstanceDataExtraCapacity;",
            "ImGuiTableInstanceData * InstanceDataExtra;",
            "int SortSpecsMultiSize;",
            "int SortSpecsMultiCapacity;",
            "ImGuiTableColumnSortSpecs * SortSpecsMulti;",
        ),
        "ImGuiTableTempData": (
            "int AngledHeadersRequestsSize;",
            "int AngledHeadersRequestsCapacity;",
            "ImGuiTableHeaderData * AngledHeadersRequests;",
            "int ReconcileColumnsRequestsSize;",
            "int ReconcileColumnsRequestsCapacity;",
            "ImGuiTableReconcileColumnData * ReconcileColumnsRequests;",
        ),
        "ImGuiContext": (
            "int TablesTempDataSize;",
            "int TablesTempDataCapacity;",
            "ImGuiTableTempData * TablesTempData;",
        ),
    }
    for record, declarations in expected_fields.items():
        body = record_body(record)
        missing = [declaration for declaration in declarations if declaration not in body]
        if missing:
            raise RuntimeError(
                f"flattened {record} vector fields are incomplete: "
                + ", ".join(missing)
            )

    c_probe = generated / "table_vector_layout.c"
    cpp_probe = generated / "table_vector_layout.cpp"
    c_probe.write_text(
        '''#include "imgui_c89_internal.h"
#include <stddef.h>
#include <stdio.h>

#define PRINT_LAYOUT(key, value) printf(key "=%lu\\n", (unsigned long)(value))

int main(void)
{
    PRINT_LAYOUT("ImGuiTable.size", sizeof(ImGuiTable));
    PRINT_LAYOUT("ImGuiTable.InstanceDataExtra.Size", offsetof(ImGuiTable, InstanceDataExtraSize));
    PRINT_LAYOUT("ImGuiTable.InstanceDataExtra.Capacity", offsetof(ImGuiTable, InstanceDataExtraCapacity));
    PRINT_LAYOUT("ImGuiTable.InstanceDataExtra.Data", offsetof(ImGuiTable, InstanceDataExtra));
    PRINT_LAYOUT("ImGuiTable.SortSpecsMulti.Size", offsetof(ImGuiTable, SortSpecsMultiSize));
    PRINT_LAYOUT("ImGuiTable.SortSpecsMulti.Capacity", offsetof(ImGuiTable, SortSpecsMultiCapacity));
    PRINT_LAYOUT("ImGuiTable.SortSpecsMulti.Data", offsetof(ImGuiTable, SortSpecsMulti));
    PRINT_LAYOUT("ImGuiTableTempData.size", sizeof(ImGuiTableTempData));
    PRINT_LAYOUT("ImGuiTableTempData.AngledHeadersRequests.Size", offsetof(ImGuiTableTempData, AngledHeadersRequestsSize));
    PRINT_LAYOUT("ImGuiTableTempData.AngledHeadersRequests.Capacity", offsetof(ImGuiTableTempData, AngledHeadersRequestsCapacity));
    PRINT_LAYOUT("ImGuiTableTempData.AngledHeadersRequests.Data", offsetof(ImGuiTableTempData, AngledHeadersRequests));
    PRINT_LAYOUT("ImGuiTableTempData.ReconcileColumnsRequests.Size", offsetof(ImGuiTableTempData, ReconcileColumnsRequestsSize));
    PRINT_LAYOUT("ImGuiTableTempData.ReconcileColumnsRequests.Capacity", offsetof(ImGuiTableTempData, ReconcileColumnsRequestsCapacity));
    PRINT_LAYOUT("ImGuiTableTempData.ReconcileColumnsRequests.Data", offsetof(ImGuiTableTempData, ReconcileColumnsRequests));
    PRINT_LAYOUT("ImGuiContext.size", sizeof(ImGuiContext));
    PRINT_LAYOUT("ImGuiContext.TablesTempData.Size", offsetof(ImGuiContext, TablesTempDataSize));
    PRINT_LAYOUT("ImGuiContext.TablesTempData.Capacity", offsetof(ImGuiContext, TablesTempDataCapacity));
    PRINT_LAYOUT("ImGuiContext.TablesTempData.Data", offsetof(ImGuiContext, TablesTempData));
    return 0;
}
''',
        encoding="utf-8",
    )
    cpp_probe.write_text(
        '''#include "imgui.h"
#include "imgui_internal.h"
#include <stddef.h>
#include <stdio.h>

#define PRINT_LAYOUT(key, value) printf(key "=%lu\\n", (unsigned long)(value))
#define VECTOR_OFFSET(owner, field, type, member) (offsetof(owner, field) + offsetof(ImVector<type>, member))

int main()
{
    PRINT_LAYOUT("ImGuiTable.size", sizeof(ImGuiTable));
    PRINT_LAYOUT("ImGuiTable.InstanceDataExtra.Size", VECTOR_OFFSET(ImGuiTable, InstanceDataExtra, ImGuiTableInstanceData, Size));
    PRINT_LAYOUT("ImGuiTable.InstanceDataExtra.Capacity", VECTOR_OFFSET(ImGuiTable, InstanceDataExtra, ImGuiTableInstanceData, Capacity));
    PRINT_LAYOUT("ImGuiTable.InstanceDataExtra.Data", VECTOR_OFFSET(ImGuiTable, InstanceDataExtra, ImGuiTableInstanceData, Data));
    PRINT_LAYOUT("ImGuiTable.SortSpecsMulti.Size", VECTOR_OFFSET(ImGuiTable, SortSpecsMulti, ImGuiTableColumnSortSpecs, Size));
    PRINT_LAYOUT("ImGuiTable.SortSpecsMulti.Capacity", VECTOR_OFFSET(ImGuiTable, SortSpecsMulti, ImGuiTableColumnSortSpecs, Capacity));
    PRINT_LAYOUT("ImGuiTable.SortSpecsMulti.Data", VECTOR_OFFSET(ImGuiTable, SortSpecsMulti, ImGuiTableColumnSortSpecs, Data));
    PRINT_LAYOUT("ImGuiTableTempData.size", sizeof(ImGuiTableTempData));
    PRINT_LAYOUT("ImGuiTableTempData.AngledHeadersRequests.Size", VECTOR_OFFSET(ImGuiTableTempData, AngledHeadersRequests, ImGuiTableHeaderData, Size));
    PRINT_LAYOUT("ImGuiTableTempData.AngledHeadersRequests.Capacity", VECTOR_OFFSET(ImGuiTableTempData, AngledHeadersRequests, ImGuiTableHeaderData, Capacity));
    PRINT_LAYOUT("ImGuiTableTempData.AngledHeadersRequests.Data", VECTOR_OFFSET(ImGuiTableTempData, AngledHeadersRequests, ImGuiTableHeaderData, Data));
    PRINT_LAYOUT("ImGuiTableTempData.ReconcileColumnsRequests.Size", VECTOR_OFFSET(ImGuiTableTempData, ReconcileColumnsRequests, ImGuiTableReconcileColumnData, Size));
    PRINT_LAYOUT("ImGuiTableTempData.ReconcileColumnsRequests.Capacity", VECTOR_OFFSET(ImGuiTableTempData, ReconcileColumnsRequests, ImGuiTableReconcileColumnData, Capacity));
    PRINT_LAYOUT("ImGuiTableTempData.ReconcileColumnsRequests.Data", VECTOR_OFFSET(ImGuiTableTempData, ReconcileColumnsRequests, ImGuiTableReconcileColumnData, Data));
    PRINT_LAYOUT("ImGuiContext.size", sizeof(ImGuiContext));
    PRINT_LAYOUT("ImGuiContext.TablesTempData.Size", VECTOR_OFFSET(ImGuiContext, TablesTempData, ImGuiTableTempData, Size));
    PRINT_LAYOUT("ImGuiContext.TablesTempData.Capacity", VECTOR_OFFSET(ImGuiContext, TablesTempData, ImGuiTableTempData, Capacity));
    PRINT_LAYOUT("ImGuiContext.TablesTempData.Data", VECTOR_OFFSET(ImGuiContext, TablesTempData, ImGuiTableTempData, Data));
    return 0;
}
''',
        encoding="utf-8",
    )
    c_executable = generated / "table_vector_layout_c"
    cpp_executable = generated / "table_vector_layout_cpp"
    run([
        cc, "-std=c89", "-pedantic-errors", "-Wall",
        "-Wno-overlength-strings", "-I", str(generated),
        *define_flags, *undefine_flags, *dependency_cflags,
        str(c_probe), "-o", str(c_executable),
    ])
    run([
        cxx, f"-std={facade_standard}", "-pedantic-errors", "-Wall",
        "-I", str(upstream), "-I", str(upstream / "backends"),
        *define_flags, *undefine_flags, *dependency_cflags,
        str(cpp_probe), "-o", str(cpp_executable),
    ])
    c_layout = capture([str(c_executable)])
    cpp_layout = capture([str(cpp_executable)])
    if c_layout != cpp_layout:
        raise RuntimeError(
            "flattened table vector ABI differs from upstream C++:\n"
            f"C89:\n{c_layout}\nC++:\n{cpp_layout}"
        )
    print("flattened table vectors: PASS (no specialized C shell, C/C++ ABI match)")


def assert_flattened_table_pool(
    generated: Path,
    units: list[dict],
    ir_data: dict,
    cc: str,
    cxx: str,
    facade_standard: str,
    upstream: Path,
    define_flags: list[str],
    undefine_flags: list[str],
    dependency_cflags: list[str],
) -> None:
    """Prove the table pool is direct C storage with the upstream layout."""
    if not ir_data.get("translator_options", {}).get("flatten_table_pool"):
        return
    internal_namespace = ir_data.get("translator_options", {}).get(
        "internal_namespace", "imgui_i_"
    )

    checked_names = [
        "imgui_c89.h",
        "imgui_c89_internal.h",
        "imgui_c89_api.h",
        "imgui_c89_api.c",
        "imgui_translated.h",
        "imgui_translated.c",
        *(item["source"] for item in units),
    ]
    shell_patterns = (
        re.compile(r"\bImPool_ImGuiTable(?:\b|__)"),
        re.compile(r"\bImVector_ImGuiTable(?:\b|__)"),
    )
    leaks: list[str] = []
    canonical_text: list[str] = []
    for name in checked_names:
        text = (generated / name).read_text(encoding="utf-8")
        canonical_text.append(text)
        tokens = [pattern.pattern for pattern in shell_patterns if pattern.search(text)]
        if tokens:
            leaks.append(f"{name} ({', '.join(tokens)})")
    if leaks:
        raise RuntimeError(
            "flattened table pool shells leaked into canonical C output: "
            + ", ".join(leaks)
        )

    internal = (generated / "imgui_c89_internal.h").read_text(encoding="utf-8")
    match = re.search(
        r"struct ImGuiTablePool\s*\{(.*?)\n\};",
        internal,
        flags=re.DOTALL,
    )
    if match is None:
        raise RuntimeError("flattened table pool owner ImGuiTablePool is missing")
    pool_body = match.group(1)
    expected_fields = (
        "int Size;",
        "int Capacity;",
        "ImGuiTable * Data;",
        "ImGuiStorage Map;",
        "ImPoolIdx FreeIdx;",
        "ImPoolIdx AliveCount;",
    )
    missing = [field for field in expected_fields if field not in pool_body]
    if missing:
        raise RuntimeError(
            "flattened ImGuiTablePool fields are incomplete: "
            + ", ".join(missing)
        )
    context_match = re.search(
        r"struct ImGuiContext\s*\{(.*?)\n\};",
        internal,
        flags=re.DOTALL,
    )
    if context_match is None or "ImGuiTablePool Tables;" not in context_match.group(1):
        raise RuntimeError("ImGuiContext Tables is not a direct ImGuiTablePool owner")

    all_c = "\n".join(canonical_text)
    required_surface = tuple(
        internal_namespace + "table_pool_" + operation + "("
        for operation in (
            "init", "fini", "clear", "add", "find", "at", "index",
            "get_or_add", "alive_count", "map_size", "map_at", "remove",
            "remove_at",
        )
    ) + (internal_namespace + "table_fini(",)
    missing_surface = [name for name in required_surface if name not in all_c]
    if missing_surface:
        raise RuntimeError(
            "flattened table pool stable C surface is incomplete: "
            + ", ".join(missing_surface)
        )

    c_probe = generated / "table_pool_layout.c"
    cpp_probe = generated / "table_pool_layout.cpp"
    c_probe.write_text(
        '''#include "imgui_c89_internal.h"
#include <stddef.h>
#include <stdio.h>

#define PRINT_LAYOUT(key, value) printf(key "=%lu\\n", (unsigned long)(value))

int main(void)
{
    PRINT_LAYOUT("ImGuiTablePool.size", sizeof(ImGuiTablePool));
    PRINT_LAYOUT("ImGuiTablePool.Size", offsetof(ImGuiTablePool, Size));
    PRINT_LAYOUT("ImGuiTablePool.Capacity", offsetof(ImGuiTablePool, Capacity));
    PRINT_LAYOUT("ImGuiTablePool.Data", offsetof(ImGuiTablePool, Data));
    PRINT_LAYOUT("ImGuiTablePool.Map", offsetof(ImGuiTablePool, Map));
    PRINT_LAYOUT("ImGuiTablePool.FreeIdx", offsetof(ImGuiTablePool, FreeIdx));
    PRINT_LAYOUT("ImGuiTablePool.AliveCount", offsetof(ImGuiTablePool, AliveCount));
    PRINT_LAYOUT("ImGuiContext.Tables.offset", offsetof(ImGuiContext, Tables));
    PRINT_LAYOUT("ImGuiContext.Tables.size", sizeof(((ImGuiContext *)0)->Tables));
    return 0;
}
''',
        encoding="utf-8",
    )
    cpp_probe.write_text(
        '''#include "imgui.h"
#include "imgui_internal.h"
#include <stddef.h>
#include <stdio.h>

#define PRINT_LAYOUT(key, value) printf(key "=%lu\\n", (unsigned long)(value))
#define POOL_VECTOR_OFFSET(member) (offsetof(ImPool<ImGuiTable>, Buf) + offsetof(ImVector<ImGuiTable>, member))

int main()
{
    PRINT_LAYOUT("ImGuiTablePool.size", sizeof(ImPool<ImGuiTable>));
    PRINT_LAYOUT("ImGuiTablePool.Size", POOL_VECTOR_OFFSET(Size));
    PRINT_LAYOUT("ImGuiTablePool.Capacity", POOL_VECTOR_OFFSET(Capacity));
    PRINT_LAYOUT("ImGuiTablePool.Data", POOL_VECTOR_OFFSET(Data));
    PRINT_LAYOUT("ImGuiTablePool.Map", offsetof(ImPool<ImGuiTable>, Map));
    PRINT_LAYOUT("ImGuiTablePool.FreeIdx", offsetof(ImPool<ImGuiTable>, FreeIdx));
    PRINT_LAYOUT("ImGuiTablePool.AliveCount", offsetof(ImPool<ImGuiTable>, AliveCount));
    PRINT_LAYOUT("ImGuiContext.Tables.offset", offsetof(ImGuiContext, Tables));
    PRINT_LAYOUT("ImGuiContext.Tables.size", sizeof(((ImGuiContext *)0)->Tables));
    return 0;
}
''',
        encoding="utf-8",
    )
    c_executable = generated / "table_pool_layout_c"
    cpp_executable = generated / "table_pool_layout_cpp"
    run([
        cc, "-std=c89", "-pedantic-errors", "-Wall",
        "-Wno-overlength-strings", "-I", str(generated),
        *define_flags, *undefine_flags, *dependency_cflags,
        str(c_probe), "-o", str(c_executable),
    ])
    run([
        cxx, f"-std={facade_standard}", "-pedantic-errors", "-Wall",
        "-I", str(upstream), "-I", str(upstream / "backends"),
        *define_flags, *undefine_flags, *dependency_cflags,
        str(cpp_probe), "-o", str(cpp_executable),
    ])
    c_layout = capture([str(c_executable)])
    cpp_layout = capture([str(cpp_executable)])
    if c_layout != cpp_layout:
        raise RuntimeError(
            "flattened table pool ABI differs from upstream C++:\n"
            f"C89:\n{c_layout}\nC++:\n{cpp_layout}"
        )
    print("flattened table pool: PASS (direct C pool, stable surface, ABI match)")


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

    assert_flattened_table_spans(
        generated_a, units, ir_data, cc, cxx, facade_standard, upstream,
        define_flags, undefine_flags, dependency_cflags,
    )
    assert_flattened_table_vectors(
        generated_a, units, ir_data, cc, cxx, facade_standard, upstream,
        define_flags, undefine_flags, dependency_cflags,
    )
    assert_flattened_table_pool(
        generated_a, units, ir_data, cc, cxx, facade_standard, upstream,
        define_flags, undefine_flags, dependency_cflags,
    )
    assert_idiomatic_table_style(root, generated_a, units, ir_data)

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
