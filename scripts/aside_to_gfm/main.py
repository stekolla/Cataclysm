#!/usr/bin/env python3
"""Convert Starlight asides to GitHub Markdown alerts."""

import argparse
import re
from pathlib import Path

ASIDE = re.compile(r":::(\w+)(?:\{([^}]*)\})?\n+(.+?)\n+:::", re.DOTALL | re.ASCII)
TITLE = re.compile(r'title\s*=\s*"(.*?)"')
LINK = re.compile(r"\[[^\]]+\]:\s*.+", re.ASCII)


def aside_to_gfm(text):
    def format_alert(match):
        kind, attributes, content = match.groups()
        title = TITLE.search(attributes or "")
        lines = content.strip().split("\n")
        links = [line.strip() for line in lines if LINK.search(line)]
        body = [line for line in lines if not LINK.search(line)]
        heading = kind.upper() if kind != "danger" else "CAUTION"
        output = [f"> [!{heading}]", ">"]
        if title and title[1]:
            output.extend([f"> #### {title[1]}", ">"])
        for index, line in enumerate(body):
            output.append(f"> {line}".strip() if line or index != len(body) - 1 else "")
        return "\n".join(output + links)

    return ASIDE.sub(format_alert, text)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", type=Path)
    path = parser.parse_args().path
    if not path.exists():
        parser.error(f"Path does not exist: {path}")
    sources = [path] if path.is_file() else path.rglob("*.md")
    for source in sources:
        if source.is_file() and source.suffix == ".md":
            content = source.read_text(encoding="utf-8")
            converted = aside_to_gfm(content)
            if converted != content:
                source.write_text(converted, encoding="utf-8")


if __name__ == "__main__":
    main()
