#include "cocos2d.h"
#include "global.h"
#include "AudioEngine.h"
#include "Afflictions.h"
#include "GameObjects.h" // Needed for enum class ImbuementType
#include "FX.h"
#include "Actors.h"
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

					// Afflictions(turns, wait, name)

//		BURN
Burn::Burn(int turns) : Afflictions(turns, 0, BURN) {

}

void Burn::afflict(Actors &a) {
	// if burn flag hasn't been set yet, set it
	if (!a.isBurned() && getTurnsLeft() != 0)
		a.setBurned(true);

	if (getTurnsLeft() > 0) {
		// play burn sound effect
		playSound("On_Fire1.mp3");

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
HealOverTime::HealOverTime(int turns) : Afflictions(turns, 0, HEAL_OVER_TIME) {

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
Bleed::Bleed(int turns) : Afflictions(turns, 3, BLEED) {

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
Stun::Stun(int turns) : Afflictions(turns, 0, STUN) {

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
Freeze::Freeze(int turns) : Afflictions(turns, 0, FREEZE) {

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
Poison::Poison(int turns, int wait, int str, int dex) : Afflictions(turns, wait, POISON), m_str(str), m_dex(dex) {

}

void Poison::afflict(Actors &a) {
	// if poison flag hasn't been set yet, set it
	if (!a.isPoisoned() && getTurnsLeft() != 0) {
		a.setPoisoned(true);
		a.setStr(std::max(0, a.getStr() - m_str));
		a.setDex(std::max(0, a.getDex() - m_dex));
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
	// else if wait time is not over and there's still poison left
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
Invisibility::Invisibility(int turns) : Afflictions(turns, 0, INVISIBILITY) {

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
Ethereality::Ethereality(int turns) : Afflictions(turns, 0, ETHEREALITY) {

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
		playSound("Uninvisible.mp3");

		a.setEthereal(false);
		setExhaustion(true);

		// fade back in
		fadeIn(a.getSprite());

		// if player was on a wall when etherality wore off, they die instantly
		
	}
}


//		CONFUSION
Confusion::Confusion(int turns, int bonus) : Afflictions(turns, 0, CONFUSION), m_bonus(bonus) {

}

void Confusion::afflict(Actors &a) {
	// if ethereal flag hasn't been set yet, set it
	if (!a.isConfused()) {
		a.setConfused(true);

		// dex boost
		a.setDex(a.getDex() + m_bonus);
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
Buff::Buff(int turns, int str, int dex, int intellect) : Afflictions(turns, 0, BUFF), m_str(str), m_dex(dex), m_int(intellect) {

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
Invulnerability::Invulnerability(int turns) : Afflictions(turns, 0, INVULNERABILITY) {

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
Stuck::Stuck(int turns) : Afflictions(turns, 0, STUCK) {

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


//		POSSESSION
Possessed::Possessed(int turns) : Afflictions(turns, 0, POSSESSED) {

}

void Possessed::afflict(Actors &a) {
	// if possessed flag hasn't been set yet, set it
	if (!a.isPossessed())
		a.setPossessed(true);

	if (getTurnsLeft() > 0) {
		// possessed effect
		//tintPoisoned(a.getSprite());

		// decrease the count by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		a.setPossessed(false);
		setExhaustion(true);
	}
}


//		CRIPPLE
Cripple::Cripple(int turns) : Afflictions(turns, 1, CRIPPLE) {

}

void Cripple::afflict(Actors &a) {
	// if flag hasn't been set yet, set it
	if (!a.isCrippled() && getTurnsLeft() != 0)
		a.setCrippled(true);

	// if wait time is over and there's still turns left
	if (getWaitTime() == 0 && getTurnsLeft() > 0) {
		// Sound effect
		// 

		a.addAffliction(std::make_shared<Stun>(1));

		setTurnsLeft(getTurnsLeft() - 1);

		/// check to move this into the LAST else statement later
		if (getTurnsLeft() == 0) {
			a.setCrippled(false);
		}

		// resets wait time
		setWaitTime(getMaxWait());
	}
	// else if wait time is not over and there are still turns left
	else if (getWaitTime() > 0 && getTurnsLeft() > 0) {
		setWaitTime(getWaitTime() - 1);
	}
	// else there are no turns left
	else {
		setExhaustion(true);
	}
}


//		FRAGILE
Fragile::Fragile(int turns) : Afflictions(turns, 0, FRAGILE) {

}

void Fragile::afflict(Actors &a) {
	// ifflag hasn't been set yet, set it
	if (!a.isFragile())
		a.setFragile(true);

	if (getTurnsLeft() > 0) {
		// 
		tintStunned(a.getSprite());

		// Decrease the count by 1
		setTurnsLeft(getTurnsLeft() - 1);
	}
	// flag to remove affliction
	else {
		a.setFragile(false);
		setExhaustion(true);
	}
}