#include "ammo.h"
#include "avatar.h"
#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "item.h"
#include "itype.h"
#include "martialarts.h"
#include "material.h"
#include "mtype.h"
#include "type_id.h"

#include <ranges>
#include <set>
#include <vector>

namespace cata::detail {
auto reg_item_classification_and_charges(sol::usertype<item>& ut) -> void {
#define UT_CLASS item
    SET_FX(is_null);
    SET_FX(is_unarmed_weapon);
    SET_FX(is_sided);
    SET_FX(is_power_armor);
    SET_FX(is_money);
    SET_FX(is_gun);
    SET_FX(is_firearm);
    SET_FX(is_silent);
    SET_FX(is_gunmod);
    SET_FX(is_bionic);
    SET_FX(is_ammo_belt);
    SET_FX(is_bandolier);
    SET_FX(is_holster);
    SET_FX(is_ammo);
    SET_FX(is_comestible);
    SET_FX(is_food);
    SET_FX(is_medication);
    SET_FX(is_brewable);
    SET_FX(is_food_container);
    SET_FX(is_med_container);
    SET_FX(is_corpse);
    SET_FX(is_ammo_container);
    SET_FX(is_armor);
    SET_FX(is_book);
    SET_FX(is_map);
    SET_FX(is_container);
    SET_FX(is_watertight_container);
    SET_FX(is_non_resealable_container);
    SET_FX(is_bucket);
    SET_FX(is_bucket_nonempty);
    SET_FX(is_engine);
    SET_FX(is_wheel);
    SET_FX(is_fuel);
    SET_FX(is_toolmod);
    SET_FX(is_faulty);
    SET_FX(is_irremovable);
    SET_FX(is_container_empty);
    SET_FX(is_salvageable);
    SET_FX(is_craft);
    SET_FX(is_emissive);
    SET_FX(is_deployable);
    SET_FX(is_tool);
    SET_FX(is_transformable);
    SET_FX(is_artifact);
    SET_FX(is_relic);
    SET_FX(is_seed);
    SET_FX(is_dangerous);
    SET_FX(is_tainted);
    SET_FX(is_soft);
    SET_FX(is_reloadable);
    luna::set_fx(ut, "has_use", [](item& it, std::string use) { return it.get_use(use); });
    DOC("DEPRECATED: Items are no longer filthy");
    luna::set_fx(ut, "is_filthy", []() { return false; });
    SET_FX(is_active);
    SET_FX(is_upgrade);

    SET_FX(activate);
    SET_FX(deactivate);
    DOC("Immediately invokes this item's use action at the given map-square position. Returns the charges consumed by the use action.");
    luna::set_fx(ut, "invoke_at", [](item& it, const tripoint_bub_ms& pos) -> int {
        return it.type->invoke(get_avatar(), it, pos);
    });
    SET_FX(set_charges);

    SET_FX(set_counter);
    SET_FX(get_counter);

    DOC("Is this item an effective melee weapon for the given damage type?");
    luna::set_fx(ut, "is_melee", sol::resolve<bool(damage_type) const>(&item::is_melee));

    DOC("Gets the weapon category of the item.");
    luna::set_fx(ut, "weapon_categories", [](const item& it) -> std::vector<weapon_category_id> {
        return it.typeId()->weapon_category | std::ranges::to<std::vector>();
    });

    DOC("Is this a magazine? (batteries are magazines)");
    SET_FX(is_magazine);

    DOC("DEPRECATED: Is this a battery? (spoiler: it isn't)");
    SET_FX(is_battery);

    SET_FX(conductive);

    luna::set_fx(ut, "is_stackable", sol::resolve<bool() const>(&item::count_by_charges));

    luna::set(ut, "charges", &item::charges);

    SET_FX(energy_remaining);

    SET_FX(has_infinite_charges);

    SET_FX(mod_charges);

    luna::set_fx(ut, "made_of", [](item& it) { return it.made_of(); });

    luna::set_fx(ut, "is_made_of",
                 sol::resolve<auto(const material_id&) const->bool>(&item::made_of));

    luna::set_fx(ut, "get_kcal", [](item& it) -> int {
        return it.is_comestible() ? it.get_comestible()->default_nutrition.kcal : 0;
    });
    luna::set_fx(ut, "get_quench", [](item& it) -> int {
        return it.is_comestible() ? it.get_comestible()->quench : 0;
    });
    luna::set_fx(ut, "get_comestible_fun", [](item& it) -> int { return it.get_comestible_fun(); });
    DOC("Gets the TimeDuration until this item rots");
    SET_FX(get_rot);

#undef UT_CLASS
}
} // namespace cata::detail
