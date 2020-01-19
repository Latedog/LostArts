#include "cocos2d.h"
#include "GameObjects.h"
#include "Actors.h"
#include <vector>

#ifndef DUNGEON_H
#define DUNGEON_H

// :::: Level boundaries ::::
const int MAXROWS = 18;
const int MAXCOLS = 70;

const int MAXROWS_SHOP1 = 15;
const int MAXCOLS_SHOP1 = 27;

const int MAXROWS2 = 38;
const int MAXCOLS2 = 78;

const int MAXROWS3 = 38 + 3;
const int MAXCOLS3 = 80 + 5;

const int BOSSROWS = 24;// 32;
const int BOSSCOLS = 26;

//		SPRITE EFFECTS
void runPlayerDamage(cocos2d::Sprite* sprite);
void runMonsterDamage(cocos2d::Sprite* sprite);
void runMonsterDamageBlink(cocos2d::Sprite* sprite);
void deathFade(cocos2d::Sprite* sprite);

struct _Tile {

	// :::: HIERARCHY OF TILE LAYER CONTENTS ::::
	//		-----------------------------
	//		Smasher		: TOP			 2
	//		Actors/Walls				 1
	//		Projectiles					 0
	//		Items						-1
	//		Extras						 x
	//	    Gold/Money					-3
	//		Stairs/Traps				-4
	//		Floor		: BOTTOM		-5
	//


	char upper;			//	  ^    layer for smasher to sit in
	char top;			//	/ | \  walls, player, monsters, etc.; Also used for doors
	char projectile;	//	  |	   objects that fly through the air
	char bottom;		//	  |	   items, weapons, etc.
	int gold;			//	  |	   exclusively for gold and money
	char traptile;		//	  |	   stairs, traps, lava, etc.; Also used to save door type.
	char extra;			//	  |	   for things like active bombs, or other odds and ends


	// tells if there is already something present in a particular spot
	bool wall;
	bool item;
	bool trap;
	bool enemy;
	bool hero; // the player

	// shop information
	int price;

	// monster identifier
	std::string monster_name;

	// item information
	std::string item_name;
	std::shared_ptr<Objects> object;

	// trap identifier
	std::string trap_name;

	// shop item identifier
	std::string shop_action;

	// tells if there's overlap of multiple objects(spikes atm), and counts how many instances
	bool overlap;
	int overlap_count;

	// Normally, monsters cannot overlap, but this frequently can happen during the smasher boss fight.
	// So this is used for telling if smasher was on top of an enemy and prevents enemy overlap bug.
	// :::: currently unused, replaced with pushing instead of overlapping ::::
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

	virtual void peekDungeon(int x, int y, char move);

