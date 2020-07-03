7/1:

- Implemented Passives and added them to the Inventory screen
- Added a buttload of new Passives and associated player properties
- Implemented Heavy attribute and applied it to work for Springs, Puffs, and Chargers
- Implemented Decoy class (subclass of Traps). Decoys attract monsters and monsters prioritize the decoy over the player.
- Decoy vector added to the Dungeon and associated functions. Monsters will do a greedy chase toward the decoy if they are in range.
- Notable passives added: SoulSplit, Heavy, BrickBreaker, BombImmune, SteelPunch
- Added more potion types
- Added virtual moveTo function to the Actors classes. Streamlines pushing monsters and their sprites to their appropriate locations.
- Implemented Passive that illuminates traps
- Updated level lighting to work more closely to how the Shop does. Zappers now have lit spark effects.

7/2:

- Overhauled and reorganized the Weapons class. Added LongWeapon and PiercingWeapon subclasses.
- Implemented Bo Staff
- Corrected error in enemy knockback revealed by the Bo Staff (enemies would be knocked on top of the player because it only took player's facing direction into consideration)
- Implemented Nunchuks. Need some identification that enemies have been stunned.
- Implemented the Estoc, a special lunging weapon
- Implemented the first metered weapon, the Jian which cuts in a ring around the player
- Implemented Claw weapon which acts like a hook that pulls monsters in. There's some puzzle potential here...
- Added CursedStrength trinket which lets monsters and traps move first

7/3:

- Implemented some new health potions
- Finally implemented BearTrap item and trap
- Added Cripple affliction (used for Bear Trap) which stuns the actor every other turn.
