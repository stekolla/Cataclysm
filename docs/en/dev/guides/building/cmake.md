# Build for the Browser

Cataclysm: Bright Nights builds with the Emscripten SDK for desktop browsers. Native game builds
are not supported.

## Containerized build environment

Use the repository Docker environment to build and test from Linux on macOS, Windows, or Linux. It
includes the Emscripten SDK, native test compiler and libraries, Deno, and headless Chromium. Docker
uses the host architecture automatically: Apple Silicon uses `linux/arm64`, and x86-64 systems use
`linux/amd64`. Do not add a `--platform` override.

Install Docker Desktop or Docker Engine, then build the image and run the desired task:

```sh
just docker-image
just docker-test-fast
just docker-test-wasm
just docker-build-wasm
just docker-serve-wasm
```

The fast route builds and runs non-graphical tests in the Linux curses test-only configuration. The
browser route runs `[wasm]` tests in headless Chromium. Neither route builds a native game.

## Build and run the game

The `docker-build-wasm` recipe configures and builds the browser target. To run the commands inside
the container directly:

```sh
bash build-scripts/run-docker.sh bash -lc 'cmake --preset wasm-tiles && cmake --build --preset wasm-tiles --parallel 2'
```

`docker-serve-wasm` serves the generated files on port 8000 from inside the container.

Open <http://127.0.0.1:8000/cataclysm-bn-tiles.html> in a desktop browser with WebAssembly JavaScript
Promise Integration (JSPI). Use **Save and Quit** (`S`) before reloading. Saves remain in that browser
profile for the same site address and port. Use one Bright Nights tab per site to keep saves consistent.

The browser build uses the RetroDays tileset and CPU compute. It runs in English with sound disabled.

## Build and run tests

Use the native test-only preset inside the container for fast non-graphical tests. It builds the C++
sources and Catch2 runner with curses enabled, but does not create a native game executable:

```sh
bash build-scripts/run-docker.sh bash -lc 'cmake --preset linux-tests && cmake --build --preset linux-tests --parallel 2 && ctest --test-dir out/build/linux-tests --output-on-failure'
```

The container runs this test-only configuration as Linux. It omits tiles and SDL, so tests that need
video or GPU initialization run only in the browser test route. Those tests use the `[wasm]` Catch2
tag. Configure and build the WebAssembly test runner, then run it through headless Chromium:

```sh
bash build-scripts/run-docker.sh bash -lc 'cmake --preset wasm-tests && cmake --build --preset wasm-tests --parallel 2 && ctest --test-dir out/build/wasm-tests --output-on-failure'
```

The browser runner uses a fixed random seed and the CPU compute backend. `CHROME_BIN` points to the
Chromium binary in the container.

## Standalone JSON formatter

The JSON formatter is a repository tool, separate from the game build. It can be built without
Emscripten by running:

```sh
just fmt-json
```

This tooling-only configuration cannot build the game.
