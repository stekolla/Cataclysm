#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

mkdir -p out/build out/docker-build
host_dir="$(mktemp -d "$repo_root/out/build/docker-isolation.XXXXXX")"
docker_dir="$repo_root/out/docker-build/$(basename "$host_dir")"
docker_created=0
cleanup() {
    rm -f -- "$host_dir/result"
    rmdir -- "$host_dir"
    if (( docker_created )); then
        rm -f -- "$docker_dir/result"
        rmdir -- "$docker_dir"
    fi
}
trap cleanup EXIT

printf 'host\n' > "$host_dir/result"
bash build-scripts/run-docker.sh bash -c '
    set -euo pipefail
    mkdir "$1"
    trap '\''rm -f -- "$1/result"; rmdir -- "$1"'\'' ERR
    printf "container\n" > "$1/result"
' _ "out/build/$(basename "$host_dir")"
docker_created=1

test "$(cat "$host_dir/result")" = "host"
test "$(cat "$docker_dir/result")" = "container"
echo "Docker and host build directories are isolated"
