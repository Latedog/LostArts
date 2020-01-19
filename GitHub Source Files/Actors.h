#include "cocos2d.h"
#include "GameObjects.h"
#include "Afflictions.h"
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

void playGotHit();
void playMiss(float volume = 1.0f);
void playEnemyHit(float volume = 1.0f);
void playHitSmasher(float volume = 1.0f);
void playShieldEquip(std::string shield);
void playShieldHit(std::string shield);
void playShieldBroken(std::string shield);

class Actors {
public:
	Actors();
	Actors(int x, int y, int hp, int armor, int str, int dex, std::string name, bool burnable = true, bool bleedable = true, bool healable = true, bool stunnable = true, bool freezable = true);
	Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep);
	Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name);
	Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name, bool burnable, bool bleedable, bool healable, bool stunnable, bool freezable);
	virtual ~Actors();

	int getPosX() const;
	int getPosY() const;
	int getMaxHP() const;
	int getHP() const;
	int getArmor() const;
	int getStr() const;
	int getDex() const;
	int getInt() const;
	int getLuck() const;
	//virtual Weapon getWeapon() const;
	virtual std::shared_ptr<Weapon>& getWeapon();
	std::string getName() const;
	std::vector<std::shared_ptr<Afflictions>>& getAfflictions();
	cocos2d::Sprite* getSprite();
	
	void setPosX(int x);
	void setPosY(int y);
	void setrandPosX(int maxcols);
	void setrandPosY(int maxrows);
	void setMaxHP(int maxhp);
	void setHP(int hp);
	void setArmor(int armor);
	void setStr(int str);
	void setDex(int dex);
	void setInt(int intellect);
	void setLuck(int luck);
	void setWeapon(std::shared_ptr<Weapon> wep);
	void setName(std::string name);
	void addAffliction(std::shared_ptr<Afflictions> affliction);
	void setSprite(cocos2d::Sprite* sprite);


	//	AFFLICTIONS
	void checkAfflictions();
	int findAffliction(std::string name);
	void removeAffliction(std::string name);

	bool canBeStunned() const;
	bool canBeBurned() const;
	bool canBeBled() const;
	bool canBeHealed() const;
	bool canBeFrozen() const;

	bool isBurned() const;
	void setBurned(bool burned);

	bool isBled() const;
	void setBleed(bool bled);

	bool isStunned() const;
	void setStunned(bool stun);

	bool isFrozen() const;
	void setFrozen(bool freeze);

	bool isInvisible() const;
	void setInvisible(int invisible);

	bool isEthereal() const;
	void setEthereal(int ethereal);

	bool isConfused() const;
	void setConfused(int confused);
	
private:
	int m_x;
	int m_y;
	int m_maxhp;
	int m_hp;
	int m_armor;
	int m_str;
	int m_dex;
	int m_int = 0;
	int m_luck = 0;
	//Weapon m_wep;
	std::shared_ptr<Weapon> m_wep;
	std::string m_name;
	
	// Keeps track of any afflictions inflicted on the actor
	std::vector<std::shared_ptr<Afflictions>> m_afflictions;

	bool m_burnable;
	bool m_stunnable;
	bool m_bleedable;
	bool m_healable;
	bool m_freezable;

	bool m_burned;
	bool m_bled;
	bool m_healed;
	bool m_poisoned;
	bool m_stunned;
	bool m_frozen;

	bool m_invisible = false;
	bool m_ethereal = false;
	bool m_confused = false;

	cocos2d::Sprite* m_sprite = nullptr;
};

class Player : public Actors {
public:
	Player();
	~Player();

	void attack(std::vector<std::shared_ptr<Monster>> &monsters, std::vector<std::shared_ptr<Objects>> &actives, int pos, std::vector<std::string> &text);
	void attack(Dungeon &dungeon, Actors &a);

	void showInventory();
	void showWeapons(std::vector<std::string> &text);
	void showItems(std::vector<std::string> &text);

