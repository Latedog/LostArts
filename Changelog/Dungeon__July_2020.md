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
- Moved tile initialization into a function called initializeTiles() which is called when the Dungeon constructor is called.

7/11:

- Updated Active Item HUD functions to allow the display of the Spelunker's Rock count. The Spelunker has a getRockCount function for this.
- Redid some shop/chest/dungeon item generation and monster generation
- Finally added an attack animation
- Added some new character sprites and redid more monster generation for the SecondFloor.
- Fixed bug where sprite blinking would cause a sprite to become invisible
- Bombs are now stackable

7/12:

- Added show and hideHUD() to the HUDLayer for use with the Memorizer NPC
- Fixed a bug where the Spelunker could throw rocks on top of Walls, making them irretrievable.
- Items and gold now fall down pits (added activeTrapAction to Pits)
- The Spelunker now auto picks up Rocks
- Added Dungeon sprite functions that allow for removing sprites with additional actions. e.g. Money and items can now appear to fall down pits before being removed.
- Implemented Acrobat character which has a dodge rolling/evasive active ability.
- Added superDead flag which indicates that a character cannot regain HP even if they were invulnerable (used for Pits).
- Removed unused weaponMenu functions in HUDLayer
- Added destructors for Dungeon, MenuScene, HUDLayer, and PauseMenuScene
- Added 'Scene' parameter to the constructors of all Dungeons. This allows a lot more flexibility in creating sprites early.
- Added globals for the level numbers
- Added Shrine class. Chests are implemented and Health recovery is partially implemented. Trinkets are not implemented.

7/13:

- Fixed Major bug involving the Serpent. Upon death, it called the monsterDeath function for the tail, which was causing problems later when monsters were being removed. Instead, we set the tail to be destroyed as it should have been.
- The Serpent now has a moveTo function that so that its tail is moved around correctly when the Serpent is pushed.
- The Braziers in the Shrine are now lit when the player walks close enough
- Removed the Shop's specialAction() function and added the light source in the constructor. This has the added benefit of updating the lighting right away.
- Fixed bug where Spinner fire sprites would become detached after being pushed by Puffs. To achieve this, the Spinner's member variables for the inner and outer fire sprites were removed and merged into the fire Object variables. We can do this now since the Dungeon now has the scene to work with, so adding the sprites are handled in the constructor.
- Firebars were given the same treatment as Spinners
- Zappers also changed to accept a Dungeon and add the sprites right away
- Auto spikes and Trigger spikes constructors now accept a Dungeon for the same reasons as above.
- Removed the code used for adding sprites for Spinners, Zappers, Firebars, and Spikes from the renderDungeon function in GUI
- Added Destructors for the SpikeTraps and TriggerSpikes
- Removed seemingly unused line of code in SpikeTrap activeTrapAction about reassigning itself to the trap vector for lighting purposes
- Inlined the set and get functions in the GameObjects and Actor class
- Removed unused addWeapon and wield functions from the Player class

7/14:

- Weapons now have a sharpness stat which is used when the player increases their weapon's damage. After N hits, the weapon reduces in sharpness, all the way to the base level.
- Removed dialogue and choice setter functions from the NPC class. Instead, the corresponding vectors are now protected.
- Removed the dialogue vector parameter from the add dialogue and useResponse functions in the NPC class.
- Partially implemented Enchanter NPC.
- Weapons now have an Imbuement variable used for enchanting weapons. We can specify the type of Imbuement by using the ImbuementType enum.

7/15:

- Added Trader NPC
- Functions for removing passives, single items, and trading weapons have been added to the Player class for use with the Trader.
- You can now loop through the pause menu
- Added TreasureChest which contains money
- Added spriteCleanup functions to the Monster and Traps class to properly remove any additional sprites they may have. Removed the destructors for Spinners, Zappers, Embers, and Firebars as a result. This should, hopefully, prevent any crashes related to the sequence vector trying to access sprites already removed.
- Most traps now require a a Dungeon in its constructor so that the sprites can be created. Now almost all traps are no longer created in the renderDungeon funtion in GUI. This result fixes a bug where if spiders created Webs over the staircase, stepping over it would cause the staircase to vanish.
- Spiders now wait one turn before trying to get primed again after priming.
- Fixed bug where PouncingSpiders wouldn't jump

7/16:

- Removed deprecated Trinkets and redid old ones to be upgradeable. There are now 6 different trinkets that can be upgraded.
- Implemented upgrade and change trinket functionality in Shrines.
- Tons of new player abilities have been added as a result of adding the new Trinkets (Relics). Among them: most items and spells that inflict burning, poison or freeze are now affected by harshAfflictions and afflictionOverride attributes. The Dungeon can now sometimes stop briefly through MatrixVision. The level timer can now be affected.
- Implemented weapon sacrifice for Shrines. equipWeapon and removeStoredWeapon were added to Player class for this purpose. Shrine functionality is now mostly complete.
- Moved Stackable functions and member variable into the Drops class. This is because a new passive was added that lets all items stack.
- Updated HUD functions to reflect this new passive's abilities.
- New affliction added: Fragile. Actors with this affliction die instantly if hit.
- Added 7th Trinket: Protection. Grants bonus affliction save rolls and chance to inflict new Affliction 'Fragile'.
- Fixed bug where hp would become negative upon death.

7/17:

- Added a temporary working fade transition. Currently there's a noticeable camera "jerk" once the transition completes, but it works. Fix will likely do with no repositioning the scenes and adjust all sprite placements accordingly (which will take some time to redo).
- Removed the monsterDeath function from the FirstBoss class
- Smasher's death now creates an idol object and assigns it to a _Tile_. This removes the last instance of the Tile 'bottom' variable.

7/18:

- All Chests now determine their item when they are constructed for polymorphism and game mechanical purposes. attemptOpen and openEffect functions added for chests that have special effects like TeleportingChest and HauntedChest.
- Added destructor to the Chests class since it now has an Object pointer member variable
- Partially implemented Favor system in Player class
- Added ForgottenSpirit enemy which is a special monster that kills the player instantly when touched. A 'spirit' flag has been added to the _Tile_ struct to facilitate this, since this monster ignores all other enemies and walls.
- Added ThunderCloud Spell and corresponding ThunderCloud trap.
- The light source vector now requires a name tag to identify the source more explicitly. As such, adding and removing light sources requires this tag to be provided.
- Fixed bug where items would not be stacked if player had FatStacks and their inventory was full.
- Fixed bug where MovingBlocks would be overlayed on top of the player if they were crushed.
- Removed queueMoveSprite and moveSprite functions that used a char parameter, as these were unnecessary.
- Added coordinate transformation functions to convert between sprite and dungeon coordinates. This consolidates all instances of these transformations into one place so that they can be changed easily.
- Fixed bug on "Third" floor where the unbreakable walls would be invisible--helped in part by this transition to transformation functions.
- Fixed bug where the ThunderCloud would not move vertically
- Redid Smasher's move function and consolidated all 4 directions into a single function.
- All door, trap, and wall sprites are now created when the Dungeon is first being constructed. A function for creating wall sprites was added.
- Dungeon constructor now accepts a pointer to a player. The player is no longer assigned anywhere else.
- Moved the randInt function into global.h
- Created new source files LootUtils.h/.cpp to handle loot tables and rarity.

7/19:

- Removed FirstBoss class's monster/player pushing functions.
- Removed the overloaded assignment operators for the Dungeons since they weren't used/needed.
- Removed peekDungeon function from FirstBoss and devirtualized the peekDungeon function
- Added WindTunnel trap

7/20:

- Added a 'range' parameter to linearActorPush to specify how far we check for monsters. This was needed to make WindTunnel work as intended. In addition, linearActorPush was not working correctly when pushing variable amounts of monsters, especially with the WindBlast Spell. This has been corrected, and now monsters are pushed correctly to give a "piled up" effect. Previously, monsters could be pushed a distance equal to the range that the first monster was supposed to be pushed, but this was incorrect behavior if there was a large gap between them.
- Added LightAbsorber trap
- Created a Wall class for all types of walls. Includes boundary walls and the Fountain. All created instances of walls are now Wall objects. This also resolves a problem where some wall sprites would not be illuminated because of imprecision in converting between sprite and dungeon coordinates. The Dungeon class now has functions for removing walls.
- Completely eliminated the wall_sprites vector
- Removed the monster_name, item_name, and overlap variables from the _Tile_ struct
- Reclassified Doors as Walls which also allows them to be destroyed.
- Added wallObject variable to _Tile_ struct to allow simple/fast access to walls if they need to be destroyed
- Eliminated the door_sprites vector. Since doors are now added to the Wall vector, they do not need to be checked individually when updating sprite lighting.
- Removed openDoor function from the Dungeon class and changed ThirdFloor door functions to not use the wall_type identifier.
- Eliminated wall_type identifier from the _Tile_ struct.
- Door checking that was done in the player move function was moved a new interact function for Doors.
