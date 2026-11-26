#include "bionics.h"
#include "catalua_bindings/character_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_item_filters.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "creature.h"
#include "flag.h"
#include "inventory.h"
#include "item.h"
#include "skill.h"
#include "type_id.h"
#include "units_mass.h"
#include "units_volume.h"

namespace cata::detail {
auto reg_character_bionics_and_equipment(sol::usertype<Character>& ut) -> void {
#define UT_CLASS Character
    // Power saving / auto-start controls (by bionic_id)
    luna::set_fx(ut, "toggle_safe_fuel_mod", [](UT_CLASS& ch, const bionic_id& bid) {
        if (ch.has_bionic(bid)) { ch.get_bionic_state(bid).toggle_safe_fuel_mod(); }
    });
    luna::set_fx(ut, "toggle_auto_start_mod", [](UT_CLASS& ch, const bionic_id& bid) {
        if (ch.has_bionic(bid)) { ch.get_bionic_state(bid).toggle_auto_start_mod(); }
    });
    luna::set_fx(ut, "set_auto_start_thresh", [](UT_CLASS& ch, const bionic_id& bid, float val) {
        if (ch.has_bionic(bid)) { ch.get_bionic_state(bid).set_auto_start_thresh(val); }
    });
    luna::set_fx(ut, "get_auto_start_thresh", [](UT_CLASS& ch, const bionic_id& bid) -> float {
        if (ch.has_bionic(bid)) { return ch.get_bionic_state(bid).get_auto_start_thresh(); }
        return -1.0f;
    });
    luna::set_fx(ut, "is_auto_start_on", [](UT_CLASS& ch, const bionic_id& bid) -> bool {
        if (ch.has_bionic(bid)) { return ch.get_bionic_state(bid).is_auto_start_on(); }
        return false;
    });
    luna::set_fx(ut, "is_auto_start_keep_full", [](UT_CLASS& ch, const bionic_id& bid) -> bool {
        if (ch.has_bionic(bid)) { return ch.get_bionic_state(bid).is_auto_start_keep_full(); }
        return false;
    });

    SET_FX_T(get_power_level, units::energy() const);
    SET_FX_T(get_max_power_level, units::energy() const);
    SET_FX_T(mod_power_level, void(const units::energy&));
    SET_FX_T(mod_max_power_level, void(const units::energy&));
    SET_FX_T(set_power_level, void(const units::energy&));
    SET_FX_T(set_max_power_level, void(const units::energy&));
    SET_FX_T(is_max_power, bool() const);
    SET_FX_T(has_power, bool() const);
    SET_FX_T(has_max_power, bool() const);
    SET_FX_T(enough_power_for, bool(const bionic_id&) const);

    SET_FX_T(is_worn, bool(const item&) const);

    SET_FX_T(weight_carried, units::mass() const);
    SET_FX_T(volume_carried, units::volume() const);

    SET_FX_T(volume_capacity, units::volume() const);

    SET_FX_T(can_pick_volume, bool(units::volume) const);
    SET_FX_T(can_pick_weight, bool(units::mass, bool) const);

    SET_FX_T(is_armed, bool() const);

    luna::set_fx(ut, "can_wield", [](const UT_CLASS& utObj, const item& i) -> bool {
        const auto result = utObj.can_wield(i);
        return result.success() && result.value();
    });

    SET_FX_T(wield, bool(item & target));

    luna::set_fx(ut, "can_unwield", [](const UT_CLASS& utObj, const item& i) -> bool {
        const auto result = utObj.can_unwield(i);
        return result.success() && result.value();
    });

    SET_FX_T(unwield, bool());

    SET_FX_T(is_wielding, bool(const item&) const);

    SET_FX(wielded_items);

    SET_FX_T(is_wearing, bool(const item&) const);

    SET_FX_T(is_wearing_on_bp, bool(const itype_id&, const bodypart_id&) const);

    SET_FX_T(worn_with_flag, bool(const flag_id&, const bodypart_id&) const);

    SET_FX_T(worn_with_id, bool(const itype_id&, const bodypart_id&) const);

    SET_FX_T(item_worn_with_flag, const item*(const flag_id&, const bodypart_id&) const);

    SET_FX_T(item_worn_with_id, const item*(const itype_id&, const bodypart_id&) const);

    SET_FX_T(get_skill_level, int(const skill_id&) const);

    SET_FX_T(get_all_skills, const SkillLevelMap&() const);
    SET_FX_T(get_skill_level_object, SkillLevel & (const skill_id& ident));

    SET_FX_T(set_skill_level, void(const skill_id&, int));
    SET_FX_T(mod_skill_level, void(const skill_id&, int));

    SET_FX_T(rust_rate, int() const);

    SET_FX_T(practice, void(const skill_id&, int, int, bool));

    SET_FX_T(read_speed, int(bool) const);

    SET_FX_T(get_time_died, time_point() const);

    SET_FX_T(is_rad_immune, bool() const);

    SET_FX_T(is_throw_immune, bool() const);

    SET_FX_T(rest_quality, float() const);

    SET_FX_T(healing_rate, float(float) const);

    SET_FX_T(healing_rate_medicine, float(float, const bodypart_id&) const);

    SET_FX_T(mutation_value, float(const std::string&) const);

    SET_FX_T(get_base_traits, std::vector<trait_id>() const);

    SET_FX_T(get_mutations, std::vector<trait_id>(bool) const);

    SET_FX_T(clear_skills, void());

    SET_FX_T(clear_mutations, void());

    SET_FX_T(crossed_threshold, bool() const);

    SET_FX_T(add_addiction, void(add_type, int));
    SET_FX_T(rem_addiction, void(add_type));
    SET_FX_T(has_addiction, bool(add_type) const);
    SET_FX_T(addiction_level, int(add_type) const);

    SET_FX_T(is_hauling, bool() const);

    DOC("Adds a detached item to the player inventory");
    luna::set_fx(ut, "add_item", [](UT_CLASS& c, detached_ptr<item>& i) { c.i_add(std::move(i)); });

    DOC("Creates an item with the given id and adds it to the player inventory.");
    DOC("`count` sets the item's charges, not the number of items: exactly one item is created.");
    DOC("For stackable (count-by-charges) items such as ammo, `count` is the stack size. For non-stackable items, pass a negative value (e.g. -1), since a positive one is still applied as charges. Tools spawned with a negative value get their default charges.");
    luna::set_fx(ut, "create_item", [](UT_CLASS& c, const itype_id& itype, int count) {
        return &c.add_item_with_id(itype, count);
    });

    DOC("DEPRECATED: use create_item instead");
    SET_FX_T(add_item_with_id, item & (const itype_id& itype, int count));

    DOC("Checks for an item with the given id");
    SET_FX_T(has_item_with_id, bool(const itype_id& itype, bool need_charges) const);

    DOC("Gets the first occurrence of an item with the given id");
    SET_FX_T(get_item_with_id, const item*(const itype_id& itype, bool need_charges) const);

    DOC("Checks for an item with the given flag");
    SET_FX_T(has_item_with_flag, bool(const flag_id& flag, bool need_charges) const);

    DOC("Gets all items with the given flag");
    SET_FX_T(all_items_with_flag, std::vector<item*>(const flag_id& flag, bool need_charges) const);

    DOC("Gets all items");
    SET_FX_T(all_items, std::vector<item*>(bool need_charges) const);

    DOC("Filters items");
    luna::set_fx(ut, "items_with", &Character::items_with);

    DOC("Returns items from the inventory, wielded and worn, matching every given filter as an array of Item.  ");
    DOC("Takes a table where each key is a filter; unknown keys show a debug message and are ignored. Available filters:  ");
    DOC("* `category` (string or string[]): Item category id is one of these, e.g. \"food\".  ");
    DOC("* `type` (ItemTypeId or ItemTypeId[]): Item type is one of these.  ");
    DOC("* `is_food` (boolean): Whether the item is food.  ");
    DOC("* `flags` (FlagId or FlagId[]): Item has at least one of these flags.  ");
    DOC("* `need_charges` (boolean): Tool items must have charges left.  ");
    DOC("* `has_charges` (table): { id = ItemTypeId, amount = integer }: item type is `id` and has at least `amount` charges.  ");
    DOC("* `addiction_type` (AddictionType, string, or array of those): Comestible's addiction type is one of these, e.g. `AddictionType.ALCOHOL` or `\"alcohol\"`.  ");
    DOC("* `material` (MaterialTypeId, string, or array of those): Item is made of at least one of these materials, e.g. `\"stone\"`.  ");
    DOC("* `worn` (boolean): Whether the item is worn.  ");
    DOC("* `wielded` (boolean): Whether the item is wielded.");
    luna::
        set_fx(ut, "get_items_if", [](const UT_CLASS& c, sol::table filters) -> std::vector<item*> {
            return c.get_items_if(character_item_predicates_from_lua(c, filters));
        });

    DOC("Removes and returns items from the inventory, wielded and worn, matching every given filter as an array of Item.  ");
    DOC("Accepts the same filter table as `get_items_if`.");
    luna::set_fx(
        ut, "remove_items_if",
        [](UT_CLASS& c, sol::table filters) -> std::vector<detached_ptr<item>> {
            return c.remove_items_if(character_item_predicates_from_lua(c, filters));
        });

    DOC("DEPRECATED: use remove_item instead");
    luna::set_fx(ut, "inv_remove_item", &Character::inv_remove_item);

    DOC("Removes given `Item` from character's inventory. The `Item` must be in the inventory, neither wielded nor worn.");
    luna::set_fx(ut, "remove_item", [](UT_CLASS& c, item& it) -> detached_ptr<item> {
        return c.inv_remove_item(&it);
    });

    DOC("Checks if a given `Item` can be taken off.");
    luna::set_fx(ut, "can_takeoff", [](const UT_CLASS& c, const item& it) {
        const auto res = c.can_takeoff(it);
        return res.success() && res.value();
    });

    DOC("Attempts to take off the worn `Item` from character.");
    luna::set_fx(ut, "takeoff", [](UT_CLASS& c, item& it) { return c.takeoff(it, nullptr); });

    DOC("Attempts to remove the worn `Item` from character.");
    luna::set_fx(ut, "remove_worn", [](UT_CLASS& c, item& it) -> std::optional<detached_ptr<item>> {
        std::vector<detached_ptr<item>> res{};
        if (c.takeoff(it, &res)) { return std::make_optional(std::move(res[0])); }
        return std::nullopt;
    });

    luna::set_fx(ut, "get_dependant_worn_items", [](const UT_CLASS& c, const item& it) {
        auto lst = c.get_dependent_worn_items(it);
        std::vector<item*> res = {};
        std::ranges::copy(lst, std::back_inserter(res));
        return res;
    });
#undef UT_CLASS
}
} // namespace cata::detail
