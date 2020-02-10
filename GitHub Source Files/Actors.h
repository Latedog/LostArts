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
void gotStunned(cocos2d::Sprite* sprite);
void bloodlustTint(Actors &a);

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
	virtual std::shared_ptr<Weapon>& getWeapon();
	std::string getName() const;
	std::vector<std::shared_ptr<Afflictions>>& getAfflictions();
	cocos2d::Sprite* getSprite();
	std::string getImageName() const;
	
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
	void setImageName(std::string image);

	bool isPlayer() const;
	void setPlayerFlag(bool player);

	bool isMonster() const;
	void setMonsterFlag(bool monster);

	//	AFFLICTIONS
	void checkAfflictions();
	int findAffliction(std::string name);
	void removeAffliction(std::string name);

	bool lavaImmune() const; // immunity to lava
	void setLavaImmunity(bool immune);

	bool isFlying() const;
	void setFlying(bool flying);

	bool isSturdy() const; // tells if this gets knocked back by attacks or not
	void setSturdy(bool sturdy);

	bool hasBloodlust() const;
	void setBloodlust(bool lust);

	bool canBeStunned() const;
	bool canBeBurned() const;
	bool canBeBled() const;
	bool canBeHealed() const;
	bool canBeFrozen() const;
	bool canBePoisoned() const;
	void setCanBeStunned(bool stunnable);
	void setCanBeBurned(bool burnable);
	void setCanBeBled(bool bleedable);
	void setCanBeHealed(bool healable);
	void setCanBeFrozen(bool freezable);
	void setCanBePoisoned(bool poisonable);

	bool isBurned() const;
	void setBurned(bool burned);

	bool isBled() const;
	void setBleed(bool bled);

	bool isStunned() const;
	void setStunned(bool stun);

	bool isFrozen() const;
	void setFrozen(bool freeze);

	bool isPoisoned() const;
	void setPoisoned(int poisoned);

	bool isInvisible() const;
	void setInvisible(int invisible);

	bool isEthereal() const;
	void setEthereal(int ethereal);

	bool isConfused() const;
	void setConfused(int confused);

	bool isBuffed() const;
	void setBuffed(int buffed);

	bool isInvulnerable() const;
	void setInvulnerable(bool invulnerable);

	bool isStuck() const;
	void setStuck(int stuck);
	
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
	std::shared_ptr<Weapon> m_wep;
	std::string m_name;
	std::string m_image;
	
	// Keeps track of any afflictions inflicted on the actor
	std::vector<std::shared_ptr<Afflictions>> m_afflictions;

	bool m_isPlayer = false;
	bool m_isMonster = true;
	bool m_lavaImmune = false;
	bool m_bloodlust = false;
	bool m_flying = false;
	bool m_sturdy = true;

	bool m_burnable;
	bool m_stunnable;
	bool m_bleedable;
	bool m_healable;
	bool m_freezable;
	bool m_poisonable = true;

	bool m_burned;
	bool m_bled;
	bool m_healed;
	bool m_stunned;
	bool m_frozen;
	bool m_poisoned = false;
	bool m_stuck = false;

	bool m_invisible = false;
	bool m_ethereal = false;
	bool m_confused = false;
	bool m_buffed = false;
	bool m_invulnerable = false;

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
	int getMoneyMultiplier() const;
	void setMoneyMultiplier(int multiplier);
	float getMoneyBonus() const;
	void setMoneyBonus(float bonus);
	void increaseMoneyBonus();
	void decreaseMoneyBonus();
	int getMaxMoneyBonus() const;
	std::vector<std::shared_ptr<Weapon>>& getWeapons();
	std::vector<std::shared_ptr<Drops>>& getItems();
	int getMaxWeaponInvSize() const;
	int getMaxItemInvSize() const;

	bool hasShield() const;
	void setShieldPossession(bool possesses);
	std::shared_ptr<Shield>& getShield();
	std::shared_ptr<Shield> getPlayerShield() const;
	void equipShield(Dungeon &dungeon, std::shared_ptr<Shield> shield, bool shop = false);
	void dropShield(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols);
	void shieldBroken();
	void repairShield(int repairs);
	bool isBlocking() const;
	void setBlock(bool blocking);
	void blocked();
	bool shieldCoverage(int mx, int my);
	char facingDirection() const;
	void setFacingDirection(char facing);
	char getAction() const;
	void setAction(char action);

	void addWeapon(std::shared_ptr<Weapon> weapon);
	void wield(int index);
	void addItem(std::shared_ptr<Drops> drop);
	void use(Dungeon &dungeon, _Tile &tile, int index); // new version

	bool hasTrinket() const;
	void setTrinketFlag(bool hasTrinket);
	std::shared_ptr<Trinket>& getTrinket();
	void equipTrinket(Dungeon &dungeon, std::shared_ptr<Trinket> trinket, bool shop = false);
	void swapTrinket(Dungeon& dungeon, std::shared_ptr<Trinket> trinket, bool shop = false);

	void setItemToFront(int index); // used for item quick access

	int getVision() const;
	void setVision(int vision);

	bool canLifesteal() const;
	void setLifesteal(bool steal);
	void rollHeal();

	bool hasSkeletonKey() const;
	void checkKeyConditions(std::vector<std::string> &text);
	int keyHP() const;
	void setKeyHP(int keyhp);

	void setWin(bool win);
	bool getWin() const;
	std::string getDeath() const;
	void setDeath(std::string m);

