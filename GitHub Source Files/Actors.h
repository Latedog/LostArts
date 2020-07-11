#ifndef ACTORS_H
#define ACTORS_H

#include <vector>

struct _Tile;
class Dungeon;
class FirstBoss;
class Afflictions;
class Weapon;
class Monster;
class Objects;
class Shield;
class Spell;
class Drops;
class Trinket;
class Passive;
class cocos2d::Sprite;
class Rocks;

class Actors {
public:
	Actors();
	Actors(int x, int y, int hp, int armor, int str, int dex, std::string name);
	Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep);
	Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name);
	virtual ~Actors();

	virtual void moveTo(Dungeon &dungeon, int x, int y, float time = 0.10f) { return; };

	int getPosX() const { return m_x; };
	int getPosY() const { return m_y; };
	int getMaxHP() const { return m_maxhp; };
	int getHP() const { return m_hp; };
	int getArmor() const { return m_armor; };
	int getStr() const { return m_str; };
	int getDex() const { return m_dex; };
	int getInt() const { return m_int; };
	int getLuck() const { return m_luck; };
	virtual std::shared_ptr<Weapon>& getWeapon() { return m_wep; };
	std::string getName() const { return m_name; };
	cocos2d::Sprite* getSprite() { return m_sprite; };
	std::string getImageName() const { return m_image; };
	bool hasAnimation() const { return m_hasAnimation; };
	std::string getAnimationFrames() const { return m_frames; };
	int getAnimationFrameCount() const { return m_frameCount; };
	int getFrameInterval() const { return m_frameInterval; };

	void setPosX(int x);
	void setPosY(int y);
	void setHP(int hp);
	void setMaxHP(int maxhp);
	void setArmor(int armor);
	void setStr(int str);
	void setDex(int dex);
	void setInt(int intellect);
	void setLuck(int luck);
	void setWeapon(std::shared_ptr<Weapon> wep);
	void setName(std::string name);
	void setSprite(cocos2d::Sprite* sprite);

	bool isPlayer() const { return m_isPlayer; };
	bool isMonster() const { return m_isMonster; };
	bool isDead() const { return m_isDead; };
	void setDead(bool dead) { m_isDead = dead; };

	//	AFFLICTIONS
	void checkAfflictions();
	int findAffliction(std::string name);
	void addAffliction(std::shared_ptr<Afflictions> affliction);
	void removeAffliction(std::string name);

	bool lavaImmune() const { return m_lavaImmune; }; // immunity to lava
	bool isFlying() const { return m_flying; };
	bool isSturdy() const { return m_sturdy; }; // tells if this gets knocked back by attacks or not
	bool isHeavy() const { return m_heavy; }; // Indicates if they can be pushed

	bool hasBloodlust() const { return m_bloodlust; };
	bool hasToxic() const { return m_toxic; };

	void setLavaImmunity(bool immune);
	void setFlying(bool flying);
	void setSturdy(bool sturdy);
	void setHeavy(bool heavy) { m_heavy = heavy; };

	void setBloodlust(bool lust);
	void setToxic(bool toxic); // can poison others

	bool canBeStunned() const { return m_stunnable; };
	bool canBeBurned() const { return m_burnable; };
	bool canBeBled() const { return m_bleedable; };
	bool canBeHealed() const { return m_healable; };
	bool canBeFrozen() const { return m_freezable; };
	bool canBePoisoned() const { return m_poisonable; };

	void setCanBeStunned(bool stunnable);
	void setCanBeBurned(bool burnable);
	void setCanBeBled(bool bleedable);
	void setCanBeHealed(bool healable);
	void setCanBeFrozen(bool freezable);
	void setCanBePoisoned(bool poisonable);

	bool isBurned() const { return m_burned; };
	bool isBled() const { return m_bled; };
	bool isStunned() const { return m_stunned; };
	bool isFrozen() const { return m_frozen; };
	bool isPoisoned() const { return m_poisoned; };
	bool isInvisible() const { return m_invisible; };
	bool isEthereal() const { return m_ethereal; };
	bool isConfused() const { return m_confused; };
	bool isBuffed() const { return m_buffed; };
	bool isInvulnerable() const { return m_invulnerable; };
	bool isStuck() const { return m_stuck; };
	bool isPossessed() const { return m_possessed; };
	bool isCrippled() const { return m_crippled; };

	void setBurned(bool burned);
	void setBleed(bool bled);
	void setStunned(bool stun);
	void setFrozen(bool freeze);
	void setPoisoned(bool poisoned);
	void setInvisible(bool invisible);
	void setEthereal(bool ethereal);
	void setConfused(bool confused);
	void setBuffed(bool buffed);
	void setInvulnerable(bool invulnerable);
	void setStuck(bool stuck);
	void setPossessed(bool possessed) { m_possessed = possessed; };
	void setCrippled(bool crippled) { m_crippled = crippled; };

