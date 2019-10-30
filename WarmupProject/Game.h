// Game.h

#include "Dungeon.h"

#ifndef GAME_INCLUDED
#define GAME_INCLUDED


class Game {
public:
	Game();
	
	void play();
	bool gameOver(Player p, Dungeon &dungeon);
	bool win(Dungeon &dungeon);
	void reset();
	void help(Dungeon dungeon);
private:
	Dungeon m_dungeon;
	
};

#endif // GAME_INCLUDED