private:
	std::vector<std::shared_ptr<Weapon>> m_weapons;
	std::vector<std::shared_ptr<Drops>> m_items;

	int m_maxhp;
	int m_maxarmor = 99;
	int m_maxstr = 99;
	int m_maxdex = 99;
	int m_maxwepinv = 5;
	int m_maxiteminv = 5;
	int m_money = 0;
	int m_moneyMultiplier = 1; // money multiplied upon monster death

	// flat extra money, not based on multiplier
	// increases by 0.25 upon successful hit; decreases by 0.50 if hit
	float m_moneyBonus = 0;
	int m_maxMoneyBonus = 10;

	int m_vision = 5;
	bool m_lifesteal = false;

	std::shared_ptr<Shield> m_shield;
	bool m_hasShield = false;
	bool m_blocking;
	char m_facing;
	char m_action = '-'; // used for non-movement actions

	std::shared_ptr<Trinket> m_trinket;
	bool m_hasTrinket = false;


	// keyhp is the limit before the skeleton key breaks.
	// it is set below the key's set minimum if player is already below this minimum threshold when they pick it up
	int m_keyhp;

	bool m_winner;
	std::string m_death;
};



class NPC : public Actors {
public:
	NPC(int x, int y, std::string name, std::string image);

	void talk(Dungeon& dungeon);
	virtual void checkSatisfaction(Dungeon &dungeon) { ; };
	virtual void reward(Dungeon& dungeon) { ; };

	void playDialogue(Dungeon &dungeon);
	void setDialogue(std::vector<std::string> dialogue);
	virtual void addInteractedDialogue(std::vector<std::string> &dialogue) { ; };
	virtual void addSatisfiedDialogue(std::vector<std::string> &dialogue) { ; };
	virtual void addFinalDialogue(std::vector<std::string> &dialogue) { ; };
	void setSatisfaction(bool satisfied);

private:
	std::vector<std::string> m_dialogue;

	bool m_interacted = false; // flag for initial interaction with player
	bool m_satisfied = false; // flag for determining if player has met the NPCs request
	bool m_rewardGiven = false; // flag so that players are not given more than one reward
};

class CreatureLover : public NPC {
public:
	CreatureLover(int x, int y);

	void checkSatisfaction(Dungeon& dungeon);
	void reward(Dungeon& dungeon);

	void addInteractedDialogue(std::vector<std::string> &dialogue);
	void addSatisfiedDialogue(std::vector<std::string> &dialogue);
	void addFinalDialogue(std::vector<std::string> &dialogue);

private:
	std::string m_wantedCreature;
	std::shared_ptr<Monster> m_creature = nullptr;
};



class Monster : public Actors {
public:
	Monster();
	Monster(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name, 
		bool burnable = true, bool bleedable = true, bool healable = true, bool stunnable = true, bool freezable = true);
	Monster(int x, int y, int hp, int armor, int str, int dex, std::string name, bool burnable = true, bool bleedable = true, bool healable = true, bool stunnable = true, bool freezable = true);
	~Monster();

	virtual void move(Monster &m, Dungeon &dungeon);
	virtual void move(Dungeon &dungeon) { ; };
	virtual void encounter(Player &p, Monster &m, std::vector<std::string> &text);
	virtual void encounter(Dungeon &dungeon, Player &p, Monster &m);
	virtual void encounter(Monster &m1, Monster &m2);
	virtual void attack(Player &p, std::vector<std::string> &text);
	virtual void attack(Dungeon &dungeon, Player &p);
	virtual void attack(Monster &m) { ; };
	virtual void death(Monster &m, Dungeon &dungeon);
	virtual void death(Dungeon &dungeon) { ; };
	void destroy(Dungeon &dungeon); // same as destroyMonster in Dungeon

	bool attemptChase(std::vector<_Tile> &dungeon, int maxcols, int &shortest, int smelldist, int origdist, int x, int y, char &first_move, char &optimal_move);
	bool attemptSmartChase(Dungeon &dungeon, int cols, int &shortest, int currentDist, int origDist, int x, int y, char &first_move, char &optimal_move);
	bool playerInRange(Player p, int range);
	bool playerInDiagonalRange(Player p, int range);
	void moveWithSuccessfulChase(Dungeon &dungeon, char move);
	void moveMonsterRandomly(Dungeon &dungeon);
	bool wallCollision(Dungeon &dungeon, char direction, int p_move, int m_move);
	bool clearLineOfSight(Dungeon &dungeon, Player &p);

	bool chasesPlayer() const;
	void setChasesPlayer(bool chases);

	bool isSmart() const; // tells if monster (that chases), will or will not walk on lethal traps
	void setSmart(bool smart);

