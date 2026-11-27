# Development workflow

This guide is for agents and maintainers working in an existing checkout. Start with
[AGENTS.md](../../../AGENTS.md) for repository-wide rules and the
[developer index](../dev/index.md) for task-specific guidance.

## Quickstart

1. **Establish scope.** Read the request and its linked issue or PR. Check the current branch
   and worktree before editing. Preserve unrelated work. For a new issue task, follow the
   worktree convention in [AGENTS.md](../../../AGENTS.md).
2. **Find the existing path.** Trace the behavior from the relevant call site, data definition,
   or test before adding code. For content work, start with the [authoring index](../mod/index.md)
   and look for a similar definition in the repository.
3. **Make the smallest complete change.** Update directly affected docs and tests. Use the
   [formatting guide](../dev/guides/formatting.md) for the file types you touched.
4. **Validate the changed behavior.** Use the [testing guide](../dev/guides/testing.md) to choose
   the smallest relevant route. For visible behavior, follow the
   [browser build guide](../dev/guides/building/cmake.md) to reuse an existing build when possible.
   Use a disposable world and [in-game debug tools](../mod/json/reference/ingame_debugging_tools.md)
   when you need a specific test setup.
5. **Report the outcome.** Say what changed, what checks ran, and what remains unverified.
   If a check fails, distinguish failures caused by the change from baseline failures.

## Pull request notes

When opening or updating a PR, use the [PR skill](../../../.agents/skills/pr/SKILL.md) and
[title guidelines](pr_title_guidelines.md). Explain the problem, the change, and the evidence
that it works. Keep unfinished PRs in draft. Use `Fixes #123` only if merging the PR should
close that issue.
