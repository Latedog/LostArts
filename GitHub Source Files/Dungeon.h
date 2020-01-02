#include "cocos2d.h"
#include "GameObjects.h"
#include "Actors.h"
#include <vector>

#ifndef DUNGEON_H
#define DUNGEON_H

const int MAXROWS = 18;
const int MAXCOLS = 70;

const int MAXROWS2 = 38;
const int MAXCOLS2 = 78;

const int MAXROWS3 = 38 + 3;
const int MAXCOLS3 = 80 + 5;

const int BOSSROWS = 24;// 32;
const int BOSSCOLS = 26;

struct _Tile {

	// :::: HIERARCHY OF TILE LAYER CONTENTS ::::
	//		-----------------------------
	//		Smasher		: TOP			 2
	//		Actors/Walls				 1
	//		Projectiles					 0
	//		Items						-1
	//		Stairs/Traps				-2
	//		Extras						 x
	//		Floor		: BOTTOM		-4
	//

	char upper;			//	  ^	   layer for smasher to sit in
	char top;			//	  |	   walls, player, monsters, etc.; Also used for doors
	char projectile;	//	  |	   objects that fly through the air
	char bottom;		//	  |	   items, weapons, etc.
	char traptile;		//	  |	   stairs, traps, lava, etc.; Also used to save door type.
	char extra;			//	  |	   for things like active bombs, or other odds and ends


	// tells if there is already something present in a particular spot
	bool wall;
	bool item;
	bool trap;
	bool enemy;
	bool hero; // the player

	// tells if there's overlap of multiple objects(spikes atm), and counts how many instances
	bool overlap;
	int overlap_count;

	// Normally, monsters cannot overlap, but this frequently can happen during the smasher boss fight.
	// So this is used for telling if smasher was on top of an enemy and prevents enemy overlap bug.
	bool enemy_overlap = false;
	int enemy_overlap_count = 0;

	// for recursive backtracking optimization in shortest path algorithm
	bool marked;
};

class Dungeon {
public:
	Dungeon();
	Dungeon(int level); // used to prevent previous dungeons from being generated
	virtual Dungeon& operator=(Dungeon const &dungeon);
	_Tile& operator[](int index);
	virtual ~Dungeon();

	void peekDungeon(int x, int y, char move);

	// sprite manipulation
	cocos2d::Sprite* findSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y);
	void setSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, char move);
	void moveSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, char move);
	void moveSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y, char move);
	void moveSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int cx, int cy);
	void moveAssociatedSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int X, int Y, int x, int y, char move);
	void addSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image);
	void addSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image);
	void removeSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y);
	void removeSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y);
	void removeAssociatedSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int X, int Y, int x, int y);

	virtual void checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters);
	virtual void explosion(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int x, int y, int pos);
	void trapEncounter(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, int x, int y);
	void monsterDeath(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int pos);
	
	virtual void enemyOverlap(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y);
	virtual void itemHash(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int &x, int &y);

	// Enemy movement
	bool checkGoblins(std::vector<_Tile> &dungeon, int maxcols, int pos, int &shortest, int smelldist, int origdist, int x, int y, char &first_move, char &optimal_move, char prev);
	bool playerInRange(int mx, int my, int pos, int range);
	void moveGoblins(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, char move, std::vector<std::shared_ptr<Monster>> &monsters);

	virtual void checkArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	virtual void moveArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	bool wallCollision(std::vector<_Tile> &dungeon, int maxcols, char direction, int p_move, int m_move);

	void moveWanderers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	void moveZappers(std::vector<_Tile> &dungeon, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	void moveSpinners(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);

	void moveRoundabouts(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);

	void checkMountedKnights(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	void moveMountedKnights(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);

	void moveSeekers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, char move, std::vector<std::shared_ptr<Monster>> &monsters);
	/// end enemy movement

	virtual int findMonster(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, std::vector<std::shared_ptr<Monster>> monsters);
	virtual void fight(int x, int y);

	void foundItem(std::vector<_Tile> &dungeon, int maxcols, int x, int y);
	void collectItem(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y);
	virtual void showDungeon();
	virtual void showText();

	Player getPlayer() const;
	void setPlayer(Player p);
	int getLevel() const;
	void setLevel(int level);
	void unmarkTiles();

	virtual std::vector<_Tile> getDungeon() const;
	virtual int getRows() const;
	virtual int getCols() const;
	virtual std::vector<std::shared_ptr<Monster>> getMonsters() const;
	virtual std::vector<std::shared_ptr<Objects>> getActives() const;
	virtual void callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index);

	// Sprite setting
	void setPlayerSprite(cocos2d::Sprite* sprite);
	void setMonsterSprites(std::vector<cocos2d::Sprite*> sprites);
	void setItemSprites(std::vector<cocos2d::Sprite*> sprites);
	void setTrapSprites(std::vector<cocos2d::Sprite*> sprites);
	void setProjectileSprites(std::vector<cocos2d::Sprite*> sprites);

	void setSpikeProjectileSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites);
	void setSpinnerSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites);
	void setZapperSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites);

	void setWallSprites(std::vector<cocos2d::Sprite*> sprites);
	void setDoorSprites(std::vector<cocos2d::Sprite*> sprites);
	void setScene(cocos2d::Scene* scene);

	// Level generation
	std::vector<char> topChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> middleChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> bottomChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);

	std::vector<std::vector<std::vector<char>>> mixChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols);
	virtual std::vector<char> combineChunks(std::vector<std::vector<std::vector<char>>> c);

	// BEGIN member variables
	std::vector<Player> player;
	std::vector<std::string> dungeonText;

	cocos2d::Sprite* player_sprite;

	std::vector<cocos2d::Sprite*> monster_sprites;
	std::vector<cocos2d::Sprite*> item_sprites;
	std::vector<cocos2d::Sprite*> trap_sprites;
	std::vector<cocos2d::Sprite*> projectile_sprites;

	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spike_sprites;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_sprites;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sprites;

	std::vector<cocos2d::Sprite*> wall_sprites;
	std::vector<cocos2d::Sprite*> door_sprites;

	cocos2d::Scene* m_scene; // scene needed to create sprites within dungeon

