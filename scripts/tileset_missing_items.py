#!/usr/bin/env python3
"""Report items and monsters missing direct sprites in one or more tilesets."""

import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path
from unicodedata import combining, normalize

from cata_json import json_paths, read_entries, read_json

ROOT = Path(__file__).resolve().parent.parent
TILESETS = ["gfx/MSX++UnDeadPeopleEdition", "data/json/external_tileset", "data/mods"]
TYPES = {
    "AMMO",
    "ARMOR",
    "BATTERY",
    "BIONIC_ITEM",
    "BOOK",
    "COMESTIBLE",
    "CONTAINER",
    "ENGINE",
    "GENERIC",
    "GUN",
    "GUNMOD",
    "MAGAZINE",
    "MONSTER",
    "PET_ARMOR",
    "TOOL",
    "TOOLMOD",
    "TOOL_ARMOR",
    "WHEEL",
}
VARIANT_PREFIX = re.compile(r"^overlay_(?:wielded|worn)_")
VARIANT_SUFFIX = re.compile(
    r"_(?:season_(?:spring|summer|autumn|winter)|harvested|on|off|open|closed|full|empty)$"
)


@dataclass(frozen=True)
class Definition:
    id: str
    path: str
    abstract: bool
    parent: str | None
    looks_like: str | None
    fake: bool


def display_path(path):
    return str(path.relative_to(ROOT) if path.is_relative_to(ROOT) else path)


def tileset_paths(path):
    if path.is_file():
        return [path]
    if not path.is_dir():
        raise FileNotFoundError(path)
    properties = path / "tileset.txt"
    config = path / "tile_config.json"
    if properties.is_file():
        for line in properties.read_text(encoding="utf-8").splitlines():
            name, _, value = line.partition(":")
            if name.strip() == "JSON" and value.strip():
                config = path / value.strip()
        return [config]
    if config.is_file():
        return [config]
    return sorted(entry for entry in path.rglob("*.json") if entry.is_file())


def strings(value):
    if isinstance(value, str):
        return [value]
    return (
        [item for item in value if isinstance(item, str)]
        if isinstance(value, list)
        else []
    )


def entry_tile_ids(entry):
    if isinstance(entry, dict):
        additional = entry.get("additional_tiles", [])
        ids = entry.get("id", [])
        if isinstance(ids, (str, list)) and isinstance(additional, list):
            yield from strings(ids)
            for child in additional:
                yield from entry_tile_ids(child)


def tile_ids(config):
    if isinstance(config, list):
        for child in config:
            yield from tile_ids(child)
    elif isinstance(config, dict):
        sheets, entries = config.get("tiles-new", []), config.get("tiles", [])
        if isinstance(sheets, list) and isinstance(entries, list):
            yield from tile_ids(sheets)
            for entry in entries:
                yield from entry_tile_ids(entry)


def variants(ids):
    aliases = {}
    for tile in sorted(ids):
        pending = [tile]
        visited = {tile}
        while pending:
            current = pending.pop()
            for pattern in (VARIANT_PREFIX, VARIANT_SUFFIX):
                base = pattern.sub("", current)
                if base and base not in visited:
                    visited.add(base)
                    aliases.setdefault(base, tile)
                    pending.append(base)
    return aliases


def ignored_ids(value):
    if isinstance(value, list):
        for item in value:
            yield from ignored_ids(item)
    elif isinstance(value, dict):
        fields = ["fake_item", "type", "gun_type", "ammo_type"]
        if all(isinstance(value.get(field, ""), str) for field in fields):
            if "fake_item" in value:
                yield value["fake_item"]
            if value.get("type") == "gun":
                for field in ("gun_type", "ammo_type"):
                    if field in value:
                        yield value[field]
        for item in value.values():
            yield from ignored_ids(item)


def load_items(inputs):
    items, abstracts, ignored = {}, {}, set()
    for path in dict.fromkeys(json_paths(inputs)):
        entries = read_entries(path)
        ignored.update(ignored_ids(entries))
        for entry in entries:
            if not isinstance(entry, dict) or not isinstance(entry.get("type"), str):
                continue
            if entry["type"] not in TYPES:
                continue
            if not all(
                isinstance(entry.get(key, ""), str)
                for key in ("id", "abstract", "copy-from", "looks_like")
            ):
                continue
            ident = entry.get("id", entry.get("abstract"))
            flags = entry.get("flags", [])
            if not ident or not isinstance(flags, (str, list)):
                continue
            abstract = not entry.get("id")
            target = abstracts if abstract else items
            target[ident] = Definition(
                ident,
                display_path(path),
                abstract,
                entry.get("copy-from"),
                entry.get("looks_like"),
                ident == "fake_item" or "PSEUDO" in strings(flags),
            )
    return items, abstracts, ignored


