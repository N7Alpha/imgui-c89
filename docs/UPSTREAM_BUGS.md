# Upstream Dear ImGui bug log

Only defects reproduced in the pinned native C++ build belong here. Translator
failures, compact-profile policy changes, and differences caused by a lowering
are recorded in their own test output instead.

## Confirmed upstream bugs

None observed so far.

Last audited against native-versus-translated draw-command, software-render,
and deterministic-state differentials on 2026-08-06. No divergence has yet
survived reproduction in the pinned native C++ build, so no upstream issue is
currently actionable.

## Investigated translator findings

| Finding | Resolution |
|---|---|
| Explicit C API calls changed `GImGui` | `compact_global_context` violated the translated API's multi-context contract; removed from production profiles. |
| Input focus/pixels diverged on frame 5 | Omitting upstream `NavUpdate()` prevented `SetKeyboardFocusHere()` from activating the input; its required navigation core was restored and exact differential parity recovered. |
| Minimal fixture translation raised `StopIteration` | The optional compact-vector runtime resolved Dear ImGui allocator symbols even when that lowering was disabled; the lookup is now guarded by the profile option. |
| A CFF-restored size probe was labeled TrueType-only | Size-report feature metadata came from the production profile instead of the generated probe manifest; reports now prefer the manifest used to emit the measured C. |
| CFF's local POD subroutine stack was zero-filled | Clang represents default-initialization of a trivial record array as a trivial `CXXConstructExpr`; the emitter now preserves C++'s uninitialized storage instead of treating it as value-initialization. |
| Disabling Dear ImGui's built-in file helpers broke the runtime-TTF smoke | `IMGUI_DISABLE_FILE_FUNCTIONS` also removes `AddFontFromFileTTF`; the compact profiles retain it because exact public-interface parity takes precedence over dropping stdio convenience code. |
| Differential metadata always reported ten frames | The runner had a stale literal even when the fixture length changed; it now derives the frame count from the verified framebuffer stream. |

## Toolchain findings

| Finding | Status |
|---|---|
| Apple Clang 17 and Homebrew LLVM 22 crash while compiling generated `imgui.c` when LLVM's IR outliner is combined with `-mllvm -aggregate-extracted-args` | Reproduced in both compiler families. The production size flags do not use this optional outliner mode. This is not a Dear ImGui defect. |
| LLD 22's Mach-O LTO path rejects Apple Clang 17's encoded `-Oz` option and then crashes in AArch64 code generation when linking the generated bitcode | Reproduced while probing identical-code folding. Matching Homebrew Clang/LLD 22 links successfully, but ICF is still 716 bytes larger than the Apple production toolchain result. |
