# Dear ImGui idiomatic C89 distribution

This directory is the generated, source-only distribution of the maintained
idiomatic C89 port. It contains Dear ImGui's four complete core translation
units, the explicit-context C API, and the three optional normalized-scope
helpers. No Dear ImGui feature or assertion path is disabled.

Do not edit files under `include/` or `src/`. Changes belong in the translator
or the fingerprint-checked overlay in the repository, after which this bundle
is regenerated with `make dist-idiomatic-c89`.

## Build

```sh
make
make check
```

Compile every file under `src/` as C89, add `include/` to the include path, and
link the resulting objects with the platform math library (`-lm` where
required). Applications include `imgui_c89.h`; `imgui_c89_api.h` is needed
only for the optional normalized-scope helpers.

`imgui_c89_internal.h` is shipped because the generated implementation files
include it. Application code should use the public `imgui_c89.h` surface.

## Scope convention

The exact `imgui_begin()` and `imgui_begin_child_*()` entry points preserve
Dear ImGui's unconditional matching-End convention. The optional `*_scope()`
helpers close inactive scopes internally and return `imgui_scope`; call the
matching end function only for `IMGUI_SCOPE_ACTIVE`.

`PROVENANCE.json` records the translator profile, exact upstream commit, and
translator manifest. `SHA256SUMS` covers every distributed file.
