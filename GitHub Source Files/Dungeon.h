#include "cocos2d.h"
#include "GameObjects.h"
#include "Actors.h"
#include <vector>

#ifndef DUNGEON_H
#define DUNGEON_H

// :::: Level boundaries ::::
const int MAXROWS = 38;// 18;
const int MAXCOLS = 38;// 70;

const int MAXROWS_SHOP1 = 15;
const int MAXCOLS_SHOP1 = 27;

const int MAXROWS2 = 56;// 44;// 38;
const int MAXCOLS2 = 74;// 78;

const int MAXROWS3 = 38 + 3;
const int MAXCOLS3 = 80 + 5;

const int BOSSROWS = 22;// 32;
const int BOSSCOLS = 24;

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
	bool npc;
	bool exit;

	bool noSpawn; // nothing allowed to spawn here during level creation if true

	// shop information
	int price;

	// saves floor tile sprite for easy access
	cocos2d::Sprite* floor = nullptr;

	// monster identifier
	std::string monster_name;

	// item information
	std::string item_name;
	std::shared_ptr<Objects> object;

	// trap identifier
	std::string trap_name;

	// 
	std::string actives;

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

	void checkActive();

	int findDoor(int x, int y);
	void openDoor(int x, int y);
	virtual void checkDoors();

	int findNPC(int x, int y);
	void interactWithNPC(int x, int y);

	// :::: Trap Handling ::::
	void trapRemoval(); // for removing any destroyed traps after iterating through actives
	int findTrap(int x, int y, bool endFirst = false);
	void trapEncounter(int x, int y);
	
	void monsterTrapEncounter();
	void singleMonsterTrapEncounter(int pos);

	virtual int findMonster(int mx, int my);
	virtual void fight(int x, int y);
	void destroyMonster(int pos);
	virtual void monsterDeath(int pos);
	bool monsterHash(Dungeon &dungeon, int &x, int &y, bool &switched, char move);
	void pushMonster(Dungeon &dungeon, int &mx, int &my, char move, int cx = 0, int cy = 0);
	void pushPlayer(char move);

	// :::: Item Collection ::::
	virtual void itemHash(std::vector<_Tile> &dungeon, int &x, int &y);
	void foundItem(std::vector<_Tile> &dungeon, int x, int y);
	virtual void collectItem(Dungeon &dungeon, int x, int y);
	void goldPickup(std::vector<_Tile> &dungeon, int x, int y);


	//	:::: Getters ::::
	std::vector<Player>& getPlayerVector();
	Player getPlayer() const;
	void setPlayer(Player p);
	int getLevel() const;
	void setLevel(int level);

	virtual std::vector<_Tile>& getDungeon();
	virtual int getRows() const;
	virtual int getCols() const;
	std::vector<std::shared_ptr<NPC>>& getNPCs();
	std::vector<std::shared_ptr<Monster>>& getMonsters();
	std::vector<std::shared_ptr<Traps>>& getTraps();
	std::vector<std::shared_ptr<Door>>& getDoors();
	void callUse(std::vector<_Tile> &dungeon, int x, int y, int index);


	// .......deprecated.......
	bool wallCollision(std::vector<_Tile> &dungeon, int maxcols, char direction, int p_move, int m_move);
	virtual void enemyOverlap(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y);
	virtual void showText();
	void unmarkTiles(); // unused


	// :::: SPRITE SETTING ::::

	void setPlayerSprite(cocos2d::Sprite* sprite);
	void setMoneySprites(std::vector<cocos2d::Sprite*> sprites);
	void setItemSprites(std::vector<cocos2d::Sprite*> sprites);

	void setSpikeProjectileSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites);

	void setWallSprites(std::vector<cocos2d::Sprite*> sprites);
	void setDoorSprites(std::vector<cocos2d::Sprite*> sprites);
	void setScene(cocos2d::Scene* scene);


	// :::: SPRITE MANIPULATION ::::

	cocos2d::Sprite* findSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y);
	void setSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y, char move);
	void teleportSprite(cocos2d::Sprite* sprite, int x, int y);

	// Used for sequencing actions, instead of running them immediately
	void queueMoveSprite(cocos2d::Sprite* sprite, char move, float time = .1f);
	void queueMoveSprite(cocos2d::Sprite* sprite, int cx, int cy, float time = .1f, bool front = false);

	// Moves a particular sprite in a certain direction with @move instantly
	void moveSprite(cocos2d::Sprite* sprite, char move, float time = .1);

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
	cocos2d::Sprite* createSprite(int x, int y, int z, std::string image);
	cocos2d::Sprite* createSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y, int z, std::string image);
	void addSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image);
	void addSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image);
	void addGoldSprite(std::vector<_Tile> &dungeon, int x, int y);

	void queueRemoveSprite(cocos2d::Sprite* sprite);
	void removeSprite(cocos2d::Sprite* sprite);
	void removeSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y);
	void removeSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y);
	void removeAssociatedSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int X, int Y, int x, int y);

	void updateLighting();
	void updateSecondaryLighting(int x, int y);

	int spriteFound(cocos2d::Sprite* sprite);
	void insertActionIntoSpawn(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action, bool front = false);
	void runSpawn();


	// :::: LEVEL GENERATION ::::

	/*std::vector<char> topChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> middleChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);
	std::vector<char> bottomChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols);

	std::vector<std::vector<std::vector<char>>> mixChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols);*/
	virtual std::vector<char> combineChunks(std::vector<std::vector<std::vector<char>>> c);


	std::vector<std::string> topChunks(std::vector<std::vector<std::vector<std::string>>> &c);
	std::vector<std::string> middleChunks(std::vector<std::vector<std::vector<std::string>>> &c);
	std::vector<std::string> bottomChunks(std::vector<std::vector<std::vector<std::string>>> &c);

	std::vector<std::vector<std::vector<std::string>>> mixChunks(std::vector<std::vector<std::vector<std::string>>> c);
	virtual std::vector<std::string> combineChunks(std::vector<std::vector<std::vector<std::string>>> c);

	virtual std::vector<std::string> generateLevel();
	virtual void fillLevel(std::vector<std::string> finalvec, Player p, int start = 1, int end = -1);


	// :::: Member variables ::::
	std::vector<Player> player;
	std::vector<std::string> dungeonText;

	cocos2d::Sprite* player_sprite = nullptr;

	std::vector<cocos2d::Sprite*> money_sprites;
	std::vector<cocos2d::Sprite*> monster_sprites; // unused
	std::vector<cocos2d::Sprite*> item_sprites;
	std::vector<cocos2d::Sprite*> trap_sprites; // unused
	std::vector<cocos2d::Sprite*> projectile_sprites; // unused

	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spike_sprites;
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> spinner_sprites; // unused
	std::multimap<cocos2d::Vec2, cocos2d::Sprite*> zapper_sprites; // unused

	std::vector<cocos2d::Sprite*> wall_sprites;
	std::vector<cocos2d::Sprite*> door_sprites;
	std::vector<cocos2d::Sprite*> floor_sprites; // unused

	std::vector<std::pair<int, int>> lightEmitters;

	cocos2d::Scene* m_scene; // scene needed to create sprites within dungeon

	std::vector<std::pair<int, int>> m_trapIndexes; // holds the x, y coordinates of traps to be removed at the end of checkActive()

