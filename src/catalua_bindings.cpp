#include "catalua_bindings.h"

#include <string>
#include <vector>

namespace luna::detail
{
std::vector<std::string> current_comment;
} // namespace luna::detail

auto cata::reg_all_bindings( sol::state &lua ) -> void
{
    using namespace detail;

    override_default_print( lua );
    forbid_unsafe_functions( lua );
    reg_debug_api( lua );
    reg_game_api( lua );
    reg_date_time_api( lua );
    reg_units( lua );
    reg_skill_level_map( lua );
    reg_damage_instance( lua );
    reg_creature_family( lua );
    reg_point_tripoint( lua );
    reg_item( lua );
    reg_locale_api( lua );
    reg_map( lua );
    reg_distribution_grid( lua );
    reg_ui_elements( lua );
    reg_colors( lua );
    reg_enums( lua );
    reg_game_ids( lua );
    mod_bionic_data( lua );
    mod_mutation_branch( lua );
    mod_mutation_category_trait( lua );
    reg_bionics( lua );
    reg_magic( lua );
    reg_names( lua );
    reg_mission( lua );
    reg_mission_type( lua );
    reg_recipe( lua );
    reg_coords_library( lua );
    reg_monster_type_ids( lua );
    reg_monster_groups( lua );
    reg_overmap( lua );
    reg_constants( lua );
    reg_hooks_examples( lua );
    reg_technique( lua );
    reg_types( lua );
    reg_time_types( lua );
    reg_effect( lua );
    reg_field_type( lua );
    reg_testing_library( lua );
    reg_requirement( lua );
    reg_inventory( lua );
    reg_mapgendata( lua );
    reg_perk( lua );
}
