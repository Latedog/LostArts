#ifndef DUNGEON_H
#define DUNGEON_H

#include "cocos2d.h"
#include "GameObjects.h"
#include "Actors.h"
#include <vector>

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
	bool boundary = false;
	bool wall;
	bool item;
	bool trap;
	bool enemy;
	bool hero; // the player
	bool npc;
	bool exit;

	bool noSpawn; // nothing allowed to spawn here during level creation if true
	bool menu = false; // Used only in World Hub to return to the menu

	// shop information
	int price;

	// saves floor tile sprite for easy access
	cocos2d::Sprite* floor = nullptr;

	// wall identifier
	std::string wall_type;

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
	Dungeon(int level, int rows, int cols);
	virtual Dungeon& operator=(Dungeon const &dungeon);
	_Tile& operator[](int index);
	virtual ~Dungeon();

	virtual void peekDungeon(int x, int y, char move); // Core gameplay loop
	virtual void specialActions() { return; }; // unique checks made in peekDungeon
	virtual bool specialTrapCheck(int x, int y) { return false; }; // unique trap checks made when player moves onto a trap

	// :::: User functions ::::
	void changeWeapon(int index);
	void assignQuickItem(int index);
	void callUse(int index);

	// :::: NPC functions ::::
	int findNPC(int x, int y) const;
	void interactWithNPC(int x, int y);
	void playNPCDialogue(NPC &npc, std::vector<std::string> dialogue);

	// :::: Doors ::::
	void openDoor(int x, int y);

	// :::: Trap Handling ::::
	int findTrap(int x, int y, bool endFirst = false) const;
	int countTrapNumber(int x, int y); // returns the number of traps with this (x, y) pair
	void trapEncounter(int x, int y, bool endFirst = false); // endFirst is used to find traps that we know were spawned after the initial construction (such as RockSummon)
	void damagePlayer(int damage);
	void singleMonsterTrapEncounter(int pos);

	// :::: Decoys ::::
	void addDecoy(std::shared_ptr<Decoy> decoy) { m_decoys.push_back(decoy); };

	// :::: Monster Handling ::::
	int findMonster(int mx, int my) const;
	virtual void fight(int x, int y);
	void damageMonster(int index, int damage);
	void giveAffliction(int index, std::shared_ptr<Afflictions> affliction);
	virtual void monsterDeath(int pos);

	void pushMonster(int &mx, int &my, char move, int cx = 0, int cy = 0, bool strict = false);
	virtual void pushPlayer(char move, bool strict = false);

	// For pushing players/monsters in straight lines properly (i.e. no weird skipping through enemies)
	// @x and @y is the target to be pushed. @lethal indicates that the actor should die if pushed against a wall
	void linearActorPush(int x, int y, int limit, char move, bool pulling = false, bool lethal = false);

	// @sx and @sy are the source or start point. @tx and @ty is the target to be moved.
	// Can be used in conjunction with linearActorPush.
	char getDirectionRelativeTo(int sx, int sy, int tx, int ty, bool pulling = false);

	// :::: Item Collection ::::
	void itemHash(int &x, int &y);
	virtual void purchaseItem(int x, int y) { return; };
	virtual void collectItem(int x, int y);
	void goldPickup(int x, int y);


	//	:::: Getters ::::
	std::vector<std::shared_ptr<Player>>& getPlayerVector() { return player; };
	std::shared_ptr<Player> getPlayer() const { return player.at(0); };
	int getLevel() const { return m_level; };
	bool returnedToMenu() const { return m_return; };

	std::vector<_Tile>& getDungeon() { return m_maze; };
	int getRows() const { return m_rows; };
	int getCols() const { return m_cols; };
	std::vector<std::shared_ptr<NPC>>& getNPCs() { return m_npcs; };
	std::vector<std::shared_ptr<Monster>>& getMonsters() { return m_monsters; };
	std::vector<std::shared_ptr<Traps>>& getTraps() { return m_traps; };
	std::vector<std::shared_ptr<Door>>& getDoors() { return m_doors; };

	// Helper
	bool withinBounds(int x, int y);

	// For shops prices
	virtual bool isShop() const { return false; };
	virtual void constructShopHUD(int x, int y);
	virtual void deconstructShopHUD();


	// .......deprecated.......
	bool wallCollision(char direction, int p_move, int m_move);
	void showText();


	// :::: SPRITE SETTING ::::
	void setMoneySprites(std::vector<cocos2d::Sprite*> sprites) { money_sprites = sprites; };
	void setItemSprites(std::vector<cocos2d::Sprite*> sprites) { item_sprites = sprites; };
	void setWallSprites(std::vector<cocos2d::Sprite*> sprites) { wall_sprites = sprites; };
	void setDoorSprites(std::vector<cocos2d::Sprite*> sprites) { door_sprites = sprites; };

	void setScene(cocos2d::Scene* scene) { m_scene = scene; };


	// :::: SPRITE MANIPULATION ::::
	int findSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y);
	void setSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y, char move);
	void teleportSprite(cocos2d::Sprite* sprite, int x, int y);

	// Used for sequencing actions, instead of running them immediately
	void queueMoveSprite(cocos2d::Sprite* sprite, char move, float time = .1f);
	void queueMoveSprite(cocos2d::Sprite* sprite, int cx, int cy, float time = .1f, bool front = false);
	void queueMoveSprite(cocos2d::Sprite* sprite, float cx, float cy, float time = .1f, bool front = false);

	// Moves a particular sprite in a certain direction with @move instantly
	void moveSprite(cocos2d::Sprite* sprite, char move, float time = .1f);

	// Moves vector sprites using @move as the directional parameter
	void moveSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y, char move);

	// Moves sprites to a certain spot using @cx and @cy to specify the position
	void moveSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y, int cx, int cy);

	// No difference from addSprite, just returns the sprite
	cocos2d::Sprite* createSprite(int x, int y, int z, std::string image);
	cocos2d::Sprite* createSprite(float x, float y, int z, std::string image);
	cocos2d::Sprite* createSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y, int z, std::string image);
	void addSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y, int z, std::string image);
	void addGoldSprite(int x, int y);

	void queueRemoveSprite(cocos2d::Sprite* sprite);
	void removeSprite(cocos2d::Sprite* sprite);
	void removeSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y);

	void updateLighting();
	void updateSecondaryLighting(int x, int y);
	void updateSecondaryLighting();

	// @strength is how much light is to be provided
	void addLightSource(int x, int y, double strength);
	void removeLightSource(int x, int y);

	cocos2d::Vector<cocos2d::SpriteFrame*> getAnimation(const char* format, int count);
	cocos2d::Vector<cocos2d::SpriteFrame*> getAnimation(std::string format, int count);
	cocos2d::Sprite* runAnimationForever(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z);
	void runSingleAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z);
	void runAnimationForever(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int x, int y);

	// This runs the animation and then leaves the final frame remaining.
	// Allows things such as debris to be left and have their sprites updated with proper lighting.
	void runAnimationWithCallback(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z,
		std::function<void(Dungeon&, std::vector<cocos2d::Sprite*>&, int, int, int, std::string)> cb, std::string image);

	// :::: Member variables ::::
	std::vector<std::string> dungeonText;

	std::vector<cocos2d::Sprite*> money_sprites;
	std::vector<cocos2d::Sprite*> item_sprites;
	std::vector<cocos2d::Sprite*> wall_sprites;
	std::vector<cocos2d::Sprite*> door_sprites;

	std::vector<cocos2d::Sprite*> misc_sprites; // For decorations, debris, etc.

