
1/12/2020:
- Redid dungeon class to remove dependencies on tile->top properties. In particular, the player is completely relegated to its own flag (tile->hero) which uniquely marks the position of the player.
- Afflictions removed from the Objects class and put into its own separate class named Afflictions. As a result, afflictions are removed from the checkActive() function and Actors gets its own checkAfflictions() function to check for this
- Flags for determining whether an actor can be burned, bled, etc. have been set in the constructors

- HUGE: Added sprite action sequencing for monsters. This allows actions to be followed up one after another so that there is no more strangeness in sprite movement or conceived frame skipping during the smasher boss fight when archers are pushed around. To achieve this, every monster sprite is assigned the appropriate sprite upon its creation in GUI.cpp. Several additional sprite manipulation functions (prepended with 'queue') were added to accomodate this, and updateLighting() has, accordingly, been updated such that it does not use the monster_sprites vector anymore.

1/13/2020:
- Added intellect and luck stats to the Actors class
- Added puddles, firebars, and double-sided firebars. Puddles can remove burning if stepped on while on fire.
- Resolved major issue involving firebars/spinners where they would incorrectly attack or not attack the player. Firebars now have Objects that keep track of where their fireballs are, instead of using the tile->projectile tile, which removes the need for map boundary checking.
- Finished implementation of puddles and cleaned up the Spinner code

