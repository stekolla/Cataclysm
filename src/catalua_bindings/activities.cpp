#include "activity_type.h"
#include "avatar.h"
#include "calendar.h"
#include "catalua.h"
#include "catalua_bindings.h"
#include "catalua_bindings/character_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_coord.h"
#include "catalua_luna_doc.h"
#include "catalua_serde.h"
#include "character.h"
#include "character_martial_arts.h"
#include "creature.h"
#include "json.h"
#include "player_activity.h"

#include <climits>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string_view>

namespace {

constexpr std::string_view lua_activity_data_prefix = "lua_activity_data:";
constexpr std::string_view lua_activity_on_finish_prefix = "lua_activity_on_finish:";
constexpr std::string_view lua_activity_on_turn_prefix = "lua_activity_on_turn:";

auto serialize_lua_activity_data(const sol::optional<sol::table>& maybe_data) -> std::string {
    if (!maybe_data) { return {}; }

    auto buffer = std::ostringstream{};
    auto jsout = JsonOut(buffer);
    cata::serialize_lua_table(*maybe_data, jsout);
    return buffer.str();
}

struct lua_activity_options {
    activity_id activity;
    time_duration duration;
    std::string on_finish;
    std::string on_turn;
    std::optional<tripoint_bub_ms> pos;
    std::string name;
    bool interruptable = true;
    std::string data;
};

auto parse_lua_activity_options(const sol::table& opts) -> lua_activity_options {
    // Read the pos option through the direct coord conversion helper.
    // Going through sol::optional<tripoint_bub_ms> relies on sol's ADL-based
    // conversion dispatch, which silently drops the option on MSVC builds.
    std::optional<tripoint_bub_ms> pos;
    if (const sol::object raw_pos = opts["pos"]; raw_pos.get_type() == sol::type::userdata) {
        pos = cata::detail::lua_coords::as_cpp<tripoint_bub_ms>(raw_pos);
        if (!pos) { debugmsg("assign_lua_activity: pos option is not a TripointBubMs coordinate"); }
    }
    const auto on_finish = opts.get<sol::optional<std::string>>("on_finish");
    const auto on_turn = opts.get<sol::optional<std::string>>("on_turn");
    const auto interruptable = opts.get<sol::optional<bool>>("interruptable");
    auto result = lua_activity_options{
        .activity = opts.get<activity_id>("type"),
        .duration = opts.get<time_duration>("duration"),
        .on_finish = on_finish.value_or(""),
        .on_turn = on_turn.value_or(""),
        .pos = pos,
        .name = opts.get_or<std::string>("name", ""),
        .interruptable = interruptable.value_or(true),
        .data = {},
    };
    const auto data = opts.get<sol::optional<sol::table>>("data");
    result.data = serialize_lua_activity_data(data);
    return result;
}

auto make_lua_activity(const lua_activity_options& opts) -> std::unique_ptr<player_activity> {
    auto act = std::make_unique<
        player_activity>(opts.activity, to_moves<int>(opts.duration), -1, INT_MIN, opts.name);
    act->interruptable_with_kb = opts.interruptable;
    if (!opts.on_finish.empty()) {
        act->str_values.push_back(std::string{lua_activity_on_finish_prefix} + opts.on_finish);
    }
    if (!opts.on_turn.empty()) {
        act->str_values.push_back(std::string{lua_activity_on_turn_prefix} + opts.on_turn);
    }
    if (!opts.data.empty()) {
        act->str_values.push_back(std::string{lua_activity_data_prefix} + opts.data);
    }
    auto append_pos = [&act](const tripoint_bub_ms& pos) -> tripoint_abs_ms {
        const auto abs_pos = bub_to_abs(pos);
        act->coords.push_back(abs_pos);
        return abs_pos;
    };
    if (opts.pos) { act->placement = append_pos(*opts.pos); }
    return act;
}


auto reg_player_activity(sol::state& lua) -> void {
    auto ut = luna::new_usertype<player_activity>(lua, luna::no_bases, luna::no_constructor);
    luna::set(ut, "moves_total", &player_activity::moves_total);
    luna::set(ut, "moves_left", &player_activity::moves_left);
    luna::set(ut, "interruptable_with_kb", &player_activity::interruptable_with_kb);
    luna::set(ut, "name", &player_activity::name);
    luna::set_prop(ut, "coords", [](const player_activity& act) -> std::vector<tripoint_abs_ms> {
        return act.coords;
    });
    luna::set_fx(ut, "id", [](const player_activity& act) -> activity_id { return act.id(); });
    luna::set_fx(ut, "id_str", [](const player_activity& act) -> std::string {
        return act.id().str();
    });
}

} // namespace

namespace cata::detail {
auto make_lua_activity_from_options(const sol::table& opts) -> std::unique_ptr<player_activity> {
    return make_lua_activity(parse_lua_activity_options(opts));
}
} // namespace cata::detail

void cata::detail::reg_creature_family(sol::state& lua) {
    reg_player_activity(lua);
    reg_creature(lua);
    reg_monster(lua);
    reg_character(lua);
    reg_player(lua);
    reg_npc(lua);
    reg_avatar(lua);
}
