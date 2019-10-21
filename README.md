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

More Monsters Branch
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
  - BUG! Game occasionally freezes when moving (possibly related to archer movement)
  - Added objects vector for bomb item (checkActive(), explosion()) and added monsterDeath()

10/20:
  - Allowed bombs to break walls; changed level border walls to be unbreakable
