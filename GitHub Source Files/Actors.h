#ifndef ACTORS_H
#define ACTORS_H

class cocos2d::Sprite;
class Dungeon;
class Afflictions;
class Weapon;
enum class ImbuementType;
enum class DamageType;
class Monster;
class Objects;
class Shield;
class Spell;
class Drops;
class Relic;
class Passive;
class Rocks;
class Traps;

class Actors {
public:
	Actors();
	Actors(int x, int y, int hp, int armor, int str, int dex, std::string name);
	Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep);
	Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name);
	virtual ~Actors();

	virtual void moveTo(int x, int y, float time = 0.10f) { return; };

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

	void setPosX(int x) { m_x = x; };
	void setPosY(int y) { m_y = y; };
	void setHP(int hp) {

		// If this was a killing blow, or they were fragile and this damaged them
		if (hp <= 0 || (m_hp > hp && isFragile())) {
			m_isDead = true;
			m_hp = 0;
			return;
		}

		m_hp = hp;

		if (m_hp > m_maxhp)
			m_hp = m_maxhp;
	};
	void setMaxHP(int maxhp) { m_maxhp = maxhp; };
	void setArmor(int armor) { m_armor = armor; };
	void setStr(int str) { m_str = str; };
	void setDex(int dex) { m_dex = dex; };
	void setInt(int intellect) { m_int = intellect; };
	void setLuck(int luck) { m_luck = luck; };
	void setWeapon(std::shared_ptr<Weapon> wep) { m_wep = wep; };
	void setName(std::string name) { m_name = name; };
	void setSprite(cocos2d::Sprite* sprite) { m_sprite = sprite; };
	void setDungeon(Dungeon *dungeon) { m_dungeon = dungeon; };

	bool isPlayer() const { return m_isPlayer; };
	bool isMonster() const { return m_isMonster; };
	virtual bool isSpirit() const { return false; } // Indicates if this is the insta-kill enemy
	bool isDead() const { return m_isDead; };
	void setDead(bool dead) { m_isDead = dead; };

	// SuperDead indicates that there is no way the Actor can come back to life
	// For instance, the player is invulnerable and falls down a Pit.
	bool isSuperDead() const { return m_superDead; };
	void setSuperDead(bool super) {
		if (super) {
			m_isDead = true;
			m_hp = 0;
		}
		m_superDead = super;
	};

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

	void setLavaImmunity(bool immune) { m_lavaImmune = immune; };
	void setFlying(bool flying) { m_flying = flying; };
	void setSturdy(bool sturdy) { m_sturdy = sturdy; };
	void setHeavy(bool heavy) { m_heavy = heavy; };

	void setBloodlust(bool lust) { m_bloodlust = lust; };
	void setToxic(bool toxic) { m_toxic = toxic; }; // can poison others

	bool canBeStunned() const { return m_stunnable; };
	bool canBeBurned() const { return m_burnable; };
	bool canBeBled() const { return m_bleedable; };
	bool canBeHealed() const { return m_healable; };
	bool canBeFrozen() const { return m_freezable; };
	bool canBePoisoned() const { return m_poisonable; };

	void setCanBeStunned(bool stunnable) { m_stunnable = stunnable; };
	void setCanBeBurned(bool burnable) { m_burnable = burnable; };
	void setCanBeBled(bool bleedable) { m_bleedable = bleedable; };
	void setCanBeHealed(bool healable) { m_healable = healable; };
	void setCanBeFrozen(bool freezable) { m_freezable = freezable; };
	void setCanBePoisoned(bool poisonable) { m_poisonable = poisonable; };

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
	bool isFragile() const { return m_fragile; };

	void setBurned(bool burned) { m_burned = burned; };
	void setBleed(bool bled) { m_bled = bled; };
	void setStunned(bool stun) { m_stunned = stun; };
	void setFrozen(bool freeze) { m_frozen = freeze; };
	void setPoisoned(bool poisoned) { m_poisoned = poisoned; };
	void setInvisible(bool invisible) { m_invisible = invisible; };
	void setEthereal(bool ethereal) { m_ethereal = ethereal; };
	void setConfused(bool confused) { m_confused = confused; };
	void setBuffed(bool buffed) { m_buffed = buffed; };
	void setInvulnerable(bool invulnerable) { m_invulnerable = invulnerable; };
	void setStuck(bool stuck) { m_stuck = stuck; };
	void setPossessed(bool possessed) { m_possessed = possessed; };
	void setCrippled(bool crippled) { m_crippled = crippled; };
	void setFragile(bool fragile) { m_fragile = fragile; };

