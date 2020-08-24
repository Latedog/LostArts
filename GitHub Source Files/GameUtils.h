#ifndef GAMEUTILS_H
#define GAMEUTILS_H

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
struct Coords {
	int x;
	int y;

	Coords() {
		x = 0;
		y = 0;
	};

	Coords(int _x, int _y) {
		x = _x;
		y = _y;
	};

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
};

namespace std
{
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
int calculateDistanceBetween(int sx, int sy, int ex, int ey);
void insertCoordsAdjacentTo(const Dungeon &dungeon, std::vector<Coords> &coords, int x, int y, bool diagonals = false);
void getCoordsAdjacentTo(const Dungeon &dungeon, std::vector<Coords> &coords, int x, int y, bool all = false);
void getDiagonalCoordsAdjacentTo(const Dungeon &dungeon, std::vector<Coords> &coords, int x, int y);

// Return a uniformly distributed random integer from 0 to limit-1 inclusive
inline int randInt(int limit) {
	return std::rand() % limit;
}

// Return a uniformly distributed double between [min, max]
inline double randReal(int min, int max)
{
	if (max < min)
		std::swap(max, min);

	static std::random_device rd;
	static std::default_random_engine generator(rd());
	std::uniform_real_distribution<> distro(min, max);
	return distro(generator);
}

cocos2d::Vector<cocos2d::SpriteFrame*> getAnimationFrameVector(const char* format, int count);
cocos2d::Vector<cocos2d::SpriteFrame*> getAnimationFrameVector(std::string format, int count);
void setDirectionalOffsets(char move, int &n, int &m, int offset = 1, bool reversed = false);
void incrementDirectionalOffsets(char move, int &n, int &m);
void decrementDirectionalOffsets(char move, int &n, int &m);
bool directionIsOppositeTo(const char target, const char other);
char getDirectionOppositeTo(const char other);
char getFacingDirectionRelativeTo(int sx, int sy, int ex, int ey);
char getCardinalFacingDirectionRelativeTo(int sx, int sy, int ex, int ey);
bool isMovementAction(char move);

bool hasLineOfSight(const Dungeon &dungeon, char dir, int sx, int sy, int ex, int ey);
bool hasLineOfSight(const Dungeon &dungeon, char dir, const Player &p, int sx, int sy);

// Used when we don't know/don't require an explicit direction
bool hasLineOfSight(const Dungeon &dungeon, int sx, int sy, int ex, int ey);
bool hasLineOfSight(const Dungeon &dungeon, const Player &p, int sx, int sy);

bool playerInLinearRange(int range, int sx, int sy, int ex, int ey);

// Used for when player could potentially walk into, or is already in, linear range
bool playerInBufferedLinearRange(int range, int buffer, int sx, int sy, int ex, int ey);

bool playerIsAdjacent(const Player &p, int x, int y, bool diagonals = false);

bool playerInRectangularRange(const Player &p, int length, int width, int x, int y);

#endif