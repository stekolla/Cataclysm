"""Regression fixtures for the Python tooling."""

import importlib.util
import io
import json
import subprocess
import sys
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import patch
from zipfile import ZipFile

from cata_json import read_json, write_json
from PIL import Image
from tileset_missing_items import render, report

SCRIPTS = Path(__file__).resolve().parent


def load_script(path):
    spec = importlib.util.spec_from_file_location(path.stem, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class MigratedToolsTests(unittest.TestCase):
    def test_aside_conversion(self):
        """Check the existing Starlight caution fixture against its expected Markdown."""
        directory = SCRIPTS / "aside_to_gfm"
        converter = load_script(directory / "main.py")
        self.assertEqual(
            converter.aside_to_gfm((directory / "given.md").read_text()),
            (directory / "expected.md").read_text(),
        )

    def test_affected_files(self):
        """Follow quoted includes from a changed header and use the destination of a rename."""
        with TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "src"
            source.mkdir()
            header = source / "common header.h"
            header.write_text("#pragma once\n")
            (source / "reader.cpp").write_text('#include "common header.h"\n')
            (source / "old.cpp").write_text("int value = 42;\n")

            def git(*args):
                subprocess.run(["git", *args], cwd=root, check=True)

            git("init", "--quiet")
            git("config", "user.email", "tool-tests@example.invalid")
            git("config", "user.name", "Tool Tests")
            git("config", "commit.gpgsign", "false")
            git("config", "core.hooksPath", str(root / "no-hooks"))
            git("add", ".")
            git("commit", "--quiet", "-m", "fixture")
            header.write_text("#pragma once\n#define FIXTURE 1\n")
            git("mv", "src/old.cpp", "src/renamed file.cpp")
            git("add", ".")
            git("commit", "--quiet", "-m", "changes")
            output = root / "affected"
            subprocess.run(
                [
                    sys.executable,
                    SCRIPTS / "affected_files.py",
                    "--base",
                    "HEAD^",
                    "--output",
                    output,
                ],
                cwd=root,
                check=True,
                stdout=subprocess.DEVNULL,
            )
            self.assertEqual(
                output.read_text().splitlines(),
                ["src/renamed file.cpp", "src/reader.cpp"],
            )

    def test_bundled_mod(self):
        """Check that bundling uses the locked archive, extracts the mod and writes credits."""
        bundler = load_script(SCRIPTS.parent / "build-scripts/bundle-registry-mods.py")
        archive = io.BytesIO()
        with ZipFile(archive, "w") as source:
            source.writestr("ReleaseRoot/NestedMod/modinfo.json", "[]")
        url = "https://example.com/locked.zip"
        locks = [
            {"id": "demo", "version": "1", "url": url},
            {"id": "sound", "version": "1", "url": "https://example.com/sound.zip"},
        ]
        registry = [
            {
                "id": "demo",
                "version": "2",
                "display_name": "Demo",
                "source": {"extract_path": "NestedMod"},
            },
            {
                "id": "sound",
                "version": "1",
                "display_name": "Sound",
                "source": {},
                "package_type": "soundpack",
            },
        ]
        with (
            TemporaryDirectory() as temp,
            patch.object(bundler, "fetch", return_value=archive) as fetch,
        ):
            root = Path(temp)
            archive.seek(0)
            entries = list(bundler.select_entries(locks, registry, {"soundpack"}))
            bundler.bundle(entries, root)
            fetch.assert_called_once_with(url)
            self.assertEqual(
                (root / "data/mods/NestedMod/modinfo.json").read_text(), "[]"
            )
            self.assertIn(
                "Demo (demo)",
                (root / "data/mods/external-mods-CREDITS.txt").read_text(),
            )
            self.assertFalse((root / "data/sound").exists())

    def test_missing_sprite_report(self):
        """Distinguish direct, variant, inherited, and missing sprites in each report format."""
        with TemporaryDirectory() as temp:
            root = Path(temp)
            tiles = root / "tile_config.json"
            items = root / "items.json"
            write_json(
                tiles,
                {
                    "tiles-new": [
                        {
                            "tiles": [
                                {
                                    "id": "direct",
                                    "additional_tiles": [
                                        {"id": "overlay_wielded_variant_on"}
                                    ],
                                }
                            ]
                        }
                    ]
                },
            )
            write_json(
                items,
                [
                    {"type": "TOOL", "id": "direct"},
                    {"type": "TOOL", "id": "variant"},
                    {"type": "TOOL", "id": "fallback", "copy-from": "template"},
                    {"type": "TOOL", "abstract": "template", "looks_like": "variant"},
                    {"type": "TOOL", "id": "absent"},
                    {"type": "TOOL", "id": "fake", "copy-from": "fake_item"},
                    {"type": "TOOL", "abstract": "fake_item"},
                ],
            )
            counts, rows = report([str(tiles)], [str(items)])
            self.assertEqual(list(counts.values()), [4, 1, 1, 1, 1])
            self.assertEqual(
                rows["looks_like_only"][0]["chain"], ["fallback", "variant"]
            )
            for output_format in ("md", "text"):
                self.assertIn(
                    "fallback -> variant",
                    render([], [], counts, rows, output_format, 0, True),
                )
            hidden = json.loads(render([], [], counts, rows, "json", 0, False))
            self.assertEqual(
                hidden["looks_like_only"],
                {"total": 1, "shown": 0, "hidden": 1, "entries": []},
            )

    def test_legacy_units(self):
        """Convert relative vehicle units and nested sounds without changing unrelated values."""
        with TemporaryDirectory() as temp:
            data = Path(temp) / "vehicle.json"
            write_json(
                data,
                [
                    {
                        "type": "vehicle_part",
                        "storage": 100,
                        "relative": {
                            "weight": 500,
                            "volume": -2,
                            "barrel_volume": -2,
                            "price": 5000,
                        },
                        "workbench": {"mass": 1000, "volume": 1000},
                        "bash": {"sound_vol": 2, "sound_fail_vol": "80 dB"},
                    }
                ],
            )
            command = [
                sys.executable,
                SCRIPTS / "migrations/3312.migrate_legacy_unit.py",
                "--paths",
                data,
            ]
            subprocess.run(command, check=True)
            actual = read_json(data)
            self.assertEqual(
                actual,
                [
                    {
                        "type": "vehicle_part",
                        "storage": "25 L",
                        "relative": {
                            "weight": "500 g",
                            "volume": "-500 ml",
                            "barrel_volume": "-500 ml",
                            "price": "50 USD",
                        },
                        "workbench": {"mass": "1 kg", "volume": "250 L"},
                        "bash": {"sound_vol": "26 dB", "sound_fail_vol": "80 dB"},
                    }
                ],
            )
            before = data.read_bytes()
            subprocess.run(command, check=True)
            self.assertEqual(data.read_bytes(), before)

    def test_legacy_item_migrations(self):
        """Apply the legacy reach, battery, and harvest changes to isolated data files."""
        with TemporaryDirectory() as temp:
            data = Path(temp) / "items.json"
            write_json(
                data,
                [
                    {"type": "GENERIC", "id": "spear", "flags": ["SPEAR"]},
                    {
                        "type": "GENERIC",
                        "id": "pike",
                        "flags": ["SPEAR", "REACH_ATTACK"],
                    },
                    {
                        "type": "TOOL_ARMOR",
                        "ammo": "battery",
                        "max_charges": 500,
                        "use_action": {"type": "transform", "need_charges": 1},
                    },
                    {"type": "GUN", "ups_charges": 10},
                    {
                        "type": "harvest",
                        "entries": [{"drop": "spider", "flags": ["FILTHY"]}],
                    },
                    {
                        "type": "TOOL",
                        "flags": ["FILTHY", "NO_REPAIR"],
                        "squeamish_penalty": 1,
                        "use_action": {
                            "type": "unpack",
                            "filthy_volume_threshold": "10 L",
                        },
                    },
                ],
            )
            for name, options in (
                ("4214.spear_stab", ["--paths"]),
                ("5259.migrate_energy", []),
                ("6363.remove_filthy", []),
            ):
                subprocess.run(
                    [sys.executable, SCRIPTS / f"migrations/{name}.py", *options, data],
                    check=True,
                )
            spear, pike, battery, gun, harvest, tool = read_json(data)
            self.assertEqual(spear["flags"], ["STAB"])
            self.assertEqual(pike["flags"], ["SPEAR", "REACH_ATTACK", "STAB"])
            self.assertEqual(
                battery,
                {
                    "type": "TOOL_ARMOR",
                    "max_power": "500 kJ",
                    "use_action": {"type": "transform", "need_power": "1 kJ"},
                },
            )
            self.assertEqual(gun["power_draw"], "10 kJ")
            self.assertEqual(harvest["entries"][0]["flags"], [])
            self.assertEqual(
                tool,
                {
                    "type": "TOOL",
                    "flags": ["NO_REPAIR"],
                    "use_action": {"type": "unpack"},
                },
            )

    def test_tileset_pixels(self):
        """Round-trip transparent and opaque sprites through the Python pack and unpack CLI."""
        with TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "source"
            packed = root / "packed"
            images = source / "pngs_main_2x2/images0"
            images.mkdir(parents=True)
            (source / "tileset.txt").write_text("JSON: tile_config.json\n")
            write_json(
                source / "tile_info.json",
                [
                    {"width": 2, "height": 2},
                    {"main.png": {"sprites_across": 2}},
                    {"fallback.png": {"fallback": True}},
                ],
            )
            colors = {"alpha": (220, 100, 11, 127), "beta": (10, 20, 30, 255)}
            for name, color in colors.items():
                Image.new("RGBA", (2, 2), color).save(images / f"{name}.png")
            write_json(
                images / "entries.json",
                [
                    {"id": "alpha", "fg": "alpha"},
                    {"id": "beta", "fg": "beta"},
                ],
            )
            command = [sys.executable, SCRIPTS / "tileset.py"]
            subprocess.run(
                [*command, "--pack", "--only-json", source, packed], check=True
            )
            self.assertFalse((packed / "main.png").exists())
            subprocess.run([*command, "--pack", source, packed], check=True)
            sheet = read_json(packed / "tile_config.json")["tiles-new"][0]
            sprites = {tile["id"]: tile["fg"] for tile in sheet["tiles"]}
            with Image.open(packed / "main.png") as atlas:
                for name, color in colors.items():
                    row, column = divmod(sprites[name], 2)
                    self.assertEqual(atlas.getpixel((column * 2, row * 2)), color)
            subprocess.run(
                [*command, "--pack", "--palette-copies", source, packed], check=True
            )
            with Image.open(packed / "main.png8") as palette:
                self.assertEqual(palette.mode, "P")
                for name, color in colors.items():
                    row, column = divmod(sprites[name], 2)
                    self.assertEqual(
                        palette.convert("RGBA").getpixel((column * 2, row * 2)), color
                    )
            (packed / "tileset.txt").write_text("JSON: tile_config.json\n")
            Image.new("RGBA", (2, 2)).save(packed / "fallback.png")
            subprocess.run([*command, "--unpack", packed], check=True)
            for name, color in colors.items():
                (sprite,) = packed.glob(f"pngs_main_*/*/*_{name}_*.png")
                with Image.open(sprite) as image:
                    self.assertEqual(image.tobytes(), bytes(color) * 4)


if __name__ == "__main__":
    unittest.main()
