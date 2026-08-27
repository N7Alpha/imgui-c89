#!/usr/bin/env python3
"""Extract a stable function inventory from a generated cimgui header.

The importer is deliberately textual: cimgui headers are generated C and the
inventory is for migration planning, not a C parser or ABI facade generator.
It accepts CIMGUI_API/IMGUI_API declarations, including multiline arguments,
and emits Markdown by default or machine-readable JSON.
"""

from __future__ import print_function

import argparse
import json
import re
import sys


DECLARATION = re.compile(
    r"\b(?:CIMGUI_API|CIMGUI_IMPL_API|IMGUI_API)\s+"
    r"(?P<return>[^;()]+?)\s+"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*\("
    r"(?P<arguments>.*?)\)\s*;",
    re.MULTILINE | re.DOTALL,
)


def remove_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.DOTALL)
    return re.sub(r"//[^\n]*", " ", text)


def normalize_space(text):
    return " ".join(text.split())


def inventory(text):
    result = []
    for match in DECLARATION.finditer(remove_comments(text)):
        result.append({
            "name": match.group("name"),
            "return_type": normalize_space(match.group("return")),
            "arguments": normalize_space(match.group("arguments")),
        })
    result.sort(key=lambda item: item["name"])
    return result


def markdown(items, source, comparison=None):
    lines = [
        "# cimgui API inventory",
        "",
        "Source: `{}`".format(source),
        "",
        "Functions found: {}".format(len(items)),
        "",
        "| Function | Return type | Arguments |",
        "| --- | --- | --- |",
    ]
    if comparison is not None:
        lines[3:3] = [
            "Compared native header: `{}`".format(comparison["native_source"]),
            "",
            "Shared names: {} · cimgui-only: {} · native-only: {}".format(
                len(comparison["shared"]), len(comparison["cimgui_only"]),
                len(comparison["native_only"])),
            "",
        ]
    for item in items:
        arguments = item["arguments"].replace("|", "\\|")
        lines.append("| `{}` | `{}` | `{}` |".format(
            item["name"], item["return_type"], arguments))
    lines.append("")
    if comparison is not None:
        lines.extend(["## Symbol comparison", ""])
        for title, key in (("Shared", "shared"),
                           ("cimgui-only", "cimgui_only"),
                           ("Native-only", "native_only")):
            lines.append("### {}".format(title))
            lines.append("")
            if comparison[key]:
                lines.append(", ".join("`{}`".format(name)
                                     for name in comparison[key]))
            else:
                lines.append("(none)")
            lines.append("")
    return "\n".join(lines)


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("header", help="generated cimgui header")
    parser.add_argument("-f", "--format", choices=("markdown", "json"),
                        default="markdown")
    parser.add_argument("--compare", metavar="NATIVE_HEADER",
                        help="compare symbols with a native C header")
    parser.add_argument("-o", "--output", help="output file (default: stdout)")
    args = parser.parse_args(argv)
    try:
        with open(args.header, "r") as stream:
            items = inventory(stream.read())
    except (IOError, OSError) as error:
        parser.error(str(error))
        return 2
    comparison = None
    if args.compare:
        try:
            with open(args.compare, "r") as stream:
                native_items = inventory(stream.read())
        except (IOError, OSError) as error:
            parser.error(str(error))
            return 2
        cimgui_names = sorted(item["name"] for item in items)
        native_names = sorted(item["name"] for item in native_items)
        cimgui_set = set(cimgui_names)
        native_set = set(native_names)
        comparison = {
            "native_source": args.compare,
            "shared": sorted(cimgui_set & native_set),
            "cimgui_only": sorted(cimgui_set - native_set),
            "native_only": sorted(native_set - cimgui_set),
        }
    if args.format == "json":
        document = {"source": args.header, "functions": items}
        if comparison is not None:
            document["comparison"] = comparison
        output = json.dumps(document, indent=2, sort_keys=True) + "\n"
    else:
        output = markdown(items, args.header, comparison)
    if args.output:
        with open(args.output, "w") as stream:
            stream.write(output)
    else:
        sys.stdout.write(output)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