1/14/2020:
- Auto spike traps and trigger spike traps moved to their own functions in their respective trap classes. Instead of constantly adding and removing the spike sprites, I've assigned each trap 3 of them (active, primed, and deactive) and set their visibility according to the state they're in.
- Spike traps are now accurate when lighting is updated by using a new Traps vector given to each dungeon
- Lava now shows additional lighting effects when players are nearby
- Removed global dungeon variables (hiding in Game.h and Appdelegate.h, places they shouldn't be in) and added a new struct called Dungeons which holds one of each type of dungeon. However, this is almost entirely useless as there's really no need to pass around old dungeons, only the players of that dungeon. This can be removed to only include one type of each dungeon in each scene.
- Sound effects for pits, missing ones for spiketraps, and additional ones for archers have been added
- Stairs, buttons, and lava were all added to the new traps vector for lighting purposes. As a result, lava is now a trap object instead of a being a simple tile marker entity.
- The peek() functions were all renamed to peekDungeon() to make use of inheritance
- The getActives() and getDungeon() functions for the Dungeon class have been changed to return a reference instead of a const
- Collapsed overuse of checkActive and openDoor into the Dungeon class
- Added setName() to actors. Changed name of archers that spawn from using the fountain on floor two to the name "Guardian", which allows regular archers to be spawned.
- Removed unused WeaponMenuScene and ItemMenuScene classes from GUI

1/15/2020:
- Removed redundant getName(), set and getMaxHP() functions from the Monster class
- Added 22 different spring traps. 8 that bounce in a single direction, 1 that bounces in a random direction, 1 that bounces in a random cardinal direction, and 1 that bounces in a random diagonal direction. They can either be trigger or non-trigger so in total this is 22. Adjusted all player sprite updating to occur after the key press switch statements and updated the inaction lambdas as well to reflect this change since spring traps can move players when they don't press anything.
- Changed monsterTrapEncounter() to accept a Dungeon instead of everything else
- Added an isFlying() check to the monsters class to check whether or not they are affected by things on the ground
- Added the ability for monsters to bounce on spring traps
- The "spawn" was changed from a multimap to a vector of pairs. This is so that the actions are run in the order that they were received.
- Issue with boss dungeon pushing sprites was fixed by adding a bool variable (front) to insertActionToSpawn() which says to add the action to the front of the vector of actions instead of the back, thereby making it the first to execute. This uses a single queueMoveSprite() command instead of everywhere within the smasher move functions.
- checkActives() now only checks spring traps if they are both a trigger type and have been triggered. Does not yet get triggered by monsters. Springs are now activated in trapEncounter() and actors can be chained onto other traps after being bounced.

1/16/2020:
- Tweaked wallCollision() algorithm to correctly detect walls. This error went previously undetected since archers do not use wallCollision for checking next to them
- Sound effects added for turrets and spring traps
- Added cooldown to turrets, allowed players to block their shots, and they now only hit the first target the shot comes in contact with
- Spinners now have a chance to burn, and firebars only have a chance to burn
- Fixed bug where the isBurned flag was not being set when the player was burned
- Added moving blocks (not finished); invisible wall bugs and square movement implementation not there
- Fixed invisible wall where trap flag was being set on the block's spawn and not removed, and the enemy flag in the block's move() function was not correctly set
- Monsters are now killed instantly if pushed into a wall by a moving block
- Added a function to take in a single monster for trap encounters, instead of the searching the entire monster vector again
- Changed trapEncounter() to accept just a Dungeon and the trap's coordinates as the arguments
- Changed overloading []operator from m_maze[index] to this->getDungeon()[index] from new bug that surfaced after changing trapEncounter()
- Fixed repeating game over bug where the inaction scheduler was not being unscheduled. The HUDLayer gameOver() function now takes a scene as an argument to prevent this.
- Corrected lack of polymorphism when using items as well as weapons, and added new Freeze Spell item
- Removed unused Game.h and Game.cpp from the project and commented out all instances of showDungeon(), wield(), and use()

1/17/2020:
- Weapons now have ability functions to be called if the weapon has a special trait such as a bleed or freeze effect. Added m_hasAbility variable to Weapons.
- Added new player attack() function which takes in a dungeon and an actor to allow flexibility for weapon abilities
- Added sprite flashing to monster and player to give feedback on damage taken
- Commented out callUse() from all dungeons except the base Dungeon since polymorphism is used
- Added player hit sound effect for Bombee and fixed zapper sparks from being too small on the second and third floors
- Added bloodletting sound effect and blood dripping when bleed is applied. The damage taking effects have been turned into functions.
- Freeze was turned into its own affliction and monsters are now turned blue to indicate being frozen
- Changed freeze attack damage to be the strength of the player minus twice the armor of the monster (increased armor because of ice)
- Moved affliction flag setting inside the afflict functions
- MovingBlocks now crush the player and stun them for 1 turn
- Change monster and player str, dex, and hp values to make the gameplay more consistent
- If player succeeded on the attack roll, damage is now guaranteed to be at least 1 instead of 0.
- Changed Dungeon::callUse() to remove the extra bit just for bombs. Bomb's useItem() function does everything that needs to be taken care of now.
- Added forPlayer flag to Drops class to check if item is meant to be used on the player or on surroundings
- Added destructibility flag to Objects class for checking if an object or trap can be destroyed by bombs, etc.
- Added function trapRemoval() which destroys any traps that were queued up in m_trapIndexes and created a new explosion function that takes in a Dungeon and coordinates. trapRemoval() was created since removing a trap immediately after explosion would create problems since checkActives is still iterating through the same vector.
- Added earthquake spell and fixed boundary checking for 5x5 explosion attacks
- Added fireblast spell and made Spinners immune to burning

1/18/2020:
- Finished implentation of wind spell by modifying the pushMonster and monsterHash functions. You can't really tell when a monster has been pushed into a hole currently other than the sound, hopefully I can fix this. Need to fix spinner/zapper sprites as they become detached with the monster after being pushed.
- Added invisibility, ethereal, and teleport spells and affliction statuses. Invisibility wears off early if the player damages or is damaged by something. Ethereal status allows the player to walk through walls.
- Added flag to Monster class that checks if they chase the player as their style of attack. Used for ignoring player when they are invisible. Small "bug" where enemies (archers/knights) can move into the player if they are invisible.
- Added confusion potion (DizzyElixir) and appropriate flags/functions which increases player's dex temporarily at the cost of reversed movement
- Gave all items the image name to be used and changed item discovery to item_name which uses a string. This messes up level generation quite a bit, so it will need to be fixed later.
- Fixed shop after using item_name for it's actual purpose and included new shop_action variable to take its place;
- BUG: Shops currently don't have any mechanism for checking a full inventory when purchasing
- BUG: Moving blocks are killing player instantly when crushing them since ethereal check is raising a false positive. Need to let the ethereal affliction access to the dungeon to fix.

1/19/2020:
- Added preprocessor directives for adjust spacing depending on the window resolution size. Incomplete since HUD doesn't adjust with it (it's hardcoded).
- Stat potions now buff player only temporarily with new Buff affliction added
- Added level check to the action completion while loop to resolve the freeze if player inputs during the level transition
- Fixed non-moving player sprite on boss level by adding queueMoveSprite into the empty monsters vector check
- BIG CHANGE: The traps vectors have been changed from a vector of Objects to a vector of Traps and all functions have been changed accordingly.  Stairs and buttons are now classified as traps, and all traps have been polymorphimized into trapAction() functions. trapEncounter() can now simply call the trapAction function and the function takes care of the rest including sprite cleanup and tile flagging. FallingSpikes are now added to the trap vector instead of just the actives vector.
- Added PoisonCloud item and corresponding PoisonBomb trap which actives when passed over

1/20/2020:
- Finally made some real use of the tile->object object and picking up items now depends on it to reduce redundancy. To achieve this, flags for checking what kind of item the pickup is were set (isItem, isWeapon, isChest, isShield, isTrinket, isAutoUse).
- Player's shield is now polymorphic (is a shared_ptr) to simplify and reduce code
