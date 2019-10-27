# SuperMiniRogue
MiniRogue Expansion

9/25:
  - Added item drops (life potion, armor, stat potion, chest) and most accompanying functions
  
10/04:
  - Created separate item inventory and item use menu
  - Changed Drops class to use polymorphic useItem function in place of individual derived class functions
  - Made Dungeon class's player and goblins vectors private and accomadated these changes in Game.cpp with getPlayer()
  - Converted the chunk vectors to be member functions of Dungeon class
  
10/08:
  - Fixed bug that could cause idol to not appear
  - Added reset feature

--More Monsters Branch--

10/11:
  - Added wanderers by utilizing polymorphic Monster vector
  - gameOver() monster death no longer hardcoded

10/17:
  - Fixed major bug that caused virtual functions to not work on the monster vector
  - Added archers with use of dynamic casting (checkArchers(), moveArchers(), wallCollision() )
  - Fixed small grammar issue when killed by monster that started with a vowel
  - Added help menu
  - Added chest hierarchy

10/19:
 ~~- BUG! Game occasionally freezes when moving (possibly related to archer movement)~~
  - Added objects vector for bomb item (checkActive(), explosion()) and added monsterDeath()

10/20:
  - Allowed bombs to break walls; changed level border walls to be unbreakable
  - FIXED: Game freeze caused by archers occasionally being surrounded by walls and monsters (no condition was set for being completely surrounded)
  - Player and idol now spawn on opposite sides of level
  - Added top and bottom layer chunk spawners and special chunks
  - Player now spawns with one bomb
  - RustyCutlasses were being spawned as ShortSwords
  - Added new dungeon for Smasher boss, stairs to reach new level, and level variable
  ~~- BUG: Reset button needs to be fixed~~
  - FIXED: Reset button

10/21:
  - BUG: Items can spawn on top of other objects and despawn them
  
 --Convert maps to 1d array--
 
 10/21:
  - Removed need for goblinsmelldistance parameter in Game constructor and from other derived classes
  - Resolved up/down nuance where the representations for each move were reversed
  - Converted 2d dungeon to 1d which allows ability to easily pass in dungeons to functions

10/22:
  ~~- BUG: Bombs don't damage the smasher and item inventory doesn't carry over~~
  - FIXED: Improper spawning of new players when entering new dungeons (used Player parameter for constructors)
  - BUG: Chests in the special room cause game to crash when trying to pick up

10/27:
  - Added dungeonText vector to properly display dungeon after a turn is completed
  - Fixed bug that caused quit command to not work after dying
  - Added help menu toggle
  - Smasher move1 implemented (up and down thwomp style crushing)
