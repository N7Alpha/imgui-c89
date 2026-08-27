#!/usr/bin/env python3
"""Measure the cost of restoring selectable compact-profile modules.

This is an attribution tool, not profile policy.  It starts from the already
extracted compact IR, re-enables named groups of calls/lowerings, translates
the variant, and links the normal reachable embedded-size fixture.
"""

from __future__ import annotations

import argparse
import copy
import json
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "py"))

from imgui_translator.emit import translate

from embedded_size import profile_size


MODULES = {
    "navigation": {
        "restore_calls": [
            "ImGui::NavEndFrame",
            "ImGui::NavUpdateCancelRequest",
            "ImGui::NavUpdateContextMenuRequest",
            "ImGui::NavUpdateCreateMoveRequest",
            "ImGui::NavUpdateCreateTabbingRequest",
            "ImGui::NavUpdateWindowing",
        ],
    },
    "settings": {
        "restore_calls": [
            "ImGui::AddSettingsHandler",
            "ImGui::UpdateSettings",
        ],
    },
    "context_hooks": {
        "restore_calls": ["ImGui::CallContextHooks"],
    },
    "error_recovery": {
        "restore_calls": [
            "ImGui::BeginErrorTooltip",
            "ImGui::EndErrorTooltip",
            "ImGui::ErrorCheckEndFrameFinalizeErrorTooltip",
            "ImGui::ErrorRecoveryStoreState",
            "ImGui::ErrorRecoveryTryToRecoverState",
            "ImGui::ErrorRecoveryTryToRecoverWindowState",
        ],
    },
    "cff": {
        "options": {"compact_truetype_only": False},
    },
}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("modules", nargs="+", choices=sorted(MODULES))
    parser.add_argument("--cc", default="cc")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    source_ir = root / "build/translator/embedded_compact/program.ir.json"
    if not source_ir.exists():
        raise SystemExit("compact IR missing; build embedded_compact first")
    program = json.loads(source_ir.read_text(encoding="utf-8"))
    options = copy.deepcopy(program.get("translator_options", {}))
    omitted = set(options.get("omit_calls", []))
    for module_name in args.modules:
        module = MODULES[module_name]
        omitted.difference_update(module.get("restore_calls", []))
        options.update(module.get("options", {}))
    options["omit_calls"] = sorted(omitted)
    program["translator_options"] = options

    suffix = "-".join(args.modules)
    probe_name = "embedded-compact-restore-" + suffix
    generated = root / "build/translator" / probe_name / "generated-a"
    with tempfile.TemporaryDirectory(prefix="imgui-c89-compact-module-") as temporary:
        ir_path = Path(temporary) / "program.ir.json"
        ir_path.write_text(json.dumps(program), encoding="utf-8")
        translate(ir_path, generated)
    result = profile_size(
        root,
        "embedded_compact",
        args.cc,
        generated=generated,
        output_name=probe_name,
    )
    baseline = profile_size(root, "embedded_compact", args.cc)
    print(json.dumps({
        "modules": args.modules,
        "code_delta": result["code_bytes"] - baseline["code_bytes"],
        "section_delta": result["section_bytes"] - baseline["section_bytes"],
        "result": result,
    }, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