	int getMoney() const;
	void setMoney(int money);
	//std::vector<Weapon> getWeaponInv() const;
	std::vector<std::shared_ptr<Weapon>>& getWeapons();
	//std::vector<Drops> getItemInv() const;
	std::vector<std::shared_ptr<Drops>>& getItems();
	//int getInventorySize() const;
	//int getItemInvSize() const;
	int getMaxWeaponInvSize() const;
	int getMaxItemInvSize() const;

	bool hasShield() const;
	void setShieldPossession(bool possesses);
	Shield& getShield();
	Shield getPlayerShield() const;
	void equipShield(Shield shield);
	void dropShield(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols);
	void shieldBroken();
	void repairShield(int repairs);
	bool isBlocking() const;
	void setBlock(bool blocking);
	void blocked();
	bool shieldCoverage(int mx, int my);
	char facingDirection() const;
	void setFacingDirection(char facing);

	//void addWeapon(Weapon w);
	void addWeapon(std::shared_ptr<Weapon> weapon);
	//void wield(std::vector<std::string> &text);
	//void wield(); // menu arrow selection version of wield
	void wield(int index);
	//void addItem(Drops drop);
	void addItem(std::shared_ptr<Drops> drop);
	//void use(std::vector<std::shared_ptr<Objects>> &active, _Tile &tile, std::vector<std::string> &text);
	void use(Dungeon &dungeon, _Tile &tile, int index); // new version

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
	std::vector<std::shared_ptr<Weapon>> m_weapons;
	unsigned m_invsize;
	std::vector<Drops> m_iteminv;
	std::vector<std::shared_ptr<Drops>> m_items;
	unsigned m_iteminvsize;

	int m_maxhp;
	int m_maxarmor = 99;
	int m_maxstr = 99;
	int m_maxdex = 99;
	int m_maxwepinv = 5;
	int m_maxiteminv = 5;
	int m_money = 0;

	bool m_hasShield;
	Shield m_shield;
	bool m_blocking;
	char m_facing;

	// keyhp is the limit before the skeleton key breaks.
	// it is set below the key's set minimum if player is already below this minimum threshold when they pick it up
	int m_keyhp;

	bool m_winner;
	std::string m_death;
};

class Monster : public Actors {
public:
	Monster();
	Monster(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name, 
		bool burnable = true, bool bleedable = true, bool healable = true, bool stunnable = true, bool freezable = true);
	Monster(int x, int y, int hp, int armor, int str, int dex, std::string name, bool burnable = true, bool bleedable = true, bool healable = true, bool stunnable = true, bool freezable = true);
	~Monster();

	virtual void encounter(Player &p, Monster &m, std::vector<std::string> &text);
	virtual void encounter(Monster &m1, Monster &m2);
	virtual void attack(Player &p, std::vector<std::string> &text);
	virtual void attack(Monster &m);
	
	bool isFlying() const;
	void setFlying(bool flying);

	bool chasesPlayer() const;
	void setChasesPlayer(bool chases);

private:
	bool m_flying = false;
	bool m_chases = false;
};

class Goblin : public Monster {
public:
	Goblin(int smelldist);

	int getSmellDistance() const;
private:
	int m_smelldist;
};

class Wanderer : public Monster {
public:
	Wanderer();
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
};

class Spinner : public Monster {
public:
	Spinner();

	void attack(Player &p, std::vector<std::string> &text);

	void setInitialFirePosition(int x, int y);
	void setFirePosition(char move);

	bool isClockwise() const;
	int getAngle() const;
	void setAngle(int angle);

	bool playerWasHit(const Actors &a);

	cocos2d::Sprite* getInner() const;
	cocos2d::Sprite* getOuter() const;
private:
	bool m_clockwise;
	int m_angle;

	Objects m_innerFire;
	Objects m_outerFire;

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