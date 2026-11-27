#include "../src/map/map.h"
#include "../src/overmap/overmapbuffer.h"
#include "avatar.h"
#include "calendar.h"
#include "cata_utility.h"
#include "catch/catch.hpp"
#include "character.h"
#include "character_id.h"
#include "coordinates.h"
#include "dialogue.h"
#include "effect.h"
#include "faction.h"
#include "game.h"
#include "inventory.h"
#include "item.h"
#include "item_category.h"
#include "map_helpers.h"
#include "mission.h"
#include "npc.h"
#include "npc_favor.h"
#include "npctalk.h"
#include "pimpl.h"
#include "player.h"
#include "player_helpers.h"
#include "state_helpers.h"
#include "string_formatter.h"
#include "string_id.h"
#include "type_id.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/// Check the manager's three linked jobs and the exact turn-in thresholds for their materials.
TEST_CASE("lumbermill_mission_progression", "[npc][mission][dialogue]") {
    clear_all_state();
    g->faction_manager_ptr->create_if_needed();
    auto& player = get_avatar();
    auto manager = npc();
    manager.load_npc_template(string_id<npc_template>("NPC_lumbermill_merchant"));
    const auto cleanup = on_out_of_scope([&] {
        manager.get_faction()->remove_member(manager.getID());
        clear_all_state();
    });
    REQUIRE(manager.chatbin.missions.size() == 1);
    const auto* quarters = manager.chatbin.missions.front();
    REQUIRE(quarters->get_type().id == mission_type_id("MISSION_LUMBERMILL_SLEEPING_QUARTERS"));
    const auto repair_id = quarters->get_follow_up();
    REQUIRE(repair_id == mission_type_id("MISSION_GET_DRIVEBELTS"));
    const auto* repair = mission::reserve_new(repair_id, manager.getID());
    const auto walls_id = repair->get_follow_up();
    REQUIRE(walls_id == mission_type_id("MISSION_LUMBERMILL_FORTIFY"));
    const auto* walls = mission::reserve_new(walls_id, manager.getID());
    CHECK(walls->get_follow_up().is_null());

    const auto check_delivery = [&](const auto& job, const char* material, int count) {
        CAPTURE(material);
        REQUIRE_FALSE(job.is_complete(manager.getID()));
        for (auto i = 0; i < count - 1; ++i) { player.i_add(item::spawn(material), false); }
        player.invalidate_crafting_inventory();
        CHECK_FALSE(job.is_complete(manager.getID()));
        player.i_add(item::spawn(material), false);
        player.invalidate_crafting_inventory();
        CHECK(job.is_complete(manager.getID()));
    };
    check_delivery(*quarters, "mattress", 6);
    check_delivery(*repair, "drivebelt_makeshift", 3);
    player.i_add(item::spawn("nail", calendar::turn, 4999), false);
    player.invalidate_crafting_inventory();
    CHECK_FALSE(walls->is_complete(manager.getID()));
    player.i_add(item::spawn("nail", calendar::turn, 1), false);
    player.invalidate_crafting_inventory();
    CHECK(walls->is_complete(manager.getID()));
}

/// Take Tacoma's optional lead and check that only the occupied mill completes it.
TEST_CASE("tacoma_lumbermill_discovery", "[npc][mission][dialogue]") {
    auto& player = get_avatar();
    auto previous_avatar = avatar();
    std::swap(player, previous_avatar);
    const auto restore_avatar = on_out_of_scope([&] { std::swap(player, previous_avatar); });
    clear_all_state();
    player.setID(g->assign_npc_id(), true);
    g->faction_manager_ptr->create_if_needed();
    auto foreman = npc();
    foreman.load_npc_template(string_id<npc_template>("ranch_foreman"));
    const auto cleanup = on_out_of_scope([&] {
        foreman.get_faction()->remove_member(foreman.getID());
        clear_all_state();
    });

    auto d = dialogue();
    d.alpha = &player;
    d.beta = &foreman;
    const auto find_reply = [&](const char* topic) {
        return std::ranges::find_if(d.responses, [&](const auto& reply) {
            return reply.success.next_topic.id == topic;
        });
    };
    const auto foreman_topic = talk_topic("TALK_RANCH_FOREMAN");
    const auto lead_topic = talk_topic("TALK_RANCH_FOREMAN_LUMBERMILL");
    d.gen_responses(foreman_topic);
    CHECK(find_reply("TALK_RANCH_FOREMAN_LUMBERMILL") == d.responses.end());

    REQUIRE(foreman.chatbin.missions.size() == 1);
    auto* planks = foreman.chatbin.missions.front();
    planks->assign(player);
    d.gen_responses(foreman_topic);
    REQUIRE(find_reply("TALK_RANCH_FOREMAN_LUMBERMILL") != d.responses.end());
    d.gen_responses(lead_topic);
    REQUIRE(find_reply("TALK_NONE") != d.responses.end());
    CHECK(player.get_active_missions().size() == 1);

    const auto pos = player.abs_omt_pos();
    auto& overmap = ACTIVE_OVERMAP_BUFFER;
    const auto old_terrain = overmap.ter(pos);
    auto visibility = std::vector<std::pair<tripoint_abs_omt, bool>>();
    for (auto y = -1; y <= 1; ++y) {
        for (auto x = -1; x <= 1; ++x) {
            const auto tile = pos + point_rel_omt(x, y);
            visibility.emplace_back(tile, overmap.seen(tile));
        }
    }
    const auto restore_overmap = on_out_of_scope([&] {
        overmap.ter_set(pos, old_terrain);
        for (const auto& [tile, seen] : visibility) { overmap.set_seen(tile, seen); }
    });
    const auto occupied = oter_id("lumbermill_0_0_ocu_north");
    overmap.ter_set(pos, occupied);
    const auto accept = find_reply("TALK_RANCH_FOREMAN");
    REQUIRE(accept != d.responses.end());
    CHECK(accept->success.apply(d).id == "TALK_RANCH_FOREMAN");

    const auto jobs = player.get_active_missions();
    const auto lead_id = mission_type_id("MISSION_TACOMA_FIND_LUMBERMILL");
    const auto lead = std::ranges::find_if(jobs, [&](const auto* job) {
        return job->get_type().id == lead_id;
    });
    REQUIRE(lead != jobs.end());
    CHECK((*lead)->get_target() == pos);
    CHECK(overmap.seen(pos));

    overmap.ter_set(pos, oter_id("lumbermill_0_0_north"));
    (*lead)->process();
    CHECK((*lead)->in_progress());
    overmap.ter_set(pos, occupied);
    (*lead)->process();
    CHECK_FALSE((*lead)->in_progress());
    CHECK(planks->in_progress());
    d.gen_responses(lead_topic);
    CHECK(find_reply("TALK_RANCH_FOREMAN") == d.responses.end());
}
static const efftype_id effect_gave_quest_item("gave_quest_item");
static const efftype_id effect_currently_busy("currently_busy");
static const efftype_id effect_infection("infection");
static const efftype_id effect_infected("infected");

static const trait_id trait_PROF_FED("PROF_FED");
static const trait_id trait_PROF_SWAT("PROF_SWAT");

static auto create_test_talker(const tripoint_bub_ms& talker_pos) -> npc& {
    const string_id<npc_template> test_talker("test_talker");
    const auto model_id = get_map().place_npc(talker_pos, test_talker, true);
    g->load_npcs();

    auto* model_npc = g->find_npc(model_id);
    REQUIRE(model_npc != nullptr);

    for (const trait_id& tr : model_npc->get_mutations()) { model_npc->unset_mutation(tr); }
    model_npc->set_stored_kcal(model_npc->max_stored_kcal());
    model_npc->set_thirst(0);
    model_npc->set_fatigue(0);
    model_npc->remove_effect(efftype_id("sleep"));
    // An ugly hack to prevent NPC falling asleep during testing due to massive fatigue
    model_npc->set_mutation(trait_id("WEB_WEAVER"));

    return *model_npc;
}

static void gen_response_lines(dialogue& d, size_t expected_count) {
    d.gen_responses(d.topic_stack.back());
    for (talk_response& response : d.responses) { response.create_option_line(d, ' '); }
    if (d.responses.size() != expected_count) {
        cata_printf("Test failure in %s\n", d.topic_stack.back().id.c_str());
        for (talk_response& response : d.responses) {
            cata_printf("response: %s\n", response.text.c_str());
        }
    }
    CAPTURE(d.responses);
    REQUIRE(d.responses.size() == expected_count);
}

static auto gen_dynamic_line(dialogue& d) -> std::string {
    std::string challenge = d.dynamic_line(d.topic_stack.back());
    return challenge;
}

static void change_om_type(const std::string& new_type) {
    const auto omt_pos = project_to<coords::omt>(get_player_character().abs_pos());
    ACTIVE_OVERMAP_BUFFER.ter_set(omt_pos, oter_id(new_type));
}

static auto prep_test(dialogue& d) -> npc& {
    clear_vehicles();
    auto& player_character = get_avatar();
    REQUIRE_FALSE(player_character.in_vehicle);

    const auto test_origin =
        tripoint_bub_ms(g_half_mapsize_x, g_half_mapsize_y, player_character.abs_pos().z());
    player_character.setpos(test_origin);

    g->faction_manager_ptr->create_if_needed();

    auto& talker_npc = create_test_talker(player_character.bub_pos() + point(5, 0));

    d.alpha = &player_character;
    d.beta = &talker_npc;

    return talker_npc;
}

TEST_CASE("npc_talk_start", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    d.add_topic("TALK_TEST_START");
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
}

TEST_CASE("npc_talk_describe_mission", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    d.add_topic("TALK_DESCRIBE_MISSION");
    std::string d_line = gen_dynamic_line(d);
    CHECK(d_line == "I'm looking for wounded to help.");
}

