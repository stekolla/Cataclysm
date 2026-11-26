#include "ammo.h"
#include "ammo_effect.h"
#include "bionics.h"
#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "itype.h"
#include "material.h"
#include "mtype.h"
#include "type_id.h"

#include <ranges>
#include <vector>

namespace cata::detail {
auto reg_item_slot_ammo(sol::state& lua) -> void {
#define UT_CLASS islot_magazine
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("Default type of ammo contained by a magazine (often set for ammo belts)");
        SET_MEMB_RO(default_ammo);

        DOC("Capacity of magazine (in equivalent units to ammo charges)");
        SET_MEMB_RO(capacity);

        DOC("Default amount of ammo contained by a magazine (often set for ammo belts)");
        SET_MEMB_RO(count);

        DOC("For ammo belts one linkage (of given type) is dropped for each unit of ammo consumed");
        SET_MEMB_RO(linkage);

        DOC("If false, ammo will cook off if this mag is affected by fire");
        SET_MEMB_RO(protects_contents);

        DOC("How reliable this magazine on a range of 0 to 10?");
        SET_MEMB_RO(reliability);

        DOC("How long it takes to load each unit of ammo into the magazine");
        SET_MEMB_RO(reload_time);

        DOC("What type of ammo this magazine can be loaded with");
        SET_MEMB_N_RO(type, "ammo_type");
    }
#undef UT_CLASS

#define UT_CLASS islot_battery
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("Maximum energy the battery can store");
        SET_MEMB_RO(max_capacity);
    }
#undef UT_CLASS

#define UT_CLASS islot_bionic
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("Id of the bionic");
        SET_MEMB_N_RO(id, "bionic_id");

        DOC("Arbitrary difficulty scale");
        SET_MEMB_RO(difficulty);

        DOC("Item with installation data that can be used to provide almost guaranteed successful install of corresponding bionic");
        SET_MEMB_RO(installation_data);

        DOC("Whether this CBM is an upgrade of another");
        SET_MEMB_RO(is_upgrade);
    }
#undef UT_CLASS

#define UT_CLASS islot_ammo
    {
        auto ut = luna::new_usertype<
            UT_CLASS>(lua, luna::bases<common_ranged_data>(), luna::no_constructor);

        DOC("Default charges");
        SET_MEMB_RO(def_charges);

        DOC("Ammo type, basically the \"form\" of the ammo that fits into the gun/tool");
        SET_MEMB_N_RO(type, "ammo_id");

        SET_MEMB_RO(ammo_effects);

        DOC("Type id of casings, if any");
        SET_MEMB_N_RO(casing, "casing_id");

        DOC("Should this ammo explode in fire?");
        SET_MEMB_RO(cookoff);

        DOC("Chance to fail to recover the ammo used.");
        SET_MEMB_RO(dont_recover_one_in);

        SET_MEMB_RO(drop);
        SET_MEMB_RO(drop_count);
        SET_MEMB_RO(drop_active);

        SET_MEMB_RO(force_stat_display);

        DOC("Base loudness of ammo (possibly modified by gun/gunmods)");
        SET_MEMB_RO(loudness);

        DOC("Base speed of ammo (possibly modified by gun/gunmods)");
        SET_MEMB_RO(speed);

        DOC("Recoil (per shot), roughly equivalent to kinetic energy (in Joules)");
        SET_MEMB_RO(recoil);

        // TODO: shape_factory doesn't expose its type string
        // DOC( "AoE shape or null if it's a projectile" );
        // SET_MEMB_RO( shape );

        DOC("Should this ammo apply a special explosion effect when in fire?");
        SET_MEMB_RO(special_cookoff);
    }
#undef UT_CLASS

#define UT_CLASS islot_seed
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("Name of the plant.");
        luna::set_fx(ut, "get_plant_name", [](const islot_seed& s, int num) {
            return s.plant_name.translated(num);
        });

        DOC("Type id of the fruit item.");
        SET_MEMB_RO(fruit_id);

        DOC("Time it takes for a seed to grow (based of off a season length of 91 days).");
        SET_MEMB_RO(grow);

        DOC("Additionally items (a list of their item ids) that will spawn when harvesting the plant.");
        SET_MEMB_RO(byproducts);

        DOC("Amount of harvested charges of fruits is divided by this number.");
        SET_MEMB_RO(fruit_div);
    }
#undef UT_CLASS
}
} // namespace cata::detail
