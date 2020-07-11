#ifndef AFFLICTION_H
#define AFFLICTION_H

#include <string>

class Actors;

class Afflictions {
public:
	Afflictions();
	Afflictions(int turns, int wait, std::string name);

	std::string getName() const;
	int getMaxTurns() const;
	void setMaxTurns(int max);
	int getTurnsLeft() const;
	void setTurnsLeft(int turns);

	// checks if affliction has been exhausted
	bool isExhausted() const;
	void setExhaustion(bool exhausted);

	virtual void afflict(Actors &a) = 0;

protected:
	int getMaxWait() const;
	int getWaitTime() const;
	void setWaitTime(int wait);

private:
	std::string m_name;

	// maximum number of turns that can be stacked on a particular affliction
	int m_maxTurns;

	// number of turns remaining on the affliction
	int m_turnsLeft;

	int m_maxWait;
	int m_waitTime;

	bool m_exhausted;
};

class Burn : public Afflictions {
public:
	Burn(int turns = 4);

	void afflict(Actors &a);
private:
	
};

class HealOverTime : public Afflictions {
public:
	HealOverTime(int turns = 20);

	void afflict(Actors &a);
private:
	
};

class Bleed : public Afflictions {
public:
	Bleed(int turns = 5);

	void afflict(Actors &a);
private:

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
	Poison(int turns, int wait, int str, int dex);

	void afflict(Actors &a);
private:
	int m_str;
	int m_dex;
};

class Invisibility : public Afflictions {
public:
	Invisibility(int turns = 10);

	void afflict(Actors &a);
};

class Ethereality : public Afflictions {
public:
	Ethereality(int turns = 10);

	void afflict(Actors &a);
};

class Confusion : public Afflictions {
public:
	// Reverses player movement, but increases their dodgeability (+dex)
	Confusion(int turns = 10, int bonus = 3);

	void afflict(Actors &a);

private:
	int m_bonus;
};

class Buff : public Afflictions {
public:
	Buff(int turns, int str, int dex, int intellect);

	void afflict(Actors &a);

private:
	int m_str;
	int m_dex;
	int m_int;
};

class Invulnerability : public Afflictions {
public:
	Invulnerability(int turns = 1);

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

#endif