#include <string>
#include <vector>

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

class Actors;
class Player;
//class Dungeon; // needed?
struct _Tile;

class Objects {
public:
	Objects();
	Objects(std::string item);
	Objects(int x, int y, std::string item);
	virtual ~Objects();

	//using Dungeon::addSprite; // add sprites
	virtual std::string getItem() const;
	int getPosX() const;
	int getPosY() const;
	void setPosX(int x);
	void setPosY(int y);
	void setrandPosX(int maxrows);
	void setrandPosY(int maxcols);
private:
	std::string m_item;
	int m_x;
	int m_y;
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
	Drops(int x, int y, std::string item);
	virtual void changeStats(Drops &drop, Player &p);
	virtual void useItem(Player &p) { return; };
private:
	
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

class StatPotion : public Drops {
public:
	StatPotion();
	void useItem(Player &p);
};

class Bomb : public Drops {
public:
	Bomb();
	Bomb(std::string type);

	void useItem(Player &p);
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
};


//	BEGIN CHESTS CLASSES
class Chests : public Drops {
public:
	Chests(std::string chest);
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
	std::string open(_Tile &tile, std::vector<std::string> &text);
};


//	BEGIN WEAPON CLASSES
class Weapon : public Objects {
public:
	Weapon();
	Weapon(int x, int y, std::string action, int dexbonus, int dmg, int range);
	Weapon(int x, int y, std::string action, int dexbonus, int dmg, int range, std::string affliction);
	~Weapon();

	std::string getAction() const;
	int getDexBonus() const;
	int getDmg() const;
	int getRange() const;
	bool hasBleed() const;
	bool hasBurn() const;
	bool hasPoison() const;
private:
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
	~ShortSword();
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
};

class IronLance : public Weapon {
public: 
	IronLance();
};


//	BOSS WEAPONS
class SmashersFists : public Weapon {
public:
	SmashersFists();
private:

};


//	BEGIN TRAPS
class Traps : public Objects {
public:
	Traps(int x, int y, std::string name, int damage);
	int getDmg() const;
	void setDmg(int damage);
private:
	int m_trapdmg;
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

	int getSpeed() const;
	void setSpeed(int speed);
	int getCountdown() const;
	void setCountdown(int count);
private:
	int m_cyclespeed;
	int m_countdown;
};

class TriggerSpike : public Traps {
public:
	TriggerSpike(int x, int y);

	bool isTriggered() const;
	void toggleTrigger();
private:
	bool m_triggered;
};


//	BEGIN AFFLICTIONS
class Burn : public Objects {
public:
	Burn();

	void burn(Actors &a, std::vector<std::string> &text);
	int maxBurns() const;
	int burnsLeft() const;
	void setBurn(int burn);
private:
	int m_maxburn;
	int m_burn;
};

class HealOverTime : public Objects {
public:
	HealOverTime();

	void heal(Player &p, std::vector<std::string> &text);
	int healsLeft() const;
	void setHeals(int heals);
private:
	int m_heals;
};

class Bleed : public Objects {
public:
	Bleed();

	void bleed(Actors &a, std::vector<std::string> &text);
	int maxBleed() const;
	int bleedLeft() const;
	void setBleed(int bleed);
	int maxWait() const;
	int getWait() const;
	void setWait(int wait);
private:
	int m_maxbleed;
	int m_bleed;
	int m_maxwait;
	int m_wait;
};

#endif
