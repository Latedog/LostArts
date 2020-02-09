#include "cocos2d.h"
#include <string>
#include <vector>

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

class Dungeon;
class Actors;
class Player;
struct _Tile;

void playBoneCrunch(float volume = 1.0f);
void playCrumble(float volume = 1.0f);

void poisonCloud(Dungeon &dungeon, int x, int y, int time, cocos2d::Color3B color);
void fadeOut(cocos2d::Sprite* sprite);
void flashFloor(Dungeon &dungeon, int x, int y, bool mega = false);

class Objects {
public:
	Objects();
	Objects(std::string item, bool destructible = false);
	Objects(std::string item, std::string image, bool destructible = false);
	Objects(int x, int y, std::string item, bool destructible = false);
	Objects(int x, int y, std::string item, std::string image, bool destructible = false);
	virtual ~Objects();

	virtual std::string getItem() const;
	std::string getImageName() const;
	cocos2d::Sprite* getSprite();
	int getPosX() const;
	int getPosY() const;

	void setSprite(cocos2d::Sprite* sprite);
	void setImageName(std::string image);
	void setPosX(int x);
	void setPosY(int y);
	void setrandPosX(int maxrows);
	void setrandPosY(int maxcols);

	bool isDestructible() const;
	void setDestructible(bool destructible);
	bool isWeapon() const;
	void setWeaponFlag(bool weapon);
	bool isShield() const;
	void setShieldFlag(bool shield);
	bool isItem() const;
	void setItemFlag(bool item);
	bool isAutoUse() const;
	void setAutoFlag(bool autoUse);
	bool isChest() const;
	void setChestFlag(bool chest);
	bool isTrinket() const;
	void setTrinketFlag(bool trinket);

	std::string getDescription() const;
	void setDescription(std::string description);

	std::string getSoundName() const;
	void setSoundName(std::string sound);
	bool hasExtraSprites();
	void setExtraSpritesFlag(bool extras);
	virtual void setSpriteColor(cocos2d::Color3B color) { ; };
	bool emitsLight() const; // tells if the sprite should emit extra light
	void setEmitsLight(bool emits);

private:
	std::string m_item;
	std::string m_image; // sprite image name
	std::string m_sound; // sound clip to be used on pickup
	cocos2d::Sprite* m_sprite = nullptr;
	bool m_hasExtraSprites = false;
	bool m_emitsLight = false;

	std::string m_description; // short description of the item

	int m_x;
	int m_y;

	bool m_destructible; // flag for telling if the item can be destroyed
	bool m_isWeapon = false;
	bool m_isItem = false;
	bool m_autoUse = false;
	bool m_isChest = false;
	bool m_isShield = false;
	bool m_isTrinket = false;
};

class Idol : public Objects {
public:
	Idol();
	~Idol();
private:
};

class Door : public Objects {
public:
	Door(int x = 0, int y = 0);

	bool isOpen() const;
	void toggleOpen();
	bool isHeld() const;
	void toggleHeld();
	bool isLocked() const;
	void toggleLock();
private:
	bool m_open;
	bool m_hold;
	bool m_lock;
};


//	BEGIN DROPS CLASSES
class Drops : public Objects {
public:
	Drops(int x, int y, std::string item, std::string image, bool forPlayer = true);

	virtual void changeStats(Drops &drop, Player &p);
	virtual void useItem(Player &p) { return; };

	virtual void useItem(Drops &drop, Dungeon &dungeon);
	virtual void useItem(Dungeon &dungeon) { ; };

	bool forPlayer() const;

private:
	bool m_forPlayer; // flag to check if item is used on the player or the dungeon

};

class Gold : public Drops {
	Gold(int x, int y);
};

class HeartPod : public Drops {
public:
	HeartPod(int x = 0, int y = 0);
	void useItem(Player &p);
};

class LifePotion : public Drops {
public:
	LifePotion(int x = 0, int y = 0);
	void useItem(Player &p);
};

class BigLifePotion : public Drops {
public:
	BigLifePotion(int x = 0, int y = 0);
	void useItem(Player &p);

};

