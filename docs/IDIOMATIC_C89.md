# Idiomatic C89 Dear ImGui

The idiomatic C89 implementation pins Dear ImGui `v1.92.9b` at commit
`f1cc2ae15e53a861a874c3034aae6798fde194ab`, the latest published release
verified again on 2026-08-28. It deliberately does not use `NDEBUG` or any
`IMGUI_DISABLE_*` feature macro. The built-in fonts, settings, navigation,
debug tools, recovery paths, TrueType and CFF loaders, and all four core
translation units remain present.

## Patch model

The hand-maintained overlay is
`translator/patches/idiomatic_c89.json`. It is applied to extracted IR
before C89 emission rather than as a line-oriented diff against generated C.
This keeps the literal translator as the oracle, makes the overlay usable under
the Test Engine's compile-time configuration, and lets shape-sensitive
lowerings reject an upstream change instead of silently patching the wrong
code.

The idiomatic overlay contains representation-only lowerings exercised by the
project's strict builds and semantic gates:

- shared element-size-driven `ImVector<T>` mutation, checked-accessor,
  lifecycle, and capacity helpers, plus type-erased safe subsets of the two
  `ImChunkStream<T>` and three `ImPool<T>` instantiation families used by core;
- direct C pointer/end pairs for table spans, direct size/capacity/pointer
  fields for table-owned variable arrays, and an ABI-identical
  `ImGuiTablePool`. The corresponding table-specific `ImSpan`, `ImVector`,
  and `ImPool` shells are absent from the generated C;
- exact assertion metadata dictionaries local to each generated translation
  unit: each cold failure ID reconstructs the original file, line, and
  expression through a generation-verified byte-pair decoder before invoking
  the original platform or Test Engine assertion backend;
- one shared style-palette decoder and width-shared `CheckboxFlags` cores
  (using `memcpy` so signed and unsigned overloads retain alias-safe behavior);
- compact CRC-32, key-name, input-source-name, localization,
  key-mask, UTF-8, color-format, cursor, separator, and fixed navigation data;
- a two-tier 3-bit/nibble/byte encoding of the Chinese Simplified and Japanese
  glyph deltas, decoded into the same stable upstream range arrays;
- coalesced zero initialization for selected constructors, static immutable
  local tables, packed four-symbol cursor/font-atlas data, and checked CFF
  stack guards;
- selected no-inline boundaries where the compiler otherwise duplicates an
  entire constructor, cleanup body, or compatibility overload;
- checked specialization of the non-interactive Ctrl+Tab overlay path;
- signature- and body-fingerprint-guarded handwritten C89 functions, including
  all 115 functions sourced from `imgui_tables.cpp`, polyline geometry, plot
  rendering, typed arithmetic, rounded horizontal range fills, and selected
  compact helpers elsewhere in the core;
- omission of generated scalar fallbacks and compatibility shims that the
  latest strict multi-unit build proves are unreferenced. A future reference
  becomes a compile failure rather than a silent semantic substitution.

No public function, widget family, callback, or error path is removed. The
overlay's `omit_calls` list is intentionally empty.

## Generated C style

Representation-neutral emitter cleanup is shared by the literal and idiomatic
profiles. Statement and control syntax no longer carries redundant outer
parentheses, array subscripts use their natural postfix form, scalar no-op
statements left by trivial C++ destructors are omitted, and a discarded call
returning a C++ reference is emitted as the helper call rather than a needless
C dereference. These rules reduced the four idiomatic generated C sources from
3,185,335 to 3,153,508 bytes without changing any measured object or loaded
section byte count. The fixture suite includes a discarded-reference operator
case and verifies both the C89 lowering and its C++ source facade.

The maintained table translation unit uses K&R braces and braces every control
body. Private exact-C and handwritten symbols use the `imgui__` namespace;
public `imgui_` names and translator-runtime `imgui_c89_` helpers remain
separate.

## Like-for-like result

Homebrew Clang 22.1.8 on arm64 macOS compiled every object with:

```text
-Os -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables
```

The core comparison excludes `imgui_demo.cpp`, backends, and both compatibility
adapters.

| Variant | Core archive bytes | Core object bytes | Loaded section bytes |
|---|---:|---:|---:|
| upstream C++ | 1,023,912 | 961,544 | 589,342 |
| literal translated C89 | 985,704 | 923,632 | 579,186 |
| idiomatic C89 | 811,192 | 751,888 | 516,999 |

The overlay removes 174,512 physical archive bytes and 62,187 loaded section
bytes (10.74%) from literal C89. The idiomatic core is 212,720 archive bytes
(20.78%) and 72,343 loaded section bytes (12.27%) smaller than upstream C++.

Compatibility surfaces are shown separately because an embedded C client and
a C++ compatibility client would not ship both:

| Surface | Literal archive | Idiomatic archive | Literal sections | Idiomatic sections |
|---|---:|---:|---:|---:|
| exact C API + optional scope helpers | 986,776 | 812,264 | 579,390 | 517,203 |
| exact C++ facade + core | 1,235,624 | 1,060,552 | 634,181 | 571,994 |

