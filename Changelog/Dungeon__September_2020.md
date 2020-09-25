9/1: 

- Added enUS.txt file to hold all english text for future localization purposes
- Added utility functions to get NPC and item text from language text files

9/2:

- Converted all current NPC dialogue to pull the data from text files
- Converted item info display on the inventory screen to pull data from text files
- Added utility function to format item descriptions with proper line breaks
- Fixed a bug that caused the player sprite to still appear after dying with the Teleporter
- Revamped MenuScene to include a language selection option
- Menu positions are now saved to make menuing consistent. i.e. navigating to a previous menu will now place the selector at the position the user was previously at

9/3:

- Converted all plain text to .json file format for easier future localization purposes
- Included rapidjson c++ header library for json text parsing
- Added TextUtils class and utility functions to extract menu/text/npc information from json files
- Converted most if not all of in-game menu text to use the json text file
- Adjusted addLabel to calculate the position offsets instead of before the function is called
- Changed the game text font style
- Added function to MenuScene to create the selector sprite

9/4:

- Added GameTimers class to manage the various affliction timers that the player can accrue. This fixes the problem where they wouldn't stop if the player paused the game, etc.
- Tidied up code in MenuScene and PauseMenuScene classes
- Added early infrastructure for controller support
- Changed the design resolution to 1920x1080 and adjusted all game elements to reflect this change. The result is that everything looks crisper.

9/5:

- Explicitly prevented the Dungeon class from being copyable
- Optimized playerInRange function
- Corrected a small bug in formatItemDescriptionForDisplay that would cause text to extend outside of the box due to poor line splitting detection.
- Key labels (for keyboards only, for now) are now updated correctly when the player changes their controls
- Added some functionality for changing controls on gamepad. Currently there is no way to actually use a controller yet.
- Removed unused factory code from MenuScene (create, etc.) and moved its constructor into protected

9/6:

- The HUDLayer class is now given a LevelScene pointer during LevelScene creation
- Replaced all HUD functions that took in a keyboard event listener parameter and changed it so they no longer do.
- Removed the gameOver HUD function that took in a Scene parameter and changed scheduleTimer() in the LevelScene class to no longer use this function.
- Added infrastructure to support complete controller functionality

9/7:

- Added LightSource structure to the Dungeon class
- Reclassifed Chests as Objects instead of Drops
- Added Usable and AutoUsable subclasses of Drops to eliminate the flags that were used for these item types
- Drops is now an abstract base class

9/10:

- Fixed a crash when the active item HUD was being destroyed due to a recent change
- Removed the x, y parameters from the peekDungeon function since they were no longer needed
- Opening and closing the item menu without using anything no longer costs a turn
- Removed the Dungeon parameter from all HUDLayer functions and instead added an accessor to the Dungeon pointer in the LevelScene class
- The GUI classes are now better encapsulated. For instance, it's made clearer that we can only create various scenes with their respective createScene/create functions.
- Removed the "quick" member variable from the HUDLayer class because its existence made no sense
- Moved all persistent-type HUD labels into their own container and replaced all "loose" labels accordingly, with the exception of the weapon cast key label
- Renamed the player HUDLayer member variable to m_player
- Removed the player and music id parameters from the PauseMenuScene class

9/11:

- Added MonsterTransform item
- Moved monsterDeath into public to allow MonsterTransform to call it early so that the enemy flag is not unflagged when the new monster is created.
- Added SleddingScarf item along with FactoryTile traps
- Added functionality to use the SleddingScarf item. The game stops briefly and allows the player to place special FactoryTiles for a short duration.

9/12:

- Added functionality to simulate a zoom feature
- Replaced the existing misc_sprites vector with a better one that holds Dungeon coordinates since converting between sprite to dungeon coordinates was lossy.

9/13:

- Removed the isPlayer and isMonster flags from the Actor class and replaced them with virtual versions
- Moved all passive/item abilities granted to a struct for simpler data copying and storing
- Added a Player copy constructor to be used with DNASplitter

9/17:

- Added virtual clone functions to the Player and Weapon classes that call copy constructors to create unique instances of each type. This is to prevent weird behavior from shared instances of Weapons.
- Removed the flags for indicating that a Player had an active item and replaced it with a more general function
- Removed the activeItem member variable from the Player class and created a virtual class which instead returns the Player's stored active item. This is done to reduce repetitive code in which we would need to assign an active item twice, essentially.
- Removed "PiercingWeapon" type weapons (Iron/Golden LongSwords)
- Removed Golden and Blood ShortSword
- Added a new usePattern function for the Weapon class to account for Player clones
- Added a check in the GUI factoryTileCreation function to prevent multiple instances of it occurring at the same time due to Player clones
- Added new useItem functions for all Drops to account for Player clones

