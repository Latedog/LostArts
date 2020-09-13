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
