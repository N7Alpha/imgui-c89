# Translator development

The translator has two implementation halves:

- `clang_extract/`: a C++17 LibTooling frontend pinned to LLVM/Clang major 22.
- `py/imgui_translator/`: deterministic IR validation, lowering, C89 emission,
  and C++ compatibility-facade emission.

The frontend does not expose Clang's diagnostic AST JSON as an API. It emits a
small schema owned by this project and uses stable Clang USRs for declaration
identity.

## Dependencies

- CMake 3.20 or newer.
- Ninja.
- LLVM/Clang 22 development libraries and headers.
- Python 3.10 or newer.
- A strict C89 compiler and a C++98-or-newer compiler for facade tests.
- Bloaty for the optional idiomatic-build size-opportunity report.

On Homebrew LLVM is keg-only. The default test script looks under
`/opt/homebrew/opt/llvm`; set `LLVM_PREFIX` and/or `CMAKE_PREFIX_PATH` elsewhere.

## Commands

```sh
make check
make translator-baseline
make translator-embedded
make translator-embedded-size
make translator-differential
make translator-test-engine
make translator-matrix
make translator-docking
make translator-docking-differential
make translator-docking-test-engine
make translator-portability
make translator-viewport-sdlgpu
make translator-object-sizes
make translator-idiomatic-c89
make translator-idiomatic-c89-size
make translator-idiomatic-c89-test-engine
make translator-idiomatic-c89-differential
make translator-size-opportunities
make translator-vendor

python3 translator/fetch_upstream.py --revision baseline
python3 translator/report_ir.py build/translator/upstream/imgui.ir.json
```

`translator-baseline` extracts and merges the four core translation units,
`imgui_demo.cpp`, the dependency-free null backend, and the SDL3 plus
SDL_Renderer3 platform/renderer backends. It then generates twice, compares
translator-owned files byte-for-byte, compiles each of the eight generated
implementation files independently as strict C89, compiles the exact upstream
C++ facade, and renders a translated demo frame through a hidden SDL window.
The development profile discovers SDL3 flags with `pkg-config`.

`translator-matrix` performs the same full-body extraction and strict-C89
lowering for the four core units plus `imgui_demo.cpp` across the locked 1.83
through 1.92 release families. Every row is generated twice and compared
byte-for-byte, compiles as five independent strict-C89 units, then executes
the same demo-enabled frame smoke once against native upstream C++ and once
through its exact upstream-header facade and translated C89. The facade uses
C++98 for 1.83--1.86 and C++11 afterward because the later upstream headers
themselves require C++11. Results are written to
`build/translator/matrix/summary.{json,md}`.

`translator-differential` compiles one source-identical scenario program
against native Dear ImGui and against the translated objects through their
generated C++ facade. Each executable is run twice. The gate byte-compares
37,000+ normalized state, texture-resource, draw-command, index, vertex, atlas,
and software-framebuffer records. It additionally compares all 24,576,000
RGBA bytes from the atlas-textured and texture-independent renders, and saves
the traces, framebuffer streams, and summary under
`build/translator/differential/`.

`translator-embedded` is a compact lowering profile, not a second handwritten
implementation. It disables assertions for release builds, omits the two upstream
malformed-scope recovery calls, and excludes Dear ImGui's built-in compressed
font payloads. The application supplies an ordinary TTF, which is still parsed,
rasterized, and packed at runtime by Dear ImGui. All core units, widget
families, the demo, backends, native C API, and exact C++ facade are generated.
The profile also applies lossless data-oriented lowerings that coalesce zero
stores in selected core constructors, plus compact encodings for the
four-symbol cursor atlas, key-name pointer table, and CRC-32.
Its differential gate compiles the same profile on both the native and
translated sides and requires exact state, draw-command, resource, and pixel
agreement. `translator-embedded-size` links a tiny native-C application with
`-Oz`, LTO, and dead stripping and writes its section report and linker maps to
`build/embedded-size/`.

`translator-embedded-compact` is the smallest selectable-module profile. It
retains every widget family and parses ordinary TrueType `glyf` fonts at
runtime. Programmatic focus remains differential-tested; the full profile
remains available for directional/gamepad/window-switching navigation, INI
persistence, interactive debug tools, and CFF/Type2 outlines. Context hooks
and malformed-scope recovery machinery remain linked in the compact target.
The compact translator coalesces typed `ImVector<T>` growth operations into a
small element-size-driven runtime and removes CFF-only cubic tessellation from
the TrueType rasterizer. Its `trap_calls` lowering replaces formatted
recoverable-error logging with a single compiler trap, and
`check_baseline.py` proves that path with an intentional unmatched `End()`.
The size gate fails unless the complete linked code-and-data section total is
below 100 KiB.

`translator-test-engine` compiles the pinned official Dear ImGui Test Engine
and Test Suite once against native C++ and once against the five translated
core/demo C89 units plus the generated facade. Both execute the same official
suite; the current baseline is 415/415 in each mode. The Test Engine is fetched
into `build/` rather than vendored and retains its own upstream licensing
terms.

`translator-docking` applies the same eight-unit strict-C89 build to the pinned
upstream docking branch, plus the official SDL_GPU3 renderer backend, for nine
generated units. The docking
differential creates a dockspace and performs the same trace and pixel-byte
comparison. `translator-docking-test-engine` runs 456/456 official tests in
both modes and additionally compares the documented `-viewport-mock viewport`
run (11/11 in each mode).

`translator-viewport-sdlgpu` runs an additional native-versus-translated test
through real SDL windows and SDL_GPU (Metal on the development Mac). It forces
a detached Dear ImGui window, observes platform and renderer create/destroy
callbacks, checks live backend data, renders the secondary swapchain, and
requires identical lifecycle counts.

