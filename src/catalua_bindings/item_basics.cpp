#include "calendar.h"
#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "item.h"
#include "itype.h"
#include "mtype.h"
#include "type_id.h"

#include <iterator>
#include <ranges>
#include <vector>

namespace cata::detail {
auto reg_item_basics(sol::usertype<item>& ut) -> void {
#define UT_CLASS item
    luna::set_fx(ut, "get_type", &item::typeId);
    DOC("Almost for a corpse.");
    luna::set_fx(ut, "get_mtype", [](const item& it) {
        return it.get_mtype() ? it.get_mtype()->id : mtype_id::NULL_ID();
    });

    DOC("Translated item name with prefixes");
    SET_FX(tname);

    DOC("Display name with all bells and whistles like ammo and prefixes");
    SET_FX(display_name);

    DOC("Weight of the item. The first `bool` is whether including contents, second `bool` is whether it is `integral_weight`.");
    luna::set_fx(
        ut, "weight",
        [](item& it, sol::optional<bool> include_contents, sol::optional<bool> integral) {
            return it.weight(include_contents.value_or(true), integral.value_or(false));
        });

    DOC("Volume of the item. `bool` is whether it is `integral_volume`.");
    luna::set_fx(ut, "volume", [](item& it, sol::optional<bool> integral) {
        return it.volume(integral.value_or(false));
    });

    DOC("Cents of the item. `bool` is whether it is a post-cataclysm value.");
    SET_FX(price);

    DOC("Check for variable of any type");
    SET_FX(has_var);
    DOC("Erase variable");
    SET_FX(erase_var);
    DOC("Erase all variables");
    SET_FX(clear_vars);
    DOC("Returns all stored item vars as a table mapping each var name (string) to its raw value (string).");
    luna::set_fx(ut, "vars_table", [](const item& it, sol::this_state state) {
        sol::state_view lua(state);
        sol::table vars = lua.create_table();
        std::ranges::for_each(it.item_vars(), [&](const auto& entry) {
            vars[entry.first] = entry.second;
        });
        return vars;
    });

    DOC("Spawns a new item. Same as gapi.create_item ");
    DOC("`count` sets the item's charges, not the number of items: exactly one item is created.");
    DOC("For stackable (count-by-charges) items such as ammo, `count` is the stack size. For non-stackable items, pass a negative value (e.g. -1), since a positive one is still applied as charges. Tools spawned with a negative value get their default charges.");
    luna::set_fx(ut, "spawn", [](const itype_id& itype, int count) {
        return item::spawn(itype, calendar::turn, count);
    });

#undef UT_CLASS
}
} // namespace cata::detail
