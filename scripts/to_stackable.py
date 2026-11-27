#!/usr/bin/env python3
"""Make selected item IDs stackable."""

import argparse

from cata_json import transform_files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-p", "--paths", nargs="+", required=True)
    parser.add_argument("-q", "--quiet", choices=("true", "false"), default="false")
    parser.add_argument("--format")
    parser.add_argument("ids", nargs="+")
    args = parser.parse_args()
    ids = set(args.ids)

    def stackable(entry):
        item_id = entry.get("id")
        if (
            not isinstance(item_id, str)
            or item_id not in ids
            or not isinstance(entry.get("type"), str)
        ):
            return entry
        item_type = "GENERIC" if entry["type"] == "TOOL" else entry["type"]
        print(item_id)
        return {**entry, "type": item_type, "stackable": True}

    transform_files(
        args.paths,
        stackable,
        excluded_types={"mapgen", "palette", "mod_tileset"},
        formatter=args.format,
        skip_mixed_files=True,
    )


if __name__ == "__main__":
    main()
