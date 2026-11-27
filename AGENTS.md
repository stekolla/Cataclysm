# Cataclysm - Agent Guidelines

Search for existing solutions before writing new code.

## Building and Testing

This project ships a browser game, not a native game. Run the repository's Docker wrappers from
the repository root, not CMake on the host. Start with the
[build guide](docs/en/dev/guides/building/cmake.md), [testing guide](docs/en/dev/guides/testing.md),
and [formatting guide](docs/en/dev/guides/formatting.md).

Choose the smallest relevant validation route. Run one build at a time and rebuild only when
inputs change or a failure requires it. Reuse the browser game built by the Wasm test suite for
manual checks. If the user objects to waiting, ask before starting another long build and report
what remains unverified.

## C++ Conventions (NEVER VIOLATE)

```c++
const auto foo = 3; //< **MUST** use `auto`; `const` before `auto`.

auto bar() -> int; //< **MUST** use trailing return types, also in aliases: std::function<auto( int ) -> bool>.
auto baz() -> int&; // *NOPAD*  //< **MUST** append `// *NOPAD*` to reference/pointer returns and `<=>` in astyle files (`src/*.cpp`, `src/*.h`; astyle bug). Never in clang-format files (subdirectories, `tests/`).
auto qux() -> int { return 42; } //< **MUST** use single-line functions and lambdas whenever possible.

auto qux = my_struct{ .a = 1, .b = 2 }; //< **MUST** use designated initializers.
auto two_value() -> my_data; //< **SHOULD NOT** return std::pair/std::tuple; use a struct.
auto may_have_value() -> std::optional<int>; //< **MUST** use std::optional when a value may be absent.
auto may_fail() -> std::expected<int, std::string>; //< **MUST** use std::expected when it may fail.

/// **MUST** use triple slash doc comments and snake_case names.
struct comparable {
  int x;
  int y;
  auto operator<=>( const comparable & ) const = default; // *NOPAD* //< **MUST** use `<=>` for comparisons.
}

auto values = xs
  | filter( []( const auto & v ) { return v.is_valid(); } ) //< **SHOULD** use `auto` lambda params
  | transform( &value::get ) //< prefer member/function references over lambdas
  | ranges::to<std::vector>();

namespace { // **MUST** use anonymous namespace over `static`.

struct button_options { //< **MUST** use an options struct for >3 parameters.
  point pos;
  std::string text;
  nc_color fg = c_white;
};
auto print_button( const catacurses::window &w, const button_options &opts ) -> void;
// call site: print_button( w, { .pos = p, .text = "OK" } ); //< **MUST** use designated initializers; never spell button_options{ ... } when inferable.

} // namespace
```

- Collections: **MUST** use `std::ranges`/`std::views`/`ranges::to`/`cata_algo.h` with function-local `using namespace std::views;` and `namespace ranges = std::ranges;`. No manual `it++` loops unless mutation requires it; keep a plain `for` when index access, early break, mutation, or nesting is clearer (no `iota` + `for_each` to avoid loops).
- Modern C++23. Keep unused parameter names as comments: `bool /*is_avatar*/`.
- Avoid modifying headers with >10 usages (SHOULD NOT); add a new header with pure functions.
- New files go in a subdirectory so clang-format covers them.
- After you move code into a subdirectory, run `scripts/format-trailing-return.sh <paths>` and `TIDY_CHECK=modernize-use-auto scripts/format-trailing-return.sh <paths>` on the moved files.
- Check matcher semantics before using string prefixes: `OtMatchType.PREFIX` / `is_ot_match` take the base token without trailing `_` (`"robofachq"`); the matcher itself requires the `_`.
- Bug fixes keep requested behavior and visible content; fix the cause instead of disabling the feature, unless the user asks to remove it.

## Lua

- Type every function parameter with EmmyLua/LuaLS `---@param` (`---@class`/`---@field` for table shapes), including existing and local helpers; match the file's style. No `---@return` only for enforcement when inferable.
- Fix missing binding types at the binding source.
- Test C++ Lua bindings with real bound objects; Lua-only mocks are supplementary.

## Workflow

1. Issue link: fetch it with `gh`, then `git gtr new <type>/<issue-id>/<slug>` (`feat`, `fix`, `refactor`, `chore`, `build`, `ci`).
2. Format touched files before building or testing. Do not retain unrelated formatter changes.
3. Check migrated scripts against the original CLI output and fixtures before switching their callers.
   Exercise staged formatting with the system Bash on macOS when changing the formatter wrapper.
   Add/update focused Catch2 tests in `tests/` for changed behavior. Run the relevant test route and
   `./build-scripts/lint-json.sh` when JSON changes require it; do not run unrelated suites by default.
   UI regression tests follow the production refresh path; do not prepare layout or paging manually.
   For browser canvas changes, verify the running game's CSS size, backing pixels, and terminal size
   on resize and high-DPI displays; SDL may overwrite canvas styles during startup.
   Mission tests that assign work must give the avatar a valid ID and restore it afterward.
   Count stackable currency by charges when checking payments.
   Each new test needs a short `///` comment that a developer new to that code can follow: what it checks, why, and how.
4. Commit atomically with [Conventional Commits](./docs/en/contribute/pr_title_guidelines.md); no body/footer unless critical.
5. Follow the [PR skill](.agents/skills/pr/SKILL.md) and [PR template](.github/pull_request_template.md). Prefer ASD-STE100 Simplified Technical English. Track `gh pr checks` until done; fix branch-owned failures, rerun or report transient ones with the failing job.

If a mistake is found during a task, update AGENTS.md or the skill and fix the current branch before summarizing.

## Data and Review

- Load response-bearing dialogue topic definitions before line-only definitions of the same IDs.
- Exercise conditional dialogue through response generation. A string condition can silently evaluate
  to false if its name is missing from the condition dispatcher.

- Keep named quest givers and their mission definitions in dedicated NPC files. Use shared dialogue
  topics for responses shared with generic NPCs.

- Keep unsaved moves available for manual retry after failed autosaves, and rate-limit automatic retries.

- Use `str_sp` for translatable names with identical singular and plural forms.
- Digital files keep at least 1 ml volume; zero-volume items are rejected, including scanned books.
- For standalone item groups, use `[ "item_id", weight ]` entries in `items`. When a mission
  needs multiple physical tools, verify that its goal counts the tools themselves. Item goals and
  dialogue inventory conditions can accept tool charges as quantities.
- When generated or pulled files stop being tracked, verify ignore rules (`git status --ignored` or run the generator) and that every CI and release consumer still gets them.
- References: [Code Style](./docs/en/dev/explanation/code_style.md), [Building](./docs/en/dev/guides/building/cmake.md), [Formatting](./docs/en/dev/guides/formatting.md), [Dev Index](./docs/en/dev/), [LLM Review Guide](./.github/llm_review_guide.md). Use `_( "text" )` for user-facing strings.
