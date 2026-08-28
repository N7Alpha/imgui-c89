# Dear ImGui generated C89 port

This repository provides a reproducible idiomatic C89 implementation generated
from a pinned Dear ImGui release. Behavioral compatibility comes from
translating the upstream implementation, not independently reimplementing its
algorithms.

The bootstrap milestone deliberately preserves the Dear ImGui C++ source API:

```text
upstream C++ source
        |
        v
Clang semantic extractor -> versioned ImGui IR -> C89 lowering passes
                                                  |             |
                                                  v             v
                                         generated C core   C++ facade
                                                  \             /
                                                   unmodified demo/tests
```

The C++ facade is an instrumental compatibility oracle. It contains spelling
adaptations only--namespaces, overloads, default arguments, references,
constructors, methods, and public template conveniences. It must not contain
widget or state-machine behavior. The unmodified upstream demo and test suite
must behave identically when linked either to upstream C++ or to the generated
C core.

After literal translation is green, the native C89 interface is introduced in
small, independently tested layers. The literal compatibility mode remains
available permanently so architectural changes can always be checked against
upstream behavior.

The ready-to-embed source package is checked in under
[`dist/imgui-c89/`](dist/imgui-c89/). Build it with `make -C dist/imgui-c89`
or compile its five `src/*.c` files as C89 in your own build. Those files are
generated and must not be edited directly.

## Status

The instrumental milestone is complete for both pinned `master` and the
`docking` branch. Master generates eight independently compiled C89 files;
docking generates nine by adding the official SDL_GPU3 renderer. Both include
all four core library files, `imgui_demo.cpp`, the dependency-free null
backend, and the SDL3 plus SDL_Renderer3 platform/renderer pair. An
exact-header C++ facade links those C objects. The runtime gate creates a
hidden SDL window and renderer, executes a context/frame/demo/render/shutdown
cycle, and submits the translated draw data through SDL. The docking profile
includes docking and multi-viewport code in the core and SDL3 backend. The same
complete core/demo pipeline passes the locked 1.83--1.92 compatibility matrix.

The generated C89 core now carries an explicit context through the inferred
call graph. The docking profile threads 1,822 ordinary functions and preserves
34 fixed ABI signatures (callbacks plus lifecycle/current-context entry
points). Fixed callbacks derive context from their owner object when the
upstream revision provides one; genuinely ownerless compatibility callbacks
retain upstream `GImGui` semantics. The 647-entry docking native C API calls the
threaded C core directly and never selects/restores `GImGui`. The exact C++
facade remains intact and passes its current upstream context into the C core.
Template-specialization methods such as `ImVector<T>` remain translated
implementation details instead of becoming accidental native-C entry points.
The native API also removes Dear ImGui's legacy `Begin`/`BeginChild` exception:
those calls return `imgui_scope`, close an inactive raw scope internally, and
require `End`/`EndChild` only for `IMGUI_SCOPE_ACTIVE`. The exact C++ facade
continues to require its upstream unconditional `End` calls, preserving it as
the behavioral oracle.

The current upstream header requires C++11, so its exact facade is compiled as
C++11. Release families 1.83--1.86 still prove the facade generator in C++98
mode; the implementation behind every facade is strict C89.

The native and translated implementations also run the exact same ten-frame
C++ differential scenario. The gate compares pointer-free internal state,
texture-resource metadata and pixels, every command/index/vertex bit pattern,
and 24,576,000 deterministic software-rasterized RGBA framebuffer bytes. The
scenario covers
scripted mouse, keyboard, and text input; controls, tables, child clipping,
popups, custom drawing, atlas and application-owned texture sampling, multiple
windows, and `ShowDemoWindow()`.

The official pinned Dear ImGui Test Engine builds unmodified against both
implementations. `master` passes 415/415 natively and through translated C89;
the docking revision passes 456/456 in both modes. Its documented headless
viewport mock passes 11/11 in both modes. A docking-specific differential also
creates a dockspace every frame and byte-compares 37,498 normalized records and
24,576,000 rasterized RGBA bytes.

