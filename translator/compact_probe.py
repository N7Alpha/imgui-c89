#!/usr/bin/env python3
"""Measure dead feature paths without making them embedded-profile policy.

This is an attribution tool: it emits temporary variants of the already
extracted embedded IR with additional call sites suppressed, then feeds those
variants through the normal translator and reachable-size linker.  A saving
reported here is not automatically a valid product optimization.
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


PROBES = {
    "nav": [
        "ImGui::NavUpdate",
        "ImGui::NavEndFrame",
    ],
    "settings": [
        "ImGui::UpdateSettings",
    ],
    "debug_tools": [
        "ImGui::UpdateDebugToolFlashStyleColor",
        "ImGui::UpdateDebugToolItemPathQuery",
        "ImGui::UpdateDebugToolItemPicker",
    ],
    "hooks": [
        "ImGui::CallContextHooks",
    ],
    "error_recovery": [
        "ImGui::BeginErrorTooltip",
        "ImGui::EndErrorTooltip",
        "ImGui::ErrorCheckEndFrameFinalizeErrorTooltip",
        "ImGui::ErrorRecoveryStoreState",
    ],
}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("probes", nargs="*", choices=sorted(PROBES))
    parser.add_argument("--cc", default="cc")
    args = parser.parse_args()
    selected = args.probes or list(PROBES)
    root = Path(__file__).resolve().parents[1]
    source_ir = root / "build/translator/embedded/program.ir.json"
    if not source_ir.exists():
        raise SystemExit("embedded IR missing; run `make translator-embedded`")
    program = json.loads(source_ir.read_text(encoding="utf-8"))
    profile = json.loads(
        (root / "translator/profiles/embedded.json").read_text(
            encoding="utf-8"
        )
    )
    options = copy.deepcopy(profile.get("translator_options", {}))
    omitted = set(options.get("omit_calls", []))
    for name in selected:
        omitted.update(PROBES[name])
    options["omit_calls"] = sorted(omitted)
    program["translator_options"] = options

    probe_name = "embedded-probe-" + "-".join(selected)
    generated = root / "build/translator" / probe_name / "generated-a"
    with tempfile.TemporaryDirectory(prefix="imgui-c89-probe-") as temporary:
        ir_path = Path(temporary) / "program.ir.json"
        ir_path.write_text(json.dumps(program), encoding="utf-8")
        translate(ir_path, generated)
    result = profile_size(
        root, "embedded", args.cc,
        generated=generated,
        output_name=probe_name,
    )
    baseline_report = json.loads(
        (root / "build/embedded-size/report.json").read_text(encoding="utf-8")
    )
    embedded = next(row for row in baseline_report if row["profile"] == "embedded")
    print(json.dumps({
        "probes": selected,
        "omitted_calls": sorted(omitted),
        "instruction_delta": result["code_bytes"] - embedded["code_bytes"],
        "section_delta": result["section_bytes"] - embedded["section_bytes"],
        "result": result,
    }, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
