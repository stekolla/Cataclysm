#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "faction.h"
#include "item.h"
#include "itype.h"
#include "martialarts.h"
#include "type_id.h"

#include <ranges>
#include <set>
#include <vector>

namespace cata::detail {
auto reg_item_ownership_and_contents(sol::usertype<item>& ut) -> void {
#define UT_CLASS item
    DOC("Gets the category id this item is in");
    SET_FX(get_category_id);

    DOC("Gets the faction id that owns this item");
    SET_FX(get_owner);

    DOC("Sets the ownership of this item to a faction or character");
    luna::set_fx(
        ut, "set_owner",
        sol::overload(sol::resolve<void(const faction_id&)>(&item::set_owner),
                      sol::resolve<void(const Character&)>(&item::set_owner)));

    SET_FX(get_owner_name);

    DOC("Checks if this item owned by a character");
    SET_FX(is_owned_by);

    DOC("Checks if this item has the technique as an addition. Doesn't check original techniques.");
    luna::set_fx(ut, "has_technique",
                 sol::resolve<bool(const matec_id&) const>(&item::has_technique));
    DOC("Gets all techniques. Including original techniques.");
    luna::set_fx(ut, "get_techniques",
                 sol::resolve<std::set<matec_id>() const>(&item::get_techniques));
    DOC("Adds the technique. It isn't treated original, but additional.");
    luna::set_fx(ut, "add_technique", sol::resolve<void(const matec_id&)>(&item::add_technique));
    DOC("Removes the additional technique. Doesn't affect originial techniques.");
    luna::set_fx(ut, "remove_technique",
                 sol::resolve<void(const matec_id&)>(&item::remove_technique));

    DOC("Adds an enchantment to this item");
    luna::set_fx(
        ut, "add_enchantment",
        sol::overload(sol::resolve<bool(const enchantment_id&)>(&item::add_enchantment),
                      sol::resolve<bool(const enchantment&)>(&item::add_enchantment)));

    DOC("Checks if this item can contain another");
    luna::set_fx(ut, "can_contain", sol::resolve<bool(const item&) const>(&item::can_contain));

    DOC("Gets the remaining space available for a type of liquid");
    luna::set_fx(ut, "remaining_capacity_for_id", &item::get_remaining_capacity_for_id);

    DOC("Gets maximum volume this item can hold (liquids, ammo, etc)");
    luna::set_fx(ut, "total_capacity", &item::get_total_capacity);

    DOC("Gets the current magazine");
    luna::
        set_fx(ut, "current_magazine", sol::resolve<const item*() const>(&item::magazine_current));

    DOC("Gets the maximum capacity of a magazine");
    luna::set_fx(ut, "ammo_capacity", sol::resolve<int(const bool) const>(&item::ammo_capacity));

    DOC("Get remaining ammo, works with batteries & stuff too");
    SET_FX(ammo_remaining);

    SET_FX(ammo_data);
    SET_FX(ammo_required);
    SET_FX(ammo_current);

    SET_FX(ammo_consume);
    SET_FX(ammo_set);
    SET_FX(ammo_unset);

    SET_FX(get_reload_time);

    DOC("Creates an item with the given id and adds it to contents.");
    DOC("`count` sets the item's charges, not the number of items: exactly one item is created.");
    DOC("For stackable (count-by-charges) items such as ammo, `count` is the stack size. For non-stackable items, pass a negative value (e.g. -1), since a positive one is still applied as charges. Tools spawned with a negative value get their default charges.");
    SET_FX(add_item_with_id);

    DOC("Checks item contents for a given item id");
    SET_FX(has_item_with_id);

    DOC("Checks if the item covers a bodypart");
    SET_FX(covers);

    SET_FX(set_flag);
    SET_FX(unset_flag);
    SET_FX(has_flag);
    SET_FX(has_own_flag);
    SET_FX(set_flag_recursive);
    SET_FX(unset_flags);

    DOC("Converts the item as given `ItypeId`.");
    SET_FX(convert);

#undef UT_CLASS
}
} // namespace cata::detail
