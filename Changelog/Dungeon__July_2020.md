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
- Added TheMadman character and Teleporter item
- Added functionality for multiple character selects
- Fixed a bug where menus would stay open when the player died
- HUD now updates correctly upon spawn
- Renamed ArcaneStaff's applyBonus function to giveBonus so that the applyBonus function works as intended.
- Implemented Greater and Superior Estoc
- Implemented Greater and Superior Katana
- Implemented Greater and Superior Zweihander
- Implemented Greater and Superior BoStaff
- Implemented Greater and Superior Nunchuks
- Implemented Greater and Superior Jian
- Implemented first types of stat-granting weapons: Radiant, Carbon Fiber, Enchanted, and Lucky
- Pike replaced the IronLance

7/7:

- Fixed small bug where enemies could be pulled on top of the player when using the WindVortex spell
- Implemented Hammer weapons with Greater and Superior variants
- Updated Boomerang to allow player to grab items and open chests at the peak of the throw.
- Implemented Pistol weapon
- Moved the Teleport Scroll into Drops instead of the Spell class
- Implemented MonsterIllumination and ResonantSpells passives
- Removed unsafe destroy() function from the Dungoen and Monster classes and replaced it with a flag for being destroyed. If a monster is destroyed we: Flag it as destroyed -> insert it for removal -> the death function is called -> the death() function sees that it was destroyed, so it doesn't drop any rewards
- Added HauntedChest
- Added TeleportingChest
- Implemented ExplodingChest
- Refactored chests so that they require x and y coordinates in the constructor, the open() functions are now void, and most chest functionality is handled directly in the open() functions.

7/8:

- Added Radiant, CarbonFiber, Enchanted, and Lucky versions of the ShortSword
- RustyCutlass now has durability but gives 1 turn of invulnerability on hit.

7/9:

- Added Stackable items class which allows, you guessed it, stackable items
- Changed the collectItem() functions to reflect this new change
- Added Firecrackers item
- Fixed a bug where if an item landed underneath a door, you wouldn't be able to open it
- Fixed a shop bug introduced by the new Chests open functionality where an item purchased from the shop could be collected an infinite number of times.
- Fixed a bug where the FireExplosion would attempt to give burning to a monster instead of the player.
- Implemented SmokeBomb item
- Made BearTraps stackable
- Added ignite attribute to the Traps class which indicates that it can be set on fire. The accompanying ignite() function activates the event that occurs when ignited.
- Added Braziers
- Added Matches item
- Webs are now igniteable
- Fixed a bug where you could pick up more than 5 items if the item was stackable.
- All fire spells can now ignite things
- Fixed a bug where Puddles would immediately dissipate because there was no check for turns == -1.
- Braziers can now be tipped over to produce Embers
- Embers can now burn monsters
- Removed useless trapAction() functions from the Firebars, Turrets, and MovingBlocks
- Firecrackers now have a small chance to ignite things
- Fixed a bug where Embers would burn the player instead of the monster
- !!!! All instances of insertActorForRemoval have been from everywhere except the Dungeon class. If a monster is killed or destroyed, a flag is set which will later indicate that it should be removed when checked in the checkMonsters() function.
- MovingBlock activeTrapAction() function has been cleaned up to use the linearActorPush function. Additionally, linearActorPush has a 'lethal' parameter for cases such as this where the actor should be killed.
- MovingBlocks now kill instantly and kill the first enemy against a wall if it would block them from moving.
- Implemented Spelunker character which starts with the Whip and holds Rocks as an active item. Rocks can be stacked infinitely, but cannot be switched out. Since Rocks have durability, all enemies have a chance to drop a Rock upon death.

7/10:

- Removed the HelpScene class and converted it to a simple function in the PauseMenu class
- Created a MenuScene class to manage the options menu. StartScene and PauseMenuScene both inherit from this new class.
- Volume control no longer requires hitting enter to adjust a particular volume.
- Globalized sprite scaling
- Fixed bug introduced with Player's sprite facing direction introduced by globalizing sprite scaling. The LevelScene now has a member variable to help it with deciding whether to flip the sprite or not.
- Removed useless default constructor for Pits
- Fixed a bug for DoubleFirebars where they wouldn't register hits on the player
- Adjusted FreezeSpell to only work within a limited radius.
- Removed all instances of insertTrapForRemoval that were not within the Dungeon class
- Redid the awful activeTrapFunction for FallingSpikes. Finally removed the const char global c_spike.
- Cleaned up a few Smasher functions
- The FirstBoss peekDungeon() function has been updated to reflect the updated changes in the Dungeon
- Webs now leave behind an Ember for a short time if it is ignited
- RockSummon can now tip over lit Braziers
- Embers now have animated flames
- Added new queueMoveSprite function which accepts float coordinates (For positioning the fire on Braziers currently)
- Matches now leave an Ember if the space was free (No wall or enemy)
- The HUD and item menu will now display the count of stackable items
- Updated Active Item HUD functions to allow the display of the Spelunker's Rock count. The Spelunker has a getRockCount function for this.
