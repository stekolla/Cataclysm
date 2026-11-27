# Build and Run the Browser Game

Cataclysm is built for desktop browsers with Emscripten and WebAssembly. Native game
builds are not supported. The repository's Docker environment provides the pinned Emscripten SDK,
Linux test toolchain, Python, and headless Chromium.

## Requirements

- Docker Desktop or Docker Engine, running
- A desktop browser with WebAssembly JavaScript Promise Integration (JSPI)

Run commands from the repository root. Docker uses the host architecture automatically: Apple
Silicon uses `linux/arm64`, and x86-64 systems use `linux/amd64`. Do not force a platform or use
emulation.

The launcher builds the Docker image automatically if it is missing.

## Build and Run

From the repository root, build and serve the game:

```sh
./play
```

Open <http://127.0.0.1:8000/cataclysm-bn-tiles.html> when the build finishes. Leave the command
running while you play; press Ctrl+C to stop it. The first build can take a while, but later runs
reuse it.

On the host, the build output is in `out/docker-build/wasm-tiles/web`. Inside the container, it is
available at `out/build/wasm-tiles/web`.

Docker builds use `out/docker-build` on the host, separate from host builds in `out/build`. Existing
builds in `out/build` are not modified; the first build in Docker's new directory starts from scratch.

The browser build uses the Undead People tileset, matching the upstream desktop default, and CPU
compute. It fills the browser viewport and resizes with it. If the viewport is smaller than the
game's minimum UI size, scroll to reach the rest of the game. Existing settings that name an
unavailable tileset fall back to Undead People. It runs in English with sound disabled.
Use **Save and Quit** (`S`) before reloading. Saves stay in that browser profile for the same site
address and port. Use one Cataclysm tab per site to keep saves consistent.

## Build and Run Tests

Run the fast tests or both suites through Docker:

```sh
./test
./test all
```

`./test` builds and runs the Linux curses test-only configuration. It does not build a
native game. It runs the Catch2 selection excluding hidden, `[slow]`, and `[wasm]` tests in one
process, so the suite can still take a while.

`./test wasm` builds the browser game and Wasm test runner, then runs the tests in headless
Chromium. Mark tests that need video or GPU initialization with the `[wasm]` Catch2 tag. The browser
runner uses a fixed random seed and CPU compute.
Both test runners load the fixtures in `tests/data/TEST_DATA` into a temporary test user directory.
The browser game archive does not include those fixtures.
The Wasm test runner also embeds the ASCII and isometric tilesets used by the tile-context tests.
The browser suite also checks the game canvas, viewport resizing, default tileset, and saved settings.

## Standalone JSON Formatter

The JSON formatter is repository tooling separate from the game build. Run it with:

```sh
build-scripts/fmt.sh json
```

This tooling-only configuration cannot build the game.
