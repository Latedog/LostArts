#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

class cocos2d::Sprite;
class Dungeon;
class Actors;
class Player;
enum class DamageType;

class Objects {
public:
	Objects();
	Objects(int x, int y, std::string item);
	Objects(int x, int y, std::string item, std::string image);
	virtual ~Objects();

	std::string getName() const { return m_name; };
	std::string getImageName() const { return m_image; };
	cocos2d::Sprite* getSprite() { return m_sprite; };
	int getPosX() const { return m_x; };
	int getPosY() const { return m_y; };

	void setName(std::string name) { m_name = name; };
	void setSprite(cocos2d::Sprite* sprite) { m_sprite = sprite; };
	void setPosX(int x) { m_x = x; };
	void setPosY(int y) { m_y = y; };

	bool isDestructible() const { return m_destructible; };
	virtual bool isWeapon() const { return false; };
	virtual bool isShield() const { return false; };
	virtual bool isSpell() const { return false; };
	bool isItem() const { return m_isItem; };
	bool isAutoUse() const { return m_autoUse; };
	virtual bool isChest() const { return false; };
	virtual bool isTrinket() const { return false; };
	virtual bool isPassive() const { return false; }

	std::string getDescription() const { return m_description; };
	void setDescription(std::string description) { m_description = description; };

	std::string getSoundName() const { return m_sound; };
	bool hasExtraSprites() const { return m_hasExtraSprites; };
	virtual void setSpriteColor(cocos2d::Color3B color) { ; };
	bool emitsLight() const { return m_emitsLight; }; // tells if the sprite should emit extra light

protected:
	void setImageName(std::string image) { m_image = image; };
	void setSoundName(std::string sound) { m_sound = sound; };
	void setExtraSpritesFlag(bool extras) { m_hasExtraSprites = extras; };
	void setEmitsLight(bool emits) { m_emitsLight = emits; };

	void setDestructible(bool destructible) { m_destructible = destructible; };
	void setWeaponFlag(bool weapon) { m_isWeapon = weapon; };
	void setShieldFlag(bool shield) { m_isShield = shield; };
	void setItemFlag(bool item) { m_isItem = item; };
	void setAutoFlag(bool autoUse) { m_autoUse = autoUse; };
	void setChestFlag(bool chest) { m_isChest = chest; };
	void setTrinketFlag(bool trinket) { m_isTrinket = trinket; };

	Dungeon *m_dungeon = nullptr;

private:
	std::string m_name;
	std::string m_image; // sprite image name
	std::string m_sound; // sound clip to be used on pickup
	cocos2d::Sprite* m_sprite = nullptr;
	bool m_hasExtraSprites = false;
	bool m_emitsLight = false;

	std::string m_description; // short description of the item

	int m_x;
	int m_y;

	bool m_destructible = false; // flag for telling if the object can be destroyed
	bool m_isWeapon = false;
	bool m_isItem = false;
	bool m_autoUse = false;
	bool m_isChest = false;
	bool m_isShield = false;
	bool m_isTrinket = false;
};

class Idol : public Objects {
public:
	Idol(int x, int y);
};

class Gold : public Objects {
public:
	Gold(Dungeon &dungeon, int x, int y, std::string image);
};


//		WALLS
class Wall : public Objects {
public:
	Wall(Dungeon *dungeon, int x, int y, std::string type);

	virtual void destroy();

	virtual bool isBoundary() const { return false; };
	virtual bool isDestructible() const { return true; };
	virtual bool isDoor() const { return false; };
};

class Boundary : public Wall {
public:
	Boundary(Dungeon *dungeon, int x, int y);

	void destroy() { return; };

	bool isBoundary() const { return true; };
	bool isDestructible() const { return false; };
};

class RegularWall : public Wall {
public:
	RegularWall(Dungeon *dungeon, int x, int y);
};

class UnbreakableWall : public Wall {
public:
	UnbreakableWall(Dungeon *dungeon, int x, int y);

	void destroy() { return; };

	bool isDestructible() const { return false; };
};

class Fountain : public Wall {
public:
	Fountain(Dungeon *dungeon, int x, int y);
};

class Door : public Wall {
public:
	Door(Dungeon *dungeon, int x, int y, std::string type);

	bool isDoor() const { return true; };

	void interact();

	bool isOpen() const { return m_open; };
	void toggleOpen() { m_open = !m_open; };
	bool isHeld() const { return m_hold; };
	void toggleHeld() { m_hold = !m_hold; };
	bool isLocked() const { return m_lock; };
	void toggleLock() { m_lock = !m_lock; };

private:
	bool m_open;
	bool m_hold;
	bool m_lock;
};

class RegularDoor : public Door {
public:
	RegularDoor(Dungeon *dungeon, int x, int y, std::string type);
};

class BlastDoor : public Door {
public:
	BlastDoor(Dungeon *dungeon, int x, int y, std::string type);

	bool isDestructible() const { return false; };
};


//	BEGIN DROPS CLASSES
class Drops : public Objects {
public:
	Drops(int x, int y, std::string item, std::string image);

	virtual void useItem(Dungeon &dungeon) { return; };
	virtual void pickupEffect(Dungeon &dungeon) { return; }; // Effects that occur when the item is picked up

	bool forPlayer() const { return m_forPlayer; };

	// For items that can stack.
	// The FatStacks passive lets all items stack, which is why this is here.
	bool canStack() const { return m_canStack; };
	int getCount() const { return m_count; };
	void increaseCountBy(int count) { m_count += count; };
	void decreaseCount() { m_count--; };

protected:
	void setForPlayer(bool forPlayer) { m_forPlayer = forPlayer; };
	void setCanStack(bool stack) { m_canStack = stack; };

	int m_count = 1; // Number of each stackable remaining

private:
	bool m_forPlayer = true; // flag to check if item is used on the player or the dungeon
	bool m_canStack = false; // Flag indicating that this item can stack
};

class HeartPod : public Drops {
public:
	HeartPod(int x, int y);
	void useItem(Dungeon &dungeon);
};

class LifePotion : public Drops {
public:
	LifePotion(int x, int y);
	void useItem(Dungeon &dungeon);
};

class BigLifePotion : public Drops {
public:
	BigLifePotion(int x, int y);
	void useItem(Dungeon &dungeon);
};

class SteadyLifePotion : public Drops {
public:
	// Gives player a heal over time
	SteadyLifePotion(int x, int y);
	void useItem(Dungeon &dungeon);
};

class HalfLifePotion : public Drops {
public:
	// Heals player up to half their maximum HP. Otherwise it does nothing.
	HalfLifePotion(int x, int y);
	void useItem(Dungeon &dungeon);
};

class SoulPotion : public Drops {
public:
	// Converts the player's money into hp
	SoulPotion(int x, int y);
	void useItem(Dungeon &dungeon);
};

class BinaryLifePotion : public Drops {
public:
	// Heals 50% of the player's missing hp
	BinaryLifePotion(int x, int y);
	void useItem(Dungeon &dungeon);
};

class StatPotion : public Drops {
public:
	StatPotion(int x, int y);
	void useItem(Dungeon &dungeon);
};

class RottenApple : public Drops {
public:
	// Heals to full, but applies a deadly poison
	RottenApple(int x, int y);
	void useItem(Dungeon &dungeon);
};

class Antidote : public Drops {
public:
	// Cures poison
	Antidote(int x, int y);
	void useItem(Dungeon &dungeon);
};

class ArmorDrop : public Drops {
public:
	ArmorDrop(int x, int y);
	void useItem(Dungeon &dungeon);
};

class ShieldRepair : public Drops {
public:
	ShieldRepair(int x, int y);
	void useItem(Dungeon &dungeon);
};

class DizzyElixir : public Drops {
public:
	DizzyElixir(int x, int y);
	void useItem(Dungeon &dungeon);
};

class PoisonBomb : public Drops {
public:
	PoisonBomb(int x, int y);
	void useItem(Dungeon &dungeon);
};

class RottenMeat : public Drops {
public:
	// Places a piece of rotten meat on the ground.
	// Removes aggro from you for a little while and poisons monsters that consume this.
	RottenMeat(int x, int y);
	void useItem(Dungeon &dungeon);
};

class MattockDust : public Drops {
public:
	// Created when the Mattock Head is destroyed.
	// Placeholder effect: Confuses yourself and nearby enemies.
	MattockDust(int x, int y);
	void useItem(Dungeon &dungeon);
};

class Teleport : public Drops {
public:
	// Teleport to a random location in the dungeon
	Teleport(int x, int y);
	void useItem(Dungeon &dungeon);
};

class SmokeBomb : public Drops {
public:
	// Throwable. Leaves a circular cloud of smoke on impact or when it travels far enough.
	// "Possesses" all monsters which essentially will make them move erratically.
	SmokeBomb(int x, int y);
	void useItem(Dungeon &dungeon);
};

