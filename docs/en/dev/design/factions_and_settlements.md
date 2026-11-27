# Factions, Settlements, and Regional Recovery

*Cataclysm: Fault Lines - working design framing*

**Status:** Directional guidance, not a lore canon or implementation specification. Update this document as faction designs are reviewed and decisions are made.

## North-star experience

The world contains isolated survivors, settlements, and specialists trying to recover from the Cataclysm. The player can help these groups become more capable and reconnect into a loose regional community.

This should create two complementary forms of progression:

- **Local development:** A settlement becomes safer, more productive, or more useful because the player helped it.
- **Regional development:** Independent groups establish practical relationships that improve access to goods, services, information, and opportunities.

The player helps civilization reconnect, but does not have to manage or unify it. This remains an open-world survival game, not a mandatory settlement-management campaign.

## Design principles

### Give each location a reason to exist

A major location should have a recognizable identity and a primary contribution to gameplay. Quests, NPCs, services, and rewards should reinforce that role rather than form an unrelated collection of activities.

A minor location can be valuable as a single specialist, supplier, or story encounter. Do not enlarge every outpost into a full faction hub.

### Build complementary capabilities, not duplicate hubs

Settlements need basic support functions, but they need not be best at producing everything. Distinguish a town's ability to maintain itself from a specialist's ability to supply the region.

For example, Tacoma can repair equipment and process enough timber for its own construction without replacing the scrap yard or lumberyard as reliable sources of specialist materials.

Specialization usually means better availability, quantity, convenience, expertise, or terms - not arbitrary exclusivity. Scavenging, crafting, and dealing directly with suppliers should remain viable alternatives.

### Tie effort to results the player can experience

A quest should improve something the player can recognize: a facility, service, resource supply, relationship, new opportunity, or story outcome. Visible construction is valuable, but major projects should generally make the location more useful as well.

Evaluate development quests by the capability they establish, not by the amount of material they request. A long chain of deliveries is not progression if the player cannot perceive the difference.

### Make local and regional progress reinforce each other

Local projects can create demand for specialists. Regional partnerships can make local projects easier or more rewarding. A developed settlement can eventually contribute resources or opportunities back to the regional network.

These relationships should work in both directions where appropriate, without requiring the player to complete every faction's questline.

### Preserve independence and meaningful choices

An agreement with the Free Merchants should not automatically make an independent supplier a subsidiary. Groups can trade, cooperate, compete, and disagree while retaining their own interests.

Require prerequisites only where the underlying work or story genuinely depends on them. The player should be able to prioritize different projects and obtain materials through different means.

### Establish lasting value without excessive simulation

A completed questline should often leave a location with a continuing purpose: trade, specialized supplies, treatment, equipment, information, or a place to rest. Not every small story needs repeatable rewards.

Prefer authored mission outcomes, conditional inventories or services, and dialogue changes. Do not introduce automatic caravans, supply-chain accounting, or a global settlement level unless a concrete gameplay need justifies the system.

### Keep social consequences believable

Residents can disagree about whether a project benefits them. NPCs should respond to developments relevant to their circumstances, but not every character needs to react to every quest. Economic relationships and settlement growth can create narrative tension without requiring a formal faction-conflict mechanic.

## Factions, settlements, and specialists are different

- A **faction** is an organization with interests and relationships. It may operate across several locations.
- A **settlement** is a community where people live and where facilities, population, and services can develop.
- A **specialist outpost** can be one person or a small group whose expertise or production matters to other communities.

These categories can overlap. They should not be forced into a one-faction-per-location model.

## Working roles for the regional community

These are design directions to validate against actual gameplay, not claims that every benefit is already implemented.

| Location or organization | Primary role | Distinctive player value to preserve or develop |
| --- | --- | --- |
| Free Merchants / Refugee Center | Regional commerce, exchange, procurement, and introductions | General market, common currency, supplier connections, trade opportunities |
| Tacoma Commune | Agricultural town and growing residential community | Food production, settlement facilities, local services, useful base of operations |
| Lumberyard | Timber production and processing | Reliable bulk lumber, logs, and specialist wood products |
| Scrap yard | Salvage and materials recovery | Scrap metal, specialty metals, and industrial materials |
| Cabin chemist | Specialized chemical production | Medicines, disinfectants, fungicides, and industrial chemicals |
| Old Guard | Regional security and military coordination | Security assistance, intelligence, military operations and access |

Other factions, including Isherwood Farm and Hub 01, need their own design briefs before their roles are finalized.

## Free Merchants and Tacoma: current framing

### What existing content establishes

Owen's dialogue describes helping return a survivor to his ranch and providing labor in exchange for the ranch becoming a subsidiary of Free Merchant dealings. Tacoma's foreman also describes Old Guard involvement in establishing the outpost. Tacoma and the Free Merchants have distinct faction definitions but use the same `FMCNote` currency.