protected:
	// protected to allow assignment operators access
	int m_level;

	std::vector<std::shared_ptr<NPC>> m_npcs;
	std::vector<std::shared_ptr<Monster>> m_monsters;
	std::vector<std::shared_ptr<Traps>> m_traps;
	std::vector<std::shared_ptr<Door>> m_doors;

	std::unordered_multimap<cocos2d::Sprite*, cocos2d::Vector<cocos2d::FiniteTimeAction*>> m_spawn;
	//std::vector<std::pair<cocos2d::Sprite*, cocos2d::FiniteTimeAction*>> m_seq;
	std::vector<std::pair<cocos2d::Sprite*, cocos2d::Vector<cocos2d::FiniteTimeAction*>>> m_seq;

private:
	std::vector<_Tile> m_maze;

	int playerChunk;
	int exitChunk;
	int specialChunk1;
	int specialChunk2;

	int layer;
	int specialChunkLayer1;
	int specialChunkLayer2;

	int m_rows = MAXROWS;
	int m_cols = MAXCOLS;
};

class Shop : public Dungeon {
public:
	Shop(Player p, int level);
	Shop& operator=(Shop const &dungeon);

	void peekDungeon(int x, int y, char move);

	void purchaseItem(std::vector<_Tile> &dungeon, int x, int y);
	void collectItem(Dungeon &dungeon, int x, int y);