protected:
	void setImageName(std::string image) { m_image = image; };
	void setHasAnimation(bool hasAnimation) { m_hasAnimation = hasAnimation; };
	void setAnimationFrames(std::string frames) { m_frames = frames; };
	void setAnimationFrameCount(int count) { m_frameCount = count; };
	void setFrameInterval(int interval) { m_frameInterval = interval; };

	void setPlayerFlag(bool player) { m_isPlayer = player; };
	void setMonsterFlag(bool monster) { m_isMonster = monster; };

	Dungeon *m_dungeon = nullptr;
	
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
	bool m_superDead = false;

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
	bool m_fragile = false;

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

	void moveTo(int x, int y, float time = 0.10f);
	void move(char move);
	void attack(Actors &a);
	virtual void successfulAttack(Actors &a) = 0;
	void botchedAttack(Actors &a);
	void chainLightning(Actors &a);

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
	
	std::vector<std::shared_ptr<Drops>>& getItems() { return m_items; };
	int getMaxItemInvSize() const { return m_maxiteminv; };

	// Active item (Spacebar)
	bool hasActiveItem() const { return m_hasActiveItem; };
	virtual void useActiveItem() = 0;
	virtual void equipActiveItem(std::shared_ptr<Objects> active) = 0;
	std::shared_ptr<Objects> getActiveItem() const { return m_activeItem; };

	// Used for HUD updating
	virtual bool activeHasMeter() const = 0; // For indicating if the HUD should display a meter
	virtual int getCurrentActiveMeter() const { return 0; }; // Active item's current durability/points
	virtual int getMaxActiveMeter() const { return -1; }; // Active item's maximum durability/points

	virtual bool activeHasAbility() const = 0;
	virtual void useActiveAbility(Actors &a) { return; };
	virtual void restoreActive(int repairs) { return; };

	virtual bool canUseShield() const = 0;

	// General block functions
	virtual bool canBlock() const = 0;
	virtual bool didBlock(int x, int y) const { return false; };
	virtual void successfulBlock() { return; };
	virtual int blockedDamageReduction() { return 0; };

	virtual void setBlock(bool blocking) { m_blocking = blocking; };

	void equipWeapon(std::shared_ptr<Weapon> weapon); // Shrine
	void storeWeapon(std::shared_ptr<Weapon> weapon);
	void switchWeapon();
	void throwWeaponTo(int x, int y);
	std::shared_ptr<Weapon> getStoredWeapon() const { return m_storedWeapon; };
	void tradeWeapon(std::shared_ptr<Weapon> weapon); // Trader
	void removeStoredWeapon(); // Shrine

	void addItem(std::shared_ptr<Drops> drop);
	void addItem(std::shared_ptr<Drops> drop, bool &itemAdded); // New for item collection
	void use(int index);
	void removeItems(); // Shrine effect
	void removeItem(int index); // Trader

	void equipPassive(std::shared_ptr<Passive> passive);
	std::vector<std::shared_ptr<Passive>> getPassives() const { return m_passives; };
	void removePassive(int index); // Trader

	bool hasRelic() const { return m_hasRelic; };
	void setRelicFlag(bool flag) { m_hasRelic = flag; };
	std::shared_ptr<Relic>& getRelic() { return m_relic; };
	void equipRelic(std::shared_ptr<Relic> relic); // Shrine use
	void removeRelic(); // Shrine use


	char facingDirection() const { return m_facing; };
	void setFacingDirection(char facing) { m_facing = facing; };
	char getAction() const { return m_action; };
	void setAction(char action) { m_action = action; };

	void setItemToFront(int index); // used for item quick access

	int getVision() const { return m_vision; };
	void setVision(int vision) { m_vision = vision; };

	int getFavor() const { return m_favor; };
	void increaseFavorBy(int favor);
	void decreaseFavorBy(int favor);

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

	bool hasGoldIllumination() const { return m_goldIllumination; };
	void setGoldIllumination(bool illuminate) { m_goldIllumination = illuminate; };

	bool hasGoldInvulnerability() const { return m_goldInvulnerability; };
	void setGoldInvulnerability(bool ability) { m_goldInvulnerability = ability; };

	bool hasResonantSpells() const { return m_resonantSpells; };
	void setResonantSpells(bool resonant) { m_resonantSpells = resonant; };

	bool isSlow() const { return m_slow; };
	void setSlow(bool slow) { m_slow = slow; };

	bool hasChainLightning() const { return m_chainLightning; };
	void setChainLightning(bool lightning) { m_chainLightning = lightning; };

	bool hasCripplingBlows() const { return m_cripplingBlows; };
	void setCripplingBlows(bool cripple) { m_cripplingBlows = cripple; };

	bool hasMatrixVision() const { return m_matrixVision; };
	void setMatrixVision(bool matrix) { m_matrixVision = matrix; };

	bool hasHarshAfflictions() const { return m_harshAfflictions; };
	void setHarshAfflictions(bool harsh) { m_harshAfflictions = harsh; };

	bool hasSpellRetaliation() const { return m_spellRetaliation; };
	void setSpellRetaliation(bool retaliate) { m_spellRetaliation = retaliate; };

	bool hasAfflictionOverride() const { return m_afflictionOverride; };
	void setAfflictionOverride(bool afflict) { m_afflictionOverride = afflict; };

	int getTimerReduction() const { return m_timerReduction; };
	void setTimerReduction(float reduction) { m_timerReduction = reduction; };

	bool hasFatStacks() const { return m_fatStacks; };
	void setFatStacks(bool stack) { m_fatStacks = stack; };

	bool hasBonusRoll() const { return m_bonusRoll; };
	void setBonusRoll(bool bonus) { m_bonusRoll = bonus; };

	bool hasFragileRetaliation() const { return m_fragileRetaliation; };
	void setFragileRetaliation(bool fragile) { m_fragileRetaliation = fragile; };

	bool hasScavenger() const { return m_scavenger; };
	void setScavenger(bool flag) { m_scavenger = flag; };

	bool hasDelayedHealing() const { return m_delayedHealing > 0; };
	float getDelayedHealing() const { return m_delayedHealing; };
	void setDelayedHealing(float healing) { m_delayedHealing = healing; }; // Healing is the percentage of max hp to restore
	/// End special abilities

	bool spiritActive() const { return m_spiritActive; };
	void setSpiritActive(bool flag) { m_spiritActive = flag; };

	bool hasSkeletonKey() const;
	void checkKeyConditions();
	int keyHP() const { return m_keyhp; };
	void setKeyHP(int keyhp) { m_keyhp = keyhp; };

	bool getWin() const { return m_winner; };
	void setWin(bool win) { m_winner = win; };
	std::string getDeath() const { return m_death; };
	void setDeath(std::string m) { m_death = m; };

protected:
	void checkExtraAttackEffects(Actors &a);
	void knockbackDirection(int &x, int &y, int mx, int my); // Determines the direction the enemy should be knocked back

	void dropWeapon();
	void dropStoredWeapon(); // Used mainly if player was dual wielding
	void addSecondaryWeapon(std::shared_ptr<Weapon> weapon) { m_storedWeapon = weapon; }; // For giving chracters a second starting weapon

	void setHasActiveItem(bool active) { m_hasActiveItem = active; };
	void setActiveItem(std::shared_ptr<Objects> active) { m_activeItem = active; };

