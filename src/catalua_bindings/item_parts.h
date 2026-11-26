#pragma once

#include "catalua_sol_fwd.h"

class item;

namespace cata::detail {
auto reg_item_user_type(sol::state& lua) -> void;
auto reg_item_type(sol::state& lua) -> void;
auto reg_item_slots(sol::state& lua) -> void;
auto reg_item_explosion_data(sol::state& lua) -> void;
auto reg_item_basics(sol::usertype<item>& ut) -> void;
auto reg_item_classification_and_charges(sol::usertype<item>& ut) -> void;
auto reg_item_ownership_and_contents(sol::usertype<item>& ut) -> void;
auto reg_item_variables_and_damage(sol::usertype<item>& ut) -> void;
auto reg_item_slot_consumables(sol::state& lua) -> void;
auto reg_item_slot_equipment(sol::state& lua) -> void;
auto reg_item_slot_weapons(sol::state& lua) -> void;
auto reg_item_slot_ammo(sol::state& lua) -> void;
auto reg_item_slot_misc(sol::state& lua) -> void;
} // namespace cata::detail
