#include "action.h"
#include "artifact.h"
#include "bodypart.h"
#include "calendar.h"
#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna.h"
#include "catalua_luna_doc.h"
#include "creature.h"
#include "damage.h"
#include "enum_conversions.h"
#include "enums.h"
#include "hsv_color.h"
#include "mission.h"
#include "monfaction.h"
#include "npc.h"
#include "sounds.h"
#include "trap.h"
#include "type_id.h"
#include "vitamin.h"

#include <string>
#include <type_traits>

namespace {

template <typename E> auto reg_enum(sol::state& lua) -> void {
    // Sol2 has new_enum<E>(...) function, but it needs to know all value-string
    // pairs at compile time, so we can't use it with io::enum_to_string.
    //
    // As such, hack it by creating read-only table.

    luna::userenum<E> et = luna::begin_enum<E>(lua);

    using Int = std::underlying_type_t<E>;
    constexpr auto max = static_cast<Int>(enum_traits<E>::last);

    for (Int i = 0; i < max; ++i) {
        auto e = static_cast<E>(i);
        std::string key = io::enum_to_string<E>(e);
        luna::add_val(et, key, e);
    }

    luna::finalize_enum(et);
}

} // namespace

auto cata::detail::reg_colors(sol::state& lua) -> void {
    // Colors are not enums, we have to do them manually
    luna::userenum<color_id> et = luna::begin_enum<color_id>(lua);

    using Int = std::underlying_type_t<color_id>;
    constexpr auto max = static_cast<Int>(color_id::num_colors);

    for (Int i = 0; i < max; ++i) {
        auto e = static_cast<color_id>(i);
        std::string key = get_all_colors().id_to_name(e);
        luna::add_val(et, key, e);
    }

    luna::finalize_enum(et);
    {
        sol::usertype<RGBColor> ut =
            luna::new_usertype<RGBColor>(lua, luna::no_bases, luna::no_constructor);

        luna::set(ut, "name", &RGBColor::friendly_name);

        DOC("RGB Color Getters.");
        luna::userlib lib = luna::begin_lib(lua, "rgb_colors");

        DOC("Get RGB Color from string");
        luna::set_fx(lib, "try_parse", &RGBColor::try_parse);
        DOC("Get random RGB Color from fuzzy match string");
        luna::set_fx(lib, "get_random", &RGBColor::random_named);
        DOC("Get RGBColor -> string mapping");
        luna::set_fx(lib, "get_all_named_colors", &RGBColor::get_all_named_colors);

        luna::finalize_lib(lib);
    }
}

auto cata::detail::reg_enums(sol::state& lua) -> void {
    reg_enum<action_id>(lua);
    reg_enum<add_type>(lua);
    reg_enum<Attitude>(lua);
    reg_enum<body_part>(lua);
    reg_enum<character_movemode>(lua);
    reg_enum<damage_type>(lua);
    reg_enum<direction>(lua);
    reg_enum<game_message_type>(lua);
    reg_enum<mf_attitude>(lua);
    reg_enum<m_flag>(lua);
    reg_enum<monster_attitude>(lua);
    reg_enum<creature_size>(lua);
    reg_enum<npc_attitude>(lua);
    reg_enum<npc_need>(lua);
    reg_enum<ot_match_type>(lua);
    reg_enum<sfx::channel>(lua);
    reg_enum<mission_origin>(lua);
    reg_enum<mission_goal>(lua);
    reg_enum<art_charge>(lua);
    reg_enum<art_charge_req>(lua);
    reg_enum<art_effect_active>(lua);
    reg_enum<art_effect_passive>(lua);
    reg_enum<vitamin_type>(lua);
    reg_enum<moon_phase>(lua);
}