protected:
	void setrandPosX(int maxcols);
	void setrandPosY(int maxrows);
	void setImageName(std::string image);
	void setHasAnimation(bool hasAnimation);
	void setAnimationFrames(std::string frames);
	void setAnimationFrameCount(int count);
	void setFrameInterval(int interval);

	void setPlayerFlag(bool player);
	void setMonsterFlag(bool monster);
	
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

	bool m_hasAnimation = false;
	std::string m_frames;
	int m_frameCount = 0;
	int m_frameInterval = 0;
	
	// Keeps track of any afflictions inflicted on the actor
	std::vector<std::shared_ptr<Afflictions>> m_afflictions;

	bool m_isDead = false;
	bool m_isPlayer = false;
	bool m_isMonster = true;
	bool m_lavaImmune = false;
	bool m_bloodlust = false;
	bool m_flying = false;
	bool m_sturdy = true;
	bool m_heavy = false;

	bool m_toxic = false;

	bool m_burnable = true;
	bool m_stunnable = true;
	bool m_bleedable = true;
	bool m_healable = true;
	bool m_freezable = true;
	bool m_poisonable = true;

	bool m_burned = false;
	bool m_bled = false;
	bool m_healed = false;
	bool m_stunned = false;
	bool m_frozen = false;
	bool m_poisoned = false;
	bool m_stuck = false;
	bool m_crippled = false;

	bool m_invisible = false;
	bool m_ethereal = false;
	bool m_confused = false;
	bool m_buffed = false;
	bool m_invulnerable = false;
	bool m_possessed = false;

	cocos2d::Sprite* m_sprite = nullptr;
};


// ============================================
//				:::: PLAYERS ::::
// ============================================

class Player : public Actors {
public:
	Player(int hp, std::shared_ptr<Weapon> weapon);
	~Player();

	void moveTo(Dungeon &dungeon, int x, int y, float time = 0.10f);
	void move(Dungeon &dungeon, char move);
	void attack(Dungeon &dungeon, Actors &a);
	virtual void successfulAttack(Dungeon &dungeon, Actors &a) = 0;
	void botchedAttack(Dungeon &dungeon, Actors &a);

	int getMoney() const { return m_money; };
	void setMoney(int money) { m_money = money; };
	int getMoneyMultiplier() const { return m_moneyMultiplier; };
	void setMoneyMultiplier(int multiplier) { m_moneyMultiplier = multiplier; };
	float getMoneyBonus() const { return m_moneyBonus; };
	void setMoneyBonus(float bonus) { m_moneyBonus = bonus; };
	void increaseMoneyBonus() {
		if (getMoneyBonus() < getMaxMoneyBonus()) {
			float bonus = 0.20f + (hasBetterRates() ? 0.10f : 0.0f);
			if (m_moneyBonus + bonus < m_maxMoneyBonus)
				setMoneyBonus(getMoneyBonus() + bonus);
			else
				setMoneyBonus(m_maxMoneyBonus);
		}
	};
	void decreaseMoneyBonus() { // decrease money bonus when hit
		float penalty = 0.6f - (hasBetterRates() ? 0.3f : 0.0f);
		if (getMoneyBonus() - penalty > 0)
			setMoneyBonus(getMoneyBonus() - penalty);
		else if (getMoneyBonus() - penalty <= 0) {
			setMoneyBonus(0);
		}
	};
	int getMaxMoneyBonus() const { return m_maxMoneyBonus; };
	std::vector<std::shared_ptr<Weapon>>& getWeapons() { return m_weapons; };
	std::vector<std::shared_ptr<Drops>>& getItems() { return m_items; };
	int getMaxWeaponInvSize() const { return m_maxwepinv; };
	int getMaxItemInvSize() const { return m_maxiteminv; };

