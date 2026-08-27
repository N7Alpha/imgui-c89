# Handwritten C89 rewrite protocol

The literal translator remains the upstream-sync mechanism. Handwritten C
replacements are a second, latest-release-only layer selected by qualified
function name and guarded by the complete extracted parameter signature and
body fingerprint. Normal and Test Engine configurations have separate accepted
body fingerprints. A changed upstream body must reject the replacement until
it is reviewed.

Templates refer to functions, enum constants, record types, and value
constructors by semantic token. The emitter resolves those tokens from the
current IR, so maintained C does not contain generated hash-suffixed names.

## Data policy

- Public records and every record crossing a translation-unit or facade
  boundary retain the translated Dear ImGui layout exactly.
- A replacement may flatten local/private state into scalar locals, byte
  ranges, and indexes. It may not reinterpret public storage incompatibly.
- The default private dynamic-array view is `Data`, `Size`, and `Capacity`.
  Type-erased helpers are preferred only when helper plus call sites are
  smaller than specialized direct code.
- Scratch storage must reuse context, draw-list shared data, or caller-owned
  storage. A rewrite does not introduce hidden allocation or global state.
- Small replacement-only helpers are allowed, but their combined loaded and
  physical bytes are charged to the replacement.

## Blind authoring

The rewrite author receives only the C signature, observable semantic
contract, permitted records/helpers, and C89 constraints. They do not inspect
the upstream C++, extracted IR, literal generated C, or idiomatic generated C.
The integrating maintainer adapts semantic names to generated names and may
report only black-box failures or missing contract details back to the author.

This is intended to produce an independent C implementation, not a disguised
transcription. The original implementation remains available to the build as
the oracle variant and is never linked into the replacement variant.
Raw blind submissions are retained beside the adapted `.c.in` templates in
`translator/handwritten/`. Integration is limited to semantic-name binding,
C89 ABI adaptation, and restoration of required diagnostic side effects.

## Acceptance gates

1. The replacement and oracle compile as independent strict-C89 translation
   units with identical ABI declarations.
2. Focused differential fixtures compare return values, mutated state, draw
   commands, indexes, vertices, texture resources, and rendered pixels as
   applicable. Both sides must also be deterministic.
3. The complete native-versus-C89 trace must remain byte-identical.
4. The official Dear ImGui Test Engine must pass every test through the exact
   C++ facade.
5. AddressSanitizer must remain green. UndefinedBehaviorSanitizer must add no
   diagnostic in a replacement and no diagnostic beyond the literal
   translator's known baseline. Handwritten replacements target only the
   locked latest release.
6. The replacement must reduce its original symbol's loaded bytes under the
   idiomatic `-Os`, no-exception, no-unwind settings. Whole-object loaded and
   physical bytes may not increase. Neutral or larger rewrites are rejected.

The complete-TU protocol is a deliberate extension of gate 6: byte-neutral
leaf bodies may be accepted because handwritten ownership itself is the goal,
provided their semantic cluster and complete object remain non-increasing.

## Maintained results (v1.92.9b)

| Function | Before | Handwritten | Saving | Result |
|---|---:|---:|---:|---|
| `ImDrawList::AddPolyline` | 2,496 | 2,072 | 424 (17.0%) | accepted |
| `ImGui::PlotEx` | 1,744 | 1,632 | 112 (6.4%) | accepted |
| `ImGui::DataTypeApplyOp` | 1,084 | 796 | 288 (26.6%) | accepted |
| `ImGui::RenderRectFilledInRangeH` | 996 | 980 | 16 (1.6%) | accepted |
| `ImGui::TableReconcileColumns` | 1,004 | 784 | 220 (21.9%) | accepted |
| `ImGui::UpdateInputEvents` | 1,932 | — | — | contract too broad; not integrated |

These functions exercise geometry over a stable public draw-list layout, a
stateful widget using internal helpers, saturating typed arithmetic, rounded
rectangle path construction, table-state reconciliation, and an in-place
event-queue state machine. Only targets whose contracts proved sufficiently
bounded for blind maintenance were retained.

The second batch saves 524 loaded bytes and 648 physical object bytes. Across
all five accepted replacements, the direct loaded-code saving is 1,060 bytes.
The focused native-versus-C89 differential passes 61,314 pointer-free state,
draw, resource, pixel, and diagnostic-log records with SHA-256
`cb60f1921d57c7dea90a523a3b285a89b8f8aa1dee13b1c159b90ef6dbc82a82`.
Coverage includes exhaustive S8/U8 add/subtract pairs, wider numeric boundary
samples, four rounded-range geometries, named/sequential/size-changing table
reconciliation, and the enabled table debug-log buffer in addition to the
polyline and plot cases. The official Dear ImGui Test Engine passes 415/415
tests in both native and translated-wrapper configurations.

The final ASan differential reports no errors. UBSan reports the translator's
pre-existing zero-offset arithmetic on null empty-vector storage in unrelated
generated paths; none of the five handwritten bodies emits a diagnostic. GCC
15 independently compiles the idiomatic profile as strict C90 and passes
2,495 Clang-IR record-layout assertions plus the C and C++ ABI smoke tests.

Reproduce the principal gates with:

```sh
make translator-idiomatic-c89-differential
make translator-idiomatic-c89-test-engine
make translator-idiomatic-c89-size
PYTHONDONTWRITEBYTECODE=1 python3 translator/portability_test.py --profile idiomatic_c89
```
