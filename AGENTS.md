# Cataclysm - Agent Guidelines

- Search for an existing solution before writing new code. Fix the cause of bugs without
  removing requested behavior or visible content unless asked.
- This project ships a browser game, not a native game. Run the repository's Docker wrappers
  from the repository root, not CMake on the host. See the [build guide](docs/en/dev/guides/building/cmake.md).
- Format touched files before building or testing. Do not retain unrelated formatter changes.
- For dialogue and mission JSON, run a test that loads game data. JSON syntax checks do not
  catch definitions accidentally nested inside a response array.
- For scripted item rewards, verify actual delivered quantities. Bare `u_buy_item` adds
  `count - 1` to an item's default charges; a container reward uses the specified quantity.
- Choose the smallest relevant validation route. Run one build at a time; rebuild only when
  inputs change or a failure requires it. Reuse the browser game built by the Wasm test suite
  for manual checks. If the user objects to waiting, ask before starting another long build
  and report what remains unverified.
- When a mistake reveals a missing or incorrect rule, update the relevant guide or skill and
  fix the current branch before summarizing.

## Read the guidance for your task

| Work | Start here |
| --- | --- |
| C++ | [Code style](docs/en/dev/explanation/code_style.md) |
| Formatting, linting, or tooling | [Formatting and linting](docs/en/dev/guides/formatting.md) |
| Tests, including browser UI and missions | [Testing guide](docs/en/dev/guides/testing.md) and [test-quality skill](.agents/skills/test-quality/SKILL.md) |
| Browser saves | [Browser save behavior](docs/en/game/game_files_and_save_corruption.md) |
| Lua and C++ bindings | [Lua authoring conventions](docs/en/mod/lua/guides/authoring_conventions.md) |
| New or substantially revised NPCs, dialogue, or missions | [Dialogue and quest design](docs/en/mod/json/guides/dialogue_and_quest_design.md), then the [NPC](docs/en/mod/json/reference/creatures/npcs.md) and [mission](docs/en/mod/json/reference/creatures/missions_json.md) references |
| New or substantially revised factions or settlements | [Faction and settlement design](docs/en/mod/json/guides/faction_and_settlement_design.md) |
| Item data | [Items cookbook](docs/en/dev/guides/items.md) |
| Generated files or overmap terrain matching | [Development practices](docs/en/dev/explanation/best_practices.md) |
| End-to-end task workflow | [Development workflow](docs/en/contribute/contributing.md#quickstart) |
| General development or review | [Developer docs](docs/en/dev/index.md) |

## Contribution workflow

1. For an issue link, fetch it with `gh`, then run `git gtr new <type>/<issue-id>/<slug>`
   (`feat`, `fix`, `refactor`, `chore`, `build`, `ci`).
2. Commit atomically with [Conventional Commits](docs/en/contribute/pr_title_guidelines.md);
   omit the body and footer unless critical.
3. Follow the [PR skill](.agents/skills/pr/SKILL.md) and use the repository's PR template if available.
   Prefer ASD-STE100 Simplified Technical English. Track `gh pr checks` until done; fix
   branch-owned failures, and rerun or report transient failures with the failing job.
