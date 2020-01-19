#include "cocos2d.h"
#include "AudioEngine.h"
#include "Afflictions.h"
#include "Actors.h"
#include "utilities.h"

// :::: AFFLICTIONS ::::
Afflictions::Afflictions() {

}
Afflictions::Afflictions(int turns, int wait, std::string name)
	: m_maxTurns(turns), m_turnsLeft(turns), m_maxWait(wait), m_waitTime(wait), m_name(name) {
	m_exhausted = false;
}

std::string Afflictions::getName() const {
	return m_name;
}
int Afflictions::getMaxTurns() const {
	return m_maxTurns;
}
void Afflictions::setMaxTurns(int max) {
	m_maxTurns = max;
}
int Afflictions::getTurnsLeft() const {
	return m_turnsLeft;
}
void Afflictions::setTurnsLeft(int turns) {
	m_turnsLeft = turns;
}

int Afflictions::getMaxWait() const {
	return m_maxWait;
}
int Afflictions::getWaitTime() const {
	return m_waitTime;
}
void Afflictions::setWaitTime(int wait) {
	m_waitTime = wait;
}

bool Afflictions::isExhausted() const {
	return m_exhausted;
}
void Afflictions::setExhaustion(bool exhausted) {
	m_exhausted = exhausted;
}

void Afflictions::afflict(Afflictions &affliction, Actors &a) {
	affliction.afflict(a);
}

					// Afflictions(turns, wait, name)

//		BURN
Burn::Burn(int turns) : Afflictions(turns, 0, "burn") {

}

void Burn::afflict(Actors &a) {
	// if burn flag hasn't been set yet, set it
	if (!a.isBurned() && getTurnsLeft() != 0)
		a.setBurned(true);

	if (getTurnsLeft() > 0) {
		// play burn sound effect
		cocos2d::experimental::AudioEngine::play2d("On_Fire1.mp3", false, 1.0f);

		a.setHP(a.getHP() - 1);
		setTurnsLeft(getTurnsLeft() - 1);

		// burning status set to false so that they may be burned again immediately on the next turn
		if (getTurnsLeft() == 0) {
			a.setBurned(false);
		}
	}
	else {
		setExhaustion(true);
	}
}


//		HEAL
HealOverTime::HealOverTime(int turns) : Afflictions(turns, 0, "heal") {

}

void HealOverTime::afflict(Actors &a) {
	if (getTurnsLeft() > 0) {
		// play heal sound effect
		//

		if (a.getHP() < a.getMaxHP())
			a.setHP(a.getHP() + 1);

		setTurnsLeft(getTurnsLeft() - 1);
	}
	else {
		setExhaustion(true);
	}
}


//		BLEED
Bleed::Bleed(int turns) : Afflictions(turns, 3, "bleed") {

}

void Bleed::afflict(Actors &a) {
	// if bleed flag hasn't been set yet, set it
	if (!a.isBled() && getTurnsLeft() != 0)
		a.setBleed(true);

	// if wait time is over and there's still bleeding left
	if (getWaitTime() == 0 && getTurnsLeft() > 0) {
		// play bleed sound effect
		playBleed();

		a.setHP(a.getHP() - 1);

		setTurnsLeft(getTurnsLeft() - 1);

		/// check to move this into the LAST else statement later
		if (getTurnsLeft() == 0) {
			a.setBleed(false);
		}

		// resets wait time
		setWaitTime(getMaxWait());
	}
	// else if wait time is not over and there's still bleeding left
	else if (getWaitTime() > 0 && getTurnsLeft() > 0) {
		setWaitTime(getWaitTime() - 1);
	}
	// else if there's no bleeding left
	else {
		setExhaustion(true);
	}
}


//		STUN
Stun::Stun(int turns) : Afflictions(turns, 0, "stun") {

}

void Stun::afflict(Actors &a) {
	// if stun flag hasn't been set yet, set it
	if (!a.isStunned())
		a.setStunned(true);

	if (getTurnsLeft() > 0) {
		// decrease the stun count by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		a.setStunned(false);
		setExhaustion(true);
	}
}


//		FREEZE
Freeze::Freeze(int turns) : Afflictions(turns, 0, "freeze") {

}

