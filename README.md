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
