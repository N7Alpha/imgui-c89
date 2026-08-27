#!/usr/bin/env python3
"""Translate versioned extractor IR into deterministic C89 and C++ facade."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "py"))

from imgui_translator.emit import TranslationError, translate


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("ir", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    try:
        translate(args.ir, args.output)
    except TranslationError as error:
        print(f"translation failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