The exact C API is now the core implementation surface itself. The optional
three-function normalized-scope helper object adds only 204 loaded section
bytes. The exact C++ facade is a validation/migration layer, not part of the
intended C-only deliverable.

## Validation

- Five overlaid translation units (`imgui`, draw, tables, widgets, and demo)
  compile independently with strict `-std=c89 -pedantic-errors` under the
  official Test Engine configuration.
- Generation is byte-for-byte deterministic across two passes.
- The official Dear ImGui Test Engine passes 415/415 against native C++ and
  415/415 against idiomatic C89 through the generated exact C++ facade.
- The handwritten differential compares 61,328 normalized records, including
  exhaustive 8-bit typed arithmetic, wider boundary samples, rounded-range
  geometry, changing/reordered table layouts, and enabled table diagnostic
  logs. Native C++ and C89 produce SHA-256
  `73a48d622635739cc9d1f7acf94890545e7865e711e1dbf7eea62e83894357b6`.
- ASan reports no error. UBSan adds no diagnostic in a handwritten body; its
  remaining null-empty-vector offset reports are a known translator-wide
  baseline outside these rewrites.
- GCC 15 compiles the idiomatic implementation as strict C90 and passes 2,495
  Clang-IR layout assertions, the exact C++ facade ABI smoke, and native C API
  smoke.
- The Test Engine configuration replaces the platform assertion macro; the
  checked vector factoring preserves its exact logger-plus-debugtrap backend.
- A focused native-versus-facade glyph fixture compares every decoded Chinese
  Simplified and Japanese range. The final count/hash pairs are respectively
  `5012 / 8458712642073220670` and `6008 / 6932922291721718969` on both sides.
- Every compressed string table is decoded and byte-compared with its source
  strings during generation before C is emitted.
- The core profile passes its C++ facade frame smoke and its 626-entry native
  C API two-context smoke.
- The public C89 header exposes all 738 current `imgui.h` enum constants under
  their exact upstream names. The 342 private constants are isolated in
  `imgui_c89_internal.h`; neither surface contains hash-suffixed enumerators.
- The public type/function boundary contains 67 complete records, 71 record
  declarations, 60 upstream typedefs, and 628 exact entry points (626 ordinary
  operations plus current-context accessors). The other 172 complete records,
  raw helpers, globals, and translator support declarations are private. The
  public header is 187,876 bytes, contains no identity-derived names, and
  compiles standalone as strict C89.
- The locked 1.83 through 1.92 release families each regenerate twice from
  upstream source with byte-identical output, compile as five independent
  strict-C89 units, and pass the same demo-enabled frame smoke in native C++
  and through the generated exact C++ facade/C89 implementation.

The release-family matrix validates the default source-derived translator path;
the handwritten size profile intentionally targets only the locked latest
release. This is the maintenance boundary: a new latest tag must pass the
literal translator first, after which shape guards either accept the tracked
overlay or fail loudly at the changed lowering.

## Rejected probes

Changes were retained only when loaded code savings and source clarity
justified their maintenance cost; physical object and archive bytes are always
reported separately because relocations may move them in the opposite
direction. Rejected trials
included no-inline scalar wrappers (byte-neutral), alternate glyph base-array
layouts (neutral or larger), factoring `ImPool::Remove` (larger), a packed
UTF-8 length-table rewrite (only four loaded bytes), metrics-name hoisting
(eight loaded bytes but an invasive whole-function hook), and omission of
redundant fully-covered aggregate zeroing (archive-neutral and 12 loaded bytes
larger). A shared named-window setter saved 68 loaded bytes but added 16
physical object bytes, while outlining the table-column-settings constructor
added eight physical bytes; neither is maintained. A sequential variable-length
assertion-record stream saved 328
archive bytes but added 62 loaded bytes, while factoring default `ImVector`
initialization added 3,184 archive and 2,184 loaded bytes. Generic `ImVector`
copy assignment initially appeared to save 36 loaded bytes, but preserving
upstream's self-assignment behavior made it 12 loaded bytes larger. These
probes are absent from the maintained overlay.

## Reproduction and update loop

```sh
make translator-idiomatic-c89-size
make translator-idiomatic-c89-differential
make translator-idiomatic-c89-test-engine
make translator-size-opportunities
make translator-matrix
```

For the next upstream release, update only `latest_release` in
`translator/upstream.lock.json`, rebuild, and let checked overlay lowerings fail
on changed source shapes. Adjust the small JSON overlay or its named translator
lowering, rerun the gates, and record the new report. Generated files under
`build/` remain disposable and are never hand-edited.

`translator-size-opportunities` uses Bloaty and LLVM 22 to attribute every
object, rank exact and normalized clone families, and measure compiler merge
and outlining transforms as discovery-only upper bounds. Its CSV, JSON, and
Markdown reports are written to `build/size-opportunities/`; the compiler
oracle variants never replace the separately compiled idiomatic objects.
