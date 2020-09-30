#ifndef DUNGEON_H
#define DUNGEON_H

#include <vector>

class cocos2d::Sprite;
class EffectSprite;
class cocos2d::SpriteFrame;
class cocos2d::FiniteTimeAction;
enum class DamageType;
class LevelScene;
class Afflictions;
class Player;
class NPC;
class Monster;
class Objects;
class Wall;
class Door;
class Drops;
class Spell;
class Passive;
class Relic;
class Weapon;
class Shield;
class Traps;
class Stairs;
class Decoy;
class Brazier;
class Gold;
struct Coords;

using SpriteType = EffectSprite;

// :::: Level boundaries ::::
const int MAXROWS = 38;// 18;
const int MAXCOLS = 38;// 70;

const int MAXROWS_SHOP1 = 15;
const int MAXCOLS_SHOP1 = 27;

const int MAXROWS2 = 56;// 44;// 38;
const int MAXCOLS2 = 74;// 78;

const int MAXROWS3 = 38 + 3;
const int MAXCOLS3 = 80 + 5;

const int MAXROWS_SHRINE1 = 26;
const int MAXCOLS_SHRINE1 = 26;

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
	//		Floor		: BOTTOM		-10
	//


	char upper;	// Layer for Smasher
	
	// Flags
	bool boundary = false;
	bool wall;
	bool item;
	bool trap;
	bool enemy;
	bool underEnemy = false;
	bool hero; // The player
	bool npc;
	bool exit;
	bool spirit; // The insta-kill enemy

	bool noSpawn; // nothing allowed to spawn here during level creation if true
	bool menu = false; // Used only in World Hub to return to the menu

	int gold;

	// For shop
	int price;

	// saves floor tile sprite for easy access
	cocos2d::Sprite* floor = nullptr;

	// Wall Object
	std::shared_ptr<Wall> wallObject;

	// Item Object
	std::shared_ptr<Objects> object;

	// shop item identifier
	std::string shop_action;

	// For Shrines
	std::string shrine_action;

	// for recursive backtracking optimization in shortest path algorithm
	bool marked;
};

class Dungeon {
public:
	Dungeon(LevelScene* scene, std::shared_ptr<Player> p, int level, int rows, int cols);
	_Tile& operator[](int index);
	virtual ~Dungeon();

	Dungeon(const Dungeon &) = delete;
	Dungeon& operator=(const Dungeon &) = delete;

	void peekDungeon(char move); // Core gameplay loop
	virtual void specialActions() { return; }; // Unique checks made in peekDungeon
	virtual bool specialTrapCheck(int x, int y) { return false; }; // Unique trap checks made when player moves onto a trap

	// :::: User functions ::::
	void assignQuickItem(int index);
	void callUse(int index);

	// :::: NPC functions ::::
	void interactWithNPC(int x, int y);
	void playNPCDialogue(const NPC &npc);

	// :::: Walls/Doors ::::
	void destroyWall(int x, int y);
	void removeWall(int x, int y);

	// :::: Trap Handling ::::
	void addTrap(std::shared_ptr<Traps> trap) { m_traps.push_back(trap); };
	std::vector<int> findTraps(int x, int y) const;
	int countTrapNumber(int x, int y) const; // returns the number of traps with this (x, y) pair
	void trapEncounter(int x, int y);
	void singleMonsterTrapEncounter(int pos);

	// :::: Decoys ::::
	void addDecoy(std::shared_ptr<Decoy> decoy) { m_decoys.push_back(decoy); };
	bool checkDecoys(int mx, int my, int &x, int &y);

	// :::: Actor Handling ::::
	void addMonster(std::shared_ptr<Monster> monster) { m_monsters.push_back(monster); };
	void damagePlayer(int damage, DamageType type);
	int findMonster(int x, int y) const;
	int findUndergroundMonster(int x, int y) const;
	void fight(int x, int y, DamageType type);
	void damageMonster(int index, int damage, DamageType type);
	void damageMonster(int x, int y, int damage, DamageType type);
	void giveAffliction(int index, std::shared_ptr<Afflictions> affliction);

	/**
	* NOTE: This function should almost never be called manually.
	*       It is currently public to allow MonsterTransform to call it so
	*       so that the enemy flag is not unflagged. 
	*
	* WARNING: By calling this function early, it must be certain that
	*          the Dungeon's checkMonsters() function is not currently being called.
	*          Otherwise, the incorrect monster(s) will be killed. */
	void monsterDeath(int pos);

	void pushMonster(int &mx, int &my, char move, int cx = 0, int cy = 0, bool strict = false);

