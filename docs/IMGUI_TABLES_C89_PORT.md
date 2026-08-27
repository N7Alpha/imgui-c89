# Complete `imgui_tables` C89 port

`imgui_tables.cpp` is the smallest Dear ImGui core translation unit above the
chosen 4,000-line threshold: 4,905 upstream lines in v1.92.9b. The idiomatic
generated C is 5,371 lines. This is the first whole-TU handwritten-port target.

## Definition of done

The machine-generated manifest at `build/tu-port/imgui_tables/manifest.json`
is the ownership ledger. Completion requires all 115 upstream definitions to
be signature/body-fingerprint-guarded handwritten C89 implementations. A large
function being replaced or most object bytes being handwritten is not enough:
`source_derived_count` must be zero.

Shared translator runtime helpers may remain generated, but no executable body
whose source location is `imgui_tables.cpp` may be emitted from the C++ AST.
The seven TU constants may be emitted from checked semantic values until the
last migration step, when they become named C constants owned by the port.

## Baseline

All size values use Clang 22.1.8 with
`-Os -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables`.

| Implementation | Object bytes | Loaded section bytes |
|---|---:|---:|
| upstream C++ `imgui_tables.o` | 102,472 | 57,423 |
| literal translated C89 | 94,384 | 53,879 |
| current idiomatic C89 | 79,000 | 48,872 |

The current object contains 42,988 loaded bytes attributable directly to the
115 upstream functions. `TableReconcileColumns` is the first handwritten body;
114 remain. The completed TU may not exceed the current idiomatic physical or
loaded size.

## C data model

The ABI records are shared with `imgui.cpp`, `imgui_widgets.cpp`, the public C
API, and the generated C++ facade. Their layout remains exactly the layout in
`imgui_c89_internal.h`:

- `ImGuiTable`, `ImGuiTableColumn`, `ImGuiTableTempData`, and
  `ImGuiTableInstanceData` retain their field layout and bit packing.
- `ImGuiTableSettings` followed by packed `ImGuiTableColumnSettings` remains
  the persistent settings-stream representation.
- `ImGuiOldColumns` and `ImGuiOldColumnData` remain the legacy Columns API
  state representation until that API is ported and verified.
- `ImGuiTableReconcileColumnData`, `ImGuiTableCellData`, and
  `ImGuiTableHeaderData` remain caller-visible exchange records.
- Flag and index types remain the exact integer typedefs exposed by the
  maintained headers.

Idiomatic C applies inside that fixed storage boundary:

- spans are half-open pointer pairs (`Data`, `DataEnd`) and loops use pointer
  or integer iteration directly;
- vectors are `Data`, `Size`, and `Capacity`; direct clearing is allowed for
  trivially destructible table records, while growth uses the checked shared
  allocator helper;
- `ImRect`, `ImVec2`, and short-lived C++ value objects become scalar locals or
  directly initialized C records;
- range-for adapters, reference temporaries, overloaded operators, and
  template accessors do not survive in maintained C;
- the raw table allocation retains its current columns/order/cells/bit-mask
  packing, because other TUs and debug tooling observe those addresses;
- context is always explicit as `ImGuiContext *ctx`; hidden global-context
  reads are not introduced; and
- maintained templates name functions, constants, and types semantically.
  Generated hash-suffixed spellings are resolved by the emitter and never
  appear in handwritten source.

TU-private routines will ultimately use stable `imgui_tables_*` C names.
Cross-TU and public routines keep the already-established maintained exact-C
names so the C++ facade remains a thin caller rather than a second semantic
implementation.

## Migration clusters

1. Leaf accessors, ID calculations, flag queries, and small geometry helpers.
2. Settings stream creation, parsing, application, saving, and garbage
   collection.
3. Column defaults, widths, display order, and sorting.
4. Row/cell transitions, clipping, background requests, and draw channels.
5. Headers, context menus, borders, debug views, and legacy Columns support.
6. `BeginTableEx`, queued requests, `TableUpdateLayout`, and `EndTable`.
7. TU constants, remaining local helpers, and the zero-source-derived ownership
   proof.

Zero-byte inlined helpers are migrated with their first handwritten caller;
they cannot satisfy an independent symbol-size gate. Byte-neutral bodies are
accepted when they advance TU ownership and the complete object does not grow.
Larger bodies must be offset within the same semantic cluster, and no accepted
cluster may increase the complete object.

## Semantic gates

- deterministic independent strict-C89 generation and compilation;
- focused native/C89 traces for every migrated cluster, including internal
  table state, sort specs, settings text, draw channels, geometry, pixels, and
  opt-in table debug logs;
- Dear ImGui Test Engine 415/415 natively and through the exact C++ facade;
- GCC strict-C90 record-layout, C API, and C++ ABI smokes;
- ASan clean and no new UBSan diagnostic beyond the known literal-translator
  empty-vector baseline; and
- final Bloaty and full-library measurement under the idiomatic size flags.

Rebuild the ownership ledger with:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 translator/tu_port_manifest.py
```
