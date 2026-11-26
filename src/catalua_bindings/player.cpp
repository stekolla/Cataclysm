#include "player.h"

#include "avatar.h"
#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "creature.h"
#include "mission.h"

void cata::detail::reg_player(sol::state& lua) {
    {
        // Note(AluminumAlman): skipping binding members and methods of this class because
        // most of the methods and members are already binded through Character.
        sol::usertype<player> ut = luna::new_usertype<
            player>(lua, luna::bases<Character, Creature>(), luna::no_constructor);
    }
}
void cata::detail::reg_avatar(sol::state& lua) {
#define UT_CLASS avatar
    {
        // Note(AluminumAlman): skipping binding members and methods of this class because
        // most of the methods and members are already binded through Character.
        sol::usertype<avatar> ut = luna::new_usertype<
            avatar>(lua, luna::bases<player, Character, Creature>(), luna::no_constructor);

        SET_FX_T(get_active_missions, std::vector<mission*>() const);
        SET_FX_T(get_completed_missions, std::vector<mission*>() const);
        SET_FX_T(get_failed_missions, std::vector<mission*>() const);
    }
#undef UT_CLASS // #define UT_CLASS npc_opinion
}