	/* For pushing players/monsters in straight lines properly (i.e. no weird skipping through enemies)
	*  @x and @y are the coordinates of the initial target to be pushed.
	*  @limit is how far it will attempt to push.
	*  @range is how far away from the source it works.
	*  @lethal indicates that the actor should die if pushed against a wall. */
	void linearActorPush(int x, int y, int limit, int range, char move, bool pulling = false, bool lethal = false);

	// :::: Item Collection ::::
	void addItem(std::shared_ptr<Objects> item) { m_items.push_back(item); };
	void createItem(std::shared_ptr<Objects> item);
	void removeItem(int x, int y);
	void removeItemWithActions(int x, int y, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions);
	void itemHash(int &x, int &y);
	virtual void purchaseItem(int x, int y) { return; };
	virtual void collectItem(int x, int y);
	void goldPickup(int x, int y);
	void addGold(int x, int y, int amount);
	void removeGold(int x, int y);
	void removeGoldWithActions(int x, int y, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions);

	void advanceLevel() { m_level++; };
	void factoryTileCreation();
	void clonePlayer();

	//	:::: Getters ::::
	std::shared_ptr<Player> getPlayer() const { return player.at(0); };
	int getLevel() const { return m_level; };
	bool returnedToMenu() const { return m_return; };

	std::vector<_Tile> getDungeon() const { return m_maze; };
	int getRows() const { return m_rows; };
	int getCols() const { return m_cols; };
	std::shared_ptr<Monster> monsterAt(int index) const { return m_monsters[index]; };
	size_t monsterCount() const { return m_monsters.size(); };
	std::shared_ptr<Traps> trapAt(int index) const { return m_traps[index]; };
	size_t trapCount() const { return m_traps.size(); };

	// Helper
	Coords findNearbyFreeTile(const Coords &start) const;
	bool withinBounds(int x, int y) const;
	int gold(int x, int y) const { return m_maze[y*m_cols + x].gold; };
	bool boundary(int x, int y) const { return m_maze[y*m_cols + x].boundary; };
	bool wall(int x, int y) const { return m_maze[y*m_cols + x].wall; };
	bool item(int x, int y) const { return m_maze[y*m_cols + x].item; };
	bool enemy(int x, int y) const { return m_maze[y*m_cols + x].enemy; };
	bool underEnemy(int x, int y) const { return m_maze[y*m_cols + x].underEnemy; };
	bool hero(int x, int y) const { return m_maze[y*m_cols + x].hero; };
	bool npc(int x, int y) const { return m_maze[y*m_cols + x].npc; };
	bool trap(int x, int y) const { return m_maze[y*m_cols + x].trap; };
	bool exit(int x, int y) const { return m_maze[y*m_cols + x].exit; };
	bool noSpawn(int x, int y) const { return m_maze[y*m_cols + x].noSpawn; };
	bool marked(int x, int y) const { return m_maze[y*m_cols + x].marked; };
	std::shared_ptr<Objects> itemObject(int x, int y) { return m_maze[y*m_cols + x].object; };
	std::shared_ptr<Wall> wallObject(int x, int y) { return m_maze[y*m_cols + x].wallObject; };

	// For shops prices
	virtual bool isShop() const { return false; };
	virtual void constructShopHUD(int x, int y);
	virtual void deconstructShopHUD();

	virtual bool isShrine() const { return false; };

	virtual bool isBossLevel() const { return false; };

	virtual bool isHub() const { return false; };


	// :::: SPRITE MANIPULATION ::::

	// Converts a sprite's coordinates to the Dungeon's grid coordinates and vice versa.
	void transformSpriteToDungeonCoordinates(float x, float y, int &fx, int &fy) const;
	void transformDungeonToSpriteCoordinates(int x, int y, float &fx, float &fy) const;
	void transformDungeonToSpriteCoordinates(float x, float y, float &fx, float &fy) const;
	cocos2d::Vec2 dungeonToLightCoordinates(float x, float y) const;

	cocos2d::Sprite* createInvisibleSprite(int x, int y, int z, std::string image);
	cocos2d::Sprite* createSprite(int x, int y, int z, std::string image);
	SpriteType* createEffectSprite(int x, int y, int z, std::string image);
	cocos2d::Sprite* createSprite(float x, float y, int z, std::string image);
	cocos2d::Sprite* createMiscSprite(float x, float y, int z, std::string image);
	void addSprite(int x, int y, int z, std::string image);
	cocos2d::Sprite* createWallSprite(int x, int y, int z, std::string type);
	
	void teleportSprite(cocos2d::Sprite* sprite, int x, int y);
	void teleportSprite(cocos2d::Sprite* sprite, float x, float y);