class StatPotion : public Drops {
public:
	StatPotion(int x = 0, int y = 0);
	void useItem(Player &p);
};

class ArmorDrop : public Drops {
public:
	ArmorDrop(int x = 0, int y = 0);
	void useItem(Player &p);
};

class ShieldRepair : public Drops {
public:
	ShieldRepair(int x = 0, int y = 0);
	void useItem(Player &p);
};

class DizzyElixir : public Drops {
public:
	DizzyElixir(int x = 0, int y = 0);
	void useItem(Player &p);
};

class Bomb : public Drops {
public:
	Bomb(int x = 0, int y = 0);
	Bomb(std::string type, std::string image);

	void useItem(Dungeon &dungeon);

private:

};

class MegaBomb : public Bomb {
public:
	MegaBomb();

private:

};

class PoisonCloud : public Drops {
public:
	PoisonCloud(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);
private:

};

class SkeletonKey : public Drops {
public:
	SkeletonKey(int x, int y);

	void useItem(Dungeon &dungeon);
};

class FreezeSpell : public Drops {
public:
	FreezeSpell(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);
	int getDamage() const;
private:
	int m_damage = 0;
};

class EarthquakeSpell : public Drops {
public:
	EarthquakeSpell(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);
	int getDamage() const;
private:
	int m_damage = 6;
};

class FireBlastSpell : public Drops {
public:
	FireBlastSpell(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);
	int getDamage() const;
private:
	int m_damage = 5;
};

class WindSpell : public Drops {
public:
	WindSpell(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);
	int getDamage() const;
private:
	int m_damage = 0;
};

class InvisibilitySpell : public Drops {
public: 
	InvisibilitySpell(int x = 0, int y = 0);

	void useItem(Player &p);
};

class EtherealSpell : public Drops {
public:
	EtherealSpell(int x = 0, int y = 0);

	void useItem(Player &p);
};

class Teleport : public Drops {
public:
	Teleport(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);
};


//		TRINKETS
class Trinket : public Objects {
public:
	Trinket(int x, int y, std::string name, std::string image, bool destructible = false);

	virtual void apply(Trinket &trinket, Dungeon &dungeon, Player &p);
	virtual void apply(Dungeon &dungeon, Player &p) = 0;
	virtual void unapply(Dungeon &dungeon, Player &p) { ; };
};

class DarkStar : public Trinket {
public:
	// this trinket increases the player's damage, but reduces their field of vision
	DarkStar(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);
};

class LuckyPig : public Trinket {
public:
	// luck increase, it will give you a greater chance of item drops and better roll save chances
	LuckyPig(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);
};

class GoldPot : public Trinket {
public:
	// more gold!
	GoldPot(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);
};

class RingOfCasting : public Trinket {
public:
	// increased spell potency (+intellect)
	RingOfCasting(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);
};

class VulcanRune : public Trinket {
public:
	// lava immunity
	VulcanRune(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);
};

class BrightStar : public Trinket {
public:
	// greater field of vision and slightly increased max hp while equipped
	BrightStar(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);
};

class Bloodrite : public Trinket {
public:
	// chance to heal on attack
	Bloodrite(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);
};

class Bloodlust : public Trinket {
public:
	// low hp increases damage
	Bloodlust(int x = 0, int y = 0);

	void apply(Dungeon &dungeon, Player &p);
	void unapply(Dungeon &dungeon, Player &p);
};





//	BEGIN CHESTS CLASSES
class Chests : public Drops {
public:
	Chests(std::string chest, std::string image);
	virtual std::string open(Chests &chest, _Tile &tile);
	virtual std::string open(_Tile &tile) { return ""; };
};

class BrownChest : public Chests {
public:
	BrownChest();
	std::string open(_Tile &tile);
};

class SilverChest : public Chests {
public:
	SilverChest();
	std::string open(_Tile &tile);
};

class GoldenChest : public Chests {
public:
	GoldenChest();
	std::string open(_Tile &tile);
};

class InfinityBox : public Chests {
public:
	InfinityBox();
	std::string open(_Tile &tile);
};


