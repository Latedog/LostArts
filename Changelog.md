
1/12/2020:
- Redid dungeon class to remove dependencies on tile->top properties. In particular, the player is completely relegated to its own flag (tile->hero) which uniquely marks the position of the player.
- Afflictions removed from the Objects class and put into its own separate class named Afflictions. As a result, afflictions are removed from the checkActive() function and Actors gets its own checkAfflictions() function to check for this
- Flags for determining whether an actor can be burned, bled, etc. have been set in the constructors