	// @front specifies if this move action should be placed at the beginning of the sequence vector
	void queueMoveSprite(cocos2d::Sprite* sprite, int cx, int cy, float time = .1f, bool front = false);
	void queueMoveSprite(cocos2d::Sprite* sprite, float cx, float cy, float time = .1f, bool front = false);
	cocos2d::MoveTo* getMoveAction(int cx, int cy, float time = .1f);
	cocos2d::MoveTo* getMoveAction(float cx, float cy, float time = .1f);

	void queueRemoveSprite(cocos2d::Sprite* sprite);
	void queueRemoveSpriteWithMultipleActions(cocos2d::Sprite* sprite, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions);
	void removeSprite(cocos2d::Sprite* sprite);

	void queueCustomAction(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action);
	void queueCustomActions(cocos2d::Sprite* sprite, cocos2d::Vector<cocos2d::FiniteTimeAction*> actions);
	void queueCustomSpawnActions(cocos2d::Sprite* sprite, const cocos2d::Vector<cocos2d::FiniteTimeAction*> &actions);

	// Sprite lighting
	void updateLighting();

	void zoomInBy(float factor);
	void zoomOutBy(float factor);

	// @strength is how much light is to be provided
	void addLightSource(int x, int y, double strength, std::string name);
	void removeLightSource(int x, int y, std::string name);

	/* Actions with a tag of:
	*  1: Single-use action, it is run one time and may be forced to finish early
	*  5: Single animation that should be allowed to finish naturally
	*  10: Animation, should be run forever */
	cocos2d::Sprite* runAnimationForever(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z);
	void runSingleAnimation(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z);

	// This runs the animation and then leaves the final frame remaining.
	// Allows things such as debris to be left and have their sprites updated with proper lighting.
	void runAnimationWithCallback(cocos2d::Vector<cocos2d::SpriteFrame*> frames, int frameInterval, int x, int y, int z,
		std::function<void(Dungeon&, int, int, int, std::string)> cb, std::string image);

protected:
	void addDoor(std::shared_ptr<Door> door);
	virtual void checkDoors();

	// :::: LEVEL GENERATION ::::
	void generateNPCs();
	void generateMonsters();
	void generateTraps();
	void generateItems();
	void fillLevel(std::vector<std::string> finalvec, int start = 1, int end = -1);

	std::vector<std::shared_ptr<Player>> player;
	std::shared_ptr<Stairs> m_exit;

	std::vector<std::shared_ptr<NPC>> m_npcs;
	std::vector<std::shared_ptr<Monster>> m_monsters;
	std::vector<std::shared_ptr<Objects>> m_items;
	std::vector<std::shared_ptr<Traps>> m_traps;
	std::vector<std::shared_ptr<Door>> m_doors;
	std::vector<std::shared_ptr<Wall>> m_walls;
	std::vector<std::shared_ptr<Decoy>> m_decoys; // Decoys attract monsters that normally chase the player
	std::vector<std::shared_ptr<Gold>> m_money;

	LevelScene* m_scene;

	bool m_return = false; // Flag indicating that the player return to main menu from world hub

	std::vector<_Tile> m_maze;

private:
	struct LightSource {
		Coords pos;
		double radius;
		std::string name;

		LightSource(Coords _pos, double _radius, std::string _name) : pos(_pos), radius(_radius), name(_name) {};
	};

	// Sets the defaults for all _Tiles
	void initializeTiles();
	void createFlooring(int x, int y);

	void checkActive();
	void checkMonsters();

	void insertActorForRemoval(int index);
	void actorRemoval();
	void damageClone(int damage);

	void insertTrapForRemoval(int index);
	void trapRemoval();
	void monsterTrapEncounter();

	// Finds a valid spot for a monster after being pushed
	bool monsterHash(int &x, int &y, bool &switched, char move, bool strict = false);

	int findNPC(int x, int y) const;
	int findDecoy(int x, int y) const;
	int findSegmentedMonster(int index) const;
	int findDoor(int x, int y) const;
	int findWall(int x, int y) const;
	int findItem(int x, int y) const;
	int findGold(int x, int y) const;

	int spriteFound(cocos2d::Sprite* sprite);
	void insertActionIntoSpawn(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action, bool front = false);
	void runSpawn();

	int findLightSource(int x, int y, std::string name);
	void updateSecondaryLighting();

	void updateSpriteScaling();

	struct MiscSprite {
		Coords pos;
		cocos2d::Sprite* sprite;

		MiscSprite(Coords _pos, cocos2d::Sprite* _sprite) : pos(_pos), sprite(_sprite) {};
	};
	std::vector<MiscSprite> m_miscSprites; // For decorations, debris, etc.

	std::vector<std::shared_ptr<Player>> m_playerClones;

	int m_level;

	std::set<int> m_trapIndexes;
	std::set<int> m_deadActorIndexes;

