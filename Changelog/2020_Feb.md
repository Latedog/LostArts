2/1/2020:
- Fixed bug on boss where killing an archer would incorrectly flag smasher as dead since monsterDeath() was only suited for smasher previously.
- Fixed items lighting up on floor spawn and updated lighting upon spawn for the other two floors
- SecondFloor button now spawns on the higher edge of the inner ring
- Resolved issue where spiketraps would be lit up upon spawn by setting sprite visibility to false

2/2/2020:
- Fixed audio bug where music would not stop playing on second floors and beyond if player died from not making a move.
- Fixed Arcane Staff bug where player would retain int after casting and switching weapons.
- Potential fix for springs to prevent situations where two springs point toward each other, resulting in an infinite loop
- Partial fix for moving blocks pushing stacked enemies and player; does not do a recursive check, so a stack of three or more characters leads to weird pushing

2/3/2020:
- Added Dead Seeker, a stronger variant of the Seeker that is "smart" (does not walk into lethal traps)
- Added structure for fire roundabouts, which are lava immune (need sprite)
- Actually fixed infinite spring trap looping with oppositeSprings() recursive checker (of course, it doesn't work if a spring is completely surrounded by 8 other springs, in which case I'd have to resort to turning it into a trigger spring trap. But this probably won't ever happen).

2/5/2020:
- Added ItemThief enemy which snatches items or gold from the player and runs away with it
- For now, attemptSmartChase just uses diagonal movement checking, and {1, 2, 3, 4} no longer uses quadrant numbering system. it follow left to right, top to bottom in accordance with wanderers and the way queueMoveSprite was originally setup for sprite movement using {1, 2, 3, 4}.
- Added Monster function playerInDiagonalRange which is used for monsters that can move diagonally and that chase the player. Also fixed bugs and lag associated with the ItemThief.

2/6/2020:
- Added GooSack enemy which is more first level-friendly. Stationary, but can move when trying to attack player.
- Added 'Broundabout' enemy which is not so much a variation on the Roundabout as it is just reusing code. This enemy does not move quite like the roundabout, it moves back and forth, but can chase the player if they are nearby, like the GooSack, albeit cannot move diagonally.
- Completed implementation of Fire Roundabout and cleaned code for Roundabouts' and Knight movement.
- Added new NPCs class to allow for special side quests within the dungeons. Creature Lover is the first, who asks the player to fetch them a special monster!
- Nerfed VulcanRune to make the player only unburnable
- Renamed FirstShop to just Shop to reflect that it can be reused for other shops.
- Moved the object and monster vectors to protected from private so that I do not need to "redefine" the getters used for them. There was no difference between them anyway, so now the Dungeon base class is responsible for holding the rest of the dungeon's objects.
- Potentially fixed a crashing bug where Dungeon's assignment operator was not clearing out m_seq since m_spawn was used before.

2/7/2020:
- Changed GUI to reuse Shop1Scene so that the shop can be easily rendered again. The shop's content remain unchanged since there isn't any check yet for adjusted prices and new items.
- Weapon menu selection now wraps around edges (but will be nil when weapon inventory size is changed to 1)
- Fixed third floor level generation but added a @start and @end parameter to fillLevel() since the third floor has unusual spawning properties.
- Added BigLifePotion which is a better version of the LifePotion.
- Fixed bug where Bloodrite could heal past max and BrightStar could leave player with more than max hp.
- Added shop price adjustment multiplier based on level

2/8/2020:
- Added item descriptions and inventory screen so the player can receive brief information on their items.
- Fixed inventory selection box movement and added helper text to weapon and item menus.

2/9/2020:
- Item hash now checks for traps, so that items cannot land on top of traps
- Changed menu, level 2 and level 3 music tracks
- Updated help menu and added main menu options to the pause menu and game over screen

2/10/2020:
- Added boundary flag to Tile struct so that players cannot use the ethereal spell to go out of bounds

2/11/2020:
- monsterTrapEncounter() functions now account for monsters taking damage, as well as turrets since they check to damage monsters after checkActive is called
- Added effect to show that player is blocking with shield
- !!!!! Added new function to ActionManager.h and ActionManager.cpp named getNumberOfRunningActionsByTag(int tag) const which allows for updating only those functions whose tag corresponds to one that is a single-time action
- !!!!! Changed the way pausing works and instead push the pause scene onto the stack instead of replacing the current scene

2/16/2020:
- Added the other dungeon chunks that were included on the laptop.
- Added new update() function to ActionManager.h that allows for only updating specific tags
- Fixed new bug with infinite looping springs since the new chunks uses pieces that are not named SPRING. Changed oppositeSprings() to check for the trap_name as well, since all springs are generated with the name SPRING.

2/22/2020:
- Removed unused sprite vector member variables
- Added function for inserting new trap coordinates into the trap removal vector named insertForTrapRemoval() and made the vector itself  a protected data member.
- Moved lightEmitters vector to protected
- Added changeWeapon() function to the Dungoen class that does what player->wield() did
- Changed the Dungeon player and player vector to be shared_ptr's as well as those found in other appropriate functions. As a result, new player types can be created and the Adventurer class was created as the default type.
- Overhauled the GUI scene to use a single level scene in conjunction with the shop scene by adding appropriate new functions for settings the current dungeon, music, sprites, and timer speed.
- Renamed Level1Scene to LevelScene to reflect that only one scene is technically needed
- Completed GUI transition to two scenes (level and shop), moved all member variables in GUI.h to be protected/private and added functions to access NPC screens and devils water
- Fixed broken shop camera and dark floor bugs. Dark floor was related to settings the floor sprites to (0, 0, 0) in renderDungeon and the broken player camera was from not running the actions in the Shop dungeon (the shop scene incorrectly moved the player before).
- Shop1Scene was renamed to ShopScene and now inherits from LevelScene to reduce code bloat.

2/24/2020:
- Moved all sprite and sound effects to its own file FX.h and removed utilities.h from the project. randInt was temporarily moved to GUI.h
- Footstep sound checking removed from GUI.h to the end of the peekDungeon loop
- Added damageMonster() function to do any monster damaging instead of using the monster vector directly.
- insertTrapForRemoval() was changed to accept the index of the trap instead of its coordinates. This should potentially fix the bug with bombees exploding and crashing the game, as well as just being more logically sound.
- Fixed FallingSpikes to not use the spike_sprites vector and crashes resulting from this change. Previously the sprites and themselves were destroyed at the moments of impact, but they are now queued for removal later. Their sprites are now destroyed when their destructor is called.
- Tons of general code organization (moving members and functions to protected/private, deleting unused functions, inlining functions)
- Moved Smasher functions to the Smasher class instead of being in FirstBoss
- Created new member variable wall_type in _Tile_ struct to identify types of walls. This eliminates the need to use top/bottom/traptile for identifying door types and they have been changed as such.
- More code inlining for GameObjects and Actors
- Removed unused polymorphism functions from GameObjects and Actors such as move(), trapAction(), etc.
- Removed many char global constants

2/25/2020:
- Changed the trap removal container to be a set and added a destroyed flag for traps. This flag allows destroyed traps to not do anything if it is their turn later in the same loop.
- Fire Roundabouts now drop embers randomly which can only burn the player
- Moved monster checking loop to its own function
- Added actor removal functions akin to trapRemoval() and dead variable to the Monster class
- Made the Chests class pure virtual and removed redundant code in GameObjects (functions that take in the calling object's type and then calls the function again without it)
- Made Spinner and Firebar object member variables shared_ptrs instead of raw objects
