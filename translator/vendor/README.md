# imgui-c89 vendored core

This directory is a self-contained, generated strict-C89 build of Dear ImGui's
four core translation units plus the explicit-context C API. It has no SDL or
other window-system dependency. Do not edit generated files under `include/`
or `src/`; regenerate the bundle from the translator instead.

## Build and smoke test

```sh
make
make check
```

The only link dependency is the platform math library (`-lm` where required).
`make check` does that twice: once in the compact default configuration and
once after enabling every optional provider. Each run loads
`fonts/ProggyClean.ttf` at runtime, creates a context, renders two frames,
verifies draw data, and destroys the context.

To embed it in another build, compile every file under `src/` as C89, add
`include/` to the include path, and link the resulting objects with the math
library. Application code includes `imgui_c89.h`; include `imgui_c89_api.h`
as well only when using the optional normalized scope helpers.
`imgui_c89_internal.h` is shipped solely because the generated library sources
include it; application code should not include that private header. It owns
all internal record definitions, raw cross-unit helpers, globals, and
translator support declarations. The application-facing `imgui_c89.h`
contains only public layouts/dependencies and the clean `imgui_*` API.

## Compact modules

Every widget family and runtime TrueType loading/rasterization are present in
the default linked build. Directional/gamepad/window-switching navigation, INI
settings, and OpenType CFF/Type2 outlines are generated as link-selectable
providers. Call this immediately after creating a context when those features
are required:

```c
ctx = imgui_create_context(0);
imgui_enable_full_features(ctx);
```

If that call is absent, a size-optimizing linker can discard the providers.
Keep `-ffunction-sections -fdata-sections` and use `--gc-sections` on ELF or
`-dead_strip` on Mach-O for the compact result.

The built-in compressed default font is deliberately absent. Supply a normal
TTF using `imgui_font_atlas_add_font_from_file_ttf()` or one of the memory-font
entry points. The included ProggyClean file exists for the smoke test and is
not compiled into the library.

## Scope convention

`imgui_begin()` and both `imgui_begin_child_*()` functions preserve Dear
ImGui's exact semantics: always call the matching end function, including for
an inactive result. The optional `*_scope()` helpers from `imgui_c89_api.h`
close inactive scopes internally and return `imgui_scope`; call the matching
end function only for `IMGUI_SCOPE_ACTIVE`.

`PROVENANCE.json` identifies the exact upstream commit and translator profile.
`SHA256SUMS` covers the copied package files.
