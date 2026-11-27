# Formatting & Linting

This guide explains how to format and lint code in Cataclysm.

## Quick Reference

| Scope           | Tool                | Command                      |
| --------------- | ------------------- | ---------------------------- |
| Staged files    | all formatters      | `build-scripts/fmt.sh`       |
| All files       | all formatters      | `build-scripts/fmt.sh --all` |
| C++ (`.cpp/.h`) | astyle/clang-format | `build-scripts/fmt.sh cpp`   |
| JSON            | json_formatter      | `build-scripts/fmt.sh json`  |
| Markdown        | dprint              | `build-scripts/fmt.sh docs`  |
| Lua             | dprint              | `build-scripts/fmt.sh lua`   |

## Automated Formatting

Pull requests are automatically formatted by [autofix.ci](https://autofix.ci). If your code has
style violations, a commit will be pushed to fix them.

> [!TIP]
> To avoid merge conflicts after autofix commits, either:
>
> 1. Run `git pull` to merge the autofix commit, then continue working
> 2. Format locally before pushing, then `git push --force` if needed

## C++ Formatting

Top-level `src/*.cpp` and `src/*.h` files use [astyle](http://astyle.sourceforge.net/). Most other C++ files use [clang-format](https://clang.llvm.org/docs/ClangFormat.html). Formatter fixtures are left unchanged.

```sh
# Install formatters (Ubuntu/Debian)
sudo apt install astyle clang-format

# Install formatters (Fedora)
sudo dnf install astyle clang-tools-extra

# Install formatters (macOS)
brew install astyle clang-format
```

### Using the helper

```sh
build-scripts/fmt.sh cpp
# or, for all staged file types
build-scripts/fmt.sh
```

If you already have a CMake build tree with `bash` available, `cmake --build <build-dir> --target format` calls the same C++ helper. The style configurations are in `.astylerc` and `.clang-format` at the repository root.

## JSON Formatting

JSON files are formatted with `json_formatter`, a custom tool built from the project source.

### Using the script

```sh
build-scripts/fmt.sh json
# or
build-scripts/format-json.sh
```

The script builds `json_formatter` in `out/build/json-format` and formats JSON files. It does not
require a configured game build or a CMake preset. Override the helper build directory with
`CATA_JSON_FORMAT_BUILD_DIR` if needed.

> [!NOTE]
> The `data/names/` directory is excluded from formatting because name files have special formatting
> requirements.

### JSON Syntax Validation

Before formatting, you can validate JSON syntax:

```sh
build-scripts/lint-json.sh
```

This runs Python's `json.tool` on all JSON files to catch syntax errors.

## Markdown and Lua Formatting

Install the standalone [dprint CLI](https://dprint.dev/) at the version used by CI.

```sh
npm install --global dprint@0.47.5
build-scripts/fmt.sh docs
build-scripts/fmt.sh lua
```

## Dialogue Validation

NPC dialogue files have additional validation:

```sh
tools/dialogue_validator.py data/json/npcs/* data/json/npcs/*/* data/json/npcs/*/*/*
```

## Pre-commit Workflow

Install the optional pre-commit hook with [`prek`](https://github.com/j178/prek):

```sh
prek install
```

When you commit, the hook runs `build-scripts/fmt.sh`. dprint formats Markdown and Lua across the
repository. The C++ and JSON formatters process staged additions and modifications. The hook
re-stages formatted files so the commit includes the changes.

To run the same formatting manually against staged files:

```sh
build-scripts/fmt.sh
```

Before committing without the hook, run these checks:

```sh
# Format staged files
build-scripts/fmt.sh

# Format all formattable files
build-scripts/fmt.sh --all
```

## CI Integration

The CI pipeline runs these checks automatically:

1. **JSON syntax validation** - `build-scripts/lint-json.sh`
2. **JSON formatting** - `build-scripts/format-json.sh`
3. **Dialogue validation** - `tools/dialogue_validator.py`

If any check fails, the build will fail. Use the commands above to fix issues locally before
pushing.

## Editor Integration

### VS Code

Run `build-scripts/fmt.sh cpp` for C++. The dprint extension uses the repository configuration for Lua.

### Vim/Neovim

Use the helper instead of calling astyle or clang-format directly:

```vim
autocmd BufWritePre *.cpp,*.h !build-scripts/fmt.sh cpp %
autocmd BufWritePre *.md !dprint fmt --config dprint-docs.json %
```

## Troubleshooting

### "json_formatter not found"

Run the JSON formatter script. It configures and builds the formatter helper automatically:

```sh
build-scripts/format-json.sh
```

### C++ formatter not found

Make sure `astyle` and `clang-format` are installed and in your PATH:

```sh
# Check if formatters are available
which astyle
which clang-format

# Install if missing (Ubuntu/Debian)
sudo apt install astyle clang-format
```

Then rerun:

```sh
build-scripts/format-cpp.sh
```

### C++ formatters produce different results

Use the repository helper from the repository root so each file goes through the right formatter:

```sh
build-scripts/fmt.sh cpp
```
