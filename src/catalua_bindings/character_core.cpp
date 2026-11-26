#include "bodypart.h"
#include "catalua_bindings/character_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "creature.h"
#include "faction.h"
#include "type_id.h"
#include "units_mass.h"
#include "units_temperature.h"
#include "units_volume.h"

namespace cata::detail {
auto reg_character_core(sol::usertype<Character>& ut) -> void {
#define UT_CLASS Character
    // Members
    SET_MEMB(name);
    SET_MEMB(male);

    SET_MEMB(focus_pool);
    SET_MEMB(cash);
    SET_MEMB(follower_ids);

    DOC("Strength of each mutation category, computed from the character's mutations and their prerequisites. Categories without mutations are absent.");
    SET_MEMB(mutation_category_level);

    // Magic system
    DOC("Access the character's spellbook and mana pool.");
    luna::set_fx(ut, "get_magic", [](UT_CLASS& c) -> known_magic& { return *c.magic; });

    // Methods
    SET_FX_T(getID, character_id() const);

    SET_FX_T(setID, void(character_id, bool));

    SET_FX_T(reset, void());
    SET_FX_T(reset_encumbrance, void());

    SET_FX_T(get_str, int() const);
    SET_FX_T(get_dex, int() const);
    SET_FX_T(get_per, int() const);
    SET_FX_T(get_int, int() const);

    SET_FX_T(get_str_base, int() const);
    SET_FX_T(get_dex_base, int() const);
    SET_FX_T(get_per_base, int() const);
    SET_FX_T(get_int_base, int() const);

    SET_FX_T(get_str_bonus, int() const);
    SET_FX_T(get_dex_bonus, int() const);
    SET_FX_T(get_per_bonus, int() const);
    SET_FX_T(get_int_bonus, int() const);

    SET_FX_T(set_str_bonus, void(int));
    SET_FX_T(set_dex_bonus, void(int));
    SET_FX_T(set_per_bonus, void(int));
    SET_FX_T(set_int_bonus, void(int));
    SET_FX_T(mod_str_bonus, void(int));
    SET_FX_T(mod_dex_bonus, void(int));
    SET_FX_T(mod_per_bonus, void(int));
    SET_FX_T(mod_int_bonus, void(int));
    SET_FX_T(mod_speed_bonus, void(int));
    SET_FX_T(set_speed_bonus, void(int));

    SET_FX_T(get_healthy, float() const);
    SET_FX_T(get_healthy_mod, float() const);

    SET_FX_T(mod_healthy, void(float));
    SET_FX_T(mod_healthy_mod, void(float, float));

    SET_FX_T(set_healthy, void(float));
    SET_FX_T(set_healthy_mod, void(float));

    SET_FX_T(get_stored_kcal, int() const);

    SET_FX_T(max_stored_kcal, int() const);
    SET_FX_T(get_kcal_percent, float() const);
    SET_FX_T(get_thirst, int() const);
    SET_FX_T(get_fatigue, int() const);
    SET_FX_T(get_sleep_deprivation, int() const);

    SET_FX_T(mod_stored_kcal, void(int));
    SET_FX_T(mod_thirst, void(int));
    SET_FX_T(mod_fatigue, void(int));
    SET_FX_T(mod_sleep_deprivation, void(int));

    SET_FX_T(set_stored_kcal, void(int));
    SET_FX_T(set_thirst, void(int));
    SET_FX_T(set_fatigue, void(int));
    SET_FX_T(set_sleep_deprivation, void(int));

    luna::set_fx(ut, "get_faction_id", [](const UT_CLASS& charac) -> faction_id {
        faction* fac = charac.get_faction();
        return fac == nullptr ? faction_id::NULL_ID() : fac->id;
    });
    luna::set_fx(ut, "set_faction_id", [](UT_CLASS& charac, faction_id id) {
        charac.set_fac_id(id.str());
    });

    SET_FX_T(sight_impaired, bool() const);

    SET_FX_T(has_alarm_clock, bool() const);

    SET_FX_T(has_watch, bool() const);

    DOC("Gets the current temperature of a specific body part in Celsius.");
    SET_FX_N_T(get_part_temp_cur, "get_part_temp_celsius",
               units::temperature(const bodypart_id& id) const);
    DOC("Sets a specific body part to a temperature in Celsius.");
    SET_FX_N_T(set_part_temp_cur, "set_part_temp_celsius",
               void(const bodypart_id& id, units::temperature temp));
    DOC("Gets all bodyparts and their associated temperatures in Celsius.");
    luna::set_fx(
        ut, "get_temp_celsius",
        sol::resolve<std::map<bodypart_id, units::temperature>()>(&UT_CLASS::get_temp_cur));
    DOC("Sets ALL body parts on a creature to the given temperature in Celsius.");
    SET_FX_N_T(set_temp_cur, "set_temp_celsius", void(units::temperature temp));

    // These legacy functions are named with 'BTU' (body temperature units).
    DOC("Deprecated: use get_part_temp_celsius instead. Gets the current temperature of a specific body part in legacy Body Temperature Units.");
    luna::set_fx(
        ut, "get_part_temp_btu",
        static_cast<int (*)(const UT_CLASS&, const bodypart_id&)>(
            [](const UT_CLASS& charac, const bodypart_id& id) -> int {
                return units::to_legacy_bodypart_temp(charac.get_part_temp_cur(id));
            }));
    DOC("Deprecated: use set_part_temp_celsius instead. Sets a specific body part to a given legacy Body Temperature Units value.");
    luna::set_fx(
        ut, "set_part_temp_btu",
        static_cast<void (*)(UT_CLASS&, const bodypart_id&, int)>(
            [](UT_CLASS& charac, const bodypart_id& id, int temp) -> void {
                charac.set_part_temp_cur(id, units::from_legacy_bodypart_temp(temp));
            }));
    DOC("Deprecated: use get_temp_celsius instead. Gets all bodyparts and their associated temperatures in legacy Body Temperature Units.");
    luna::set_fx(
        ut, "get_temp_btu",
        static_cast<std::map<bodypart_id, int> (*)(
            UT_CLASS&)>([](UT_CLASS& charac) -> std::map<bodypart_id, int> {
            auto legacy_temps = std::map<bodypart_id, int>{};
            for (const auto& [bp, temp] : charac.get_temp_cur()) {
                legacy_temps.emplace(bp, units::to_legacy_bodypart_temp(temp));
            }
            return legacy_temps;
        }));
    DOC("Deprecated: use set_temp_celsius instead. Sets ALL body parts on a creature to the given legacy Body Temperature Units value.");
    luna::set_fx(
        ut, "set_temp_btu",
        static_cast<void (*)(UT_CLASS&, int)>([](UT_CLASS& charac, int temp) -> void {
            charac.set_temp_cur(units::from_legacy_bodypart_temp(temp));
        }));

    SET_FX_T(blood_loss, int(const bodypart_id& bp) const);

    SET_FX_N_T(encumb, "get_part_encumbrance", int(const bodypart_str_id& bp) const);

    SET_FX_T(is_wearing_power_armor, bool(bool*) const);

    SET_FX_T(is_wearing_active_power_armor, bool() const);

    SET_FX_T(is_wearing_active_optcloak, bool() const);

    SET_FX_T(in_climate_control, bool());

    SET_FX_T(is_blind, bool() const);

    SET_FX_T(is_invisible, bool() const);

    SET_FX_T(get_movement_mode, character_movemode() const);

    SET_FX_T(set_movement_mode, void(character_movemode));

    SET_FX_T(expose_to_disease, void(diseasetype_id));

    SET_FX_T(is_quiet, bool() const);

    SET_FX_T(is_stealthy, bool() const);

    SET_FX(uncanny_dodge);

    SET_FX(get_melee_stamina_cost);

    SET_FX_T(cough, void(bool harmful, int loudness));

    SET_FX_T(bionic_armor_bonus, float(const bodypart_id&, damage_type) const);

    SET_FX_T(mabuff_armor_bonus, int(damage_type) const);

    SET_FX_T(has_base_trait, bool(const trait_id& b) const);

    SET_FX_T(has_trait_flag, bool(const trait_flag_str_id& b) const);

    SET_FX_T(has_trait_type, bool(const std::string& mut_type) const);

    SET_FX_T(has_opposite_trait, bool(const trait_id& flag) const);

    SET_FX_T(set_mutation, void(const trait_id&));
    SET_FX_T(unset_mutation, void(const trait_id&));

    SET_FX_T(activate_mutation, void(const trait_id&));
    SET_FX_T(deactivate_mutation, void(const trait_id&));

    SET_FX_T(can_mount, bool(const monster& critter) const);
    SET_FX_T(mount_creature, void(monster & z));
    SET_FX_T(is_mounted, bool() const);
    SET_FX_T(check_mount_will_move, bool(const tripoint_bub_ms& dest_loc));
    SET_FX_T(check_mount_is_spooked, bool());
    SET_FX_T(dismount, void());
    SET_FX_T(forced_dismount, void());

    SET_FX_T(is_deaf, bool() const);

    SET_FX_T(has_two_arms, bool() const);

    SET_FX_T(get_working_arm_count, int() const);

    SET_FX_T(get_working_leg_count, int() const);

    SET_FX_T(is_limb_disabled, bool(const bodypart_id&) const);

    SET_FX_T(is_limb_broken, bool(const bodypart_id&) const);

    SET_FX_T(can_run, bool());

    SET_FX_T(hurtall, void(int dam, Creature* source, bool));

    SET_FX_T(hitall, int(int dam, int vary, Creature* source));

    SET_FX_T(heal, void(const bodypart_id&, int));

    SET_FX_T(healall, void(int dam));
#undef UT_CLASS
}
} // namespace cata::detail