	bool hasExtraSprites();
	void setExtraSpritesFlag(bool extras);
	virtual void setSpriteColor(cocos2d::Color3B color) { ; };
	bool emitsLight() const; // tells if the sprite should emit extra light
	void setEmitsLight(bool emits);

private:
	bool m_chases = false;
	bool m_smart = false;
	bool m_hasExtraSprites = false;
	bool m_emitsLight = false;
};

class Goblin : public Monster {
public:
	Goblin(int x, int y, int smelldist);

	void move(Dungeon &dungeon);
	void death(Dungeon &dungeon);

	int getSmellDistance() const;
private:
	int m_smelldist;
};

class Wanderer : public Monster {
public:
	Wanderer(int x, int y);

	void move(Dungeon &dungeon);
	void death(Dungeon &dungeon);
};

class Archer : public Monster {
public:
	Archer(int x, int y);
	
	void move(Dungeon &dungeon);
	void death(Dungeon &dungeon);

	bool isPrimed() const;
	void prime(bool p);
private:
	bool m_primed;
};

class Zapper : public Monster {
public:
	Zapper(int x, int y, int rows);
	~Zapper();

	void move(Dungeon &dungeon);
	void attack(Player &p, std::vector<std::string> &text);
	void death(Dungeon &dungeon);

	bool onCooldown() const;
	void setCooldown();
	bool cardinalAttack() const;
	void swapDirection();
	std::map<int, cocos2d::Sprite*> getSparks();
	void moveSprites(int x, int y, int rows);
	void setSpriteColor(cocos2d::Color3B color);

private:
	bool m_cooldown;
	bool m_attack;

	std::map<int, cocos2d::Sprite*> sparks;
};

class Spinner : public Monster {
public:
	Spinner(int x, int y, int rows);
	~Spinner();

	void move(Dungeon &dungeon);
	void attack(Player &p, std::vector<std::string> &text);
	void death(Dungeon &dungeon);

	void setInitialFirePosition(int x, int y, int rows);
	void setFirePosition(char move);

	bool isClockwise() const;
	int getAngle() const;
	void setAngle(int angle);

	bool playerWasHit(const Actors &a);

	cocos2d::Sprite* getInner() const;
	cocos2d::Sprite* getOuter() const;
	void setSpriteVisibility(bool visible);
	void setSpriteColor(cocos2d::Color3B color);

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
	Bombee(int x, int y);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);
	void death(Dungeon &dungeon);

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
	MountedKnight(int x, int y);

	void move(Dungeon &dungeon);
	void step(Dungeon &dungeon);
	void death(Dungeon &dungeon);

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
	Roundabout(int x, int y);
	Roundabout(int x, int y, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move(Dungeon &dungeon);
	void attack(Player &p, std::vector<std::string> &text);
	void death(Dungeon &dungeon);

	char getDirection() const;
	void setDirection(char dir);

private:
	char m_direction;
};

class FireRoundabout : public Roundabout {
public:
	FireRoundabout(int x, int y);

	void move(Dungeon &dungeon);
	void death(Dungeon &dungeon);
};

class Seeker : public Monster {
public:
	Seeker(int x, int y, int range);
	Seeker(int x, int y, int range, std::string name, std::string image);

	void move(Dungeon &dungeon);
	void death(Dungeon &dungeon);

	int getRange() const;
	bool getStep() const;
	void toggleStep();
private:
	int m_range;
	bool m_step;
};

class DeadSeeker : public Seeker {
public:
	DeadSeeker(int x, int y, int range = 13);

	void death(Dungeon &dungeon);

private:
	int m_range;
	bool m_step;
};

class ItemThief : public Monster {
public:
	ItemThief(int x, int y, int range = 7);

	void move(Dungeon &dungeon);
	void run(Dungeon& dungeon);
	void attack(Dungeon &dungeon, Player &p);
	void death(Dungeon &dungeon);

	int getRange() const;

private:
	bool m_stole = false; // flag for telling that they successfully stole from the player
	std::shared_ptr<Objects> m_stolenItem = nullptr;
	int m_stolenGold = 0;
	int m_range;
};

class GooSack : public Monster {
public:
	// This enemy is stationary, but if the player is close, it will attempt to jump on them
	GooSack(int x, int y);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);
	void death(Dungeon &dungeon);

private:
	bool m_primed = false;

	// coordinates for where the player was when this was primed
	int m_x;
	int m_y;
};

class Broundabout : public Roundabout {
public:
	// This enemy moves in straight lines, back and forth. However, like the GooSack, it will try to jump onto nearby players.
	// After jumping it will continue moving in the direction it was originally moving in, unless the player is still nearby.
	Broundabout(int x, int y);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);
	void death(Dungeon &dungeon);

private:
	bool m_primed = false;

	// coordinates for where the player was when this was primed
	int m_x;
	int m_y;
};



//	BOSSES
class Smasher : public Monster {
public:
	Smasher();

	void attack(Player &p, std::vector<std::string> &text);
	void attack(Monster &m);
	void death(Dungeon &dungeon);

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