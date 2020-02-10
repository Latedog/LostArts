#include "cocos2d.h"
#include "AudioEngine.h"
#include "Afflictions.h"
#include "Actors.h"
#include "utilities.h"
#include <memory>


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
		// tint gray to indicate stunned
		//tintStunned(a.getSprite());

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


//		POISON
Poison::Poison(int turns, int wait, int str, int dex) : Afflictions(turns, wait, "poison"), m_str(str), m_dex(dex) {

}

void Poison::afflict(Actors &a) {
	// if poison flag hasn't been set yet, set it
	if (!a.isPoisoned() && getTurnsLeft() != 0) {
		a.setPoisoned(true);
		a.setStr(a.getStr() - m_str);
		a.setDex(a.getDex() - m_dex);
	}

	if (getWaitTime() == 0 && getTurnsLeft() > 0) {
		// poison effect
		playPoison(0.23f);
		tintPoisoned(a.getSprite());

		a.setHP(a.getHP() - 1);

		// decrease the poison count by 1
		setTurnsLeft(getTurnsLeft() - 1);

		/*/// check to move this into the LAST else statement later
		if (getTurnsLeft() == 0) {
			a.setPoisoned(false);
		}*/

		// resets wait time
		setWaitTime(getMaxWait());
	}
	// else if wait time is not over and there's still bleeding left
	else if (getWaitTime() > 0 && getTurnsLeft() > 0) {
		setWaitTime(getWaitTime() - 1);
	}
	// flag to remove affliction
	else {
		a.setPoisoned(false);
		a.setStr(a.getStr() + m_str);
		a.setDex(a.getDex() + m_dex);
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

		// if there are at least 3 turns remaining play bird sound
		if (getTurnsLeft() > 3) {
			playBirdSound(0.15f);
		}
		// else play crow sound to indicate that the effect is about to wear off
		else {
			playCrowSound(0.5f);
		}

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


//		BUFF STATS
Buff::Buff(int turns, int str, int dex, int intellect) : Afflictions(turns, 0, "buff"), m_str(str), m_dex(dex), m_int(intellect) {

}

void Buff::afflict(Actors &a) {
	// if buff flag hasn't been set yet, set it
	if (!a.isBuffed()) {
		a.setBuffed(true);

		// buff the actor's stats accordingly
		a.setStr(a.getStr() + m_str);
		a.setDex(a.getDex() + m_dex);
		a.setInt(a.getInt() + m_int);
	}

	if (getTurnsLeft() > 0) {
		// decrease the stun count by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		a.setBuffed(false);
		setExhaustion(true);

		// remove the buffs
		a.setStr(a.getStr() - m_str);
		a.setDex(a.getDex() - m_dex);
		a.setInt(a.getInt() - m_int);
	}
}


//		INVULNERABILITY
Invulnerability::Invulnerability(int turns) : Afflictions(turns, 0, "invulnerable") {

}

void Invulnerability::afflict(Actors &a) {
	// if invulnerability flag hasn't been set yet, set it
	if (!a.isInvulnerable())
		a.setInvulnerable(true);

	if (getTurnsLeft() > 0) {
		// poison effect
		//tintPoisoned(a.getSprite());

		// decrease the count by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		a.setInvulnerable(false);
		setExhaustion(true);
	}
}


//		STUCK
Stuck::Stuck(int turns) : Afflictions(turns, 0, "stuck") {

}

void Stuck::afflict(Actors &a) {
	// if stuck flag hasn't been set yet, set it
	if (!a.isStuck())
		a.setStuck(true);

	if (getTurnsLeft() > 0) {
		// stuck effect
		// 

		// decrease the count by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		a.setStuck(false);
		setExhaustion(true);
	}
}



//		SOUND EFFECTS
void playBleed(float volume) {
	std::string sound;
	switch (1 + randInt(4)) {
	case 1: sound = "Water_Drip1.mp3"; break;
	case 2: sound = "Water_Drip2.mp3"; break;
	case 3: sound = "Water_Drip3.mp3"; break;
	case 4: sound = "Water_Drip4.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, volume);
}
void playPoison(float volume) {
	std::string sound;
	switch (1 + randInt(2)) {
	case 1: sound = "Poison_Damage1.mp3"; break;
	case 2: sound = "Poison_Damage2.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, volume);
}
void playBirdSound(float volume) {
	std::string sound;
	switch (1 + randInt(3)) {
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
void playCrowSound(float volume) {
	std::string sound;
	switch (1 + randInt(4)) {
	case 1: sound = "Crow1.mp3"; break;
	case 2: sound = "Crow2.mp3"; break;
	case 3: sound = "Crow3.mp3"; break;
	case 4: sound = "Crow4.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, volume);
}

void tintFrozen(cocos2d::Sprite* sprite) {
	auto tintBlue = cocos2d::TintTo::create(0, cocos2d::Color3B(68, 85, 140));
	sprite->runAction(tintBlue);
	//auto tintNormal = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 255, 255));
	//sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintBlue, tintNormal));
}
void tintStunned(cocos2d::Sprite* sprite) {
	auto tintGray = cocos2d::TintTo::create(0, cocos2d::Color3B(100, 100, 100));
	sprite->runAction(tintGray);
	//sprite->runAction(cocos2d::Blink::create(0.2, 3));
}
void tintPoisoned(cocos2d::Sprite* sprite) {
	auto tintGreen = cocos2d::TintTo::create(0, cocos2d::Color3B(173, 255, 47));
	sprite->runAction(tintGreen);
}
void tintItemCast(cocos2d::Sprite* sprite) {
	auto tintGold = cocos2d::TintTo::create(0, cocos2d::Color3B(255, 215, 0));
	auto tint = cocos2d::TintTo::create(0.3, cocos2d::Color3B(255, 255, 255));
	auto tinting = cocos2d::Sequence::createWithTwoActions(tintGold, tint);
	sprite->runAction(tinting);
}
void tintStaffCast(cocos2d::Sprite* sprite) {
	auto tintGold = cocos2d::TintTo::create(0.3, cocos2d::Color3B(148, 10, 200));
	auto tint = cocos2d::TintTo::create(0.3, cocos2d::Color3B(255, 255, 255));
	auto tinting = cocos2d::Sequence::createWithTwoActions(tintGold, tint);
	sprite->runAction(tinting);
}
void untint(cocos2d::Sprite* sprite) {
	auto tint = cocos2d::TintTo::create(0, cocos2d::Color3B(255, 255, 255));
	sprite->runAction(tint);
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