/// Spawn each staff member to verify their fixed name and the mission chain in their work menu.
TEST_CASE("refugee_center_staff_offer_their_own_work", "[npc_talk][mission]") {
    struct staff_missions {
        const char* npc_id;
        const char* name;
        const char* first_offer;
        mission_type_id follow_up = mission_type_id::NULL_ID();
        const char* side_offer = nullptr;
        mission_type_id final_offer = mission_type_id::NULL_ID();
    };
    const auto expected = GENERATE(
        staff_missions{
            .npc_id = "evac_merchant",
            .name = "Owen Mercer, Merchant",
            .first_offer = "MISSION_FREE_MERCHANTS_EVAC_2",
            .follow_up = mission_type_id("MISSION_FREE_MERCHANTS_EVAC_3")},
        staff_missions{
            .npc_id = "evac_broker",
            .name = "Ruth Calder, Quartermaster",
            .first_offer = "MISSION_BROKER_CANNING_POT",
            .follow_up = mission_type_id("MISSION_FREE_MERCHANTS_EVAC_5"),
            .side_offer = "MISSION_BROKER_COFFEE",
            .final_offer = mission_type_id("MISSION_BROKER_1")},
        staff_missions{
            .npc_id = "evac_security_chief",
            .name = "Milton Lang, Security Chief",
            .first_offer = "MISSION_FREE_MERCHANTS_EVAC_1",
            .follow_up = mission_type_id("MISSION_EVAC_SECURITY_RADIO_CHECK"),
            .final_offer = mission_type_id("MISSION_EVAC_SECURITY_APPROACH")},
        staff_missions{
            .npc_id = "evac_maintenance_lead",
            .name = "Mara Velez, Maintenance Lead",
            .first_offer = "MISSION_EVAC_MAINTENANCE_SUPPLIES",
            .follow_up = mission_type_id("MISSION_EVAC_MAINTENANCE_PUMP"),
            .side_offer = "MISSION_EVAC_MAINTENANCE_KETTLE",
            .final_offer = mission_type_id("MISSION_FREE_MERCHANTS_EVAC_4")});
    CAPTURE(expected.npc_id);
    g->faction_manager_ptr->create_if_needed();
    auto staff = npc();
    staff.load_npc_template(string_id<npc_template>(expected.npc_id));
    const auto remove_staff = on_out_of_scope([&staff] {
        if (auto* staff_faction = staff.get_faction()) {
            staff_faction->remove_member(staff.getID());
        }
    });

    auto d = dialogue();
    CHECK(staff.name == expected.name);
    d.alpha = &get_avatar();
    d.beta = &staff;
    d.gen_responses(talk_topic(staff.chatbin.first_topic));
    REQUIRE(std::ranges::any_of(d.responses, [](const auto& response) {
        return response.success.next_topic.id == "TALK_MISSION_LIST";
    }));

    REQUIRE(staff.chatbin.missions.size() == (expected.side_offer == nullptr ? 1 : 2));
    d.add_topic("TALK_MISSION_LIST");
    if (expected.side_offer == nullptr) {
        REQUIRE(d.topic_stack.back().id == "TALK_MISSION_OFFER");
        const auto* offered = staff.chatbin.mission_selected;
        REQUIRE(offered != nullptr);
        CHECK(offered->get_type().id == mission_type_id(expected.first_offer));
        CHECK(offered->get_follow_up() == expected.follow_up);
        CHECK_FALSE(offered->is_assigned());
        CHECK(gen_dynamic_line(d) == offered->dialogue_for_topic("TALK_MISSION_OFFER"));
    } else {
        REQUIRE(d.topic_stack.back().id == "TALK_MISSION_LIST");
    }
    d.gen_responses(d.topic_stack.back());
    const auto check_offer = [&](const auto& id, const auto& follow_up) {
        const auto response = std::ranges::find_if(d.responses, [&](const auto& candidate) {
            return candidate.mission_selected != nullptr
                && candidate.mission_selected->get_type().id == id;
        });
        REQUIRE(response != d.responses.end());
        const auto& offered = *response->mission_selected;
        CHECK(offered.get_npc_id() == staff.getID());
        CHECK(offered.get_follow_up() == follow_up);
    };
    if (expected.side_offer != nullptr) {
        check_offer(mission_type_id(expected.first_offer), expected.follow_up);
        check_offer(mission_type_id(expected.side_offer), mission_type_id::NULL_ID());
    }
    if (!expected.final_offer.is_null()) {
        REQUIRE(expected.follow_up.is_valid());
        REQUIRE(expected.final_offer.is_valid());
        CHECK(expected.follow_up->follow_up == expected.final_offer);
        CHECK(expected.final_offer->follow_up.is_null());
    }
}

/// Generate Dana's real dialogue options to check prices, stock timers, and Tacoma handoffs.
TEST_CASE("dana_refugee_bread_and_tacoma_dialogue", "[npc_talk][refugee_center]") {
    auto& u = get_avatar();
    auto previous_avatar = avatar();
    std::swap(u, previous_avatar);
    const auto restore_avatar = on_out_of_scope([&] { std::swap(u, previous_avatar); });
    clear_all_state();
    const auto cleanup = on_out_of_scope([] { clear_all_state(); });
    auto d = dialogue();
    auto& dana = prep_test(d);
    const auto offers = [&](const auto& topic, const auto& destination) {
        d.gen_responses(talk_topic(topic));
        return std::ranges::count_if(d.responses, [&](const auto& reply) {
            return reply.success.next_topic.id == destination;
        });
    };

    const auto greeting = "TALK_REFUGEE_Dana_1";
    const auto bakery = "TALK_REFUGEE_Dana_Background_baking";
    const auto sold = "TALK_REFUGEE_Dana_Background_baking2";
    CHECK(offers(greeting, "TALK_REFUGEE_Dana_Background") == 1);
    CHECK(offers(greeting, "TALK_REFUGEE_Dana_Tacoma") == 0);
    const auto ranch = "npctalk_var_recruit_general_Nunez_Tacoma";
    u.set_value("npctalk_var_knowledge_flag_tacoma_started", "yes");
    REQUIRE(offers(greeting, "TALK_REFUGEE_Dana_Tacoma") == 1);
    d.apply_speaker_effects(talk_topic("TALK_REFUGEE_Dana_Tacoma"));
    CHECK(u.get_value(ranch) == "phase_1");
    REQUIRE(offers("TALK_RANCH_FOREMAN", "TALK_RANCH_FOREMAN_Nunez") == 1);
    d.apply_speaker_effects(talk_topic("TALK_RANCH_FOREMAN_Nunez"));
    CHECK(u.get_value(ranch) == "phase_2");
    d.apply_speaker_effects(talk_topic("TALK_REFUGEE_Dana_Tacoma"));
    CHECK(u.get_value(ranch) == "phase_2");
    const auto select = [&](const auto& topic, const auto& next) {
        REQUIRE(offers(topic, next) == 1);
        const auto reply = std::ranges::find_if(d.responses, [&](const auto& candidate) {
            return candidate.success.next_topic.id == next;
        });
        CHECK(reply->success.apply(d).id == next);
    };
    select(greeting, "TALK_REFUGEE_Dana_Tacoma2");
    CHECK(u.get_value(ranch) == "phase_3");
    select("TALK_REFUGEE_Pablo_2", "TALK_REFUGEE_Pablo_Tacoma2");
    CHECK(u.get_value(ranch) == "phase_4");
    CHECK(offers("TALK_REFUGEE_Pablo_2", "TALK_REFUGEE_Pablo_Tacoma2") == 0);
    CHECK(offers(greeting, "TALK_REFUGEE_Dana_Tacoma") == 0);
    CHECK(offers(greeting, "TALK_REFUGEE_Dana_Tacoma2") == 0);

    const auto flour = itype_id("flour");
    const auto regular = itype_id("sourdough_bread");
    const auto improved = itype_id("Dana_sourdough_bread");
    const auto revival = efftype_id("dana_sourdough_revival");
    const auto stock = efftype_id("dana_baking_sold");
    const auto discount = efftype_id("dana_barter_success");
    u.i_add(item::spawn(flour, calendar::turn, 6));
    CHECK(offers(bakery, sold) == 0);
    const auto buy = [&](const auto& bread) {
        const auto reply = std::ranges::find_if(d.responses, [&](const auto& candidate) {
            return candidate.success.next_topic.id == sold;
        });
        REQUIRE(reply != d.responses.end());
        CHECK(reply->success.apply(d).id == sold);
        CHECK(u.charges_of(flour) == 0);
        CHECK(u.has_amount(bread, 1));
        d.apply_speaker_effects(talk_topic(sold));
    };
    const auto discounted = "TALK_REFUGEE_Dana_Background_baking2_yesbarter";
    d.apply_speaker_effects(talk_topic(discounted));
    CHECK(dana.get_effect(discount).get_duration() == 24_hours);
    REQUIRE(offers(discounted, sold) == 1);
    buy(regular);
    CHECK(offers(bakery, sold) == 0);
    CHECK(dana.get_effect(stock).get_duration() == 18_hours);
    dana.remove_effect(stock);
    dana.remove_effect(discount);

    u.set_value("npctalk_var_mission_flag_Dana_Sourdough", "yes");
    u.i_add(item::spawn(flour, calendar::turn, 8));
    dana.add_effect(revival, 14_days);
    CHECK(offers(bakery, sold) == 0);
    dana.remove_effect(revival);
    REQUIRE(offers(bakery, sold) == 1);
    buy(improved);
}

