#!/usr/bin/env python3
"""Fetch an exact locked Dear ImGui revision into the disposable build tree."""

from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path


def run(*arguments: str, cwd: Path | None = None) -> str:
    result = subprocess.run(
        arguments,
        cwd=cwd,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
    )
    return result.stdout.strip()


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser()
    parser.add_argument("--revision", default="baseline")
    parser.add_argument("--destination", type=Path)
    args = parser.parse_args()

    lock = json.loads((root / "translator/upstream.lock.json").read_text())
    if args.revision in lock and isinstance(lock[args.revision], dict):
        selected = lock[args.revision]
    else:
        choices = {
            item["family"]: item for item in lock["compatibility_matrix"]
        }
        choices.update({item["tag"]: item for item in lock["compatibility_matrix"]})
        if args.revision not in choices:
            parser.error(f"unknown locked revision {args.revision!r}")
        selected = choices[args.revision]

    commit = selected["commit"]
    default_parent = (
        "build/upstream-test-engine"
        if args.revision == "test_engine" else "build/upstream"
    )
    destination = args.destination or root / default_parent / commit
    destination = destination.resolve()
    if not destination.exists():
        destination.mkdir(parents=True)
        run("git", "init", "--quiet", cwd=destination)
        repository = selected.get("repository", lock["repository"])
        run("git", "remote", "add", "origin", repository, cwd=destination)
    run("git", "fetch", "--quiet", "--depth", "1", "origin", commit, cwd=destination)
    run("git", "checkout", "--quiet", "--detach", "FETCH_HEAD", cwd=destination)
    actual = run("git", "rev-parse", "HEAD", cwd=destination)
    if actual != commit:
        raise RuntimeError(f"expected {commit}, fetched {actual}")
    print(destination)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
