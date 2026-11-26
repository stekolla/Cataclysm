#include "../src/map/map.h"
#include "../src/vehicle/vehicle_part.h"
#include "../src/vehicle/vpart_position.h"
#include "cached_options.h"
#include "calendar.h"
#include "cata_utility.h"
#include "catch/catch.hpp"
#include "character.h"
#include "coordinates.h"
#include "game.h"
#include "game_constants.h"
#include "item.h"
#include "map/field.h"
#include "map/lightmap.h"
#include "map_helpers.h"
#include "options_helpers.h"
#include "player_helpers.h"
#include "shadowcasting.h"
#include "state_helpers.h"
#include "type_id.h"
#include "vehicle/vehicle.h"
#include "vehicle/vpart_range.h"
#include "weather/weather.h"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

enum class vision_test_flags {
    none = 0,
    no_3d = 1 << 0,
    crouching = 1 << 1,
};

static auto operator&(vision_test_flags l, vision_test_flags r) -> vision_test_flags {
    return static_cast<vision_test_flags>(static_cast<unsigned>(l) & static_cast<unsigned>(r));
}

static auto operator!(vision_test_flags f) -> bool { return !static_cast<unsigned>(f); }

static void full_map_test(
    const std::vector<std::string>& setup, const std::vector<std::string>& expected_results,
    const time_point& time, const vision_test_flags flags) {
    const ter_id t_brick_wall("t_brick_wall");
    const ter_id t_window_frame("t_window_frame");
    const ter_id t_floor("t_floor");
    const ter_id t_utility_light("t_utility_light");
    const efftype_id effect_narcosis("narcosis");
    const ter_id t_flat_roof("t_flat_roof");
    const ter_id t_open_air("t_open_air");

    const auto reset_light = on_out_of_scope([] { g->reset_light_level(); });
    const auto restore_time = restore_on_out_of_scope<time_point>(calendar::turn);
    const auto restore_weather = restore_on_out_of_scope<weather_type_id>(get_weather().weather_id);
    Character& player_character = get_player_character();
    g->place_player(tripoint_bub_ms(60, 60, 0));
    get_weather().weather_id = weather_type_id("clear");
    calendar::turn = time;
    g->reset_light_level();

    if (!!(flags & vision_test_flags::crouching)) {
        player_character.set_movement_mode(character_movemode::CMM_CROUCH);
    } else {
        player_character.set_movement_mode(character_movemode::CMM_WALK);
    }

    REQUIRE(!player_character.is_blind());
    REQUIRE(!player_character.in_sleep_state());
    REQUIRE(!player_character.has_effect(effect_narcosis));

    player_character.recalc_sight_limits();

    int height = setup.size();
    REQUIRE(height > 0);
    REQUIRE(static_cast<size_t>(height) == expected_results.size());
    int width = setup[0].size();

    for (const std::string& line : setup) { REQUIRE(line.size() == static_cast<size_t>(width)); }

    for (const std::string& line : expected_results) {
        REQUIRE(line.size() == static_cast<size_t>(width));
    }

    tripoint_bub_ms origin;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            switch (setup[y][x]) {
                case 'V':
                case 'U':
                case 'H':
                case 'u':
                    origin = player_character.bub_pos() - point_rel_ms(x, y);
                    if (setup[y][x] == 'V') {
                        player_character.worn.push_back(item::spawn("wearable_light_on"));
                    }
                    break;
            }
        }
    }

    {
        // Sanity check on player placement
        REQUIRE(origin.z() < OVERMAP_HEIGHT);
        auto player_offset = player_character.bub_pos() - origin;
        REQUIRE(player_offset.y() >= 0);
        REQUIRE(player_offset.y() < height);
        REQUIRE(player_offset.x() >= 0);
        REQUIRE(player_offset.x() < width);
        char player_char = setup[player_offset.y()][player_offset.x()];
        REQUIRE(
            (player_char == 'U' || player_char == 'u' || player_char == 'V' || player_char == 'H'));
    }

    map& here = get_map();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto p = origin + point_rel_ms(x, y);
            const auto above = p + tripoint_above;
            switch (setup[y][x]) {
                case ' ':
                    break;
                case 'L':
                    here.ter_set(p, t_utility_light);
                    here.ter_set(above, t_flat_roof);
                    break;
                case '#':
                case 'H':
                    here.ter_set(p, t_brick_wall);
                    here.ter_set(above, t_flat_roof);
                    break;
                case '=':
                    here.ter_set(p, t_window_frame);
                    here.ter_set(above, t_flat_roof);
                    break;
                case '-':
                case 'u':
                    here.ter_set(p, t_floor);
                    here.ter_set(above, t_flat_roof);
                    break;
                case 'U':
                case 'V':
                    // Already handled above
                    break;
                default:
                    FAIL("unexpected setup char '" << setup[y][x] << "'");
            }
        }
    }

    // We have to run the whole thing twice, because the first time through the
    // player's vision_threshold is based on the previous lighting level (so
    // they might, for example, have poor nightvision due to having just been
    // in daylight)
    here.invalidate_map_cache(origin.z());
    here.build_map_cache(origin.z());
    here.update_visibility_cache(origin.z());
    here.invalidate_map_cache(origin.z());
    here.build_map_cache(origin.z());
    here.update_visibility_cache(origin.z());

    const level_cache& cache = here.access_cache(origin.z());
    const level_cache& above_cache = here.access_cache(origin.z() + 1);
    const visibility_variables& vvcache = here.get_visibility_variables_cache();

    std::ostringstream fields;
    std::ostringstream transparency;
    std::ostringstream seen;
    std::ostringstream lm;
    std::ostringstream apparent_light;
    std::ostringstream visibility_cache_dump;
    std::ostringstream obstructed;
    std::ostringstream floor_above;
    transparency << std::setprecision(3);
    seen << std::setprecision(3);
    lm << std::setprecision(3);
    apparent_light << std::setprecision(3);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto p = origin + point_rel_ms(x, y);
            const auto& visibility_cache = here.get_visibility_variables_cache();
            const map::apparent_light_info al =
                map::apparent_light_helper(cache, p, visibility_cache.visibility_scale_factor);
            for (auto& pr : here.field_at(p)) { fields << pr.second.name() << ','; }
            fields << ' ';
            transparency << std::setw(6) << cache.transparency_cache[cache.idx(p.x(), p.y())] << ' ';
            seen << std::setw(6) << cache.seen_cache[cache.idx(p.x(), p.y())] << ' ';
            lm << std::setw(6) << cache.lm[cache.idx(p.x(), p.y())] << ' ';
            apparent_light << std::setw(6) << al.apparent_light << ' ';
            visibility_cache_dump << std::setw(6) << cache.visibility_cache[cache.idx(p.x(), p.y())]
                                  << ' ';
            obstructed << (al.obstructed ? '#' : '.') << ' ';
            floor_above << (above_cache.floor_cache[above_cache.idx(p.x(), p.y())] ? '#' : '.')
                        << ' ';
        }
        fields << '\n';
        transparency << '\n';
        seen << '\n';
        lm << '\n';
        apparent_light << '\n';
        visibility_cache_dump << '\n';
        obstructed << '\n';
        floor_above << '\n';
    }

    INFO("origin: " << origin);
    INFO("player: " << player_character.bub_pos());
    INFO("unimpaired_range: " << player_character.unimpaired_range());
    INFO("visibility_range: " << vvcache.visibility_range);
    INFO("detail_range: " << vvcache.detail_range);
    INFO("vision_threshold: " << vvcache.vision_threshold);
    INFO("time: " << to_string(time));
    INFO("fields:\n" << fields.str());
    INFO("transparency:\n" << transparency.str());
    INFO("seen:\n" << seen.str());
    INFO("lm:\n" << lm.str());
    INFO("apparent_light_from_downloaded_seen:\n" << apparent_light.str());
    INFO("visibility_cache:\n" << visibility_cache_dump.str());
    INFO("obstructed:\n" << obstructed.str());
    INFO("floor_above:\n" << floor_above.str());

    bool success = true;
    std::ostringstream expected;
    std::ostringstream observed;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto p = origin + point_rel_ms(x, y);
            const lit_level level = here.apparent_light_at(p, vvcache);
            const char exp_char = expected_results[y][x];
            if (exp_char < '0' || exp_char > '9') {
                FAIL("unexpected result char '" << expected_results[y][x] << "'");
            }
            const int expected_level = exp_char - '0';

            observed << level << ' ';
            expected << expected_level << ' ';
            if (level != expected_level) { success = false; }
        }
        observed << '\n';
        expected << '\n';
    }

    INFO("observed:\n" << observed.str());
    INFO("expected:\n" << expected.str());
    CHECK(success);
}

