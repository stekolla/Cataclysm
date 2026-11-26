#include "bodypart.h"
#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "damage.h"
#include "martialarts.h"
#include "skill.h"

auto cata::detail::reg_skill_level_map(sol::state& lua) -> void {
    {
        sol::usertype<SkillLevel> ut =
            luna::new_usertype<SkillLevel>(lua, luna::no_bases, luna::no_constructor);

        luna::set_fx(ut, "is_training", &SkillLevel::isTraining);
        luna::set_fx(ut, "level", sol::resolve<int() const>(&SkillLevel::level));
        luna::set_fx(ut, "highest_level", &SkillLevel::highestLevel);
        luna::set_fx(ut, "train", &SkillLevel::train);
        luna::set_fx(ut, "can_train", &SkillLevel::can_train);
    }
    {
        sol::usertype<SkillLevelMap> ut = luna::new_usertype<SkillLevelMap>(
            lua, luna::bases<std::map<skill_id, SkillLevel>>(), luna::no_constructor);
        luna::set_fx(ut, "mod_skill_level", &SkillLevelMap::mod_skill_level);
        luna::set_fx(ut, "get_skill_level",
                     sol::resolve<int(const skill_id&) const>(&SkillLevelMap::get_skill_level));
        luna::set_fx(
            ut, "get_skill_level_object",
            sol::resolve<SkillLevel&(const skill_id&)>(&SkillLevelMap::get_skill_level_object));
    }
}

auto cata::detail::reg_damage_instance(sol::state& lua) -> void {
#define UT_CLASS damage_unit
    {
        DOC("Represents a damage amount");
        DOC("Constructors are:");
        DOC("new()");
        DOC("new(damageType, amount, armorPen, remainingArmorMultiplier, damageMultiplier)");
        sol::usertype<UT_CLASS> ut = luna::new_usertype<UT_CLASS>(
            lua, luna::no_bases,
            luna::constructors<UT_CLASS(damage_type, float, float, float, float)>());

        SET_MEMB(type);
        SET_MEMB(amount);
        SET_MEMB(res_pen);
        SET_MEMB(res_mult);
        SET_MEMB(damage_multiplier);

        luna::set_fx(ut, sol::meta_function::equal_to, &UT_CLASS::operator==);
    }
#undef UT_CLASS // #define UT_CLASS damage_unit
#define UT_CLASS damage_instance
    {
        DOC("Represents a bunch of damage amounts");
        DOC("Constructors are:");
        DOC("new(damageType, amount, armorPen, remainingArmorMultiplier, damageMultiplier)");
        sol::usertype<UT_CLASS> ut = luna::new_usertype<UT_CLASS>(
            lua, luna::no_bases,
            luna::constructors<UT_CLASS(), UT_CLASS(damage_type, float, float, float, float)>());

        SET_MEMB(damage_units);

        SET_FX(mult_damage);
        SET_FX(type_damage);
        SET_FX(total_damage);
        SET_FX(clear);
        SET_FX(empty);
        SET_FX(add_damage);
        SET_FX_T(add, void(const damage_unit&));

        luna::set_fx(ut, sol::meta_function::equal_to, &UT_CLASS::operator==);
    }
#undef UT_CLASS // #define UT_CLASS damage_instance
#define UT_CLASS dealt_damage_instance
    {
        DOC("Represents the final dealt damage");
        sol::usertype<UT_CLASS> ut =
            luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        SET_MEMB(dealt_dams);
        SET_MEMB(bp_hit);

        SET_FX(type_damage);
        SET_FX(total_damage);
    }
#undef UT_CLASS // #define UT_CLASS dealt_damage_instance
}

auto cata::detail::reg_technique(sol::state& lua) -> void {
    {
        sol::usertype<ma_technique> ut =
            luna::new_usertype<ma_technique>(lua, luna::no_bases, luna::no_constructor);

        luna::set(ut, "name", &ma_technique::name);
        luna::set_fx(ut, "get_description", &ma_technique::get_description);
        luna::set(ut, "avatar_message", &ma_technique::avatar_message);
        luna::set(ut, "npc_message", &ma_technique::npc_message);
        luna::set(ut, "defensive", &ma_technique::defensive);
        luna::set(ut, "side_switch", &ma_technique::side_switch);
        luna::set(ut, "down_dur", &ma_technique::down_dur);
        luna::set(ut, "stun_dur", &ma_technique::stun_dur);
        luna::set(ut, "knockback_dist", &ma_technique::knockback_dist);
        luna::set(ut, "knockback_spread", &ma_technique::knockback_spread);
        luna::set(ut, "powerful_knockback", &ma_technique::powerful_knockback);
        luna::set(ut, "controlled_knockback", &ma_technique::controlled_knockback);
        luna::set(ut, "crit_tec", &ma_technique::crit_tec);
        luna::set(ut, "crit_ok", &ma_technique::crit_ok);
        luna::set(ut, "knockback_follow", &ma_technique::knockback_follow);
        luna::set(ut, "disarms", &ma_technique::disarms);
        luna::set(ut, "take_weapon", &ma_technique::take_weapon);
        luna::set(ut, "dodge_counter", &ma_technique::dodge_counter);
        luna::set(ut, "block_counter", &ma_technique::block_counter);
        luna::set(ut, "miss_recovery", &ma_technique::miss_recovery);
        luna::set(ut, "grab_break", &ma_technique::grab_break);
        luna::set(ut, "force_unarmed", &ma_technique::force_unarmed);
    }
}
