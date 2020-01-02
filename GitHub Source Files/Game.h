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
	void help(Dungeon dungeon, char c);
	void pause(char &c);
private:
	Dungeon m_dungeon;
	bool quit;
};



#endif // GAME_INCLUDED
