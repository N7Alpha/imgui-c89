#!/usr/bin/env python3
"""Gate the sub-100-KiB modular core and report its opt-in costs."""

from __future__ import annotations

import json
import os
from pathlib import Path

from embedded_size import profile_size


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    cc = os.environ.get("CC", "cc")
    release = profile_size(root, "embedded_compact_modular", cc)
    full = profile_size(
        root,
        "embedded_compact_modular",
        cc,
        output_name="embedded_compact_modular_full",
        extra_size_flags=("-DIMGUI_C89_ENABLE_FULL_FEATURES",),
    )
    debug = profile_size(root, "embedded_compact_modular_debug", cc)
    report = {
        "gate_bytes": 100 * 1024,
        "release": release,
        "release_margin_bytes": 100 * 1024 - int(release["section_bytes"]),
        "full_feature_opt_in": full,
        "full_feature_delta_bytes": (
            int(full["section_bytes"]) - int(release["section_bytes"])
        ),
        "assert_trap_build": debug,
        "assert_trap_delta_bytes": (
            int(debug["section_bytes"]) - int(release["section_bytes"])
        ),
    }
    output = root / "build/embedded-size/compact-modular-report.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(json.dumps(report, indent=2, sort_keys=True))
    if int(release["section_bytes"]) >= 100 * 1024:
        raise SystemExit(
            "modular compact core exceeds 100 KiB: "
            f"{release['section_bytes']} bytes"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
