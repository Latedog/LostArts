#include <string>

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

class Player;
struct Tile;

class Objects {
public:
	Objects();
	Objects(int x, int y);
	virtual ~Objects();
	int getPosX() const;
	int getPosY() const;
	void setrandPosX();
	void setrandPosY();
private:
	int m_x;
	int m_y;
};

class Idol : public Objects {
public:
	Idol();
	~Idol();
private:
};

class Drops : public Objects {
public:
	Drops(int x, int y, std::string item);
	std::string getItem() const;
private:
	std::string m_item;
};

class LifePotion : public Drops {
public:
	LifePotion();
	void restoreHP(Player &p);
};

class ArmorDrop : public Drops {
public:
	ArmorDrop();
	void increaseArmor(Player &p);
};

class StatPotion : public Drops {
public:
	StatPotion();
	void buffStats(Player &p);
};

class Chest : public Drops {
public:
	Chest();
	void open(Tile &tile);
};

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
	RustyCutlass(int x, int y);
};

class BoneAxe : public Weapon {
public:
	BoneAxe(int x, int y);
	~BoneAxe();
private:
};

#endif