private:
	std::shared_ptr<Weapon> m_storedWeapon = nullptr;

	bool m_hasActiveItem;
	std::shared_ptr<Objects> m_activeItem = nullptr;

	std::vector<std::shared_ptr<Drops>> m_items;
	std::vector<std::shared_ptr<Passive>> m_passives;

	bool m_hasRelic = false;
	std::shared_ptr<Relic> m_relic = nullptr;

	int m_maxarmor = 99;
	int m_maxstr = 99;
	int m_maxdex = 99;
	int m_maxwepinv = 5;
	int m_maxiteminv = 5;
	int m_money = 0;
	int m_moneyMultiplier = 1; // money multiplied upon monster death

	// Flat extra money; not based on multiplier
	// increases by 0.25 upon successful hit; decreases by 0.50 if hit
	float m_moneyBonus = 0;
	int m_maxMoneyBonus = 10;

	int m_vision = 5;

	// Favor works sort of like curse, except favor can be positive or negative.
	// Positive favor will help the player, whereas negative favor will harm them.
	int m_favor = 0;

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
	bool m_goldIllumination = false; // Illuminates gold. Can only be obtained through BrightStar relic.
	bool m_goldInvulnerability = false; // High chance to gain 1 turn of invulnerability after collecting gold. Only obtainable through Riches relic.
	bool m_resonantSpells = false; // Spells have a chance to not be consumed on use
	bool m_slow = false; // Slowness means that monsters get to move before the player
	bool m_chainLightning = false; // Chance to chain damage through nearby enemies. Only obtainable through Lightbulb relic.
	bool m_cripplingBlows = false; // Chance to cripple enemies on hit. Only obtainable through MatrixVision relic.
	bool m_matrixVision = false; // Chance for traps and monster to skip a few turns. Only obtainable through MatrixVision relic.
	bool m_harshAfflictions = false; // All inflicted afflictions have longer duration. Only obtainable through SuperMagicEssence relic.
	bool m_spellRetaliation = false; // Chance to cast random spell after being hit. Only obtainable through SuperMagicEssence relic.
	bool m_afflictionOverride = false; // All afflictions have a chance to affect any enemy, ignoring any resistances. Only obtainable through SuperMagicEssence relic.
	float m_timerReduction = 0.0f; // Makes the level timer slower.
	bool m_fatStacks = false; // Allows all items to become stackable.
	bool m_bonusRoll = false; // Grants the player a bonus roll to save from certain afflictions.
	bool m_fragileRetaliation = false; // Small chance to give an enemy Fragile status when attacked.
	bool m_scavenger = false; // Small chance for enemies to drop items.
	float m_delayedHealing = 0; // Obtained from Magma Heart item. Heals when player goes to the next floor.
	
	bool m_spiritActive = false; // Indicator that the ForgottenSpirit is to spawn
	
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
	~Adventurer();

	void useActiveItem();
	void equipActiveItem(std::shared_ptr<Objects> active);
	bool activeHasMeter() const { return true; };
	int getCurrentActiveMeter() const;
	int getMaxActiveMeter() const;

	bool activeHasAbility() const;
	void useActiveAbility(Actors &a);
	
	bool canUseShield() const { return true; };
	bool hasShield() const { return m_hasShield; };
	
	void equipShield(std::shared_ptr<Shield> shield);
	void dropShield();
	void shieldBroken();
	void restoreActive(int repairs);
	void setBlock(bool blocking) { m_blocking = blocking; };
	
	bool canBlock() const;
	bool didBlock(int mx, int my) const;
	int blockedDamageReduction();
	void successfulBlock();

	void successfulAttack(Actors &a) { return; };

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
	~Spellcaster();

	void useActiveItem();
	void equipActiveItem(std::shared_ptr<Objects> active);
	bool activeHasMeter() const { return true; };
	int getCurrentActiveMeter() const { return m_mp; };
	int getMaxActiveMeter() const { return m_maxMP; };
	bool activeHasAbility() const { return false; };

	bool canBlock() const { return false; };
	bool canUseShield() const { return false; };
	void successfulAttack(Actors &a);

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
	~Spelunker();

	void useActiveItem();
	void equipActiveItem(std::shared_ptr<Objects> active);

	int getRockCount() const { return m_rocks.size(); }; // For HUD purposes

	bool activeHasMeter() const { return false; };
	bool activeHasAbility() const { return false; };

	bool canBlock() const { return false; };
	bool canUseShield() const { return false; };

	void successfulAttack(Actors &a) { return; };

private:
	std::vector<std::shared_ptr<Rocks>> m_rocks;
};

class Acrobat : public Player {
public:
	// The Acrobat's ability allows them to roll/maneuver past enemies and over obstacles.
	// They have a stamina meter though
	Acrobat();
	~Acrobat();

	void useActiveItem();
	void equipActiveItem(std::shared_ptr<Objects> active);

	bool activeHasMeter() const { return true; };
	int getCurrentActiveMeter() const { return m_stamina; };
	int getMaxActiveMeter() const { return m_maxStamina; };
	bool activeHasAbility() const { return false; };

	bool canBlock() const { return false; };
	bool canUseShield() const { return false; };

	void successfulAttack(Actors &a);

private:
	std::shared_ptr<Drops> m_item = nullptr;
	int m_stamina;
	int m_maxStamina;
};

class TheMadman : public Player {
public:
	// Active item is a teleporter. Cannot be replaced.
	// Starts with no weapon but increased vision radius.
	TheMadman();
	~TheMadman();

	void useActiveItem();
	void equipActiveItem(std::shared_ptr<Objects> active);
	bool activeHasMeter() const { return false; };
	int getCurrentActiveMeter() const { return 0; };
	int getMaxActiveMeter() const { return -1; };
	bool activeHasAbility() const { return false; };

	bool canBlock() const { return false; };
	bool canUseShield() const { return false; };
	void successfulAttack(Actors &a) { return; };

private:
	std::shared_ptr<Drops> m_item;
};


// ============================================
//			 	  :::: NPCS ::::
// ============================================

class NPC : public Actors {
public:
	NPC(Dungeon *dungeon, int x, int y, std::string name, std::string image);

	void talk();
	virtual void checkSatisfaction() = 0;
	virtual void reward() = 0;

	virtual void addInitialDialogue() { return; };
	virtual void addInteractedDialogue() = 0;
	virtual void addSatisfiedDialogue() = 0;
	virtual void addFinalDialogue() = 0;
	void setSatisfaction(bool satisfied) { m_satisfied = satisfied; };

	std::vector<std::string> getDialogue() const { return m_dialogue; };
	std::vector<std::string> getChoices() const { return m_promptChoices; };
	virtual void useResponse(int index) { return; }; // Index of the choice that the player made in the choices vector

protected:
	void addDialogue(std::string line) { m_dialogue.push_back(line); }; // Adds line to to back of the dialogue vector
	void addChoice(std::string line) { m_promptChoices.push_back(line); };

