#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "hsv_color.h"
#include "item.h"
#include "type_id.h"

#include <ranges>
#include <vector>

namespace cata::detail {
auto reg_item_variables_and_damage(sol::usertype<item>& ut) -> void {
#define UT_CLASS item
    DOC("Get variable as string");
    luna::set_fx(
        ut, "get_var_str",
        [](const UT_CLASS& c, const std::string& name, const std::string& def_val) {
            return c.get_var(name, def_val);
        });
    DOC("Get variable as float number");
    luna::set_fx(
        ut, "get_var_num", [](const UT_CLASS& c, const std::string& name, const double& def_val) {
            return c.get_var(name, def_val);
        });
    DOC("Get variable as tripoint");
    luna::set_fx(
        ut, "get_var_tri", [](const UT_CLASS& c, const std::string& name, const tripoint& def_val) {
            return c.get_var(name, def_val);
        });

    luna::
        set_fx(ut, "set_var_str", [](UT_CLASS& c, const std::string& name, const std::string& val) {
            c.set_var(name, val);
        });
    luna::set_fx(ut, "set_var_num", [](UT_CLASS& c, const std::string& name, const double& val) {
        c.set_var(name, val);
    });
    luna::set_fx(ut, "set_var_tri", [](UT_CLASS& c, const std::string& name, const tripoint& val) {
        c.set_var(name, val);
    });
    luna::set_fx(ut, "set_var_col", [](UT_CLASS& c, const std::string& name, const RGBColor& val) {
        c.set_var<RGBColor>(name, val);
    });
    SET_FX(attack_cost);
    SET_FX(stamina_cost);

    // Damage (breakage) related bindings
    DOC("Get current item damage value (durability). Higher values mean more damaged. Default range is -1000 (min) to 4000 (max), configurable via 'damage_states' in JSON.");
    luna::set_fx(ut, "get_damage", &item::damage);

    DOC("Get item damage as a level from 0 to max. Used for UI display and damage thresholds.");
    luna::set_fx(
        ut, "get_damage_level",
        sol::overload([](const item& self) -> int { return self.damage_level(4); },
                      [](const item& self, int max) -> int { return self.damage_level(max); }));

    DOC("Get minimum possible damage value (can be negative for reinforced items). Default is -1000, configurable via 'damage_states' in JSON.");
    luna::set_fx(ut, "get_min_damage", &item::min_damage);

    DOC("Get maximum possible damage value before item is destroyed. Default is 4000, configurable via 'damage_states' in JSON.");
    luna::set_fx(ut, "get_max_damage", &item::max_damage);

    DOC("Get relative health as ratio 0.0-1.0, where 1.0 is undamaged and 0.0 is destroyed");
    luna::set_fx(ut, "get_relative_health", &item::get_relative_health);

    DOC("Set item damage to specified value. Clamped between min_damage and max_damage.");
    luna::set_fx(ut, "set_damage", &item::set_damage);

    DOC("Modify item damage by given amount. Returns true if item should be destroyed.");
    luna::set_fx(
        ut, "mod_damage",
        sol::overload(static_cast<bool (item::*)(int)>(&item::mod_damage),
                      static_cast<bool (item::*)(int, damage_type)>(&item::mod_damage)));

    SET_FX(get_melee_damage_bonus);
    SET_FX(get_melee_hit_bonus);
    SET_FX(get_ranged_damage_bonus);
    SET_FX(get_range_bonus);
    SET_FX(get_dispersion_bonus);
    SET_FX(get_recoil_bonus);

    SET_FX(set_melee_damage_bonus);
    SET_FX(set_melee_hit_bonus);
    SET_FX(set_ranged_damage_bonus);
    SET_FX(set_range_bonus);
    SET_FX(set_dispersion_bonus);
    SET_FX(set_recoil_bonus);

#undef UT_CLASS
}
} // namespace cata::detail
