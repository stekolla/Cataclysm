#include "activity_type.h"
#include "catalua_bindings/character_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "character_martial_arts.h"
#include "creature.h"
#include "inventory.h"
#include "item.h"
#include "player_activity.h"
#include "type_id.h"

namespace cata::detail {
auto reg_character_items_and_activity(sol::usertype<Character>& ut) -> void {
#define UT_CLASS Character
    // Could also use a std::variant<item*, detached_ptr<item>*> for a single method
    DOC("Attempts to wear an item not in the creature inventory. If boolean parameter is false, item is worn instantly");
    luna::set_fx(ut, "wear_detached", [](UT_CLASS& c, detached_ptr<item>& it, bool interactive) {
        return !!c.wear_item(std::move(it), interactive, std::nullopt);
    });

    DOC("Attempts to wear an item in the creature inventory. If boolean parameter is false, item is worn instantly");
    luna::set_fx(ut, "wear", [](UT_CLASS& c, item& it, bool interactive) {
        return c.wear_possessed(it, interactive, std::nullopt);
    });

    DOC("Checks if creature can wear a given item. If boolean parameter is true, ignores already worn items");
    luna::set_fx(ut, "can_wear", [](const UT_CLASS& c, const item& it, bool ignore_worn) {
        auto res = c.can_wear(it, ignore_worn);
        return res.success() && res.value();
    });

    luna::set_fx(ut, "get_worn_items", [](const UT_CLASS& c) {
        std::vector<item*> res{};
        std::ranges::copy(c.worn, std::back_inserter(res));
        return res;
    });

    SET_FX_T(assign_activity, void(const activity_id&, int, int, int, const std::string&));

    DOC("Assigns a Lua-backed activity.  ");
    DOC("The callbacks receive a table with keys `user` (Character), `activity` (PlayerActivity), `name`, `data` and `pos`.  ");
    DOC("Takes a table with keys:  ");
    DOC("* `type` (ActivityTypeId): Activity type.  ");
    DOC("* `duration` (TimeDuration): How long the activity takes.  ");
    DOC("* `on_finish` (string, optional): Key in `game.activity_functions` called when the activity finishes.  ");
    DOC("* `on_turn` (string, optional): Key in `game.activity_functions` called every turn of the activity.  ");
    DOC("* `data` (table, optional): Serializable Lua state passed back to the callbacks as `params.data`.  ");
    DOC("* `pos` (TripointBubMs, optional): Activity position. Callbacks receive it as `params.pos` in absolute coordinates.  ");
    DOC("* `name` (string, optional): Activity name, passed to the callbacks as `params.name`.  ");
    DOC("* `interruptable` (boolean, optional): Whether a key press can interrupt the activity. Defaults to true.");
    luna::set_fx(ut, "assign_lua_activity", [](UT_CLASS& c, const sol::table& opts) -> void {
        c.assign_activity(make_lua_activity_from_options(opts));
    });

    luna::set_fx(ut, "get_activity", [](UT_CLASS& c) -> player_activity* {
        return c.activity.get();
    });

    SET_FX_T(has_activity, bool(const activity_id& type) const);

    SET_FX_T(cancel_activity, void());

    SET_FX_T(metabolic_rate, float() const);

    SET_FX_T(base_age, int() const);
    SET_FX_T(set_base_age, void(int));
    SET_FX_T(mod_base_age, void(int));

    SET_FX_T(age, int() const);

    SET_FX_T(base_height, int() const);
    SET_FX_T(set_base_height, void(int));
    SET_FX_T(mod_base_height, void(int));

    SET_FX_T(height, int() const);

    SET_FX_T(bodyweight, units::mass() const);

    SET_FX_T(bionics_weight, units::mass() const);

    SET_FX_T(get_armor_acid, int(bodypart_id bp) const);

    SET_FX_T(get_stim, int() const);
    SET_FX_T(set_stim, void(int));
    SET_FX_T(mod_stim, void(int));

    SET_FX_T(get_rad, int() const);
    SET_FX_T(set_rad, void(int));
    SET_FX_T(mod_rad, void(int));

    SET_FX_T(get_stamina, int() const);
    SET_FX_T(get_stamina_max, int() const);
    SET_FX_T(set_stamina, void(int));
    SET_FX_T(mod_stamina, void(int));

    SET_FX_T(vitamin_get, int(const vitamin_id&) const);
    SET_FX_T(vitamin_set, bool(const vitamin_id&, int));
    SET_FX_T(vitamin_mod, int(const vitamin_id&, int, bool));

    SET_FX_T(sound_hallu, void());

    SET_FX_T(wake_up, void());

    SET_FX_T(get_shout_volume, int() const);

    SET_FX_T(shout, void(std::string, bool));

    SET_FX_T(vomit, void());

    SET_FX_T(restore_scent, void());

    SET_FX_T(mod_painkiller, void(int));

    SET_FX_T(set_painkiller, void(int));

    SET_FX_T(get_painkiller, int() const);

    SET_FX_T(spores, void());
    SET_FX_T(blossoms, void());

    SET_FX_T(rooted, void());

    luna::set_fx(
        ut, "fall_asleep",
        sol::overload(sol::resolve<void()>(&UT_CLASS::fall_asleep),
                      sol::resolve<void(const time_duration& duration)>(&UT_CLASS::fall_asleep)));

    SET_FX_T(get_hostile_creatures, std::vector<Creature*>(int) const);

    SET_FX_T(get_visible_creatures, std::vector<Creature*>(int) const);

    SET_FX_T(wearing_something_on, bool(const bodypart_id&) const);

    SET_FX_T(is_wearing_helmet, bool() const);

    SET_FX_T(get_morale_level, int() const);
    luna::set_fx(
        ut, "add_morale",
        [](UT_CLASS& c, const morale_type& type, int bonus, int max_bonus,
           const time_duration& duration, const time_duration& decay_start, bool capped,
           sol::optional<const itype*> item_type) -> void {
            c.add_morale(type, bonus, max_bonus, duration, decay_start, capped,
                         item_type.value_or(nullptr));
        });
    SET_FX_T(has_morale, bool(const morale_type&) const);
    SET_FX_T(get_morale, int(const morale_type&) const);
    SET_FX_T(rem_morale, void(const morale_type&));
    SET_FX_T(clear_morale, void());
    SET_FX_T(has_morale_to_read, bool() const);
    SET_FX_T(has_morale_to_craft, bool() const);

    luna::set_fx(ut, "knows_recipe", [](const UT_CLASS& utObj, const recipe_id& rec) -> bool {
        return utObj.knows_recipe(&(rec.obj()));
    });
    luna::set_fx(ut, "learn_recipe", [](UT_CLASS& utObj, const recipe_id& rec) -> void {
        utObj.learn_recipe(&(rec.obj()));
    });

    luna::set_fx(
        ut, "knows_martial_art", [](const UT_CLASS& utObj, const matype_id& ma_type_id) -> bool {
            return utObj.martial_arts_data->has_martialart(ma_type_id);
        });
    luna::set_fx(
        ut, "learn_martial_art", [](const UT_CLASS& utObj, const matype_id& ma_type_id) -> void {
            utObj.martial_arts_data->add_martialart(ma_type_id);
        });

    SET_FX_T(suffer, void());

    SET_FX_T(irradiate, bool(float rads, bool bypass));

    DOC("Whether the character knows about the trap at the given tripoint.");
    SET_FX(knows_trap);

    DOC("Character learns that the given trap is on the given tripoint. If the trap is null, the character learns that there is no trap there.");
    luna::set_fx(ut, "add_known_trap", [](UT_CLASS& c, const tripoint_bub_ms& p, const trap_id& tr) {
        c.add_known_trap(p, tr.obj());
    });

    SET_FX_T(can_hear, bool(const tripoint_bub_ms& source, int volume) const);

    SET_FX_T(hearing_ability, float() const);

    SET_FX_T(get_lowest_hp, int() const);
#undef UT_CLASS
}
} // namespace cata::detail
