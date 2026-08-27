#!/usr/bin/env python3
"""Measure and gate the all-modules compact target."""

from __future__ import annotations

import json
import os
from pathlib import Path

from embedded_size import profile_size


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    result = profile_size(
        root, "embedded_compact_full", os.environ.get("CC", "cc")
    )
    output = root / "build/embedded-size/full-parity-report.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(result, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    total = int(result["section_bytes"])
    print(json.dumps(result, indent=2, sort_keys=True))
    if total >= 100 * 1024:
        raise SystemExit(
            f"full-parity compact target exceeds 100 KiB: {total} bytes "
            f"({total - 100 * 1024:+} bytes)"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
