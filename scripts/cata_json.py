"""Read and write JSON files used by Cataclysm data scripts."""

import json
import re
import subprocess
from pathlib import Path

SPECIAL_FILES = re.compile(r"(modinfo|default|replacements|mod_tileset)\.json")
SPECIAL_TYPES = {
    "speech",
    "sound_effect",
    "mapgen",
    "palette",
    "faction",
    "talk_topic",
    "mod_tileset",
    "MOD_INFO",
}


def json_paths(paths, *, legacy=False):
    for path in map(Path, paths):
        if path.is_file():
            if legacy and (
                path.suffix != ".json"
                or path.name in {"default.json", "replacements.json"}
            ):
                raise ValueError(f"Invalid JSON input: {path}")
            yield path
        elif path.is_dir():
            for entry in sorted(path.rglob("*.json")):
                skipped = (
                    entry.name in {"default.json", "replacements.json"}
                    if legacy
                    else SPECIAL_FILES.search(entry.as_posix())
                )
                if entry.is_file() and not skipped:
                    yield entry
        else:
            raise FileNotFoundError(path)


def read_json(path):
    return json.loads(path.read_text(encoding="utf-8"))


def read_entries(path):
    value = read_json(path)
    return value if isinstance(value, list) else [value]


def restore_key_order(original, updated):
    if not isinstance(original, dict) or not isinstance(updated, dict):
        return updated
    ordered = {
        key: restore_key_order(value, updated[key])
        for key, value in original.items()
        if key in updated
    }
    ordered.update(
        (key, value) for key, value in updated.items() if key not in original
    )
    return ordered


def write_json(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding="utf-8")


def transform_files(
    paths,
    transform,
    *,
    excluded_types=SPECIAL_TYPES,
    formatter=None,
    skip_mixed_files=False,
):
    for path in json_paths(paths, legacy=skip_mixed_files):
        entries = read_json(path)
        if not isinstance(entries, list):
            if not skip_mixed_files:
                continue
            entries = [entries]
        ignored_file = skip_mixed_files and any(
            isinstance(entry, dict)
            and isinstance(entry.get("type"), str)
            and entry["type"] in excluded_types
            for entry in entries
        )
        if not ignored_file:
            updated = []
            for entry in entries:
                if isinstance(entry, dict) and (
                    not isinstance(entry.get("type"), str)
                    or entry["type"] not in excluded_types
                ):
                    migrated = transform(entry)
                    entry = (
                        migrated
                        if skip_mixed_files
                        else restore_key_order(entry, migrated)
                    )
                updated.append(entry)
            if updated != entries:
                write_json(path, updated)
        if formatter:
            subprocess.run([formatter, str(path)], check=True)