	// Active item (Spacebar)
	bool hasActiveItem() const { return m_hasActiveItem; };
	virtual void useActiveItem(Dungeon &dungeon) = 0;
	virtual void equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop = false) = 0;
	std::shared_ptr<Objects> getActiveItem() const { return m_activeItem; };

	// Used for HUD updating
	virtual bool activeHasMeter() const = 0; // For indicating if the HUD should display a meter
	virtual int getCurrentActiveMeter() const { return 0; }; // Active item's current durability/points
	virtual int getMaxActiveMeter() const { return -1; }; // Active item's maximum durability/points

	virtual bool activeHasAbility() const = 0;
	virtual void useActiveAbility(Dungeon &dungeon, Actors &a) { return; };
	virtual void restoreActive(int repairs) { return; };

	virtual bool canUseShield() const = 0;

	// General block functions
	virtual bool canBlock() const = 0;
	virtual bool didBlock(int x, int y) const { return false; };
	virtual void successfulBlock() { return; };
	virtual int blockedDamageReduction() { return 0; };

	virtual void setBlock(bool blocking) { m_blocking = blocking; };

	// Unused
	void addWeapon(std::shared_ptr<Weapon> weapon);
	void wield(int index);
	///

	void storeWeapon(Dungeon &dungeon, std::shared_ptr<Weapon> weapon, bool shop = false);
	void switchWeapon();
	void throwWeaponTo(Dungeon &dungeon, int x, int y);
	std::shared_ptr<Weapon> getStoredWeapon() const { return m_storedWeapon; };

	void addItem(std::shared_ptr<Drops> drop);
	void addItem(std::shared_ptr<Drops> drop, bool &itemAdded); // New for item collection
	void use(Dungeon &dungeon, int index);

	void equipPassive(Dungeon &dungeon, std::shared_ptr<Passive> passive);
	std::vector<std::shared_ptr<Passive>> getPassives() const { return m_passives; };

	bool hasTrinket() const { return m_hasTrinket; };
	void setTrinketFlag(bool hasTrinket) { m_hasTrinket = hasTrinket; };
	std::shared_ptr<Trinket>& getTrinket() { return m_trinket; };
	void equipTrinket(Dungeon &dungeon, std::shared_ptr<Trinket> trinket, bool shop = false);
	void swapTrinket(Dungeon& dungeon, std::shared_ptr<Trinket> trinket, bool shop = false);

	char facingDirection() const { return m_facing; };
	void setFacingDirection(char facing) { m_facing = facing; };
	char getAction() const { return m_action; };
	void setAction(char action) { m_action = action; };

	void setItemToFront(int index); // used for item quick access

	int getVision() const { return m_vision; };
	void setVision(int vision) { m_vision = vision; };

	// Dual wielding bonus
	bool isDualWielding() const { return m_dualWield; };
	void setDualWielding(bool dual) { m_dualWield = dual; };

	// Special abilities gained from passives
	bool canLifesteal() const { return m_lifesteal; };
	void setLifesteal(bool steal) { m_lifesteal = steal; };
	void rollHeal();

	bool hasSteelPunch() const { return m_steelPunch; };
	void setSteelPunch(bool punch) { m_steelPunch = punch; };

	bool spikeImmunity() const { return m_spikeImmunity; };
	void setSpikeImmunity(bool immune) { m_spikeImmunity = immune; };

	bool hasPoisonTouch() const { return m_poisonTouch; };
	void setPoisonTouch(bool poison) { m_poisonTouch = poison; };

	bool hasFireTouch() const { return m_fireTouch; };
	void setFireTouch(bool fire) { m_fireTouch = fire; };

	bool hasFrostTouch() const { return m_frostTouch; };
	void setFrostTouch(bool frost) { m_frostTouch = frost; };

	bool explosionImmune() const { return m_explosionImmunity; };
	void setExplosionImmune(bool explosion) { m_explosionImmunity = explosion; };

	bool hasPotentPotions() const { return m_potentPotions; };
	void setPotentPotions(bool potent) { m_potentPotions = potent; };

	bool hasSoulSplit() const { return m_soulSplit; };
	void setSoulSplit(bool soul) { m_soulSplit = soul; };

	bool canBreakWalls() const { return m_wallBreak; };
	void setCanBreakWalls(bool wallBreak) { m_wallBreak = wallBreak; }

	bool hasCharismaNPC() const { return m_charismaNPC; };
	void setCharismaNPC(bool charisma) { m_charismaNPC = charisma; };
	
	bool hasCheapShops() const { return m_cheapShops; };
	void setCheapShops(bool cheap) { m_cheapShops = cheap; };

	bool hasBetterRates() const { return m_betterRates; };
	void setBetterRates(bool rates) { m_betterRates = rates; };

	bool hasTrapIllumination() const { return m_trapIllumination; };
	void setTrapIllumination(bool illuminate) { m_trapIllumination = illuminate; };

	bool hasItemIllumination() const { return m_itemIllumination; };
	void setItemIllumination(bool illuminate) { m_itemIllumination = illuminate; };

	bool hasMonsterIllumination() const { return m_monsterIllumination; };
	void setMonsterIllumination(bool illuminate) { m_monsterIllumination = illuminate; };

	bool hasResonantSpells() const { return m_resonantSpells; };
	void setResonantSpells(bool resonant) { m_resonantSpells = resonant; };

	bool isSlow() const { return m_slow; };
	void setSlow(bool slow) { m_slow = slow; };
	/// End special abilities

	bool hasSkeletonKey() const;
	void checkKeyConditions();
	int keyHP() const { return m_keyhp; };
	void setKeyHP(int keyhp) { m_keyhp = keyhp; };

	bool getWin() const { return m_winner; };
	void setWin(bool win) { m_winner = win; };
	std::string getDeath() const { return m_death; };
	void setDeath(std::string m) { m_death = m; };

protected:
	void checkExtraAttackEffects(Dungeon &dungeon, Actors &a);
	void knockbackDirection(int &x, int &y, int mx, int my); // Determines the direction the enemy should be knocked back

	void dropWeapon(Dungeon &dungeon, bool shop);
	void dropStoredWeapon(Dungeon &dungeon); // Used mainly if player was dual wielding
	void addSecondaryWeapon(std::shared_ptr<Weapon> weapon) { m_storedWeapon = weapon; }; // For giving chracters a second starting weapon

	void setHasActiveItem(bool active) { m_hasActiveItem = active; };
	void setActiveItem(std::shared_ptr<Objects> active) { m_activeItem = active; };

private:
	std::vector<std::shared_ptr<Weapon>> m_weapons;
	std::shared_ptr<Weapon> m_storedWeapon = nullptr;

	bool m_hasActiveItem;
	std::shared_ptr<Objects> m_activeItem = nullptr;

	std::vector<std::shared_ptr<Drops>> m_items;
	std::vector<std::shared_ptr<Passive>> m_passives;

	std::shared_ptr<Trinket> m_trinket = nullptr;
	bool m_hasTrinket = false;

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

	bool m_dualWield = false;

	// Special abilities
	bool m_lifesteal = false;
	bool m_steelPunch = false; // All enemies become unsturdy
	bool m_spikeImmunity = false;
	bool m_poisonTouch = false;
	bool m_fireTouch = false;
	bool m_frostTouch = false;
	bool m_explosionImmunity = false;
	bool m_potentPotions = false;
	bool m_soulSplit = false; // Damage split between hp and money
	bool m_wallBreak = false; // Allows player to destroy walls
	bool m_charismaNPC = false; // Increases NPC spawn rate
	bool m_cheapShops = false; // Reduces shop prices
	bool m_betterRates = false; // Reduces money penalty when hit and increases money gain on hit
	bool m_trapIllumination = false; // Illuminates certain traps
	bool m_itemIllumination = false; // Illuminates item locations
	bool m_monsterIllumination = false; // Illuminates monsters
	bool m_resonantSpells = false; // Spells have a chance to not be consumed on use
	bool m_slow = false; // Slowness means that monsters get to move before the player

	
	bool m_blocking = false;

	char m_facing = 'r'; // Choose 'right' arbitrarily as the default looking direction
	char m_action = '-'; // used for non-movement actions

	// keyhp is the limit before the skeleton key breaks.
	// it is set below the key's set minimum if player is already below this minimum threshold when they pick it up
	int m_keyhp;

	bool m_winner = false;
	std::string m_death;
};

