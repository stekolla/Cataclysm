#!/usr/bin/env python3
"""List changed source files and direct users of changed headers for clang-tidy."""

import argparse
import os
import posixpath
import re
import subprocess
from collections import defaultdict
from pathlib import Path, PurePosixPath

SOURCE_DIRS = ("src", "tests")
INCLUDE = re.compile(r'#include\s+"([^"]+)"')


def changed_files(base, head):
    output = subprocess.check_output(
        [
            "git",
            "diff",
            "--name-status",
            "-z",
            "--find-renames",
            "--find-copies",
            f"{base}...{head}",
            "--",
        ]
    )
    fields = iter(os.fsdecode(output).split("\0")[:-1])
    for status in fields:
        path = next(fields)
        if status.startswith(("R", "C")):
            path = next(fields)
        if not status.startswith("D"):
            yield PurePosixPath(path)


def header_users():
    sources = {
        path.as_posix()
        for directory in SOURCE_DIRS
        for path in Path(directory).rglob("*")
        if path.suffix in {".cpp", ".h"}
        and ".tmp" not in path.as_posix()
        and path.is_file()
    }
    users = defaultdict(set)
    for source in sources:
        for include in INCLUDE.findall(
            Path(source).read_text(encoding="utf-8", errors="replace")
        ):
            local = posixpath.normpath(
                posixpath.join(posixpath.dirname(source), include)
            )
            root = posixpath.normpath(posixpath.join("src", include))
            if local in sources:
                users[local].add(source)
            elif root in sources:
                users[root].add(source)
    return users


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pr", type=int, nargs="?", help=argparse.SUPPRESS)
    parser.add_argument("--base", default="origin/main")
    parser.add_argument("--head", default="HEAD")
    parser.add_argument("-o", "--output", type=Path)
    args = parser.parse_args()

    sources = []
    headers = []
    for path in changed_files(args.base, args.head):
        if len(path.parts) < 2 or path.parts[0] not in SOURCE_DIRS:
            continue
        if path.suffix == ".h":
            headers.append(path.as_posix())
        elif path.suffix in {".cpp", ".hpp"}:
            sources.append(path.as_posix())

    users = header_users() if headers else {}
    affected = dict.fromkeys(sources)
    for header in headers:
        affected.update(dict.fromkeys(sorted(users.get(header, ()))))
    print({"src": sources, "headers": headers})
    print(set(affected))
    if args.output:
        args.output.write_text("\n".join(affected), encoding="utf-8")


if __name__ == "__main__":
    main()
