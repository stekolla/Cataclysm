#!/usr/bin/env python3
"""Extract post-apocalyptic item prices as CSV."""

import argparse
import csv
import re
import sys
from pathlib import Path

from cata_json import json_paths, read_entries

UNITS = {"cent": 1, "USD": 100, "kUSD": 100_000}


def to_cents(value):
    amount = re.match(r"^\s*([+-]?[0-9]+)", value)
    for unit, factor in UNITS.items():
        if re.search(rf"[0-9]+ {unit}", value):
            return int(amount[1]) * factor if amount else 0
    return 0


def prices(paths):
    for path in json_paths(paths, legacy=True):
        try:
            entries = read_entries(path)
        except ValueError as error:
            print(f"{path}: {error}", file=sys.stderr)
            continue
        for entry in entries:
            if (
                isinstance(entry, dict)
                and isinstance(entry.get("id"), str)
                and isinstance(entry.get("price_postapoc"), str)
            ):
                yield entry["id"], to_cents(entry["price_postapoc"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-p", "--paths", nargs="+", required=True)
    parser.add_argument("-q", "--quiet", choices=("true", "false"), default="false")
    parser.add_argument("-o", "--output", type=Path)
    args = parser.parse_args()
    rows = list(prices(args.paths))
    if not rows:
        print("no matching entries found.")
        return
    rows.insert(0, ("id", "price_postapoc"))
    if args.output:
        with args.output.open("w", encoding="utf-8", newline="") as file:
            csv.writer(file).writerows(rows)
    else:
        csv.writer(sys.stdout).writerows(rows)


if __name__ == "__main__":
    main()
