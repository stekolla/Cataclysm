#include "avatar.h"
#include "calendar.h"
#include "catch/catch.hpp"
#include "faction.h"
#include "game.h"
#include "item.h"
#include "npc.h"
#include "npctrade.h"
#include "options.h"
#include "state_helpers.h"

#include <algorithm>

namespace {

const auto barter = skill_id("barter");

struct merchant_test {
    npc merchant;
    avatar& player = get_avatar();

    explicit merchant_test(const char* template_id = "evac_merchant") {
        clear_all_state();
        g->faction_manager_ptr->create_if_needed();
        merchant.load_npc_template(string_id<npc_template>(template_id));
        merchant.remove_primary_weapon();
        merchant.clear_skills();
        merchant.str_cur = merchant.dex_cur = merchant.int_cur = merchant.per_cur = 8;
        merchant.set_stored_kcal(merchant.max_stored_kcal());
        merchant.set_thirst(0);
        merchant.set_skill_level(barter, 4);
        player.int_cur = 8;
        player.set_skill_level(barter, 4);
    }

    ~merchant_test() {
        merchant.get_faction()->remove_member(merchant.getID());
        clear_all_state();
    }

    auto quotes() -> npc_trading::trade_state {
        auto state = npc_trading::trade_state{};
        npc_trading::setup_trade_state(state, 0, merchant);
        return state;
    }
};

auto give(Character& owner, detached_ptr<item> object) -> item& {
    object->set_owner(owner);
    return owner.i_add(std::move(object), false);
}

auto line_for(auto& quotes, const char* type) -> decltype(quotes.begin()) {
    return std::ranges::find_if(quotes, [id = itype_id(type)](const auto& line) {
        return line.locs.front()->typeId() == id;
    });
}

auto quote_for(const auto& quotes, const char* type, int quantity = 1) -> int {
    CAPTURE(type, quantity);
    const auto line = line_for(quotes, type);
    REQUIRE(line != quotes.end());
    REQUIRE(std::max(line->count, line->charges) >= quantity);
    return static_cast<int>(line->price * quantity);
}

} // namespace

/// Check that the scrapper buys salvage and restocks basic goods without selling the quest reward.
TEST_CASE("scrap_trader_salvage_stock", "[npc][trade][pricing][merchant]") {
    const auto restore_turn = restore_on_out_of_scope(calendar::turn);
    auto fixture = merchant_test("NPC_scrap_trader");
    auto& [trader, player] = fixture;
    calendar::turn += 1_days;
    give(player, item::spawn("scrap"));
    const auto check_stock = [&] {
        const auto state = fixture.quotes();
        CHECK(quote_for(state.theirs, "scrap", 60) > 0);
        CHECK(quote_for(state.theirs, "motor_small") > 0);
        CHECK(quote_for(state.theirs, "circuit") > 0);
        CHECK(quote_for(state.theirs, "wrench") > 0);
        CHECK(quote_for(state.yours, "scrap") < quote_for(state.theirs, "scrap"));
        CHECK(line_for(state.theirs, "rebar_railgun") == state.theirs.end());
    };
    check_stock();
    trader.shop_restock();
    trader.inv_clear();
    calendar::turn += 3_days * get_option<float>("RESTOCK_DELAY_MULT");
    trader.shop_restock();
    check_stock();
}

/// Check that a new manager can supply Tacoma's full lumber order, and that normal restocking
/// replenishes the shop only after the cooldown.
TEST_CASE("lumbermill_bulk_stock_and_restock", "[npc][trade][pricing][merchant]") {
    const auto restore_turn = restore_on_out_of_scope(calendar::turn);
    auto fixture = merchant_test("NPC_lumbermill_merchant");
    auto& [manager, player] = fixture;
    calendar::turn += 1_days;
    REQUIRE(manager.is_shopkeeper());
    give(player, item::spawn("2x4"));

    const auto check_stock = [&] {
        const auto state = fixture.quotes();
        const auto planks = quote_for(state.theirs, "2x4", 200);
        CHECK(planks <= 50 * 250);
        CHECK(quote_for(state.yours, "2x4") < planks / 200);
        CHECK(quote_for(state.theirs, "log") > 0);
        CHECK(quote_for(state.theirs, "wood_beam") > 0);
        CHECK(quote_for(state.theirs, "wood_panel") > 0);
        CHECK(quote_for(state.theirs, "wood_sheet") > 0);
    };

    check_stock();
    manager.shop_restock();
    manager.inv_clear();
    manager.shop_restock();
    auto empty = fixture.quotes();
    CHECK(line_for(empty.theirs, "2x4") == empty.theirs.end());

    calendar::turn += 3_days * get_option<float>("RESTOCK_DELAY_MULT");
    manager.shop_restock();
    check_stock();
}

