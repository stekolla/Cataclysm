#!/usr/bin/env python3
"""Pack or unpack Cataclysm tileset atlases using Pillow."""

import argparse
import subprocess
import sys
from pathlib import Path

GRAPHICS_TOOLS = Path(__file__).resolve().parent.parent / "scripts/gfx_tools"
PACK_OPTIONS = {
    "--use-all": "Add unused images using their basenames as IDs",
    "--obsolete-fillers": "Warn about obsolete fillers",
    "--palette-copies": "Also produce sheets quantized to 8-bit palettes",
    "--palette": "Quantize sheets to 8-bit palettes",
    "--format-json": "Format tile_config.json",
    "--only-json": "Write only tile_config.json",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--version", action="version", version="%(prog)s 0.1.0")
    modes = parser.add_mutually_exclusive_group(required=True)
    modes.add_argument("--pack", action="store_true", help="Compose an atlas")
    modes.add_argument(
        "--unpack", action="store_true", help="Extract sprites and tile definitions"
    )
    for option, help_text in PACK_OPTIONS.items():
        parser.add_argument(option, action="store_true", help=help_text)
    parser.add_argument(
        "--delete-file", help="File listing sprite index ranges to delete"
    )
    parser.add_argument("source_dir")
    parser.add_argument("output_dir", nargs="?")
    args = parser.parse_args()

    if args.pack:
        options = [
            option
            for option in PACK_OPTIONS
            if getattr(args, option[2:].replace("-", "_"))
        ]
        command = [GRAPHICS_TOOLS / "compose.py", *options, args.source_dir]
        if args.output_dir:
            command.append(args.output_dir)
    else:
        command = [GRAPHICS_TOOLS / "decompose.py", args.source_dir]
        if args.delete_file:
            command.extend(("--delete-file", args.delete_file))
    return subprocess.run([sys.executable, *command], check=False).returncode


if __name__ == "__main__":
    sys.exit(main())
