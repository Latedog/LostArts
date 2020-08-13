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
