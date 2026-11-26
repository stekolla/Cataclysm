#include "bionics.h"
#include "catalua_bindings/character_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "character_martial_arts.h"
#include "creature.h"
#include "damage.h"
#include "flag_trait.h"
#include "magic/magic.h"
#include "make_static.h"
#include "mutation.h"
#include "type_id.h"

namespace cata::detail {
auto reg_character_mutations(sol::usertype<Character>& ut) -> void {
#define UT_CLASS Character
    // Legacy Lua API aliases — scripts using these names still work.
    luna::set_fx(ut, "global_square_location", [](const Character& c) { return c.abs_pos(); });
    SET_FX_N_T(abs_sm_pos, "global_sm_location", tripoint_abs_sm() const);

    SET_FX_T(has_mabuff, bool(const mabuff_id&) const);

    SET_FX_T(mabuff_tohit_bonus, float() const);

    SET_FX_T(mabuff_dodge_bonus, float() const);

    SET_FX_T(mabuff_block_bonus, int() const);

    SET_FX_T(mabuff_speed_bonus, int() const);

    SET_FX_T(mabuff_arpen_bonus, int(damage_type) const);

    SET_FX_T(mabuff_damage_mult, float(damage_type) const);

    SET_FX_T(mabuff_damage_bonus, int(damage_type) const);

    SET_FX_T(mabuff_attack_cost_penalty, int() const);

    SET_FX_T(mabuff_attack_cost_mult, float() const);

    SET_FX_T(mutation_effect, void(const trait_id&));

    SET_FX_T(mutation_loss_effect, void(const trait_id&));

    luna::set_fx(ut, "activate_mutation_id", [](UT_CLASS& utObj, trait_id& mut) {
        utObj.activate_mutation(mut);
    });

    luna::set_fx(ut, "deactivate_mutation_id", [](UT_CLASS& utObj, trait_id& mut) {
        utObj.deactivate_mutation(mut);
    });

    SET_FX_T(has_active_mutation, bool(const trait_id&) const);

    SET_FX_T(mutate, void());

    SET_FX_T(mutation_ok, bool(const trait_id&, bool, bool) const);

    luna::set_fx(
        ut, "mutate_category",
        sol::overload(
            sol::resolve<void(const mutation_category_id&)>(&UT_CLASS::mutate_category),
            sol::resolve<void(const mutation_category_id&, bool)>(&UT_CLASS::mutate_category)));

    luna::set_fx(
        ut, "mutate_towards",
        sol::overload(sol::resolve<bool(std::vector<trait_id>, int)>(&UT_CLASS::mutate_towards),
                      sol::resolve<bool(const trait_id&)>(&UT_CLASS::mutate_towards)));


    SET_FX_T(remove_mutation, void(const trait_id&, bool));

    SET_FX_T(has_child_flag, bool(const trait_id& flag) const);

    SET_FX_T(remove_child_flag, void(const trait_id& flag));

    SET_FX_T(get_highest_category, mutation_category_id() const);

    SET_FX_T(is_weak_to_water, bool() const);

    SET_FX_T(mutation_armor, float(bodypart_id, damage_type) const);

    SET_FX_T(get_bionics, std::vector<bionic_id>() const);

    luna::set_fx(ut, "get_bionic", [](UT_CLASS& utObj, const bionic_id& bid) -> bionic {
        return utObj.get_bionic_state(bid);
    });

    SET_FX_T(has_bionic, bool(const bionic_id& b) const);

    luna::set_fx(
        ut, "activate_bionic",
        [](UT_CLASS& utObj, const bionic_id& bid, std::optional<bool> block_message) -> bool {
            if (utObj.has_bionic(bid)) {
                bionic& bio = utObj.get_bionic_state(bid);
                bio.powered =
                    bio.info().has_flag(STATIC(flag_id("BIONIC_TOGGLED")))
                    || bio.info().charge_time > 0;
                if (bio.info().charge_time > 0) { bio.charge_timer = bio.info().charge_time; }
                if (!bio.id->enchantments.empty()) { utObj.recalculate_enchantment_cache(); }
                return utObj.activate_bionic(bio, block_message.value_or(true));
            }
            return false;
        });

    luna::set_fx(
        ut, "deactivate_bionic",
        [](UT_CLASS& utObj, const bionic_id& bid, std::optional<bool> block_message) -> bool {
            if (utObj.has_bionic(bid)) {
                bionic& bio = utObj.get_bionic_state(bid);
                return utObj.deactivate_bionic(bio, block_message.value_or(true));
            }
            return false;
        });

    SET_FX_T(has_active_bionic, bool(const bionic_id& b) const);

    SET_FX_T(has_any_bionic, bool() const);

    SET_FX_T(has_bionics, bool() const);

    SET_FX_T(clear_bionics, void());
    SET_FX_T(get_used_bionics_slots, int(const bodypart_id&) const);
    SET_FX_T(get_total_bionics_slots, int(const bodypart_id&) const);
    SET_FX_T(get_free_bionics_slots, int(const bodypart_id&) const);

    SET_FX_T(remove_bionic, void(const bionic_id&));

    SET_FX_T(add_bionic, void(const bionic_id&));
#undef UT_CLASS
}
} // namespace cata::detail
