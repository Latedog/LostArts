#include "GameObjects.h"
#include "Actors.h"
//#include <vector>

#ifndef DUNGEON_H
#define DUNGEON_H

#include <vector>


std::vector<char> chunks(std::vector<std::vector<std::vector<char>>> &c);
std::vector<std::vector<std::vector<char>>> mixChunks(std::vector<std::vector<std::vector<char>>> c);
std::vector<char> combineChunks(std::vector<std::vector<std::vector<char>>> &c);


const int MAXROWS = 18;
const int MAXCOLS = 70;

struct Tile {
	char top;
	char bottom;
	bool marked;
};

class Dungeon {
public:
	Dungeon();
	Dungeon(int smelldist);
	~Dungeon();
	void peekDungeon(int x, int y, char move);
	bool moveGoblins(int pos, int &shortest, int smelldist, int x, int y, \
		char &first_move, char &optimal_move, char prev);
	bool goblinInRange(int pos);
	void foundItem(int x, int y);
	void collectItem(int x, int y);
	void showDungeon();
	void unmarkTiles();
	std::vector<Player> player;
	std::vector<Goblin> goblins;
private:
	Tile m_maze[MAXROWS][MAXCOLS];
};

#endif