# Translator implementation status

This document records the literal-translation milestone, generated native C
API, and first internal explicit-context pass. Broader opacity, ownership, and
protocol cleanup is not yet complete.

## Completed

- LLVM/Clang 22 LibTooling frontend builds locally.
- The frontend emits versioned JSON IR rather than consuming diagnostic AST
  dumps.
- Stable Clang USRs identify records, fields, functions, overloads, methods,
  and parameters.
- IR currently captures records and layouts, enums, functions, default
  arguments, constructor initializers, statement/expression trees, implicit
  casts, `this`, member calls, and project macro definitions/replacement tokens.
- A deterministic emitter lowers the complete pinned Dear ImGui core,
  `imgui_demo.cpp`, `imgui_impl_null.cpp`, `imgui_impl_sdl3.cpp`, and
  `imgui_impl_sdlrenderer3.cpp` to eight independent strict-C89 translation
  units.
- A generated source facade includes the exact upstream headers and implements
  their C++ API by forwarding into the C89 symbols. It carries no Dear ImGui
  behavior of its own.
- The reference C++ fixture and generated-C candidate produce the same result.
- Generation is run twice and checked byte-for-byte.
- Unsupported constructs fail with a source-located diagnostic.
- Exact commits are locked for release families 1.83 through 1.92.
- Full-body extraction, byte-identical two-pass C89 generation, five
  independent C compilations, and source-identical demo-enabled frame smoke
  pass in both native C++ and exact-facade/translated-C89 modes across all ten
  families. Facades use C++98 through 1.86 and C++11 from 1.87 onward,
  matching the minimum accepted by each upstream header.
- One source-identical ten-frame C++ scenario is compiled against native Dear
  ImGui and against the generated facade/C89 implementation. Both runs are
  deterministic and byte-identical across 37,000+ canonical records covering
  selected internal state, texture resources and atlas pixels, every draw
  command/index/vertex, and 24,576,000 byte-compared software-rendered RGBA
  framebuffer bytes. The scenario includes scripted pointer, keyboard, and
  text input, atlas and application-owned texture switching, plus the upstream
  demo.
- The official Test Engine passes 415/415 for master and 456/456 for docking in
  both native and translated modes. Docking's official viewport mock passes
  11/11 in both modes.
- The canonical latest-release, hand-maintained IR size profile keeps the complete
  v1.92.9b feature and assertion surface while reducing the four core strict-C89
  objects to 851,368 archive bytes / 524,281 loaded section bytes under
  like-for-like `-Os` and no-unwind settings. This is 152,440 archive bytes and
  54,905 loaded bytes below literal C89, and 172,544 archive bytes / 65,061
  loaded bytes below upstream C++. The same exact C++ facade passes all 415
  official tests, including the Test Engine's custom assertion backend. Bloaty
  and LLVM clone/outliner analysis provide reproducible per-object attribution
  and a ranked queue of future representation-preserving factoring work.
- The pinned docking profile also translates the official SDL_GPU3 renderer.
  A real native-versus-translated SDL_GPU test creates, renders, and destroys a
  secondary platform window with matching callback counts.
- GCC 15 compiles all nine docking units as strict C90. A generated ABI probe
  checks 3,040 size, alignment, and byte-field-offset facts for 299 records
  against the layouts captured by Clang.
- Constructor value/placement adapters are demand-driven and retain external
  linkage only when a generated translation-unit boundary requires it. The
  emitter recognizes header-defined field-only value constructors and emits
  their tiny helpers translation-unit-locally, matching C++ inline behavior.
  Clang branch-prediction hints lower to a C89 expression macro instead of an
  out-of-line identity call. The size report uses C's no-unwind-table build
  mode and excludes the exact C++ wrapper from translated totals.
- The canonical `imgui_c89.h` exact-C surface covers public namespace functions
  and ordinary public non-template record methods (647 entry points for the
  docking pin). These are the implementation symbols, not forwarding adapters.
  Public enum constants retain their upstream spellings in C89 enum blocks;
  private constants are isolated in `imgui_c89_internal.h`, and enum constants
  no longer use identity hashes.
  Public record definitions are limited to the upstream `imgui.h` roots and
  their by-value closure; internal layouts, globals, raw helpers, and translator
  support declarations live only in `imgui_c89_internal.h`. Template and local
  record tags use readable structural/location-qualified names, leaving zero
  identity hashes in the public header.
  Template-specialization methods stay internal to the translated core. Its
  C-only smoke builds the font atlas and produces draw data without the C++
  facade.