9/18:

- Added getPlayerAt function to the Dungeon class to return the correct player when we also need to check for clones
- Clones are now accounted for when lighting is checked
- Added helper functions to the Dungeon class to retrieve the closest Player, including clones, as well as returning the coordinates of all players
- The player and any new clone now has their hp cut in half when first duplicated.
- Adjusted Monster functions (playerIn..., chase, attemptGreedyChase) to account for clones
- Made a slight adjustment to the SandCentipede to correct behavior. If the player stood in one spot, it would attack them, move away, and then move back to their spot. They no longer move away if the player does not move.
- Adjusted WatcherStatues, Springs, Spikes/Spiketraps, Turrets, Firebars, and Quicksand to account for clones
- Modified most, if not all, of the monsters to account for clones. The behavior should still be unaffected when the player is not cloned, but if clones exist, monsters will simply try to look for the closest player.
- Fixed Katanas to account for clones
- Replaced almost all instances of pairs of ints with Coords for readability and consistency
- Added an indicator for Drops (Usable items) that shows if they were used so that it can be removed from the Player's inventory. Previously, it was assumed that the item is used once useItem() is called, but this change allows for greater flexibility in item types and behaviors.
- The Buff affliction is now stackable to allow multiple Buffs at the same time
- Added Banana item and Banana Peel trap
- Fixed sprite Z ordering issue with SkyCrasher
- Added BananaBunch item. The Banana class's throwBananaPeel was made static and BananaBunch is now a friend of Banana.
- Fixed a bug that prevented multiple Buffs from being applied to the Player
- Fixed a bug that would causes BananaBunches/Bananas to be consumed immediately if the Player has the FatStacks passive

9/19:

- Removed the older useItem and usePattern functions from Drops and Weapons that did not have a Player parameter
- Fixed a bug that would not trigger Chest opening effects such as HauntedChest

9/20:

- Removed functions: blockedDamageReduction and successfulBlock from the Player class
- Renamed didBlock to didBlockAttackTo and added a damage parameter
- Removed the Adventurer specific code from the Player's attack function and generalized its functionality
- Removed function successfulAttack from the Player class and instead changed it to work with checkExtraAttackEffects. This declutters the code and makes the intention of such functions more clear.
- Removed unnecessary functions from the various Player subclasses
- Removed function setBlock from the Player class and m_blocking member variable. This has been replaced by the function resetFlags as a more general purpose use.
- Removed the function setRelicFlag and m_hasRelic variable from the Player class and simplified implementation
- Removed m_hasAbility variable and accompanying functions from the Shield class
- Fixed bug where flying actors would get sucked into Quicksand
- Added chest creation function to the Dungeon class for streamlined usage

9/21:

- Added Player parameter to Monster's reactToDamage function to account for clones
- Fixed bug where the isMonster function returned false for Monsters
- Added many new sprite related functions for allowing Coords/Vec2 parameters as well as offsets
- ShieldMasters no longer have their shields added as misc sprites

9/22:

- Updated function transformSpriteToDungeonCoordinates
- Removed "raw" transformDungeonToSpriteCoordinates functions that took in ints/floats
- Added conversion operator from Coords to cocos2d Vec2's
- Added addition operator for Coords
- Made CrumbleFloor an abstract class and added CrumblePits to replace the previous implementation
- CrumbleFloors no longer use a pointer to an Actor to track if it was walked over
- Added trapActionCheck function to the Traps class to check if it's destroyed before attempting to act
- Made function trapAction in the Traps class protected
- Fixed game crashing bug caused by calling trapRemoval too early (inside trapEncounter).
- Lava now turns into Laze when met with a fluid. Laze is a poisonous plume of smoke that deals poison damage and also poisons.
- Fixed game crashing bug caused by opening a TreasureChest since the open function was incorrectly devirtualized

9/23:

- Fixed a misspelling in "Factory Tile Timer" that would cause multiple instances of SleddingScarf to be activated at once
- Simplified Archer and Pikeman code and many other various monsters
- Removed raw int parameter utility functions of type positionInXRange and getFacingDirection, and replaced all instances wtih ones that use Coords parameters
- Archers now wait 1 turn after un-priming before being able to become primed again

9/24:

- Improved code for Player's chainLightning
- Removed Dungeon's getDungeon function
