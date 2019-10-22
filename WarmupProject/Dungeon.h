#include "GameObjects.h"
#include "Actors.h"
#include <vector>

#ifndef DUNGEON_H
#define DUNGEON_H

const int MAXROWS = 18;
const int MAXCOLS = 70;

const int BOSSROWS = 32;
const int BOSSCOLS = 26;

struct Tile {
	char top;
	char bottom;
	bool enemy;

	bool marked;
};

class Dungeon {
public:
	Dungeon();
	Dungeon(int smelldist);
	virtual ~Dungeon();

	void bossFight();

	void peekDungeon(int x, int y, char move);
	virtual void checkActive();
	virtual void explosion(int x, int y);
	virtual void monsterDeath(int x, int y, int pos);

	int getSmellDistance() const;
	bool moveGoblins(int pos, int &shortest, int smelldist, int x, int y, \
		char &first_move, char &optimal_move, char prev);
	bool goblinInRange(int pos);

	void checkArchers(int x, int y, int pos);
	void moveArchers(int mx, int my, int pos);
	bool wallCollision(char direction, int p_move, int m_move);

	void foundItem(int x, int y);
	void collectItem(int x, int y);
	virtual void showDungeon();
	virtual Player getPlayer() const;
	int getLevel() const;
	void setLevel(int level);
	
	virtual void fight(int x, int y);

	void unmarkTiles();
	int getRows() const;
	int getCols() const;

	std::vector<char> topChunks(std::vector<std::vector<std::vector<char>>> &c);
	std::vector<char> middleChunks(std::vector<std::vector<std::vector<char>>> &c);
	std::vector<char> bottomChunks(std::vector<std::vector<std::vector<char>>> &c);
	std::vector<std::vector<std::vector<char>>> mixChunks(std::vector<std::vector<std::vector<char>>> c);
	std::vector<char> combineChunks(std::vector<std::vector<std::vector<char>>> &c);

	std::vector<std::shared_ptr<Objects>> active;
private:
	Tile m_maze[MAXROWS][MAXCOLS];

	std::vector<Player> player;
	std::vector<std::shared_ptr<Monster>> monsters;
	//std::vector<std::shared_ptr<Objects>> active;
	int m_smelldist;
	int m_level;

	int m_rows = MAXROWS;
	int m_cols = MAXCOLS;
};

class FirstBoss : public Dungeon {
public:
	FirstBoss(Player &p);

	void peekFirstBossDungeon(int x, int y, char move);

	void checkActive();
	void explosion(int x, int y);
	void monsterDeath(int x, int y, int pos);

	void moveSmasher(int x, int y);
	void fight(int x, int y);

	void showDungeon();
	Player getPlayer() const;
private:
	Tile m_boss[BOSSROWS][BOSSCOLS];
	Player m_bossplayer;
	//std::vector<Player> m_bossplayer;
	
	std::vector<std::shared_ptr<Monster>> bossMonsters;
	std::vector<std::shared_ptr<Objects>> firstbossActive;
};
#endif