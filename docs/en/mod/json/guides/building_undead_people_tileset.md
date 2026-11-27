# Building UDP tileset

UndeadPeople-style unpacked tilesets are composed with the Python tileset tool.

Links:

- Unpacked tileset repository: <https://github.com/Theawesomeboophis/UndeadPeopleUnpacked>
- Project Discord: <https://discord.gg/ftgMS5Rcsd>

## Setup

1. Install Python and run `python3 -m pip install -r scripts/requirements.txt` in a virtual environment.
2. Get an unpacked tileset repository.
3. From a Cataclysm working copy, run the pack command with the unpacked tileset
   directory as input.

```sh
python3 scripts/tileset.py --pack path/to/UndeadPeopleUnpacked path/to/UndeadPeoplePacked
```

The first path is the unpacked tileset root. It should contain files such as `tileset.txt`,
`tile_info.json`, and `pngs_*` directories. The second path is where the composed `tile_config.json`
and tilesheet PNG files will be written.

For more detail, see [Tilesets](../reference/graphics/tileset.md#python-tileset-tool).
Use the [Python tileset tool](../reference/graphics/tileset.md#python-tileset-tool) to compose and decompose tilesets.
