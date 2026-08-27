# Translator implementation and handoff guide

## 1. Bootstrap sequence

1. Fetch and verify the exact upstream commit from the lock file.
2. Freeze a baseline configuration and compile the upstream reference.
3. Build the LibTooling semantic extractor with the pinned Clang major.
4. Extract versioned IR and validate all referenced source files/hashes.
5. Lower the supported closed C++ subset to strict C89.
6. Generate the raw C ABI and C++ source-compatibility facade from the same IR.
7. Compile unmodified upstream demo/examples against the facade.
8. Compare reference and candidate behavior before adding native API changes.

Do not add new behavior to the handwritten prototype while bootstrap work is in
progress. Do not repair generated output manually.

## 2. Repository boundaries

```text
translator/                 translator source and focused fixtures
translator/clang_extract/   LibTooling semantic frontend
translator/py/              IR validation, lowering, and emitters
translator/profiles/        compile definitions and versioned exceptions
generated/                  disposable generated output
src/, include/              frozen handwritten prototype
tests/                      integration and legacy tests
```

Files under the ChatGPT project `sources/` mirror remain read-only.

## 3. Change discipline

Every translator feature includes:

- A minimal positive fixture.
- A negative fixture or explicit diagnostic where appropriate.
- IR assertions.
- Strict-C89 compilation of emitted code.
- A C++-facade compile/link/run check when the feature is public-facing.

Each exceptional rewrite additionally pins its upstream version range and
expected cardinality. Generated diffs alone are not an adequate test.

## 4. Translation order

Implement lowering in dependency order:

1. Scalar typedefs, records, enums, constants, and free functions.
2. Namespaces, overload identity, default arguments, and facade wrappers.
3. Methods, `this`, references, and operators.
4. Constructors, destructors, local lifetimes, and cleanup control flow.
5. Concrete `ImVector<T>` and helper-template monomorphization.
6. Remaining C++11 constructs observed by the inventory.
7. Full upstream core translation.
8. Demo and test-suite compatibility.
9. Explicit-context call-graph transformation.
10. Native C89 facade and extension layers.

## 5. Acceptance ladder

Use increasingly strong gates:

1. Extractor and IR fixture tests.
2. Generated fixture compiles with `-std=c89 -pedantic-errors`.
3. Fixture behaves identically in original C++ and generated C builds.
4. Upstream core compiles as generated C.
5. Unmodified demo compiles through the C++ facade.
6. Deterministic scenarios match state and draw bytes.
7. Official test suite passes.
8. Ten-release matrix passes generation, compilation, and its applicable tests.

## 6. Native interface work

Native C89 work starts only after ladder step 5. Every native change keeps the
literal C++ compatibility build green. Prefer adapters for ownership or
protocol redesigns and mechanical translator passes for implementation-only
changes.

The existing native API documents are design input, not proof that their
semantics are complete. Reuse their names or structures only after mapping them
to upstream behavior and adding differential tests.