Every full translation emits the canonical `imgui_c89.h`. Its exact C
definitions use stable non-overloaded names, give ordinary calls an explicit
`ImGuiContext*`, and cover public namespace functions plus ordinary public
non-template record methods. Variadic spellings use their `va_list` siblings.
Public enumerators keep their exact upstream spellings in anonymous C89 enum
blocks while their integer storage typedefs remain ABI-compatible. Constants
from internal headers and implementation sources live in
`imgui_c89_internal.h`, which is included only by generated C units. Public
name collisions fail translation; genuine private collisions receive readable
source/owner-qualified names instead of identity hashes.
The same provenance boundary applies to types and functions: `imgui_c89.h`
contains public `imgui.h` layouts, their by-value dependency closure, and the
exact readable `imgui_*` entry points. Pointer-only internal types remain
forward declarations. Complete internal layouts, raw cross-unit functions,
globals, and translator support declarations are emitted only in
`imgui_c89_internal.h`. Template record names encode their element types
directly (`_ptr` for pointer elements), and local-name collisions are qualified
by source location. Consequently the canonical public header has no
identity-hash-derived tokens.
The current docking profile exposes 647 C89 entry points and has a C-only two-frame draw
smoke that does not link the C++ facade. Its call-graph pass threads an ambient
context through 1,822 ordinary docking functions. The C API calls those
symbols directly without selecting or restoring `GImGui`; its smoke leaves a
different context current while rendering and verifies that global selection
does not change. Address-taken callbacks keep their upstream ABI and derive
context from `ImGuiContext`, input-text, draw-list, font-atlas, or viewport
owners when those fields exist in the extracted revision. Ownerless legacy
callbacks retain the upstream global-context behavior at that fixed boundary.
The exact C++ facade remains the bootstrap oracle: it includes `imgui_c89.h`
in declaration-only mode, obtains the upstream current context, and forwards
into those same C definitions. Private facade endpoints use readable
`imgui_i_*` names; no hash-derived symbol is present at the facade boundary.
Template-specialization methods such as `ImVector<T>` stay internal to the
translated core.

Size-sensitive helpers follow their C++ linkage model: header-defined,
field-only value constructors are emitted as translation-unit-local C89
helpers, while cross-unit constructors retain external adapters. Clang's
branch-prediction identity intrinsic lowers to an expression macro, avoiding
an out-of-line call at every use site.

The plain `imgui_begin()` and `imgui_begin_child_*()` definitions preserve
upstream semantics, including the unconditional matching End call. The
optional `imgui_c89_api.{h,c}` convenience layer contains only three
`*_scope()` helpers. They close an inactive raw scope internally and return
`imgui_scope`, so callers using those helpers call End only for
`IMGUI_SCOPE_ACTIVE`. There is no duplicated widget implementation path.

`translator-portability` compiles every generated unit and the native API with
GCC in strict C90 mode, links those GCC objects to the Clang-built exact C++
facade, runs both C and C++ smokes, and generates compile-time size/alignment/
field-offset assertions directly from Clang's record-layout IR. The Linux CI
profile repeats the dependency-free core gate in both LP64 and `-m32` ILP32.

`translator-object-sizes` recompiles the baseline and docking native C++ with
`-Os` and the translated C89 with `-Os -fno-unwind-tables
-fno-asynchronous-unwind-tables`. C has no exception unwinding, so this is the
recommended size-build configuration; omit the latter flags when platform
backtraces or profiling require unwind metadata. It writes physical object-file and
section-byte totals to `build/object-sizes/report.{md,json}`. The generated C++
facade is measured separately and deliberately excluded from translated-C
totals.

`translator-idiomatic-c89-size` fetches the immutable release recorded as
`latest_release`, translates its four core units literally and with the
hand-maintained `patches/idiomatic_c89.json` overlay, verifies both C89/C++
facades, and measures complete static archives with identical `-Os`,
no-exception, and no-unwind-table flags. `translator-idiomatic-c89-test-engine`
applies the same overlay under the official Test Engine configuration and runs
the native and exact-C++-facade control pair.

The idiomatic v1.92.9b build keeps all features and assertions while reducing
the four-core-unit C89 result from 993,272 to 825,600 archive bytes and from
579,186 to 515,582 loaded section bytes. Its largest generic lowerings share
checked `ImVector` operations and encode exact assertion metadata in TU-local
byte-pair dictionaries; both retain the assertion backend found in extracted
IR, including the official Test Engine logger/debugtrap configuration. See
`docs/IDIOMATIC_C89.md` for the complete byte table, semantic
guards, and rejected experiments.

`translator-size-opportunities` adds a reproducible Bloaty/LLVM analysis over
the idiomatic objects. It emits per-object and per-symbol attribution, exact
machine-code clone groups, normalized extracted-IR clone groups, remaining
template families, and compiler merge/outliner upper bounds under
`build/size-opportunities/`. Install Bloaty with `brew install bloaty` on
Homebrew systems. These compiler transforms are discovery oracles only; they
do not change the idiomatic-build compiler flags or distribution.

The historical `translator-latest-release-patched*` and
`translator-canonical*` Make targets remain aliases for existing scripts. New
automation should use the `translator-idiomatic-c89*` names.

`translator-vendor` extracts only Dear ImGui's four core translation units
with the modular compact profile, so backend types never enter the generated C
header. It assembles `build/vendor/imgui-c89/`, containing the strict-C89 core,
the explicit-context C API, a static-library Makefile, license and provenance,
an external test font, and a standalone smoke test. The target rejects any
accidental SDL reference in the public generated header and builds the copied
directory without SDL or the C++ facade.

## Generated-file rule

Files below `build/` or a future top-level `generated/` directory are disposable
outputs. Fix the extractor, IR pass, or emitter and regenerate; never patch an
emitted file.