class Adventurer : public Player {
public:
	// The standard character. Wields a shield.
	Adventurer();

	void useActiveItem(Dungeon &dungeon);
	void equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop = false);
	bool activeHasMeter() const { return true; };
	int getCurrentActiveMeter() const;
	int getMaxActiveMeter() const;

	bool activeHasAbility() const;
	void useActiveAbility(Dungeon &dungeon, Actors &a);
	
	bool canUseShield() const { return true; };
	bool hasShield() const { return m_hasShield; };
	
	void equipShield(Dungeon &dungeon, std::shared_ptr<Shield> shield, bool shop = false);
	void dropShield(std::vector<_Tile> &dungeon, const int cols);
	void shieldBroken();
	void restoreActive(int repairs);
	void setBlock(bool blocking) { m_blocking = blocking; };
	
	bool canBlock() const;
	bool didBlock(int mx, int my) const;
	int blockedDamageReduction();
	void successfulBlock();

	void successfulAttack(Dungeon &dungeon, Actors &a) { return; };

private:
	std::shared_ptr<Shield> m_shield;
	bool m_hasShield = false;
	bool m_blocking = false;
};

class Spellcaster : public Player {
public:
	// Starts with Whirlwind as active item. Can only use spells.
	// Picking up other spells replaces the Spellcaster's current spell.
	Spellcaster();

	void useActiveItem(Dungeon &dungeon);
	void equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop = false);
	bool activeHasMeter() const { return true; };
	int getCurrentActiveMeter() const { return m_mp; };
	int getMaxActiveMeter() const { return m_maxMP; };
	bool activeHasAbility() const { return false; };

	bool canBlock() const { return false; };
	bool canUseShield() const { return false; };
	void successfulAttack(Dungeon &dungeon, Actors &a);

	int getMP() const { return m_mp; };
	int getMaxMP() const { return m_maxMP; };

private:
	std::shared_ptr<Spell> m_spell;
	int m_mp = 25; // magic casting points left
	int m_maxMP = 25;
};

class Spelunker : public Player {
public:
	// Starts with the Whip and uses Rocks as their active item.
	// Rocks can break, so all enemies have a chance to drop Rocks.
	Spelunker();

	void useActiveItem(Dungeon &dungeon);
	void equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop = false);

	int getRockCount() const { return m_rocks.size(); }; // For HUD purposes

	bool activeHasMeter() const { return false; };
	bool activeHasAbility() const { return false; };

	bool canBlock() const { return false; };
	bool canUseShield() const { return false; };

	void successfulAttack(Dungeon &dungeon, Actors &a) { return; };

private:
	std::vector<std::shared_ptr<Rocks>> m_rocks;
};

class TheMadman : public Player {
public:
	// Active item is a teleporter. Cannot be replaced.
	// Starts with no weapon but increased vision radius.
	TheMadman();

	void useActiveItem(Dungeon &dungeon);
	void equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop = false);
	bool activeHasMeter() const { return false; };
	int getCurrentActiveMeter() const { return 0; };
	int getMaxActiveMeter() const { return -1; };
	bool activeHasAbility() const { return false; };

	bool canBlock() const { return false; };
	bool canUseShield() const { return false; };
	void successfulAttack(Dungeon &dungeon, Actors &a) { return; };

private:
	std::shared_ptr<Drops> m_item;
};


// ============================================
//			 	  :::: NPCS ::::
// ============================================

class NPC : public Actors {
public:
	NPC(int x, int y, std::string name, std::string image);

	void talk(Dungeon& dungeon);
	virtual void checkSatisfaction(Dungeon &dungeon) = 0;
	virtual void reward(Dungeon& dungeon) = 0;

	void setDialogue(std::vector<std::string> dialogue) { m_dialogue = dialogue; };
	void setChoices(std::vector<std::string> choices) { m_promptChoices = choices; };
	virtual void addInteractedDialogue(std::vector<std::string> &dialogue) = 0;
	virtual void addSatisfiedDialogue(std::vector<std::string> &dialogue) = 0;
	virtual void addFinalDialogue(std::vector<std::string> &dialogue) = 0;
	void setSatisfaction(bool satisfied) { m_satisfied = satisfied; };

	std::vector<std::string> getDialogue() const { return m_dialogue; };
	std::vector<std::string> getChoices() const { return m_promptChoices; };
	void useResponse(int index) { useResponse(m_dialogue, index); }; // Index of the choice that the player made in the choices vector

protected:
	void playDialogue(Dungeon &dungeon);
	virtual void useResponse(std::vector<std::string> &dialogue, int index) { return; }; // Index of the choice that the player made in the choices vector

	void rewardWasGiven() { m_rewardGiven = true; };

	int getInteractionStage() const { return m_interactedDialogueStage; };
	void incInteractionStage() { m_interactedDialogueStage++; };
	int getInteractionLimit() const { return m_interactionLimit; };
	void setInteractionLimit(int limit) { m_interactionLimit = limit; };

private:
	std::vector<std::string> m_dialogue;
	std::vector<std::string> m_promptChoices;