	void playDialogue();

	void rewardWasGiven() { m_rewardGiven = true; };

	int getInteractionStage() const { return m_interactedDialogueStage; };
	void incInteractionStage() { m_interactedDialogueStage++; };
	int getInteractionLimit() const { return m_interactionLimit; };
	void setInteractionLimit(int limit) { m_interactionLimit = limit; };

	// Indicates what stage of prompt we're at so that we use
	// the appropriate set of choices.
	int m_promptStage = 1;

	std::vector<std::string> m_dialogue;
	std::vector<std::string> m_promptChoices;

private:
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
	CreatureLover(Dungeon *dungeon, int x, int y);

	void checkSatisfaction();
	void reward();

	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

private:
	std::string m_wantedCreature;
	std::shared_ptr<Monster> m_creature = nullptr;
};

class Memorizer : public NPC {
public:
	Memorizer(Dungeon *dungeon, int x, int y);

	void useResponse(int index);
	void checkSatisfaction();
	void reward();

	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

private:
	//Dungeon *m_dungeon = nullptr;
	std::string m_topic;
	std::string m_correctChoice;

	// Since there are multiple prompts, this indicates what stage of prompt we're at so that we use
	// the appropriate set of choices.
	//int m_promptStage = 1;
};

class Shopkeeper : public NPC {
public:
	Shopkeeper(Dungeon *dungeon, int x, int y);

	void checkSatisfaction();
	void reward();

	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

private:

};

class Blacksmith : public NPC {
public:
	// Allows the player the upgrade their weapon's damage. It does not carry over between weapons.
	// The player can also repair items, if applicable.
	// On occasion, the Blacksmith will offer weapons for sale.
	Blacksmith(Dungeon *dungeon, int x, int y);

	void useResponse(int index);
	void improveWeapon();
	void buyItem();
	void mingle();

	void checkSatisfaction();
	void reward();

	void addInitialDialogue();
	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

private:
	int m_improveCost;
};

class Enchanter : public NPC {
public:
	// Can imbue weapons with magical effects such as igniting, poison, freezing.
	// Can increase the player's active item maximums, if applicable.
	// Rarely, offers Magic Essence for sale (+intellect).
	Enchanter(Dungeon *dungeon, int x, int y);

	void useResponse(int index);
	void imbueWeapon();
	void buyItem();
	void mingle();

	void checkSatisfaction();
	void reward();

	void addInitialDialogue();
	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

private:
	int determineCost(ImbuementType type);

	int m_imbuementCost;
};

class Trader : public NPC {
public:
	// The player can trade out one of their passives for a new one at random.
	// Can trade inventory items for gold or another item.
	// Can trade one weapon for another at random.
	Trader(Dungeon *dungeon, int x, int y);

	void useResponse(int index);
	void startTrade();
	void makeTrade(int index);
	void tradePassive();
	void tradeItem();
	void tradeWeapon();
	void mingle();

	void checkSatisfaction();
	void reward();

	void addInitialDialogue();
	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

private:
	bool m_passiveTraded = false;
	bool m_itemTraded = false;
	bool m_weaponTraded = false;
};

class OutsideMan1 : public NPC {
public:
	// A man that hangs outside the entrance to the world hub
	OutsideMan1(Dungeon *dungeon, int x, int y);

	void checkSatisfaction() { return; };
	void reward() { return; };

	void addInteractedDialogue();
	void addSatisfiedDialogue() { return; };
	void addFinalDialogue() { return; };
};

class OutsideMan2 : public NPC {
public:
	// A man that hangs outside the entrance to the world hub
	OutsideMan2(Dungeon *dungeon, int x, int y);

	void useResponse(int index);
	void checkSatisfaction() { return; };
	void reward() { return; };

	void addInteractedDialogue();
	void addSatisfiedDialogue() { return; };
	void addFinalDialogue() { return; };
};

class OutsideWoman1 : public NPC {
public:
	// A woman that hangs outside the entrance to the world hub
	OutsideWoman1(Dungeon *dungeon, int x, int y);

	void checkSatisfaction() { return; };
	void reward() { return; };

	void addInteractedDialogue();
	void addSatisfiedDialogue() { return; };
	void addFinalDialogue() { return; };
};



// ============================================
//				:::: MONSTERS ::::
// ============================================

class Monster : public Actors {
public:
	Monster(Dungeon *dungeon, int x, int y, int hp, int armor, int str, int dex, std::string name);
	Monster(Dungeon *dungeon, int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name);
	~Monster();

	// Moves the monster to the coordinates @x and @y
	virtual void moveTo(int x, int y, float time = 0.10f);

	void moveCheck() {
		if (isDead())
			return;

		move();
	};
	virtual void move() = 0;
	virtual void attack(Player &p);
	virtual void attack(Monster &m) { return; }; // for monsters attacking other monsters
	virtual void extraAttackEffects() { return; };
	void death();
	virtual void deathDrops() { return; };
	virtual void spriteCleanup();

	bool wasDestroyed() const { return m_destroyed; };
	void setDestroyed(bool destroyed) { m_destroyed = destroyed; };

	bool attemptChase(int x, int y, int range, char &best) {
		char first;
		first = best = '0';
		int shortest = 0;

		return attemptChase(*m_dungeon, shortest, range, range, x, y, first, best);
	};
	bool attemptAllChase(int x, int y, int range, char &best) {
		char first;
		first = best = '0';
		int shortest = 0;

		return attemptAllChase(*m_dungeon, shortest, range, range, x, y, first, best);
	};
	void attemptGreedyChase(bool diagonals = false, int x = -1, int y = -1); // @x and @y are used specifically for decoys

	char attemptDryRunGreedyChase(bool diagonals = false); // Attempt greedy chase without actually moving
	bool attemptDryRunMove(char move);

	bool playerInRange(int range) const;
	bool playerInDiagonalRange(int range) const;
	bool playerIsAdjacent(bool diagonals = false) const;
	bool attemptMove(char move);
	bool moveWithSuccessfulChase(char move);
	char moveMonsterRandomly(bool diagonals = false);

