#!/usr/bin/env python3
"""Report construct coverage from extracted IR."""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path
from typing import Any


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("ir", type=Path)
    parser.add_argument("--limit", type=int, default=40)
    args = parser.parse_args()
    data = json.loads(args.ir.read_text())
    kinds: Counter[str] = Counter()
    unsupported: Counter[str] = Counter()

    def visit(value: Any) -> None:
        if isinstance(value, dict):
            kind = value.get("kind")
            if kind:
                kinds[kind] += 1
                if value.get("unsupported"):
                    unsupported[kind] += 1
            for child in value.values():
                visit(child)
        elif isinstance(value, list):
            for child in value:
                visit(child)

    visit(data)
    report = {
        "schema_version": data["schema_version"],
        "clang_version": data["clang_version"],
        "records": len(data["records"]),
        "enums": len(data["enums"]),
        "functions": len(data["functions"]),
        "unsupported_nodes": sum(unsupported.values()),
        "unsupported_kinds": dict(unsupported.most_common(args.limit)),
        "most_common_nodes": dict(kinds.most_common(args.limit)),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