	virtual void checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters);
	virtual void explosion(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int x, int y, int pos);
	virtual void explosion(Dungeon &dungeon, int x, int y);

	// Traps
	void trapRemoval(Dungeon &dungeon); // for removing any destroyed traps after interating through actives
	int findTrap(int x, int y, std::vector<std::shared_ptr<Objects>> &actives);
	void trapEncounter(Dungeon &dungeon, int x, int y);
	void moveFirebar(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, int pos);
	void moveDoubleFirebar(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, int pos);
	void openDoor(Dungeon &dungeon, int x, int y);
	
	void monsterTrapEncounter(Dungeon &dungeon);
	void singleMonsterTrapEncounter(Dungeon &dungeon, int pos);
	void destroyMonster(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Monster>> &monsters, int pos);
	void monsterDeath(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int pos);
	
	virtual void enemyOverlap(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y);
	virtual void itemHash(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int &x, int &y);

	bool monsterHash(Dungeon &dungeon, int &x, int &y, bool &switched, char move);
	void pushMonster(Dungeon &dungeon, int &mx, int &my, char move, int cx = 0, int cy = 0);

	// Enemy movement
	bool attemptChase(std::vector<_Tile> &dungeon, int maxcols, int pos, int &shortest, int smelldist, int origdist, int x, int y, char &first_move, char &optimal_move, char prev);
	bool playerInRange(int mx, int my, int pos, int range);
	void moveWithSuccessfulChase(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, char move, std::vector<std::shared_ptr<Monster>> &monsters);
	void moveSeekers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, char move, std::vector<std::shared_ptr<Monster>> &monsters);

	void checkArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	void checkMountedKnights(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	void moveMountedKnights(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	void moveWanderers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);

	void moveRoundabouts(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);

	void moveZappers(std::vector<_Tile> &dungeon, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	void moveSpinners(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters);
	/// end enemy movement

	// Enemy utility functions
	virtual void moveMonsterRandomly(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Monster>> &monsters, int pos);
	bool wallCollision(std::vector<_Tile> &dungeon, int maxcols, char direction, int p_move, int m_move);

	virtual int findMonster(int mx, int my, std::vector<std::shared_ptr<Monster>> monsters);
	virtual void fight(Dungeon &dungeon, int x, int y);

	void foundItem(std::vector<_Tile> &dungeon, int maxcols, int x, int y);
	virtual void collectItem(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y);
	void goldPickup(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y);

	// .......deprecated.......
	//virtual void showDungeon();
	virtual void showText();

	std::vector<Player>& getPlayerVector();
	Player getPlayer() const;
	void setPlayer(Player p);
	int getLevel() const;
	void setLevel(int level);

	void unmarkTiles();

	virtual std::vector<_Tile>& getDungeon();
	virtual int getRows() const;
	virtual int getCols() const;
	virtual std::vector<std::shared_ptr<Monster>>& getMonsters();
	virtual std::vector<std::shared_ptr<Objects>>& getActives();
	virtual std::vector<std::shared_ptr<Objects>>& getTraps();
	void callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index);



	// :::: SPRITE SETTING ::::

	void setPlayerSprite(cocos2d::Sprite* sprite);
	void setMoneySprites(std::vector<cocos2d::Sprite*> sprites);
	void setMonsterSprites(std::vector<cocos2d::Sprite*> sprites);
	void setItemSprites(std::vector<cocos2d::Sprite*> sprites);
	void setTrapSprites(std::vector<cocos2d::Sprite*> sprites);
	void setProjectileSprites(std::vector<cocos2d::Sprite*> sprites);

	void setSpikeProjectileSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites);
	void setSpinnerSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites);
	void setZapperSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites);

	void setWallSprites(std::vector<cocos2d::Sprite*> sprites);
	void setDoorSprites(std::vector<cocos2d::Sprite*> sprites);
	void setFloorSprites(std::vector<cocos2d::Sprite*> sprites);
	void setScene(cocos2d::Scene* scene);


	// :::: SPRITE MANIPULATION ::::

	cocos2d::Sprite* findSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y);
	void setSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, char move);
	void teleportSprite(cocos2d::Sprite* sprite, int x, int y);

	// Used for sequencing actions, instead of running them immediately
	void queueMoveSprite(std::shared_ptr<Monster> monster, char move);
	void queueMoveSprite(std::shared_ptr<Monster> monster, int maxrows, int cx, int cy);
	void queueMoveSprite(cocos2d::Sprite* sprite, int maxrows, int cx, int cy, float time = .1, bool front = false);
	void queueMoveSprite(cocos2d::Sprite* sprite, char move, float time = .1);

	// Moves unordered sprites using @move as the directional parameter
	void moveSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, char move);

	// Moves sprites to a certain spot using @cx and @cy to specify the position
	void moveSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int cx, int cy);

	/* Moves sprites that may be overlapping using @move as the directional parameter
	*  Currently only used for ceiling spikes that spawn on the Smasher boss fight
	*/
	void moveSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y, char move);

	/* Moves sprites that are associated with another sprite
	*  X and Y are the "parent" sprite's cooridnates
	*  x and y are the coordinates of the sprite we want to move
	*  Vec2 in the multimap contains the coordinates of all such parent sprites
	*/
	void moveAssociatedSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int X, int Y, int x, int y, char move);

	// No difference from addSprite, just returns the sprite
	cocos2d::Sprite* createSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image);
	void addSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image);
	void addSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image);
	void addGoldSprite(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y);

	void queueRemoveSprite(int x, int y, std::vector<std::shared_ptr<Monster>> monsters);
	void queueRemoveSprite(int x, int y, cocos2d::Sprite* sprite);
	void removeSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y);
	void removeSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y);
	void removeAssociatedSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int X, int Y, int x, int y);

	void updateLighting(Dungeon &dungeon, std::vector<std::shared_ptr<Monster>> &monsters, std::vector<std::shared_ptr<Objects>> &traps);

	int spriteFound(cocos2d::Sprite* sprite);
	void insertActionIntoSpawn(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action, bool front = false);
	void runSpawn();


	// :::: LEVEL GENERATION ::::

	std::vector<char> topChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> middleChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> bottomChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);

	std::vector<std::vector<std::vector<char>>> mixChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols);
	virtual std::vector<char> combineChunks(std::vector<std::vector<std::vector<char>>> c);


	// :::: Member variables ::::
	std::vector<Player> player;
	std::vector<std::string> dungeonText;

	cocos2d::Sprite* player_sprite = nullptr;

	std::vector<cocos2d::Sprite*> money_sprites;
	std::vector<cocos2d::Sprite*> monster_sprites;
	std::vector<cocos2d::Sprite*> item_sprites;
	std::vector<cocos2d::Sprite*> trap_sprites;
	std::vector<cocos2d::Sprite*> projectile_sprites;

	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spike_sprites;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_sprites;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sprites;

	std::vector<cocos2d::Sprite*> wall_sprites;
	std::vector<cocos2d::Sprite*> door_sprites;
	std::vector<cocos2d::Sprite*> floor_sprites;

	cocos2d::Scene* m_scene; // scene needed to create sprites within dungeon

	std::vector<std::pair<int, int>> m_trapIndexes;

