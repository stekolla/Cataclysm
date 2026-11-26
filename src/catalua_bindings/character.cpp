#include "character.h"

#include "catalua_bindings.h"
#include "catalua_bindings/character_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "creature.h"
#include "type_id.h"

void cata::detail::reg_character(sol::state& lua) {
#define UT_CLASS Character
    {
        sol::usertype<UT_CLASS> ut =
            luna::new_usertype<UT_CLASS>(lua, luna::bases<Creature>(), luna::no_constructor);

        reg_character_core(ut);
        reg_character_mutations(ut);
        reg_character_bionics_and_equipment(ut);
        reg_character_items_and_activity(ut);
        reg_character_world_and_crafting(ut);
    }
#undef UT_CLASS // #define UT_CLASS Character

#define UT_CLASS character_id
    {
        sol::usertype<UT_CLASS> ut = luna::new_usertype<
            UT_CLASS>(lua, luna::no_bases, luna::constructors<UT_CLASS(), UT_CLASS(int)>());

        SET_FX(is_valid);
        SET_FX(get_value);
    }
#undef UT_CLASS // #define UT_CLASS character_id
}
