#include "GameObjects.h"
#include <vector>

#ifndef ACTORS_H
#define ACTORS_H

struct Tile;
class Objects;
class Weapon;
class Player;
class Monster;
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
	//void attack(std::vector<Monster*> monsters, int pos);
	void attack(std::vector<std::shared_ptr<Monster>> monsters, int pos);
	void showInventory();
	void showWeapons();
	void showItems();
	int getInventorySize() const;
	int getItemInvSize() const;
	void addWeapon(Weapon w);
	void wield();
	void addItem(Drops drop);
	void use(std::vector<std::shared_ptr<Objects>> &active, Tile &tile);
	void setMaxHP(int maxhp);
	int getMaxHP() const;
	void rollHeal();
	void setWin(bool win);
	bool getWin() const;
	std::string getDeath() const;
	void setDeath(std::string m);
private:
	std::vector<Weapon> m_inv;
	unsigned m_invsize;
	std::vector<Drops> m_iteminv;
	unsigned m_iteminvsize;

	int m_maxhp;
	int m_maxarmor = 99;
	int m_maxstr = 99;
	int m_maxdex = 99;
	bool m_winner;
	std::string m_death;
};

class Monster : public Actors {
public:
	Monster();
	Monster(int x, int y, int hp, int armor, int str, int dex, Weapon wep, std::string name);
	~Monster();
	virtual bool doAction(Player &p) { return false; };
	virtual void encounter(Player &p, Monster &m);
	void attack(Player &p);
	std::string getName();
private:
	std::string m_name;
};

class Goblin : public Monster {
public:
	Goblin(int smelldist);
	~Goblin();
	int getSmellDistance() const;
private:
	int m_smelldist;
};

class Wanderer : public Monster {
public:
	Wanderer();
	~Wanderer();
};

class Archer : public Monster {
public:
	Archer();
	bool doAction(Player &p);
	bool isPrimed() const;
	void prime(bool p);
private:
	bool m_primed;
};


//	BOSSES
class Smasher : public Monster {
public:
	Smasher();
	void attack(Player &p);
private:

};

#endif