	bool m_interacted = false; // flag for initial interaction with player
	bool m_satisfied = false; // flag for determining if player has met the NPCs request
	bool m_rewardGiven = false; // flag so that players are not given more than one reward

	// When an NPC is continuously interacted with, this keeps track of what lines to output next
	// if the player keeps talking to them, up to a certain limit
	int m_interactedDialogueStage = 1;
	int m_interactionLimit;
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

class Memorizer : public NPC {
public:
	Memorizer(Dungeon *dungeon, int x, int y);

	void useResponse(std::vector<std::string> &dialogue, int index);
	void checkSatisfaction(Dungeon& dungeon);
	void reward(Dungeon& dungeon);

	void addInteractedDialogue(std::vector<std::string> &dialogue);
	void addSatisfiedDialogue(std::vector<std::string> &dialogue);
	void addFinalDialogue(std::vector<std::string> &dialogue);

private:
	Dungeon *m_dungeon = nullptr;
	std::string m_topic;
	std::string m_correctChoice;

	// Since there are multiple prompts, this indicates what stage of prompt we're at so that we use
	// the appropriate set of choices.
	int m_promptStage = 1;
};

class Shopkeeper : public NPC {
public:
	Shopkeeper(int x, int y);

	void checkSatisfaction(Dungeon& dungeon);
	void reward(Dungeon& dungeon);

	void addInteractedDialogue(std::vector<std::string> &dialogue);
	void addSatisfiedDialogue(std::vector<std::string> &dialogue);
	void addFinalDialogue(std::vector<std::string> &dialogue);

private:

};

class OutsideMan1 : public NPC {
public:
	// A man that hangs outside the entrance to the world hub
	OutsideMan1(int x, int y);

	void checkSatisfaction(Dungeon& dungeon) { return; };
	void reward(Dungeon& dungeon) { return; };

	void addInteractedDialogue(std::vector<std::string> &dialogue);
	void addSatisfiedDialogue(std::vector<std::string> &dialogue) { return; };
	void addFinalDialogue(std::vector<std::string> &dialogue) { return; };
};

class OutsideMan2 : public NPC {
public:
	// A man that hangs outside the entrance to the world hub
	OutsideMan2(int x, int y);

	void checkSatisfaction(Dungeon& dungeon) { return; };
	void reward(Dungeon& dungeon) { return; };

	void addInteractedDialogue(std::vector<std::string> &dialogue);
	void addSatisfiedDialogue(std::vector<std::string> &dialogue) { return; };
	void addFinalDialogue(std::vector<std::string> &dialogue) { return; };

protected:
	void useResponse(std::vector<std::string> &dialogue, int index);
};

class OutsideWoman1 : public NPC {
public:
	// A woman that hangs outside the entrance to the world hub
	OutsideWoman1(int x, int y);

	void checkSatisfaction(Dungeon& dungeon) { return; };
	void reward(Dungeon& dungeon) { return; };

	void addInteractedDialogue(std::vector<std::string> &dialogue);
	void addSatisfiedDialogue(std::vector<std::string> &dialogue) { return; };
	void addFinalDialogue(std::vector<std::string> &dialogue) { return; };
};



// ============================================
//				:::: MONSTERS ::::
// ============================================

class Monster : public Actors {
public:
	Monster(int x, int y, int hp, int armor, int str, int dex, std::string name);
	Monster(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name);
	~Monster();

	// Moves the monster to the coordinates @x and @y
	virtual void moveTo(Dungeon &dungeon, int x, int y, float time = 0.10f);

	virtual void moveCheck(Dungeon& dungeon); // checks if monster is dead first before moving
	virtual void move(Dungeon &dungeon) = 0;
	virtual void attack(Dungeon &dungeon, Player &p);
	virtual void attack(Monster &m) { return; }; // for monsters attacking other monsters
	virtual void extraAttackEffects(Dungeon& dungeon) { return; };
	virtual void death(Dungeon &dungeon);
	virtual void deathDrops(Dungeon &dungeon) { return; };

	bool wasDestroyed() const { return m_destroyed; };
	void setDestroyed(bool destroyed) { m_destroyed = destroyed; };

	bool attemptChase(std::vector<_Tile> &dungeon, int maxcols, int &shortest, int smelldist, int origdist, int x, int y, char &first_move, char &optimal_move);
	bool attemptSmartChase(Dungeon &dungeon, int cols, int &shortest, int currentDist, int origDist, int x, int y, char &first_move, char &optimal_move);
	void attemptGreedyChase(Dungeon &dungeon, bool diagonals = false, int x = -1, int y = -1); // @x and @y are used specifically for decoys

	bool playerInRange(const Player& p, int range);
	bool playerInDiagonalRange(const Player& p, int range);
	bool playerIsAdjacent(const Player &p, bool diagonals = false);
	bool attemptMove(Dungeon &dungeon, char move);
	bool moveWithSuccessfulChase(Dungeon &dungeon, char move);
	char moveMonsterRandomly(Dungeon &dungeon);
	bool wallCollision(Dungeon &dungeon, char direction, int p_move, int m_move);
	bool clearLineOfSight(Dungeon &dungeon, const Player &p);

	virtual bool canBeDamaged(Dungeon &dungeon) { return true; }; // Indicates if the enemy can be damaged (not armored, etc.)

