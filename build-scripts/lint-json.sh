#!/usr/bin/env bash
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"
git ls-files -z -- '*.json' |
python3 -c '
import json
import os
import sys

failed = False
for raw_path in sys.stdin.buffer.read().split(b"\0"):
    if not raw_path:
        continue
    path = os.fsdecode(raw_path)
    if any(part.startswith(".") for part in path.split("/")):
        continue
    try:
        with open(path, encoding="utf-8") as file:
            json.load(file)
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        print(f"FAILED: {path}: {error}", file=sys.stderr)
        failed = True
sys.exit(1 if failed else 0)
'