class WildMushroom : public Drops {
public:
	// 50% chance to poison the player.
	// 50% chance to give strength or heal. Very small chance to grant the strength bonus.
	WildMushroom(int x, int y);
	void useItem(Dungeon &dungeon);
};

class MagmaHeart : public Drops {
public:
	// After consumption, this restores 20% of the player's health when they reach the next floor
	MagmaHeart(int x, int y);
	void useItem(Dungeon &dungeon);
};

class CactusWater : public Drops {
public:
	// Dropped by Cacti
	CactusWater(int x, int y);

	void pickupEffect(Dungeon &dungeon);
	void useItem(Dungeon &dungeon);
};

class Stackable : public Drops {
public:
	// Items that can be stacked beyond 1
	Stackable(int x, int y, std::string name, std::string image, int count);
};

class Bomb : public Stackable {
public:
	Bomb(int x, int y);
	void useItem(Dungeon &dungeon);
};

class BearTrap : public Stackable {
public:
	// Places a bear trap on the ground.
	// Anything that steps on it is immediately damaged, bled, and crippled.
	BearTrap(int x, int y);
	void useItem(Dungeon &dungeon);
};

class Matches : public Stackable {
public:
	// Used to light braziers, or ignite other burnable objects
	Matches(int x, int y);
	void useItem(Dungeon &dungeon);
};

class Firecrackers : public Stackable {
public:
	// Throwable item. Stuns enemies for 1 turn and has a small chance to ignite.
	Firecrackers(int x, int y);
	void useItem(Dungeon &dungeon);
};

class SkeletonKey : public Drops {
public:
	SkeletonKey(int x, int y);

	void useItem(Dungeon &dungeon);
};


//		SPECIAL
class Teleporter : public Drops {
public:
	// Teleporters the player X spaces in the direction they're facing.
	// If there is no free space, then it looks for an adjacent (cardinal) spot that is free.
	// If none, they die.
	Teleporter(int x = 0, int y = 0);
	void useItem(Dungeon &dungeon);
};

class Rocks : public Drops {
public:
	// Throwable item. Stuns enemies for 1 turn and knocks them back by up to two tiles.
	Rocks(int x = 0, int y = 0);
	void useItem(Dungeon &dungeon);

private:
	int m_durability;
};

class Mobility : public Drops {
public:
	// The special ability for the Acrobat character.
	Mobility(int x = 0, int y = 0);
	void useItem(Dungeon &dungeon);
};


//		SPELLS
class Spell : public Drops {
public:
	Spell(int x, int y, int damage, std::string item, std::string image);

	bool isSpell() const { return true; };

protected:
	int getDamage() const { return m_damage; };
	void setDamage(int damage) { m_damage = damage; };

private:
	int m_damage = 0;

};

// Ice
class FreezeSpell : public Spell {
public:
	FreezeSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

class IceShardSpell : public Spell {
public:
	IceShardSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

class HailStormSpell : public Spell {
public:
	HailStormSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

// Earth
class EarthquakeSpell : public Spell {
public:
	EarthquakeSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

class RockSummonSpell : public Spell {
public:
	RockSummonSpell(int x, int y);
	void useItem(Dungeon &dungeon);

private:
	void setCoords(Dungeon &dungeon);
	std::vector<std::pair<int, int>> coords; // For the spaces that the special move will attempt to strike
};

class ShockwaveSpell : public Spell {
public:
	ShockwaveSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

// Fire
class FireBlastSpell : public Spell {
public:
	FireBlastSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

class FireCascadeSpell : public Spell {
public:
	FireCascadeSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

class FireExplosionSpell : public Spell {
public:
	FireExplosionSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

// Air
class WhirlwindSpell : public Spell {
public:
	WhirlwindSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

class WindBlastSpell : public Spell {
public:
	WindBlastSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

class WindVortexSpell : public Spell {
public:
	WindVortexSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

// Misc
class ThunderCloudSpell : public Spell {
public:
	// Sends forth a slow-moving cloud that shocks nearby enemies and chains lightning.
	ThunderCloudSpell(int x, int y);
	void useItem(Dungeon &dungeon);
};

class InvisibilitySpell : public Spell {
public:
	InvisibilitySpell(int x, int y);

	void useItem(Dungeon &dungeon);
};

class EtherealSpell : public Spell {
public:
	EtherealSpell(int x, int y);

	void useItem(Dungeon &dungeon);
};


//		PASSIVES
class Passive : public Objects {
public:
	// Passives are one time only equips.
	// Once the player acquires it, it cannot be found again in the same run.
	// However, passives tend to be stronger than use items from the inventory.
	Passive(int x, int y, std::string name, std::string image);

	virtual void apply(Player &p) = 0;
	virtual void unapply(Player &p) = 0;
	bool isPassive() const { return true; };
};

class BatWing : public Passive {
public:
	// +Dex
	BatWing(int x, int y);

	void apply(Player &p);
	void unapply(Player &p);
};

class LifeElixir : public Passive {
public:
	// Max HP increase
	LifeElixir(int x, int y);

	void apply(Player &p);
	void unapply(Player &p);
};

class MagicEssence : public Passive {
public:
	// +Intellect
	MagicEssence(int x, int y);

	void apply(Player &p);
	void unapply(Player &p);
};

class Flying : public Passive {
public:
	// Provides flying
	Flying(int x, int y);