	virtual bool canBeDamaged(DamageType type) { return true; }; // Indicates if the enemy can be damaged (not armored, etc.)
	virtual bool canBeHit() { return true; }; // Indicates if enemy can by physically struck (i.e. sand centipedes when underground)
	virtual bool isUnderground() { return false; }; // For Sand Centipede

	bool chasesPlayer() const { return m_chases; };
	bool isSmart() const { return m_smart; }; // tells if monster (that chases), will or will not walk on lethal traps
	bool hasWeapon() const { return m_hasWeapon; }
	bool isMultiSegmented() const { return m_multiSegmented; }; // Tells if monster spans multiple tiles
	virtual bool isMonsterSegment() const { return false; } // Indicates if this is a monster segment. Used for ignoring traps.

	virtual bool isBreakable() const { return false; } // Indicates if this is a breakable object

	virtual void removeSegments() { return; }; // Removes all segments of a monster if it is multi-segmented
	virtual int getParentX() const { return getPosX(); }; // X coordinate of the segmented monster's main part
	virtual int getParentY() const { return getPosY(); }; // Y coordinate of the segmented monster's main part

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
	bool attemptChase(Dungeon &dungeon, int &shortest, int currentDist, int origdist, int x, int y, char &first_move, char &optimal_move);
	bool attemptAllChase(Dungeon &dungeon, int &shortest, int currentDist, int origDist, int x, int y, char &first_move, char &optimal_move);

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
	Breakables(Dungeon *dungeon, int x, int y, int hp, int armor, std::string name, std::string image);

	void move() { return; };

	bool isBreakable() { return true; };
	virtual bool canBeDamaged(DamageType type) = 0;
};

class SturdyBreakables : public Breakables {
public:
	// These breakables do not move when hit
	SturdyBreakables(Dungeon *dungeon, int x, int y, int hp, int armor, std::string name, std::string image);

};

class WeakCrate : public SturdyBreakables {
public:
	WeakCrate(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };
	void deathDrops();
};

class WeakBarrel : public SturdyBreakables {
public:
	WeakBarrel(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };
	void deathDrops();
};

class SmallPot : public SturdyBreakables {
public:
	SmallPot(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };
	void deathDrops();
};

class LargePot : public SturdyBreakables {
public:
	LargePot(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };
	void deathDrops();
};

class Sign : public SturdyBreakables {
public:
	Sign(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };
	void deathDrops();
};

class ArrowSign : public SturdyBreakables {
public:
	ArrowSign(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };
	void deathDrops();
};

class ExplosiveBarrel : public Breakables {
public:
	ExplosiveBarrel(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };
	void deathDrops();
};

class CharredWood : public Breakables {
public:
	CharredWood(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };
	void deathDrops();
};

//		SPECIAL
class ForgottenSpirit : public Monster {
public:
	// A slow-moving, unkillable spirit that can travel through walls,
	// can travel through all enemies, and is unaffected by all traps.
	// If it touches the player, they die instantly.
	ForgottenSpirit(Dungeon *dungeon, int x, int y);

	bool isSpirit() const { return true; }

	void moveTo(int x, int y, float time = 0.10f);
	void move();
	void attack(Player &p);

private:
	int m_turns = 4;
};

class Ghost : public Monster {
public:
	// The ghost is a rare enemy that can go through walls.
	// If it touches the player, they become possessed and cannot control their movements
	// for a few turns.
	Ghost(Dungeon *dungeon, int x, int y, int range = 15);

	void move();
	void attack(Player &p);

private:
	int m_range;
};

class MonsterSegment : public Monster {
public:
	// Monster Segments are simply parts of a monster that can be attacked or interacted with.
	// They typically don't do anything other than moving around with the main monster's body section.
	MonsterSegment(Dungeon *dungeon, int x, int y, int sx, int sy, std::string name, std::string image);

	void move() { return; };

	bool isMonsterSegment() const { return true; }

	void setParentX(int x) { m_parentX = x; };
	void setParentY(int y) { m_parentY = y; };
	virtual int getParentX() const { return m_parentX; }; // X coordinate of the segmented monster's parent
	virtual int getParentY() const { return m_parentY; }; // Y coordinate of the segmented monster's parent

private:
	// Coordinates of the main monster to which it is attached
	int m_parentX;
	int m_parentY;
};

//		FLOOR I
class Seeker : public Monster {
public:
	Seeker(Dungeon *dungeon, int x, int y, int range = 10);
	Seeker(Dungeon *dungeon, int x, int y, int range, std::string name, std::string image);

	void move();

private:
	int m_range;
	bool m_step;
};

class GooSack : public Monster {
public:
	// This enemy is stationary, but if the player is close, it will attempt to jump on them
	GooSack(Dungeon *dungeon, int x, int y);

	void move();
	void attack(Player &p);

private:
	bool m_primed = false;

	// coordinates for where the player was when this was primed
	int m_x;
	int m_y;
};

class Broundabout : public Monster {
public:
	// This enemy moves in straight lines, back and forth. However, like the GooSack, it will try to jump onto nearby players.
	// After jumping it will continue moving in the direction it was originally moving in, unless the player is still nearby.
	Broundabout(Dungeon *dungeon, int x, int y);

	void move();
	void attack(Player &p);

private:
	bool m_primed = false;
	char m_dir;

	// coordinates for where the player was when this was primed
	int m_x;
	int m_y;
};

class Rat : public Monster {
public:
	// Rats run from the player if they're too close, but will chase the player
	// if they do not move on their turn.
	Rat(Dungeon *dungeon, int x, int y, int range = 10);

	void move();

private:
	void run();

	int m_range;
	int m_prevPlayerX = -1;
	int m_prevPlayerY = -1;
};

class Spider : public Monster {
public:
	// Spiders spin spiderwebs and chase down the player if they are caught in them
	// If the player is adjacent (all directions), then they will prime to attack on the next turn.
	Spider(Dungeon *dungeon, int x, int y, int webCount = 4);
	Spider(Dungeon *ungeon, int x, int y, int webCount, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move();
	void attack(Player &p);
	virtual bool specialMove() { return false; };

protected:
	void setCapture(bool captured) { m_captured = captured; };

private:
	void createWebs(int x, int y);

	std::vector<std::pair<int, int>> m_webs; // coordinates of the webs
	bool m_captured = false; // Flag indicating if the player stepped into a spider's web
	int m_webCount; // number of webs the spiders will create up to

	bool m_primed = false;
	bool m_wait = false;
};

class ShootingSpider : public Spider {
public:
	ShootingSpider(Dungeon *dungeon, int x, int y, int webCount = 4, int range = 5);