//	BEGIN WEAPON CLASSES
class Weapon : public Objects {
public:
	Weapon();
	Weapon(int x, int y, std::string action, int dexbonus, int dmg, int range, bool ability = false);
	Weapon(int x, int y, std::string action, std::string image, int dexbonus, int dmg, int range, bool ability = false);

	virtual void applyBonus(Actors &a) { ; }; // flat stat upgrades while equipped
	virtual void unapplyBonus(Actors &a) { ; };
	virtual void useAbility(Weapon &weapon, Dungeon &dungeon, Actors &a);
	virtual void useAbility(Dungeon &dungeon, Actors &a) { ; };
	virtual void usePattern(Weapon &weapon, Dungeon &dungeon, bool &moveUsed);
	virtual void usePattern(Dungeon &dungeon, bool &moveUsed) { ; };

	bool hasBonus() const;
	void setHasBonus(bool bonus);
	bool hasAbility() const; // tells if weapon has special effect
	void setHasAbility(bool ability);
	bool hasAttackPattern() const; // tells if weapon has non-standard attack pattern
	void setAttackPattern(bool pattern);
	bool canBeCast() const;
	void setCast(bool cast);
	std::string getAction() const;
	int getDexBonus() const;
	int getDmg() const;
	int getRange() const;
	bool hasBleed() const;
	bool hasBurn() const;
	bool hasPoison() const;

	void setDamage(int damage);

private:
	bool m_hasBonus = false;
	bool m_hasAbility;
	bool m_hasPattern = false;
	bool m_canBeCast = false;
	std::string m_action;
	int m_dexbonus;
	int m_dmg;
	int m_range;
	bool m_bleed;
	bool m_burn;
	bool m_poison;
};

class ShortSword : public Weapon {
public:
	ShortSword();
	
private:
};
class BloodShortSword : public Weapon {
public:
	BloodShortSword();
	void useAbility(Dungeon &dungeon, Actors &a);
};
class GoldenShortSword : public Weapon {
public:
	GoldenShortSword();

	void useAbility(Dungeon &dungeon, Actors &a);
};

class GoldenLongSword : public Weapon {
public:
	// has piercing properties
	GoldenLongSword();
	void usePattern(Dungeon &dungeon, bool &moveUsed);
	void useAbility(Dungeon &dungeon, Actors &a);
};

class Katana : public Weapon {
public:
	// has piercing properties
	Katana();
	void usePattern(Dungeon &dungeon, bool &moveUsed);
};
class IronLongSword : public Weapon {
public:
	// has piercing properties
	IronLongSword();
	void usePattern(Dungeon &dungeon, bool &moveUsed);
};

class RustyCutlass : public Weapon {
public:
	RustyCutlass();
	RustyCutlass(int x, int y);
};

class BoneAxe : public Weapon {
public:
	BoneAxe(int x, int y);
private:
};

class Bow : public Weapon {
public:
	Bow(int x, int y, std::string name, std::string image, int damage, int dexbonus, int range);
	void usePattern(Dungeon &dungeon, bool &moveUsed);
};
class WoodBow : public Bow {
public:
	WoodBow();
};
class IronBow : public Bow {
public:
	IronBow();
};
class VulcanBow : public Bow {
public:
	VulcanBow();

	void useAbility(Dungeon &dungeon, Actors &a);

private:
	int m_burnChance = 10;
};

class BronzeDagger : public Weapon {
public:
	BronzeDagger();

	void useAbility(Dungeon &dungeon, Actors &a);
	int getBleedChance() const;

private:
	int m_bleedChance;
};

class IronLance : public Weapon {
public: 
	IronLance();

	void usePattern(Dungeon &dungeon, bool &moveUsed);
};

class VulcanSword : public Weapon {
public:
	VulcanSword();

	void useAbility(Dungeon &dungeon, Actors &a);

private:
	int m_burnChance = 10;
};

class VulcanHammer : public Weapon {
public:
	VulcanHammer();
	void usePattern(Dungeon &dungeon, bool &moveUsed);
	void useAbility(Dungeon &dungeon, Actors &a);

private:
	int m_burnChance = 10;
	bool m_woundUp = false; // hammers have a one turn wind-up before launching
};

