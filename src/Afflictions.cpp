#include "cocos2d.h"
#include "GUI.h"
#include "global.h"
#include "AudioEngine.h"
#include "FX.h"
#include "Afflictions.h"
#include "GameUtils.h"
#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include <string>
#include <memory>


// :::: AFFLICTIONS ::::
Afflictions::Afflictions(int turns, int wait, std::string name)
	: m_turnsLeft(turns), m_maxWait(wait), m_waitTime(wait), m_name(name) {
	
}
Afflictions::Afflictions(std::string name) : m_name(name) {

}


//		HEAL
HealOverTime::HealOverTime(int turns) : Afflictions(turns, 0, HEAL_OVER_TIME) {

}

void HealOverTime::afflict(Actors &a) {
	if (m_turnsLeft > 0) {
		// play heal sound effect
		//

		if (a.getHP() < a.getMaxHP())
			a.setHP(a.getHP() + 1);

		decreaseTurnsBy(1);
	}
	else {
		setExhaustion(true);
	}
}


//		BUFF STATS
Buff::Buff(int turns, int str, int dex, int intellect) : Afflictions(turns, 0, BUFF), m_str(str), m_dex(dex), m_int(intellect) {

}

void Buff::afflict(Actors &a) {
	if (!m_buffApplied) {
		m_buffApplied = true;

		a.increaseStatBy(StatType::STRENGTH, m_str);
		a.increaseStatBy(StatType::DEXTERITY, m_dex);
		a.increaseStatBy(StatType::INTELLECT, m_int);
	}

	if (m_turnsLeft > 0) {
		decreaseTurnsBy(1);
	}
	else {
		//a.setBuffed(false);
		setExhaustion(true);

		a.decreaseStatBy(StatType::STRENGTH, m_str);
		a.decreaseStatBy(StatType::DEXTERITY, m_dex);
		a.decreaseStatBy(StatType::INTELLECT, m_int);
	}
}


//		BURN
Burn::Burn(Player &player, int turns) : Afflictions(turns, 0, BURN) {
	m_player = &player;
}