	bool chasesPlayer() const { return m_chases; };
	bool isSmart() const { return m_smart; }; // tells if monster (that chases), will or will not walk on lethal traps
	bool hasWeapon() const { return m_hasWeapon; }
	bool isMultiSegmented() const { return m_multiSegmented; }; // Tells if monster spans multiple tiles
	virtual bool isMonsterSegment() const { return false; } // Indicates if this is a monster segment. Used for ignoring traps.

	virtual bool isBreakable() const { return false; } // Indicates if this is a breakable object

	// @x and @y are the coordinates of the parent's previous position
	virtual void moveSegments(Dungeon &dungeon, int x, int y) { return; }; // For moving the segments of a monster if the parent monster was pushed

	virtual void addSegments(Dungeon &dungeon) { return; }; // Adds all segments of a monster if it is multi-segmented
	virtual void removeSegments(Dungeon &dungeon) { return; }; // Removes all segments of a monster if it is multi-segmented
	virtual int getSegmentX() const { return getPosX(); }; // X coordinate of the segmented monster's main part
	virtual int getSegmentY() const { return getPosY(); }; // Y coordinate of the segmented monster's main part

	bool hasExtraSprites() { return m_hasExtraSprites; };
	virtual void setSpriteColor(cocos2d::Color3B color) { return; };
	bool emitsLight() const { return m_emitsLight; }; // tells if the sprite should emit extra light
	virtual void addLightEmitters(const Dungeon &dungeon, std::vector<std::pair<int, int>> &lightEmitters) { return; };

protected:
	void setGold(int gold) { m_gold = gold; };
	void setChasesPlayer(bool chases) { m_chases = chases; };
	void setSmart(bool smart) { m_smart = smart; };
	void setHasWeapon(bool hasWeapon) { m_hasWeapon = hasWeapon; };
	void setMultiSegmented(bool segmented) { m_multiSegmented = segmented; };
	void setExtraSpritesFlag(bool extras) { m_hasExtraSprites = extras; };
	void setEmitsLight(bool emits) { m_emitsLight = emits; };

private:
	bool m_destroyed = false; // Indicates that monster should not drop any rewards

	int m_gold = 0;
	bool m_chases = false;
	bool m_smart = false;
	bool m_hasWeapon = false;
	bool m_multiSegmented = false;

	bool m_hasExtraSprites = false;
	bool m_emitsLight = false;
};

// Breakables
class Breakables : public Monster {
public:
	// Breakables can be destroyed and may contain items inside
	Breakables(int x, int y, int hp, int armor, std::string name, std::string image);

	void move(Dungeon &dungeon) { return; };

	bool isBreakable() { return true; };
	virtual bool canBeDamaged(Dungeon &dungeon) = 0;
};

class SturdyBreakables : public Breakables {
public:
	// These breakables do not move when hit
	SturdyBreakables(int x, int y, int hp, int armor, std::string name, std::string image);

};

class WeakCrate : public SturdyBreakables {
public:
	WeakCrate(int x, int y);

	bool canBeDamaged(Dungeon &dungeon) { return true; };
	void deathDrops(Dungeon &dungeon);
};

class WeakBarrel : public SturdyBreakables {
public:
	WeakBarrel(int x, int y);

	bool canBeDamaged(Dungeon &dungeon) { return true; };
	void deathDrops(Dungeon &dungeon);
};

class SmallPot : public SturdyBreakables {
public:
	SmallPot(int x, int y);

	bool canBeDamaged(Dungeon &dungeon) { return true; };
	void deathDrops(Dungeon &dungeon);
};

class LargePot : public SturdyBreakables {
public:
	LargePot(int x, int y);

	bool canBeDamaged(Dungeon &dungeon) { return true; };
	void deathDrops(Dungeon &dungeon);
};

class Sign : public SturdyBreakables {
public:
	Sign(int x, int y);

	bool canBeDamaged(Dungeon &dungeon) { return true; };
	void deathDrops(Dungeon &dungeon);
};

class ArrowSign : public SturdyBreakables {
public:
	ArrowSign(int x, int y);

	bool canBeDamaged(Dungeon &dungeon) { return true; };
	void deathDrops(Dungeon &dungeon);
};
///


class MonsterSegment : public Monster {
public:
	// Monster Segments are simply parts of a monster that can be attacked or interacted with.
	// They typically don't do anything other than moving around with the main monster's body section.
	MonsterSegment(int x, int y, int sx, int sy, std::string name, std::string image);

	void move(Dungeon &dungeon) { return; };

	bool isMonsterSegment() const { return true; }

	void setSegmentX(int x) { m_segmentX = x; };
	void setSegmentY(int y) { m_segmentY = y; };
	virtual int getSegmentX() const { return m_segmentX; }; // X coordinate of the segmented monster's main part
	virtual int getSegmentY() const { return m_segmentY; }; // Y coordinate of the segmented monster's main part

private:
	// Coordinates of the main monster to which it is attached
	int m_segmentX;
	int m_segmentY;
};

class Goblin : public Monster {
public:
	Goblin(int x, int y, int smelldist);

	void move(Dungeon &dungeon);
	void deathDrops(Dungeon &dungeon);

private:
	int m_smelldist;
};

class Wanderer : public Monster {
public:
	Wanderer(int x, int y);
	Wanderer(int x, int y, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move(Dungeon &dungeon);
	void deathDrops(Dungeon &dungeon);
};

class RabidWanderer : public Wanderer {
public:
	// Chance to poison the player
	RabidWanderer(int x, int y);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);
	void deathDrops(Dungeon &dungeon);

private:
	bool m_turn = false;
};

class SleepingWanderer : public Wanderer {
public:
	SleepingWanderer(int x, int y);