	bool specialMove();

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
	PouncingSpider(Dungeon *dungeon, int x, int y);

	void move();

private:
	bool m_primed = false;
	int m_turns = 2;
	int m_maxTurns = 2;

	// The spider's original location, it will always try to jump back to these coordinates
	// after attacking the player. If it can't, then its new position becomes its
	// new hub coordinates.
	int m_x;
	int m_y;

	bool boundaryCheck() const;
};

class Roundabout : public Monster {
public:
	Roundabout(Dungeon *dungeon, int x, int y);
	Roundabout(Dungeon *dungeon, int x, int y, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move();
	void attack(Player &p);
	virtual void specialMove() { return; };

	char getDirection() const { return m_direction; };
	void setDirection(char dir) { m_direction = dir; };

private:
	char m_direction;
};

class Goblin : public Monster {
public:
	Goblin(Dungeon *dungeon, int x, int y, int range = 10);

	void move();
	void deathDrops();

private:
	int m_range;
};

class Wanderer : public Monster {
public:
	Wanderer(Dungeon *dungeon, int x, int y);
	Wanderer(Dungeon *dungeon, int x, int y, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move();
	void deathDrops();
};

class SleepingWanderer : public Wanderer {
public:
	SleepingWanderer(Dungeon *dungeon, int x, int y);

	void move();
	void deathDrops();

private:
	bool m_provoked = false;
	int m_range = 8;
	int m_provocationRange = 4;
};

class ProvocableWanderer : public Wanderer {
public:
	ProvocableWanderer(Dungeon *dungeon, int x, int y);

	void move();
	void attack(Player &p);
	void deathDrops();

private:
	bool m_provoked = false;
	int m_turns = 2;
	int m_prevHP;
};

//		FLOOR II
class RabidWanderer : public Wanderer {
public:
	// Chance to poison the player
	RabidWanderer(Dungeon *dungeon, int x, int y);

	void move();
	void attack(Player &p);
	void deathDrops();

private:
	bool m_turn = false;
};

class Toad : public Monster {
public:
	// Toads spit out poisonous puddles
	Toad(Dungeon *dungeon, int x, int y);

	void move();

private:
	// The `-` character indicates that the toad did not move on its previous turn
	char m_facing = '-';

	// The toad will cycle through the following moves:
	// Move randomly -> wait for three turns -> move twice randomly -> wait 1 turn -> roll to spit poison puddle -> repeat
	// This value cycles through from 1 to 8.
	int m_moveType = 1;
};

class PoisonBubble : public Monster {
public:
	// Does not move or attack, but creates poison when killed.
	PoisonBubble(Dungeon *dungeon, int x, int y);

	void move();
	void deathDrops();
};

class Piranha : public Monster {
public:
	// 
	Piranha(Dungeon *dungeon, int x, int y);

	void move();

private:
	void attemptMoveInWater();
};

class WaterSpirit : public Monster {
public:
	WaterSpirit(Dungeon *dungeon, int x, int y);

	void move();
	void deathDrops();

private:
	void douseTiles();

	int m_range = 15;
	int m_wait = 1;
	int m_maxWait;
};

class AngledBouncer : public Monster {
public:
	AngledBouncer(Dungeon *dungeon, int x, int y);

	void move();

private:
	void changeDirection();

	char m_dir;
};

class Firefly : public Monster {
public:
	// Harmless, but provides light.
	Firefly(Dungeon *dungeon, int x, int y);

	void move();

private:
	int m_wait = 3;
	int m_maxWait;
};



//		FLOOR III
class DeadSeeker : public Seeker {
public:
	DeadSeeker(Dungeon *dungeon, int x, int y, int range = 13);

private:
	int m_range;
	bool m_step;
};

class CrystalTurtle : public Monster {
public:
	// 
	CrystalTurtle(Dungeon *dungeon, int x, int y);

	void move();

private:
	int m_wait = 4;
	int m_maxWait;

	bool m_hit = false;
	int m_prevHP;
};

class CrystalHedgehog : public Monster {
public:
	// 
	CrystalHedgehog(Dungeon *dungeon, int x, int y, char dir = '-');

	void move();

private:
	int m_wait = 7;
	int m_maxWait;
	char m_dir;
};

class CrystalShooter : public Monster {
public:
	// 
	CrystalShooter(Dungeon *dungeon, int x, int y);

	void move();

private:
	int m_wait = 5;
	int m_maxWait;
};

class CrystalBeetle : public Monster {
public:
	// 
	CrystalBeetle(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type);
	void move();

private:
	char m_dir;
	int m_range = 10;
	int m_wait = 2;
	int m_maxWait;
	int m_backShield = 2; // Hits before the back shield is broken
};

class Rabbit : public Monster {
public:
	// 
	Rabbit(Dungeon *dungeon, int x, int y);

	void move();
	void run();

private:
	bool m_beingChased = false;
	int m_range = 6;
	int m_runSteps = 3;
	int m_maxRunSteps;
};

class Bombee : public Monster {
public:
	Bombee(Dungeon *dungeon, int x, int y, int range = 11);
	Bombee(Dungeon *dungeon, int x, int y, int range, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move();
	void attack(Player &p);
	void deathDrops();

private:
	int m_fuse;
	bool m_fused;
	int m_aggroRange;
};

//		FLOOR IV
class FlameWanderer : public Wanderer {
public:
	// Moves every other turn and has a chance to ignite.
	// If the player is adjacent, then this is guaranteed to attack.
	FlameWanderer(Dungeon *dungeon, int x, int y);

	void move();
	void attack(Player &p);

private:
	bool m_turn = false;
};

class Zapper : public Monster {
public:
	Zapper(Dungeon *dungeon, int x, int y);

	void spriteCleanup();

	void moveTo(int x, int y, float time = 0.10f);
	void move();
	void attack(Player &p);

