#!/usr/bin/env python3
"""Bundle entries from a mod registry into release data."""

import argparse
import json
import shutil
import sys
from contextlib import nullcontext
from pathlib import Path, PurePosixPath
from tempfile import TemporaryDirectory
from urllib.parse import urlsplit
from urllib.request import Request, urlopen
from zipfile import ZipFile

HEADERS = {"User-Agent": "Cataclysm-release-bundler"}
MARKERS = {"mod": ("modinfo.json",), "soundpack": ("soundpack.txt", "soundpack.json")}


def fetch(url):
    return urlopen(Request(url, headers=HEADERS), timeout=60)


def load_json(path):
    if urlsplit(path).scheme in ("http", "https"):
        with fetch(path) as response:
            return json.load(response)
    with Path(path).open(encoding="utf-8") as file:
        return json.load(file)


def validate_entries(entries, required, optional, *, lock):
    if not isinstance(entries, list):
        raise TypeError("Expected an array of registry entries")
    for entry in entries:
        if not isinstance(entry, dict) or not all(
            isinstance(entry.get(key), str) for key in required
        ):
            raise ValueError(f"Invalid registry entry: {entry!r}")
        if not all(isinstance(entry.get(key, ""), str) for key in optional):
            raise ValueError(f"Invalid optional fields: {entry!r}")
        if lock:
            url = urlsplit(entry["url"])
            if not (
                (url.scheme in {"http", "https"} and url.netloc)
                or (url.scheme == "data" and url.path.startswith("application/zip;"))
            ):
                raise ValueError(f"Invalid archive URL for {entry['id']}")
        else:
            source = entry.get("source")
            if not isinstance(source, dict) or not isinstance(
                source.get("extract_path", ""), str
            ):
                raise ValueError(f"Invalid source for {entry['id']}")
            if entry.get("package_type", "mod") not in MARKERS:
                raise ValueError(f"Invalid package type for {entry['id']}")


def select_entries(locks, registry, excluded=()):
    validate_entries(locks, ("id", "version", "url"), ("extract_path",), lock=True)
    validate_entries(
        registry, ("id", "version", "display_name"), ("homepage", "license"), lock=False
    )
    by_id = {entry["id"]: entry for entry in registry}
    for lock in locks:
        package_id = lock["id"]
        if package_id not in by_id:
            raise ValueError(f"{package_id} is not present in the mod registry")
        entry = by_id[package_id]
        if entry["version"] != lock["version"]:
            print(
                f"{package_id} version mismatch: lockfile has {lock['version']}, "
                f"registry has {entry['version']}; using locked version",
                file=sys.stderr,
            )
        package_type = entry.get("package_type", "mod")
        if package_type not in excluded:
            yield entry, lock, package_type


def extract_parts(path):
    parts = PurePosixPath(path.replace("\\", "/")).parts
    if ".." in parts or (parts and (parts[0] == "/" or ":" in parts[0])):
        raise ValueError(f"Invalid relative path: {path}")
    return parts


def extract_dir(root, parts):
    if parts:
        for directory in sorted(path for path in root.rglob("*") if path.is_dir()):
            if directory.relative_to(root).parts[-len(parts) :] == parts:
                return directory
        raise ValueError(f"Archive did not contain {'/'.join(parts)}")
    directories = [path for path in root.iterdir() if path.is_dir()]
    if len(directories) != 1:
        raise ValueError("Archive must have exactly one root directory")
    return directories[0]


def install(output_root, cache_dir, entry, lock, package_type):
    package_id = entry["id"]
    if extract_parts(package_id) != (package_id,):
        raise ValueError(f"Invalid package ID: {package_id}")
    parts = extract_parts(
        lock.get("extract_path", entry["source"].get("extract_path", ""))
    )
    target = output_root / "data" / ("sound" if package_type == "soundpack" else "mods")
    target /= parts[-1] if parts else package_id
    archive = cache_dir / f"{package_id}.zip"
    with fetch(lock["url"]) as response, archive.open("wb") as file:
        shutil.copyfileobj(response, file)
    with TemporaryDirectory(prefix="cataclysm-registry-unzip-") as temp:
        root = Path(temp)
        with ZipFile(archive) as source:
            for member in source.infolist():
                extract_parts(member.filename)
            source.extractall(root)
        directory = extract_dir(root, parts)
        markers = MARKERS[package_type]
        if not any((directory / marker).exists() for marker in markers):
            raise ValueError(f"{package_id} did not extract {' or '.join(markers)}")
        if target.exists():
            shutil.rmtree(target)
        shutil.copytree(directory, target)
    print(f"bundled {package_id} -> {target}")


def bundle(entries, output_root, cache_dir=None):
    cache = (
        TemporaryDirectory(prefix="cataclysm-registry-mods-")
        if cache_dir is None
        else nullcontext(cache_dir)
    )
    credits = ["Bundled registry mods", "=====================", ""]
    with cache as directory:
        directory = Path(directory)
        directory.mkdir(parents=True, exist_ok=True)
        for entry, lock, package_type in entries:
            print(f"{entry['id']}: {entry['display_name']} ({package_type})")
            install(output_root, directory, entry, lock, package_type)
            credits.extend(
                [
                    f"{entry['display_name']} ({entry['id']})",
                    f"Source: {lock['url']}",
                    f"Homepage: {entry.get('homepage', 'unknown')}",
                    f"License: {entry.get('license', 'unknown')}",
                    "",
                ]
            )
    path = output_root / "data/mods/external-mods-CREDITS.txt"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(credits), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", default="data/mods/external.json")
    parser.add_argument("--registry-url", required=True)
    parser.add_argument("--output-root", type=Path, default=Path("."))
    parser.add_argument("--cache-dir", type=Path)
    parser.add_argument("--exclude-package-type", action="append", default=[])
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()
    entries = list(
        select_entries(
            load_json(args.manifest),
            load_json(args.registry_url),
            args.exclude_package_type,
        )
    )
    if args.dry_run:
        for entry, _, package_type in entries:
            print(f"{entry['id']}: {entry['display_name']} ({package_type})")
    else:
        bundle(entries, args.output_root, args.cache_dir)


if __name__ == "__main__":
    main()