/// Deliver the actual supplies and collect explicit rewards through each refugee's mission chain.
TEST_CASE("refugee_center_supply_mission_rewards", "[npc_talk][refugee_center]") {
    struct delivery {
        const char* mission_id;
        const char* item_id;
        int quantity;
        int reward;
    };
    const auto npc_id = GENERATE(
        "refugee_JennyForcette", "refugee_UyenTran", "refugee_DracoDune", "refugee_FatimaalJadir",
        "refugee_VanessaToby", "refugee_DanaNunez", "refugee_BorisBorichenko");
    const auto name = std::string_view(npc_id);
    auto jobs = std::vector<delivery>();
    if (name == "refugee_JennyForcette") {
        jobs =
            {{.mission_id = "MISSION_REFUGEE_Jenny_GET_MOTOR",
              .item_id = "motor",
              .quantity = 1,
              .reward = 4},
             {.mission_id = "MISSION_REFUGEE_Jenny_GET_TANK",
              .item_id = "metal_tank",
              .quantity = 1,
              .reward = 5}};
    } else if (name == "refugee_UyenTran") {
        jobs = {
            {.mission_id = "MISSION_REFUGEE_Uyen_1",
             .item_id = "disinfectant",
             .quantity = 50,
             .reward = 8},
            {.mission_id = "MISSION_REFUGEE_Uyen_2",
             .item_id = "bandages",
             .quantity = 30,
             .reward = 7},
            {.mission_id = "MISSION_REFUGEE_Uyen_3",
             .item_id = "prozac",
             .quantity = 60,
             .reward = 5}};
    } else if (name == "refugee_DracoDune") {
        jobs = {
            {.mission_id = "MISSION_REFUGEE_Draco_GET_GUITAR",
             .item_id = "acoustic_guitar",
             .quantity = 1,
             .reward = 0}};
    } else if (name == "refugee_FatimaalJadir") {
        jobs = {
            {.mission_id = "MISSION_REFUGEE_Fatima_1",
             .item_id = "holybook_quran",
             .quantity = 1,
             .reward = 5}};
    } else if (name == "refugee_DanaNunez") {
        jobs = {
            {.mission_id = "MISSION_REFUGEE_Dana_Sourdough",
             .item_id = "landough_calrisean",
             .quantity = 1,
             .reward = 0}};
    } else if (name == "refugee_BorisBorichenko") {
        jobs = {
            {.mission_id = "MISSION_REFUGEE_Boris_CLEANUP",
             .item_id = "",
             .quantity = 0,
             .reward = 5},
            {.mission_id = "MISSION_REFUGEE_Boris_WORKSPACE",
             .item_id = "circsaw_off",
             .quantity = 1,
             .reward = 5},
            {.mission_id = "MISSION_REFUGEE_Boris_LAPTOP",
             .item_id = "laptop_boris",
             .quantity = 1,
             .reward = 5}};
    } else {
        jobs = {
            {.mission_id = "MISSION_REFUGEE_Vanessa_1",
             .item_id = "survivor_hairtrimmer",
             .quantity = 1,
             .reward = 0}};
    }

    auto& u = get_avatar();
    auto previous_avatar = avatar();
    std::swap(u, previous_avatar);
    const auto restore_avatar = on_out_of_scope([&] { std::swap(u, previous_avatar); });
    clear_all_state();
    const auto cleanup = on_out_of_scope([] { clear_all_state(); });
    u.setID(g->assign_npc_id(), true);
    g->faction_manager_ptr->create_if_needed();
    const auto giver_id =
        get_map().place_npc(u.bub_pos() + point(5, 0), string_id<npc_template>(npc_id), true);
    g->load_npcs();
    auto* giver = g->find_npc(giver_id);
    REQUIRE(giver != nullptr);
    auto* faction = giver->get_faction();
    REQUIRE(faction != nullptr);
    const auto restore_likes = restore_on_out_of_scope(faction->likes_u);
    const auto restore_respect = restore_on_out_of_scope(faction->respects_u);
    const auto restore_power = restore_on_out_of_scope(faction->power);
    auto d = dialogue();
    d.alpha = &u;
    d.beta = giver;
    const auto merch = itype_id("FMCNote");

    const auto find_response = [&](const auto& next) {
        return std::ranges::find_if(d.responses, [&](const auto& reply) {
            return reply.success.next_topic.id == next;
        });
    };
    if (name == "refugee_BorisBorichenko") {
        const auto work = talk_topic("TALK_REFUGEE_Boris_Work1");
        const auto next = "TALK_REFUGEE_Boris_Work2";
        d.gen_responses(work);
        CHECK(find_response(next) == d.responses.end());
        u.set_value("npctalk_var_mission_flag_FMShopkeep_Mission1", "yes");
        d.gen_responses(work);
        CHECK(find_response(next) != d.responses.end());
    }

    for (const auto& delivery : jobs) {
        CAPTURE(delivery.mission_id);
        REQUIRE(giver->chatbin.missions.size() == 1);
        auto* job = giver->chatbin.missions.front();
        REQUIRE(job->get_type().id == mission_type_id(delivery.mission_id));
        giver->chatbin.mission_selected = job;
        // Item placement quests need a known target outside the crafting inventory radius.
        const auto target = u.abs_omt_pos() + point_rel_omt(6, 0);
        const auto previous_terrain = ACTIVE_OVERMAP_BUFFER.ter(target);
        const auto restore_terrain = on_out_of_scope([&] {
            ACTIVE_OVERMAP_BUFFER.ter_set(target, previous_terrain);
        });
        if (std::string_view(delivery.item_id) == "landough_calrisean") {
            ACTIVE_OVERMAP_BUFFER.ter_set(target, oter_id("bakery_north"));
        } else if (std::string_view(delivery.item_id) == "laptop_boris") {
            ACTIVE_OVERMAP_BUFFER.ter_set(target, oter_id("shelter_2_vandal_north"));
        }
        talk_function::assign_mission(*giver);
        CHECK_FALSE(job->is_complete(giver_id));
        if (delivery.quantity == 0) {
            const auto recruit =
                [&](const auto& menu, const auto& accept, const auto& recruit_npc) {
                    auto volunteer = npc();
                    volunteer.load_npc_template(string_id<npc_template>(recruit_npc));
                    const auto restore_talker = on_out_of_scope([&] {
                        d.beta = giver;
                        if (auto* volunteer_faction = volunteer.get_faction()) {
                            volunteer_faction->remove_member(volunteer.getID());
                        }
                    });
                    d.beta = &volunteer;
                    d.gen_responses(talk_topic(menu));
                    const auto reply = find_response(accept);
                    REQUIRE(reply != d.responses.end());
                    CHECK(reply->success.apply(d).id == accept);
                    d.apply_speaker_effects(talk_topic(accept));
                    d.gen_responses(talk_topic(menu));
                    CHECK(find_response(accept) == d.responses.end());
                };
            recruit("TALK_REFUGEE_Aleesha_2", "TALK_REFUGEE_Accept_Boris_Mission_1",
                    "refugee_AleeshaSeward");
            recruit("TALK_REFUGEE_Garry_2", "TALK_REFUGEE_Accept_Boris_Mission_1",
                    "refugee_GarryVilleneuve");
            recruit("TALK_OLD_GUARD_REP", "TALK_OLD_GUARD_REP_Accept_Boris_Mission_1",
                    "old_guard_rep");
            CHECK_FALSE(job->is_complete(giver_id));

            auto merchant = npc();
            merchant.load_npc_template(string_id<npc_template>("evac_merchant"));
            const auto restore_talker = on_out_of_scope([&] {
                d.beta = giver;
                if (auto* merchant_faction = merchant.get_faction()) {
                    merchant_faction->remove_member(merchant.getID());
                }
            });
            d.beta = &merchant;
            const auto offer = "TALK_EVAC_MERCHANT_Boris_Mission_1";
            d.gen_responses(talk_topic("TALK_EVAC_MERCHANT"));
            REQUIRE(find_response(offer) != d.responses.end());
            d.apply_speaker_effects(talk_topic(offer));
            u.i_add(item::spawn(merch, calendar::turn, 5));
            d.gen_responses(talk_topic(offer));
            const auto hire = find_response("TALK_EVAC_MERCHANT_Boris_Mission_1b");
            REQUIRE(hire != d.responses.end());
            CHECK(hire->success.apply(d).id == "TALK_EVAC_MERCHANT_Boris_Mission_1b");
        } else {
            const auto supply = itype_id(delivery.item_id);
            REQUIRE(supply.is_valid());
            if (item::count_by_charges(supply)) {
                u.i_add(item::spawn(supply, calendar::turn, delivery.quantity));
            } else {
                REQUIRE(delivery.quantity == 1);
                u.i_add(item::spawn(supply));
            }
        }
        u.invalidate_crafting_inventory();
        REQUIRE(job->is_complete(giver_id));
        const auto before = u.charges_of(merch);
        d.gen_responses(talk_topic("TALK_MISSION_INQUIRE"));
        const auto report = std::ranges::find_if(d.responses, [](const auto& reply) {
            return reply.success.next_topic.id == "TALK_MISSION_SUCCESS";
        });
        REQUIRE(report != d.responses.end());
        CHECK(report->success.apply(d).id == "TALK_MISSION_SUCCESS");
        talk_function::mission_success(*giver);
        CHECK(u.charges_of(merch) == before + delivery.reward);
        CHECK_FALSE(job->in_progress());
        CHECK_FALSE(job->has_generic_rewards());
        d.gen_responses(talk_topic("TALK_MISSION_SUCCESS"));
        REQUIRE(d.responses.size() == 1);
        CHECK(d.responses.front().success.apply(d).id == "TALK_NONE");

        if (name == "refugee_DanaNunez") {
            const auto revival = efftype_id("dana_sourdough_revival");
            CHECK(giver->get_effect(revival).get_duration() == 14_days);
        }
        if (name == "refugee_JennyForcette") {
            const auto project = d.dynamic_line(talk_topic("TALK_REFUGEE_JENNY_Project2"));
            if (delivery.reward == 4) {
                CHECK(project.find("still need a 60-liter metal tank") != std::string::npos);
            } else {
                CHECK(project.find("start building a prototype") != std::string::npos);
                u.remove_value("npctalk_var_mission_completed_Jenny_mission_3");
                CHECK(d.dynamic_line(talk_topic("TALK_REFUGEE_JENNY_Project2")) == project);
            }
        }
    }
    CHECK(giver->chatbin.missions.empty());

    if (name == "refugee_UyenTran") {
        d.gen_responses(talk_topic("TALK_REFUGEE_Uyen_Background"));
        REQUIRE(find_response("TALK_REFUGEE_Uyen_Teach") != d.responses.end());
        d.apply_speaker_effects(talk_topic("TALK_REFUGEE_Uyen_Teach"));
        d.gen_responses(talk_topic("TALK_REFUGEE_Uyen_Teach"));
        CHECK(find_response("TALK_TRAIN") != d.responses.end());
    } else if (name == "refugee_DracoDune") {
        CHECK(d.dynamic_line(talk_topic("TALK_REFUGEE_Draco_7")).find("patron of the arts")
              != std::string::npos);
        const auto joint = itype_id("joint");
        for (auto i = 0; i < 5; ++i) { u.i_add(item::spawn(joint)); }
        const auto trading = talk_topic("TALK_REFUGEE_Draco_10_approve");
        const auto sold = "TALK_REFUGEE_Draco_10_sold";
        d.gen_responses(trading);
        const auto sale = find_response(sold);
        REQUIRE(sale != d.responses.end());
        CHECK(sale->success.apply(d).id == sold);
        CHECK(u.charges_of(merch) == 6);
        CHECK_FALSE(u.has_amount(joint, 1));
        d.gen_responses(trading);
        CHECK(find_response(sold) == d.responses.end());
    } else if (name == "refugee_VanessaToby") {
        d.gen_responses(talk_topic("TALK_REFUGEE_Vanessa_2"));
        CHECK(find_response("TALK_REFUGEE_Vanessa_Haircut") != d.responses.end());
        const auto services = talk_topic("TALK_REFUGEE_Vanessa_Haircut");
        d.gen_responses(services);
        CHECK(std::ranges::any_of(d.responses, [](const auto& reply) {
            return reply.truetext.translated().find("owe me a free haircut") != std::string::npos;
        }));
        u.i_add(item::spawn(merch, calendar::turn, 6));
        d.gen_responses(services);
        for (const auto* option : {"Please give me a haircut.", "Please give me a shave."}) {
            CHECK(std::ranges::any_of(d.responses, [&](const auto& reply) {
                return reply.truetext.translated() == option;
            }));
        }
    }
}