protected:
	// Sets the defaults for all _Tiles
	void initializeTiles();

	void checkActive();
	virtual void checkMonsters();

	bool checkDecoys(int mx, int my, int &x, int &y);
	int findDecoy(int x, int y);

	int findSegmentedMonster(int index) const;

	int findDoor(int x, int y) const;
	virtual void checkDoors();

	void insertActorForRemoval(int index);
	void actorRemoval();

	void insertTrapForRemoval(int index);
	void trapRemoval(); // for removing any destroyed traps after iterating through actives
	void monsterTrapEncounter();

	// finds a valid spot for a monster after being pushed
	bool monsterHash(int &x, int &y, bool &switched, char move, bool strict = false);

	void setPlayer(std::shared_ptr<Player> p) { player.at(0) = p; };
	void setLevel(int level) { m_level = level; };

	int spriteFound(cocos2d::Sprite* sprite);
	void insertActionIntoSpawn(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action, bool front = false);
	void runSpawn();

	int findLightSource(int x, int y);

	// :::: LEVEL GENERATION ::::
	void fillLevel(std::vector<std::string> finalvec, std::shared_ptr<Player> p, int start = 1, int end = -1);

	std::vector<std::shared_ptr<Player>> player;

	// protected to allow assignment operators access
	int m_level;

	std::vector<std::shared_ptr<NPC>> m_npcs;
	std::vector<std::shared_ptr<Monster>> m_monsters;
	std::vector<std::shared_ptr<Traps>> m_traps;
	std::vector<std::shared_ptr<Door>> m_doors;
	std::vector<std::shared_ptr<Decoy>> m_decoys; // Decoys attract monsters that normally chase the player

	std::set<int> m_trapIndexes;
	std::set<int> m_deadActorIndexes;
	std::vector<std::pair<int, int>> lightEmitters;

	// Inner pair: x & y coordinates; Outer pair: The inner pair and the light cutoff value
	// Holds coordinates and light strength values that should remain constant.
	std::vector<std::pair<std::pair<int, int>, double>> m_lightSources;

	// Vector containing the sprite movement actions
	std::vector<std::pair<cocos2d::Sprite*, cocos2d::Vector<cocos2d::FiniteTimeAction*>>> m_seq;

	cocos2d::Scene* m_scene; // scene needed to create sprites within dungeon

	bool m_return = false; // Flag indicating that the player return to main menu from world hub

	std::vector<_Tile> m_maze;

