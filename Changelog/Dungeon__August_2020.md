8/1:

- Added SquareMovingTiles trap
- Fixed bug that would require an ActivePoisonBomb to be walked over twice to be activated
- Added new custom action functions to the Dungeon class that provide support for multiple actions and spawn actions
- trapEncounter, monsterTrapEncounter and singleMonsterTrapEncounter now check for ALL traps with a matching (x, y) pair
- Fixed bug with PoisonBombs that would cause player Dex to become extremely large. When a PoisonBomb is set off a poison is added to the player, but this adds a PoisonCloud trap which then poisons them on the same turn an additional time. This causes the Poison's adjust function to use m_dexTaken which hadn't been set yet in the afflict() function. The strTaken and dexTaken variables are now set to -1 by default. If these are -1 in the adjust function, then we don't do any str or dex adjustment.
- Fixed bug that would cause monsters that walked over PoisonWater to not be poisoned
- Separated standard doors into their own class: RegularDoors
- Added BlastDoors, which cannot be destroyed by explosives
- Added an addDoor function to the Dungeon class for easier door creation
- Added LinearMovingTile traps

8/2:

- Added WatcherStatue series traps: FireballWatcher, FreezeWatcher, DartWatcher, CrumbleWatcher, SpawnWatcher, GuardianWatcher
- Added Watcher enemy that awakens from the GuardianWatcher trap
- Added hasLineOfSight game utility function
- Added LavaGrating trap
- Changed the Rarity enum to be an enum class
- Removed the getAnimation functions from the Dungeon class since these were moved into GameUtils
- Added another hasLineOfSight function that is agnostic to a particular direction--calculates the appropriate direction when called. Replaced the wallCollision function used by the Knight with this function.
- Cleaned up Turret code further by using the hasLineOfSight function. As a side effect, Turrets now do not trigger unless they have a clear line of sight with the player
- Removed the clearLineOfSight function from the Monster class and replaced each instance with hasLineOfSight
- Added playerInLinearRange function to GameUtils
- Cleaned up the moveMonsterRandomly code
- Cleaned up various monster movement code
- Renamed attemptSmartChase to attemptAllChase. Simpler functions are now used to call these chase functions.
- Added diagonal directions to the drectional offset functions in GameUtils
- Code cleanup for Spinners, Zappers, and Firebars

8/3:

- Fixed bug that caused single animations to appear in weird spots before moving to the correct position on screen: Changed runSingleAnimation to use teleportSprite instead of queueMoveSprite
- Added Shrinekeeper enemy
- Fixed inconsistency in directionIsOppositeTo where directions 1-4 did not follow non-quadrant rules
- Fixed bug in moveMonsterRandomly that would cause monsters to move out of bounds
- Added getFacingDirectionRelativeTo and isMovementAction utility functions
- Added Wisp, LightningWisp, and Grabber enemies
- Added conditionals for the Stuck affliction (Used by Grabber enemy) in player move function and Estoc's pattern function
- Added conditional in moveMonsterRandomly which allows movement by flying enemies even if they move on top of a trap
- Added EvilShadow enemy
- Added AbyssalSummoner enemy and corresponding accompanying AbyssalMaw trap

8/4:

- Added universal support for getFacingDirectionRelativeTo
- Wisps now use getFacingDirectionRelativeTo to determine their shooting direction

8/5:

- Removed the pushPlayer function from the Dungeon class
- Cleaned up WindSpell code
- Renamed WindSpell to WhirlwindSpell
- Added DamageType parameter to the fight() functions
- Gave Weapons a DamageType member variable and cleaned up the weapon code
- Added CrawlingSpine enemy and Goop trap
- Added MagicalBerserker enemy
- Added AngledBouncer enemy
- Removed the dungeon parameter from the Actor moveTo functions and from the player successfulAttack function

8/6:

- The Broundabout now inherits directly from Monster instead of from Roundabout
- Removed the dungeon parameter from ALL Monster functions
- Removed the moveSegments and addSegments functions from the Monster class
- Removed the dungeon parameter from ALL player functions
- Removed the player parameter from Monster's moveCheck, playerIn(Diagonal)Range and playerIsAdjacent functions
- Removed dungeon parameter from all NPC functions
- Modified attemptChase and attemptAllChase to set an optimal move direction when player is already adjacent
- Removed dungeon parameters from all extraneous monster functions
- Renamed the Mounted Knight to Pikeman
- Adjust the Pikeman to attack up to two spaces in front of them while charging

8/8:

- Added CombustionGolem enemy and associated MiniEruption and Combustion traps
- Made CombustionGolem immune to explosions
- Added playerIsAdjacent utility function
- Added MalevolentPlant trap
- Added WaterSpirit enemy
- Fixed a bug where Embers would be doused twice which would cause the game to crush because the sprite is attempted to be removed twice
- Added Ice trap and freeze functions to the Traps class
- Water and Puddles can now be frozen
- Added FrozenPuddle trap
- Cacti can now be ignited
- Matches and Webs now check for all traps on a tile to be ignited
- Added a buffer parameter to playerInBufferedLinearRange
- Added ConstructorDemon enemy and DirtMound traps
- Implemented Piranha enemy
- Added dry run greedy chase functions to the Monster class. These attempt greedy chases without actually moving the monster.
- If water is poisoned, it now checks to spread among adjacent Water tiles
- Fixed bug in PoisonBomb and PoisonCloud where the x and y coords were switched when the checking for traps to poison

8/9:

- Added Firefly enemy
- Added a diagonals parameter to moveMonsterRandomly
- Organized Monsters into their expected respective floors
- Changed all reference dungeon parameters to pointer parameters in the Monster functions
- Added rollMonster function that chooses a random monster appropriate for the floor
- Added getCardinalFacingDirectionRelativeTo utility function
- Added TriHorn enemy
- Added Sand trap
- Added Blindness affliction
- Added SandBeaver enemy. Still need to implement a blindness effect when player is blinded.
- Added SuperRoot item. Implemented new style of use item duration that uses time-based duration.
- Added isHarmful identifier to the Afflictions class that helps to identify afflictions to "block" with SuperRoot
- Added TimedAfflictions subclass to the Afflictions class
- Added TimedBuff and TimedHeal TimedAfflictions

8/10:

- Added TumbleShroom enemy and Spores trap
- Added Slipped affliction that simply rotations the sprite back upright. Used for when things slip on traps.
- Removed the sprite rotating done in checkAfflictions since it doesn't belong there
- Added BenevolentBark enemy
- Added attackedEnemy flag to the Player class which tracks when the player has attacked an enemy. Used for BenevolentBark enemy.
- Added enum class StatType
- Added increase/decrease StatBy functions to the actor class for simpler stat changing
- Added RPGInABottle item that temporarily allows the player to gain experience on kills, which levels them up
- Added experience gain related functions to the Player class for RPGInABottle
- Added a charisma stat to the Actors class
- Added Tick enemy and Ticked affliction
- Added DamageType parameter to damagePlayer function. Monsters now have a DamageType characteristic.

8/11:

- Implemented Wriggler enemy
- Added extraDeathEffects function to the Monster class
- Implemented BarbedCaterpillar enemy
- There is now feedback when the player hits the Serpent's tail to indicate that the entire monster is being damaged. (Added line that allows player to directly attack the monster segment in fight() ).
- Removed removeSegments function from the Monster class. It was only used by Serpents, so this functionality has been absobred into the extraDeathEffects function for Serpents.
- Added reactToDamage function the Monster class to handle special cases for when a monster is attacked.
- TumbleShrooms and MagicalBerserkers had their functionality in canBeDamaged moved into reactToDamage functions. This better reflects the behavior that they must be damaged for the effect to trigger. Previously, it triggered even if the player missed.
- Reduced the Player's attack function into a single if statement for attacks. A separate branch for stunned and frozen was unnecessary.
- Added ExoticFeline enemy

8/12:

- Fixed a bug with the Charger enemy that would cause its sprite to detach after attacking the player
- Added SandAlbatross enemy and DustDevil trap
- Messed around with throwWeaponTo and added a temporary throwing animation

8/13:

- Moved randInt and randReal functions to GameUtils from global.h
- Removed getPlayerVector function and all instances
- Removed sprite vector parameter from addSprite and adjusted runAnimationWithCallback to account for this change
- Removed Monster pointer variable from CreatureLover
- Moved misc_sprites variable into protected in the Dungeon class
- Removed the getMonsters and getTraps functions and replaced them with monsterAt/trapAt and respective size functions
- Added trap removal checks in trapEncounter and monsterTrapEncounter
- Removed all instances of the single findTrap function and removed the endFirst parameter from trapEncounter
- Removed last public instance of monsterDeath and made monsterDeath protected
- Added createMonsterByName utility function

8/14:

- Changed Firebar sprites to use the Ember animation
- Firebar fireballs now become deactive for a short time after the player runs into one
- Moved the guardian check for the Lava level into specialActions()
- Removed the monsterDeath function from the current SecondFloor and devirtualized the monsterDeath function
- Fixed a "bug" with Smasher which could cause it to get stuck. It checked for wallObjects on the boundary, but the level boundaries are not assigned to the wallObjects during level creation.
- Fixed inconsistencies in SpikeTrap and TriggerSpike trap code
- Fixed spike trap spawns for the Smasher fight causing them to be invisible
- Removed the checkMonsters and fight functions from FirstBoss and devirtualized these functions
- Removed getDirectionRelativeTo function and old linearActorPush function from the Dungeon class
- Updated WindVortex trap to use the getFacingDirectionRelativeTo function to clean up the code
- Finally cleaned up MovingBlock code and separated them into Linear/Square variants
- Fixed bug with FallingSpikes, causing them to sometimes go through the player

8/15:

- Added FacelessHorror, ToweringBrute, SteamBot, and Swapper enemies
- Fixed a game-crashing bug with RPG in a Bottle. RPG and SuperRoot both used Dungeon parameters to adjust their timers, but if the dungeon was deconstructed, then it would attempt to dereference an object that is now null. To fix this, SuperRoot and RPGInABottle now both give the player special afflictions since all that needed to happen was to apply a timer that could count down.
- On that note: Added ExperienceGain and AfflictionImmunity timed afflictions

8/16:

- Added offset parameter to setDirectionalOffsets
- Added playerInRectangularRange utility function
- Finally implemented SkyCrasher without infinite looping or overlapping

8/17:

- Added ShieldMaster enemy
- Defined Coords struct which holds x and y coordinates.
- Added new chase() function to the Monster class which implements A*. This was done because the current chase function has performance issues when there are multiple enemies that use these functions from afar. This new function performs much better.
- Defined hash function for Coords struct, a heuristic for the A* search, and another helper utility function for A*.
- Replaced all instances of attemptChase and attemptAllChase with the much faster chase() function
- The new pathfinding algorithm is now capable of making optimal moves toward the player regardless of enemy configurations, but without going all the way around a structure because there was another enemy between itself and the player
- Added PseudoDople enemy
- Fixed a bug where Water would not remove burns from the Player if they were already standing in water
- Water can now remove burning from monsters
- Added Flare enemy and FlareCandle trap

8/18:

- Adjusted setDirectionalOffsets to use the offset parameter for diagonal movement
- Added BladePsychic enemy and PsychicSlash trap (its attack)
- Added Electromagnetizer enemy

8/19:

- Added Disabler enemy, Disabled afflictions, and DisablingField trap
- Fixed a major bug with Timed afflictions where they would immediately stop working after one turn. The bug was that the m_exhausted variable was not being set in the new Affliction constructor, thus it would inadvertantly cause the afflictions to be destroyed, and thus the values became garbage. The m_exhausted member variable is now set to false in the class declaration.
- The Monster chase() function now works nondeterministically to add some randomness to their movements. Previously, they would consistently move in the same pattern which could block the player's path constantly.
- Added IncendiaryInfuser enemy, Incendiary affliction, and ActiveFireBomb trap
- Changed Monster extraDeathEffects to check for the Incendiary affliction
- Fixed a bug that could allow the player to cheese the game by pausing and unpausing repeatedly. If the player pauses, the level timer is not unscheduled but is instead paused. It is unpaused when the player unpauses.
- Added DashMaster enemy

8/20:

- Added ACIDIC DamageType
- Added AcidicBeast enemy and Acid trap
- Added getCoords function for diagonals only to be used in the A* algorithm
- Added another bool parameter to chase to allow for diagonal movement only
- Added a reverse parameter to setDirectionalOffsets to account for confused movement
- Added DarkCanine enemy
- Added Z Order constants for proper sprite Z ordering. Adjusted all z sprite orderings to account for this change.

8/21:

- Removed multisegment functions and variables from the Monster class
- Removed the Actors parameter from the activeTrapAction function
- Did some encapsulation cleanup on the Traps and Monster classes
- The PsychicSlash is now blockable
- Added CRUSHING and LIGHTNING DamageTypes
- Added LightningStriker, FlameArchaic, MasterConjurer, AdvancedRockSummoner, AscendedShot enemies
- Added accompanying FlameArchaicFirePillars, MegaRock, and ReflectiveShot traps
- Added RoyalSwordsman enemy

8/22:

- Added LightEntity enemy and LightBeam trap
- Fixed SandCentipede to only move when in Sand
- Removed m_maxTurns member variable from the Afflictions class
- Added Wet affliction
- Water now adds the Wet affliction instead of the other nonsense that was done before
- Sand now has a chance to spawn SandCentipedes on top of them in the constructor
- Changed the SandCentipede deathDrops to extraDeathEffects for correct usage
- Added item and passive helper functions to the Player class to remove reliance on the vectors
- getRelic() in Player class now returns by value instead of by reference
- Removed getItems and getPassives from the Player class
