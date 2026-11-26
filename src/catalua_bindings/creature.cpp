#include "creature.h"

#include "avatar.h"
#include "bodypart.h"
#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "damage.h"
#include "enums.h"
#include "flag_trait.h"
#include "monster.h"
#include "npc.h"
#include "type_id.h"

void cata::detail::reg_creature(sol::state& lua) {
#define UT_CLASS Creature
    {
        sol::usertype<UT_CLASS> ut =
            luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        // Methods
        SET_FX_T(get_name, std::string() const);
        DOC_PARAMS("possessive", "capitalize_first");
        SET_FX_T(disp_name, std::string(bool, bool) const);
        SET_FX_T(skin_name, std::string() const);
        SET_FX_T(get_grammatical_genders, std::vector<std::string>() const);

        SET_FX_T(is_avatar, bool() const);
        SET_FX_T(is_npc, bool() const);
        SET_FX_T(is_monster, bool() const);
        SET_FX_T(as_monster, monster * ());
        SET_FX_T(as_npc, npc * ());
        SET_FX_T(as_character, Character * ());
        SET_FX_T(as_avatar, avatar * ());

        SET_FX_T(dodge_roll, float());
        SET_FX_T(stability_roll, float() const);

        SET_FX_T(attitude_to, Attitude(const Creature&) const);

        luna::set_fx(ut, "sees", [](const Creature& cr, const tripoint_bub_ms& t) -> bool {
            return cr.sees(t);
        });

        SET_FX_T(sight_range, int(int) const);

        SET_FX_T(power_rating, float() const);

        SET_FX_T(speed_rating, float() const);

        SET_FX_T(ranged_target_size, double() const);

        SET_FX_T(knock_back_to, void(const tripoint_bub_ms&));

        SET_FX_T(
            deal_damage,
            dealt_damage_instance(Creature * source, bodypart_id bp, const damage_instance& dam));

        SET_FX_T(apply_damage,
                 void(Creature * source, bodypart_id bp, int amount, bool bypass_med));

        SET_FX_T(size_melee_penalty, int() const);

        SET_FX_T(digging, bool() const);
        SET_FX_T(is_on_ground, bool() const);
        SET_FX_T(is_underwater, bool() const);
        SET_FX_T(set_underwater, void(bool x));
        SET_FX_T(is_warm, bool() const);
        SET_FX_T(in_species, bool(const species_id&) const);

        SET_FX_T(has_weapon, bool() const);
        SET_FX_T(is_hallucination, bool() const);
        SET_FX_N_T(is_dead_state, "is_dead", bool() const);

        SET_FX_T(is_elec_immune, bool() const);
        SET_FX_T(is_immune_effect, bool(const efftype_id&) const);
        SET_FX_T(is_immune_damage, bool(damage_type) const);

        SET_FX_N_T(bub_pos, "get_pos_ms", tripoint_bub_ms() const);

        SET_FX_N_T(bub_pos, "bub_pos", tripoint_bub_ms() const);

        SET_FX_N_T(abs_pos, "abs_pos", tripoint_abs_ms() const);

        SET_FX_N_T(setpos, "set_pos_ms", void(const tripoint_bub_ms&));

        luna::set_fx(
            ut, "set_pos",
            sol::overload([](Creature& cr, const tripoint_bub_ms& p) -> void { cr.setpos(p); },
                          [](Creature& cr, const tripoint_abs_ms& p) -> void { cr.setpos(p); }));

        luna::set_fx(
            ut, "has_effect",
            [](const Creature& cr, const efftype_id& eff,
               sol::optional<const bodypart_str_id&> bpid) -> bool {
                if (bpid.has_value()) { return cr.has_effect(eff, *bpid); }
                return cr.has_effect(eff);
            });

        luna::set_fx(
            ut, "get_effect",
            [](Creature& cr, const efftype_id& eff,
               sol::optional<const bodypart_str_id&> bpid) -> effect& {
                if (bpid.has_value()) {
                    return cr.get_effect(eff, *bpid);
                } else {
                    return cr.get_effect(eff);
                }
            });

        luna::set_fx(
            ut, "has_effect_with_flag",
            [](const Creature& cr, const flag_id& flag,
               sol::optional<const bodypart_str_id&> bpid) -> bool {
                const bodypart_str_id& bp = bpid ? *bpid : bodypart_str_id::NULL_ID();
                return cr.has_effect_with_flag(flag, bp);
            });

        luna::set_fx(
            ut, "get_effect_dur",
            [](const Creature& cr, const efftype_id& eff,
               sol::optional<const bodypart_str_id&> bpid) -> time_duration {
                const bodypart_str_id& bp = bpid ? *bpid : bodypart_str_id::NULL_ID();
                return cr.get_effect_dur(eff, bp);
            });

        luna::set_fx(
            ut, "get_effect_int",
            [](const Creature& cr, const efftype_id& eff,
               sol::optional<const bodypart_str_id&> bpid) -> int {
                const bodypart_str_id& bp = bpid ? *bpid : bodypart_str_id::NULL_ID();
                return cr.get_effect_int(eff, bp);
            });

        DOC("Effect type, duration, bodypart and intensity");
        luna::set_fx(
            ut, "add_effect",
            [](Creature& cr, const efftype_id& eff, const time_duration& dur,
               sol::optional<const bodypart_str_id&> bpid, sol::optional<int> intensity) {
                int eint = intensity ? *intensity : 0;
                const bodypart_str_id& bp = bpid ? *bpid : bodypart_str_id::NULL_ID();
                cr.add_effect(eff, dur, bp, eint);
            });

        luna::set_fx(
            ut, "remove_effect",
            [](Creature& cr, const efftype_id& eff,
               sol::optional<const bodypart_str_id&> bpid) -> bool {
                const bodypart_str_id& bp = bpid ? *bpid : bodypart_str_id::NULL_ID();
                return cr.remove_effect(eff, bp);
            });

        SET_FX_T(clear_effects, void());

        DOC("Sets an arbitrary key : value pair for the creature.NPC dialogue system uses this, with the format(\"npctalk_var\" + \"_\" + type_var + \"_\" + var_context + \"_\" + var_base_name) used for the key, skipping type or context if empty.");
        SET_FX_T(set_value, void(const std::string&, const std::string&));
        DOC("Removes an arbitrary entry using the same key format as set_value.");
        SET_FX_T(remove_value, void(const std::string&));
        DOC("Retrieves an arbitrary entry using the same key format as set_value.");
        luna::set_fx(ut, "get_value", [](const Creature& cr, const std::string& key) {
            return cr.get_value(key);
        });
        DOC("Returns all stored creature vars as a table mapping each key (string) to its value (string), as written by set_value.");
        luna::set_fx(ut, "values_table", [](const Creature& cr, sol::this_state state) {
            sol::state_view lua(state);
            sol::table vars = lua.create_table();
            std::ranges::for_each(cr.get_values_map(), [&](const auto& entry) {
                vars[entry.first] = entry.second;
            });
            return vars;
        });

        DOC("Removes this creature from the game without death notifications or a corpse.");
        SET_FX_T(erase, void());

        SET_FX_T(get_weight, units::mass() const);

        SET_FX_T(has_trait, bool(const trait_id&) const);

        SET_FX_T(mod_pain, void(int));
        SET_FX_T(mod_pain_noresist, void(int));
        SET_FX_T(set_pain, void(int));
        SET_FX_T(get_pain, int() const);
        SET_FX_T(get_perceived_pain, int() const);

        SET_FX_T(get_moves, int() const);
        SET_FX_T(mod_moves, void(int));
        SET_FX_T(set_moves, void(int));

        SET_FX_T(get_num_blocks, int() const);
        SET_FX_T(get_num_dodges, int() const);

        SET_FX_T(get_env_resist, int(bodypart_id) const);

        SET_FX_T(get_armor_bash, int(bodypart_id) const);
        SET_FX_T(get_armor_cut, int(bodypart_id) const);
        SET_FX_T(get_armor_bullet, int(bodypart_id) const);
        SET_FX_T(get_armor_bash_base, int(bodypart_id) const);
        SET_FX_T(get_armor_cut_base, int(bodypart_id) const);
        SET_FX_T(get_armor_bullet_base, int(bodypart_id) const);
        SET_FX_T(get_armor_bash_bonus, int() const);
        SET_FX_T(get_armor_cut_bonus, int() const);
        SET_FX_T(get_armor_bullet_bonus, int() const);

        SET_FX_T(get_armor_type, int(damage_type, bodypart_id) const);

        SET_FX_T(get_dodge, float() const);

        SET_FX_T(get_melee, float() const);
        SET_FX_T(get_hit, float() const);

        SET_FX_T(get_speed, int() const);
        SET_FX_T(get_size, creature_size() const);
        luna::set_fx(
            ut, "get_hp", [](const Creature& cr, sol::optional<const bodypart_id&> bpid) -> int {
                if (bpid.has_value()) {
                    return cr.get_hp(*bpid);
                } else {
                    return cr.get_hp();
                }
            });
        luna::set_fx(
            ut, "get_hp_max",
            [](const Creature& cr, sol::optional<const bodypart_id&> bpid) -> int {
                if (bpid.has_value()) {
                    return cr.get_hp_max(*bpid);
                } else {
                    return cr.get_hp_max();
                }
            });

        SET_FX_T(hp_percentage, int() const);
        SET_FX_T(has_flag, bool(const m_flag) const);

        SET_FX_T(get_part_hp_cur, int(const bodypart_id&) const);
        SET_FX_T(get_part_hp_max, int(const bodypart_id&) const);

        SET_FX_T(get_part_healed_total, int(const bodypart_id&) const);

        SET_FX_T(set_part_hp_cur, void(const bodypart_id&, int));
        SET_FX_T(set_part_hp_max, void(const bodypart_id&, int));
        SET_FX_T(mod_part_hp_cur, void(const bodypart_id&, int));
        SET_FX_T(mod_part_hp_max, void(const bodypart_id&, int));

        SET_FX_T(set_all_parts_hp_cur, void(int));
        SET_FX_T(mod_all_parts_hp_cur, void(int));
        SET_FX_T(set_all_parts_hp_to_max, void());

        SET_FX_T(get_random_body_part, bodypart_id(bool) const);
        SET_FX_T(get_all_body_parts, std::vector<bodypart_id>(bool) const);

        SET_FX_T(set_armor_bash_bonus, void(int));
        SET_FX_T(set_armor_cut_bonus, void(int));
        SET_FX_T(set_armor_bullet_bonus, void(int));

        SET_FX_T(get_speed_base, int() const);
        SET_FX_T(get_speed_bonus, int() const);
        SET_FX_T(get_speed_mult, float() const);
        SET_FX_T(get_block_bonus, int() const);

        SET_FX_T(get_dodge_base, float() const);
        SET_FX_T(get_hit_base, float() const);
        SET_FX_T(get_dodge_bonus, float() const);
        SET_FX_T(get_hit_bonus, float() const);

        SET_FX_T(has_grab_break_tec, bool() const);

        luna::set_fx(ut, "get_weight_capacity", [](UT_CLASS& cr) -> std::int64_t {
            return cr.weight_capacity().value();
        });
    }
#undef UT_CLASS // #define UT_CLASS Creature
}