struct vision_test_case {
    std::vector<std::string> setup;
    std::vector<std::string> expected_results;
    time_point time;
    vision_test_flags flags;
    static void transpose(std::vector<std::string>& v) {
        if (v.empty()) { return; }
        std::vector<std::string> new_v(v[0].size());

        for (const std::string& col : v) {
            for (size_t y = 0; y < new_v.size(); ++y) { new_v[y].push_back(col.at(y)); }
        }

        v = new_v;
    }

    void transpose() {
        transpose(setup);
        transpose(expected_results);
    }

    void reflect_x() {
        for (std::string& s : setup) { std::reverse(s.begin(), s.end()); }
        for (std::string& s : expected_results) { std::reverse(s.begin(), s.end()); }
    }

    void reflect_y() {
        std::reverse(setup.begin(), setup.end());
        std::reverse(expected_results.begin(), expected_results.end());
    }

    void test() const { full_map_test(setup, expected_results, time, flags); }

    void test_all_transformations() const {
        // Three reflections generate all possible rotations and reflections of
        // the test case
        for (int transform = 0; transform < 8; ++transform) {
            INFO("test case transformation: " << transform);
            vision_test_case copy(*this);
            if (transform & 1) { copy.transpose(); }
            if (transform & 2) { copy.reflect_x(); }
            if (transform & 4) { copy.reflect_y(); }
            copy.test();
        }
    }

