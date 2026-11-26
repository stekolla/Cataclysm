#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_log.h"
#include "catalua_luna.h"
#include "catalua_luna_doc.h"
#include "debug.h"
#include "game.h"

#include <string>
#include <utility>

namespace cata {
auto reload_lua_code() -> void;
} // namespace cata

namespace {

auto lua_log_info_impl(sol::variadic_args va) -> void {
    std::string msg = cata::detail::fmt_lua_va(va);

    DebugLog(DL::Info, DC::Lua) << msg;
    cata::get_lua_log_instance().add(cata::LuaLogLevel::Info, std::move(msg));
}

auto lua_log_warn_impl(sol::variadic_args va) -> void {
    std::string msg = cata::detail::fmt_lua_va(va);

    DebugLog(DL::Warn, DC::Lua) << msg;
    cata::get_lua_log_instance().add(cata::LuaLogLevel::Warn, std::move(msg));
}

auto lua_log_error_impl(sol::variadic_args va) -> void {
    std::string msg = cata::detail::fmt_lua_va(va);

    DebugLog(DL::Error, DC::Lua) << msg;
    cata::get_lua_log_instance().add(cata::LuaLogLevel::Error, std::move(msg));
}

auto lua_debugmsg_impl(sol::variadic_args va) -> void {
    std::string msg = cata::detail::fmt_lua_va(va);

    debugmsg("%s", msg);
    cata::get_lua_log_instance().add(cata::LuaLogLevel::DebugMsg, std::move(msg));
}

} // namespace

auto cata::detail::reg_debug_api(sol::state& lua) -> void {
    DOC("Debugging and logging API.");
    luna::userlib lib = luna::begin_lib(lua, "gdebug");

    luna::set_fx(lib, "log_info", &lua_log_info_impl);
    luna::set_fx(lib, "log_warn", &lua_log_warn_impl);
    luna::set_fx(lib, "log_error", &lua_log_error_impl);
    luna::set_fx(lib, "debugmsg", &lua_debugmsg_impl);
    luna::set_fx(lib, "clear_lua_log", []() { cata::get_lua_log_instance().clear(); });
    luna::set_fx(lib, "set_log_capacity", [](int v) {
        cata::get_lua_log_instance().set_log_capacity(v);
    });
    luna::set_fx(lib, "reload_lua_code", &cata::reload_lua_code);
    luna::set_fx(lib, "save_game", []() -> bool { return g->save(false); });

    luna::finalize_lib(lib);
}

auto cata::detail::override_default_print(sol::state& lua) -> void {
    lua.globals()["print"] = &lua_log_info_impl;
}

auto cata::detail::forbid_unsafe_functions(sol::state& lua) -> void {
    auto g = lua.globals();
    g["dofile"] = sol::nil;
    g["loadfile"] = sol::nil;
    g["load"] = sol::nil;
    g["loadstring"] = sol::nil;
}
