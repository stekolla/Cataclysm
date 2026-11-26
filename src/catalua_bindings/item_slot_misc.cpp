#include "artifact.h"
#include "catalua_bindings/item_artifact_descriptions.h"
#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "enum_conversions.h"
#include "itype.h"
#include "relic.h"
#include "type_id.h"

#include <ranges>
#include <string>
#include <vector>


namespace cata::detail {
auto reg_item_slot_misc(sol::state& lua) -> void {
#define UT_CLASS islot_artifact
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        SET_MEMB_RO(charge_req);
        SET_MEMB_RO(charge_type);
        SET_MEMB_RO(dream_freq_met);
        SET_MEMB_RO(dream_freq_unmet);
        SET_MEMB_RO(dream_msg_met);
        SET_MEMB_RO(dream_msg_unmet);

        SET_MEMB_RO(effects_activated);
        SET_MEMB_RO(effects_carried);
        SET_MEMB_RO(effects_wielded);
        SET_MEMB_RO(effects_worn);

        luna::set_fx(
            ut, "effects_activated_names",
            [](const islot_artifact& slot) -> std::vector<std::string> {
                return artifact_effect_names(slot.effects_activated);
            });
        luna::set_fx(
            ut, "effects_carried_names",
            [](const islot_artifact& slot) -> std::vector<std::string> {
                return artifact_effect_names(slot.effects_carried);
            });
        luna::set_fx(
            ut, "effects_wielded_names",
            [](const islot_artifact& slot) -> std::vector<std::string> {
                return artifact_effect_names(slot.effects_wielded);
            });
        luna::set_fx(
            ut, "effects_worn_names", [](const islot_artifact& slot) -> std::vector<std::string> {
                return artifact_effect_names(slot.effects_worn);
            });

        luna::set_fx(
            ut, "effects_activated_descriptions",
            [](const islot_artifact& slot) -> std::vector<std::string> {
                return artifact_effect_descriptions(slot.effects_activated);
            });
        luna::set_fx(
            ut, "effects_carried_descriptions",
            [](const islot_artifact& slot) -> std::vector<std::string> {
                return artifact_effect_descriptions(slot.effects_carried);
            });
        luna::set_fx(
            ut, "effects_wielded_descriptions",
            [](const islot_artifact& slot) -> std::vector<std::string> {
                return artifact_effect_descriptions(slot.effects_wielded);
            });
        luna::set_fx(
            ut, "effects_worn_descriptions",
            [](const islot_artifact& slot) -> std::vector<std::string> {
                return artifact_effect_descriptions(slot.effects_worn);
            });

        luna::set_fx(ut, "charge_type_description", [](const islot_artifact& slot) -> std::string {
            return artifact_charge_description(slot.charge_type);
        });
        luna::set_fx(ut, "charge_req_description", [](const islot_artifact& slot) -> std::string {
            return artifact_charge_req_description(slot.charge_req);
        });
    }
#undef UT_CLASS

#define UT_CLASS islot_milling
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        SET_MEMB_N_RO(conversion_rate_, "conversion_rate");
        SET_MEMB_N_RO(into_, "converts_into");
    }
#undef UT_CLASS

#define UT_CLASS common_ranged_data
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        SET_MEMB_N_RO(aimedcritbonus, "aimed_crit_bonus");
        SET_MEMB_N_RO(aimedcritmaxbonus, "aimed_crit_max_bonus");
        SET_MEMB_RO(damage);
        SET_MEMB_RO(dispersion);
        SET_MEMB_RO(range);
    }
#undef UT_CLASS

#define UT_CLASS relic
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        SET_FX(name);
        SET_FX(get_enchantments);
        SET_FX(get_recharge_scheme);
        SET_FX(get_spells);
    }
#undef UT_CLASS
}
} // namespace cata::detail