/// Generate conversations on both sides of staff milestones to verify the new options and lines.
TEST_CASE("refugee_center_milestone_dialogue", "[npc_talk][refugee_center]") {
    auto& u = get_avatar();
    auto previous_avatar = avatar();
    std::swap(u, previous_avatar);
    const auto restore_avatar = on_out_of_scope([&] { std::swap(u, previous_avatar); });
    clear_all_state();
    const auto cleanup = on_out_of_scope([] { clear_all_state(); });
    auto d = dialogue();
    prep_test(d);

    const auto has_option = [&](const auto& menu, const auto& next) {
        d.gen_responses(talk_topic(menu));
        return std::ranges::any_of(d.responses, [&](const auto& reply) {
            return reply.success.next_topic.id == next;
        });
    };
    CHECK_FALSE(has_option("TALK_REFUGEE_Dana_Situation", "TALK_REFUGEE_Dana_Preservation"));
    CHECK_FALSE(has_option("TALK_REFUGEE_JENNY_Project1", "TALK_REFUGEE_JENNY_Solar"));
    u.set_value("npctalk_var_mission_flag_FMShopkeep_Mission5", "yes");
    u.set_value("npctalk_var_mission_flag_FMShopkeep_Mission4", "yes");
    CHECK(has_option("TALK_REFUGEE_Dana_Situation", "TALK_REFUGEE_Dana_Preservation"));
    CHECK(has_option("TALK_REFUGEE_JENNY_Project1", "TALK_REFUGEE_JENNY_Solar"));

    const auto uyen = talk_topic("TALK_REFUGEE_Uyen_Situation");
    const auto garry = talk_topic("TALK_REFUGEE_Garry_Situation");
    const auto stan = talk_topic("TALK_REFUGEE_Stan_Situation");
    const auto security = talk_topic("TALK_EVAC_SECURITY_CHIEF");
    CHECK(d.dynamic_line(uyen).find("water pressure") == std::string::npos);
    CHECK(d.dynamic_line(garry).find("workshop") == std::string::npos);
    CHECK(d.dynamic_line(stan).find("workshop") == std::string::npos);
    CHECK(d.dynamic_line(security).find("approach is safer") == std::string::npos);
    u.set_value("npctalk_var_mission_flag_evac_maintenance_pump", "yes");
    u.set_value("npctalk_var_mission_completed_Boris_mission_2", "yes");
    u.set_value("npctalk_var_mission_flag_evac_security_approach", "yes");
    CHECK(d.dynamic_line(uyen).find("water pressure") != std::string::npos);
    CHECK(d.dynamic_line(garry).find("workshop") != std::string::npos);
    CHECK(d.dynamic_line(stan).find("workshop") != std::string::npos);
    CHECK(d.dynamic_line(security).find("approach is safer") != std::string::npos);
}

/// Turn in coffee before the canning chain and check each reward and the final jar-buyback unlock.
TEST_CASE("ruth_calder_completes_supply_missions", "[npc_talk][mission][quartermaster]") {
    auto& u = get_avatar();
    auto previous_avatar = avatar();
    std::swap(u, previous_avatar);
    const auto restore_avatar = on_out_of_scope([&] { std::swap(u, previous_avatar); });
    clear_all_state();
    const auto cleanup = on_out_of_scope([] { clear_all_state(); });
    u.setID(g->assign_npc_id(), true);
    g->faction_manager_ptr->create_if_needed();
    const auto ruth_id = get_map().place_npc(
        u.bub_pos() + point(5, 0), string_id<npc_template>("evac_broker"), true);
    g->load_npcs();
    auto* ruth = g->find_npc(ruth_id);
    REQUIRE(ruth != nullptr);
    auto d = dialogue();
    d.alpha = &u;
    d.beta = ruth;
    const auto merch = itype_id("FMCNote");

    const auto complete =
        [&](const auto& mission_id, const auto& item_id, const auto count, const auto reward) {
            CAPTURE(mission_id);
            const auto offer =
                std::ranges::find_if(ruth->chatbin.missions, [&](const auto* candidate) {
                    return candidate->get_type().id == mission_type_id(mission_id);
                });
            REQUIRE(offer != ruth->chatbin.missions.end());
            auto* job = *offer;
            ruth->chatbin.mission_selected = job;
            talk_function::assign_mission(*ruth);
            const auto supply = itype_id(item_id);
            const auto by_charges = item::count_by_charges(supply);
            const auto add_supply = [&](const auto quantity) {
                if (quantity == 0) { return; }
                if (by_charges) {
                    u.i_add(item::spawn(supply, calendar::turn, quantity));
                } else {
                    for (auto i = 0; i < quantity; ++i) { u.i_add(item::spawn(supply)); }
                }
                u.invalidate_crafting_inventory();
            };
            add_supply(count - 1);
            CHECK_FALSE(job->is_complete(ruth_id));
            add_supply(1);
            REQUIRE(job->is_complete(ruth_id));
            const auto before = u.charges_of(merch);
            const auto credit_before = ruth->op_of_u.owed;
            d.gen_responses(talk_topic("TALK_MISSION_INQUIRE"));
            const auto report = std::ranges::find_if(d.responses, [](const auto& response) {
                return response.success.next_topic.id == "TALK_MISSION_SUCCESS";
            });
            REQUIRE(report != d.responses.end());
            CHECK(report->success.apply(d).id == "TALK_MISSION_SUCCESS");
            talk_function::mission_success(*ruth);
            CHECK(u.charges_of(merch) == before + reward);
            CHECK(ruth->op_of_u.owed == credit_before);
            CHECK(u.amount_of(supply) == 0);
            CHECK_FALSE(job->in_progress());
            d.gen_responses(talk_topic("TALK_MISSION_SUCCESS"));
            REQUIRE(d.responses.size() == 1);
            CHECK(d.responses.front().success.apply(d).id == "TALK_NONE");
            CHECK(ruth->chatbin.missions_assigned.empty());
        };

    auto* ruth_faction = ruth->get_faction();
    REQUIRE(ruth_faction != nullptr);
    const auto restore_likes = restore_on_out_of_scope(ruth_faction->likes_u);
    const auto restore_respect = restore_on_out_of_scope(ruth_faction->respects_u);
    const auto restore_power = restore_on_out_of_scope(ruth_faction->power);
    complete("MISSION_BROKER_COFFEE", "coffee_raw", 20, 3);
    REQUIRE(ruth->chatbin.missions.size() == 1);
    complete("MISSION_BROKER_CANNING_POT", "pot_canning", 1, 10);
    REQUIRE(ruth->chatbin.missions.size() == 1);
    complete("MISSION_FREE_MERCHANTS_EVAC_5", "jar_glass", 25, 20);
    REQUIRE(ruth->chatbin.missions.size() == 1);
    d.gen_responses(talk_topic("TALK_FREE_MERCHANT_STOCKS"));
    CHECK_FALSE(std::ranges::any_of(d.responses, [](const auto& response) {
        return response.success.next_topic.id == "TALK_FREE_MERCHANT_STOCKS_JARS";
    }));
    complete("MISSION_BROKER_1", "jar_3l_glass", 20, 25);
    CHECK(ruth->chatbin.missions.empty());
    d.add_topic("TALK_MISSION_LIST");
    CHECK(d.topic_stack.back().id == "TALK_MISSION_LIST");
    d.gen_responses(d.topic_stack.back());
    REQUIRE(d.responses.size() == 1);
    CHECK(d.responses.front().success.next_topic.id == "TALK_NONE");
    d.gen_responses(talk_topic("TALK_FREE_MERCHANT_STOCKS"));
    CHECK(std::ranges::any_of(d.responses, [](const auto& response) {
        return response.success.next_topic.id == "TALK_FREE_MERCHANT_STOCKS_JARS";
    }));
}

/// Exercise the shared turn-in choices before and after the objective, including failed jobs.
TEST_CASE(
    "mission_turn_in_requires_objective_and_uses_only_explicit_rewards", "[npc_talk][mission]") {
    const auto suffix = GENERATE("KILL", "ASSASSINATE", "TRAVEL");
    const auto fail_mission = GENERATE(false, true);
    CAPTURE(suffix, fail_mission);
    auto& u = get_avatar();
    auto previous_avatar = avatar();
    std::swap(u, previous_avatar);
    const auto restore_avatar = on_out_of_scope([&] { std::swap(u, previous_avatar); });
    clear_all_state();
    const auto cleanup = on_out_of_scope([] { clear_all_state(); });
    u.setID(g->assign_npc_id(), true);
    g->faction_manager_ptr->create_if_needed();
    auto giver = npc();
    auto* giver_faction = giver.get_faction();
    REQUIRE(giver_faction != nullptr);
    const auto restore_likes = restore_on_out_of_scope(giver_faction->likes_u);
    const auto restore_respect = restore_on_out_of_scope(giver_faction->respects_u);
    const auto restore_power = restore_on_out_of_scope(giver_faction->power);
    giver.setID(g->assign_npc_id(), true);
    auto d = dialogue();
    d.alpha = &u;
    d.beta = &giver;
    auto* job = mission::
        reserve_new(mission_type_id(std::string("MISSION_TEST_TURN_IN_") + suffix), giver.getID());
    giver.add_new_mission(job);
    d.add_topic("TALK_MISSION_LIST");
    REQUIRE(d.topic_stack.back().id == "TALK_MISSION_OFFER");
    d.gen_responses(d.topic_stack.back());
    const auto accept = std::ranges::find_if(d.responses, [](const auto& response) {
        return response.success.next_topic.id == "TALK_MISSION_ACCEPTED";
    });
    REQUIRE(accept != d.responses.end());
    CHECK(accept->success.apply(d).id == "TALK_MISSION_ACCEPTED");
    REQUIRE(job->in_progress());
    const auto merch = itype_id("FMCNote");
    REQUIRE(u.charges_of(merch) == 1);
    REQUIRE(job->get_likely_rewards().size() == 1);
    CHECK(job->get_likely_rewards().front().first == 2);

    auto* new_offer = mission::reserve_new(job->get_type().id, giver.getID());
    giver.add_new_mission(new_offer);
    d.add_topic("TALK_MISSION_LIST");
    CHECK(d.topic_stack.back().id == "TALK_MISSION_OFFER");
    CHECK(giver.chatbin.mission_selected == new_offer);
    CHECK_FALSE(new_offer->is_assigned());
    d.add_topic("TALK_MISSION_INQUIRE");
    CHECK(giver.chatbin.mission_selected == job);

    const auto query = talk_topic("TALK_MISSION_INQUIRE");
    d.gen_responses(query);
    CHECK(std::ranges::none_of(d.responses, [](const auto& response) {
        return response.success.next_topic.id == "TALK_MISSION_SUCCESS";
    }));
    talk_function::mission_success(giver);
    CHECK(job->in_progress());

    if (job->get_type().goal == MGOAL_GO_TO) {
        job->set_target(u.abs_omt_pos());
    } else {
        job->step_complete(1);
    }
    if (fail_mission) { job->fail(); }
    d.gen_responses(query);
    if (fail_mission) {
        REQUIRE(d.responses.size() == 1);
        CHECK(d.responses.front().success.apply(d).id == "TALK_MISSION_FAILURE");
        d.gen_responses(talk_topic("TALK_MISSION_FAILURE"));
        REQUIRE(d.responses.size() == 1);
        CHECK(d.responses.front().success.apply(d).id == "TALK_NONE");
        CHECK(u.get_failed_missions().size() == 1);
        CHECK(u.charges_of(merch) == 1);
    } else {
        const auto report = std::ranges::find_if(d.responses, [](const auto& response) {
            return response.success.next_topic.id == "TALK_MISSION_SUCCESS";
        });
        REQUIRE(report != d.responses.end());
        CHECK(report->success.apply(d).id == "TALK_MISSION_SUCCESS");
        talk_function::mission_success(giver);
        CHECK(u.get_completed_missions().size() == 1);
        CHECK(u.charges_of(merch) == 3);
        d.gen_responses(talk_topic("TALK_MISSION_SUCCESS"));
        REQUIRE(d.responses.size() == 1);
        CHECK(d.responses.front().success.apply(d).id == "TALK_NONE");
    }
    CHECK(u.get_active_missions().empty());
    CHECK(giver.chatbin.missions_assigned.empty());
    CHECK(giver.op_of_u.owed == 0);
}

