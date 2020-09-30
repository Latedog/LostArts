#ifndef ACTORS_H
#define ACTORS_H

class cocos2d::Sprite;
class Dungeon;
class Afflictions;
class Weapon;
enum class ImbuementType;
enum class DamageType;
enum class StatType;
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
	Actors(const Actors &other);
	virtual ~Actors();

	virtual void moveTo(int x, int y, float time = 0.10f) { return; };

	int getPosX() const { return m_x; };
	int getPosY() const { return m_y; };
	Coords getPos() const;
	int getMaxHP() const { return m_maxhp; };
	int getHP() const { return m_hp; };
	int getArmor() const { return m_armor; };
	int getStr() const { return m_str; };
	int getDex() const { return m_dex; };
	int getInt() const { return m_int; };
	int getLuck() const { return m_luck; };
	int getCha() const { return m_cha; };
	std::shared_ptr<Weapon> getWeapon() const { return m_weapon; };
	std::string getName() const { return m_name; };
	cocos2d::Sprite* getSprite() const { return m_sprite; };
	std::string getImageName() const { return m_image; };
	bool hasAnimation() const { return m_hasAnimation; };
	std::string getAnimationFrames() const { return m_frames; };
	int getAnimationFrameCount() const { return m_frameCount; };
	int getFrameInterval() const { return m_frameInterval; };

	void setPosX(int x) { m_x = x; };
	void setPosY(int y) { m_y = y; };
	void setPos(int x, int y) { m_x = x; m_y = y; };
	void setPos(const Coords &pos);
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
	void setCha(int charisma) { m_cha = charisma; };
	void setLuck(int luck) { m_luck = luck; };
	void setWeapon(std::shared_ptr<Weapon> weapon) { m_weapon = weapon; };
	void setName(std::string name) { m_name = name; };
	void setSprite(cocos2d::Sprite* sprite) { m_sprite = sprite; };
	void setDungeon(Dungeon *dungeon) { m_dungeon = dungeon; };

	void increaseStatBy(StatType stat, int amount);
	void decreaseStatBy(StatType stat, int amount);

	virtual bool isPlayer() const { return false; };
	virtual bool isMonster() const { return false; };
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
	int findAffliction(std::string name) const;
	void addAffliction(std::shared_ptr<Afflictions> affliction);
	void removeAffliction(std::string name);

	bool lavaImmune() const { return m_lavaImmune; }; // immunity to lava
	bool isFlying() const { return m_flying; };
	bool isSturdy() const { return m_sturdy; }; // tells if this gets knocked back by attacks or not
	bool isHeavy() const { return m_heavy; }; // Indicates if they can be pushed

	bool hasBloodlust() const { return m_bloodlust; };

	void setLavaImmunity(bool immune) { m_lavaImmune = immune; };
	void setFlying(bool flying) { m_flying = flying; };
	void setSturdy(bool sturdy) { m_sturdy = sturdy; };
	void setHeavy(bool heavy) { m_heavy = heavy; };

	void setBloodlust(bool lust) { m_bloodlust = lust; };

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
	bool isBlinded() const { return m_blinded; };
	bool isDisabled() const { return m_disabled; };
	bool isWet() const { return m_wet; };

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
	void setBlinded(bool blind) { m_blinded = blind; };
	void setDisabled(bool flag) { m_disabled = flag; };
	void setWet(bool flag) { m_wet = flag; };

protected:
	void setImageName(std::string image) { m_image = image; };
	void setHasAnimation(bool hasAnimation) { m_hasAnimation = hasAnimation; };
	void setAnimationFrames(std::string frames) { m_frames = frames; };
	void setAnimationFrameCount(int count) { m_frameCount = count; };
	void setFrameInterval(int interval) { m_frameInterval = interval; };

	Dungeon *m_dungeon = nullptr;
	