- Call-graph analysis materializes `GImGui` as a generated first argument on
  1,822 ordinary docking functions. Thirty-four fixed signatures retain their
  upstream ABI; callbacks derive context from available owner records and only
  lifecycle or ownerless legacy boundaries fall back to upstream `GImGui`.
  The native C API contains no context push/pop or `SetCurrentContext` adapter.
  A two-context smoke leaves one context globally current while rendering the
  other through explicit C calls and verifies both isolation and unchanged
  global selection.
- Plain `Begin` and both `BeginChild` overloads retain Dear ImGui's exact
  unconditional-end contract. Three optional `*_scope()` helpers return a
  tri-state result and close inactive raw scopes, giving C callers an explicit
  opt-in normalized convention. Both behaviors have runtime smoke coverage.
- A systematic embedded profile is generated from the same semantic IR. It
  keeps all widget families, runtime TTF loading, context hooks, and malformed-
  scope recovery machinery, while removing assertions and built-in compressed
  font payloads.
  Its caller-supplied TTF path passes an exact 37,498-record state/draw/pixel
  differential. Lossless constructor, immutable-local-table, cursor-atlas,
  key-name, CRC-table, style-palette, and typed-vector lowerings, plus a
  TrueType-only font module, reduce
  a strict-C89 `-Oz`/LTO/dead-stripped one-window executable to 101,185 bytes
  of code and data versus 253,207 for the literal profile on the development
  arm64 Mach-O toolchain. This is 1,215 bytes below the 100 KiB gate. The
  measured target retains every widget family, runtime TrueType parsing and
  rasterization, programmatic keyboard focus, and compact recoverable-error
  traps, context hooks, and malformed-scope recovery machinery; larger
  selectable modules remain available in the full profile.

- A dedicated `embedded_compact_vendor` profile extracts only the four core
  translation units, so SDL and backend declarations never enter its IR or
  generated public header. `make translator-vendor` assembles a self-contained
  3.3 MiB source bundle under `build/vendor/imgui-c89/` with the strict-C89
  core, 623-entry explicit-context C API, license/provenance/checksums, external
  smoke-test font, and standalone Makefile. Both its compact/default and
  full-provider C-only smokes pass after copying the directory outside the
  repository. Its optimized compact linked measurement is unchanged at 99,645
  code-and-data bytes.

