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
	bool trap;
	char traptile;

	bool marked;
};

class Dungeon {
public:
	Dungeon();
	virtual ~Dungeon();

	void peekDungeon(int x, int y, char move);
	void checkActive(Tile dungeon[], int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters);
	void explosion(Tile dungeon[], int maxcols, std::vector<std::shared_ptr<Monster>> &monsters, int x, int y);
	void trapEncounter(Tile dungeon[], int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, int x, int y);
	void monsterDeath(Tile dungeon[], int maxcols, std::vector<std::shared_ptr<Monster>> &monsters, int x, int y, int pos);

	bool moveGoblins(int pos, int &shortest, int smelldist, int origdist, int x, int y, char &first_move, char &optimal_move, char prev);
	bool goblinInRange(int pos);

	void checkArchers(Tile dungeon[], int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	void moveArchers(Tile dungeon[], int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	bool wallCollision(Tile dungeon[], int maxcols, char direction, int p_move, int m_move);

	void foundItem(Tile dungeon[], int maxcols, int x, int y);
	void collectItem(Tile dungeon[], int maxcols, int x, int y);
	virtual void showDungeon();
	virtual void showText();

	Player getPlayer() const;
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

	std::vector<Player> player;
	std::vector<std::string> dungeonText;
private:
	Tile m_maze[MAXROWS * MAXCOLS];
	//Player m_player;
	std::vector<std::shared_ptr<Monster>> m_monsters;
	std::vector<std::shared_ptr<Objects>> m_actives;

	int m_level;

	int m_rows = MAXROWS;
	int m_cols = MAXCOLS;
};

class FirstBoss : public Dungeon {
public:
	FirstBoss(Player p);

	void peekFirstBossDungeon(int x, int y, char move);
	void monsterDeath(int x, int y, int pos);
	using Dungeon::monsterDeath;

	void checkSmasher(int x, int y);
	void move1();
	void move2();
	void avalanche();
	void move3();
	void move4();
	void move5();

	void moveSmasher();
	void moveSmasherL();
	void moveSmasherR();
	void moveSmasherU();
	void moveSmasherD();

	void resetUpward();
	void resetDownward();

	void fight(int x, int y);

	void showDungeon();
private:
	Tile m_boss[BOSSROWS * BOSSCOLS];

	std::vector<std::shared_ptr<Monster>> m_firstbossMonsters;
	std::vector<std::shared_ptr<Objects>> m_firstbossActives;
};
#endif