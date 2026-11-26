# Build and Run the Browser Game

Cataclysm: Bright Nights is built for desktop browsers with Emscripten and WebAssembly. Native game
builds are not supported. The repository's Docker environment provides the pinned Emscripten SDK,
Linux test toolchain, Deno, and headless Chromium.

## Requirements

- Docker Desktop or Docker Engine
- [just](https://just.systems/) to run the repository's build recipes

Run commands from the repository root. Docker uses the host architecture automatically: Apple
Silicon uses `linux/arm64`, and x86-64 systems use `linux/amd64`. Do not force a platform or use
emulation.

Each `docker-*` recipe builds the Docker image automatically if it is missing. To build it ahead
of time, run `just docker-image`.

## Build and Run

Build the browser game, then serve it locally:

```sh
just docker-build-wasm
just docker-serve-wasm
```

Open <http://127.0.0.1:8000/cataclysm-bn-tiles.html> in a desktop browser with WebAssembly JavaScript
Promise Integration (JSPI). The build output is in `out/build/wasm-tiles/web`.

The browser build uses the RetroDays tileset and CPU compute. It runs in English with sound disabled.
Use **Save and Quit** (`S`) before reloading. Saves stay in that browser profile for the same site
address and port. Use one Bright Nights tab per site to keep saves consistent.

To configure and build without the `just` recipes, run these commands inside the Docker environment:

```sh
bash build-scripts/run-docker.sh bash -lc 'cmake --preset wasm-tiles && cmake --build --preset wasm-tiles --parallel 2'
CATA_DOCKER_PUBLISH=1 bash build-scripts/run-docker.sh python3 -m http.server 8000 --bind 0.0.0.0 --directory out/build/wasm-tiles/web
```

The `docker-serve-wasm` recipe publishes port 8000 from the container to the host.

## Build and Run Tests

Run both test routes through Docker:

```sh
just docker-test-fast
just docker-test-wasm
```

`docker-test-fast` builds and runs the Linux curses test-only configuration. It does not build a
native game. It runs the Catch2 selection excluding `[slow]` and `[wasm]` tests in one process, so
the suite can still take a while.

`docker-test-wasm` builds the browser game and Wasm test runner, then runs the tests in headless
Chromium. Mark tests that need video or GPU initialization with the `[wasm]` Catch2 tag. The browser
runner uses a fixed random seed and CPU compute.

For direct CMake commands inside Docker:

```sh
bash build-scripts/run-docker.sh bash -lc 'cmake --preset linux-tests && cmake --build --preset linux-tests --parallel 2 && ctest --test-dir out/build/linux-tests --output-on-failure'
bash build-scripts/run-docker.sh bash -lc 'cmake --preset wasm-tests && cmake --build --preset wasm-tests --parallel 2 && ctest --test-dir out/build/wasm-tests --output-on-failure'
```

## Standalone JSON Formatter

The JSON formatter is repository tooling separate from the game build. Run it with:

```sh
just fmt-json
```

This tooling-only configuration cannot build the game.
