# Third-party dependencies

The font-atlas implementation is intentionally based on the pinned
`stb_truetype.h` single-header implementation (the stb TTF parser/rasterizer).
It is already vendored here and is consumed by `src/imgui_c89_font.c`:

```text
third_party/stb_truetype.h
```

Only one C translation unit should define `STB_TRUETYPE_IMPLEMENTATION` and
include the header. Other translation units should include it without the
implementation define. Keep the exact stb revision recorded in `UPSTREAM.md`
and in the build manifest used for pixel-parity tests.

Do not add a replacement font rasterizer to this project. If a host already
provides a C89-compatible stb truetype include, the font backend may point its
include path at that copy instead, provided the revision and configuration are
identical for deterministic atlas output.

`ProggyClean.ttf` is retained as a reference fixture for differential font
tests. It is the font distributed with Dear ImGui's examples; it is not
loaded implicitly by the C89 core, whose font API accepts caller-owned TTF
bytes and textures.
