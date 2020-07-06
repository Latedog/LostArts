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
- Added LifeChest which contains strictly health items.
- Added Mattock and Mattock Head weapons. The Mattock can dig out walls but breaks into a Mattock Head.
- Added Mattock Dust item which is created from breaking the Mattock Head. Currently confuses nearby enemies when used, but might use it for something else later.
- Added new throwWeaponTo() function for the player that will easily allow for more throwable weapons.
- Added Options menu to the Main Menu which currently includes options to tweak various sound volumes.
- Added Options menu to the Pause Menu
- Converted most sounds to play using the global sound functions
- Began playing around with Resolution changing, but it requires much more fiddling with to get right. Will come back to this near the end.

7/4:

- Implemented the ability to rebind keys and reset them
- Redesigned the BoneAxe to be throwable.
- Redesigned the Katana to have an after attack effect.
- Implemented Boomerang weapon

7/5:

- Implemented FireCascade spell
- Implemented FireExplosion spell
- Implemented WindBlast spell
- Fixed a bug in the linearActorPush() function and created one in the Dungeon class (since that's where it should be)
- Implemented IceShard spell
- Implemented Shockwave spell
- Implemented RockSummon spell
- Changed trapEncounter() to accept 'endFirst' parameter so that it can look for RockSummon correctly
- Implemented HailStorm spell
- Implemented WindVortex spell

7/6:

- Implemented Dual Wielding bonus
- Major: Completely refactored Player class to accept general active item functions. All shield related functions were moved to the Adventurer and Spell functions were removed. Blocking is now generalized and all Monster and Trap functions that took this into consideration have been changed to reflect this.
- HUD now uses a set of generalized active item update functions
