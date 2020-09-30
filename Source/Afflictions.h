#ifndef AFFLICTION_H
#define AFFLICTION_H

class Player;
class Actors;

class Afflictions {
public:
	Afflictions(int turns, int wait, std::string name);
	Afflictions(std::string name);

	std::string getName() const { return m_name; };
	int getTurnsLeft() const { return m_turnsLeft; };

	bool isExhausted() const { return m_exhausted; };

	// Indicates if multiple of these afflictions can exist on an Actor at the same time. e.g. Buff
	virtual bool canBeStacked() const { return false; };

	// Used for SuperRoot
	virtual bool isHarmful() const { return true; };

	virtual void afflict(Actors &a) = 0;
	virtual void adjust(Actors &a, Afflictions &affliction) { m_turnsLeft += affliction.m_turnsLeft; };
	void remove() { m_turnsLeft = 0; };

protected:
	void setExhaustion(bool exhausted) { m_exhausted = exhausted; };

	int getMaxWait() const { return m_maxWait; };
	int getWaitTime() const { return m_waitTime; };
	void setWaitTime(int wait) { m_waitTime = wait; };
	void increaseWaitBy(int amount) { m_waitTime += amount; };
	void decreaseWaitBy(int amount) { m_waitTime -= amount; };
	void resetWaitTime() { m_waitTime = m_maxWait; };

	void decreaseTurnsBy(int amount) { m_turnsLeft -= amount; };

	Player *m_player = nullptr;

	int m_turnsLeft;

private:
	std::string m_name;

	int m_maxWait;
	int m_waitTime;

	bool m_exhausted = false;
};

class HealOverTime : public Afflictions {
public:
	HealOverTime(int turns = 20);

	bool isHarmful() const { return false; };
	void afflict(Actors &a);
};

class Buff : public Afflictions {
public:
	Buff(int turns, int str, int dex, int intellect);

	bool canBeStacked() const override { return true; };
	bool isHarmful() const override { return false; };

	void afflict(Actors &a);

private:
	bool m_buffApplied = false;
	int m_str;
	int m_dex;
	int m_int;
};

class Burn : public Afflictions {
public:
	Burn(Player &player, int turns = 4);

	void afflict(Actors &a);
private:
	
};

class Bleed : public Afflictions {
public:
	Bleed(int turns = 5);

	void afflict(Actors &a);
	void adjust(Actors &a, Afflictions &affliction);

private:
	void reduceHealth(Actors &a);

	int m_healthReduction = 1;
};

class Stun : public Afflictions {
public:
	Stun(int turns = 1);

	void afflict(Actors &a);
private:

};

class Freeze : public Afflictions {
public:
	Freeze(int turns = 10);

	void afflict(Actors &a);
};

class Poison : public Afflictions {
public:
	// Poisoning weakens the actor's str and dex and applies a slow poisoning effect
	Poison(Player &player, int turns, int wait, int str, int dex);

	void afflict(Actors &a);
	void adjust(Actors &a, Afflictions &affliction);

	int getStrengthPenalty() const { return m_str; };
	int getDexPenalty() const { return m_dex; };

private:
	int m_str;
	int m_dex;

	int m_strTaken = -1;
	int m_dexTaken = -1;
};

class Invisibility : public Afflictions {
public:
	Invisibility(int turns = 10);

	bool isHarmful() const { return false; };
	void afflict(Actors &a);
};

class Ethereality : public Afflictions {
public:
	Ethereality(int turns = 10);

	bool isHarmful() const { return false; };
	void afflict(Actors &a);
};

class Confusion : public Afflictions {
public:
	// Reverses movement
	Confusion(int turns = 10);

	void afflict(Actors &a);

private:
	int m_bonus;
};

class Invulnerability : public Afflictions {
public:
	Invulnerability(int turns = 1);

	bool isHarmful() const { return false; };
	void afflict(Actors &a);
};

class Stuck : public Afflictions {
public:
	// Stuck means they're stuck in place, but can still perform an action
	Stuck(int turns);

	void afflict(Actors &a);
};

class Possessed : public Afflictions {
public:
	// If the player is possessed, they are no longer in control of their actions
	Possessed(int turns);

	void afflict(Actors &a);
};

class Cripple : public Afflictions {
public:
	// Adds a stun every other turn
	Cripple(int turns);

	void afflict(Actors &a);
};

class Fragile : public Afflictions {
public:
	// If an enemy is fragile, they die in instantly in one hit.
	Fragile(int turns);

	void afflict(Actors &a);
};

class Incendiary : public Afflictions {
public:
	// Affliction caused by IncendiaryInfuser
	Incendiary(int turns);

	void afflict(Actors &a);
};

class Blindness : public Afflictions {
public:
	// Causes player's vision to become blurry
	Blindness(int turns);

	void afflict(Actors &a);
};

class Disabled : public Afflictions {
public:
	// Disables player's use items and active item
	Disabled(int turns);

	void afflict(Actors &a);
};

class Wet : public Afflictions {
public:
	// Reduces dex, but prevents burning
	Wet(int turns);

	void adjust(Actors &a, Afflictions &affliction);
	void afflict(Actors &a);

private:
	// One can only be so wet
	int m_maxTurns = 12;

	int m_dexPenalty = 2;
	int m_dexTaken = 0;
};

class Slipped : public Afflictions {
public:
	// Used only by things you can slip on
	Slipped(int turns = 0);

	bool isHarmful() const { return false; };
	void afflict(Actors &a);
};

class Ticked : public Afflictions {
public:
	// From tick attacks
	Ticked();

	void afflict(Actors &a);
	void adjust(Actors &a, Afflictions &affliction);

private:
	int m_amount = 1;
};

class TimedAffliction : public Afflictions {
public:
	TimedAffliction(float duration, float interval, std::string name);

	void adjust(Actors &a, Afflictions &affliction);

	std::string getTimerName() const { return m_timerName; };

protected:
	void reduceDurationBy(float time) { m_duration -= time; };

	std::string m_timerName;
	bool m_timerIsSet = false;
	float m_duration;
	float m_interval;
};

class TimedBuff : public TimedAffliction {
public:
	TimedBuff(float duration, int str, int dex, int intellect);

	bool isHarmful() const { return false; };
	void afflict(Actors &a);

private:
	int m_str;
	int m_dex;
	int m_int;
};

class TimedHeal : public TimedAffliction {
public:
	TimedHeal(float duration, float interval, int amount = 1);

	bool isHarmful() const { return false; };
	void afflict(Actors &a);

private:
	int m_amount;
};

class ExperienceGain : public TimedAffliction {
public:
	ExperienceGain(float duration, float interval);

	bool isHarmful() const { return false; };
	void afflict(Actors &a);
};

class AfflictionImmunity : public TimedAffliction {
public:
	AfflictionImmunity(float duration, float interval);

	bool isHarmful() const { return false; };
	void afflict(Actors &a);
};

#endif