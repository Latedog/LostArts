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