protected:
	// protected to allow assignment operators access
	int m_level;

private:
	std::vector<_Tile> m_maze;

	std::vector<std::shared_ptr<Monster>> m_monsters;
	std::vector<std::shared_ptr<Objects>> m_actives;

	//int m_level;

	int layer;
	int specialChunkLayer1;
	int specialChunkLayer2;

	int m_rows = MAXROWS;
	int m_cols = MAXCOLS;
};

class SecondFloor : public Dungeon {
public:
	SecondFloor(Player p);
	SecondFloor& operator=(SecondFloor const &dungeon);

	void peekSecondFloor(int x, int y, char move);
	//bool devilsWater();
	void devilsWater(bool bathed);
	void devilsGift();
	void guardiansDefeated();
	void devilsWaterPrompt();
	bool getWaterPrompt();

	void checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters);
	void explosion(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int x, int y, int pos);
	void findMonster(int x, int y);
	void fight(int x, int y);

	std::vector<char> outermostChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> outerChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> innerChunks1(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> innerChunksCT(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> innerChunksCB(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> centerChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);

	std::vector<std::vector<std::vector<char>>> mixOutermostChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols);
	std::vector<std::vector<std::vector<char>>> mixOuterChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols);
	std::vector<std::vector<std::vector<char>>> mixInnerChunks1(std::vector<std::vector<std::vector<char>>> c, int maxcols);
	std::vector<std::vector<std::vector<char>>> mixInnerChunksCT(std::vector<std::vector<std::vector<char>>> c, int maxcols);
	std::vector<std::vector<std::vector<char>>> mixInnerChunksCB(std::vector<std::vector<std::vector<char>>> c, int maxcols);
	std::vector<std::vector<std::vector<char>>> mixCenterChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols);

	void showDungeon();
	std::vector<_Tile> getDungeon() const;
	std::vector<std::shared_ptr<Monster>> getMonsters() const;
	std::vector<std::shared_ptr<Objects>> getActives() const;
	void callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index);

	virtual int getRows() const;
	virtual int getCols() const;
