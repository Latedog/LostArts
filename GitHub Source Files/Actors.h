#include "cocos2d.h"
#include "GameObjects.h"
#include <vector>

#ifndef ACTORS_H
#define ACTORS_H

struct _Tile;
class Objects;
class Weapon;
class Player;
class Monster;
class Goblin;
class Dungeon;

class Actors {
public:
	Actors();
	Actors(int x, int y, int hp, int armor, int str, int dex, std::string name);
	Actors(int x, int y, int hp, int armor, int str, int dex, Weapon wep);
	Actors(int x, int y, int hp, int armor, int str, int dex, Weapon wep, std::string name);
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
	void setrandPosX(int maxcols);
	void setrandPosY(int maxrows);
	void setHP(int hp);
	void setArmor(int armor);
	void setStr(int str);
	void setDex(int dex);
	void setWeapon(Weapon wep);
	std::string getName() const;

	//	AFFLICTIONS
	bool isBurned() const;
	void toggleBurn();
	int burnsLeft() const;
	void setBurn(int burn);

	bool isBled() const;
	void toggleBleed();
	int bleedLeft() const;
	void setBleed(int bleed);
	
private:
	int m_x;
	int m_y;
	int m_hp;
	int m_armor;
	int m_str;
	int m_dex;
	Weapon m_wep;
	std::string m_name;

	bool m_burned;
	int m_burncount;
	bool m_bleed;
	int m_bleedcount;
	bool m_poisoned;
	int m_poisoncount;
};

class Player : public Actors {
public:
	Player();
	~Player();

	void attack(std::vector<std::shared_ptr<Monster>> &monsters, std::vector<std::shared_ptr<Objects>> &actives, int pos, std::vector<std::string> &text);

	void showInventory();
	void showWeapons(std::vector<std::string> &text);
	void showItems(std::vector<std::string> &text);

	std::vector<Weapon> getWeaponInv() const;
	std::vector<Drops> getItemInv() const;
	int getInventorySize() const;
	int getItemInvSize() const;
	int getMaxWeaponInvSize() const;
	int getMaxItemInvSize() const;

	void addWeapon(Weapon w);
	void wield(std::vector<std::string> &text);
	void wield(); // menu arrow selection version of wield
	void wield(int index);
	void addItem(Drops drop);
	void use(std::vector<std::shared_ptr<Objects>> &active, _Tile &tile, std::vector<std::string> &text);
	std::string use(std::vector<std::shared_ptr<Objects>> &active, _Tile &tile, int index); // new version

	void setMaxHP(int maxhp);
	int getMaxHP() const;
	void rollHeal(std::vector<std::string> &text);

	bool hasSkeletonKey() const;
	void checkKeyConditions(std::vector<std::string> &text);
	int keyHP() const;
	void setKeyHP(int keyhp);

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
	int m_maxwepinv = 5;
	int m_maxiteminv = 5;

	int m_keyhp;

	bool m_winner;
	std::string m_death;
};

class Monster : public Actors {
public:
	Monster();
	Monster(int x, int y, int hp, int armor, int str, int dex, Weapon wep, std::string name);
	Monster(int x, int y, int hp, int armor, int str, int dex, std::string name);
	~Monster();

	virtual void encounter(Player &p, Monster &m, std::vector<std::string> &text);
	virtual void encounter(Monster &m1, Monster &m2);
	virtual void attack(Player &p, std::vector<std::string> &text);
	virtual void attack(Monster &m);
	std::string getName();
	void setMaxHP(int maxhp);
	int getMaxHP() const;

	cocos2d::Action* getMove() const;
	void setMove(cocos2d::Action* action);
private:
	std::string m_name;
	int m_maxhp;

	cocos2d::Action* m_move;
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
	
	bool isPrimed() const;
	void prime(bool p);
private:
	bool m_primed;
};

class Zapper : public Monster {
public:
	Zapper();

	void attack(Player &p, std::vector<std::string> &text);
	bool onCooldown() const;
	void setCooldown();
	bool cardinalAttack() const;
	void swapDirection();
	std::map<int, cocos2d::Sprite*> getSparks();
private:
	bool m_cooldown;
	bool m_attack;

	std::map<int, cocos2d::Sprite*> sparks;
	/*cocos2d::Sprite* spark1;
	cocos2d::Sprite* spark2;
	cocos2d::Sprite* spark3;
	cocos2d::Sprite* spark4;
	cocos2d::Sprite* spark5;
	cocos2d::Sprite* spark6;
	cocos2d::Sprite* spark7;
	cocos2d::Sprite* spark8;*/
};

class Spinner : public Monster {
public:
	Spinner();

	void attack(Player &p, std::vector<std::string> &text);
	bool isClockwise() const;
	int getAngle() const;
	void setAngle(int angle);

	cocos2d::Sprite* getInner() const;
	cocos2d::Sprite* getOuter() const;
private:
	bool m_clockwise;
	int m_angle;

	cocos2d::Sprite* inner;
	cocos2d::Sprite* outer;
};

class Bombee : public Monster {
public:
	Bombee();

	void attack(Player &p, std::vector<std::string> &text);
	int getFuse() const;
	void setFuse();
	bool isFused() const;
	int getRange() const;
private:
	int m_fuse;
	bool m_fused;
	int m_aggroRange;
};

class MountedKnight : public Monster {
public:
	MountedKnight();

	//void attack(Player &p, std::vector<std::string> &text);
	bool isAlerted() const;
	void toggleAlert();
	char getDirection() const;
	void setDirection(char dir);
private:
	bool m_alerted;
	char m_direction;
};

class Roundabout : public Monster {
public:
	Roundabout();

	void attack(Player &p, std::vector<std::string> &text);
	char getDirection() const;
	void setDirection(char dir);
private:
	char m_direction;
};

class Seeker : public Monster {
public:
	Seeker(int range);

	int getRange() const;
	bool getStep() const;
	void toggleStep();
private:
	int m_range;
	bool m_step;
};


//	BOSSES
class Smasher : public Monster {
public:
	Smasher();

	void attack(Player &p, std::vector<std::string> &text);
	void attack(Monster &m);

	bool isActive() const;
	void setActive(bool status);
	bool isEnded() const;
	void setEnded(bool status);
	int getMove() const;
	void setMove(int move);
	bool isFrenzied() const;
	void startFrenzy();
private:
	bool m_moveActive;
	bool m_moveEnding;
	int m_moveType;
	bool m_frenzy;
};



#endif