private:
	virtual void reactToDamage() { return; };

	int m_x;
	int m_y;
	int m_maxhp;
	int m_hp;
	int m_armor;
	int m_str;
	int m_dex;
	int m_int = 0;
	int m_luck = 0;
	int m_cha = 0;
	std::shared_ptr<Weapon> m_weapon;
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

	bool m_lavaImmune = false;
	bool m_bloodlust = false;
	bool m_flying = false;
	bool m_sturdy = true;
	bool m_heavy = false;

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
	bool m_blinded = false;
	bool m_disabled = false;
	bool m_wet = false;

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
	~Player() override;

	std::shared_ptr<Player> createNewPlayer() const;
	virtual std::shared_ptr<Player> clone() const = 0;

	bool isPlayer() const override { return true; };

	void moveTo(int x, int y, float time = 0.10f) override;
	void move(char move);
	bool didAttack() const { return m_attackedEnemy; };
	void fightMonsterAt(const Coords &pos, DamageType type);
	void attack(Actors &a);
	void botchedAttack(Actors &a);
	void takeDamage(int damage, DamageType type);
	void reactToDamage(Monster &m);
	void death();

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
	
	size_t itemCount() const { return m_items.size(); };
	bool hasItems() const { return !m_items.empty(); };
	std::shared_ptr<Drops> itemAt(int index) const { return m_items[index]; };
	int maxItemCount() const { return m_maxItemCount; };

	// Active item (Spacebar)
	bool hasActiveItem() const { return getActiveItem() != nullptr; };
	virtual std::shared_ptr<Objects> getActiveItem() const = 0;
	virtual void equipActiveItem(std::shared_ptr<Objects> active) = 0;
	virtual void restoreActive(int repairs) { return; };

	// Used for HUD updating
	virtual bool activeHasMeter() const = 0; // For indicating if the HUD should display a meter
	virtual int getCurrentActiveMeter() const { return 0; }; // Active item's current durability/points
	virtual int getMaxActiveMeter() const { return -1; }; // Active item's maximum durability/points

	virtual bool canUseShield() const { return false; };

	virtual bool canBlock() const { return false; };

	/* If this returns true, @damage will be adjusted to reflect that the attack was blocked.
	*  NOTE: @damage should already be set before this function is called */
	virtual bool didBlockAttackAt(int x, int y, int &damage) { return false; };

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
	void removeItem(int index); // Trader & Item Thief
	void setItemToFront(int index); // Used for item quick access

	size_t passiveCount() const { return m_passives.size(); };
	bool hasPassives() const { return !m_passives.empty(); };
	std::shared_ptr<Passive> passiveAt(int index) const { return m_passives[index]; };
	void equipPassive(std::shared_ptr<Passive> passive);
	void removePassive(int index); // Trader

	bool hasRelic() const { return m_relic != nullptr; };
	std::shared_ptr<Relic> getRelic() const { return m_relic; };
	void equipRelic(std::shared_ptr<Relic> relic); // Shrine use
	void removeRelic(); // Shrine use

	char facingDirection() const { return m_facing; };
	char getAction() const { return m_action; };

	int getVision() const { return m_vision; };
	void setVision(int vision) { m_vision = vision; };
	void increaseVisionBy(int amount) { m_vision += amount; };
	void decreaseVisionBy(int amount) { 
		m_vision -= amount;
		if (m_vision < 0)
			m_vision = 0;
	};

	int getFavor() const { return m_favor; };
	void increaseFavorBy(int favor);
	void decreaseFavorBy(int favor);

	// Dual wielding bonus
	bool isDualWielding() const { return m_dualWield; };
	void setDualWielding(bool dual) { m_dualWield = dual; };

	bool spiritActive() const { return m_spiritActive; };
	void setSpiritActive(bool flag) { m_spiritActive = flag; };

	bool hasSkeletonKey() const;
	void checkKeyConditions();

	bool getWin() const { return m_winner; };
	void setWin(bool win) { m_winner = win; };
	std::string getCauseOfDeath() const { return m_causeOfDeath; };
	void setCauseOfDeath(std::string causeOfDeath) { m_causeOfDeath = causeOfDeath; };

	/* Special abilities gained from items */
	bool hasAfflictionAversion() const;

	bool hasExperienceGain() const;
	int getXP() const { return m_abilityBonuses.m_xp; };
	void increaseXPBy(int amount);

	/* Special abilities gained from passives */
	bool canLifesteal() const { return m_abilityBonuses.m_lifesteal; };
	void setLifesteal(bool steal) { m_abilityBonuses.m_lifesteal = steal; };
	void rollHeal();

	bool hasSteelPunch() const { return m_abilityBonuses.m_steelPunch; };
	void setSteelPunch(bool punch) { m_abilityBonuses.m_steelPunch = punch; };

	bool spikeImmunity() const { return m_abilityBonuses.m_spikeImmunity; };
	void setSpikeImmunity(bool immune) { m_abilityBonuses.m_spikeImmunity = immune; };

	bool hasPoisonTouch() const { return m_abilityBonuses.m_poisonTouch; };
	void setPoisonTouch(bool poison) { m_abilityBonuses.m_poisonTouch = poison; };

	bool hasFireTouch() const { return m_abilityBonuses.m_fireTouch; };
	void setFireTouch(bool fire) { m_abilityBonuses.m_fireTouch = fire; };

	bool hasFrostTouch() const { return m_abilityBonuses.m_frostTouch; };
	void setFrostTouch(bool frost) { m_abilityBonuses.m_frostTouch = frost; };

	bool explosionImmune() const { return m_abilityBonuses.m_explosionImmunity; };
	void setExplosionImmune(bool explosion) { m_abilityBonuses.m_explosionImmunity = explosion; };

	bool hasPotentPotions() const { return m_abilityBonuses.m_potentPotions; };
	void setPotentPotions(bool potent) { m_abilityBonuses.m_potentPotions = potent; };

	bool hasSoulSplit() const { return m_abilityBonuses.m_soulSplit; };
	void setSoulSplit(bool soul) { m_abilityBonuses.m_soulSplit = soul; };

	bool canBreakWalls() const { return m_abilityBonuses.m_wallBreak; };
	void setCanBreakWalls(bool wallBreak) { m_abilityBonuses.m_wallBreak = wallBreak; }

	bool hasCharismaNPC() const { return m_abilityBonuses.m_charismaNPC; };
	void setCharismaNPC(bool charisma) { m_abilityBonuses.m_charismaNPC = charisma; };
	
	bool hasCheapShops() const { return m_abilityBonuses.m_cheapShops; };
	void setCheapShops(bool cheap) { m_abilityBonuses.m_cheapShops = cheap; };

	bool hasBetterRates() const { return m_abilityBonuses.m_betterRates; };
	void setBetterRates(bool rates) { m_abilityBonuses.m_betterRates = rates; };

	bool hasTrapIllumination() const { return m_abilityBonuses.m_trapIllumination; };
	void setTrapIllumination(bool illuminate) { m_abilityBonuses.m_trapIllumination = illuminate; };

	bool hasItemIllumination() const { return m_abilityBonuses.m_itemIllumination; };
	void setItemIllumination(bool illuminate) { m_abilityBonuses.m_itemIllumination = illuminate; };

	bool hasMonsterIllumination() const { return m_abilityBonuses.m_monsterIllumination; };
	void setMonsterIllumination(bool illuminate) { m_abilityBonuses.m_monsterIllumination = illuminate; };

	bool hasGoldIllumination() const { return m_abilityBonuses.m_goldIllumination; };
	void setGoldIllumination(bool illuminate) { m_abilityBonuses.m_goldIllumination = illuminate; };

	bool hasGoldInvulnerability() const { return m_abilityBonuses.m_goldInvulnerability; };
	void setGoldInvulnerability(bool ability) { m_abilityBonuses.m_goldInvulnerability = ability; };

	bool hasResonantSpells() const { return m_abilityBonuses.m_resonantSpells; };
	void setResonantSpells(bool resonant) { m_abilityBonuses.m_resonantSpells = resonant; };

	bool isSlow() const { return m_abilityBonuses.m_slow; };
	void setSlow(bool slow) { m_abilityBonuses.m_slow = slow; };

	bool hasChainLightning() const { return m_abilityBonuses.m_chainLightning; };
	void setChainLightning(bool lightning) { m_abilityBonuses.m_chainLightning = lightning; };

	bool hasCripplingBlows() const { return m_abilityBonuses.m_cripplingBlows; };
	void setCripplingBlows(bool cripple) { m_abilityBonuses.m_cripplingBlows = cripple; };

	bool hasMatrixVision() const { return m_abilityBonuses.m_matrixVision; };
	void setMatrixVision(bool matrix) { m_abilityBonuses.m_matrixVision = matrix; };

	bool hasHarshAfflictions() const { return m_abilityBonuses.m_harshAfflictions; };
	void setHarshAfflictions(bool harsh) { m_abilityBonuses.m_harshAfflictions = harsh; };

	bool hasSpellRetaliation() const { return m_abilityBonuses.m_spellRetaliation; };
	void setSpellRetaliation(bool retaliate) { m_abilityBonuses.m_spellRetaliation = retaliate; };

	bool hasAfflictionOverride() const { return m_abilityBonuses.m_afflictionOverride; };
	void setAfflictionOverride(bool afflict) { m_abilityBonuses.m_afflictionOverride = afflict; };

	int getTimerReduction() const { return m_abilityBonuses.m_timerReduction; };
	void setTimerReduction(float reduction) { m_abilityBonuses.m_timerReduction = reduction; };

	bool hasFatStacks() const { return m_abilityBonuses.m_fatStacks; };
	void setFatStacks(bool stack) { m_abilityBonuses.m_fatStacks = stack; };

	bool hasBonusRoll() const { return m_abilityBonuses.m_bonusRoll; };
	void setBonusRoll(bool bonus) { m_abilityBonuses.m_bonusRoll = bonus; };

	bool hasFragileRetaliation() const { return m_abilityBonuses.m_fragileRetaliation; };
	void setFragileRetaliation(bool fragile) { m_abilityBonuses.m_fragileRetaliation = fragile; };

	bool hasScavenger() const { return m_abilityBonuses.m_scavenger; };
	void setScavenger(bool flag) { m_abilityBonuses.m_scavenger = flag; };

	bool hasDelayedHealing() const { return m_abilityBonuses.m_delayedHealing > 0; };
	float getDelayedHealing() const { return m_abilityBonuses.m_delayedHealing; };
	void setDelayedHealing(float healing) { m_abilityBonuses.m_delayedHealing = healing; }; // Healing is the percentage of max hp to restore
	/// End special abilities

