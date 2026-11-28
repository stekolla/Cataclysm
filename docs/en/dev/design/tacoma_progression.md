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
itself; the lumberjack's existing service is unchanged. The foreman then opens the clinic.
In parallel, the Crop Overseer requests 30 units of liquid fertilizer (5 Merch).
The expanded stock uses the same prices and appears at the next normal restock.

| Legacy stages | New owner and location |
| --- | --- |
| 5–7: pit, covered well, pump and fireplaces | Secure Clean Water installs the finished pump on the existing sidewalk apron (58), with the original fireplaces and logs (66). |
| 5, 7–8: second farmer, plot clearance and rows | Improve the Fields adds the rows (65) and second farmer (66), independently of the foreman. |
| 4–8: sawmill stages and roof | Complete the Sawmill places the final layout (58) and roof (58 z+1), without the temporary frames and destructive tile clearing. The mill gets the woodworker; the original lumberjack stays at the barn. |
| 4–8: vehicles, defenses, tool shed, outhouse | Complete the Sawmill preserves these updates (56, 59, 66, 67, 68). Vehicle placement skips obstructed footprints rather than merging with parked vehicles. |

The former stages 4–8 are removed. Only the new project route is supported.

## Phase 3: Medical care

- **Open the Clinic:** Six matching first-aid kits, of any of the three accepted kinds,
  earn 72 Merch. The foreman opens the furnished southern clinic and its roof on tile 50.
  The nurse offers paid treatment at the existing 80/200 Merch rates and trades in
  routine medical supplies.
- **Expand Medical Care:** Three hotplates earn 5 Merch. The nurse opens the furnished
  northern ward and its roof, and the dispensary gets more supplies at the next restock.
- **Recruit a Physician:** One anesthesia kit earns 24 Merch. Kits are found in doctors'
  offices and labs, or can be crafted at first aid 3. The physician provides the existing
  implant installation and removal services.

The foreman can offer the workshop project immediately after opening the clinic, regardless
of the nurse's quests.

The old foreman stages 9–11 and ten nurse deliveries are retired. As in Phase 2, saves on
the superseded Tacoma routes are not migrated.

## Phase 4: Workshops and commerce

- **Equip the Vehicle Workshop:** Two arc welders earn 24 Merch. The project finishes the
  workshop and its roof, supplies the work area, and brings in the Scrapper. The Scrapper
  trades in ordinary mechanical parts and tools; there is no NPC repair service.
- **Establish Scavenger Operations:** Two two-way radios earn 8 Merch. The foreman opens
  the furnished depot and its roof. The Scavenger Boss trades in practical secondhand goods
  immediately and the foreman can move on to the bar.
- **Equip the Scavenging Teams:** Five wearable flashlights earn 3 Merch. This optional
  delivery adds stock at the depot's next normal restock, without affecting foreman quests.

| Former owner | New owner |
| --- | --- |
| Open the Clinic: workshop shell and roof, tool shed, outhouse, parked vehicles | Workshop project; completed workshop replaces the intermediate shell |
| Foreman 12: completed workshop and van | Workshop project |
| Foreman 12–14: depot construction, roof, barber and remaining parked vehicles | Scavenger operations project |
| Foreman 14: initial bar construction | Foreman 15, before its existing bar and roof updates |

Phase 4 supports fresh games only. Old Tacoma routes and already-spawned merchants are not
migrated, including saves at the previous clinic, workshop, shop, or bar handoffs.

## Remaining foreman work

| Stage | Delivery | Existing effects to preserve in later restructuring |
| --- | --- | --- |
| 15 | Homebrewer's Bible | Bar and roof |
| 16 | Sugar | Finish bar, start greenhouse |
| 17 | Glass sheets | Finish greenhouse and roof |

Phase 5 can replace the existing bar and greenhouse sequence and reassign its construction
updates. The scavenger stock upgrade is independent.