	std::map<int, cocos2d::Sprite*> getSparks() { return sparks; };
	void moveSprites(int x, int y);
	void setSpriteColor(cocos2d::Color3B color);
	void addLightEmitters(const Dungeon &dungeon, std::vector<std::pair<int, int>> &lightEmitters);

private:
	bool m_cooldown;
	bool m_cardinalAttack;

	std::map<int, cocos2d::Sprite*> sparks;
};

class Spinner : public Monster {
public:
	Spinner(Dungeon *dungeon, int x, int y);

	void spriteCleanup();

	void moveTo(int x, int y, float time = 0.10f);
	void move();
	void attack(Player &p);

	void setInitialFirePosition(int x, int y);
	void setFirePosition(char move);

	bool playerWasHit(const Actors &a) const;

	void setSpriteVisibility(bool visible);
	void setSpriteColor(cocos2d::Color3B color);
	void addLightEmitters(const Dungeon &dungeon, std::vector<std::pair<int, int>> &lightEmitters);

private:
	bool m_clockwise;
	int m_angle;

	std::shared_ptr<Objects> m_innerFire;
	std::shared_ptr<Objects> m_outerFire;
};

class CharredBombee : public Bombee {
public:
	// The main difference is that this bombee is immune to lava
	CharredBombee(Dungeon *dungeon, int x, int y, int range = 10);

	void attack(Player &p);
};

class FireRoundabout : public Roundabout {
public:
	FireRoundabout(Dungeon *dungeon, int x, int y);

	void specialMove();
};

class ItemThief : public Monster {
public:
	ItemThief(Dungeon *dungeon, int x, int y, int range = 7);

	void move();
	void run();
	void attack(Player &p);
	void deathDrops();

private:
	bool m_stole = false; // flag for telling that they successfully stole from the player
	std::shared_ptr<Objects> m_stolenItem = nullptr;
	int m_stolenGold = 0;
	int m_range;
};

class Charger : public Monster {
public:
	// Charger will attempt to dash toward the player (instantly after charging up).
	// They move every two turns toward the player and will destroy walls if they miss.
	// If they hit the player, then they are sent flying backward.
	Charger(Dungeon *dungeon, int x, int y);

	void move();
	void attack(Player &p);

private:
	bool m_primed = false;
	int m_turns = 2; // Number of turns before moving
	char m_direction;
	int m_range = 8;
	int m_losRange = 5;
};

class Serpent : public Monster {
public:
	// Serpents are two tiles long and players can attack the head or tail.
	// Attacking the head results in damage, but attacking the tail results in none.
	Serpent(Dungeon *dungeon, int &x, int &y, int turns = 1);
	Serpent(Dungeon *dungeon, int &x, int &y, int turns, std::string name, std::string image, int hp, int armor, int str, int dex);

	bool setTailPosition();
	void rerollMonsterPosition(int &x, int &y);

	void moveTo(int x, int y, float time = 0.10f);
	void move();
	void moveSegments(int x, int y);
	void extraAttackEffects();

	void addSegments();
	void removeSegments();

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
	ArmoredSerpent(Dungeon *dungeon, int x, int y, int turns = 1);

	bool canBeDamaged(DamageType type);
};

class CombustionGolem : public Monster {
public:
	CombustionGolem(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type);
	void move();
	void deathDrops();

private:
	int m_moveRange = 15;
	int m_attackRange = 7;

	int m_wait = 3;
	int m_maxWait;

	int m_eruptCooldown = 0;
};

class OldSmokey : public Monster {
public:
	// A very slow, tanky enemy that spews black smoke in a large area that obscures vision.
	// If killed, the player gains negative favor.
	OldSmokey(Dungeon *dungeon, int x, int y);

	void move();
	void deathDrops();

private:
	int m_wait = 10;
	int m_maxWait;
};

//		FLOOR V
class Puff : public Monster {
public:
	Puff(Dungeon *dungeon, int x, int y, int turns = 4, int pushDist = 2, int range = 2);
	Puff(Dungeon *dungeon, int x, int y, int turns, int pushDist, int range, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move();

	// Indicates if this Puff pushes in all 8 directions.
	// The regular version only pushes in the 4 cardinal directions.
	virtual bool pushesAll() { return false; };

private:
	int m_turns;
	int m_maxTurns;
	int m_pushDist;
	int m_range;
};

class GustyPuff : public Puff {
public:
	// Pushes in all directions a maximum of one tile
	GustyPuff(Dungeon *dungeon, int x, int y);

	bool pushesAll() { return true; };
};

class StrongGustyPuff : public Puff {
public:
	// Pushes in all directions a maximum of two tiles
	StrongGustyPuff(Dungeon *dungeon, int x, int y);

	bool pushesAll() { return true; };
};

class InvertedPuff : public Monster {
public:
	// InvertedPuffs suck players and enemies in instead of pushing them away
	// @pullDist is how from how far away from where the Puff currently is it will try to pull.
	// For instance, P O o X: If P is the Puff, it will pull in the 'o' but not the X.
	InvertedPuff(Dungeon *dungeon, int x, int y, int turns = 4, int pullDist = 2, int range = 1);
	InvertedPuff(Dungeon *dungeon, int x, int y, int turns, int pullDist, int range, std::string name, std::string image, int hp, int armor, int str, int dex);

	void move();

	// Indicates if this Puff pushes in all 8 directions.
	// The regular version only pushes in the 4 cardinal directions.
	virtual bool pullsAll() { return false; };

	virtual void specialMove() { return; };

private:
	int m_turns;
	int m_maxTurns;
	int m_pullDist;
	int m_range;
};

class SpikedInvertedPuff : public InvertedPuff {
public:
	// The Spiked Inverted Puff attempts to suck players in and damages them
	SpikedInvertedPuff(Dungeon *dungeon, int x, int y);
	SpikedInvertedPuff(Dungeon *dungeon, int x, int y, int turns, int pullDist, int range, std::string name, std::string image, int hp, int armor, int str, int dex);

	void attack(Player &p);
	void specialMove();
};

class GustySpikedInvertedPuff : public SpikedInvertedPuff {
public:
	// Pulls in all 8 directions
	GustySpikedInvertedPuff(Dungeon *dungeon, int x, int y);

	void specialMove();
	bool pullsAll() { return true; };
};

class JeweledScarab : public Monster {
public:
	// Rare enemy with erratic movement. Drops passive if killed.
	JeweledScarab(Dungeon *dungeon, int x, int y);

