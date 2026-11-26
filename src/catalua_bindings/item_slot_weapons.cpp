#include "ammo.h"
#include "ammo_effect.h"
#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "fault.h"
#include "flag.h"
#include "gun_mode.h"
#include "itype.h"
#include "martialarts.h"
#include "material.h"
#include "mod_manager.h"
#include "mtype.h"
#include "skill.h"
#include "type_id.h"

#include <iterator>
#include <ranges>
#include <vector>

namespace cata::detail {
auto reg_item_slot_weapons(sol::state& lua) -> void {
#define UT_CLASS islot_mod
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("If non-empty restrict mod to items with those base (before modifiers) ammo types");
        SET_MEMB_RO(acceptable_ammo);

        DOC("If set modifies parent ammo to this type");
        SET_MEMB_RO(ammo_modifier);

        DOC("Proportional adjustment of parent item ammo capacity");
        SET_MEMB_RO(capacity_multiplier);

        DOC("If non-empty replaces the compatible magazines for the parent item");
        SET_MEMB_RO(magazine_adaptor);
    }
#undef UT_CLASS

#define UT_CLASS islot_engine
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("For combustion engines, the displacement");
        SET_MEMB_RO(displacement);
    }
#undef UT_CLASS

#define UT_CLASS islot_wheel
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("Diameter of wheel in millimeters.  Integer JSON values are legacy inches.");
        SET_MEMB_RO(diameter);

        DOC("Width of wheel in millimeters.  Integer JSON values are legacy inches.");
        SET_MEMB_RO(width);
    }
#undef UT_CLASS

#define UT_CLASS islot_fuel
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("Energy of the fuel (kilojoules per charge)");
        SET_MEMB_RO(energy);

        SET_MEMB_RO(explosion_data);
        SET_MEMB_N_RO(has_explode_data, "has_explosion_data");
        SET_MEMB_RO(pump_terrain);
    }
#undef UT_CLASS

#define UT_CLASS islot_gun
    {
        auto ut = luna::new_usertype<
            UT_CLASS>(lua, luna::bases<common_ranged_data>(), luna::no_constructor);

        DOC("What skill this gun uses");
        SET_MEMB_RO(skill_used);

        DOC("What type of ammo this gun uses");
        SET_MEMB_RO(ammo);

        DOC("Gun durability, affects gun being damaged during shooting");
        SET_MEMB_RO(durability);

        DOC("For guns with an integral magazine what is the capacity?");
        SET_MEMB_RO(clip);

        DOC("Reload time, in moves");
        SET_MEMB_RO(reload_time);

        DOC("Noise displayed when reloading the weapon");
        SET_MEMB_RO(reload_noise);

        DOC("Volume of the noise made when reloading this weapon");
        SET_MEMB_RO(reload_noise_volume);

        DOC("Maximum aim achievable using base weapon sights");
        SET_MEMB_RO(sight_dispersion);

        DOC("Modifies base loudness as provided by the currently loaded ammo");
        SET_MEMB_RO(loudness);

        DOC("Modifies projectile speed as provided by the currently loaded ammo");
        SET_MEMB_RO(speed);

        DOC("If this uses UPS charges, how many (per shoot), 0 for no UPS charges at all");
        SET_MEMB_RO(ups_charges);

        DOC("One in X chance for gun to require major cleanup after firing blackpowder shot");
        SET_MEMB_RO(blackpowder_tolerance);

        DOC("Minimum ammo recoil for gun to be able to fire more than once per attack");
        SET_MEMB_RO(min_cycle_recoil);

        DOC("Volume of material removed by sawing down the barrel, if left unspecified barrel can't be sawed down");
        SET_MEMB_RO(barrel_volume);

        DOC("Effects that are applied to the ammo when fired");
        SET_MEMB_RO(ammo_effects);

        DOC("Location for gun mods. Key is the location (untranslated!), value is the number of mods that the location can have. The value should be > 0");
        luna::set_fx(ut, "get_gunmod_locations", [](const UT_CLASS& c) {
            std::map<std::string, int> rv{};
            for (const auto& [k, v] : c.valid_mod_locations) { rv[k.str()] = v; }
            return rv;
        });

        DOC("Built in mods. string is id of mod. These mods will get the IRREMOVABLE flag set");
        SET_MEMB_RO(built_in_mods);

        DOC("Default mods, string is id of mod. These mods are removable but are default on the weapon");
        SET_MEMB_RO(default_mods);

        DOC("Firing modes are supported by the gun. Always contains at least DEFAULT mode");
        luna::set_fx(ut, "get_modes", [](const UT_CLASS& c) {
            std::vector<std::string> rv{};
            std::ranges::copy(
                c.modes | std::views::keys | std::views::transform([](auto& v) { return v.str(); }),
                std::back_inserter(rv));
            return rv;
        });

        DOC("Burst size for AUTO mode (legacy field for items not migrated to specify modes )");
        SET_MEMB_RO(burst);

        DOC("How easy is control of recoil? If unset value automatically derived from weapon type");
        SET_MEMB_RO(handling);

        DOC("Additional recoil applied per shot before effects of handling are considered, useful for adding recoil effect to guns which otherwise consume no ammo");
        SET_MEMB_RO(recoil);

        DOC("How much ammo is consumed per shot");
        SET_MEMB_RO(ammo_to_fire);
    }
