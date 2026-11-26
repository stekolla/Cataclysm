#include "bodypart.h"
#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "itype.h"
#include "martialarts.h"
#include "material.h"
#include "recipe.h"
#include "skill.h"
#include "type_id.h"

#include <ranges>
#include <vector>

namespace cata::detail {
auto reg_item_slot_equipment(sol::state& lua) -> void {
#define UT_CLASS islot_armor
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("Layer, encumbrance and coverage information");
        SET_MEMB_N_RO(data, "layer_data");

        DOC("Resistance to environmental effects");
        SET_MEMB_RO(env_resist);

        DOC("Environmental protection of a gas mask with installed filter");
        SET_MEMB_RO(env_resist_w_filter);

        DOC("Damage negated by this armor. Usually calculated from materials+thickness");
        SET_MEMB_RO(resistance);

        DOC("Whether this item can be worn on either side of the body");
        SET_MEMB_RO(sided);

        DOC("How much storage this items provides when worn");
        SET_MEMB_RO(storage);

        DOC("Multiplier on resistances provided by armor's materials. Damaged armors have lower effective thickness, low capped at 1. Note: 1 thickness means item retains full resistance when damaged.");
        SET_MEMB_RO(thickness);

        DOC("Whitelisted clothing mods. Restricted clothing mods must be listed here by id to be compatible.");
        SET_MEMB_RO(valid_mods);

        DOC("How much warmth this item provides");
        SET_MEMB_RO(warmth);

        DOC("Bonus to weight capacity");
        SET_MEMB_RO(weight_capacity_bonus);

        DOC("Factor modifying weight capacity");
        SET_MEMB_RO(weight_capacity_modifier);
    }
#undef UT_CLASS

#define UT_CLASS islot_pet_armor
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("The minimum volume a pet can be and wear this armor");
        SET_MEMB_RO(min_vol);

        DOC("The maximum volume a pet can be and wear this armor");
        SET_MEMB_RO(max_vol);

        DOC("Resistance to environmental effects");
        SET_MEMB_RO(env_resist);

        DOC("Environmental protection of a gas mask with installed filter");
        SET_MEMB_RO(env_resist_w_filter);

        DOC(" How much storage this items provides when worn");
        SET_MEMB_RO(storage);

        DOC("Multiplier on resistances provided by this armor");
        SET_MEMB_RO(thickness);

        DOC("What animal bodytype can wear this armor");
        SET_MEMB_RO(bodytype);
    }
#undef UT_CLASS

#define UT_CLASS islot_book
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("How long in minutes it takes to read. \"To read\" means getting 1 skill point, not all of them.");
        SET_MEMB_RO(time);

        DOC("Fun books have chapters; after all are read, the book is less fun.");
        SET_MEMB_RO(chapters);

        DOC("Which martial art it teaches.  Can be MartialArtsId.NULL_ID");
        SET_MEMB_RO(martial_art);

        DOC("How fun reading this is, can be negative");
        SET_MEMB_RO(fun);

        DOC("Intelligence required to read it");
        SET_MEMB_N_RO(intel, "intelligence");

        DOC("Which skill it upgrades, if any. Can be SkillId.NULL_ID");
        SET_MEMB_RO(skill);

        DOC("The skill level required to understand it");
        SET_MEMB_N_RO(req, "skill_min");

        DOC("The skill level the book provides");
        SET_MEMB_N_RO(level, "skill_max");

        DOC("Recipes contained in this book");
        SET_MEMB_RO(recipes);
    }
#undef UT_CLASS
}
} // namespace cata::detail
