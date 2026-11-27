---
name: test-quality
description: Write or review Cataclysm tests that avoid flakes, use deterministic fixtures, and clean up global state.
---

# Test Quality

Use when adding or changing automated tests. See the [testing guide](../../../docs/en/dev/guides/testing.md)
to select a runner.

## Gates

- Test one behavior contract, not incidental implementation details.
- Use deterministic fixtures from `tests/data/TEST_DATA` for data with random, balance, or evolving behavior.
- Avoid assertions that rely on probability, wall-clock time, locale, file order, or current global options unless the test pins them.
- Reproduce reported flakes with the failing `--rng-seed` before accepting the fix.
- When a CI test job fails, inspect its exact invocation. Reproduce it on the final revision with the same filters, ordering, seed, user directory, backend, and shard if applicable. A narrower tag filter or a different suite does not replace that check.
- Before pushing test fixes that touch global state, NPCs, maps, overmaps, player position, RNG, options, or fixtures, run both the relevant focused test and the CI-equivalent shard/full command that can expose state bleed from earlier tests.
- Keep Lua/C++ integration tests on real bound objects when the binding behavior is under test.
- Expected `DebugLog(DL::Error, ...)` still makes the test process fail even when assertions pass. Use
  `capture_debug_errors_during` for the injected failure only and assert its exact messages; never clear
  the global observed-error flag or disable the test runner's error check.

## Cleanup

- Restore global state with `restore_on_out_of_scope` for calendar, options, globals, and singleton state.
- Use `clear_map()` and explicit player placement when map contents matter.
- Do not leave creatures, items, hooks, worlds, temp files, or option changes that can affect later tests.
- Prefer new `TEST_DATA` fixtures over mutating base-game data in the test body.