/// Use Ruth's delivery and confirmation topics to verify that every food can be sold at its quote.
TEST_CASE("ruth_calder_quotes_and_transfers_bulk_food", "[npc_talk][quartermaster]") {
    const auto food = itype_id(GENERATE(
        "jerky", "meat_smoked", "fish_smoked", "dry_veggy", "dry_fruit", "cooking_oil",
        "cooking_oil2", "cornmeal", "flour", "fruit_wine", "beer", "sugar", "salt", "vinegar"));
    CAPTURE(food.str());
    g->faction_manager_ptr->create_if_needed();
    auto ruth = npc();
    ruth.set_fac(faction_id("free_merchants"));
    const auto cleanup = on_out_of_scope([&] { ruth.get_faction()->remove_member(ruth.getID()); });
    auto u = avatar();
    auto d = dialogue();
    d.alpha = &u;
    d.beta = &ruth;
    const auto merch = itype_id("FMCNote");
    const auto currency_value = static_cast<int>(item::spawn_temporary(merch)->price(true));
    const auto credit = GENERATE(0, 225);
    ruth.op_of_u.owed = credit;

    for (auto sale = 0; sale < 2; ++sale) {
        auto stock = item::spawn(food);
        const auto value = static_cast<int>(stock->price(true));
        const auto by_charges = stock->count_by_charges();
        const auto quantity = by_charges ? stock->charges : 1;
        const auto carried = [&](const auto& actor) {
            return by_charges ? actor.charges_of(food) : actor.amount_of(food);
        };
        if (stock->made_of(LIQUID)) {
            auto container = item::spawn(itype_id("jug_plastic"));
            container->put_in(std::move(stock));
            stock = std::move(container);
        }
        u.i_add(std::move(stock));
        REQUIRE(carried(u) == quantity);

        d.gen_responses(talk_topic("TALK_FREE_MERCHANT_STOCKS_DELIVER"));
        const auto option = std::ranges::find_if(d.responses, [&](const auto& response) {
            return response.success.next_topic.item_type == food;
        });
        REQUIRE(option != d.responses.end());
        const auto topic = option->success.next_topic;
        auto line = d.dynamic_line(topic);
        parse_tags(line, u, ruth, topic.item_type);
        const auto before_credit = ruth.op_of_u.owed;
        const auto before_currency = u.charges_of(merch);
        const auto before_stock = carried(ruth);
        const auto payment = (value + before_credit) / currency_value;
        const auto remaining = (value + before_credit) % currency_value;
        CHECK(line.find(string_format(" (%d),", quantity)) != std::string::npos);
        CHECK(line.find(string_format("I'll pay you %d Merch", payment)) != std::string::npos);
        if (remaining > 0) {
            CHECK(line.find(string_format("%d/%d of a Merch", remaining, currency_value))
                  != std::string::npos);
        }
        d.gen_responses(topic);
        REQUIRE(d.responses.size() == 2);
        CHECK(d.responses[1].success.apply(d).id == "TALK_NONE");
        CHECK(carried(u) == quantity);
        CHECK(carried(ruth) == before_stock);
        CHECK(ruth.op_of_u.owed == before_credit);

        d.responses[0].success.apply(d);
        CHECK(carried(u) == 0);
        CHECK(carried(ruth) == before_stock + quantity);
        CHECK(u.charges_of(merch) == before_currency + payment);
        CHECK(ruth.op_of_u.owed == remaining);
        d.responses[0].success.apply(d);
        CHECK(carried(ruth) == before_stock + quantity);
        CHECK(u.charges_of(merch) == before_currency + payment);
        CHECK(ruth.op_of_u.owed == remaining);
    }
}

/// Offer fewer than two batches of each jar size and verify that each accepted sale pays one Merch.
TEST_CASE("ruth_calder_buys_fixed_batches_of_jars", "[npc_talk][quartermaster]") {
    const auto large = GENERATE(false, true);
    const auto jar = itype_id(large ? "jar_3l_glass" : "jar_glass");
    const auto batch = large ? 5 : 10;
    auto ruth = npc();
    auto u = avatar();
    auto d = dialogue();
    d.alpha = &u;
    d.beta = &ruth;
    const auto merch = itype_id("FMCNote");
    const auto topic = talk_topic("TALK_FREE_MERCHANT_STOCKS_JARS");
    const auto is_sale = [&](const auto& response) {
        return response.success.next_topic.id == topic.id;
    };
    for (auto i = 0; i < batch - 1; ++i) { u.i_add(item::spawn(jar)); }
    d.gen_responses(topic);
    CHECK_FALSE(std::ranges::any_of(d.responses, is_sale));
    for (auto i = 0; i <= batch + 1; ++i) { u.i_add(item::spawn(jar)); }
    for (auto remaining = 1; remaining >= 0; --remaining) {
        d.gen_responses(topic);
        const auto sale = std::ranges::find_if(d.responses, is_sale);
        REQUIRE(sale != d.responses.end());
        sale->success.apply(d);
        CHECK(u.amount_of(jar) == remaining * batch + 1);
        CHECK(u.charges_of(merch) == 2 - remaining);
    }
    d.gen_responses(topic);
    CHECK_FALSE(std::ranges::any_of(d.responses, is_sale));
}

/// Check Milton's dialogue before and after the Back Bay turn-in, and after accepting the pen job.
TEST_CASE("milton_offers_incident_reports_after_back_bay", "[npc_talk][mission]") {
    g->faction_manager_ptr->create_if_needed();
    auto milton = npc();
    milton.load_npc_template(string_id<npc_template>("evac_security_chief"));
    const auto remove_milton = on_out_of_scope([&milton] {
        if (auto* faction = milton.get_faction()) { faction->remove_member(milton.getID()); }
    });

    auto player = avatar();
    auto d = dialogue();
    d.alpha = &player;
    d.beta = &milton;
    const auto has_pen_offer = [&]() {
        d.gen_responses(talk_topic(milton.chatbin.first_topic));
        return std::ranges::any_of(d.responses, [](const auto& response) {
            return response.success.next_topic.id == "TALK_EVAC_SECURITY_INCIDENT_REPORTS";
        });
    };

    CHECK_FALSE(has_pen_offer());
    player.set_value("npctalk_var_mission_flag_FMShopkeep_Mission1", "yes");
    CHECK(has_pen_offer());
    player.set_value("npctalk_var_evac_security_incident_reports_accepted", "yes");
    CHECK_FALSE(has_pen_offer());
}

/// Ask a generic guard for work to check that their referral names all four staff members.
TEST_CASE("refugee_center_guards_direct_players_to_staff", "[npc_talk]") {
    g->faction_manager_ptr->create_if_needed();
    auto guard = npc();
    guard.load_npc_template(string_id<npc_template>("guard"));
    const auto remove_guard = on_out_of_scope([&guard] {
        if (auto* guard_faction = guard.get_faction()) {
            guard_faction->remove_member(guard.getID());
        }
    });

    auto d = dialogue();
    d.alpha = &get_avatar();
    d.beta = &guard;
    const auto referral = talk_topic("TALK_GUARD_FM_STAFF");
    d.gen_responses(talk_topic(guard.chatbin.first_topic));
    REQUIRE(std::ranges::any_of(d.responses, [&referral](const auto& response) {
        return response.success.next_topic.id == referral.id;
    }));
    const auto directions = d.dynamic_line(referral);
    CHECK(directions.contains("Owen Mercer"));
    CHECK(directions.contains("Ruth Calder"));
    CHECK(directions.contains("Milton Lang"));
    CHECK(directions.contains("Mara Velez"));
}

TEST_CASE("npc_talk_stats", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    avatar& player_character = get_avatar();
    player_character.str_cur = 8;
    player_character.dex_cur = 8;
    player_character.int_cur = 8;
    player_character.per_cur = 8;

    d.add_topic("TALK_TEST_SIMPLE_STATS");
    gen_response_lines(d, 5);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a strength test response.");
    CHECK(d.responses[2].text == "This is a dexterity test response.");
    CHECK(d.responses[3].text == "This is an intelligence test response.");
    CHECK(d.responses[4].text == "This is a perception test response.");
    player_character.str_cur = 6;
    player_character.dex_cur = 6;
    player_character.int_cur = 6;
    player_character.per_cur = 6;
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");

    d.add_topic("TALK_TEST_NEGATED_STATS");
    gen_response_lines(d, 5);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a low strength test response.");
    CHECK(d.responses[2].text == "This is a low dexterity test response.");
    CHECK(d.responses[3].text == "This is a low intelligence test response.");
    CHECK(d.responses[4].text == "This is a low perception test response.");
}

TEST_CASE("npc_talk_skills", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    const skill_id skill("driving");

    avatar& player_character = get_avatar();
    player_character.set_skill_level(skill, 8);

    d.add_topic("TALK_TEST_SIMPLE_SKILLS");
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a driving test response.");

    player_character.set_skill_level(skill, 6);
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");

    d.add_topic("TALK_TEST_NEGATED_SKILLS");
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a low driving test response.");
}