protected:
	Player(const Player& other);

	virtual void resetFlags() {
		m_attackedEnemy = false;
	};

	virtual void checkExtraAttackEffects(Actors &a);
	void knockbackDirection(int &x, int &y, int mx, int my); // Determines the direction the enemy should be knocked back

	void dropWeapon();
	void dropStoredWeapon(); // Used mainly if player was dual wielding
	void addSecondaryWeapon(std::shared_ptr<Weapon> weapon) { m_storedWeapon = weapon; }; // For giving chracters a second starting weapon

private:
	void reactToDamage() override;

	virtual void useActiveItem() = 0;

	void setFacingDirection(char facing) { m_facing = facing; };
	void setAction(char action) { m_action = action; };

	int keyHP() const { return m_keyhp; };
	void setKeyHP(int keyhp) { m_keyhp = keyhp; };

	std::shared_ptr<Weapon> m_storedWeapon = nullptr;

	std::vector<std::shared_ptr<Drops>> m_items;
	std::vector<std::shared_ptr<Passive>> m_passives;

	std::shared_ptr<Relic> m_relic = nullptr;

	int m_maxarmor = 99;
	int m_maxstr = 99;
	int m_maxdex = 99;
	
	int m_maxItemCount = 5;

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

	struct AbilityBonuses {
		// From RPG in a bottle
		int m_xp = 0;
		int m_xpMax = 10;

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
	};

	AbilityBonuses m_abilityBonuses;

	/*
	// From RPG in a bottle
	int m_xp = 0;
	int m_xpMax = 10;

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
	*/
	
	bool m_spiritActive = false; // Indicator that the ForgottenSpirit is to spawn

	char m_facing = 'r'; // Choose 'right' arbitrarily as the default looking direction
	char m_action = '-'; // used for non-movement actions

	// Used for BenevolentBark enemy
	bool m_attackedEnemy = false;

	// keyhp is the limit before the skeleton key breaks.
	// it is set below the key's set minimum if player is already below this minimum threshold when they pick it up
	int m_keyhp;

	bool m_winner = false;
	std::string m_causeOfDeath;
};

class Adventurer : public Player {
public:
	// The standard character. Wields a shield.
	Adventurer();
	Adventurer(const Adventurer &other);
	~Adventurer() override;

private:
	std::shared_ptr<Player> clone() const override { return std::make_shared<Adventurer>(*this); };

	void resetFlags() override {
		Player::resetFlags();
		m_blocking = false;
	};

	std::shared_ptr<Objects> getActiveItem() const override { return std::static_pointer_cast<Objects>(m_shield); };
	void useActiveItem() override;
	void equipActiveItem(std::shared_ptr<Objects> active) override;

	bool activeHasMeter() const override { return true; };
	int getCurrentActiveMeter() const override;
	int getMaxActiveMeter() const override;
	void restoreActive(int repairs) override;

	bool canUseShield() const override { return true; };

	void equipShield(std::shared_ptr<Shield> shield);
	void dropShield();
	void shieldBroken();

	bool canBlock() const override;
	bool didBlockAttackAt(int x, int y, int &damage) override;

	void successfulBlock();

	std::shared_ptr<Shield> m_shield = nullptr;
	bool m_blocking = false;
};

class Spellcaster : public Player {
public:
	// Starts with Whirlwind as active item. Can only use spells.
	// Picking up other spells replaces the Spellcaster's current spell.
	Spellcaster();
	Spellcaster(const Spellcaster &other);
	~Spellcaster() override;

private:
	std::shared_ptr<Player> clone() const override { return std::make_shared<Spellcaster>(*this); };

	std::shared_ptr<Objects> getActiveItem() const override { return std::static_pointer_cast<Objects>(m_spell); };
	void useActiveItem() override;
	void equipActiveItem(std::shared_ptr<Objects> active) override;

	bool activeHasMeter() const override { return true; };
	int getCurrentActiveMeter() const override { return m_mp; };
	int getMaxActiveMeter() const override { return m_maxMP; };

	void checkExtraAttackEffects(Actors &a) override;

	std::shared_ptr<Spell> m_spell;
	int m_mp = 25; // magic casting points left
	int m_maxMP = 25;
};

class Spelunker : public Player {
public:
	// Starts with the Whip and uses Rocks as their active item.
	// Rocks can break, so all enemies have a chance to drop Rocks.
	Spelunker();
	Spelunker(const Spelunker &other);
	~Spelunker() override;

	std::shared_ptr<Player> clone() const override { return std::make_shared<Spelunker>(*this); };

	std::shared_ptr<Objects> getActiveItem() const override {
		if (!m_rocks.empty())
			 return std::static_pointer_cast<Objects>(m_rocks[0]);

		return nullptr;
	};
	void useActiveItem() override;
	void equipActiveItem(std::shared_ptr<Objects> active) override;

	int getRockCount() const { return m_rocks.size(); }; // For HUD purposes

	bool activeHasMeter() const override { return false; };

private:
	std::vector<std::shared_ptr<Rocks>> m_rocks;
};

class Acrobat : public Player {
public:
	// The Acrobat's ability allows them to roll/maneuver past enemies and over obstacles.
	// They have a stamina meter though
	Acrobat();
	Acrobat(const Acrobat &other);
	~Acrobat() override;

private:
	std::shared_ptr<Player> clone() const override { return std::make_shared<Acrobat>(*this); };

	std::shared_ptr<Objects> getActiveItem() const override { return std::static_pointer_cast<Objects>(m_item); };
	void useActiveItem() override;
	void equipActiveItem(std::shared_ptr<Objects> active) override;

	bool activeHasMeter() const override { return true; };
	int getCurrentActiveMeter() const override { return m_stamina; };
	int getMaxActiveMeter() const override { return m_maxStamina; };

	void checkExtraAttackEffects(Actors &a) override;

	std::shared_ptr<Drops> m_item = nullptr;
	int m_stamina;
	int m_maxStamina;
};

class TheMadman : public Player {
public:
	// Active item is a teleporter. Cannot be replaced.
	// Starts with no weapon but increased vision radius.
	TheMadman();
	TheMadman(const TheMadman &other);
	~TheMadman() override;

private:
	std::shared_ptr<Player> clone() const override { return std::make_shared<TheMadman>(*this); };

	std::shared_ptr<Objects> getActiveItem() const override { return std::static_pointer_cast<Objects>(m_item); };
	void useActiveItem() override;
	void equipActiveItem(std::shared_ptr<Objects> active) override;

	bool activeHasMeter() const override { return false; };

	std::shared_ptr<Drops> m_item;
};


// ============================================
//			 	  :::: NPCS ::::
// ============================================

class NPC : public Actors {
public:
	NPC(Dungeon *dungeon, int x, int y, std::string name, std::string image);

	void talk();
	void provoke();

	virtual void useResponse(int index) { return; }; // Index of the choice that the player made in the choices vector

	std::vector<std::string> getDialogue() const { return m_dialogue; };
	std::vector<std::string> getChoices() const { return m_promptChoices; };

protected:
	void addDialogue(std::string line) { m_dialogue.push_back(line); };
	void addChoice(std::string line) { m_promptChoices.push_back(line); };

	void setSatisfaction(bool satisfied) { m_satisfied = satisfied; };
	void rewardWasGiven() { m_rewardGiven = true; };
	void successfullyRescued() { m_rescued = true; };
	void setUnrescued() { m_rescued = false; };

	int getInteractionStage() const { return m_interactedDialogueStage; };
	void incInteractionStage() { m_interactedDialogueStage++; };
	int getInteractionLimit() const { return m_interactionLimit; };
	void setInteractionLimit(int limit) { m_interactionLimit = limit; };

