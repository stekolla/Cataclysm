#include "ammo.h"
#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "disease.h"
#include "itype.h"
#include "martialarts.h"
#include "material.h"
#include "mtype.h"
#include "recipe.h"
#include "skill.h"
#include "translations.h"
#include "type_id.h"
#include "vitamin.h"

#include <ranges>
#include <vector>

namespace cata::detail {
auto reg_item_slot_consumables(sol::state& lua) -> void {
#define UT_CLASS islot_container
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("Inner volume of the container");
        SET_MEMB_RO(contains);

        DOC("Contents do not spoil");
        SET_MEMB_RO(preserves);

        DOC("Can be resealed");
        SET_MEMB_RO(seals);

        DOC("If this is set to anything but \"null\", changing this container's contents in any way will turn this item into that type");
        SET_MEMB_RO(unseals_into);

        DOC("Can hold liquids");
        SET_MEMB_RO(watertight);
    }
#undef UT_CLASS

#define UT_CLASS islot_tool
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        SET_MEMB_RO(charge_factor);
        SET_MEMB_RO(ammo_id);
        SET_MEMB_RO(charges_per_use);
        SET_MEMB_RO(def_charges);
        SET_MEMB_RO(default_ammo);
        SET_MEMB_RO(max_charges);
        SET_MEMB_RO(power_draw);
        SET_MEMB_RO(rand_charges);
        SET_MEMB_RO(revert_msg);
        SET_MEMB_RO(revert_to);
        SET_MEMB_RO(subtype);
        SET_MEMB_RO(turns_active);
        SET_MEMB_RO(turns_per_charge);
        SET_MEMB_RO(ups_eff_mult);
        SET_MEMB_RO(ups_recharge_rate);
    }
#undef UT_CLASS

#define UT_CLASS islot_comestible
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("comestible subtype - eg. FOOD, DRINK, MED");
        SET_MEMB_N_RO(comesttype, "comest_type");

        DOC("tool needed to consume (e.g. lighter for cigarettes)");
        SET_MEMB_RO(tool);

        DOC("Defaults # of charges (drugs, loaf of bread? etc)");
        SET_MEMB_RO(def_charges);

        DOC("effect on character thirst (may be negative)");
        SET_MEMB_RO(quench);

        DOC("Nutrition values to use for this type when they aren't calculated from components");
        luna::set_fx(ut, "get_default_nutrition", [](const UT_CLASS& c) -> auto& {
            return c.default_nutrition.vitamins;
        });

        DOC("Time until becomes rotten at standard temperature, or zero if never spoils");
        SET_MEMB_RO(spoils);

        DOC("addiction potential");
        SET_MEMB_N_RO(addict, "addict_value");

        DOC("effects of addiction");
        SET_MEMB_N_RO(add, "addict_type");

        DOC("stimulant effect");
        SET_MEMB_N_RO(stim, "stimulant_type");

        DOC("fatigue altering effect");
        SET_MEMB_RO(fatigue_mod);

        DOC("Reference to other item that replaces this one as a component in recipe results");
        SET_MEMB_RO(cooks_like);

        DOC("Reference to item that will be received after smoking current item");
        SET_MEMB_RO(smoking_result);

        // DOC("TODO: add documentation");
        SET_MEMB_RO(healthy);

        DOC("chance (odds) of becoming parasitised when eating (zero if never occurs)");
        SET_MEMB_RO(parasites);

        DOC("Amount of radiation you get from this comestible");
        SET_MEMB_RO(radiation);

        DOC("pet food category");
        SET_MEMB_RO(petfood);

        DOC("freezing point in degrees Fahrenheit, below this temperature item can freeze");
        SET_MEMB_RO(freeze_point);

        DOC("List of diseases carried by this comestible and their associated probability");
        SET_MEMB_RO(contamination);

        DOC("specific heats in J/(g K) and latent heat in J/g");
        SET_MEMB_RO(specific_heat_liquid);
        SET_MEMB_RO(specific_heat_solid);
        SET_MEMB_RO(latent_heat);

        DOC("A penalty applied to fun for every time this food has been eaten in the last 48 hours");
        SET_MEMB_RO(monotony_penalty);

        SET_FX(has_calories);

        SET_FX(get_default_nutr);

        DOC("The monster group that is drawn from when the item rots away");
        SET_MEMB_RO(rot_spawn);

        DOC("Chance the above monster group spawns");
        SET_MEMB_RO(rot_spawn_chance);
    }
#undef UT_CLASS

#define UT_CLASS islot_brewable
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("What are the results of fermenting this item");
        SET_MEMB_RO(results);

        DOC("How long for this brew to ferment");
        SET_MEMB_RO(time);
    }
#undef UT_CLASS
}
} // namespace cata::detail