Some Tacoma residents describe work quotas, limited choices, and difficult conditions. Preserve these perspectives: an arrangement that helps both organizations may still affect individual residents differently.

### Intended division of responsibility

**Free Merchants:** Connect producers, customers, and communities. Their primary progression should improve trade routes, procurement, security of commerce, and the usefulness of their market. They need their own maintenance and basic services, but should not become a comprehensive production settlement.

**Tacoma:** Grow from a vulnerable agricultural outpost into a small town. Its economy should remain anchored in food production. Housing, water, basic repairs, medical care, local trading, and social spaces support a permanent population; they do not transform Tacoma into the best regional source of every good.

**Independent specialists:** Remain worth visiting directly, even when connected to the Free Merchants. A regional agreement can improve convenience or supply through the network without eliminating the supplier's own advantage.

| Need | Tacoma's appropriate capability | Regional specialist's advantage |
| --- | --- | --- |
| Building materials | Basic processing and construction for local projects | Lumberyard offers dependable larger quantities and specialized timber |
| Metal and equipment | Repair, salvage use, small fabrication | Scrap yard supplies bulk and specialty metals |
| Medical needs | Clinic and treatment services | Chemist manufactures specialized medicines and chemicals |
| Food | Agriculture, preservation, and eventual surplus | Free Merchants distribute and trade across settlements |

The division is about **comparative advantage**, not prohibiting overlapping items or services.

## A model regional connection

The existing Free Merchant trade-route quests to the lumberyard, scrap yard, and chemist are a foundation for regional progression. An agreement should do more than complete a journal entry.

For example, a Free Merchants-lumberyard-Tacoma relationship might work as follows:

1. The player meets the lumberyard independently and can already buy from it.
2. The player helps establish a commercial relationship with the Free Merchants.
3. The completed agreement improves access to some lumber through the Refugee Center, while the lumberyard remains the best source for large orders.
4. Tacoma's development can benefit from this improved supply or recognize the partnership, without making the agreement mandatory for construction.
5. The player receives a tangible benefit from enabling the relationship: useful stock, better access, a service, or a relevant new opportunity.

This is an **illustrative target**, not a committed feature list. The scrap yard and chemist should follow the same logic only where it creates a real gameplay benefit.

## Applying the framework to faction work

Before changing a major faction or settlement, record a short brief that answers:

1. **Identity:** Who are they, what do they want, and what distinguishes them?
2. **Primary gameplay:** What does the player do here that is meaningfully different?
3. **Existing content:** Which quests, vendors, services, map changes, and relationships already support this role?
4. **Player benefit:** What becomes available or improves, including after quests end?
5. **Dependencies:** Which developments genuinely depend on others, and which can proceed independently?
6. **Regional relationships:** What can this location supply, obtain, or enable through other groups?
7. **Gaps:** Which specific missing outcomes or contradictions are worth fixing?

Audit existing gameplay before proposing new systems. Preserve content that works. For each proposed change, identify the player-facing result and how it will be verified.

## Working decisions

- Tacoma should become an **agriculturally anchored small town**, not a general-purpose industrial center.
- The Free Merchants should develop primarily as a **commercial network and regional connector**.
- The lumberyard, scrap yard, and cabin chemist should remain **independent specialists**, not generic branches of the Free Merchants.
- Regional ties should provide **tangible benefits** while preserving direct access and sandbox alternatives.
- Local construction and regional trade should reinforce each other, but should not form one mandatory linear quest chain.
- Use existing gameplay systems first. A simulated regional economy is not a prerequisite.

## Open design questions

Resolve these during faction-specific reviews rather than answering them speculatively here:

- What degree of operational independence does Tacoma retain as a Free Merchant-sponsored settlement, and how should residents experience that arrangement?
- Which Tacoma facilities already provide practical benefits, and which currently offer only visual progression?
- What are the actual rewards and ongoing effects of the existing trade-route agreements?
- Which local development paths should be independent after Tacoma's initial infrastructure is established?
- What commercial advantages should accrue at the Refugee Center versus at the originating specialist?
- What, if anything, should represent wider regional cooperation in the late game beyond authored changes to individual locations?

## Related design guidance

- [Faction and settlement design guide](../../mod/json/guides/faction_and_settlement_design.md)
  for designing or reviewing individual locations
- [General game design principles](../explanation/design_document.md)
- [NPC dialogue and quest design](../../mod/json/guides/dialogue_and_quest_design.md)
- [Merchant and item economy](../../mod/json/guides/postapoc_price_guide.md)

This document defines the intended regional experience. The linked guides help authors apply
that direction to individual locations, quests, and rewards.