	// Indicates what stage of prompt we're at so that we use
	// the appropriate set of choices.
	int m_promptStage = 1;

	// Indicates the dialogue stage when the NPC is in the Hub.
	int m_hubDialogueStage = 1;

	std::vector<std::string> m_dialogue;
	std::vector<std::string> m_promptChoices;

private:
	void playDialogue();

	virtual void checkSatisfaction() = 0;
	virtual void reward() = 0;

	virtual void addInitialDialogue() { return; };
	virtual void addInteractedDialogue() = 0;
	virtual void addSatisfiedDialogue() = 0;
	virtual void addFinalDialogue() = 0;

	virtual void checkIfRescued() { return; };

	virtual void addInitialUnrescuedDialogue() { return; };
	virtual void addInteractedUnrescuedDialogue() { return; };
	virtual void addRescuedDialogue() { return; };
	virtual void addFinalRescuedDialogue() { return; };

	virtual void addHubDialogue() { return; };

	bool m_interacted = false; // flag for initial interaction with player
	bool m_satisfied = false; // flag for determining if player has met the NPCs request
	bool m_rewardGiven = false; // flag so that players are not given more than one reward

	// Indicates if the npc must be saved from somewhere within the ruins.
	// True indicates that they do not need to be rescued.
	bool m_rescued = true;

	bool m_unrescuedInteracted = false;

	// When an NPC is continuously interacted with, this keeps track of what lines to output next
	// if the player keeps talking to them, up to a certain limit
	int m_interactedDialogueStage = 1;
	int m_interactionLimit;
};

class CreatureLover : public NPC {
public:
	CreatureLover(Dungeon *dungeon, int x, int y);

private:
	void checkSatisfaction();
	void reward();

	void addInitialDialogue();
	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

	std::string m_wantedCreature;
};

class Memorizer : public NPC {
public:
	Memorizer(Dungeon *dungeon, int x, int y);

private:
	void useResponse(int index);
	void checkSatisfaction();
	void reward();

	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

	std::string m_topic;
	std::string m_correctChoice;

	// Since there are multiple prompts, this indicates what stage of prompt we're at so that we use
	// the appropriate set of choices.
	//int m_promptStage = 1;
};

class InjuredExplorer : public NPC {
public:
	InjuredExplorer(Dungeon *dungeon, int x, int y);

private:
	void useResponse(int index);
	void checkSatisfaction();
	void reward();

	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

	bool playerHasHealingItem() const;

	int m_healingItemCount = 0;
};

class PortalResearcher : public NPC {
public:
	// This NPC will be responsible for allowing the player to use shortcuts in the World Hub.
	// They need to be found first before they appear in the Hub.
	// Once found, they will require special charms to allow them to open portal shortcuts to the later floors.
	// However, the player must still first figure out how to unlock the shortcuts from one end first.
	PortalResearcher(Dungeon *dungeon, int x, int y);

private:
	void useResponse(int index);
	void checkSatisfaction();
	void reward();

	void addInitialDialogue();
	void addInteractedDialogue();
	void addSatisfiedDialogue();
	void addFinalDialogue();

	virtual void checkIfRescued();

	virtual void addInitialUnrescuedDialogue();
	virtual void addInteractedUnrescuedDialogue();
	virtual void addRescuedDialogue();
	virtual void addFinalRescuedDialogue();

	virtual void addHubDialogue();
};

class Shopkeeper : public NPC {
public:
	Shopkeeper(Dungeon *dungeon, int x, int y);

protected:
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

protected:
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

protected:
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

protected:
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

class TutorialNPC1 : public NPC {
public:
	// A man that hangs outside the entrance to the world hub
	TutorialNPC1(Dungeon *dungeon, int x, int y);

protected:
	void checkSatisfaction() { return; };
	void reward() { return; };

	void addInteractedDialogue();
	void addSatisfiedDialogue() { return; };
	void addFinalDialogue() { return; };
};

class TutorialNPC2 : public NPC {
public:
	// A man that hangs outside the entrance to the world hub
	TutorialNPC2(Dungeon *dungeon, int x, int y);

protected:
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

protected:
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

	bool isMonster() const override { return true; };

	// Moves the monster to the coordinates @x and @y
	void moveTo(int x, int y, float time = 0.10f) override;

	void moveCheck();
	void death();
	virtual void reactToDamage(Player &player) { return; };

	bool wasDestroyed() const { return m_destroyed; };
	void setDestroyed(bool destroyed) { m_destroyed = destroyed; };

	DamageType getDamageType() const { return m_damageType; };

	virtual bool canBeDamaged(DamageType type) { return true; }; // Indicates if the enemy can be damaged (not armored, etc.)
	virtual bool canBeHit() { return true; }; // Indicates if enemy can by physically struck (i.e. sand centipedes when underground)
	virtual bool isUnderground() { return false; }; // For Sand Centipede

	virtual bool isMonsterSegment() const { return false; } // Indicates if this is a monster segment. Used for ignoring traps.
	virtual bool isBreakable() const { return false; } // Indicates if this is a breakable object
	virtual bool isBoss() const { return false; };

	virtual int getParentX() const { return getPosX(); }; // X coordinate of the segmented monster's main part
	virtual int getParentY() const { return getPosY(); }; // Y coordinate of the segmented monster's main part

	bool hasExtraSprites() { return m_hasExtraSprites; };
	virtual void updateSpriteLighting(const cocos2d::Color3B &color) { return; };
	bool emitsLight() const { return m_emitsLight; }; // tells if the sprite should emit extra light
	virtual void addLightEmitters(std::vector<Coords> &lightEmitters) { return; };

protected:
	virtual void move() = 0;
	virtual void attack(Player &p);
	virtual void extraAttackEffects() { return; };
	void damagePlayer(int damage, DamageType type);

	virtual void extraDeathEffects(); // For non drop related effects that should occur after death
	virtual void deathDrops() { return; };
	virtual void spriteCleanup();

	bool chase(char &best, bool all = false, bool diagonalsOnly = false);
	bool attemptChase(int range, char &best) {
		char first;
		first = best = '0';
		int shortest = 0;

		return attemptChase(*m_dungeon, shortest, range, range, getPosX(), getPosY(), first, best);
	};
	void attemptGreedyChase(bool diagonals = false, int x = -1, int y = -1); // @x and @y are used specifically for decoys
	void attemptDiagonalGreedyChase(); // Diagonal movement only

	char attemptDryRunGreedyChase(bool diagonals = false); // Attempt greedy chase without actually moving
	bool attemptDryRunMove(char move);

	bool attemptMove(char move);
	char moveMonsterRandomly(bool diagonals = false);

	bool playerInRange(int range) const;
	bool playerInDiagonalRange(int range) const;
	bool playerIsAdjacent(bool diagonals = false) const;

	void setDamageType(DamageType type) { m_damageType = type; };
	void setGold(int gold) { m_gold = gold; };
	void setChasesPlayer(bool chases) { m_chases = chases; };
	void setSmart(bool smart) { m_smart = smart; };
	void setHasWeapon(bool hasWeapon) { m_hasWeapon = hasWeapon; };
	void setExtraSpritesFlag(bool extras) { m_hasExtraSprites = extras; };
	void setEmitsLight(bool emits) { m_emitsLight = emits; };

private:
	bool attemptChase(Dungeon &dungeon, int &shortest, int currentDist, int origdist, int x, int y, char &first_move, char &optimal_move);

