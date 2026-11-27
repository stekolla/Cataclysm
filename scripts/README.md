# Scripts

Run repository scripts with Python 3.11 or later. The tileset pack and unpack command needs Pillow.
Install it in a virtual environment.

```sh
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r scripts/requirements.txt
python3 -m unittest discover -s scripts
```

Run any script with `--help` for its command-line options.

- `python3 scripts/tileset.py --pack SOURCE [OUTPUT]` composes a tileset atlas.
- `python3 scripts/tileset.py --unpack TILESET` extracts sprites and tile definitions.
- `python3 scripts/tileset_missing_items.py` reports missing sprites.
- `python3 scripts/affected_files.py -o affected_files.txt` lists files for clang-tidy.
- `python3 scripts/semantic.py` regenerates the semantic PR title configuration.
- `python3 scripts/migrations/3312.migrate_legacy_unit.py --paths data/json data/mods`
  converts legacy unit fields in place.

The Wasm test image installs Chromium and the Python WebSocket client for `./test wasm`.
