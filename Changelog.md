
1/12/2020:
- Redid dungeon class to remove dependencies on tile->top properties. In particular, the player is completely relegated to its own flag (tile->hero) which uniquely marks the position of the player.
- Afflictions removed from the Objects class and put into its own separate class named Afflictions. As a result, afflictions are removed from the checkActive() function and Actors gets its own checkAfflictions() function to check for this
- Flags for determining whether an actor can be burned, bled, etc. have been set in the constructors

- HUGE: Added sprite action sequencing for monsters. This allows actions to be followed up one after another so that there is no more strangeness in sprite movement or conceived frame skipping during the smasher boss fight when archers are pushed around. To achieve this, every monster sprite is assigned the appropriate sprite upon its creation in GUI.cpp. Several additional sprite manipulation functions (prepended with 'queue') were added to accomodate this, and updateLighting() has, accordingly, been updated such that it does not use the monster_sprites vector anymore.

1/13/2020:
- Added intellect and luck stats to the Actors class