	void move(Dungeon &dungeon);
	void deathDrops(Dungeon &dungeon);

private:
	bool m_provoked = false;
	int m_range = 8;
	int m_provocationRange = 4;
};

class ProvocableWanderer : public Wanderer {
public:
	ProvocableWanderer(int x, int y);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);
	void deathDrops(Dungeon &dungeon);

private:
	bool m_provoked = false;
	int m_turns = 2;
	int m_prevHP;
};

class FlameWanderer : public Wanderer {
public:
	// Moves every other turn and has a chance to ignite.
	// If the player is adjacent, then this is guaranteed to attack.
	FlameWanderer(int x, int y);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);

private:
	bool m_turn = false;
};

class Archer : public Monster {
public:
	Archer(int x, int y, int range = 10);
	
	void move(Dungeon &dungeon);
	void deathDrops(Dungeon &dungeon);

private:
	bool m_primed;
	int m_range;
};

class Zapper : public Monster {
public:
	Zapper(int x, int y, int rows);
	~Zapper();

	void moveTo(Dungeon &dungeon, int x, int y, float time = 0.10f);
	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);

	bool onCooldown() const;
	void setCooldown();
	bool cardinalAttack() const;
	void swapDirection();
	std::map<int, cocos2d::Sprite*> getSparks();
	void moveSprites(int x, int y, int rows);
	void setSpriteColor(cocos2d::Color3B color);
	void addLightEmitters(const Dungeon &dungeon, std::vector<std::pair<int, int>> &lightEmitters);

private:
	bool m_cooldown;
	bool m_attack;

	std::map<int, cocos2d::Sprite*> sparks;
};

class Spinner : public Monster {
public:
	Spinner(int x, int y, int rows);
	~Spinner();

	void moveTo(Dungeon &dungeon, int x, int y, float time = 0.10f);
	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);

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
	void addLightEmitters(const Dungeon &dungeon, std::vector<std::pair<int, int>> &lightEmitters);

private:
	bool m_clockwise;
	int m_angle;

	std::shared_ptr<Objects> m_innerFire;
	std::shared_ptr<Objects> m_outerFire;

	cocos2d::Sprite* inner;
	cocos2d::Sprite* outer;
};

class Bombee : public Monster {
public:
	Bombee(int x, int y, int range = 11);
	Bombee(int x, int y, int range, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);
	void deathDrops(Dungeon &dungeon);

private:
	int m_fuse;
	bool m_fused;
	int m_aggroRange;
};

class CharredBombee : public Bombee {
public:
	// The main difference is that this bombee is immune to lava
	CharredBombee(int x, int y, int range = 10);

	void attack(Dungeon &dungeon, Player &p);
};

class MountedKnight : public Monster {
public:
	MountedKnight(int x, int y);

	void move(Dungeon &dungeon);
	void step(Dungeon &dungeon);
	void deathDrops(Dungeon &dungeon);

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
	void attack(Dungeon &dungeon, Player &p);

	char getDirection() const;
	void setDirection(char dir);

private:
	char m_direction;
};

class FireRoundabout : public Roundabout {
public:
	FireRoundabout(int x, int y);

	void move(Dungeon &dungeon);
	
};

class Seeker : public Monster {
public:
	Seeker(int x, int y, int range);
	Seeker(int x, int y, int range, std::string name, std::string image);

	void move(Dungeon &dungeon);

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

private:
	bool m_primed = false;

	// coordinates for where the player was when this was primed
	int m_x;
	int m_y;
};

class Charger : public Monster {
public:
	// Charger will attempt to dash toward the player (instantly after charging up).
	// They move every two turns toward the player and will destroy walls if they miss.
	// If they hit the player, then they are sent flying backward.
	Charger(int x, int y);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);

private:
	bool m_primed = false;
	int m_turns = 2; // Number of turns before moving
	char m_direction;
	int m_range = 8;
	int m_losRange = 5;
};

class Rat : public Monster {
public:
	// Rats run from the player if they're too close, but will chase the player
	// if they do not move on their turn.
	Rat(int x, int y, int range = 10);

	void move(Dungeon &dungeon);
	void run(Dungeon &dungeon);

private:
	int m_range;
	int m_prevPlayerX = -1;
	int m_prevPlayerY = -1;
};

class Toad : public Monster {
public:
	// Toads spit out poisonous puddles
	Toad(int x, int y);

	void move(Dungeon &dungeon);

private:
	// The `-` character indicates that the toad did not move on its previous turn
	char m_facing = '-';

	// The toad will cycle through the following moves:
	// Move randomly -> wait for three turns -> move twice randomly -> wait 1 turn -> roll to spit poison puddle -> repeat
	// This value cycles through from 1 to 8.
	int m_moveType = 1;
};

class Spider : public Monster {
public:
	// Spiders spin spiderwebs and chase down the player if they are caught in them
	// If the player is adjacent (all directions), then they will prime to attack on the next turn.
	Spider(Dungeon &dungeon, int x, int y, int webCount = 4);
	Spider(Dungeon &dungeon, int x, int y, int webCount, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);
	virtual bool specialMove(Dungeon &dungeon) { return false; };

protected:
	void setCapture(bool captured) { m_captured = captured; };

private:
	void createWebs(Dungeon &dungeon, int x, int y);

