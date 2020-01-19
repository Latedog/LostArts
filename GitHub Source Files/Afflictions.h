#ifndef AFFLICTION_H
#define AFFLICTION_H

#include <string>

void playBleed(float volume = 1.0f);
void playBirdSound(float volume = 1.0f);
void tintFrozen(cocos2d::Sprite* sprite);
void fadeTransparent(cocos2d::Sprite* sprite);
void fadeIn(cocos2d::Sprite* sprite);

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

	int getMaxWait() const;
	int getWaitTime() const;
	void setWaitTime(int wait);

	virtual void afflict(Afflictions &affliction, Actors &a);
	virtual void afflict(Actors &a) = 0;
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
	Confusion(int turns = 10);

	void afflict(Actors &a);
};

#endif