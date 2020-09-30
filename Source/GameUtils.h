<<<<<<< HEAD
#ifndef GAMEUTILS_H
#define GAMEUTILS_H

#include "include/rapidjson/document.h"

class cocos2d::SpriteFrame;
class Dungeon;
class Player;
class NPC;
class Monster;
class Drops;
class Spell;
class Passive;
class Relic;
class Chests;
class Weapon;
class Shield;

extern std::string GAME_TEXT_FILE;

enum class DamageType {
	NORMAL, PIERCING, CRUSHING, EXPLOSIVE, FIRE, MELTING, ACIDIC, POISONOUS, LIGHTNING, MAGICAL
};

enum class StatType {
	MAXHP, HP, STRENGTH, ARMOR, DEXTERITY, INTELLECT, LUCK, CHARISMA
};

enum class Rarity {
	ALL, COMMON, UNCOMMON, RARE, MYTHICAL, LEGENDARY
};

// If @fallThrough is true, any item at a Rarity level of @rarity and lower can be chosen
std::shared_ptr<Drops> rollItem(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false, bool create = true);
std::shared_ptr<Spell> rollSpell(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Passive> rollPassive(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Passive> rollStatPassive(Dungeon *dungeon, int x, int y);
std::shared_ptr<Relic> rollRelic(Dungeon *dungeon, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Chests> rollChest(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Weapon> rollWeapon(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Shield> rollShield(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);

// If Rarity is ALL, then roll to determine the rarity to use
Rarity rollRarity();

// Bump up the rarity level by one if all passives of level @rarity have been seen
Rarity increaseRarityLevel(Rarity rarity);

class GameTable {
public:
	static void initializeTables();
};

class LootTable {
public:
	static void initializeLootTable();

	static bool allPassivesSeen();
	static bool allPassivesOfRaritySeen(Rarity rarity);
	static void updatePassiveRarityCount(Rarity rarity); // @@@ This must be updated as more passives are added @@@
	static bool passiveWasAcquired(std::string passive);
	static void acquiredPassive(std::string passive, Rarity rarity);

	static bool relicWasAcquired(std::string relic);
	static void acquiredRelic(std::string relic);

private:
	static std::unordered_map<std::string, bool> m_passivesObtained;
	static unsigned int m_commonPassiveCount;
	static unsigned int m_uncommonPassiveCount;
	static unsigned int m_rarePassiveCount;
	static unsigned int m_mythicalPassiveCount;
	static unsigned int m_legendaryPassiveCount;

	static std::unordered_map<std::string, bool> m_relicsObtained;
};

class NPCTable {
public:
	static void initializeNPCTable();

	static bool allNPCsSeen();
	static bool NPCWasSeen(std::string name);
	static void encounteredNPC(std::string name);

private:
	static std::unordered_map<std::string, bool> m_NPCsEncountered;
};

std::shared_ptr<NPC> rollNPC(Dungeon *dungeon, int x, int y);

std::shared_ptr<Monster> createMonsterByName(std::string name, Dungeon *dungeon, int x, int y);
std::shared_ptr<Monster> rollMonster(int level, Dungeon *dungeon, int x, int y);

// Common utilities
class GameTimers {
public:
	static void addGameTimer(const std::string &name, void *target);
	static void removeGameTimer(const std::string &name);
	static void resumeAllGameTimers();
	static void pauseAllGameTimers();
	static void removeAllGameTimers();

private:
	static std::map<std::string, void*> m_gameTimers;
};

struct Coords {
	int x;
	int y;

	Coords() : x(0), y(0) {};
	Coords(int _x, int _y) : x(_x), y(_y) {};
	Coords(const Coords &other) : x(other.x), y(other.y) {};

	operator cocos2d::Vec2() const { return cocos2d::Vec2(x, y); };

	Coords& operator=(const Coords &other) {
		x = other.x;
		y = other.y;

		return *this;
	};

	bool operator==(const Coords &other) const {
		return x == other.x && y == other.y;
	};
	bool operator!=(const Coords &other) const {
		return !(*this == other);
	};
	bool operator<(const Coords &other) const {
		return x < other.x && y < other.y;
	};
	Coords operator+(const Coords &right) const {
		return Coords(x + right.x, y + right.y);
	};

	bool isAbove(const Coords &other) const {
		return y < other.y;
	};
	bool isBelow(const Coords &other) const {
		return y > other.y;
	};
	bool isLeftOf(const Coords &other) const {
		return x < other.x;
	};
	bool isRightOf(const Coords &other) const {
		return x > other.x;
	};
};

namespace std {
	template <>
	struct hash<Coords>
	{
		size_t operator()(const Coords& c) const
		{
			// Compute individual hash values for two data members and combine them using XOR and bit shifting
			return ((hash<int>()(c.x) ^ (hash<int>()(c.y) << 1)) >> 1);
		}
	};
}

int calculateDistanceBetween(const Coords &start, const Coords &end);
void getCoordsAdjacentTo(const Dungeon &dungeon, std::vector<Coords> &coords, const Coords &start, bool all = false);
void getDiagonalCoordsAdjacentTo(const Dungeon &dungeon, std::vector<Coords> &coords, const Coords &start);

// Return a uniformly distributed random integer between [min, max]
inline int randInt(int min, int max) {
	if (max < min)
		std::swap(max, min);

	static std::random_device rd;
	static std::default_random_engine generator(rd());
	std::uniform_int_distribution<> distro(min, max);
	return distro(generator);
}

// Return a uniformly distributed double between [min, max]
inline double randReal(int min, int max) {
	if (max < min)
		std::swap(max, min);

	static std::random_device rd;
	static std::default_random_engine generator(rd());
	std::uniform_real_distribution<> distro(min, max);
	return distro(generator);
}

cocos2d::Vec2 getHalfTileYOffset();
cocos2d::Vector<cocos2d::SpriteFrame*> getAnimationFrameVector(const char* format, int count);
cocos2d::Vector<cocos2d::SpriteFrame*> getAnimationFrameVector(std::string format, int count);
void setDirectionalOffsets(char move, int &n, int &m, int offset = 1, bool reversed = false);
void incrementDirectionalOffsets(char move, int &n, int &m);
void decrementDirectionalOffsets(char move, int &n, int &m);
bool directionIsOppositeTo(const char target, const char other);
char getDirectionOppositeTo(const char other);
char getFacingDirectionRelativeTo(const Coords &start, const Coords &end);
char getCardinalFacingDirectionRelativeTo(const Coords &start, const Coords &end);
bool isMovementAction(char move);

bool hasLineOfSight(const Dungeon &dungeon, char dir, const Coords &start, const Coords &end);

// Used when we don't know/don't require an explicit direction
bool hasLineOfSight(const Dungeon &dungeon, const Coords &start, const Coords &end);

bool positionInRange(const Coords &start, const Coords &end, int range);

bool positionInLinearRange(const Coords &start, const Coords &end, int range);

// Used for when player could potentially walk into, or is already in, linear range
bool positionInBufferedLinearRange(const Coords &start, const Coords &end, int range, int buffer);

bool positionIsAdjacent(const Coords &start, const Coords &end, bool diagonals = false);

bool positionInRectangularRange(const Coords &start, const Coords &end, int length, int width);


// Text Utils
class TextUtils {
public:
	static void initJsonText();

	static rapidjson::Document jsonTree;
};

// json text parsers
std::string fetchMenuText(const std::string &menuType, const std::string &id);
std::string fetchPromptText(const std::string &promptType, const std::string &id);
void fetchNPCDialogue(const std::string &name, const std::string &id, std::vector<std::string> &dialogue);
void fetchItemInfo(const std::string &type, const std::string &id, std::string &name, std::string &desc);
std::string fetchItemName(const std::string &type, const std::string &id);

// Deprecated. Plain text parsers.
void getNPCDialogue(const std::string &filename, const std::string &id, std::vector<std::string> &dialogue);
void getItemInfo(const std::string &filename, const std::string &id, std::string &name, std::string &desc);
std::string getItemName(const std::string &filename, const std::string &id);

void formatItemDescriptionForDisplay(std::string &desc);
void replaceTextVariableWith(std::vector<std::string> &text, const std::string &varText, const std::string &actualText);

// Save File System
class SaveManager {
public:
	static void init();
	static void createNewSaveFile();
	static void loadSaveFile(const std::string &file);
	static void overwriteSaveFile(const std::string &file);
	static void deleteSaveFile(const std::string &file);

private:
	
};

=======
#ifndef GAMEUTILS_H
#define GAMEUTILS_H

#include "include/rapidjson/document.h"

class cocos2d::SpriteFrame;
class Dungeon;
class Player;
class NPC;
class Monster;
class Drops;
class Spell;
class Passive;
class Relic;
class Chests;
class Weapon;
class Shield;

extern std::string GAME_TEXT_FILE;

enum class DamageType {
	NORMAL, PIERCING, CRUSHING, EXPLOSIVE, FIRE, MELTING, ACIDIC, POISONOUS, LIGHTNING, MAGICAL
};

enum class StatType {
	MAXHP, HP, STRENGTH, ARMOR, DEXTERITY, INTELLECT, LUCK, CHARISMA
};

enum class Rarity {
	ALL, COMMON, UNCOMMON, RARE, MYTHICAL, LEGENDARY
};

// If @fallThrough is true, any item at a Rarity level of @rarity and lower can be chosen
std::shared_ptr<Drops> rollItem(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false, bool create = true);
std::shared_ptr<Spell> rollSpell(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Passive> rollPassive(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Passive> rollStatPassive(Dungeon *dungeon, int x, int y);
std::shared_ptr<Relic> rollRelic(Dungeon *dungeon, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Chests> rollChest(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Weapon> rollWeapon(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);
std::shared_ptr<Shield> rollShield(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough = false);

// If Rarity is ALL, then roll to determine the rarity to use
Rarity rollRarity();

// Bump up the rarity level by one if all passives of level @rarity have been seen
Rarity increaseRarityLevel(Rarity rarity);

class GameTable {
public:
	static void initializeTables();
};

class LootTable {
public:
	static void initializeLootTable();

	static bool allPassivesSeen();
	static bool allPassivesOfRaritySeen(Rarity rarity);
	static void updatePassiveRarityCount(Rarity rarity); // @@@ This must be updated as more passives are added @@@
	static bool passiveWasAcquired(std::string passive);
	static void acquiredPassive(std::string passive, Rarity rarity);

	static bool relicWasAcquired(std::string relic);
	static void acquiredRelic(std::string relic);

private:
	static std::unordered_map<std::string, bool> m_passivesObtained;
	static unsigned int m_commonPassiveCount;
	static unsigned int m_uncommonPassiveCount;
	static unsigned int m_rarePassiveCount;
	static unsigned int m_mythicalPassiveCount;
	static unsigned int m_legendaryPassiveCount;

	static std::unordered_map<std::string, bool> m_relicsObtained;
};

class NPCTable {
public:
	static void initializeNPCTable();

	static bool allNPCsSeen();
	static bool NPCWasSeen(std::string name);
	static void encounteredNPC(std::string name);

private:
	static std::unordered_map<std::string, bool> m_NPCsEncountered;
};

std::shared_ptr<NPC> rollNPC(Dungeon *dungeon, int x, int y);

std::shared_ptr<Monster> createMonsterByName(std::string name, Dungeon *dungeon, int x, int y);
std::shared_ptr<Monster> rollMonster(int level, Dungeon *dungeon, int x, int y);

// Common utilities
class GameTimers {
public:
	static void addGameTimer(const std::string &name, void *target);
	static void removeGameTimer(const std::string &name);
	static void resumeAllGameTimers();
	static void pauseAllGameTimers();
	static void removeAllGameTimers();

private:
	static std::map<std::string, void*> m_gameTimers;
};

struct Coords {
	int x;
	int y;

	Coords() : x(0), y(0) {};
	Coords(int _x, int _y) : x(_x), y(_y) {};
	Coords(const Coords &other) : x(other.x), y(other.y) {};

	operator cocos2d::Vec2() const { return cocos2d::Vec2(x, y); };

	Coords& operator=(const Coords &other) {
		x = other.x;
		y = other.y;

		return *this;
	};

	bool operator==(const Coords &other) const {
		return x == other.x && y == other.y;
	};
	bool operator!=(const Coords &other) const {
		return !(*this == other);
	};
	bool operator<(const Coords &other) const {
		return x < other.x && y < other.y;
	};
	Coords operator+(const Coords &right) const {
		return Coords(x + right.x, y + right.y);
	};

	bool isAbove(const Coords &other) const {
		return y < other.y;
	};
	bool isBelow(const Coords &other) const {
		return y > other.y;
	};
	bool isLeftOf(const Coords &other) const {
		return x < other.x;
	};
	bool isRightOf(const Coords &other) const {
		return x > other.x;
	};
};

namespace std {
	template <>
	struct hash<Coords>
	{
		size_t operator()(const Coords& c) const
		{
			// Compute individual hash values for two data members and combine them using XOR and bit shifting
			return ((hash<int>()(c.x) ^ (hash<int>()(c.y) << 1)) >> 1);
		}
	};
}

int calculateDistanceBetween(const Coords &start, const Coords &end);
void getCoordsAdjacentTo(const Dungeon &dungeon, std::vector<Coords> &coords, const Coords &start, bool all = false);
void getDiagonalCoordsAdjacentTo(const Dungeon &dungeon, std::vector<Coords> &coords, const Coords &start);

// Return a uniformly distributed random integer between [min, max]
inline int randInt(int min, int max) {
	if (max < min)
		std::swap(max, min);

	static std::random_device rd;
	static std::default_random_engine generator(rd());
	std::uniform_int_distribution<> distro(min, max);
	return distro(generator);
}

// Return a uniformly distributed double between [min, max]
inline double randReal(int min, int max) {
	if (max < min)
		std::swap(max, min);

	static std::random_device rd;
	static std::default_random_engine generator(rd());
	std::uniform_real_distribution<> distro(min, max);
	return distro(generator);
}

cocos2d::Vec2 getHalfTileYOffset();
cocos2d::Vector<cocos2d::SpriteFrame*> getAnimationFrameVector(const char* format, int count);
cocos2d::Vector<cocos2d::SpriteFrame*> getAnimationFrameVector(std::string format, int count);
void setDirectionalOffsets(char move, int &n, int &m, int offset = 1, bool reversed = false);
void incrementDirectionalOffsets(char move, int &n, int &m);
void decrementDirectionalOffsets(char move, int &n, int &m);
bool directionIsOppositeTo(const char target, const char other);
char getDirectionOppositeTo(const char other);
char getFacingDirectionRelativeTo(const Coords &start, const Coords &end);
char getCardinalFacingDirectionRelativeTo(const Coords &start, const Coords &end);
bool isMovementAction(char move);

bool hasLineOfSight(const Dungeon &dungeon, char dir, const Coords &start, const Coords &end);

// Used when we don't know/don't require an explicit direction
bool hasLineOfSight(const Dungeon &dungeon, const Coords &start, const Coords &end);

bool positionInRange(const Coords &start, const Coords &end, int range);

bool positionInLinearRange(const Coords &start, const Coords &end, int range);

// Used for when player could potentially walk into, or is already in, linear range
bool positionInBufferedLinearRange(const Coords &start, const Coords &end, int range, int buffer);

bool positionIsAdjacent(const Coords &start, const Coords &end, bool diagonals = false);

bool positionInRectangularRange(const Coords &start, const Coords &end, int length, int width);


// Text Utils
class TextUtils {
public:
	static void initJsonText();

	static rapidjson::Document jsonTree;
};

// json text parsers
std::string fetchMenuText(const std::string &menuType, const std::string &id);
std::string fetchPromptText(const std::string &promptType, const std::string &id);
void fetchNPCDialogue(const std::string &name, const std::string &id, std::vector<std::string> &dialogue);
void fetchItemInfo(const std::string &type, const std::string &id, std::string &name, std::string &desc);
std::string fetchItemName(const std::string &type, const std::string &id);

// Deprecated. Plain text parsers.
void getNPCDialogue(const std::string &filename, const std::string &id, std::vector<std::string> &dialogue);
void getItemInfo(const std::string &filename, const std::string &id, std::string &name, std::string &desc);
std::string getItemName(const std::string &filename, const std::string &id);

void formatItemDescriptionForDisplay(std::string &desc);
void replaceTextVariableWith(std::vector<std::string> &text, const std::string &varText, const std::string &actualText);

// Save File System
class SaveManager {
public:
	static void init();
	static void createNewSaveFile();
	static void loadSaveFile(const std::string &file);
	static void overwriteSaveFile(const std::string &file);
	static void deleteSaveFile(const std::string &file);

private:
	
};

>>>>>>> 2989379ff3214d5b91cd4c670e60b6aee17c666b
#endif