3/1/2020:
- Changed Player constructor to accept hp and weapon type to accomodate for new player types
- NPCs will now have extra dialogue when player has not fulfilled their needs.
- Fixed bug where Archers would be stunned from a distance when blocking from above with a normal shield.
- The first Dungeon was not doing an opposite-facing spring check when spawning springs. This is now fixed which should resolve all instances of springs facing toward each other.
- Fixed item collection in the shop where sprites would not be removed.
- Consolidated the Monster death() function to one for monsters that only drop gold. Monsters that drop other things upon death now have a function called deathDrops() in addition to death().
- Fixed bug where Broundabouts and Goo Sacks would jump back toward the player if they had pushed them away with a Wind Spell.
- Removed the Shop's peekDungeon() function and added construct/deconstruct ShopHUD functions to the Dungeon class. Also added isShop() identifier function.

3/3/2020:
- Implemented the Spellcaster character
- Added character select screen to main menu. The default Dungeon constructor no longer exists and now accepts a Player pointer to address this change.
- Fixed crumble tiles so that they do not break multiple times when an enemy is on top of one. Currently they break a little oddly if an enemy breaks them, as there is a delay since there is no check immediately for when an enemy walks off the tile like there is for the player.

3/22/2020:
- Created Shopkeeper NPC that currently only has dialogue. No special interactions yet.