class ArcaneStaff : public Weapon {
public:
	ArcaneStaff();
	void applyBonus(Actors &a);
	void unapplyBonus(Actors &a);
	void usePattern(Dungeon &dungeon, bool &moveUsed);
	void useAbility(Dungeon &dungeon, Actors &a);

private:
	bool m_bonusApplied = false;
	bool m_isCast = false; // for telling if staff's ability was cast

	// tells the player's past position, cast is not reset until player moves
	int m_px;
	int m_py;
};



///	BOSS WEAPONS
class SmashersFists : public Weapon {
public:
	SmashersFists();
};


//  BEGIN SHIELD CLASSES
class Shield : public Objects {
public:
	Shield();
	Shield(int defense, int durability, int coverage, std::string type, std::string image);
	Shield(int x, int y, int defense, int durability, int coverage, std::string type, std::string image);

	// used for shield archetype copy constructors
	Shield(int defense, int durability, int max_durability, int coverage, std::string type, std::string image);

	virtual void useAbility(Dungeon &dungeon, Actors &a) { ; };

	int getDefense() const;
	void setDefense(int defense);
	int getDurability() const;
	void setDurability(int durability);
	int getMaxDurability() const;
	void setMaxDurability(int durability);
	int getCoverage() const;
	void setCoverage(int coverage);
	bool hasAbility() const;
	void setAbility(bool ability);

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
	WoodShield();
	WoodShield(Shield const shield);
	WoodShield(int x, int y);
};

class IronShield : public Shield {
public:
	IronShield();
	IronShield(Shield const shield);
	IronShield(int x, int y);
};

class ThornedShield : public Shield {
public:
	ThornedShield(int x = 0, int y = 0);

	void useAbility(Dungeon &dungeon, Actors &a);
};

class FrostShield : public Shield {
public:
	FrostShield(int x = 0, int y = 0);

	void useAbility(Dungeon &dungeon, Actors &a);
};

class ReflectShield : public Shield {
public:
	ReflectShield(int x = 0, int y = 0);

	void useAbility(Dungeon &dungeon, Actors &a);
	void checkLineOfFire(Dungeon &dungeon, Actors &a);
};


//	BEGIN TRAPS
class Traps : public Objects {
public:
	Traps(int x, int y, std::string name, std::string image, int damage, bool destructible = false);

	// trap actions for when actives is being checked
	virtual void activeTrapAction(Traps &trap, Dungeon &dungeon, Actors &a);
	virtual void activeTrapAction(Dungeon &dungeon, Actors &a) { ; };

	// trap actions for when a player/monster steps on a trap
	virtual void trapAction(Traps &trap, Dungeon &dungeon, Actors &a);
	virtual void trapAction(Dungeon &dungeon, Actors &a);
	virtual void trapAction(Actors &a) { ; };

	int getDmg() const;
	void setDmg(int damage);

	// if a trap is temporary, it is removed after being activated/encountered
	bool isTemporary() const;
	void setTemporary(int temp);

	// tells if the trap's status constantly needs to be checked
	bool isActive() const;
	void setActive(bool active);

	// tells if the trap also acts as a wall
	bool actsAsWall() const;
	void setWallFlag(bool wall);

	// tells if trap immediately damages
	bool isLethal() const;
	void setLethal(bool lethal);

	// tells if this can be set off by explosions
	bool isExplosive() const;
	void setExplosive(bool explosive);

private:
	int m_trapdmg;
	bool m_temp = false;
	bool m_active;
	bool m_wall = false;
	bool m_lethal = false;
	bool m_explosive = false;
};

class Stairs : public Traps {
public:
	Stairs(int x = 0, int y = 0);
};

class Button : public Traps {
public:
	Button(int x = 0, int y = 0);
};

class Pit : public Traps {
public:
	// pits are instant death traps
	Pit();
	Pit(int x, int y);

	void trapAction(Dungeon &dungeon, Actors &a);
};

class FallingSpike : public Traps {
public:
	FallingSpike(int x, int y, int speed);

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Dungeon &dungeon, Actors &a); // used if a player steps in the way

	int getSpeed() const;
	void setSpeed(int speed);
