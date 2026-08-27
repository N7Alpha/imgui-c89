# Upstream baselines

The primary translation target is Dear ImGui `master` at:

- Repository: <https://github.com/ocornut/imgui>
- Commit: `9b7699f32597e7c7a799f22b1860ac586c2857b9`
- Commit date: 2026-07-31
- Expected version family: 1.92.9

The machine-readable authority is `translator/upstream.lock.json`. Fetch by
commit, never by a moving branch:

```sh
python3 translator/fetch_upstream.py --revision baseline
```

The lock file also selects one exact stable revision from every release family
1.83 through 1.92. These selections form the translator compatibility matrix;
changing any row is a reviewed compatibility event.

Docking and multi-viewport development is separately pinned to the upstream
`docking` branch at commit
`b48d1afbe8ee8b238e2961dc363a949dd7304e23`. Fetch it with:

```sh
python3 translator/fetch_upstream.py --revision docking
```

The official Test Engine/Test Suite is pinned separately:

- Repository: <https://github.com/ocornut/imgui_test_engine>
- Commit: `71d29895426566c2fca058399572edb720d2b466`

Fetch it with `python3 translator/fetch_upstream.py --revision test_engine`.
It is an external acceptance dependency with its own licensing terms and is
kept under disposable `build/` output, not vendored into this repository.

## Translation boundary

The pinned upstream source is the normative implementation and the source
template for generated C89. Literal compatibility mode preserves its algorithms,
state transitions, ordering, quirks, and observable public structures. Manual
edits to generated output are forbidden.

The C++ source facade is generated from the same semantic IR and contains no
behavioral implementation. Its purpose is to compile upstream examples, demo,
and tests against the generated C core.

Native C89 redesign begins only after literal mode is passing. Native adapters
may change spelling, ownership, and protocols, but the literal mode remains as a
permanent behavioral oracle.

## Relevant upstream surfaces

- `imgui.h`: public API, public structures, inline methods, templates.
- `imgui_internal.h`: internal records, helpers, and invariants.
- `imgui.cpp`, `imgui_draw.cpp`, `imgui_widgets.cpp`, `imgui_tables.cpp`: core
  translation units.
- `imgui_demo.cpp`: unmodified C++ facade acceptance client.
- `docs/CHANGELOG.txt`: language and compatibility transitions.
- Dear ImGui Test Engine/Test Suite: external behavioral acceptance suite with
  its own licensing terms.

The old handwritten prototype retains its pinned stb_truetype and ProggyClean
fixtures under `third_party/`. Those dependencies are not automatically part of
the generated core; upstream font behavior is authoritative for literal mode.
