# Designing factions and settlements

Use this guide when planning a new faction or settlement or substantially revising an existing
one. Start with what the player can do there. Factions should make the world feel inhabited
and worth exploring, offering ways to trade, learn, build, and form relationships without
turning the survival sandbox into a mandatory tour of quest hubs.

For project-wide direction and working roles for specific locations, see
[Factions, Settlements, and Regional Recovery](../../../dev/design/factions_and_settlements.md).

## Separate the organization, the place, and the people

A **faction** is an organization or community with shared interests, capabilities, and
relationships. A **location** is where the player encounters one part of that organization
or the wider world. An **NPC** is an individual with their own responsibilities and concerns.

One faction can operate several locations with different functions. One settlement can host
people with different allegiances. A resident's personal problem need not advance the
settlement's agenda. Design each at the right scale rather than making every NPC speak for
the faction or treating every location as an independent faction.

## Give the player a reason to seek it out

Choose one primary gameplay focus for each major faction or location. Let it follow from what
the group wants, what it can do, and what its surroundings make possible. Secondary roles
can support that focus without making the location interchangeable with its neighbors.

| Focus | What it can offer |
| --- | --- |
| Survival and self-sufficiency | Food, medicine, shelter, or practical knowledge |
| Trade and economy | A market, reliable supplies, or access to trade partners |
| Construction and production | Workshops, infrastructure, or settlement projects |
| Combat and security | Defensive improvements, specialized equipment, or operations |
| Exploration and discovery | Leads, unusual locations, or research |
| Social and narrative | Personal stories, competing interests, or difficult decisions |

Make NPCs, quests, merchants, services, and rewards reinforce the chosen role. A construction
settlement might offer materials, workshop access, and projects that improve its facilities.
A research organization might offer leads on unusual sites and access to specialized knowledge.
Neither needs a generic merchant or an unrelated fetch quest just to fill a slot.

**Specialize without imposing artificial restrictions.** Reliable stock, better quantities,
convenience, or a more capable service can distinguish a settlement without making its goods
exclusive. A dependable supplier of construction materials is useful even if the player can
scavenge those materials elsewhere. Match rewards to the effort and risk involved, and use the
[merchant economy guide](postapoc_price_guide.md) for monetary benchmarks.

## Make involvement change something

Match the consequence to the work. A personal favor might improve a relationship or open a
small service. A major settlement project should give the player a chance to see and use the
result. If the player delivers materials for a workshop, show when it opens, let relevant NPCs
acknowledge it, and give the workshop a practical use where possible.

Check what the player sees before, during, and after a milestone. The map, dialogue, services,
and available activities should agree about what has happened. A newly cleared room is not yet
an operational workshop. Avoid repeated resource deliveries with no visible payoff.

Consider what remains useful after the main quests: a reliable merchant, improved services,
specialized facilities, information, or access to a location. A small site may serve its
purpose in one visit. Lasting value does not require repeatable quests, simulated production,
or an ever-growing questline. Prefer authored changes and existing mechanics when they convey
the same result.

## Connect places without forcing an itinerary

Give factions relationships grounded in what they need and what they can provide. A food
producer might trade with a commercial hub; a security organization might rely on civilian
suppliers. Shared interests can create cooperation without erasing differences. Conflicts can
make sense without becoming quests, and not every faction needs a connection to every other.

Sequence projects when one genuinely depends on another. A workshop cannot open before its
space is ready, but an independent merchant need not wait for either milestone. As effort,
resources, trust, or risk increase, progression can expand access and capabilities without
requiring every player to visit the same factions in the same order. Reserve mandatory
prerequisites for dependencies the setting or gameplay can justify.

## Write a short design brief

Before creating or substantially revising a faction or location, answer these questions in a
few sentences or bullets. Describe the intended player experience, not the implementation.
For a small location, mark dimensions that do not apply rather than inventing content.

| Dimension | Question |
| --- | --- |
| Identity | Who are they, what do they want, and what sets them apart? |
| Gameplay focus | What part of the game does this place emphasize? |
| Player motivation | Why would the player seek it out? |
| Core activities | What does the player actually do there? |
| Progression | What changes as the player acts, and what depends on those changes? |
| Rewards and services | What practical or narrative value does the player receive? |
| Lasting value | What might bring the player back after the main quests? |
| World relationships | Which other groups or locations matter here, and why? |

For existing content, infer the current role from lore, geography, NPCs, quests, and resources.
Inventory what players can do and receive, then trace how those opportunities change through
play. Look for specific problems: a promised service that never opens, unrelated quests blocking
one another, overlapping faction roles, or a project with no payoff. Preserve what already
works and fix the most consequential gaps first. Do not redesign a location just to make it
match the brief's structure.

## Review the design

- **Recognizable:** Can players tell what makes this faction or location different?
- **Relevant:** Do its quests, merchants, services, and rewards support that identity?
- **Rewarding:** Are its benefits and experiences worth the effort and risk?
- **Coherent:** Do its relationships, prerequisites, and visible changes make sense in the world?

If removing the faction or location would only cost the player another source of generic quests
or goods, clarify what experience it should provide before adding more content.

Once the brief is clear, [design individual quests and NPC stories](../../../dev/design/quests_and_npc_stories.md).
For implementation, see [authoring NPC dialogue and quests](dialogue_and_quest_design.md)
and the [faction JSON reference](../reference/creatures/factions.md).