TEST_CASE("npc_talk_wearing_and_trait", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);

    avatar& player_character = get_avatar();
    for (const trait_id& tr : player_character.get_mutations()) {
        player_character.unset_mutation(tr);
    }

    d.add_topic("TALK_TEST_WEARING_AND_TRAIT");
    gen_response_lines(d, 1);

    CHECK(d.responses[0].text == "This is a basic test response.");
    player_character.toggle_trait(trait_id("ELFA_EARS"));
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a trait test response.");
    CHECK(d.responses[2].text == "This is a short trait test response.");
    player_character.wear_item(item::spawn("badge_marshal"));
    gen_response_lines(d, 4);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a trait test response.");
    CHECK(d.responses[2].text == "This is a short trait test response.");
    CHECK(d.responses[3].text == "This is a wearing test response.");
    talker_npc.toggle_trait(trait_id("ELFA_EARS"));
    gen_response_lines(d, 6);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a trait test response.");
    CHECK(d.responses[2].text == "This is a short trait test response.");
    CHECK(d.responses[3].text == "This is a wearing test response.");
    CHECK(d.responses[4].text == "This is a npc trait test response.");
    CHECK(d.responses[5].text == "This is a npc short trait test response.");
    player_character.toggle_trait(trait_id("ELFA_EARS"));
    talker_npc.toggle_trait(trait_id("ELFA_EARS"));
    player_character.toggle_trait(trait_id("PSYCHOPATH"));
    talker_npc.toggle_trait(trait_id("SAPIOVORE"));
    gen_response_lines(d, 4);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a wearing test response.");
    CHECK(d.responses[2].text == "This is a trait flags test response.");
    CHECK(d.responses[3].text == "This is a npc trait flags test response.");
    player_character.toggle_trait(trait_id("PSYCHOPATH"));
    talker_npc.toggle_trait(trait_id("SAPIOVORE"));
}

TEST_CASE("npc_talk_effect", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);
    avatar& player_character = get_avatar();

    d.add_topic("TALK_TEST_EFFECT");
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    talker_npc.add_effect(effect_gave_quest_item, 9999_turns);
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is an npc effect test response.");
    player_character.add_effect(effect_gave_quest_item, 9999_turns);
    d.gen_responses(d.topic_stack.back());
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is an npc effect test response.");
    CHECK(d.responses[2].text == "This is a player effect test response.");
}

TEST_CASE("npc_talk_service", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);
    avatar& player_character = get_avatar();

    d.add_topic("TALK_TEST_SERVICE");
    player_character.cash = 0;
    talker_npc.add_effect(effect_currently_busy, 9999_turns);
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    player_character.cash = 800;
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a cash test response.");
    talker_npc.remove_effect(effect_currently_busy);
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a cash test response.");
    CHECK(d.responses[2].text == "This is an npc available test response.");
}

TEST_CASE("npc_talk_location", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    change_om_type("pond_swamp_north");
    d.add_topic("TALK_TEST_LOCATION");
    d.gen_responses(d.topic_stack.back());
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    change_om_type("field");
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a om_location_field test response.");
}

TEST_CASE("npc_talk_role", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);

    d.add_topic("TALK_TEST_NPC_ROLE");
    talker_npc.companion_mission_role_id = "NO_TEST_ROLE";
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    talker_npc.companion_mission_role_id = "TEST_ROLE";
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a nearby role test response.");
}

TEST_CASE("npc_talk_class", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);

    d.add_topic("TALK_TEST_NPC_CLASS");
    talker_npc.myclass = npc_class_id("NC_NONE");
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    talker_npc.myclass = npc_class_id("NC_TEST_CLASS");
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a class test response.");
}

TEST_CASE("npc_talk_allies", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);

    for (npc* guy : g->allies()) { talk_function::leave(*guy); }
    talk_function::follow(talker_npc);
    d.add_topic("TALK_TEST_NPC_ALLIES");
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a npc allies 1 test response.");
}

TEST_CASE("npc_talk_mind_control_clears_assigned_missions", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);
    avatar& player_character = get_avatar();

    mission* test_mission = nullptr;
    for (const mission_type& some_mission : mission_type::get_all()) {
        if (some_mission.goal == MGOAL_ASSASSINATE) {
            test_mission = mission::reserve_new(some_mission.id, talker_npc.getID());
            break;
        }
    }
    REQUIRE(test_mission != nullptr);

    if (test_mission->get_assigned_player_id() == player_character.getID()) {
        player_character.on_mission_assignment(*test_mission);
    } else {
        test_mission->assign(player_character);
    }
    talker_npc.chatbin.missions_assigned.push_back(test_mission);
    talker_npc.chatbin.mission_selected = test_mission;

    d.add_topic("TALK_MIND_CONTROL");
    CHECK(gen_dynamic_line(d) == "YES, MASTER!");
    CHECK(player_character.get_active_missions().empty());
    CHECK(player_character.get_failed_missions().size() == 1);
    CHECK(player_character.get_failed_missions().front() == test_mission);
    CHECK(talker_npc.chatbin.missions_assigned.empty());
    CHECK(talker_npc.chatbin.mission_selected == nullptr);
    CHECK(talker_npc.is_player_ally());
}

TEST_CASE("npc_talk_rules", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);

    d.add_topic("TALK_TEST_NPC_RULES");
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    talker_npc.rules.engagement = combat_engagement::ALL;
    talker_npc.rules.aim = aim_rule::SPRAY;
    talker_npc.rules.set_flag(ally_rule::use_silent);
    gen_response_lines(d, 4);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a npc engagement rule test response.");
    CHECK(d.responses[2].text == "This is a npc aim rule test response.");
    CHECK(d.responses[3].text == "This is a npc rule test response.");
    talker_npc.rules.clear_flag(ally_rule::use_silent);
}

TEST_CASE("npc_talk_needs", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);

    d.add_topic("TALK_TEST_NPC_NEEDS");
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    talker_npc.set_thirst(90);
    talker_npc.set_stored_kcal(100);
    talker_npc.set_fatigue(static_cast<int>(fatigue_levels::exhausted));
    gen_response_lines(d, 4);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a npc thirst test response.");
    CHECK(d.responses[2].text == "This is a npc hunger test response.");
    CHECK(d.responses[3].text == "This is a npc fatigue test response.");
}

TEST_CASE("npc_talk_mission_goal", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);

    d.add_topic("TALK_TEST_MISSION_GOAL");
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    const std::vector<mission_type>& all_missions = mission_type::get_all();
    bool set_mission = false;
    for (const mission_type& some_mission : all_missions) {
        if (some_mission.goal == MGOAL_ASSASSINATE) {
            mission* assassinate = mission::reserve_new(some_mission.id, talker_npc.getID());
            talker_npc.chatbin.mission_selected = assassinate;
            set_mission = true;
            break;
        }
    }
    CHECK(set_mission);
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a mission goal test response.");
}

TEST_CASE("npc_talk_season", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    const time_point old_calendar = calendar::turn;
    calendar::turn = calendar::start_of_cataclysm;
    d.add_topic("TALK_TEST_SEASON");
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a season spring test response.");
    calendar::turn += calendar::season_length();
    gen_response_lines(d, 4);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a days since Cataclysm 5 test response.");
    CHECK(d.responses[2].text == "This is a season summer test response.");
    CHECK(d.responses[3].text == "This is a days since Cataclysm 20 test response.");
    calendar::turn += calendar::season_length();
    gen_response_lines(d, 6);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a days since Cataclysm 5 test response.");
    CHECK(d.responses[2].text == "This is a days since Cataclysm 20 test response.");
    CHECK(d.responses[3].text == "This is a season autumn test response.");
    CHECK(d.responses[4].text == "This is a days since Cataclysm 35 test response.");
    CHECK(d.responses[5].text == "This is a days since Cataclysm 50 test response.");
    calendar::turn += calendar::season_length();
    gen_response_lines(d, 6);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a days since Cataclysm 5 test response.");
    CHECK(d.responses[2].text == "This is a days since Cataclysm 20 test response.");
    CHECK(d.responses[3].text == "This is a days since Cataclysm 35 test response.");
    CHECK(d.responses[4].text == "This is a season winter test response.");
    CHECK(d.responses[5].text == "This is a days since Cataclysm 50 test response.");
    calendar::turn += calendar::season_length();
    gen_response_lines(d, 6);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a season spring test response.");
    CHECK(d.responses[2].text == "This is a days since Cataclysm 5 test response.");
    CHECK(d.responses[3].text == "This is a days since Cataclysm 20 test response.");
    CHECK(d.responses[4].text == "This is a days since Cataclysm 35 test response.");
    CHECK(d.responses[5].text == "This is a days since Cataclysm 50 test response.");
    calendar::turn = old_calendar;
}

TEST_CASE("npc_talk_time", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    const time_point old_calendar = calendar::turn;
    calendar::turn = sunrise(calendar::turn) + 4_hours;
    d.add_topic("TALK_TEST_TIME");
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a is day test response.");
    calendar::turn = sunset(calendar::turn) + 2_hours;
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a is night test response.");
    calendar::turn = old_calendar;
}

TEST_CASE("npc_talk_switch", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);
    avatar& player_character = get_avatar();

    d.add_topic("TALK_TEST_SWITCH");
    player_character.cash = 1000;
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is an switch 1 test response.");
    CHECK(d.responses[2].text == "This is another basic test response.");
    player_character.cash = 100;
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is an switch 2 test response.");
    CHECK(d.responses[2].text == "This is another basic test response.");
    player_character.cash = 10;
    gen_response_lines(d, 4);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is an switch default 1 test response.");
    CHECK(d.responses[2].text == "This is an switch default 2 test response.");
    CHECK(d.responses[3].text == "This is another basic test response.");
    player_character.cash = 0;
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is an switch default 2 test response.");
    CHECK(d.responses[2].text == "This is another basic test response.");
}

TEST_CASE("npc_talk_or", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);
    avatar& player_character = get_avatar();

    d.add_topic("TALK_TEST_OR");
    player_character.cash = 0;
    talker_npc.add_effect(effect_currently_busy, 9999_turns);
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    player_character.toggle_trait(trait_id("ELFA_EARS"));
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is an or trait test response.");
}

TEST_CASE("npc_talk_and", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);
    avatar& player_character = get_avatar();

    player_character.toggle_trait(trait_id("ELFA_EARS"));
    d.add_topic("TALK_TEST_AND");
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    player_character.cash = 800;
    talker_npc.remove_effect(effect_currently_busy);
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is an and cash, available, trait test response.");
}

TEST_CASE("npc_talk_nested", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);
    avatar& player_character = get_avatar();

    d.add_topic("TALK_TEST_NESTED");
    talker_npc.add_effect(effect_currently_busy, 9999_turns);
    player_character.cash = 0;
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    player_character.cash = 800;
    player_character.int_cur = 11;
    gen_response_lines(d, 2);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a complex nested test response.");
}

TEST_CASE("npc_talk_conditionals", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    avatar& player_character = get_avatar();
    prep_test(d);
    player_character.cash = 800;

    d.add_topic("TALK_TEST_TRUE_FALSE_CONDITIONAL");
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a true/false true response.");
    CHECK(d.responses[2].text == "This is a conditional trial response.");
    talk_response& chosen = d.responses[2];
    bool trial_success = chosen.trial.roll(d);
    CHECK(trial_success == true);
    talk_effect_t& trial_effect = trial_success ? chosen.success : chosen.failure;
    CHECK(trial_effect.next_topic.id == "TALK_TEST_TRUE_CONDITION_NEXT");
    player_character.cash = 0;
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a true/false false response.");
    CHECK(d.responses[2].text == "This is a conditional trial response.");
    chosen = d.responses[2];
    trial_success = chosen.trial.roll(d);
    CHECK(trial_success == false);
    trial_effect = trial_success ? chosen.success : chosen.failure;
    CHECK(trial_effect.next_topic.id == "TALK_TEST_FALSE_CONDITION_NEXT");
}

