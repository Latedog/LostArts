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