    void test_all() const {
        if (!!(flags & vision_test_flags::no_3d)) {
            SUCCEED("2D-only vision fixture skipped; current visibility is always cross-z");
            return;
        }
        INFO("using current visibility casting");
        test_all_transformations();
    }
};

static const time_point midnight = calendar::turn_zero + 0_hours;
static const time_point midday = calendar::turn_zero + 12_hours;

// The following characters are used in these setups:
// ' ' - empty, outdoors
// '-' - empty, indoors
// 'U' - player, outdoors
// 'u' - player, indoors
// 'V' - player, with light in inventory
// 'H' - player, indoors, standing inside a wall
// 'L' - light, indoors
// '#' - wall
// '=' - window frame

TEST_CASE("vision_daylight", "[shadowcasting][vision]") {
    clear_all_state();
    vision_test_case t{
        {
            "   ",
            "   ",
            " U ",
        },
        {
            "444",
            "444",
            "444",
        },
        midday,
        vision_test_flags::none};

    t.test();
}

TEST_CASE("vision_day_indoors", "[shadowcasting][vision]") {
    clear_all_state();
    vision_test_case t{
        {
            "###",
            "#u#",
            "###",
        },
        {
            "111",
            "111",
            "111",
        },
        midday,
        vision_test_flags::none};

    t.test();
}

TEST_CASE("vision_light_shining_in", "[shadowcasting][vision]") {
    clear_all_state();
    vision_test_case t{
        {
            "##########",
            "#--------#",
            "#u-------#",
            "#--------=",
            "##########",
        },
        {
            "1144444666",
            "1144444466",
            "1144444444",
            "1144444444",
            "1144444444",
        },
        midday,
        // Legacy 2D-only expectation; current visibility is always cross-z.
        vision_test_flags::no_3d};

    t.test_all();
}

TEST_CASE("vision_no_lights", "[shadowcasting][vision]") {
    clear_all_state();
    vision_test_case t{
        {
            "   ",
            " U ",
        },
        {
            "111",
            "111",
        },
        midnight,
        vision_test_flags::none};

    t.test_all();
}

TEST_CASE("vision_utility_light", "[shadowcasting][vision]") {
    clear_all_state();
    vision_test_case t{
        {
            " L ",
            "   ",
            " U ",
        },
        {
            "444",
            "444",
            "444",
        },
        midnight,
        vision_test_flags::none};

    t.test_all();
}

TEST_CASE("vision_wall_obstructs_light", "[shadowcasting][vision]") {
    clear_all_state();
    vision_test_case t{
        {
            " L ",
            "###",
            " U ",
        },
        {
            "666",
            "111",
            "111",
        },
        midnight,
        vision_test_flags::none};

    t.test_all();
}

TEST_CASE("vision_wall_can_be_lit_by_player", "[shadowcasting][vision]") {
    clear_all_state();
    vision_test_case t{
        {
            " V",
            "  ",
            "  ",
            "##",
            "--",
        },
        {
            "44",
            "44",
            "44",
            "44",
            "66",
        },
        midnight,
        vision_test_flags::none};

    t.test_all();
}

