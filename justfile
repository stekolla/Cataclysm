set shell := ["bash", "-eu", "-o", "pipefail", "-c"]

default:
    @just --list

# Build and run in the pinned, host-architecture Linux container.
docker-image:
    docker build --tag cataclysm-bn-build-env:emsdk-6.0.8 --file docker/Dockerfile docker/

docker-shell:
    CATA_DOCKER_INTERACTIVE=1 bash build-scripts/run-docker.sh bash

docker-test-fast:
    bash build-scripts/run-docker.sh bash -lc 'set -euo pipefail; cmake --preset linux-tests; cmake --build --preset linux-tests --parallel 2; ctest --test-dir out/build/linux-tests --output-on-failure'

docker-test-wasm:
    bash build-scripts/run-docker.sh bash -lc 'set -euo pipefail; cmake --preset wasm-tests; cmake --build --preset wasm-tests --parallel 2; ctest --test-dir out/build/wasm-tests --output-on-failure'

docker-build-wasm:
    bash build-scripts/run-docker.sh bash -lc 'set -euo pipefail; cmake --preset wasm-tiles; cmake --build --preset wasm-tiles --parallel 2'

docker-serve-wasm:
    CATA_DOCKER_PUBLISH=1 bash build-scripts/run-docker.sh python3 -m http.server 8000 --bind 0.0.0.0 --directory out/build/wasm-tiles/web

fmt *ARGS:
    build-scripts/fmt.sh {{ARGS}}

fmt-cpp *FILES:
    build-scripts/fmt.sh cpp {{FILES}}

fmt-json *FILES:
    build-scripts/fmt.sh json {{FILES}}

fmt-return *PATHS:
    build-scripts/format-trailing-return.sh {{PATHS}}

# Use `auto` for declarations whose type the initializer already names.
fmt-auto *PATHS:
    TIDY_CHECK=modernize-use-auto build-scripts/format-trailing-return.sh {{PATHS}}

fmt-docs:
    build-scripts/fmt.sh docs

fmt-lua:
    build-scripts/fmt.sh lua

lint: lint-json lint-dialogue

lint-json:
    build-scripts/lint-json.sh

lint-dialogue:
    tools/dialogue_validator.py data/json/npcs/* data/json/npcs/*/* data/json/npcs/*/*/*

hooks-setup:
    prek install

# Download loading screen images into data/.
loading-screens:
    build-scripts/fetch-loading-screens.sh

check: lint
