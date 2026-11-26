#include "npc.h"

#include "activity_type.h"
#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "creature.h"
#include "game.h"
#include "monfaction.h"
#include "player.h"
#include "type_id.h"

void cata::detail::reg_npc(sol::state& lua) {
#define UT_CLASS npc
    {
        sol::usertype<UT_CLASS> ut = luna::new_usertype<
            UT_CLASS>(lua, luna::bases<player, Character, Creature>(), luna::no_constructor);

        // Members
        SET_MEMB(current_activity_id);
        SET_MEMB(personality);
        SET_MEMB(op_of_u);
        SET_MEMB(patience);
        SET_MEMB(marked_for_death);
        SET_MEMB(hit_by_player);
        SET_MEMB(needs);

        // Methods
        SET_FX_N_T(set_fac, "set_faction_id", void(const faction_id& id));

        DOC("True if this NPC is in a simulated (fully loaded, AI-eligible) submap.");
        SET_FX_T(is_simulated, bool() const);

        SET_FX_T(turned_hostile, bool() const);

        SET_FX_T(hostile_anger_level, int() const);

        SET_FX_T(make_angry, void());

        SET_FX_T(is_enemy, bool() const);

        SET_FX_T(is_following, bool() const);

        SET_FX_T(is_obeying, bool(const Character& p) const);

        SET_FX_T(is_friendly, bool(const Character& p) const);

        SET_FX_T(is_leader, bool() const);

        SET_FX_T(is_walking_with, bool() const);

        SET_FX_T(is_ally, bool(const Character& p) const);

        SET_FX_T(is_player_ally, bool() const);

        SET_FX_T(is_stationary, bool(bool include_guards) const);

        SET_FX_T(is_guarding, bool() const);

        SET_FX_T(is_patrolling, bool() const);

        SET_FX_T(has_player_activity, bool() const);

        DOC("Returns true if the npc has the 'NPC_MISSION_TRAVELLING' mission.");

        SET_FX_T(is_travelling, bool() const);

        DOC("Returns true if the npc is a player ally and their op_of_u.trust is >= 5.");

        SET_FX_T(is_minion, bool() const);

        DOC("Returns true if the npc would be hostile to the player on sight regardless of their current attitude.");

        SET_FX_T(guaranteed_hostile, bool() const);

        DOC("Causes the npc to leave your faction, incur various negative opinion maluses, and say explatives to the player.");

        SET_FX_T(mutiny, void());

        DOC("Returns the npc's faction id. If it's part of a monster faction, it returns the id of the monster faction.");

        SET_FX_T(get_monster_faction, mfaction_id() const);

        DOC("Gets the npc's follow distance. This can only be a few values, and is determined by npc rules.");

        SET_FX_T(follow_distance, int() const);

        DOC("Returns the npc's current target as a creature, if it has one.");

        SET_FX_T(current_target, Creature * ());

        DOC("Returns the npc's current ally, if it has one.");

        SET_FX_T(current_ally, Creature * ());

        DOC("Gets a value based on the npc's perspective of the area's danger.");

        SET_FX_T(danger_assessment, float());

        DOC("Gets the npc's blunt damage with their currently equipped weapon.");

        SET_FX_T(smash_ability, int() const);

        DOC("Returns the first topic of the npc's chatbin.");

        luna::set_fx(ut, "get_first_topic", [](UT_CLASS& npchar) -> std::string {
            return npchar.chatbin.first_topic;
        });

        DOC("Sets the first topic of the npc's chatbin. Note that some circumstances may cause this to not be the first topic used, such as player allies.");

        luna::set_fx(ut, "set_first_topic", [](UT_CLASS& npchar, const std::string& str) -> void {
            npchar.chatbin.first_topic = str;
        });

        DOC("Triggers the npc menu to open.");
        luna::set_fx(ut, "npc_menu", [](UT_CLASS& npchar, sol::optional<bool> force) -> void {
            g->npc_menu(npchar, force.value_or(false));
        });

        DOC("Causes the npc to talk to you. Passing a topic will force that topic to be used in place of all others, including code enforced topics such as 'TALK_STOLE_ITEM'.");
        luna::set_fx(
            ut, "talk_to_u",
            [](UT_CLASS& npchar, sol::optional<std::string> topic,
               sol::optional<bool> radio_contact) -> void {
                if (topic.has_value() && !topic.value().empty()) {
                    npchar.chatbin.first_topic = topic.value();
                }
                npchar.talk_to_u(radio_contact.value_or(false), topic.has_value());
            });

        DOC("Has the npc say the given string in the sidebar.");

        luna::set_fx(ut, "say", &UT_CLASS::say<>);

        DOC("Complain about an issue if enough time has passed. The first string is the issue identifier, with the second being the complaint text.");
        DOC("The optional bool allows you to force a complaint without concern for the input time.");

        luna::set_fx(
            ut, "complain_about",
            [](UT_CLASS& npchar, const std::string& issue, const time_duration& dur,
               const std::string& speech, sol::optional<bool> force) -> bool {
                return npchar.complain_about(issue, dur, speech, force.value_or(false));
            });

        DOC("Wrapper for complain_about that warns about a specific type of threat, with");

        DOC("different warnings for hostile or friendly NPCs and hostile NPCs always complaining.");

        SET_FX_T(warn_about,
                 void(const std::string& type, const time_duration&, const std::string&, int,
                      const tripoint_bub_ms&));

        DOC("Finds something to complain about and complains. Returns if complained.");

        SET_FX_T(complain, bool());

        DOC("Rates how dangerous a target is from 0 (harmless) to 1 (max danger).");

        SET_FX_T(evaluate_enemy, float(const Creature&) const);

        SET_FX_T(can_open_door, bool(const tripoint_bub_ms&, bool) const);

        SET_FX_T(can_move_to, bool(const tripoint_bub_ms&, bool) const);

        DOC("Attempts to move the NPC to an adjacent map square.");
        luna::set_fx(
            ut, "move_to",
            [](UT_CLASS& npchar, const tripoint_bub_ms& pos, sol::optional<bool> no_bashing)
                -> void { npchar.move_to(pos, no_bashing.value_or(false), nullptr); });
        luna::set_fx(
            ut, "set_move_target",
            sol::overload(
                [](npc& npchar, const tripoint_bub_ms& p, sol::optional<bool> no_bashing,
                   sol::optional<bool> force) -> bool {
                    return npchar.update_path(p, no_bashing.value_or(false), force.value_or(true));
                },
                [](npc& npchar, const tripoint& p, sol::optional<bool> no_bashing,
                   sol::optional<bool> force) -> bool {
                    return npchar.update_path(
                        tripoint_bub_ms(p), no_bashing.value_or(false), force.value_or(true));
                }));

        SET_FX_T(saw_player_recently, bool() const);

        SET_FX_T(has_omt_destination, bool() const);

        SET_FX_T(get_attitude, npc_attitude() const);

        SET_FX_T(set_attitude, void(npc_attitude new_attitude));

        SET_FX_T(has_activity, bool() const);
    }
#undef UT_CLASS // #define UT_CLASS npc

#define UT_CLASS npc_personality
    {
        sol::usertype<UT_CLASS> ut =
            luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::constructors<UT_CLASS()>());
        SET_MEMB(aggression);
        SET_MEMB(bravery);
        SET_MEMB(collector);
        SET_MEMB(altruism);
    }
#undef UT_CLASS // #define UT_CLASS npc_personality

#define UT_CLASS npc_opinion
    {
        sol::usertype<UT_CLASS> ut = luna::new_usertype<UT_CLASS>(
            lua, luna::no_bases,
            luna::constructors<UT_CLASS(), UT_CLASS(int, int, int, int, int)>());
        SET_MEMB(trust);
        SET_MEMB(fear);
        SET_MEMB(value);
        SET_MEMB(anger);
        SET_MEMB(owed);
    }
#undef UT_CLASS // #define UT_CLASS npc_opinion
}
