# Generated Dear ImGui C89 architecture

Status: governing design

## 1. Objective

Translate a pinned Dear ImGui implementation into strict C89 while preserving
its observable behavior and C++ source-level interface through a generated thin
facade. The translator is the maintained product; generated C is a build
artifact.

The project subsequently exposes a first-class native C89 interface and may
remove selected architectural debt. Those changes are applied only after the
literal translation is a passing behavioral oracle.

## 2. Compatibility contracts

The literal mode targets:

- Equivalent widget, layout, input, navigation, docking, settings, font, and
  draw behavior.
- Equivalent public structure semantics and application-visible mutation.
- Source compatibility for supported Dear ImGui C++ clients.
- The ability to compile unmodified `imgui_demo.cpp` and the official test
  suite against the generated implementation.

It does not promise C++ binary ABI compatibility with precompiled objects.
Backends and applications are rebuilt against the generated facade.

The native mode targets a coherent strict-C89 API. It need not preserve C++
spelling, but it must remain representationally complete: anything supported by
upstream must remain expressible or have an explicit, tested exclusion.

## 3. Three-layer implementation

### 3.1 Generated faithful core

Clang parses and type-checks upstream under a pinned configuration. A semantic
extractor writes a small, versioned ImGui-specific IR. Deterministic lowering
passes convert that IR to C89.

The faithful core preserves upstream algorithms and quirks. Generated files
carry source provenance and are never manually edited.

### 3.2 Generated compatibility surfaces

The raw C ABI may initially use mechanical names. A C++ facade reconstructs
namespaces, overloads, default arguments, references, constructors, methods,
and required public template conveniences. It contains no behavioral logic.

An upstream exception is allowed only as a declarative rule that states:

- The upstream version range.
- The semantic or source pattern being matched.
- The expected number of matches.
- The generated replacement.
- A focused regression fixture.

Rules fail generation when their expected match count changes.

### 3.3 Native C89 facade and extensions

The native interface is layered over the faithful core. Candidate improvements
include explicit contexts, semantic non-overloaded names, opaque internals,
uniform scope pairing, managed text buffers, and immutable typed render
packets.

Changes that merely lower C++ mechanisms may become translator passes. Changes
that alter ownership, lifetime, sequencing, or visible contracts remain
adapters/extensions unless moving them into the core has a proven benefit.

The first such adapter normalizes conditional scope closure. Native `Begin`
and `BeginChild` return active/inactive/error scope results. If the faithful
upstream call returns inactive, the native adapter immediately performs the
legacy mandatory `End`/`EndChild`; native callers end active scopes only. The
raw translated core and exact C++ facade retain the upstream contract, so this
behavioral improvement cannot weaken compatibility testing.

## 4. Parser boundary

Production translation uses a small C++ LibTooling executable, not diagnostic
AST JSON and not a handwritten C++ parser. It uses Clang semantic declarations,
types, instantiated templates, implicit conversions/lifetimes, record layouts,
source locations, and preprocessing callbacks.

The extractor serializes only the stable information required by our lowering
passes. Clang AST node addresses and diagnostic dump structure never enter the
IR contract. Python orchestrates extraction, validation, lowering, generation,
version rules, and tests.

The Clang major version is recorded in the generated manifest. Upgrading Clang
is an explicit compatibility event with deterministic-output review.

## 5. Initial lowering boundary

The translator supports the closed C++ subset used by the selected upstream
releases. Required pass families include:

- Namespace and symbol normalization.
- Record methods and implicit `this`.
- References and member access.
- Constructors, destructors, and automatic lifetime cleanup.
- Overloads and default arguments.
- Finite template monomorphization, including `ImVector<T>` specializations.
- Operators and implicit conversions.
- `auto`, range-for, lambdas, and other used C++11 constructs.
- C89 declaration placement, comments, booleans, constants, and syntax.
- Call-graph-based explicit context threading. Ordinary functions gain a
  generated ambient-context parameter; address-taken callbacks keep their ABI
  and recover context from structurally detected owner fields where possible.
  Lifecycle and genuinely ownerless legacy callbacks remain compatibility
  roots in the upstream current-context global.

Unsupported constructs are diagnosed with source locations. They are never
silently copied into C output.

## 6. Configuration and preprocessing

The first parity milestone freezes one exact `imconfig.h`, compile-definition
set, platform, scalar layout, and upstream commit. The preprocessor may resolve
that configuration before extraction.

Configuration breadth is added only after literal parity. Each supported
configuration is a named profile. If multiple profiles must share one emitted
source distribution, conditional regions are preserved or merged as a
separate, tested capability rather than inferred accidentally.

## 7. Verification

Each upstream revision produces two separate executables driven by the same
scenario:

- Reference: original Dear ImGui C++.
- Candidate: generated C89 core plus C++ compatibility facade.

Compare lifecycle outputs, public state, internal diagnostic state where
available, settings bytes, draw commands, vertices, indices, texture changes,
and framebuffer pixels. Mismatches report the first differing frame and field.

The official test engine is a high-value acceptance suite, subject to its
separate license. Focused translator fixtures remain necessary because
behavioral tests do not guarantee that every lowering edge is exercised.

## 8. Version matrix

"Ten minor revisions" means the newest selected stable patch from each Dear
ImGui release family 1.83 through 1.92, with exact commits recorded in a lock
file. A matching test-engine commit or an explicitly versioned substitute is
recorded for every row.

The newest pinned release is developed first. The historical matrix becomes a
gate before the translator is considered robust; it is not necessary for the
first function-level lowering fixture.

## 9. Generated-source policy

- `generated/` is disposable and reproducible.
- Generated files include translator, Clang, upstream, profile, and IR schema
  versions.
- A clean regeneration must be byte-identical.
- Formatting is deterministic and performed by the emitter, not clang-format.
- Source maps connect generated functions and diagnostics to upstream files.
- Upstream license and attribution accompany translated output.

## 10. Existing handwritten prototype

The handwritten port is frozen as non-normative prototype code. Useful pieces
may be adopted only after an audit demonstrates that they do not replace
upstream behavior with approximations. Its tests become supplemental native-API
tests, not evidence of upstream parity.