protected:
	// protected to allow assignment operators access
	int m_level;

	std::unordered_multimap<cocos2d::Sprite*, cocos2d::Vector<cocos2d::FiniteTimeAction*>> m_spawn;
	//std::vector<std::pair<cocos2d::Sprite*, cocos2d::FiniteTimeAction*>> m_seq;
	std::vector<std::pair<cocos2d::Sprite*, cocos2d::Vector<cocos2d::FiniteTimeAction*>>> m_seq;
	//std::list<std::pair<cocos2d::Sprite*, cocos2d::Vector<cocos2d::FiniteTimeAction*>>> m_list;


private:
	std::vector<_Tile> m_maze;

	std::vector<std::shared_ptr<Monster>> m_monsters;
	std::vector<std::shared_ptr<Objects>> m_actives;
	std::vector<std::shared_ptr<Objects>> m_traps;

	int layer;
	int specialChunkLayer1;
	int specialChunkLayer2;

	int m_rows = MAXROWS;
	int m_cols = MAXCOLS;
};

class FirstShop : public Dungeon {
public:
	FirstShop(Player p);
	FirstShop& operator=(FirstShop const &dungeon);

	void peekDungeon(int x, int y, char move);

	void purchaseItem(std::vector<_Tile> &dungeon, int x, int y);
	void collectItem(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y);

	// shop layout
	std::vector<char> generate();

	// helper functions
	virtual std::vector<_Tile>& getDungeon();
	virtual int getRows() const;
	virtual int getCols() const;
	virtual std::vector<std::shared_ptr<Monster>>& getMonsters();
	virtual std::vector<std::shared_ptr<Objects>>& getActives();
	std::vector<std::shared_ptr<Objects>>& getTraps();

private:
	std::vector<_Tile> m_shop1;

