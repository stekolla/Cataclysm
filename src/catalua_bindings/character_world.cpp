#include "catalua_bindings/character_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "character.h"
#include "craft_command.h"
#include "crafting.h"
#include "creature.h"
#include "game.h"
#include "inventory.h"
#include "make_static.h"
#include "map/map.h"
#include "morale_types.h"
#include "recipe.h"
#include "requirements.h"
#include "trap.h"
#include "type_id.h"

namespace cata::detail {
auto reg_character_world_and_crafting(sol::usertype<Character>& ut) -> void {
#define UT_CLASS Character
    // Respawn Stuff
    SET_FX_T(drop_inv, void(const int count));

    DOC("Drops all items (inventory, worn, wielded) at the character's current position.");
    luna::set_fx(ut, "drop_all_items", [](Character& ch) -> void {
        std::vector<detached_ptr<item>> tmp = ch.inv_dump_remove();
        map& here = get_map();
        for (auto& itm : tmp) { here.add_item_or_charges(ch.bub_pos(), std::move(itm)); }
    });

    DOC("Returns how exposed each body part is through worn items, from 0.0 (fully covered) to 1.0 (bare).");
    SET_FX(bodypart_exposure);

    luna::set_fx(
        ut, "drench",
        [](UT_CLASS& c, const int saturation, const std::vector<bodypart_id>& parts,
           const bool ignore_waterproof) {
            auto drenched_parts = body_part_set();
            drenched_parts.fill(parts);
            c.drench(saturation, drenched_parts, ignore_waterproof);
        });

    luna::set_fx(
        ut, "use_charges",
        sol::overload(
            [](UT_CLASS& c, const itype_id& what, int qty) -> std::vector<detached_ptr<item>> {
                return c.use_charges(what, qty);
            },
            [](UT_CLASS& c, const itype_id& what, int qty,
               const sol::function& filter) -> std::vector<detached_ptr<item>> {
                return c.use_charges(what, qty, [&filter](const item& it) { return filter(it); });
            }));
    SET_FX(use_charges_if_avail);

    DOC("Returns the crafting inventory for this character (includes nearby items)");
    luna::set_fx(ut, "crafting_inventory", [](UT_CLASS& ch) -> const inventory& {
        return ch.crafting_inventory(tripoint_bub_ms::zero(), PICKUP_RANGE, true);
    });

    DOC("Invalidates the cached crafting inventory");
    SET_FX_T(invalidate_crafting_inventory, void());

    DOC("Consumes a requirement's item components from the inventory and nearby map. Tools and qualities are not used. Returns false if the player cancels a component choice.  ");
    DOC("Takes a table with keys:  ");
    DOC("* `range` (integer, optional): Radius for nearby map items. Defaults to the pickup range.  ");
    DOC("* `count` (integer, optional): Number of times to consume the requirement. Defaults to 1.");
    luna::set_fx(
        ut, "consume_requirement",
        [](UT_CLASS& ch, const requirement_data& req, const sol::table& opts) -> bool {
            int range = opts.get_or("range", PICKUP_RANGE);
            int size = opts.get_or("count", 1);
            inventory map_inv;
            map_inv.form_from_map(ch.bub_pos(), range);
            for (const auto& it : req.get_components()) {
                auto chosen = ch.select_item_component(it, size, map_inv, true);
                if (chosen.use_from == usage_from::cancel) { return false; }
                ch.consume_items(chosen, size);
            }
            return true;
        });

    DOC("Consumes items from inventory based on item component list");
    luna::set_fx(
        ut, "consume_items", [](UT_CLASS& ch, const std::vector<item_comp>& components) -> void {
            ch.consume_items(components);
        });

    DOC("Consumes tool charges from inventory based on tool component list");
    luna::set_fx(ut, "consume_tools", [](UT_CLASS& ch, const std::vector<tool_comp>& tools) -> void {
        ch.consume_tools(tools);
    });

    DOC("Gets the bonus from the enchantment value. Doesn't handle max logic itself.");
    luna::set_fx(
        ut, "bonus_from_enchantments",
        [](UT_CLASS& ch, const double base, const enchantment_value_id& ench_val_id,
           sol::optional<bool> round) -> double {
            return ch.bonus_from_enchantments(base, ench_val_id, round.value_or(false));
        });
    SET_FX(has_enchantment_flag);

    SET_FX(add_perk);
    SET_FX(remove_perk);
    SET_FX(has_perk);
#undef UT_CLASS
}
} // namespace cata::detail
