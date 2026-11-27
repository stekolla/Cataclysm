#!/usr/bin/env python3
"""Group entries read from a JSON directory into one output file per type."""

import argparse
from collections import defaultdict
from pathlib import Path

from cata_json import json_paths, read_entries, write_json


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    groups = defaultdict(list)
    for path in json_paths([args.input]):
        for entry in read_entries(path):
            if isinstance(entry, dict) and isinstance(entry.get("type"), str):
                groups[entry["type"]].append(entry)
    args.output.mkdir(parents=True, exist_ok=True)
    for name, entries in groups.items():
        filename = f"{name.lower()}.json"
        if Path(filename).name != filename or "\\" in filename:
            raise ValueError(f"Invalid type: {name}")
        write_json(args.output / filename, entries)


if __name__ == "__main__":
    main()
