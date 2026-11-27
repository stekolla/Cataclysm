# Designing NPC dialogue and quests

Read this guide before designing a new NPC, quest, or faction storyline. Use the
[NPC dialogue reference](../reference/creatures/npcs.md) and
[mission reference](../reference/creatures/missions_json.md) when you are ready to implement it.

## Give the NPC a reason to be there

Start with the character's role, motivation, and perspective. Decide what they need, what they
can do for themselves, and what they think of a stranger offering help. A request should follow
from their circumstances, not from a gameplay system that needs an owner.

Not every named NPC needs a quest. Information, trade, services, relationships, and even a short
conversation can make a character worth meeting. Give the player a reason to remember the person,
not just the task attached to them.

At the Refugee Center, staff generally deal with operational needs. Refugees can have personal
concerns, relationships, and useful skills without being responsible for running the Center.
Both groups help establish what life there is like.

## Give the quest a purpose

Before writing an offer, answer five questions:

1. Why does this person need help?
2. Why can't they reasonably do the work themselves?
3. What does the player actually do?
4. Why is it worth doing?
5. What changes when the job is done?

The change can be physical, economic, narrative, or personal. Recovering Dana's family sourdough
starter gives her something she thought she had lost and eventually improves the bread she sells.
It does not need to transform the Center or start a new chain to matter.

Fetching an item can serve a good story. If the item could be swapped for anything else without
changing the request or its outcome, revisit why the quest belongs here.

## Plan progression before writing branches

Sketch when the offer appears, what counts as completion, and what happens after success or
failure. Keep the chain short enough that the player understands why one job leads to the next.
Do not gate an otherwise independent character's work behind another NPC just to impose an order.

Check that ordinary play leaves a reasonable path to completion. Consider whether a required
item can be lost, whether a contact can become unavailable, and whether rejecting or failing one
job should close off later work. If a failure is meant to have lasting consequences, make them
deliberate rather than the result of an unreachable dialogue branch.

Boris needs five volunteers to clean up the back bay. The player can ask across the Center,
including guards, merchants, and refugees. The offer and advice point to those groups so players
can find several ways to meet the same goal without replacing the recruitment mechanic.

A choice matters when it changes the approach, cost, risk, or outcome. A second confirmation
that leads to the same result is not another choice. Keep the actual accept-or-decline decision;
skip extra prompts that do not help the player decide.

## Make conversations natural and easy to navigate

Put the request in the NPC's own words. Let a busy guard and a worried refugee ask for help
differently. Avoid making every interaction start with the same generic question about work.
Use shared mechanics without forcing characters into the same conversational template.

Keep frequent actions such as trading, asking about available work, and reporting progress
close to the main conversation. If background questions accumulate, group them under a clear
topic so they do not crowd out those actions.

Give each submenu a sensible way back. Avoid nesting a useful response behind several layers of
questions. Read the exchange in order, including the player's responses, to catch awkward
transitions or replies that only make sense on the first visit.

Let conversations change when a relationship or an important objective changes. Keep ordinary
services and unaffected topics accessible. See the [NPC dialogue reference](../reference/creatures/npcs.md)
for entry topics, shared responses, and back navigation.

## Match the reward to the job

Consider the difficulty, risk, and usefulness of the work. For cash or faction currency, use the
[merchant economy guide](postapoc_price_guide.md) to compare the payment with what it buys.
Money is not the only useful reward: a personal favor might earn a service, information, access
to resources, or a change to the settlement.

Tell the player what the NPC is promising before they accept. Grant the defined reward
automatically on completion rather than making the player find a second dialogue option to
claim it. If the reward is access to a service that becomes available later, explain the delay
and whether using the service costs anything.

Do not add generic reputation or reward negotiation choices unless the result has a gameplay
effect. A response that implies a benefit the game does not deliver undermines the next offer.

## Write for both the conversation and the journal

Dialogue conveys why the work matters and how the character feels about it. The journal
should let a player resume the quest after a long break without recalling the conversation.
Together, they should establish:

- What the player must accomplish.
- Where to go or how to start looking.
- How many items or people are needed, and any special conditions.
- What the player receives, including any delayed benefit.
- Who to report to, if a report is needed.

For example, a short version of Dana's request could read:

> My family kept that starter alive for generations. I left it at the bakery when I ran.
> Bring it back, and in a couple of weeks I'll have better bread to sell you.

The journal instead needs directions and a clear outcome:

> Find one jar of Dana's family sourdough starter in the right-hand fridge at the marked
> bakery. Return it to Dana at the Refugee Center.
>
> Reward: Dana sells improved sourdough after two weeks.

The journal does not need to repeat the family history. The NPC does not need to recite the
journal entry, but should give the player enough information to decide whether to take the job.

## Let the world respond where it matters

Reserve physical changes for developments that meaningfully affect the setting. Use dialogue
for effects that are personal or best seen through a character's perspective. Two people can
react differently to the same event, and most people need not mention it at all.

The Refugee Center's back bay illustrates the difference. Milton's clearing mission addresses
an immediate problem; Boris's workshop gives the space a new use. Comments from refugees who
care about the bay make the developments feel connected without making every local quest part
of one chain.

Gate each line on the event that makes it true. Clearing a space is not the same as opening a
workshop, and accepting a job is not the same as doing it. Check what the NPC says between
milestones, not just before the first quest and after the last.

## Keep the implementation proportional

Prefer existing mission goals, dialogue conditions, and effects. If a completed mission already
represents the state you need, check that state instead of introducing a second flag that could
get out of sync.

Start with the smallest implementation that delivers the visible result. A new service at one
settlement does not require a simulated settlement economy, and a character remembering a favor
does not require a generic reputation system.

## Review the player experience

- Is the request believable for this NPC?
- Is the objective understandable and achievable?
- Can players discover the available approaches?
- Is the reward clear and worthwhile?
- Does dialogue reflect what has actually happened?
- Can players navigate the conversation without unnecessary steps?
- Do quest effects, services, and follow-ups work as promised?
- Does the quest fit the setting without imposing unnecessary dependencies?

Use the [testing guide](../../../dev/guides/testing.md) and
[dialogue validator](../reference/creatures/npcs.md#validating-dialogues). The validator checks
topic links, not the player experience. Play through the offer, objective, and turn-in, and check
the refusal, failure, and return-visit paths that your change affects.
