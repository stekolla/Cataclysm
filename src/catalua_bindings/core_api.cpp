#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna.h"
#include "catalua_luna_doc.h"
#include "game_constants.h"
#include "name.h"
#include "rng.h"
#include "translations.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <ctime>
#include <string>
#include <utility>

auto cata::detail::fmt_lua_va(sol::variadic_args va) -> std::string {
    lua_State* L = va.lua_state();
    sol::state_view lua(L);

    std::string msg;
    for (auto it : va) { msg += lua["tostring"](it); }

    return msg;
}
auto cata::detail::reg_constants(sol::state& lua) -> void {
    DOC("Various game constants");
    luna::userlib lib = luna::begin_lib(lua, "const");

    luna::set(lib, "OM_OMT_SIZE", OMAPX);
    luna::set(lib, "OM_SM_SIZE", OMAPX * 2);
    luna::set(lib, "OM_MS_SIZE", OMAPX * 2 * SEEX);
    luna::set(lib, "OMT_SM_SIZE", 2);
    luna::set(lib, "OMT_MS_SIZE", SEEX * 2);
    luna::set(lib, "SM_MS_SIZE", SEEX);

    luna::finalize_lib(lib);
}
namespace {

auto local_time_impl() -> tm* {
    const time_t timestamp = time(nullptr);
    return localtime(&timestamp);
}

// This is from weather.cpp, but requires calandar.h.
// I don't want to include that here since that's fairly awkward.
const std::array<std::string, 7> weekday_names = {
    {translate_marker("Sunday"), translate_marker("Monday"), translate_marker("Tuesday"),
     translate_marker("Wednesday"), translate_marker("Thursday"), translate_marker("Friday"),
     translate_marker("Saturday")}};

} // namespace

auto cata::detail::reg_date_time_api(sol::state& lua) -> void {
    DOC("System date and time API.");
    luna::userlib lib = luna::begin_lib(lua, "date_time");

    luna::set_fx(lib, "year", []() { return local_time_impl()->tm_year + 1900; });
    // It makes sense to start month at 1, not 0
    luna::set_fx(lib, "month", []() { return local_time_impl()->tm_mon + 1; });
    DOC("Days since Saturday.");
    luna::set_fx(lib, "weekday", []() { return local_time_impl()->tm_wday; });
    luna::set_fx(lib, "weekday_str", []() { return weekday_names[local_time_impl()->tm_wday]; });
    luna::set_fx(lib, "day", []() { return local_time_impl()->tm_mday; });
    // Hour is different, since digital clocks wrap around at 24
    DOC("0 -> 23");
    luna::set_fx(lib, "hour", []() { return local_time_impl()->tm_hour; });
    luna::set_fx(lib, "minute", []() { return local_time_impl()->tm_min; });
    luna::set_fx(lib, "second", []() { return local_time_impl()->tm_sec; });
    luna::set_fx(lib, "millisecond", []() -> int {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now).count() % 1000;
    });

    luna::finalize_lib(lib);
}

const auto& gettext_raw = sol::resolve<const char*(const char*)>(&detail::_translate_internal);

auto cata::detail::reg_locale_api(sol::state& lua) -> void {
    DOC("English string helpers. These functions return the supplied English text.");
    luna::userlib lib = luna::begin_lib(lua, "locale");
    luna::set_fx(lib, "gettext", gettext_raw);
    luna::set_fx(lib, "vgettext", &vgettext);
    luna::set_fx(lib, "pgettext", &pgettext);
    luna::set_fx(lib, "vpgettext", &vpgettext);
    luna::finalize_lib(lib);
}
namespace {

const auto lowercase = [](std::string t) {
    if (!t.empty()) { t.front() = std::tolower(t.front()); }
    return t;
};

auto string_search(sol::variadic_args va) -> std::string {
    auto flags = static_cast<nameFlags>(0);
    // Only 9 flags exist, so cap
    for (auto i = 0; i < std::min(static_cast<int>(va.size()), 10); i++) {
        if (!va[i].is<std::string>()) { continue; }
        const auto in = lowercase(va.get<std::string>(i));
        flags = flags | Name::usage_flag(in) | Name::gender_flag(in);
    }
    return Name::get(flags);
}

} // namespace

auto cata::detail::reg_names(sol::state& lua) -> void {
    luna::userlib lib = luna::begin_lib(lua, "ch_names");
    DOC("Generates a random full name with an optional boolean for gender.");
    DOC("The loaded name is one of usage with optional gender.");
    DOC("The combinations used in names files are as follows:");
    DOC("");
    DOC("Backer | (Female|Male|Unisex)");
    DOC("Given  | (Female|Male)        // unisex names are duplicated in each group");
    DOC("Family | Unisex");
    DOC("Nick");
    DOC("City");
    DOC("World");
    luna::set_fx(lib, "generate", [](const sol::optional<bool> male) -> std::string {
        if (male.has_value()) { return Name::generate(male.value()); }
        return Name::generate(one_in(2));
    });
    DOC("Generates a single name using any combination of search flags.");
    luna::set_fx(lib, "pick", [](sol::variadic_args va) -> std::string {
        if (va.size() < 1 || !va[0].is<std::string>()) { return std::string(); };
        return string_search(va);
    });

    luna::finalize_lib(lib);
}
auto cata::detail::reg_testing_library(sol::state& lua) -> void {
    DOC("Library for testing purposes");
    luna::userlib lib = luna::begin_lib(lua, "tests_lib");

    // Regression test for https://github.com/ThePhD/sol2/issues/1444
    luna::set_fx(lib, "my_awesome_lambda_1", []() -> int { return 1; });
    luna::set_fx(lib, "my_awesome_lambda_2", []() -> int { return 2; });

    luna::finalize_lib(lib);
}
