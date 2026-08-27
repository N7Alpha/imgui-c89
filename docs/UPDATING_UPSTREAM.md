# Updating the idiomatic C89 port

The maintained source of truth is not a textual diff against generated C. It
is the combination of:

1. `translator/upstream.lock.json`, which pins Dear ImGui;
2. the deterministic extractor and literal C89 translator;
3. `translator/patches/idiomatic_c89.json`, which identifies replacements by
   upstream qualified name, signature, and accepted body fingerprint; and
4. the C89 fragments under `translator/handwritten/`.

Files under `dist/imgui-c89/` are checked in for consumers, but they are
generated artifacts. Never edit them directly.

## Rebase procedure

1. Update only the `latest_release` tag and commit in
   `translator/upstream.lock.json`.
2. Run `make translator-latest-release`. This proves the new upstream source
   still translates literally before the idiomatic overlay is involved.
3. Run `make translator-idiomatic-c89`.
4. Review every rejected overlay entry. An unchanged body fingerprint applies
   automatically. A changed body, signature, or missing qualified name fails
   loudly and must be reconciled against the new upstream implementation.
5. Update only the affected handwritten fragment, token mapping, and accepted
   fingerprint. Do not edit emitted C.
6. Run the semantic and size gates:

   ```sh
   make translator-idiomatic-c89-differential
   make translator-idiomatic-c89-test-engine
   make translator-idiomatic-c89-size
   make translator-size-opportunities
   ```

7. Regenerate and verify the checked-in distribution:

   ```sh
   make dist-idiomatic-c89
   git diff --exit-code -- dist/imgui-c89
   ```

8. Commit the upstream lock update, reconciled overlay, regenerated
   distribution, and refreshed measurements together.

## Review aids

A unified diff between the literal and idiomatic generated trees is useful for
code review and release notes:

```sh
git diff --no-index \
  build/translator/latest_release/generated-a \
  build/translator/idiomatic_c89/generated-a
```

That diff is deliberately disposable. Line movement, declaration ordering,
or translator spelling cleanup can make it noisy without changing semantics.
The fingerprint-checked overlay remains the reapplication mechanism.

## Change placement

- Put representation-neutral C++-to-C cleanup in the translator so literal
  and idiomatic output both benefit.
- Put qualified-function replacements and deliberately selected size
  boundaries in `idiomatic_c89.json` and `translator/handwritten/`.
- Do not move behavior into the C++ facade; it is a compatibility and testing
  surface over the same C implementation.
- Keep public ABI changes separate from body rewrites and document them in
  `docs/C89_ABI_MIGRATION.md`.
