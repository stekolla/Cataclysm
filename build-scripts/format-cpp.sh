#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

clang_format_sources=()
astyle_sources=()

append_source() {
    local file="${1#"$repo_root/"}"
    file="${file#./}"
    if [[ ! -f "$file" ]]; then
        return
    fi

    case "$file" in
        src/lua/*|src/sol/*|src/third-party/*|tests/catch/*|scripts/clang-tidy-plugin/test/*)
            return
            ;;
    esac

    if [[ "$file" =~ ^src/[^/]+\.(cpp|h)$ ]]; then
        astyle_sources+=( "$file" )
    elif [[ "$file" == src/*/* || "$file" == tests/* || "$file" == scripts/format/* || "$file" == scripts/clang-tidy-plugin/* ]]; then
        case "$file" in
            *.cpp|*.h|*.hpp)
                clang_format_sources+=( "$file" )
                ;;
        esac
    fi
}

if (( $# > 0 )); then
    for file in "$@"; do
        append_source "$file"
    done
else
    while IFS= read -r -d '' file; do
        append_source "$file"
    done < <(
        find src tests scripts/format scripts/clang-tidy-plugin \
            \( -path 'src/lua' -o -path 'src/sol' -o -path 'src/third-party' \
               -o -path 'tests/catch' -o -path 'scripts/clang-tidy-plugin/test' \) -prune -o \
            -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0
    )
fi

if (( ${#clang_format_sources[@]} > 0 )); then
    if ! command -v clang-format >/dev/null 2>&1; then
        echo "error: clang-format executable was not found" >&2
        exit 1
    fi
    clang-format -i "${clang_format_sources[@]}"
elif (( $# == 0 )); then
    echo "warning: no clang-format C++ files available" >&2
fi

if (( ${#astyle_sources[@]} > 0 )); then
    if ! command -v astyle >/dev/null 2>&1; then
        echo "error: Artistic Style executable was not found" >&2
        exit 1
    fi
    astyle --options=.astylerc -n "${astyle_sources[@]}"
elif (( $# == 0 )); then
    echo "warning: no astyle C++ files available" >&2
fi

if (( ${#clang_format_sources[@]} == 0 && ${#astyle_sources[@]} == 0 && $# > 0 )); then
    echo "warning: no supported C++ files were provided" >&2
fi
