#!/usr/bin/env bash
set -euo pipefail

cata_repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${cata_repo_root}"

mkdir -p out/docker-build out/docker-cache
export CATA_UID="$(id -u)"
export CATA_GID="$(id -g)"

cata_image="cataclysm-bn-build-env:emsdk-6.0.8"
if ! docker image inspect "${cata_image}" >/dev/null 2>&1; then
    docker build --tag "${cata_image}" --file docker/Dockerfile docker/
fi

cata_docker_args=(
    run
    --rm
    --init
    --user "${CATA_UID}:${CATA_GID}"
    --volume "${cata_repo_root}:/workspace"
    --volume "${cata_repo_root}/out/docker-build:/workspace/out/build"
    --volume "${cata_repo_root}/out/docker-cache:/tmp/cata-cache"
    --volume "${cata_repo_root}/out/docker-cache/emscripten:/emsdk/upstream/emscripten/cache"
    --workdir /workspace
    --env HOME=/tmp
    --env DENO_DIR=/tmp/cata-cache/deno
    --env EM_CACHE=/tmp/cata-cache/emscripten
    --env CHROME_BIN=/usr/local/bin/cata-chromium
)

if [[ "${CATA_DOCKER_PUBLISH:-}" == "1" ]]; then
    cata_docker_args+=(--publish 8000:8000)
fi

if [[ "${CATA_DOCKER_INTERACTIVE:-}" == "1" ]]; then
    cata_docker_args+=(--interactive --tty)
fi

exec docker "${cata_docker_args[@]}" "${cata_image}" env \
    HOME=/tmp \
    DENO_DIR=/tmp/cata-cache/deno \
    CHROME_BIN=/usr/local/bin/cata-chromium \
    "$@"
