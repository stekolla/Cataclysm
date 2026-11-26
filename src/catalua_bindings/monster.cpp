#include "monster.h"

#include "bionics.h"
#include "bodypart.h"
#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "creature.h"
#include "damage.h"
#include "enums.h"
#include "flag.h"
#include "flag_trait.h"
#include "game.h"
#include "inventory.h"
#include "map/map.h"
#include "monfaction.h"
#include "mtype.h"
#include "npc.h"
#include "player.h"
#include "type_id.h"

void cata::detail::reg_monster(sol::state& lua) {
#define UT_CLASS monster
    {
        sol::usertype<UT_CLASS> ut =
            luna::new_usertype<UT_CLASS>(lua, luna::bases<Creature>(), luna::no_constructor);

        // Members
        SET_MEMB(friendly);
        SET_MEMB(anger);
        SET_MEMB(morale);
        SET_MEMB(faction);
        SET_MEMB(death_drops);
        SET_MEMB(unique_name);
        DOC("Pet training level gained through pet training.");
        SET_MEMB_RO(training_level);
        DOC("Bond level with the character this pet is bonded to.");
        SET_MEMB_RO(pet_bond_level);

        // Methods
        // I really don't want to break the uniformity, but...
        luna::set_fx(ut, "get_type", [](const monster& m) { return m.type->id; });
        SET_FX_T(can_upgrade, bool() const);
        SET_FX_T(hasten_upgrade, void());
        SET_FX_T(get_upgrade_time, int() const);
        SET_FX_T(try_upgrade, void(bool));
        SET_FX_T(try_reproduce, void());
        SET_FX_T(refill_udders, void());
        DOC("Whether this monster's current type and runtime state contain the exact attack ID.");
        SET_FX_T(has_special_attack, bool(const std::string&) const);
        DOC("Checks only enabled state and zero cooldown; does not predict range, target, sight or ammo.");
        SET_FX_T(special_attack_ready, bool(const std::string&) const);
        DOC("Calls a ready special attack once. True means the actor handled use, not necessarily a hit or shot.");
        DOC("Resets cooldown only on true. Actor side effects are not rolled back on false.");
        DOC("Does not apply the scheduler's pacified/hallucination restrictions or plan a target.");
        DOC("On true the stock scheduler skips its own pick for the rest of this action.");
        SET_FX_T(use_special_attack, bool(const std::string&));
        DOC("Whether this action's one special attack has already been spent.");
        SET_FX_N_T(special_attack_budget_spent, "special_attack_budget_spent", bool() const);
        DOC("Frees the budget so a further use_special_attack can run in the same action.");
        DOC("Only for a deliberately multi-attack action; the budget is what stops Lua and");
        DOC("the stock scheduler from stacking attacks onto one action.");
        SET_FX_N_T(clear_special_attack_budget, "clear_special_attack_budget", void());
        DOC("Sorted IDs of every special attack this monster's current type defines.");
        SET_FX_N_T(special_attack_ids, "get_special_attack_ids", std::vector<std::string>() const);
        DOC("Whether the attack is enabled. False for attacks this monster does not have.");
        SET_FX_N_T(special_attack_enabled, "special_attack_enabled", bool(const std::string&) const);
        DOC("Enables or disables an attack without touching its cooldown.");
        DOC("The enabled flag is serialized, so it persists across save/load.");
        DOC("A disabled attack is skipped by the stock scheduler and by special_attack_ready.");
        SET_FX_N_T(set_special_attack_enabled, "set_special_attack_enabled",
                   void(const std::string&, bool));
        DOC("Remaining cooldown in turns, or nil for attacks this monster does not have.");
        SET_FX_N_T(get_special_attack_cooldown, "get_special_attack_cooldown",
                   std::optional<int>(const std::string&) const);
        DOC("Sets the remaining cooldown in turns. Negative values are clamped to 0.");
        DOC("Does nothing for attacks this monster does not have.");
        SET_FX_N_T(set_special_attack_cooldown, "set_special_attack_cooldown",
                   void(const std::string&, int));
        SET_FX_T(spawn, void(const tripoint_bub_ms&));

        SET_FX_T(name, std::string(unsigned int) const);
        SET_FX_T(name_with_armor, std::string() const);

        SET_FX_T(can_see, bool() const);
        SET_FX_T(can_hear, bool() const);
        SET_FX_T(can_submerge, bool() const);
        SET_FX_T(can_drown, bool() const);
        SET_FX_T(can_climb, bool() const);
        SET_FX_T(can_dig, bool() const);
        SET_FX_T(digs, bool() const);
        SET_FX_T(flies, bool() const);
        SET_FX_T(climbs, bool() const);
        SET_FX_T(swims, bool() const);
        SET_FX_T(made_of, bool(const material_id&) const);

        SET_FX_T(move_target, tripoint_bub_ms());
        SET_FX_N_T(is_wandering, "is_wandering", bool() const);
        DOC("Hostile creature this monster sees at its move target, or nil while wandering.");
        SET_FX_T(attack_target, Creature * ());
        DOC("Whether this monster flees from the given character.");
        SET_FX_T(is_fleeing, bool(Character&) const);
        DOC("Whether this monster has moves left and is not stunned, downed or webbed.");
        SET_FX_T(can_act, bool() const);
        DOC("Whether this monster has an effect that impairs movement.");
        SET_FX_T(movement_impaired, bool());
        DOC("Intensity of the grabbed effect this monster applies.");
        SET_FX_T(get_grab_strength, int() const);
        DOC("Whether this monster is dead or at 0 HP. Unlike is_dead, which checks only HP,");
        DOC("this also includes monsters that already died with HP left, e.g. by self-destructing.");
        SET_FX_N_T(is_dead, "is_dead_or_dying", bool() const);

        SET_FX_T(wander_to, void(const tripoint_bub_ms& p, int f));
        luna::set_fx(ut, "add_armor_item", [](monster& m, detached_ptr<item>& armor) {
            return m.set_armor_item(std::move(armor));
        });
        luna::set_fx(ut, "get_armor_item", [](monster& m) { return m.get_armor_item(); });
        luna::set_fx(ut, "remove_armor_item", [](monster& m) { return m.remove_armor_item(); });

        luna::set_fx(ut, "add_saddle_item", [](monster& m, detached_ptr<item>& saddle) {
            return m.set_tack_item(std::move(saddle));
        });
        luna::set_fx(ut, "get_saddle_item", [](monster& m) { return m.get_tack_item(); });
        luna::set_fx(ut, "remove_saddle_item", [](monster& m) { return m.remove_tack_item(); });

        luna::set_fx(ut, "add_storage_item", [](monster& m, detached_ptr<item>& storage) {
            return m.set_storage_item(std::move(storage));
        });
        luna::set_fx(ut, "get_storage_item", [](monster& m) { return m.get_storage_item(); });
        luna::set_fx(ut, "remove_storage_item", [](monster& m) { return m.remove_storage_item(); });

        luna::set_fx(ut, "add_battery_item", [](monster& m, detached_ptr<item>& storage) {
            return m.set_battery_item(std::move(storage));
        });
        luna::set_fx(ut, "get_battery_item", [](monster& m) { return m.get_battery_item(); });
        luna::set_fx(ut, "remove_battery_item", [](monster& m) { return m.remove_battery_item(); });

        luna::set_fx(
            ut, "set_move_target",
            sol::overload(
                [](monster& mon, const tripoint_bub_ms& p) -> void { mon.set_dest(p); },
                [](monster& mon, const tripoint& p) -> void { mon.set_dest(tripoint_bub_ms(p)); }));
        luna::set_fx(ut, "set_target", [](monster& mon, Creature* target) -> void {
            if (target == nullptr) {
                mon.unset_dest();
                return;
            }
            mon.set_dest(target->bub_pos());
        });
        luna::set_fx(ut, "clear_move_target", [](monster& mon) -> void { mon.unset_dest(); });
        luna::set_fx(ut, "run_normal_ai_turn", [](monster& mon) -> void {
            mon.plan();
            const auto action = mon.decide_action();
            mon.execute_action(action);
        });
        luna::set_fx(
            ut, "move_to",
            sol::overload(
                [](monster& mon, const tripoint_bub_ms& p, bool force, bool step_on_critter,
                   float stagger_adjustment) -> bool {
                    return mon.move_to(p, force, step_on_critter, stagger_adjustment);
                },
                [](monster& mon, const tripoint& p, bool force, bool step_on_critter,
                   float stagger_adjustment) -> bool {
                    return mon
                        .move_to(tripoint_bub_ms(p), force, step_on_critter, stagger_adjustment);
                }));
        luna::set_fx(
            ut, "bash_at",
            sol::overload(
                [](monster& mon, const tripoint_bub_ms& p) -> bool { return mon.bash_at(p); },
                [](monster& mon, const tripoint& p) -> bool {
                    return mon.bash_at(tripoint_bub_ms(p));
                }));

        SET_FX_T(attitude, monster_attitude(const Character*) const);
        luna::set_fx(ut, "set_attitude", [](monster& mon, monster_attitude att) -> void {
            static const auto effect_docile = efftype_id("docile");
            static const auto effect_pacified = efftype_id("pacified");
            switch (att) {
                case MATT_FLEE:
                    mon.anger = 0;
                    mon.morale = -100;
                    break;
                case MATT_ATTACK:
                    mon.anger = 100;
                    mon.morale = 100;
                    mon.remove_effect(effect_docile);
                    mon.remove_effect(effect_pacified);
                    break;
                case MATT_IGNORE:
                    mon.anger = 0;
                    mon.morale = 0;
                    mon.remove_effect(effect_docile);
                    mon.remove_effect(effect_pacified);
                    break;
                case MATT_FOLLOW:
                    mon.anger = 5;
                    mon.morale = -5;
                    mon.remove_effect(effect_docile);
                    mon.remove_effect(effect_pacified);
                    break;
                case MATT_FRIEND:
                    mon.make_friendly();
                    mon.remove_effect(effect_docile);
                    mon.remove_effect(effect_pacified);
                    break;
                case MATT_FPASSIVE:
                    mon.make_friendly();
                    mon.add_effect(effect_docile, 1_turns);
                    break;
                case MATT_ZLAVE:
                    mon.add_effect(effect_pacified, 1_turns);
                    break;
                default:
                    break;
            }
        });

        SET_FX_T(heal, int(int, bool));

        SET_FX_T(set_hp, void(int));

        SET_FX_T(make_fungus, bool());
        SET_FX_T(make_friendly, void());
        SET_FX_T(make_pet, void());

        SET_FX_T(make_ally, void(const monster&));

        SET_FX_T(get_items, const std::vector<item*>&() const);
        luna::set_fx(ut, "add_item", [](monster& m, item* it) {
            if (it == nullptr) { return; }
            detached_ptr<item> ptr = item::spawn(*it);
            m.add_item(std::move(ptr));
        });
        luna::set_fx(ut, "add_detached_item", [](monster& m, detached_ptr<item>& it) {
            if (!it) { return; }
            m.add_item(std::move(it));
        });
        SET_FX_T(remove_item, detached_ptr<item>(item*));
        SET_FX_T(clear_items, std::vector<detached_ptr<item>>());
        SET_FX_T(drop_items, void(const tripoint_bub_ms&));
        SET_FX_N_T(drop_items, "drop_items_here", void());

        luna::set_fx(
            ut, "add_faction_anger", [](monster& m, const std::string& faction_str, int amount) {
                m.add_faction_anger(mfaction_id(faction_str), amount);
            });

        luna::set_fx(
            ut, "get_faction_anger", [](const monster& m, const std::string& faction_str) -> int {
                return m.get_faction_anger(mfaction_id(faction_str));
            });
    }
#undef UT_CLASS // #define UT_CLASS monster
}
