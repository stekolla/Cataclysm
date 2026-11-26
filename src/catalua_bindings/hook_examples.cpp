#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna.h"
#include "catalua_luna_doc.h"

auto cata::detail::reg_hooks_examples(sol::state& lua) -> void {
    DOC("Documentation for hooks");
    luna::userlib lib = luna::begin_lib(lua, "hooks");

    DOC("Called when game is about to save.");
    luna::set_fx(lib, "on_game_save", []() {});

    DOC("Called right after game has loaded.");
    luna::set_fx(lib, "on_game_load", []() {});

    DOC("Called when the game has first started.");
    luna::set_fx(lib, "on_game_started", []() {});

    DOC("Called when the weather has changed.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `weather_id` (string): Current weather ID  ");
    DOC("* `old_weather_id` (string): Previous weather ID  ");
    DOC("* `temperature` (float): Current temperature in Celsius  ");
    DOC("* `temperature_f` (float): Current temperature in Fahrenheit  ");
    DOC("* `windspeed` (float): Wind speed  ");
    DOC("* `winddirection` (integer): Wind direction in degrees  ");
    DOC("* `humidity` (float): Humidity percentage  ");
    DOC("* `pressure` (float): Atmospheric pressure  ");
    DOC("* `is_sheltered` (boolean): Whether player is sheltered  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_weather_changed", [](const sol::table&) {});

    DOC("Called every 5 minutes when weather data is updated.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `weather_id` (string): Current weather ID  ");
    DOC("* `temperature` (float): Current temperature in Celsius  ");
    DOC("* `temperature_f` (float): Current temperature in Fahrenheit  ");
    DOC("* `windspeed` (float): Wind speed  ");
    DOC("* `winddirection` (integer): Wind direction in degrees  ");
    DOC("* `humidity` (float): Humidity percentage  ");
    DOC("* `pressure` (float): Atmospheric pressure  ");
    DOC("* `is_sheltered` (boolean): Whether player is sheltered  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_weather_updated", [](const sol::table&) {});

    DOC("Called when the player tries to interact with an NPC.  ");
    DOC("Return false to prevent the npc interaction menu from appearing.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (NPC): The NPC being interacted with  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_try_npc_interaction", [](const sol::table&) {});

    DOC("Called when the player interacts with an NPC.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (NPC): The NPC being interacted with  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_npc_interaction", [](const sol::table&) {});

    DOC("Called when the player tries to interact with a monster.  ");
    DOC("Return false to prevent monster interaction actions from running.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `monster` (Monster): The monster being interacted with  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_try_monster_interaction", [](const sol::table&) {});

    DOC("Called when building the examine menu of a pet monster.  ");
    DOC("Return an array of tables with `menu_id` (string) and `menu_label` (string) to add menu entries.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `avatar` (Avatar)  ");
    DOC("* `monster` (Monster)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_monster_get_examine_menu_entries", [](const sol::table&) {});

    DOC("Called when an entry is selected in the examine menu of a pet monster.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `avatar` (Avatar)  ");
    DOC("* `monster` (Monster)  ");
    DOC("* `entry` (string): The selected entry id, either built-in or a `menu_id` added by `on_monster_get_examine_menu_entries`  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_monster_examine_menu_entry", [](const sol::table&) {});

    DOC("Called when a monster is tamed by a character.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `avatar` (Character): The character who tamed the monster  ");
    DOC("* `monster` (Monster)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_monster_tame", [](const sol::table&) {});

    DOC("Called when the player swaps control to an npc.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (NPC): The NPC being controlled.  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_control_npc", [](const sol::table&) {});

    DOC("Called just before the dialogue window opens and the first topic is chosen.  ");
    DOC("Return a new talk_topic id to add it as the next topic.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (NPC): The NPC speaking  ");
    DOC("* `next_topic` (string): The topic that will be shown first  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_dialogue_start", [](const sol::table&) {});

    DOC("Called when a dialogue option is selected.  ");
    DOC("Return a new talk_topic id to add it as the next topic.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (NPC): The NPC speaking  ");
    DOC("* `next_topic` (string): The topic that was selected  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_dialogue_option", [](const sol::table&) {});

    DOC("Called when the dialogue window closes.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (NPC): The NPC speaking  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_dialogue_end", [](const sol::table&) {});

    DOC("Called when a character or monster successfully dodges.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC("* `source` (Creature)  ");
    DOC("* `difficulty` (integer)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_creature_dodged", [](const sol::table&) {});

    DOC("Called when a character successfully blocks.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC("* `source` (Creature)  ");
    DOC("* `bodypart_id` (BodyPartTypeId)  ");
    DOC("* `damage_instance` (DamageInstance)  ");
    DOC("* `damage_blocked` (float)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_creature_blocked", [](const sol::table&) {});

    DOC("Called when a character has performed a technique.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC("* `technique` (MartialArtsTechniqueRaw)  ");
    DOC("* `target` (Creature)  ");
    DOC("* `damage_instance` (DamageInstance)  ");
    DOC("* `move_cost` (integer)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_creature_performed_technique", [](const sol::table&) {});

    DOC("Called after a character or monster has attacked in melee.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC("* `target` (Creature)  ");
    DOC("* `success` (bool)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_creature_melee_attacked", [](const sol::table&) {});

    DOC("Called after a character has attacked a creature in melee or hit it with a projectile.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC("* `target` (Creature)  ");
    DOC("* `success` (bool)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_creature_attacked_by_character", [](const sol::table&) {});

    DOC("Called every turn for each active creature, before it acts.  ");
    DOC("This is the base hook; `on_monster_do_turn` and `on_npc_do_turn` also trigger this.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `creature` (Creature)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_creature_do_turn", [](const sol::table&) {});

    DOC("Called every turn for each active monster, before it acts.  ");
    DOC("Also triggers `on_creature_do_turn`.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `monster` (Monster)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_monster_do_turn", [](const sol::table&) {});

    DOC("Called every turn for each simulated NPC, before it acts.  ");
    DOC("Also triggers `on_creature_do_turn`.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (Npc)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_npc_do_turn", [](const sol::table&) {});

    DOC("Called when the player attempts to move.  ");
    DOC("All registered callbacks run; if any returns false, movement is blocked.  ");
    DOC("Return false to block the move.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `player` (Player)  ");
    DOC("* `from` (TripointBubMs)  ");
    DOC("* `to` (TripointBubMs)  ");
    DOC("* `movement_mode` (CharacterMoveMode)  ");
    DOC("* `via_ramp` (bool)  ");
    DOC("* `mounted` (bool)  ");
    DOC("* `mount` (Creature, if mounted)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_player_try_move", [](const sol::table&) {});

    DOC("Called when an NPC attempts to move.  ");
    DOC("All registered callbacks run; if any returns false, movement is blocked.  ");
    DOC("Return false to block the move.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (Npc)  ");
    DOC("* `from` (TripointBubMs)  ");
    DOC("* `to` (TripointBubMs)  ");
    DOC("* `movement_mode` (CharacterMoveMode)  ");
    DOC("* `via_ramp` (bool)  ");
    DOC("* `mounted` (bool)  ");
    DOC("* `mount` (Creature, if mounted)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_npc_try_move", [](const sol::table&) {});

    DOC("Called when a monster attempts to move.  ");
    DOC("All registered callbacks run; if any returns false, movement is blocked.  ");
    DOC("Return false to block the move.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `monster` (Monster)  ");
    DOC("* `from` (TripointBubMs)  ");
    DOC("* `to` (TripointBubMs)  ");
    DOC("* `force` (bool): If the monster move call was forced  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_monster_try_move", [](const sol::table&) {});

    DOC("Called before the player uses elevator controls.  ");
    DOC("All registered callbacks run; if any returns false, elevator use is blocked.  ");
    DOC("Return false to block elevator use.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `player` (Player)  ");
    DOC("* `pos` (TripointBubMs)  ");
    DOC("* `om_terrain` (string)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_elevator_try_use", [](const sol::table&) {});

    DOC("Called after on_player_try_move or on_npc_try_move regardless of whether the specific hook vetoed.  ");
    DOC("All registered callbacks run; if any returns false, movement is blocked.  ");
    DOC("Return false to block the move.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC("* `from` (TripointBubMs)  ");
    DOC("* `to` (TripointBubMs)  ");
    DOC("* `movement_mode` (CharacterMoveMode)  ");
    DOC("* `via_ramp` (bool)  ");
    DOC("* `mounted` (bool)  ");
    DOC("* `mount` (Creature, if mounted)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_try_move", [](const sol::table&) {});

    DOC("Called when character stat gets reset.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `character` (Character)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_reset_stats", [](const sol::table&) {});

    DOC("Called when a character wakes up from sleep.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_wake_up", [](const sol::table&) {});

    DOC("Called when a character tries to wear an item.  ");
    DOC("Return false to block wearing the item.  ");
    DOC("Set `params.results.allowed = false` and `params.results.message` (string), then return `params.results` to block it with a custom message.  ");
    DOC("Once a callback blocks, the remaining callbacks registered to this hook are not called.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `who` (Character)  ");
    DOC("* `item` (Item)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_try_wear", [](const sol::table&) {});

    DOC("Called when a character tries to take off a worn item.  ");
    DOC("Return false to block taking off the item.  ");
    DOC("Set `params.results.allowed = false` and `params.results.message` (string), then return `params.results` to block it with a custom message.  ");
    DOC("Once a callback blocks, the remaining callbacks registered to this hook are not called.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `who` (Character)  ");
    DOC("* `item` (Item)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_try_takeoff", [](const sol::table&) {});

    DOC("Called after a character has put on an item.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `who` (Character)  ");
    DOC("* `item` (Item)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_item_wear", [](const sol::table&) {});

    DOC("Called after a character has taken off an item.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `who` (Character)  ");
    DOC("* `item` (Item)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_item_takeoff", [](const sol::table&) {});

    DOC("Called when a craft is completed, and when the crafting menu builds a preview of the result.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `crafter` (Character)  ");
    DOC("* `craft` (Item, nil in the crafting menu): The in-progress craft item  ");
    DOC("* `item` (Item): The resulting item, or its contents if the result is a container  ");
    DOC("* `recipe` (RecipeRaw)  ");
    DOC("* `batch_size` (int)  ");
    DOC("* `hot_result` (bool): True if the result is food and the recipe requires a heating tool (e.g. `surface_heat`)  ");
    DOC("* `dehydrated_result` (bool): True if the result is food and the recipe requires a dehydrator or a charcoal smoker  ");
    DOC("* `crafting_menu` (bool): True if called for the crafting menu preview  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_craft_result", [](const sol::table&) {});

    DOC("Called when the player tries to cast a spell, before the casting activity starts.  ");
    DOC("Return false to block casting.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Avatar)  ");
    DOC("* `spell` (Spell)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_spell_try_cast", [](const sol::table&) {});

    DOC("Called after a spell has been cast.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC("* `spell` (Spell)  ");
    DOC("* `target_pos` (TripointBubMs)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_spell_cast", [](const sol::table&) {});

    DOC("Called when a skill is confirmed on the character display (`@`) skill tab.  ");
    DOC("Set `params.results.handled = true` to prevent the default training toggle.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `character` (Character)  ");
    DOC("* `skill` (SkillId)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_display_skill_action", [](const sol::table&) {});

    DOC("Called when drawing skill info on the character display (`@`) skill tab.  ");
    DOC("Set `params.results.text` to append text below the regular skill description.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `character` (Character)  ");
    DOC("* `skill` (SkillId)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_display_skill_info", [](const sol::table&) {});

    DOC("Called when character gets the effect which has `EFFECT_LUA_ON_ADDED` flag.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC("* `effect` (Effect)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_effect_added", [](const sol::table&) {});

    DOC("Called when character is on the effect which has `EFFECT_LUA_ON_TICK` flag.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `character` (Character)  ");
    DOC("* `effect` (Effect)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_effect", [](const sol::table&) {});

    DOC("Called when character loses the effect which has `EFFECT_LUA_ON_REMOVED` flag.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `character` (Character)  ");
    DOC("* `effect` (Effect)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_effect_removed", [](const sol::table&) {});

    DOC("Called when a character is dead.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `char` (Character)  ");
    DOC("* `killer` (Creature)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_character_death", [](const sol::table&) {});

    DOC("Called when shot(s) is fired from a gun.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `shooter` (Character)  ");
    DOC("* `target_pos` (TripointBubMs)  ");
    DOC("* `shots` (int)  ");
    DOC("* `gun` (item)  ");
    DOC("* `ammo` (item): For `RELOAD_AND_SHOOT` guns like a bow. On the others, it returns `nil` value.  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_shoot", [](const sol::table&) {});

    DOC("Called when an item is thrown.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `thrower` (Character)  ");
    DOC("* `target_pos` (TripointBubMs)  ");
    DOC("* `throw_from_pos` (TripointBubMs)  ");
    DOC("* `thrown` (item)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_throw", [](const sol::table&) {});

    DOC("Called when monster gets the effect which has `EFFECT_LUA_ON_ADDED` flag.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `mon` (Monster)  ");
    DOC("* `effect` (Effect)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_mon_effect_added", [](const sol::table&) {});

    DOC("Called when character is on the effect which has `EFFECT_LUA_ON_TICK` flag.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `mon` (Monster)  ");
    DOC("* `effect` (Effect)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_mon_effect", [](const sol::table&) {});

    DOC("Called when monster loses the effect which has `EFFECT_LUA_ON_REMOVED` flag.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `mon` (Monster)  ");
    DOC("* `effect` (Effect)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_mon_effect_removed", [](const sol::table&) {});

    DOC("Called when a monster is dead.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `mon` (Monster)  ");
    DOC("* `killer` (Creature)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_mon_death", [](const sol::table&) {});

    DOC("Called when any creature is spawned for the first time.  ");
    DOC("This is the base hook; `on_monster_spawn` and `on_npc_spawn` also trigger this.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `creature` (Creature)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_creature_spawn", [](const sol::table&) {});

    DOC("Called when a monster is spawned for the first time.  ");
    DOC("Also triggers `on_creature_spawn`.  ");
    DOC("Note: monsters spawned via mapgen submap spawn points will fire this hook,  ");
    DOC("but monsters materialized from overmap monster groups may not.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `monster` (Monster)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_monster_spawn", [](const sol::table&) {});

    DOC("Called when an NPC is spawned for the first time.  ");
    DOC("Also triggers `on_creature_spawn`.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (Npc)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_npc_spawn", [](const sol::table&) {});

    DOC("Called when any creature is loaded onto the active map.  ");
    DOC("This is the base hook; `on_monster_loaded` and `on_npc_loaded` also trigger this.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `creature` (Creature)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_creature_loaded", [](const sol::table&) {});

    DOC("Called when a monster is loaded onto the active map.  ");
    DOC("Also triggers `on_creature_loaded`.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `monster` (Monster)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_monster_loaded", [](const sol::table&) {});

    DOC("Called when an NPC is loaded onto the active map.  ");
    DOC("Also triggers `on_creature_loaded`.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `npc` (Npc)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_npc_loaded", [](const sol::table&) {});

    DOC("Called every in-game period.  ");
    DOC("Not registered through `game.add_hook`; use `gapi.add_on_every_x_hook( interval, fn )` instead.  ");
    DOC("The callback receives no arguments. Return `false` to unregister it.");
    luna::set_fx(lib, "on_every_x", []() {});

    DOC("Called when an explosion starts.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `pos` (TripointBubMs)  ");
    DOC("* `damage` (int)  ");
    DOC("* `radius` (int)  ");
    DOC("* `fire` (bool)  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_explosion_start", [](const sol::table&) {});

    DOC("Called right after mapgen has completed.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `map` (MapgenConstructor): The OMT-local mapgen surface.  ");
    DOC("* `omt` (TripointAbsOmt): The absolute overmap terrain position.  ");
    DOC("* `when` (TimePoint): The current time (for time-based effects).  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_mapgen_postprocess", [](const sol::table&) {});

    DOC("Called when checking which mapgen ids are used, during mapgen consistency checks.  ");
    DOC("Insert mapgen ids that are only used from Lua to suppress the \"not used by anything\" error.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `results` (set of string): The mapgen ids that are in use  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_make_mapgen_factory_list", [](const sol::table&) {});

    DOC("Called right after mission has started.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `mission_type` (mission_type): The type of the mission.  ");
    DOC("* `mission` (mission): The mission instance.  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_mission_start", [](const sol::table&) {});

    DOC("Called right after mission has ended.  ");
    DOC("The hook receives a table with keys:  ");
    DOC("* `mission_type` (mission_type): The type of the mission.  ");
    DOC("* `mission` (mission): The mission instance.  ");
    DOC("* `success` (bool): Successful if true else failed.  ");
    DOC_PARAMS("params");
    luna::set_fx(lib, "on_mission_end", [](const sol::table&) {});

    luna::finalize_lib(lib);
}