	std::vector<std::pair<int, int>> m_webs; // coordinates of the webs
	bool m_captured = false; // Flag indicating if the player stepped into a spider's web
	int m_webCount; // number of webs the spiders will create up to

	bool m_primed = false;
};

class ShootingSpider : public Spider {
public:
	ShootingSpider(Dungeon &dungeon, int x, int y, int webCount = 4, int range = 5);

	bool specialMove(Dungeon &dungeon);

private:
	int m_range;
	int m_turns;
	int m_maxTurns;
	bool m_primed = false;
};

class PouncingSpider : public Monster {
public:
	// Pouncing Spiders are territorial and will guard a region from outsiders.
	// 
	PouncingSpider(int x, int y);

	void move(Dungeon &dungeon);

private:
	bool m_primed = false;
	int m_turns = 2;
	int m_maxTurns = 2;

	// The spider's original location, it will always try to jump back to these coordinates
	// after attacking the player. If it can't, then its new position becomes its
	// new hub coordinates.
	int m_x;
	int m_y;

	bool boundaryCheck(const Dungeon &dungeon);
};

class Ghost : public Monster {
public:
	// The ghost is a rare enemy that can go through walls.
	// If it touches the player, they become possessed and cannot control their movements
	// for a few turns.
	Ghost(int x, int y, int range = 15);

	void move(Dungeon &dungeon);
	void attack(Dungeon &dungeon, Player &p);

private:
	int m_range;
};

class Puff : public Monster {
public:
	Puff(int x, int y, int turns = 4, int pushDist = 2);
	Puff(int x, int y, int turns, int pushDist, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move(Dungeon &dungeon);

	// Indicates if this Puff pushes in all 8 directions.
	// The regular version only pushes in the 4 cardinal directions.
	virtual bool pushesAll() { return false; };

private:
	int m_turns;
	int m_maxTurns;
	int m_pushDist;
};

class GustyPuff : public Puff {
public:
	GustyPuff(int x, int y);

	bool pushesAll() { return true; };
};

class StrongGustyPuff : public Puff {
public:
	StrongGustyPuff(int x, int y);

	bool pushesAll() { return true; };
};

class InvertedPuff : public Monster {
public:
	// InvertedPuffs suck players and enemies in instead of pushing them away
	InvertedPuff(int x, int y, int turns = 4, int pullDist = 2);
	InvertedPuff(int x, int y, int turns, int pullDist, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move(Dungeon &dungeon);

	// Indicates if this Puff pushes in all 8 directions.
	// The regular version only pushes in the 4 cardinal directions.
	virtual bool pullsAll() { return false; };

	virtual void specialMove(Dungeon &dungeon) { return; };

private:
	int m_turns;
	int m_maxTurns;
	int m_pullDist;
};

class SpikedInvertedPuff : public InvertedPuff {
public:
	// The Spiked Inverted Puff attempts to suck players in and damages them
	SpikedInvertedPuff(int x, int y);
	SpikedInvertedPuff(int x, int y, int turns, int pullDist, std::string name, std::string image, int hp, int armor, int str, int dex);

	void attack(Dungeon &dungeon, Player &p);
	void specialMove(Dungeon &dungeon);
};

class GustySpikedInvertedPuff : public SpikedInvertedPuff {
public:
	// Pulls in all 8 directions
	GustySpikedInvertedPuff(int x, int y);

	void specialMove(Dungeon &dungeon);
	bool pullsAll() { return true; };
};

class Serpent : public Monster {
public:
	// Serpents are two tiles long and players can attack the head or tail.
	// Attacking the head results in damage, but attacking the tail results in none.
	Serpent(Dungeon &dungeon, int &x, int &y, int turns = 1);
	Serpent(Dungeon &dungeon, int &x, int &y, int turns, std::string name, std::string image, int hp, int armor, int str, int dex);

	bool setTailPosition(Dungeon &dungeon);
	void rerollMonsterPosition(Dungeon &dungeon, int &x, int &y);
	void move(Dungeon &dungeon);
	void extraAttackEffects(Dungeon& dungeon);

	void moveSegments(Dungeon &dungeon, int x, int y);
	void addSegments(Dungeon &dungeon);
	void removeSegments(Dungeon &dungeon);

private:
	int m_turns;
	int m_maxTurns;
	int m_range = 7;

	int m_tailX;
	int m_tailY;
	std::shared_ptr<MonsterSegment> m_tail = nullptr;
};

class ArmoredSerpent : public Serpent {
public:
	// Armored Serpents cannot be damaged directly--the player must hit their tail.
	ArmoredSerpent(Dungeon &dungeon, int x, int y, int turns = 1);

	bool canBeDamaged(Dungeon &dungeon) { return false; };
};





//	BOSSES
class Smasher : public Monster {
public:
	Smasher(FirstBoss *dungeon);

	void move(Dungeon &dungeon);

	void pickMove();
	void move1();
	void move2();
	void avalanche();
	void move3();
	void move4();
	void move5();

	void moveSmasher();
	void moveSmasherL();
	void moveSmasherR();
	void moveSmasherU();
	void moveSmasherD();

	void resetLeftward();
	void resetRightward();
	void resetUpward();
	void resetDownward();

	void attack(Dungeon &dungeon, Player &p);
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
	FirstBoss *m_dungeon = nullptr;

	// used for telling what direction smasher is moving in if executing rock slide
	char m_move = '0';

	bool m_moveActive;
	bool m_moveEnding;
	int m_moveType;
	bool m_frenzy;
};



#endif