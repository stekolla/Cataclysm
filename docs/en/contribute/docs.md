# Updating documentation

Documentation is stored as Markdown in this directory. Edit the relevant `.md` file, keep links to
other documentation relative and include the `.md` extension, then submit the change with a pull
request. The [docs index](../../README.md) links to the main sections.

Use standard Markdown and fenced code blocks. Keep screenshots and diagrams in the repository next
to the page that uses them. Avoid site-specific front matter, HTML widgets, and links that depend on
site URL routing.

The CLI and Lua API reference pages are generated from the game source. From the repository root,
run:

```sh
deno task docs:gen
```

This updates `docs/en/dev/reference/cli_options.md`, `docs/en/mod/lua/reference/lua.md`, and
`lua_annotations.lua` (used for Lua IDE autocompletion).

## License

- By contributing to Markdown files, you agree to license your contributions under
  [CC-BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/), the same license as the game.
