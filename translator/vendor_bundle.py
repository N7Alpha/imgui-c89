#!/usr/bin/env python3
"""Assemble a copyable, dependency-free generated C89 distribution."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path


DEFAULT_PROFILE_NAME = "embedded_compact_vendor"
GENERATED_HEADERS = (
    "imgui_c89.h", "imgui_c89_internal.h", "imgui_translated.h",
    "imgui_c89_api.h",
)
GENERATED_SOURCES = (
    "imgui.c",
    "imgui_draw.c",
    "imgui_tables.c",
    "imgui_widgets.c",
    "imgui_c89_api.c",
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--output",
        type=Path,
        default=root / "build/vendor/imgui-c89",
    )
    parser.add_argument("--profile", default=DEFAULT_PROFILE_NAME)
    parser.add_argument(
        "--template",
        choices=("vendor", "idiomatic_dist"),
        default="vendor",
    )
    args = parser.parse_args()
    output = args.output.resolve()
    generated = root / "build/translator" / args.profile / "generated-a"
    template = root / "translator" / args.template
    required = [
        *(generated / name for name in GENERATED_HEADERS),
        *(generated / name for name in GENERATED_SOURCES),
        generated / "manifest.json",
    ]
    missing = [str(path) for path in required if not path.is_file()]
    if missing:
        raise SystemExit(
            "vendor profile has not been generated: " + ", ".join(missing)
        )

    header = (generated / "imgui_c89.h").read_text(encoding="utf-8")
    if "SDL3/" in header or "SDL_" in header:
        raise RuntimeError("core vendor header unexpectedly depends on SDL3")

    profile = json.loads(
        (root / "translator/profiles" / f"{args.profile}.json").read_text()
    )
    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    revision = profile.get("upstream_revision", args.profile)
    commit = lock[revision]["commit"]
    upstream = root / "build/upstream" / commit

    staging = output.parent / f".{output.name}.tmp"
    if staging.exists():
        shutil.rmtree(staging)
    (staging / "include").mkdir(parents=True)
    (staging / "src").mkdir()
    (staging / "examples").mkdir()
    (staging / "fonts").mkdir()

    for name in GENERATED_HEADERS:
        shutil.copy2(generated / name, staging / "include" / name)
    for name in GENERATED_SOURCES:
        shutil.copy2(generated / name, staging / "src" / name)
    shutil.copy2(template / "Makefile", staging / "Makefile")
    shutil.copy2(template / "README.md", staging / "README.md")
    shutil.copy2(template / "smoke.c", staging / "examples/smoke.c")
    shutil.copy2(upstream / "LICENSE.txt", staging / "LICENSE.txt")
    shutil.copy2(
        upstream / "misc/fonts/ProggyClean.ttf",
        staging / "fonts/ProggyClean.ttf",
    )

    source_manifest = json.loads((generated / "manifest.json").read_text())
    # Compiler branding is host-specific even when emitted sources are
    # byte-identical. Keep the checked-in provenance reproducible across the
    # supported LLVM 22 hosts.
    source_manifest.pop("clang_version", None)
    provenance = {
        "profile": args.profile,
        "upstream_commit": commit,
        "translator_manifest": source_manifest,
    }
    (staging / "PROVENANCE.json").write_text(
        json.dumps(provenance, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )

    checksum_paths = sorted(
        path for path in staging.rglob("*")
        if path.is_file() and path.name != "SHA256SUMS"
    )
    checksum_text = "".join(
        f"{sha256(path)}  {path.relative_to(staging).as_posix()}\n"
        for path in checksum_paths
    )
    (staging / "SHA256SUMS").write_text(checksum_text, encoding="utf-8")

    if output.exists():
        shutil.rmtree(output)
    staging.replace(output)
    print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
