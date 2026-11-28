# Designing Quests and NPC Stories

## Purpose

Quests should make the world more interesting to explore, give players meaningful experiences, and help its inhabitants feel like people with lives beyond the player's immediate objectives.

A quest may involve combat, exploration, investigation, conversation, recovering an item, constructing a facility, or resolving a personal problem. It does not need to involve all of these.

What matters is whether the experience is engaging, coherent, and appropriate to its place in the world.

**Design the experience first. Implement the mission second.**

This guide is about what players experience, not whether the mission machinery runs. A quest
can assign the correct target, advance every dialogue branch, and pay out exactly as configured,
yet leave the player wondering why they should care, what they were supposed to infer, or why
this particular person asked for help.

Review a quest from the player's side: **What makes me curious? What can I understand from what
I've seen and heard? What makes this feel like something these people would do, in this place?**
Use the [authoring guide](../../mod/json/guides/dialogue_and_quest_design.md) to build and verify
the mechanics once you know what they need to accomplish.

Use the guide in order for a new storyline, or jump to the stage you need:

- [Develop the premise and characters](#1-start-with-the-player-experience)
- [Plan scenes, investigations, and choices](#5-design-a-playable-sequence-of-scenes)
- [Write dialogue for characters](#10-write-dialogue-for-characters-not-mission-systems)
- [Preserve the experience when scoping implementation](#13-let-the-experience-drive-the-implementation)
- [Follow the workflow](#17-a-practical-design-workflow), use the
  [design brief](#18-lightweight-quest-design-brief), or run the
  [final review](#19-final-design-review)

For faction storylines, see [regional recovery and faction roles](factions_and_settlements.md)
and the [faction and settlement design guide](../../mod/json/guides/faction_and_settlement_design.md).

## 1. Start with the Player Experience

Before deciding how a quest will be implemented, establish why a player would enjoy experiencing it.

Consider what the player will:

- **Encounter:** An interesting person, unusual situation, danger, opportunity, or mystery.
- **Do:** Explore, fight, investigate, negotiate, recover something, help someone, or make a decision.
- **Discover:** Learn something about a person, location, faction, or the world.
- **Experience:** Curiosity, tension, humor, satisfaction, surprise, sympathy, or some other response appropriate to the story.
- **Gain or change:** Obtain something useful, resolve a problem, influence a relationship, or experience a satisfying conclusion.

Not every quest needs all five elements.

A small personal interaction might primarily offer humor or characterization. A combat mission might provide an exciting encounter and valuable equipment. A major story could combine several types of experience.

The essential question is:

**What makes this worth playing, rather than merely worth completing?**

A quest that asks the player to deliver five flashlights so a merchant can stock slightly more merchandise is mechanically functional, but its experience is weak.

A quest that sends the player to investigate why a scavenging expedition disappeared could be compelling even if it uses only existing mission goals and dialogue conditions.

### The player should be an active participant

A quest is not necessarily interesting because an NPC tells an interesting story.

Consider the difference between:

- Listening to an NPC explain what happened at an abandoned farm.
- Visiting the farm, observing what happened, discovering evidence, and reaching a conclusion.

Both can provide world-building, but the second gives the player a role in uncovering the story.

Whenever appropriate, let the player discover information through gameplay instead of delivering it through exposition.

This is not a requirement for every quest. Some of the best small encounters are conversations.

## 2. Design People Before Their Requests

An NPC should have a recognizable identity independent of the quest they offer.

Before developing a story around an NPC, understand:

| Question | Purpose |
| --- | --- |
| Who are they? | Their background, profession, circumstances, and personality |
| What do they want? | Their immediate and longer-term motivations |
| What do they believe? | Their understanding of the world and current situation |
| What are they worried about? | Their fears, responsibilities, or vulnerabilities |
| How do they relate to others? | Their relationships, loyalties, and disagreements |
| Why involve the player? | What the player can do that the NPC cannot or will not do themselves |
Not every detail needs to appear in dialogue.

The purpose is to establish a consistent character whose behavior follows from their circumstances.

### Different people should see the same situation differently

A farmer might view a shipment of grain as the product of months of labor.

A merchant might see it as a valuable trading opportunity.

A foreman might see it as necessary income to maintain the settlement.

A hungry worker might see it as food they should have been allowed to eat.

These interpretations need not represent mutually exclusive truths.

Different perspectives make conversations interesting because they reveal something about the characters.

They also allow the player to understand conflicts without requiring every story to contain a villain.

At the Refugee Center, staff generally deal with operational needs. Refugees can have personal
concerns, relationships, and useful skills without being responsible for running the Center.
Both groups help establish what life there is like.

### Not everyone needs a quest

A character can be memorable because of their dialogue, personality, relationships, profession, or observations.

Do not manufacture a delivery request simply because an NPC has no missions.

Small conversations can establish a community more effectively than additional low-value quests.

## 3. Give Every Story a Reason to Exist

Before designing objectives, write a short description of the situation.

It should explain:

1. What is happening?
2. Why does it matter to the people involved?
3. Why does this person need outside help, and what would the player actually do?
4. What makes the situation interesting?
5. What could change as a result?

For example:

**Weak concept**

A farmer needs thirty units of fertilizer. Deliver them to improve the merchant's stock.

**Stronger concept**

A farmer believes someone has been diverting part of the harvest. The foreman insists the missing food was needed to meet Tacoma's obligations to the Free Merchants. The player can investigate the discrepancy and potentially help the two sides reach an agreement.

The second concept has characters, conflicting interests, information to uncover, and a reason for the player to participate.

It may ultimately involve obtaining supplies, but the activity would serve the story rather than constitute the entire story.

### Avoid interchangeable quests

Ask whether the quest would still make sense if its requested item, NPC, or location were replaced with something unrelated.

If almost nothing changes, the quest may be too generic.

Recovering Dana's family sourdough starter and delivering twenty packets of yeast could use the
same mission goal. The starter matters because Dana kept something from her life before the
Cataclysm alive, thought she'd lost it, and can bake with it again. Better bread on a later
visit lets the player experience what changed. If the player only learns that a jar is needed
to improve merchant stock, the mission can work perfectly and still lose what makes it Dana's
story.

## 4. Use Narrative Techniques Deliberately

Classic RPGs such as *Planescape: Torment* and *Fallout 1 and 2* demonstrate that simple gameplay systems can support sophisticated stories.

Their most useful techniques are broadly applicable.

### Revelation: Change the meaning of earlier information

Establish an initial understanding of a situation, then introduce information that changes how the player interprets it.

For example:

- A scavenger claims his companions were killed by monsters.
- The player discovers evidence suggesting they fought among themselves.
- A later conversation reveals why the survivor lied.

The revelation should matter because the player already knows something about the people involved.

A surprising twist is not automatically a good story. The new information must be consistent with the evidence and meaningful to the characters.

### Conflicting perspectives: Let several explanations make sense

Different characters can interpret the same events according to their interests and experiences.

The player learns more by comparing their accounts.

The conflict need not have an objectively correct side.

However, avoid artificial moral ambiguity. Some situations really do involve someone behaving cruelly, dishonestly, or selfishly.

The goal is believable motivation, not moral symmetry.

### Knowledge: Let discovery create opportunities

Information can be a reward and a tool.

The player might discover:

- Evidence contradicting someone's account.
- A technical flaw in a proposed solution.
- An overlooked alternative to violence.
- A fact that changes an NPC's willingness to cooperate.
- A personal detail that helps them understand another character.

Whenever possible, discovering useful information should give the player a meaningful new option.

In *Fallout*, evidence concerning super mutant sterility can fundamentally change the confrontation with the Master.

The principle is valuable even for much smaller stories.

### Recontextualization: Make earlier encounters matter later

A minor character introduced during one quest might become important in another.

A conversation that initially seems incidental might contain information relevant to a later problem.

A personal favor can change how a character responds when the player returns.

Avoid relying entirely on newly introduced NPCs who disappear once their quests are complete.

Reusing established characters and locations builds continuity.

### Personal stakes: Small problems can matter

Not every story needs a dangerous expedition or major faction conflict.

Recovering a personal possession, helping someone regain their profession, or resolving an embarrassing mistake can be worthwhile.

The significance of an object depends on what it means to the character, not its market value.

### Humor and ordinary life

A believable world contains absurdities, inconveniences, disagreements, routines, and moments of levity.

A carpenter who exaggerated his qualifications to escape a refugee camp can support a humorous story without becoming a caricature.

A broken communal kettle can create an entertaining interaction without becoming a major plot.

Avoid making every NPC's story a tragedy, every problem a moral dilemma, or every revelation a betrayal.

Narrative variety is as important as gameplay variety.

## 5. Design a Playable Sequence of Scenes

A story should be planned as a sequence of experiences before it is translated into mission definitions.

A scene is a distinct encounter, discovery, interaction, or challenge.

For each scene, identify:

| Element | Question |
| --- | --- |
| Location | Where does this happen? |
| Characters | Who is involved? |
| Player activity | What does the player actually do? |
| Information | What can the player learn? |
| Change | What becomes possible or different afterward? |
For example, an investigation might consist of:

| Scene | Player activity | Narrative purpose |
| --- | --- | --- |
| Meet a sick laborer | Conversation | Establish the problem |
| Question other residents | Investigation through dialogue | Introduce competing explanations |
| Explore a storage building | Exploration | Discover physical evidence |
| Examine shipment records | Discovery | Establish what actually happened |
| Confront the foreman | Dialogue and decision | Resolve the conflict |
| Return later | Conversation | Observe consequences |
This does not necessarily require six missions.

It might be implemented as one condition-based quest, several short missions, or a combination of formal objectives and optional dialogue.

**Mission definitions are implementation units. Scenes are narrative units. Do not confuse them.**

### Avoid unnecessary stages

Each additional stage should introduce a meaningful change in the player's activity, knowledge, or circumstances.

Do not divide a quest into several stages merely to make it longer.

Likewise, do not combine several distinct experiences into one confusing mission just to reduce the quest count.

The number of stages should follow the story.

## 6. Provide Meaningful Gameplay Variety

Different quests should offer different experiences, particularly within a major settlement or faction.

Useful activities include:

| Activity | Examples |
| --- | --- |
| Exploration | Discovering a hidden location, following clues, investigating ruins |
| Combat | Eliminating a threat, surviving an encounter, protecting someone |
| Investigation | Comparing testimony, interpreting evidence, uncovering a deception |
| Social interaction | Persuasion, negotiation, recruitment, resolving disputes |
| Technical problem-solving | Diagnosing equipment, identifying a structural problem |
| Resource gathering | Procuring specific materials that matter to the situation |
| Personal favors | Recovering something meaningful, helping someone regain a capability |
| Construction | Improving a settlement or establishing a useful facility |
| Discovery | Learning about regional history, factions, or unusual events |
Not every quest needs a unique mechanic.

An interesting retrieval mission may be better than a poorly conceived combat encounter.

The goal is variety across the player's broader experience.

### Do not use arbitrary quotas

A settlement does not need exactly three combat quests, two mysteries, and four personal favors.

Instead, review its existing content.

If most quests involve resource deliveries, look for opportunities to introduce exploration, conversation, and danger.

If every storyline concerns betrayal or exploitation, introduce moments of humor, cooperation, or hope.

The intended experience and setting should determine the mix.

## 7. Make Investigation Require Discovery

Investigation quests deserve particular care.

A poor investigation requires the player to speak to three marked NPCs and then automatically reveals the answer.

A better investigation allows the player to observe inconsistencies, compare accounts, and discover evidence.

### Establish what actually happened

Before designing the player's investigation, write down the complete underlying event.

Identify:

- What happened.
- Who knows the truth.
- Who misunderstands it.
- Who is hiding information.
- What physical evidence remains.
- What the player can reasonably infer.

This prevents mysteries from becoming a collection of arbitrary clues.

### Build the investigation backward

Start with the truth.

Determine what evidence would reasonably remain.

Then decide which evidence the player will encounter and what conclusions it supports.

Not every clue should explicitly state the answer.

A damaged vehicle, an abandoned campsite, or a missing shipment record may be more effective than a note explaining the entire mystery.

### Allow reasonable alternative approaches

A medically trained player might recognize symptoms.

A skilled mechanic might identify a vehicle malfunction.

Another player might discover equivalent information through conversation or exploration.

Skill checks should often provide shortcuts, additional insight, or alternative resolutions.

Do not require an unrelated skill simply because the quest needs a difficulty gate.

### Avoid fake investigations

If the player has no opportunity to observe evidence or interpret information, the quest may be better presented as a straightforward conversation or retrieval mission.

Do not pretend that following a marker and collecting an item constitutes detective work.

## 8. Make Choices Matter Without Excessive Branching

Choices are meaningful when they affect at least one of the following:

- How the player approaches a problem.
- What information becomes available.
- Which risks or costs they accept.
- How characters respond.
- What outcome occurs.
- What opportunities remain available.

Not every decision needs to create a different ending.

A minor conversational choice might simply reveal the player's perspective and change an NPC's reaction.

A significant decision might affect a relationship, settlement service, or later quest.

### Prefer informed decisions

Players should understand the relevant stakes before making consequential choices.

They don't need perfect information, but the decision should not depend on facts the game has deliberately concealed without warning.

### Avoid cosmetic branching

If several dialogue responses produce identical outcomes and reactions, consider whether they provide meaningful roleplaying expression.

If they don't, remove the unnecessary choices.

### Avoid branching for its own sake

Every additional branch requires writing, implementation, testing, and ongoing maintenance.

A well-authored linear story may be stronger than several shallow alternatives.

Use branches where they improve the experience, not to demonstrate technical capability.

## 9. Design Rewards Around the Player

A reward can take many forms:

- Useful equipment.
- Money or trade goods.
- Access to a service.
- A distinctive item.
- Information or a new location.
- A relationship or personal favor.
- An improved settlement.
- A satisfying narrative conclusion.

The reward should fit the scale and purpose of the quest.

A small personal favor need not provide substantial compensation.

A dangerous expedition should generally offer something more compelling than a minor merchant stock adjustment.

For cash or faction currency, use the [merchant economy guide](../../mod/json/guides/postapoc_price_guide.md) to
compare payment with what it buys.

### Evaluate the entire cost

Consider:

- Difficulty obtaining required materials.
- Weight and volume of deliveries.
- Travel distance.
- Combat risk.
- Time investment.
- Skills or equipment required.
- Opportunity cost.
- Prerequisite quests.

A requirement for 200 planks is not merely a request for inexpensive building materials. It represents substantial hauling and logistics.

That effort must be justified by the experience and outcome.

### Avoid rewards made redundant by the objective

If the player must acquire advanced equipment to unlock a merchant selling common equipment, the benefit may arrive too late to matter.

Likewise, asking a player to collect large quantities of seeds to unlock access to a seed merchant deserves scrutiny.

### Make the stakes understandable

Give the player enough information to decide whether to take the job. A promised reward should
be clear; a discovery or surprise doesn't have to be. If the story is about investigating what
went wrong, make the first lead understandable without giving away the answer.

## 10. Write Dialogue for Characters, Not Mission Systems

Dialogue should sound like something the character would say.

Avoid making every NPC use the same conversation structure:

- What is your job?
- Do you need help?
- Here are your requirements.
- Thank you for completing my mission.

A guarded or resentful NPC may need to be approached differently from a friendly merchant.

A nervous worker might reveal information reluctantly.

A confident foreman might frame a request as an obvious necessity.

A grieving resident might initially refuse to discuss the problem.

### Establish individual voices

Characters can differ through:

- Vocabulary and phrasing.
- Humor.
- Patience.
- Formality.
- Optimism or pessimism.
- Willingness to disclose information.
- Attitudes toward the player.
- Relationships with other residents.

Avoid exaggerated quirks that reduce characters to caricatures.

### Let dialogue reveal information naturally

Characters should talk about things that matter to them.

A farmer complaining about compensation can reveal how the settlement's economy works without reciting a formal explanation of its operating structure.

A merchant mentioning a missing caravan can establish a future adventure without immediately offering a quest.

This kind of incidental world-building helps a location feel interconnected.

### Change dialogue when circumstances change

If the player resolves a personal problem, affected characters should remember it.

If a facility opens, relevant residents should acknowledge it.

If the player makes an unpopular decision, not everyone should respond with gratitude.

Preserve differences of opinion.

Use a physical change when it matters to the setting and dialogue when the effect is personal
or depends on the speaker's perspective. At the Refugee Center, Milton's clearing mission
addresses an immediate problem; Boris's workshop gives the back bay a new use. Comments from
refugees who care about the bay connect the events without making every local quest part of
one chain.

### Write for return visits

Consider what happens when the player speaks to an NPC:

- Before a quest is available.
- During the quest.
- After completion.
- After failure or rejection.
- After a related story changes the situation.

NPCs should not repeatedly deliver introductory exposition after the player already knows them.

## 11. Respect the Open-World Sandbox

Quests should provide opportunities rather than force the player through a predetermined narrative.

### Avoid artificial prerequisites

Require previous quests only when the next activity genuinely depends on them.

A clinic does not necessarily need a sawmill to exist.

A farmer's personal problem should not require the bar to have opened.

An independent merchant should not be unavailable because the player hasn't completed an unrelated construction project.

### Allow players to decline

Not every character's problem needs to be solved.

A player might be uninterested in a labor dispute or unwilling to risk their life for a missing scavenger.

Ordinary services and unrelated opportunities should remain available.

### Account for unexpected behavior

Players may:

- Discover locations before receiving quests.
- Kill hostile targets before being asked to.
- Find quest items through unrelated exploration.
- Lose or destroy required items.
- Attack important NPCs.
- Abandon missions.
- Return after substantial time has passed.
- Complete related quests in an unexpected order.

Design for reasonable behavior without attempting to simulate every possible situation.

Where a quest depends on specific sequencing, make that dependency clear and test it.

Decide which unscripted actions should count. If the player already dealt with a threat or
found the relevant evidence while exploring, would the NPC recognize that work? If refusing
or failing closes off later opportunities, make that a consequence of the situation, not
just of the order in which the player found the quests.

## 12. Connect Stories Across the World

The game's broader direction emphasizes independent communities gradually reconnecting after the Cataclysm.

Quests can support this through:

- Shared characters and relationships.
- Information about other locations.
- Trade connections.
- Refugee relocation.
- Conflicting faction interests.
- Regional dangers.
- Cooperation between specialists.

Connections should be meaningful rather than mandatory.

A player might learn about the Lumberyard from Tacoma while remaining free to discover it independently.

A quest involving agricultural production could influence the Refugee Center's food supply.

A personal story could introduce an NPC who later relocates between communities.

Not every quest needs a regional connection.

Some stories are valuable precisely because they concern one person and one small place.

### Preserve faction identities

Characters should not automatically speak for their entire faction.

A settlement can benefit from a commercial agreement while some residents oppose it.

An independent supplier can cooperate with the Free Merchants without becoming their subsidiary.

These distinctions create opportunities for believable conflict and world-building.

## 13. Let the Experience Drive the Implementation

Fault Lines already supports a substantial range of narrative functionality.

Before proposing engine changes, determine whether the experience can be expressed through existing systems.

Use the [authoring guide's mechanism table](../../mod/json/guides/dialogue_and_quest_design.md#choose-existing-mechanisms)
to check what's available. Choose the simplest mechanism that preserves what the player is
meant to do, learn, or decide. Simplicity is a virtue only if the resulting experience still
works.

### Do not force a poor experience into an existing mechanism

Sometimes a story requires an interaction the engine doesn't support well.

For example:

- Examining a specific physical object and recording a discovery.
- Moving an existing named NPC between settlements.
- Representing a multi-stage investigation clearly in the mission journal.

In these cases, evaluate three options:

1. Adapt the story without sacrificing its intended experience.
2. Use an existing mechanism that provides substantially equivalent gameplay.
3. Propose a reusable engine improvement.

Do not silently replace an investigation with an ordinary item delivery merely because it is easier to implement.

### Require a concrete justification for new systems

A proposed engine capability should identify:

- The player experience it enables.
- Why existing mechanisms are insufficient.
- Whether several quests could use it.
- The smallest reusable implementation.
- Its maintenance and testing implications.

Avoid building specialized systems for individual quests unless the experience clearly warrants the cost.

## 14. Scope Stories Proportionally

Different stories require different amounts of planning.

| Story scale | Typical characteristics | Documentation |
| --- | --- | --- |
| Incidental encounter | Conversation, discovery, minor interaction | Short concept and dialogue outline |
| Personal quest | One character, focused problem, limited consequences | Brief narrative and implementation outline |
| Local adventure | Multiple scenes, exploration or combat, some state changes | Scene outline, encounter design, tests |
| Major storyline | Several characters or locations, revelations, choices, lasting consequences | Full narrative design and implementation plan |

These are guidelines, not rigid categories.

A small story may require careful dialogue but little technical planning.

A major storyline may use simple mechanics while requiring substantial narrative preparation.

Do not impose a large design document on every incidental interaction.

## 15. Review the Wider Quest Portfolio

Individual quests should not be evaluated in isolation.

A settlement containing twelve well-implemented fetch quests may still be boring.

Review the overall experience.

Ask:

- Are most quests assigned by one NPC?
- Do most quests use the same gameplay activity?
- Are characters primarily defined by their requests?
- Do players spend meaningful time exploring?
- Do they encounter memorable dangers or discoveries?
- Are there opportunities for humor and quieter character moments?
- Are there stories that don't involve construction or trading?
- Do decisions have consequences appropriate to their importance?
- Does the settlement become more interesting as the player learns about its residents?

Do not enforce artificial counts of mission types.

Instead, identify missing experiences and opportunities.

The Refugee Center provides a useful reference because its larger security and faction stories coexist with personal favors, technical projects, mundane inconveniences, and relationships between residents.

That variety is more important than the specific number of quests.

## 16. Common Design Failures

### The interchangeable fetch quest

**Problem:** An NPC requests arbitrary materials, and completion provides money or generic stock.

**Better approach:** Give the request a specific purpose, meaningful context, or distinct player benefit—or remove it.

### Construction as the entire story

**Problem:** Every quest exists to build another facility.

**Better approach:** Treat construction as one category of content alongside personal stories, exploration, conflict, and discovery.

### The passive investigation

**Problem:** The player talks to marked NPCs, collects a clue, and receives the answer.

**Better approach:** Let evidence contradict assumptions and allow the player to discover information through meaningful exploration or reasoning.

### The automatic Speech solution

**Problem:** A difficult social conflict disappears because the player passes a skill check.

**Better approach:** Let skills provide insight, leverage, or alternative approaches. Ground persuasive arguments in the characters' actual motivations.

### The false moral dilemma

**Problem:** Every problem presents two supposedly equally valid choices, regardless of context.

**Better approach:** Write believable conflicts. Some problems have clear answers; others involve legitimate competing interests.

### The inconsequential choice

**Problem:** Dialogue presents several options that produce the same reaction and outcome.

**Better approach:** Provide distinct responses or remove unnecessary branching.

### The excessive tragedy

**Problem:** Every resident has suffered a terrible loss, and every quest attempts to be emotionally devastating.

**Better approach:** Include humor, ordinary frustrations, friendships, ambition, competence, and hope.

### The reward that comes too late

**Problem:** A player must obtain equipment or resources more valuable than the service the quest unlocks.

**Better approach:** Reevaluate the requirement, reward, and expected stage of player progression.

### The quest that should have been a conversation

**Problem:** A small character interaction is inflated into a formal mission without adding meaningful gameplay.

**Better approach:** Use ordinary dialogue, an incidental encounter, or a short authored event.

### The overengineered story

**Problem:** A single quest requires a custom system that duplicates existing capabilities.

**Better approach:** Use the simplest mechanism that preserves the intended experience.

## 17. A Practical Design Workflow

Use this sequence when developing new content.

### Step 1: Review the existing world

Read relevant NPC dialogue, location history, faction relationships, existing quests, and available services.

Identify what is already established.

Do not invent a conflicting backstory merely to create a quest.

### Step 2: Write the narrative premise

Describe the interesting situation in a few sentences.

Establish who is involved, what they want, and why the player might care.

### Step 3: Define the player experience

Describe what the player will encounter, do, discover, and gain.

If these elements aren't interesting, revise the premise before implementation planning.

### Step 4: Outline the scenes

Identify the significant encounters and actions.

Determine what information becomes available at each point.

Avoid designing mission stages until the scene sequence is coherent.

### Step 5: Design consequences and alternatives

Decide what happens when the player succeeds, refuses, fails, or pursues a different reasonable approach.

Keep branches proportional to the story.

### Step 6: Write dialogue and environmental content

Give characters distinct voices.

Use the environment to communicate what happened.

Ensure journal descriptions communicate actionable objectives without spoiling discoveries unnecessarily.

### Step 7: Hand off to implementation

Record what each scene must let the player do, what becomes knowable, and what changes. Use the
[authoring guide](../../mod/json/guides/dialogue_and_quest_design.md) to map those requirements
to supported mechanics and verify them.

### Step 8: Play through the experience

If possible, have someone try it without reading the brief. Ask what made them want to continue,
what they understood the NPC to want, how they knew where to go, and what they thought had changed
afterward. Their account is more useful than knowing only that they reached the turn-in.

If a player follows the markers and finishes without understanding why the job mattered, the
mission passed its functional test but missed a design goal. Revisit what the player actually
sees, hears, and does before adding more exposition or more objectives.

### Step 9: Review the surrounding content

Ensure the quest contributes something distinctive to its location and does not introduce redundant services, unnecessary dependencies, or contradictory world-building.

## 18. Lightweight Quest Design Brief

Use this template before implementation.

For a small quest, a few sentences may be sufficient.

For larger stories, expand the sections as needed.

### Working title

A short, distinctive name.

### Narrative premise

What is happening, and why is it interesting?

### Characters and motivations

Who is involved? What does each character want, believe, or misunderstand?

### Player experience

What will the player actually do?

What might they discover, decide, or experience?

### Scene outline

| Scene | Location | Player activity | Narrative development |
| --- | --- | --- | --- |
| 1 |  |  | |
| 2 |  |  | |
| 3 |  |  | |

### Outcomes

What changes when the story is resolved?

What happens if the player refuses or fails?

Are there meaningful alternative outcomes?

### Rewards

What does the player receive or gain access to?

Is the reward proportionate to the effort and risk?

### Dependencies

What must already have happened?

Are those prerequisites genuinely necessary?

### Essential player-facing moments

Which discoveries, choices, or changes must survive implementation? What can be simplified
without losing what makes the quest worth playing?

### Validation

What would a playtester need to notice, understand, or care about to show that the design landed?

## 19. Final Design Review

Before implementation, the designer should be able to answer these questions.

**Narrative**

- Is the situation interesting and believable?
- Do the characters have recognizable motivations?
- Does the story fit the established world?
- Does it reveal something worth discovering?

**Gameplay**

- What does the player actually do?
- Is the experience varied and engaging?
- Are the challenges appropriate?
- Are players given reasonable alternatives where appropriate?
- Is the quest's length justified?

**Consequences**

- Does the outcome matter at the appropriate scale?
- Do the relevant characters respond?
- Can the player tell what changed and why it matters?
- Are choices meaningful without unnecessary branching?

**World integration**

- Does the quest reinforce the location's identity?
- Does it respect established relationships and lore?
- Does it avoid unnecessary prerequisites?
- Does it complement rather than duplicate existing content?

**Player understanding**

- Can the player tell what is happening without reading the design brief?
- Are the next steps discoverable without giving away the discovery?
- Do the NPC's words and actions make sense from their own point of view?
- Would a player remember this particular person or place, not just the objective?

**The final question:**

If the player finishes this quest and remembers only one thing about it, what should that be?

If the answer is merely the quantity of materials delivered or the amount of money received, reconsider whether the quest is worth adding.

## Guiding Principle

Good quests create experiences that players remember.

Sometimes that means a dangerous expedition, an unexpected discovery, or a difficult decision.

Sometimes it means helping someone recover a treasured possession, making a new friend, or witnessing an ordinary moment of life after the Cataclysm.

The mechanics supporting those experiences can be simple.

The design should not be.

**Build interesting situations involving believable people. Give players meaningful ways to participate. Let the world respond appropriately.**
