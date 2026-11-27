# Lua authoring conventions

- Type every function parameter with EmmyLua/LuaLS `---@param`, including parameters of local
  helpers and existing functions you change. Use `---@class` and `---@field` for table shapes.
  Match the file's annotation style. Do not add `---@return` solely for enforcement when the
  return type can be inferred.
- Fix missing binding types at the binding or documentation-generation source, not by adding
  handwritten classes to generated output.
- Test C++ Lua bindings with real bound objects. Lua-only mocks can supplement those tests but
  cannot verify the bindings themselves.

For binding patterns, see the [Lua binding reference skill](../../../../../.agents/skills/lua-binding-reference/SKILL.md).
