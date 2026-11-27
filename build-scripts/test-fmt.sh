#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
scratch="$(mktemp -d)"
trap 'find "$scratch" -depth -delete' EXIT
cd "$scratch"
git init -q
cp "$repo_root/dprint-docs.json" .

printf '#   Staged\n' > staged.md
printf '#   Unstaged\n' > unstaged.md
git add -- staged.md
bash "$repo_root/build-scripts/fmt.sh"
test "$(git show :staged.md)" = '# Staged'
test "$(cat unstaged.md)" = '#   Unstaged'

printf '#   Index\n' > staged.md
git add -- staged.md
printf '#   Worktree\n' > staged.md
if bash "$repo_root/build-scripts/fmt.sh"; then
    echo "Expected formatting to reject a partially staged file" >&2
    exit 1
fi
test "$(git show :staged.md)" = '#   Index'
test "$(cat staged.md)" = '#   Worktree'
