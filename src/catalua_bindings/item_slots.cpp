#include "catalua_bindings/item_parts.h"

namespace cata::detail {
auto reg_item_slots(sol::state& lua) -> void {
    reg_item_slot_consumables(lua);
    reg_item_slot_equipment(lua);
    reg_item_slot_weapons(lua);
    reg_item_slot_ammo(lua);
    reg_item_slot_misc(lua);
}
} // namespace cata::detail
