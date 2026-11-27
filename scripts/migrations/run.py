"""Shared command-line handling for data migrations."""

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from cata_json import transform_files


def migrate(
    description, transform, *, positional=False, multiple=True, excluded_types=None
):
    parser = argparse.ArgumentParser(description=description)
    if positional:
        parser.add_argument("paths", nargs="+" if multiple else 1)
    else:
        parser.add_argument("-p", "--paths", nargs="+", required=True)
        parser.add_argument("-q", "--quiet", choices=("true", "false"), default="false")
        parser.add_argument("--format")
    args = parser.parse_args()
    options = (
        {"excluded_types": excluded_types, "skip_mixed_files": True}
        if excluded_types
        else {}
    )
    transform_files(
        args.paths, transform, formatter=getattr(args, "format", None), **options
    )
