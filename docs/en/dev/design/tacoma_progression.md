# Tacoma: housing and agriculture

The foreman's first two requests still enclose and furnish the barn. The third accepts
40 seeds of any one of six staple crops and prepares both field blocks. It also brings
in the first farmer and opens trade with the Crop Overseer. The existing Refugee Center
introduction and optional Occupied Lumbermill lead are unchanged.

The Overseer uses standard curated stock, Merch, and normal shop restocking. Wheat,
corn, beans, and seed potatoes are always stocked. Fresh produce is optional, ages
normally, and is replaced on restock. Later upgrades should keep the staple collection
and add separate stock tiers when the relevant project is complete. If using the existing
`npc_change_class` dialogue effect to select a class with a larger collection, account
for when the next normal restock will make that collection visible.

## Existing saves

- Saves before the third quest play the new sequence. The first two rewards are not
  retroactive.
- An active nail quest keeps its mission ID but now checks the new group and quantity.
  Nails are not accepted. The standard item-group objective checks one item type at a
  time, so separate crop types do not add up to 40.
- If the old third quest was completed but the fourth was not, accept the salt quest and
  return to the foreman's main greeting. It adds the missing rows and Overseer once.
  If you turn the salt in without visiting that greeting, the next greeting still applies
  the legacy update.
- If the old fourth quest was already completed, its fields and Overseer stay in place.
  The existing Overseer changes class on conversation and stocks goods at the next
  eligible trade restock. This does not replace dead or missing NPCs.

## Remaining foreman work

| Stage | Delivery | Existing effects to preserve in later restructuring |
| --- | --- | --- |
| 4 | Salt | Flatbed, sawmill start, palisade |
| 5 | Liquid fertilizer | Car chassis, sawmill and roof, second farmer, palisades |
| 6 | Rocks | Sawmill and barn work |
| 7 | Pipes | Well, sawmill, western palisade, more field space |
| 8 | Electric motors | Finish sawmill and western palisade, till more soil, start tool shed and outhouse |
| 9 | Bleach | Finish tool shed and outhouse, start clinic |
| 10 | First aid kits | Clinic and roof, start vehicle workshop |
| 11 | Arc welders | Finish clinic, workshop and roof, supporting site work |
| 12 | Car batteries | Finish vehicle workshop, start scavenger shop, supporting site work |
| 13 | Two-way radios | Scavenger shop and roof, barber, supporting site work |
| 14 | Backpacks | Finish scavenger shop, start bar, supporting site work |
| 15 | Homebrewer's Bible | Bar and roof |
| 16 | Sugar | Finish bar, start greenhouse |
| 17 | Glass sheets | Finish greenhouse and roof |

Salt and fertilizer still advance unrelated construction. Clean water, clinic, workshops,
commerce, and greenhouse access still depend on the linear chain. Field expansion and
greenhouse stock upgrades are follow-up work, not part of this phase.
