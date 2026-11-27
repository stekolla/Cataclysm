# Balancing item prices and merchant economies

Scavenging should remain rewarding, trading should save time, and quests should earn the player
something worth spending. Favor predictable prices and curated stock over simulated supply and
demand, inflation, or logistics. A merchant should provide another way to get supplies, not make
scavenging or crafting unnecessary.

## Set the reference value first

[`price_postapoc`](../reference/items/item_creation.md#generic-items) is a shared reference value,
not the price a shop charges or the amount it pays for loot. Do not derive it from pre-Cataclysm
retail prices. The loader falls back to `price` when no `price_postapoc` is set or inherited,
so give tradable items an explicit post-Cataclysm value when that fallback would misprice them.
Compare the item to plausible substitutes and ask:

- **What can it do?** A tool that unlocks repairs or construction can be worth much more than a
  common weapon or a pile of raw material.
- **How easy is it to replace?** Look at scavenging locations, recipes, and alternative tools.
  Scarcity alone does not make an item valuable. Common materials should be inexpensive, but
  enough to finish a project should be worth buying.
- **What condition is it in?** Compare a usable item with damaged, incomplete, empty, or partly
  used versions. For food, compare nutrition and storage life as well as availability. Preserved
  food is generally more useful than equally nutritious food that spoils quickly, and medicine
  without a practical substitute can command a premium.

Use the in-game quantity when making comparisons. For charge-counted items, the JSON price is
for `stack_size` charges, not necessarily the quantity that spawns. For example, bandages are
priced at 200 cents for nine charges, but a default stack of three has a reference value of about
67 cents. A bottle or tube can also change what the trade window values or accepts: compare
packaged liquids with packaged liquids, not with loose contents. Check the actual quote; the
trade window truncates fractional cents after multiplying by the selected quantity.

Change the shared reference only when the item is out of scale across merchants and other
systems. In the Free Merchant rebalance, planks, pipes, hammers, hacksaws, and welders warranted
shared corrections for their practical uses. Owen's prices for items such as repair kits and
axes were changed locally, so the same loot did not become more valuable to unrelated merchants.

## Price buying and selling separately

The **ask** is what the player pays the merchant. The **bid** is what the merchant offers the
player. Use a spread so players pay for the convenience of reliable stock but can still earn
something from scavenging. Start with the following rates for an opted-in shopkeeper, then
validate actual trades:

| Transaction                              |          Starting point |
| ---------------------------------------- | ----------------------: |
| Merchant sells merchandise               |       140% of reference |
| Merchant buys ordinary merchandise       |        40% of reference |
| Merchant buys goods in their specialty   | Around 60% of reference |
| Merchant buys unwanted goods             |       10–20%, or refuse |
| Merchant trades its own faction currency |              Face value |

Use an [NPC class pricing profile](../reference/creatures/npcs.md#trade_pricing) for these rules.
The profile keeps item and category exceptions local to the merchant. Barter and Intelligence
still affect quotes for goods, but the profile caps the adjustment at 20%. Test both sides at
the skill extremes: buying and immediately reselling equivalent goods should never earn money.

Leave unreviewed merchants on the legacy path. Personal valuation is useful for an ordinary NPC
deciding whether they need a tool or can use a weapon, but it does not make a good shop price.
Before Owen had a profile, at matched skills his ask and bid were identical for 26 of the 30
non-currency samples he agreed to buy. Four had higher bids than asks, partly because he valued
a tool differently depending on whether he already owned one.

## Give each merchant a specialty

Decide what the merchant sells, what they prefer to buy, and what they refuse. These lists
can differ: taking an item as salvage need not put it up for sale. A hardware trader can buy
construction materials and spare parts, stock basic tools, and turn away firearms and food.
Other merchants can specialize in weapons, medicine, or electronics instead.

Use dedicated [item groups](../reference/items/item_spawn.md) with three stock tiers:

| Tier           | What belongs there                                                                                                                              |
| -------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| Staples        | Useful quantities of consumables and materials, plus dependable access to basic tools. Size batches against real recipes and construction jobs. |
| Rotating stock | Tools, spare parts, repair kits, practical clothing, or other goods that make repeat visits worthwhile.                                         |
| Uncommon stock | A small selection of specialized equipment or machinery. Don't make rare equipment the only attractive purchase at a high budget.               |

Check what the merchant actually offers after a restock, not just the item group probabilities.
Merchandise sampled against a faction budget can change in quantity or variety when reference
prices change. Owen instead uses a curated collection rolled once per restock: it supplies
12–24 planks and 200–400 nails alongside rotating tools, with at most one uncommon item per
roll. His base restock interval and the calculation for faction currency stock were unchanged.
Check the trade list after normal inventory cleanup, especially for bulky stock. Also check that
earlier player sales and old stock do not bypass the sale restrictions.

## Use quest rewards as purchasing-power benchmarks

Keep faction currency denominations stable and trade the merchant's own currency at face
value. One Free Merchant Certified Note (`FMCNote`) is **1 Merch = 250 internal cents**. A piece
of jerky also has a 250-cent reference value, but that is a comparison, not a guarantee of
what a merchant charges or whether they will trade it at all.

| Currency item                            | Reference value (cents) |
| ---------------------------------------- | ----------------------: |
| Free Merchant Certified Note (`FMCNote`) |                     250 |
| Hub 01 Gold Coin (`RobofacCoin`)         |                   5,000 |
| Campus Exchange Token (`FlatCoin`)       |                     250 |
| Signed chit (`signed_chit`)              |                     250 |
| Church icon (`icon`)                     |                     250 |

Use existing rewards to set useful baskets before considering a mission change. At Owen's
novice quotes, a small reward of **5 Merch** covers 200 nails and three pipes or ten planks.
**15 Merch** covers an axe or machete plus five planks. **30 Merch** covers repair equipment
plus nails; **40–60 Merch** should offer several substantial purchases or specialized
equipment when available. These are hardware-shop examples, not mandatory prices for other
merchants.

Compare rewards against what the same merchant pays for common loot, as well as what they
charge. A few routine sales should not trivialize a dangerous quest. A delivery reward should
normally exceed what the requested goods would earn through ordinary trade. Flag exceptions
for separate balancing rather than silently changing missions. Count explicit currency
payouts separately from mission credit, and don't treat a quoted bid as guaranteed cash or
retained credit.

## Measure and test the trade players actually get

1. **Capture a baseline before changing data.** For a broad rebalance, sample roughly 30–40
   items across staples, tools, weapons, food, medicine, and specialized equipment. Record the
   item ID, condition, quantity, reference value, ask, and bid in a small Markdown or CSV table.
   Record a refusal separately from a quote that rounds down to zero.
2. **Compare like with like.** Use the actual trade-window path (`npc_trading::setup_trade_state`),
   not a calculation from raw JSON. Keep the fixture's skills, Intelligence, inventory, and item
   condition consistent before and after the change. Measure at matched skill and at least one
   other Barter level. Test worn or partly used items, packaging, and whole and partial stacks.
3. **Check budgets and return visits.** Try small, medium, and major quest budgets with actual
   stock. Deplete staples, revisit before the timer expires, and sample several eligible
   restocks. Confirm there is something useful to buy even when rare gear does not appear.
4. **Try the obvious resale loops.** Buy an item and sell it straight back at low and high
   Barter. Try ordinary loot outside the merchant's specialty, and check for effortless
   profit from differences between merchants or currencies.
   Compare the cost of buying recipe inputs with the bid for the output and any byproducts.
   For example, Owen's matched-skill ask for a plank and six nails is 105 cents, versus a
   20-cent bid for a nailboard. Profitable crafting is fine when it requires meaningful time,
   skill, scarce materials, or equipment; trivial repeatable profit is not.
5. **Check neighboring systems.** Test at least one unprofiled merchant and review faction
   currencies, scripted exchanges, and quest rewards separately. Add regression tests for
   the rules the change relies on, use real item and NPC definitions where practical, and
   validate the modified JSON.