	std::vector<LightSource> m_lightSources;
	std::vector<std::pair<int, int>> lightEmitters;

	// Vector containing the sprite movement actions
	std::vector<std::pair<cocos2d::Sprite*, cocos2d::Vector<cocos2d::FiniteTimeAction*>>> m_seq;

	int m_rows;
	int m_cols;

	int matrixTurns = 0; // Used for MatrixVision
	int matrixCooldown = 0; // Cooldown before MatrixVision can be used again
};

class TutorialFloor : public Dungeon {
public:
	TutorialFloor(LevelScene* scene, std::shared_ptr<Player> p);

private:
	std::vector<char> generate();
};

class WorldHub : public Dungeon {
public:
	WorldHub(LevelScene* scene, std::shared_ptr<Player> p);

	void specialActions() override;

private:
	bool isHub() const override { return true; };

	std::vector<char> generate();
};

class Shop : public Dungeon {
public:
	Shop(LevelScene* scene, std::shared_ptr<Player> p, int level);

	bool isShop() const override { return true; };
	void constructShopHUD(int x, int y) override;
	void deconstructShopHUD() override;

	void purchaseItem(int x, int y) override;
	void collectItem(int x, int y) override;

private:
	std::vector<char> generate();

	float m_priceMultiplier;
};

class Shrine : public Dungeon {
public:
	// Shrines are shaping up to act like Devil Deals. However, accepting a gift from
	// a shrine will increase the player's 'Curse' stat which will have negative effects
	// for the player. But with enough Curse, the player can access the bonus floors, assuming
	// other appropirate conditions are also filled.
	Shrine(LevelScene* scene, std::shared_ptr<Player> p, int level);
	~Shrine();

	void specialActions() override;

	void useChoice(int index);
	bool sacrificeWeapon();
	void giveRelic();
	void upgradeRelic();
	void swapRelic();
	void spawnChests();

	bool isShrine() const override { return true; };

private:
	std::vector<char> generate();

	bool makingChoice = false; // So the shrine prompt does not appear multiple times

	bool choseChests = false; // If the player decided with chests
	std::vector<std::pair<int, int>> chestCoordinates;

	bool braziersLit = false;
	std::vector<std::shared_ptr<Brazier>> litBraziers;
};

class FirstFloor : public Dungeon {
public:
	FirstFloor(LevelScene* scene, std::shared_ptr<Player> p);

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
	SecondFloor(LevelScene* scene, std::shared_ptr<Player> p);

	void specialActions() override;
	bool specialTrapCheck(int x, int y) override;

	void devilsWater(bool bathed);
	void devilsGift();
	void guardiansDefeated();
	void devilsWaterPrompt();
	bool getWaterPrompt() const { return m_waterPrompt; };
	bool watersUsed() const { return m_watersUsed; };

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
	std::vector<std::shared_ptr<Monster>> m_guardians;

	bool m_waterPrompt = false;
	bool m_watersUsed = false;
	bool m_watersCleared;
	int m_guardiansLeft;

	int layer = 1;
	int specialChunkLayer1;
	int specialChunkLayer2;
};

class ThirdFloor : public Dungeon {
public:
	ThirdFloor(LevelScene* scene, std::shared_ptr<Player> p);

	void hideRooms();

protected:
	void checkDoors() override;

	std::vector<std::string> generateLevel();
	std::vector<std::string> combineChunks(std::vector<std::vector<std::vector<std::string>>> c);

private:
	bool roomCleared();
	void toggleDoorLocks(int dx, int dy);
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
	FourthFloor(LevelScene* scene, std::shared_ptr<Player> p);

private:
	
};

class FifthFloor : public Dungeon {
public:
	FifthFloor(LevelScene* scene, std::shared_ptr<Player> p);

	

private:
	
};

class SixthFloor : public Dungeon {
public:
	SixthFloor(LevelScene* scene, std::shared_ptr<Player> p);

private:
	
};

class SeventhFloor : public Dungeon {
public:
	SeventhFloor(LevelScene* scene, std::shared_ptr<Player> p);

	

private:
	
};

class EighthFloor : public Dungeon {
public:
	EighthFloor(LevelScene* scene, std::shared_ptr<Player> p);

	

private:
	
};

class NinthFloor : public Dungeon {
public:
	NinthFloor(LevelScene* scene, std::shared_ptr<Player> p);

	

private:
	
};

class TenthFloor : public Dungeon {
public:
	TenthFloor(std::shared_ptr<Player> p);

	

private:
	
};

class FirstBoss : public Dungeon {
public:
	FirstBoss(LevelScene* scene, std::shared_ptr<Player> p);

	bool isBossLevel() const override { return true; };

private:
	
};



#endif