static auto has_item(Character& p, const std::string& id, int count) -> bool {
    item& old_item = *item::spawn_temporary(id);
    if (old_item.count_by_charges()) {
        return p.has_charges(itype_id(id), count);
    } else {
        return p.has_amount(itype_id(id), count);
    }
}

static auto has_beer_bottle(Character& p, int count) -> bool {
    return has_item(p, "bottle_glass", 1) && has_item(p, "beer", count);
}

static void give_item(Character& p, const std::string& id, int count) {
    for (int i = 0; i < count; i++) { p.i_add(item::spawn(id)); }
}

TEST_CASE("npc_talk_effects_advanced", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);
    avatar& player_character = get_avatar();

    player_character.int_cur = 8;
    player_character.cash = 1000;
    player_character.remove_top_items_with([](detached_ptr<item>&& it) {
        return (it->get_category().get_id() == item_category_id("books")
                || it->get_category().get_id() == item_category_id("food")
                || it->get_category().get_id() == item_category_id("drugs")
                || it->typeId() == itype_id("bottle_glass"))
                 ? detached_ptr<item>()
                 : std::move(it);
    });

    SECTION("basic_has_item") {
        d.add_topic("TALK_TEST_HAS_ITEM");
        gen_response_lines(d, 1);
        CHECK(d.responses[0].text == "This is a basic test response.");
    }

    SECTION("basic_item_repeat") {
        d.add_topic("TALK_TEST_ITEM_REPEAT");
        gen_response_lines(d, 1);
        CHECK(d.responses[0].text == "This is a basic test response.");
    }

    SECTION("add_remove_effect") {
        d.add_topic("TALK_TEST_EFFECTS_CHAREFFECTS");
        gen_response_lines(d, 5);
        // add and remove effect
        REQUIRE_FALSE(player_character.has_effect(effect_infection));
        talk_effect_t& effects = d.responses[1].success;
        effects.apply(d);
        CHECK(player_character.has_effect(effect_infection));
        CHECK(player_character.get_effect_dur(effect_infection) == time_duration::from_turns(10));
        REQUIRE_FALSE(talker_npc.has_effect(effect_infection));
        effects = d.responses[2].success;
        effects.apply(d);
        CHECK(talker_npc.has_effect(effect_infection));
        CHECK(talker_npc.get_effect(effect_infection).is_permanent());
        effects = d.responses[3].success;
        effects.apply(d);
        CHECK_FALSE(player_character.has_effect(effect_infection));
        effects = d.responses[4].success;
        effects.apply(d);
        CHECK_FALSE(talker_npc.has_effect(effect_infection));
    }

    SECTION("add_remove_trait") {
        d.add_topic("TALK_TEST_EFFECTS_CHARTRAIT");
        gen_response_lines(d, 5);
        // add and remove trait
        REQUIRE_FALSE(player_character.has_trait(trait_PROF_FED));
        talk_effect_t& effects = d.responses[1].success;
        effects.apply(d);
        CHECK(player_character.has_trait(trait_PROF_FED));
        REQUIRE_FALSE(talker_npc.has_trait(trait_PROF_FED));
        effects = d.responses[2].success;
        effects.apply(d);
        CHECK(talker_npc.has_trait(trait_PROF_FED));
        effects = d.responses[3].success;
        effects.apply(d);
        CHECK_FALSE(player_character.has_trait(trait_PROF_FED));
        effects = d.responses[4].success;
        effects.apply(d);
        CHECK_FALSE(talker_npc.has_trait(trait_PROF_FED));
    }

    SECTION("buy_and_spend") {
        d.add_topic("TALK_TEST_EFFECTS_BUY_SPEND");
        gen_response_lines(d, 4);
        // buying and spending
        talker_npc.op_of_u.owed = 1000;
        REQUIRE_FALSE(has_beer_bottle(player_character, 2));
        REQUIRE(talker_npc.op_of_u.owed == 1000);
        REQUIRE(player_character.cash == 1000);
        talk_effect_t& effects = d.responses[1].success;
        effects.apply(d);
        CHECK(talker_npc.op_of_u.owed == 500);
        CHECK(has_beer_bottle(player_character, 2));
        REQUIRE_FALSE(has_item(player_character, "bottle_plastic", 1));
        effects = d.responses[2].success;
        effects.apply(d);
        CHECK(has_item(player_character, "bottle_plastic", 1));
        // No price specified means gift
        CHECK(talker_npc.op_of_u.owed == 500);
        effects = d.responses[3].success;
        effects.apply(d);
        // Ecash should NOT be used for trade or to settle a debt
        CHECK(player_character.cash == 500);
        CHECK(talker_npc.op_of_u.owed == 500);
    }

    SECTION("effect_chains") {
        d.add_topic("TALK_TEST_EFFECTS_CHAIN");
        gen_response_lines(d, 2);
        // effect chains
        REQUIRE_FALSE(player_character.has_effect(effect_infected));
        REQUIRE_FALSE(talker_npc.has_effect(effect_infected));
        REQUIRE_FALSE(player_character.has_trait(trait_PROF_SWAT));
        REQUIRE_FALSE(talker_npc.has_trait(trait_PROF_SWAT));
        talk_effect_t& effects = d.responses[1].success;
        effects.apply(d);
        CHECK(player_character.has_effect(effect_infected));
        CHECK(player_character.get_effect_dur(effect_infected) == time_duration::from_turns(10));
        CHECK(talker_npc.has_effect(effect_infected));
        CHECK(talker_npc.get_effect(effect_infected).is_permanent());
        CHECK(player_character.has_trait(trait_PROF_SWAT));
        CHECK(talker_npc.has_trait(trait_PROF_SWAT));
        CHECK(talker_npc.op_of_u.owed == 0);
        CHECK(talker_npc.get_attitude() == NPCATT_KILL);
    }

    SECTION("opinion") {
        d.add_topic("TALK_TEST_EFFECTS_OPINION");
        gen_response_lines(d, 2);
        // opinion changes
        talker_npc.op_of_u = npc_opinion();
        REQUIRE_FALSE(talker_npc.op_of_u.trust);
        REQUIRE_FALSE(talker_npc.op_of_u.fear);
        REQUIRE_FALSE(talker_npc.op_of_u.value);
        REQUIRE_FALSE(talker_npc.op_of_u.anger);
        REQUIRE_FALSE(talker_npc.op_of_u.owed);
        talk_effect_t& effects = d.responses[1].success;
        REQUIRE(effects.opinion.trust == 10);
        effects.apply(d);
        CHECK(talker_npc.op_of_u.trust == 10);
        CHECK(talker_npc.op_of_u.fear == 11);
        CHECK(talker_npc.op_of_u.value == 12);
        CHECK(talker_npc.op_of_u.anger == 13);
        CHECK(talker_npc.op_of_u.owed == 14);
    }

    SECTION("has_item") {
        give_item(player_character, "beer", 2);
        give_item(player_character, "bottle_glass", 1);
        give_item(player_character, "manual_speech", 1);
        give_item(player_character, "dnd_handbook", 1);

        d.add_topic("TALK_TEST_HAS_ITEM");
        gen_response_lines(d, 7);
        CHECK(d.responses[0].text == "This is a repeated item manual_speech test response");
        CHECK(d.responses[0].success.next_topic.item_type == itype_id("manual_speech"));
        CHECK(d.responses[1].text == "This is a basic test response.");
        CHECK(d.responses[2].text == "This is a u_has_item beer test response.");
        CHECK(d.responses[3].text == "This is a u_has_item bottle_glass test response.");
        CHECK(d.responses[4].text == "This is a u_has_items beer test response.");
        CHECK(d.responses[5].text == "This is a u_has_item_category books test response.");
        CHECK(d.responses[6].text == "This is a u_has_item_category books count 2 test response.");
    }

    SECTION("item_repeat") {
        give_item(player_character, "beer", 2);
        give_item(player_character, "bottle_glass", 1);
        give_item(player_character, "manual_speech", 1);
        give_item(player_character, "dnd_handbook", 1);

        d.add_topic("TALK_TEST_ITEM_REPEAT");
        gen_response_lines(d, 8);
        CHECK(d.responses[0].text == "This is a repeated category books, food test response");
        CHECK(d.responses[0].success.next_topic.item_type == itype_id("beer"));
        CHECK(d.responses[1].text == "This is a repeated category books, food test response");
        CHECK(d.responses[1].success.next_topic.item_type == itype_id("dnd_handbook"));
        CHECK(d.responses[2].text == "This is a repeated category books, food test response");
        CHECK(d.responses[2].success.next_topic.item_type == itype_id("manual_speech"));
        CHECK(d.responses[3].text == "This is a repeated category books test response");
        CHECK(d.responses[3].success.next_topic.item_type == itype_id("dnd_handbook"));
        CHECK(d.responses[4].text == "This is a repeated category books test response");
        CHECK(d.responses[4].success.next_topic.item_type == itype_id("manual_speech"));
        CHECK(d.responses[5].text == "This is a repeated item beer, bottle_glass test response");
        CHECK(d.responses[5].success.next_topic.item_type == itype_id("bottle_glass"));
        CHECK(d.responses[6].text == "This is a repeated item beer, bottle_glass test response");
        CHECK(d.responses[6].success.next_topic.item_type == itype_id("beer"));
        CHECK(d.responses[7].text == "This is a basic test response.");
    }

    SECTION("sell_and_consume") {
        give_item(player_character, "beer", 2);
        give_item(player_character, "bottle_glass", 1);
        give_item(player_character, "bottle_plastic", 1);

        // test sell and consume
        d.add_topic("TALK_TEST_EFFECTS_SELL_CONSUME");
        gen_response_lines(d, 5);
        REQUIRE(player_character.cash == 1000);
        REQUIRE(has_item(player_character, "bottle_plastic", 1));
        REQUIRE(has_beer_bottle(player_character, 2));
        REQUIRE(player_character.wield(player_character.i_at(
            player_character.inv_position_by_type(itype_id("bottle_glass")))));
        talk_effect_t& effects = d.responses[1].success;
        effects.apply(d);
        CHECK_FALSE(has_item(player_character, "bottle_plastic", 1));
        CHECK_FALSE(has_item(player_character, "beer", 1));
        CHECK(has_item(talker_npc, "bottle_plastic", 1));
        CHECK(has_item(talker_npc, "beer", 2));
        effects = d.responses[2].success;
        effects.apply(d);
        CHECK_FALSE(has_item(talker_npc, "beer", 2));
        CHECK(has_item(talker_npc, "beer", 1));
        effects = d.responses[3].success;
        effects.apply(d);
        CHECK(has_item(player_character, "beer", 1));
        effects = d.responses[4].success;
        effects.apply(d);
        CHECK(has_item(player_character, "beer", 0));
        CHECK_FALSE(has_item(player_character, "beer", 1));
        // Ecash should NOT be used for trade
        REQUIRE(player_character.cash == 1000);
    }
}