private:
	int m_rows;
	int m_cols;
};

class TutorialFloor : public Dungeon {
public:
	TutorialFloor(std::shared_ptr<Player> p);

private:
	std::vector<char> generate();
};

class WorldHub : public Dungeon {
public:
	WorldHub(std::shared_ptr<Player> p);

	void specialActions();

private:
	std::vector<char> generate();
};

class Shop : public Dungeon {
public:
	Shop(std::shared_ptr<Player> p, int level);
	Shop& operator=(Shop const &dungeon);

	virtual void specialActions();

	bool isShop() const { return true; };
	void constructShopHUD(int x, int y);
	void deconstructShopHUD();

	void purchaseItem(int x, int y);
	void collectItem(int x, int y);

private:
	// shop layout
	std::vector<char> generate();

	float m_priceMultiplier;
};

class Shrine : public Dungeon {
public:
	// 
	Shrine(std::shared_ptr<Player> p);

};

class FirstFloor : public Dungeon {
public:
	FirstFloor(std::shared_ptr<Player> p);

private:
	// :::: LEVEL GENERATION ::::
	std::vector<std::string> topChunks(std::vector<std::vector<std::vector<std::string>>> &c);
	std::vector<std::string> middleChunks(std::vector<std::vector<std::vector<std::string>>> &c);
	std::vector<std::string> bottomChunks(std::vector<std::vector<std::vector<std::string>>> &c);

	std::vector<std::vector<std::vector<std::string>>> mixChunks(std::vector<std::vector<std::vector<std::string>>> c);
	std::vector<std::string> combineChunks(std::vector<std::vector<std::vector<std::string>>> c);

	std::vector<std::string> generateLevel();

	int playerChunk;
	int exitChunk;
	int specialChunk1;
	int specialChunk2;

