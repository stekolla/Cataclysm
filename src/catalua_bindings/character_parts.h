#pragma once

#include "catalua_luna.h"
#include "catalua_sol_fwd.h"

#include <memory>

class Character;
class player_activity;

LUNA_VAL(player_activity, "PlayerActivity")

namespace cata::detail {
auto reg_character_core(sol::usertype<Character>& ut) -> void;
auto reg_character_mutations(sol::usertype<Character>& ut) -> void;
auto reg_character_bionics_and_equipment(sol::usertype<Character>& ut) -> void;
auto reg_character_items_and_activity(sol::usertype<Character>& ut) -> void;
auto reg_character_world_and_crafting(sol::usertype<Character>& ut) -> void;
auto make_lua_activity_from_options(const sol::table& opts) -> std::unique_ptr<player_activity>;
} // namespace cata::detail