private:
	//Tile m_maze2[MAXROWS2 * MAXCOLS2];
	std::vector<_Tile> m_maze2;
	std::vector<std::shared_ptr<Monster>> m_f2monsters;
	std::vector<std::shared_ptr<Monster>> m_f2guardians;
	std::vector<std::shared_ptr<Objects>> m_f2actives;

	bool m_openexit;

	bool m_waterPrompt = false;
	bool m_watersUsed;
	bool m_watersCleared;
	int m_guardians;

	int m_stairsX;
	int m_stairsY;

	int layer;
	int specialChunkLayer1;
	int specialChunkLayer2;

	int m_rows = MAXROWS2;
	int m_cols = MAXCOLS2;
};

class ThirdFloor : public Dungeon {
public:
	ThirdFloor(Player p);
	ThirdFloor& operator=(ThirdFloor const &dungeon);

	void peekThirdFloor(int x, int y, char move);
	
	void checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters);
	bool roomCleared(std::vector<_Tile> &dungeon, std::vector<std::shared_ptr<Monster>> monsters, const int maxrows, const int maxcols);
	void openDoor(int dx, int dy);
	void toggleDoorLocks(int dx, int dy, char doortype);

	void fight(int x, int y);

	void hideRooms();
	void revealRoom();

	void showDungeon();

	std::vector<_Tile> getDungeon() const;
	std::vector<std::shared_ptr<Monster>> getMonsters() const;
	std::vector<std::shared_ptr<Objects>> getActives() const;
	void callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index);

	virtual int getRows() const;
	virtual int getCols() const;

	// LEVEL GENERATION
	void TLChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);
	void topChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);
	std::vector<char> TRChunks(std::vector<std::vector<std::vector<char>>> current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);

	void leftEdgeChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);
	void middleChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);
	std::vector<char> rightEdgeChunks(std::vector<std::vector<std::vector<char>>> current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);

	void BLChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);
	void bottomChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);
	std::vector<char> BRChunks(std::vector<std::vector<std::vector<char>>> current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);

	std::vector<std::vector<char>> playerChunks();
	std::vector<std::vector<char>> exitChunks();

	void mixChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);
	void pickSingleChunk(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols);

	std::vector<char> combineChunks(std::vector<std::vector<std::vector<char>>> c);
private:
	//Tile m_maze3[MAXROWS3 * MAXCOLS3];
	std::vector<_Tile> m_maze3;
	std::vector<std::shared_ptr<Monster>> m_f3monsters;
	std::vector<std::shared_ptr<Objects>> m_f3actives;

	bool m_locked;

	bool m_playerplaced;
	bool m_exitplaced;

	int m_layer;
	int m_playerchunk;
	int m_exitchunk;

	int m_rows = MAXROWS3;
	int m_cols = MAXCOLS3;
};

class FirstBoss : public Dungeon {
public:
	FirstBoss(Player p);
	FirstBoss& operator=(FirstBoss const &dungeon);

	void peekFirstBossDungeon(int x, int y, char move);
	void monsterDeath(int pos);
	using Dungeon::monsterDeath;

	void checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters);

	void checkSmasher();
	void pickMove();
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

	void resetLeftward();
	void resetRightward();
	void resetUpward();
	void resetDownward();

	bool monsterHash(int &x, int &y, bool &switched, char move);
	void pushMonster(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, char move, std::vector<std::shared_ptr<Monster>> &monsters, int cx = 0, int cy = 0);
	int findMonster(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, std::vector<std::shared_ptr<Monster>> monsters);
	void pushPlayer(std::vector<_Tile> &dungeon, int maxrows, int maxcols, char move);

	void checkArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	void moveArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);

	void fight(int x, int y);

	void showDungeon();

	std::vector<_Tile> getDungeon() const;
	int getRows() const;
	int getCols() const;
	std::vector<std::shared_ptr<Monster>> getMonsters() const;
	std::vector<std::shared_ptr<Objects>> getActives() const;
	void callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index);

	// used for smasher boss fight enemy spawns
	void finishActions();

	std::multimap<cocos2d::Sprite*, cocos2d::Action*> spawn;
private:
	std::vector<_Tile> m_boss;

	//std::vector<Smasher> m_smasher;
	std::vector<std::shared_ptr<Monster>> m_firstbossMonsters;
	std::vector<std::shared_ptr<Objects>> m_firstbossActives;

	// used for telling what direction smasher is moving in if executing rock slide
	char move = '0';

	int m_rows = BOSSROWS;
	int m_cols = BOSSCOLS;
};
#endif