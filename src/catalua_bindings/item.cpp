#include "item.h"

#include "catalua_bindings.h"
#include "catalua_bindings/item_parts.h"
#include "catalua_luna.h"
#include "catalua_luna_doc.h"

namespace cata::detail {
auto reg_item(sol::state& lua) -> void {
    reg_item_explosion_data(lua);
    reg_item_type(lua);
    reg_item_slots(lua);
    reg_item_user_type(lua);
}

auto reg_item_user_type(sol::state& lua) -> void {
    auto ut = luna::new_usertype<item>(lua, luna::no_bases, luna::no_constructor);
    reg_item_basics(ut);
    reg_item_classification_and_charges(ut);
    reg_item_ownership_and_contents(ut);
    reg_item_variables_and_damage(ut);
}
} // namespace cata::detail
