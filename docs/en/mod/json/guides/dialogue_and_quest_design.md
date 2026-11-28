# Authoring NPC dialogue and quests

Use this guide to turn a quest outline into mission data, dialogue, and world changes. Start
with [Designing Quests and NPC Stories](../../../dev/design/quests_and_npc_stories.md) if you
are still deciding what should make the player curious, what they should learn, or why this
character would ask for help. A working mission proves that its mechanics run, not that the
story is clear or worth playing.

Use the [mission JSON reference](../reference/creatures/missions_json.md) and
[NPC dialogue reference](../reference/creatures/npcs.md) for exact syntax and supported behavior.

## Plan the quest's states

For each scene, record how the player reaches it, what action or condition moves the quest
forward, and what the player can see afterward. Sketch the offer, active objective, completion,
and any refusal or failure paths before writing dialogue branches. Note which services and
other quests should remain available in each state.

Check whether a player can arrive with the work partly done, lose a required item, or find a
contact unavailable. Decide which cases should still allow completion. If a failure should
close off later work, encode that deliberately rather than relying on an unreachable topic.

Boris needs five volunteers to clean up the Refugee Center's back bay. The offer and advice
point to guards, merchants, and refugees. Players can find different people to meet the same
goal; the author does not need a separate mission for each recruitment route.

## Choose existing mechanisms

Map each player action and visible consequence to the simplest supported behavior that
preserves it. An incidental conversation can use talk topics without a mission.

| Requirement | Mechanism to check |
| --- | --- |
| Record information or decisions | Player and NPC dialogue variables |
| Complete after a condition established across conversations | `MGOAL_CONDITION` with dialogue conditions and persistent state |
| Show different dialogue by quest state | Dialogue conditions and talk topics |
| Offer persuasion, intimidation, or deception | Dialogue trials |
| Reveal skill-dependent information | Skill conditions and dialogue |
| Recover evidence | Mission-specific items |
| Send the player to an authored location | `assign_mission_target` |
| Place an encounter | Mission mapgen |
| Track a combat objective | Existing monster and NPC mission goals |
| Change a location | `update_mapgen` |
| Change a merchant's stock | NPC class changes |
| Change relationships | NPC opinion and faction effects |
| Deliver a reward | Mission completion effects |

If mission completion already records the state you need, check it instead of adding a second
flag. A new service at one settlement does not need a simulated settlement economy, and an NPC
remembering a favor does not need a generic reputation system. If no existing mechanism
preserves the intended experience, [revisit the design or justify the missing capability](../../../dev/design/quests_and_npc_stories.md#13-let-the-experience-drive-the-implementation).

## Implement the offer and conversation

Keep trading, available work, and progress reports close to the main conversation. Group
optional background questions under clear topics and give each submenu a way back. Use
shared responses where appropriate without forcing NPCs into the same conversational template.
Keep the actual accept-or-decline choice; skip extra confirmations that add no decision.
Don't offer reputation or reward negotiation if it changes neither the outcome nor the reaction.

Gate each line on the event that makes it true. At the Refugee Center, Milton's clearing
mission and the opening of Boris's workshop are separate milestones. Accepting a job is not
the same as completing it. Check what residents say between milestones, and keep unaffected
topics and services accessible.

Read both sides of the exchange in order. Check that the player's response makes sense after
the line they actually see and that return visits don't replay introductions unnecessarily.

## Keep the objective clear in the journal

Dialogue conveys why the work matters and what the NPC is promising. The journal lets players
resume after a break. Give them the objective, a place to start, any required quantities or
conditions, whom to report to, and the promised outcome. Avoid giving away discoveries the
player is meant to make on site.

For example, Dana can say:

> My family kept that starter alive for generations. I left it at the bakery when I ran.
> Bring it back, and in a couple of weeks I'll have better bread to sell you.

The journal can give directions without repeating the family history:

> Find one jar of Dana's family sourdough starter in the right-hand fridge at the marked
> bakery. Return it to Dana at the Refugee Center.
>
> Reward: Dana sells improved sourdough after two weeks.

## Implement rewards and lasting changes

Grant the promised reward on completion rather than hiding it behind a second dialogue
option. Say before acceptance whether a service costs money or only becomes available later.
If stock changes at the next restock, explain the delay. Don't describe a facility as
operational if the player can't use what it was built to provide. For cash or faction currency,
use the [merchant economy guide](postapoc_price_guide.md) to check what the payment buys.

Keep ordinary trade separate from mission rewards. At the Occupied Lumbermill, the manager
sells lumber from the start. The jobs improve the crew's quarters, machinery, and perimeter.
Tacoma's foreman can mark the mill as an optional lead; the player can still bring planks from
any source.

The Scrap Yard's salvage trade is also open from the start. Its trader's retrieval jobs place
their items only at the selected warehouse and military helipad. The repaired rifle is a
one-time quest reward, not part of the trader's stock.

## Verify the mechanics in game

Use the [testing guide](../../../dev/guides/testing.md) and the
[dialogue validator](../reference/creatures/npcs.md#validating-dialogues). The validator checks
topic links, not whether the player can complete the quest or understand it. Use existing
repository patterns for data validation and relevant gameplay tests.

Play through the offer, objective, turn-in, and return visits. Verify that:

- The offer appears when intended, with no unnecessary prerequisites.
- The journal and markers point to the right objective, and supported alternative routes work.
- Refusal, failure, and out-of-order actions leave the intended dialogue and services available.
- The turn-in recognizes the right state and delivers the reward and world changes as promised.
- Subsequent visits show the correct dialogue and make any new service usable at the expected time.

A separate [player-experience review](../../../dev/design/quests_and_npc_stories.md#step-8-play-through-the-experience)
asks whether players understood the situation, found it worth pursuing, and noticed what changed.