void Freeze::afflict(Actors &a) {
	// if freeze flag hasn't been set yet, set it
	if (!a.isFrozen())
		a.setFrozen(true);

	if (getTurnsLeft() > 0) {
		// frozen effect
		tintFrozen(a.getSprite());

		// decrease the stun count by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		a.setFrozen(false);
		setExhaustion(true);
	}
}


//		INVISIBILITY
Invisibility::Invisibility(int turns) : Afflictions(turns, 0, "invisibility") {

}

void Invisibility::afflict(Actors &a) {
	// if invisibility flag hasn't been set yet, set it
	if (!a.isInvisible())
		a.setInvisible(true);

	if (getTurnsLeft() > 0) {
		// invisible effect
		fadeTransparent(a.getSprite());

		// decrease the turns by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		// uninvisible sound effect
		cocos2d::experimental::AudioEngine::play2d("Uninvisible.mp3", false, 0.8f);

		// fade back in
		fadeIn(a.getSprite());

		a.setInvisible(false);
		setExhaustion(true);
	}
}


//		ETHEREALITY
Ethereality::Ethereality(int turns) : Afflictions(turns, 0, "ethereality") {

}

void Ethereality::afflict(Actors &a) {
	// if ethereal flag hasn't been set yet, set it
	if (!a.isEthereal())
		a.setEthereal(true);

	if (getTurnsLeft() > 0) {
		// ethereal effect
		fadeTransparent(a.getSprite());

		// decrease the turns by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		// uninvisible sound effect
		cocos2d::experimental::AudioEngine::play2d("Uninvisible.mp3", false, 0.8f);

		a.setEthereal(false);
		setExhaustion(true);

		// fade back in
		fadeIn(a.getSprite());

		// if player was on a wall when etherality wore off, they die instantly
		
	}
}


//		CONFUSION
Confusion::Confusion(int turns) : Afflictions(turns, 0, "confusion") {

}

void Confusion::afflict(Actors &a) {
	// if ethereal flag hasn't been set yet, set it
	if (!a.isConfused()) {
		a.setConfused(true);

		// dex boost
		a.setDex(a.getDex() + 3);
	}

	if (getTurnsLeft() > 0) {
		// play bird (confusion) sound
		playBirdSound(0.5f);

		// decrease the stun count by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		a.setConfused(false);
		setExhaustion(true);

		// remove dex boost
		a.setDex(a.getDex() - 3);
	}
}




//		SOUND EFFECTS
void playBleed(float volume) {
	std::string sound;
	switch (randInt(4)) {
	case 1: sound = "Water_Drip1.mp3"; break;
	case 2: sound = "Water_Drip2.mp3"; break;
	case 3: sound = "Water_Drip3.mp3"; break;
	case 4: sound = "Water_Drip4.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, volume);
}
void playBirdSound(float volume) {
	std::string sound;
	switch (1 + randInt(4)) {
	case 1: sound = "Bird1.mp3"; break;
	case 2: sound = "Bird2.mp3"; break;
	case 3: sound = "Bird3.mp3"; break;
	case 4: sound = "Crow1.mp3"; break;
	case 5: sound = "Crow2.mp3"; break;
	case 6: sound = "Crow3.mp3"; break;
	case 7: sound = "Crow4.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, volume);
}

void tintFrozen(cocos2d::Sprite* sprite) {
	auto tintBlue = cocos2d::TintTo::create(0, cocos2d::Color3B(68, 85, 140));
	sprite->runAction(tintBlue);
	//auto tintNormal = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 255, 255));
	//sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintBlue, tintNormal));
}
void fadeTransparent(cocos2d::Sprite* sprite) {
	auto fadeTransparent = cocos2d::FadeTo::create(.3, 40);
	//auto fadeTransparentReverse = cocos2d::FadeTo::reverse(fadeTransparent);
	sprite->runAction(fadeTransparent);
	//sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintBlue, tintNormal));
}
void fadeIn(cocos2d::Sprite* sprite) {
	auto fadeIn = cocos2d::FadeTo::create(.3, 255);
	sprite->runAction(fadeIn);
}