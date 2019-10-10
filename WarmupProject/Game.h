// Game.h

#include "Dungeon.h"

#ifndef GAME_INCLUDED
#define GAME_INCLUDED


class Game {
public:
	Game(int goblinSmellDistance);
	int getSmellDistance() const;
	void play();
	bool gameOver();
	bool win();
	void reset();
private:
	Dungeon m_dungeon;
	int m_smelldist;
};

#endif // GAME_INCLUDED
