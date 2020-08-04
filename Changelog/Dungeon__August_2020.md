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