private:
	int m_speed;
};

class Spikes : public Traps {
public:
	Spikes(int x, int y);

	void trapAction(Dungeon &dungeon, Actors &a); // used if a player steps in the way
};

class SpikeTrap : public Traps {
public:
	SpikeTrap();
	SpikeTrap(int x, int y, int speed);

	void activeTrapAction(Dungeon &dungeon, Actors &a); // called if player was standing on top of one that becomes active
	void trapAction(Dungeon &dungeon, Actors &a); // used if player steps on one already active
	//void trapAction(Actors &a); 
	void setSpriteVisibility(bool deactive, bool primed, bool active);

	int getSpeed() const;
	void setSpeed(int speed);
	int getCountdown() const;
	void setCountdown(int count);

	cocos2d::Sprite* getSpriteD();
	cocos2d::Sprite* getSpriteP();
	cocos2d::Sprite* getSpriteA();
private:
	int m_cyclespeed;
	int m_countdown;

	cocos2d::Sprite* m_deactive;
	cocos2d::Sprite* m_primed;
	cocos2d::Sprite* m_active;
};

class TriggerSpike : public Traps {
public:
	TriggerSpike(int x, int y);

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Actors &a);
	void setSpriteVisibility(bool deactive, bool primed, bool active);

	bool isTriggered() const;
	void toggleTrigger();

	cocos2d::Sprite* getSpriteD();
	cocos2d::Sprite* getSpriteP();
	cocos2d::Sprite* getSpriteA();
private:
	bool m_triggered;

	cocos2d::Sprite* m_deactive;
	cocos2d::Sprite* m_primed;
	cocos2d::Sprite* m_active;
};

class Puddle : public Traps {
public:
	Puddle(int x, int y);
	
	void trapAction(Dungeon &dungeon, Actors &a);
private:
	
};

class Firebar : public Traps {
public:
	Firebar(int x, int y, int rows);
	Firebar(int x, int y, std::string firebar); // for double firebar
	virtual ~Firebar();

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Actors &a);

	virtual void setInitialFirePosition(int x, int y, int rows);
	int getAngle() const;
	void setAngle(int angle);
	bool isClockwise() const;

	virtual bool playerWasHit(const Actors &a);
	virtual void setFirePosition(char move);

	void setSpriteColor(cocos2d::Color3B color);
	virtual void setSpriteVisibility(bool visible);

	cocos2d::Sprite* getInner();
	cocos2d::Sprite* getOuter();
private:
	int m_angle;
	bool m_clockwise;

	// these are helper objects to keep track of where the firebar's fireball is
	Objects m_innerFire;
	Objects m_outerFire;

	cocos2d::Sprite* m_inner = nullptr;
	cocos2d::Sprite* m_outer = nullptr;
};

class DoubleFirebar : public Firebar {
public:
	DoubleFirebar(int x, int y, int rows);
	~DoubleFirebar();

	void activeTrapAction(Dungeon &dungeon, Actors &a);

	void setInitialFirePosition(int x, int y, int rows);
	bool playerWasHit(const Actors &a);
	void setFirePosition(char move);

	void setSpriteColor(cocos2d::Color3B color);
	void setSpriteVisibility(bool visible);
	cocos2d::Sprite* getInner();
	cocos2d::Sprite* getInnerMirror();
	cocos2d::Sprite* getOuter();
	cocos2d::Sprite* getOuterMirror();

private:
	Objects m_innerFire;
	Objects m_innerFireMirror;
	Objects m_outerFire;
	Objects m_outerFireMirror;

	cocos2d::Sprite* m_inner = nullptr;
	cocos2d::Sprite* m_innerMirror = nullptr;
	cocos2d::Sprite* m_outer = nullptr;
	cocos2d::Sprite* m_outerMirror = nullptr;
};

class Lava : public Traps {
public:
	Lava(int x, int y);

	void trapAction(Actors &a);
};

class Spring : public Traps {
public:
	/* Spring traps take one turn to trigger before launching the player in a direction
	*  They will typically point in one direction, but there is a type that will launch in an unknown direction.
	*/

