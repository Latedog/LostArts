// Game.h

#include "Dungeon.h"

#ifndef GAME_INCLUDED
#define GAME_INCLUDED


class Game {
public:
	Game(int goblinSmellDistance);
	int getSmellDistance() const;
	void play();
	bool gameOver(Player p);
	bool win();
	void reset();
	void help();
private:
	Dungeon m_dungeon;
	//FirstBoss m_firstBoss;
	
	int m_smelldist;
	int m_level;
};

#endif // GAME_INCLUDED
