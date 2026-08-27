# Idiomatic C89 ABI

The generated C implementation now has one exact semantic entry point for
each representable public Dear ImGui operation. Native C and the generated C++
facade call the same definition:

```text
C caller ────────────────┐
                         ├── imgui_* exact C definition
C++ syntax/context thunk ┘
```

## Headers and names

- `imgui_c89.h` is the maintained C89 header. Public overloads receive stable,
  readable suffixes derived from the parameter positions that distinguish
  them. Ordinary operations take an explicit `ImGuiContext *` when the
  translator's context-flow analysis proves that they consume ambient state.
- The C++ facade includes the same header with
  `IMGUI_C89_USE_CPP_TYPES` after upstream `imgui.h` and `imgui_internal.h`.
  This declaration-only mode reuses upstream C++ record types and prevents a
  second hand-maintained ABI declaration list.
- Facade-only constructors, destructors, internal APIs, and backend endpoints
  use stable private `imgui_i_*` names. The generated facade contains no
  identity-hash-suffixed C symbols.
- `imgui_translated.h` is only a compatibility include for `imgui_c89.h`.
  New code should not include it.

Public enumerators from upstream `imgui.h` keep their exact Dear ImGui
spellings and are emitted in anonymous C89 `enum` blocks. Their storage aliases
remain integer typedefs, matching Dear ImGui's ABI and flag-combination rules.
The translated implementation includes `imgui_c89_internal.h`, which contains
constants originating in `imgui_internal.h`, implementation sources, and the
bundled stb headers. Application code must not include that private header.

The generator rejects public enumerator collisions. A genuine private
collision receives a readable source/owner-qualified `imgui_i_*` spelling;
identity hashes are never used for enum constants. If a future enumerator is
outside C89's signed `int` range, it is emitted as an integer macro instead of
silently changing its value or storage ABI.

Hash-derived names may still appear on translation-unit-local implementation
helpers and other private lowering artifacts. They are not part of the C or
C++ facade ABI. Template-specialization records now use structural names such
as `ImVector_ImDrawList_ptr`; anonymous and genuinely colliding local records
use readable source/line/column qualification.

The public header is also a real declaration boundary, not a dump of the
translator's entire type universe. Ordinary records originating in `imgui.h`
form its roots. The generator recursively includes definitions required by
by-value fields and emits only forward declarations for pointer-only
dependencies. Definitions from `imgui_internal.h`, stb, and implementation
sources, all raw cross-unit helpers, globals, and translator support routines
live in `imgui_c89_internal.h`. On the current core this partitions 67 complete
public records from 172 private records and reduces `imgui_c89.h` from 584,532
to 187,876 bytes. The public header contains no identity-derived names.

## Exact semantics and optional convenience

Plain `imgui_begin()` and both `imgui_begin_child_*()` functions preserve
Dear ImGui's exact rule: the matching End function is required even when Begin
returns false. This exact path is what the C++ facade calls.

`imgui_c89_api.h` and `imgui_c89_api.c` now contain only three optional
`*_scope()` helpers. A helper closes an inactive Begin internally and returns
`IMGUI_SCOPE_ACTIVE`, `IMGUI_SCOPE_INACTIVE`, or `IMGUI_SCOPE_ERROR`. This
keeps the normalized C policy available without maintaining two versions of
the underlying widget operation.

## Upstream update behavior

Stable names are regenerated structurally from qualified names, record
owners, and overload parameter types. A name collision is a translation error;
the generator does not silently add a hash fallback. This makes upstream API
changes visible in review and keeps the maintained handwritten patch focused
on implementation changes rather than ABI glue.

The generated manifest records public and facade-private exact-C function
counts. The baseline gate also rejects any hash-suffixed symbol in the C++
facade and verifies that the facade consumes `imgui_c89.h`.

## Validation requirements

Every ABI migration must retain all of these gates:

- deterministic generation and independent strict-C89 compilation;
- the native C two-context smoke;
- the exact C++ facade smoke;
- the 57,270-record native/translated differential trace;
- all 415 Dear ImGui Test Engine cases in native and translated modes;
- like-for-like `-Os` core, optional-scope-helper, and C++-facade size reports.