- `embedded_compact_full` is the non-modular completion gate. It restores
  directional/gamepad/window-switching navigation, INI settings, context
  hooks, malformed-scope recovery, and both TrueType `glyf` and OpenType
  CFF/Type2 outlines in one generated target. The normal and CFF-font pixel
  differentials pass. Requiring the host to provide the optional platform
  open-in-shell callback removes Dear ImGui's desktop `fork`/`exec` default
  without removing the callback capability. Size builds also discard unused
  math `errno`/exception and signed-zero distinctions; normal and CFF pixel
  differentials remain exact under those flags. The differential includes a
  NaN-bearing plot and rejects both the otherwise smaller `-fno-honor-nans`
  build and fast floating-point contraction. A deterministic byte-pair
  lowering compresses the debugging-only key-name table and the differential
  enumerates all 155 decoded names against native. Shared element-size-driven
  vector mutation helpers also cover erase, unsorted erase, insertion, and
  push-front operations; both font-path differentials remain exact. The
  cursor's existing two-bit atlas representation is further byte-pair
  encoded and expanded only while the runtime atlas is built. Navigation's
  two fixed key arrays are emitted as checked enum ranges rather than stored
  data. The non-interactive Ctrl+Tab overlay now lowers its one public
  `Selectable()` call to an exact layout/render helper: the overlay is
  explicitly `NoInputs`, so this avoids retaining unreachable multi-select and
  interaction machinery while leaving the public widget intact for callers.
  The rule validates the upstream call shape and fails translation if the
  overlay becomes interactive or gains non-default flags/size. Forced-overlay
  normal and CFF differentials remain byte- and pixel-exact. The same checked
  lowering now writes the overlay's fixed next-window state directly and
  performs the padding push/pop with the original style-stack representation,
  avoiding four general API paths while preserving observable context state.
  This saves another 228 bytes. Two further checked navigation rules exploit
  the upstream-guaranteed contiguous direction/key enums: four directional
  move tests become one order-preserving loop, and the four X/Y wrapping
  branches share one axis implementation. Together they save another 188
  bytes. The differential now injects a keyboard move and forces the wrapping
  path through the original C++ API. Its current linked code-and-data total is
  118,030 bytes, 15,630 bytes above the 100 KiB
  requirement; the size
  command fails deliberately until that gap is removed.

  The embedded full profile now also uses upstream's
  `IMGUI_DISABLE_TIME_FUNCTIONS` contract. All settings date fields, INI text,
  cleanup policies, and APIs remain present; an embedded host that wants
  automatic age-based cleanup supplies `Platform_SessionDate` instead of
  pulling `time()`/`localtime_r()` into the core. This saves 124 linked bytes.
  Deterministic generation, strict C89/API/trap checks, and exact normal and
  CFF differentials pass with the contract enabled.

  The differential now also saves a live window/table INI image, clears all
  settings, reloads that image, and requires the native and translated builds
  to reproduce the same serialized byte count and hash. This closes the prior
  coverage hole where settings code was linked and sized but not behaviorally
  exercised.

  Controlled restoration from the selectable-module build
  attributes 11,332 bytes to full navigation/window switching, 5,435 bytes to
  window/table INI settings, and 5,013 bytes to CFF/Type2 outlines. Their
  current combined full-build cost is 21,284 bytes over the 100,222-byte
  selectable build; roughly 496 bytes are shared or optimized differently
  compared with the isolated restorations. These are attribution figures, not
  permission to remove the modules.

  A trial lowering replaced the three window/table INI `sscanf` readers with
  a shared deterministic C89 integer/float scanner. It was rejected: the full
  linked target grew from 121,506 to 121,853 bytes (+347). This establishes
  that the settings cost is dominated by storage, lifecycle, and serialization
  rather than those small parsing call sites.

  A separate policy probe removed only automatic INI disk load/save calls
  while retaining the complete in-memory settings machinery. It measured
  120,423 bytes, a 1,043-byte saving. This is not enabled: an exact C++ facade
  must first reproduce `IniFilename` behavior, while the compact C interface
  needs an explicit host-I/O contract that remains context-safe. The result is
  useful but too small to justify silently weakening parity.

  Broadening the existing zero-constructor lowering from 19 selected records
  to 53 was also rejected. The complete batch grew by 16 bytes; bisection
  found only instruction-alignment/outliner-scale changes (the best 4-record
  group saved 12 bytes). The production profile therefore keeps the smaller,
  evidence-backed constructor list rather than accumulating inert exceptions.

  Mach-O identical-code folding was measured with matching Homebrew Clang/LLD
  22. ICF removes 284 bytes relative to that linker's own non-ICF result, but
  the resulting 122,182-byte image is still 716 bytes larger than the Apple
  Clang/ld production result. It is therefore not a route to the target.

  Globally disabling function inlining was also rejected. It expands the same
  reachable image to 143,201 bytes (+24,535), confirming that the remaining
  large `NewFrame`/`Begin` symbols mostly reflect profitable interprocedural
  simplification rather than accidental duplicated inline bodies.

  Two narrower attempts to factor full-profile code were rejected as well.
  Replacing the three `RouteAlways` shortcut calls in `NavUpdateWindowing`
  with a checked specialized helper grew the image by 172 bytes, and forcing
  `UpdateSettings` out of line grew it by 44 bytes. LLVM already specializes
  and folds those paths more compactly than either source-level boundary.
  A fixed-format settings serializer experiment likewise grew the image by
  740 bytes. All three transformations were removed rather than retained as
  dormant compact-profile complexity.

  A like-for-like linked-symbol comparison against the 100,222-byte selectable
  build locates 4,796 bytes of the full-profile increase inside `NewFrame` and
  2,356 bytes inside the LTO-folded smoke entry point. The largest remaining
  standalone addition is the 2,056-byte Type2 charstring interpreter, followed
  by the table/window settings readers and writers. This points the next
  compact-runtime work toward cold CFF/settings state machines and away from
  more helper extraction.

  Further full-profile probes confirmed that this is not an ABI, relocation,
  or superficial source-shape problem. Keeping the eleven C entry points used
  by the size smoke out of line grew the image by 268 bytes; keeping only
  `NavUpdateWindowing` out of line grew it by 108 bytes. Re-enabling the old
  implicit-`GImGui` compact mode grew the image to 122,138 bytes even while the
  2,800-byte Ctrl+Tab overlay lowering was disabled, so explicit context
  threading is materially smaller as well as semantically cleaner. Fixed-
  address/no-PIC compilation, minimum alignment controls, and a direct Type2
  flex-operator source rewrite all produced byte-identical linked output.
  GCC 15's `-Os`/LTO result was also larger (124,464 instruction bytes before
  non-text sections) than Apple Clang's 113,212-byte text. The accepted profile
  therefore remains unchanged; crossing the gate requires a smaller semantic
  representation of the remaining cold state machines.

  The size fixture is not concealing a large host-program charge inside the
  LTO result. Compiling its `main` outside LTO makes the host contribution
  independently measurable at 416 bytes, but grows the linked image to
  119,010 bytes. The production 118,542-byte whole-program measurement is
  therefore retained: the large symbol labeled `main` in its linker map is
  reachable Dear ImGui behavior specialized into the caller, not fixture
  implementation or C++ wrapper code.

  Additional focused inlining probes were likewise rejected. Forcing the
  complete `NavUpdate` phase out of line grows the image by 292 bytes. Sweeping
  LLVM's size-inline threshold around the production value found no smaller
  result: thresholds 0, 2, 4, and 12 produced 118,830, 118,726, 118,726, and
  118,814 bytes respectively, while threshold 8 tied the 118,542-byte
  production result. The next reduction therefore remains a semantic compact
  runtime rather than another compiler-boundary exception.

  Promoting conservatively non-escaping C++ `bool` locals to C `int` was
  byte-identical, confirming that Clang already performs the useful scalar
  promotion. Disabling jump tables trades 132 data bytes for 168 instruction
  bytes and grows the total by 36 bytes. Neither probe is retained.

  Further backend controls did not expose hidden generic compiler waste.
  LLVM merge-functions and AArch64 global merging each grew the image by 8
  bytes, as did their combination. AArch64 jump-table compression, branch-
  target disabling, sink-folding, and load/store optimization probes were no
  better. Hot/cold splitting was byte-identical at both its default and zero
  threshold. The current backend has already reached the useful local minimum
  for these generic controls.

  Compact Type2/CFF source experiments found one small systematic lowering
  worth retaining. The translator now validates the exact outer-operator
  shape and consolidates thirteen repeated charstring stack-depth guards into
  a 32-byte minimum-stack table; an unexpected upstream shape is rejected.
  This reduced the production image by 96 bytes, from 118,542 to 118,446;
  the subsequent exact navigation-overlay setup lowering reduced it to
  118,218 bytes, and the checked directional/wrapping lowerings reduced it to
  118,030 bytes.
  Native and translated STIXGeneral OpenType/CFF rendering match across 55,790
  canonical state/draw/resource/pixel records. A direct flex helper saved only
  another 8 bytes in combination and was not promoted. Replacing the explicit
  clear-stack state with a derived condition gave back 48 bytes and was also
  rejected.

  Finally, a strict fixed-shape lowering for the five internal window-chrome
  `ButtonBehavior` calls removed the reachable 2,408-byte generic function,
  but caused LLVM to materialize `ItemHoverable` separately and duplicate the
  specialized path inside `End`. The final image grew from 118,542 to 118,642
  bytes; forcing the helper out of line was byte-identical. The probe remains
  build-only and is not a production transformation.

