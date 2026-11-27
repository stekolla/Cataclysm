#!/usr/bin/env bash
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"
build_path="${BUILD_PATH:-build}"
jobs="${NUM_JOBS:-$(getconf _NPROCESSORS_ONLN)}"
affected_files="${AFFECTED_FILES:-affected_files.txt}"
plugin="$build_path/scripts/clang-tidy-plugin/libCataAnalyzerPlugin.so"

if [[ ! -r "$affected_files" ]]; then
    printf 'error: cannot read affected-files list: %s\n' "$affected_files" >&2
    exit 1
fi

files=()
while IFS= read -r file || [[ -n "$file" ]]; do
    if [[ "$file" =~ ^(src|tests)/.*\.(cpp|h|hpp)$ ]]; then
        files+=( "$file" )
    fi
done < "$affected_files"

if [[ ! ${files[0]+set} ]]; then
    echo "No files to analyze."
    exit 0
fi

if [[ ! -f "$plugin" ]]; then
    printf 'error: clang-tidy plugin not found: %s\n' "$plugin" >&2
    exit 1
fi

printf '%s\0' "${files[@]}" |
    xargs -0 -n1 -P "$jobs" clang-tidy -p "$build_path" --load="$plugin" \
        --enable-check-profile --store-check-profile=clang-tidy-trace -quiet
