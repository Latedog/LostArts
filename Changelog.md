
1/12/2020:
- Redid dungeon class to remove dependencies on tile->top properties. In particular, the player is completely relegated to its own flag (tile->hero) which uniquely marks the position of the player.
- Afflictions removed from the Objects class and put into its own separate class named Afflictions. As a result, afflictions are removed from the checkActive() function and Actors gets its own checkAfflictions() function to check for this
- Flags for determining whether an actor can be burned, bled, etc. have been set in the constructors

- HUGE: Added sprite action sequencing for monsters. This allows actions to be followed up one after another so that there is no more strangeness in sprite movement or conceived frame skipping during the smasher boss fight when archers are pushed around. To achieve this, every monster sprite is assigned the appropriate sprite upon its creation in GUI.cpp. Several additional sprite manipulation functions (prepended with 'queue') were added to accomodate this, and updateLighting() has, accordingly, been updated such that it does not use the monster_sprites vector anymore.

1/13/2020:
- Added intellect and luck stats to the Actors class
- Added puddles, firebars, and double-sided firebars. Puddles can remove burning if stepped on while on fire.
- Resolved major issue involving firebars/spinners where they would incorrectly attack or not attack the player. Firebars now have Objects that keep track of where their fireballs are, instead of using the tile->projectile tile, which removes the need for map boundary checking.
- Finished implementation of puddles and cleaned up the Spinner code

1/14/2020:
- Auto spike traps and trigger spike traps moved to their own functions in their respective trap classes. Instead of constantly adding and removing the spike sprites, I've assigned each trap 3 of them (active, primed, and deactive) and set their visibility according to the state they're in.
- Spike traps are now accurate when lighting is updated by using a new Traps vector given to each dungeon
- Lava now shows additional lighting effects when players are nearby
- Removed global dungeon variables (hiding in Game.h and Appdelegate.h, places they shouldn't be in) and added a new struct called Dungeons which holds one of each type of dungeon. However, this is almost entirely useless as there's really no need to pass around old dungeons, only the players of that dungeon. This can be removed to only include one type of each dungeon in each scene.
- Sound effects for pits, missing ones for spiketraps, and additional ones for archers have been added
- Stairs, buttons, and lava were all added to the new traps vector for lighting purposes
