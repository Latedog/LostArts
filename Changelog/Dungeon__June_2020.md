6/16:

- Knockback (sturdiness) changed to only stun monster if they were actually pushed back. No stun if they did not move from their current position.
- Goblins, Archers, Knights, and Seekers now all experience knockback when they are hit.
- Removed redundant/unused constructors for the WoodShield and IronShield.

6/18:

- Implemented Charger enemy (has temporary artwork as a Goo Sack).

6/19:

- Implemented 3 new Wanderer types: Rabid, Sleeping, and Provocable
- SleepingWanderers now use a less intelligent version of pathfinding using a greedy search.

6/20:

- Implemented Rat enemy
- Certain enemies had their weapons removed since they shouldn't technically be wielding one. Added hasWeapon() checking function as a result for monster attacks.

6/21:

- Implemented Toad enemy
- Added PoisonPuddle trap that Toads can create
- Modified moveMonsterRandomly to also check for the player. If the player is invisible, it prevents monsters from overlapping on top of the player.