TEST_CASE("npc_talk_combat_commands", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    d.add_topic("TALK_COMBAT_COMMANDS");
    gen_response_lines(d, 10);
    CHECK(d.responses[0].text == "Change your engagement rules…");
    CHECK(d.responses[1].text == "Change your aiming rules…");
    CHECK(d.responses[2].text == "Stick close to me, no matter what.");
    CHECK(d.responses[3].text == "Follow me about four paces away.");
    CHECK(d.responses[4].text == "Don't use ranged weapons anymore.");
    CHECK(d.responses[5].text == "Use only silent weapons.");
    CHECK(d.responses[6].text == "Don't use grenades anymore.");
    CHECK(d.responses[7].text == "Don't worry about shooting an ally.");
    CHECK(d.responses[8].text == "Hold the line: don't move onto obstacles adjacent to me.");
    CHECK(d.responses[9].text == "Never mind.");
}

TEST_CASE("npc_talk_vars", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    d.add_topic("TALK_TEST_VARS");
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a u_add_var test response.");
    CHECK(d.responses[2].text == "This is a npc_add_var test response.");
    talk_effect_t& effects = d.responses[1].success;
    effects.apply(d);
    effects = d.responses[2].success;
    effects.apply(d);
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a u_has_var, u_remove_var test response.");
    CHECK(d.responses[2].text == "This is a npc_has_var, npc_remove_var test response.");
    effects = d.responses[1].success;
    effects.apply(d);
    effects = d.responses[2].success;
    effects.apply(d);
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a u_add_var test response.");
    CHECK(d.responses[2].text == "This is a npc_add_var test response.");
}

TEST_CASE("npc_talk_adjust_vars", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    prep_test(d);

    d.add_topic("TALK_TEST_ADJUST_VARS");

    // At the starting point, the var hasn't been set or adjusted, so it should default to 0.
    gen_response_lines(d, 11);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a u_adjust_var test response that increments by 1.");
    CHECK(d.responses[2].text == "This is a u_adjust_var test response that decrements by 1.");
    CHECK(d.responses[3].text == "This is a npc_adjust_var test response that increments by 1.");
    CHECK(d.responses[4].text == "This is a npc_adjust_var test response that decrements by 1.");
    CHECK(d.responses[5].text == "This is a u_compare_var test response for == 0.");
    CHECK(d.responses[6].text == "This is a u_compare_var test response for <= 0.");
    CHECK(d.responses[7].text == "This is a u_compare_var test response for >= 0.");
    CHECK(d.responses[8].text == "This is a npc_compare_var test response for == 0.");
    CHECK(d.responses[9].text == "This is a npc_compare_var test response for <= 0.");
    CHECK(d.responses[10].text == "This is a npc_compare_var test response for >= 0.");

    // Increment the u and npc vars by 1, so that it has a value of 1.
    talk_effect_t& effects = d.responses[1].success;
    effects.apply(d);
    effects = d.responses[3].success;
    effects.apply(d);

    // Now we're comparing the var, which should be 1, to our condition value which is 0.
    gen_response_lines(d, 11);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a u_adjust_var test response that increments by 1.");
    CHECK(d.responses[2].text == "This is a u_adjust_var test response that decrements by 1.");
    CHECK(d.responses[3].text == "This is a npc_adjust_var test response that increments by 1.");
    CHECK(d.responses[4].text == "This is a npc_adjust_var test response that decrements by 1.");
    CHECK(d.responses[5].text == "This is a u_compare_var test response for != 0.");
    CHECK(d.responses[6].text == "This is a u_compare_var test response for >= 0.");
    CHECK(d.responses[7].text == "This is a u_compare_var test response for > 0.");
    CHECK(d.responses[8].text == "This is a npc_compare_var test response for != 0.");
    CHECK(d.responses[9].text == "This is a npc_compare_var test response for >= 0.");
    CHECK(d.responses[10].text == "This is a npc_compare_var test response for > 0.");

    // Decrement the u and npc vars by 1 twice, so that it has a value of -1.
    effects = d.responses[2].success;
    effects.apply(d);
    effects.apply(d);
    effects = d.responses[4].success;
    effects.apply(d);
    effects.apply(d);

    // Now we're comparing the var, which should be -1, to our condition value which is 0.
    gen_response_lines(d, 11);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a u_adjust_var test response that increments by 1.");
    CHECK(d.responses[2].text == "This is a u_adjust_var test response that decrements by 1.");
    CHECK(d.responses[3].text == "This is a npc_adjust_var test response that increments by 1.");
    CHECK(d.responses[4].text == "This is a npc_adjust_var test response that decrements by 1.");
    CHECK(d.responses[5].text == "This is a u_compare_var test response for != 0.");
    CHECK(d.responses[6].text == "This is a u_compare_var test response for <= 0.");
    CHECK(d.responses[7].text == "This is a u_compare_var test response for < 0.");
    CHECK(d.responses[8].text == "This is a npc_compare_var test response for != 0.");
    CHECK(d.responses[9].text == "This is a npc_compare_var test response for <= 0.");
    CHECK(d.responses[10].text == "This is a npc_compare_var test response for < 0.");
}

TEST_CASE("npc_talk_bionics", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);
    avatar& player_character = get_avatar();

    player_character.clear_bionics();
    talker_npc.clear_bionics();
    d.add_topic("TALK_TEST_BIONICS");
    gen_response_lines(d, 1);
    CHECK(d.responses[0].text == "This is a basic test response.");
    player_character.add_bionic(bionic_id("bio_ads"));
    talker_npc.add_bionic(bionic_id("bio_power_storage"));
    gen_response_lines(d, 3);
    CHECK(d.responses[0].text == "This is a basic test response.");
    CHECK(d.responses[1].text == "This is a u_has_bionics bio_ads test response.");
    CHECK(d.responses[2].text == "This is a npc_has_bionics ANY response.");
}

TEST_CASE("npc_talk_effects", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);
    avatar& player_character = get_avatar();

    // Speaker effects test use cash for simplicity
    player_character.cash = 10000;
    CHECK(player_character.cash == 10000);

    SECTION("simple") {
        d.add_topic("TALK_TEST_SPEAKER_EFFECT_SIMPLE");
        d.apply_speaker_effects(d.topic_stack.back());
        REQUIRE(player_character.cash == 9500);
        d.apply_speaker_effects(d.topic_stack.back());
        REQUIRE(player_character.cash == 9000);

        SECTION("simple_conditional") {
            d.add_topic("TALK_TEST_SPEAKER_EFFECT_SIMPLE_CONDITIONAL");
            d.apply_speaker_effects(d.topic_stack.back());
            REQUIRE(player_character.cash == 8500);
            d.apply_speaker_effects(d.topic_stack.back());
            REQUIRE(player_character.cash == 8500);
        }
    }

    SECTION("sentinel") {
        d.add_topic("TALK_TEST_SPEAKER_EFFECT_SENTINEL");
        d.apply_speaker_effects(d.topic_stack.back());
        REQUIRE(player_character.cash == 9500);
        d.apply_speaker_effects(d.topic_stack.back());
        REQUIRE(player_character.cash == 9500);

        SECTION("sentinel_conditional") {
            d.add_topic("TALK_TEST_SPEAKER_EFFECT_SENTINEL_CONDITIONAL");
            d.apply_speaker_effects(d.topic_stack.back());
            REQUIRE(player_character.cash == 9000);
            d.apply_speaker_effects(d.topic_stack.back());
            REQUIRE(player_character.cash == 9000);
        }
    }

    SECTION("compound") {
        d.add_topic("TALK_TEST_SPEAKER_EFFECT_COMPOUND");
        d.apply_speaker_effects(d.topic_stack.back());
        REQUIRE(player_character.cash == 9000);
        d.apply_speaker_effects(d.topic_stack.back());
        REQUIRE(player_character.cash == 8000);

        SECTION("compound_conditional") {
            d.add_topic("TALK_TEST_SPEAKER_EFFECT_COMPOUND_CONDITIONAL");
            d.apply_speaker_effects(d.topic_stack.back());
            REQUIRE(player_character.cash == 6500);
            d.apply_speaker_effects(d.topic_stack.back());
            REQUIRE(player_character.cash == 6000);
        }
    }

    SECTION("compound_sentinel") {
        d.add_topic("TALK_TEST_SPEAKER_EFFECT_COMPOUND_SENTINEL");
        d.apply_speaker_effects(d.topic_stack.back());
        REQUIRE(player_character.cash == 9250);
        d.apply_speaker_effects(d.topic_stack.back());
        REQUIRE(player_character.cash == 8750);

        SECTION("sentinel_conditional") {
            d.add_topic("TALK_TEST_SPEAKER_EFFECT_COMPOUND_SENTINEL_CONDITIONAL");
            d.apply_speaker_effects(d.topic_stack.back());
            REQUIRE(player_character.cash == 8000);
            d.apply_speaker_effects(d.topic_stack.back());
            REQUIRE(player_character.cash == 8000);
        }
    }

    SECTION("change_npc_class") {
        // test change class
        talker_npc.myclass = npc_class_id("NC_TEST_CLASS");
        d.add_topic("TALK_TEST_EFFECTS_NPC_CLASS");
        gen_response_lines(d, 2);
        talk_effect_t& effects = d.responses[1].success;
        effects.apply(d);
        CHECK(talker_npc.myclass == npc_class_id("NC_NONE"));
    }
}

TEST_CASE("npc_talk_effect_change_first_topic", "[npc_talk]") {
    clear_all_state();
    dialogue d;
    npc& talker_npc = prep_test(d);

    d.add_topic("TALK_TEST_CHANGE_FIRST_TOPIC_1");
    gen_response_lines(d, 2);
    talk_effect_t& effect_1 = d.responses[1].success;
    effect_1.apply(d);
    CHECK(talker_npc.chatbin.first_topic == "TALK_TEST_CHANGE_FIRST_TOPIC_2");
    d.add_topic("TALK_TEST_CHANGE_FIRST_TOPIC_2");
    gen_response_lines(d, 2);
    talk_effect_t& effect_2 = d.responses[1].success;
    effect_2.apply(d);
    CHECK(talker_npc.chatbin.first_topic == "TALK_TEST_CHANGE_FIRST_TOPIC_1");
}