	int layer;
	int specialChunkLayer1;
	int specialChunkLayer2;
};

class SecondFloor : public Dungeon {
public:
	SecondFloor(std::shared_ptr<Player> p);
	SecondFloor& operator=(SecondFloor const &dungeon);

	void specialActions();
	bool specialTrapCheck(int x, int y);

	void devilsWater(bool bathed);
	void devilsGift();
	void guardiansDefeated();
	void devilsWaterPrompt();
	bool getWaterPrompt() { return m_waterPrompt; };

	void monsterDeath(int pos);

protected:
	std::vector<std::vector<std::string>> outermostChunks();	// top and bottom rows
	std::vector<std::vector<std::string>> edgeChunks();			// left and right columns
	std::vector<std::vector<std::string>> cornerChunks(int corner, bool innerRing = false);			// corner pieces
	std::vector<std::vector<std::string>> junctionChunks();		// chunks before and after crossing the bridge between rings
	std::vector<std::vector<std::string>> lavaChunks();			// lava chunks
	std::vector<std::vector<std::string>> fixedChunks(std::string chunk);	// special chunks that are fixed
	//std::vector<std::string> innerChunksCT(std::vector<std::vector<std::vector<std::string>>> &c);
	//std::vector<std::string> innerChunksCB(std::vector<std::vector<std::vector<std::string>>> &c);
	//std::vector<std::string> centerChunks(std::vector<std::vector<std::vector<std::string>>> &c);

	std::vector<std::string> generateLevel();
	std::vector<std::string> combineChunks(std::vector<std::vector<std::vector<std::string>>> c);

private:
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
};

class ThirdFloor : public Dungeon {
public:
	ThirdFloor(std::shared_ptr<Player> p);
	ThirdFloor& operator=(ThirdFloor const &dungeon);

	void hideRooms();

protected:
	void checkDoors();

	std::vector<std::string> generateLevel();
	std::vector<std::string> combineChunks(std::vector<std::vector<std::vector<std::string>>> c);

private:
	bool roomCleared();
	void toggleDoorLocks(int dx, int dy, std::string doortype);
	void revealRoom();


	//		LEVEL GENERATION
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

	bool m_locked;

	// used for checking level generation of player and exit chunks
	bool m_playerplaced;
	bool m_exitplaced;

	// the row that the level generator is currently on
	int m_layer;

	// value from 1-4, tells the location of where the chunks are to be placed
	int m_playerchunk;
	int m_exitchunk;
};

class FourthFloor : public Dungeon {
public:
	FourthFloor(std::shared_ptr<Player> p);

private:
	
};

class FifthFloor : public Dungeon {
public:
	FifthFloor(std::shared_ptr<Player> p);

	

private:
	
};

class SixthFloor : public Dungeon {
public:
	SixthFloor(std::shared_ptr<Player> p);

private:
	
};

class SeventhFloor : public Dungeon {
public:
	SeventhFloor(std::shared_ptr<Player> p);

	

private:
	
};

class EighthFloor : public Dungeon {
public:
	EighthFloor(std::shared_ptr<Player> p);

	

private:
	
};

class NinthFloor : public Dungeon {
public:
	NinthFloor(std::shared_ptr<Player> p);

	

private:
	
};

class TenthFloor : public Dungeon {
public:
	TenthFloor(std::shared_ptr<Player> p);

	

private:
	
};

class FirstBoss : public Dungeon {
public:
	FirstBoss(std::shared_ptr<Player> p);
	FirstBoss& operator=(FirstBoss const &dungeon);

	void peekDungeon(int x, int y, char move);

	void fight(int x, int y);
	void monsterDeath(int pos);
	bool monsterHash(int &x, int &y, bool &switched, char move);
	void pushMonster(std::vector<_Tile> &dungeon, int mx, int my, char move, int cx = 0, int cy = 0);
	void pushPlayer(char move);

protected:
	void checkMonsters();

private:
	// used for telling what direction smasher is moving in if executing rock slide
	char move = '0';
};



#endif