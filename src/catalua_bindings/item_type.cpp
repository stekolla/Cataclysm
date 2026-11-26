#include "ammo.h"
#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "fault.h"
#include "flag.h"
#include "itype.h"
#include "martialarts.h"
#include "map/emit.h"
#include "material.h"
#include "mod_manager.h"
#include "mtype.h"
#include "recipe.h"
#include "relic.h"
#include "skill.h"
#include "type_id.h"
#include "vitamin.h"

#include <iterator>
#include <ranges>
#include <vector>

namespace cata::detail {
#define VALUE_PTR_MEMB_N(prop_name, lua_name_str) \
    luna::set_fx(ut, lua_name_str, [](const UT_CLASS& c) { return c.prop_name.get(); })

auto reg_item_type(sol::state& lua) -> void {
#define UT_CLASS itype
    {
        DOC("Slots for various item type properties. Each slot may contain a valid value or nil");
        auto ut = luna::new_usertype<itype>(lua, luna::no_bases, luna::no_constructor);

        VALUE_PTR_MEMB_N(container, "slot_container");
        VALUE_PTR_MEMB_N(tool, "slot_tool");
        VALUE_PTR_MEMB_N(comestible, "slot_comestible");
        VALUE_PTR_MEMB_N(brewable, "slot_brewable");
        VALUE_PTR_MEMB_N(armor, "slot_armor");
        VALUE_PTR_MEMB_N(pet_armor, "slot_pet_armor");
        VALUE_PTR_MEMB_N(book, "slot_book");
        VALUE_PTR_MEMB_N(mod, "slot_mod");
        VALUE_PTR_MEMB_N(engine, "slot_engine");
        VALUE_PTR_MEMB_N(wheel, "slot_wheel");
        VALUE_PTR_MEMB_N(fuel, "slot_fuel");
        VALUE_PTR_MEMB_N(gun, "slot_gun");
        VALUE_PTR_MEMB_N(gunmod, "slot_gunmod");
        VALUE_PTR_MEMB_N(magazine, "slot_magazine");
        VALUE_PTR_MEMB_N(battery, "slot_battery");
        VALUE_PTR_MEMB_N(bionic, "slot_bionic");
        VALUE_PTR_MEMB_N(ammo, "slot_ammo");
        VALUE_PTR_MEMB_N(seed, "slot_seed");
        VALUE_PTR_MEMB_N(artifact, "slot_artifact");
        VALUE_PTR_MEMB_N(relic_data, "slot_relic");
        VALUE_PTR_MEMB_N(milling_data, "slot_milling");

        // TODO: Check nothing important is missing below

        auto get_action = [](const use_function& c) { return c.get_type(); };
        auto get_uses = [=](const UT_CLASS& c) {
            std::vector<std::string> rv{};
            std::ranges::
                copy(c.use_methods | std::views::values | std::views::transform(get_action),
                     std::back_inserter(rv));
            return rv;
        };

        SET_FX_N(get_id, "type_id");
        SET_FX(can_have_charges);
        SET_FX(can_use);
        SET_FX(charge_factor);
        SET_FX(charges_default);
        SET_FX(charges_per_volume);
        SET_FX(charges_to_use);
        SET_FX_N(count_by_charges, "is_stackable");
        SET_FX(damage_max);
        SET_FX(damage_min);
        SET_FX(get_flags);
        SET_FX(has_flag);
        SET_FX(has_use);
        SET_FX(maximum_charges);
        SET_FX_N(nname, "get_name");
        SET_MEMB_N_RO(explosion, "explosion_data");
        SET_MEMB_N_RO(m_to_hit, "melee_to_hit");
        luna::set_fx(ut, "source_mod", [](const UT_CLASS& c) {
            std::vector<mod_id> rv{};
            std::ranges::copy(c.src | std::views::transform([](auto& p) { return p.second; }),
                              std::back_inserter(rv));
            return rv;
        });
        DOC("Attacks this weapon can perform, keyed by attack id.");
        SET_MEMB_RO(attacks);
        SET_MEMB_RO(countdown_destroy);
        SET_MEMB_RO(countdown_interval);
        SET_MEMB_RO(default_container);
        SET_MEMB_RO(emits);
        SET_MEMB_RO(explode_in_fire);
        SET_MEMB_RO(faults);
        SET_MEMB_RO(integral_volume);
        SET_MEMB_RO(integral_weight);
        SET_MEMB_RO(item_tags);
        SET_MEMB_RO(layer);
        SET_MEMB_RO(light_emission);
        SET_MEMB_RO(looks_like);
        SET_MEMB_RO(materials);
        SET_MEMB_RO(min_dex);
        SET_MEMB_RO(min_int);
        SET_MEMB_RO(min_per);
        DOC("Minimum skill levels required to use this item, keyed by skill.");
        SET_MEMB_RO(min_skills);
        SET_MEMB_RO(min_str);
        SET_MEMB_RO(phase);
        luna::set_fx(ut, "price", [](const UT_CLASS& c) { return c.price.value(); });
        luna::set_fx(ut, "price_post", [](const UT_CLASS& c) { return c.price.value(); });
        DOC("Free-form string properties from the item's JSON `properties` field, keyed by property name.");
        SET_MEMB_RO(properties);
        DOC("Tool qualities this item provides, mapped to their level.");
        SET_MEMB_RO(qualities);
        SET_MEMB_RO(recipes);
        SET_MEMB_RO(repair);
        SET_MEMB_RO(repairs_like);
        SET_MEMB_RO(rigid);
        SET_MEMB_RO(stack_size);
        SET_MEMB_RO(techniques);
        SET_MEMB_RO(thrown_damage);
        SET_MEMB_RO(volume);
        SET_MEMB_RO(weapon_category);
        SET_MEMB_RO(weight);
        luna::set_fx(ut, "get_countdown_action", [=](const UT_CLASS& c) {
            return get_action(c.countdown_action);
        });
        luna::set_fx(ut, "get_description", [](const UT_CLASS& c, const int num) {
            return c.description.translated(num);
        });
        luna::set_fx(ut, "get_drop_action", [=](const UT_CLASS& c) {
            return get_action(c.drop_action);
        });
        luna::set_fx(ut, "get_uses", get_uses);
    }
#undef UT_CLASS
}


} // namespace cata::detail
#undef VALUE_PTR_MEMB_N
