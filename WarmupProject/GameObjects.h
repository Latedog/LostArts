#include <string>

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

class Player;
struct Tile;

class Objects {
public:
	Objects();
	Objects(int x, int y, std::string item);
	virtual ~Objects();
	virtual std::string getItem() const;
	int getPosX() const;
	int getPosY() const;
	void setPosX(int x);
	void setPosY(int y);
	void setrandPosX();
	void setrandPosY();
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
	void useItem(Player &p);
	int getFuse() const;
	void setFuse(int fuse);
	void lightBomb();
	bool isLit() const;
private:
	int m_fuse;
	bool m_lit;
};


//		BEGIN CHESTS FUNCTIONS
class Chests : public Drops {
public:
	Chests();
	virtual void open(Tile &tile) = 0;
};

class BrownChest : public Drops {
public:
	BrownChest();
	void open(Tile &tile);
};

class SilverChest : public Drops {
public:
	SilverChest();
	void open(Tile &tile);
};

class GoldenChest : public Drops {
public:
	GoldenChest();
	void open(Tile &tile);
};


//	BEGIN WEAPON CLASSES
class Weapon : public Objects {
public:
	Weapon();
	Weapon(int x, int y, std::string action, int dexbonus, int dmg);
	~Weapon();
	std::string getAction() const;
	int getDexBonus() const;
	int getDmg() const;
private:
	std::string m_action;
	int m_dexbonus;
	int m_dmg;
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
	~BoneAxe();
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


//	BOSS WEAPONS
class SmashersFists : public Weapon {
public:
	SmashersFists();
private:

};
#endif