	std::vector<std::shared_ptr<Monster>> m_shopMonsters;
	std::vector<std::shared_ptr<Objects>> m_shopActives;
	std::vector<std::shared_ptr<Objects>> m_traps;

	int m_rows = MAXROWS_SHOP1;
	int m_cols = MAXCOLS_SHOP1;
};

class SecondFloor : public Dungeon {
public:
	SecondFloor(Player p);
	SecondFloor& operator=(SecondFloor const &dungeon);

	void peekDungeon(int x, int y, char move);

	void devilsWater(bool bathed);
	void devilsGift();
	void guardiansDefeated();
	void devilsWaterPrompt();
	bool getWaterPrompt();

	//void explosion(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int x, int y, int pos);
	void explosion(Dungeon &dungeon, int x, int y);

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

	//void showDungeon();

	std::vector<_Tile>& getDungeon();
	int getRows() const;
	int getCols() const;
	std::vector<std::shared_ptr<Monster>>& getMonsters();
	std::vector<std::shared_ptr<Objects>>& getActives();
	std::vector<std::shared_ptr<Objects>>& getTraps();

private:
	std::vector<_Tile> m_maze2;
	std::vector<std::shared_ptr<Monster>> m_f2monsters;
	std::vector<std::shared_ptr<Monster>> m_f2guardians;
	std::vector<std::shared_ptr<Objects>> m_f2actives;
	std::vector<std::shared_ptr<Objects>> m_traps;

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

	void peekDungeon(int x, int y, char move);
	
	void checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters);
	bool roomCleared(std::vector<_Tile> &dungeon, std::vector<std::shared_ptr<Monster>> monsters, const int maxrows, const int maxcols);
	void toggleDoorLocks(int dx, int dy, char doortype);

	void hideRooms();
	void revealRoom();

	//void showDungeon();

	std::vector<_Tile>& getDungeon();
	int getRows() const;
	int getCols() const;
	std::vector<std::shared_ptr<Monster>>& getMonsters();
	std::vector<std::shared_ptr<Objects>>& getActives();
	std::vector<std::shared_ptr<Objects>>& getTraps();
	

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
	std::vector<_Tile> m_maze3;
	std::vector<std::shared_ptr<Monster>> m_f3monsters;
	std::vector<std::shared_ptr<Objects>> m_f3actives;
	std::vector<std::shared_ptr<Objects>> m_traps;

	bool m_locked;

	// used for checking level generation of player and exit chunks
	bool m_playerplaced;
	bool m_exitplaced;

	// the row that the level generator is currently on
	int m_layer;

	// value from 1-4, tells the location of where the chunks are to be placed
	int m_playerchunk;
	int m_exitchunk;

	int m_rows = MAXROWS3;
	int m_cols = MAXCOLS3;
};

class FirstBoss : public Dungeon {
public:
	FirstBoss(Player p);
	FirstBoss& operator=(FirstBoss const &dungeon);

	void peekDungeon(int x, int y, char move);

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
	void pushPlayer(std::vector<_Tile> &dungeon, int maxrows, int maxcols, char move);

	void fight(int x, int y);

	//void showDungeon();

	std::vector<_Tile>& getDungeon();
	int getRows() const;
	int getCols() const;
	std::vector<std::shared_ptr<Monster>>& getMonsters();
	std::vector<std::shared_ptr<Objects>>& getActives();
	std::vector<std::shared_ptr<Objects>>& getTraps();
	

	// ......deprecated.......
	void finishActions();

private:
	std::vector<_Tile> m_boss;

	std::vector<std::shared_ptr<Monster>> m_firstbossMonsters;
	std::vector<std::shared_ptr<Objects>> m_firstbossActives;
	std::vector<std::shared_ptr<Objects>> m_traps;

	// used for telling what direction smasher is moving in if executing rock slide
	char move = '0';

	int m_rows = BOSSROWS;
	int m_cols = BOSSCOLS;
};


void playMonsterDeathByPit(Player p, Monster m);
void playArcherDaggerSwipe();

#endif