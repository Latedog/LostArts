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
- Puddles now have a "turn" modifier that signifies if it dissipates over time or stays until it's walked over.
- Modified moveMonsterRandomly to also check for the player. If the player is invisible, it prevents monsters from overlapping on top of the player.
- Implemented Ghost enemy
- Added new Possession affliction which causes the player to become not in control of their actions

6/22:

- Implemented Puff enemy which pushes enemies and players
- Added function linearActorPush() to facilitate new style of actor pushing. This function properly pushes the actors linearly until they are out of range of the pushee.
- Added GustyPuff and StrongGustyPuff variants of Puff. GustyPuff pushes in all directions and StrongGustyPuff is a tougher version of it.
- Added InvertedPuff and SpikedInvertedPuff enemies

6/23:

- Implemented Spider and ShootingSpider enemies
- Added Web (spiderweb) trap that is guaranteed to stun the player

6/24:

- Implemented PouncingSpider enemy
- Implemented Serpent enemy, a multisegmented monster
- Added multi-segmented monster infrastructure to allow for more multi-segmented monsters

6/25:

- Added CharredBombee variant which is slightly stronger than a regular Bombee but also immune to lava.
- Implemented ArmoredSerpents and introduced notion of monsters blocking attacks

6/26:

- Potential name for the game: Lost Arts
- Restructured level creation and moved the first floor into its own class. The Dungeon class now serves as a pseudo pure virtual class.
- Added Breakables subclass to Monsters which introduces breakable objects
- Fixed awkward shop lighting of the various objects

6/27:

- Began implementing Tutorial Level and World Hub
- Added some NPCs for the tutorial
- Created new callback function that allows for animations to end smoothly with their last frame
- Implemented NPC textbox prompting infrastructure

6/28:

- Implemented return to menu functionality from the World Hub
- Consolidated some of the sound effects into one function so that sound is not unusually loud when player is far away
- Partially implemented Memorizer NPC. Still need to hide the HUD when the player interacts with them.

6/29:

- Partially fixed some funkiness with WindSpell and Spring Traps pushing away the Serpents
- Rearranged peekDungeon() action ordering so that all objects/monsters move, then the lighting is updated.
- Removed SecondFloor's fight function since it was redundant.

6/30:

- Consolidated all dungeon map layouts (Tile vectors) in a single one held in the Dungeon base class.