	bool m_destroyed = false; // Indicates that monster should not drop any rewards

	DamageType m_damageType;

	int m_gold = 0;
	bool m_chases = false;
	bool m_smart = false; // Indicates if monster (that chases), will or will not walk on lethal traps
	bool m_hasWeapon = false;

	bool m_hasExtraSprites = false;
	bool m_emitsLight = false;
};

// Breakables
class Breakables : public Monster {
public:
	// Breakables can be destroyed and may contain items inside
	Breakables(Dungeon *dungeon, int x, int y, int hp, int armor, std::string name, std::string image);

	bool isBreakable() { return true; };

protected:
	virtual bool canBeDamaged(DamageType type) = 0;
	void move() { return; };
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

private:
	void deathDrops();
};

class WeakBarrel : public SturdyBreakables {
public:
	WeakBarrel(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };

private:
	void deathDrops();
};

class SmallPot : public SturdyBreakables {
public:
	SmallPot(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };

private:
	void deathDrops();
};

class LargePot : public SturdyBreakables {
public:
	LargePot(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };

private:
	void deathDrops();
};

class Sign : public SturdyBreakables {
public:
	Sign(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };

private:
	void deathDrops();
};

class ArrowSign : public SturdyBreakables {
public:
	ArrowSign(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };

private:
	void deathDrops();
};

class ExplosiveBarrel : public Breakables {
public:
	ExplosiveBarrel(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };

private:
	void deathDrops();
};

class CharredWood : public Breakables {
public:
	CharredWood(Dungeon *dungeon, int x, int y);

	bool canBeDamaged(DamageType type) { return true; };

private:
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

private:
	void moveTo(int x, int y, float time = 0.10f);
	void move();
	void attack(Player &p);

	int m_turns = 4;
};

class Ghost : public Monster {
public:
	// The ghost is a rare enemy that can go through walls.
	// If it touches the player, they become possessed and cannot control their movements
	// for a few turns.
	Ghost(Dungeon *dungeon, int x, int y, int range = 15);

private:
	void move();
	void attack(Player &p);

	int m_range;
};

class Flare : public Monster {
public:
	// Flares are released by FlareCandles. They chase the player and inflict burns on contact.
	Flare(Dungeon *dungeon, int x, int y);

private:
	void move();
	void attack(Player &p);
	void reactToDamage(Player &player) override;

	int m_range = 15;
};

class MonsterSegment : public Monster {
public:
	// Monster Segments are simply parts of a monster that can be attacked or interacted with.
	// They typically don't do anything other than moving around with the main monster's body section.
	MonsterSegment(Dungeon *dungeon, int x, int y, int sx, int sy, std::string name, std::string image);

	bool isMonsterSegment() const { return true; }

	void setParentX(int x) { m_parentX = x; };
	void setParentY(int y) { m_parentY = y; };
	virtual int getParentX() const { return m_parentX; }; // X coordinate of the segmented monster's parent
	virtual int getParentY() const { return m_parentY; }; // Y coordinate of the segmented monster's parent

private:
	bool canBeDamaged(DamageType type);
	void move() { return; };

	// Coordinates of the main monster to which it is attached
	int m_parentX;
	int m_parentY;

	bool m_damaged = false;
};

//		FLOOR I
class Seeker : public Monster {
public:
	Seeker(Dungeon *dungeon, int x, int y, int range = 10);
	Seeker(Dungeon *dungeon, int x, int y, int range, std::string name, std::string image);

private:
	void move();

	int m_range;
	bool m_step;
};

class GooSack : public Monster {
public:
	// This enemy is stationary, but if the player is close, it will attempt to jump on them
	GooSack(Dungeon *dungeon, int x, int y);

private:
	void move();
	void attack(Player &p);

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

private:
	void move();
	void attack(Player &p);

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

private:
	void move();
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

protected:
	bool playerIsCaptured() const { return m_captured; };
	void setCapture(bool captured) { m_captured = captured; };

private:
	void move();
	void attack(Player &p);
	virtual bool specialMove() { return false; };
	void createWebs(int x, int y);

	std::vector<Coords> m_webs; // coordinates of the webs
	bool m_captured = false; // Flag indicating if the player stepped into a spider's web
	int m_webCount; // number of webs the spiders will create up to

	bool m_primed = false;
	bool m_wait = false;
};

class ShootingSpider : public Spider {
public:
	ShootingSpider(Dungeon *dungeon, int x, int y, int webCount = 4, int range = 5);

private:
	bool specialMove();

	int m_range;
	int m_turns;
	int m_maxTurns;
	bool m_primed = false;
};

class PouncingSpider : public Monster {
public:
	// Pouncing Spiders are territorial and will guard a region from outsiders.
	PouncingSpider(Dungeon *dungeon, int x, int y);

private:
	void move();

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

private:
	void move();
	void attack(Player &p);
	virtual void specialMove() { return; };
	void setDirection(char dir) { m_dir = dir; };

	char m_dir;
};

class Goblin : public Monster {
public:
	Goblin(Dungeon *dungeon, int x, int y, int range = 10);

private:
	void move();
	void deathDrops();

	int m_range;
};

class Wanderer : public Monster {
public:
	Wanderer(Dungeon *dungeon, int x, int y);
	Wanderer(Dungeon *dungeon, int x, int y, std::string name, std::string image, int hp, int armor, int str, int dex);

private:
	void move();
	void deathDrops();
};

class SleepingWanderer : public Wanderer {
public:
	SleepingWanderer(Dungeon *dungeon, int x, int y);

private:
	void move();
	void deathDrops();

	bool m_provoked = false;
	int m_range = 8;
	int m_provocationRange = 4;
};

class ProvocableWanderer : public Wanderer {
public:
	ProvocableWanderer(Dungeon *dungeon, int x, int y);

private:
	void move();
	void attack(Player &p);
	void deathDrops();

	bool m_provoked = false;
	int m_turns = 2;
	int m_prevHP;
};

//		FLOOR II
class DeadSeeker : public Seeker {
public:
	DeadSeeker(Dungeon *dungeon, int x, int y, int range = 13);

private:
	int m_range;
	bool m_step;
};

class RabidWanderer : public Wanderer {
public:
	// Chance to poison the player
	RabidWanderer(Dungeon *dungeon, int x, int y);

private:
	void move();
	void extraAttackEffects();
	void deathDrops();

	bool m_turn = false;
};

class Toad : public Monster {
public:
	// Toads spit out poisonous puddles
	Toad(Dungeon *dungeon, int x, int y);

private:
	void move();

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

private:
	void move();
	void deathDrops();
};

class Piranha : public Monster {
public:
	Piranha(Dungeon *dungeon, int x, int y);

private:
	void moveTo(int x, int y, float time = 0.10f);
	void move();
	void attemptMoveInWater();
};

class WaterSpirit : public Monster {
public:
	WaterSpirit(Dungeon *dungeon, int x, int y);

private:
	void move();
	void deathDrops();
	void douseTile(const Coords &pos);

	int m_range = 15;
	int m_wait = 1;
	int m_maxWait;
};

class AngledBouncer : public Monster {
public:
	AngledBouncer(Dungeon *dungeon, int x, int y);

private:
	void move();
	void changeDirection();

