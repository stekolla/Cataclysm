#!/usr/bin/env python3
"""Generate Monster Girls mutation overrides from vanilla categories."""

import argparse
import sys
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))

from cata_json import json_paths, read_entries, write_json

DATA = SCRIPTS.parent / "data"
MOD_MUTATIONS = DATA / "mods/Monster_Girls/vanilla_mutations.json"
THRESHOLDS = {
    "FELINE": "NEKO",
    "BEAST": "KITSUNE",
    "LUPINE": "DOGGIRL",
    "PLANT": "DRYAD",
    "BIRD": "HARPY",
    "URSINE": "BEARGIRL",
    "SPIDER": "SPIDERGIRL",
    "SLIME": "SLIMEGIRL",
    "MOUSE": "MOUSEGIRL",
    "CATTLE": "COWGIRL",
    "RABBIT": "BUNNYGIRL",
    "ELFA": "ELF",
    "SERPENT": "LAMIA",
}


def mutations(paths):
    for path in json_paths(paths):
        for entry in read_entries(path):
            if (
                isinstance(entry, dict)
                and entry.get("type") == "mutation"
                and isinstance(entry.get("id"), str)
            ):
                yield entry


def is_category(value):
    return isinstance(value, list) and all(isinstance(item, str) for item in value)


def generate(inputs, extension):
    mapping = {}
    for entry in mutations([extension]):
        extend = entry.get("extend")
        if isinstance(extend, dict) and is_category(extend.get("category")):
            mapping[entry["id"]] = list(dict.fromkeys(extend["category"]))
    result = []
    for entry in mutations(inputs):
        category = entry.get("category")
        thresholds = entry.get("threshreq", [])
        if not is_category(category) or not is_category(thresholds):
            continue
        ident = entry["id"]
        output = {
            "type": "mutation",
            "id": ident,
            "copy-from": ident,
            "delete": {"category": category},
        }
        if ident in mapping:
            extend = {"category": mapping[ident]}
            if "threshreq" in entry or ident == "FELINE_FLEXIBILITY":
                if ident == "FELINE_FLEXIBILITY":
                    converted = ["KITSUNE"]
                else:
                    converted = [
                        THRESHOLDS[value[7:]]
                        for value in thresholds
                        if value.startswith("THRESH_") and value[7:] in THRESHOLDS
                    ]
                extend["threshreq"] = [
                    f"THRESH_{value}" for value in converted if value in mapping[ident]
                ]
            output["extend"] = extend
        else:
            output["valid"] = False
        result.append(output)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input",
        nargs="+",
        type=Path,
        default=[DATA / "json/mutations/mutations.json"],
    )
    parser.add_argument("--output", type=Path, default=MOD_MUTATIONS)
    parser.add_argument("--extend", type=Path, default=MOD_MUTATIONS)
    args = parser.parse_args()
    write_json(args.output, generate(args.input, args.extend))


if __name__ == "__main__":
    main()