	// This is the standard spring trap; it points in a single random direction if @move is not specified
	// @trigger specifies if it needs to be triggered or if it springs immediately
	Spring(int x, int y, bool trigger = true, char move = '-');


	/* This constructor lets you explicitly specify that the spring direction can or cannot be @known
	*  It will also let you specify if it will only allow cardinal directionality with @cardinal
	*  Boolean table:
	*  known | cardinal |        RESULT
	*    0   |    0     | points in all directions (unknown)
	*    0   |    1     | points in all directions (unknown)
	*    1   |    0     | points in only cardinal directions
	*    1   |    1     | points in only diagonal directions

	*/
	Spring(int x, int y, bool trigger, bool known, bool cardinal);

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Dungeon &dungeon, Actors &a);

	char getDirection() const;
	void setDirection(char dir);
	bool isTrigger() const;
	bool triggered() const;
	void setImage();

	// tells if the trap bounces in multiple directions
	bool isMultiDirectional() const;

	// tells if the trap bounces in all directions
	bool isAny() const;

	// if multidirectional, tells if bounces only in cardinal directions
	bool isCardinal() const;

	// tells if two spring traps are pointing toward each other
	// for preventing infinite bouncing loops
	bool isOpposite(Spring other) const;

	void oppositeSprings(Dungeon &dungeon);

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
	*
	*/
	Turret(int x, int y, char move, int range = 10);

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Dungeon &dungeon, Actors &a);
	void checkLineOfFire(Dungeon &dungeon);

	char getDirection() const;
	void setDirection(char dir);
	int getRange() const;
	void setRange(int range);
	bool isTriggered() const;
	void setTrigger(bool trigger);
	bool onCooldown() const;
	void setCooldown(bool cooldown);
private:
	char m_dir;
	int m_range;
	bool m_triggered;
	bool m_cooldown = false;
};

class MovingBlock : public Traps {
public:
	MovingBlock(int x, int y, char pattern, int spaces = 3);

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Dungeon &dungeon, Actors &a);

	char getPattern() const;
	void setPattern(char pattern);
	int getSpaces() const;
	int getCounter() const;
	void setCounter(int count);
	void resetCounter();
	void flip();
	bool turn() const;
	void toggleTurn();
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

	// m_turn is tells it to move in the opposite direction if spaces reaches zero
	bool m_turn = false;
};

class ActiveBomb : public Traps {
public:
	ActiveBomb(int x, int y, int timer = 3);
	ActiveBomb(int x, int y, std::string type, std::string image, int damage, int timer = 3);

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	virtual void explosion(Dungeon &dungeon, Actors &a);

	int getTimer() const;
	void setTimer(int timer);

private:
	int m_timer;
	int m_fuseID; // for getting and stopping fuse sound when bomb explodes
};

class ActiveMegaBomb : public ActiveBomb {
public:
	ActiveMegaBomb(int x, int y);

	void activeTrapAction(Dungeon &dungeon, Actors &a);

private:
	//
};

class PoisonBomb : public ActiveBomb {
public:
	PoisonBomb(int x, int y, int turns);

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Dungeon &dungeon, Actors &a);
	void explosion(Dungeon &dungeon, Actors &a);
	void radiusCheck(Dungeon &dungeon, Actors &a);

private:
	int m_turns;
	bool m_blown;
	bool m_triggered = false;
};

class BearTrap : public Traps {
public:
	BearTrap(int x, int y);

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Dungeon &dungeon, Actors &a);

private:
	bool m_triggered = false;
	int m_wait = 1;
};

class CrumbleFloor : public Traps {
public:
	CrumbleFloor(int x, int y , int strength = 4);
	CrumbleFloor(int x, int y, int strength, std::string name, std::string image);

	void activeTrapAction(Dungeon &dungeon, Actors &a);
	void trapAction(Dungeon &dungeon, Actors &a);

private:
	int m_strength; // how many turns before the floor crumbles away
	bool m_crossed = false; // flag for when player walks off

	int m_x, m_y; // saves coordinates of whatever walked on it
};

class CrumbleLava : public CrumbleFloor {
public:
	CrumbleLava(int x, int y, int strength = 4);

};



#endif