	char m_dir;
};

class Firefly : public Monster {
public:
	// Harmless, but provides light.
	Firefly(Dungeon *dungeon, int x, int y);

private:
	void move();

	int m_wait = 3;
	int m_maxWait;
};

class TriHorn : public Monster {
public:
	// Primes for a triple-tile attack.
	TriHorn(Dungeon *dungeon, int x, int y);

private:
	void move();

	char m_dir;
	int m_range = 2;
	bool m_primed = false;
	int m_wait = 2;
	int m_maxWait;
};

class TumbleShroom : public Monster {
public:
	TumbleShroom(Dungeon *dungeon, int x, int y);

private:
	void move();
	void moveToWithFlip(int x, int y, float rotation);
	void reactToDamage(Player &player) override;
	void releaseSpores();

	bool m_flipped = false;
	int m_flippedTurns = 0;

	bool m_primed = false;
	int m_primedWait = 0;

	int m_range = 12;
	int m_wait = 1;
	int m_maxWait;
};

class Wriggler : public Monster {
public:
	static std::shared_ptr<Wriggler> create(Dungeon *dungeon, const Coords &pos);

	/* NOTE: Do not call this directly, use create() instead. */
	Wriggler(Dungeon *dungeon, int x, int y);

	/* WARNING: May only be called by Wriggler's addSegments function. */
	Wriggler(Dungeon *dungeon, int x, int y, std::string image, bool isHead, std::weak_ptr<Wriggler> prev);

private:
	void move();
	void extraDeathEffects();

	void addSegments(std::shared_ptr<Wriggler> head);
	void convertToHead();

	int m_length = 8;

	char m_dir = '-';
	bool m_isHead = false;
	std::weak_ptr<Wriggler> m_prev;
	std::weak_ptr<Wriggler> m_next;

	bool m_primed = false;
};

class BarbedCaterpillar : public Monster {
public:
	BarbedCaterpillar(Dungeon *dungeon, int x, int y);

private:
	void move();
	void extraAttackEffects();
	void reactToDamage(Player &player) override;

	bool m_willBeGood; // Determines if it will turn into a good or bad butterfly

	int m_caterpillarTurns;
	int m_wait = 1;
	int m_maxWait;

	bool m_cocoon = false;
	int m_cocoonTurns;

	bool m_transformed = false;

	int m_range = 5;
};

//		FLOOR III
class CrystalTurtle : public Monster {
public:
	// 
	CrystalTurtle(Dungeon *dungeon, int x, int y);

private:
	void move();

	int m_wait = 4;
	int m_maxWait;

	bool m_hit = false;
	int m_prevHP;
};

class CrystalHedgehog : public Monster {
public:
	// 
	CrystalHedgehog(Dungeon *dungeon, int x, int y, char dir = '-');

private:
	void move();

	int m_wait = 7;
	int m_maxWait;
	char m_dir;
};

class CrystalShooter : public Monster {
public:
	// 
	CrystalShooter(Dungeon *dungeon, int x, int y);

private:
	void move();

	int m_wait = 5;
	int m_maxWait;
};

class CrystalBeetle : public Monster {
public:
	// 
	CrystalBeetle(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();

	char m_dir;
	int m_range = 10;
	int m_wait = 2;
	int m_maxWait;
	int m_backShield = 2; // Hits before the back shield is broken
};

class CrystalWarrior : public Monster {
public:
	CrystalWarrior(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();

	bool m_primed = false;
	char m_dir;
	int m_shieldStrength = 3;

	int m_wait = 2;
	int m_maxWait;
};

class Rabbit : public Monster {
public:
	// 
	Rabbit(Dungeon *dungeon, int x, int y);

private:
	void move();
	void run();

	bool m_beingChased = false;
	int m_range = 6;
	int m_runSteps = 3;
	int m_maxRunSteps;
};

class Bombee : public Monster {
public:
	Bombee(Dungeon *dungeon, int x, int y, int range = 11);
	Bombee(Dungeon *dungeon, int x, int y, int range, std::string name, std::string image, int hp, int armor, int str, int dex);

private:
	void move();
	void attack(Player &p);
	void deathDrops();

	int m_fuse;
	bool m_fused;
	int m_aggroRange;
};

class BenevolentBark : public Monster {
public:
	// Follows player without attacking, but if they or
	// any other "enemy" is damaged (including breakables), then they
	// chase the player until they attempt an attack on them.
	BenevolentBark(Dungeon *dungeon, int x, int y);

private:
	void move();

	bool m_attackMode = false;

	int m_range = 15;
	int m_stopRange = 3;
};

class Tick : public Monster {
public:
	Tick(Dungeon *dungeon, int x, int y);

private:
	void move();

	bool m_firstMove = true;
	bool m_primed = false;
	int m_px;
	int m_py;

	int m_wait = 0;
	int m_maxWait = 1;
};

class ExoticFeline : public Monster {
public:
	ExoticFeline(Dungeon *dungeon, int x, int y);

private:
	void move();
	void reactToDamage(Player &player) override;

	void cloak();
	void uncloak();

	char m_dir;
	bool m_staggered = false;
	int m_preyRange = 15;
	int m_approachRange = 10;
	int m_attackRange = 6;
	bool m_primed = false;
};

//		FLOOR IV
class FlameWanderer : public Wanderer {
public:
	// Moves every other turn and has a chance to ignite.
	// If the player is adjacent, then this is guaranteed to attack.
	FlameWanderer(Dungeon *dungeon, int x, int y);

private:
	void move();
	void extraAttackEffects();

	bool m_turn = false;
};

class Zapper : public Monster {
public:
	Zapper(Dungeon *dungeon, int x, int y);

private:
	void moveTo(int x, int y, float time = 0.10f);
	void move();
	void attack(Player &p);

	void spriteCleanup();

	void updateSpriteLighting(const cocos2d::Color3B &color) override;
	void addLightEmitters(std::vector<Coords> &lightEmitters);

	void moveSprites(int x, int y);

	bool m_cooldown;
	bool m_cardinalAttack;

	std::map<int, cocos2d::Sprite*> m_sparks;
};

class Spinner : public Monster {
public:
	Spinner(Dungeon *dungeon, int x, int y);

private:
	void moveTo(int x, int y, float time = 0.10f);
	void move();
	void attack(Player &p);

	void spriteCleanup();

	void updateSpriteLighting(const cocos2d::Color3B &color) override;
	void addLightEmitters(std::vector<Coords> &lightEmitters);

	void setInitialFirePosition(int x, int y);
	void setFirePosition(char move);
	bool playerWasHit() const;
	void setSpriteVisibility(bool visible);

	bool m_clockwise;
	int m_angle;

	std::shared_ptr<Objects> m_innerFire;
	std::shared_ptr<Objects> m_outerFire;
};

class CharredBombee : public Bombee {
public:
	// The main difference is that this bombee is immune to lava
	CharredBombee(Dungeon *dungeon, int x, int y, int range = 10);

private:
	void attack(Player &p);
};

class FireRoundabout : public Roundabout {
public:
	FireRoundabout(Dungeon *dungeon, int x, int y);

private:
	void specialMove();
};

class ItemThief : public Monster {
public:
	ItemThief(Dungeon *dungeon, int x, int y, int range = 7);

private:

