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

## Testing and Reusing a Browser Build

See the [testing guide](../testing.md) to choose a suite. If you already built the game with
`./test wasm` or `./test all`, serve that output without rebuilding. From the repository root,
with port 8000 free:

```sh
CATA_DOCKER_PUBLISH=1 bash build-scripts/run-docker.sh \
    python3 -m http.server 8000 --bind 0.0.0.0 --directory out/build/wasm-tests/web
```

Open the same URL as for `./play`.

## Troubleshooting

If a host CMake command reports a missing Emscripten toolchain or Ninja, use `./play` or the test
wrapper instead. The presets run inside the Docker environment, which supplies those tools. If
the wrapper cannot connect to Docker, start Docker and check that `docker info` succeeds.

When modifying the Wasm toolchain, match the `setjmp`/`longjmp` implementation of C dependencies
to the game's C++ exception mode. With native Wasm exceptions, use FreeType's
`freetype-legacysjlj` port archive and JSPI for browser waits. Asyncify's Wasm rewriting cannot
reliably combine those waits with native Wasm exceptions.

## Standalone JSON Formatter

The JSON formatter is repository tooling separate from the game build. Run it with:

```sh
build-scripts/fmt.sh json
```

This tooling-only configuration cannot build the game.