/// Check the live quote in each direction and immediately resell an actual purchase.
TEST_CASE("owen_merchandise_spread_and_currency", "[npc][trade][pricing][merchant]") {
    auto fixture = merchant_test();
    auto& [owen, player] = fixture;
    owen.inv_clear();
    give(owen, item::spawn("hammer"));
    give(owen, item::spawn("FMCNote"));
    give(player, item::spawn("FMCNote"));
    give(player, item::spawn("bat"));
    auto state = fixture.quotes();
    const auto purchase = line_for(state.theirs, "hammer");
    REQUIRE(purchase != state.theirs.end());
    purchase->selected = true;
    purchase->u_has = 1;
    npc_trading::transfer_items(state.theirs, owen, player, true);
    give(owen, item::spawn("hammer"));
    give(owen, item::spawn("wrench"));

    state = fixture.quotes();
    CHECK(quote_for(state.theirs, "hammer") == 1050);
    CHECK(quote_for(state.yours, "hammer") == 450);
    CHECK(quote_for(state.theirs, "wrench") == 650);
    CHECK(quote_for(state.yours, "bat") == 300);
    for (const auto skill : {0, 4, 20}) {
        for (const auto intelligence : {1, 8, 20}) {
            CAPTURE(skill, intelligence);
            player.set_skill_level(barter, skill);
            player.int_cur = intelligence;
            state = fixture.quotes();
            const auto ask = quote_for(state.theirs, "hammer");
            const auto bid = quote_for(state.yours, "hammer");
            CHECK(ask >= 1050);
            CHECK(ask <= 1260);
            CHECK(bid >= 450);
            CHECK(bid <= 540);
            CHECK(bid < ask);
            if (skill == 0 && intelligence == 1) { CHECK(ask == 1260); }
            if (skill == 20 && intelligence == 20) { CHECK(bid == 540); }
            CHECK(quote_for(state.theirs, "FMCNote") == 250);
            CHECK(quote_for(state.yours, "FMCNote") == 250);
        }
    }
}

/// Compare worn tools, partial stacks, and default packaging against their normal quotes.
TEST_CASE("owen_quotes_preserve_item_condition_and_quantity", "[npc][trade][pricing][merchant]") {
    auto fixture = merchant_test();
    auto& [owen, player] = fixture;
    owen.inv_clear();
    auto& hammer = give(owen, item::spawn("hammer"));
    give(owen, item::spawn("nail", calendar::turn, 200));
    give(player, item::spawn("nail", calendar::turn, 50));
    give(owen, item::in_its_container(item::spawn("superglue")));
    auto state = fixture.quotes();
    const auto fresh = quote_for(state.theirs, "hammer");
    CHECK(quote_for(state.theirs, "nail", 100) == 350);
    CHECK(quote_for(state.yours, "nail", 50) == 75);
    CHECK(quote_for(state.theirs, "squeeze_tube") == 1000);
    hammer.set_damage(hammer.max_damage() / 2);
    CHECK(quote_for(fixture.quotes().theirs, "hammer") < fresh);
}

/// Ensure two common guns and other out-of-specialty items stay out of both trade lists.
TEST_CASE("owen_restricts_trade_to_hardware", "[npc][trade][pricing][merchant]") {
    auto fixture = merchant_test();
    auto& [owen, player] = fixture;
    owen.inv_clear();
    for (const auto* id : {"m9", "glock_19", "9mm", "glockmag", "laser_sight", "jerky"}) {
        CAPTURE(id);
        give(owen, item::spawn(id));
        give(player, item::spawn(id));
        const auto state = fixture.quotes();
        CHECK(line_for(state.theirs, id) == state.theirs.end());
        CHECK(line_for(state.yours, id) == state.yours.end());
    }
    auto carton = item::spawn("box_small");
    carton->put_in(item::spawn("glock_19"));
    give(player, std::move(carton));
    const auto boxed = fixture.quotes();
    CHECK(line_for(boxed.yours, "box_small") == boxed.yours.end());

    give(player, item::spawn("kevlar"));
    const auto before = fixture.quotes();
    auto sale = line_for(before.yours, "kevlar");
    REQUIRE(sale != before.yours.end());
    give(owen, item::spawn(*sale->locs.front()));
    give(owen, item::spawn("improvised_pipebomb"));
    const auto after = fixture.quotes();
    CHECK(line_for(after.theirs, "kevlar") == after.theirs.end());
    CHECK(line_for(after.theirs, "improvised_pipebomb") == after.theirs.end());
}