The previous handwritten behavioral port is retained in `src/`, `include/`,
and `tests/smoke.c` as a prototype and source of reusable test/rendering
infrastructure. It is no longer the normative implementation and should not
receive new parity features.

The translator lives under `translator/`. Generated files belong under
`generated/` and are never edited by hand. The exact input revision is recorded
in `UPSTREAM.md` and `translator/upstream.lock.json`.

The maintained idiomatic release is Dear ImGui v1.92.9b translated to strict
C89 and then processed by the tracked, fingerprint-checked overlay. It retains
every core feature and assertion path, passes the official 415-test suite
through the exact C++ facade, and occupies 812,592 archive bytes / 516,999
loaded bytes under the documented like-for-like `-Os` build. The literal
translation remains the upstream-derived semantic and maintenance oracle.

## Development gates

The completed gates cover deterministic generation, strict-C89 compilation,
exact-header C++ facade linking, translated-demo execution, native-versus-
translated state/draw/resource/pixel comparison, focused translator fixtures,
the official Test Engine on master and docking (including viewport mock), and
the configured ten-release compatibility matrix.

See [`docs/DESIGN.md`](docs/DESIGN.md) for the architecture and
[`docs/PORTING.md`](docs/PORTING.md) for the implementation sequence.

Run the master gates with `make check`. Docking has separate
`translator-docking`, `translator-docking-differential`, and
`translator-docking-test-engine` targets. `make translator-object-sizes`
rebuilds native objects with `-Os` and translated C objects with unwind tables
disabled; its report explains that size/backtrace tradeoff and explicitly
excludes the generated C++ facade from the C89 total.

Use `make translator-idiomatic-c89` for the maintained C89 build,
`make translator-idiomatic-c89-size` for the like-for-like
C++/literal/idiomatic byte report, and
`make translator-idiomatic-c89-test-engine` for the official test gate.
`make translator-size-opportunities` runs Bloaty plus LLVM clone and outlining
analyses to rank the next safe factoring work. See
[`docs/IDIOMATIC_C89.md`](docs/IDIOMATIC_C89.md) for the maintenance contract.
The upstream rebase procedure is documented in
[`docs/UPDATING_UPSTREAM.md`](docs/UPDATING_UPSTREAM.md).

`make translator-embedded` builds a size-oriented target from the same IR and
requires exact native-versus-translated draw and pixel parity. It keeps runtime
TTF parsing and every widget implementation, but compiles without assertions,
omits automatic recovery from malformed Begin/End nesting, and requires the
application to supply a TTF rather than linking the built-in compressed fonts.
The literal target remains the full-behavior oracle. Run
`make translator-embedded-size` for a dead-stripped minimal-application report.

`make translator-embedded-compact` builds the sub-100-KiB linked profile. It
keeps all widget implementations and runtime TrueType outline rasterization,
including the navigation core required by programmatic focus, while selecting
out directional/gamepad/window-switching navigation, INI persistence, context
hooks, interactive debug tools, and OpenType CFF/Type2 parsing. Those optional
modules remain in the full `embedded` and literal profiles. Typed `ImVector<T>`
growth operations lower to a small element-size-driven C89 runtime, and the
TrueType-only path removes the unreachable CFF cubic-curve tessellator.
Recoverable user errors become compact one-instruction debug traps; the gate
deliberately triggers one to verify the panic path. The size report treats 100
KiB as code plus all linked data, not instruction bytes alone.

## Existing prototype

The old prototype can still be built with `make legacy-check`. Its API coverage
record remains in [`docs/API_COVERAGE.md`](docs/API_COVERAGE.md) for historical
and salvage purposes. The trace protocol, packet comparison code, reference
software renderer, font fixtures, and differential-runner work may be reused by
the generated track after their assumptions are audited.