#undef UT_CLASS

#define UT_CLASS islot_gunmod
    {
        auto ut = luna::new_usertype<
            UT_CLASS>(lua, luna::bases<common_ranged_data>(), luna::no_constructor);

        DOC("Where is this gunmod installed (e.g. \"stock\", \"rail\")?");
        luna::set_fx(ut, "get_location", [](const UT_CLASS& c) { return c.location.str(); });

        DOC("What kind of weapons this gunmod can be used with");
        SET_MEMB_RO(usable);

        DOC("What category of weapons this gunmod can be used with");
        SET_MEMB_RO(usable_category);

        DOC("What kind of weapons this gunmod can't be used with");
        SET_MEMB_RO(exclusion);

        DOC("What category of weapons this gunmod can't be used with");
        SET_MEMB_RO(exclusion_category);

        DOC("If this value is set (non-negative), this gunmod functions as a sight. A sight is only usable to aim by a character whose current Character::recoil is at or below this value.");
        SET_MEMB_RO(sight_dispersion);

        DOC("For sights (see @ref sight_dispersion), this value affects time cost of aiming.");
        DOC("Higher is better. In case of multiple usable sights,");
        DOC("the one with highest aim speed is used.");
        SET_MEMB_RO(aim_speed);

        DOC("Modifies base loudness as provided by the currently loaded ammo");
        SET_MEMB_RO(loudness);

        DOC("Modifies projectile speed as provided by the currently loaded ammo");
        SET_MEMB_RO(speed);

        DOC("How many moves does this gunmod take to install?");
        SET_MEMB_RO(install_time);

        DOC("Increases base gun UPS consumption by this many times per shot");
        SET_MEMB_RO(ups_charges_multiplier);

        DOC("Increases base gun UPS consumption by this value per shot");
        SET_MEMB_RO(ups_charges_modifier);

        DOC("Increases base gun ammo to fire by this many times per shot");
        SET_MEMB_RO(ammo_to_fire_multiplier);

        DOC("Increases base gun ammo to fire by this value per shot");
        SET_MEMB_RO(ammo_to_fire_modifier);

        DOC("Increases gun weight by this many times");
        SET_MEMB_RO(weight_multiplier);

        DOC("Increases gun volume by this many times");
        SET_MEMB_RO(volume_multiplier);

        DOC("Firing modes added to or replacing those of the base gun");
        luna::set_fx(ut, "get_mode_modifiers", [](const UT_CLASS& c) {
            std::set<std::string> ret{};
            for (const auto& k : c.mode_modifier | std::views::keys) { ret.insert(k.str()); }
            return ret;
        });

        SET_MEMB_RO(ammo_effects);

        DOC("Relative adjustment to base gun handling");
        SET_MEMB_RO(handling);

        DOC("Percentage value change to the gun's loading time. Higher is slower");
        SET_MEMB_RO(reload_modifier);

        DOC("Percentage value change to the gun's loading time. Higher is less likely");
        SET_MEMB_RO(consume_chance);

        DOC("Divsor to scale back gunmod consumption damage. lower is more damaging. Affected by ammo loudness and recoil, see ranged.cpp for how much.");
        SET_MEMB_RO(consume_divisor);

        DOC("Modifies base strength required");
        SET_MEMB_RO(min_str_required_mod);

        DOC("Additional gunmod slots to add to the gun. Key is the location (untranslated!), value is the number of extra mods that location can hold.");
        luna::set_fx(ut, "get_added_slots", [](const UT_CLASS& c) {
            std::map<std::string, int> ret;
            for (const auto& [k, v] : c.add_mod) { ret[k.str()] = v; }
            return ret;
        });

        DOC("Not compatible on weapons that have this mod slot");
        luna::set_fx(ut, "get_mod_blacklist", [](const UT_CLASS& c) {
            std::set<std::string> ret{};
            for (const auto& v : c.blacklist_mod) { ret.insert(v.str()); }
            return ret;
        });
    }
#undef UT_CLASS
}
} // namespace cata::detail