def resolve_items(items, abstracts):
    cache = {}
    visiting = set()

    def resolve(item):
        key = item.abstract, item.id
        if key in cache:
            return cache[key]
        if key in visiting:
            return item.looks_like, item.fake

        visiting.add(key)
        fallback, fake = item.looks_like, item.fake
        parent = items.get(item.parent) or abstracts.get(item.parent)
        if not fallback and parent:
            fallback = parent.id
            if parent.abstract or not fake:
                inherited, parent_fake = resolve(parent)
                if parent.abstract and inherited is not None:
                    fallback = inherited
                fake = fake or parent_fake
        visiting.remove(key)
        cache[key] = fallback, fake
        return cache[key]

    return {item.id: resolve(item) for item in items.values()}


def lookup(start, resolved, direct, alias):
    chain = []
    current = start
    while current and current not in chain and len(chain) < 10:
        chain.append(current)
        if current in direct:
            return current, chain
        if current in alias:
            return alias[current], chain
        current = resolved.get(current, (None, False))[0]
    return None, chain


SECTIONS = {
    "variant_only": ("Missing id (but variant tile exists)", ("id", "tile_id", "path")),
    "missing": ("Missing id", ("id", "path")),
    "looks_like_only": ("Missing id (but looks_like tile exists)", ("chain", "path")),
}


def item_id_order(item_id):
    """Sort report IDs by base letters and put underscores before digits."""
    normalized = normalize("NFD", item_id.casefold().replace("_", "\0"))
    primary = "".join(character for character in normalized if not combining(character))
    return primary, normalized, item_id


def report(tilesets, inputs):
    paths = dict.fromkeys(
        path for source in tilesets for path in tileset_paths(ROOT / source)
    )
    direct = {tile for path in paths for tile in tile_ids(read_json(path))}
    alias = variants(direct)
    items, abstracts, ignored = load_items(ROOT / source for source in inputs)
    resolved = resolve_items(items, abstracts)
    rows = {key: [] for key in SECTIONS}
    direct_count = 0
    for item_id in sorted(items, key=item_id_order):
        if item_id in ignored or resolved[item_id][1]:
            continue
        path = items[item_id].path
        if item_id in direct:
            direct_count += 1
        elif item_id in alias:
            rows["variant_only"].append(
                {"id": item_id, "tile_id": alias[item_id], "path": path}
            )
        else:
            tile, chain = lookup(item_id, resolved, direct, alias)
            if tile:
                rows["looks_like_only"].append(
                    {"id": item_id, "tile_id": tile, "chain": chain, "path": path}
                )
            else:
                rows["missing"].append({"id": item_id, "path": path})
    counts = {
        "checked_ids": direct_count + sum(map(len, rows.values())),
        "direct_sprites": direct_count,
        "variant_sprites_only": len(rows["variant_only"]),
        "looks_like_fallback_only": len(rows["looks_like_only"]),
        "missing_entirely": len(rows["missing"]),
    }
    return counts, rows


def render(tilesets, inputs, counts, rows, output_format, limit, show_fallback):
    sections = {}
    for key, entries in rows.items():
        shown = entries[:limit] if limit > 0 else entries
        if key == "looks_like_only" and not show_fallback:
            shown = []
        sections[key] = {
            "total": len(entries),
            "shown": len(shown),
            "hidden": len(entries) - len(shown),
            "entries": shown,
        }
    if output_format == "json":
        return json.dumps(
            dict(tilesets=tilesets, inputs=inputs, counts=counts, **sections),
            indent=2,
            ensure_ascii=False,
        )

    markdown = output_format == "md"
    lines = ["# Tileset Missing Sprites", ""] if markdown else []
    for label, paths in (("Tilesets", tilesets), ("Inputs", inputs)):
        formatted = ", ".join(f"`{path}`" if markdown else path for path in paths)
        lines.append(f"{'- ' if markdown else ''}{label}: {formatted}")
    labels = (
        "Checked ids",
        "Direct sprites",
        "Variant sprites only",
        "Looks_like fallback only",
        "Missing entirely",
    )
    for (key, count), label in zip(counts.items(), labels):
        if key != "looks_like_fallback_only" or show_fallback:
            lines.append(f"{'- ' if markdown else ''}{label}: {count}")
    for key, (title, fields) in SECTIONS.items():
        if key == "looks_like_only" and not show_fallback:
            continue
        section = sections[key]
        lines.extend(["", f"{'## ' if markdown else ''}{title} ({section['total']})"])
        if markdown:
            lines.append("")
        if not section["total"]:
            lines.append("(none)" if markdown else "  (none)")
            continue
        if markdown:
            lines.append("```text")
        for entry in section["entries"]:
            lines.append(
                "\t".join(
                    " -> ".join(entry[k]) if k == "chain" else entry[k] for k in fields
                )
            )
        if section["hidden"]:
            lines.append(f"... {section['hidden']} more")
        if markdown:
            lines.append("```")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tileset", action="append")
    parser.add_argument("--input", action="append")
    parser.add_argument("--format", choices=("text", "json", "md"), default="text")
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--looks-like", action="store_true")
    args = parser.parse_args()
    tilesets, inputs = args.tileset or TILESETS, args.input or ["data/json"]
    try:
        counts, rows = report(tilesets, inputs)
        print(
            render(
                [display_path(Path(p)) for p in tilesets],
                [display_path(Path(p)) for p in inputs],
                counts,
                rows,
                args.format,
                args.limit,
                args.looks_like,
            )
        )
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
