#include "cocos2d.h"
#include <string>
#include <vector>

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

void playBoneCrunch(float volume = 1.0f);

class Dungeon;
class Actors;
class Player;
struct _Tile;

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
	void setPosX(int x);
	void setPosY(int y);
	void setrandPosX(int maxrows);
	void setrandPosY(int maxcols);

	bool isDestructible() const;
	void setDestructible(bool destructible);

private:
	std::string m_item;
	std::string m_image;
	cocos2d::Sprite* m_sprite = nullptr;
	int m_x;
	int m_y;

	bool m_destructible; // flag for telling if the item can be destroyed
};

class Idol : public Objects {
public:
	Idol();
	~Idol();
private:
};

class Stairs : public Objects {
public:
	Stairs();
private:

};

class Door : public Objects {
public:
	Door();

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

class Button : public Objects {
public:
	Button();
};


//	BEGIN DROPS CLASSES
class Drops : public Objects {
public:
	//Drops(int x, int y, std::string item, bool forPlayer = true);
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
	HeartPod();
	HeartPod(int x, int y);
	void useItem(Player &p);
};

class LifePotion : public Drops {
public:
	LifePotion();
	void useItem(Player &p);
};

class ArmorDrop : public Drops {
public:
	ArmorDrop();
	void useItem(Player &p);
};

class ShieldRepair : public Drops {
public:
	ShieldRepair();
	void useItem(Player &p);
};

class StatPotion : public Drops {
public:
	StatPotion();
	void useItem(Player &p);
};

class DizzyElixir : public Drops {
public:
	DizzyElixir(int x = 0, int y = 0);
	void useItem(Player &p);
};

class Bomb : public Drops {
public:
	Bomb();
	Bomb(std::string type, std::string image);

	void useItem(Dungeon &dungeon);

	int getFuse() const;
	void setFuse(int fuse);
	void lightBomb();
	bool isLit() const;
private:
	int m_fuse;
	bool m_lit;
};

class MegaBomb : public Bomb {
public:
	MegaBomb();

private:

};

class SkeletonKey : public Drops {
public:
	SkeletonKey();

	void useItem(Dungeon &dungeon);
};

class FreezeSpell : public Drops {
public:
	FreezeSpell(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);
private:

};

class EarthquakeSpell : public Drops {
public:
	EarthquakeSpell(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);
private:

};

class FireBlastSpell : public Drops {
public:
	FireBlastSpell(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);

private:
	
};

class WindSpell : public Drops {
public:
	WindSpell(int x = 0, int y = 0);

	void useItem(Dungeon &dungeon);
private:

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


//	BEGIN CHESTS CLASSES
class Chests : public Drops {
public:
	Chests(std::string chest, std::string image);
	virtual std::string open(_Tile &tile) { return ""; };
};

class BrownChest : public Chests {
public:
	BrownChest();
	std::string open(_Tile &tile, std::vector<std::string> &text);
};

class SilverChest : public Chests {
public:
	SilverChest();
	std::string open(_Tile &tile, std::vector<std::string> &text);
};

class GoldenChest : public Chests {
public:
	GoldenChest();
	std::string open(_Tile &tile, std::vector<std::string> &text);
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
	Weapon(int x, int y, std::string action, int dexbonus, int dmg, int range, std::string affliction);

	virtual void useAbility(Weapon &weapon, Dungeon &dungeon, Actors &a);
	virtual void useAbility(Dungeon &dungeon, Actors &a);

	bool hasAbility() const;
	std::string getAction() const;
	int getDexBonus() const;
	int getDmg() const;
	int getRange() const;
	bool hasBleed() const;
	bool hasBurn() const;
	bool hasPoison() const;
private:
	bool m_hasAbility;
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

class WoodBow : public Weapon {
public:
	WoodBow();
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

	int getDefense() const;
	void setDefense(int defense);
	int getDurability() const;
	void setDurability(int durability);
	int getMaxDurability() const;
	void setMaxDurability(int durability);
	int getCoverage() const;
	void setCoverage(int coverage);

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


//	BEGIN TRAPS
class Traps : public Objects {
public:
	Traps(int x, int y, std::string name, int damage, bool destructible = false);

	int getDmg() const;
	void setDmg(int damage);

private:
	int m_trapdmg;
};

class Pit : public Traps {
public:
	// pits are instant death traps
	Pit();
	Pit(int x, int y);

	void fall(Actors &a);
};

class FallingSpike : public Traps {
public:
	FallingSpike(int x, int y, int speed);
	int getSpeed() const;
	void setSpeed(int speed);
private:
	int m_speed;
};

class SpikeTrap : public Traps {
public:
	SpikeTrap();
	SpikeTrap(int x, int y, int speed);

	void cycle(Actors &a);
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

	void trigger(Actors &a);
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
	
	void slip(Actors &a);
private:
	
};

class Firebar : public Traps {
public:
	Firebar(int x, int y);
	Firebar(int x, int y, std::string firebar); // for double firebar

	void burn(Actors &a);

	virtual void setInitialFirePosition(int x, int y);
	int getAngle() const;
	void setAngle(int angle);
	bool isClockwise() const;

	virtual bool playerWasHit(const Actors &a);
	virtual void setFirePosition(char move);

	virtual void setSpriteVisibility(bool visible);

	cocos2d::Sprite* getInner();
	cocos2d::Sprite* getOuter();
private:
	int m_angle;
	bool m_clockwise;

	// these are helper objects to keep track of where the firebar's fireball is
	Objects m_innerFire;
	Objects m_outerFire;

	cocos2d::Sprite* m_inner;
	cocos2d::Sprite* m_outer;
};

class DoubleFirebar : public Firebar {
public:
	DoubleFirebar(int x, int y);

	void setInitialFirePosition(int x, int y);
	bool playerWasHit(const Actors &a);
	void setFirePosition(char move);

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

	cocos2d::Sprite* m_inner;
	cocos2d::Sprite* m_innerMirror;
	cocos2d::Sprite* m_outer;
	cocos2d::Sprite* m_outerMirror;
};

class Lava : public Traps {
public:
	Lava(int x, int y);

	void burn(Actors &a);
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

	void trigger(Dungeon &dungeon, Actors &a);

	char getDirection() const;
	bool isTrigger() const;
	bool triggered() const;

	// tells if the trap bounces in multiple directions
	bool isMultiDirectional() const;

	// tells if the trap bounces in all directions
	bool isAny() const;

	// if multidirectional, tells if bounces only in cardinal directions
	bool isCardinal() const;
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

	void shoot(Dungeon &dungeon, Actors &a);
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

	void move(Dungeon &dungeon, Actors &a);

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

#endif