TEST_CASE("vision_crouching_blocks_vision_but_not_light", "[shadowcasting][vision]") {
    clear_all_state();
    vision_test_case t{
        {
            "###",
            "#u#",
            "#=#",
            "   ",
        },
        {
            "444",
            "444",
            "444",
            "666",
        },
        midday,
        vision_test_flags::crouching};

    t.test_all();
}

TEST_CASE("vision_see_wall_in_moonlight", "[shadowcasting][vision][.]") {
    clear_all_state();
    const time_point full_moon = calendar::turn_zero + calendar::season_length() / 6;
    // Verify that I've picked the full_moon time correctly.
    CHECK(get_moon_phase(full_moon) == MOON_FULL);

    vision_test_case t{
        {
            "---",
            "###",
            "   ",
            "   ",
            " U ",
        },
        {
            "666",
            "111",
            "111",
            "111",
            "111",
        },
        // Want a night time
        full_moon - time_past_midnight(full_moon),
        vision_test_flags::none};

    t.test_all();
}

TEST_CASE("nv_range_math_correct", "[vision]") {
    clear_all_state();
    for (int i = 0; i < 80; i++) {
        float threshold = vision::threshold_for_nv_range(i);
        // minus, because LIGHT_RANGE is for luminosity at a distance
        // tl;dr: exp/log math, luminosity * exp( -range ) = threshold
        int range = -LIGHT_RANGE(threshold);
        CHECK(range == i);
    }
}

TEST_CASE("vision_single_tile_skylight", "[shadowcasting][vision]") {
    clear_all_state();
    /**
     * Diffused sunlight through the single-tile roof opening should be symmetrical.
     */
    vision_test_case t{
        {
            "-----------",
            "-#########-",
            "-#-------#-",
            "-#-------#-",
            "-#-------#-",
            "-#---U---#-",
            "-#-------#-",
            "-#-------#-",
            "-#-------#-",
            "-#########-",
            "-----------",
        },
        {
            "66666666666",
            "64444444446",
            "64444444446",
            "64444444446",
            "64444444446",
            "64444444446",
            "64444444446",
            "64444444446",
            "64444444446",
            "64444444446",
            "66666666666",
        },
        midday,
        vision_test_flags::none};

    t.test_all();
}

/// A light-override field blocks skylight diffusion along a roofed corridor.
TEST_CASE("daylight_diffusion_respects_light_override", "[shadowcasting][vision]") {
    clear_all_state();
    const auto reset_light = on_out_of_scope([] { g->reset_light_level(); });
    const auto restore_time = restore_on_out_of_scope<time_point>(calendar::turn);
    const auto restore_weather = restore_on_out_of_scope<weather_type_id>(get_weather().weather_id);
    const auto cleanup = on_out_of_scope([] { clear_map(); });
    auto& here = get_map();
    const auto origin = tripoint_bub_ms(60, 60, 0);
    g->place_player(origin);
    get_weather().weather_id = weather_type_id("clear");
    calendar::turn = midday;
    g->reset_light_level();

    for (auto x = -1; x <= 8; ++x) {
        for (auto y = -1; y <= 1; ++y) {
            const auto p = origin + tripoint_rel_ms(x, y, 0);
            const auto wall = y != 0 || x == -1 || x == 8;
            here.ter_set(p, ter_id(wall ? "t_brick_wall" : "t_floor"));
            if (x != 0 || y != 0) { here.ter_set(p + tripoint_above, ter_id("t_flat_roof")); }
        }
    }

    const auto target = origin + tripoint_rel_ms(3, 0, 0);
    const auto sample = [&]() -> float {
        here.invalidate_map_cache(0);
        here.build_map_cache(0);
        const auto& cache = here.access_cache(0);
        return cache.lm[cache.idx(target.x(), target.y())];
    };
    const auto open_light = sample();
    REQUIRE(here.add_field(origin + tripoint_rel_ms(1, 0, 0), field_type_id("fd_shadow"), 1));
    const auto blocked_light = sample();
    CHECK(open_light > blocked_light);
}