	void move();
	void run();
	void attack(Player &p);
	void deathDrops();

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

private:
	void move();
	void attack(Player &p);

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

private:
	void moveTo(int x, int y, float time = 0.10f);
	void move();
	void extraAttackEffects();
	void extraDeathEffects();

	bool setTailPosition();
	void rerollMonsterPosition(int &x, int &y);

	void addSegments();
	void moveSegments(int x, int y);

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

private:
	bool canBeDamaged(DamageType type);
};

class CombustionGolem : public Monster {
public:
	CombustionGolem(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();
	void deathDrops();

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

private:
	void move();
	void deathDrops();

	int m_wait = 10;
	int m_maxWait;
};

//		FLOOR V
class Puff : public Monster {
public:
	Puff(Dungeon *dungeon, int x, int y, int turns = 4, int pushDist = 2, int range = 2);
	Puff(Dungeon *dungeon, int x, int y, int turns, int pushDist, int range, std::string name, std::string image, int hp, int armor, int str, int dex);

protected:
	// Indicates if this Puff pushes in all 8 directions.
	// The regular version only pushes in the 4 cardinal directions.
	virtual bool pushesAll() { return false; };

private:
	void move();

	int m_turns;
	int m_maxTurns;
	int m_pushDist;
	int m_range;
};

class GustyPuff : public Puff {
public:
	// Pushes in all directions a maximum of one tile
	GustyPuff(Dungeon *dungeon, int x, int y);

protected:
	bool pushesAll() { return true; };
};

class StrongGustyPuff : public Puff {
public:
	// Pushes in all directions a maximum of two tiles
	StrongGustyPuff(Dungeon *dungeon, int x, int y);

protected:
	bool pushesAll() { return true; };
};

class InvertedPuff : public Monster {
public:
	// InvertedPuffs suck players and enemies in instead of pushing them away
	// @pullDist is how from how far away from where the Puff currently is it will try to pull.
	// For instance, P O o X: If P is the Puff, it will pull in the 'o' but not the X.
	InvertedPuff(Dungeon *dungeon, int x, int y, int turns = 4, int pullDist = 2, int range = 1);
	InvertedPuff(Dungeon *dungeon, int x, int y, int turns, int pullDist, int range, std::string name, std::string image, int hp, int armor, int str, int dex);

protected:
	// Indicates if this Puff pushes in all 8 directions.
	// The regular version only pushes in the 4 cardinal directions.
	virtual bool pullsAll() { return false; };

	virtual void specialMove() { return; };

private:
	void move();

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

protected:
	void attack(Player &p);
	void specialMove();
};

class GustySpikedInvertedPuff : public SpikedInvertedPuff {
public:
	// Pulls in all 8 directions
	GustySpikedInvertedPuff(Dungeon *dungeon, int x, int y);

private:
	void specialMove();
	bool pullsAll() { return true; };
};

class JeweledScarab : public Monster {
public:
	// Rare enemy with erratic movement. Drops passive if killed.
	JeweledScarab(Dungeon *dungeon, int x, int y);

private:
	void move();
	void deathDrops();

	void coordCheck(std::vector<Coords> &coords, bool &moved);
};

class ArmoredBeetle : public Monster {
public:
	// 
	ArmoredBeetle(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();

	int m_wait;
	int m_maxWait;
};

class SpectralSword : public Monster {
public:
	// 
	SpectralSword(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();
	void reactToDamage(Player &player) override;

	char m_dir;
	bool m_attacked = false;
	bool m_primed = false;
};

class SandCentipede : public Monster {
public:
	// Lurks underneath in sand. Chases player if they are in the sand.
	// Can't be hit until they lunge upwards.
	SandCentipede(Dungeon *dungeon, int x, int y);

private:
	void moveTo(int x, int y, float time = 0.10f);
	bool canBeDamaged(DamageType type);
	bool canBeHit();
	bool isUnderground();

	void move();
	void extraDeathEffects();

	bool m_primed = false;
	int m_wait;
	int m_maxWait;
	int m_range = 12;
};

class SandBeaver : public Monster {
public:
	SandBeaver(Dungeon *dungeon, int x, int y);

private:
	void move();

	bool m_primed = false;
	int m_moveRange = 15;
	int m_chaseRange = 8;
	int m_attackRange = 3;

	char m_dir;
	int m_wait = 2;
	int m_maxWait;
};

class SandAlbatross : public Monster {
public:
	SandAlbatross(Dungeon *dungeon, int x, int y);

private:
	void move();
	void run();
	void moveSideways();

	char m_dir;
	bool m_primed = false;
	int m_wait = 1;
	int m_maxWait;
	int m_range = 15;
	int m_moveTurns = 2;

};

class Archer : public Monster {
public:
	Archer(Dungeon *dungeon, int x, int y);

private:
	void move();
	void deathDrops();

	bool m_primed = false;
	int m_wait = 0;
	int m_range = 10;
};

//		FLOOR VI
class Wisp : public Monster {
public:
	Wisp(Dungeon *dungeon, int x, int y);

private:
	void move();

	int m_range = 6;
	int m_wait = 4;
	int m_maxWait;
};

class LightningWisp : public Monster {
public:
	LightningWisp(Dungeon *dungeon, int x, int y);

private:
	void move();

	bool m_cardinal;
	int m_wait = 1;
	int m_maxWait;
};

class Grabber : public Monster {
public:
	Grabber(Dungeon *dungeon, int x, int y);

private:
	void move();

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

private:
	void moveTo(int x, int y, float time = 0.1f);
	bool canBeDamaged(DamageType type);
	bool canBeHit();
	bool isUnderground();

	void move();

	int m_range = 12;
	bool m_revealed = false;
	bool m_primed = false;
	bool m_hittable = false;
};

class CrawlingSpine : public Monster {
public:
	CrawlingSpine(Dungeon *dungeon, int x, int y);

private:
	void move();

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

private:
	void move();
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

private:
	void move();

	int m_wait = 3;
	int m_maxWait;
};

class FacelessHorror : public Monster {
public:
	FacelessHorror(Dungeon *dungeon, int x, int y);

private:
	void move();

	char m_dir;

	bool m_primed = false;
	int m_primedWait = 0;

	int m_wait = 1;
	int m_maxWait;
	int m_range = 15;
};

class ToweringBrute : public Monster {
public:
	ToweringBrute(Dungeon *dungeon, int x, int y);

private:
	void move();

	int m_wait = 2;
	int m_maxWait;
};

class SkyCrasher : public Monster {
public:
	SkyCrasher(Dungeon *dungeon, int x, int y);

private:
	bool canBeHit();
	void move();

	void getPrimedTowardsPlayer();
	void setLandingPosition(bool reposition = false);

	int m_moveRange = 15;
	int m_attackRange = 5;
	bool m_primed = false;
	int m_wait = 1;
	int m_maxWait;

	int m_x;
	int m_y;
};

class SteamBot : public Monster {
public:
	SteamBot(Dungeon *dungeon, int x, int y);

private:
	void move();

	// 1-2: Greedy chase, 3: Random move, 4: Do nothing, 5: Attack
	int m_stage = 1;
	int m_stageMax = 5;

	int m_range = 15;
};

class Watcher : public Monster {
public:
	// Spawned from the GuardianWatcher trap.
	// A badass enemy. Moves every 3 turns, but moves twice on their turn.
	Watcher(Dungeon *dungeon, int x, int y);

private:
	void move();