## Baseline inventory

For pinned commit `9b7699f32597e7c7a799f22b1860ac586c2857b9`, the merged
eight-unit program contains:

- 288 record entries and 104 enum entries.
- 5,416 unique function identities, of which 3,147 have translated
  definitions.
- 69 global entries and 238 captured project macros.
- Generated units for `imgui`, draw, tables, widgets, demo, the null backend,
  SDL3, and SDL_Renderer3.

Regenerate the baseline IR and rerun its acceptance gate with:

```sh
make translator-baseline
```

## Current acceptance commands

```sh
make check
make translator-baseline
make translator-embedded
make translator-embedded-size
make translator-differential
make translator-matrix
make translator-docking
make translator-docking-test-engine
make translator-docking-differential
make translator-portability
make translator-viewport-sdlgpu
```

`make check` includes the focused strict-C89/reference/facade fixture.
`translator-baseline` is the deterministic eight-unit current-source gate.
`translator-embedded` applies the compact profile and runs its exact
native-versus-translated differential. `translator-embedded-size` reproduces
the minimal linked-footprint report and linker maps.
`translator-differential` is the native-versus-translated state/draw/pixel
gate and is included by the default `make check` target.
`translator-matrix` is the ten-family full-translation gate.

## Deliberate limits of this milestone

- The baseline machine-backend gate uses SDL's dummy video driver and
  SDL_Renderer3. Docking additionally has a real-window SDL_GPU/Metal
  multi-viewport differential; the null backend remains the dependency-free
  fallback.
- Differential acceptance and the Test Engine are broad but cannot prove every
  host integration or GPU driver behavior.
- Fixed-signature callbacks that have no context or owner in their upstream ABI
  necessarily retain upstream `GImGui` behavior. Current revisions derive an
  owner structurally for input-text, draw-list, font-atlas, viewport, and
  explicit-context callback shapes; older record layouts are detected from IR
  rather than version checks. Hiding internals behind opaque types and final
  ownership/protocol cleanup remain future passes.
- Generated C is strict-C89-valid but still has non-fatal warning noise that
  can be normalized without changing semantics.