TEST_CASE("vision_player_opaque_neighbors_still_visible_night", "[shadowcasting][vision]") {
    clear_all_state();
    /**
     *  Even when stating inside the opaque wall and surrounded by opaque walls,
     *  you should see yourself and immediate surrounding.
     *  (walls here simulate the behavior of the fully opaque fields, e.g. thick smoke)
     */
    vision_test_case t{
        {
            "#####",
            "#####",
            "##H##",
            "#####",
            "#####",
        },
        {
            "66666",
            "61116",
            "61116",
            "61116",
            "66666",
        },
        midnight,
        vision_test_flags::none};

    t.test_all();
}

namespace {
auto check_vehicle_vision(const bool from_inside) -> void {
    clear_all_state();
    const auto reset_light = on_out_of_scope([] { g->reset_light_level(); });
    const auto restore_time = restore_on_out_of_scope<time_point>(calendar::turn);
    const auto restore_weather = restore_on_out_of_scope<weather_type_id>(get_weather().weather_id);
    const auto cleanup = on_out_of_scope([] { clear_map(); });
    auto& here = get_map();
    auto* const veh =
        here.add_vehicle(vproto_id("cube_van_test"), tripoint_bub_ms(60, 60, 0), -45_degrees, 0, 0);
    REQUIRE(veh != nullptr);
    for (const auto& part : veh->get_avail_parts("OPENABLE")) { veh->close(part.part_index()); }

    const auto inside = veh->mount_to_bubble(tripoint_mnt_veh(0, 0, 0));
    const auto window = veh->mount_to_bubble(tripoint_mnt_veh(1, 0, 0));
    const auto outside = window + (window - inside);
    REQUIRE(inside != window);
    g->place_player(from_inside ? inside : outside);
    get_weather().weather_id = weather_type_id("clear");
    calendar::turn = midday;
    g->reset_light_level();
    get_player_character().recalc_sight_limits();
    here.invalidate_map_cache(0);
    here.build_map_cache(0);
    here.update_visibility_cache(0);
    here.invalidate_map_cache(0);
    here.build_map_cache(0);
    here.update_visibility_cache(0);

    const auto target = from_inside ? outside : inside;
    const auto& cache = here.access_cache(0);
    CHECK(cache.transparency_cache[cache.idx(window.x(), window.y())] > LIGHT_TRANSPARENCY_SOLID);
    CHECK(cache.seen_cache[cache.idx(target.x(), target.y())] > 0.0f);
    CHECK(
        here.apparent_light_at(target, here.get_visibility_variables_cache()) != lit_level::BLANK);
}
} // namespace

/// Check that a survivor inside a rotated vehicle can see outside through its windshield.
TEST_CASE("vision_see_out_of_vehicle", "[shadowcasting][vision]") { check_vehicle_vision(true); }

/// Check that a survivor outside a rotated vehicle can see its cabin through the windshield.
TEST_CASE("vision_see_into_vehicle", "[shadowcasting][vision]") { check_vehicle_vision(false); }

TEST_CASE("lighting_circular_distances", "[shadowcasting][vision]") {
    clear_all_state();
    const auto restore_trigdist = restore_on_out_of_scope<bool>(trigdist);

    auto& player_character = get_player_character();
    const auto origin = tripoint_bub_ms(60, 60, 0);
    g->place_player(origin);
    get_weather().weather_id = weather_type_id("clear");
    calendar::turn = midnight;
    g->reset_light_level();
    player_character.set_movement_mode(character_movemode::CMM_WALK);
    player_character.worn.push_back(item::spawn("wearable_light_on"));

    auto& here = get_map();
    const auto cardinal = origin + point_rel_ms(5, 0);
    const auto diagonal = origin + point_rel_ms(5, 5);

    struct sampled_light {
        float cardinal;
        float diagonal;
    };
    const auto sample = [&](const bool circular) -> sampled_light {
        const auto opt = override_option("CIRCLEDIST", circular ? "true" : "false");
        trigdist = circular;
        here.invalidate_map_cache(origin.z());
        here.build_map_cache(origin.z());
        const auto& cache = here.access_cache(origin.z());
        return {
            .cardinal = cache.lm[cache.idx(cardinal.x(), cardinal.y())],
            .diagonal = cache.lm[cache.idx(diagonal.x(), diagonal.y())],
        };
    };

    const auto square = sample(false);
    CHECK(square.cardinal > LIGHT_AMBIENT_LOW);
    CHECK(square.cardinal == Approx(square.diagonal));

    const auto circular = sample(true);
    CHECK(circular.cardinal > circular.diagonal);
    CHECK(square.diagonal > circular.diagonal);
}
