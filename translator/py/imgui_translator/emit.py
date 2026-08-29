"""Initial deterministic C89 and C++-facade emitter for schema version 1.

This intentionally supports only the constructs exercised by the bootstrap
fixture. Unsupported IR is a hard error with a source location; silently
copying C++ syntax into generated C is forbidden.
"""

from __future__ import annotations

import copy
import hashlib
import functools
import json
import os
import re
import shutil
import struct
import subprocess
import textwrap
from dataclasses import dataclass
from pathlib import Path
from typing import Any


class TranslationError(RuntimeError):
    pass


@dataclass(frozen=True)
class Output:
    c_header: str
    c_internal_header: str
    c_source: str
    cpp_header: str
    cpp_source: str
    native_c_header: str
    native_c_source: str
    manifest: str


def _symbol(identifier: str, qualified_name: str) -> str:
    stem = qualified_name.replace("::", "_").replace("~", "dtor_")
    stem = re.sub(r"[^A-Za-z0-9_]", "_", stem)
    digest = hashlib.sha256(identifier.encode("utf-8")).hexdigest()[:10]
    return f"{stem}__{digest}"


def _c_identifier(spelling: str) -> str:
    value = spelling.replace("::", "_")
    value = re.sub(r"[^A-Za-z0-9_]", "_", value)
    value = re.sub(r"_+", "_", value).strip("_")
    if value and value[0].isdigit():
        value = "imgui_c89_" + value
    return value


def format_table_c89_source(source: str) -> str:
    """Apply the maintained table TU's strict K&R/braced control style."""
    candidates: list[Path] = []
    configured = os.environ.get("CLANG_FORMAT")
    if configured:
        candidates.append(Path(configured))
    llvm_prefix = os.environ.get("LLVM_PREFIX")
    if llvm_prefix:
        candidates.append(Path(llvm_prefix) / "bin/clang-format")
    candidates.append(Path("/opt/homebrew/opt/llvm/bin/clang-format"))
    for command in ("clang-format-22", "clang-format"):
        found = shutil.which(command)
        if found:
            candidates.append(Path(found))
    executable = next((path for path in candidates if path.is_file()), None)
    if executable is None:
        raise TranslationError(
            "formatted table source requires LLVM 22 clang-format; set "
            "CLANG_FORMAT or LLVM_PREFIX"
        )
    version = subprocess.run(
        [str(executable), "--version"], check=True, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout
    if re.search(r"\bversion 22(?:\.|\b)", version) is None:
        raise TranslationError(
            "formatted table source requires clang-format major 22, got: "
            + version.strip()
        )
    style = (
        "{BasedOnStyle: LLVM, BreakBeforeBraces: Attach, InsertBraces: true, "
        "AllowShortIfStatementsOnASingleLine: Never, "
        "AllowShortLoopsOnASingleLine: false, "
        "AllowShortBlocksOnASingleLine: Never, "
        "AllowShortFunctionsOnASingleLine: Empty, SortIncludes: Never, "
        "ColumnLimit: 0, IndentWidth: 4, ContinuationIndentWidth: 4, "
        "UseTab: Never}"
    )
    formatted = subprocess.run(
        [str(executable), "-assume-filename=imgui_tables.c", "-style=" + style],
        check=True, text=True, input=source,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout
    if not formatted.endswith("\n"):
        formatted += "\n"
    return formatted


class Emitter:
    # Dear ImGui's window and child-window scopes are historical exceptions:
    # their End function must be called even when Begin returns false.  The
    # optional C scope helpers remove that exception by closing an inactive
    # raw scope internally. The exact C API and C++ facade bypass this table
    # and therefore retain the upstream contract verbatim.
    NATIVE_CONDITIONAL_SCOPE_ENDS = {
        "ImGui::Begin": "ImGui::End",
        "ImGui::BeginChild": "ImGui::EndChild",
    }
    OPTIONAL_NAV_PHASES = {
        "ImGui::NavUpdateCancelRequest": 0,
        "ImGui::NavUpdateContextMenuRequest": 1,
        "ImGui::NavUpdateCreateMoveRequest": 2,
        "ImGui::NavUpdateCreateTabbingRequest": 3,
        "ImGui::NavUpdateWindowing": 4,
        "ImGui::NavEndFrame": 5,
    }

    def enum_is_public(self, enum: dict[str, Any]) -> bool:
        """Return whether an enum belongs to Dear ImGui's public header."""
        location = enum.get("location", {})
        return (not self.split_public_header
                or Path(location.get("file", "")).name == "imgui.h")

    def stable_enum_constant_names(self) -> dict[str, str]:
        """Use source names for enum constants and qualify real collisions.

        Dear ImGui's public enumerators are deliberately globally prefixed and
        therefore already form an appropriate C namespace.  Identity hashes
        were a bootstrap convenience, not an ABI requirement.  A collision
        involving two public declarations is an upstream/API error; private
        collisions receive a readable owner-qualified spelling instead.
        """
        entries = [
            (enum, constant)
            for enum in self.ir.get("enums", [])
            for constant in enum.get("constants", [])
        ]
        by_spelling: dict[str, list[tuple[dict[str, Any], dict[str, Any]]]] = {}
        for enum, constant in entries:
            spelling = constant.get("name", "")
            if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", spelling) is None:
                raise TranslationError(
                    f"enum constant is not a C identifier: {spelling!r}"
                )
            by_spelling.setdefault(spelling, []).append((enum, constant))
        for spelling, declarations in by_spelling.items():
            public_ids = {
                constant["id"] for enum, constant in declarations
                if enum["id"] in self.public_enum_ids
            }
            if len(public_ids) > 1:
                raise TranslationError(
                    f"public enum constant name collision: {spelling}"
                )

        result: dict[str, str] = {}
        claimed: dict[str, str] = {}
        ordered = sorted(
            entries,
            key=lambda pair: (
                pair[0]["id"] not in self.public_enum_ids,
                pair[0].get("qualified_name", ""),
                pair[1]["id"],
            ),
        )
        for enum, constant in ordered:
            identifier = constant["id"]
            spelling = constant["name"]
            declarations = by_spelling[spelling]
            if (len({item["id"] for _, item in declarations}) == 1
                    or enum["id"] in self.public_enum_ids):
                candidate = spelling
            else:
                location = enum.get("location", {})
                source = Path(location.get("file", "enum")).stem
                owner = enum.get("qualified_name") or enum.get("name") or "enum"
                candidate = _c_identifier(
                    f"{self.internal_namespace}{source}_{location.get('line', 0)}_"
                    f"{location.get('column', 0)}_{owner}_{spelling}"
                )
            previous = claimed.get(candidate)
            if previous is not None and previous != identifier:
                raise TranslationError(
                    f"stable enum constant name collision: {candidate}"
                )
            claimed[candidate] = identifier
            result[identifier] = candidate
        return result

    def declaration_is_public(self, item: dict[str, Any]) -> bool:
        """Return whether a declaration originates in upstream imgui.h."""
        location = item.get("location", {})
        return (not self.split_public_header
                or Path(location.get("file", "")).name == "imgui.h")

    @staticmethod
    def field_needs_complete_record(field: dict[str, Any]) -> bool:
        """C fields need a complete dependency unless they store a pointer."""
        spelling = field.get("type", "")
        suffix = spelling.rsplit(">", 1)[-1] if ">" in spelling else spelling
        return "*" not in suffix and "&" not in suffix

    def configure_flattened_table_spans(self) -> None:
        """Replace table-owned two-pointer spans with ordinary C pointers."""
        configurations = (
            ("ImGuiTable", "Columns", "ImSpan<ImGuiTableColumn>",
             "ImSpan<ImGuiTableColumn>",
             "ImGuiTableColumn *", "ColumnsEnd"),
            ("ImGuiTable", "DisplayOrderToIndex",
             "ImSpan<ImGuiTableColumnIdx>", "ImSpan<short>",
             "ImGuiTableColumnIdx *",
             "DisplayOrderToIndexEnd"),
            ("ImGuiTable", "RowCellData", "ImSpan<ImGuiTableCellData>",
             "ImSpan<ImGuiTableCellData>",
             "ImGuiTableCellData *", "RowCellDataEnd"),
            ("ImGuiTableTempData", "OldColumnsData",
             "ImSpan<ImGuiTableColumn>", "ImSpan<ImGuiTableColumn>",
             "ImGuiTableColumn *",
             "OldColumnsDataEnd"),
        )
        used_spans: set[str] = set()
        for (owner_name, field_name, owner_field_type, span_name,
             pointer_type, end_name) in configurations:
            owner = self.records_by_spelling.get(owner_name)
            span = self.records_by_spelling.get(span_name)
            if owner is None or span is None:
                raise TranslationError(
                    f"flatten table spans missing {owner_name}.{field_name}"
                )
            fields = [
                field for field in owner.get("fields", [])
                if field.get("name") == field_name
            ]
            span_fields = span.get("fields", [])
            expected_span_fields = [("Data", pointer_type), ("DataEnd", pointer_type)]
            actual_span_fields = [
                (field.get("name"), field.get("type"))
                for field in span_fields
            ]
            if len(fields) != 1 or fields[0].get("type") != owner_field_type:
                raise TranslationError(
                    f"flatten table spans changed {owner_name}.{field_name}"
                )
            # ImGuiTableColumnIdx is a typedef of short in current upstream;
            # accept either spelling while requiring identical pointer halves.
            if len(span_fields) != 2 or span_fields[0].get("name") != "Data" \
                    or span_fields[1].get("name") != "DataEnd" \
                    or span_fields[0].get("type") != span_fields[1].get("type"):
                raise TranslationError(
                    f"flatten table span layout changed for {span_name}"
                )
            if span_name != "ImSpan<short>" \
                    and actual_span_fields != expected_span_fields:
                raise TranslationError(
                    f"flatten table span pointer type changed for {span_name}"
                )
            field = fields[0]
            self.flattened_table_span_fields[field["id"]] = (
                pointer_type, end_name
            )
            synthetic_id = field["id"] + "#end"
            self.field_names[synthetic_id] = end_name
            used_spans.add(span["id"])
        allocator = self.records_by_spelling.get("ImSpanAllocator<6>")
        if allocator is None:
            allocator = self.records_by_spelling.get("ImSpanAllocator_6")
        if allocator is not None:
            allocator_fields = [
                (field.get("name"), field.get("type"))
                for field in allocator.get("fields", [])
            ]
            expected = [
                ("BasePtr", "char *"), ("CurrOff", "int"),
                ("CurrIdx", "int"), ("Offsets", "int[6]"),
                ("Sizes", "int[6]"),
            ]
            if allocator_fields != expected:
                raise TranslationError("ImSpanAllocator<6> layout changed")
            used_spans.add(allocator["id"])
        self.omitted_record_ids.update(used_spans)

    def configure_flattened_table_vectors(self) -> None:
        """Expose selected table-owned ImVectors as their C storage fields."""
        configurations = (
            ("ImGuiTable", ((4736, 3584),), "InstanceDataExtra",
             "ImVector<ImGuiTableInstanceData>",
             "ImGuiTableInstanceData *"),
            ("ImGuiTable", ((4736, 3840),), "SortSpecsMulti",
             "ImVector<ImGuiTableColumnSortSpecs>",
             "ImGuiTableColumnSortSpecs *"),
            ("ImGuiTableTempData", ((1408, 128),), "AngledHeadersRequests",
             "ImVector<ImGuiTableHeaderData>", "ImGuiTableHeaderData *"),
            ("ImGuiTableTempData", ((1408, 256),), "ReconcileColumnsRequests",
             "ImVector<ImGuiTableReconcileColumnData>",
             "ImGuiTableReconcileColumnData *"),
            ("ImGuiContext", ((88000, 72000), (87744, 71744)),
             "TablesTempData",
             "ImVector<ImGuiTableTempData>", "ImGuiTableTempData *"),
        )
        for (owner_name, owner_layouts, field_name,
             vector_name, pointer_type) in configurations:
            owner = self.records_by_spelling.get(owner_name)
            vector = self.records_by_spelling.get(vector_name)
            if (owner is None or owner.get("align_bits") != 64
                    or vector is None):
                raise TranslationError(
                    f"flatten table vectors changed {owner_name}.{field_name}"
                )
            matches = [
                field for field in owner.get("fields", [])
                if field.get("name") == field_name
            ]
            expected_vector_fields = [
                ("Size", "int", 0), ("Capacity", "int", 32),
                ("Data", pointer_type, 64),
            ]
            actual_vector_fields = [
                (field.get("name"), field.get("type"),
                 field.get("offset_bits"))
                for field in vector.get("fields", [])
            ]
            if (len(matches) != 1
                    or matches[0].get("type") != vector_name
                    or (owner.get("size_bits"),
                        matches[0].get("offset_bits")) not in owner_layouts
                    or vector.get("size_bits") != 128
                    or vector.get("align_bits") != 64
                    or actual_vector_fields != expected_vector_fields):
                raise TranslationError(
                    f"flatten table vector layout changed for "
                    f"{owner_name}.{field_name}"
                )
            field = matches[0]
            size_id = field["id"] + "#size"
            capacity_id = field["id"] + "#capacity"
            size_name = field_name + "Size"
            capacity_name = field_name + "Capacity"
            self.flattened_table_vector_fields[field["id"]] = (
                pointer_type, size_name, capacity_name, vector_name
            )
            self.field_names[size_id] = size_name
            self.field_names[capacity_id] = capacity_name
            if field_name == "SortSpecsMulti":
                self.field_names[field["id"] + "#padding"] = (
                    "SortSpecsMultiPadding"
                )
            self.flattened_table_vector_record_ids.add(vector["id"])
        self.omitted_record_ids.update(self.flattened_table_vector_record_ids)

    def configure_flattened_table_pool(self) -> None:
        """Expose ImPool<ImGuiTable> as an ordinary C storage record."""
        pool = self.records_by_spelling.get("ImPool<ImGuiTable>")
        vector = self.records_by_spelling.get("ImVector<ImGuiTable>")
        if (pool is None or pool.get("size_bits") != 320
                or pool.get("align_bits") != 64 or vector is None
                or vector.get("size_bits") != 128
                or vector.get("align_bits") != 64):
            raise TranslationError("flatten table pool layout changed")
        expected_pool = [
            ("Buf", "ImVector<ImGuiTable>", 0),
            ("Map", "ImGuiStorage", 128),
            ("FreeIdx", "ImPoolIdx", 256),
            ("AliveCount", "ImPoolIdx", 288),
        ]
        expected_vector = [
            ("Size", "int", 0), ("Capacity", "int", 32),
            ("Data", "ImGuiTable *", 64),
        ]
        actual_pool = [
            (field.get("name"), field.get("type"), field.get("offset_bits"))
            for field in pool.get("fields", [])
        ]
        actual_vector = [
            (field.get("name"), field.get("type"), field.get("offset_bits"))
            for field in vector.get("fields", [])
        ]
        if actual_pool != expected_pool or actual_vector != expected_vector:
            raise TranslationError("flatten table pool fields changed")
        field = pool["fields"][0]
        self.flattened_table_vector_fields[field["id"]] = (
            "ImGuiTable *", "Size", "Capacity", "ImVector<ImGuiTable>"
        )
        self.field_names[field["id"]] = "Data"
        self.field_names[field["id"] + "#size"] = "Size"
        self.field_names[field["id"] + "#capacity"] = "Capacity"
        self.flattened_table_vector_record_ids.add(vector["id"])
        self.omitted_record_ids.add(vector["id"])
        pair = self.records_by_spelling.get("ImGuiStoragePair")
        pair_fields = pair.get("fields", []) if pair is not None else []
        if len(pair_fields) != 2 or pair_fields[1].get("name"):
            raise TranslationError("flatten table pool storage pair changed")
        value_union = self.records.get(
            pair_fields[1].get("record_dependency", "")
        )
        value_fields = value_union.get("fields", []) if value_union else []
        if [(item.get("name"), item.get("type")) for item in value_fields] != [
            ("val_i", "int"), ("val_f", "float"), ("val_p", "void *")
        ]:
            raise TranslationError("flatten table pool storage value changed")
        self.table_pool_map_int_path = (
            self.field_names[pair_fields[1]["id"]] + ".val_i"
        )

    def compute_public_complete_record_ids(self) -> set[str]:
        """Close public imgui.h records over their by-value dependencies."""
        if not self.split_public_header:
            return set(self.records)
        result = {
            identifier
            for identifier, record in self.records.items()
            if self.declaration_is_public(record)
            and "<" not in record.get("qualified_name", "")
        }
        changed = True
        while changed:
            changed = False
            for identifier in tuple(result):
                record = self.records[identifier]
                dependencies = {
                    base.get("record") for base in record.get("bases", [])
                }
                dependencies.update(
                    field.get("record_dependency")
                    for field in record.get("fields", [])
                    if self.field_needs_complete_record(field)
                )
                for dependency in dependencies:
                    if dependency in self.records and dependency not in result:
                        result.add(dependency)
                        changed = True
        return result - self.omitted_record_ids

    def record_id_for_simple_type(self, spelling: str) -> str | None:
        """Resolve an ordinary pointer/reference/array type to a record."""
        value = re.sub(r"\[[^\]]*\]", "", spelling).strip()
        value = value.replace("*", " ").replace("&", " ")
        value = re.sub(r"\b(const|volatile|struct|class|union)\b", " ", value)
        value = re.sub(r"\s+", " ", value).strip()
        record = self.records_by_spelling.get(value)
        return record.get("id") if record else None

    def compute_public_forward_record_ids(self) -> set[str]:
        """Find records whose tags are visible at the clean C API boundary."""
        result = set(self.public_complete_record_ids)
        for identifier in self.public_complete_record_ids:
            record = self.records[identifier]
            result.update(
                dependency for dependency in (
                    field.get("record_dependency")
                    for field in record.get("fields", [])
                )
                if dependency in self.records
            )
            result.update(
                base.get("record") for base in record.get("bases", [])
                if base.get("record") in self.records
            )
            for field in record.get("fields", []):
                dependency = self.record_id_for_simple_type(
                    field.get("type", "")
                )
                if dependency is not None:
                    result.add(dependency)
        for _, _, function in self.exact_c_api_functions():
            parent = function.get("parent")
            if parent in self.records:
                result.add(parent)
            spellings = [function.get("return_type", "")]
            spellings.extend(
                parameter.get("type", "")
                for parameter in function.get("parameters", [])
            )
            for spelling in spellings:
                identifier = self.record_id_for_simple_type(spelling)
                if identifier is not None:
                    result.add(identifier)
        context = self.records_by_spelling.get("ImGuiContext")
        if context is not None:
            result.add(context["id"])
        return result - self.omitted_record_ids

    def typedef_is_public(self, item: dict[str, Any]) -> bool:
        return self.declaration_is_public(item)

    def __init__(
        self,
        ir: dict[str, Any],
        linkage_analysis: tuple[
            dict[str, int], dict[str, set[str]], set[str]
        ] | None = None,
    ) -> None:
        if ir.get("schema_version") != 1:
            raise TranslationError(
                f"unsupported IR schema {ir.get('schema_version')!r}")
        self.ir = ir
        self.internal_helper_declaration_lines: list[str] = []
        self.split_public_header = any(
            Path(item.get("location", {}).get("file", "")).name == "imgui.h"
            for group in ("records", "enums", "typedefs")
            for item in ir.get(group, [])
        )
        options = ir.get("translator_options", {})
        self.internal_namespace = options.get(
            "internal_namespace", "imgui_i_"
        )
        if (not isinstance(self.internal_namespace, str)
                or re.fullmatch(
                    r"[A-Za-z_][A-Za-z0-9_]*_", self.internal_namespace
                ) is None):
            raise TranslationError("internal namespace must be a C prefix")
        self.format_table_source = bool(
            options.get("format_table_source", False)
        )
        self.omitted_call_names = set(options.get("omit_calls", []))
        self.trapped_call_names = set(options.get("trap_calls", []))
        self.noinline_function_names = set(
            options.get("noinline_functions", [])
        )
        self.handwritten_functions = options.get("handwritten_functions", {})
        if not isinstance(self.handwritten_functions, dict):
            raise TranslationError("handwritten_functions must be an object")
        self.handwritten_groups = options.get("handwritten_groups", {})
        if not isinstance(self.handwritten_groups, dict) or any(
            not isinstance(name, str) or not isinstance(group, dict)
            for name, group in self.handwritten_groups.items()
        ):
            raise TranslationError("handwritten_groups must be an object of objects")
        self.compact_zero_constructor_names = set(
            options.get("compact_zero_constructors", [])
        )
        self.packed_char_global_names = set(
            options.get("pack_four_character_globals", [])
        )
        self.packed_string_pointer_global_names = set(
            options.get("pack_string_pointer_globals", [])
        )
        self.compressed_string_pointer_global_names = set(
            options.get("compress_string_pointer_globals", [])
        )
        self.compact_crc32 = bool(options.get("compact_crc32", False))
        self.compact_style_colors = bool(
            options.get("compact_style_colors", False)
        )
        self.compact_name_switches = set(
            options.get("compact_name_switches", [])
        )
        self.compact_global_context = bool(
            options.get("compact_global_context", False)
        )
        self.compact_truetype_only = bool(
            options.get("compact_truetype_only", False)
        )
        self.compact_imvector = bool(options.get("compact_imvector", False))
        self.compact_imvector_accessors = bool(
            options.get("compact_imvector_accessors", False)
        )
        self.compact_imvector_lifecycle = bool(
            options.get("compact_imvector_lifecycle", False)
        )
        self.compact_imvector_capacity = bool(
            options.get("compact_imvector_capacity", False)
        )
        self.compact_imchunkstream = bool(
            options.get("compact_imchunkstream", False)
        )
        self.compact_impool = bool(options.get("compact_impool", False))
        self.flatten_table_pool = bool(
            options.get("flatten_table_pool", False)
        )
        self.flatten_table_spans = bool(
            options.get("flatten_table_spans", False)
        )
        self.flatten_table_vectors = bool(
            options.get("flatten_table_vectors", False)
        )
        self.compact_checkbox_flags = bool(
            options.get("compact_checkbox_flags", False)
        )
        self.omit_unused_scalar_helpers = bool(
            options.get("omit_unused_scalar_helpers", False)
        )
        self.omit_unused_compatibility_shims = bool(
            options.get("omit_unused_compatibility_shims", False)
        )
        self.compact_input_source_names = bool(
            options.get("compact_input_source_names", False)
        )
        self.compact_assert_metadata = bool(
            options.get("compact_assert_metadata", False)
        )
        if self.compact_imchunkstream and not self.compact_imvector:
            raise TranslationError(
                "compact ImChunkStream requires compact ImVector"
            )
        if self.compact_imvector_accessors and not self.compact_imvector:
            raise TranslationError(
                "compact ImVector accessors require compact ImVector"
            )
        if self.compact_imvector_lifecycle and not self.compact_imvector:
            raise TranslationError(
                "compact ImVector lifecycle requires compact ImVector"
            )
        if self.compact_imvector_capacity and not self.compact_imvector:
            raise TranslationError(
                "compact ImVector capacity requires compact ImVector"
            )
        if self.flatten_table_vectors and not self.compact_imvector:
            raise TranslationError(
                "flattened table vectors require compact ImVector"
            )
        if self.compact_impool and not self.compact_imvector:
            raise TranslationError("compact ImPool requires compact ImVector")
        if self.compact_impool and not self.compact_imvector_lifecycle:
            raise TranslationError(
                "compact ImPool requires compact ImVector lifecycle"
            )
        if self.flatten_table_pool and not self.compact_impool:
            raise TranslationError(
                "flatten table pool requires compact ImPool"
            )
        self.compact_nav_key_ranges = bool(
            options.get("compact_nav_key_ranges", False)
        )
        self.compact_nav_overlay_selectable = bool(
            options.get("compact_nav_overlay_selectable", False)
        )
        self.compact_cff_stack_guards = bool(
            options.get("compact_cff_stack_guards", False)
        )
        self.compact_key_char_mask = bool(
            options.get("compact_key_char_mask", False)
        )
        self.compact_localization_entries = bool(
            options.get("compact_localization_entries", False)
        )
        self.compact_cursor_data = bool(
            options.get("compact_cursor_data", False)
        )
        self.compact_separator_table = bool(
            options.get("compact_separator_table", False)
        )
        self.compact_utf8_tables = bool(
            options.get("compact_utf8_tables", False)
        )
        self.compact_color_format_tables = bool(
            options.get("compact_color_format_tables", False)
        )
        self.compact_glyph_deltas = bool(
            options.get("compact_glyph_deltas", False)
        )
        self.compact_optional_modules = bool(
            options.get("compact_optional_modules", False)
        )
        self.compact_assert_traps = bool(
            options.get("compact_assert_traps", False)
        )
        self.compact_static_const_arrays = bool(
            options.get("compact_static_const_arrays", False)
        )
        if self.compact_truetype_only:
            self.omitted_call_names.update((
                "stbtt__GetGlyphInfoT2",
                "stbtt__GetGlyphShapeT2",
            ))
        self.current_compact_zero_constructor = False
        self.current_compact_nav_key_ranges = False
        self.static_const_local_ids: set[str] = set()
        self.current_constructor_initialized_fields: set[str] = set()
        self.optional_settings_init_lines: list[str] | None = None
        self.optional_cff_init_lines: list[str] | None = None
        self.optional_cff_init_locals: list[str] | None = None
        self.compact_assert_ids: dict[tuple[str, int, str], int] = {}
        self.compact_assert_records: list[tuple[str, int, str]] = []
        if self.compact_optional_modules and self.compact_truetype_only:
            raise TranslationError(
                "optional CFF module cannot be combined with TrueType-only "
                "lowering"
            )
        self.compact_assert_trap_count = 0

        def count_assert_traps(node: Any) -> None:
            if isinstance(node, dict):
                if (node.get("kind") == "CallExpr"
                        and node.get("callee_name") == "__builtin_trap"):
                    self.compact_assert_trap_count += 1
                for value in node.values():
                    count_assert_traps(value)
            elif isinstance(node, list):
                for value in node:
                    count_assert_traps(value)

        if self.compact_assert_traps:
            cached_assert_traps = ir.get("_imgui_c89_assert_trap_count")
            if cached_assert_traps is None:
                count_assert_traps(ir.get("functions", []))
                ir["_imgui_c89_assert_trap_count"] = (
                    self.compact_assert_trap_count
                )
            else:
                self.compact_assert_trap_count = int(cached_assert_traps)
            if self.compact_assert_trap_count == 0:
                raise TranslationError(
                    "compact assertion-trap profile captured no trap checks"
                )
        self.typedefs = [
            item for item in ir.get("typedefs", [])
            if item.get("name") and not item.get("dependent")
            and "<" not in item.get("qualified_name", "")
        ]
        self.scalar_aliases: dict[str, str] = {}
        for item in self.typedefs:
            underlying = item["underlying_type"]
            if (not any(token in underlying for token in ("(", "[", "<"))
                    and not underlying.startswith(("struct ", "union ", "enum "))
                    and item["name"] not in underlying.split()):
                self.scalar_aliases[item["qualified_name"]] = underlying
        self.records = {
            item["id"]: item
            for item in ir.get("records", [])
            if item.get("definition") and not item.get("dependent")
        }
        self.record_names: dict[str, str] = {}
        self.record_names_by_id: dict[str, str] = {}
        self.records_by_spelling: dict[str, dict[str, Any]] = {}
        record_base_names: dict[str, str] = {}
        for item in self.records.values():
            qualified = item["qualified_name"]
            if not item.get("name"):
                location = item.get("location", {})
                source = Path(location.get("file", "record")).stem
                base_name = _c_identifier(
                    f"imgui_c89_anon_{source}_{location.get('line', 0)}_"
                    f"{location.get('column', 0)}"
                )
            else:
                readable = qualified.replace("::", "_")
                readable = readable.replace("*", "_ptr").replace("&", "_ref")
                base_name = re.sub(r"[^A-Za-z0-9_]", "_", readable)
                base_name = re.sub(r"_+", "_", base_name).strip("_")
            if (self.flatten_table_pool
                    and qualified == "ImPool<ImGuiTable>"):
                base_name = "ImGuiTablePool"
            record_base_names[item["id"]] = base_name
        record_base_counts: dict[str, int] = {}
        for base_name in record_base_names.values():
            record_base_counts[base_name] = (
                record_base_counts.get(base_name, 0) + 1
            )
        claimed_record_names: dict[str, str] = {}
        for item in self.records.values():
            qualified = item["qualified_name"]
            c_name = record_base_names[item["id"]]
            if record_base_counts[c_name] > 1:
                location = item.get("location", {})
                source = Path(location.get("file", "record")).stem
                c_name = _c_identifier(
                    f"{c_name}_{source}_{location.get('line', 0)}_"
                    f"{location.get('column', 0)}"
                )
            previous = claimed_record_names.get(c_name)
            if previous is not None and previous != item["id"]:
                raise TranslationError(
                    f"readable record name collision: {c_name}"
                )
            claimed_record_names[c_name] = item["id"]
            self.record_names_by_id[item["id"]] = c_name
            self.record_names[item["qualified_name"]] = c_name
            self.records_by_spelling[item["qualified_name"]] = item
            if not item.get("name"):
                location = item.get("location", {})
                source = location.get("file")
                line = location.get("line")
                column = location.get("column")
                if source and line and column:
                    for alias in (
                        f"struct (unnamed at {source}:{line}:{column})",
                        f"(unnamed struct at {source}:{line}:{column})",
                        f"union (unnamed at {source}:{line}:{column})",
                        f"(unnamed union at {source}:{line}:{column})",
                    ):
                        self.record_names[alias] = c_name
                        self.records_by_spelling[alias] = item
            if item.get("name") and "<" not in item["qualified_name"]:
                self.record_names[item["name"]] = c_name
                self.records_by_spelling[item["name"]] = item
        self.field_names = {
            field["id"]: (
                field.get("name")
                or "imgui_c89_unnamed_"
                + hashlib.sha256(field["id"].encode()).hexdigest()[:8]
            )
            for record in self.records.values()
            for field in record.get("fields", [])
        }
        self.fields = {
            field["id"]: field
            for record in self.records.values()
            for field in record.get("fields", [])
        }
        self.flattened_table_span_fields: dict[str, tuple[str, str]] = {}
        self.flattened_table_vector_fields: dict[
            str, tuple[str, str, str, str]
        ] = {}
        self.flattened_table_vector_record_ids: set[str] = set()
        self.omitted_record_ids: set[str] = set()
        if self.flatten_table_spans:
            self.configure_flattened_table_spans()
        if self.flatten_table_vectors:
            self.configure_flattened_table_vectors()
        if self.flatten_table_pool:
            self.configure_flattened_table_pool()
        self.records_by_c_name = {
            self.record_names_by_id[record["id"]]: record
            for record in self.records.values()
        }
        if self.compact_imchunkstream:
            chunk_records = [
                record for record in self.records.values()
                if record.get("qualified_name") in {
                    "ImChunkStream<ImGuiTableSettings>",
                    "ImChunkStream<ImGuiWindowSettings>",
                }
            ]
            vector_records = [
                record for record in self.records.values()
                if record.get("qualified_name") == "ImVector<char>"
            ]
            if (len(chunk_records) != 2 or len(vector_records) != 1
                    or any([(field.get("name"), field.get("type"))
                            for field in record.get("fields", [])]
                           != [("Buf", "ImVector<char>")]
                           for record in chunk_records)
                    or [(field.get("name"), field.get("type"))
                        for field in vector_records[0].get("fields", [])]
                    != [("Size", "int"), ("Capacity", "int"),
                        ("Data", "char *")]):
                raise TranslationError(
                    "compact ImChunkStream layout changed shape"
                )
        self.typedef_c_names: dict[str, str] = {}
        for item in self.typedefs:
            c_name = _c_identifier(item["qualified_name"])
            canonical = item.get("canonical_underlying_type", "")
            record_name = self.record_names.get(canonical)
            if record_name:
                c_name = record_name
            self.typedef_c_names[item["qualified_name"]] = c_name
            self.typedef_c_names.setdefault(item["name"], c_name)
        # Named C++ enums are emitted as integer typedefs.  Dear ImGui often
        # keeps an internal enum named Foo_ beside its public storage typedef
        # Foo; _c_identifier intentionally folds the trailing underscore so
        # both spellings use that one representable C type.
        self.enum_c_names = {
            item["qualified_name"]: _c_identifier(item["qualified_name"])
            for item in ir.get("enums", []) if item.get("name")
        }
        for item in ir.get("enums", []):
            if item.get("name"):
                self.enum_c_names.setdefault(
                    item["name"], _c_identifier(item["qualified_name"])
                )
        self.function_declarations = self._merge_function_declarations(
            ir.get("functions", [])
        )
        self.flattened_span_function_ids = {
            identifier
            for identifier, function in self.function_declarations.items()
            if function.get("parent") in self.omitted_record_ids
        }
        self.omitted_call_ids = {
            identifier
            for identifier, function in self.function_declarations.items()
            if function.get("qualified_name") in self.omitted_call_names
        }
        self.trapped_call_ids = {
            identifier
            for identifier, function in self.function_declarations.items()
            if function.get("qualified_name") in self.trapped_call_names
        }
        invalid_traps = [
            self.function_declarations[identifier].get("qualified_name", identifier)
            for identifier in self.trapped_call_ids
            if self.base_spelling(
                self.function_declarations[identifier].get("return_type", "")
            ) in self.records_by_spelling
        ]
        if invalid_traps:
            raise TranslationError(
                "trap_calls cannot synthesize aggregate results: "
                + ", ".join(invalid_traps)
            )
        self.imvector_assert_backend: str | None = None
        self.imvector_assert_file: str | None = None
        self.functions = {
            identifier: item
            for identifier, item in self.function_declarations.items()
            if item.get("definition")
        }
        self.noinline_function_ids = {
            identifier
            for identifier, function in self.functions.items()
            if function.get("qualified_name") in self.noinline_function_names
        }
        missing_noinline = self.noinline_function_names - {
            self.functions[identifier].get("qualified_name", "")
            for identifier in self.noinline_function_ids
        }
        if missing_noinline:
            raise TranslationError(
                "noinline function targets are missing: "
                + ", ".join(sorted(missing_noinline))
            )
        self.compact_checkbox_flag_helpers: dict[int, str] = {}
        if self.compact_checkbox_flags:
            helper_candidates: dict[int, list[str]] = {32: [], 64: []}
            for identifier, function in self.functions.items():
                if function.get("qualified_name") != "ImGui::CheckboxFlagsT":
                    continue
                parameters = function.get("parameters", [])
                if len(parameters) != 3:
                    continue
                spelling = parameters[1].get("canonical_type", "")
                if spelling == "unsigned int *":
                    helper_candidates[32].append(identifier)
                elif spelling == "unsigned long long *":
                    helper_candidates[64].append(identifier)
            if any(len(identifiers) != 1
                   for identifiers in helper_candidates.values()):
                raise TranslationError(
                    "compact CheckboxFlags expected unsigned 32/64 helpers"
                )
            self.compact_checkbox_flag_helpers = {
                width: identifiers[0]
                for width, identifiers in helper_candidates.items()
            }
        self.imvector_accessor_lines: dict[str, int] = {}
        if self.compact_imvector_accessors:
            accessor_messages = {
                "operator[]": "i >= 0 && i < Size",
                "back": "Size > 0",
                "pop_back": "Size > 0",
                "index_from_ptr": "it >= Data && it < Data + Size",
            }
            lines_by_method: dict[str, set[int]] = {
                method: set() for method in accessor_messages
            }
            for function in self.functions.values():
                qualified_name = function.get("qualified_name", "")
                if not qualified_name.startswith("ImVector<"):
                    continue
                method = qualified_name.rsplit("::", 1)[-1]
                if method in accessor_messages:
                    lines_by_method[method].add(self.imvector_assert_line(
                        function, method, accessor_messages[method]
                    ))
            if (any(len(lines) != 2 for method, lines in lines_by_method.items()
                    if method in {"operator[]", "back"})
                    or any(len(lines) != 1
                           for method, lines in lines_by_method.items()
                           if method in {"pop_back", "index_from_ptr"})):
                raise TranslationError(
                    "compact ImVector accessor assertion locations diverged"
                )
            self.imvector_accessor_lines = {
                method: next(iter(lines))
                for method, lines in lines_by_method.items()
                if len(lines) == 1
            }
        self.static_locals: dict[str, tuple[str, dict[str, Any]]] = {}
        for function_id, function in self.functions.items():
            for declaration in self.collect_local_declarations(
                function.get("body", {})
            ):
                if declaration.get("static_local"):
                    self.static_locals.setdefault(
                        declaration["id"], (function_id, declaration)
                    )
        self.static_local_names = {
            identifier: _symbol(
                identifier, declaration.get("name", "static_local")
            )
            for identifier, (_, declaration) in self.static_locals.items()
        }
        self.compact_separator_local_ids = {
            identifier
            for identifier, (owner, declaration) in self.static_locals.items()
            if self.compact_separator_table
            and declaration.get("name") == "separator_list"
            and self.functions[owner].get("qualified_name")
            == "ImStb::ImCharIsSeparatorW"
        }
        self.compact_utf8_local_ids = {
            identifier
            for identifier, (owner, declaration) in self.static_locals.items()
            if self.compact_utf8_tables
            and declaration.get("name") in {"masks", "shiftc", "shifte"}
            and self.functions[owner].get("qualified_name")
            == "ImTextCharFromUtf8"
        }
        self.compact_color_table_local_ids = {
            identifier
            for identifier, (owner, declaration) in self.static_locals.items()
            if self.compact_color_format_tables
            and declaration.get("name") in {
                "ids", "fmt_table_int", "fmt_table_float"
            }
            and self.functions[owner].get("qualified_name")
            == "ImGui::ColorEdit4"
        }
        self.compact_glyph_delta_local_ids = {
            identifier
            for identifier, (owner, declaration) in self.static_locals.items()
            if self.compact_glyph_deltas
            and declaration.get("name")
            == "accumulative_offsets_from_0x4E00"
            and self.functions[owner].get("qualified_name") in {
                "ImFontAtlas::GetGlyphRangesChineseSimplifiedCommon",
                "ImFontAtlas::GetGlyphRangesJapanese",
            }
        }
        if (self.compact_glyph_deltas
                and len(self.compact_glyph_delta_local_ids) not in {0, 2}):
            raise TranslationError(
                "compact glyph deltas expected both Chinese and Japanese "
                "tables or neither"
            )
        self.public_enum_ids = {
            enum["id"] for enum in ir.get("enums", [])
            if self.enum_is_public(enum)
        }
        self.enum_constant_names = self.stable_enum_constant_names()
        self.enum_constant_values = {
            constant["id"]: int(constant["value"])
            for enum in ir.get("enums", [])
            for constant in enum.get("constants", [])
        }
        self.globals = {
            item["id"]: item for item in ir.get("globals", [])
            if item.get("definition")
        }
        self.packed_char_global_ids = {
            identifier for identifier, item in self.globals.items()
            if item.get("qualified_name") in self.packed_char_global_names
        }
        self.packed_string_pointer_global_ids = {
            identifier for identifier, item in self.globals.items()
            if item.get("qualified_name")
            in self.packed_string_pointer_global_names
        }
        self.compressed_string_pointer_global_ids = {
            identifier for identifier, item in self.globals.items()
            if item.get("qualified_name")
            in self.compressed_string_pointer_global_names
        }
        self.compact_crc32_global_ids = {
            identifier for identifier, item in self.globals.items()
            if self.compact_crc32
            and item.get("qualified_name") == "GCrc32LookupTable"
        }
        self.compact_localization_global_ids = {
            identifier for identifier, item in self.globals.items()
            if self.compact_localization_entries
            and item.get("qualified_name") == "GLocalizationEntriesEnUS"
        }
        self.compact_cursor_global_ids = {
            identifier for identifier, item in self.globals.items()
            if self.compact_cursor_data
            and item.get("qualified_name")
            == "FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA"
        }
        self.global_names = {
            identifier: (
                item["name"]
                if item.get("external_linkage")
                and "::" not in item["qualified_name"]
                else _symbol(identifier, item["qualified_name"])
            )
            for identifier, item in self.globals.items()
        }
        self.compat_context_global_id = next((
            identifier for identifier, item in self.globals.items()
            if item.get("qualified_name") == "GImGui"
        ), None)
        self.function_names = {
            identifier: _symbol(identifier, item["qualified_name"])
            for identifier, item in self.function_declarations.items()
        }
        # Public, C-representable imgui.h entry points are the canonical C ABI,
        # not a second adapter layered over the translated implementation. Give
        # their actual definitions stable readable names so both native C and
        # the exact C++ facade call one symbol and one semantic path. Private
        # and template machinery keeps identity-derived names until it receives
        # an intentionally designed internal C surface.
        self.public_exact_c_names = {
            identifier: api_name
            for api_name, identifier, _ in self.exact_c_api_functions()
        }
        self.exact_c_names = dict(self.public_exact_c_names)
        self.exact_c_names.update({
            identifier: api_name
            for api_name, identifier, _ in self.internal_exact_c_api_functions(
                set(self.exact_c_names)
            )
        })
        exact_name_counts: dict[str, int] = {}
        for exact_name in self.exact_c_names.values():
            exact_name_counts[exact_name] = exact_name_counts.get(exact_name, 0) + 1
        duplicate_exact_names = {
            name for name, count in exact_name_counts.items() if count > 1
        }
        if duplicate_exact_names:
            raise TranslationError(
                "duplicate stable exact-C names: "
                + ", ".join(sorted(duplicate_exact_names))
            )
        self.function_names.update(self.exact_c_names)
        self.compact_internal_c_names = (
            self.stable_table_pool_function_names()
            if self.flatten_table_pool else {
                identifier: self.internal_namespace + "table_pool_add"
                for identifier, function in self.functions.items()
                if self.compact_impool
                and function.get("qualified_name")
                == "ImPool<ImGuiTable>::Add"
            }
        )
        if (not self.flatten_table_pool
                and len(self.compact_internal_c_names) > 1):
            raise TranslationError(
                "compact ImPool expected one ImPool<ImGuiTable>::Add"
            )
        self.function_names.update(self.compact_internal_c_names)
        self.handwritten_c_names = self.stable_handwritten_function_names(
            set(self.exact_c_names) | set(self.compact_internal_c_names)
        )
        stable_name_owners: dict[str, str] = {
            name: identifier for identifier, name in self.exact_c_names.items()
        }
        for identifier, name in self.compact_internal_c_names.items():
            owner = stable_name_owners.get(name)
            if owner is not None and owner != identifier:
                raise TranslationError(
                    f"stable compact C name collision: {name}"
                )
            if name in set(self.global_names.values()):
                raise TranslationError(
                    f"stable compact C name collides with global: {name}"
                )
            stable_name_owners[name] = identifier
        for identifier, name in sorted(self.handwritten_c_names.items()):
            owner = stable_name_owners.get(name)
            if owner is not None and owner != identifier:
                raise TranslationError(
                    f"stable handwritten C name collision: {name}"
                )
            if name in set(self.global_names.values()):
                raise TranslationError(
                    f"stable handwritten C name collides with global: {name}"
                )
            stable_name_owners[name] = identifier
        self.function_names.update(self.handwritten_c_names)
        if len(set(self.function_names.values())) != len(self.function_names):
            raise TranslationError("function C-name assignment is not unique")
        self.handwritten_function_dependencies: dict[str, set[str]] = {}
        self.handwritten_constructor_value_dependencies: dict[
            str, set[str]
        ] = {}
        self.public_complete_record_ids = (
            self.compute_public_complete_record_ids()
        )
        self.public_forward_record_ids = (
            self.compute_public_forward_record_ids()
        )
        self.constructor_helpers = {
            identifier: _symbol(identifier + "#value", item["qualified_name"] + "_value")
            for identifier, item in self.functions.items()
            if item.get("constructor")
        }
        self.constructor_at_helpers = {
            identifier: _symbol(identifier + "#at", item["qualified_name"] + "_at")
            for identifier, item in self.functions.items()
            if item.get("constructor")
        }
        self.default_constructor_by_record = {
            item["parent"]: identifier
            for identifier, item in self.functions.items()
            if item.get("constructor")
            and item.get("parent") in self.records
            and not item.get("parameters")
        }
        self.destructors_by_type: dict[str, str] = {}
        self.destructor_ids_by_type: dict[str, str] = {}
        for identifier, item in self.functions.items():
            if item.get("destructor") and item.get("parent") in self.records:
                record = self.records[item["parent"]]
                self.destructors_by_type[record["qualified_name"]] = self.function_names[identifier]
                self.destructors_by_type[record["name"]] = self.function_names[identifier]
                self.destructor_ids_by_type[record["qualified_name"]] = identifier
                self.destructor_ids_by_type[record["name"]] = identifier
        (
            self.context_threaded_functions,
            self.context_boundary_sources,
            self.context_consumers,
        ) = self.analyze_context_flow()
        self.explicit_context_threaded_function_count = len(
            self.context_threaded_functions
        )
        if self.compact_global_context:
            # The native boundary still accepts an explicit context pointer,
            # but binds it once through upstream's own GImGui slot. Internal
            # signatures then recover the compact shape of the C++ source.
            self.context_threaded_functions = set()
        self.reference_parameters: set[str] = set()
        self.array_reference_ids: set[str] = set()
        # Aggregate helpers also cover records owned by an external C API
        # (SDL, in particular).  Clang still gives us the aggregate's field
        # names even though its declaration is deliberately outside the
        # translated project set.
        self.aggregate_helpers: dict[
            tuple[str, tuple[str, ...]],
            tuple[str, str, list[str], list[dict[str, Any]]],
        ] = {}
        self.lambda_helpers: dict[str, tuple[str, dict[str, Any]]] = {}
        self.local_names: dict[str, str] = {}
        self.current_return_type = "void"
        self.current_function_id: str | None = None
        self.expression_temporaries: list[tuple[str, str]] = []
        self.cleanup_scopes: list[list[tuple[str, str]]] = []
        self.break_cleanup_depths: list[int] = []
        self.continue_cleanup_depths: list[int] = []
        self.function_exit_cleanup: list[str] = []
        self.active_function_ids: set[str] | None = None
        self.active_global_ids: set[str] | None = None
        self.emit_runtime_support = True
        self.prime_handwritten_function_dependencies()
        if linkage_analysis is None:
            linkage_analysis = self.analyze_function_linkage()
        (
            self.function_owners,
            self.function_references,
            self.internal_functions,
        ) = linkage_analysis
        (
            self.constructor_value_consumers,
            self.constructor_at_consumers,
        ) = self.analyze_constructor_helper_usage()
        self.internal_constructor_value_helpers = {
            identifier
            for identifier, consumers in self.constructor_value_consumers.items()
            if consumers
            and self.function_owners.get(identifier) is not None
            and all(
                self.function_owners.get(consumer)
                == self.function_owners[identifier]
                for consumer in consumers
            )
        }
        self.internal_constructor_at_helpers = {
            identifier
            for identifier, consumers in self.constructor_at_consumers.items()
            if consumers
            and self.function_owners.get(identifier) is not None
            and all(
                self.function_owners.get(consumer)
                == self.function_owners[identifier]
                for consumer in consumers
            )
        }
        self.inline_constructor_value_helpers = {
            identifier
            for identifier in self.constructor_helpers
            if self.is_inline_value_constructor(self.functions[identifier])
        }
        self.nav_overlay_function_id: str | None = None
        self.nav_overlay_selectable_call_id: str | None = None
        self.nav_overlay_helper_function_ids: dict[str, str] = {}
        if self.compact_nav_overlay_selectable:
            self.configure_nav_overlay_selectable()

    def configure_nav_overlay_selectable(self) -> None:
        """Validate the one non-interactive Selectable used by the nav overlay.

        The Ctrl+Tab list is created with ``NoInputs`` but upstream renders its
        rows through the complete public Selectable interaction machinery.
        This compact target retains the exact layout and rendering path while
        avoiding that unreachable interaction dependency.  Be deliberately
        strict here: an upstream call-shape change must fail translation rather
        than silently applying the specialization to a newly interactive row.
        """
        overlays = [
            (identifier, function)
            for identifier, function in self.functions.items()
            if function.get("qualified_name")
            == "ImGui::NavUpdateWindowingOverlay"
        ]
        if len(overlays) != 1:
            raise TranslationError(
                "compact nav overlay requires one NavUpdateWindowingOverlay"
            )
        overlay_id, overlay = overlays[0]
        calls: list[dict[str, Any]] = []
        referenced_constants: set[str] = set()

        def visit(node: Any) -> None:
            if isinstance(node, list):
                for value in node:
                    visit(value)
                return
            if not isinstance(node, dict):
                return
            if node.get("kind") == "CallExpr":
                callee = self.function_declarations.get(
                    node.get("callee", ""), {}
                )
                if callee.get("qualified_name") == "ImGui::Selectable":
                    calls.append(node)
            if node.get("kind") == "DeclRefExpr":
                constant_id = node.get("decl", "")
                if constant_id in self.enum_constant_names:
                    referenced_constants.add(constant_id)
            for value in node.values():
                visit(value)

        visit(overlay.get("body", {}))
        if len(calls) != 1:
            self.fail(
                overlay,
                "compact nav overlay expected exactly one Selectable call",
            )
        call = calls[0]
        callee = self.function_declarations[call["callee"]]
        parameters = [
            parameter.get("type") for parameter in callee.get("parameters", [])
        ]
        arguments = call.get("arguments", [])
        flags_argument = arguments[2] if len(arguments) > 2 else {}
        while flags_argument.get("kind") in {
            "CXXDefaultArgExpr", "ImplicitCastExpr", "ParenExpr",
        }:
            flags_argument = flags_argument.get("operand", {})
        size_argument = arguments[3] if len(arguments) > 3 else {}
        while size_argument.get("kind") in {
            "CXXDefaultArgExpr", "MaterializeTemporaryExpr",
            "ImplicitCastExpr", "ParenExpr",
        }:
            size_argument = size_argument.get("operand", {})
        size_values = size_argument.get("arguments", [])
        unwrapped_size_values = []
        for value in size_values:
            while value.get("kind") in {
                "ImplicitCastExpr", "ParenExpr",
            }:
                value = value.get("operand", {})
            unwrapped_size_values.append(value)
        if (
            parameters != [
                "const char *", "bool", "ImGuiSelectableFlags",
                "const ImVec2 &",
            ]
            or len(arguments) != 4
            or flags_argument.get("kind") != "IntegerLiteral"
            or flags_argument.get("value") != "0"
            or size_argument.get("kind") not in {
                "CXXTemporaryObjectExpr", "CXXConstructExpr"
            }
            or len(unwrapped_size_values) != 2
            or any(
                value.get("kind") not in {
                    "IntegerLiteral", "FloatingLiteral"
                }
                or float(value.get("value", "nan")) != 0.0
                for value in unwrapped_size_values
            )
        ):
            self.fail(call, "compact nav overlay Selectable shape changed")
        no_inputs_ids = {
            constant["id"]
            for enum in self.ir.get("enums", [])
            for constant in enum.get("constants", [])
            if constant.get("name") == "ImGuiWindowFlags_NoInputs"
        }
        if not (referenced_constants & no_inputs_ids):
            self.fail(
                overlay,
                "compact nav overlay requires an explicit NoInputs window",
            )
        if overlay_id not in self.context_threaded_functions:
            self.fail(
                overlay,
                "compact nav overlay requires explicit context threading",
            )

        requirements = {
            "get_id": ("ImGuiWindow::GetID", ["const char *", "const char *"]),
            "find_text_end": (
                "ImGui::FindRenderedTextEnd", ["const char *", "const char *"]
            ),
            "calc_text_size": (
                "ImGui::CalcTextSize",
                ["const char *", "const char *", "bool", "float"],
            ),
            "item_size": ("ImGui::ItemSize", ["const ImVec2 &", "float"]),
            "item_add": (
                "ImGui::ItemAdd",
                ["const ImRect &", "ImGuiID", "const ImRect *", "ImGuiItemFlags"],
            ),
            "render_frame": (
                "ImGui::RenderFrame",
                ["ImVec2", "ImVec2", "ImU32", "bool", "float"],
            ),
            "get_color": ("ImGui::GetColorU32", ["ImGuiCol", "float"]),
            "render_text": (
                "ImGui::RenderTextClipped",
                [
                    "const ImVec2 &", "const ImVec2 &", "const char *",
                    "const char *", "const ImVec2 *", "const ImVec2 &",
                    "const ImRect *",
                ],
            ),
        }
        resolved: dict[str, str] = {}
        for key, (qualified_name, parameter_types) in requirements.items():
            matches = [
                identifier
                for identifier, function in self.function_declarations.items()
                if function.get("qualified_name") == qualified_name
                and [
                    parameter.get("type")
                    for parameter in function.get("parameters", [])
                ] == parameter_types
            ]
            if len(matches) != 1:
                raise TranslationError(
                    f"compact nav overlay cannot uniquely resolve {qualified_name}"
                )
            resolved[key] = matches[0]
        self.nav_overlay_function_id = overlay_id
        self.nav_overlay_selectable_call_id = call["callee"]
        self.nav_overlay_helper_function_ids = resolved

    def is_inline_value_constructor(self, function: dict[str, Any]) -> bool:
        """Return whether a constructor can be cloned as a tiny local helper.

        C++ keeps header-defined field-only constructors inline.  Calling one
        canonical C definition from every generated unit loses that property;
        on common ABIs ImVec2/ImVec4 construction becomes a call to a function
        whose entire body is ``ret``.  Preserve the source-level intent for
        constructors that only initialize direct fields with simple scalar
        expressions.  The raw constructor still exists for the exact facade.
        """
        source = function.get("location", {}).get("file", "")
        parent = self.records.get(function.get("parent", ""))
        body = function.get("body", {})
        if (
            not source.endswith((".h", ".hpp"))
            or not parent
            or parent.get("bases")
            or parent.get("union")
            or body.get("kind") != "CompoundStmt"
            or body.get("statements")
            or function.get("id") in self.context_threaded_functions
        ):
            return False
        field_ids = {
            field["id"] for field in parent.get("fields", [])
        }

        def simple(node: Any) -> bool:
            if not isinstance(node, dict):
                return False
            kind = node.get("kind")
            if kind in {
                "FloatingLiteral", "IntegerLiteral", "CXXBoolLiteralExpr",
                "CharacterLiteral", "CXXNullPtrLiteralExpr", "GNUNullExpr",
                "DeclRefExpr",
            }:
                return True
            if kind in {
                "ImplicitCastExpr", "ParenExpr", "ConstantExpr",
                "CStyleCastExpr", "CXXStaticCastExpr",
                "CXXFunctionalCastExpr", "CXXConstCastExpr",
                "CXXReinterpretCastExpr", "UnaryOperator",
            }:
                return "operand" in node and simple(node["operand"])
            if kind in {"BinaryOperator", "CompoundAssignOperator"}:
                return simple(node.get("lhs")) and simple(node.get("rhs"))
            if kind == "ConditionalOperator":
                return all(simple(node.get(key)) for key in (
                    "condition", "true", "false"
                ))
            return False

        initializers = function.get("initializers", [])
        return bool(initializers) and all(
            initializer.get("target") in field_ids
            and simple(initializer.get("value"))
            for initializer in initializers
        )

    def analyze_constructor_helper_usage(
        self,
    ) -> tuple[dict[str, set[str]], dict[str, set[str]]]:
        """Find the value/placement adapters that lowering actually calls.

        Raw constructors are part of the exact facade, but their generated
        by-value and placement adapters are C-lowering details.  Emitting two
        external adapters for every constructor retained a large amount of
        otherwise dead code.  Track the expression forms that require each
        adapter and give it external linkage only when its consumers cross a
        generated translation-unit boundary.
        """
        value_consumers = {
            identifier: set() for identifier in self.constructor_helpers
        }
        at_consumers = {
            identifier: set() for identifier in self.constructor_at_helpers
        }

        def visit(node: Any, consumer: str, direct: bool = False) -> None:
            if isinstance(node, list):
                for value in node:
                    visit(value, consumer)
                return
            if not isinstance(node, dict):
                return
            kind = node.get("kind")
            if kind == "DeclStmt":
                for declaration in node.get("declarations", []):
                    initializer = declaration.get("initializer")
                    if initializer:
                        visit(
                            initializer, consumer,
                            initializer.get("kind") == "CXXConstructExpr",
                        )
                return
            if kind == "CXXNewExpr":
                visit(node.get("placement_arguments", []), consumer)
                initializer = node.get("initializer", {})
                if initializer.get("kind") == "CXXConstructExpr":
                    identifier = initializer.get("constructor")
                    if identifier in at_consumers:
                        at_consumers[identifier].add(consumer)
                    visit(initializer.get("arguments", []), consumer)
                else:
                    visit(initializer, consumer)
                return
            if kind in {"CXXConstructExpr", "CXXTemporaryObjectExpr"}:
                identifier = node.get("constructor")
                if not direct and identifier in value_consumers:
                    value_consumers[identifier].add(consumer)
                visit(node.get("arguments", []), consumer)
                return
            for value in node.values():
                visit(value, consumer)

        for identifier, function in self.functions.items():
            if self.is_flattened_table_pool_function(function):
                # Every flattened pool definition replaces its C++ body, so
                # constructor adapters in the discarded template AST are not
                # consumers.
                continue
            if self.compact_table_pool_add_constructor(function) is not None:
                # The compact C body initializes the slot directly.  The
                # placement-new expression belongs to the discarded C++ body,
                # so it must not keep a placement adapter alive.
                continue
            if identifier in self.handwritten_function_dependencies:
                for constructor in self.handwritten_constructor_value_dependencies.get(
                    identifier, set()
                ):
                    value_consumers[constructor].add(identifier)
                continue
            for initializer in function.get("initializers", []):
                value = initializer.get("value", {})
                visit(
                    value, identifier,
                    value.get("kind") == "CXXConstructExpr",
                )
            visit(function.get("body", {}), identifier)
        return value_consumers, at_consumers

    def analyze_function_linkage(
        self,
    ) -> tuple[dict[str, int], dict[str, set[str]], set[str]]:
        """Find functions that can safely retain translation-unit linkage.

        The merged IR contains one definition for each C++ function, while
        ``translation_units`` records which generated C file owns it.  A
        function only needs external C linkage when a generated facade calls
        it or a different generated translation unit references it.  Keeping
        every other function static lets an ordinary ``-Os`` compile discard
        unused header/template instantiations and optimize local calls without
        relying on whole-program LTO.
        """
        owners: dict[str, int] = {}
        claimed: set[str] = set()
        for index, unit in enumerate(self.ir.get("translation_units", [])):
            identifiers = set(unit.get("function_definitions", [])) - claimed
            claimed.update(identifiers)
            for identifier in identifiers:
                if identifier in self.functions:
                    owners[identifier] = index

        references: dict[str, set[str]] = {
            identifier: set() for identifier in self.functions
        }

        def implicit_constructor_dependencies(
            record: dict[str, Any], seen: set[str] | None = None,
        ) -> set[str]:
            """Mirror calls synthesized by implicit_default_construction."""
            seen = set() if seen is None else seen
            if record["id"] in seen:
                return set()
            seen.add(record["id"])
            result: set[str] = set()
            for base in record.get("bases", []):
                base_record = self.records.get(base.get("record", ""))
                if not base_record:
                    continue
                constructor = self.default_constructor_by_record.get(
                    base_record["id"]
                )
                if constructor:
                    result.add(constructor)
                else:
                    result.update(
                        implicit_constructor_dependencies(base_record, seen)
                    )
            for field in record.get("fields", []):
                if field.get("id") in self.flattened_table_vector_fields:
                    continue
                constructor = self.default_constructor_by_record.get(
                    field.get("record_dependency", "")
                )
                if constructor:
                    result.add(constructor)
            return result

        def visit(node: Any, result: set[str]) -> None:
            if isinstance(node, dict):
                if (
                    node.get("kind") == "CXXConstructExpr"
                    and node.get("constructor") not in self.functions
                ):
                    spelling = self.base_spelling(node.get("type", ""))
                    record = self.records_by_spelling.get(spelling)
                    if not record:
                        record = self.records_by_c_name.get(
                            self.c_type(spelling)
                        )
                    if record:
                        result.update(implicit_constructor_dependencies(record))
                if node.get("kind") == "DeclRefExpr":
                    declaration = node.get("decl")
                    if declaration in self.functions:
                        result.add(declaration)
                for key, value in node.items():
                    if (
                        key in {"callee", "constructor", "destructor"}
                        and isinstance(value, str)
                        and value in self.functions
                        and value not in self.flattened_span_function_ids
                    ):
                        result.add(value)
                    visit(value, result)
            elif isinstance(node, list):
                for value in node:
                    visit(value, result)

        for identifier, function in self.functions.items():
            table_pool_dependencies = (
                self.flattened_table_pool_dependencies(function)
            )
            if table_pool_dependencies is not None:
                references[identifier].update(table_pool_dependencies)
            elif self.compact_table_pool_add_constructor(function) is not None:
                # The specialized body calls only the shared pool runtime and
                # performs its own POD initialization.
                references[identifier] = set()
            elif identifier in self.handwritten_function_dependencies:
                references[identifier].update(
                    self.handwritten_function_dependencies[identifier]
                )
            else:
                visit(function.get("body", {}), references[identifier])
                visit(function.get("initializers", []), references[identifier])
                for declaration in self.collect_local_declarations(
                    function.get("body", {})
                ):
                    if not declaration.get("static_local"):
                        references[identifier].update(
                            self.object_destructor_dependencies(
                                declaration.get("type", "")
                            )
                        )
            if (function.get("destructor")
                    and function.get("parent") in self.records
                    and not self.is_flattened_table_pool_function(function)):
                references[identifier].update(
                    self.record_subobject_destructor_dependencies(
                        self.records[function["parent"]]
                    )
                )

        external_roots = {
            identifier for identifier, _ in self.exact_wrapper_functions()
        }
        external_roots.update(
            identifier for _, identifier, _ in self.native_api_functions()
        )
        # A function stored in static data can be observed through that data.
        # Keep it externally linked until global ownership is modeled with the
        # same precision as function ownership.
        for item in self.globals.values():
            visit(item.get("initializer", {}), external_roots)

        consumers: dict[str, set[str]] = {
            identifier: set() for identifier in self.functions
        }
        for source, targets in references.items():
            for target in targets:
                # Declaration-only external bridges have no translated
                # linkage vertex and therefore no ownership classification.
                if target in consumers:
                    consumers[target].add(source)

        internal: set[str] = set()
        for identifier in self.functions:
            owner = owners.get(identifier)
            if owner is None or identifier in external_roots:
                continue
            if all(owners.get(source) == owner for source in consumers[identifier]):
                internal.add(identifier)
        return owners, references, internal

    def analyze_context_flow(
        self,
    ) -> tuple[set[str], dict[str, str], set[str]]:
        """Materialize Dear ImGui's implicit GImGui argument.

        Ordinary functions receive a generated first ``ctx`` parameter.
        Address-taken functions cannot change signature; for the callback
        shapes used by Dear ImGui we derive their context from an existing
        parameter.  The four compatibility lifecycle functions intentionally
        remain rooted in GImGui for the exact C++ facade.
        """
        if self.compat_context_global_id is None:
            return set(), {}, set()

        lambda_functions: dict[str, dict[str, Any]] = {}

        def collect_lambdas(node: Any) -> None:
            if isinstance(node, dict):
                if node.get("kind") == "LambdaExpr":
                    function = node.get("call_operator", {})
                    identifier = function.get("id")
                    if identifier:
                        lambda_functions[identifier] = function
                    collect_lambdas(function.get("body", {}))
                    return
                for value in node.values():
                    collect_lambdas(value)
            elif isinstance(node, list):
                for value in node:
                    collect_lambdas(value)

        for function in self.functions.values():
            collect_lambdas(function.get("body", {}))
            collect_lambdas(function.get("initializers", []))
        all_functions = dict(self.functions)
        all_functions.update(lambda_functions)

        calls: dict[str, set[str]] = {}
        direct: set[str] = set()
        address_taken: set[str] = set()

        def visit(node: Any, callees: set[str], references: set[str]) -> bool:
            uses_context = False
            if isinstance(node, dict):
                # A capture-free lambda is emitted as its own fixed-signature
                # C callback. Analyze its embedded call operator separately;
                # its context dependencies do not belong to the enclosing
                # function merely because Clang nests its AST there.
                if node.get("kind") == "LambdaExpr":
                    return False
                if node.get("kind") == "DeclRefExpr":
                    declaration = node.get("decl")
                    if declaration == self.compat_context_global_id:
                        uses_context = True
                    if declaration in self.functions:
                        references.add(declaration)
                for key, value in node.items():
                    if (
                        key in {"callee", "constructor", "destructor"}
                        and isinstance(value, str)
                        and value in self.functions
                    ):
                        callees.add(value)
                    if visit(value, callees, references):
                        uses_context = True
            elif isinstance(node, list):
                for value in node:
                    if visit(value, callees, references):
                        uses_context = True
            return uses_context

        for identifier, function in all_functions.items():
            callees: set[str] = set()
            references: set[str] = set()
            uses_context = visit(function.get("body", {}), callees, references)
            if visit(function.get("initializers", []), callees, references):
                uses_context = True
            for declaration in self.collect_local_declarations(
                function.get("body", {})
            ):
                if not declaration.get("static_local"):
                    callees.update(self.object_destructor_dependencies(
                        declaration.get("type", "")
                    ))
            if function.get("destructor") and function.get("parent") in self.records:
                callees.update(self.record_subobject_destructor_dependencies(
                    self.records[function["parent"]]
                ))
            calls[identifier] = callees
            address_taken.update(references)
            if uses_context:
                direct.add(identifier)
        for item in self.globals.values():
            ignored_calls: set[str] = set()
            references: set[str] = set()
            visit(item.get("initializer", {}), ignored_calls, references)
            address_taken.update(references)

        lifecycle_names = {
            "ImGui::CreateContext", "ImGui::DestroyContext",
            "ImGui::GetCurrentContext", "ImGui::SetCurrentContext",
        }
        lifecycle = {
            identifier for identifier, function in self.functions.items()
            if function.get("qualified_name") in lifecycle_names
        }
        boundaries = address_taken | lifecycle | set(lambda_functions)
        threaded = direct - boundaries
        changed = True
        while changed:
            changed = False
            for identifier, callees in calls.items():
                if (
                    identifier not in boundaries
                    and identifier not in threaded
                    and callees & threaded
                ):
                    threaded.add(identifier)
                    changed = True
        boundary_consumers = {
            identifier for identifier in boundaries
            if identifier in direct or calls.get(identifier, set()) & threaded
        }

        def parameter_name(function: dict[str, Any], index: int) -> str:
            parameter = function.get("parameters", [])[index]
            return parameter.get("name") or f"arg_{index}"

        def callback_context(function: dict[str, Any]) -> str | None:
            fallback = self.global_names[self.compat_context_global_id]

            def has_field(record_name: str, field_name: str) -> bool:
                record = self.records_by_spelling.get(record_name)
                return bool(record and any(
                    field.get("name") == field_name
                    for field in record.get("fields", [])
                ))

            for index, parameter in enumerate(function.get("parameters", [])):
                spelling = parameter.get("type", "").replace("const ", "").strip()
                name = parameter_name(function, index)
                if spelling == "ImGuiContext *":
                    return name
                if spelling == "ImGuiInputTextCallbackData *":
                    if not has_field("ImGuiInputTextCallbackData", "Ctx"):
                        return fallback
                    return f"({name}->Ctx ? {name}->Ctx : {fallback})"
                if spelling == "ImDrawList *":
                    if not (
                        has_field("ImDrawList", "_Data")
                        and has_field("ImDrawListSharedData", "Context")
                    ):
                        return fallback
                    return (
                        f"({name}->_Data->Context ? "
                        f"{name}->_Data->Context : {fallback})"
                    )
                if spelling == "ImFontAtlas *":
                    if not has_field("ImFontAtlas", "OwnerContext"):
                        return fallback
                    return (
                        f"({name}->OwnerContext ? "
                        f"{name}->OwnerContext : {fallback})"
                    )
                if spelling == "ImGuiViewport *":
                    if not (
                        has_field("ImGuiViewportP", "Window")
                        and has_field("ImGuiWindow", "Ctx")
                    ):
                        return fallback
                    viewport = self.record_names.get(
                        "ImGuiViewportP", "ImGuiViewportP"
                    )
                    owner = f"(({viewport} *){name})->Window"
                    return f"({owner} ? {owner}->Ctx : {fallback})"
            return None

        global_name = self.global_names[self.compat_context_global_id]
        sources: dict[str, str] = {}
        for identifier in boundary_consumers:
            function = all_functions[identifier]
            if identifier in lifecycle:
                sources[identifier] = global_name
                continue
            source = callback_context(function)
            # Older Dear ImGui callbacks (notably the pre-1.87 clipboard
            # hooks) carry neither an owner object nor a context parameter.
            # Their upstream semantics are inherently GImGui-based, so retain
            # that compatibility root only at this irreducible ABI boundary.
            sources[identifier] = source if source is not None else global_name
        return threaded, sources, threaded | boundary_consumers

    def current_context_expression(self) -> str:
        if self.current_function_id in self.context_threaded_functions:
            return "imgui_c89_ctx"
        source = self.context_boundary_sources.get(
            self.current_function_id or ""
        )
        if source is not None:
            return source
        if self.compat_context_global_id is not None:
            return self.global_names[self.compat_context_global_id]
        raise TranslationError("a context argument was requested without GImGui")

    def with_context_argument(
        self, callee_id: str | None, arguments: list[str]
    ) -> list[str]:
        if callee_id in self.context_threaded_functions:
            return [self.current_context_expression(), *arguments]
        return arguments

    def external_callee(self, node: dict[str, Any]) -> str | None:
        """Return the C89 spelling for an intentionally external C helper.

        Dear ImGui uses a small, portable subset of the C library.  Those
        declarations are not part of our translated definition set, so calls
        to them must remain ordinary C calls instead of being treated as
        missing ImGui functions.
        """
        name = node.get("callee_name")
        if name and node.get("callee_project") is False:
            # System headers may expose C names through std:: as well.  The C
            # translation always calls the underlying C89 spelling.
            if name == "ImGuiTestEngine_AssertLog":
                return "imgui_c89_external_ImGuiTestEngine_AssertLog"
            return name
        if name in {
            "fabs", "fabsf", "floor", "floorf", "ceil", "ceilf",
            "fmod", "fmodf", "sqrt", "sqrtf", "pow", "powf",
            "log", "logf", "log10", "log10f", "exp", "expf",
            "sin", "sinf", "cos", "cosf", "tan", "tanf",
            "atan2", "atan2f",
            "memcpy", "memmove", "memset", "memcmp",
            "strlen", "strcmp", "strncmp", "strstr", "strchr",
            "strrchr", "strncpy", "strcpy",
            "malloc", "calloc", "realloc", "free",
            "qsort", "atoi", "atof", "strtol", "vsnprintf", "snprintf",
            "sscanf", "vsscanf",
        }:
            return name
        if name == "__builtin_expect":
            return "imgui_c89_expect"
        if name == "__builtin_va_start":
            return "va_start"
        if name == "__builtin_va_end":
            return "va_end"
        if name == "__builtin_va_copy":
            return "__builtin_va_copy"
        if name == "__builtin_alloca":
            # ISO C89 has no alloca(), but Clang and GCC accept their builtin
            # in strict-C89 mode.  Keeping stack allocation preserves the
            # source lifetime across return/break/continue/goto paths.
            return "__builtin_alloca"
        if name in {"__builtin_debugtrap", "__builtin_trap"}:
            return "imgui_c89_debugtrap"
        if name == "__builtin_unreachable":
            return "imgui_c89_debugtrap"
        if name == "__assert_rtn":
            return "imgui_c89_assert_rtn"
        if name == "ImAtoi":
            return "imgui_c89_atoi_int"
        if name == "ImStrncpy":
            return "imgui_c89_strncpy"
        if name == "ImStricmp":
            return "imgui_c89_stricmp"
        if name == "ImStrnicmp":
            return "imgui_c89_strnicmp"
        if name == "localtime_r":
            return "imgui_c89_localtime_r"
        # Clang gives each implicit template specialization a distinct USR,
        # while the template declaration itself has no emitted C symbol.  The
        # small scalar helpers below are the C89 form of ImGui's ubiquitous
        # arithmetic templates.
        scalar = node.get("type", "").replace("const ", "").strip()
        if name == "ImLerp" and scalar in {"int", "unsigned int", "float", "double"}:
            return {
                "int": "imgui_c89_lerp_int",
                "unsigned int": "imgui_c89_lerp_uint",
                "float": "imgui_c89_lerp_float",
                "double": "imgui_c89_lerp_double",
            }[scalar]
        integer_scalar = scalar in {
            "long", "unsigned long", "size_t", "short", "unsigned short",
            "unsigned char", "signed char",
        }
        if name in {"ImMin", "ImMax"} and (scalar in {
            "int", "unsigned int", "float", "double"
        } or integer_scalar):
            prefix = "min" if name == "ImMin" else "max"
            suffix = {"int": "int", "unsigned int": "uint",
                      "float": "float", "double": "double",
                      "long": "long", "unsigned long": "ulong",
                      "size_t": "size", "short": "long",
                      "unsigned short": "ulong", "unsigned char": "ulong",
                      "signed char": "long"}[scalar]
            return f"imgui_c89_{prefix}_{suffix}"
        if name == "ImClamp" and (scalar in {"int", "unsigned int", "float", "double"} or integer_scalar):
            suffix = {"int": "int", "unsigned int": "uint",
                      "float": "float", "double": "double",
                      "long": "long", "unsigned long": "ulong",
                      "size_t": "size", "short": "long",
                      "unsigned short": "ulong", "unsigned char": "ulong",
                      "signed char": "long"}[scalar]
            return f"imgui_c89_clamp_{suffix}"
        return None

    @staticmethod
    def base_spelling(value: str) -> str:
        result = value.strip()
        while result.startswith(("const ", "volatile ", "struct ", "class ")):
            result = result.split(" ", 1)[1].strip()
        return result

    @staticmethod
    def _merge_function_declarations(
        items: list[dict[str, Any]]
    ) -> dict[str, dict[str, Any]]:
        groups: dict[str, list[dict[str, Any]]] = {}
        for item in items:
            groups.setdefault(item["id"], []).append(item)
        result: dict[str, dict[str, Any]] = {}
        for identifier, declarations in groups.items():
            usable = [
                item for item in declarations if not item.get("dependent")
            ]
            if not usable:
                continue
            definitions = [item for item in usable if item.get("definition")]
            merged = dict((definitions or usable)[-1])
            parameters = [dict(item) for item in merged.get("parameters", [])]
            for declaration in declarations:
                for index, parameter in enumerate(declaration.get("parameters", [])):
                    if index < len(parameters) and "default" in parameter:
                        parameters[index]["default"] = parameter["default"]
            merged["parameters"] = parameters
            merged["declaration_files"] = sorted({
                declaration.get("location", {}).get("file", "")
                for declaration in declarations
            })
            result[identifier] = merged
        return result

    @functools.lru_cache(maxsize=None)
    def c_type(self, spelling: str) -> str:
        value = spelling.strip()
        reference = value.endswith("&")
        if reference:
            value = value[:-2].rstrip() if value.endswith("&&") else value[:-1].rstrip()
        # A profile may name external-header types whose C and C++ spellings
        # differ.  Keep these rare SDK shims declarative rather than baking a
        # platform or backend special case into expression lowering.
        for source, replacement in self.ir.get("c_type_replacements", {}).items():
            prefix = r"(?<!struct )" if replacement == f"struct {source}" else ""
            value = re.sub(
                prefix + rf"(?<![A-Za-z0-9_:]){re.escape(source)}(?![A-Za-z0-9_:])",
                replacement, value,
            )
        if "<" in value:
            # Clang sometimes prints nested record template arguments in
            # their short form even though the specialization declaration
            # uses the qualified spelling.
            for item in self.records.values():
                short = item.get("name")
                qualified = item.get("qualified_name")
                if short and qualified and short != qualified and "<" not in qualified:
                    value = re.sub(
                        rf"(?<![A-Za-z0-9_:]){re.escape(short)}(?![A-Za-z0-9_:])",
                        qualified, value,
                    )
        # Resolve complete record spellings before replacing typedefs nested
        # inside them.  For example ImVector<stbtt_packedchar> is a known
        # specialization; rewriting stbtt_packedchar to its anonymous struct
        # tag first would make that specialization impossible to recognize.
        for cpp_name in sorted(self.record_names, key=len, reverse=True):
            value = re.sub(
                rf"(?<![A-Za-z0-9_:]){re.escape(cpp_name)}(?![A-Za-z0-9_:])",
                self.record_names[cpp_name],
                value,
            )
        for alias in sorted(self.typedef_c_names, key=len, reverse=True):
            value = re.sub(
                rf"(?<![A-Za-z0-9_:]){re.escape(alias)}(?![A-Za-z0-9_:])",
                self.typedef_c_names[alias], value,
            )
        for enum_name in sorted(self.enum_c_names, key=len, reverse=True):
            value = re.sub(
                rf"(?<![A-Za-z0-9_:]){re.escape(enum_name)}(?![A-Za-z0-9_:])",
                self.enum_c_names[enum_name], value,
            )
        if "<" in value:
            for _ in range(len(self.scalar_aliases) + 1):
                previous = value
                for alias in sorted(self.scalar_aliases, key=len, reverse=True):
                    value = re.sub(
                        rf"(?<![A-Za-z0-9_:]){re.escape(alias)}(?![A-Za-z0-9_:])",
                        self.scalar_aliases[alias], value,
                    )
                if value == previous:
                    break
            for item in self.records.values():
                short = item.get("name")
                qualified = item.get("qualified_name")
                if short and qualified and short != qualified and "<" not in qualified:
                    value = re.sub(
                        rf"(?<![A-Za-z0-9_:]){re.escape(short)}(?![A-Za-z0-9_:])",
                        qualified, value,
                    )
        for cpp_name in sorted(self.record_names, key=len, reverse=True):
            value = re.sub(
                rf"(?<![A-Za-z0-9_:]){re.escape(cpp_name)}(?![A-Za-z0-9_:])",
                self.record_names[cpp_name],
                value,
            )
        value = value.replace("bool", "unsigned char")
        value = value.replace("unsigned long long", "imgui_c89_u64")
        value = value.replace("long long", "imgui_c89_i64")
        value = re.sub(r"\buint64_t\b", "imgui_c89_u64", value)
        value = re.sub(r"\bint64_t\b", "imgui_c89_i64", value)
        if reference:
            value += " *"
        return value

    def c_declaration(self, spelling: str, name: str) -> str:
        value = spelling.strip()
        array_reference = re.match(r"^(.*?)\s*\(&\)\s*(\[[^\]]*\].*)$", value)
        if array_reference:
            return f"{self.c_type(array_reference.group(1).strip())} *{name}"
        if "(*)" in value:
            return self.c_type(value).replace("(*)", f"(*{name})", 1)
        array = re.match(r"^(.*?)(\[[^\]]*\].*)$", value)
        if array:
            return f"{self.c_type(array.group(1).strip())} {name}{array.group(2)}"
        return f"{self.c_type(value)} {name}"

    def c_field_declaration(self, field: dict[str, Any], name: str) -> str:
        spelling = field["type"]
        dependency = field.get("record_dependency")
        if dependency in self.record_names_by_id:
            record = self.records[dependency]
            source_name = record.get("qualified_name", "")
            target_name = self.record_names_by_id[dependency]
            if source_name and source_name in spelling:
                spelling = spelling.replace(source_name, target_name, 1)
            elif "<" in spelling:
                suffix = spelling.rsplit(">", 1)[-1]
                spelling = target_name + suffix
        return self.c_declaration(spelling, name)

    def flattened_fields(self, record: dict[str, Any]) -> list[dict[str, Any]]:
        result: list[dict[str, Any]] = []
        for base in record.get("bases", []):
            if base.get("virtual"):
                self.fail(record, "virtual inheritance is outside the Dear ImGui subset")
            base_record = self.records.get(base.get("record"))
            if not base_record:
                self.fail(record, f"unresolved base record {base.get('type')}")
            result.extend(self.flattened_fields(base_record))
        for field in record.get("fields", []):
            flattened = self.flattened_table_span_fields.get(field["id"])
            if flattened is not None:
                pointer_type, end_name = flattened
                begin = dict(field)
                begin["type"] = pointer_type
                begin.pop("record_dependency", None)
                end = dict(begin)
                end["id"] = field["id"] + "#end"
                end["name"] = end_name
                result.extend((begin, end))
                continue
            vector = self.flattened_table_vector_fields.get(field["id"])
            if vector is not None:
                pointer_type, size_name, capacity_name, _ = vector
                size = dict(field)
                size["id"] = field["id"] + "#size"
                size["name"] = size_name
                size["type"] = "int"
                size.pop("record_dependency", None)
                capacity = dict(size)
                capacity["id"] = field["id"] + "#capacity"
                capacity["name"] = capacity_name
                data = dict(field)
                data["type"] = pointer_type
                data.pop("record_dependency", None)
                if field.get("name") == "SortSpecsMulti":
                    padding = dict(size)
                    padding["id"] = field["id"] + "#padding"
                    padding["name"] = "SortSpecsMultiPadding"
                    padding["c_comment"] = (
                        "Preserve upstream ImVector aggregate alignment."
                    )
                    result.append(padding)
                result.extend((size, capacity, data))
                continue
            result.append(field)
        return result

    def ordered_records(self) -> list[dict[str, Any]]:
        remaining = {
            identifier: record
            for identifier, record in self.records.items()
            if identifier not in self.omitted_record_ids
        }
        emitted: set[str] = set()
        result: list[dict[str, Any]] = []
        while remaining:
            selected: str | None = None
            for identifier, record in sorted(
                remaining.items(), key=lambda item: item[1]["qualified_name"]
            ):
                dependencies: set[str] = {
                    base["record"] for base in record.get("bases", [])
                    if base.get("record") in self.records
                    and base.get("record") not in self.omitted_record_ids
                }
                for field in record.get("fields", []):
                    dependency = field.get("record_dependency")
                    if (dependency in self.records
                            and dependency not in self.omitted_record_ids):
                        dependencies.add(dependency)
                dependencies.discard(identifier)
                if dependencies <= emitted:
                    selected = identifier
                    break
            if selected is None:
                selected = min(
                    remaining, key=lambda key: remaining[key]["qualified_name"]
                )
            record = remaining.pop(selected)
            result.append(record)
            emitted.add(selected)
        return result

    def ordered_typedefs(self) -> list[dict[str, Any]]:
        remaining = {item["qualified_name"]: item for item in self.typedefs}
        emitted: set[str] = set()
        result: list[dict[str, Any]] = []
        while remaining:
            selected = None
            for name, item in sorted(
                remaining.items(), key=lambda pair: (
                    pair[1].get("location", {}).get("file", ""),
                    pair[1].get("location", {}).get("line", 0), pair[0]
                )
            ):
                underlying = item["underlying_type"]
                dependencies = {
                    candidate for candidate in remaining
                    if candidate != name and re.search(
                        rf"(?<![A-Za-z0-9_:]){re.escape(candidate)}(?![A-Za-z0-9_:])",
                        underlying,
                    )
                }
                if not dependencies:
                    selected = name
                    break
            if selected is None:
                selected = min(remaining)
            item = remaining.pop(selected)
            result.append(item)
            emitted.add(selected)
        return result

    @staticmethod
    def _float_literal(node: dict[str, Any]) -> str:
        value = node["value"]
        if "." not in value and "e" not in value.lower():
            value += ".0"
        return value + ("f" if node.get("type") == "float" else "")

    def _integer_literal(self, node: dict[str, Any]) -> str:
        """Spell large integers without relying on C99 long-long suffixes.

        On an ILP32 C90 target, even a bare 64-bit decimal constant can trigger
        a pedantic diagnostic before it is assigned to our extension-backed
        64-bit typedef.  Assemble it from four portable 16-bit chunks after
        the first cast instead.
        """
        spelling = node["value"]
        value = int(spelling, 0)
        if -2147483647 <= value <= 2147483647:
            return spelling
        negative = value < 0
        magnitude = -value if negative else value
        chunks = [
            (magnitude >> shift) & 0xFFFF
            for shift in (48, 32, 16, 0)
        ]
        expression = f"((imgui_c89_u64)({chunks[0]}))"
        for chunk in chunks[1:]:
            expression = f"(({expression} << 16) | {chunk})"
        target = self.c_type(node.get("type", "imgui_c89_u64"))
        result = f"(({target})({expression}))"
        return f"(-{result})" if negative else result

    @staticmethod
    def _string_literal(value: str) -> str:
        pieces = ['"']
        for byte in value.encode("utf-8"):
            if byte == 34:
                pieces.append('\\"')
            elif byte == 92:
                pieces.append("\\\\")
            elif byte == 9:
                pieces.append("\\t")
            elif byte == 10:
                pieces.append("\\n")
            elif byte == 13:
                pieces.append("\\r")
            elif 32 <= byte < 127:
                pieces.append(chr(byte))
            else:
                pieces.append(f"\\{byte:03o}")
        pieces.append('"')
        return "".join(pieces)

    @staticmethod
    def _unwrap(node: dict[str, Any]) -> dict[str, Any]:
        while node.get("kind") in {
            "ImplicitCastExpr", "ParenExpr", "CXXConstCastExpr"
        }:
            node = node["operand"]
        return node

    @staticmethod
    def find_lambda(node: Any) -> dict[str, Any] | None:
        if isinstance(node, dict):
            if node.get("kind") == "LambdaExpr":
                return node
            for value in node.values():
                found = Emitter.find_lambda(value)
                if found:
                    return found
        elif isinstance(node, list):
            for value in node:
                found = Emitter.find_lambda(value)
                if found:
                    return found
        return None

    def call_result(self, callee_id: str | None, value: str) -> str:
        function = self.function_declarations.get(callee_id or "")
        if function and function.get("return_type", "").strip().endswith("&"):
            return f"(*({value}))"
        return value

    @staticmethod
    def reference_storage_type(spelling: str) -> str:
        value = re.sub(r"\s*&&?\s*$", "", spelling.strip())
        if "*" in value:
            value = re.sub(r"\s*const\s*$", "", value)
        else:
            value = re.sub(r"^const\s+", "", value)
            value = re.sub(r"const\s*$", "", value)
        return value.rstrip()

    def reference_argument(self, argument: dict[str, Any],
                           parameter: dict[str, Any]) -> str:
        # A materialized prvalue is a distinct C++ object whose lifetime is
        # extended for the reference binding.  Preserve that storage instead
        # of unwrapping down to an lvalue which may have a different type
        # (notably Derived* -> Base* when pushing viewport pointers).
        materialized = argument
        while materialized.get("kind") in {
            "ExprWithCleanups", "ConstantExpr", "CXXDefaultArgExpr",
            "CXXBindTemporaryExpr", "CXXDefaultInitExpr",
        } and "operand" in materialized:
            materialized = materialized["operand"]
        if (materialized.get("kind") == "MaterializeTemporaryExpr"
                and "operand" in materialized):
            storage_type = self.reference_storage_type(parameter["type"])
            name = f"imgui_c89_temporary_{len(self.expression_temporaries)}"
            self.expression_temporaries.append((name, storage_type))
            return (
                f"({name} = {self.expression(materialized['operand'])}, "
                f"&{name})"
            )
        node = self._unwrap(argument)
        while node.get("kind") in {
            "MaterializeTemporaryExpr", "ExprWithCleanups", "ConstantExpr",
            "CXXDefaultArgExpr", "CXXBindTemporaryExpr", "CXXDefaultInitExpr",
        } and "operand" in node:
            node = self._unwrap(node["operand"])
        if (node.get("kind") == "DeclRefExpr"
                and node.get("decl") in self.reference_parameters):
            return self.local_names.get(node.get("decl", ""), node["name"])
        storage_type = self.reference_storage_type(parameter["type"])
        if (node.get("kind") == "StringLiteral" and "*" in storage_type):
            name = f"imgui_c89_temporary_{len(self.expression_temporaries)}"
            self.expression_temporaries.append((name, storage_type))
            return f"({name} = {self.expression(node)}, &{name})"
        if node.get("kind") == "ConditionalOperator":
            return "({} ? {} : {})".format(
                self.expression(node["condition"]),
                self.reference_argument(node["true"], parameter),
                self.reference_argument(node["false"], parameter),
            )
        if (node.get("kind") in {"BinaryOperator", "CompoundAssignOperator"}
                and node.get("opcode", "").endswith("=")):
            return f"({self.expression(node)}, {self.object_pointer(node['lhs'])})"
        if (node.get("kind") == "CXXOperatorCallExpr"
                and node.get("callee_name") == "operator="
                and len(node.get("arguments", [])) == 2):
            return "({}, {})".format(
                self.expression(node),
                self.object_pointer(node["arguments"][0]),
            )
        value = self.expression(node)
        if node.get("value_category") == "lvalue":
            return f"&({value})"
        name = f"imgui_c89_temporary_{len(self.expression_temporaries)}"
        self.expression_temporaries.append((name, storage_type))
        return f"({name} = {value}, &{name})"

    def call_arguments(self, callee_id: str | None,
                       arguments: list[dict[str, Any]]) -> list[str]:
        function = self.function_declarations.get(callee_id or "")
        parameters = function.get("parameters", []) if function else []
        result = []
        for index, argument in enumerate(arguments):
            if (index < len(parameters)
                    and parameters[index]["type"].strip().endswith("&")):
                result.append(self.reference_argument(argument, parameters[index]))
            else:
                result.append(self.expression(argument))
        return result

    def method_object_argument(self, callee_id: str | None,
                               node: dict[str, Any]) -> str:
        value = self.object_pointer(node)
        function = self.function_declarations.get(callee_id or "")
        if function and function.get("parent") in self.records:
            record = self.records[function["parent"]]
            target = self.record_names_by_id[record["id"]]
            return f"(({target} *)({value}))"
        return value

    def flattened_table_vector_components(
        self, node: dict[str, Any]
    ) -> tuple[str, str, str, str, str] | None:
        """Return data/size/capacity expressions for a flattened owner field."""
        field = self._unwrap(node)
        if (field.get("kind") != "MemberExpr"
                or field.get("member") not in self.flattened_table_vector_fields):
            return None
        owner = self._unwrap(field["base"])
        reference_owner = (
            owner.get("kind") == "DeclRefExpr"
            and owner.get("decl") in self.reference_parameters
        )
        owner_expression = (
            self.local_names.get(owner.get("decl", ""), owner.get("name", ""))
            if reference_owner else self.expression(owner)
        )
        owner_pointer = (
            field.get("arrow")
            or owner.get("kind") == "CXXThisExpr"
            or reference_owner
        )
        separator = "->" if owner_pointer else "."
        pointer_type, size_name, capacity_name, vector_name = (
            self.flattened_table_vector_fields[field["member"]]
        )
        data_name = self.field_names[field["member"]]
        return (
            f"{owner_expression}{separator}{data_name}",
            f"{owner_expression}{separator}{size_name}",
            f"{owner_expression}{separator}{capacity_name}",
            pointer_type, vector_name,
        )

    def flattened_table_vector_call(
        self, node: dict[str, Any]
    ) -> str | None:
        components = self.flattened_table_vector_components(node["object"])
        if components is None:
            return None
        data, size, capacity, pointer_type, vector_name = components
        function = self.function_declarations.get(node.get("callee", ""), {})
        method = node.get("callee_name", "")
        arguments = node.get("arguments", [])
        context = self.current_context_expression()
        if method == "begin" and not arguments:
            return data
        if method == "end" and not arguments:
            return f"({data} == 0 ? 0 : {data} + {size})"
        if method == "clear" and not arguments:
            return (
                f"imgui_c89_vector_clear({context}, (void **)&{data}, "
                f"&{size}, &{capacity})"
            )
        if method == "clear_destruct" and not arguments:
            if (vector_name != "ImVector<ImGuiTableTempData>"
                    or self.function_body_sha256(function)
                    != "0c7764ae3faaac6c6704f85d4b7ec67b8cd47b3da80dc6dd4a05eebc63af3d91"):
                self.fail(node, "flattened table vector clear_destruct changed")
            return (
                f"imgui_c89_vector_clear({context}, (void **)&{data}, "
                f"&{size}, &{capacity})"
            )
        element_type = self.c_type(pointer_type[:-1].strip())
        if method == "reserve" and len(arguments) == 1:
            new_capacity = self.expression(arguments[0])
            return (
                f"({data} = ({element_type} *)imgui_c89_vector_reserve("
                f"{context}, {data}, {size}, &{capacity}, {new_capacity}, "
                f"sizeof(*{data}), 0))"
            )
        if method == "resize" and len(arguments) in {1, 2}:
            new_size = self.expression(arguments[0])
            if len(arguments) == 1:
                return (
                    f"({data} = ({element_type} *)imgui_c89_vector_resize("
                    f"{context}, {data}, &{size}, &{capacity}, {new_size}, "
                    f"sizeof(*{data})))"
                )
            fill = self.call_arguments(node.get("callee"), arguments)[1]
            return (
                f"({data} = ({element_type} *)imgui_c89_vector_resize_fill("
                f"{context}, {data}, &{size}, &{capacity}, {new_size}, "
                f"sizeof(*{data}), {fill}))"
            )
        if method == "push_back" and len(arguments) == 1:
            value = self.call_arguments(node.get("callee"), arguments)[0]
            return (
                f"({data} = ({element_type} *)imgui_c89_vector_push_back("
                f"{context}, {data}, &{size}, &{capacity}, "
                f"sizeof(*{data}), {value}))"
            )
        if method == "back" and not arguments:
            line = self.imvector_assert_line(
                function, "back", "Size > 0"
            )
            return (
                f"(*(({element_type} *)imgui_c89_vector_back("
                f"{data}, {size}, sizeof(*{data}), {line})))"
            )
        self.fail(node, f"unsupported flattened table vector method {method}")

    def expression(self, node: dict[str, Any]) -> str:
        kind = node.get("kind")
        if kind == "IntegerLiteral":
            return self._integer_literal(node)
        if kind == "FloatingLiteral":
            return self._float_literal(node)
        if kind == "CharacterLiteral":
            return str(node["value"])
        if kind == "CXXBoolLiteralExpr":
            return "1" if node["value"] else "0"
        if kind in {"CXXNullPtrLiteralExpr", "GNUNullExpr"}:
            return "0"
        if kind == "ImplicitValueInitExpr":
            value = self.default_record_value(node)
            if value is not None:
                return value
            return "0"
        if kind == "VAArgExpr":
            return f"va_arg({self.expression(node['operand'])}, {self.c_type(node['type'])})"
        if kind == "OffsetOfExpr":
            designator = ""
            for component in node.get("components", []):
                if component["kind"] == "field":
                    designator += ("." if designator else "") + component["name"]
                elif component["kind"] == "array":
                    designator += f"[{self.expression(component['index'])}]"
                else:
                    self.fail(node, "unsupported offsetof component")
            return f"offsetof({self.c_type(node['record_type'])}, {designator})"
        if kind == "InitListExpr":
            spelling = self.base_spelling(node.get("type", ""))
            record = self.records_by_spelling.get(spelling)
            if not record:
                record = self.records_by_c_name.get(self.c_type(spelling))
            field_names = node.get("field_names", [])
            if not record and not field_names:
                self.fail(node, "aggregate value is not a translated record")
            values = node.get("values", [])
            key = (spelling, tuple(value.get("type", "") for value in values))
            if key not in self.aggregate_helpers:
                digest = hashlib.sha256(repr(key).encode()).hexdigest()[:10]
                result_type = (
                    self.record_names_by_id[record["id"]]
                    if record else self.c_type(spelling)
                )
                fields = (
                    [field["name"] for field in record.get("fields", [])]
                    if record else field_names
                )
                name = f"imgui_c89_aggregate_{_c_identifier(result_type)}__{digest}"
                self.aggregate_helpers[key] = (
                    name, result_type, fields, values
                )
            name = self.aggregate_helpers[key][0]
            return f"{name}({', '.join(self.expression(value) for value in values)})"
        if kind == "StringLiteral":
            value = node["value"]
            if len(value) > 400:
                return " ".join(
                    self._string_literal(value[index:index + 400])
                    for index in range(0, len(value), 400)
                )
            return self._string_literal(value)
        if kind == "PredefinedExpr":
            # C89 has no __func__; assertion diagnostics do not depend on the
            # spelling of this optional function-name argument.
            return '""'
        if kind == "DeclRefExpr":
            if (
                node.get("decl") == self.compat_context_global_id
                and self.current_function_id in self.context_consumers
            ):
                return self.current_context_expression()
            name = self.local_names.get(
                node.get("decl", ""),
                self.static_local_names.get(
                    node.get("decl", ""),
                    self.global_names.get(node.get("decl", ""), node["name"]),
                ),
            )
            name = self.enum_constant_names.get(node.get("decl", ""), name)
            name = self.function_names.get(node.get("decl", ""), name)
            if node.get("decl") in self.reference_parameters:
                if node.get("decl") in self.array_reference_ids:
                    return name
                return f"(*{name})"
            return name
        if kind == "CXXThisExpr":
            return "self"
        if kind == "ImplicitCastExpr" and node.get("cast_kind") in {
            "DerivedToBase", "UncheckedDerivedToBase", "BaseToDerived",
        }:
            return f"(({self.c_type(node['type'])})({self.expression(node['operand'])}))"
        if kind == "ImplicitCastExpr" and node.get("cast_kind") == "IntegralCast":
            target = self.c_type(node["type"])
            source = self.c_type(node["operand"].get("type", ""))
            if target != source and (
                target in {"imgui_c89_i64", "imgui_c89_u64"}
                or source in {"imgui_c89_i64", "imgui_c89_u64"}
            ):
                return f"(({target})({self.expression(node['operand'])}))"
        if kind in {"ImplicitCastExpr", "ParenExpr"}:
            return self.expression(node["operand"])
        if kind in {
            "MaterializeTemporaryExpr",
            "ExprWithCleanups",
            "ConstantExpr",
            "CXXDefaultArgExpr",
            "CXXBindTemporaryExpr",
            "CXXDefaultInitExpr",
            "SubstNonTypeTemplateParmExpr",
            "SubstNonTypeTemplateParmPackExpr",
        }:
            if "operand" in node:
                return self.expression(node["operand"])
            children = node.get("children", [])
            if len(children) == 1:
                return self.expression(children[0])
            self.fail(node, f"expected one child for transparent {kind}")
        if kind == "CXXConstructExpr":
            arguments = node.get("arguments", [])
            constructor_id = node.get("constructor")
            helper = self.constructor_helpers.get(constructor_id)
            if helper:
                values = self.call_arguments(constructor_id, arguments)
                values = self.with_context_argument(constructor_id, values)
                return f"{helper}({', '.join(values)})"
            if len(arguments) == 1:
                return self.expression(arguments[0])
            if not arguments:
                value = self.default_record_value(node)
                if value is not None:
                    return value
            self.fail(node, "non-copy CXX construction requires aggregate lowering")
        if kind == "CXXTemporaryObjectExpr":
            constructor_id = node.get("constructor")
            helper = self.constructor_helpers.get(constructor_id)
            if not helper:
                value = self.default_record_value(node)
                if value is not None:
                    return value
                self.fail(node, "unresolved temporary-object constructor")
            arguments = self.with_context_argument(
                constructor_id,
                self.call_arguments(constructor_id, node.get("arguments", [])),
            )
            return f"{helper}({', '.join(arguments)})"
        if kind == "CXXNewExpr":
            if node.get("array"):
                self.fail(node, "array new is outside the supported Dear ImGui subset")
            placement = node.get("placement_arguments", [])
            if not placement:
                self.fail(node, "ordinary C++ new has no C89 allocation policy")
            storage = self.expression(placement[-1])
            initializer = node.get("initializer")
            allocated = self.c_type(node["allocated_type"])
            if not initializer:
                return f"(({allocated} *)({storage}))"
            if initializer.get("kind") == "CXXConstructExpr":
                constructor_id = initializer.get("constructor")
                helper = self.constructor_at_helpers.get(constructor_id)
                if helper:
                    arguments = [storage]
                    arguments.extend(
                        self.expression(item)
                        for item in initializer.get("arguments", [])
                    )
                    arguments = self.with_context_argument(
                        constructor_id, arguments
                    )
                    return f"{helper}({', '.join(arguments)})"
                if initializer.get("trivial") and not initializer.get("arguments"):
                    return f"(({allocated} *)({storage}))"
                if not initializer.get("arguments"):
                    spelling = self.base_spelling(initializer.get("type", ""))
                    record = self.records_by_spelling.get(spelling)
                    if record:
                        values = [
                            field.get("default", {
                                "kind": "ImplicitValueInitExpr",
                                "type": field["type"],
                                "value_category": "prvalue",
                            })
                            for field in record.get("fields", [])
                        ]
                        aggregate = {
                            "kind": "InitListExpr", "type": spelling,
                            "values": values, "location": node.get("location", {}),
                        }
                        self.expression(aggregate)
                        key = (spelling, tuple(value.get("type", "") for value in values))
                        name = self.aggregate_helpers[key][0] + "_at"
                        arguments = [storage]
                        arguments.extend(self.expression(value) for value in values)
                        return f"{name}({', '.join(arguments)})"
            self.fail(node, "unsupported placement-new initializer")
        if kind in {
            "CStyleCastExpr", "CXXStaticCastExpr", "CXXFunctionalCastExpr",
            "CXXConstCastExpr", "CXXReinterpretCastExpr",
        }:
            operand = node["operand"]
            if (
                self.records_by_spelling.get(self.base_spelling(node["type"]))
                or (
                    kind == "CXXFunctionalCastExpr"
                    and operand.get("kind") == "InitListExpr"
                    and operand.get("field_names")
                )
            ):
                return self.expression(operand)
            return f"(({self.c_type(node['type'])})({self.expression(operand)}))"
        if kind in {"BinaryOperator", "CompoundAssignOperator"}:
            lhs = self.expression(node["lhs"])
            rhs = self.expression(node["rhs"])
            return f"({lhs} {node['opcode']} {rhs})"
        if kind == "UnaryOperator":
            operand = self.expression(node["operand"])
            if node.get("postfix"):
                return f"({operand}{node['opcode']})"
            return f"({node['opcode']}{operand})"
        if kind == "ConditionalOperator":
            return "({} ? {} : {})".format(
                self.expression(node["condition"]),
                self.expression(node["true"]),
                self.expression(node["false"]),
            )
        if kind == "MemberExpr":
            base_node = self._unwrap(node["base"])
            vector = self.flattened_table_vector_components(base_node)
            if vector is not None and node.get("name") in {
                "Data", "Size", "Capacity"
            }:
                return vector[{"Data": 0, "Size": 1, "Capacity": 2}[node["name"]]]
            if (base_node.get("kind") == "MemberExpr"
                    and base_node.get("member") in self.flattened_table_span_fields
                    and node.get("name") in {"Data", "DataEnd"}):
                owner = self._unwrap(base_node["base"])
                reference_owner = (
                    owner.get("kind") == "DeclRefExpr"
                    and owner.get("decl") in self.reference_parameters
                )
                owner_expression = (
                    self.local_names.get(
                        owner.get("decl", ""), owner.get("name", "")
                    )
                    if reference_owner else self.expression(owner)
                )
                owner_pointer = (
                    base_node.get("arrow")
                    or owner.get("kind") == "CXXThisExpr"
                    or reference_owner
                )
                field_name = self.field_names[base_node["member"]]
                if node.get("name") == "DataEnd":
                    field_name = self.flattened_table_span_fields[
                        base_node["member"]
                    ][1]
                return (
                    f"{owner_expression}{'->' if owner_pointer else '.'}"
                    f"{field_name}"
                )
            reference_base = (
                base_node.get("kind") == "DeclRefExpr"
                and base_node.get("decl") in self.reference_parameters
            )
            base = (
                self.local_names.get(base_node.get("decl", ""), base_node.get("name", ""))
                if reference_base else self.expression(base_node)
            )
            pointer = (
                node.get("arrow")
                or base_node.get("kind") == "CXXThisExpr"
                or (
                    reference_base
                )
            )
            field_name = self.field_names.get(node.get("member"), node["name"])
            return f"{base}{'->' if pointer else '.'}{field_name}"
        if kind == "ArraySubscriptExpr":
            base = self._unwrap(node["base"])
            if base.get("decl") in self.compact_color_table_local_ids:
                name = self.static_local_names[base["decl"]]
                index = self.expression(node["index"])
                return f"({name}_data + {name}_offsets[{index}])"
            if base.get("kind") == "ArraySubscriptExpr":
                row_base = self._unwrap(base["base"])
                if row_base.get("decl") in self.compact_color_table_local_ids:
                    declaration = self.static_locals[row_base["decl"]][1]
                    if declaration.get("name") == "ids":
                        self.fail(node, "compact color ID table gained a dimension")
                    name = self.static_local_names[row_base["decl"]]
                    row = self.expression(base["index"])
                    column = self.expression(node["index"])
                    return (
                        f"({name}_data + {name}_offsets["
                        f"({row}) * 4 + ({column})])"
                    )
            if base.get("decl") in self.compressed_string_pointer_global_ids:
                name = self.global_names[base["decl"]]
                index = self.expression(node["index"])
                return (
                    f"imgui_c89_compressed_string_at({name}_data, "
                    f"{name}_rules, {name}_rule_count, {index}, "
                    f"{name}_buffer)"
                )
            if base.get("decl") in self.packed_string_pointer_global_ids:
                name = self.global_names[base["decl"]]
                index = self.expression(node["index"])
                return f"({name}_data + {name}_offsets[{index}])"
            return f"{self.expression(node['base'])}[{self.expression(node['index'])}]"
        if kind == "CXXMemberCallExpr":
            flattened_vector_call = self.flattened_table_vector_call(node)
            if flattened_vector_call is not None:
                return flattened_vector_call
            lambda_node = self.find_lambda(node.get("object"))
            if lambda_node:
                if lambda_node.get("capture_count"):
                    self.fail(lambda_node, "capturing lambda is outside the Dear ImGui subset")
                function = lambda_node["call_operator"]
                identifier = function["id"]
                if identifier not in self.lambda_helpers:
                    name = _symbol(identifier + "#lambda", "imgui_lambda")
                    self.lambda_helpers[identifier] = (name, function)
                return self.lambda_helpers[identifier][0]
            callee = self.function_names.get(node.get("callee"))
            if not callee:
                callee = self.external_callee(node)
            if not callee:
                if str(node.get("callee_name", "")).startswith("~"):
                    # Calls to an implicit trivial destructor have no emitted
                    # definition. Non-trivial project destructors are present
                    # in the function table and take the ordinary path.
                    return "0"
                self.fail(node, f"unresolved member callee {node.get('callee')}")
            object_arg = self.method_object_argument(
                node.get("callee"), node["object"]
            )
            arguments = [object_arg]
            arguments.extend(self.call_arguments(
                node.get("callee"), node.get("arguments", [])
            ))
            arguments = self.with_context_argument(
                node.get("callee"), arguments
            )
            return self.call_result(
                node.get("callee"), f"{callee}({', '.join(arguments)})"
            )
        if kind == "CXXOperatorCallExpr":
            arguments = node.get("arguments", [])
            operator_name = node.get("callee_name", "")
            if operator_name == "operator[]" and len(arguments) == 2:
                vector = self.flattened_table_vector_components(arguments[0])
                if vector is not None:
                    data, size, _, pointer_type, _ = vector
                    element_type = self.c_type(pointer_type[:-1].strip())
                    function = self.function_declarations.get(
                        node.get("callee", ""), {}
                    )
                    line = self.imvector_assert_line(
                        function, "operator[]", "i >= 0 && i < Size"
                    )
                    return (
                        f"(*(({element_type} *)imgui_c89_vector_at("
                        f"{data}, {size}, {self.expression(arguments[1])}, "
                        f"sizeof(*{data}), {line})))"
                    )
            if (node.get("callee") in self.flattened_span_function_ids
                    and operator_name == "operator[]"
                    and len(arguments) == 2):
                return "(*({} + {}))".format(
                    self.expression(arguments[0]),
                    self.expression(arguments[1]),
                )
            infix = operator_name.removeprefix("operator")
            resolved = self.functions.get(node.get("callee", ""))
            if resolved:
                callee = self.function_names[node["callee"]]
                if resolved.get("method") and arguments:
                    values = [self.method_object_argument(
                        node.get("callee"), arguments[0]
                    )]
                    values.extend(
                        self.call_arguments(node.get("callee"), arguments[1:])
                    )
                else:
                    values = self.call_arguments(node.get("callee"), arguments)
                values = self.with_context_argument(node.get("callee"), values)
                return self.call_result(
                    node.get("callee"), f"{callee}({', '.join(values)})"
                )
            # Implicit record copy/move assignment has no source definition,
            # and scalar/friend operators are more faithfully represented by
            # the corresponding C operator than by an artificial helper.
            if infix in {
                "=", "+", "-", "*", "/", "%", "+=", "-=", "*=", "/=",
                "%=", "==", "!=", "<", "<=", ">", ">=", "&", "|", "^",
                "&&", "||", "<<", ">>", "&=", "|=", "^=", "<<=", ">>=",
            } and len(arguments) == 2:
                return f"({self.expression(arguments[0])} {infix} {self.expression(arguments[1])})"
            if infix in {"+", "-", "!", "~", "*", "&", "++", "--"} and len(arguments) == 1:
                return f"({infix}{self.expression(arguments[0])})"
            if infix in {"++", "--"} and len(arguments) == 2:
                return f"({self.expression(arguments[0])}{infix})"
            if infix == "[]" and len(arguments) == 2:
                return f"({self.expression(arguments[0])}[{self.expression(arguments[1])}])"
            # Non-trivial operators (notably ImVector::operator[]) retain the
            # same ordinary function lowering as any other method.
            callee = self.function_names.get(node.get("callee"))
            if not callee:
                callee = self.external_callee(node)
            if not callee:
                self.fail(node, f"unresolved operator callee {node.get('callee')}")
            function = self.functions.get(node.get("callee", ""))
            if function and function.get("method") and arguments:
                values = [self.method_object_argument(
                    node.get("callee"), arguments[0]
                )]
                values.extend(self.call_arguments(node.get("callee"), arguments[1:]))
            else:
                values = self.call_arguments(node.get("callee"), arguments)
            values = self.with_context_argument(node.get("callee"), values)
            return self.call_result(
                node.get("callee"), f"{callee}({', '.join(values)})"
            )
        if kind == "CallExpr":
            if (self.compact_assert_metadata
                    and node.get("callee_name") == "__assert_rtn"):
                metadata = self.assertion_metadata(node)
                identifier = self.compact_assert_ids.get(metadata)
                if identifier is None:
                    self.fail(node, "compact assertion metadata was not indexed")
                return f"imgui_c89_assert_id({identifier})"
            optional_function = self.function_declarations.get(
                node.get("callee", ""), {}
            )
            optional_name = optional_function.get("qualified_name", "")
            if self.compact_optional_modules:
                if optional_name in self.OPTIONAL_NAV_PHASES:
                    if optional_function.get("return_type") != "void":
                        raise TranslationError(
                            f"optional navigation phase {optional_name} "
                            "no longer returns void"
                        )
                    return "imgui_c89_optional_nav({}, {})".format(
                        self.current_context_expression(),
                        self.OPTIONAL_NAV_PHASES[optional_name],
                    )
                if optional_name == "ImGui::UpdateSettings":
                    if optional_function.get("return_type") != "void":
                        raise TranslationError(
                            "optional settings update no longer returns void"
                        )
                    return "imgui_c89_optional_settings_update({})".format(
                        self.current_context_expression()
                    )
                cff_helpers = {
                    "stbtt__GetGlyphInfoT2": "imgui_c89_optional_cff_info",
                    "stbtt__GetGlyphShapeT2": "imgui_c89_optional_cff_shape",
                }
                if optional_name in cff_helpers:
                    if optional_function.get("return_type") != "int":
                        raise TranslationError(
                            f"optional CFF helper {optional_name} no longer "
                            "returns int"
                        )
                    arguments = self.call_arguments(
                        node.get("callee"), node.get("arguments", [])
                    )
                    return f"{cff_helpers[optional_name]}({', '.join(arguments)})"
            if (
                self.compact_nav_overlay_selectable
                and self.current_function_id == self.nav_overlay_function_id
                and node.get("callee") == self.nav_overlay_selectable_call_id
            ):
                arguments_nodes = node.get("arguments", [])
                return "imgui_c89_nav_overlay_selectable({}, {}, {})".format(
                    self.current_context_expression(),
                    self.expression(arguments_nodes[0]),
                    self.expression(arguments_nodes[1]),
                )
            if node.get("callee") in self.trapped_call_ids:
                trapped = self.function_declarations.get(
                    node.get("callee", ""), {}
                )
                if trapped.get("return_type") == "void":
                    return "imgui_c89_debugtrap()"
                return "(imgui_c89_debugtrap(), 0)"
            if node.get("callee") in self.omitted_call_ids:
                omitted = self.function_declarations.get(
                    node.get("callee", ""), {}
                )
                if omitted.get("return_type") == "void":
                    return "((void)0)"
                # Omitted predicates and scalar/pointer results model an
                # unavailable optional path.  A plain integer zero is valid
                # C89 for each of those categories and, unlike `(void)0`, is
                # also valid when the call is nested in a larger expression.
                return "0"
            function = self.function_declarations.get(node.get("callee", ""))
            arguments_nodes = node.get("arguments", [])
            if (
                function
                and function.get("qualified_name")
                == "ImFontAtlasBuildRenderBitmapFromString"
                and len(arguments_nodes) == 7
                and self._unwrap(arguments_nodes[5]).get("decl")
                in self.packed_char_global_ids
            ):
                arguments = self.call_arguments(node.get("callee"), arguments_nodes)
                arguments = self.with_context_argument(node.get("callee"), arguments)
                packed_id = self._unwrap(arguments_nodes[5]).get("decl", "")
                packed_name = self.global_names[packed_id]
                arguments[5:6] = [
                    packed_name,
                    f"(int)sizeof({packed_name})",
                    f"{packed_name}_symbols",
                    f"(int)sizeof({packed_name}_symbols)",
                    f"{packed_name}_rules",
                ]
                return "imgui_c89_render_packed_2bit({})".format(
                    ", ".join(arguments)
                )
            indirect = node.get("callee_expression", {})
            if indirect.get("kind") == "CXXPseudoDestructorExpr":
                destructor = self.destructors_by_type.get(indirect.get("destroyed_type", ""))
                if destructor:
                    destructor_id = self.destructor_ids_by_type.get(
                        indirect.get("destroyed_type", "")
                    )
                    values = self.with_context_argument(
                        destructor_id, [self.expression(indirect["base"])]
                    )
                    return f"{destructor}({', '.join(values)})"
                return "0"
            callee = self.function_names.get(node.get("callee"))
            if not callee:
                callee = self.external_callee(node)
            if not callee:
                if "callee_expression" in node:
                    callee = self.expression(node["callee_expression"])
                else:
                    callee = None
            if not callee:
                if node.get("callee") is None and not node.get("arguments"):
                    # Dependent C++ destructor calls (p->~T()) have no
                    # concrete callee in Clang's non-instantiated AST.  The
                    # translated records are trivially destructible; release
                    # their storage at the owning call site instead.
                    return "0"
                self.fail(node, f"unresolved callee {node.get('callee')}")
            arguments = self.call_arguments(
                node.get("callee"), node.get("arguments", [])
            )
            arguments = self.with_context_argument(
                node.get("callee"), arguments
            )
            return self.call_result(
                node.get("callee"), f"{callee}({', '.join(arguments)})"
            )
        if kind == "UnaryExprOrTypeTraitExpr":
            if node.get("operator") != "sizeof":
                self.fail(node, f"unsupported unary type trait {node.get('operator')}")
            if "argument_type" in node:
                return f"sizeof({self.c_type(node['argument_type'])})"
            argument = self._unwrap(node["argument"])
            field = self.fields.get(argument.get("decl", ""))
            if field:
                return f"sizeof({self.c_type(field['type'])})"
            return f"sizeof({self.expression(node['argument'])})"
        self.fail(node, f"unsupported expression {kind}")

    def default_record_value(self, node: dict[str, Any]) -> str | None:
        spelling = self.base_spelling(node.get("type", ""))
        record = self.records_by_spelling.get(spelling)
        if not record:
            record = self.records_by_c_name.get(self.c_type(spelling))
        if not record:
            return None
        values = []
        for field in record.get("fields", []):
            value = field.get("default")
            constructor = self.default_constructor_by_record.get(
                field.get("record_dependency", "")
            )
            if value is None and constructor:
                value = {
                    "kind": "CXXConstructExpr",
                    "constructor": constructor,
                    "arguments": [],
                    "type": field["type"],
                    "value_category": "prvalue",
                }
            if value is None:
                value = {
                    "kind": "ImplicitValueInitExpr",
                    "type": field["type"],
                    "value_category": "prvalue",
                }
            values.append(value)
        aggregate = {
            "kind": "InitListExpr",
            "type": spelling,
            "values": values,
            "location": node.get("location", {}),
        }
        return self.expression(aggregate)

    def object_pointer(self, node: dict[str, Any]) -> str:
        unwrapped = self._unwrap(node)
        while unwrapped.get("kind") in {
            "MaterializeTemporaryExpr", "ExprWithCleanups", "ConstantExpr",
            "CXXDefaultArgExpr", "CXXBindTemporaryExpr", "CXXDefaultInitExpr",
        } and "operand" in unwrapped:
            unwrapped = self._unwrap(unwrapped["operand"])
        if unwrapped.get("kind") == "ConditionalOperator":
            return "({} ? {} : {})".format(
                self.expression(unwrapped["condition"]),
                self.object_pointer(unwrapped["true"]),
                self.object_pointer(unwrapped["false"]),
            )
        value = self.expression(unwrapped)
        if unwrapped.get("kind") == "CXXThisExpr":
            return value
        if (
            unwrapped.get("kind") == "DeclRefExpr"
            and unwrapped.get("decl") in self.reference_parameters
        ):
            return self.local_names.get(unwrapped.get("decl", ""), unwrapped["name"])
        if unwrapped.get("type", "").strip().endswith("*"):
            return value
        if re.search(r"\[[^\]]*\]$", unwrapped.get("type", "").strip()):
            return value
        if unwrapped.get("value_category") != "lvalue":
            storage_type = self.reference_storage_type(unwrapped.get("type", ""))
            name = f"imgui_c89_temporary_{len(self.expression_temporaries)}"
            self.expression_temporaries.append((name, storage_type))
            return f"({name} = {value}, &{name})"
        return f"&({value})"

    def declaration_name(self, declaration: dict[str, Any]) -> str:
        identifier = declaration.get("id", "")
        return self.local_names.get(
            identifier,
            self.static_local_names.get(
                identifier, declaration.get("name", "local")
            ),
        )

    def collect_local_declarations(self, node: Any) -> list[dict[str, Any]]:
        result: list[dict[str, Any]] = []
        if isinstance(node, dict):
            if node.get("kind") == "LambdaExpr":
                return result
            if node.get("kind") == "DeclStmt":
                result.extend(node.get("declarations", []))
            for value in node.values():
                result.extend(self.collect_local_declarations(value))
        elif isinstance(node, list):
            for value in node:
                result.extend(self.collect_local_declarations(value))
        return result

    def prepare_locals(self, function: dict[str, Any]) -> list[str]:
        self.local_names = {}
        self.static_const_local_ids = set()
        used = {
            parameter.get("name") or f"arg_{index}"
            for index, parameter in enumerate(function.get("parameters", []))
        }
        for index, parameter in enumerate(function.get("parameters", [])):
            self.local_names[parameter["id"]] = parameter.get("name") or f"arg_{index}"
        lines: list[str] = []
        for index, declaration in enumerate(
            self.collect_local_declarations(function.get("body", {}))
        ):
            if (declaration.get("name", "").startswith("__range")
                    and self.base_spelling(
                        declaration.get("type", "").strip()
                        .removesuffix("&").strip()
                    )
                    in {
                        vector[3]
                        for vector in self.flattened_table_vector_fields.values()
                    }):
                self.local_names[declaration["id"]] = declaration.get(
                    "name", "range"
                )
                continue
            if (self.current_compact_nav_key_ranges
                    and (declaration.get("name", "").startswith("__begin2")
                         or declaration.get("name", "").startswith("__end2")
                         or declaration.get("name", "").startswith("__range2")
                         or declaration.get("name", "").startswith("key")
                         or declaration.get("name") in {
                             "nav_gamepad_keys_to_change_source",
                             "nav_keyboard_keys_to_change_source",
                         })):
                self.local_names[declaration["id"]] = declaration.get("name", "local")
                self.static_const_local_ids.add(declaration["id"])
                continue
            if declaration.get("static_local"):
                self.local_names[declaration["id"]] = self.static_local_names[
                    declaration["id"]
                ]
                initializer = declaration.get("initializer", {})
                if (initializer.get("kind") == "CXXConstructExpr"
                        and re.search(r"\[\d+\]$", initializer.get("type", ""))):
                    lines.append(
                        "    int imgui_c89_init_"
                        + self.static_local_names[declaration["id"]]
                        + ";"
                    )
                continue
            base = declaration.get("name") or f"local_{index}"
            name = base
            if name in used:
                name = f"{base}__{hashlib.sha256(declaration['id'].encode()).hexdigest()[:8]}"
            used.add(name)
            self.local_names[declaration["id"]] = name
            initializer = declaration.get("initializer", {})
            if (
                self.compact_static_const_arrays
                and declaration.get("top_level_const")
                and re.search(r"\[\d+\]$", declaration.get("type", ""))
                and initializer.get("kind") == "InitListExpr"
                and not self.static_initializer_requires_runtime(initializer)
            ):
                local_type = declaration["type"]
                if "<" in local_type:
                    local_type = declaration.get("canonical_type", local_type)
                lines.append(
                    f"    static {self.c_declaration(local_type, name)} = "
                    f"{self.c_initializer(initializer)};"
                )
                self.static_const_local_ids.add(declaration["id"])
                continue
            prefix = ""
            suffix = ""
            local_type = declaration["type"]
            if "<" in local_type:
                local_type = declaration.get("canonical_type", local_type)
            if (not declaration.get("static_local")
                    and declaration.get("top_level_const")):
                local_type = declaration.get("unqualified_type", local_type)
                if "[" in local_type:
                    local_type = re.sub(r"^const\s+", "", local_type)
            lines.append(f"    {prefix}{self.c_declaration(local_type, name)}{suffix};")
            if (initializer.get("kind") == "CXXConstructExpr"
                    and not initializer.get("trivial")
                    and re.search(r"\[\d+\]$", initializer.get("type", ""))):
                synthetic = f"imgui_c89_init_{name}"
                if synthetic not in used:
                    used.add(synthetic)
                    lines.append(f"    int {synthetic};")
        if lines:
            lines.append("")
        return lines


    def static_initializer_requires_runtime(self, node: Any) -> bool:
        if isinstance(node, list):
            return any(
                self.static_initializer_requires_runtime(value)
                for value in node
            )
        if not isinstance(node, dict):
            return False
        while node.get("kind") in {
            "MaterializeTemporaryExpr", "ExprWithCleanups", "ConstantExpr",
            "CXXDefaultArgExpr", "CXXBindTemporaryExpr", "CXXDefaultInitExpr",
            "ImplicitCastExpr",
        } and "operand" in node:
            node = node["operand"]
        if node.get("kind") in {
            "CXXConstructExpr", "CXXTemporaryObjectExpr", "CallExpr",
            "CXXMemberCallExpr", "CXXOperatorCallExpr",
        }:
            return True
        if node.get("kind") in {
            "MemberExpr", "ArraySubscriptExpr", "CXXThisExpr",
        }:
            return True
        if node.get("kind") == "DeclRefExpr":
            declaration = node.get("decl")
            return (
                declaration not in self.enum_constant_names
                and declaration not in self.global_names
            )
        return any(
            self.static_initializer_requires_runtime(value)
            for value in node.values()
            if isinstance(value, (dict, list))
        )

    def c_initializer(self, node: dict[str, Any]) -> str:
        while "operand" in node and node.get("kind") in {
            "MaterializeTemporaryExpr", "ExprWithCleanups", "ConstantExpr",
            "CXXDefaultArgExpr", "CXXBindTemporaryExpr", "CXXDefaultInitExpr",
            "ImplicitCastExpr",
        }:
            node = node["operand"]
        if node.get("kind") == "ImplicitValueInitExpr":
            spelling = self.base_spelling(node.get("type", ""))
            record = self.records_by_spelling.get(spelling)
            if record:
                values = [
                    field.get("default", {
                        "kind": "ImplicitValueInitExpr",
                        "type": field["type"],
                        "value_category": "prvalue",
                    })
                    for field in record.get("fields", [])
                ]
                return "{ " + ", ".join(
                    self.c_initializer(value) for value in values
                ) + " }" if values else "{ 0 }"
            return "0"
        if node.get("kind") == "InitListExpr":
            values = node.get("values", [])
            if not values:
                return "{ 0 }"
            return "{ " + ", ".join(
                self.c_initializer(value) for value in values
            ) + " }"
        if node.get("kind") in {"CXXConstructExpr", "CXXTemporaryObjectExpr"}:
            function = self.functions.get(node.get("constructor", ""))
            arguments = node.get("arguments", [])
            if node.get("trivial") and len(arguments) == 1:
                return self.c_initializer(arguments[0])
            record = None
            if function and function.get("parent") in self.records:
                record = self.records[function["parent"]]
            if record is None:
                record = self.records_by_spelling.get(
                    self.base_spelling(node.get("type", ""))
                )
            if record:
                if not arguments:
                    values = [
                        field.get("default", {
                            "kind": "ImplicitValueInitExpr",
                            "type": field["type"],
                            "value_category": "prvalue",
                        })
                        for field in record.get("fields", [])
                    ]
                    return "{ " + ", ".join(
                        self.c_initializer(value) for value in values
                    ) + " }" if values else "{ 0 }"
                if len(arguments) == len(record.get("fields", [])):
                    return "{ " + ", ".join(
                        self.c_initializer(value) for value in arguments
                    ) + " }"
        return self.expression(node)

    def aggregate_assignments(self, target: str, node: dict[str, Any],
                              indent: str) -> list[str]:
        values = node.get("values", [])
        spelling = node.get("type", "")
        record = self.records_by_spelling.get(spelling)
        if not record:
            record = self.records_by_spelling.get(self.base_spelling(spelling))
        if not record:
            record = self.records_by_c_name.get(self.c_type(spelling))
        result: list[str] = []
        if record:
            fields = record.get("fields", [])
            if len(values) > len(fields):
                self.fail(node, "too many record initializers")
            for field, value in zip(fields, values):
                child = f"{target}.{field['name']}"
                if value.get("kind") == "InitListExpr":
                    result.extend(self.aggregate_assignments(child, value, indent))
                else:
                    result.append(indent + f"{child} = {self.expression(value)};")
            return result
        field_names = node.get("field_names", [])
        if field_names:
            if len(values) > len(field_names):
                self.fail(node, "too many external-record initializers")
            for field_name, value in zip(field_names, values):
                child = f"{target}.{field_name}"
                if value.get("kind") == "InitListExpr":
                    result.extend(self.aggregate_assignments(child, value, indent))
                else:
                    result.append(indent + f"{child} = {self.expression(value)};")
            return result
        if "[" in node.get("type", ""):
            for index, value in enumerate(values):
                child = f"{target}[{index}]"
                if value.get("kind") == "InitListExpr":
                    result.extend(self.aggregate_assignments(child, value, indent))
                else:
                    result.append(indent + f"{child} = {self.expression(value)};")
            return result
        if all(value.get("kind") == "ImplicitValueInitExpr" for value in values):
            return result
        self.fail(node, "unresolved aggregate initializer type")

    def implicit_default_construction(
        self, target: str, record: dict[str, Any], indent: str,
        zero_initialize: bool = True,
    ) -> list[str]:
        """Lower an implicit default constructor, including bases/members."""
        result = []
        if zero_initialize:
            result.append(indent + f"memset(&{target}, 0, sizeof({target}));")
        for base in record.get("bases", []):
            base_record = self.records.get(base.get("record", ""))
            if not base_record:
                continue
            constructor_id = self.default_constructor_by_record.get(
                base_record["id"]
            )
            if constructor_id:
                base_type = self.record_names[base_record["qualified_name"]]
                arguments = self.with_context_argument(
                    constructor_id, [f"({base_type} *)&{target}"]
                )
                result.append(
                    indent + f"{self.function_names[constructor_id]}("
                    f"{', '.join(arguments)});"
                )
            else:
                result.extend(self.implicit_default_construction(
                    target, base_record, indent, False
                ))
        for field in record.get("fields", []):
            if field.get("id") in self.flattened_table_vector_fields:
                continue
            field_name = self.field_names[field["id"]]
            if "default" in field:
                result.append(
                    indent + f"{target}.{field_name} = "
                    f"{self.expression(field['default'])};"
                )
                continue
            constructor_id = self.default_constructor_by_record.get(
                field.get("record_dependency", "")
            )
            if constructor_id:
                array = re.search(r"\[(\d+)\]$", field["type"])
                targets = (
                    [f"{target}.{field_name}[{index}]"
                     for index in range(int(array.group(1)))]
                    if array else [f"{target}.{field_name}"]
                )
                result.extend(
                    indent + f"{self.function_names[constructor_id]}("
                    + ", ".join(self.with_context_argument(
                        constructor_id, [f"&{item}"]
                    )) + ");"
                    for item in targets
                )
        return result

    def compact_zero_value(self, node: dict[str, Any]) -> bool:
        """Whether an expression is a side-effect-free scalar zero."""
        node = self._unwrap(node)
        kind = node.get("kind")
        if kind in {"CXXNullPtrLiteralExpr", "GNUNullExpr"}:
            return True
        if kind == "CXXBoolLiteralExpr":
            return not node.get("value")
        if kind in {"IntegerLiteral", "FloatingLiteral"}:
            try:
                return float(node.get("value", 0)) == 0.0
            except (TypeError, ValueError):
                return False
        if kind == "DeclRefExpr":
            return self.enum_constant_values.get(node.get("decl")) == 0
        if kind in {
            "ImplicitCastExpr", "ParenExpr", "CStyleCastExpr",
            "CXXStaticCastExpr", "CXXFunctionalCastExpr",
            "CXXConstCastExpr", "CXXReinterpretCastExpr", "ConstantExpr",
        } and "operand" in node:
            return self.compact_zero_value(node["operand"])
        return False

    @staticmethod
    def compress_string_table(
        strings: list[str],
    ) -> tuple[list[int], list[tuple[int, int]], int]:
        """Byte-pair encode a NUL-delimited debug string table.

        Tokens occupy 128..255 and never span a string boundary, so a caller
        can locate an entry without a separate pointer/offset array.  Rules
        are emitted in creation order and therefore only reference literals
        or earlier rules, keeping the decoder tiny and deterministic.
        """
        sequences = [list(value.encode("utf-8")) for value in strings]
        if any(byte >= 128 for sequence in sequences for byte in sequence):
            raise TranslationError(
                "compressed string pointer globals currently require ASCII"
            )
        rules: list[tuple[int, int]] = []
        for token in range(128, 256):
            counts: dict[tuple[int, int], int] = {}
            for sequence in sequences:
                for pair in zip(sequence, sequence[1:]):
                    counts[pair] = counts.get(pair, 0) + 1
            if not counts:
                break
            pair = max(
                counts,
                key=lambda value: (counts[value], -value[0], -value[1]),
            )
            if counts[pair] <= 2:
                break
            replaced_sequences: list[list[int]] = []
            replacements = 0
            for sequence in sequences:
                replaced: list[int] = []
                index = 0
                while index < len(sequence):
                    if (index + 1 < len(sequence)
                            and (sequence[index], sequence[index + 1]) == pair):
                        replaced.append(token)
                        replacements += 1
                        index += 2
                    else:
                        replaced.append(sequence[index])
                        index += 1
                replaced_sequences.append(replaced)
            if replacements <= 2:
                break
            rules.append(pair)
            sequences = replaced_sequences
        data = [byte for sequence in sequences for byte in (*sequence, 0)]
        def expand(token: int) -> bytes:
            if token < 128:
                return bytes((token,))
            rule_index = token - 128
            if rule_index >= len(rules):
                raise TranslationError(
                    "compressed string table references a missing rule"
                )
            first, second = rules[rule_index]
            return expand(first) + expand(second)

        decoded = [
            b"".join(expand(token) for token in sequence)
            for sequence in sequences
        ]
        expected = [value.encode("utf-8") for value in strings]
        if decoded != expected:
            raise TranslationError("compressed string table round-trip failed")
        maximum = max((len(value.encode("utf-8")) for value in strings), default=0)
        return data, rules, maximum

    @staticmethod
    def compress_byte_stream(
        values: list[int],
    ) -> tuple[list[int], list[int], list[tuple[int, int]]]:
        """Deterministically byte-pair encode a small immutable byte stream.

        Literal bytes are first remapped to a dense alphabet.  New tokens
        follow that alphabet and only reference literals or earlier rules, so
        the emitted C89 decoder needs no dictionary allocation.  A rule is
        accepted only when its two-byte definition is smaller than the bytes
        it removes.
        """
        symbols = sorted(set(values))
        if len(symbols) >= 256:
            return values, list(range(256)), []
        symbol_ids = {value: index for index, value in enumerate(symbols)}
        encoded = [symbol_ids[value] for value in values]
        rules: list[tuple[int, int]] = []
        for token in range(len(symbols), 256):
            counts: dict[tuple[int, int], int] = {}
            for pair in zip(encoded, encoded[1:]):
                counts[pair] = counts.get(pair, 0) + 1
            if not counts:
                break
            pair = max(
                counts,
                key=lambda value: (counts[value], -value[0], -value[1]),
            )
            if counts[pair] <= 2:
                break
            replaced: list[int] = []
            index = 0
            while index < len(encoded):
                if (index + 1 < len(encoded)
                        and (encoded[index], encoded[index + 1]) == pair):
                    replaced.append(token)
                    index += 2
                else:
                    replaced.append(encoded[index])
                    index += 1
            rules.append(pair)
            encoded = replaced
        return encoded, symbols, rules

    def compact_zero_assignment_fields(
        self, node: dict[str, Any]
    ) -> set[str] | None:
        """Return top-level self fields set to zero by an assignment tree."""
        node = self._unwrap(node)
        if node.get("kind") not in {"BinaryOperator", "CompoundAssignOperator"}:
            return None
        if node.get("opcode") != "=":
            return None
        rhs = node.get("rhs", {})
        nested = self.compact_zero_assignment_fields(rhs)
        if nested is None and not self.compact_zero_value(rhs):
            return None
        lhs = self._unwrap(node.get("lhs", {}))
        while lhs.get("kind") in {"ArraySubscriptExpr", "MemberExpr"}:
            if lhs.get("kind") == "MemberExpr":
                base = self._unwrap(lhs.get("base", {}))
                if base.get("kind") == "CXXThisExpr":
                    field = self.field_names.get(
                        lhs.get("member"), lhs.get("name", "")
                    )
                    return (nested or set()) | {field}
                lhs = base
            else:
                lhs = self._unwrap(lhs.get("base", {}))
        return None

    def omit_compact_zero_statement(self, node: dict[str, Any]) -> bool:
        if not self.current_compact_zero_constructor:
            return False
        fields = self.compact_zero_assignment_fields(node)
        return bool(fields) and fields.isdisjoint(
            self.current_constructor_initialized_fields
        )

    @staticmethod
    def compact_crc32_body(qualified_name: str) -> list[str] | None:
        if qualified_name == "ImHashData":
            return [
                "    ImU32 crc = ~seed;",
                "    const unsigned char *data = (const unsigned char *)data_p;",
                "    while (data_size-- > 0)",
                "        crc = imgui_c89_crc32_byte(crc, *data++);",
                "    return ~crc;",
            ]
        if qualified_name == "ImHashStr":
            return [
                "    ImU32 initial = ~seed;",
                "    ImU32 crc = initial;",
                "    const unsigned char *data = (const unsigned char *)data_p;",
                "    unsigned char c;",
                "    if (data_size != 0) {",
                "        while (data_size-- > 0) {",
                "            c = *data++;",
                "            if (c == '#' && data_size >= 2 && data[0] == '#' && data[1] == '#') {",
                "                crc = initial; data += 2; data_size -= 2; continue;",
                "            }",
                "            crc = imgui_c89_crc32_byte(crc, c);",
                "        }",
                "    } else {",
                "        while ((c = *data++) != 0) {",
                "            if (c == '#' && data[0] == '#' && data[1] == '#') {",
                "                crc = initial; data += 2; continue;",
                "            }",
                "            crc = imgui_c89_crc32_byte(crc, c);",
                "        }",
                "    }",
                "    return ~crc;",
            ]
        return None

    @staticmethod
    def _float32(value: float) -> float:
        return struct.unpack("=f", struct.pack("=f", value))[0]

    @staticmethod
    def _palette_unwrap(node: dict[str, Any]) -> dict[str, Any]:
        while node.get("kind") in {
            "CXXConstCastExpr", "ExprWithCleanups", "ImplicitCastExpr",
            "MaterializeTemporaryExpr", "ParenExpr",
        }:
            node = node["operand"]
        return node

    @staticmethod
    def _find_expression_kind(
        node: dict[str, Any], kind: str
    ) -> dict[str, Any] | None:
        if node.get("kind") == kind:
            return node
        for value in node.values():
            if isinstance(value, dict):
                found = Emitter._find_expression_kind(value, kind)
                if found is not None:
                    return found
            elif isinstance(value, list):
                for item in value:
                    if isinstance(item, dict):
                        found = Emitter._find_expression_kind(item, kind)
                        if found is not None:
                            return found
        return None

    def _palette_index(self, node: dict[str, Any]) -> int:
        subscript = self._find_expression_kind(node, "ArraySubscriptExpr")
        if subscript is None:
            raise TranslationError(
                "compact style palette expected a color subscript"
            )
        index = self._palette_unwrap(subscript.get("index", {}))
        value = self.enum_constant_values.get(index.get("decl", ""))
        if value is None:
            raise TranslationError(
                "compact style palette expected an enum index"
            )
        return value

    def _palette_float(self, node: dict[str, Any]) -> float:
        value = self._palette_unwrap(node)
        if value.get("kind") in {"FloatingLiteral", "IntegerLiteral"}:
            return self._float32(float(value["value"]))
        raise TranslationError(
            "compact style palette expected a numeric literal"
        )

    def compact_style_colors_body(
        self, function: dict[str, Any]
    ) -> list[str] | None:
        if not self.compact_style_colors or function.get("qualified_name") not in {
            "ImGui::StyleColorsClassic",
            "ImGui::StyleColorsDark",
            "ImGui::StyleColorsLight",
        }:
            return None
        colors: dict[int, tuple[float, float, float, float]] = {}
        statements = function.get("body", {}).get("statements", [])[2:]
        for statement in statements:
            assignment = self._palette_unwrap(statement)
            arguments = assignment.get("arguments", [])
            if len(arguments) != 2:
                raise TranslationError(
                    "compact style palette expected assignment"
                )
            destination = self._palette_index(arguments[0])
            source = self._palette_unwrap(arguments[1])
            if source.get("kind") == "CXXTemporaryObjectExpr":
                values = tuple(
                    self._palette_float(value)
                    for value in source.get("arguments", [])
                )
                if len(values) != 4:
                    raise TranslationError(
                        "compact style palette expected ImVec4"
                    )
                colors[destination] = values  # type: ignore[assignment]
            elif source.get("kind") == "ArraySubscriptExpr":
                colors[destination] = colors[self._palette_index(source)]
            elif (source.get("kind") == "CallExpr"
                  and source.get("callee_qualified_name") == "ImLerp"):
                lerp_arguments = source.get("arguments", [])
                left = colors[self._palette_index(lerp_arguments[0])]
                right = colors[self._palette_index(lerp_arguments[1])]
                amount = self._palette_float(lerp_arguments[2])
                result = []
                for a, b in zip(left, right):
                    difference = self._float32(b - a)
                    product = self._float32(difference * amount)
                    result.append(self._float32(a + product))
                colors[destination] = tuple(result)  # type: ignore[assignment]
            else:
                raise TranslationError(
                    "compact style palette encountered a non-constant value"
                )
        if not colors or sorted(colors) != list(range(max(colors) + 1)):
            raise TranslationError(
                "compact style palette is not contiguous"
            )

        def literal(value: float) -> str:
            text = format(value, ".9g")
            if not any(character in text for character in ".eE"):
                text += ".0"
            return text + "f"

        get_style = next(
            self.function_names[identifier]
            for identifier, declaration in self.function_declarations.items()
            if declaration.get("qualified_name") == "ImGui::GetStyle"
        )
        components: list[float] = []
        component_indices: dict[float, int] = {}
        indexed_colors: list[tuple[int, int, int, int]] = []
        for index in range(len(colors)):
            indices = []
            for value in colors[index]:
                if value not in component_indices:
                    component_indices[value] = len(components)
                    components.append(value)
                indices.append(component_indices[value])
            indexed_colors.append(tuple(indices))  # type: ignore[arg-type]
        if len(components) > 256:
            raise TranslationError(
                "compact style palette has too many distinct components"
            )
        lines = [
            "    ImGuiStyle *style;",
            f"    static const float components[{len(components)}] = {{",
            "        " + ", ".join(literal(value) for value in components),
            "    };",
            f"    static const unsigned char palette[{len(colors)}][4] = {{",
        ]
        for indices in indexed_colors:
            lines.append(
                "        {" + ", ".join(str(value) for value in indices) + "},"
            )
        lines.extend((
            "    };",
            f"    style = dst ? dst : {get_style}({'' if self.compact_global_context else 'imgui_c89_ctx'});",
            f"    imgui_c89_apply_style_palette(style, components, palette, {len(colors)});",
        ))
        return lines

    def compact_name_switch_body(
        self, function: dict[str, Any]
    ) -> list[str] | None:
        """Pack a contiguous enum-to-literal switch into one stable blob.

        Clang otherwise emits a pointer-width switch table and one relocation
        per entry.  The returned pointers still address immutable static
        storage, exactly like the original string literals.  Shape and enum
        contiguity checks make this a latest-source patch rather than a broad
        semantic guess.
        """
        qualified_name = function.get("qualified_name", "")
        if qualified_name not in self.compact_name_switches:
            return None
        parameters = function.get("parameters", [])
        if len(parameters) != 1:
            raise TranslationError(
                f"compact name switch {qualified_name} expected one parameter"
            )
        parameter = parameters[0].get("name") or "arg_0"
        statements = function.get("body", {}).get("statements", [])
        if len(statements) < 2 or statements[0].get("kind") != "SwitchStmt":
            raise TranslationError(
                f"compact name switch {qualified_name} expected a leading switch"
            )
        switch = statements[0]
        condition = self._find_expression_kind(
            switch.get("condition", {}), "DeclRefExpr"
        )
        if condition is None or condition.get("decl") != parameters[0].get("id"):
            raise TranslationError(
                f"compact name switch {qualified_name} changed its selector"
            )
        entries: dict[int, str] = {}
        for case in switch.get("body", {}).get("statements", []):
            if case.get("kind") != "CaseStmt":
                raise TranslationError(
                    f"compact name switch {qualified_name} contains a non-case"
                )
            constant = self._find_expression_kind(
                case.get("value", {}), "DeclRefExpr"
            )
            literal = self._find_expression_kind(
                case.get("body", {}), "StringLiteral"
            )
            if constant is None or literal is None:
                raise TranslationError(
                    f"compact name switch {qualified_name} expected literal cases"
                )
            value = self.enum_constant_values.get(constant.get("decl", ""))
            if value is None or value in entries:
                raise TranslationError(
                    f"compact name switch {qualified_name} has invalid case values"
                )
            entries[value] = literal["value"]
        if not entries or sorted(entries) != list(range(len(entries))):
            raise TranslationError(
                f"compact name switch {qualified_name} cases are not contiguous"
            )
        names = [entries[index] for index in range(len(entries))]
        offsets: list[int] = []
        offset = 0
        for name in names:
            offsets.append(offset)
            offset += len(name.encode("utf-8")) + 1
        if offset > 65535 or len(names) > 65535:
            raise TranslationError(
                f"compact name switch {qualified_name} exceeds 16-bit storage"
            )
        blob = self._string_literal("\0".join(names) + "\0")
        lines = [
            f"    static const char names[{offset + 1}] = {blob};",
            f"    static const unsigned short offsets[{len(offsets)}] = {{",
            "        " + ", ".join(str(value) for value in offsets),
            "    };",
            f"    if ({parameter} >= 0 && {parameter} < {len(names)})",
            f"        return names + offsets[{parameter}];",
        ]
        for statement in statements[1:]:
            lines.extend(self.statement(statement))
        return lines

    @staticmethod
    def compact_cursor_values(item: dict[str, Any]) -> list[int]:
        values: list[int] = []

        def collect(node: Any) -> None:
            if isinstance(node, dict):
                if node.get("kind") == "IntegerLiteral":
                    values.append(int(node["value"], 0))
                    return
                for value in node.values():
                    collect(value)
            elif isinstance(node, list):
                for value in node:
                    collect(value)

        collect(item.get("initializer", {}))
        if (item.get("type") != "const ImVec2[11][3]"
                or len(values) != 66
                or any(value < 0 or value > 255 for value in values)):
            raise TranslationError(
                "compact cursor metadata changed shape or left byte range"
            )
        return values

    def compact_cursor_body(
        self, function: dict[str, Any]
    ) -> tuple[list[str], list[str]] | None:
        if (not self.compact_cursor_data
                or function.get("qualified_name")
                != "ImFontAtlasGetMouseCursorTexData"):
            return None
        if len(self.compact_cursor_global_ids) != 1:
            raise TranslationError(
                "compact cursor metadata expected one source table"
            )
        identifier = next(iter(self.compact_cursor_global_ids))
        self.compact_cursor_values(self.globals[identifier])
        name = self.global_names[identifier]
        source = self.statement(function["body"])
        hits = [
            index for index, line in enumerate(source) if name in line
        ]
        if (len(hits) != 3
                or hits != [hits[0], hits[0] + 1, hits[0] + 3]
                or not source[hits[0]].strip().startswith("pos = ")
                or not source[hits[1]].strip().startswith("size = ")
                or "out_size" not in source[hits[0] + 2]
                or "out_offset" not in source[hits[2]]):
            raise TranslationError(
                "compact cursor metadata use sites changed shape"
            )
        start = hits[0]
        advance = [
            line for line in source[start + 4:]
            if line.strip().startswith("pos.x += ")
        ]
        if len(advance) != 1:
            raise TranslationError(
                "compact cursor fill offset changed shape"
            )
        # The source expression uses ImVec2 operators and therefore normally
        # needs three aggregate temporaries.  Once the cursor table has been
        # byte-packed, direct component arithmetic is both clearer C and much
        # smaller code.  Keep the validated control-flow prefix (including the
        # atlas rectangle lookup) and replace the remaining vector shell.
        local_lines = [
            "    ImTextureRect *r;",
            "    const unsigned char *cursor_data;",
            "    float x;",
            "    float y;",
            "    float width;",
            "    float height;",
        ]
        replacement = [
            f"    cursor_data = {name} + cursor_type * 6;",
            "    x = (float)cursor_data[0] + (float)r->x;",
            "    y = (float)cursor_data[1] + (float)r->y;",
            "    width = (float)cursor_data[2];",
            "    height = (float)cursor_data[3];",
            "    out_size->x = width;",
            "    out_size->y = height;",
            "    out_offset->x = (float)cursor_data[4];",
            "    out_offset->y = (float)cursor_data[5];",
            "    out_uv_border[0].x = x * atlas->TexUvScale.x;",
            "    out_uv_border[0].y = y * atlas->TexUvScale.y;",
            "    out_uv_border[1].x = (x + width) * atlas->TexUvScale.x;",
            "    out_uv_border[1].y = (y + height) * atlas->TexUvScale.y;",
            advance[0].replace("pos.x", "x"),
            "    out_uv_fill[0].x = x * atlas->TexUvScale.x;",
            "    out_uv_fill[0].y = y * atlas->TexUvScale.y;",
            "    out_uv_fill[1].x = (x + width) * atlas->TexUvScale.x;",
            "    out_uv_fill[1].y = (y + height) * atlas->TexUvScale.y;",
            "    return 1;",
        ]
        return local_lines, source[:start] + replacement

    def compact_separator_values(
        self, declaration: dict[str, Any]
    ) -> list[int]:
        values: list[int] = []
        for item in declaration.get("initializer", {}).get("values", []):
            item = self._unwrap(item)
            while item.get("kind") in {"ConstantExpr", "CStyleCastExpr"}:
                item = self._unwrap(item.get("operand", {}))
            if item.get("kind") not in {"IntegerLiteral", "CharacterLiteral"}:
                self.fail(declaration, "compact separator table is not integral")
            values.append(int(item["value"], 0) if isinstance(item["value"], str)
                          else int(item["value"]))
        if (declaration.get("canonical_type") != "const unsigned int[29]"
                or len(values) != 29
                or any(value < 0 or value > 65535 for value in values)):
            self.fail(declaration, "compact separator table changed shape")
        return values

    def compact_utf8_values(
        self, declaration: dict[str, Any]
    ) -> list[int]:
        values: list[int] = []
        for item in declaration.get("initializer", {}).get("values", []):
            item = self._unwrap(item)
            while item.get("kind") in {"ConstantExpr", "CStyleCastExpr"}:
                item = self._unwrap(item.get("operand", {}))
            if item.get("kind") != "IntegerLiteral":
                self.fail(declaration, "compact UTF-8 table is not integral")
            values.append(int(item["value"], 0))
        if (declaration.get("canonical_type") != "const int[5]"
                or len(values) != 5
                or any(value < 0 or value > 255 for value in values)):
            self.fail(declaration, "compact UTF-8 table changed shape")
        return values

    def compact_color_table_data(
        self, declaration: dict[str, Any]
    ) -> tuple[list[int], str]:
        strings: list[str] = []

        def collect(node: dict[str, Any]) -> None:
            node = self._unwrap(node)
            if node.get("kind") == "InitListExpr":
                for value in node.get("values", []):
                    collect(value)
                return
            if node.get("kind") != "StringLiteral":
                self.fail(declaration, "compact color format table is not strings")
            strings.append(node["value"])

        collect(declaration.get("initializer", {}))
        name = declaration.get("name")
        expected_type = (
            "const char *[4]" if name == "ids" else "const char *[3][4]"
        )
        expected_count = 4 if name == "ids" else 12
        actual_type = declaration.get("canonical_type", declaration.get("type"))
        if actual_type != expected_type or len(strings) != expected_count:
            self.fail(declaration, "compact color format table changed shape")
        string_offsets: dict[str, int] = {}
        blob = ""
        for string in sorted(set(strings), key=lambda value: (-len(value), value)):
            needle = string + "\0"
            offset = blob.find(needle)
            if offset < 0:
                offset = len(blob)
                blob += needle
            string_offsets[string] = offset
        if len(blob) > 65535:
            self.fail(declaration, "compact color format table exceeds 16-bit offsets")
        return [string_offsets[string] for string in strings], blob

    def compact_glyph_delta_data(
        self, declaration: dict[str, Any]
    ) -> list[int]:
        values: list[int] = []
        for item in declaration.get("initializer", {}).get("values", []):
            item = self._unwrap(item)
            while item.get("kind") in {"ConstantExpr", "CStyleCastExpr"}:
                item = self._unwrap(item.get("operand", {}))
            if item.get("kind") != "IntegerLiteral":
                self.fail(declaration, "compact glyph delta table is not integral")
            values.append(int(item["value"], 0))
        canonical = declaration.get("canonical_type", declaration.get("type", ""))
        if (not re.fullmatch(r"const short\[\d+\]", canonical)
                or not values
                or any(value < 0 or value > 32767 for value in values)):
            self.fail(declaration, "compact glyph delta table changed shape")
        if len(values) > 65535:
            self.fail(declaration, "compact glyph delta count exceeds 16 bits")
        # Three direct bits cover the dominant 1..7 offsets.  Escapes get a
        # second nibble tier for 8..22 before falling back to byte/ushort.
        # Keeping each tier contiguous makes the decoder table-free.
        primary = [value if 1 <= value <= 7 else 0 for value in values]
        escaped = [value for value in values if not 1 <= value <= 7]
        secondary = [value - 8 if 8 <= value <= 22 else 15
                     for value in escaped]

        def pack_bits(items: list[int], width: int) -> list[int]:
            packed = [0] * ((len(items) * width + 7) // 8)
            for index, value in enumerate(items):
                bit = index * width
                packed[bit // 8] |= value << (bit & 7)
                if (bit & 7) + width > 8:
                    packed[bit // 8 + 1] |= value >> (8 - (bit & 7))
            return [value & 255 for value in packed]

        encoded = [
            len(values) & 255, len(values) >> 8,
            len(escaped) & 255, len(escaped) >> 8,
            *pack_bits(primary, 3), *pack_bits(secondary, 4),
        ]
        for value in escaped:
            if not 8 <= value <= 22:
                if value < 255:
                    encoded.append(value)
                else:
                    encoded.extend((255, value & 255, value >> 8))
        return encoded

    def compact_glyph_delta_body(
        self, function: dict[str, Any]
    ) -> tuple[list[str], list[str]] | None:
        if (not self.compact_glyph_deltas
                or function.get("qualified_name")
                != "UnpackAccumulativeOffsetsIntoRanges"):
            return None
        parameters = function.get("parameters", [])
        if (len(parameters) != 4
                or parameters[1].get("type") != "const short *"):
            raise TranslationError("compact glyph delta decoder changed signature")
        return [
            "    int delta_index;", "    int delta_count;",
            "    int escape_index;", "    int escape_count;",
            "    int secondary_index;", "    int extra_index;",
            "    int bit_index;", "    int shift;", "    int delta;",
        ], [
            "    delta_count = accumulative_offsets[0]",
            "        | ((int)accumulative_offsets[1] << 8);",
            "    escape_count = accumulative_offsets[2]",
            "        | ((int)accumulative_offsets[3] << 8);",
            "    secondary_index = 4 + (delta_count * 3 + 7) / 8;",
            "    extra_index = secondary_index + (escape_count + 1) / 2;",
            "    escape_index = 0;",
            "    for (delta_index = 0; delta_index < delta_count; ++delta_index) {",
            "        bit_index = delta_index * 3;",
            "        shift = bit_index & 7;",
            "        delta = accumulative_offsets[4 + bit_index / 8] >> shift;",
            "        if (shift > 5)",
            "            delta |= (int)accumulative_offsets[5 + bit_index / 8]",
            "                << (8 - shift);",
            "        delta &= 7;",
            "        if (delta == 0) {",
            "            delta = (accumulative_offsets[secondary_index",
            "                + escape_index / 2] >> ((escape_index & 1) * 4)) & 15;",
            "            ++escape_index;",
            "            if (delta < 15) {",
            "                delta += 8;",
            "            } else {",
            "                delta = accumulative_offsets[extra_index++];",
            "                if (delta == 255) {",
            "                    delta = accumulative_offsets[extra_index]",
            "                        | ((int)accumulative_offsets[extra_index + 1] << 8);",
            "                    extra_index += 2;",
            "                }",
            "            }",
            "        }",
            "        out_ranges[0] = out_ranges[1] = (ImWchar)(base_codepoint + delta);",
            "        base_codepoint += delta;",
            "        out_ranges += 2;",
            "    }",
            "    (void)accumulative_offsets_count;",
            "    out_ranges[0] = 0;",
        ]

    def compact_separator_body(
        self, function: dict[str, Any]
    ) -> tuple[list[str], list[str]] | None:
        if (not self.compact_separator_table
                or function.get("qualified_name")
                != "ImStb::ImCharIsSeparatorW"):
            return None
        matches = [
            identifier for identifier, (owner, _) in self.static_locals.items()
            if owner == function["id"]
            and identifier in self.compact_separator_local_ids
        ]
        if len(matches) != 1:
            raise TranslationError(
                "compact separator table expected one local array"
            )
        identifier = matches[0]
        self.compact_separator_values(self.static_locals[identifier][1])
        name = self.static_local_names[identifier]
        return ["    int separator_index;"], [
            "    for (separator_index = 0; separator_index < 29; ++separator_index) {",
            f"        if (c == {name}[separator_index])",
            "            return 1;",
            "    }",
            "    return 0;",
        ]

    def compact_truetype_only_body(
        self, function: dict[str, Any]
    ) -> list[str] | None:
        if not self.compact_truetype_only:
            return None

        if function.get("qualified_name") == "stbtt_FlattenCurves":
            body = copy.deepcopy(function.get("body", {}))
            switches: list[dict[str, Any]] = []

            def collect_switches(node: Any) -> None:
                if isinstance(node, dict):
                    if node.get("kind") == "SwitchStmt":
                        switches.append(node)
                    for value in node.values():
                        collect_switches(value)
                elif isinstance(node, list):
                    for value in node:
                        collect_switches(value)

            collect_switches(body)
            candidates: list[tuple[dict[str, Any], int]] = []
            for switch in switches:
                statements = switch.get("body", {}).get("statements", [])
                for index, statement in enumerate(statements):
                    if statement.get("kind") != "CaseStmt":
                        continue
                    names: list[str] = []

                    def collect_names(node: Any) -> None:
                        if isinstance(node, dict):
                            if node.get("kind") == "DeclRefExpr":
                                names.append(node.get("name", ""))
                            for value in node.values():
                                collect_names(value)
                        elif isinstance(node, list):
                            for value in node:
                                collect_names(value)

                    collect_names(statement.get("value"))
                    if "STBTT_vcubic" in names:
                        candidates.append((switch, index))
            if len(candidates) != 1:
                raise TranslationError(
                    "compact TrueType curves expected one STBTT_vcubic case"
                )
            switch, first = candidates[0]
            statements = switch["body"]["statements"]
            last = first + 1
            while (last < len(statements)
                   and statements[last].get("kind") not in {
                       "CaseStmt", "DefaultStmt"
                   }):
                last += 1
            del statements[first:last]
            return self.statement(body)

        if function.get("qualified_name") != "stbtt_InitFont_internal":
            return None

        body = copy.deepcopy(function.get("body", {}))
        candidates = []
        for statement in body.get("statements", []):
            if statement.get("kind") != "IfStmt" or "else" not in statement:
                continue
            members: list[str] = []

            def collect_members(node: Any) -> None:
                if isinstance(node, dict):
                    if node.get("kind") == "MemberExpr":
                        members.append(node.get("name", ""))
                    for value in node.values():
                        collect_members(value)
                elif isinstance(node, list):
                    for value in node:
                        collect_members(value)

            collect_members(statement.get("condition"))
            if members == ["glyf"]:
                candidates.append(statement)
        if len(candidates) != 1:
            raise TranslationError(
                "compact TrueType init expected one glyf/CFF branch"
            )
        candidates[0]["else"] = {
            "kind": "CompoundStmt",
            "statements": [{
                "kind": "ReturnStmt",
                "value": {
                    "kind": "IntegerLiteral",
                    "type": "int",
                    "value": "0",
                    "value_category": "prvalue",
                },
            }],
        }
        return self.statement(body)

    def compact_imvector_body(
        self, function: dict[str, Any]
    ) -> list[str] | None:
        qualified_name = function.get("qualified_name", "")
        if not qualified_name.startswith("ImVector<"):
            return None
        method = qualified_name.rsplit("::", 1)[-1]
        parameters = [
            parameter.get("name") or f"arg_{index}"
            for index, parameter in enumerate(function.get("parameters", []))
        ]
        if method == "end":
            if parameters:
                raise TranslationError("ImVector end signature changed")
            # The source spells this as Data + Size.  Empty ImVectors use a
            # null Data pointer, and null + 0 is undefined pointer arithmetic
            # in C even though the intended result is plainly null.
            return [
                "    return self->Data == 0 ? 0 : self->Data + self->Size;"
            ]
        if not self.compact_imvector:
            return None
        if self.compact_imvector_accessors and method in {
            "operator[]", "back", "pop_back", "index_from_ptr"
        }:
            result_type = self.c_type(function.get("return_type", "void"))
            if method == "operator[]":
                if parameters != ["i"]:
                    raise TranslationError(
                        "compact ImVector operator[] signature changed"
                    )
                line = self.imvector_assert_line(
                    function, "operator[]", "i >= 0 && i < Size"
                )
                return [
                    f"    return ({result_type})imgui_c89_vector_at(",
                    "        self->Data, self->Size, i, sizeof(*self->Data), "
                    f"{line});",
                ]
            if method == "back":
                if parameters:
                    raise TranslationError("compact ImVector back signature changed")
                line = self.imvector_assert_line(
                    function, "back", "Size > 0"
                )
                return [
                    f"    return ({result_type})imgui_c89_vector_back(",
                    "        self->Data, self->Size, sizeof(*self->Data), "
                    f"{line});",
                ]
            if method == "pop_back":
                if parameters:
                    raise TranslationError(
                        "compact ImVector pop_back signature changed"
                    )
                self.imvector_assert_line(
                    function, "pop_back", "Size > 0"
                )
                return ["    imgui_c89_vector_pop(&self->Size);"]
            if parameters != ["it"] or result_type != "int":
                raise TranslationError(
                    "compact ImVector index_from_ptr signature changed"
                )
            self.imvector_assert_line(
                function, "index_from_ptr", "it >= Data && it < Data + Size"
            )
            return [
                "    return imgui_c89_vector_index(self->Data, self->Size, it,",
                "        sizeof(*self->Data));",
            ]
        if self.compact_imvector_lifecycle and method in {"~ImVector", "clear"}:
            if parameters or function.get("return_type") != "void":
                raise TranslationError(
                    f"compact ImVector {method} signature changed"
                )
            self.validate_imvector_lifecycle(function, method)
            if method == "~ImVector":
                return [
                    "    imgui_c89_vector_destroy(",
                    f"        {self.current_context_expression()}, self->Data);",
                ]
            return [
                "    imgui_c89_vector_clear(",
                f"        {self.current_context_expression()}, (void **)&self->Data,",
                "        &self->Size, &self->Capacity);",
            ]
        if self.compact_imvector_capacity and method == "_grow_capacity":
            if parameters != ["sz"] or function.get("return_type") != "int":
                raise TranslationError(
                    "compact ImVector _grow_capacity signature changed"
                )
            self.validate_imvector_capacity(function)
            return [
                "    return imgui_c89_vector_grow_capacity(self->Capacity, sz);"
            ]
        if method in {"reserve", "reserve_discard"}:
            if parameters != ["new_capacity"]:
                raise TranslationError(
                    f"compact ImVector {method} signature changed"
                )
            discard = "1" if method == "reserve_discard" else "0"
            return [
                "    self->Data = imgui_c89_vector_reserve("
                + self.current_context_expression()
                + ", self->Data, self->Size,",
                "        &self->Capacity, new_capacity, sizeof(*self->Data), "
                + discard + ");",
            ]
        if method == "resize":
            if parameters == ["new_size"]:
                return [
                    "    self->Data = imgui_c89_vector_resize("
                    + self.current_context_expression() + ", self->Data,",
                    "        &self->Size, &self->Capacity, new_size, sizeof(*self->Data));",
                ]
            if parameters == ["new_size", "v"]:
                return [
                    "    self->Data = imgui_c89_vector_resize_fill("
                    + self.current_context_expression() + ", self->Data,",
                    "        &self->Size, &self->Capacity, new_size, sizeof(*self->Data), v);",
                ]
            raise TranslationError("compact ImVector resize signature changed")
        if method == "push_back":
            if parameters != ["v"]:
                raise TranslationError("compact ImVector push_back signature changed")
            return [
                "    self->Data = imgui_c89_vector_push_back("
                + self.current_context_expression() + ", self->Data,",
                "        &self->Size, &self->Capacity, sizeof(*self->Data), v);",
            ]
        if method == "erase":
            if parameters == ["it"]:
                return [
                    "    return imgui_c89_vector_erase(self->Data, &self->Size,",
                    "        sizeof(*self->Data), it, it + 1);",
                ]
            if parameters == ["it", "it_last"]:
                return [
                    "    return imgui_c89_vector_erase(self->Data, &self->Size,",
                    "        sizeof(*self->Data), it, it_last);",
                ]
            raise TranslationError("compact ImVector erase signature changed")
        if method == "erase_unsorted":
            if parameters != ["it"]:
                raise TranslationError(
                    "compact ImVector erase_unsorted signature changed"
                )
            return [
                "    return imgui_c89_vector_erase_unsorted(self->Data,",
                "        &self->Size, sizeof(*self->Data), it);",
            ]
        if method == "insert":
            if parameters != ["it", "v"]:
                raise TranslationError("compact ImVector insert signature changed")
            return [
                "    return imgui_c89_vector_insert("
                + self.current_context_expression()
                + ", (void **)&self->Data,",
                "        &self->Size, &self->Capacity, sizeof(*self->Data), it, v);",
            ]
        if method == "push_front":
            if parameters != ["v"]:
                raise TranslationError("compact ImVector push_front signature changed")
            return [
                "    (void)imgui_c89_vector_insert("
                + self.current_context_expression()
                + ", (void **)&self->Data,",
                "        &self->Size, &self->Capacity, sizeof(*self->Data), self->Data, v);",
            ]
        return None

    def validate_imvector_lifecycle(
        self, function: dict[str, Any], method: str
    ) -> None:
        """Require the exact raw-storage clear/destructor source shape."""
        body = function.get("body", {})
        statements = body.get("statements", [])
        if body.get("kind") != "CompoundStmt" or len(statements) != 1:
            raise TranslationError(
                f"compact ImVector {method} body changed shape"
            )
        branch = statements[0]

        def member(node: dict[str, Any]) -> str | None:
            node = self._unwrap(node)
            return node.get("name") if node.get("kind") == "MemberExpr" else None

        condition = self._unwrap(branch.get("condition", {}))
        if (branch.get("kind") != "IfStmt"
                or member(condition) != "Data"
                or "else" in branch):
            raise TranslationError(
                f"compact ImVector {method} condition changed shape"
            )
        consequent = branch.get("then", {})
        inner = (
            [consequent] if method == "~ImVector"
            else consequent.get("statements", [])
        )
        expected_kinds = (
            ["CallExpr"] if method == "~ImVector"
            else ["BinaryOperator", "CallExpr", "BinaryOperator"]
        )
        if [item.get("kind") for item in inner] != expected_kinds:
            raise TranslationError(
                f"compact ImVector {method} operations changed shape"
            )
        call = inner[0] if method == "~ImVector" else inner[1]
        callee = self.function_declarations.get(call.get("callee", ""), {})
        arguments = call.get("arguments", [])
        if (callee.get("qualified_name") != "ImGui::MemFree"
                or len(arguments) != 1
                or member(arguments[0]) != "Data"):
            raise TranslationError(
                f"compact ImVector {method} allocator call changed"
            )
        if method == "~ImVector":
            return
        first, _, last = inner
        nested = self._unwrap(first.get("rhs", {}))
        if (first.get("opcode") != "=" or member(first.get("lhs", {})) != "Size"
                or nested.get("kind") != "BinaryOperator"
                or nested.get("opcode") != "="
                or member(nested.get("lhs", {})) != "Capacity"
                or not self.compact_zero_value(nested.get("rhs", {}))
                or last.get("opcode") != "="
                or member(last.get("lhs", {})) != "Data"
                or not self.compact_zero_value(last.get("rhs", {}))):
            raise TranslationError("compact ImVector clear assignments changed")

    def validate_imvector_capacity(self, function: dict[str, Any]) -> None:
        """Require upstream's exact 1.5x/minimum-eight capacity formula."""
        body = function.get("body", {})
        statements = body.get("statements", [])
        if (body.get("kind") != "CompoundStmt" or len(statements) != 2
                or [item.get("kind") for item in statements]
                != ["DeclStmt", "ReturnStmt"]):
            raise TranslationError("compact ImVector capacity body changed")
        declarations = statements[0].get("declarations", [])
        if (len(declarations) != 1
                or declarations[0].get("name") != "new_capacity"
                or declarations[0].get("type") != "int"):
            raise TranslationError("compact ImVector capacity local changed")
        initializer = self._unwrap(declarations[0].get("initializer", {}))

        def member(node: dict[str, Any]) -> str | None:
            node = self._unwrap(node)
            return node.get("name") if node.get("kind") == "MemberExpr" else None

        def declaration(node: dict[str, Any]) -> str | None:
            node = self._unwrap(node)
            return node.get("name") if node.get("kind") == "DeclRefExpr" else None

        def integer(node: dict[str, Any], value: int) -> bool:
            node = self._unwrap(node)
            return (node.get("kind") == "IntegerLiteral"
                    and int(node.get("value", "-1"), 0) == value)

        growth = self._unwrap(initializer.get("true", {}))
        half = self._unwrap(growth.get("rhs", {}))
        result = self._unwrap(statements[1].get("value", {}))
        comparison = self._unwrap(result.get("condition", {}))
        if (initializer.get("kind") != "ConditionalOperator"
                or member(initializer.get("condition", {})) != "Capacity"
                or growth.get("kind") != "BinaryOperator"
                or growth.get("opcode") != "+"
                or member(growth.get("lhs", {})) != "Capacity"
                or half.get("kind") != "BinaryOperator"
                or half.get("opcode") != "/"
                or member(half.get("lhs", {})) != "Capacity"
                or not integer(half.get("rhs", {}), 2)
                or not integer(initializer.get("false", {}), 8)
                or result.get("kind") != "ConditionalOperator"
                or comparison.get("kind") != "BinaryOperator"
                or comparison.get("opcode") != ">"
                or declaration(comparison.get("lhs", {})) != "new_capacity"
                or declaration(comparison.get("rhs", {})) != "sz"
                or declaration(result.get("true", {})) != "new_capacity"
                or declaration(result.get("false", {})) != "sz"):
            raise TranslationError("compact ImVector capacity formula changed")


    def imvector_assert_line(
        self, function: dict[str, Any], method: str, message: str
    ) -> int:
        """Validate and retain the diagnostic metadata of an ImVector check."""
        calls: list[dict[str, Any]] = []

        def visit(node: Any) -> None:
            if isinstance(node, list):
                for value in node:
                    visit(value)
                return
            if not isinstance(node, dict):
                return
            if (node.get("kind") == "CallExpr"
                    and node.get("callee_name") in {
                        "__assert_rtn", "ImGuiTestEngine_AssertLog"
                    }):
                calls.append(node)
            for value in node.values():
                visit(value)

        def string_value(node: dict[str, Any]) -> str | None:
            node = self._unwrap(node)
            if node.get("kind") == "StringLiteral":
                return node.get("value")
            for value in node.values():
                if isinstance(value, dict):
                    found = string_value(value)
                    if found is not None:
                        return found
                elif isinstance(value, list):
                    for item in value:
                        if isinstance(item, dict):
                            found = string_value(item)
                            if found is not None:
                                return found
            return None

        visit(function.get("body", {}))
        if len(calls) != 1:
            raise TranslationError(
                f"compact ImVector {method} expected one assertion"
            )
        call = calls[0]
        arguments = call.get("arguments", [])
        backend = call.get("callee_name")
        if backend == "__assert_rtn":
            file_name = string_value(arguments[1]) if len(arguments) == 4 else None
            line_node = self._unwrap(arguments[2]) if len(arguments) == 4 else {}
            actual_message = string_value(arguments[3]) if len(arguments) == 4 else None
        else:
            actual_message = string_value(arguments[0]) if len(arguments) == 4 else None
            file_name = string_value(arguments[1]) if len(arguments) == 4 else None
            line_node = self._unwrap(arguments[3]) if len(arguments) == 4 else {}
        if (len(arguments) != 4
                or file_name is None or not file_name.endswith("imgui.h")
                or line_node.get("kind") != "IntegerLiteral"
                or actual_message != message):
            raise TranslationError(
                f"compact ImVector {method} assertion metadata changed"
            )
        if self.imvector_assert_backend is None:
            self.imvector_assert_backend = str(backend)
            self.imvector_assert_file = file_name
        elif (self.imvector_assert_backend != backend
                or self.imvector_assert_file != file_name):
            raise TranslationError(
                "compact ImVector assertion backend changed within program"
            )
        return int(line_node["value"], 0)

    def imvector_assert_runtime_lines(
        self, indent: str, message: str, line: str
    ) -> list[str]:
        file_name = self._string_literal(self.imvector_assert_file or "imgui.h")
        message_literal = self._string_literal(message)
        if self.imvector_assert_backend == "ImGuiTestEngine_AssertLog":
            return [
                f"{indent}imgui_c89_external_ImGuiTestEngine_AssertLog(",
                f"{indent}    {message_literal}, {file_name}, \"\", {line});",
                f"{indent}imgui_c89_debugtrap();",
            ]
        if self.imvector_assert_backend != "__assert_rtn":
            raise TranslationError("compact ImVector assertion backend missing")
        return [
            f"{indent}imgui_c89_assert_rtn(\"\", {file_name}, {line}, "
            f"{message_literal});"
        ]

    def assertion_metadata(
        self, node: dict[str, Any]
    ) -> tuple[str, int, str]:
        """Recover the exact metadata passed by an expanded IM_ASSERT."""
        arguments = node.get("arguments", [])
        if (node.get("kind") != "CallExpr"
                or node.get("callee_name") != "__assert_rtn"
                or len(arguments) != 4):
            self.fail(node, "compact assertion metadata call shape changed")

        def find_kind(value: Any, kind: str) -> dict[str, Any] | None:
            if isinstance(value, dict):
                if value.get("kind") == kind:
                    return value
                for child in value.values():
                    found = find_kind(child, kind)
                    if found is not None:
                        return found
            elif isinstance(value, list):
                for child in value:
                    found = find_kind(child, kind)
                    if found is not None:
                        return found
            return None

        function = find_kind(arguments[0], "PredefinedExpr")
        file_name = find_kind(arguments[1], "StringLiteral")
        line = self._unwrap(arguments[2])
        message = find_kind(arguments[3], "StringLiteral")
        if (function is None or file_name is None or message is None
                or line.get("kind") != "IntegerLiteral"):
            self.fail(node, "compact assertion metadata arguments changed")
        return (
            str(file_name["value"]), int(line["value"], 0),
            str(message["value"]),
        )

    def configure_assert_metadata(self) -> None:
        """Build a deterministic TU-local dictionary of assertion records."""
        self.compact_assert_ids = {}
        self.compact_assert_records = []
        if not self.compact_assert_metadata:
            return
        records: set[tuple[str, int, str]] = set()

        def visit(node: Any) -> None:
            if isinstance(node, list):
                for value in node:
                    visit(value)
                return
            if not isinstance(node, dict):
                return
            if (node.get("kind") == "CallExpr"
                    and node.get("callee_name") == "__assert_rtn"):
                records.add(self.assertion_metadata(node))
            for value in node.values():
                visit(value)

        for identifier, function in self.functions.items():
            if (self.active_function_ids is not None
                    and identifier not in self.active_function_ids):
                continue
            visit(function.get("body", {}))
        self.compact_assert_records = sorted(records)
        self.compact_assert_ids = {
            record: index
            for index, record in enumerate(self.compact_assert_records)
        }

    def configure_effective_reachability(self) -> None:
        """Prune TU-local definitions made dead by handwritten overlays.

        Cross-TU and externally observable definitions are unconditional roots.
        A handwritten definition replaces, rather than augments, its original
        IR call edges; its effective edges are the symbols actually selected by
        the template's resolved function tokens. Only definitions already
        proven TU-local by ``analyze_function_linkage`` may be removed.
        """
        if self.active_function_ids is None:
            return
        active = set(self.active_function_ids)
        known_active = active & set(self.functions)
        effective_references = {
            identifier: set(references)
            for identifier, references in self.function_references.items()
        }
        for identifier in sorted(known_active):
            function = self.functions[identifier]
            previous_function_id = self.current_function_id
            self.current_function_id = identifier
            try:
                compact_vector = self.compact_imvector_body(function)
                table_pool_dependencies = (
                    self.flattened_table_pool_dependencies(function)
                )
                compact_table_add = (
                    self.compact_table_pool_add_constructor(function)
                )
            finally:
                self.current_function_id = previous_function_id
            if table_pool_dependencies is not None:
                effective_references[identifier] = set(
                    table_pool_dependencies
                )
                continue
            if compact_vector is not None or compact_table_add is not None:
                # These lowerings call only the shared C runtime.  Keeping the
                # original template-method edges here retained dead reserve /
                # grow helpers in source after their callers were replaced.
                effective_references[identifier] = set()
                continue
            if self.handwritten_specification(function) is None:
                continue
            body = self.handwritten_function_body(
                function, resolve_asserts=False
            )
            if body is None:
                raise TranslationError(
                    "matched handwritten function lost its replacement body: "
                    + function.get("qualified_name", identifier)
                )
            effective_references[identifier] = set(
                self.handwritten_function_dependencies.get(identifier, set())
            )
            if function.get("destructor") and function.get("parent") in self.records:
                effective_references[identifier].update(
                    self.record_subobject_destructor_dependencies(
                        self.records[function["parent"]]
                    )
                )

        roots = {
            identifier for identifier in known_active
            if identifier not in self.internal_functions
        }
        reachable = set(roots)
        pending = list(sorted(roots))
        while pending:
            source = pending.pop()
            for target in effective_references.get(source, set()):
                if target not in known_active or target in reachable:
                    continue
                reachable.add(target)
                pending.append(target)

        removed = known_active - reachable
        unsafe = removed - self.internal_functions
        if unsafe:
            raise TranslationError(
                "effective reachability attempted to prune non-local "
                "definitions: " + ", ".join(sorted(unsafe))
            )
        self.active_function_ids = (active - removed)

    def emit_assert_metadata_support(self) -> list[str]:
        if not self.compact_assert_records:
            return []
        records = self.compact_assert_records
        files = sorted({record[0] for record in records})
        messages = sorted({record[2] for record in records})

        def blob_offsets(values: list[str]) -> tuple[str, dict[str, int]]:
            blob = ""
            offsets: dict[str, int] = {}
            for value in values:
                offsets[value] = len(blob.encode("utf-8"))
                blob += value + "\0"
            return blob, offsets

        file_blob, file_offsets = blob_offsets(files)
        message_data, message_rules, message_maximum = (
            self.compress_string_table(messages)
        )
        if (len(file_blob.encode("utf-8")) > 65535
                or len(files) > 255
                or len(messages) > 65535
                or any(record[1] < 0 or record[1] > 65535 for record in records)):
            raise TranslationError("compact assertion metadata exceeds encoding")
        file_ids = {value: index for index, value in enumerate(files)}
        message_ids = {value: index for index, value in enumerate(messages)}
        file_id_width = max(1, (len(files) - 1).bit_length())
        packed_file_ids = [
            0 for _ in range((len(records) * file_id_width + 7) // 8 + 1)
        ]
        for index, record in enumerate(records):
            value = file_ids[record[0]]
            bit = index * file_id_width
            packed_file_ids[bit // 8] |= value << (bit & 7)
            packed_file_ids[bit // 8 + 1] |= value >> (8 - (bit & 7))
        packed_file_ids = [value & 255 for value in packed_file_ids]
        decoded_file_ids = []
        for index in range(len(records)):
            bit = index * file_id_width
            offset = bit // 8
            value = (
                packed_file_ids[offset]
                | packed_file_ids[offset + 1] << 8
            )
            decoded_file_ids.append(
                (value >> (bit & 7)) & ((1 << file_id_width) - 1)
            )
        if decoded_file_ids != [file_ids[record[0]] for record in records]:
            raise TranslationError("compact assertion file ID round-trip failed")
        file_offset_type = (
            "unsigned char"
            if len(file_blob.encode("utf-8")) <= 256
            else "unsigned short"
        )
        message_id_type = (
            "unsigned char" if len(messages) <= 256 else "unsigned short"
        )
        if len(messages) > 256:
            message_id_width = (len(messages) - 1).bit_length()
            packed_message_ids = [
                0
                for _ in range(
                    (len(records) * message_id_width + 7) // 8 + 2
                )
            ]
            for index, record in enumerate(records):
                value = message_ids[record[2]]
                bit = index * message_id_width
                packed_message_ids[bit // 8] |= value << (bit & 7)
                packed_message_ids[bit // 8 + 1] |= value >> (8 - (bit & 7))
                packed_message_ids[bit // 8 + 2] |= value >> (16 - (bit & 7))
            packed_message_ids = [value & 255 for value in packed_message_ids]
            decoded_message_ids = []
            for index in range(len(records)):
                bit = index * message_id_width
                offset = bit // 8
                value = (
                    packed_message_ids[offset]
                    | packed_message_ids[offset + 1] << 8
                    | packed_message_ids[offset + 2] << 16
                )
                decoded_message_ids.append(
                    (value >> (bit & 7)) & ((1 << message_id_width) - 1)
                )
            if decoded_message_ids != [
                message_ids[record[2]] for record in records
            ]:
                raise TranslationError(
                    "compact assertion message ID round-trip failed"
                )
            message_id_declaration = [
                "static const unsigned char imgui_c89_assert_message_ids[] = { "
                + ", ".join(str(value) for value in packed_message_ids) + " };"
            ]
            message_id_locals = [
                f"    unsigned int message_bit = id * {message_id_width};",
                "    unsigned int message_offset = message_bit >> 3;",
                "    unsigned int message_id = (",
                "        imgui_c89_assert_message_ids[message_offset]",
                "        | (unsigned int)imgui_c89_assert_message_ids[message_offset + 1] << 8",
                "        | (unsigned int)imgui_c89_assert_message_ids[message_offset + 2] << 16)",
                f"        >> (message_bit & 7) & {(1 << message_id_width) - 1};",
            ]
            message_id_expression = "message_id"
        else:
            message_id_declaration = [
                f"static const {message_id_type} imgui_c89_assert_message_ids[] = {{ "
                + ", ".join(str(message_ids[record[2]]) for record in records)
                + " };"
            ]
            message_id_locals = []
            message_id_expression = "imgui_c89_assert_message_ids[id]"
        return [
            "static const char imgui_c89_assert_files[] = "
            + self._string_literal(file_blob) + ";",
            f"static const {file_offset_type} imgui_c89_assert_file_offsets[] = {{ "
            + ", ".join(str(file_offsets[value]) for value in files) + " };",
            "static const unsigned char imgui_c89_assert_message_data[] = { "
            + ", ".join(str(value) for value in message_data) + " };",
            "static const unsigned char imgui_c89_assert_message_rules[] = { "
            + ", ".join(
                str(value) for pair in message_rules for value in pair
            ) + " };",
            *message_id_declaration,
            "static const unsigned short imgui_c89_assert_lines[] = { "
            + ", ".join(str(record[1]) for record in records) + " };",
            "static const unsigned char imgui_c89_assert_file_ids[] = { "
            + ", ".join(str(value) for value in packed_file_ids) + " };",
            "static void imgui_c89_assert_id(unsigned int id)",
            "{",
            f"    char message[{message_maximum + 1}];",
            *message_id_locals,
            f"    unsigned int file_bit = id * {file_id_width};",
            "    unsigned int file_offset = file_bit >> 3;",
            "    unsigned int file_id = (imgui_c89_assert_file_ids[file_offset]",
            "        | (unsigned int)imgui_c89_assert_file_ids[file_offset + 1] << 8)",
            f"        >> (file_bit & 7) & {(1 << file_id_width) - 1};",
            "    imgui_c89_assert_rtn(\"\",",
            "        imgui_c89_assert_files + imgui_c89_assert_file_offsets[file_id],",
            "        imgui_c89_assert_lines[id],",
            "        imgui_c89_compressed_string_at(imgui_c89_assert_message_data,",
            "            imgui_c89_assert_message_rules,",
            f"            {len(message_rules)}, (int){message_id_expression}, message));",
            "}",
            "",
        ]

    def compact_imchunkstream_body(
        self, function: dict[str, Any]
    ) -> list[str] | None:
        if not self.compact_imchunkstream:
            return None
        qualified_name = function.get("qualified_name", "")
        match = re.fullmatch(
            r"ImChunkStream<(ImGuiTableSettings|ImGuiWindowSettings)>::"
            r"(alloc_chunk|begin|chunk_size|clear|empty|end|next_chunk|"
            r"offset_from_ptr|ptr_from_offset|size|swap)",
            qualified_name,
        )
        if not match:
            return None
        method = match.group(2)
        result_type = self.c_type(function.get("return_type", "void"))
        if method == "alloc_chunk":
            return [
                f"    return ({result_type})imgui_c89_chunk_alloc(",
                "        imgui_c89_ctx, self, sz);",
            ]
        if method == "begin":
            return [f"    return ({result_type})imgui_c89_chunk_begin(self);"]
        if method == "chunk_size":
            return ["    (void)self;", "    return imgui_c89_chunk_size(p);"]
        if method == "clear":
            return ["    imgui_c89_chunk_clear(imgui_c89_ctx, self);"]
        if method == "empty":
            return ["    return imgui_c89_chunk_empty(self);"]
        if method == "end":
            return [f"    return ({result_type})imgui_c89_chunk_end(self);"]
        if method == "next_chunk":
            return [
                f"    return ({result_type})imgui_c89_chunk_next(self, p);"
            ]
        if method == "offset_from_ptr":
            return ["    return imgui_c89_chunk_offset(self, p);"]
        if method == "ptr_from_offset":
            return [f"    return ({result_type})imgui_c89_chunk_ptr(self, off);"]
        if method == "size":
            return ["    return imgui_c89_chunk_size_bytes(self);"]
        if method == "swap":
            return ["    imgui_c89_chunk_swap(self, rhs);"]
        raise AssertionError(method)

    @staticmethod
    def expression_has_only_self_effects(node: Any) -> bool:
        """Accept expressions that can only read values or assign into self."""
        if not isinstance(node, dict):
            return False
        kind = node.get("kind")
        if kind in {
            "FloatingLiteral", "IntegerLiteral", "CXXBoolLiteralExpr",
            "CharacterLiteral", "CXXNullPtrLiteralExpr", "GNUNullExpr",
            "DeclRefExpr", "CXXThisExpr",
        }:
            return True
        if kind == "MemberExpr":
            return Emitter.expression_has_only_self_effects(node.get("base"))
        if kind in {
            "ImplicitCastExpr", "ParenExpr", "ConstantExpr",
            "CStyleCastExpr", "CXXStaticCastExpr", "CXXFunctionalCastExpr",
            "CXXConstCastExpr", "CXXReinterpretCastExpr", "UnaryOperator",
            "MaterializeTemporaryExpr", "ExprWithCleanups",
        }:
            return Emitter.expression_has_only_self_effects(node.get("operand"))
        if kind in {"BinaryOperator", "CompoundAssignOperator"}:
            lhs = node.get("lhs")
            if node.get("opcode") == "=":
                lhs = Emitter.unwrap_expression_node(lhs)
                if (lhs.get("kind") != "MemberExpr"
                        or not Emitter.expression_has_only_self_effects(
                            lhs.get("base")
                        )):
                    return False
            elif not Emitter.expression_has_only_self_effects(lhs):
                return False
            return Emitter.expression_has_only_self_effects(node.get("rhs"))
        if kind == "ConditionalOperator":
            return all(
                Emitter.expression_has_only_self_effects(node.get(key))
                for key in ("condition", "true", "false")
            )
        if kind == "UnaryExprOrTypeTraitExpr":
            argument = node.get("argument")
            return argument is None or Emitter.expression_has_only_self_effects(
                argument
            )
        return False

    @staticmethod
    def unwrap_expression_node(node: Any) -> dict[str, Any]:
        while isinstance(node, dict) and node.get("kind") in {
            "ImplicitCastExpr", "ParenExpr", "ConstantExpr",
            "CStyleCastExpr", "CXXStaticCastExpr", "CXXFunctionalCastExpr",
            "CXXConstCastExpr", "CXXReinterpretCastExpr",
        }:
            node = node.get("operand", {})
        return node if isinstance(node, dict) else {}

    @classmethod
    def zero_memset_of_self(cls, node: Any) -> bool:
        if not isinstance(node, dict) or node.get("kind") != "CallExpr":
            return False
        arguments = node.get("arguments", [])
        if node.get("callee_name") != "memset" or len(arguments) != 3:
            return False
        target = cls.unwrap_expression_node(arguments[0])
        zero = cls.unwrap_expression_node(arguments[1])
        size = cls.unwrap_expression_node(arguments[2])
        if (target.get("kind") != "CXXThisExpr"
                or zero.get("kind") != "IntegerLiteral"
                or int(zero.get("value", "1"), 0) != 0
                or size.get("kind") != "UnaryExprOrTypeTraitExpr"
                or size.get("operator") != "sizeof"):
            return False
        argument = cls.unwrap_expression_node(size.get("argument", {}))
        return (
            argument.get("kind") == "UnaryOperator"
            and argument.get("opcode") == "*"
            and cls.unwrap_expression_node(argument.get("operand", {})).get(
                "kind"
            ) == "CXXThisExpr"
        )

    def constructor_has_only_self_effects(
        self, constructor_id: str, visiting: set[str] | None = None,
    ) -> bool:
        """Prove a constructor discarded before a later memset has no effects."""
        visiting = set() if visiting is None else set(visiting)
        if constructor_id in visiting:
            return False
        constructor = self.functions.get(constructor_id)
        if constructor is None or not constructor.get("constructor"):
            return False
        visiting.add(constructor_id)
        for initializer in constructor.get("initializers", []):
            value = initializer.get("value", {})
            if value.get("kind") == "CXXConstructExpr":
                nested = value.get("constructor", "")
                if (not all(
                        self.expression_has_only_self_effects(argument)
                        for argument in value.get("arguments", [])
                    ) or not self.constructor_has_only_self_effects(
                        nested, visiting
                    )):
                    return False
            elif not self.expression_has_only_self_effects(value):
                return False
        for statement in constructor.get("body", {}).get("statements", []):
            if self.zero_memset_of_self(statement):
                continue
            if (statement.get("kind") not in {
                    "BinaryOperator", "CompoundAssignOperator"
                } or statement.get("opcode") != "="
                    or not self.expression_has_only_self_effects(statement)):
                return False
        return True

    def validate_compact_table_constructor(self, constructor_id: str) -> None:
        constructor = self.functions.get(constructor_id, {})
        statements = constructor.get("body", {}).get("statements", [])
        if (constructor.get("qualified_name") != "ImGuiTable::ImGuiTable"
                or constructor.get("parameters")
                or len(statements) != 2
                or not self.zero_memset_of_self(statements[0])):
            raise TranslationError(
                "compact ImPool<ImGuiTable>::Add constructor changed shape"
            )
        assignment = statements[1]
        lhs = self.unwrap_expression_node(assignment.get("lhs", {}))
        rhs = self.unwrap_expression_node(assignment.get("rhs", {}))
        minus_one = (
            rhs.get("kind") == "UnaryOperator"
            and rhs.get("opcode") == "-"
            and self.unwrap_expression_node(rhs.get("operand", {})).get(
                "kind"
            ) == "IntegerLiteral"
            and int(
                self.unwrap_expression_node(rhs.get("operand", {})).get(
                    "value", "0"
                ), 0
            ) == 1
        )
        if (assignment.get("kind") != "BinaryOperator"
                or assignment.get("opcode") != "="
                or lhs.get("kind") != "MemberExpr"
                or lhs.get("name") != "LastFrameActive"
                or self.unwrap_expression_node(lhs.get("base", {})).get(
                    "kind"
                ) != "CXXThisExpr"
                or not minus_one):
            raise TranslationError(
                "compact ImPool<ImGuiTable>::Add defaults changed"
            )
        for initializer in constructor.get("initializers", []):
            value = initializer.get("value", {})
            nested = value.get("constructor", "")
            if (value.get("kind") != "CXXConstructExpr"
                    or not all(
                        self.expression_has_only_self_effects(argument)
                        for argument in value.get("arguments", [])
                    )
                    or not self.constructor_has_only_self_effects(nested)):
                raise TranslationError(
                    "compact ImGuiTable member construction gained side effects"
                )

    def compact_table_pool_add_constructor(
        self, function: dict[str, Any]
    ) -> str | None:
        if (not self.compact_impool
                or function.get("qualified_name")
                != "ImPool<ImGuiTable>::Add"):
            return None
        constructors = [
            identifier for identifier, candidate in self.functions.items()
            if candidate.get("qualified_name") == "ImGuiTable::ImGuiTable"
            and candidate.get("constructor")
            and not candidate.get("parameters")
        ]
        if len(constructors) != 1:
            raise TranslationError(
                "compact ImPool expected one ImGuiTable constructor"
            )
        constructor_id = constructors[0]
        statements = function.get("body", {}).get("statements", [])
        if [statement.get("kind") for statement in statements] != [
            "DeclStmt", "IfStmt", "ExprWithCleanups", "UnaryOperator",
            "ReturnStmt",
        ]:
            raise TranslationError(
                "compact ImPool<ImGuiTable>::Add body changed shape"
            )
        placement = statements[2].get("operand", {})
        initializer = placement.get("initializer", {})
        alive = statements[3]
        alive_operand = self.unwrap_expression_node(alive.get("operand", {}))
        if (placement.get("kind") != "CXXNewExpr"
                or placement.get("array")
                or placement.get("allocated_type") != "ImGuiTable"
                or initializer.get("kind") != "CXXConstructExpr"
                or initializer.get("constructor") != constructor_id
                or initializer.get("arguments")
                or alive.get("kind") != "UnaryOperator"
                or alive.get("opcode") != "++"
                or alive_operand.get("kind") != "MemberExpr"
                or alive_operand.get("name") != "AliveCount"):
            raise TranslationError(
                "compact ImPool<ImGuiTable>::Add placement construction changed"
            )
        self.validate_compact_table_constructor(constructor_id)
        return constructor_id

    def is_flattened_table_pool_function(
        self, function: dict[str, Any]
    ) -> bool:
        return (
            self.flatten_table_pool
            and function.get("qualified_name", "").startswith(
                "ImPool<ImGuiTable>::"
            )
        )

    def function_id_with_c_name(self, name: str) -> str:
        matches = [
            identifier for identifier, candidate in self.function_names.items()
            if candidate == name and identifier in self.functions
        ]
        if len(matches) != 1:
            raise TranslationError(f"expected one function named {name}")
        return matches[0]

    def flattened_table_pool_dependencies(
        self, function: dict[str, Any]
    ) -> set[str] | None:
        if not self.is_flattened_table_pool_function(function):
            return None
        operation = self.function_names[function["id"]]
        pool = self.internal_namespace + "table_pool_"
        table_fini = self.internal_namespace + "table_fini"
        names: dict[str, tuple[str, ...]] = {
            pool + "fini": (pool + "clear",),
            pool + "clear": (
                table_fini, "imgui_storage_clear",
            ),
            pool + "find": ("imgui_storage_get_int",),
            pool + "get_or_add": (
                "imgui_storage_get_int_ref", pool + "add",
            ),
            pool + "remove": (
                pool + "index", pool + "remove_at",
            ),
            pool + "remove_at": (
                table_fini, "imgui_storage_set_int",
            ),
        }
        return {
            self.function_id_with_c_name(name)
            for name in names.get(operation, ())
        }

    def compact_flattened_table_pool_body(
        self, function: dict[str, Any]
    ) -> list[str] | None:
        dependencies = self.flattened_table_pool_dependencies(function)
        if dependencies is None:
            return None
        operation = self.function_names[function["id"]]
        pool = self.internal_namespace + "table_pool_"
        table_fini = self.internal_namespace + "table_fini"
        if operation == pool + "init":
            return ["    memset(self, 0, sizeof(*self));"]
        if operation == pool + "fini":
            return [f"    {pool}clear(imgui_c89_ctx, self);"]
        if operation == pool + "clear":
            value = "self->Map.Data.Data[n]." + self.table_pool_map_int_path
            return [
                "    int idx;",
                "    int n;",
                "",
                "    for (n = 0; n < self->Map.Data.Size; n++) {",
                f"        idx = {value};",
                "        if (idx != -1) {",
                f"            {table_fini}(imgui_c89_ctx, self->Data + idx);",
                "        }",
                "    }",
                "    imgui_storage_clear(imgui_c89_ctx, &self->Map);",
                "    imgui_c89_vector_clear(imgui_c89_ctx,",
                "        (void **)&self->Data, &self->Size, &self->Capacity);",
                "    self->FreeIdx = self->AliveCount = 0;",
            ]
        if operation == pool + "add":
            self.compact_table_pool_add_constructor(function)
            return [
                "    ImGuiTable *item;",
                "",
                "    item = (ImGuiTable *)imgui_c89_pool_add_slot(",
                "        imgui_c89_ctx, (void **)&self->Data,",
                "        &self->Size, &self->Capacity,",
                "        &self->FreeIdx, &self->AliveCount, sizeof(*item));",
                "    memset(item, 0, sizeof(*item));",
                "    item->LastFrameActive = -1;",
                "    return item;",
            ]
        if operation == pool + "find":
            return [
                "    int index;",
                "",
                "    index = imgui_storage_get_int(&self->Map, key, -1);",
                "    return index == -1 ? 0 : self->Data + index;",
            ]
        if operation == pool + "at":
            return ["    return self->Data + n;"]
        if operation == pool + "index":
            return [
                "    return (ImPoolIdx)imgui_c89_pool_index(",
                "        self->Data, self->Size, p, sizeof(*p));",
            ]
        if operation == pool + "get_or_add":
            return [
                "    int *index;",
                "",
                "    index = imgui_storage_get_int_ref(",
                "        imgui_c89_ctx, &self->Map, key, -1);",
                "    if (*index != -1) {",
                "        return self->Data + *index;",
                "    }",
                "    *index = self->FreeIdx;",
                f"    return {pool}add(imgui_c89_ctx, self);",
            ]
        if operation == pool + "remove":
            return [
                f"    {pool}remove_at(imgui_c89_ctx, self, key,",
                f"        {pool}index(self, p));",
            ]
        if operation == pool + "remove_at":
            return [
                f"    {table_fini}(imgui_c89_ctx, self->Data + idx);",
                "    *(int *)(void *)(self->Data + idx) = self->FreeIdx;",
                "    self->FreeIdx = idx;",
                "    imgui_storage_set_int(imgui_c89_ctx, &self->Map, key, -1);",
                "    self->AliveCount--;",
            ]
        if operation == pool + "alive_count":
            return ["    return self->AliveCount;"]
        if operation == pool + "map_size":
            return ["    return self->Map.Data.Size;"]
        if operation == pool + "map_at":
            value = "self->Map.Data.Data[n]." + self.table_pool_map_int_path
            return [
                "    int idx;",
                "",
                f"    idx = {value};",
                "    return idx == -1 ? 0 : self->Data + idx;",
            ]
        raise TranslationError(
            f"unsupported flattened table pool definition: {operation}"
        )

    def compact_impool_body(
        self, function: dict[str, Any]
    ) -> list[str] | None:
        if not self.compact_impool:
            return None
        flattened = self.compact_flattened_table_pool_body(function)
        if flattened is not None:
            return flattened
        qualified_name = function.get("qualified_name", "")
        match = re.fullmatch(
            r"ImPool<(ImGuiMultiSelectState|ImGuiTabBar|ImGuiTable)>::"
            r"(Add|Clear|Contains|GetByIndex|GetByKey|GetIndex|GetOrAddByKey)",
            qualified_name,
        )
        if not match:
            return None
        element = match.group(1)
        method = match.group(2)
        result_type = self.c_type(function.get("return_type", "void"))
        size = f"sizeof({element})"
        if method == "Clear":
            if element == "ImGuiTable":
                return None
            statements = function.get("body", {}).get("statements", [])
            expected = [
                "ForStmt", "CXXMemberCallExpr", "CXXMemberCallExpr",
                "BinaryOperator",
            ]
            if [item.get("kind") for item in statements] != expected:
                raise TranslationError(
                    f"compact ImPool<{element}>::Clear body changed shape"
                )
            loop_statements = statements[0].get("body", {}).get("statements", [])
            destructor = f"{element}::~{element}"
            if (len(loop_statements) != 2
                    or loop_statements[1].get("kind") != "IfStmt"
                    or loop_statements[1].get("then", {}).get(
                        "callee_qualified_name"
                    ) != destructor
                    or any(item.get("qualified_name") == destructor
                           for item in self.functions.values())):
                raise TranslationError(
                    f"compact ImPool<{element}>::Clear destructor is not trivial"
                )
            return [
                "    imgui_c89_vector_clear(imgui_c89_ctx,",
                "        (void **)&self->Map.Data.Data, &self->Map.Data.Size,",
                "        &self->Map.Data.Capacity);",
                "    imgui_c89_vector_clear(imgui_c89_ctx, (void **)&self->Buf.Data,",
                "        &self->Buf.Size, &self->Buf.Capacity);",
                "    self->FreeIdx = self->AliveCount = 0;",
            ]
        if method == "Add":
            table_constructor = self.compact_table_pool_add_constructor(function)
            if table_constructor is not None:
                return [
                    "    ImGuiTable *item;",
                    "",
                    "    item = (ImGuiTable *)imgui_c89_pool_add_slot(",
                    "        imgui_c89_ctx, (void **)&self->Buf.Data,",
                    "        &self->Buf.Size, &self->Buf.Capacity,",
                    "        &self->FreeIdx, &self->AliveCount, sizeof(*item));",
                    "    memset(item, 0, sizeof(*item));",
                    "    item->LastFrameActive = -1;",
                    "    return item;",
                ]
            constructors = [
                identifier for identifier, candidate in self.functions.items()
                if candidate.get("qualified_name") == f"{element}::{element}"
                and candidate.get("constructor")
                and not candidate.get("parameters")
            ]
            if len(constructors) != 1:
                raise TranslationError(
                    f"compact ImPool expected one {element} constructor"
                )
            constructor_id = constructors[0]
            constructor = self.constructor_at_helpers[constructor_id]
            arguments = ["item"]
            if constructor_id in self.context_threaded_functions:
                arguments.insert(0, "imgui_c89_ctx")
            return [
                f"    {element} *item;",
                "",
                "    item = ({0} *)imgui_c89_pool_add_slot(".format(element),
                "        imgui_c89_ctx, (void **)&self->Buf.Data,",
                "        &self->Buf.Size, &self->Buf.Capacity,",
                f"        &self->FreeIdx, &self->AliveCount, {size});",
                f"    {constructor}({', '.join(arguments)});",
                "    return item;",
            ]
        if method == "Contains":
            return [
                "    return imgui_c89_pool_contains(",
                f"        self->Buf.Data, self->Buf.Size, p, {size});",
            ]
        if method == "GetByIndex":
            return [
                f"    return ({result_type})imgui_c89_pool_at(",
                f"        self->Buf.Data, n, {size});",
            ]
        if method == "GetIndex":
            return [
                "    return (ImPoolIdx)imgui_c89_pool_index(",
                f"        self->Buf.Data, self->Buf.Size, p, {size});",
            ]
        storage_name = "GetInt" if method == "GetByKey" else "GetIntRef"
        storage_functions = [
            (identifier, candidate) for identifier, candidate in self.functions.items()
            if candidate.get("qualified_name")
            == f"ImGuiStorage::{storage_name}"
        ]
        if len(storage_functions) != 1:
            raise TranslationError(
                f"compact ImPool expected ImGuiStorage::{storage_name}"
            )
        storage_id = storage_functions[0][0]
        storage_call = self.function_names[storage_id]
        if method == "GetByKey":
            return [
                "    int index;",
                "",
                f"    index = {storage_call}(&self->Map, key, -1);",
                f"    return index == -1 ? 0 : ({result_type})imgui_c89_pool_at(",
                f"        self->Buf.Data, index, {size});",
            ]
        add_functions = [
            (identifier, candidate) for identifier, candidate in self.functions.items()
            if candidate.get("parent") == function.get("parent")
            and candidate.get("name") == "Add"
        ]
        if len(add_functions) != 1:
            raise TranslationError(f"compact ImPool expected one {element} Add")
        add_id = add_functions[0][0]
        add_call = self.function_names[add_id]
        storage_args = ["&self->Map", "key", "-1"]
        if storage_id in self.context_threaded_functions:
            storage_args.insert(0, "imgui_c89_ctx")
        add_args = ["self"]
        if add_id in self.context_threaded_functions:
            add_args.insert(0, "imgui_c89_ctx")
        return [
            "    int *index;",
            "",
            f"    index = {storage_call}({', '.join(storage_args)});",
            "    if (*index != -1) {",
            f"        return ({result_type})imgui_c89_pool_at(",
            f"            self->Buf.Data, *index, {size});",
            "    }",
            "    *index = self->FreeIdx;",
            f"    return {add_call}({', '.join(add_args)});",
        ]

    def compact_checkbox_flags_body(
        self, function: dict[str, Any]
    ) -> list[str] | None:
        if not self.compact_checkbox_flags:
            return None
        qualified_name = function.get("qualified_name", "")
        if qualified_name not in {
            "ImGui::CheckboxFlags", "ImGui::CheckboxFlagsT"
        }:
            return None
        parameters = function.get("parameters", [])
        if ([item.get("name") for item in parameters]
                != ["label", "flags", "flags_value"]
                or function.get("return_type") != "bool"):
            raise TranslationError(
                f"compact {qualified_name} signature changed"
            )
        flag_type = parameters[1].get("canonical_type", "")
        widths = {
            "int *": 32,
            "unsigned int *": 32,
            "long long *": 64,
            "unsigned long long *": 64,
        }
        width = widths.get(flag_type)
        if width is None:
            return None
        statements = function.get("body", {}).get("statements", [])
        if qualified_name == "ImGui::CheckboxFlags":
            if (len(statements) != 1
                    or statements[0].get("kind") != "ReturnStmt"
                    or statements[0].get("value", {}).get(
                        "callee_qualified_name"
                    ) != "ImGui::CheckboxFlagsT"):
                raise TranslationError(
                    "compact ImGui::CheckboxFlags body changed shape"
                )
        elif [item.get("kind") for item in statements] != [
            "DeclStmt", "DeclStmt", "DeclStmt", "IfStmt", "IfStmt",
            "ReturnStmt",
        ]:
            raise TranslationError(
                "compact ImGui::CheckboxFlagsT body changed shape"
            )

        helper_id = self.compact_checkbox_flag_helpers[width]
        helper = self.functions[helper_id]
        helper_parameters = helper.get("parameters", [])
        pointer_type = self.c_type(helper_parameters[1]["type"])
        value_type = self.c_type(helper_parameters[2]["type"])
        helper_call = self.function_names[helper_id]
        if function.get("id") != helper_id:
            return [
                f"    return {helper_call}(imgui_c89_ctx, label,",
                f"        ({pointer_type})flags, ({value_type})flags_value);",
            ]

        checkbox_ids = [
            identifier
            for identifier, candidate in self.functions.items()
            if candidate.get("qualified_name") == "ImGui::Checkbox"
        ]
        mixed_ids = [
            constant["id"]
            for enum in self.ir.get("enums", [])
            for constant in enum.get("constants", [])
            if constant.get("name") == "ImGuiItemFlags_MixedValue"
        ]
        if len(checkbox_ids) != 1 or len(mixed_ids) != 1:
            raise TranslationError(
                "compact CheckboxFlags dependencies changed shape"
            )
        checkbox = self.function_names[checkbox_ids[0]]
        mixed = self.enum_constant_names[mixed_ids[0]]
        return [
            f"    {value_type} bits;",
            "    unsigned char all_on;",
            "    unsigned char any_on;",
            "    unsigned char pressed;",
            "",
            "    memcpy(&bits, flags, sizeof(bits));",
            "    all_on = (bits & flags_value) == flags_value;",
            "    any_on = (bits & flags_value) != 0;",
            "    if (!all_on && any_on)",
            f"        imgui_c89_ctx->NextItemData.ItemFlagsSet |= {mixed};",
            f"    pressed = {checkbox}(imgui_c89_ctx, label, &all_on);",
            "    if (pressed) {",
            "        bits = all_on ? bits | flags_value : bits & ~flags_value;",
            "        memcpy(flags, &bits, sizeof(bits));",
            "    }",
            "    return pressed;",
        ]

    def compact_input_source_name_body(
        self, function: dict[str, Any]
    ) -> list[str] | None:
        if not self.compact_input_source_names:
            return None
        qualified_name = function.get("qualified_name", "")
        tables = {
            "GetInputSourceName": [
                "None", "Mouse", "Keyboard", "Gamepad", "Unknown"
            ],
            "GetMouseSourceName": [
                "Mouse", "TouchScreen", "Pen", "Unknown"
            ],
        }
        if qualified_name not in tables:
            return None
        values = tables[qualified_name]
        source = "\n".join(self.statement(function.get("body", {})))
        for value in values:
            if source.count(self._string_literal(value)) != 1:
                raise TranslationError(
                    f"compact {qualified_name} string table changed shape"
                )
        parameter = function.get("parameters", [])
        if (len(parameter) != 1
                or parameter[0].get("name") != "source"
                or function.get("return_type") != "const char *"):
            raise TranslationError(
                f"compact {qualified_name} signature changed shape"
            )
        blob = ""
        offsets: list[int] = []
        for value in values:
            offsets.append(len(blob))
            blob += value + "\0"
        count = len(values) - 1
        return [
            f"    static const char names[] = {self._string_literal(blob)};",
            "    static const unsigned char offsets[] = { "
            + ", ".join(str(value) for value in offsets) + " };",
            "",
            f"    if ((unsigned int)source >= {count}) source = {count};",
            "    return names + offsets[source];",
        ]

    def compact_cff_stack_guard_body(
        self, function: dict[str, Any]
    ) -> tuple[list[str], list[str]] | None:
        """Coalesce Type2's repeated outer-op stack checks into one table.

        The escaped flex operators retain their independent b1 checks.  This
        deliberately transforms the emitted strict-C89 shape only after the
        ordinary AST lowering, and rejects upstream shapes it cannot prove.
        """
        if (not self.compact_cff_stack_guards
                or function.get("qualified_name") != "stbtt__run_charstring"):
            return None
        body = self.statement(function["body"])
        result: list[str] = []
        removed: list[int] = []
        inserted = False
        escaped = False
        index = 0
        pattern = re.compile(r"\s*if \((?:\()?sp < (\d+)(?:\))?\) \{")
        while index < len(body):
            line = body[index]
            if line.strip() == "case 12:":
                escaped = True
            match = pattern.fullmatch(line)
            if (not escaped and match is not None and index + 2 < len(body)
                    and body[index + 1].strip() == "return 0;"
                    and body[index + 2].strip() == "}"):
                removed.append(int(match.group(1)))
                index += 3
                continue
            result.append(line)
            if (not inserted and "b0 = " in line
                    and "stbtt__buf_get8" in line):
                result.append(
                    "        if (b0 < 32 && sp < imgui_c89_cff_min_stack[b0]) return 0;"
                )
                inserted = True
            index += 1
        expected = [2, 1, 1, 2, 1, 1, 4, 4, 6, 8, 8, 4, 1]
        if removed != expected or not inserted:
            raise TranslationError(
                "compact CFF stack guards encountered an unexpected "
                f"charstring shape: checks={removed!r}, inserted={inserted}"
            )
        locals_ = [
            "    static const unsigned char imgui_c89_cff_min_stack[32] = {",
            "        0,0,0,0,1,2,1,1,6,0,1,0,0,0,0,0,",
            "        0,0,0,0,0,2,1,0,8,8,4,4,0,1,4,4",
            "    };",
        ]
        return locals_, result

    @staticmethod
    def function_body_sha256(function: dict[str, Any]) -> str:
        body_json = json.dumps(
            function.get("body", {}), sort_keys=True, separators=(",", ":")
        )
        return hashlib.sha256(body_json.encode("utf-8")).hexdigest()

    def handwritten_specification(
        self, function: dict[str, Any]
    ) -> dict[str, Any] | None:
        """Return the unique signature/body-guarded overlay specification."""
        qualified_name = function.get("qualified_name", "")
        specification = self.handwritten_functions.get(qualified_name)
        if specification is None:
            return None
        actual_parameter_types = [
            parameter.get("type")
            for parameter in function.get("parameters", [])
        ]
        actual = self.function_body_sha256(function)
        if isinstance(specification, list):
            def body_hash_matches(item: dict[str, Any]) -> bool:
                expected = item.get("body_sha256")
                if expected is None:
                    return True
                values = expected if isinstance(expected, list) else [expected]
                return actual in values

            matches = [
                item for item in specification
                if isinstance(item, dict)
                and item.get("parameter_types") == actual_parameter_types
                and body_hash_matches(item)
            ]
            if len(matches) != 1:
                raise TranslationError(
                    f"handwritten overload set for {qualified_name} expected "
                    f"one match for {actual_parameter_types}, got {len(matches)}"
                )
            specification = matches[0]
        if not isinstance(specification, dict):
            raise TranslationError(
                f"handwritten replacement for {qualified_name} must be an object"
            )
        group_name = specification.get("group")
        if group_name is not None:
            group = self.handwritten_groups.get(group_name)
            if not isinstance(group_name, str) or group is None:
                raise TranslationError(
                    f"handwritten replacement for {qualified_name} has an "
                    f"unknown group {group_name!r}"
                )
            if "group" in group:
                raise TranslationError("handwritten groups cannot inherit groups")
            merged = dict(group)
            mapping_fields = {
                "constants", "functions", "snippets", "snippets_by_body_sha256",
                "types", "value_functions",
            }
            for key, value in specification.items():
                if (key in mapping_fields and key in merged
                        and isinstance(merged[key], dict)
                        and isinstance(value, dict)):
                    merged[key] = {**merged[key], **value}
                else:
                    merged[key] = value
            specification = merged
        parameter_types = specification.get("parameter_types")
        if (parameter_types is not None
                and actual_parameter_types != parameter_types):
            return None
        expected_value = specification.get("body_sha256")
        expected = (
            set(expected_value) if isinstance(expected_value, list)
            else {expected_value}
        )
        if actual not in expected:
            raise TranslationError(
                f"handwritten replacement body changed for {qualified_name}: "
                f"expected {sorted(str(item) for item in expected)}, got {actual}"
            )
        return specification

    def stable_handwritten_function_names(
        self, excluded_ids: set[str]
    ) -> dict[str, str]:
        """Assign readable deterministic names to overlay-owned definitions."""
        candidates: list[tuple[str, str, dict[str, Any]]] = []
        for identifier, function in sorted(self.functions.items()):
            if identifier in excluded_ids:
                continue
            if self.handwritten_specification(function) is None:
                continue
            operation = self.native_snake_name(
                self.stable_operation_name(function)
            ) or "function"
            owner_token = ""
            if function.get("method") and function.get("parent") in self.records:
                owner = self.records[function["parent"]].get("name", "record")
                owner_token = self.native_snake_name(owner) or "record"
                if owner_token.startswith("im_gui_"):
                    owner_token = owner_token[len("im_gui_"):]
            else:
                qualified = function.get("qualified_name", operation)
                owner, separator, _ = qualified.rpartition("::")
                if separator:
                    owner_token = self.native_snake_name(owner)
                    if owner_token == "im_gui":
                        owner_token = ""
                elif operation.startswith("im_"):
                    # Dear ImGui's C-style private helpers carry an `Im`
                    # prefix that is useful in the C++ source but redundant
                    # below our private C namespace.
                    operation = operation[len("im_"):]
            base = self.internal_namespace
            if owner_token:
                base += owner_token + "_"
            base += operation
            candidates.append((base, identifier, function))

        grouped: dict[str, list[tuple[str, dict[str, Any]]]] = {}
        for base, identifier, function in candidates:
            grouped.setdefault(base, []).append((identifier, function))
        result: dict[str, str] = {}
        used: dict[str, str] = {}
        for base, group in sorted(grouped.items()):
            for identifier, function in sorted(group):
                name = base
                if len(group) > 1:
                    tokens = [
                        self.native_type_token(parameter["type"])
                        for parameter in function.get("parameters", [])
                    ]
                    if function.get("const"):
                        tokens.append("const")
                    if function.get("variadic"):
                        tokens.append("varargs")
                    name += "_" + "_".join(tokens or ["void"])
                previous = used.get(name)
                if previous is not None and previous != identifier:
                    raise TranslationError(
                        f"stable handwritten overload collision: {name}"
                    )
                used[name] = identifier
                result[identifier] = name
        return result

    def stable_table_pool_function_names(self) -> dict[str, str]:
        """Name the flattened table pool as a small idiomatic C module."""
        operations = {
            ("Add", ()): "add",
            ("Clear", ()): "clear",
            ("Contains", ("const ImGuiTable *",)): "contains",
            ("GetAliveCount", ()): "alive_count",
            ("GetBufSize", ()): "size",
            ("GetByIndex", ("ImPoolIdx",)): "at",
            ("GetByKey", ("ImGuiID",)): "find",
            ("GetIndex", ("const ImGuiTable *",)): "index",
            ("GetMapSize", ()): "map_size",
            ("GetOrAddByKey", ("ImGuiID",)): "get_or_add",
            ("ImPool", ()): "init",
            ("Remove", ("ImGuiID", "const ImGuiTable *")): "remove",
            ("Remove", ("ImGuiID", "ImPoolIdx")): "remove_at",
            ("Reserve", ("int",)): "reserve",
            ("TryGetMapData", ("ImPoolIdx",)): "map_at",
            ("~ImPool", ()): "fini",
        }
        result: dict[str, str] = {}
        seen: set[tuple[str, tuple[str, ...]]] = set()
        for identifier, function in self.function_declarations.items():
            if not function.get("qualified_name", "").startswith(
                "ImPool<ImGuiTable>::"
            ):
                continue
            key = (
                function.get("name", ""),
                tuple(
                    parameter.get("type", "")
                    for parameter in function.get("parameters", [])
                ),
            )
            operation = operations.get(key)
            if operation is None:
                raise TranslationError(
                    "flatten table pool gained an unknown operation: "
                    + function.get("qualified_name", "")
                )
            result[identifier] = (
                self.internal_namespace + "table_pool_" + operation
            )
            seen.add(key)
        missing = set(operations) - seen
        if missing:
            raise TranslationError(
                "flatten table pool lost operations: "
                + ", ".join(
                    name + str(parameters)
                    for name, parameters in sorted(missing)
                )
            )
        table_destructors = [
            (identifier, function)
            for identifier, function in self.function_declarations.items()
            if function.get("qualified_name") == "ImGuiTable::~ImGuiTable"
        ]
        if len(table_destructors) != 1:
            raise TranslationError(
                "flatten table pool expected one ImGuiTable destructor"
            )
        result[table_destructors[0][0]] = (
            self.internal_namespace + "table_fini"
        )
        self.validate_table_pool_fingerprints()
        return result

    @staticmethod
    def table_pool_fingerprint_variants(
    ) -> dict[tuple[str, tuple[str, ...]], set[str]]:
        """Return the exact accepted AST bodies for table-pool operations."""
        return {
            ("Add", ()): {
                "e4512c8cc8437efafd977b8ccbe0a2b8559cb8415c4715b36b41e7d13dd86792",
            },
            ("Clear", ()): {
                "71bcd63005704cbb2333db6d80b1d4c18b2cf6dcc1058fce78886de269923fec",
            },
            ("GetAliveCount", ()): {
                "421ac9b2db78fda7a09b23e91304ee780c878e1c94987fe3680bd635c7ceaa6c",
            },
            ("GetByIndex", ("ImPoolIdx",)): {
                "00cde8c09aaacf6fb79b7b66c2ebc088771d2623a4b0e4760319ad9a3db65440",
            },
            ("GetByKey", ("ImGuiID",)): {
                "cb85573afd58dbcac0ea7d6381180c8c38dc167d0ed7aa6280be5f7c29bcde54",
            },
            ("GetIndex", ("const ImGuiTable *",)): {
                # The Test Engine replaces IM_ASSERT with an assert-log and
                # debug-trap sequence; the predicate and return are unchanged.
                "1087de96a2483f5891e1795751b1fa56cf71c8c626019a3753e822a9c12b1e19",
                "b55ba7fdc5908b3033e057cd4119baabb381146be5e9826103d5fc1f9f00fa50",
            },
            ("GetMapSize", ()): {
                "df5a136d0f4f29faecd7fd76ecc86ac2966b6941a6152f2873f3ed969274dd92",
            },
            ("GetOrAddByKey", ("ImGuiID",)): {
                "8f9d4e2e97dbb9a09f5fd91cde57d398a0ebca7f5316cd025f3d4a10fad2e559",
            },
            ("ImPool", ()): {
                "8336a30da2b2b01c791cc216e77080a06f92e516d7dbde947a69fe51a32f6509",
            },
            ("Remove", ("ImGuiID", "const ImGuiTable *")): {
                "e85392fcfaf0069d15c22a006f44ea290b629d57a6d5e8a18ea68c1c216c87d1",
            },
            ("Remove", ("ImGuiID", "ImPoolIdx")): {
                "dff6374bf31dc83be9152290ee64500d89ab253d1d1f332e299d7519d71d386f",
            },
            ("TryGetMapData", ("ImPoolIdx",)): {
                "1c433700c423cda80ce275d55b77a069a66849655203c6de0c234820479dee0b",
            },
            ("~ImPool", ()): {
                "16117568484ae3e1a7cad4c8ac913ea68273a31a70819b96e0250726a7e34546",
            },
        }

    def validate_table_pool_fingerprints(self) -> None:
        """Fail closed when an upstream pool operation changes semantics."""
        expected = self.table_pool_fingerprint_variants()
        actual: dict[tuple[str, tuple[str, ...]], str] = {}
        for function in self.functions.values():
            if not function.get("qualified_name", "").startswith(
                "ImPool<ImGuiTable>::"
            ):
                continue
            key = (
                function.get("name", ""),
                tuple(
                    parameter.get("type", "")
                    for parameter in function.get("parameters", [])
                ),
            )
            actual[key] = self.function_body_sha256(function)
        changed = sorted(
            key for key in set(actual) | set(expected)
            if key not in actual or key not in expected
            or actual[key] not in expected[key]
        )
        if changed:
            raise TranslationError(
                "flatten table pool method fingerprints changed: "
                + ", ".join(name + str(parameters) for name, parameters in changed)
            )
        destructor = next(
            function for function in self.functions.values()
            if function.get("qualified_name") == "ImGuiTable::~ImGuiTable"
        )
        if self.function_body_sha256(destructor) != (
            "cf849c76c22272bfb8a8595e5bbce62c9302e3aab4720149dc08c19066bd21b2"
        ):
            raise TranslationError("ImGuiTable destructor fingerprint changed")

    def handwritten_function_body(
        self, function: dict[str, Any], *, resolve_asserts: bool = True
    ) -> list[str] | None:
        """Load a signature/body-guarded independent C89 implementation."""
        qualified_name = function.get("qualified_name", "")
        specification = self.handwritten_specification(function)
        if specification is None:
            return None
        actual = self.function_body_sha256(function)
        relative = specification.get("template")
        if not isinstance(relative, str):
            raise TranslationError(
                f"handwritten replacement for {qualified_name} has no template"
            )
        translator_root = Path(__file__).resolve().parents[2]
        template = (translator_root / relative).resolve()
        handwritten_root = (translator_root / "handwritten").resolve()
        if handwritten_root not in template.parents:
            raise TranslationError(
                f"handwritten replacement template escapes its directory: {relative}"
            )
        try:
            source = template.read_text(encoding="utf-8")
        except OSError as error:
            raise TranslationError(
                f"cannot read handwritten replacement {relative}: {error}"
            ) from error
        fragment = specification.get("fragment")
        if fragment is not None:
            if not isinstance(fragment, str) or not fragment:
                raise TranslationError(
                    f"handwritten replacement fragment for {qualified_name} "
                    "must be a non-empty string"
                )
            begin = f"/* IMGUI_C89_FUNCTION_BEGIN {fragment} */"
            end = f"/* IMGUI_C89_FUNCTION_END {fragment} */"
            if source.count(begin) != 1 or source.count(end) != 1:
                raise TranslationError(
                    f"handwritten replacement {qualified_name} cannot find "
                    f"unique fragment {fragment} in {relative}"
                )
            source = source.split(begin, 1)[1].split(end, 1)[0]

        def resolve_function_identifier(requirement: Any) -> str:
            if not isinstance(requirement, dict):
                raise TranslationError("handwritten function requirement is invalid")
            name = requirement.get("qualified_name")
            parameter_types = requirement.get("parameter_types")
            method_const = requirement.get("method_const")
            if method_const is not None and not isinstance(method_const, bool):
                raise TranslationError(
                    "handwritten function requirement method_const is invalid"
                )
            matches = [
                identifier
                for identifier, declaration in self.function_declarations.items()
                if declaration.get("qualified_name") == name
                and (parameter_types is None or [
                    parameter.get("type")
                    for parameter in declaration.get("parameters", [])
                ] == parameter_types)
                and (
                    method_const is None
                    or declaration.get("const", False) == method_const
                )
            ]
            if len(set(matches)) != 1:
                raise TranslationError(
                    f"handwritten {qualified_name} cannot resolve function {name}"
                )
            return matches[0]

        def resolve_value_function(requirement: Any) -> str:
            identifier = resolve_function_identifier(requirement)
            try:
                return self.constructor_helpers[identifier]
            except KeyError as error:
                raise TranslationError(
                    f"handwritten {qualified_name} requirement is not a "
                    "value constructor"
                ) from error

        def resolve_constant(name: Any) -> str:
            matches = [
                self.enum_constant_names[constant["id"]]
                for enum in self.ir.get("enums", [])
                for constant in enum.get("constants", [])
                if constant.get("name") == name
            ]
            if len(set(matches)) != 1:
                raise TranslationError(
                    f"handwritten {qualified_name} cannot resolve constant {name}"
                )
            return matches[0]

        snippets = dict(specification.get("snippets", {}))
        snippets.update(
            specification.get("snippets_by_body_sha256", {}).get(actual, {})
        )
        dependencies: set[str] = set()
        constructor_value_dependencies: set[str] = set()
        for token, snippet in snippets.items():
            if not isinstance(snippet, str):
                raise TranslationError("handwritten snippet is invalid")
            source = source.replace(token, snippet)
        for token, requirement in specification.get("functions", {}).items():
            if token in source:
                dependency = resolve_function_identifier(requirement)
                dependencies.add(dependency)
                source = source.replace(token, self.function_names[dependency])
        for token, requirement in specification.get("value_functions", {}).items():
            if token in source:
                dependency = resolve_function_identifier(requirement)
                dependencies.add(dependency)
                constructor_value_dependencies.add(dependency)
                source = source.replace(token, resolve_value_function(requirement))
        for token, spelling in specification.get("types", {}).items():
            if not isinstance(spelling, str):
                raise TranslationError("handwritten type requirement is invalid")
            if token in source:
                source = source.replace(token, self.c_type(spelling))
        for token, name in specification.get("constants", {}).items():
            if token in source:
                source = source.replace(token, resolve_constant(name))
        # Snippets may refer to an already resolved generated function name.
        # Recognize those references as well, while requiring every generated
        # function name to remain globally unique.
        names_to_ids = {
            name: identifier for identifier, name in self.function_names.items()
        }
        for name in set(re.findall(r"\b[A-Za-z_][A-Za-z0-9_]*\b", source)):
            dependency = names_to_ids.get(name)
            if dependency is not None:
                dependencies.add(dependency)
        identifier = function.get("id")
        if identifier in self.functions:
            dependencies.discard(identifier)
            self.handwritten_function_dependencies[identifier] = dependencies
            self.handwritten_constructor_value_dependencies[identifier] = (
                constructor_value_dependencies
            )

        assertion_pattern = re.compile(r"@ASSERT_(\d+)@")
        if resolve_asserts:
            source_path = Path(
                function.get("location", {}).get("file", "")
            )
            source_file = source_path.name
            try:
                source_lines = source_path.read_text(
                    encoding="utf-8"
                ).splitlines()
            except OSError:
                source_lines = []

            def replace_assertion(match: re.Match[str]) -> str:
                line = int(match.group(1), 10)
                matches = [
                    assertion_id
                    for (file_name, record_line, _), assertion_id
                    in self.compact_assert_ids.items()
                    if Path(file_name).name == source_file
                    and record_line == line
                ]
                if len(matches) == 1:
                    return f"imgui_c89_assert_id({matches[0]})"
                if not matches and self.imvector_assert_backend == (
                    "ImGuiTestEngine_AssertLog"
                ):
                    expression = (
                        source_lines[line - 1].strip()
                        if 0 < line <= len(source_lines)
                        else f"assertion at line {line}"
                    )
                    return (
                        "do { "
                        "imgui_c89_external_ImGuiTestEngine_AssertLog("
                        f"{self._string_literal(expression)}, "
                        f"{self._string_literal(str(source_path))}, "
                        f"{self._string_literal(function.get('name', ''))}, "
                        f"{line}); imgui_c89_debugtrap(); "
                        "} while (0)"
                    )
                if len(matches) != 1:
                    condition = "missing" if not matches else "ambiguous"
                    raise TranslationError(
                        f"handwritten {qualified_name} has {condition} "
                        f"assertion metadata for {source_file}:{line}"
                    )
                raise AssertionError("unreachable assertion resolution")

            source = assertion_pattern.sub(replace_assertion, source)
        unresolved = sorted(set(re.findall(r"@[A-Z0-9_]+@", source)))
        if not resolve_asserts:
            unresolved = [
                token for token in unresolved
                if assertion_pattern.fullmatch(token) is None
            ]
        if unresolved:
            raise TranslationError(
                f"handwritten {qualified_name} has unresolved tokens: "
                + ", ".join(unresolved)
            )
        source = textwrap.dedent(source)
        lines = source.splitlines()
        while lines and not lines[0].strip():
            lines.pop(0)
        while lines and not lines[-1].strip():
            lines.pop()
        return [("    " + line if line else "") for line in lines]

    def prime_handwritten_function_dependencies(self) -> None:
        """Resolve replacement call edges before linkage is classified.

        A handwritten body replaces its upstream body completely.  Linkage and
        constructor-adapter analysis therefore must not see calls that exist
        only in the discarded C++ AST.  Resolve every guarded template once up
        front; ordinary emission will resolve it again with assertion metadata.
        """
        previous_function_id = self.current_function_id
        try:
            for identifier, function in sorted(self.functions.items()):
                if self.handwritten_specification(function) is None:
                    continue
                self.current_function_id = identifier
                body = self.handwritten_function_body(
                    function, resolve_asserts=False
                )
                if body is None:
                    raise TranslationError(
                        "matched handwritten function lost its replacement "
                        "body: "
                        + function.get("qualified_name", identifier)
                    )
        finally:
            self.current_function_id = previous_function_id

    def compact_nav_overlay_setup_body(
        self, function: dict[str, Any]
    ) -> tuple[list[str], list[str]] | None:
        """Lower the fixed Ctrl+Tab overlay setup without generic style APIs.

        The native style stack entry is preserved so Begin hooks and recovery
        code observe the same state.  Only the constant next-window setters
        and the known WindowPadding push/pop are specialized.
        """
        if (not self.compact_nav_overlay_selectable
                or function.get("qualified_name")
                != "ImGui::NavUpdateWindowingOverlay"):
            return None

        def function_name(qualified: str) -> str:
            matches = [
                self.function_names[identifier]
                for identifier, declaration
                in self.function_declarations.items()
                if declaration.get("qualified_name") == qualified
            ]
            if len(set(matches)) != 1:
                raise TranslationError(
                    f"compact nav overlay expected one {qualified}"
                )
            return matches[0]

        def constant_name(spelling: str) -> str:
            matches = [
                self.enum_constant_names[constant["id"]]
                for enum in self.ir.get("enums", [])
                for constant in enum.get("constants", [])
                if constant.get("name") == spelling
            ]
            if len(set(matches)) != 1:
                raise TranslationError(
                    f"compact nav overlay expected one {spelling}"
                )
            return matches[0]

        set_constraints = function_name("ImGui::SetNextWindowSizeConstraints")
        set_position = function_name("ImGui::SetNextWindowPos")
        push_style = next(
            self.function_names[identifier]
            for identifier, declaration in self.function_declarations.items()
            if declaration.get("qualified_name") == "ImGui::PushStyleVar"
            and [p.get("type") for p in declaration.get("parameters", [])]
            == ["ImGuiStyleVar", "const ImVec2 &"]
        )
        pop_style = function_name("ImGui::PopStyleVar")
        style_constructor = next(
            self.constructor_helpers[identifier]
            for identifier, declaration in self.function_declarations.items()
            if declaration.get("constructor")
            and declaration.get("qualified_name") == "ImGuiStyleMod::ImGuiStyleMod"
            and [p.get("type") for p in declaration.get("parameters", [])]
            == ["ImGuiStyleVar", "ImVec2"]
        )
        has_size = constant_name("ImGuiNextWindowDataFlags_HasSizeConstraint")
        has_pos = constant_name("ImGuiNextWindowDataFlags_HasPos")
        always = constant_name("ImGuiCond_Always")
        window_padding = constant_name("ImGuiStyleVar_WindowPadding")

        source = self.statement(function["body"])
        result: list[str] = []
        replaced = {"constraints": 0, "position": 0, "push": 0, "pop": 0}
        for line in source:
            if f"{set_constraints}(" in line:
                replaced["constraints"] += 1
                result.extend((
                    f"    (g->NextWindowData.HasFlags |= {has_size});",
                    "    g->NextWindowData.SizeConstraintRect.Min.x = viewport->Size.x * 0.200000003f;",
                    "    g->NextWindowData.SizeConstraintRect.Min.y = viewport->Size.y * 0.200000003f;",
                    "    g->NextWindowData.SizeConstraintRect.Max.x = 3.40282347E+38f;",
                    "    g->NextWindowData.SizeConstraintRect.Max.y = 3.40282347E+38f;",
                    "    g->NextWindowData.SizeCallback = 0;",
                    "    g->NextWindowData.SizeCallbackUserData = 0;",
                ))
            elif f"{set_position}(" in line:
                replaced["position"] += 1
                result.extend((
                    f"    (g->NextWindowData.HasFlags |= {has_pos});",
                    "    g->NextWindowData.PosVal.x = viewport->Pos.x + viewport->Size.x * 0.5f;",
                    "    g->NextWindowData.PosVal.y = viewport->Pos.y + viewport->Size.y * 0.5f;",
                    "    g->NextWindowData.PosPivotVal.x = 0.5f;",
                    "    g->NextWindowData.PosPivotVal.y = 0.5f;",
                    f"    g->NextWindowData.PosCond = {always};",
                ))
            elif f"{push_style}(" in line:
                replaced["push"] += 1
                result.extend((
                    "    backup_window_padding = g->Style.WindowPadding;",
                    f"    compact_style_mod = {style_constructor}({window_padding}, backup_window_padding);",
                    "    g->StyleVarStack.Data = imgui_c89_vector_push_back(imgui_c89_ctx, g->StyleVarStack.Data, &g->StyleVarStack.Size, &g->StyleVarStack.Capacity, sizeof(compact_style_mod), &compact_style_mod);",
                    "    g->Style.WindowPadding.x *= 2.0f;",
                    "    g->Style.WindowPadding.y *= 2.0f;",
                ))
            elif f"{pop_style}(" in line:
                replaced["pop"] += 1
                result.extend((
                    "    g->Style.WindowPadding = backup_window_padding;",
                    "    g->StyleVarStack.Size--;",
                ))
            else:
                result.append(line)
        if any(count != 1 for count in replaced.values()):
            raise TranslationError(
                "compact nav overlay setup encountered an unexpected call "
                f"shape: {replaced!r}"
            )
        return [
            "    ImVec2 backup_window_padding;",
            "    ImGuiStyleMod compact_style_mod;",
        ], result

    def compact_nav_direction_loop_body(
        self, function: dict[str, Any]
    ) -> tuple[list[str], list[str]] | None:
        """Fold the four directional move-key tests into checked enum ranges.

        Dear ImGui intentionally evaluates Left, Right, Up, then Down, with a
        later pressed direction replacing an earlier one.  The three enums
        involved are contiguous in that same order, so one loop preserves the
        behavior while avoiding four copies of the key-routing path.
        """
        if (not self.compact_nav_key_ranges
                or function.get("qualified_name")
                != "ImGui::NavUpdateCreateMoveRequest"):
            return None

        def function_name(
            qualified: str, parameter_types: list[str]
        ) -> str:
            matches = [
                self.function_names[identifier]
                for identifier, declaration
                in self.function_declarations.items()
                if declaration.get("qualified_name") == qualified
                and [p.get("type") for p in declaration.get("parameters", [])]
                == parameter_types
            ]
            if len(set(matches)) != 1:
                raise TranslationError(
                    f"compact nav direction loop expected one {qualified}"
                )
            return matches[0]

        def constant(spelling: str) -> tuple[str, int]:
            matches = [
                (self.enum_constant_names[item["id"]], int(item["value"]))
                for enum in self.ir.get("enums", [])
                for item in enum.get("constants", [])
                if item.get("name") == spelling
            ]
            if len(set(matches)) != 1:
                raise TranslationError(
                    f"compact nav direction loop expected one {spelling}"
                )
            return matches[0]

        is_active = function_name(
            "ImGui::IsActiveIdUsingNavDir", ["ImGuiDir"]
        )
        is_pressed = function_name(
            "ImGui::IsKeyPressed",
            ["ImGuiKey", "ImGuiInputFlags", "ImGuiID"],
        )
        directions = [constant("ImGuiDir_" + suffix)
                      for suffix in ("Left", "Right", "Up", "Down")]
        keyboards = [constant("ImGuiKey_" + suffix + "Arrow")
                     for suffix in ("Left", "Right", "Up", "Down")]
        gamepads = [constant("ImGuiKey_GamepadDpad" + suffix)
                    for suffix in ("Left", "Right", "Up", "Down")]
        for values, label in (
            (directions, "directions"),
            (keyboards, "keyboard keys"),
            (gamepads, "gamepad keys"),
        ):
            if [value for _, value in values] != list(
                range(values[0][1], values[0][1] + 4)
            ):
                raise TranslationError(
                    f"compact nav {label} are no longer contiguous"
                )

        source = self.statement(function["body"])
        indices = [
            index for index, line in enumerate(source)
            if f"{is_active}(" in line and f"{is_pressed}(" in line
        ]
        if len(indices) != 4 or indices != list(
            range(indices[0], indices[0] + 12, 3)
        ):
            raise TranslationError(
                "compact nav direction loop encountered an unexpected "
                f"branch layout: {indices!r}"
            )
        for index, (direction, keyboard, gamepad) in zip(
            indices, zip(directions, keyboards, gamepads)
        ):
            condition = source[index]
            assignment = source[index + 1]
            if (direction[0] not in condition
                    or keyboard[0] not in condition
                    or gamepad[0] not in condition
                    or direction[0] not in assignment
                    or source[index + 2].strip() != "}"):
                raise TranslationError(
                    "compact nav direction loop branch shape changed"
                )

        left = directions[0][0]
        down = directions[-1][0]
        keyboard_left = keyboards[0][0]
        gamepad_left = gamepads[0][0]
        replacement = [
            f"            for (imgui_c89_nav_dir = {left}; "
            f"imgui_c89_nav_dir <= {down}; ++imgui_c89_nav_dir) {{",
            f"                if ((!{is_active}(imgui_c89_ctx, imgui_c89_nav_dir)) &&",
            f"                    ((nav_gamepad_active && {is_pressed}(imgui_c89_ctx, {gamepad_left} + imgui_c89_nav_dir, repeat_mode, ((ImGuiID)((-1))))) ||",
            f"                     (nav_keyboard_active && {is_pressed}(imgui_c89_ctx, {keyboard_left} + imgui_c89_nav_dir, repeat_mode, ((ImGuiID)((-1))))))) {{",
            "                    g->NavMoveDir = imgui_c89_nav_dir;",
            "                }",
            "            }",
        ]
        start = indices[0]
        return ["    int imgui_c89_nav_dir;"], (
            source[:start] + replacement + source[start + 12:]
        )

    def compact_key_char_mask_body(
        self, function: dict[str, Any]
    ) -> tuple[list[str], list[str]] | None:
        """Precompute Initialize()'s fixed character-producing key bitset."""
        if (not self.compact_key_char_mask
                or function.get("qualified_name") != "ImGui::Initialize"):
            return None

        def constant(spelling: str) -> tuple[str, int]:
            matches = [
                (self.enum_constant_names[item["id"]], int(item["value"]))
                for enum in self.ir.get("enums", [])
                for item in enum.get("constants", [])
                if item.get("name") == spelling
            ]
            if len(set(matches)) != 1:
                raise TranslationError(
                    f"compact key character mask expected one {spelling}"
                )
            return matches[0]

        begin = constant("ImGuiKey_NamedKey_BEGIN")
        end = constant("ImGuiKey_NamedKey_END")
        ranges = [
            (constant("ImGuiKey_0"), constant("ImGuiKey_9")),
            (constant("ImGuiKey_A"), constant("ImGuiKey_Z")),
            (constant("ImGuiKey_Keypad0"), constant("ImGuiKey_Keypad9")),
        ]
        singles = [constant("ImGuiKey_" + name) for name in (
            "Tab", "Space", "Apostrophe", "Comma", "Minus", "Period",
            "Slash", "Semicolon", "Equal", "LeftBracket", "RightBracket",
            "GraveAccent", "KeypadDecimal", "KeypadDivide",
            "KeypadMultiply", "KeypadSubtract", "KeypadAdd", "KeypadEqual",
        )]
        if begin[1] >= end[1]:
            raise TranslationError("compact key character mask has empty range")
        words = (end[1] - begin[1] + 31) // 32
        if words != 5:
            raise TranslationError(
                "compact key character mask storage shape changed"
            )
        mask = [0] * words
        values: set[int] = set()
        for first, last in ranges:
            if first[1] > last[1]:
                raise TranslationError(
                    "compact key character mask range order changed"
                )
            values.update(range(first[1], last[1] + 1))
        values.update(value for _, value in singles)
        for value in values:
            if value < begin[1] or value >= end[1]:
                raise TranslationError(
                    "compact key character mask key escaped named-key range"
                )
            bit = value - begin[1]
            mask[bit // 32] |= 1 << (bit & 31)

        source = self.statement(function["body"])
        starts = [
            index for index, line in enumerate(source)
            if line.strip().startswith("key = ") and begin[0] in line
        ]
        if len(starts) != 1:
            raise TranslationError(
                "compact key character mask expected one initialization loop"
            )
        start = starts[0]
        original = source[start:start + 6]
        expected_names = {
            begin[0], end[0],
            *(name for pair in ranges for name, _ in pair),
            *(name for name, _ in singles),
        }
        original_text = "\n".join(original)
        if (len(original) != 6
                or not original[1].strip().startswith("for (")
                or not original[2].strip().startswith("if (")
                or "KeysMayBeCharInput" not in original[3]
                or any(name not in original_text for name in expected_names)
                or original[4].strip() != "}"
                or original[5].strip() != "}"):
            raise TranslationError(
                "compact key character mask initialization shape changed"
            )
        literal = ", ".join(f"0x{word:08x}u" for word in mask)
        replacement = [
            "    memcpy(g->KeysMayBeCharInput.Data, imgui_c89_char_input_mask,",
            "           sizeof(imgui_c89_char_input_mask));",
        ]
        locals_out = [
            line for line in self.prepare_locals(function)
            if line.strip() != "ImGuiKey key;"
        ]
        locals_out[0:0] = [
            f"    static const ImU32 imgui_c89_char_input_mask[{words}] = {{",
            f"        {literal}",
            "    };",
        ]
        return locals_out, source[:start] + replacement + source[start + 6:]

    def compact_optional_settings_body(
        self,
        function: dict[str, Any],
        existing: tuple[list[str], list[str]] | None,
    ) -> tuple[list[str], list[str]] | None:
        """Move Initialize's INI handler registration behind a provider.

        The optional-call shim itself has no relocation to the settings
        implementation.  Consequently a section-GC link which never enables
        the module can discard the provider and all of its handler closure.
        """
        if (not self.compact_optional_modules
                or function.get("qualified_name") != "ImGui::Initialize"):
            return None
        if self.optional_settings_init_lines is not None:
            raise TranslationError(
                "optional settings registration was captured more than once"
            )
        if existing is None:
            local_lines = self.prepare_locals(function)
            source = self.statement(function["body"])
        else:
            local_lines, source = existing
            local_lines = list(local_lines)
            source = list(source)

        constructors = [
            self.function_names[identifier]
            for identifier, declaration in self.function_declarations.items()
            if declaration.get("qualified_name")
            == "ImGuiSettingsHandler::ImGuiSettingsHandler"
            and declaration.get("constructor")
        ]
        table_handlers = [
            self.function_names[identifier]
            for identifier, declaration in self.function_declarations.items()
            if declaration.get("qualified_name")
            == "ImGui::TableSettingsAddSettingsHandler"
        ]
        add_handlers = [
            self.function_names[identifier]
            for identifier, declaration in self.function_declarations.items()
            if declaration.get("qualified_name")
            == "ImGui::AddSettingsHandler"
        ]
        if (len(set(constructors)) != 1
                or len(set(table_handlers)) != 1
                or len(set(add_handlers)) != 1):
            raise TranslationError(
                "optional settings registration helpers changed"
            )
        starts = [
            index for index, line in enumerate(source)
            if constructors[0] in line and "&ini_handler" in line
        ]
        ends = [
            index for index, line in enumerate(source)
            if table_handlers[0] in line
        ]
        if len(starts) != 1 or len(ends) != 1 or starts[0] > ends[0]:
            raise TranslationError(
                "optional settings registration block shape changed"
            )
        start, end = starts[0], ends[0]
        captured = source[start:end + 1]
        required = (
            "TypeName", "TypeHash", "ClearAllFn", "CleanupFn",
            "ReadOpenFn", "ReadLineFn", "ApplyAllFn", "WriteAllFn",
            add_handlers[0], table_handlers[0],
        )
        captured_text = "\n".join(captured)
        if (len(captured) != 11
                or any(captured_text.count(token) < 1 for token in required)):
            raise TranslationError(
                "optional settings registration contents changed"
            )
        declarations = [
            index for index, line in enumerate(local_lines)
            if line.strip() == "ImGuiSettingsHandler ini_handler;"
        ]
        if len(declarations) != 1:
            raise TranslationError(
                "optional settings handler local declaration changed"
            )
        del local_lines[declarations[0]]
        self.optional_settings_init_lines = captured
        replacement = [
            "    imgui_c89_optional_settings_init(imgui_c89_ctx);",
        ]
        return local_lines, source[:start] + replacement + source[end + 1:]

    def compact_localization_body(
        self,
        function: dict[str, Any],
        existing: tuple[list[str], list[str]] | None,
    ) -> tuple[list[str], list[str]] | None:
        """Register the fixed English localization table from packed data."""
        if (not self.compact_localization_entries
                or function.get("qualified_name") != "ImGui::Initialize"):
            return existing
        if len(self.compact_localization_global_ids) != 1:
            raise TranslationError(
                "compact localization expected one English entry table"
            )
        identifier = next(iter(self.compact_localization_global_ids))
        item = self.globals[identifier]
        entries = item.get("initializer", {}).get("values", [])
        keys: list[int] = []
        strings: list[str] = []
        for entry in entries:
            values = entry.get("values", [])
            if (entry.get("kind") != "InitListExpr"
                    or entry.get("field_names") != ["Key", "Text"]
                    or len(values) != 2):
                self.fail(item, "compact localization entry shape changed")
            key = self._find_expression_kind(values[0], "DeclRefExpr")
            text = self._find_expression_kind(values[1], "StringLiteral")
            if key is None or text is None:
                self.fail(item, "compact localization entry is not constant")
            value = self.enum_constant_values.get(key.get("decl", ""))
            if value is None:
                self.fail(item, "compact localization key is not an enum")
            keys.append(value)
            strings.append(text["value"])
        if not strings or keys != list(range(len(strings))):
            self.fail(item, "compact localization keys are not contiguous")
        offsets: list[int] = []
        offset = 0
        for value in strings:
            offsets.append(offset)
            offset += len(value.encode("utf-8")) + 1
        if offset > 65535:
            self.fail(item, "compact localization text exceeds 16-bit offsets")
        if existing is None:
            local_lines = self.prepare_locals(function)
            source = self.statement(function["body"])
        else:
            local_lines, source = map(list, existing)
        global_name = self.global_names[identifier]
        registration_names = {
            self.function_names[function_id]
            for function_id, candidate in self.function_declarations.items()
            if candidate.get("qualified_name") == "ImGui::LocalizeRegisterEntries"
        }
        calls = [
            index for index, line in enumerate(source)
            if global_name in line
            and any(name in line for name in registration_names)
        ]
        if len(calls) != 1:
            raise TranslationError(
                "compact localization registration call changed"
            )
        local_lines.extend((
            f"    static const unsigned short imgui_c89_loc_offsets[{len(offsets)}] = {{",
            "        " + ", ".join(str(value) for value in offsets),
            "    };",
            f"    static const char imgui_c89_loc_data[{offset + 1}] = "
            + self._string_literal("\0".join(strings) + "\0") + ";",
            "    int imgui_c89_loc_index;",
        ))
        start = calls[0]
        replacement = [
            f"    for (imgui_c89_loc_index = 0; imgui_c89_loc_index < {len(strings)}; ++imgui_c89_loc_index)",
            "        g->LocalizationTable[imgui_c89_loc_index] = imgui_c89_loc_data + imgui_c89_loc_offsets[imgui_c89_loc_index];",
        ]
        return local_lines, source[:start] + replacement + source[start + 1:]

    def compact_optional_cff_body(
        self, function: dict[str, Any]
    ) -> tuple[list[str], list[str]] | None:
        """Extract stb_truetype's CFF initialization branch as a module.

        This recognizes the semantic choke point (the `glyf` table branch)
        but preserves the ordinary emitted statements inside the provider.
        The fixed local-name check deliberately turns upstream drift into a
        translation error rather than guessing which state can be moved.
        """
        if (not self.compact_optional_modules
                or function.get("qualified_name") != "stbtt_InitFont_internal"):
            return None
        if (self.optional_cff_init_lines is not None
                or self.optional_cff_init_locals is not None):
            raise TranslationError(
                "optional CFF initialization was captured more than once"
            )

        body = function.get("body", {})
        candidates: list[dict[str, Any]] = []
        for statement in body.get("statements", []):
            if statement.get("kind") != "IfStmt" or "else" not in statement:
                continue
            members: list[str] = []

            def collect_members(node: Any) -> None:
                if isinstance(node, dict):
                    if node.get("kind") == "MemberExpr":
                        members.append(node.get("name", ""))
                    for value in node.values():
                        collect_members(value)
                elif isinstance(node, list):
                    for value in node:
                        collect_members(value)

            collect_members(statement.get("condition"))
            if members == ["glyf"]:
                candidates.append(statement)
        if len(candidates) != 1:
            raise TranslationError(
                "optional CFF init expected one glyf/CFF branch"
            )

        source = self.statement(body)
        starts = [
            index for index, line in enumerate(source)
            if line == "    if (info->glyf) {"
        ]
        if len(starts) != 1:
            raise TranslationError(
                "optional CFF init emitted glyf branch changed"
            )
        start = starts[0]
        else_candidates = [
            index for index in range(start + 1, len(source))
            if source[index] == "    } else {"
        ]
        if not else_candidates:
            raise TranslationError(
                "optional CFF init emitted else branch changed"
            )
        else_index = else_candidates[0]
        depth = 1
        end = else_index
        while depth and end + 1 < len(source):
            end += 1
            depth += source[end].count("{") - source[end].count("}")
        if depth != 0 or source[end] != "    }":
            raise TranslationError(
                "optional CFF init could not delimit emitted else branch"
            )

        cff_local_names = (
            "b", "topdict", "topdictidx", "cstype", "charstrings",
            "fdarrayoff", "fdselectoff", "cff",
        )
        local_lines = self.prepare_locals(function)
        cff_locals: list[str] = []
        main_locals: list[str] = []
        for line in local_lines:
            matched = next((
                name for name in cff_local_names
                if re.search(rf"\b{re.escape(name)};$", line.strip())
            ), None)
            if matched is None:
                main_locals.append(line)
            else:
                cff_locals.append(line)
        if len(cff_locals) != len(cff_local_names):
            raise TranslationError(
                "optional CFF init local set changed: "
                + ", ".join(line.strip() for line in cff_locals)
            )
        outside = "\n".join(source[:else_index] + source[end + 1:])
        leaking = [
            name for name in cff_local_names
            if re.search(
                rf"(?<!->)(?<!\.)\b{re.escape(name)}\b", outside
            )
        ]
        if leaking:
            raise TranslationError(
                "optional CFF locals also used outside module: "
                + ", ".join(leaking)
            )

        captured = [
            line[4:] if line.startswith("    ") else line
            for line in source[else_index + 1:end]
        ]
        captured_text = "\n".join(captured)
        required = (
            '"CFF "', "fontdicts", "fdselect", "charstrings", "gsubrs",
            "cstype", "stbtt__cff_get_index", "stbtt__dict_get_ints",
        )
        if any(token not in captured_text for token in required):
            raise TranslationError(
                "optional CFF initialization contents changed"
            )
        self.optional_cff_init_lines = captured
        self.optional_cff_init_locals = cff_locals
        replacement = [
            "    } else {",
            "        if (!imgui_c89_optional_cff_init(info, data, fontstart)) return 0;",
            "    }",
        ]
        return main_locals, source[:else_index] + replacement + source[end + 1:]

    def optional_module_function(
        self, qualified_name: str
    ) -> tuple[str, dict[str, Any]]:
        matches = [
            (identifier, function)
            for identifier, function in self.functions.items()
            if function.get("qualified_name") == qualified_name
        ]
        if len(matches) != 1:
            raise TranslationError(
                f"optional module expected one {qualified_name} definition, "
                f"found {len(matches)}"
            )
        return matches[0]

    def optional_module_function_active(self, identifier: str) -> bool:
        return (
            self.active_function_ids is None
            or identifier in self.active_function_ids
        )

    def emit_optional_module_helpers(self) -> list[str]:
        """Emit relocation-free shims and separately collectible providers."""
        if not self.compact_optional_modules:
            return []
        lines: list[str] = []

        nav_functions = [
            (*self.optional_module_function(name), phase)
            for name, phase in self.OPTIONAL_NAV_PHASES.items()
        ]
        nav_active = [
            (identifier, function, phase)
            for identifier, function, phase in nav_functions
            if self.optional_module_function_active(identifier)
        ]
        if nav_active:
            if len(nav_active) != len(nav_functions):
                raise TranslationError(
                    "optional navigation phases span translation units"
                )
            for identifier, function, _ in nav_active:
                if (function.get("return_type") != "void"
                        or function.get("parameters")):
                    raise TranslationError(
                        "optional navigation phase signature changed: "
                        + function.get("qualified_name", "")
                    )
                if identifier not in self.context_threaded_functions:
                    raise TranslationError(
                        "optional navigation phase lost context threading: "
                        + function.get("qualified_name", "")
                    )
            lines.extend([
                "static void (*imgui_c89_nav_module)(ImGuiContext *, int);",
                "static void imgui_c89_optional_nav(ImGuiContext *ctx, int phase)",
                "{",
                "    if (imgui_c89_nav_module != 0) imgui_c89_nav_module(ctx, phase);",
                "}",
                "static void imgui_c89_nav_provider(ImGuiContext *ctx, int phase)",
                "{",
                "    switch (phase) {",
            ])
            for identifier, _, phase in sorted(
                nav_active, key=lambda item: item[2]
            ):
                lines.append(
                    f"        case {phase}: {self.function_names[identifier]}(ctx); break;"
                )
            lines.extend([
                "        default: break;",
                "    }",
                "}",
                "static void imgui_c89_enable_navigation_module(void)",
                "{",
                "    imgui_c89_nav_module = imgui_c89_nav_provider;",
                "}",
                "",
            ])
        initialize_id, _ = self.optional_module_function("ImGui::Initialize")
        update_id, update_function = self.optional_module_function(
            "ImGui::UpdateSettings"
        )
        settings_active = self.optional_module_function_active(initialize_id)
        if settings_active:
            if (not self.optional_module_function_active(update_id)
                    or self.optional_settings_init_lines is None):
                raise TranslationError(
                    "optional settings functions span translation units or "
                    "registration was not captured"
                )
            if (update_function.get("return_type") != "void"
                    or update_function.get("parameters")
                    or update_id not in self.context_threaded_functions):
                raise TranslationError(
                    "optional settings update signature changed"
                )
            lines.extend([
                "static void (*imgui_c89_settings_module)(ImGuiContext *, int);",
                "static void imgui_c89_optional_settings_init(ImGuiContext *ctx)",
                "{",
                "    if (imgui_c89_settings_module != 0) imgui_c89_settings_module(ctx, 0);",
                "}",
                "static void imgui_c89_optional_settings_update(ImGuiContext *ctx)",
                "{",
                "    if (imgui_c89_settings_module != 0) imgui_c89_settings_module(ctx, 1);",
                "}",
                "static void imgui_c89_settings_provider(ImGuiContext *ctx, int phase)",
                "{",
                "    if (phase == 0) {",
                "        ImGuiSettingsHandler ini_handler;",
                "        if (ctx == 0 || ctx->SettingsHandlers.Size != 0) return;",
            ])
            lines.extend(
                "    " + line.replace("imgui_c89_ctx", "ctx")
                for line in self.optional_settings_init_lines
            )
            lines.extend([
                "    } else {",
                f"        {self.function_names[update_id]}(ctx);",
                "    }",
                "}",
                "static void imgui_c89_enable_settings_module(ImGuiContext *ctx)",
                "{",
                "    imgui_c89_settings_module = imgui_c89_settings_provider;",
                "    imgui_c89_optional_settings_init(ctx);",
                "}",
                "",
            ])

        cff_names = (
            "stbtt__GetGlyphInfoT2", "stbtt__GetGlyphShapeT2",
            "stbtt_InitFont_internal",
        )
        cff_functions = [
            self.optional_module_function(name) for name in cff_names
        ]
        cff_active = [
            (identifier, function)
            for identifier, function in cff_functions
            if self.optional_module_function_active(identifier)
        ]
        if cff_active:
            if (len(cff_active) != len(cff_functions)
                    or self.optional_cff_init_lines is None
                    or self.optional_cff_init_locals is None):
                raise TranslationError(
                    "optional CFF functions span translation units or init "
                    "was not captured"
                )
            (info_id, info_function), (shape_id, shape_function), (
                init_id, init_function
            ) = cff_functions
            for function in (info_function, shape_function, init_function):
                if function.get("return_type") != "int":
                    raise TranslationError(
                        "optional CFF helper return type changed: "
                        + function.get("qualified_name", "")
                    )

            def parameters(function: dict[str, Any]) -> str:
                return ", ".join(self.c_parameters(function)) or "void"

            def arguments(function: dict[str, Any]) -> str:
                return ", ".join(
                    parameter.get("name") or f"arg_{index}"
                    for index, parameter in enumerate(
                        function.get("parameters", [])
                    )
                )

            info_parameters = parameters(info_function)
            shape_parameters = parameters(shape_function)
            init_parameters = parameters(init_function)
            info_arguments = arguments(info_function)
            shape_arguments = arguments(shape_function)
            init_arguments = arguments(init_function)
            lines.extend([
                f"static int (*imgui_c89_cff_info_module)({info_parameters});",
                f"static int (*imgui_c89_cff_shape_module)({shape_parameters});",
                f"static int (*imgui_c89_cff_init_module)({init_parameters});",
                f"static int imgui_c89_optional_cff_info({info_parameters})",
                "{",
                "    return imgui_c89_cff_info_module != 0",
                f"        ? imgui_c89_cff_info_module({info_arguments}) : 0;",
                "}",
                f"static int imgui_c89_optional_cff_shape({shape_parameters})",
                "{",
                "    return imgui_c89_cff_shape_module != 0",
                f"        ? imgui_c89_cff_shape_module({shape_arguments}) : 0;",
                "}",
                f"static int imgui_c89_optional_cff_init({init_parameters})",
                "{",
                "    return imgui_c89_cff_init_module != 0",
                f"        ? imgui_c89_cff_init_module({init_arguments}) : 0;",
                "}",
                f"static int imgui_c89_cff_init_provider({init_parameters})",
                "{",
            ])
            lines.extend(self.optional_cff_init_locals)
            lines.extend(self.optional_cff_init_lines)
            lines.extend([
                "    return 1;",
                "}",
                "void imgui_c89_enable_cff_module(void)",
                "{",
                f"    imgui_c89_cff_info_module = {self.function_names[info_id]};",
                f"    imgui_c89_cff_shape_module = {self.function_names[shape_id]};",
                "    imgui_c89_cff_init_module = imgui_c89_cff_init_provider;",
                "}",
                "",
            ])

        if nav_active and settings_active:
            lines.extend([
                "void imgui_c89_enable_full_features(ImGuiContext *ctx)",
                "{",
                "    imgui_c89_enable_navigation_module();",
                "    imgui_c89_enable_settings_module(ctx);",
                "    imgui_c89_enable_cff_module();",
                "}",
                "",
            ])
        return lines

    def compact_nav_wrapping_body(
        self, function: dict[str, Any]
    ) -> tuple[list[str], list[str]] | None:
        """Share the X/Y halves of the four-direction wrapping state machine."""
        if (not self.compact_nav_key_ranges
                or function.get("qualified_name")
                != "ImGui::NavUpdateCreateWrappingRequest"):
            return None

        def constant_name(spelling: str) -> str:
            matches = [
                self.enum_constant_names[item["id"]]
                for enum in self.ir.get("enums", [])
                for item in enum.get("constants", [])
                if item.get("name") == spelling
            ]
            if len(set(matches)) != 1:
                raise TranslationError(
                    f"compact nav wrapping expected one {spelling}"
                )
            return matches[0]

        left = constant_name("ImGuiDir_Left")
        right = constant_name("ImGuiDir_Right")
        up = constant_name("ImGuiDir_Up")
        down = constant_name("ImGuiDir_Down")
        wrap_x = constant_name("ImGuiNavMoveFlags_WrapX")
        loop_x = constant_name("ImGuiNavMoveFlags_LoopX")
        wrap_y = constant_name("ImGuiNavMoveFlags_WrapY")
        loop_y = constant_name("ImGuiNavMoveFlags_LoopY")

        source = self.statement(function["body"])
        starts = [
            index for index, line in enumerate(source)
            if "if (" in line
            and "g->NavMoveDir ==" in line
            and "ImGuiNavMoveFlags_" in line
        ]
        if len(starts) != 4:
            raise TranslationError(
                "compact nav wrapping encountered an unexpected branch "
                f"count: {starts!r}"
            )
        expected = (
            (left, wrap_x, loop_x, up),
            (right, wrap_x, loop_x, down),
            (up, wrap_y, loop_y, left),
            (down, wrap_y, loop_y, right),
        )
        end = starts[-1]
        depth = 0
        found_open = False
        while end < len(source):
            depth += source[end].count("{") - source[end].count("}")
            found_open = found_open or "{" in source[end]
            end += 1
            if found_open and depth == 0:
                break
        if (end >= len(source) or "if (" not in source[end]
                or "!do_forward" not in source[end]):
            raise TranslationError(
                "compact nav wrapping could not delimit direction branches"
            )
        for branch, (direction, wrap, loop, clip) in enumerate(expected):
            begin = starts[branch]
            finish = starts[branch + 1] if branch + 1 < 4 else end
            text = "\n".join(source[begin:finish])
            if not all(token in text for token in (
                direction, wrap, loop, clip, "do_forward = 1"
            )):
                raise TranslationError(
                    "compact nav wrapping branch shape changed"
                )

        replacement = [
            f"    if (g->NavMoveDir >= {left} && g->NavMoveDir <= {right} &&",
            f"        (move_flags & ({wrap_x} | {loop_x}))) {{",
            f"        bb_rel.Min.x = bb_rel.Max.x = (g->NavMoveDir == {left}) ? wrap_size.x : -window->WindowPadding.x;",
            f"        if (move_flags & {wrap_x}) {{",
            "            imgui_c89_wrap_delta = bb_rel.Max.y - bb_rel.Min.y;",
            f"            if (g->NavMoveDir == {left}) imgui_c89_wrap_delta = -imgui_c89_wrap_delta;",
            "            bb_rel.Min.y += imgui_c89_wrap_delta;",
            "            bb_rel.Max.y += imgui_c89_wrap_delta;",
            f"            clip_dir = (g->NavMoveDir == {left}) ? {up} : {down};",
            "        }",
            "        do_forward = 1;",
            f"    }} else if (g->NavMoveDir >= {up} && g->NavMoveDir <= {down} &&",
            f"               (move_flags & ({wrap_y} | {loop_y}))) {{",
            f"        bb_rel.Min.y = bb_rel.Max.y = (g->NavMoveDir == {up}) ? wrap_size.y : -window->WindowPadding.y;",
            f"        if (move_flags & {wrap_y}) {{",
            "            imgui_c89_wrap_delta = bb_rel.Max.x - bb_rel.Min.x;",
            f"            if (g->NavMoveDir == {up}) imgui_c89_wrap_delta = -imgui_c89_wrap_delta;",
            "            bb_rel.Min.x += imgui_c89_wrap_delta;",
            "            bb_rel.Max.x += imgui_c89_wrap_delta;",
            f"            clip_dir = (g->NavMoveDir == {up}) ? {left} : {right};",
            "        }",
            "        do_forward = 1;",
            "    }",
        ]
        return ["    float imgui_c89_wrap_delta;"], (
            source[:starts[0]] + replacement + source[end:]
        )

    def record_for_object_type(self, spelling: str) -> dict[str, Any] | None:
        """Resolve a non-pointer object type to its translated record."""
        value = spelling.strip()
        while value.startswith(("const ", "volatile ")):
            value = value.split(" ", 1)[1].strip()
        while value.endswith((" const", " volatile")):
            value = value.rsplit(" ", 1)[0].strip()
        record = self.records_by_spelling.get(value)
        if record is None:
            record = self.records_by_c_name.get(self.c_type(value))
        return record

    def object_destructor_dependencies(
        self, spelling: str, seen: set[str] | None = None
    ) -> set[str]:
        value = spelling.strip()
        if value.endswith("&"):
            return set()
        array = re.match(r"^(.*)\[(\d+)\]$", value)
        if array:
            return self.object_destructor_dependencies(
                array.group(1).strip(), seen
            )
        record = self.record_for_object_type(value)
        if record is None or record.get("union"):
            return set()
        destructor_id = self.destructor_ids_by_type.get(
            record["qualified_name"]
        ) or self.destructor_ids_by_type.get(record.get("name", ""))
        if destructor_id:
            return {destructor_id}
        return self.record_subobject_destructor_dependencies(record, seen)

    def record_subobject_destructor_dependencies(
        self, record: dict[str, Any], seen: set[str] | None = None
    ) -> set[str]:
        seen = set() if seen is None else seen
        if record["id"] in seen:
            return set()
        seen.add(record["id"])
        result: set[str] = set()
        for field in record.get("fields", []):
            if field.get("id") in self.flattened_table_vector_fields:
                continue
            result.update(self.object_destructor_dependencies(
                field["type"], seen
            ))
        for base in record.get("bases", []):
            base_record = self.records.get(base.get("record", ""))
            if base_record is not None:
                result.update(self.object_destructor_dependencies(
                    base_record["qualified_name"], seen
                ))
        return result

    def object_cleanup_lines(
        self, spelling: str, target: str, indent: str
    ) -> list[str]:
        """Emit destruction of one fully-constructed C++ object lvalue."""
        value = spelling.strip()
        if value.endswith("&"):
            return []
        array = re.match(r"^(.*)\[(\d+)\]$", value)
        if array:
            element_type = array.group(1).strip()
            if not self.object_cleanup_lines(element_type, target + "[0]", indent):
                return []
            index = f"imgui_c89_cleanup_{len(self.expression_temporaries)}"
            self.expression_temporaries.append((index, "int"))
            result = [
                indent + f"for ({index} = {array.group(2)}; {index} > 0; --{index}) {{"
            ]
            result.extend(self.object_cleanup_lines(
                element_type, f"{target}[{index} - 1]", indent + "    "
            ))
            result.append(indent + "}")
            return result

        record = self.record_for_object_type(value)
        if record is None:
            return []
        destructor_id = self.destructor_ids_by_type.get(
            record["qualified_name"]
        ) or self.destructor_ids_by_type.get(record.get("name", ""))
        if destructor_id:
            arguments = self.with_context_argument(
                destructor_id, [f"&({target})"]
            )
            return [
                indent + f"{self.function_names[destructor_id]}("
                + ", ".join(arguments) + ");"
            ]
        if record.get("union"):
            return []
        return self.record_subobject_cleanup_lines(record, target, indent)

    def object_type_needs_cleanup(self, spelling: str,
                                  seen: set[str] | None = None) -> bool:
        return bool(self.object_destructor_dependencies(spelling, seen))

    def record_subobject_cleanup_lines(
        self, record: dict[str, Any], target: str, indent: str
    ) -> list[str]:
        """Emit implicit member/base destruction in C++ reverse order."""
        result: list[str] = []
        for field in reversed(record.get("fields", [])):
            field_name = self.field_names[field["id"]]
            vector = self.flattened_table_vector_fields.get(field["id"])
            if vector is not None:
                result.append(
                    indent + "imgui_c89_vector_destroy("
                    + self.current_context_expression()
                    + f", ({target}).{field_name});"
                )
                continue
            result.extend(self.object_cleanup_lines(
                field["type"], f"({target}).{field_name}", indent
            ))
        for base in reversed(record.get("bases", [])):
            base_record = self.records.get(base.get("record", ""))
            if base_record is None:
                continue
            base_type = self.record_names[base_record["qualified_name"]]
            result.extend(self.object_cleanup_lines(
                base_record["qualified_name"],
                f"*(({base_type} *)&({target}))", indent,
            ))
        return result

    def scope_cleanup_lines(self, scope: list[tuple[str, str]],
                            indent: str) -> list[str]:
        result: list[str] = []
        for spelling, target in reversed(scope):
            result.extend(self.object_cleanup_lines(spelling, target, indent))
        return result

    def active_cleanup_lines(self, indent: str, minimum_depth: int = 0,
                             include_function_exit: bool = False) -> list[str]:
        result: list[str] = []
        for scope in reversed(self.cleanup_scopes[minimum_depth:]):
            result.extend(self.scope_cleanup_lines(scope, indent))
        if include_function_exit:
            result.extend(indent + line.lstrip() for line in self.function_exit_cleanup)
        return result

    def scoped_statement(self, node: dict[str, Any], indent: str) -> list[str]:
        if node.get("kind") == "CompoundStmt":
            return self.statement(node, indent)
        self.cleanup_scopes.append([])
        result = self.statement(node, indent)
        result.extend(self.scope_cleanup_lines(self.cleanup_scopes.pop(), indent))
        return result

    @staticmethod
    def strip_outer_parentheses(value: str) -> str:
        """Drop grouping already supplied by a statement or control construct."""
        while len(value) >= 2 and value[0] == "(" and value[-1] == ")":
            depth = 0
            quote = ""
            escaped = False
            encloses_all = True
            for index, character in enumerate(value):
                if quote:
                    if escaped:
                        escaped = False
                    elif character == "\\":
                        escaped = True
                    elif character == quote:
                        quote = ""
                    continue
                if character in {"'", '"'}:
                    quote = character
                elif character == "(":
                    depth += 1
                elif character == ")":
                    depth -= 1
                    if depth == 0 and index != len(value) - 1:
                        encloses_all = False
                        break
            if not encloses_all or depth != 0 or quote:
                break
            value = value[1:-1]
        return value

    def statement_expression(self, node: dict[str, Any]) -> str:
        return self.strip_outer_parentheses(self.expression(node))

    def discarded_expression(self, node: dict[str, Any]) -> str:
        """Emit a value-discarding expression without reading a returned ref."""
        value = self.statement_expression(node)
        current = node
        while current.get("kind") in {
            "ImplicitCastExpr", "ParenExpr", "ConstantExpr",
            "MaterializeTemporaryExpr", "ExprWithCleanups",
        } and "operand" in current:
            current = current["operand"]
        if current.get("kind") in {
            "CallExpr", "CXXMemberCallExpr", "CXXOperatorCallExpr",
        }:
            function = self.function_declarations.get(
                current.get("callee", ""), {}
            )
            if (function.get("return_type", "").strip().endswith("&")
                    and value.startswith("*(") and value.endswith(")")):
                return value[2:-1]
        return value

    @staticmethod
    def is_constant_noop_statement(node: dict[str, Any]) -> bool:
        """Recognize scalar literals left behind by no-op C++ macros."""
        while node.get("kind") in {
            "ImplicitCastExpr", "ParenExpr", "ConstantExpr",
            "CStyleCastExpr", "CXXStaticCastExpr", "CXXFunctionalCastExpr",
            "CXXConstCastExpr", "CXXReinterpretCastExpr",
        } and "operand" in node:
            node = node["operand"]
        return node.get("kind") in {
            "IntegerLiteral", "FloatingLiteral", "CharacterLiteral",
        }

    def statement(self, node: dict[str, Any], indent: str = "    ") -> list[str]:
        kind = node.get("kind")
        if kind == "CompoundStmt":
            self.cleanup_scopes.append([])
            result: list[str] = []
            for child in node.get("statements", []):
                result.extend(self.statement(child, indent))
            result.extend(self.scope_cleanup_lines(
                self.cleanup_scopes.pop(), indent
            ))
            return result
        if kind == "DeclStmt":
            result = []
            for declaration in node.get("declarations", []):
                if declaration.get("id") in self.static_const_local_ids:
                    continue
                name = self.declaration_name(declaration)
                if declaration.get("static_local"):
                    if self.static_initializer_requires_runtime(
                        declaration.get("initializer")
                    ):
                        ordinary = dict(declaration)
                        ordinary["static_local"] = False
                        # This recursive DeclStmt emits one-time construction
                        # into static storage.  Its lifetime is still static;
                        # never register it for block-exit destruction.
                        ordinary["skip_scope_cleanup"] = True
                        result.append(indent + f"if (!{name}__imgui_c89_initialized) {{")
                        result.extend(self.statement({
                            "kind": "DeclStmt",
                            "declarations": [ordinary],
                        }, indent + "    "))
                        result.append(indent + f"    {name}__imgui_c89_initialized = 1;")
                        result.append(indent + "}")
                    continue
                if (not declaration.get("skip_scope_cleanup")
                        and self.cleanup_scopes
                        and self.object_type_needs_cleanup(
                            declaration.get("type", "")
                        )):
                    self.cleanup_scopes[-1].append((declaration["type"], name))
                if "initializer" in declaration:
                    initializer = declaration["initializer"]
                    while "operand" in initializer and initializer.get("kind") in {
                        "MaterializeTemporaryExpr", "ExprWithCleanups", "ConstantExpr",
                        "CXXDefaultArgExpr", "CXXBindTemporaryExpr", "CXXDefaultInitExpr",
                    }:
                        initializer = initializer["operand"]
                    if initializer.get("kind") == "InitListExpr":
                        result.append(indent + f"memset(&{name}, 0, sizeof({name}));")
                        result.extend(self.aggregate_assignments(
                            name, initializer, indent
                        ))
                        continue
                    if (re.search(r"\[[^\]]*\]$", declaration["type"])
                            and initializer.get("kind") == "StringLiteral"):
                        result.append(
                            indent + f"strcpy({name}, {self.expression(initializer)});"
                        )
                        continue
                    if declaration["type"].strip().endswith("&"):
                        result.append(
                            indent + f"{name} = "
                            f"{self.object_pointer(initializer)};"
                        )
                        continue
                    if initializer.get("kind") == "CXXConstructExpr":
                        constructor = self.function_names.get(
                            initializer.get("constructor")
                        )
                        arguments = initializer.get("arguments", [])
                        array_match = re.match(
                            r"^(.*)\[(\d+)\]$", initializer.get("type", "")
                        )
                        if not arguments and array_match:
                            base_type = self.base_spelling(array_match.group(1))
                            count = array_match.group(2)
                            index_name = f"imgui_c89_init_{name}"
                            element_constructor = self.function_names.get(
                                initializer.get("constructor")
                            )
                            record = self.records_by_spelling.get(base_type)
                            needs_defaults = record and any(
                                "default" in field
                                for field in record.get("fields", [])
                            )
                            if not element_constructor and not needs_defaults:
                                if initializer.get("trivial"):
                                    continue
                                result.append(
                                    indent + f"memset(&{name}, 0, sizeof({name}));"
                                )
                                continue
                            result.append(
                                indent + f"memset(&{name}, 0, sizeof({name}));"
                            )
                            if element_constructor or needs_defaults:
                                result.append(indent + f"for ({index_name} = 0; {index_name} < {count}; ++{index_name}) {{")
                                if element_constructor:
                                    values = self.with_context_argument(
                                        initializer.get("constructor"),
                                        [f"&{name}[{index_name}]"],
                                    )
                                    result.append(indent + f"    {element_constructor}({', '.join(values)});")
                                if record:
                                    for field in record.get("fields", []):
                                        if "default" in field:
                                            field_name = self.field_names[field["id"]]
                                            result.append(
                                                indent + f"    {name}[{index_name}].{field_name} = "
                                                f"{self.expression(field['default'])};"
                                            )
                                result.append(indent + "}")
                            continue
                        if constructor:
                            values = [f"&{name}"]
                            values.extend(self.call_arguments(
                                initializer.get("constructor"), arguments
                            ))
                            values = self.with_context_argument(
                                initializer.get("constructor"), values
                            )
                            result.append(
                                indent + f"{constructor}({', '.join(values)});"
                            )
                            continue
                        if len(arguments) == 1:
                            result.append(
                                indent + f"{name} = "
                                f"{self.expression(arguments[0])};"
                            )
                            continue
                        if not arguments and initializer.get("trivial"):
                            continue
                        array_match = re.match(r"^(.*)\[(\d+)\]$", initializer.get("type", ""))
                        if not arguments and array_match:
                            base_type = self.base_spelling(array_match.group(1))
                            count = array_match.group(2)
                            index_name = f"imgui_c89_init_{name}"
                            result.append(
                                indent + f"memset(&{name}, 0, sizeof({name}));"
                            )
                            constructor = self.function_names.get(initializer.get("constructor"))
                            record = self.records_by_spelling.get(base_type)
                            if constructor or (record and any("default" in field for field in record.get("fields", []))):
                                result.append(indent + f"for ({index_name} = 0; {index_name} < {count}; ++{index_name}) {{")
                                if constructor:
                                    values = self.with_context_argument(
                                        initializer.get("constructor"),
                                        [f"&{name}[{index_name}]"],
                                    )
                                    result.append(indent + f"    {constructor}({', '.join(values)});")
                                if record:
                                    for field in record.get("fields", []):
                                        if "default" in field:
                                            result.append(
                                                indent + f"    {name}[{index_name}].{field['name']} = "
                                                f"{self.expression(field['default'])};"
                                            )
                                result.append(indent + "}")
                            continue
                        if not arguments:
                            record = self.records_by_spelling.get(
                                self.base_spelling(initializer.get("type", ""))
                            )
                            if record:
                                result.extend(
                                    self.implicit_default_construction(
                                        name, record, indent
                                    )
                                )
                                continue
                        self.fail(initializer, "unresolved default constructor")
                    result.append(
                        indent + f"{name} = "
                        f"{self.expression(initializer)};"
                    )
            return result
        if kind == "ReturnStmt":
            cleanup = self.active_cleanup_lines(
                indent, include_function_exit=True
            )
            if "value" not in node:
                return cleanup + [indent + "return;"]
            if self.current_return_type.strip().endswith("&"):
                value = self.reference_argument(
                    node["value"], {"type": self.current_return_type}
                )
            else:
                value = self.statement_expression(node["value"])
            value = self.strip_outer_parentheses(value)
            if self.current_return_type == "void":
                return [indent + value + ";"] + cleanup + [indent + "return;"]
            if not cleanup:
                return [indent + f"return {value};"]
            temporary = f"imgui_c89_return_{len(self.expression_temporaries)}"
            self.expression_temporaries.append(
                (temporary, self.current_return_type)
            )
            return (
                [indent + f"{temporary} = {value};"]
                + cleanup
                + [indent + f"return {temporary};"]
            )
        if self.is_constant_noop_statement(node):
            return []
        if kind in {
            "BinaryOperator",
            "CompoundAssignOperator",
            "CallExpr",
            "CXXMemberCallExpr",
            "CXXOperatorCallExpr",
            "UnaryOperator",
            "ParenExpr",
            "ImplicitCastExpr",
            "CStyleCastExpr",
            "CXXStaticCastExpr",
            "CXXFunctionalCastExpr",
            "CXXConstCastExpr",
            "CXXReinterpretCastExpr",
            "ConditionalOperator",
            "ArraySubscriptExpr",
        }:
            if self.omit_compact_zero_statement(node):
                return []
            value = self.discarded_expression(node)
            return [] if value in {"0", "(void)0"} else [indent + value + ";"]
        if kind == "IfStmt":
            self.cleanup_scopes.append([])
            result = []
            if "condition_variable" in node:
                result.extend(self.statement(node["condition_variable"], indent))
            result.append(
                indent + f"if ({self.statement_expression(node['condition'])}) {{"
            )
            result.extend(self.scoped_statement(node["then"], indent + "    "))
            if "else" in node:
                result.append(indent + "} else {")
                result.extend(self.scoped_statement(node["else"], indent + "    "))
            result.append(indent + "}")
            result.extend(self.scope_cleanup_lines(
                self.cleanup_scopes.pop(), indent
            ))
            return result
        if kind == "ForStmt":
            self.cleanup_scopes.append([])
            initializer = node.get("initializer")
            prefix: list[str] = []
            if initializer and initializer.get("kind") == "DeclStmt":
                # C89 forbids declarations in a for initializer.  Hoist the
                # loop variable immediately before the loop; the surrounding
                # function is already emitted with C89 declaration ordering.
                prefix.extend(self.statement(initializer, indent))
                initializer = None
            init_text = ""
            if initializer:
                if initializer.get("kind") not in {
                    "BinaryOperator", "CompoundAssignOperator", "UnaryOperator"
                }:
                    self.fail(initializer, "unsupported for-loop initializer")
                init_text = self.statement_expression(initializer)
            condition = (
                self.statement_expression(node["condition"])
                if "condition" in node else ""
            )
            increment = (
                self.statement_expression(node["increment"])
                if "increment" in node else ""
            )
            result = prefix + [indent + f"for ({init_text}; {condition}; {increment}) {{"]
            boundary = len(self.cleanup_scopes)
            self.break_cleanup_depths.append(boundary)
            self.continue_cleanup_depths.append(boundary)
            result.extend(self.scoped_statement(node["body"], indent + "    "))
            self.continue_cleanup_depths.pop()
            self.break_cleanup_depths.pop()
            result.append(indent + "}")
            result.extend(self.scope_cleanup_lines(
                self.cleanup_scopes.pop(), indent
            ))
            return result
        if kind == "CXXForRangeStmt":
            if self.current_compact_nav_key_ranges:
                loop_declarations = node.get("loop_variable", {}).get(
                    "declarations", []
                )
                begin_declarations = node.get("begin", {}).get(
                    "declarations", []
                )
                if len(loop_declarations) != 1 or len(begin_declarations) != 1:
                    raise TranslationError(
                        "compact navigation key range shape changed"
                    )
                loop_id = loop_declarations[0]["id"]
                self.local_names[loop_id] = "key"
                array_type = begin_declarations[0].get("initializer", {}).get(
                    "operand", {}
                ).get("type", "")
                if array_type.endswith("[8]"):
                    ranges = [
                        ("ImGuiKey_GamepadFaceLeft", "ImGuiKey_GamepadDpadDown")
                    ]
                elif array_type.endswith("[7]"):
                    ranges = [
                        ("ImGuiKey_LeftArrow", "ImGuiKey_DownArrow"),
                        ("ImGuiKey_Space", "ImGuiKey_Escape"),
                    ]
                else:
                    raise TranslationError(
                        "compact navigation key array changed"
                    )

                def enum_name(spelling: str) -> str:
                    matches = [
                        self.enum_constant_names[constant["id"]]
                        for enum in self.ir.get("enums", [])
                        for constant in enum.get("constants", [])
                        if constant.get("name") == spelling
                    ]
                    if len(matches) != 1:
                        raise TranslationError(
                            f"compact navigation expected enum {spelling}"
                        )
                    return matches[0]

                result = []
                for first, last in ranges:
                    result.append(
                        indent + f"for (key = {enum_name(first)}; "
                        f"key <= {enum_name(last)}; ++key) {{"
                    )
                    result.extend(self.statement(node["body"], indent + "    "))
                    result.append(indent + "}")
                return result
            self.cleanup_scopes.append([])
            result = []
            range_declarations = node.get("range", {}).get(
                "declarations", []
            )
            flattened_range = None
            if len(range_declarations) == 1:
                flattened_range = self.flattened_table_vector_components(
                    range_declarations[0].get("initializer", {})
                )
            if flattened_range is None:
                for key in ("initializer", "range", "begin", "end"):
                    if key in node:
                        result.extend(self.statement(node[key], indent))
            else:
                if "initializer" in node:
                    result.extend(self.statement(node["initializer"], indent))
                begin_declarations = node.get("begin", {}).get(
                    "declarations", []
                )
                end_declarations = node.get("end", {}).get(
                    "declarations", []
                )
                if (len(begin_declarations) != 1
                        or len(end_declarations) != 1):
                    raise TranslationError(
                        "flattened table vector range shape changed"
                    )
                data, size, _, _, _ = flattened_range
                begin_name = self.local_names[begin_declarations[0]["id"]]
                end_name = self.local_names[end_declarations[0]["id"]]
                result.append(indent + f"{begin_name} = {data};")
                result.append(
                    indent + f"{end_name} = {data} == 0 ? 0 : {data} + {size};"
                )
            condition = self.statement_expression(node["condition"])
            increment = self.statement_expression(node["increment"])
            result.append(indent + f"for (; {condition}; {increment}) {{")
            inner = indent + "    "
            boundary = len(self.cleanup_scopes)
            self.break_cleanup_depths.append(boundary)
            self.continue_cleanup_depths.append(boundary)
            self.cleanup_scopes.append([])
            result.extend(self.statement(node["loop_variable"], inner))
            result.extend(self.scoped_statement(node["body"], inner))
            result.extend(self.scope_cleanup_lines(
                self.cleanup_scopes.pop(), inner
            ))
            self.continue_cleanup_depths.pop()
            self.break_cleanup_depths.pop()
            result.append(indent + "}")
            result.extend(self.scope_cleanup_lines(
                self.cleanup_scopes.pop(), indent
            ))
            return result
        if kind == "WhileStmt":
            if "condition_variable" in node:
                result = [indent + "while (1) {"]
                inner = indent + "    "
                self.cleanup_scopes.append([])
                result.extend(self.statement(node["condition_variable"], inner))
                result.append(
                    inner + f"if (!({self.statement_expression(node['condition'])})) break;"
                )
                boundary = len(self.cleanup_scopes)
                self.break_cleanup_depths.append(boundary)
                self.continue_cleanup_depths.append(boundary)
                result.extend(self.scoped_statement(node["body"], inner))
                self.continue_cleanup_depths.pop()
                self.break_cleanup_depths.pop()
                result.extend(self.scope_cleanup_lines(
                    self.cleanup_scopes.pop(), inner
                ))
                result.append(indent + "}")
                return result
            result = [
                indent + f"while ({self.statement_expression(node['condition'])}) {{"
            ]
            boundary = len(self.cleanup_scopes)
            self.break_cleanup_depths.append(boundary)
            self.continue_cleanup_depths.append(boundary)
            result.extend(self.scoped_statement(node["body"], indent + "    "))
            self.continue_cleanup_depths.pop()
            self.break_cleanup_depths.pop()
            result.append(indent + "}")
            return result
        if kind == "DoStmt":
            result = [indent + "do {"]
            boundary = len(self.cleanup_scopes)
            self.break_cleanup_depths.append(boundary)
            self.continue_cleanup_depths.append(boundary)
            result.extend(self.scoped_statement(node["body"], indent + "    "))
            self.continue_cleanup_depths.pop()
            self.break_cleanup_depths.pop()
            result.append(
                indent + f"}} while ({self.statement_expression(node['condition'])});"
            )
            return result
        if kind == "SwitchStmt":
            result = [
                indent + f"switch ({self.statement_expression(node['condition'])}) {{"
            ]
            boundary = len(self.cleanup_scopes)
            self.break_cleanup_depths.append(boundary)
            result.extend(self.scoped_statement(node["body"], indent + "    "))
            self.break_cleanup_depths.pop()
            result.append(indent + "}")
            return result
        if kind == "CaseStmt":
            result = [indent + f"case {self.expression(node['value'])}:"]
            body = self.statement(node["body"], indent + "    ")
            result.extend(body or [indent + "    ;"])
            return result
        if kind == "DefaultStmt":
            result = [indent + "default:"]
            body = self.statement(node["body"], indent + "    ")
            result.extend(body or [indent + "    ;"])
            return result
        if kind == "GotoStmt":
            if any(self.cleanup_scopes):
                self.fail(node, "goto with active C++ destructors is unsupported")
            return [indent + f"goto {node['label']};"]
        if kind == "LabelStmt":
            result = [indent + f"{node['label']}:"]
            body = self.statement(node["body"], indent + "    ")
            result.extend(body or [indent + "    ;"])
            return result
        if kind == "BreakStmt":
            minimum = self.break_cleanup_depths[-1] if self.break_cleanup_depths else 0
            return self.active_cleanup_lines(indent, minimum) + [indent + "break;"]
        if kind == "ContinueStmt":
            minimum = (
                self.continue_cleanup_depths[-1]
                if self.continue_cleanup_depths else 0
            )
            return self.active_cleanup_lines(indent, minimum) + [indent + "continue;"]
        if kind == "NullStmt":
            return [indent + ";"]
        if "value_category" in node:
            value = self.discarded_expression(node)
            return [] if value in {"0", "(void)0"} else [indent + value + ";"]
        self.fail(node, f"unsupported statement {kind}")

    @staticmethod
    def contains_callee(node: Any, name: str) -> bool:
        if isinstance(node, dict):
            if node.get("callee_name") == name:
                return True
            return any(Emitter.contains_callee(value, name)
                       for value in node.values())
        if isinstance(node, list):
            return any(Emitter.contains_callee(value, name) for value in node)
        return False

    @staticmethod
    def fail(node: dict[str, Any], message: str) -> None:
        location = node.get("location", {})
        where = f"{location.get('file', '?')}:{location.get('line', '?')}"
        raise TranslationError(f"{where}: {message}")

    @staticmethod
    def local_reference_ids(node: Any) -> set[str]:
        result: set[str] = set()
        if isinstance(node, dict):
            if node.get("kind") == "DeclStmt":
                for declaration in node.get("declarations", []):
                    if declaration.get("type", "").strip().endswith("&"):
                        result.add(declaration["id"])
            for value in node.values():
                result.update(Emitter.local_reference_ids(value))
        elif isinstance(node, list):
            for value in node:
                result.update(Emitter.local_reference_ids(value))
        return result

    def c_parameters(self, function: dict[str, Any]) -> list[str]:
        result: list[str] = []
        if function.get("id") in self.context_threaded_functions:
            result.append("ImGuiContext *imgui_c89_ctx")
        if function.get("method") and not function.get("static"):
            parent = self.records[function["parent"]]
            result.append(f"{self.record_names_by_id[parent['id']]} *self")
        for index, parameter in enumerate(function.get("parameters", [])):
            name = parameter.get("name") or f"arg_{index}"
            parameter_type = parameter["type"]
            if (self.compact_glyph_deltas
                    and function.get("qualified_name")
                    == "UnpackAccumulativeOffsetsIntoRanges"
                    and index == 1):
                parameter_type = "const unsigned char *"
            result.append(self.c_declaration(parameter_type, name))
        if function.get("variadic"):
            result.append("...")
        return result or ["void"]

    def constructor_value_helper_active(self, identifier: str) -> bool:
        consumers = self.constructor_value_consumers[identifier]
        if not consumers:
            return False
        if self.active_function_ids is None:
            return True
        if identifier in self.inline_constructor_value_helpers:
            return bool(consumers & self.active_function_ids)
        if identifier in self.internal_constructor_value_helpers:
            return identifier in self.active_function_ids
        return True

    def emit_inline_constructor_value_helper(
        self,
        identifier: str,
        function: dict[str, Any],
        result_type: str,
        parameters: list[str],
    ) -> list[str]:
        """Emit a TU-local field-only value constructor clone."""
        saved_function = self.current_function_id
        saved_locals = self.local_names
        saved_references = self.reference_parameters
        saved_arrays = self.array_reference_ids
        self.current_function_id = identifier
        self.local_names = {
            parameter["id"]: parameter.get("name") or f"arg_{index}"
            for index, parameter in enumerate(function.get("parameters", []))
        }
        self.reference_parameters = {
            parameter["id"]
            for parameter in function.get("parameters", [])
            if parameter["type"].strip().endswith("&")
        }
        self.array_reference_ids = set()
        lines = [
            f"static {result_type} {self.constructor_helpers[identifier]}("
            f"{', '.join(parameters) or 'void'})",
            "{",
            f"    {result_type} result;",
        ]
        for initializer in function.get("initializers", []):
            field_name = self.field_names[initializer["target"]]
            lines.append(
                f"    result.{field_name} = "
                f"{self.expression(initializer['value'])};"
            )
        lines.extend(["    return result;", "}", ""])
        self.current_function_id = saved_function
        self.local_names = saved_locals
        self.reference_parameters = saved_references
        self.array_reference_ids = saved_arrays
        return lines

    def emit_enum_constant_lines(self, public: bool) -> list[str]:
        """Emit public or private constants with idiomatic C89 spellings."""
        lines: list[str] = []
        for enum in sorted(
            self.ir.get("enums", []),
            key=lambda item: item["qualified_name"],
        ):
            if (enum["id"] in self.public_enum_ids) != public:
                continue
            constants = enum.get("constants", [])
            if not constants:
                continue
            values = [int(constant["value"]) for constant in constants]
            if all(-2147483648 <= value <= 2147483647 for value in values):
                label = enum.get("qualified_name", "") or "anonymous"
                lines.extend([f"/* {label} */", "enum {"])
                for index, constant in enumerate(constants):
                    comma = "," if index + 1 < len(constants) else ""
                    lines.append(
                        f"    {self.enum_constant_names[constant['id']]} = "
                        f"{constant['value']}{comma}"
                    )
                lines.extend(["};", ""])
            else:
                # ISO C90 requires every enumerator to be representable as an
                # int. Keep the source spelling but use macros for wider
                # integer constants rather than changing the storage ABI.
                for constant in constants:
                    lines.append(
                        f"#define {self.enum_constant_names[constant['id']]} "
                        f"({constant['value']})"
                    )
                lines.append("")
        return lines

    def emit_c_internal_header(self) -> str:
        lines = [
            "/* Generated private declarations for translated C units. */",
            "#ifndef IMGUI_C89_INTERNAL_H",
            "#define IMGUI_C89_INTERNAL_H",
            '#include "imgui_c89.h"',
            "",
        ]
        lines.extend(self.internal_helper_declaration_lines)
        if self.compact_optional_modules and self.split_public_header:
            lines.extend([
                "void imgui_c89_enable_cff_module(void);",
                "void imgui_c89_enable_full_features(ImGuiContext *ctx);",
                "",
            ])
        emitted_record_names: set[str] = set()
        for record in sorted(
            self.records.values(), key=lambda item: item["qualified_name"]
        ):
            if record["id"] in self.omitted_record_ids:
                continue
            if record["id"] in self.public_forward_record_ids:
                continue
            name = self.record_names_by_id[record["id"]]
            if name in emitted_record_names:
                continue
            emitted_record_names.add(name)
            tag = "union" if record.get("union") else "struct"
            lines.append(f"typedef {tag} {name} {name};")
        lines.append("")
        emitted_typedef_names: set[str] = set()
        for item in self.ordered_typedefs():
            if self.typedef_is_public(item):
                continue
            name = _c_identifier(item["qualified_name"])
            if name in self.record_names.values() or name in emitted_typedef_names:
                continue
            emitted_typedef_names.add(name)
            underlying = item["underlying_type"]
            if "<" in underlying:
                underlying = item.get("canonical_underlying_type", underlying)
                if self.c_type(underlying) == underlying:
                    continue
            lines.append(f"typedef {self.c_declaration(underlying, name)};")
        lines.append("")
        emitted_enum_names: set[str] = set()
        typedef_names = {
            _c_identifier(item["qualified_name"]) for item in self.typedefs
        }
        for enum in sorted(
            self.ir.get("enums", []), key=lambda item: item["qualified_name"]
        ):
            if self.enum_is_public(enum):
                continue
            enum_name = (
                _c_identifier(enum.get("qualified_name", ""))
                if enum.get("name") else ""
            )
            if (enum_name and enum_name not in emitted_enum_names
                    and enum_name not in typedef_names):
                lines.append(f"typedef int {enum_name};")
                emitted_enum_names.add(enum_name)
        lines.append("")
        lines.extend(self.emit_enum_constant_lines(public=False))
        emitted_record_names = set()
        for record in self.ordered_records():
            if record["id"] in self.public_complete_record_ids:
                continue
            name = self.record_names_by_id[record["id"]]
            if name in emitted_record_names:
                continue
            emitted_record_names.add(name)
            tag = "union" if record.get("union") else "struct"
            lines.append(f"{tag} {name} {{")
            fields = self.flattened_fields(record)
            for field in fields:
                if field.get("c_comment"):
                    lines.append(f"    /* {field['c_comment']} */")
                field_name = self.field_names[field["id"]]
                declaration = self.c_field_declaration(field, field_name)
                if "bit_width" in field:
                    declaration = (
                        "IMGUI_C89_EXTENSION " + declaration
                        + f" : {field['bit_width']}"
                    )
                lines.append(f"    {declaration};")
            if not fields:
                lines.append("    unsigned char imgui_c89_empty;")
            lines.extend(["};", ""])
        for identifier, function in sorted(self.function_declarations.items()):
            if (identifier in self.flattened_span_function_ids
                    or identifier in self.internal_functions
                    or identifier in self.public_exact_c_names):
                continue
            c_name = self.function_names[identifier]
            parameters = ", ".join(self.c_parameters(function))
            lines.append(
                f"{self.c_type(function['return_type'])} "
                f"{c_name}({parameters});"
            )
        if self.external_cpp_bridge_functions():
            lines.append(
                "void imgui_c89_external_ImGuiTestEngine_AssertLog("
                "const char *expr, const char *file, "
                "const char *function, int line);"
            )
        for identifier, item in sorted(self.globals.items()):
            if not item.get("static"):
                lines.append(
                    f"extern {self.c_declaration(item['type'], self.global_names[identifier])};"
                )
        lines.extend(["#endif", ""])
        return "\n".join(lines)

    def emit_c_header(self) -> str:
        lines = [
            "/* Generated Dear ImGui C89 header. Do not edit. */",
            "#ifndef IMGUI_C89_H",
            "#define IMGUI_C89_H",
            "#ifdef IMGUI_C89_USE_CPP_TYPES",
            "/* Declaration-only mode for the generated C++ facade. The",
            " * upstream imgui.h/imgui_internal.h types must already exist. */",
            "#ifdef __cplusplus",
            'extern "C" {',
            "#endif",
        ]
        for api_name, _, function in self.exact_c_api_functions():
            lines.append(
                f"{self.cpp_c_abi_type(function['return_type'])} {api_name}"
                f"({', '.join(self.exact_c_abi_parameters(function)) or 'void'});"
            )
        lines.extend([
            "#ifdef __cplusplus",
            "}",
            "#endif",
            "#else",
            "#include <stddef.h>",
            "#include <limits.h>",
            "#include <stdio.h>",
            "#include <stdarg.h>",
            "#include <time.h>",
        ])
        lines.extend(
            f"#include <{header}>" for header in self.ir.get("c_includes", [])
        )
        lines.extend([
            "",
            "/* C89 has no standard 64-bit integer spelling. Prefer the",
            " * native long where it is wide enough and isolate the required",
            " * compiler extension on ILP32/LLP64 targets. */",
            "#if ULONG_MAX > 0xffffffffUL",
            "typedef long imgui_c89_i64;",
            "typedef unsigned long imgui_c89_u64;",
            "#elif defined(_MSC_VER)",
            "typedef __int64 imgui_c89_i64;",
            "typedef unsigned __int64 imgui_c89_u64;",
            "#elif defined(__GNUC__)",
            "__extension__ typedef long long imgui_c89_i64;",
            "__extension__ typedef unsigned long long imgui_c89_u64;",
            "#else",
            '#error "A 64-bit integer extension is required on this C89 target"',
            "#endif",
            "#if defined(__GNUC__)",
            "#define IMGUI_C89_EXTENSION __extension__",
            "#else",
            "#define IMGUI_C89_EXTENSION",
            "#endif",
            "#if defined(__clang__) || defined(__GNUC__)",
            "#define IMGUI_C89_NOINLINE __attribute__((noinline))",
            "#elif defined(_MSC_VER)",
            "#define IMGUI_C89_NOINLINE __declspec(noinline)",
            "#else",
            "#define IMGUI_C89_NOINLINE",
            "#endif",
            "#define imgui_c89_expect(condition, expected) \\",
            "    ((void)(expected), (condition))",
            "",
            "#ifdef __cplusplus",
            'extern "C" {',
            "#endif",
            "",
        ])
        internal_helper_start = len(lines)
        lines.extend([
            "void imgui_c89_debugtrap(void);",
            "void imgui_c89_strncpy(char *dst, const char *src, size_t count);",
            "int imgui_c89_stricmp(const char *a, const char *b);",
            "int imgui_c89_strnicmp(const char *a, const char *b, size_t n);",
            "const char *imgui_c89_atoi_int(const char *src, int *output);",
            "const char *imgui_c89_compressed_string_at(const unsigned char *data, const unsigned char *rules, unsigned int rule_count, int index, char *buffer);",
            "struct tm *imgui_c89_localtime_r(const time_t *clock_value, struct tm *result);",
            "void imgui_c89_assert_rtn(const char *function_name, const char *file_name, int line_number, const char *message);",
            "int imgui_c89_lerp_int(int a, int b, float t);",
            "unsigned int imgui_c89_lerp_uint(unsigned int a, unsigned int b, float t);",
            "float imgui_c89_lerp_float(float a, float b, float t);",
            "double imgui_c89_lerp_double(double a, double b, float t);",
            "int imgui_c89_min_int(int a, int b);",
            "unsigned int imgui_c89_min_uint(unsigned int a, unsigned int b);",
            "float imgui_c89_min_float(float a, float b);",
            "double imgui_c89_min_double(double a, double b);",
            "long imgui_c89_min_long(long a, long b);",
            "unsigned long imgui_c89_min_ulong(unsigned long a, unsigned long b);",
            "size_t imgui_c89_min_size(size_t a, size_t b);",
            "int imgui_c89_max_int(int a, int b);",
            "unsigned int imgui_c89_max_uint(unsigned int a, unsigned int b);",
            "float imgui_c89_max_float(float a, float b);",
            "double imgui_c89_max_double(double a, double b);",
            "long imgui_c89_max_long(long a, long b);",
            "unsigned long imgui_c89_max_ulong(unsigned long a, unsigned long b);",
            "size_t imgui_c89_max_size(size_t a, size_t b);",
            "int imgui_c89_clamp_int(int v, int lo, int hi);",
            "unsigned int imgui_c89_clamp_uint(unsigned int v, unsigned int lo, unsigned int hi);",
            "float imgui_c89_clamp_float(float v, float lo, float hi);",
            "double imgui_c89_clamp_double(double v, double lo, double hi);",
            "long imgui_c89_clamp_long(long v, long lo, long hi);",
            "unsigned long imgui_c89_clamp_ulong(unsigned long v, unsigned long lo, unsigned long hi);",
            "size_t imgui_c89_clamp_size(size_t v, size_t lo, size_t hi);",
            "",
        ])
        if self.omit_unused_compatibility_shims:
            unused_shims = (
                "imgui_c89_strncpy", "imgui_c89_stricmp",
                "imgui_c89_strnicmp", "imgui_c89_atoi_int",
                "imgui_c89_localtime_r",
            )
            lines = [
                line for line in lines
                if not any(name in line for name in unused_shims)
            ]
        if self.omit_unused_scalar_helpers:
            scalar_start = lines.index(
                "int imgui_c89_lerp_int(int a, int b, float t);"
            )
            scalar_end = lines.index(
                "size_t imgui_c89_clamp_size(size_t v, size_t lo, size_t hi);"
            ) + 1
            del lines[scalar_start:scalar_end]
        if self.compact_imvector:
            lines.extend([
                "void *imgui_c89_vector_reserve(void *context, void *data, int size, int *capacity, int new_capacity, size_t element_size, int discard);",
                "void *imgui_c89_vector_resize(void *context, void *data, int *size, int *capacity, int new_size, size_t element_size);",
                "void *imgui_c89_vector_resize_fill(void *context, void *data, int *size, int *capacity, int new_size, size_t element_size, const void *value);",
                "void *imgui_c89_vector_push_back(void *context, void *data, int *size, int *capacity, size_t element_size, const void *value);",
                "void *imgui_c89_vector_erase(void *data, int *size, size_t element_size, const void *first, const void *last);",
                "void *imgui_c89_vector_erase_unsorted(void *data, int *size, size_t element_size, const void *item);",
                "void *imgui_c89_vector_insert(void *context, void **data, int *size, int *capacity, size_t element_size, const void *item, const void *value);",
                "",
            ])
        if self.compact_imvector_accessors:
            lines.extend([
                "void *imgui_c89_vector_at(void *data, int size, int index, size_t element_size, int line);",
                "void *imgui_c89_vector_back(void *data, int size, size_t element_size, int line);",
                "void imgui_c89_vector_pop(int *size);",
                "int imgui_c89_vector_index(const void *data, int size, const void *item, size_t element_size);",
                "",
            ])
        if self.compact_imvector_lifecycle:
            lines.extend([
                "void imgui_c89_vector_destroy(void *context, void *data);",
                "void imgui_c89_vector_clear(void *context, void **data, int *size, int *capacity);",
                "",
            ])
        if self.compact_imvector_capacity:
            lines.extend([
                "int imgui_c89_vector_grow_capacity(int capacity, int size);",
                "",
            ])
        if self.compact_imchunkstream:
            lines.extend([
                "void *imgui_c89_chunk_alloc(void *context, void *stream, size_t size);",
                "void *imgui_c89_chunk_begin(void *stream);",
                "int imgui_c89_chunk_size(const void *chunk);",
                "void imgui_c89_chunk_clear(void *context, void *stream);",
                "int imgui_c89_chunk_empty(const void *stream);",
                "void *imgui_c89_chunk_end(void *stream);",
                "void *imgui_c89_chunk_next(void *stream, const void *chunk);",
                "int imgui_c89_chunk_offset(const void *stream, const void *chunk);",
                "void *imgui_c89_chunk_ptr(void *stream, int offset);",
                "int imgui_c89_chunk_size_bytes(const void *stream);",
                "void imgui_c89_chunk_swap(void *stream, void *other);",
                "",
            ])
        if self.compact_impool:
            lines.extend([
                "void *imgui_c89_pool_add_slot(void *context, void **data, int *size, int *capacity, int *free_index, int *alive_count, size_t element_size);",
                "void *imgui_c89_pool_at(void *data, int index, size_t element_size);",
                "int imgui_c89_pool_contains(const void *data, int size, const void *item, size_t element_size);",
                "int imgui_c89_pool_index(const void *data, int size, const void *item, size_t element_size);",
                "",
            ])
        if self.split_public_header:
            self.internal_helper_declaration_lines = lines[
                internal_helper_start:
            ]
            del lines[internal_helper_start:]
        emitted_record_names: set[str] = set()
        for record in sorted(self.records.values(), key=lambda item: item["qualified_name"]):
            if record["id"] not in self.public_forward_record_ids:
                continue
            name = self.record_names_by_id[record["id"]]
            if name in emitted_record_names:
                continue
            emitted_record_names.add(name)
            tag = "union" if record.get("union") else "struct"
            lines.append(f"typedef {tag} {name} {name};")
        lines.append("")
        if self.compact_optional_modules and not self.split_public_header:
            lines.extend([
                "void imgui_c89_enable_cff_module(void);",
                "void imgui_c89_enable_full_features(ImGuiContext *ctx);",
                "",
            ])
        emitted_typedef_names: set[str] = set()
        for item in self.ordered_typedefs():
            if not self.typedef_is_public(item):
                continue
            name = _c_identifier(item["qualified_name"])
            if name in self.record_names.values():
                continue
            if name in emitted_typedef_names:
                continue
            emitted_typedef_names.add(name)
            underlying = item["underlying_type"]
            if "<" in underlying:
                underlying = item.get("canonical_underlying_type", underlying)
                if self.c_type(underlying) == underlying:
                    # An unused alias alone does not instantiate its class
                    # template in C++.  There is no concrete layout to emit;
                    # actual uses will produce a specialization record and a
                    # representable alias.
                    continue
            lines.append(f"typedef {self.c_declaration(underlying, name)};")
        lines.append("")
        emitted_enum_names: set[str] = set()
        for enum in sorted(self.ir.get("enums", []), key=lambda item: item["qualified_name"]):
            if not self.enum_is_public(enum):
                continue
            enum_name = (
                _c_identifier(enum.get("qualified_name", ""))
                if enum.get("name") else ""
            )
            if (enum_name and enum_name not in emitted_enum_names
                    and enum_name not in {
                        _c_identifier(item["qualified_name"])
                        for item in self.typedefs
                    }):
                lines.append(f"typedef int {enum_name};")
                emitted_enum_names.add(enum_name)
        lines.append("")
        lines.extend(self.emit_enum_constant_lines(public=True))
        emitted_record_names = set()
        for record in self.ordered_records():
            if record["id"] not in self.public_complete_record_ids:
                continue
            name = self.record_names_by_id[record["id"]]
            if name in emitted_record_names:
                continue
            emitted_record_names.add(name)
            tag = "union" if record.get("union") else "struct"
            lines.append(f"{tag} {name} {{")
            fields = self.flattened_fields(record)
            for field in fields:
                if field.get("c_comment"):
                    lines.append(f"    /* {field['c_comment']} */")
                field_name = self.field_names[field["id"]]
                declaration = self.c_field_declaration(field, field_name)
                if "bit_width" in field:
                    declaration = (
                        "IMGUI_C89_EXTENSION " + declaration
                        + f" : {field['bit_width']}"
                    )
                lines.append(f"    {declaration};")
            if not fields:
                lines.append("    unsigned char imgui_c89_empty;")
            lines.extend(["};", ""])
        if self.split_public_header:
            public_declarations = (
                (c_name, function)
                for c_name, _, function in self.exact_c_api_functions()
            )
        else:
            public_declarations = (
                (self.function_names[identifier], function)
                for identifier, function in sorted(
                    self.function_declarations.items()
                )
                if identifier not in self.internal_functions
            )
        for c_name, function in public_declarations:
            parameters = ", ".join(self.c_parameters(function))
            lines.append(
                f"{self.c_type(function['return_type'])} "
                f"{c_name}({parameters});"
            )
        if not self.split_public_header and self.external_cpp_bridge_functions():
            lines.append(
                "void imgui_c89_external_ImGuiTestEngine_AssertLog("
                "const char *expr, const char *file, "
                "const char *function, int line);"
            )
        if not self.split_public_header:
            for identifier, item in sorted(self.globals.items()):
                if not item.get("static"):
                    lines.append(
                        f"extern {self.c_declaration(item['type'], self.global_names[identifier])};"
                    )
        lines.extend([
            "",
            "#ifdef __cplusplus",
            "}",
            "#endif",
            "",
            "#endif /* IMGUI_C89_USE_CPP_TYPES */",
            "#endif",
            "",
        ])
        return "\n".join(lines)

    def emit_c_source(self) -> str:
        self.configure_effective_reachability()
        self.configure_assert_metadata()
        mem_alloc_name = ""
        mem_free_name = ""
        mem_alloc_has_context = False
        mem_free_has_context = False
        if self.compact_imvector:
            mem_alloc_id = next(
                identifier
                for identifier, function in self.function_declarations.items()
                if function.get("qualified_name") == "ImGui::MemAlloc"
            )
            mem_free_id = next(
                identifier
                for identifier, function in self.function_declarations.items()
                if function.get("qualified_name") == "ImGui::MemFree"
            )
            mem_alloc_name = self.function_names[mem_alloc_id]
            mem_free_name = self.function_names[mem_free_id]
            mem_alloc_has_context = mem_alloc_id in self.context_threaded_functions
            mem_free_has_context = mem_free_id in self.context_threaded_functions
        lines = [
            "/* Generated by imgui-c89 translator. Do not edit. */",
            '#include "imgui_c89_internal.h"',
            "#include <assert.h>",
            "#include <ctype.h>",
            "#include <math.h>",
            "#include <signal.h>",
            "#include <stddef.h>",
            "#include <stdarg.h>",
            "#include <stdio.h>",
            "#include <stdlib.h>",
            "#include <string.h>",
            "#include <time.h>",
            "#include <sys/types.h>",
            "#include <sys/wait.h>",
            "#include <unistd.h>",
            "",
            "void imgui_c89_debugtrap(void)",
            "{",
            "#if defined(__clang__) || defined(__GNUC__)",
            "    __builtin_trap();",
            "#else",
            "    raise(SIGTRAP);",
            "#endif",
            "}",
            "",
            "void imgui_c89_strncpy(char *dst, const char *src, size_t count)",
            "{",
            "    if (count > 0) { if (count > 1) strncpy(dst, src, count - 1); dst[count - 1] = 0; }",
            "}",
            "int imgui_c89_stricmp(const char *a, const char *b)",
            "{",
            "    while (*a && *b) { int ca = tolower((unsigned char)*a); int cb = tolower((unsigned char)*b); if (ca != cb) return ca - cb; ++a; ++b; }",
            "    return tolower((unsigned char)*a) - tolower((unsigned char)*b);",
            "}",
            "int imgui_c89_strnicmp(const char *a, const char *b, size_t n)",
            "{",
            "    while (n > 0 && *a && *b) { int ca = tolower((unsigned char)*a); int cb = tolower((unsigned char)*b); if (ca != cb) return ca - cb; ++a; ++b; --n; }",
            "    return n == 0 ? 0 : tolower((unsigned char)*a) - tolower((unsigned char)*b);",
            "}",
            "",
            "const char *imgui_c89_atoi_int(const char *src, int *output)",
            "{",
            "    char *end; long value = strtol(src, &end, 10);",
            "    *output = (int)value; return end;",
            "}",
            "static char *imgui_c89_expand_string_token(unsigned int value,",
            "    const unsigned char *rules, unsigned int rule_count, char *output)",
            "{",
            "    unsigned int rule;",
            "    if (value < 128 || (value - 128) >= rule_count) { *output++ = (char)value; return output; }",
            "    rule = (value - 128) * 2;",
            "    output = imgui_c89_expand_string_token(rules[rule], rules, rule_count, output);",
            "    return imgui_c89_expand_string_token(rules[rule + 1], rules, rule_count, output);",
            "}",
            "const char *imgui_c89_compressed_string_at(const unsigned char *data,",
            "    const unsigned char *rules, unsigned int rule_count, int index, char *buffer)",
            "{",
            "    char *output = buffer;",
            "    while (index-- > 0) { while (*data != 0) ++data; ++data; }",
            "    while (*data != 0) output = imgui_c89_expand_string_token(*data++, rules, rule_count, output);",
            "    *output = 0;",
            "    return buffer;",
            "}",
            "struct tm *imgui_c89_localtime_r(const time_t *clock_value, struct tm *result)",
            "{",
            "    struct tm *value = localtime(clock_value);",
            "    if (value == 0) return 0;",
            "    *result = *value; return result;",
            "}",
            "",
            "void imgui_c89_assert_rtn(const char *function_name,",
            "    const char *file_name, int line_number, const char *message)",
            "{",
            "    (void)function_name; (void)file_name; (void)line_number;",
            "    (void)message;",
            "    assert(0);",
            "}",
            "",
            "int imgui_c89_lerp_int(int a, int b, float t)",
            "{ return (int)(a + (b - a) * t); }",
            "unsigned int imgui_c89_lerp_uint(unsigned int a, unsigned int b, float t)",
            "{ return (unsigned int)(a + (b - a) * t); }",
            "float imgui_c89_lerp_float(float a, float b, float t)",
            "{ return a + (b - a) * t; }",
            "double imgui_c89_lerp_double(double a, double b, float t)",
            "{ return a + (b - a) * t; }",
            "int imgui_c89_min_int(int a, int b) { return a < b ? a : b; }",
            "unsigned int imgui_c89_min_uint(unsigned int a, unsigned int b) { return a < b ? a : b; }",
            "float imgui_c89_min_float(float a, float b) { return a < b ? a : b; }",
            "double imgui_c89_min_double(double a, double b) { return a < b ? a : b; }",
            "long imgui_c89_min_long(long a, long b) { return a < b ? a : b; }",
            "unsigned long imgui_c89_min_ulong(unsigned long a, unsigned long b) { return a < b ? a : b; }",
            "size_t imgui_c89_min_size(size_t a, size_t b) { return a < b ? a : b; }",
            "int imgui_c89_max_int(int a, int b) { return a > b ? a : b; }",
            "unsigned int imgui_c89_max_uint(unsigned int a, unsigned int b) { return a > b ? a : b; }",
            "float imgui_c89_max_float(float a, float b) { return a > b ? a : b; }",
            "double imgui_c89_max_double(double a, double b) { return a > b ? a : b; }",
            "long imgui_c89_max_long(long a, long b) { return a > b ? a : b; }",
            "unsigned long imgui_c89_max_ulong(unsigned long a, unsigned long b) { return a > b ? a : b; }",
            "size_t imgui_c89_max_size(size_t a, size_t b) { return a > b ? a : b; }",
            "int imgui_c89_clamp_int(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }",
            "unsigned int imgui_c89_clamp_uint(unsigned int v, unsigned int lo, unsigned int hi) { return v < lo ? lo : (v > hi ? hi : v); }",
            "float imgui_c89_clamp_float(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }",
            "double imgui_c89_clamp_double(double v, double lo, double hi) { return v < lo ? lo : (v > hi ? hi : v); }",
            "long imgui_c89_clamp_long(long v, long lo, long hi) { return v < lo ? lo : (v > hi ? hi : v); }",
            "unsigned long imgui_c89_clamp_ulong(unsigned long v, unsigned long lo, unsigned long hi) { return v < lo ? lo : (v > hi ? hi : v); }",
            "size_t imgui_c89_clamp_size(size_t v, size_t lo, size_t hi) { return v < lo ? lo : (v > hi ? hi : v); }",
            "",
        ]
        if self.omit_unused_compatibility_shims:
            signatures = (
                "void imgui_c89_strncpy(char *dst, const char *src, size_t count)",
                "int imgui_c89_stricmp(const char *a, const char *b)",
                "int imgui_c89_strnicmp(const char *a, const char *b, size_t n)",
                "const char *imgui_c89_atoi_int(const char *src, int *output)",
                "struct tm *imgui_c89_localtime_r(const time_t *clock_value, struct tm *result)",
            )
            for signature in signatures:
                start = lines.index(signature)
                depth = 0
                end = start
                while end < len(lines):
                    depth += lines[end].count("{") - lines[end].count("}")
                    if end > start and depth == 0:
                        break
                    end += 1
                if end == len(lines):
                    raise TranslationError(
                        "unused compatibility shim body changed shape"
                    )
                del lines[start:end + 1]
        if self.omit_unused_scalar_helpers:
            scalar_start = lines.index(
                "int imgui_c89_lerp_int(int a, int b, float t)"
            )
            scalar_end = lines.index(
                "size_t imgui_c89_clamp_size(size_t v, size_t lo, size_t hi) { return v < lo ? lo : (v > hi ? hi : v); }"
            ) + 1
            del lines[scalar_start:scalar_end]
        if self.compact_imvector:
            mem_alloc_context = "imgui_c89_ctx, " if mem_alloc_has_context else ""
            mem_free_context = "imgui_c89_ctx, " if mem_free_has_context else ""
            lines.extend([
                "IMGUI_C89_NOINLINE void *imgui_c89_vector_reserve(",
                "    void *imgui_c89_ctx, void *data, int size, int *capacity, int new_capacity,",
                "    size_t element_size, int discard)",
                "{",
                "    void *new_data;",
                "    if (new_capacity <= *capacity) return data;",
                f"    new_data = {mem_alloc_name}({mem_alloc_context}(size_t)new_capacity * element_size);",
                "    if (data != 0) {",
                "        if (!discard) memcpy(new_data, data, (size_t)size * element_size);",
                f"        {mem_free_name}({mem_free_context}data);",
                "    }",
                "    *capacity = new_capacity;",
                "    return new_data;",
                "}",
                "IMGUI_C89_NOINLINE void *imgui_c89_vector_resize(",
                "    void *imgui_c89_ctx, void *data, int *size, int *capacity, int new_size,",
                "    size_t element_size)",
                "{",
                "    if (new_size > *capacity) {",
                "        int grown = *capacity ? *capacity + *capacity / 2 : 8;",
                "        if (grown < new_size) grown = new_size;",
                "        data = imgui_c89_vector_reserve(imgui_c89_ctx, data, *size, capacity, grown, element_size, 0);",
                "    }",
                "    *size = new_size;",
                "    return data;",
                "}",
                "IMGUI_C89_NOINLINE void *imgui_c89_vector_resize_fill(",
                "    void *imgui_c89_ctx, void *data, int *size, int *capacity, int new_size,",
                "    size_t element_size, const void *value)",
                "{",
                "    int index = *size;",
                "    data = imgui_c89_vector_resize(imgui_c89_ctx, data, size, capacity, new_size, element_size);",
                "    while (index < new_size) {",
                "        memcpy((unsigned char *)data + (size_t)index * element_size, value, element_size);",
                "        ++index;",
                "    }",
                "    return data;",
                "}",
                "IMGUI_C89_NOINLINE void *imgui_c89_vector_push_back(",
                "    void *imgui_c89_ctx, void *data, int *size, int *capacity, size_t element_size,",
                "    const void *value)",
                "{",
                "    int index = *size;",
                "    data = imgui_c89_vector_resize(imgui_c89_ctx, data, size, capacity, index + 1, element_size);",
                "    memcpy((unsigned char *)data + (size_t)index * element_size, value, element_size);",
                "    return data;",
                "}",
                "IMGUI_C89_NOINLINE void *imgui_c89_vector_erase(",
                "    void *data, int *size, size_t element_size,",
                "    const void *first, const void *last)",
                "{",
                "    size_t offset = (size_t)((const unsigned char *)first - (const unsigned char *)data) / element_size;",
                "    size_t count = (size_t)((const unsigned char *)last - (const unsigned char *)first) / element_size;",
                "    memmove((unsigned char *)data + offset * element_size,",
                "        (const unsigned char *)data + (offset + count) * element_size,",
                "        ((size_t)*size - offset - count) * element_size);",
                "    *size -= (int)count;",
                "    return (unsigned char *)data + offset * element_size;",
                "}",
                "IMGUI_C89_NOINLINE void *imgui_c89_vector_erase_unsorted(",
                "    void *data, int *size, size_t element_size, const void *item)",
                "{",
                "    size_t offset = (size_t)((const unsigned char *)item - (const unsigned char *)data) / element_size;",
                "    --*size;",
                "    if (offset < (size_t)*size)",
                "        memcpy((unsigned char *)data + offset * element_size,",
                "            (const unsigned char *)data + (size_t)*size * element_size,",
                "            element_size);",
                "    return (unsigned char *)data + offset * element_size;",
                "}",
                "IMGUI_C89_NOINLINE void *imgui_c89_vector_insert(",
                "    void *imgui_c89_ctx, void **data, int *size, int *capacity, size_t element_size,",
                "    const void *item, const void *value)",
                "{",
                "    size_t offset = *data != 0",
                "        ? (size_t)((const unsigned char *)item - (const unsigned char *)*data) / element_size",
                "        : 0;",
                "    if (*size == *capacity) {",
                "        int grown = *capacity ? *capacity + *capacity / 2 : 8;",
                "        *data = imgui_c89_vector_reserve(imgui_c89_ctx, *data, *size, capacity,",
                "            grown, element_size, 0);",
                "    }",
                "    if (offset < (size_t)*size)",
                "        memmove((unsigned char *)*data + (offset + 1) * element_size,",
                "            (const unsigned char *)*data + offset * element_size,",
                "            ((size_t)*size - offset) * element_size);",
                "    memcpy((unsigned char *)*data + offset * element_size, value, element_size);",
                "    ++*size;",
                "    return (unsigned char *)*data + offset * element_size;",
                "}",
                "",
            ])
        if self.compact_imvector_accessors:
            accessor_lines = self.imvector_accessor_lines
            lines.extend([
                "IMGUI_C89_NOINLINE void *imgui_c89_vector_at(",
                "    void *data, int size, int index, size_t element_size, int line)",
                "{",
                "    if (imgui_c89_expect(!(index >= 0 && index < size), 0)) {",
                *self.imvector_assert_runtime_lines(
                    "        ", "i >= 0 && i < Size", "line"
                ),
                "    }",
                "    return (unsigned char *)data + (size_t)index * element_size;",
                "}",
                "IMGUI_C89_NOINLINE void *imgui_c89_vector_back(",
                "    void *data, int size, size_t element_size, int line)",
                "{",
                "    if (imgui_c89_expect(!(size > 0), 0)) {",
                *self.imvector_assert_runtime_lines(
                    "        ", "Size > 0", "line"
                ),
                "    }",
                "    return (unsigned char *)data + (size_t)(size - 1) * element_size;",
                "}",
                "IMGUI_C89_NOINLINE void imgui_c89_vector_pop(int *size)",
                "{",
                "    if (imgui_c89_expect(!(*size > 0), 0)) {",
                *self.imvector_assert_runtime_lines(
                    "        ", "Size > 0",
                    str(accessor_lines["pop_back"]),
                ),
                "    }",
                "    --*size;",
                "}",
                "IMGUI_C89_NOINLINE int imgui_c89_vector_index(",
                "    const void *data, int size, const void *item, size_t element_size)",
                "{",
                "    const unsigned char *bytes = (const unsigned char *)data;",
                "    const unsigned char *position = (const unsigned char *)item;",
                "    if (imgui_c89_expect(!(position >= bytes",
                "            && position < bytes + (size_t)size * element_size), 0)) {",
                *self.imvector_assert_runtime_lines(
                    "        ", "it >= Data && it < Data + Size",
                    str(accessor_lines["index_from_ptr"]),
                ),
                "    }",
                "    return (int)((position - bytes) / element_size);",
                "}",
                "",
            ])
        if self.compact_imvector_lifecycle:
            lines.extend([
                "IMGUI_C89_NOINLINE void imgui_c89_vector_destroy(",
                "    void *imgui_c89_ctx, void *data)",
                "{",
                "    if (data != 0)",
                f"        {mem_free_name}({mem_free_context}data);",
                "}",
                "IMGUI_C89_NOINLINE void imgui_c89_vector_clear(",
                "    void *imgui_c89_ctx, void **data, int *size, int *capacity)",
                "{",
                "    if (*data != 0) {",
                "        *size = *capacity = 0;",
                f"        {mem_free_name}({mem_free_context}*data);",
                "        *data = 0;",
                "    }",
                "}",
                "",
            ])
        if self.compact_imvector_capacity:
            lines.extend([
                "IMGUI_C89_NOINLINE int imgui_c89_vector_grow_capacity(",
                "    int capacity, int size)",
                "{",
                "    int grown = capacity ? capacity + capacity / 2 : 8;",
                "    return grown > size ? grown : size;",
                "}",
                "",
            ])
        if self.compact_imchunkstream:
            lines.extend([
                "typedef struct imgui_c89_chunk_stream {",
                "    int size; int capacity; unsigned char *data;",
                "} imgui_c89_chunk_stream;",
                "IMGUI_C89_NOINLINE void *imgui_c89_chunk_alloc(",
                "    void *imgui_c89_ctx, void *stream_pointer, size_t size)",
                "{",
                "    imgui_c89_chunk_stream *stream = (imgui_c89_chunk_stream *)stream_pointer;",
                "    int offset = stream->size;",
                "    size = (size + 7) & ~(size_t)3;",
                "    stream->data = (unsigned char *)imgui_c89_vector_resize(",
                "        imgui_c89_ctx, stream->data, &stream->size, &stream->capacity,",
                "        offset + (int)size, 1);",
                "    *(int *)(void *)(stream->data + offset) = (int)size;",
                "    return stream->data + offset + 4;",
                "}",
                "void *imgui_c89_chunk_begin(void *stream_pointer)",
                "{",
                "    imgui_c89_chunk_stream *stream = (imgui_c89_chunk_stream *)stream_pointer;",
                "    return stream->data ? stream->data + 4 : 0;",
                "}",
                "int imgui_c89_chunk_size(const void *chunk)",
                "{ return ((const int *)chunk)[-1]; }",
                "IMGUI_C89_NOINLINE void imgui_c89_chunk_clear(",
                "    void *imgui_c89_ctx, void *stream_pointer)",
                "{",
                "    imgui_c89_chunk_stream *stream = (imgui_c89_chunk_stream *)stream_pointer;",
                "    if (stream->data != 0)",
                f"        {mem_free_name}({mem_free_context}stream->data);",
                "    stream->size = stream->capacity = 0; stream->data = 0;",
                "}",
                "int imgui_c89_chunk_empty(const void *stream_pointer)",
                "{ return ((const imgui_c89_chunk_stream *)stream_pointer)->size == 0; }",
                "void *imgui_c89_chunk_end(void *stream_pointer)",
                "{",
                "    imgui_c89_chunk_stream *stream = (imgui_c89_chunk_stream *)stream_pointer;",
                "    return stream->data + stream->size;",
                "}",
                "IMGUI_C89_NOINLINE void *imgui_c89_chunk_next(",
                "    void *stream_pointer, const void *chunk)",
                "{",
                "    imgui_c89_chunk_stream *stream = (imgui_c89_chunk_stream *)stream_pointer;",
                "    unsigned char *begin = stream->data ? stream->data + 4 : 0;",
                "    unsigned char *end = stream->data + stream->size;",
                "    unsigned char *next = (unsigned char *)chunk;",
                "    if (!(next >= begin && next < end))",
                "        imgui_c89_assert_rtn(\"\", \"imgui_internal.h\", 816, \"p >= begin() && p < end()\");",
                "    next += ((const int *)chunk)[-1];",
                "    if (next == end + 4) return 0;",
                "    if (!(next < end))",
                "        imgui_c89_assert_rtn(\"\", \"imgui_internal.h\", 816, \"p < end()\");",
                "    return next;",
                "}",
                "IMGUI_C89_NOINLINE int imgui_c89_chunk_offset(",
                "    const void *stream_pointer, const void *chunk)",
                "{",
                "    const imgui_c89_chunk_stream *stream = (const imgui_c89_chunk_stream *)stream_pointer;",
                "    const unsigned char *begin = stream->data ? stream->data + 4 : 0;",
                "    const unsigned char *value = (const unsigned char *)chunk;",
                "    if (!(value >= begin && value < stream->data + stream->size))",
                "        imgui_c89_assert_rtn(\"\", \"imgui_internal.h\", 819, \"p >= begin() && p < end()\");",
                "    return (int)(value - stream->data);",
                "}",
                "IMGUI_C89_NOINLINE void *imgui_c89_chunk_ptr(",
                "    void *stream_pointer, int offset)",
                "{",
                "    imgui_c89_chunk_stream *stream = (imgui_c89_chunk_stream *)stream_pointer;",
                "    if (!(offset >= 4 && offset < stream->size))",
                "        imgui_c89_assert_rtn(\"\", \"imgui_internal.h\", 820, \"off >= 4 && off < Buf.Size\");",
                "    return stream->data + offset;",
                "}",
                "int imgui_c89_chunk_size_bytes(const void *stream_pointer)",
                "{ return ((const imgui_c89_chunk_stream *)stream_pointer)->size; }",
                "IMGUI_C89_NOINLINE void imgui_c89_chunk_swap(void *a_pointer, void *b_pointer)",
                "{",
                "    imgui_c89_chunk_stream temporary = *(imgui_c89_chunk_stream *)a_pointer;",
                "    *(imgui_c89_chunk_stream *)a_pointer = *(imgui_c89_chunk_stream *)b_pointer;",
                "    *(imgui_c89_chunk_stream *)b_pointer = temporary;",
                "}",
                "",
            ])
        if self.compact_impool:
            lines.extend([
                "IMGUI_C89_NOINLINE void *imgui_c89_pool_add_slot(",
                "    void *imgui_c89_ctx, void **data, int *size, int *capacity,",
                "    int *free_index, int *alive_count, size_t element_size)",
                "{",
                "    int index = *free_index;",
                "    if (index == *size) {",
                "        *data = imgui_c89_vector_resize(imgui_c89_ctx, *data, size,",
                "            capacity, *size + 1, element_size);",
                "        ++*free_index;",
                "    } else {",
                "        *free_index = *(int *)((unsigned char *)*data",
                "            + (size_t)index * element_size);",
                "    }",
                "    ++*alive_count;",
                "    return (unsigned char *)*data + (size_t)index * element_size;",
                "}",
                "void *imgui_c89_pool_at(void *data, int index, size_t element_size)",
                "{ return (unsigned char *)data + (size_t)index * element_size; }",
                "int imgui_c89_pool_contains(",
                "    const void *data, int size, const void *item, size_t element_size)",
                "{",
                "    const unsigned char *begin = (const unsigned char *)data;",
                "    const unsigned char *value = (const unsigned char *)item;",
                "    return value >= begin && value < begin + (size_t)size * element_size;",
                "}",
                "IMGUI_C89_NOINLINE int imgui_c89_pool_index(",
                "    const void *data, int size, const void *item, size_t element_size)",
                "{",
                "    const unsigned char *begin = (const unsigned char *)data;",
                "    const unsigned char *value = (const unsigned char *)item;",
                "    if (!(value >= begin && value < begin + (size_t)size * element_size))",
                "        imgui_c89_assert_rtn(\"\", \"imgui_internal.h\", 784,",
                "            \"p >= Buf.Data && p < Buf.Data + Buf.Size\");",
                "    return (int)((size_t)(value - begin) / element_size);",
                "}",
                "",
            ])
        if not self.emit_runtime_support:
            lines = lines[:15]
        lines.extend(self.emit_assert_metadata_support())
        style_color_ids = {
            identifier
            for identifier, function in self.functions.items()
            if function.get("qualified_name") in {
                "ImGui::StyleColorsClassic",
                "ImGui::StyleColorsDark",
                "ImGui::StyleColorsLight",
            }
        }
        if (self.compact_style_colors
                and (self.active_function_ids is None
                     or style_color_ids & self.active_function_ids)):
            lines.extend([
                "static IMGUI_C89_NOINLINE void imgui_c89_apply_style_palette(",
                "    ImGuiStyle *style, const float *components,",
                "    const unsigned char (*palette)[4], int color_count)",
                "{",
                "    int i;",
                "    for (i = 0; i < color_count; ++i) {",
                "        style->Colors[i].x = components[palette[i][0]];",
                "        style->Colors[i].y = components[palette[i][1]];",
                "        style->Colors[i].z = components[palette[i][2]];",
                "        style->Colors[i].w = components[palette[i][3]];",
                "    }",
                "}",
                "",
            ])
        cpp_facade_destructor_ids = {
            identifier
            for identifier, function in self.exact_wrapper_functions()
            if function.get("destructor")
        }
        for identifier, function in sorted(self.functions.items()):
            if identifier in self.flattened_span_function_ids:
                continue
            if identifier not in self.internal_functions:
                continue
            if (self.active_function_ids is not None
                    and identifier not in self.active_function_ids):
                continue
            parameters = ", ".join(self.c_parameters(function))
            lines.append(
                f"static {self.c_type(function['return_type'])} "
                f"{self.function_names[identifier]}({parameters});"
            )
        if self.internal_functions:
            lines.append("")
        for identifier, function in sorted(self.functions.items()):
            if identifier in self.flattened_span_function_ids:
                continue
            if not function.get("constructor") or function.get("parent") not in self.records:
                continue
            record = self.records[function["parent"]]
            result_type = self.record_names_by_id[record["id"]]
            original_parameters = []
            for index, parameter in enumerate(function.get("parameters", [])):
                parameter_name = parameter.get("name") or f"arg_{index}"
                original_parameters.append(
                    self.c_declaration(parameter["type"], parameter_name)
                )
            parameters = (
                ["ImGuiContext *imgui_c89_ctx", *original_parameters]
                if identifier in self.context_threaded_functions
                else original_parameters
            )
            if self.constructor_value_helper_active(identifier):
                value_internal = (
                    identifier in self.internal_constructor_value_helpers
                    or identifier in self.inline_constructor_value_helpers
                )
                prefix = "static " if value_internal else ""
                lines.append(
                    f"{prefix}{result_type} {self.constructor_helpers[identifier]}({', '.join(parameters) or 'void'});"
                )
            at_parameters = (
                ["ImGuiContext *imgui_c89_ctx", "void *memory", *original_parameters]
                if identifier in self.context_threaded_functions
                else ["void *memory", *original_parameters]
            )
            at_used = bool(self.constructor_at_consumers[identifier])
            at_internal = identifier in self.internal_constructor_at_helpers
            if at_used and (
                not at_internal
                or self.active_function_ids is None
                or identifier in self.active_function_ids
            ):
                prefix = "static " if at_internal else ""
                lines.append(
                    f"{prefix}{result_type} *{self.constructor_at_helpers[identifier]}("
                    + ", ".join(at_parameters) + ");"
                )
        lines.append("")
        helper_insert_at = len(lines)
        emitted_packed_char_global = False
        packed_char_maximum = 0
        for identifier, item in sorted(self.globals.items()):
            if (self.active_global_ids is not None
                    and identifier not in self.active_global_ids):
                continue
            if identifier in self.compact_crc32_global_ids:
                continue
            if identifier in self.compact_localization_global_ids:
                continue
            prefix = "static " if item.get("static") else ""
            if identifier in self.compact_cursor_global_ids:
                values = self.compact_cursor_values(item)
                lines.append(
                    f"{prefix}const unsigned char {self.global_names[identifier]}[66] = {{ "
                    + ", ".join(str(value) for value in values) + " };"
                )
                continue
            if identifier in self.compressed_string_pointer_global_ids:
                values = [
                    self._unwrap(value)
                    for value in item.get("initializer", {}).get("values", [])
                ]
                if not values or any(
                    value.get("kind") != "StringLiteral" for value in values
                ):
                    self.fail(
                        item,
                        "compressed string pointer global is not a string array",
                    )
                strings = [value["value"] for value in values]
                data, rules, maximum = self.compress_string_table(strings)
                name = self.global_names[identifier]
                lines.append(
                    f"{prefix}const unsigned char {name}_data[] = {{ "
                    + ", ".join(str(value) for value in data) + " };"
                )
                lines.append(
                    f"{prefix}const unsigned char {name}_rules[] = {{ "
                    + ", ".join(
                        str(value) for pair in rules for value in pair
                    ) + " };"
                )
                lines.append(
                    f"{prefix}const unsigned int {name}_rule_count = "
                    f"{len(rules)};"
                )
                lines.append(
                    f"{prefix}char {name}_buffer[{maximum + 1}];"
                )
                continue
            if identifier in self.packed_string_pointer_global_ids:
                values = [
                    self._unwrap(value)
                    for value in item.get("initializer", {}).get("values", [])
                ]
                if not values or any(
                    value.get("kind") != "StringLiteral" for value in values
                ):
                    self.fail(item, "packed string pointer global is not a string array")
                strings = [value["value"] for value in values]
                string_offsets: dict[str, int] = {}
                blob = ""
                for string in sorted(
                    set(strings), key=lambda value: (-len(value), value)
                ):
                    needle = string + "\0"
                    offset = blob.find(needle)
                    if offset < 0:
                        offset = len(blob)
                        blob += needle
                    string_offsets[string] = offset
                offsets = [string_offsets[string] for string in strings]
                if len(blob) > 65535:
                    self.fail(item, "packed string pointer global exceeds 16-bit offsets")
                name = self.global_names[identifier]
                lines.append(
                    f"{prefix}const unsigned short {name}_offsets[] = {{ "
                    + ", ".join(str(offset) for offset in offsets) + " };"
                )
                lines.append(
                    f"{prefix}const char {name}_data[] = "
                    + self._string_literal(blob) + ";"
                )
                continue
            if identifier in self.packed_char_global_ids:
                initializer_node = item.get("initializer", {})
                values = initializer_node.get("values", [])
                if (len(values) != 1
                        or values[0].get("kind") != "StringLiteral"):
                    self.fail(item, "packed character global is not one string")
                source = values[0]["value"]
                alphabet = " .-X"
                if any(character not in alphabet for character in source):
                    self.fail(item, "packed character global uses more than four characters")
                packed: list[int] = []
                for offset in range(0, len(source), 4):
                    byte = 0
                    for shift, character in enumerate(source[offset:offset + 4]):
                        byte |= alphabet.index(character) << (shift * 2)
                    packed.append(byte)
                encoded, symbols, rules = self.compress_byte_stream(packed)
                name = self.global_names[identifier]
                lines.append(
                    f"{prefix}const unsigned char {name}[] = "
                    "{ " + ", ".join(str(value) for value in encoded) + " };"
                )
                lines.append(
                    f"{prefix}const unsigned char {name}_symbols[] = "
                    "{ " + ", ".join(str(value) for value in symbols) + " };"
                )
                lines.append(
                    f"{prefix}const unsigned char {name}_rules[] = "
                    "{ " + ", ".join(
                        str(value) for pair in rules for value in pair
                    ) + " };"
                )
                emitted_packed_char_global = True
                packed_char_maximum = max(packed_char_maximum, len(packed))
                continue
            initializer = ""
            if "initializer" in item:
                initializer = " = " + self.c_initializer(item["initializer"])
            lines.append(
                f"{prefix}{self.c_declaration(item['type'], self.global_names[identifier])}{initializer};"
            )
        if emitted_packed_char_global:
            render_id = next(
                identifier
                for identifier, function in self.function_declarations.items()
                if function.get("qualified_name")
                == "ImFontAtlasBuildRenderBitmapFromString"
            )
            render_name = self.function_names[render_id]
            lines.extend([
                "static void imgui_c89_expand_packed_token(",
                "    unsigned char token, const unsigned char *symbols,",
                "    int symbol_count, const unsigned char *rules,",
                "    unsigned char *output, int *output_count)",
                "{",
                "    if ((int)token < symbol_count) {",
                "        output[(*output_count)++] = symbols[token];",
                "    } else {",
                "        int rule = ((int)token - symbol_count) * 2;",
                "        imgui_c89_expand_packed_token(rules[rule], symbols, symbol_count, rules, output, output_count);",
                "        imgui_c89_expand_packed_token(rules[rule + 1], symbols, symbol_count, rules, output, output_count);",
                "    }",
                "}",
                "static void imgui_c89_render_packed_2bit(",
                "    ImFontAtlas *atlas, int x, int y, int w, int h,",
                "    const unsigned char *encoded, int encoded_count,",
                "    const unsigned char *symbols, int symbol_count,",
                "    const unsigned char *rules, char marker)",
                "{",
                "    static const char alphabet[4] = { ' ', '.', '-', 'X' };",
                f"    unsigned char packed[{packed_char_maximum}];",
                "    char row[122];",
                "    int encoded_index;",
                "    int packed_count = 0;",
                "    int row_index;",
                "    int column;",
                "    int offset;",
                "    for (encoded_index = 0; encoded_index < encoded_count; ++encoded_index)",
                "        imgui_c89_expand_packed_token(encoded[encoded_index], symbols, symbol_count, rules, packed, &packed_count);",
                "    for (row_index = 0; row_index < h; ++row_index) {",
                "        for (column = 0; column < w; ++column) {",
                "            offset = row_index * w + column;",
                "            row[column] = alphabet[(packed[offset >> 2] >> ((offset & 3) * 2)) & 3];",
                "        }",
                f"        {render_name}(atlas, x, y + row_index, w, 1, row, marker);",
                "    }",
                "}",
                "",
            ])
        compact_crc_active = self.compact_crc32 and any(
            function.get("qualified_name") in {"ImHashData", "ImHashStr"}
            and (
                self.active_function_ids is None
                or identifier in self.active_function_ids
            )
            for identifier, function in self.functions.items()
        )
        if compact_crc_active:
            lines.extend([
                "static const ImU32 imgui_c89_crc32_nibbles[16] = {",
                "    0x00000000UL, 0x105ec76fUL, 0x20bd8edeUL, 0x30e349b1UL,",
                "    0x417b1dbcUL, 0x5125dad3UL, 0x61c69362UL, 0x7198540dUL,",
                "    0x82f63b78UL, 0x92a8fc17UL, 0xa24bb5a6UL, 0xb21572c9UL,",
                "    0xc38d26c4UL, 0xd3d3e1abUL, 0xe330a81aUL, 0xf36e6f75UL",
                "};",
                "static ImU32 imgui_c89_crc32_byte(ImU32 crc, unsigned char value)",
                "{",
                "    crc ^= value;",
                "    crc = (crc >> 4) ^ imgui_c89_crc32_nibbles[crc & 15];",
                "    return (crc >> 4) ^ imgui_c89_crc32_nibbles[crc & 15];",
                "}",
                "",
            ])
        for identifier, (owner, declaration) in sorted(self.static_locals.items()):
            if (self.active_function_ids is not None
                    and owner not in self.active_function_ids):
                continue
            name = self.static_local_names[identifier]
            if identifier in self.compact_utf8_local_ids:
                values = self.compact_utf8_values(declaration)
                lines.append(
                    f"static const unsigned char {name}[5] = {{ "
                    + ", ".join(str(value) for value in values) + " };"
                )
                continue
            if identifier in self.compact_separator_local_ids:
                values = self.compact_separator_values(declaration)
                lines.append(
                    f"static const unsigned short {name}[29] = {{ "
                    + ", ".join(str(value) for value in values) + " };"
                )
                continue
            if identifier in self.compact_color_table_local_ids:
                offsets, blob = self.compact_color_table_data(declaration)
                lines.append(
                    f"static const unsigned short {name}_offsets[{len(offsets)}] = {{ "
                    + ", ".join(str(offset) for offset in offsets) + " };"
                )
                lines.append(
                    f"static const char {name}_data[] = "
                    + self._string_literal(blob) + ";"
                )
                continue
            if identifier in self.compact_glyph_delta_local_ids:
                encoded = self.compact_glyph_delta_data(declaration)
                lines.append(
                    f"static const unsigned char {name}[{len(encoded)}] = {{ "
                    + ", ".join(str(value) for value in encoded) + " };"
                )
                continue
            local_type = declaration["type"]
            if "<" in local_type:
                local_type = declaration.get("canonical_type", local_type)
            runtime = self.static_initializer_requires_runtime(
                declaration.get("initializer")
            )
            if runtime:
                local_type = declaration.get("unqualified_type", local_type)
            initializer = ""
            if "initializer" in declaration and not runtime:
                initializer = " = " + self.c_initializer(
                    declaration["initializer"]
                )
            lines.append(
                f"static {self.c_declaration(local_type, name)}{initializer};"
            )
            if runtime:
                lines.append(f"static int {name}__imgui_c89_initialized;")
        if self.globals or self.static_locals:
            lines.append("")
        for identifier, function in sorted(self.functions.items()):
            if identifier in self.flattened_span_function_ids:
                continue
            if not function.get("constructor"):
                continue
            helper = self.constructor_helpers[identifier]
            parent = self.records.get(function.get("parent"))
            if not parent:
                continue
            result_type = self.record_names_by_id[parent["id"]]
            parameters = []
            arguments = []
            if identifier in self.context_threaded_functions:
                parameters.append("ImGuiContext *imgui_c89_ctx")
            for index, parameter in enumerate(function.get("parameters", [])):
                name = parameter.get("name") or f"arg_{index}"
                parameters.append(f"{self.c_type(parameter['type'])} {name}")
                arguments.append(name)
            if (
                identifier in self.inline_constructor_value_helpers
                and self.constructor_value_helper_active(identifier)
            ):
                lines.extend(self.emit_inline_constructor_value_helper(
                    identifier, function, result_type, parameters
                ))
            owner_active = (
                self.active_function_ids is None
                or identifier in self.active_function_ids
            )
            if (
                owner_active
                and self.constructor_value_consumers[identifier]
                and identifier not in self.inline_constructor_value_helpers
            ):
                prefix = (
                    "static "
                    if identifier in self.internal_constructor_value_helpers
                    else ""
                )
                lines.append(
                    f"{prefix}{result_type} {helper}({', '.join(parameters) or 'void'})"
                )
                lines.append("{")
                lines.append(f"    {result_type} result;")
                values = ["&result"] + arguments
                if identifier in self.context_threaded_functions:
                    values.insert(0, "imgui_c89_ctx")
                lines.append(f"    {self.function_names[identifier]}({', '.join(values)});")
                lines.append("    return result;")
                lines.extend(["}", ""])
            at_helper = self.constructor_at_helpers[identifier]
            original_parameters = (
                parameters[1:]
                if identifier in self.context_threaded_functions else parameters
            )
            at_parameters = (
                ["ImGuiContext *imgui_c89_ctx", "void *memory", *original_parameters]
                if identifier in self.context_threaded_functions
                else ["void *memory", *original_parameters]
            )
            if owner_active and self.constructor_at_consumers[identifier]:
                prefix = (
                    "static "
                    if identifier in self.internal_constructor_at_helpers
                    else ""
                )
                lines.append(
                    f"{prefix}{result_type} *{at_helper}({', '.join(at_parameters)})"
                )
                lines.append("{")
                lines.append(f"    {result_type} *result = ({result_type} *)memory;")
                values = ["result"] + arguments
                if identifier in self.context_threaded_functions:
                    values.insert(0, "imgui_c89_ctx")
                lines.append(f"    {self.function_names[identifier]}({', '.join(values)});")
                lines.append("    return result;")
                lines.extend(["}", ""])
        for identifier, function in sorted(self.functions.items()):
            if identifier in self.flattened_span_function_ids:
                continue
            if (self.active_function_ids is not None
                    and identifier not in self.active_function_ids):
                continue
            self.reference_parameters = {
                parameter["id"]
                for parameter in function.get("parameters", [])
                if parameter["type"].strip().endswith("&")
            }
            self.reference_parameters.update(
                self.local_reference_ids(function.get("body", {}))
            )
            self.array_reference_ids = {
                parameter["id"] for parameter in function.get("parameters", [])
                if "(&)" in parameter["type"]
            }
            self.array_reference_ids.update(
                declaration["id"]
                for declaration in self.collect_local_declarations(
                    function.get("body", {})
                )
                if "(&)" in declaration.get("type", "")
            )
            self.reference_parameters.update(self.array_reference_ids)
            self.current_function_id = identifier
            c_name = self.function_names[identifier]
            parameters = ", ".join(self.c_parameters(function))
            self.current_return_type = function["return_type"]
            self.expression_temporaries = []
            self.cleanup_scopes = []
            self.break_cleanup_depths = []
            self.continue_cleanup_depths = []
            self.function_exit_cleanup = []
            if (function.get("destructor")
                    and function.get("parent") in self.records
                    and not self.is_flattened_table_pool_function(function)):
                self.function_exit_cleanup = self.record_subobject_cleanup_lines(
                    self.records[function["parent"]], "(*self)", "    "
                )
            self.current_compact_nav_key_ranges = bool(
                self.compact_nav_key_ranges
                and function.get("qualified_name") == "ImGui::NavUpdate"
            )
            compact_crc_body = (
                self.compact_crc32_body(function.get("qualified_name", ""))
                if self.compact_crc32 else None
            )
            compact_style_body = self.compact_style_colors_body(function)
            compact_name_switch_body = self.compact_name_switch_body(function)
            compact_truetype_body = self.compact_truetype_only_body(function)
            compact_vector_body = self.compact_imvector_body(function)
            compact_chunk_body = self.compact_imchunkstream_body(function)
            compact_pool_body = self.compact_impool_body(function)
            compact_checkbox_body = self.compact_checkbox_flags_body(function)
            compact_source_name_body = self.compact_input_source_name_body(
                function
            )
            compact_cursor = self.compact_cursor_body(function)
            compact_separator = self.compact_separator_body(function)
            compact_glyph_delta = self.compact_glyph_delta_body(function)
            compact_cff_stack_guard = self.compact_cff_stack_guard_body(function)
            compact_nav_overlay_setup = self.compact_nav_overlay_setup_body(function)
            compact_nav_direction_loop = self.compact_nav_direction_loop_body(function)
            compact_key_char_mask = self.compact_key_char_mask_body(function)
            compact_initialize = self.compact_optional_settings_body(
                function, compact_key_char_mask
            )
            if compact_initialize is None:
                compact_initialize = compact_key_char_mask
            compact_initialize = self.compact_localization_body(
                function, compact_initialize
            )
            compact_optional_cff = self.compact_optional_cff_body(function)
            compact_nav_wrapping = self.compact_nav_wrapping_body(function)
            handwritten_body = self.handwritten_function_body(function)
            local_lines = (
                [] if (handwritten_body is not None
                       or compact_crc_body is not None
                       or compact_style_body is not None
                       or compact_name_switch_body is not None
                       or compact_vector_body is not None
                       or compact_chunk_body is not None
                       or compact_pool_body is not None
                       or compact_checkbox_body is not None
                       or compact_source_name_body is not None)
                else self.prepare_locals(function)
            )
            if compact_cff_stack_guard is not None:
                local_lines.extend(compact_cff_stack_guard[0])
            if compact_nav_overlay_setup is not None:
                local_lines.extend(compact_nav_overlay_setup[0])
            if compact_nav_direction_loop is not None:
                local_lines.extend(compact_nav_direction_loop[0])
            if compact_initialize is not None:
                local_lines = compact_initialize[0]
            if compact_cursor is not None:
                local_lines = compact_cursor[0]
                self.expression_temporaries = []
            if compact_separator is not None:
                local_lines = compact_separator[0]
            if compact_glyph_delta is not None:
                local_lines = compact_glyph_delta[0]
            if compact_optional_cff is not None:
                local_lines = compact_optional_cff[0]
            if compact_nav_wrapping is not None:
                local_lines.extend(compact_nav_wrapping[0])
            if self.current_compact_nav_key_ranges:
                local_lines.insert(0, "    ImGuiKey key;")
            body_lines: list[str] = []
            self.current_compact_zero_constructor = bool(
                function.get("constructor")
                and function.get("qualified_name")
                in self.compact_zero_constructor_names
            )
            self.current_constructor_initialized_fields = {
                initializer.get("name", "")
                for initializer in function.get("initializers", [])
                if not initializer.get("base")
            }
            if self.current_compact_zero_constructor:
                body_lines.append("    memset(self, 0, sizeof(*self));")
            if handwritten_body is not None:
                body_lines.extend(handwritten_body)
            elif compact_crc_body is not None:
                body_lines.extend(compact_crc_body)
            elif compact_style_body is not None:
                body_lines.extend(compact_style_body)
            elif compact_name_switch_body is not None:
                body_lines.extend(compact_name_switch_body)
            elif compact_truetype_body is not None:
                body_lines.extend(compact_truetype_body)
            elif compact_vector_body is not None:
                body_lines.extend(compact_vector_body)
            elif compact_chunk_body is not None:
                body_lines.extend(compact_chunk_body)
            elif compact_pool_body is not None:
                body_lines.extend(compact_pool_body)
            elif compact_checkbox_body is not None:
                body_lines.extend(compact_checkbox_body)
            elif compact_source_name_body is not None:
                body_lines.extend(compact_source_name_body)
            elif compact_cursor is not None:
                body_lines.extend(compact_cursor[1])
            elif compact_separator is not None:
                body_lines.extend(compact_separator[1])
            elif compact_glyph_delta is not None:
                body_lines.extend(compact_glyph_delta[1])
            elif compact_cff_stack_guard is not None:
                body_lines.extend(compact_cff_stack_guard[1])
            elif compact_nav_overlay_setup is not None:
                body_lines.extend(compact_nav_overlay_setup[1])
            elif compact_nav_direction_loop is not None:
                body_lines.extend(compact_nav_direction_loop[1])
            elif compact_initialize is not None:
                body_lines.extend(compact_initialize[1])
            elif compact_optional_cff is not None:
                body_lines.extend(compact_optional_cff[1])
            elif compact_nav_wrapping is not None:
                body_lines.extend(compact_nav_wrapping[1])
            elif function.get("constructor"):
                for initializer in function.get("initializers", []):
                    if (initializer.get("target") in self.flattened_table_span_fields
                            or initializer.get("target")
                            in self.flattened_table_vector_fields):
                        continue
                    if initializer.get("base"):
                        value = initializer["value"]
                        if value.get("kind") == "CXXConstructExpr":
                            constructor = self.function_names.get(value.get("constructor"))
                            if constructor:
                                arguments = [f"({self.c_type(initializer['name'])} *)self"]
                                arguments.extend(self.call_arguments(
                                    value.get("constructor"), value.get("arguments", [])
                                ))
                                arguments = self.with_context_argument(
                                    value.get("constructor"), arguments
                                )
                                body_lines.append(f"    {constructor}({', '.join(arguments)});")
                                continue
                            if value.get("trivial"):
                                continue
                        self.fail(value, "unresolved base initializer")
                    value = initializer["value"]
                    target = f"self->{initializer['name']}"
                    if value.get("kind") == "CXXConstructExpr":
                        arguments = value.get("arguments", [])
                        array_match = re.match(
                            r"^(.*)\[(\d+)\]$", value.get("type", "")
                        )
                        if array_match and not arguments:
                            index_name = (
                                "imgui_c89_array_index_"
                                + str(len(self.expression_temporaries))
                            )
                            self.expression_temporaries.append((index_name, "int"))
                            body_lines.append(
                                f"    memset(&{target}, 0, sizeof({target}));"
                            )
                            element_constructor = self.function_names.get(
                                value.get("constructor")
                            )
                            if element_constructor:
                                count = array_match.group(2)
                                element_arguments = self.with_context_argument(
                                    value.get("constructor"),
                                    [f"&{target}[{index_name}]"],
                                )
                                body_lines.append(
                                    f"    for ({index_name} = 0; {index_name} < {count}; ++{index_name}) {{"
                                )
                                body_lines.append(
                                    f"        {element_constructor}({', '.join(element_arguments)});"
                                )
                                body_lines.append("    }")
                            continue
                        constructor = self.function_names.get(value.get("constructor"))
                        if constructor:
                            call_arguments = [f"&{target}"]
                            call_arguments.extend(self.call_arguments(
                                value.get("constructor"), arguments
                            ))
                            call_arguments = self.with_context_argument(
                                value.get("constructor"), call_arguments
                            )
                            body_lines.append(f"    {constructor}({', '.join(call_arguments)});")
                            continue
                        if len(arguments) == 1:
                            body_lines.append(f"    {target} = {self.expression(arguments[0])};")
                            continue
                        if not arguments:
                            record = self.records_by_spelling.get(
                                self.base_spelling(value.get("type", ""))
                            )
                            if record:
                                body_lines.extend(
                                    self.implicit_default_construction(
                                        target, record, "    "
                                    )
                                )
                                continue
                    body_lines.append(f"    {target} = {self.expression(value)};")
            if (handwritten_body is None
                    and compact_crc_body is None and compact_style_body is None
                    and compact_name_switch_body is None
                    and compact_truetype_body is None
                    and compact_vector_body is None
                    and compact_chunk_body is None
                    and compact_pool_body is None
                    and compact_checkbox_body is None
                    and compact_source_name_body is None
                    and compact_cursor is None
                    and compact_separator is None
                    and compact_glyph_delta is None
                    and compact_cff_stack_guard is None
                    and compact_nav_overlay_setup is None
                    and compact_nav_direction_loop is None
                    and compact_initialize is None
                    and compact_optional_cff is None
                    and compact_nav_wrapping is None):
                body_lines.extend(self.statement(function["body"]))
            prefix = "static " if identifier in self.internal_functions else ""
            if (identifier in self.compact_checkbox_flag_helpers.values()
                    or identifier in self.noinline_function_ids):
                prefix += "IMGUI_C89_NOINLINE "
            split_cpp_facade_destructor = (
                function.get("destructor")
                and identifier in cpp_facade_destructor_ids
            )
            if split_cpp_facade_destructor:
                if prefix:
                    raise TranslationError(
                        "exact C++ facade destructor was made internal"
                    )
                lines.append(
                    f"{self.c_type(function['return_type'])} "
                    f"{self.cpp_facade_destructor_name(identifier)}({parameters})"
                )
            else:
                if function.get("destructor"):
                    body_lines.extend(self.function_exit_cleanup)
                lines.append(
                    f"{prefix}{self.c_type(function['return_type'])} "
                    f"{c_name}({parameters})"
                )
            lines.append("{")
            for temporary_name, temporary_type in self.expression_temporaries:
                lines.append(
                    f"    {self.c_declaration(temporary_type, temporary_name)};"
                )
            if self.expression_temporaries:
                lines.append("")
            lines.extend(local_lines)
            lines.extend(body_lines)
            lines.extend(["}", ""])
            if split_cpp_facade_destructor:
                arguments: list[str] = []
                if identifier in self.context_threaded_functions:
                    arguments.append("imgui_c89_ctx")
                if function.get("method") and not function.get("static"):
                    arguments.append("self")
                arguments.extend(
                    parameter.get("name") or f"arg_{index}"
                    for index, parameter in enumerate(
                        function.get("parameters", [])
                    )
                )
                lines.extend([
                    f"{self.c_type(function['return_type'])} "
                    f"{c_name}({parameters})",
                    "{",
                    f"    {self.cpp_facade_destructor_name(identifier)}("
                    + ", ".join(arguments) + ");",
                    *self.function_exit_cleanup,
                    "}",
                    "",
                ])
            self.current_compact_zero_constructor = False
            self.current_constructor_initialized_fields = set()
            self.cleanup_scopes = []
            self.function_exit_cleanup = []
        self.current_function_id = None
        self.reference_parameters = set()
        helper_lines: list[str] = self.emit_optional_module_helpers()
        if (
            self.compact_nav_overlay_selectable
            and self.nav_overlay_function_id is not None
            and (
                self.active_function_ids is None
                or self.nav_overlay_function_id in self.active_function_ids
            )
        ):
            function_ids = self.nav_overlay_helper_function_ids

            def helper_call(key: str, arguments: list[str]) -> str:
                identifier = function_ids[key]
                if identifier in self.context_threaded_functions:
                    arguments = ["imgui_c89_ctx", *arguments]
                return (
                    f"{self.function_names[identifier]}("
                    + ", ".join(arguments) + ")"
                )

            header_constant = next(
                self.enum_constant_names[constant["id"]]
                for enum in self.ir.get("enums", [])
                for constant in enum.get("constants", [])
                if constant.get("name") == "ImGuiCol_Header"
            )
            helper_lines.extend([
                "static unsigned char imgui_c89_nav_overlay_selectable(",
                "    ImGuiContext *imgui_c89_ctx, const char *label, unsigned char selected)",
                "{",
                "    ImGuiContext *g = imgui_c89_ctx;",
                "    ImGuiWindow *window = g->CurrentWindow;",
                "    ImGuiID id;",
                "    const char *label_end;",
                "    ImVec2 label_size;",
                "    ImVec2 size;",
                "    ImVec2 pos;",
                "    ImVec2 pos_max;",
                "    ImRect bb;",
                "    float spacing_l;",
                "    float spacing_u;",
                "    if (window->SkipItems) return 0;",
                "    id = " + helper_call("get_id", ["window", "label", "0"]) + ";",
                "    label_end = " + helper_call("find_text_end", ["label", "0"]) + ";",
                "    label_size = " + helper_call(
                    "calc_text_size", ["label", "label_end", "0", "-1.0f"]
                ) + ";",
                "    size = label_size;",
                "    pos = window->DC.CursorPos;",
                "    pos.y += window->DC.CurrLineTextBaseOffset;",
                "    " + helper_call("item_size", ["&size", "0.0f"]) + ";",
                "    if (size.x < window->WorkRect.Max.x - pos.x)",
                "        size.x = window->WorkRect.Max.x - pos.x;",
                "    bb.Min = pos;",
                "    bb.Max.x = pos.x + size.x;",
                "    bb.Max.y = pos.y + size.y;",
                "    spacing_l = (float)(int)(g->Style.ItemSpacing.x * 0.5f);",
                "    spacing_u = (float)(int)(g->Style.ItemSpacing.y * 0.5f);",
                "    bb.Min.x -= spacing_l;",
                "    bb.Min.y -= spacing_u;",
                "    bb.Max.x += g->Style.ItemSpacing.x - spacing_l;",
                "    bb.Max.y += g->Style.ItemSpacing.y - spacing_u;",
                "    if (!" + helper_call(
                    "item_add", ["&bb", "id", "0", "0"]
                ) + ") return 0;",
                "    if (selected)",
                "        " + helper_call("render_frame", [
                    "bb.Min", "bb.Max",
                    helper_call("get_color", [header_constant, "1.0f"]),
                    "0", "g->Style.SelectableRounding",
                ]) + ";",
                "    if (pos.x + size.x > window->WorkRect.Max.x)",
                "        size.x = window->WorkRect.Max.x - pos.x;",
                "    pos_max.x = pos.x + size.x;",
                "    pos_max.y = pos.y + size.y;",
                "    " + helper_call("render_text", [
                    "&pos", "&pos_max", "label", "label_end", "&label_size",
                    "&g->Style.SelectableTextAlign", "&bb",
                ]) + ";",
                "    return 0;",
                "}",
                "",
            ])
        for name, function in sorted(self.lambda_helpers.values(), key=lambda item: item[0]):
            self.current_function_id = function.get("id")
            parameters = []
            for index, parameter in enumerate(function.get("parameters", [])):
                parameter_name = parameter.get("name") or f"arg_{index}"
                parameters.append(self.c_declaration(parameter["type"], parameter_name))
            self.current_return_type = function["return_type"]
            self.reference_parameters = {
                parameter["id"] for parameter in function.get("parameters", [])
                if parameter["type"].strip().endswith("&")
            }
            self.array_reference_ids = {
                parameter["id"] for parameter in function.get("parameters", [])
                if "(&)" in parameter["type"]
            }
            self.reference_parameters.update(self.array_reference_ids)
            self.expression_temporaries = []
            self.cleanup_scopes = []
            self.break_cleanup_depths = []
            self.continue_cleanup_depths = []
            self.function_exit_cleanup = []
            local_lines = self.prepare_locals(function)
            body_lines = self.statement(function["body"])
            helper_lines.append(
                f"static {self.c_type(function['return_type'])} {name}({', '.join(parameters) or 'void'})"
            )
            helper_lines.append("{")
            for temporary_name, temporary_type in self.expression_temporaries:
                helper_lines.append(
                    f"    {self.c_declaration(temporary_type, temporary_name)};"
                )
            if self.expression_temporaries:
                helper_lines.append("")
            helper_lines.extend(local_lines)
            helper_lines.extend(body_lines)
            helper_lines.extend(["}", ""])
        self.current_function_id = None
        self.cleanup_scopes = []
        self.function_exit_cleanup = []
        for name, result_type, fields, values in sorted(
            self.aggregate_helpers.values(), key=lambda item: item[0]
        ):
            parameters = [
                self.c_declaration(value["type"], f"value_{index}")
                for index, value in enumerate(values)
            ]
            helper_lines.append(
                f"static {result_type} {name}({', '.join(parameters) or 'void'})"
            )
            helper_lines.append("{")
            helper_lines.append(f"    {result_type} result;")
            helper_lines.append("    memset(&result, 0, sizeof(result));")
            for index, field_name in enumerate(fields[:len(values)]):
                target = f"result.{field_name}"
                if "[" in values[index].get("type", ""):
                    helper_lines.append(
                        f"    memcpy({target}, value_{index}, sizeof({target}));"
                    )
                else:
                    helper_lines.append(f"    {target} = value_{index};")
            helper_lines.append("    return result;")
            helper_lines.extend(["}", ""])
            helper_lines.append(
                f"static {result_type} *{name}_at(void *memory{', ' if parameters else ''}{', '.join(parameters)})"
            )
            helper_lines.append("{")
            helper_lines.append(f"    {result_type} *result = ({result_type} *)memory;")
            helper_lines.append(
                f"    *result = {name}({', '.join(f'value_{index}' for index in range(len(values)))});"
            )
            helper_lines.append("    return result;")
            helper_lines.extend(["}", ""])
        lines[helper_insert_at:helper_insert_at] = helper_lines
        return "\n".join(lines)

    def cpp_type(self, spelling: str) -> str:
        value = spelling.strip()
        for qualified in sorted(
            (item["qualified_name"] for item in self.records.values()),
            key=len,
            reverse=True,
        ):
            if value.startswith(qualified):
                value = value.replace(qualified, qualified.split("::")[-1], 1)
        return value

    def cpp_default(self, node: dict[str, Any]) -> str:
        kind = node.get("kind")
        if kind == "FloatingLiteral":
            return self._float_literal(node)
        if kind == "IntegerLiteral":
            return node["value"]
        if kind == "CXXBoolLiteralExpr":
            return "true" if node["value"] else "false"
        if kind in {"CXXNullPtrLiteralExpr", "GNUNullExpr"}:
            return "0"
        if kind == "CharacterLiteral":
            return str(node["value"])
        if kind == "StringLiteral":
            return self._string_literal(node["value"])
        if kind == "DeclRefExpr":
            return node["name"]
        if kind in {
            "ImplicitCastExpr", "ParenExpr", "MaterializeTemporaryExpr",
            "ExprWithCleanups", "ConstantExpr", "CXXDefaultArgExpr",
            "CXXBindTemporaryExpr", "CXXDefaultInitExpr",
        }:
            child = node.get("operand")
            if child is None and len(node.get("children", [])) == 1:
                child = node["children"][0]
            if child is not None:
                return self.cpp_default(child)
        if kind in {"BinaryOperator", "CompoundAssignOperator"}:
            return f"({self.cpp_default(node['lhs'])} {node['opcode']} {self.cpp_default(node['rhs'])})"
        if kind == "UnaryOperator":
            operand = self.cpp_default(node["operand"])
            return f"({operand}{node['opcode']})" if node.get("postfix") else f"({node['opcode']}{operand})"
        if kind in {"CXXConstructExpr", "CXXTemporaryObjectExpr"}:
            arguments = ", ".join(self.cpp_default(value) for value in node.get("arguments", []))
            return f"{self.cpp_type(node['type'])}({arguments})"
        if kind in {
            "CStyleCastExpr", "CXXStaticCastExpr", "CXXFunctionalCastExpr",
            "CXXConstCastExpr", "CXXReinterpretCastExpr",
        }:
            return f"(({self.cpp_type(node['type'])})({self.cpp_default(node['operand'])}))"
        if kind == "ConditionalOperator":
            return f"({self.cpp_default(node['condition'])} ? {self.cpp_default(node['true'])} : {self.cpp_default(node['false'])})"
        if kind == "UnaryExprOrTypeTraitExpr" and node.get("operator") == "sizeof":
            if "argument_type" in node:
                return f"sizeof({self.cpp_type(node['argument_type'])})"
            return f"sizeof({self.cpp_default(node['argument'])})"
        self.fail(node, f"unsupported C++ default argument {kind}")

    def cpp_parameters(self, function: dict[str, Any], defaults: bool) -> str:
        values = []
        for index, parameter in enumerate(function.get("parameters", [])):
            name = parameter.get("name") or f"arg_{index}"
            value = f"{self.cpp_type(parameter['type'])} {name}"
            if defaults and "default" in parameter:
                value += f" = {self.cpp_default(parameter['default'])}"
            values.append(value)
        return ", ".join(values)

    @staticmethod
    def cpp_declaration(spelling: str, name: str) -> str:
        value = spelling.strip()
        if "(*)" in value:
            return value.replace("(*)", f"(*{name})", 1)
        array = re.match(r"^(.*?)(\[[^\]]*\].*)$", value)
        if array:
            return f"{array.group(1).strip()} {name}{array.group(2)}"
        return f"{value} {name}"

    @staticmethod
    def cpp_c_abi_type(spelling: str) -> str:
        value = spelling.strip()
        reference = value.endswith("&")
        if reference:
            value = value[:-2].rstrip() if value.endswith("&&") else value[:-1].rstrip()
        value = re.sub(r"\bbool\b", "unsigned char", value)
        if reference:
            value += " *"
        return value

    def cpp_c_abi_declaration(self, spelling: str, name: str) -> str:
        return self.cpp_declaration(self.cpp_c_abi_type(spelling), name)

    def exact_wrapper_functions(self) -> list[tuple[str, dict[str, Any]]]:
        result = []
        for identifier, function in sorted(self.functions.items()):
            if identifier in self.flattened_span_function_ids:
                continue
            source = function.get("location", {}).get("file", "")
            if not source.endswith((".cpp", ".mm")):
                continue
            if not any(
                path.endswith((".h", ".hpp"))
                for path in function.get("declaration_files", [])
            ):
                continue
            if function.get("static") and not function.get("method"):
                continue
            result.append((identifier, function))
        return result

    def cpp_facade_destructor_name(self, identifier: str) -> str:
        return self.function_names[identifier] + "_body"

    def external_cpp_bridge_functions(self) -> list[tuple[str, dict[str, Any]]]:
        """C++ declarations called by translated C through a generated ABI bridge."""
        return [
            (identifier, function)
            for identifier, function in sorted(
                self.function_declarations.items()
            )
            if not function.get("definition")
            and function.get("qualified_name", "").startswith(
                "ImGuiTestEngine"
            )
        ]

    def exact_wrapper_static_data(self) -> list[dict[str, Any]]:
        """Return out-of-line record static-data definitions visible to C++.

        The C core uses a mechanically named copy of each global.  A C++
        client nevertheless needs the original mangled symbol when an inline
        upstream header method directly names a static data member.  Record
        membership is structural here; this deliberately does not guess from
        capitalization or maintain a list of known Dear ImGui symbols.
        """
        public_record_names = {
            item["qualified_name"] for item in self.records.values()
            if item.get("location", {}).get("file", "").endswith((".h", ".hpp"))
        }
        result = []
        for item in self.globals.values():
            qualified = item.get("qualified_name", "")
            parent, separator, _ = qualified.rpartition("::")
            source = item.get("location", {}).get("file", "")
            if (separator and parent in public_record_names
                    and source.endswith((".cpp", ".mm"))):
                result.append(item)
        return sorted(result, key=lambda item: item["qualified_name"])

    def exact_cpp_initializer(self, node: dict[str, Any]) -> str:
        if node.get("kind") == "InitListExpr":
            values = ", ".join(
                self.cpp_default(value) for value in node.get("values", [])
            )
            return "{ " + values + " }"
        return self.cpp_default(node)

    def exact_cpp_parameters(self, function: dict[str, Any]) -> str:
        values = [
            self.cpp_declaration(
                parameter["type"], parameter.get("name") or f"arg_{index}"
            )
            for index, parameter in enumerate(function.get("parameters", []))
        ]
        if function.get("variadic"):
            values.append("...")
        return ", ".join(values)

    def exact_call_arguments(self, function: dict[str, Any]) -> list[str]:
        values = []
        for index, parameter in enumerate(function.get("parameters", [])):
            name = parameter.get("name") or f"arg_{index}"
            spelling = parameter["type"].strip()
            value = f"&{name}" if spelling.endswith("&") else name
            if re.search(r"\bbool\b", spelling) and (
                "*" in spelling or spelling.endswith("&")
            ):
                target = self.cpp_c_abi_type(spelling)
                value = f"reinterpret_cast<{target}>({value})"
            values.append(value)
        return values

    def exact_c_abi_parameters(self, function: dict[str, Any]) -> list[str]:
        """C ABI parameters spelled with upstream C++ types when available."""
        parameters = []
        if function["id"] in self.context_threaded_functions:
            parameters.append("ImGuiContext *imgui_c89_ctx")
        if function.get("method") and not function.get("static"):
            parent = self.records[function["parent"]]["qualified_name"]
            parameters.append(f"{parent} *self")
        for index, parameter in enumerate(function.get("parameters", [])):
            name = parameter.get("name") or f"arg_{index}"
            abi_type = parameter["type"]
            if "<" in abi_type:
                abi_type = parameter.get("canonical_type", abi_type)
            parameters.append(self.cpp_c_abi_declaration(abi_type, name))
        if function.get("variadic"):
            parameters.append("...")
        return parameters

    def emit_exact_cpp_source(self) -> str:
        functions = self.exact_wrapper_functions()
        external_bridges = self.external_cpp_bridge_functions()
        current_context_id = next(
            (identifier for identifier, function in self.functions.items()
             if function.get("qualified_name") == "ImGui::GetCurrentContext"),
            None,
        )
        current_context_call = (
            self.function_names[current_context_id] + "()"
            if current_context_id is not None else "NULL"
        )
        backend_headers = sorted({
            "backends/" + Path(path).name
            for _, function in functions
            for path in function.get("declaration_files", [])
            if "/backends/" in path.replace("\\", "/")
            and path.endswith((".h", ".hpp"))
        })
        lines = [
            "/* Generated exact Dear ImGui C++ facade. Do not edit. */",
            '#include "imgui.h"',
            '#include "imgui_internal.h"',
            *(f'#include "{header}"' for header in backend_headers),
            "#include <stdarg.h>",
            "#include <stdio.h>",
            "#include <stdlib.h>",
            "#define IMGUI_C89_USE_CPP_TYPES",
            '#include "imgui_c89.h"',
            "",
            'extern "C" {',
        ]
        for identifier, function in functions:
            if identifier in self.public_exact_c_names:
                continue
            parameters = self.exact_c_abi_parameters(function)
            lines.append(
                f"{self.cpp_c_abi_type(function['return_type'])} "
                f"{(self.cpp_facade_destructor_name(identifier) if function.get('destructor') else self.function_names[identifier])}"
                f"({', '.join(parameters) or 'void'});"
            )
        if self.compact_optional_modules:
            lines.append(
                "void imgui_c89_enable_full_features(ImGuiContext *ctx);"
            )
        for identifier, function in external_bridges:
            parameters = []
            for index, parameter in enumerate(function.get("parameters", [])):
                name = parameter.get("name") or f"arg_{index}"
                parameters.append(
                    self.cpp_c_abi_declaration(parameter["type"], name)
                )
            if function.get("variadic"):
                parameters.append("...")
            lines.append(
                f"{self.cpp_c_abi_type(function['return_type'])} "
                f"{self.function_names[identifier]}"
                f"({', '.join(parameters) or 'void'});"
            )
        if external_bridges:
            lines.append(
                "void imgui_c89_external_ImGuiTestEngine_AssertLog("
                "const char *expr, const char *file, const char *function, "
                "int line);"
            )
        lines.extend(["}", ""])

        for identifier, function in external_bridges:
            parameters = []
            arguments = []
            for index, parameter in enumerate(function.get("parameters", [])):
                name = parameter.get("name") or f"arg_{index}"
                parameters.append(
                    self.cpp_c_abi_declaration(parameter["type"], name)
                )
                arguments.append(
                    f"*{name}" if parameter["type"].strip().endswith("&")
                    else name
                )
            c_name = self.function_names[identifier]
            if function.get("variadic"):
                fmt_name = parameters and (
                    function.get("parameters", [])[-1].get("name")
                    or f"arg_{len(parameters) - 1}"
                )
                lines.extend([
                    f'extern "C" void {c_name}('
                    f"{', '.join(parameters)}, ...)",
                    "{",
                    "    char imgui_c89_message[4096];",
                    "    va_list imgui_c89_args;",
                    f"    va_start(imgui_c89_args, {fmt_name});",
                    f"    vsnprintf(imgui_c89_message, sizeof(imgui_c89_message), {fmt_name}, imgui_c89_args);",
                    "    va_end(imgui_c89_args);",
                    f"    ::{function['qualified_name']}({arguments[0]}, \"%s\", imgui_c89_message);",
                    "}", "",
                ])
                continue
            call = (
                f"::{function['qualified_name']}({', '.join(arguments)})"
            )
            lines.append(
                f'extern "C" {function["return_type"]} {c_name}('
                f"{', '.join(parameters) or 'void'})"
            )
            lines.append("{")
            lines.append(
                f"    {'return ' if function['return_type'] != 'void' else ''}"
                f"{call};"
            )
            lines.extend(["}", ""])
        if external_bridges:
            lines.extend([
                'extern "C" void imgui_c89_external_ImGuiTestEngine_AssertLog(',
                "    const char *expr, const char *file, const char *function, int line)",
                "{",
                "    ::ImGuiTestEngine_AssertLog(expr, file, function, line);",
                "}", "",
            ])

        for item in self.exact_wrapper_static_data():
            declaration = self.cpp_declaration(
                item["type"], item["qualified_name"]
            )
            initializer = ""
            if "initializer" in item:
                initializer = " = " + self.exact_cpp_initializer(item["initializer"])
            lines.extend([declaration + initializer + ";", ""])

        by_qualified = {
            function["qualified_name"]: function
            for _, function in functions
        }
        for identifier, function in functions:
            parameters = self.exact_cpp_parameters(function)
            qualified = function["qualified_name"]
            if function.get("constructor"):
                parent = self.records[function["parent"]]["qualified_name"]
                signature = f"{parent}::{function['name']}({parameters})"
                initializers = []
                for initializer in function.get("initializers", []):
                    value = initializer["value"]
                    arguments = value.get("arguments", [])
                    if arguments:
                        target = initializer["name"]
                        values = ", ".join(self.cpp_default(item) for item in arguments)
                        initializers.append(f"{target}({values})")
                    elif value.get("kind") not in {
                        "CXXConstructExpr", "CXXTemporaryObjectExpr"
                    }:
                        initializers.append(
                            f"{initializer['name']}({self.cpp_default(value)})"
                        )
                if initializers:
                    signature += " : " + ", ".join(initializers)
            elif function.get("destructor"):
                parent = self.records[function["parent"]]["qualified_name"]
                signature = f"{parent}::~{function['name'].lstrip('~')}({parameters})"
            else:
                conversion = function.get("conversion")
                prefix = "" if conversion else function["return_type"] + " "
                signature = prefix + qualified + f"({parameters})"
                if function.get("const"):
                    signature += " const"
            lines.extend([signature, "{"])

            call_arguments = self.exact_call_arguments(function)
            if function.get("method") and not function.get("static"):
                parent = self.records[function["parent"]]["qualified_name"]
                self_value = (
                    f"const_cast<{parent} *>(this)"
                    if function.get("const") else "this"
                )
                call_arguments.insert(0, self_value)
            if identifier in self.context_threaded_functions:
                call_arguments.insert(0, current_context_call)

            if function.get("variadic"):
                candidates = [qualified + "V", qualified + "v"]
                target = next((by_qualified.get(name) for name in candidates if name in by_qualified), None)
                if target is None:
                    lines.extend([
                        "    /* No va_list sibling was present in this upstream revision. */",
                        "    abort();",
                    ])
                    if function["return_type"] != "void":
                        lines.append(f"    return {function['return_type']}();")
                    lines.extend(["}", ""])
                    continue
                named = function.get("parameters", [])
                if not named:
                    self.fail(function, "variadic wrapper has no va_start parameter")
                last = named[-1].get("name") or f"arg_{len(named) - 1}"
                args_name = "imgui_c89_args"
                lines.append(f"    va_list {args_name};")
                lines.append(f"    va_start({args_name}, {last});")
                forwarded = [
                    parameter.get("name") or f"arg_{index}"
                    for index, parameter in enumerate(function.get("parameters", []))
                ] + [args_name]
                target_call = target["qualified_name"] + f"({', '.join(forwarded)})"
                if function["return_type"] == "void":
                    lines.append(f"    {target_call};")
                    lines.append(f"    va_end({args_name});")
                else:
                    lines.append(f"    {function['return_type']} result = {target_call};")
                    lines.append(f"    va_end({args_name});")
                    lines.append("    return result;")
                lines.extend(["}", ""])
                continue

            call_name = (
                self.cpp_facade_destructor_name(identifier)
                if function.get("destructor")
                else self.function_names[identifier]
            )
            call = call_name + f"({', '.join(call_arguments)})"
            if (self.compact_optional_modules
                    and qualified == "ImGui::CreateContext"):
                if function["return_type"] != "ImGuiContext *":
                    raise TranslationError(
                        "optional-module CreateContext signature changed"
                    )
                lines.append(f"    ImGuiContext *result = {call};")
                lines.append("    imgui_c89_enable_full_features(result);")
                lines.append("    return result;")
            elif function["return_type"].strip().endswith("&"):
                lines.append(f"    return *{call};")
            elif (re.search(r"\bbool\b", function["return_type"])
                    and "*" in function["return_type"]):
                lines.append(
                    f"    return reinterpret_cast<{function['return_type']}>({call});"
                )
            elif function["return_type"] == "void" or function.get("constructor") or function.get("destructor"):
                lines.append(f"    {call};")
            else:
                lines.append(f"    return {call};")
            lines.extend(["}", ""])
        return "\n".join(lines)

    def cpp_call_arguments(self, function: dict[str, Any]) -> list[str]:
        result = []
        for index, parameter in enumerate(function.get("parameters", [])):
            name = parameter.get("name") or f"arg_{index}"
            spelling = parameter["type"].strip()
            if spelling.endswith("&"):
                c_type = self.c_type(spelling).replace(" *", "")
                result.append(f"reinterpret_cast<{c_type} *>(&{name})")
            else:
                result.append(name)
        return result

    @staticmethod
    def append_cpp_scope(lines: list[str], namespace: str, body: list[str]) -> None:
        parts = [part for part in namespace.split("::") if part]
        for part in parts:
            lines.extend([f"namespace {part}", "{"])
        lines.extend(body)
        for _ in reversed(parts):
            lines.append("}")
        lines.append("")

    def emit_cpp_header(self) -> str:
        lines = [
            "/* Generated Dear ImGui C++ source facade. Do not edit. */",
            "#ifndef IMGUI_TRANSLATED_CPP_H",
            "#define IMGUI_TRANSLATED_CPP_H",
            '#include "imgui_c89.h"',
            "",
        ]
        methods_by_parent: dict[str, list[tuple[str, dict[str, Any]]]] = {}
        free_functions: list[tuple[str, dict[str, Any]]] = []
        for identifier, function in self.functions.items():
            if function.get("method"):
                methods_by_parent.setdefault(function["parent"], []).append((identifier, function))
            else:
                free_functions.append((identifier, function))

        for record in sorted(self.records.values(), key=lambda item: item["qualified_name"]):
            body = [f"struct {record['name']}", "{"]
            for field in record["fields"]:
                body.append(f"    {self.cpp_type(field['type'])} {field['name']};")
            for _, function in sorted(methods_by_parent.get(record["id"], [])):
                params = self.cpp_parameters(function, defaults=True)
                if function.get("constructor"):
                    body.append(f"    {record['name']}({params});")
                elif function.get("destructor"):
                    body.append(f"    ~{record['name']}();")
                else:
                    suffix = " const" if function.get("const") else ""
                    body.append(
                        f"    {self.cpp_type(function['return_type'])} {function['name']}({params}){suffix};"
                    )
            body.append("};")
            self.append_cpp_scope(lines, record.get("namespace", ""), body)

        for _, function in sorted(free_functions):
            self.append_cpp_scope(
                lines,
                function.get("namespace", ""),
                [
                    f"{self.cpp_type(function['return_type'])} {function['name']}"
                    f"({self.cpp_parameters(function, defaults=True)});"
                ],
            )

        for identifier, function in sorted(self.functions.items()):
            c_name = self.function_names[identifier]
            params = self.cpp_parameters(function, defaults=False)
            args = self.cpp_call_arguments(function)
            if function.get("method") and not function.get("static"):
                record = self.records[function["parent"]]
                raw = self.record_names_by_id[record["id"]]
                if function.get("const"):
                    cast = (
                        f"reinterpret_cast<{raw} *>("
                        f"const_cast<{record['name']} *>(this))"
                    )
                else:
                    cast = f"reinterpret_cast<{raw} *>(this)"
                args.insert(0, cast)
                if function.get("constructor"):
                    self.append_cpp_scope(
                        lines,
                        record.get("namespace", ""),
                        [
                            f"inline {record['name']}::{record['name']}({params})",
                            "{",
                            f"    {c_name}({', '.join(args)});",
                            "}",
                        ],
                    )
                    continue
                if function.get("destructor"):
                    signature = (
                        f"inline {record['name']}::{function['name']}({params})"
                    )
                else:
                    signature = (
                        f"inline {self.cpp_type(function['return_type'])} "
                        f"{record['name']}::{function['name']}({params})"
                        f"{' const' if function.get('const') else ''}"
                    )
            else:
                signature = (
                    f"inline {self.cpp_type(function['return_type'])} "
                    f"{function['name']}({params})"
                )
            prefix = "" if function["return_type"] == "void" else "return "
            namespace = (
                self.records[function["parent"]].get("namespace", "")
                if function.get("method") else function.get("namespace", "")
            )
            if (self.compact_optional_modules
                    and function.get("qualified_name")
                    == "ImGui::CreateContext"):
                if function["return_type"] != "ImGuiContext *":
                    raise TranslationError(
                        "optional-module source-facade CreateContext signature changed"
                    )
                self.append_cpp_scope(
                    lines,
                    namespace,
                    [
                        signature,
                        "{",
                        f"    ImGuiContext *result = {c_name}({', '.join(args)});",
                        "    imgui_c89_enable_full_features(result);",
                        "    return result;",
                        "}",
                    ],
                )
                continue
            call = f"{c_name}({', '.join(args)})"
            if function["return_type"].strip().endswith("&"):
                target = re.sub(
                    r"\s*&&?\s*$", "", function["return_type"].strip()
                )
                statement = (
                    f"return *reinterpret_cast<{self.cpp_type(target)} *>("
                    f"{call});"
                )
            else:
                statement = f"{prefix}{call};"
            self.append_cpp_scope(
                lines, namespace, [signature, "{", f"    {statement}", "}"],
            )
        lines.extend(["#endif", ""])
        return "\n".join(lines)

    @staticmethod
    def native_snake_name(name: str) -> str:
        value = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", name)
        value = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", value)
        return re.sub(r"_+", "_", value).strip("_").lower()

    def native_type_token(self, spelling: str) -> str:
        value = spelling.strip()
        value = value.replace("const char *", "string")
        value = value.replace("char *", "mutable_string")
        value = value.replace("const void *", "const_pointer")
        value = value.replace("void *", "pointer")
        value = value.replace("unsigned int", "uint")
        value = value.replace("unsigned short", "ushort")
        value = value.replace("unsigned char", "ubyte")
        value = value.replace("long long", "i64")
        value = value.replace("&&", "_rvalue")
        value = value.replace("&", "")
        value = value.replace("*", "_pointer")
        value = re.sub(r"\b(const|volatile|struct|class)\b", "", value)
        value = value.replace("ImGui", "").replace("Im", "")
        value = self.native_snake_name(_c_identifier(value))
        return value or "value"

    def native_api_functions(self) -> list[tuple[str, str, dict[str, Any]]]:
        """Return stable names for public, representable ImGui namespace APIs."""
        qualified_names = {
            function.get("qualified_name") for function in self.functions.values()
        }
        if not {
            "ImGui::GetCurrentContext", "ImGui::SetCurrentContext"
        }.issubset(qualified_names):
            # Focused lowering fixtures are not Dear ImGui programs and do
            # not carry its context machinery.  Their native API is empty.
            return []
        candidates: list[tuple[str, str, dict[str, Any]]] = []
        for identifier, function in sorted(self.functions.items()):
            namespace_api = (
                function.get("qualified_name", "").startswith("ImGui::")
                and not function.get("method")
            )
            record_api = (
                function.get("method")
                and function.get("parent") in self.records
                # A template specialization in imgui.h is an implementation
                # detail, not a stable native-C surface.  Exporting all
                # ImVector<T> methods also pins hundreds of otherwise local
                # instantiations into every optimized build.
                and "<" not in self.records[function["parent"]].get(
                    "qualified_name", ""
                )
                and not function.get("constructor")
                and not function.get("destructor")
                and not function.get("conversion")
                and not function.get("name", "").startswith("operator")
            )
            if not (namespace_api or record_api) or function.get("variadic"):
                continue
            if not any(
                Path(path).name == "imgui.h"
                for path in function.get("declaration_files", [])
            ):
                continue
            if function.get("name") in {"GetCurrentContext", "SetCurrentContext"}:
                continue
            if record_api:
                record = self.records[function["parent"]]
                owner = self.native_snake_name(
                    record.get("name", "record").removeprefix("ImGui").removeprefix("Im")
                )
                base = "imgui_" + owner + "_" + self.native_snake_name(function["name"])
            else:
                base = "imgui_" + self.native_snake_name(function["name"])
            candidates.append((base, identifier, function))

        by_name: dict[str, list[tuple[str, dict[str, Any]]]] = {}
        for item in candidates:
            by_name.setdefault(item[0], []).append((item[1], item[2]))
        result: list[tuple[str, str, dict[str, Any]]] = []
        used: set[str] = set()
        for base, group in sorted(by_name.items()):
            signatures = [
                [parameter["type"] for parameter in function.get("parameters", [])]
                for _, function in group
            ]
            differing_positions = [
                index for index in range(max(map(len, signatures), default=0))
                if len({
                    signature[index] if index < len(signature) else "missing"
                    for signature in signatures
                }) > 1
            ]
            for identifier, function in sorted(group):
                api_name = base
                if len(group) > 1:
                    parameter_types = [
                        parameter["type"]
                        for parameter in function.get("parameters", [])
                    ]
                    tokens = [
                        self.native_type_token(
                            parameter_types[index]
                            if index < len(parameter_types) else "none"
                        )
                        for index in differing_positions
                    ]
                    api_name += "_" + "_".join(tokens)
                if api_name in used:
                    api_name += "_" + hashlib.sha256(identifier.encode()).hexdigest()[:8]
                used.add(api_name)
                result.append((api_name, identifier, function))
        return sorted(result)

    def exact_c_api_functions(self) -> list[tuple[str, str, dict[str, Any]]]:
        """Return canonical exact-C definitions with stable public names.

        The explicit-context native API and exact C++ facade share these
        definitions directly. Current-context accessors are included because
        the C++ facade needs them at its implicit-context boundary, even though
        ordinary native calls receive their context explicitly.
        """
        result = self.native_api_functions()
        present = {identifier for _, identifier, _ in result}
        for identifier, function in sorted(self.functions.items()):
            if (identifier not in present
                    and function.get("qualified_name") in {
                        "ImGui::GetCurrentContext",
                        "ImGui::SetCurrentContext",
                    }):
                result.append((
                    "imgui_" + self.native_snake_name(function["name"]),
                    identifier,
                    function,
                ))
        return sorted(result)

    @staticmethod
    def stable_operation_name(function: dict[str, Any]) -> str:
        if function.get("constructor"):
            return "init"
        if function.get("destructor"):
            return "destroy"
        name = function.get("name", "function")
        operators = {
            "operator[]": "index",
            "operator()": "call",
            "operator==": "equal",
            "operator!=": "not_equal",
            "operator<": "less",
            "operator<=": "less_equal",
            "operator>": "greater",
            "operator>=": "greater_equal",
            "operator+": "add",
            "operator-": "subtract",
            "operator*": "multiply",
            "operator/": "divide",
            "operator+=": "add_assign",
            "operator-=": "subtract_assign",
            "operator*=": "multiply_assign",
            "operator/=": "divide_assign",
            "operator=": "assign",
            "operator bool": "as_bool",
        }
        return operators.get(name, name)

    def internal_exact_c_api_functions(
        self, public_ids: set[str]
    ) -> list[tuple[str, str, dict[str, Any]]]:
        """Stable readable ABI names needed only by the exact C++ facade."""
        candidates: list[tuple[str, str, dict[str, Any]]] = []
        for identifier, function in self.exact_wrapper_functions():
            if identifier in public_ids:
                continue
            operation = self.native_snake_name(
                self.stable_operation_name(function)
            ) or "function"
            if function.get("method") and function.get("parent") in self.records:
                owner = self.records[function["parent"]].get("name", "record")
                owner = self.native_snake_name(owner) or "record"
                if owner.startswith("im_gui_"):
                    owner = owner[len("im_gui_"):]
                base = f"{self.internal_namespace}{owner}_{operation}"
            else:
                qualified = function.get("qualified_name", operation)
                owner, separator, _ = qualified.rpartition("::")
                owner_token = self.native_snake_name(owner) if separator else ""
                if owner_token == "im_gui":
                    owner_token = ""
                base = self.internal_namespace
                if owner_token:
                    base += owner_token + "_"
                base += operation
            candidates.append((base, identifier, function))

        grouped: dict[str, list[tuple[str, dict[str, Any]]]] = {}
        for base, identifier, function in candidates:
            grouped.setdefault(base, []).append((identifier, function))
        result: list[tuple[str, str, dict[str, Any]]] = []
        used: set[str] = set()
        for base, group in sorted(grouped.items()):
            for identifier, function in sorted(group):
                name = base
                if len(group) > 1:
                    tokens = [
                        self.native_type_token(parameter["type"])
                        for parameter in function.get("parameters", [])
                    ]
                    if function.get("const"):
                        tokens.append("const")
                    if function.get("variadic"):
                        tokens.append("varargs")
                    name += "_" + "_".join(tokens or ["void"])
                if name in used:
                    raise TranslationError(
                        f"stable internal exact-C overload collision: {name}"
                    )
                used.add(name)
                result.append((name, identifier, function))
        return sorted(result)

    def native_parameters(self, function: dict[str, Any], context: bool) -> list[str]:
        parameters = ["ImGuiContext *ctx"] if context else []
        if function.get("method") and not function.get("static"):
            parent = self.records[function["parent"]]
            parameters.append(
                f"{self.record_names_by_id[parent['id']]} *self"
            )
        for index, parameter in enumerate(function.get("parameters", [])):
            name = parameter.get("name") or f"arg_{index}"
            parameters.append(self.c_declaration(parameter["type"], name))
        return parameters

    def native_scope_end_identifier(self, function: dict[str, Any]) -> str | None:
        end_name = self.NATIVE_CONDITIONAL_SCOPE_ENDS.get(
            function.get("qualified_name", "")
        )
        if end_name is None:
            return None
        matches = [
            identifier
            for identifier, candidate in self.functions.items()
            if candidate.get("qualified_name") == end_name
        ]
        if len(matches) != 1:
            raise TranslationError(
                f"native scope policy for {function.get('qualified_name')} "
                f"expected exactly one {end_name} definition, found {len(matches)}"
            )
        return matches[0]

    def native_return_type(self, function: dict[str, Any]) -> str:
        if function.get("qualified_name") in self.NATIVE_CONDITIONAL_SCOPE_ENDS:
            return "imgui_scope"
        return self.c_type(function["return_type"])

    def emit_native_c_header(self) -> str:
        lines = [
            "/* Generated Dear ImGui C89 API. Do not edit. */",
            "#ifndef IMGUI_C89_API_H",
            "#define IMGUI_C89_API_H",
            '#include "imgui_c89.h"',
            "",
            "#ifdef __cplusplus",
            'extern "C" {',
            "#endif",
            "",
            "typedef enum imgui_scope {",
            "    IMGUI_SCOPE_ERROR = -1,",
            "    IMGUI_SCOPE_INACTIVE = 0,",
            "    IMGUI_SCOPE_ACTIVE = 1",
            "} imgui_scope;",
            "",
            "/* Plain imgui_* functions are exact implementation entry points",
            " * shared by C and the C++ facade. These optional helpers normalize",
            " * Dear ImGui's exceptional Begin/End rule by closing an inactive",
            " * Begin or BeginChild before returning. */",
            "/* Variadic C++ entry points are represented by their va_list (V) siblings. */",
        ]
        if self.compact_optional_modules:
            lines.extend([
                "/* Opt into navigation, INI persistence, and CFF/Type2 fonts.",
                " * The exact C++ facade performs this automatically. */",
                "void imgui_enable_full_features(ImGuiContext *ctx);",
            ])
        for api_name, _, function in self.native_api_functions():
            if self.native_scope_end_identifier(function) is None:
                continue
            parameters = self.native_parameters(function, True)
            lines.append(
                f"imgui_scope {api_name}_scope"
                f"({', '.join(parameters) or 'void'});"
            )
        lines.extend([
            "",
            "#ifdef __cplusplus",
            "}",
            "#endif",
            "#endif",
            "",
        ])
        return "\n".join(lines)

    def emit_native_c_source(self) -> str:
        lines = [
            "/* Generated optional C89 convenience helpers. Do not edit. */",
            '#include "imgui_c89_api.h"',
        ]
        if self.split_public_header:
            lines.append('#include "imgui_c89_internal.h"')
        lines.append("")
        for api_name, identifier, function in self.native_api_functions():
            scope_end_identifier = self.native_scope_end_identifier(function)
            if scope_end_identifier is None:
                continue
            parameters = self.native_parameters(function, True)
            arguments = [
                parameter.get("name") or f"arg_{index}"
                for index, parameter in enumerate(function.get("parameters", []))
            ]
            if function.get("method") and not function.get("static"):
                arguments.insert(0, "self")
            if identifier in self.context_threaded_functions:
                arguments.insert(0, "ctx")
            raw_call = f"{self.function_names[identifier]}({', '.join(arguments)})"
            lines.append(
                f"imgui_scope {api_name}_scope({', '.join(parameters) or 'void'})"
            )
            lines.append("{")
            if self.compact_global_context:
                global_context = self.global_names[self.compat_context_global_id]
                lines.append(f"    {global_context} = ctx;")
            end_arguments = (
                "ctx" if scope_end_identifier in self.context_threaded_functions else ""
            )
            end_call = f"{self.function_names[scope_end_identifier]}({end_arguments})"
            lines.append("    if (ctx == 0) return IMGUI_SCOPE_ERROR;")
            lines.append(f"    if ({raw_call}) return IMGUI_SCOPE_ACTIVE;")
            lines.append(f"    {end_call};")
            lines.append("    return IMGUI_SCOPE_INACTIVE;")
            lines.extend(["}", ""])
        if self.compact_optional_modules:
            lines.extend([
                "void imgui_enable_full_features(ImGuiContext *ctx)",
                "{",
                "    imgui_c89_enable_full_features(ctx);",
                "}",
                "",
            ])
        return "\n".join(lines)

    def output(self) -> Output:
        c_header = self.emit_c_header()
        c_internal_header = self.emit_c_internal_header()
        c_source = self.emit_c_source()
        cpp_header = self.emit_cpp_header()
        cpp_source = self.emit_exact_cpp_source()
        native_c_header = self.emit_native_c_header()
        native_c_source = self.emit_native_c_source()
        manifest_data = {
            "schema_version": 1,
            "clang_version": self.ir.get("clang_version"),
            "c_header_sha256": hashlib.sha256(c_header.encode()).hexdigest(),
            "c_header_bytes": len(c_header.encode()),
            "c_internal_header_sha256": hashlib.sha256(
                c_internal_header.encode()
            ).hexdigest(),
            "c_internal_header_bytes": len(c_internal_header.encode()),
            "c_source_sha256": hashlib.sha256(c_source.encode()).hexdigest(),
            "cpp_header_sha256": hashlib.sha256(cpp_header.encode()).hexdigest(),
            "cpp_source_sha256": hashlib.sha256(cpp_source.encode()).hexdigest(),
            "native_c_header_sha256": hashlib.sha256(native_c_header.encode()).hexdigest(),
            "native_c_source_sha256": hashlib.sha256(native_c_source.encode()).hexdigest(),
            "native_c_function_count": len(self.native_api_functions()),
            "exact_c_function_count": len(self.exact_c_api_functions()),
            "split_public_header": self.split_public_header,
            "public_complete_record_count": len(
                self.public_complete_record_ids
            ),
            "public_forward_record_count": len(
                self.public_forward_record_ids
            ),
            "private_complete_record_count": (
                len(self.records) - len(self.public_complete_record_ids)
            ),
            "public_typedef_count": sum(
                self.typedef_is_public(item) for item in self.typedefs
            ),
            "public_enum_constant_count": sum(
                len(enum.get("constants", []))
                for enum in self.ir.get("enums", [])
                if enum["id"] in self.public_enum_ids
            ),
            "private_enum_constant_count": sum(
                len(enum.get("constants", []))
                for enum in self.ir.get("enums", [])
                if enum["id"] not in self.public_enum_ids
            ),
            "internal_exact_c_function_count": (
                len(self.exact_c_names) - len(self.public_exact_c_names)
            ),
            "native_conditional_scope_count": sum(
                1 for _, _, function in self.native_api_functions()
                if function.get("qualified_name")
                in self.NATIVE_CONDITIONAL_SCOPE_ENDS
            ),
            "context_threaded_function_count": len(self.context_threaded_functions),
            "context_fixed_signature_count": len(self.context_boundary_sources),
            "internal_function_count": len(self.internal_functions),
            "constructor_value_helper_count": sum(
                bool(consumers)
                for consumers in self.constructor_value_consumers.values()
            ),
            "constructor_at_helper_count": sum(
                bool(consumers)
                for consumers in self.constructor_at_consumers.values()
            ),
            "inline_constructor_value_helper_count": len(
                self.inline_constructor_value_helpers
            ),
            "omitted_call_count": len(self.omitted_call_ids),
            "omitted_calls": sorted(self.omitted_call_names),
            "trapped_call_count": len(self.trapped_call_ids),
            "trapped_calls": sorted(self.trapped_call_names),
            "compact_crc32": self.compact_crc32,
            "compact_style_colors": self.compact_style_colors,
            "compact_checkbox_flags": self.compact_checkbox_flags,
            "noinline_functions": sorted(self.noinline_function_names),
            "handwritten_functions": sorted(self.handwritten_functions),
            "compact_name_switches": sorted(self.compact_name_switches),
            "compact_global_context": self.compact_global_context,
            "compact_truetype_only": self.compact_truetype_only,
            "compact_nav_key_ranges": self.compact_nav_key_ranges,
            "compact_nav_overlay_selectable": (
                self.compact_nav_overlay_selectable
            ),
            "compact_cff_stack_guards": self.compact_cff_stack_guards,
            "compact_key_char_mask": self.compact_key_char_mask,
            "compact_localization_entries": self.compact_localization_entries,
            "compact_cursor_data": self.compact_cursor_data,
            "compact_separator_table": self.compact_separator_table,
            "compact_utf8_tables": self.compact_utf8_tables,
            "compact_color_format_tables": self.compact_color_format_tables,
            "compact_glyph_deltas": self.compact_glyph_deltas,
            "compact_assert_metadata": self.compact_assert_metadata,
            "compact_optional_modules": self.compact_optional_modules,
            "compact_assert_traps": self.compact_assert_traps,
            "compact_assert_trap_count": self.compact_assert_trap_count,
            "compact_imvector": self.compact_imvector,
            "compact_imvector_accessors": self.compact_imvector_accessors,
            "compact_imvector_lifecycle": self.compact_imvector_lifecycle,
            "compact_imvector_capacity": self.compact_imvector_capacity,
            "compact_imchunkstream": self.compact_imchunkstream,
            "compact_impool": self.compact_impool,
            "internal_namespace": self.internal_namespace,
            "format_table_source": self.format_table_source,
            "omit_unused_scalar_helpers": self.omit_unused_scalar_helpers,
            "omit_unused_compatibility_shims": (
                self.omit_unused_compatibility_shims
            ),
            "compact_input_source_names": self.compact_input_source_names,
            "compact_static_const_arrays": self.compact_static_const_arrays,
            "explicit_context_threaded_function_count": (
                self.explicit_context_threaded_function_count
            ),
            "compact_zero_constructors": sorted(
                self.compact_zero_constructor_names
            ),
            "packed_four_character_globals": sorted(
                self.packed_char_global_names
            ),
            "packed_string_pointer_globals": sorted(
                self.packed_string_pointer_global_names
            ),
            "compressed_string_pointer_globals": sorted(
                self.compressed_string_pointer_global_names
            ),
        }
        manifest = json.dumps(manifest_data, indent=2, sort_keys=True) + "\n"
        return Output(
            c_header, c_internal_header, c_source, cpp_header, cpp_source,
            native_c_header, native_c_source, manifest,
        )


def translate(ir_path: Path, output_directory: Path) -> None:
    ir = json.loads(ir_path.read_text(encoding="utf-8"))
    emitter = Emitter(ir)
    output = emitter.output()
    output_directory.mkdir(parents=True, exist_ok=True)
    files = {
        "imgui_c89.h": output.c_header,
        "imgui_c89_internal.h": output.c_internal_header,
        "imgui_translated.h": (
            "/* Compatibility include; use imgui_c89.h in new code. */\n"
            "#ifndef IMGUI_TRANSLATED_H\n"
            "#define IMGUI_TRANSLATED_H\n"
            '#include "imgui_c89.h"\n'
            "#endif\n"
        ),
        "imgui_translated.c": output.c_source,
        "imgui_translated.hpp": output.cpp_header,
        "imgui_translated_wrapper.cpp": output.cpp_source,
        "imgui_c89_api.h": output.native_c_header,
        "imgui_c89_api.c": output.native_c_source,
        "manifest.json": output.manifest,
    }
    for name, contents in files.items():
        (output_directory / name).write_text(contents, encoding="utf-8", newline="\n")

    claimed_functions: set[str] = set()
    claimed_globals: set[str] = set()
    unit_manifest = []
    for index, unit in enumerate(ir.get("translation_units", [])):
        function_ids = (
            set(unit.get("function_definitions", [])) - claimed_functions
        )
        global_ids = set(unit.get("global_definitions", [])) - claimed_globals
        claimed_functions.update(function_ids)
        claimed_globals.update(global_ids)

        unit_emitter = Emitter(ir, (
            emitter.function_owners,
            emitter.function_references,
            emitter.internal_functions,
        ))
        # Type spelling is a pure property of the merged IR. Reuse the root
        # emitter's populated cache instead of repeating thousands of regex
        # substitutions independently for every output translation unit.
        unit_emitter.c_type = emitter.c_type  # type: ignore[method-assign]
        unit_emitter.active_function_ids = function_ids
        unit_emitter.active_global_ids = global_ids
        unit_emitter.emit_runtime_support = index == 0
        source = unit_emitter.emit_c_source()
        stem = unit["name"].removesuffix("_cpp")
        if stem.startswith("backends_"):
            stem = stem.removeprefix("backends_")
        filename = stem + ".c"
        if filename == "imgui_tables.c" and unit_emitter.format_table_source:
            source = format_table_c89_source(source)
        (output_directory / filename).write_text(
            source, encoding="utf-8", newline="\n"
        )
        unit_manifest.append({
            "source": filename,
            "function_count": len(
                function_ids & set(unit_emitter.functions)
            ),
            "global_count": len(global_ids & set(unit_emitter.globals)),
            "sha256": hashlib.sha256(source.encode()).hexdigest(),
        })
    if unit_manifest:
        (output_directory / "translation_units.json").write_text(
            json.dumps(unit_manifest, indent=2, sort_keys=True) + "\n",
            encoding="utf-8", newline="\n",
        )