	int m_range = 20;
	int m_wait = 3;
	int m_maxWait;
};

//		FLOOR VII
class Pikeman : public Monster {
public:
	Pikeman(Dungeon *dungeon, int x, int y);

private:
	void move();
	void step();
	void deathDrops();

	bool m_alerted;
	char m_direction;
};

class Shrinekeeper : public Monster {
public:
	Shrinekeeper(Dungeon *dungeon, int x, int y);

private:
	void move();
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

class Swapper : public Monster {
public:
	Swapper(Dungeon *dungeon, int x, int y);

private:
	void move();
	void swap();

	int m_moveRange = 15;
	int m_swapRange = 7;

	bool m_primed = false;
	int m_wait = 1;
	int m_maxWait;

	int m_px;
	int m_py;
};

class ShieldMaster : public Monster {
public:
	ShieldMaster(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void moveTo(int x, int y, float time = 0.1f);
	void move();
	void spriteCleanup();

	void moveShieldsTo(int x, int y, float time = 0.1f);
	void rotateShields(int x, int y, float time = 0.1f);

	void updateSpriteLighting(const cocos2d::Color3B &color) override;

	bool m_clockwise;
	std::vector<std::pair<char, cocos2d::Sprite*>> m_shields;
	
	int m_range = 15;
	char m_exposedDir;
	int m_wait = 1;
	int m_maxWait;
};

class PseudoDoppel : public Monster {
public:
	PseudoDoppel(Dungeon *dungeon, int x, int y);

private:
	void move();

	bool m_mirror = false;
};

class Electromagnetizer : public Monster {
public:
	Electromagnetizer(Dungeon *dungeon, int x, int y);

private:
	void extraAttackEffects();
	void move();
	void extraDeathEffects();

	int m_lightStolen = 0;
	int m_range = 15;
	bool m_primed = false;
	int m_wait = 1;
	int m_maxWait;
};

class BladePsychic : public Monster {
public:
	BladePsychic(Dungeon *dungeon, int x, int y);

private:
	void move();

	int m_range = 5;
	int m_moveRange = 10;
	int m_wait = 1;
	int m_maxWait;

	// Turns until another blade can be launched
	int m_bladeWait = 0;
	int m_maxBladeWait = 10;
};

class DashMaster : public Monster {
public:
	DashMaster(Dungeon *dungeon, int x, int y);

private:
	void move();

	bool m_primed = false;
	int m_range = 18;
	int m_wait = 4;
	int m_maxWait;

	int m_primedTurns = 0;
};

class AcidicBeast : public Monster {
public:
	// Alternates between two directions as they move.
	// Leaves pools of acid where they walk.
	AcidicBeast(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();

	void changeDirections();

	bool m_switch = false;
	char m_dir1;
	char m_dir2;

	int m_leapWait = 0;
	int m_maxLeapWait = 5;
};

class DarkCanine : public Monster {
public:
	DarkCanine(Dungeon *dungeon, int x, int y);

private:
	void extraAttackEffects();
	void move();
	void reactToDamage(Player &player) override;

	int m_latchedTurns = 0;
	int m_range = 15;
	int m_wait = 1;
	int m_maxWait;
};

//		FLOOR VIII
class AbyssSummoner : public Monster {
public:
	// Summons a gaping maw underneath the player that will attempt to bite them.
	AbyssSummoner(Dungeon *dungeon, int x, int y);

private:
	void move();
	void extraDeathEffects();

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

private:
	void move();
	void reactToDamage(Player &player) override;

	void teleportAway();

	int m_teleportRadius = 8;
	int m_stage = 1;
	int m_wait = 1;
};

class Disabler : public Monster {
public:
	Disabler(Dungeon *dungeon, int x, int y);

private:
	void move();

	void castField();

	int m_range = 15;
	int m_wait = 1;
	int m_maxWait;

	int m_castWait = 0;
	int m_maxCastWait = 12;
};

class IncendiaryInfuser : public Monster {
public:
	IncendiaryInfuser(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();

	int m_range = 15;
	int m_wait = 4;
	int m_maxWait;

	int m_castWait = 0;
	int m_maxCastWait = 10;
};

class LightningStriker : public Monster {
public:
	LightningStriker(Dungeon *dungeon, int x, int y);

private:
	void move();

	void radiusCheck();

	int m_moveRange = 15;
	int m_castRange = 2;

	int m_castTurns = 0;
	int m_maxCastTurns = 3;

	int m_castCooldown = 0;

	int m_wait = 1;
	int m_maxWait;
};

class FlameArchaic : public Monster {
public:
	FlameArchaic(Dungeon *dungeon, int x, int y);

private:
	void move();

	void castFireBlast();
	void castFirePillars();

	int m_moveRange = 15;
	int m_castRange = 5;

	int m_castTurns = 0;
	int m_maxCastTurns = 2;

	int m_castCooldown = 0;

	int m_wait = 1;
	int m_maxWait;
};

class MasterConjurer : public Monster {
public:
	MasterConjurer(Dungeon *dungeon, int x, int y);

private:
	void move();
	void reactToDamage(Player &player) override;

	void summonRandomMonster();

	int m_range = 15;

	int m_castTurns = 0;
	int m_maxCastTurns = 2;

	int m_castCooldown = 0;

	int m_summonRadius = 2;
	int m_wait = 2;
	int m_maxWait;
};

class AdvancedRockSummoner : public Monster {
public:
	AdvancedRockSummoner(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();
	void reactToDamage(Player &player) override;

	void summonRock();

	int m_moveRange = 15;
	int m_castRange = 6;

	int m_castTurns = 0;
	int m_maxCastTurns = 2;

	int m_castCooldown = 0;

	int m_wait = 1;
	int m_maxWait;
};

class AscendedShot : public Monster {
public:
	AscendedShot(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();

	void fireShot();

	int m_moveRange = 15;
	int m_castRange = 7;

	int m_castTurns = 0;
	int m_maxCastTurns = 4;

	int m_castCooldown = 0;

	int m_wait = 1;
	int m_maxWait;
};

class RoyalSwordsman : public Monster {
public:
	RoyalSwordsman(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type);
	void move();

	void singleDash(bool diagonals = false);
	void doubleDash();

	int m_stage = 0;
	int m_cooldown = 0;
	int m_maxCooldown = 2;

	bool m_primed = false;
	int m_range = 18;
	int m_wait = 0;
	int m_maxWait = 1;
};

class LightEntity : public Monster {
public:
	LightEntity(Dungeon *dungeon, int x, int y);

private:
	void move();

	void releaseBeams();

	int m_movesLeft;
	int m_maxMoves = 6;

	int m_wait = 1;
	int m_maxWait;
};

class DarkEntity : public Monster {
public:
	DarkEntity(Dungeon *dungeon, int x, int y);

private:
	bool canBeDamaged(DamageType type) { return false; };
	void move();

	int m_size = 3;
	int m_maxSize = 10;

	int m_wait = 1;
	int m_maxWait;
};

//		FLOOR IX



//		BOSSES
class Smasher : public Monster {
public:
	Smasher(Dungeon *dungeon);

	bool isBoss() const { return true; };

private:
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
	void deathDrops();

	// used for telling what direction smasher is moving in if executing rock slide
	char m_move = '0';

	bool m_moveActive;
	bool m_moveEnding;
	int m_moveType;
	bool m_frenzy;
};



#endif