	// shop layout
	std::vector<char> generate();

	// helper functions
	std::vector<_Tile>& getDungeon();
	int getRows() const;
	int getCols() const;

private:
	std::vector<_Tile> m_shop1;

	float m_priceMultiplier;

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

	void monsterDeath(int pos);
	void fight(int x, int y);

	std::vector<std::vector<std::string>> outermostChunks();	// top and bottom rows
	std::vector<std::vector<std::string>> edgeChunks();			// left and right columns
	std::vector<std::vector<std::string>> lavaChunks();			// lava chunks
	std::vector<std::vector<std::string>> fixedChunks(std::string chunk);	// special chunks that are fixed
	//std::vector<std::string> innerChunksCT(std::vector<std::vector<std::vector<std::string>>> &c);
	//std::vector<std::string> innerChunksCB(std::vector<std::vector<std::vector<std::string>>> &c);
	//std::vector<std::string> centerChunks(std::vector<std::vector<std::vector<std::string>>> &c);

	std::vector<std::string> generateLevel();
	std::vector<std::string> combineChunks(std::vector<std::vector<std::vector<std::string>>> c);


	std::vector<_Tile>& getDungeon();
	int getRows() const;
	int getCols() const;

private:
	std::vector<_Tile> m_maze2;

	std::vector<std::shared_ptr<Monster>> m_f2guardians;

	bool m_openexit;

	bool m_waterPrompt = false;
	bool m_watersUsed;
	bool m_watersCleared;
	int m_guardians;

	int m_stairsX;
	int m_stairsY;

	int layer = 1;
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
	
	void checkDoors();
	bool roomCleared();
	void toggleDoorLocks(int dx, int dy, char doortype);

	void hideRooms();
	void revealRoom();

	std::vector<_Tile>& getDungeon();
	int getRows() const;
	int getCols() const;
	

	// LEVEL GENERATION
	void TLChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool);
	void topChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool);
	std::vector<std::string> TRChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool);

	void leftEdgeChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool);
	void middleChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool);
	std::vector<std::string> rightEdgeChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool);

	void BLChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool);
	void bottomChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool);
	std::vector<std::string> BRChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool);

	std::vector<std::vector<std::string>> playerChunks();
	std::vector<std::vector<std::string>> exitChunks();

	void mixChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool);
	void pickSingleChunk(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool);

	std::vector<std::string> generateLevel();
	std::vector<std::string> combineChunks(std::vector<std::vector<std::vector<std::string>>> c);

private:
	std::vector<_Tile> m_maze3;

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
	//using Dungeon::monsterDeath;

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

	std::vector<_Tile>& getDungeon();
	int getRows() const;
	int getCols() const;
	

	// ......deprecated.......
	void finishActions();

private:
	std::vector<_Tile> m_boss;

	// used for telling what direction smasher is moving in if executing rock slide
	char move = '0';

	int m_rows = BOSSROWS;
	int m_cols = BOSSCOLS;
};


void playMonsterDeathByPit(Player p, Monster m);
void playArcherDaggerSwipe();

#endif