/// Deplete the shelves between visits and check the cadence, bulk staples and reward budgets.
TEST_CASE("owen_restocks_staples_and_reward_purchases", "[npc][trade][pricing][merchant]") {
    auto fixture = merchant_test();
    auto& [owen, player] = fixture;
    player.set_skill_level(barter, 0);
    const auto delay = 3_days * get_option<float>("RESTOCK_DELAY_MULT");
    const auto check_offerings = [&] {
        owen.drop_invalid_inventory();
        const auto retail = fixture.quotes().theirs;
        CHECK(quote_for(retail, "2x4", 10) <= 5 * 250);
        CHECK(quote_for(retail, "nail", 200) + quote_for(retail, "pipe", 3) <= 5 * 250);
        const auto medium = line_for(retail, "ax") != retail.end() ? "ax" : "machete";
        CHECK(quote_for(retail, medium) + quote_for(retail, "2x4", 5) <= 15 * 250);
        const auto major =
            line_for(retail, "toolbox") != retail.end() ? "toolbox"
            : line_for(retail, "small_repairkit") != retail.end()
                ? "small_repairkit"
                : "misc_repairkit";
        CHECK(quote_for(retail, major) + quote_for(retail, "nail", 200) <= 30 * 250);
        CHECK(
            std::ranges::count_if(
                retail,
                [](const auto& line) {
                    const auto& id = line.locs.front()->typeId();
                    return id == itype_id("welder") || id == itype_id("jack")
                        || id == itype_id("generator_7500w") || id == itype_id("alternator_truck");
                })
            <= 1);
    };
    check_offerings();
    auto state = fixture.quotes();
    const auto planks = line_for(state.theirs, "2x4");
    REQUIRE(planks != state.theirs.end());
    planks->selected = true;
    if (planks->charges) {
        planks->u_charges = 2;
    } else {
        planks->u_has = 2;
    }
    npc_trading::transfer_items(state.theirs, owen, player, true);
    const auto owned_planks = player.items_with([&](const auto& it) {
        return it.typeId() == itype_id("2x4") && it.is_owned_by(player);
    });
    CHECK(std::ranges::fold_left(
              owned_planks, 0, [](int count, const auto* it) { return count + it->count(); })
          == 2);
    calendar::turn += 1_days;
    for (auto visit = 0; visit < 3; ++visit) {
        owen.shop_restock();
        check_offerings();
        owen.inv_clear();
        owen.shop_restock();
        const auto empty = fixture.quotes();
        CHECK(line_for(empty.theirs, "2x4") == empty.theirs.end());
        calendar::turn += delay;
    }
}

/// Compare a cheap crafting recipe's real inputs with the bid, and check a legacy merchant.
TEST_CASE("owen_crafting_and_unprofiled_merchants", "[npc][trade][pricing][merchant]") {
    SECTION("A bought plank and six nails cost more than the completed nailboard") {
        auto fixture = merchant_test();
        give(fixture.player, item::spawn("nailboard"));
        const auto state = fixture.quotes();
        CHECK(quote_for(state.yours, "nailboard")
              < quote_for(state.theirs, "2x4") + quote_for(state.theirs, "nail", 6));
    }
    SECTION("A Tacoma scavenger still uses personal valuation and honors faction currency") {
        auto fixture = merchant_test("ranch_scavenger_1");
        auto& [trader, player] = fixture;
        trader.inv_clear();
        auto& gun = give(player, item::spawn("glock_19"));
        give(trader, item::spawn("FMCNote"));
        auto state = fixture.quotes();
        CHECK(quote_for(state.yours, "glock_19") == trader.value(gun));
        CHECK(quote_for(state.theirs, "FMCNote") == 250);
    }
}
