8/1:

- Added SquareMovingTiles trap
- Fixed bug that would require an ActivePoisonBomb to be walked over twice to be activated
- Added new custom action functions to the Dungeon class that provide support for multiple actions and spawn actions
- trapEncounter, monsterTrapEncounter and singleMonsterTrapEncounter now check for ALL traps with a matching (x, y) pair
- Fixed bug with PoisonBombs that would cause player Dex to become extremely large. When a PoisonBomb is set off a poison is added to the player, but this adds a PoisonCloud trap which then poisons them on the same turn an additional time. This causes the Poison's adjust function to use m_dexTaken which hadn't been set yet in the afflict() function. The strTaken and dexTaken variables are now set to -1 by default. If these are -1 in the adjust function, then we don't do any str or dex adjustment.
- Fixed bug that would cause monsters that walked over PoisonWater to not be poisoned