	void apply(Player &p);
	void unapply(Player &p);
};

class SteelPunch : public Passive {
public:
	// Makes all enemies unsturdy
	SteelPunch(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class IronCleats : public Passive {
public:
	// Immunity to spike traps
	IronCleats(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class PoisonTouch : public Passive {
public:
	// Chance to poison on hit
	PoisonTouch(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class FireTouch : public Passive {
public:
	// Chance to ignite on hit
	FireTouch(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class FrostTouch : public Passive {
public:
	// Chance to freeze on hit
	FrostTouch(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class RainbowTouch : public Passive {
public:
	// Grants chance to poison, ignite, or freeze
	RainbowTouch(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class PoisonImmune : public Passive {
public:
	// Immunity to poison
	PoisonImmune(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class FireImmune : public Passive {
public:
	// Immunity to burning
	FireImmune(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class LavaImmune : public Passive {
public:
	// Immunity to lava damage
	LavaImmune(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class BombImmune : public Passive {
public:
	// Immunity to explosion + more damaging bombs
	BombImmune(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class PotionAlchemy : public Passive {
public:
	// More potent potions
	PotionAlchemy(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class SoulSplit : public Passive {
public:
	// Damage is split between hp and gold
	SoulSplit(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class LuckUp : public Passive {
public:
	// Luck increase, gives a greater chance of item drops and better roll save chances
	LuckUp(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class Berserk : public Passive {
public:
	// Low HP = Increased Damage
	Berserk(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class LifeSteal : public Passive {
public:
	// Chance to heal on hit
	LifeSteal(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class Heavy : public Passive {
public:
	// Gives the heavy status which prevents knockback and pushing
	Heavy(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class BrickBreaker : public Passive {
public:
	// Break walls at the cost of hp
	BrickBreaker(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class SummonNPCs : public Passive {
public:
	// Significantly increases NPC spawn rate
	SummonNPCs(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class CheapShops : public Passive {
public:
	// Significantly increases NPC spawn rate
	CheapShops(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class BetterRates : public Passive {
public:
	// Money penalty is reduced on hit/Money bonus is increased on hit
	BetterRates(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class TrapIllumination : public Passive {
public:
	// Illuminates deadly traps
	TrapIllumination(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class ItemIllumination : public Passive {
public:
	// Illuminates items
	ItemIllumination(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class MonsterIllumination : public Passive {
public:
	// Illuminates monsters that are nearby
	MonsterIllumination(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class ResonantSpells : public Passive {
public:
	// Spells have a chance to not be consumed on use
	ResonantSpells(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class FatStacks : public Passive {
public:
	// All drops are now stackable
	FatStacks(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};

class Scavenger : public Passive {
public:
	// All enemies have a small chance to drop items
	Scavenger(int x, int y);
	void apply(Player &p);
	void unapply(Player &p);
};



//		RELICS
class Relic : public Objects {
public:
	// Relics are powerful passives that can only be obtained from the Shrines.
	// They are upgradeable to a maximum level of 4. As before, the player can only hold one of these at a time.
	Relic(int x, int y, std::string name, std::string image);

	bool isTrinket() const { return true; };

	virtual void apply(Dungeon &dungeon, Player &p) = 0;
	virtual void unapply(Dungeon &dungeon, Player &p) = 0;

	virtual void upgrade(Dungeon &dungeon, Player &p) = 0;

	int getLevel() const { return m_level; };
	void setLevel(int level) { m_level = level; };

protected:
	void increaseLevel() { m_level++; };
	void decreaseLevel() { m_level--; };

private:
	int m_level = 1;
};

class CursedStrength : public Relic {
public:
	// Grants immense strength, but at the cost of letting monsters and traps move first.
	// Base: +8 Strength, +Slow attribute
	// +1:   +Armor
	// +2:   +2 Strength
	// +3:   -Slow attribute
	CursedStrength(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);

	void upgrade(Dungeon &dungeon, Player &p);
};

class BrightStar : public Relic {
public:
	// Base: +1 Vision
	// +1:   +15% Max HP, Gold Illumination
	// +2:   +1 Vision
	// +3:   +20 Luck, +5 Favor
	BrightStar(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);

	void upgrade(Dungeon &dungeon, Player &p);
};

class DarkStar : public Relic {
public:
	// Base: -1 Vision, +3 Strength
	// +1:   +2 Dex
	// +2:   -1 Vision, +2 Str, +1 Dex
	// +3:   +1 Vision, Chain lightning; Attacks chain through adjacent enemies
	DarkStar(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);

	void upgrade(Dungeon &dungeon, Player &p);
};

class Riches : public Relic {
public:
	// Base: +3 Money Bonus
	// +1:   +1 Money Multiplier
	// +2:   +1 Money Multiplier
	// +3:   Picking up money has high (100% ?) chance to give player 1 turn of invulnerability
	Riches(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);

	void upgrade(Dungeon &dungeon, Player &p);
};

class MatrixVision : public Relic {
public:
	// Base: +0.05s on level timer
	// +1:   Attacks have chance to Cripple enemies
	// +2:   +0.05s on level timer
	// +3:   Small chance (<= 5%) to skip trap and monster actions for a few turns.
	//       Cannot activate more than once every 15 turns.
	MatrixVision(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);

	void upgrade(Dungeon &dungeon, Player &p);
};

class SuperMagicEssence : public Relic {
public:
	// Base: +3 Intellect
	// +1:   +2 Intellect, all inflicted afflictions have increased durations (that hinder enemies and benefit players)
	// +2:   +1 Intellect, chance to cast random spell when attacked
	// +3:   All afflictions have a chance to affect any enemy, regardless of resistances
	SuperMagicEssence(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);

	void upgrade(Dungeon &dungeon, Player &p);
};

class Protection : public Relic {
public:
	// 
	// Base: +1 Armor, +10% Max HP
	// +1:   +15% Max HP, +5 Luck
	// +2:   +2 Armor; Gain bonus roll to save from afflictions
	// +3:   +10% Max HP; After being hit, small chance for enemies to gain Fragile affliction
	Protection(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);

	void upgrade(Dungeon &dungeon, Player &p);
};


//	BEGIN CHESTS CLASSES
class Chests : public Drops {
public:
	Chests(Dungeon *dungeon, int x, int y, std::string chest, std::string image);
	~Chests();

	bool isChest() const { return true; };

	// Used to check if this can be opened
	virtual void attemptOpen(Dungeon &dungeon) { open(dungeon); };
	virtual void open(Dungeon &dungeon);
	virtual void openEffect(Dungeon &dungeon); // Check if anything happens after opening

protected:
	// Contains the item that the chest will unlock.
	// It's set when the chest is constructed.
	std::shared_ptr<Objects> m_item = nullptr;
};

class TreasureChest : public Chests {
public:
	// Contains currency
	TreasureChest(Dungeon *dungeon, int x, int y);
	void open(Dungeon &dungeon);
};

class LifeChest : public Chests {
public:
	// Life Chests are guaranteed to provide a healing-based item
	LifeChest(Dungeon *dungeon, int x, int y);
};

class BrownChest : public Chests {
public:
	BrownChest(Dungeon *dungeon, int x, int y);
};

class SilverChest : public Chests {
public:
	SilverChest(Dungeon *dungeon, int x, int y);
};

class GoldenChest : public Chests {
public:
	GoldenChest(Dungeon *dungeon, int x, int y);
};

class HauntedChest : public Chests {
public:
	// Unleashes 3 ghosts nearby when opened
	HauntedChest(Dungeon *dungeon, int x, int y);
	void openEffect(Dungeon &dungeon);
};

class TeleportingChest : public Chests {
public:
	// Must be found and opened multiple times
	TeleportingChest(Dungeon *dungeon, int x, int y);
	void attemptOpen(Dungeon &dungeon);

private:
	int m_teleports; // Number of times it will teleport
};

class RiggedChest : public Chests {
public:
	// This class of chests has traps that occur when opened
	RiggedChest(Dungeon *dungeon, int x, int y, std::string name, std::string image, int turns);

protected:
	int m_turns; // Number of turns before setting the trap off
};

class ExplodingChest : public RiggedChest {
public:
	// Places a bomb where the chest was which explodes 1 turn later
	ExplodingChest(Dungeon *dungeon, int x, int y);
	void openEffect(Dungeon &dungeon);
};

class InfinityBox : public Chests {
public:
	InfinityBox(Dungeon *dungeon, int x, int y);
	void attemptOpen(Dungeon &dungeon) { return; };
};



//	BEGIN WEAPON CLASSES

enum class ImbuementType {
	BURNING,
	POISONING,
	FREEZING
};

class Weapon : public Objects {
public:
	Weapon(int x, int y, std::string name, std::string image, int dmg, int dexbonus);

	bool isWeapon() const { return true; };

	virtual void applyBonus(Actors &a) { return; }; // flat stat upgrades while equipped
	virtual void unapplyBonus(Actors &a) { return; };
	virtual void useAbility(Dungeon &dungeon, Actors &a) { return; };
	virtual void usePattern(Dungeon &dungeon, bool &moveUsed) { return; };
	void useImbuement(Dungeon &dungeon, Actors &a); // Enchanter NPC effects

	// Indicates if the weapon grants the player stats
	bool hasBonus() const { return m_hasBonus; };
	void setHasBonus(bool bonus) { m_hasBonus = bonus; };

	// Indicates if the weapon has special effects on hit
	bool hasAbility() const { return m_hasAbility; };
	void setHasAbility(bool ability) { m_hasAbility = ability; };

	// Indicates if the weapon has a non-standard attack pattern
	bool hasAttackPattern() const { return m_hasPattern; };
	void setAttackPattern(bool pattern) { m_hasPattern = pattern; };

	// Indicates if the weapon can be cast (has a wind-up)
	bool canBeCast() const { return m_canBeCast; };
	void setCast(bool cast) { m_canBeCast = cast; };

	// Indicates if this weapon has a charge meter
	virtual bool hasMeter() const { return false; };
	virtual void increaseCharge() { return; };

	// Indicates if the weapon has been imbued with special powers from the Enchanter
	bool isImbued() const { return m_imbued; };
	void setImbuement(ImbuementType imbuement, int chance);
	int getImbuementLevel(ImbuementType imbuement) const; // Returns the maximum of all existing imbuements that this weapon has

	/**
	*  Dual wielding increases the player's damage output via twice the damage of one of the weapons.
	*  When a player dual wields, they cannot use their active item.
	*  If the player attempts to use their active item while dual wielding,
	*  then the player drops one of their weapons at the cost of being able to use it.
	*/
	// Indicates if this weapon has a dual-wield bonus
	virtual bool canDualWield() const { return false; };
	virtual void applyDualWieldBonus() { return; };
	virtual void unapplyDualWieldBonus() { return; };

	int getDexBonus() const { return m_dexbonus; };
	int getDmg() const { return m_dmg; };

	DamageType getDamageType() const { return m_damageType; };

	// Sharpness is how much a weapon has been improved by a Blacksmith
	int getSharpnessBonus(); // Calculates the extra damage that the sharpness provides
	int getSharpness() const { return m_sharpness; };
	void increaseSharpnessBy(int sharp) { m_sharpness += sharp; };
	void decreaseSharpnessBy(int sharp) { 
		m_sharpness -= sharp;
		if (m_sharpness < 0) m_sharpness = 0;
	};

protected:
	void setDamage(int damage) { m_dmg = damage; };
	void setDamageType(DamageType type) { m_damageType = type; };

private:
	bool m_hasBonus = false;
	bool m_hasAbility = false;
	bool m_hasPattern = false;
	bool m_canBeCast = false;

	bool m_imbued = false;

	struct Imbuement {
		bool burn = false;
		int burnChance = 0;

		bool poison = false;
		int poisonChance = 0;

		bool freeze = false;
		int freezeChance = 0;
	};
	Imbuement m_imbuement;
	
	int m_dmg;
	DamageType m_damageType;
	int m_sharpness = 0; // How much a weapon has been improved by a blacksmith

	int m_dexbonus;
};

class Hands : public Weapon {
public:
	// You can't attack with your hands, unfortunately.
	// These become equipped when a weapon breaks or you throw it.
	Hands(int x = 0, int y = 0);

};

//		BASIC WEAPONS
class ShortSword : public Weapon {
public:
	// Standard weapon
	ShortSword(int x = 0, int y = 0);

};
class RadiantShortSword : public Weapon {
public:
	// +1 Vision
	RadiantShortSword(int x, int y);
	void applyBonus(Actors &a);
	void unapplyBonus(Actors &a);
};
class CarbonFiberShortSword : public Weapon {
public:
	// +Dex
	CarbonFiberShortSword(int x, int y);
	void applyBonus(Actors &a);
	void unapplyBonus(Actors &a);
};
class EnchantedShortSword : public Weapon {
public:
	// +Intellect
	EnchantedShortSword(int x, int y);
	void applyBonus(Actors &a);
	void unapplyBonus(Actors &a);
};
class LuckyShortSword : public Weapon {
public:
	// +Luck
	LuckyShortSword(int x, int y);
	void applyBonus(Actors &a);
	void unapplyBonus(Actors &a);
};

class BloodShortSword : public Weapon {
public:
	BloodShortSword(int x, int y);
	void useAbility(Dungeon &dungeon, Actors &a);
};
class GoldenShortSword : public Weapon {
public:
	GoldenShortSword(int x, int y);
	void useAbility(Dungeon &dungeon, Actors &a);
};

class BronzeDagger : public Weapon {
public:
	// High bleed chance
	BronzeDagger(int x, int y);
	void useAbility(Dungeon &dungeon, Actors &a);

private:
	int m_bleedChance = 40;
};

class RustyCutlass : public Weapon {
public:
	// Single-range, counters attacks. Breakable.
	// May make this upgradeable at a special merchant.
	RustyCutlass(int x, int y);
	void usePattern(Dungeon &dungeon, bool &moveUsed);

private:
	int m_durability;
};

class BoneAxe : public Weapon {
public:
	// Can be thrown. Successful throws do extra damage.
	BoneAxe(int x, int y);
	void usePattern(Dungeon &dungeon, bool &moveUsed);

	bool canDualWield() const { return true; };

private:
	bool m_woundUp = false;
};

class Katana : public Weapon {
public:
	// On hit, there is a one turn wait. Afterward, there is a straight strike originating from
	// where the player was standing.
	// Example:
	//
	//  o o       o o       o X
	//  P x  -->  P o  -->  P X
	//  o o       o o       o X
	//
	// 'x' represents the initial strike. 'X' represents the after strike.

	Katana(int x, int y);
	Katana(int x, int y, std::string name, std::string image, int dmg, int dex);
	void usePattern(Dungeon &dungeon, bool &moveUsed);

protected:
	virtual void setCoords(Dungeon &dungeon);
	std::vector<std::pair<int, int>> coords; // For the spaces that the special move will attempt to strike

private:
	bool m_hit = false; // Indicates an attempted strike
	bool m_waitOver = false; // The one turn waiting indicator
};
class GreaterKatana : public Katana {
public:
	// On hit, there is a one turn wait. Afterward, there is a straight strike originating from
	// where the player was standing.
	// Example:
	//
	//  o o       o o       X X
	//  P x  -->  P o  -->  P X
	//  o o       o o       X X
	//
	// 'x' represents the initial strike. 'X' represents the after strike.
	GreaterKatana(int x, int y);
	
protected:
	void setCoords(Dungeon &dungeon);
};
class SuperiorKatana : public Katana {
public:
	// On hit, there is a one turn wait. Afterward, there is a straight strike originating from
	// where the player was standing.
	// Example:
	//
	//  o o       o o       X X X
	//  P x  -->  P o  -->  X P X
	//  o o       o o       X X X
	//
	// 'x' represents the initial strike. 'X' represents the after strike.
	SuperiorKatana(int x, int y);
	
protected:
	void setCoords(Dungeon &dungeon);
};

class VulcanSword : public Weapon {
public:
	VulcanSword(int x, int y);
	void useAbility(Dungeon &dungeon, Actors &a);

private:
	int m_burnChance = 10;
};

//		LONG WEAPONS
class LongWeapon : public Weapon {
public:
	// These weapons have a greater reach of 2 tiles or more, but no piercing.
	LongWeapon(int x, int y, int range, int dmg, int dex, int intellect, std::string name, std::string image);

	void usePattern(Dungeon &dungeon, bool &moveUsed);

	// For weapons with casting
	virtual void checkSpecial(Dungeon &dungeon, bool &moveUsed) { return; };

	int getRange() const { return m_range; };
	virtual bool hasPiercing() const { return false; };

private:
	int m_range;
};

class Pike : public LongWeapon {
public:
	// Standard 2 range weapon.
	Pike(int x, int y);
};
class RadiantPike : public LongWeapon {
public:
	// +1 Vision
	RadiantPike(int x, int y);
	void applyBonus(Actors &a);
	void unapplyBonus(Actors &a);
};
class CarbonFiberPike : public LongWeapon {
public:
	// +Dex
	CarbonFiberPike(int x, int y);
	void applyBonus(Actors &a);
	void unapplyBonus(Actors &a);
};
class EnchantedPike : public LongWeapon {
public:
	// +Intellect
	EnchantedPike(int x, int y);
	void applyBonus(Actors &a);
	void unapplyBonus(Actors &a);
};
class LuckyPike : public LongWeapon {
public:
	// +Luck
	LuckyPike(int x, int y);
	void applyBonus(Actors &a);
	void unapplyBonus(Actors &a);
};


//		PIERCING WEAPONS
class PiercingWeapon : public LongWeapon {
public:
	// These weapons have a range of 2 or more and can hit multiple enemies
	// in a straight line.
	PiercingWeapon(int x, int y, int range, int dmg, int dex, int intellect, std::string name, std::string image);

	bool hasPiercing() const { return true; };

private:
	
};

class GoldenLongSword : public PiercingWeapon {
public:
	// More gold = more damage
	GoldenLongSword(int x, int y);
	void useAbility(Dungeon &dungeon, Actors &a);
};
class IronLongSword : public PiercingWeapon {
public:
	// Slightly higher damage
	IronLongSword(int x, int y);
};


//		BOWS
class Bow : public Weapon {
public:
	// Bows have a range of 4 and hit the first enemy it strikes.
	// They do reduced damage if the player is adjacent to the enemy.
	Bow(int x, int y, std::string name, std::string image, int damage, int dexbonus, int range);

	void usePattern(Dungeon &dungeon, bool &moveUsed);
	void checkSpecial(Dungeon &dungeon, bool &moveUsed);

	int getRange() const { return m_range; };

private:
	int m_range;
	bool m_woundUp = false;
};
class WoodBow : public Bow {
public:
	WoodBow(int x, int y);
};
class IronBow : public Bow {
public:
	IronBow(int x, int y);
};
class VulcanBow : public Bow {
public:
	VulcanBow(int x, int y);

	void useAbility(Dungeon &dungeon, Actors &a);

private:
	int m_burnChance = 10;
};


//		SPECIALTY WEAPONS
class Estoc : public Weapon {
public:
	// The Estoc lets the player leap forward up to two spaces away and deal damage.
	// The farther they leap, the more damage they deal.
	// Also pierces up to one extra enemy when lunging.
	// Allows the player to jump over some traps if they lunge from two tiles away.
	Estoc(int x, int y);
	Estoc(int x, int y, std::string name, std::string image, int dmg, int dex);

	void usePattern(Dungeon &dungeon, bool &moveUsed);
	virtual bool canPierce() const { return false; }; // Greater + Superior
	virtual bool isSuperior() const { return false; }; // Superior

protected:
	int m_limit = 3; // The range of the Estoc

private:
	bool m_woundUp = false; // One turn wind-up before launching
};
class GreaterEstoc : public Estoc {
public:
	// Pierces up to one extra enemy when lunging.
	GreaterEstoc(int x, int y);
	bool canPierce() const { return true; };
	
};
class SuperiorEstoc : public Estoc {
public:
	// Stuns enemies + Greater Estoc effect
	SuperiorEstoc(int x, int y);
	bool canPierce() const { return true; };
	bool isSuperior() const { return true; };
};

class Zweihander : public Weapon {
public:
	// This attacks normally in a 3-tile pattern across.
	Zweihander(int x, int y);
	Zweihander(int x, int y, std::string name, std::string image, int dmg, int dex);

	void usePattern(Dungeon &dungeon, bool &moveUsed);
	virtual void checkSpecial(Dungeon &dungeon, bool &moveUsed) { return; };
	virtual bool isSuperior() const { return false; };
};
class GreaterZweihander : public Zweihander {
public:
	// Has a priming stab that hits one enemy in front of you up to two tiles away.
	GreaterZweihander(int x, int y);
	
	void checkSpecial(Dungeon &dungeon, bool &moveUsed);

private:
	bool m_woundUp = false;
};
class SuperiorZweihander : public Zweihander {
public:
	// The priming ability now does not need to be charged up
	SuperiorZweihander(int x, int y);
	bool isSuperior() const { return true; };
};

class Claw : public Weapon {
public:
	// You have become scorpion. Latches onto enemies and pulls them in.
	// Very high chance to induce bleeding when in melee range.
	Claw(int x, int y);
	void usePattern(Dungeon &dungeon, bool &moveUsed);
	void useAbility(Dungeon &dungeon, Actors &a);

private:
	int m_bleedChance = 50;
	bool m_woundUp = false;
};

class Hammer : public Weapon {
public:
	// Special stuns enemies for 1 turn and does bonus damage.
	Hammer(int x, int y);
	Hammer(int x, int y, std::string name, std::string image, int dmg, int dex);

	void usePattern(Dungeon &dungeon, bool &moveUsed);
	void TPattern(Dungeon &dungeon);
	void boxPattern(Dungeon &dungeon);
	virtual bool TStun() const { return false; };
	virtual bool boxStun() const { return false; };

protected:
	bool m_woundUp = false;
};
class GreaterHammer : public Hammer {
public:
	// Stuns in a T formation on impact. Smaller damage boost on adjacent enemies.
	GreaterHammer(int x, int y);
	bool TStun() const { return true; };
};
class SuperiorHammer : public Hammer {
public:
	// Stuns in a 2x3 box. Smaller damage boost on adjacent enemies.
	SuperiorHammer(int x, int y);
	bool boxStun() const { return true; };
};
class VulcanHammer : public Hammer {
public:
	// Legendary item. Should be rare to find.
	VulcanHammer(int x, int y);
	void useAbility(Dungeon &dungeon, Actors &a);

private:
	int m_burnChance = 10;
};

class ArcaneStaff : public Weapon {
public:
	ArcaneStaff(int x, int y);
	void giveBonus(Actors &a);
	void unapplyBonus(Actors &a);
	void usePattern(Dungeon &dungeon, bool &moveUsed);

private:
	bool m_bonusApplied = false;
	bool m_isCast = false; // for telling if staff's ability was cast

	// tells the player's past position, cast is not reset until player moves
	int m_px;
	int m_py;
};

class BoStaff : public Weapon {
public:
	// The Bo Staff has single range but always knocks the enemy back if possible.
	// On successful hit, it also hits any enemies directly behind the player and
	// attempts to knock them backward as well.
	BoStaff(int x, int y);
	BoStaff(int x, int y, std::string name, std::string image, int dmg, int dex);

	void usePattern(Dungeon &dungeon, bool &moveUsed);
	virtual bool canBackHit() const { return false; }; // Used in Greater/Superior Bo Staff
	virtual bool canPush() const { return false; }; // Superior only
};
class GreaterBoStaff : public BoStaff {
public:
	// Walking away from an enemy behind you knocks them back, regardless on hit or not.
	GreaterBoStaff(int x, int y);
	
	bool canBackHit() const { return true; };
	bool canPush() const { return false; };
};
class SuperiorBoStaff : public BoStaff  {
public:
	// Pushes back -all- enemies by one space in the front and rear, if possible + GreaterBoStaff effect.
	SuperiorBoStaff(int x, int y);
	
	bool canBackHit() const { return true; };
	bool canPush() const { return true; };
};

class Nunchuks : public Weapon {
public:
	// Nunchuks are single range weapons that can also stun enemies beside the player.
	// Enemies struck on the side are not damaged.
	// If there is no enemy in front of the player, the player is allowed to move
	// forward while simultaneously stunning any enemies that were next to them.
	// Does not do any stun on hit.
	Nunchuks(int x, int y);
	Nunchuks(int x, int y, std::string name, std::string image, int dmg, int dex);

	void usePattern(Dungeon &dungeon, bool &moveUsed);
	virtual bool stationaryStun() const { return false; };
	virtual bool moreStun() const { return false; };
};
class GreaterNunchuks : public Nunchuks {
public:
	// Successful hits can also stun enemies on the side.
	GreaterNunchuks(int x, int y);
	bool stationaryStun() const { return true; };
	bool moreStun() const { return false; };
};
class SuperiorNunchuks : public Nunchuks {
public:
	// Stuns enemies next to the space adjacent to where the player is going to move.
	SuperiorNunchuks(int x, int y);
	bool stationaryStun() const { return true; };
	bool moreStun() const { return true; };
};

class Mattock : public Weapon {
public:
	// Yes, it's a mattock. It can dig out walls but is susceptible to breaking.
	// The number of wall swings is variable but only hitting walls affects its durability.
	Mattock(int x, int y);
	void usePattern(Dungeon &dungeon, bool &moveUsed);

private:
	int m_durability;
};

class MattockHead : public Weapon {
public:
	// The broken head of the Mattock. Throwable.
	// Also breaks. Turns into Mattock Dust item when it breaks.
	MattockHead(int x, int y);
	void usePattern(Dungeon &dungeon, bool &moveUsed);

private:
	int m_durability;
	bool m_woundUp = false;
};

class Pistol : public Weapon {
public:
	// Can only hold two rounds at a time. Both rounds must be shot before being reloaded.
	// Must reload both rounds to begin shooting again. Infinite range.
	Pistol(int x, int y);
	void usePattern(Dungeon &dungeon, bool &moveUsed);

private:
	bool m_reloading = false; // To indicate that rounds are being reloaded
	int m_rounds = 2; // Number of rounds in the chamber
};

class Whip : public Weapon {
public:
	// Has three range, can knock enemies back one tile if hit at the tip.
	Whip(int x, int y);
	void usePattern(Dungeon &dungeon, bool &moveUsed);
};

class MeterWeapon : public Weapon {
public:
	// Metered weapons must have enough charge to
	// use its special ability.
	MeterWeapon(int x, int y, int charge, int dmg, int dex, std::string name, std::string image);

	bool hasMeter() const { return true; };

	int getCharge() const { return m_charge; };
	int getMaxCharge() const { return m_maxCharge; };

	void increaseCharge();
	void resetCharge() { m_charge = 0; };

private:
	int m_charge;
	int m_maxCharge;
};

class Jian : public MeterWeapon {
public:
	// The Jian (Chinese double-edged sword) hits up to three enemies in front of you normally.
	// It has a special ability that must be charged up which sweeps in a ring around the player.
	Jian(int x, int y);
	Jian(int x, int y, std::string name, std::string image, int charge, int dmg, int dex);
	void usePattern(Dungeon &dungeon, bool &moveUsed);

	//virtual bool canBleed() const { return false; };
	virtual bool hasBonusDamage() const { return false; }; // Bonus damage for high enough hp
	virtual bool hasKnockback() const { return false; }; // Hits produce knockback

};
class GreaterJian : public Jian {
public:
	// The special does bonus damage if the player has enough hp (>= 90%).
	GreaterJian(int x, int y);
	
	bool hasBonusDamage() const { return true; };
	bool hasKnockback() const { return false; };
};
class SuperiorJian : public Jian {
public:
	// The special ability knocks all enemies backward and stuns for one turn (+ GreaterJian's effect).
	SuperiorJian(int x, int y);
	
	bool hasBonusDamage() const { return true; };
	bool hasKnockback() const { return true; };
};

class Boomerang : public MeterWeapon {
public:
	// Can be thrown in an elliptical pattern that stuns enemies it hits.
	// At the apex of the throw, can open chests or collect items and place them where the player is.
	Boomerang(int x, int y);
	void usePattern(Dungeon &dungeon, bool &moveUsed);
};



//  BEGIN SHIELD CLASSES
class Shield : public Objects {
public:
	Shield(int x, int y, int defense, int durability, int coverage, std::string type, std::string image);

	bool isShield() const { return true; };

	virtual void useAbility(Dungeon &dungeon, Actors &a) { ; };

	int getDefense() const { return m_defense; };
	void setDefense(int defense) { m_defense = defense; };
	int getDurability() const { return m_durability; };
	void setDurability(int durability) { m_durability = durability; };
	int getMaxDurability() const { return m_max_durability; };
	void setMaxDurability(int durability) { m_max_durability = durability; };
	int getCoverage() const { return m_coverage; };
	void setCoverage(int coverage) { m_coverage = coverage; };
	bool hasAbility() const { return m_hasAbility; };
	void setAbility(bool ability) { m_hasAbility = ability; };

private:
	// provides x amount of armor/protection against attacks when used
	int m_defense;

	// durability is a stat that tells how long if a shield breaks,
	// if it reaches zero then it breaks.
	// A value of -1 means it is indestructible and cannot break.
	int m_durability;
	int m_max_durability;

	// coverage is how much area the shield can block
	// Example: 'p' is player, 'm' is monster, 'o' means protected, '_' means not protected
	//		_ _ _ _
	//		_ p o m
	//		_ _ _ _
	//
	// a higher coverage stat allows the shield to block from more directions
	// Key: 1-front only, 2-front and front diagonals, 3-front and sides, 4-full coverage
	int m_coverage;

	// hasAbility tells if the shield has some extra abilty
	bool m_hasAbility = false;
};

class WoodShield : public Shield {
public:
	WoodShield(int x, int y);
};

class IronShield : public Shield {
public:
	IronShield(int x, int y);
};

class ThornedShield : public Shield {
public:
	ThornedShield(int x, int y);

	void useAbility(Dungeon &dungeon, Actors &a);
};

class FrostShield : public Shield {
public:
	FrostShield(int x, int y);

	void useAbility(Dungeon &dungeon, Actors &a);
};

class ReflectShield : public Shield {
public:
	ReflectShield(int x, int y);

	void useAbility(Dungeon &dungeon, Actors &a);
};


//	BEGIN TRAPS
class Traps : public Objects {
public:
	Traps(Dungeon *dungeon, int x, int y, std::string name, std::string image, int damage);

	virtual void moveTo(int x, int y, float time = 0.1f);

	// trap actions for when actives is being checked
	virtual void activeTrapAction(Actors &a) { return; };

	// trap actions for when a player/monster steps on or interacts with a trap
	virtual void trapAction(Actors &a) { return; };

	// Occurs when the trap is destroyed
	virtual void destroyTrap();
	virtual void drops() { return; };
	virtual void spriteCleanup();

	int getDmg() const { return m_trapdmg; };
	void setDmg(int damage) { m_trapdmg = damage; };

	// tells if the trap's status constantly needs to be checked
	bool isActive() const { return m_active; };
	void setActive(bool active) { m_active = active; };

	// tells if the trap also acts as a wall
	bool actsAsWall() const { return m_wall; };
	void setWallFlag(bool wall) { m_wall = wall; };

	// tells if trap immediately damages
	bool isLethal() const { return m_lethal; };
	void setLethal(bool lethal) { m_lethal = lethal; };

	// Indicates if this can be set off by explosions
	bool isExplosive() const { return m_explosive; };
	void setExplosive(bool explosive) { m_explosive = explosive; };
	virtual void explode() { return; };
	void checkExplosion(int x, int y);

	// Indicates if this can be ignited/lit on fire
	bool canBeIgnited() const { return m_canBeIgnited; };
	void setCanBeIgnited(bool ignite) { m_canBeIgnited = ignite; };
	virtual void ignite() { return; }; // Used to do whatever happens if this trap is ignited
	void checkBurn(int x, int y);
	virtual bool isLightSource() const { return false; }; // For removing the light source if it is destroyed

	// Some traps are affected if poison is present
	bool canBePoisoned() const { return m_canBePoisoned; };
	void setCanBePoisoned(bool flag) { m_canBePoisoned = flag; };
	virtual void poison() { return; };
	void checkPoison(int x, int y);

	// Some traps can be doused by water
	bool canBeDoused() const { return m_canBeDoused; };
	void setCanBeDoused(bool flag) { m_canBeDoused = flag; };
	virtual void douse() { return; };
	void checkDouse(int x, int y);

	// Some can be frozen
	bool canBeFrozen() const { return m_canBeFrozen; };
	void setCanBeFrozen(bool flag) { m_canBeFrozen = flag; };
	virtual void freeze() { return; };
	void checkFreeze(int x, int y);

	// Checks if the trap could be destroyed manually
	//virtual void checkDestroy(int x, int y);

	// tells if the trap was destroyed or exhausted in some way
	bool isDestroyed() const { return m_destroyed; };
	void setDestroyed(bool destroyed) { m_destroyed = destroyed; };

	// Indicates if this was a decoy trap
	virtual bool isDecoy() const { return false; }

	// Used for Player's Trap Illumination passive
	virtual bool canBeIlluminated() const { return false; };

private:
	int m_trapdmg;
	bool m_destroyed = false;
	bool m_active;
	bool m_wall = false;
	bool m_lethal = false;

	bool m_explosive = false;
	bool m_canBeIgnited = false;
	bool m_canBePoisoned = false;
	bool m_canBeDoused = false;
	bool m_canBeFrozen = false;
};

class Stairs : public Traps {
public:
	Stairs(Dungeon &dungeon, int x, int y, bool locked = false);

	bool canBeIlluminated() const { return true; };

	void interact();

	bool isLocked() const { return m_locked; };
	void toggleLock() { m_locked = !m_locked; };

private:
	bool m_locked;
};

class Button : public Traps {
public:
	Button(Dungeon &dungeon, int x, int y);
};

class DevilsWater : public Traps {
public:
	DevilsWater(Dungeon *dungeon, int x, int y);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);

private:
	bool m_firstUse = true;
	int m_x;
	int m_y;
};

class Brazier : public Traps {
public:
	// These can be lit to provide more light within the dungeon.
	// Braziers can be tipped over to leave a 3x3 grid of Embers
	Brazier(Dungeon &dungeon, int x, int y);
	~Brazier();

	void trapAction(Actors &a);
	void ignite();

	bool isLightSource() const { return true; };

private:
	cocos2d::Sprite* m_flame = nullptr;
	bool m_lit = false;
	bool m_tipped = false; // Indicates that it was tipped over after being lit
};

class Pit : public Traps {
public:
	// Pits kill all non-flying enemies instantly
	Pit(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
};

class FallingSpike : public Traps {
public:
	FallingSpike(Dungeon &dungeon, int x, int y, int speed);
	~FallingSpike();

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a); // used if a player steps in the way

	int getSpeed() const { return m_speed; };
	void setSpeed(int speed) { m_speed = speed; };

private:
	int m_speed;
};

class Spikes : public Traps {
public:
	Spikes(Dungeon &dungeon, int x, int y);

	//void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
};

class SpikeTrap : public Traps {
public:
	SpikeTrap(Dungeon &dungeon, int x, int y, int speed);
	~SpikeTrap();

	void activeTrapAction(Actors &a); // called if player was standing on top of one that becomes active
	void trapAction(Actors &a); // used if player steps on one already active
	
	void setSpriteVisibility(bool deactive, bool primed, bool active);

	int getSpeed() const { return m_cyclespeed; };
	void setSpeed(int speed) { m_cyclespeed = speed; };
	int getCountdown() const { return m_countdown; };
	void setCountdown(int count) { m_countdown = count; };

private:
	int m_cyclespeed;
	int m_countdown;

	cocos2d::Sprite* m_deactive;
	cocos2d::Sprite* m_primed;
	cocos2d::Sprite* m_active;
};

class TriggerSpike : public Traps {
public:
	TriggerSpike(Dungeon &dungeon, int x, int y);
	~TriggerSpike();

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
	
	void setSpriteVisibility(bool deactive, bool primed, bool active);

	bool isTriggered() const { return m_triggered; };
	void toggleTrigger() { m_triggered = !m_triggered; };

private:
	bool m_triggered;

	cocos2d::Sprite* m_deactive;
	cocos2d::Sprite* m_primed;
	cocos2d::Sprite* m_active;
};

class Puddle : public Traps {
public:
	Puddle(Dungeon &dungeon, int x, int y, int turns = -1);
	Puddle(Dungeon &dungeon, int x, int y, int turns, std::string name, std::string image);
	
	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
	virtual void specialAction(Actors &a) { return; };
	void freeze();

	void spriteCleanup();

private:
	int m_turns; // if this value is -1, then the puddle does not dissipate over time
};

class PoisonPuddle : public Puddle {
public:
	PoisonPuddle(Dungeon &dungeon, int x, int y, int turns = -1);

	void specialAction(Actors &a);
};

class FrozenPuddle : public Traps {
public:
	FrozenPuddle(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
	void ignite();

private:
	int m_turns = 20;
};

class Water : public Traps {
public:
	Water(Dungeon &dungeon, int x, int y);
	Water(Dungeon &dungeon, int x, int y, std::string name, std::string image);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
	void poison();
	void freeze();

protected:
	virtual void specialActiveAction() { return; };
	virtual void specialAction(Actors &a) { return; };

private:
	bool m_firstUse = true;
	int m_x;
	int m_y;
};

class PoisonWater : public Water {
public:
	PoisonWater(Dungeon &dungeon, int x, int y);

protected:
	void specialActiveAction();
	void specialAction(Actors &a);

private:
	bool m_wait = true; // For poison spread among traps
};

class Ice : public Traps {
public:
	Ice(Dungeon &dungeon, int x, int y);

	void trapAction(Actors &a);
	void ignite();
};

class PoisonMister : public Traps {
public:
	// 
	PoisonMister(Dungeon &dungeon, int x, int y, int wait = 15);

	void activeTrapAction(Actors &a);

private:
	int m_wait;
	int m_maxWait;
};

class Stalactite : public Traps {
public:
	// 
	Stalactite(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);

private:
	int m_stalactiteY;
	bool m_activated = false;
	int m_wait = 2; // Turns left until it hits the ground
};

class GiantCrystal : public Traps {
public:
	// 
	GiantCrystal(Dungeon &dungeon, int x, int y);

	void drops();
	void trapAction(Actors &a);

private:
	int m_strength = 3; // How many hits until it breaks
	int m_strengthCutoff = 5; // Player must have this at least this much strength to damage the crystal
};

class BrokenCrystals : public Traps {
public:
	//
	BrokenCrystals(Dungeon &dungeon, int x, int y);

	void trapAction(Actors &a);
};

class Tree : public Traps {
public:
	//
	Tree(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
	void ignite();

	void spriteCleanup();

private:
	void createRoots();

	std::vector<std::pair<int, int>> m_roots;
	std::vector<cocos2d::Sprite*> m_flames;
	bool m_lit = false;
	int m_litTurns = 40;
};

class TreeRoot : public Traps {
public:
	// 
	TreeRoot(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
	void ignite();

	void spriteCleanup();

private:
	cocos2d::Sprite* m_flame = nullptr;
	bool m_lit = false;
	int m_litTurns = 8;
};

class MalevolentPlant : public Traps {
public:
	MalevolentPlant(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
	void ignite();

private:
	bool m_primed = false;
};

class Firebar : public Traps {
public:
	Firebar(Dungeon &dungeon, int x, int y);
	Firebar(Dungeon &dungeon, int x, int y, std::string firebar); // for double firebar

	bool canBeIlluminated() const { return true; };

	void activeTrapAction(Actors &a);

	virtual void setInitialFirePosition(int x, int y);
	int getAngle() const { return m_angle; };
	void setAngle(int angle) { m_angle = angle; };
	bool isClockwise() const { return m_clockwise; };

	virtual bool playerWasHit(const Actors &a) const;
	virtual void setFirePosition(char move);

	void setSpriteColor(cocos2d::Color3B color);
	virtual void setSpriteVisibility(bool visible);

	void spriteCleanup();

private:
	int m_angle;
	bool m_clockwise;

	// these are helper objects to keep track of where the firebar's fireball is
	std::shared_ptr<Objects> m_innerFire;
	std::shared_ptr<Objects> m_outerFire;
};

class DoubleFirebar : public Firebar {
public:
	DoubleFirebar(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);

	void setInitialFirePosition(int x, int y);
	bool playerWasHit(const Actors &a) const;
	void setFirePosition(char move);

	void setSpriteColor(cocos2d::Color3B color);
	void setSpriteVisibility(bool visible);

	void spriteCleanup();

private:
	std::shared_ptr<Objects> m_innerFire;
	std::shared_ptr<Objects> m_innerFireMirror;
	std::shared_ptr<Objects> m_outerFire;
	std::shared_ptr<Objects> m_outerFireMirror;
};

class Lava : public Traps {
public:
	Lava(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
};

class MagmaTide : public Traps {
public:
	// Magma Tides are pools of lava that expand and contract
	// Pattern: Single -> Ring -> Single -> Ring -> Double Ring -> Ring -> Repeat
	MagmaTide(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);

private:
	int m_wait;
	int m_maxWait;

	// Stage cycles through from 1 to 7, then repeats
	int m_stage = 1;
	bool m_expanded = false;
};

class MoltenPillar : public Traps {
public:
	// Provides light that pulsates slowly
	MoltenPillar(Dungeon &dungeon, int x, int y);

	bool isLightSource() const { return true; };

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
	void drops();

private:
	// Light pattern: 3 -> 4 -> Repeat
	int m_stage;

	int m_wait = 3;
	int m_maxWait;

	int m_strength = 1; // How many hits until it breaks
	int m_strengthCutoff = 2; // Player must have this at least this much strength to damage it
};

class Spring : public Traps {
public:
	/* Spring traps take one turn to trigger before launching the player in a direction
	*  They will typically point in one direction, but there is a type that will launch in an unknown direction.
	*/

	// This is the standard spring trap; it points in a single random direction if @move is not specified
	// @trigger specifies if it needs to be triggered or if it springs immediately
	Spring(Dungeon &dungeon, int x, int y, bool trigger = true, char move = '-');


	/* This constructor lets you explicitly specify that the spring direction can or cannot be @known
	*  It will also let you specify if it will only allow cardinal directionality with @cardinal
	*  Boolean table:
	*  known | cardinal |        RESULT
	*    0   |    0     | points in all directions (unknown)
	*    0   |    1     | points in all directions (unknown)
	*    1   |    0     | points in only cardinal directions
	*    1   |    1     | points in only diagonal directions

	*/
	Spring(Dungeon &dungeon, int x, int y, bool trigger, bool known, bool cardinal);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);

	char getDirection() const { return m_dir; };
	void setDirection(char dir) { m_dir = dir; };
	bool isTrigger() const { return m_isTrigger; };
	bool triggered() const { return m_triggered; };
	void setImage();

	// tells if the trap bounces in multiple directions
	bool isMultiDirectional() const { return m_multidirectional; };

	// tells if the trap bounces in all directions
	bool isAny() const { return m_any; };

	// if multidirectional, tells if bounces only in cardinal directions
	bool isCardinal() const { return m_cardinal; };

	// tells if two spring traps are pointing toward each other
	// for preventing infinite bouncing loops
	bool isOpposite(const Spring &other) const;

	void oppositeSprings();

private:
	char m_dir; // used if spring is a single direction

	bool m_isTrigger;
	bool m_triggered;
	bool m_multidirectional; // specifies if single or multi-directional
	bool m_any; // specifies if the spring points in all directions
	bool m_cardinal; // specifies if the spring is multidirectional but only in cardinal directions
};

class Turret : public Traps {
public:
	/* 
	*  Turrets, like archers, must be set off before shooting. Turrets are stationary, have a limited detection range, but infinite shooting range.
	*  They also only face in one direction at a time, whereas archers are able to shoot in any of the 4 cardinal directions.
	*/
	Turret(Dungeon &dungeon, int x, int y, char move, int range = 8);

	bool canBeIlluminated() const { return true; };

	void activeTrapAction(Actors &a);
	void checkLineOfFire();

	char getDirection() const { return m_dir; };
	void setDirection(char dir) { m_dir = dir; };
	int getRange() const { return m_range; };
	void setRange(int range) { m_range = range; };
	bool isTriggered() const { return m_triggered; };
	void setTrigger(bool trigger) { m_triggered = trigger; };
	bool onCooldown() const { return m_cooldown; };
	void setCooldown(bool cooldown) { m_cooldown = cooldown; };

private:
	char m_dir;
	int m_range;
	bool m_triggered;
	bool m_cooldown = false;
};

class MovingBlock : public Traps {
public:
	MovingBlock(Dungeon &dungeon, int x, int y, char pattern, int spaces = 3);

	void activeTrapAction(Actors &a);

	char getPattern() const { return m_pattern; };
	void setPattern(char pattern) { m_pattern = pattern; };
	int getSpaces() const { return m_spaces; };
	int getCounter() const { return m_counter; };
	void setCounter(int count) { m_counter = count; };
	void resetCounter() { m_counter = m_spaces; };
	void flip() { m_dir = -m_dir; };
	bool turn() const { return m_turn; };
	void toggleTurn() { m_turn = !m_turn; };

private:
	/* m_pattern tells the way in which the block moves
	*  Key:
	*  h : side to side
	*  v : up and down
	*  s : in a 2x2 square
	*  S : in a 3x3 square
	*/
	char m_pattern;

	/* m_spaces specifies how many tiles the block moves if it moves in a linear path (min: 2) 
	*
	*/
	int m_spaces;

	// value that counts how many spaces are left to travel
	int m_counter;

	// can be only 1 or -1, tells the direction to count in
	int m_dir;

	// m_turn tells it to move in the opposite direction if spaces reaches zero
	bool m_turn = false;
};

class ActiveBomb : public Traps {
public:
	ActiveBomb(Dungeon &dungeon, int x, int y, int timer = 3);
	ActiveBomb(Dungeon &dungeon, int x, int y, std::string type, std::string image, int damage, int timer = 3);

	void activeTrapAction(Actors &a);
	virtual void explosion();

	int getTimer() const { return m_timer; };
	void setTimer(int timer) { m_timer = timer; };

protected:
	void setRange(int range) { m_range = range; };

	int m_fuseID; // for getting and stopping fuse sound when bomb explodes

private:
	int m_timer;
	int m_range;
};

class ActiveMegaBomb : public ActiveBomb {
public:
	ActiveMegaBomb(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);
};

class ActivePoisonBomb : public Traps {
public:
	ActivePoisonBomb(Dungeon &dungeon, int x, int y);

	void trapAction(Actors &a);
	void explosion();
	
	void drops();

	void explode();

private:
	int m_fuseID;
	bool m_set = false; // One turn before becoming active
};

class PoisonCloud : public Traps {
public:
	PoisonCloud(Dungeon &dungeon, int x, int y, int turns);

	void activeTrapAction(Actors &a);

private:
	int m_turns;
};

class SetBearTrap : public Traps {
public:
	// The trap laid by the Bear Trap item
	SetBearTrap(Dungeon &dungeon, int x, int y);

	void trapAction(Actors &a);

private:
	bool m_set = false; // One turn before becoming active
};

class CrumbleFloor : public Traps {
public:
	CrumbleFloor(Dungeon &dungeon, int x, int y, int strength = 4);
	CrumbleFloor(Dungeon &dungeon, int x, int y, int strength, std::string name, std::string image);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);

	virtual void crumble();

private:
	int m_strength; // how many turns before the floor crumbles away
	bool m_crossed = false; // flag for when player walks off

	Actors* m_triggerer = nullptr; // keep track of what walked over it
	int m_x, m_y; // saves coordinates of whatever walked on it
};

class CrumbleLava : public CrumbleFloor {
public:
	CrumbleLava(Dungeon &dungeon, int x, int y, int strength = 4);

	void crumble();
};

class Ember : public Traps {
public:
	// Temporary embers that subside after a certain amount of time
	Ember(Dungeon &dungeon, int x, int y, int turns = 5);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);

	void douse();

	bool isLightSource() const { return true; };

	void spriteCleanup();

private:
	int m_turns;

};

class Web : public Traps {
public:
	Web(Dungeon &dungeon, int x, int y, int stickiness = 1);

	void trapAction(Actors &a);
	void ignite();

private:
	int m_stickiness; // number of turns to stun the actor
};

class WindTunnel : public Traps {
public: 
	// Acts like a giant fan that pushes all actors backwards one tile at a time.
	// Has a large range.
	WindTunnel(Dungeon &dungeon, int x, int y, char dir);
	void activeTrapAction(Actors &a);

private:
	char m_dir;
};

class Quicksand : public Traps {
public:
	// Kills actors if they stand in this trap too long.
	// If players struggle by mashing, they will be killed faster.
	// To escape, you must continuously press the direction opposite to the direction you're currently facing.
	Quicksand(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);

private:
	std::shared_ptr<Actors> m_capturedActor = nullptr;
	char m_pDir = '-';
	int m_depth;
	int m_maxDepth;
};

class Cactus : public Traps {
public:
	Cactus(Dungeon &dungeon, int x, int y);

	void trapAction(Actors &a);
	void ignite();
	void drops();
};

class MovingTile : public Traps {
public:
	MovingTile(Dungeon &dungeon, int x, int y, int spaces);

	void moveTo(int x, int y, float time = 0.1f);
	virtual void setDirection() = 0;
	void activeTrapAction(Actors &a);

protected:
	// Specifies the maximum of how many tiles it moves in a straight line
	int m_maxSpaces;

	// Counts how many spaces are left to travel
	int m_spaces;

	char m_dir;
};

class LinearMovingTile : public MovingTile {
public:
	LinearMovingTile(Dungeon &dungeon, int x, int y, char dir, int spaces);

	void setDirection();
};

class SquareMovingTile : public MovingTile {
public:
	SquareMovingTile(Dungeon &dungeon, int x, int y, char dir, int spaces, bool clockwise);

	void setDirection();
	//void activeTrapAction(Actors &a);

private:
	bool m_clockwise;
};

class LavaGrating : public Traps {
public:
	// These traps have lava underneath them and sink further down the more bodies there
	// are on top of it.
	// Currently a fixed 5x5 area
	LavaGrating(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);

private:
	cocos2d::Sprite* m_lava = nullptr;
};

class LightAbsorber : public Traps {
public:
	// Reduces the player's vision radius when they are near
	LightAbsorber(Dungeon &dungeon, int x, int y);
	void activeTrapAction(Actors &a);

private:
	bool m_activated = false;
	int m_lightReductionLevel = 1;
};

class WatcherStatue : public Traps {
public:
	WatcherStatue(Dungeon &dungeon, int x, int y, std::string image, char dir);

	void activeTrapAction(Actors &a);
	virtual void react() = 0;

protected:
	bool m_primed = false;
	char m_dir;

private:
	int m_cooldown = 0;
};

class FireballWatcher : public WatcherStatue {
public:
	FireballWatcher(Dungeon &dungeon, int x, int y, char dir);

	void react();
};

class FreezeWatcher : public WatcherStatue {
public:
	FreezeWatcher(Dungeon &dungeon, int x, int y, char dir);

	void react();
};

class DartWatcher : public WatcherStatue {
public:
	DartWatcher(Dungeon &dungeon, int x, int y, char dir);

	void react();
};

class CrumbleWatcher : public WatcherStatue {
public:
	CrumbleWatcher(Dungeon &dungeon, int x, int y, char dir);

	void react();
};

class SpawnWatcher : public WatcherStatue {
public:
	SpawnWatcher(Dungeon &dungeon, int x, int y, char dir);

	void react();
};

class GuardianWatcher : public WatcherStatue {
public:
	GuardianWatcher(Dungeon &dungeon, int x, int y, char dir);

	void react();
};

class Decoy : public Traps {
public:
	// Decoys attract monsters away from the player
	Decoy(Dungeon &dungeon, int x, int y, int range, int turns, std::string name, std::string image);

	bool isDecoy() const { return true; };

	int getAttractRange() const { return m_attractRange; };
	int getTurns() const { return m_turns; };
	void decTurns() { m_turns--; };

private:
	int m_attractRange;
	int m_turns; // How many times this decoy can be used before being exhausted
};

class RottingDecoy : public Decoy {
public:
	// The trap laid when RottenMeat is used.
	// Attracts monsters and poisons them if they eat (touch) it.
	RottingDecoy(Dungeon &dungeon, int x, int y, int bites = 20);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
};

// From enemies
class EnergyHelix : public Traps {
public:
	// Launched by Wisp.
	// @dir is the primary/initial direction to move in.
	// @dir2 is the one it alternates between.
	// This distinction is needed for diagionally moving helixes.
	EnergyHelix(Dungeon &dungeon, int x, int y, char dir, char dir2);

	bool isLightSource() const { return true; };

	void moveTo(int x, int y, float time = 0.1f);
	void activeTrapAction(Actors &a);

	void spriteCleanup();

private:
	char m_currentDir;
	char m_dir;
	char m_secondaryDir;
	int m_spaces = 1;
	int m_maxSpaces;
};

class AbyssalMaw : public Traps {
public:
	// Released by AbyssSummoner.
	AbyssalMaw(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);

private:
	int m_wait = 2;
};

class Goop : public Traps {
public:
	// Created by CrawlingSpine.
	Goop(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);
	void trapAction(Actors &a);
};

class MiniEruption : public Traps {
public:
	// Released by CombustionGolem
	MiniEruption(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);

private:
	void addCoords();
	std::vector<std::pair<int, int>> m_coords;
	int m_radius = 1;

	int m_maxTurns = 3;
	int m_turns = -1;
};

class Combustion : public Traps {
public:
	// Released by CombustionGolem on death.
	Combustion(Dungeon &dungeon, int x, int y);

	void activeTrapAction(Actors &a);

private:
	int m_countdown = 2;
};

class DirtMound : public Traps {
public:
	// Created by ConstructorDemon
	DirtMound(Dungeon &dungeon, int x, int y);

	void trapAction(Actors &a);
};

// Traps from spells
class FirePillars : public Traps {
public:
	// Released from the FireCascade spell
	FirePillars(Dungeon &dungeon, int x, int y, int limit = 1);
	void activeTrapAction(Actors &a);

private:
	int m_limit;
	int m_ring = -1; // The next ring; Starts at -1 to ignore the turn the player used it on
};

class IceShards : public Traps {
public:
	// Released from IceShard Spell
	IceShards(Dungeon &dungeon, int x, int y, int limit = 3);
	void activeTrapAction(Actors &a);

private:
	int m_shards = 0; // Shards left
	int m_limit; // Number of ice shards to spawn
};

class HailStorm : public Traps {
public:
	// Released from HailStorm spell
	HailStorm(Dungeon &dungeon, int x, int y, char dir, int limit = 3);
	void activeTrapAction(Actors &a);

private:
	char m_dir; // Where to strike
	int m_turns = 0; // Turns left
	int m_limit; // Maximum number of strikes
};

class Shockwaves : public Traps {
public:
	// Released from the Shockwave spell
	Shockwaves(Dungeon &dungeon, int x, int y, char dir, int limit = 5);
	void activeTrapAction(Actors &a);

private:
	char m_dir; // Direction the wave ripples in
	int m_waves = 1; // The shockwave it's on
	int m_limit; // How many shockwaves to produce
};

class RockSummon : public Traps {
public:
	// Released from RockSummon spell
	RockSummon(Dungeon &dungeon, int x, int y);
	void trapAction(Actors &a);
};

class WindVortex : public Traps {
public:
	// Released from WindVortex spell
	WindVortex(Dungeon &dungeon, int x, int y, int limit = 20);
	void activeTrapAction(Actors &a);

private:
	int m_turns = 0; // Turns left
	int m_limit; // Maximum number of turns
};

class ThunderCloud : public Traps {
public:
	// Released from ThunderCloud spell
	ThunderCloud(Dungeon &dungeon, int x, int y, char dir, int limit = 20);
	void activeTrapAction(Actors &a);

	bool isLightSource() const { return true; };

private:
	char m_dir;
	int m_turns = 0; // Turns left
	int m_limit; // Maximum number of turns
	int m_wait = 1;
};

#endif
