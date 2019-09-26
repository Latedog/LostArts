#include "GameObjects.h"
#include <vector>

#ifndef ACTORS_H
#define ACTORS_H

class Weapon;
class Player;
class Goblin;
class Dungeon;

class Actors {
public:
	Actors();
	Actors(int x, int y, int hp, int armor, int str, int dex, Weapon wep);
	virtual ~Actors();
	int getPosX() const;
	int getPosY() const;
	int getHP() const;
	int getArmor() const;
	int getStr() const;
	int getDex() const;
	virtual Weapon getWeapon() const;
	void setPosX(int x);
	void setPosY(int y);
	void setrandPosX();
	void setrandPosY();
	void setHP(int hp);
	void setArmor(int armor);
	void setStr(int str);
	void setDex(int dex);
	void setWeapon(Weapon wep);

private:
	int m_x;
	int m_y;
	int m_hp;
	int m_armor;
	int m_str;
	int m_dex;
	Weapon m_wep;
};

class Player : public Actors {
public:
	Player();
	~Player();
	void attack(Goblin &g);
	void showInventory();
	std::vector<Weapon> getInventory();
	int getInventorySize() const;
	void addInventory(Weapon w);
	int getItemInvSize() const;
	void addItem(Drops drop);
	void wield();
	void showItems();
	void use();
	int getMaxHP() const;
	void rollHeal();
	void setWin(bool win);
	bool getWin() const;
private:
	std::vector<Weapon> m_inv;
	int m_invsize;
	std::vector<Drops> m_iteminv;
	int m_iteminvsize;

	int m_maxhp;
	int m_maxarmor = 99;
	int m_maxstr = 99;
	int m_maxdex = 99;
	bool m_winner;
};

class Monster : public Actors {
public:
	Monster();
	Monster(int x, int y, int hp, int armor, int str, int dex, Weapon wep);
	~Monster();
private:
};

class Goblin : public Monster {
public:
	Goblin(int smelldist);
	~Goblin();
	void attack(Player &p);
	int getSmellDistance() const;
private:
	int m_smelldist;
};

#endif