void Burn::afflict(Actors &a) {
	// if burn flag hasn't been set yet, set it
	if (!a.isBurned() && m_turnsLeft != 0)
		a.setBurned(true);

	if (m_turnsLeft > 0) {
		playSound("On_Fire1.mp3", m_player->getPos(), a.getPos());

		a.setHP(a.getHP() - 1);
		decreaseTurnsBy(1);

		// burning status set to false so that they may be burned again immediately on the next turn
		if (m_turnsLeft == 0)
			a.setBurned(false);
		
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
	if (!a.isBled() && m_turnsLeft != 0)
		a.setBleed(true);

	// if wait time is over and there's still bleeding left
	if (getWaitTime() == 0 && m_turnsLeft > 0) {
		// play bleed sound effect
		playBleed();

		reduceHealth(a);

		decreaseTurnsBy(1);

		/// check to move this into the LAST else statement later
		if (m_turnsLeft == 0)
			a.setBleed(false);		

		// resets wait time
		setWaitTime(getMaxWait());
	}
	// else if wait time is not over and there's still bleeding left
	else if (getWaitTime() > 0 && m_turnsLeft > 0) {
		setWaitTime(getWaitTime() - 1);
	}
	// else if there's no bleeding left
	else {
		setExhaustion(true);
	}
}
void Bleed::adjust(Actors &a, Afflictions &affliction) {
	setWaitTime(std::max(0, getWaitTime() - 1));
	m_turnsLeft += affliction.getTurnsLeft();

	m_healthReduction++;
}
void Bleed::reduceHealth(Actors &a) {
	a.setHP(a.getHP() - m_healthReduction);
}


//		STUN
Stun::Stun(int turns) : Afflictions(turns, 0, STUN) {

}

void Stun::afflict(Actors &a) {
	// if stun flag hasn't been set yet, set it
	if (!a.isStunned())
		a.setStunned(true);

	if (m_turnsLeft > 0) {
		// tint gray to indicate stunned
		//tintStunned(a.getSprite());

		decreaseTurnsBy(1);
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

	if (m_turnsLeft > 0) {
		// frozen effect
		tintFrozen(a.getSprite());

		// decrease the stun count by 1
		decreaseTurnsBy(1);
	}
	// flag to remove affliction
	else {
		untint(a.getSprite());

		a.setFrozen(false);
		setExhaustion(true);
	}
}


//		POISON
Poison::Poison(Player &player, int turns, int wait, int str, int dex) : Afflictions(turns, wait, POISON), m_str(str), m_dex(dex) {
	m_player = &player;
}

void Poison::afflict(Actors &a) {
	if (!a.isPoisoned() && m_turnsLeft != 0) {
		a.setPoisoned(true);

		m_strTaken = a.getStr() - m_str >= 0 ? m_str : a.getStr();
		m_dexTaken = a.getDex() - m_dex >= 0 ? m_dex : a.getDex();
		a.setStr(std::max(0, a.getStr() - m_str));
		a.setDex(std::max(0, a.getDex() - m_dex));
	}

	if (getWaitTime() == 0 && m_turnsLeft > 0) {
		// poison effect
		playPoison(*m_player, a.getPosX(), a.getPosY());
		tintPoisoned(a.getSprite());

		a.decreaseStatBy(StatType::HP, 1);

		decreaseTurnsBy(1);

		resetWaitTime();
	}
	else if (getWaitTime() > 0 && m_turnsLeft > 0) {
		setWaitTime(getWaitTime() - 1);
	}
	else {
		a.setPoisoned(false);

		a.increaseStatBy(StatType::STRENGTH, m_strTaken);
		a.increaseStatBy(StatType::DEXTERITY, m_dexTaken);

		setExhaustion(true);
	}
}
void Poison::adjust(Actors &a, Afflictions &affliction) {
	m_turnsLeft += affliction.getTurnsLeft();

	if (m_strTaken == -1 || m_dexTaken == -1)
		return;

	Poison &poison = dynamic_cast<Poison&>(affliction);

	a.setStr(std::max(0, a.getStr() + m_strTaken));
	m_str = std::max(m_str, poison.getStrengthPenalty());
	m_strTaken = a.getStr() - m_str >= 0 ? m_str : a.getStr();
	a.setStr(std::max(0, a.getStr() - m_strTaken));

	a.setDex(std::max(0, a.getDex() + m_dexTaken));
	m_dex = std::max(m_dex, poison.getDexPenalty());
	m_dexTaken = a.getDex() - m_dex >= 0 ? m_dex : a.getDex();
	a.setDex(std::max(0, a.getDex() - m_dexTaken));
}


//		INVISIBILITY
Invisibility::Invisibility(int turns) : Afflictions(turns, 0, INVISIBILITY) {

}

void Invisibility::afflict(Actors &a) {
	// if invisibility flag hasn't been set yet, set it
	if (!a.isInvisible())
		a.setInvisible(true);

	if (m_turnsLeft > 0) {
		// invisible effect
		fadeTransparent(a.getSprite());

		// decrease the turns by 1
		decreaseTurnsBy(1);
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
Ethereality::Ethereality(Dungeon *dungeon, int turns) : Afflictions(turns, 0, ETHEREALITY), m_dungeon(dungeon) {
	
}

void Ethereality::afflict(Actors &a) {
	// if ethereal flag hasn't been set yet, set it
	if (!a.isEthereal())
		a.setEthereal(true);

	if (m_turnsLeft > 0) {
		// ethereal effect
		fadeTransparent(a.getSprite());

		decreaseTurnsBy(1);
	}
	// flag to remove affliction
	else {
		// Becoming uninvisible sound effect
		playSound("Uninvisible.mp3");

		a.setEthereal(false);
		setExhaustion(true);

		// fade back in
		fadeIn(a.getSprite());

		// If player was on a wall when etherality wore off, they die instantly
		if (m_dungeon->wall(a.getPosX(), a.getPosY())) {
			bool shouldDie = true;

			if (m_dungeon->trap(a.getPosX(), a.getPosY())) {
				bool trapActsAsWall = false;
				std::vector<int> indexes = m_dungeon->findTraps(a.getPosX(), a.getPosY());
				for (size_t i = 0; i < indexes.size(); ++i) {
					if (m_dungeon->trapAt(indexes.at(i))->actsAsWall()) {
						trapActsAsWall = true;
						break;
					}
				}

				if (!trapActsAsWall)
					shouldDie = false;
			}

			if (shouldDie) {
				playSound("Female_Falling_Scream_License.mp3");
				deathFade(a.getSprite());
				a.setHP(0);
			}
		}		
	}
}


//		CONFUSION
Confusion::Confusion(int turns) : Afflictions(turns, 0, CONFUSION) {

}

void Confusion::afflict(Actors &a) {
	// if ethereal flag hasn't been set yet, set it
	if (!a.isConfused()) {
		a.setConfused(true);
	}

	if (m_turnsLeft > 0) {

		if (a.isPlayer()) {
			// if there are at least 3 turns remaining play bird sound
			if (m_turnsLeft > 3)
				playBirdSound(0.15f);			
			// else play crow sound to indicate that the effect is about to wear off
			else
				playCrowSound(0.5f);			
		}

		// decrease the stun count by 1
		decreaseTurnsBy(1);
	}
	// flag to remove affliction
	else {
		a.setConfused(false);
		setExhaustion(true);
	}
}


//		INVULNERABILITY
Invulnerability::Invulnerability(int turns) : Afflictions(turns, 0, INVULNERABILITY) {

}

void Invulnerability::afflict(Actors &a) {
	// if invulnerability flag hasn't been set yet, set it
	if (!a.isInvulnerable())
		a.setInvulnerable(true);

	if (m_turnsLeft > 0) {
		// poison effect
		//tintPoisoned(a.getSprite());

		// decrease the count by 1
		decreaseTurnsBy(1);
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

	if (m_turnsLeft > 0) {
		// stuck effect
		// 

		// decrease the count by 1
		decreaseTurnsBy(1);
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

	if (m_turnsLeft > 0) {
		// possessed effect
		//tintPoisoned(a.getSprite());

		// decrease the count by 1
		decreaseTurnsBy(1);
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
	if (!a.isCrippled() && m_turnsLeft != 0)
		a.setCrippled(true);

	// if wait time is over and there's still turns left
	if (getWaitTime() == 0 && m_turnsLeft > 0) {
		// Sound effect
		// 

		a.addAffliction(std::make_shared<Stun>(1));

		decreaseTurnsBy(1);

		/// check to move this into the LAST else statement later
		if (m_turnsLeft == 0) {
			a.setCrippled(false);
		}

		// resets wait time
		setWaitTime(getMaxWait());
	}
	// else if wait time is not over and there are still turns left
	else if (getWaitTime() > 0 && m_turnsLeft > 0) {
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
	// if flag hasn't been set yet, set it
	if (!a.isFragile())
		a.setFragile(true);

	if (m_turnsLeft > 0) {
		// 
		tintStunned(a.getSprite());

		decreaseTurnsBy(1);
	}
	// flag to remove affliction
	else {
		a.setFragile(false);
		setExhaustion(true);
	}
}


Incendiary::Incendiary(int turns) : Afflictions(turns, 0, INCENDIARY) {

}

void Incendiary::afflict(Actors &a) {
	//if (!a.isFragile())
	//	a.setFragile(true);

	if (m_turnsLeft > 0) {
		// 
		//tintStunned(a.getSprite());

		decreaseTurnsBy(1);
	}
	else {
		setExhaustion(true);
	}
}


//		BLINDNESS
Blindness::Blindness(int turns) : Afflictions(turns, 0, BLINDNESS) {

}

void Blindness::afflict(Actors &a){
	if (!a.isBlinded())
		a.setBlinded(true);

	if (m_turnsLeft > 0) {
		// 
		//tintStunned(a.getSprite());

		decreaseTurnsBy(1);
	}
	else {
		a.setBlinded(false);
		setExhaustion(true);
	}
}


Disabled::Disabled(int turns) : Afflictions(turns, 0, DISABLED) {

}

void Disabled::afflict(Actors &a) {
	if (!a.isDisabled())
		a.setDisabled(true);

	if (m_turnsLeft > 0) {
		// 
		//tintStunned(a.getSprite());

		decreaseTurnsBy(1);
	}
	else {
		untint(a.getSprite());
		a.setDisabled(false);
		setExhaustion(true);
	}
}


Wet::Wet(int turns) : Afflictions(turns, 0, WET) {

}

void Wet::adjust(Actors &a, Afflictions &affliction) {
	m_turnsLeft += affliction.getTurnsLeft();

	if (m_turnsLeft > m_maxTurns)
		m_turnsLeft = m_maxTurns;
}
void Wet::afflict(Actors &a) {
	if (!a.isWet()) {
		a.setWet(true);

		m_dexTaken = a.getDex() - m_dexPenalty >= 0 ? m_dexPenalty : a.getDex();
		a.decreaseStatBy(StatType::DEXTERITY, m_dexTaken);
		//a.setDex(std::max(0, a.getDex() - m_dexPenalty));
	}

	if (m_turnsLeft > 0) {
		//playPoison(*m_player, a.getPosX(), a.getPosY());
		//tintPoisoned(a.getSprite());

		decreaseTurnsBy(1);
	}
	else {
		a.setWet(false);

		a.increaseStatBy(StatType::DEXTERITY, m_dexTaken);

		setExhaustion(true);
	}
}


//		SLIPPED
Slipped::Slipped(int turns) : Afflictions(turns, 0, SLIPPED) {

}

void Slipped::afflict(Actors &a) {
	if (m_turnsLeft > 0) {
		decreaseTurnsBy(1);
	}
	else {
		a.getSprite()->setRotation(0);
		setExhaustion(true);
	}
}


//		TICKED
Ticked::Ticked() : Afflictions(-1, 10, TICKED) {

}

void Ticked::afflict(Actors &a) {
	if (m_turnsLeft == 0) {
		setExhaustion(true);
		return;
	}

	if (getWaitTime() > 0) {
		decreaseWaitBy(1);
	}
	else {
		playBleed();
		a.decreaseStatBy(StatType::HP, m_amount);

		resetWaitTime();
	}
}
void Ticked::adjust(Actors &a, Afflictions &affliction) {
	// If multiple ticks bite, then the player loses more hp per "tick"
	m_amount++;
}


TimedAffliction::TimedAffliction(float duration, float interval, std::string name)
	: Afflictions(name), m_duration(duration), m_interval(interval) {

}

void TimedAffliction::adjust(Actors &a, Afflictions &affliction) {
	TimedAffliction &timed = dynamic_cast<TimedAffliction&>(affliction);
	m_duration += timed.m_duration;
}

TimedBuff::TimedBuff(float duration, int str, int dex, int intellect)
	: TimedAffliction(duration, 1.0f, TIMED_BUFF), m_str(str), m_dex(dex), m_int(intellect) {
	m_timerName = "Timed Buff Timer";
	GameTimers::addGameTimer(m_timerName, this);
}
void TimedBuff::afflict(Actors &a) {
	if (!m_timerIsSet) {
		a.increaseStatBy(StatType::STRENGTH, m_str);
		a.increaseStatBy(StatType::DEXTERITY, m_dex);
		a.increaseStatBy(StatType::INTELLECT, m_int);

		cocos2d::Director::getInstance()->getScheduler()->schedule([this, &a](float) {
			playSound("Bomb_Pickup2.mp3");

			reduceDurationBy(m_interval);

			if (m_duration <= 0) {
				cocos2d::Director::getInstance()->getScheduler()->unschedule(m_timerName, this);
				GameTimers::removeGameTimer(m_timerName);

				a.decreaseStatBy(StatType::STRENGTH, m_str);
				a.decreaseStatBy(StatType::DEXTERITY, m_dex);
				a.decreaseStatBy(StatType::INTELLECT, m_int);
				setExhaustion(true);
			}

		}, this, m_interval, false, m_timerName);

		m_timerIsSet = true;
	}
}

TimedHeal::TimedHeal(float duration, float interval, int amount) : TimedAffliction(duration, interval, TIMED_HEAL), m_amount(amount) {
	m_timerName = "Timed Heal Timer";
	GameTimers::addGameTimer(m_timerName, this);
}
void TimedHeal::afflict(Actors &a) {
	if (!m_timerIsSet) {
		cocos2d::Director::getInstance()->getScheduler()->schedule([this, &a](float) {
			playSound("Bomb_Pickup2.mp3");

			reduceDurationBy(m_interval);

			a.increaseStatBy(StatType::HP, m_amount);

			if (m_duration <= 0) {
				cocos2d::Director::getInstance()->getScheduler()->unschedule(m_timerName, this);
				GameTimers::removeGameTimer(m_timerName);
				setExhaustion(true);
			}

		}, this, m_interval, false, m_timerName);

		m_timerIsSet = true;
	}
}

ExperienceGain::ExperienceGain(float duration, float interval) : TimedAffliction(duration, interval, XP_GAIN) {
	m_timerName = "RPG In A Bottle Timer";
	GameTimers::addGameTimer(m_timerName, this);
}
void ExperienceGain::afflict(Actors &a) {
	if (!m_timerIsSet) {
		cocos2d::Director::getInstance()->getScheduler()->schedule([this](float) {
			playSound("Bomb_Pickup2.mp3");

			reduceDurationBy(m_interval);

			if (m_duration <= 0) {
				cocos2d::Director::getInstance()->getScheduler()->unschedule(m_timerName, this);
				GameTimers::removeGameTimer(m_timerName);
				setExhaustion(true);
			}

		}, this, m_interval, false, m_timerName);

		m_timerIsSet = true;
	}
}

AfflictionImmunity::AfflictionImmunity(float duration, float interval) : TimedAffliction(duration, interval, AFFLICTION_IMMUNITY) {
	m_timerName = "Affliction Immunity Timer";
	GameTimers::addGameTimer(m_timerName, this);
}
void AfflictionImmunity::afflict(Actors &a) {
	if (!m_timerIsSet) {
		cocos2d::Director::getInstance()->getScheduler()->schedule([this](float) {
			playSound("Bomb_Pickup2.mp3");

			reduceDurationBy(m_interval);

			if (m_duration <= 0) {
				cocos2d::Director::getInstance()->getScheduler()->unschedule(m_timerName, this);
				GameTimers::removeGameTimer(m_timerName);
				setExhaustion(true);
			}

		}, this, m_interval, false, m_timerName);

		m_timerIsSet = true;
	}
}
