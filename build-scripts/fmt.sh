#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

usage() {
    cat >&2 <<'USAGE'
usage: build-scripts/fmt.sh [--all|cpp [--all|files...]|json [--all|files...]|docs|lua]
USAGE
}

mode=staged
case "${1:-}" in
    "")
        ;;
    --all)
        mode=all
        shift
        ;;
    cpp|json|docs|lua)
        mode="$1"
        shift
        ;;
    *)
        usage
        exit 2
        ;;
esac

if (( $# > 0 )) && [[ "$mode" != cpp && "$mode" != json ]]; then
    usage
    exit 2
fi

if [[ "$mode" == cpp ]]; then
    if (( $# == 1 )) && [[ "${1:-}" == "--all" ]]; then
        shift
    fi
    build-scripts/format-cpp.sh "$@"
    exit 0
fi

if [[ "$mode" == json ]]; then
    if (( $# == 1 )) && [[ "${1:-}" == "--all" ]]; then
        shift
    fi
    build-scripts/format-json.sh "$@"
    exit 0
fi

if [[ "$mode" == docs ]]; then
    dprint fmt --config dprint-docs.json
    exit 0
fi

if [[ "$mode" == lua ]]; then
    dprint fmt
    exit 0
fi

cpp_files=()
json_files=()
format_docs=false
format_lua=false
stage_files=()

if [[ "$mode" == staged ]]; then
    while IFS= read -r -d '' file; do
        file="${file#./}"
        case "$file" in
            *.cpp|*.h|*.hpp)
                cpp_files+=( "$file" )
                stage_files+=( "$file" )
                ;;
            *.json)
                if [[ "$file" != data/names/* && "$file" != .vscode/* ]]; then
                    json_files+=( "$file" )
                    stage_files+=( "$file" )
                fi
                ;;
            *.md)
                format_docs=true
                stage_files+=( "$file" )
                ;;
            *.lua)
                format_lua=true
                stage_files+=( "$file" )
                ;;
        esac
    done < <(git diff --cached --name-only --diff-filter=ACMR -z)
fi

if [[ "$mode" == all ]]; then
    dprint fmt --config dprint-docs.json
    dprint fmt
    build-scripts/format-cpp.sh
    build-scripts/format-json.sh
else
    for file in "${stage_files[@]+"${stage_files[@]}"}"; do
        if ! git --literal-pathspecs diff --quiet -- "$file"; then
            printf 'error: cannot format staged file with unstaged changes: %s\n' "$file" >&2
            exit 1
        fi
    done
    if "$format_docs"; then
        dprint fmt --config dprint-docs.json --staged --allow-no-files
    fi
    if "$format_lua"; then
        dprint fmt --staged --allow-no-files
    fi
    if [[ ${cpp_files[0]+set} ]]; then
        build-scripts/format-cpp.sh "${cpp_files[@]}"
    fi
    if [[ ${json_files[0]+set} ]]; then
        build-scripts/format-json.sh "${json_files[@]}"
    fi
    if [[ ${stage_files[0]+set} ]]; then
        git --literal-pathspecs add -- "${stage_files[@]}"
    fi
fi
