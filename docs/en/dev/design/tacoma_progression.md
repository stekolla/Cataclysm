# Tacoma: housing and agriculture

The foreman's first two requests still enclose and furnish the barn. The third accepts
40 seeds of any one of six staple crops and prepares both field blocks. It also brings
in the first farmer and opens trade with the Crop Overseer. The existing Refugee Center
introduction and optional Occupied Lumbermill lead are unchanged.

The Overseer uses standard curated stock, Merch, and normal shop restocking. Wheat,
corn, beans, and seed potatoes are always stocked. Fresh produce is optional, ages
normally, and is replaced on restock. Phase 2 keeps the staple collection and uses
`npc_change_class` to select an expanded collection, which becomes visible on the next
normal restock.

## Phase 2 projects

After Prepare the Fields the foreman requests 50 steel pipes for a well and clean-water pump
(24 Merch), then two motors to complete the sawmill (18 Merch). The sawmill supports Tacoma
itself; the lumberjack's existing service is unchanged. The next foreman mission remains
stage 9. In parallel, the Crop Overseer requests 30 units of liquid fertilizer (5 Merch).
The expanded stock uses the same prices and appears at the next normal restock.

| Legacy stages | New owner and location |
| --- | --- |
| 5–7: pit, covered well, pump and fireplaces | Secure Clean Water installs the finished pump on the existing sidewalk apron (58), with the original fireplaces and logs (66). |
| 5, 7–8: second farmer, plot clearance and rows | Improve the Fields adds the rows (65) and second farmer (66), independently of the foreman. |
| 4–8: sawmill stages and roof | Complete the Sawmill places the final layout (58) and roof (58 z+1), without the temporary frames and destructive tile clearing. The mill gets the woodworker; the original lumberjack stays at the barn. |
| 4–8: vehicles, defenses, tool shed, outhouse | Complete the Sawmill preserves these updates (56, 59, 66, 67, 68). Vehicle placement skips obstructed footprints rather than merging with parked vehicles. |

The former stages 4–8 are removed. Only the new project route is supported.

## Remaining foreman work

| Stage | Delivery | Existing effects to preserve in later restructuring |
| --- | --- | --- |
| 9 | Bleach | Finish tool shed and outhouse, start clinic |
| 10 | First aid kits | Clinic and roof, start vehicle workshop |
| 11 | Arc welders | Finish clinic, workshop and roof, supporting site work |
| 12 | Car batteries | Finish vehicle workshop, start scavenger shop, supporting site work |
| 13 | Two-way radios | Scavenger shop and roof, barber, supporting site work |
| 14 | Backpacks | Finish scavenger shop, start bar, supporting site work |
| 15 | Homebrewer's Bible | Bar and roof |
| 16 | Sugar | Finish bar, start greenhouse |
| 17 | Glass sheets | Finish greenhouse and roof |

Clinic, workshop, commerce, and greenhouse development still follow the existing linear
chain. Their quest ownership and greenhouse stock upgrades are follow-up work.