	void move();
	void deathDrops();

private:
	void coordCheck(std::vector<std::pair<int, int>> &coords, bool &moved);
};

class ArmoredBeetle : public Monster {
public:
	// 
	ArmoredBeetle(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type);
	void move();

private:
	int m_wait;
	int m_maxWait;
};

class SpectralSword : public Monster {
public:
	// 
	SpectralSword(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type);
	void move();

private:
	char m_dir;
	bool m_attacked = false;
	bool m_primed = false;
};

class SandCentipede : public Monster {
public:
	// Lurks underneath in the sand. Chases player if they are in the sand.
	// Can't be hit until they lunge upwards.
	SandCentipede(Dungeon *dungeon, int x, int y);

	void moveTo(int x, int y, float time = 0.10f);
	bool canBeDamaged(DamageType type);
	bool canBeHit();
	bool isUnderground();

	void move();
	void deathDrops();

private:
	bool m_primed = false;
	int m_wait;
	int m_maxWait;
	int m_range = 12;
};

class Archer : public Monster {
public:
	Archer(Dungeon *dungeon, int x, int y);

	void move();
	void deathDrops();

private:
	bool m_primed = false;
	int m_range = 10;
};

//		FLOOR VI
class Wisp : public Monster {
public:
	Wisp(Dungeon *dungeon, int x, int y);

	void move();

private:
	int m_range = 6;
	int m_wait = 4;
	int m_maxWait;
};

class LightningWisp : public Monster {
public:
	LightningWisp(Dungeon *dungeon, int x, int y);

	void move();

private:
	bool m_cardinal;
	int m_wait = 1;
	int m_maxWait;
};

class Grabber : public Monster {
public:
	Grabber(Dungeon *dungeon, int x, int y);

	void move();

private:
	char m_dir;
	int m_grabTurns = 3;
	bool m_primed = false;
	bool m_capturedPlayer = false;

	int m_range = 7;
	int m_wait = 3;
	int m_maxWait;
};

class EvilShadow : public Monster {
public:
	// Lurks underneath other enemies and chases the player when revealed.
	EvilShadow(Dungeon *dungeon, int x, int y);

	void moveTo(int x, int y, float time = 0.1f);
	bool canBeDamaged(DamageType type);
	bool canBeHit();
	bool isUnderground();

	void move();

private:
	int m_range = 12;
	bool m_revealed = false;
	bool m_primed = false;
	bool m_hittable = false;
};

class CrawlingSpine : public Monster {
public:
	CrawlingSpine(Dungeon *dungeon, int x, int y);

	void move();

private:
	bool m_primed = false;
	int m_goopCooldown = 0;
	int m_px;
	int m_py;

	int m_range = 15;
	int m_wait = 5;
	int m_maxWait;
};

class ConstructorDemon : public Monster {
public:
	// Attempts to build destructible wall traps around the player, boxing them in.
	ConstructorDemon(Dungeon *dungeon, int x, int y);

	void move();

private:
	void run();
	void moveSideways();

	bool m_primed = false;
	int m_actionRange = 14;
	int m_range = 6;
	int m_cooldown = 0;
};

class Howler : public Monster {
public:
	// Causes lights in the level to fade in and out.
	Howler(Dungeon *dungeon, int x, int y);

	void move();

private:
	int m_wait = 3;
	int m_maxWait;
};

class Watcher : public Monster {
public:
	// Spawned from the GuardianWatcher trap.
	// A badass enemy. Moves every 3 turns, but moves twice on their turn.
	Watcher(Dungeon *dungeon, int x, int y);

	void move();

private:
	int m_wait = 3;
	int m_maxWait;
};

//		FLOOR VII
class Pikeman : public Monster {
public:
	Pikeman(Dungeon *dungeon, int x, int y);

	void move();
	void step();
	void deathDrops();

private:
	bool m_alerted;
	char m_direction;
};

class Shrinekeeper : public Monster {
public:
	Shrinekeeper(Dungeon *dungeon, int x, int y);

	void move();

private:
	bool canTeleportAdjacentToPlayer(int &ex, int &ey) const;
	void teleportAway();

	bool m_primed = false;
	int m_moveRange = 12;
	int m_attackRange = 5;
	int m_teleportRadius = 7;
	int m_wait = 3;
	int m_maxWait;

	int m_px;
	int m_py;
};

//		FLOOR VIII
class AbyssSummoner : public Monster {
public:
	// Summons a gaping maw underneath the player that will attempt to bite them.
	AbyssSummoner(Dungeon *dungeon, int x, int y);
	~AbyssSummoner();

	void move();

private:
	bool m_summoning = false;
	int m_summonTurns = 3;
	std::shared_ptr<Traps> m_maw = nullptr;

	int m_range = 20;
	int m_wait = 4;
	int m_maxWait;
};

class MagicalBerserker : public Monster {
public:
	MagicalBerserker(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type);
	void move();

private:
	void teleportAway();

	int m_teleportRadius = 8;
	int m_stage = 1;
	int m_wait = 1;
};


//		BOSSES
class Smasher : public Monster {
public:
	Smasher(Dungeon *dungeon);

	void moveTo(int x, int y, float time = 0.10f);
	void move();

	void pickMove();
	void move1();
	void move2();
	void avalanche();
	void move3();
	void move4();
	void move5();

	void moveSmasher(char dir = '-');

	void resetLeftward();
	void resetRightward();
	void resetUpward();
	void resetDownward();

	void attack(Player &p);
	void attack(Monster &m);
	void deathDrops();

	bool isActive() const { return m_moveActive; };
	void setActive(bool status) { m_moveActive = status; };
	bool isEnded() const { return m_moveEnding; };
	void setEnded(bool status) { m_moveEnding = status; };
	int getMove() const { return m_moveType; };
	void setMove(int move) { m_moveType = move; };
	bool isFrenzied() const { return m_frenzy; };
	void startFrenzy() { m_frenzy = true; };

private:
	// used for telling what direction smasher is moving in if executing rock slide
	char m_move = '0';

	bool m_moveActive;
	bool m_moveEnding;
	int m_moveType;
	bool m_frenzy;
};



#endif