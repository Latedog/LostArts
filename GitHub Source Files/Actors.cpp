#include "cocos2d.h"
#include "AudioEngine.h"
#include "global.h"
#include "GUI.h"
#include "FX.h"
#include "GameUtils.h"
#include "Afflictions.h"
#include "Actors.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <random>
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <functional>


//		ACTOR CLASS
Actors::Actors() : m_x(0), m_y(0), m_hp(0), m_armor(0), m_str(0), m_dex(0), m_wep(std::make_shared<ShortSword>()) {

}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, std::string name)
	: m_x(x), m_y(y), m_maxhp(hp), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_name(name) {

}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep)
	: m_x(x), m_y(y), m_maxhp(hp), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_wep(wep) {

}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name)
	: m_x(x), m_y(y), m_maxhp(hp), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_wep(wep), m_name(name) {

}
Actors::~Actors() {
	for (auto &it : m_afflictions)
		it.reset();

	if (m_wep)
		m_wep.reset();
}

void Actors::increaseStatBy(StatType stat, int amount) {
	switch (stat) {
	case StatType::MAXHP: m_maxhp += amount; break;
	case StatType::HP: setHP(m_hp + amount); break;
	case StatType::STRENGTH: m_str += amount; break;
	case StatType::DEXTERITY: m_dex += amount; break;
	case StatType::INTELLECT: m_int += amount; break;
	case StatType::ARMOR: m_armor += amount; break;
	case StatType::LUCK: m_luck += amount; break;
	case StatType::CHARISMA: m_cha += amount; break;
	}
}
void Actors::decreaseStatBy(StatType stat, int amount) {
	switch (stat) {
	case StatType::MAXHP:
		m_maxhp -= amount;
		if (m_hp > m_maxhp)
			m_hp = m_maxhp;

		break;
	case StatType::HP: setHP(m_hp - amount); break;
	case StatType::STRENGTH: m_str -= amount; break;
	case StatType::DEXTERITY: m_dex -= amount; break;
	case StatType::INTELLECT: m_int -= amount; break;
	case StatType::ARMOR: m_armor -= amount; break;
	case StatType::LUCK: m_luck -= amount; break;
	case StatType::CHARISMA: m_cha -= amount; break;
	}
}

void Actors::checkAfflictions() {
	int pos;

	for (int i = 0; i < (int)m_afflictions.size(); i++) {
		m_afflictions[i]->afflict(*this);

		if (m_afflictions[i]->isExhausted()) {
			pos = i;
			m_afflictions.erase(m_afflictions.begin() + pos);

			i--; // prevents skipping of any afflictions
		}
	}
}
int Actors::findAffliction(std::string name) const {
	for (int i = 0; i < (int)m_afflictions.size(); i++) {
		if (m_afflictions.at(i)->getName() == name)
			return i;		
	}
	return -1;
}
void Actors::addAffliction(std::shared_ptr<Afflictions> affliction) {

	if (affliction->getName() == BURN && isWet())
		return;

	if (isPlayer()) {
		Player &p = dynamic_cast<Player&>(*this);
		if (p.hasBonusRoll()) {
			// If player has bonus roll, and it's a certain negative type, then try to save the roll
			if (affliction->getName() == BURN || affliction->getName() == POISON || affliction->getName() == FREEZE || affliction->getName() == POSSESSED) {
				if (randReal(1, 100) + p.getLuck() > 60)
					return;
			}
		}

		if (p.hasAfflictionAversion() && affliction->isHarmful())
			return;		
	}

	// The Forgotten Spirit cannot incur any afflictions
	if (isSpirit())
		return;

	int index = findAffliction(affliction->getName());

	// if affliction wasn't found, then add new affliction
	if (index == -1) {
		m_afflictions.push_back(affliction);

		return;
	}

	m_afflictions.at(index)->adjust(*this, *affliction);
}
void Actors::removeAffliction(std::string name) {
	int pos = findAffliction(name);

	if (pos == -1)
		return;

	m_afflictions.at(pos)->remove(); // Sets turns to zero and lets the affliction do the rest
}


// =================================================
//				:::: THE PLAYERS ::::
// =================================================
Player::Player(int hp, std::shared_ptr<Weapon> weapon) : Actors(0, 0, hp, 2, 2, 2, weapon, "Player") {	
	setPlayerFlag(true);
	setMonsterFlag(false);
}
Player::~Player() {
	if (m_storedWeapon)
		m_storedWeapon.reset();

	if (m_relic)
		m_relic.reset();

	for (auto &it : m_items)
		it.reset();

	for (auto &it : m_passives)
		it.reset();
}

void Player::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].hero = false;
	(*m_dungeon)[y*cols + x].hero = true;
	setPosX(x); setPosY(y);

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	getSprite()->setLocalZOrder(y + Z_ACTOR);
	playFootstepSound();
}
void Player::move(char move) {
	int cols = m_dungeon->getCols();

	int x = getPosX();
	int y = getPosY();

	bool boundary, wall, item, trap, enemy, npc, spirit;

	// Reset certain flags
	setBlock(false);
	m_attackedEnemy = false;

	// set player's facing direction
	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd':
		setFacingDirection(move); break;
	default: break;
	}

	// set player's action key (for items that have casting time)
	if (!isDisabled())
		setAction(move);
	else
		setAction('-');

	// red tint if player has bloodlust
	if (hasBloodlust())
		bloodlustTint(*this);

	// check for any afflictions
	checkAfflictions();

	// if player is confused, switch their movement direction
	if (isConfused()) {
		switch (move) {
		case 'l': move = 'r'; break;
		case 'r': move = 'l'; break;
		case 'u': move = 'd'; break;
		case 'd': move = 'u'; break;
		}
	}

	// if player is possessed, select a random movement direction
	if (isPossessed()) {
		switch (randInt(4)) {
		case 0: move = 'r'; break;
		case 1: move = 'l'; break;
		case 2: move = 'd'; break;
		case 3: move = 'u'; break;
		}
		setFacingDirection(move);
	}

	// if player was ethereal and is on top of a wall, they die instantly
	if (!isEthereal() && m_dungeon->wall(x, y)) {
		playSound("Female_Falling_Scream_License.mp3");
		deathFade(getSprite());
		setHP(0);
		return;
	}

	// if player is not immobilized, move them
	if (!(isStunned() || isFrozen())) {

		bool moveUsed = false;

		// Check for special weapon attack patterns
		if (getWeapon()->hasAttackPattern())
			getWeapon()->usePattern(*m_dungeon, moveUsed);
		
		if (!moveUsed) {

			// if it was a movement action
			if (isMovementAction(move)) {
				int n = 0, m = 0;
				setDirectionalOffsets(move, n, m);
				
				boundary = m_dungeon->boundary(x + n, y + m);
				wall = (isEthereal() ? false : m_dungeon->wall(x + n, y + m));
				item = m_dungeon->item(x + n, y + m);
				trap = m_dungeon->trap(x + n, y + m);
				enemy = m_dungeon->enemy(x + n, y + m);
				npc = m_dungeon->npc(x + n, y + m);
				spirit = (*m_dungeon)[(y + m)*cols + (x + n)].spirit;

				if (npc) {
					m_dungeon->interactWithNPC(x + n, y + m);
				}
				else if (spirit && !wall) {
					if (!isStuck()) {
						playSound("Devils_Gift.mp3");

						cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
						m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);

						moveTo(x + n, y + m);

						getSprite()->setVisible(false);
						setSuperDead(true);
						return;
					}
				}
				else if (!(wall || enemy || boundary)) {
					if (!isStuck()) {
						moveTo(x + n, y + m);

						// Pick up any gold/money that was on the ground
						if (m_dungeon->gold(x + n, y + m) > 0) {
							m_dungeon->goldPickup(getPosX(), getPosY());

							if (hasGoldInvulnerability()) {
								setInvulnerable(true);
								addAffliction(std::make_shared<Invulnerability>(1));
							}
						}

						// For Spelunker
						if (m_dungeon->itemObject(x + n, y + m) && m_dungeon->itemObject(x + n, y + m)->getName() == ROCKS && getName() == SPELUNKER) {
							if (!isStuck()) {
								playSound((*m_dungeon)[(y + m)*cols + (x + n)].object->getSoundName());

								equipActiveItem((*m_dungeon)[(y + m)*cols + (x + n)].object);
								m_dungeon->removeItem(x + n, y + m);

								moveTo(x + n, y + m);
							}
						}

						if (trap) {
							if (!m_dungeon->specialTrapCheck(x + n, y + m))
								m_dungeon->trapEncounter(x + n, y + m);
						}

						if (m_dungeon->isShop()) {

							if ((*m_dungeon)[(y + m)*cols + (x + n)].shop_action == PURCHASE)
								m_dungeon->constructShopHUD(x + n, y + m);

							else if ((*m_dungeon)[(y + m)*cols + (x + n)].shop_action != PURCHASE)
								m_dungeon->deconstructShopHUD();

						}
					}
				}
				else if (wall && !boundary) {

					if (item && m_dungeon->itemObject(x + n, y + m)->isChest()) {
						// If it's a chest, open it
						if (m_dungeon->itemObject(x + n, y + m)->isChest()) {
							std::shared_ptr<Chests> chest = std::dynamic_pointer_cast<Chests>((*m_dungeon)[(y + m)*cols + (x + n)].object);
							chest->attemptOpen(*m_dungeon);
							chest.reset();
						}
					}				

					// Wall breaking ability
					else if (canBreakWalls() && m_dungeon->wallObject(x + n, y + m) && m_dungeon->wallObject(x + n, y + m)->isDestructible()) {
						m_dungeon->damagePlayer(10, DamageType::MAGICAL);
						m_dungeon->destroyWall(x + n, y + m);
					}

					// For traps that act as walls that can be interacted with (such as Braziers)
					else if (trap) {
						m_dungeon->trapEncounter(x + n, y + m);
					}

					else if (m_dungeon->wallObject(x + n, y + m) && m_dungeon->wallObject(x + n, y + m)->isDoor()) {
						std::shared_ptr<Door> door = std::dynamic_pointer_cast<Door>((*m_dungeon)[(y + m)*cols + (x + n)].wallObject);
						door->interact();
						door.reset();
					}

				}
				else if (enemy) {
					m_dungeon->fight(x + n, y + m, getWeapon()->getDamageType());
				}
			}

			// Collect/Interact
			else if (move == 'e') {
				int currentLevel = m_dungeon->getLevel();
				
				// do this if item needs to be purchased
				std::string shop_action = (*m_dungeon)[y*cols + x].shop_action;
				if (shop_action == "purchase")
					m_dungeon->purchaseItem(x, y);
				else
					m_dungeon->collectItem(x, y);

				if (m_dungeon->getLevel() > currentLevel)
					return;
			}

			// Spacebar item
			else if (move == 'b') {
				if (!isDisabled())
					useActiveItem();
			}

			// Switch weapon
			else if (move == 'w') {
				if (!isDisabled())
					switchWeapon();
			}

			//// Check if player is idling over a trap
			//if (!(move == 'l' || move == 'r' || move == 'u' || move == 'd')) {
			//	if (dungeon[getPosY()*cols + getPosX()].trap)
			//		m_dungeon->trapEncounter(getPosX(), getPosY());
			//}

		}

	}

	// Pick up any gold/money that was on the ground
	if (m_dungeon->gold(getPosX(), getPosY()) > 0) {
		m_dungeon->goldPickup(getPosX(), getPosY());

		if (hasGoldInvulnerability()) {
			setInvulnerable(true);
			addAffliction(std::make_shared<Invulnerability>(1));
		}
	}

}
void Player::attack(Actors &a) {
	m_attackedEnemy = true;

	int playerPoints = getDex() + getWeapon()->getDexBonus();
	int monsterPoints = a.getDex();

	// If the player has no weapon equipped (i.e. their hands), do nothing
	if (getWeapon()->getName() == HANDS)
		return;

	// if player is invisible, they lose invisibility when attacking
	if (isInvisible())
		removeAffliction(INVISIBILITY);
	
	if ((playerPoints > 0 ? randInt(playerPoints) : 0) >= (monsterPoints > 0 ? randInt(monsterPoints) : 0) ||
		a.isFrozen() || a.isStunned()) {
		
		if (a.getName() == "Smasher")
			playHitSmasher();		
		else
			playEnemyHit();
		

		// Check additional effects of attacking successfully
		checkExtraAttackEffects(a);

		if (getWeapon()->hasAbility())
			getWeapon()->useAbility(*m_dungeon, a);
				
		if (getWeapon()->isImbued())
			getWeapon()->useImbuement(*m_dungeon, a);


		/** Damage is calculated by the following:
		* 
		*   Base damage: Weapon's damage
		*   Player Str increases max damage.
		*   Max damage:  Base + Str
		*
		*   Damage = Base + (Value from 1 to Str) - Enemy's armor
		*   Dual wielding doubles a weapon's damage.
		*/
		int weaponDamage = getWeapon()->getDamage() * (m_dualWield ? 2 : 1);
		int armorMultiplier = a.isFrozen() ? 2 : 1; // Armor is doubled if they are frozen
		int damage = std::max(1, weaponDamage + (1 + randInt(std::max(1, getStr()))) - (armorMultiplier * a.getArmor()));

		damage += getWeapon()->getSharpnessBonus();
		
		if (hasBloodlust()) {
			float percent = getHP() / (float)getMaxHP();
			if (percent > 0.25f && percent <= 0.4f) damage += 1;
			else if (percent > 0.15f && percent <= 0.25f) damage += 2;
			else if (percent > 0.10f && percent <= 0.15f) damage += 3;
			else if (percent > 0.05f && percent <= 0.10f) damage += 5;
			else if (percent > 0.00f && percent <= 0.05f) damage += 8;
		}
				
		a.decreaseStatBy(StatType::HP, damage);

		int pos = m_dungeon->findMonster(a.getPosX(), a.getPosY());
		if (pos != -1)
			m_dungeon->monsterAt(pos)->reactToDamage();

		// tint monster sprite red and flash
		if (!a.isFrozen())
			runMonsterDamage(a.getSprite());
		else
			runMonsterDamageBlink(a.getSprite());

		// check sturdiness of monster (if hitting them results in them getting knocked back)
		if ((!a.isSturdy() || hasSteelPunch()) && !a.isDead()) {		
			int ax = a.getPosX();
			int ay = a.getPosY();

			int n = 0, m = 0;
			knockbackDirection(n, m, ax, ay);

			if (!(m_dungeon->wall(ax + n, ay + m) || m_dungeon->enemy(ax + n, ay + m))) {
				a.moveTo(ax + n, ay + m);

				// if knocking them back was successful, stun them for a turn
				a.addAffliction(std::make_shared<Stun>(1));
			}
		}

		// additional effects as a result of attacking successfully that are unique to a player
		successfulAttack(a);
	}
	else {
		playMiss();
	}
}
void Player::checkExtraAttackEffects(Actors &a) {
	// increase money bonus on successful hit
	increaseMoneyBonus();

	// Can lifesteal
	if (canLifesteal())
		rollHeal();

	// Can poison enemies
	if (hasPoisonTouch() && (a.canBePoisoned() || hasAfflictionOverride())) {
		if (1 + randInt(100) + getLuck() > 95) {
			int turns = 5 + (hasHarshAfflictions() ? 5 : 0);
			a.addAffliction(std::make_shared<Poison>(*this, turns, 2, 1, 1));
		}
	}

	// Can ignite enemies
	if (hasFireTouch() && (a.canBeBurned() || hasAfflictionOverride())) {
		if (1 + randInt(100) + getLuck() > 95) {
			int turns = 5 + (hasHarshAfflictions() ? 5 : 0);
			a.addAffliction(std::make_shared<Burn>(*this, turns));
		}		
	}

	// Can freeze enemies
	if (hasFrostTouch() && (a.canBeFrozen() || hasAfflictionOverride())) {
		if (1 + randInt(100) + getLuck() > 95) {
			int turns = 5 + (hasHarshAfflictions() ? 5 : 0);
			a.addAffliction(std::make_shared<Freeze>(turns));
		}
	}

	// Crippling blows
	if (hasCripplingBlows()) {
		if (1 + randInt(100) + getLuck() > 95)
			a.addAffliction(std::make_shared<Cripple>(8));
	}

	// Chain lightning
	if (hasChainLightning() && 1 + randInt(100) + getLuck() > 95)
		chainLightning(a);
	

	// Check if weapon is metered
	if (getWeapon()->hasMeter())
		getWeapon()->increaseCharge();

}
void Player::botchedAttack(Actors &a) {
	// Metal sound effect
	playHitSmasher();

	// check sturdiness of monster (if hitting them results in them getting knocked back)
	if (!a.isSturdy() && !a.isDead()) {
		int ax = a.getPosX();
		int ay = a.getPosY();

		int n = 0, m = 0;
		setDirectionalOffsets(m_facing, n, m);

		if (!(m_dungeon->wall(ax + n, ay + m) || m_dungeon->enemy(ax + n, ay + m))) {
			a.moveTo(ax + n, ay + m);

			// if knocking them back was successful, stun them for a turn
			a.addAffliction(std::make_shared<Stun>(1));
		}
	}
}
void Player::knockbackDirection(int &x, int &y, int mx, int my) {

	int px = getPosX();
	int py = getPosY();

	switch (facingDirection()) {
	case 'l':
		// If monster is to the left of the player, then assign as normal
		if (mx < px) {
			x = -1; y = 0;
		}
		else if (mx > px) {
			x = 1; y = 0;
		}
		// If monster is below the player
		else if (my > py) {
			x = 0; y = 1;
		}
		else {
			x = 0; y = -1;
		}
		break;
	case 'r': 
		// If monster is to the right of the player, then assign as normal
		if (mx > px) {
			x = 1; y = 0;
		}
		else if (mx < px) {
			x = -1; y = 0;
		}
		// If monster is below the player
		else if (my > py) {
			x = 0; y = 1;
		}
		else {
			x = 0; y = -1;
		}
		break;
	case 'u': 
		// Normal assignment
		if (my < py) {
			x = 0; y = -1;
		}
		else if (my > py) {
			x = 0; y = 1;
		}
		else if (mx < px) {
			x = -1; y = 0;
		}
		else {
			x = 1; y = 0;
		}
		break;
	case 'd':
		// Normal assignment
		if (my > py) {
			x = 0; y = 1;
		}
		else if (my < py) {
			x = 0; y = -1;
		}
		else if (mx < px) {
			x = -1; y = 0;
		}
		else {
			x = 1; y = 0;
		}
		break;
	}
}
void Player::chainLightning(Actors &a) {
	int ax, ay;
	int cols = m_dungeon->getCols();

	// Use dummy map so we don't have to unmark tiles at the end
	std::vector<_Tile> dummyMap = m_dungeon->getDungeon();
	
	std::stack<std::shared_ptr<Actors>> st;
	st.push(std::make_shared<Actors>(a));

	while (!st.empty()) {
		std::shared_ptr<Actors> actor = st.top();
		st.pop();

		ax = actor->getPosX();
		ay = actor->getPosY();

		for (int i = ax - 1; i < ax + 2; i++) {
			for (int j = ay - 1; j < ay + 2; j++) {

				if (dummyMap[j*cols + i].marked)
					continue;

				dummyMap[j*cols + i].marked = true;

				// Add any adjacent enemies onto the stack
				if (dummyMap[j*cols + i].enemy) {
					int pos = m_dungeon->findMonster(i, j);
					if (pos != -1)
						st.push(m_dungeon->monsterAt(pos));
				}
			}
		}

		// Now damage the current actor
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, ax, ay, 2);

		actor->setHP(actor->getHP() - (a.getStr() + a.getInt()));
	}
}
void Player::reactToDamage(Monster &m) {
	if (hasFragileRetaliation() && randReal(1, 100) + getLuck() > 92) {
		playSound("IceBreak7.mp3");
		tintStunned(m.getSprite());

		m.setFragile(true);
		m.addAffliction(std::make_shared<Fragile>(1));
	}
}

void Player::equipWeapon(std::shared_ptr<Weapon> weapon) {
	// Remove dual wielding, if applicable
	if (m_dualWield)
		m_dualWield = false;

	// Unapply any previous bonuses
	if (getWeapon()->hasBonus())
		getWeapon()->unapplyBonus(*this);

	getWeapon().reset();
	setWeapon(weapon);

	// Check for dual wielding capability
	if (weapon->canDualWield() && getStoredWeapon() && weapon->getName() == getStoredWeapon()->getName())
		m_dualWield = true;
	
	if (getWeapon()->hasBonus())
		getWeapon()->applyBonus(*this);
}
void Player::storeWeapon(std::shared_ptr<Weapon> weapon) {
	int cols = m_dungeon->getCols();

	int x = getPosX();
	int y = getPosY();

	m_dungeon->removeItem(weapon->getPosX(), weapon->getPosY());

	// If the player is empty-handed because they threw a weapon or it broke, then directly equip the new weapon
	if (getWeapon()->getName() == HANDS) {
		setWeapon(weapon);
		return;
	}

	// Check for dual wielding capability
	if (weapon->canDualWield() && weapon->getName() == getWeapon()->getName())
		m_dualWield = true;
	

	// If this is the first time a player is picking up a new weapon, simply add it to their inventory
	if (m_storedWeapon == nullptr) {
		m_storedWeapon = weapon;
		return;
	}

	// Otherwise we drop the player's current weapon and switch it with the new one
	dropWeapon();
	setWeapon(weapon);

	if (getWeapon()->hasBonus())
		getWeapon()->applyBonus(*this);
}
void Player::dropWeapon() {
	int cols = m_dungeon->getCols();
	
	int x = getPosX();
	int y = getPosY();

	// Unapply any previous bonuses
	if (getWeapon()->hasBonus())
		getWeapon()->unapplyBonus(*this);

	getWeapon()->setPosX(x); getWeapon()->setPosY(y);
	m_dungeon->createItem(getWeapon());
	/*dungeon[y*cols + x].object = getWeapon();
	dungeon[y*cols + x].item = true;
	dungeon[y*cols + x].object->setSprite(m_dungeon->createSprite(x, y, -1, getWeapon()->getImageName()));
	m_dungeon->addItem(dungeon[y*cols + x].object);*/
}
void Player::dropStoredWeapon() {
	int x = getPosX();
	int y = getPosY();

	// Remove dual wielding, if applicable
	if (m_dualWield)
		m_dualWield = false;

	std::shared_ptr<Objects> storedWeapon = m_storedWeapon;

	if (m_dungeon->itemObject(x, y))
		m_dungeon->itemHash(x, y);

	m_storedWeapon->setPosX(x); m_storedWeapon->setPosY(y);
	m_dungeon->createItem(m_storedWeapon);
	/*dungeon[y*cols + x].object = storedWeapon;
	dungeon[y*cols + x].item = true;
	dungeon[y*cols + x].object->setSprite(m_dungeon->createSprite(x, y, -1, storedWeapon->getImageName()));
	m_dungeon->addItem(dungeon[y*cols + x].object);*/

	m_storedWeapon = nullptr;
	storedWeapon.reset();
}
void Player::switchWeapon() {

	// If player doesn't have any stored weapon, then just return
	if (m_storedWeapon == nullptr)
		return;

	playInterfaceSound("Confirm 1.mp3");

	std::shared_ptr<Weapon> prevWeapon = getWeapon();

	// Unapply any previous bonuses
	if (getWeapon()->hasBonus())
		getWeapon()->unapplyBonus(*this);

	setWeapon(m_storedWeapon);
	m_storedWeapon = prevWeapon;

	if (getWeapon()->hasBonus())
		getWeapon()->applyBonus(*this);

	prevWeapon.reset();
}
void Player::throwWeaponTo(int x, int y) {
	int cols = m_dungeon->getCols();

	// Unapply any previous bonuses
	if (getWeapon()->hasBonus())
		getWeapon()->unapplyBonus(*this);

	// Remove dual wielding, if applicable
	if (m_dualWield)
		m_dualWield = false;

	//std::shared_ptr<Objects> oldWeapon = getWeapon();

	if (m_dungeon->itemObject(x, y))
		m_dungeon->itemHash(x, y);

	getWeapon()->setPosX(x); getWeapon()->setPosY(y);
	//m_dungeon->createItem(getWeapon());
	
	int px = getPosX();
	int py = getPosY();

	(*m_dungeon)[y*cols + x].object = getWeapon();
	(*m_dungeon)[y*cols + x].object->setSprite(m_dungeon->createInvisibleSprite(px, py, y + Z_ITEM, getWeapon()->getImageName()));
	(*m_dungeon)[y*cols + x].item = true;
	m_dungeon->addItem((*m_dungeon)[y*cols + x].object);

	// Throw animation
	m_dungeon->queueCustomAction(getWeapon()->getSprite(), cocos2d::Show::create());
	cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	v.pushBack(cocos2d::RotateBy::create(.1f, 720));

	float fx, fy;
	m_dungeon->transformDungeonToSpriteCoordinates(x, y, fx, fy);
	auto move = cocos2d::MoveTo::create(.1f, cocos2d::Vec2(fx, fy));
	v.pushBack(move);

	m_dungeon->queueCustomSpawnActions(getWeapon()->getSprite(), v);

	m_dungeon->queueCustomAction(getSprite(), cocos2d::DelayTime::create(.1f));
	auto sound = cocos2d::CallFunc::create([]() {
		playSound("Metal_Hit8.mp3");
	});
	m_dungeon->queueCustomAction(getSprite(), sound);

	// If player had no other weapon, then they get equipped with their hands (which do nothing)
	if (m_storedWeapon == nullptr) {
		setWeapon(std::make_shared<Hands>());
	}
	else {
		setWeapon(m_storedWeapon);
		m_storedWeapon = nullptr;
	}
}
void Player::tradeWeapon(std::shared_ptr<Weapon> weapon) {
	getWeapon().reset();
	setWeapon(weapon);
}
void Player::removeStoredWeapon() {
	// Remove dual wielding, if applicable
	if (m_dualWield)
		m_dualWield = false;

	m_storedWeapon.reset();
	m_storedWeapon = nullptr;
}

void Player::addItem(std::shared_ptr<Drops> drop) {
	if (m_items.size() == m_maxiteminv && !(drop->canStack() || hasFatStacks()))
		return;

	// If this item is stackable, check if the player already has the same item
	if (drop->canStack() || hasFatStacks()) {
		for (unsigned int i = 0; i < m_items.size(); i++) {

			// If they do, add the current drop's count to the existing count
			if (m_items.at(i)->getName() == drop->getName()) {
				playSound(drop->getSoundName());
				m_items.at(i)->increaseCountBy(drop->getCount());
				return;
			}
		}
	}

	if ((int)m_items.size() < m_maxiteminv)
		m_items.push_back(drop);
}
void Player::addItem(std::shared_ptr<Drops> drop, bool &itemAdded) {
	if (m_items.size() == m_maxiteminv && !(drop->canStack() || hasFatStacks()))
		return;

	// If this item is stackable, check if the player already has the same item
	if (drop->canStack() || hasFatStacks()) {
		for (unsigned int i = 0; i < m_items.size(); i++) {

			// If they do, add the current drop's count to the existing count
			if (m_items.at(i)->getName() == drop->getName()) {
				playSound(drop->getSoundName());

				drop->pickupEffect(*m_dungeon);
				m_items.at(i)->increaseCountBy(drop->getCount());

				itemAdded = true;
				return;
			}
		}
	}

	if ((int)m_items.size() < m_maxiteminv) {
		playSound(drop->getSoundName());

		drop->pickupEffect(*m_dungeon);
		m_items.push_back(drop);
		itemAdded = true;
	}
}
void Player::use(int index) {
	if (isDisabled())
		return;

	m_items.at(index)->useItem(*m_dungeon);
	
	if (m_items.at(index)->getName() != SKELETON_KEY) {

		// Resonant Spells passive: 50% base chance to keep the spell
		if (m_resonantSpells && m_items.at(index)->isSpell()) {
			if (1 + randInt(100) + getLuck() > 50)
				return;
		}

		if (m_items.at(index)->canStack() || hasFatStacks()) {
			m_items.at(index)->decreaseCount();

			if (m_items.at(index)->getCount() == 0)
				m_items.erase(m_items.begin() + index);

			return;
		}

		m_items.erase(m_items.begin() + index);	// remove item just used
	}
}
void Player::removeItems() {
	for (int i = m_items.size() - 1; i >= 0; i--) {
		m_items.at(i).reset();
		m_items.erase(m_items.begin() + i);
	}
}
void Player::removeItem(int index) {
	m_items.at(index).reset();
	m_items.erase(m_items.begin() + index);
}

void Player::equipPassive(std::shared_ptr<Passive> passive) {
	m_passives.push_back(passive);
	passive->apply(*this);
	//LootTable::acquiredPassive(passive->getName());
}
void Player::removePassive(int index) {
	m_passives.at(index)->unapply(*this);
	m_passives.at(index).reset();
	m_passives.erase(m_passives.begin() + index);
}

void Player::equipRelic(std::shared_ptr<Relic> relic) {
	playSound("Potion_Pickup.mp3");

	setRelicFlag(true);
	m_relic = relic;
	m_relic->apply(*m_dungeon, *this);
	LootTable::acquiredRelic(relic->getName());
}
void Player::removeRelic() {
	m_relic->unapply(*m_dungeon, *this);
	m_relic.reset();
	m_relic = nullptr;
	setRelicFlag(false);
}

void Player::setItemToFront(int index) {
	std::shared_ptr<Drops> temp = m_items[0];
	m_items[0] = m_items[index];
	m_items[index] = temp;
}

void Player::increaseFavorBy(int favor) {

	while (favor > 0 && m_favor < 20) {
		m_favor++;

		switch (m_favor) {
		case -15: m_timerReduction += 0.10f; break; // Level timer is restored
		case -11: break;
		case -7: break;
		case -3: break; // Removes chest rigging
		case 0: break;
		case 4: setLuck(getLuck() + 5); break;
		case 8: setVision(getVision() + 1); break;
		case 12: equipPassive(rollPassive(m_dungeon, getPosX(), getPosY(), Rarity::ALL, false)); break; // Free passive
		case 16: setStr(getStr() + 1); setDex(getDex() + 1); setInt(getInt() + 1); setMaxHP(getMaxHP() * 1.05); break;
		case 20: // Special item
		default: break;
		}

		favor--;
	}
}
void Player::decreaseFavorBy(int favor) {

	while (favor > 0 && m_favor > -20) {
		m_favor--;

		switch (m_favor) {
		case 19: break;
		case 15: setStr(getStr() - 1); setDex(getDex() - 1); setInt(getInt() - 1); setMaxHP(getMaxHP() / 1.05); break;
		case 11: break; // Player keeps their passive even if they drop
		case 7: setVision(getVision() - 1); break;
		case 3: setLuck(getLuck() - 5); break;
		case 0: break;
		case -4: break; // Chests more likely to be rigged. Partially implemented passively through Chest openEffect.
		case -8: break; // Prices become more expensive. Partially implemented for shops.
		case -12: break; // NPCs less likely to spawn. Partially implemented in first floor constructor.
		case -16: m_timerReduction -= 0.10f; break; // Level timer becomes faster
		case -20: m_spiritActive = true; break; // Evil spirit chases player permanently
		default: break;
		}

		favor--;
	}
}

bool Player::hasAfflictionAversion() const {
	return findAffliction(AFFLICTION_IMMUNITY) != -1;
}

bool Player::hasExperienceGain() const {
	return findAffliction(XP_GAIN) != -1;
}
void Player::increaseXPBy(int amount) {
	m_xp += amount;
	if (m_xp >= m_xpMax) {
		playSound("Life_Potion_Used.mp3");

		m_xp = 0;
		increaseStatBy(StatType::MAXHP, 10);

		int i = 0;
		while (i < 2) {
			switch (1 + randInt(3)) {
			case 1: increaseStatBy(StatType::STRENGTH, 1); break;
			case 2: increaseStatBy(StatType::DEXTERITY, 1); break;
			case 3: increaseStatBy(StatType::INTELLECT, 1); break;
			}

			i++;
		}
	}
};

bool Player::hasSkeletonKey() const {
	for (unsigned int i = 0; i < m_items.size(); i++) {
		if (m_items.at(i)->getName() == SKELETON_KEY)
			return true;
	}
	return false;
}
void Player::checkKeyConditions() {
	// if player hp is less than a specified threshold, and player was hit after picking up the key, break it
	if (getHP() < 8 && getHP() < keyHP()) {
		for (unsigned int i = 0; i < m_items.size(); i++) {
			if (m_items.at(i)->getName() ==	SKELETON_KEY) {
				// play key broken sound effect
				playSound("Skeleton_Key_Broken.mp3");

				m_items.erase(m_items.begin() + i);	// remove the key
				return;
			}
		}
	}
}

void Player::rollHeal() {
	if (getHP() > 0 && getHP() + 2 < getMaxHP()) {
		if (randInt(100) + 1 > 90) {
			// 10% chance to heal the player if below max hp
			setHP(getHP() + 2);
		}
	}
}


Adventurer::Adventurer() : Player(1000, std::make_shared<BoneAxe>(0, 0)) {
	//setStr(1000);
	//setHP(100);
	setName(ADVENTURER);
	setImageName("Player1_48x48.png");
	
	m_shield = std::make_shared<WoodShield>(0, 0);
	setHasActiveItem(true);
	setActiveItem(m_shield);
	addItem(std::make_shared<RPGInABottle>(0, 0));
	addItem(std::make_shared<MagmaHeart>(0, 0));
	addItem(std::make_shared<RPGInABottle>(0, 0));
	addItem(std::make_shared<LifePotion>(0, 0));

	addSecondaryWeapon(std::make_shared<Estoc>(0, 0));

	if (getWeapon()->hasBonus())
		getWeapon()->applyBonus(*this);
}
Adventurer::~Adventurer() {
	if (m_shield)
		m_shield.reset();
}

void Adventurer::useActiveItem() {
	if (m_shield == nullptr)
		return;

	// play setting up shield stance sound effect
	playSound("Shield_Stance.mp3");

	m_blocking = true;
	showShieldBlock(*m_dungeon, *this);

	// If the player is dual wielding, then they have to drop their second weapon
	if (isDualWielding())
		dropStoredWeapon();
	
}
void Adventurer::equipActiveItem(std::shared_ptr<Objects> active) {
	if (active->isShield()) {
		std::shared_ptr<Shield> shield = std::dynamic_pointer_cast<Shield>(active);
		equipShield(shield);
		shield.reset();
	}
}
int Adventurer::getCurrentActiveMeter() const {
	return m_shield->getDurability();
}
int Adventurer::getMaxActiveMeter() const {
	return m_shield->getMaxDurability();
}
bool Adventurer::activeHasAbility() const {
	if (m_shield == nullptr)
		return false;

	return m_shield->hasAbility();
}
void Adventurer::useActiveAbility(Actors &a) {
	m_shield->useAbility(*m_dungeon, a);
}

void Adventurer::equipShield(std::shared_ptr<Shield> shield) {
	int x = getPosX();
	int y = getPosY();

	m_dungeon->removeItem(shield->getPosX(), shield->getPosY());

	// If player has a shield, then drop and replace with new one
	if (m_shield != nullptr) {
		dropShield();

		m_shield = shield;
		setActiveItem(m_shield);

		return;
	}

	setHasActiveItem(true);
	m_shield = shield;
	setActiveItem(m_shield);

	//playShieldEquip(getPlayerShield()->getName());
}
void Adventurer::dropShield() {
	int x = getPosX();
	int y = getPosY();

	// play shield drop sound
	playSound("Wood_Shield_Drop.mp3");

	m_shield->setPosX(x); m_shield->setPosY(y);
	m_dungeon->createItem(m_shield);
	/*dungeon[y*cols + x].object = m_shield;
	dungeon[y*cols + x].item = true;
	dungeon[y*cols + x].object->setSprite(m_dungeon->createSprite(x, y, -1, m_shield->getImageName()));
	m_dungeon->addItem(m_shield);*/
}
void Adventurer::shieldBroken() {
	// play shield broken sound effect
	playShieldBroken(m_shield->getName());

	setHasActiveItem(false);
	
	m_shield.reset();
	m_shield = nullptr;
}
void Adventurer::restoreActive(int repairs) {
	if (m_shield != nullptr) {
		// if repairs do not exceed the shield's durability, then just repair it as normal
		if (m_shield->getDurability() + repairs < m_shield->getMaxDurability()) {
			m_shield->setDurability(m_shield->getDurability() + repairs);
		}
		// if repairs exceeds but does not equal the shield's max durability, then set to max
		else if (m_shield->getDurability() + repairs > m_shield->getMaxDurability()) {
			m_shield->setDurability(m_shield->getMaxDurability());
		}
		// else if equal, then do nothing
		else {
			;
		}
	}
}

bool Adventurer::canBlock() const {
	if (m_shield == nullptr)
		return false;

	return true;
}
bool Adventurer::didBlock(int mx, int my) const {

	if (m_shield == nullptr || !m_blocking)
		return false;

	int px = getPosX();
	int py = getPosY();
	bool blocked = false;

	// check shield's coverage
	switch (m_shield->getCoverage()) {
		// level 1 coverage blocks only in the front
	case 1: {
		// check direction player is facing
		switch (facingDirection()) {
		case 'l': {
			if (mx < px && my == py) {
				blocked = true;
			}
		}
				  break;
		case 'r': {
			if (mx > px && my == py) {
				blocked = true;
			}
		}
				  break;
		case 'u': {
			if (mx == px && my < py) {
				blocked = true;
			}
		}
				  break;
		case 'd': {
			if (mx == px && my > py) {
				blocked = true;
			}
		}
				  break;
		}
	}
			break;
			// level 2 coverage blocks front and diagonals
	case 2: {
		// check direction player is facing
		switch (facingDirection()) {
		case 'l': {
			if (mx < px && abs(mx - px) >= abs(my - py)) {
				blocked = true;
			}
		}
				  break;
		case 'r': {
			if (mx > px && abs(mx - px) >= abs(my - py)) {
				blocked = true;
			}
		}
				  break;
		case 'u': {
			if (abs(mx - px) <= abs(my - py) && my < py) {
				blocked = true;
			}
		}
				  break;
		case 'd': {
			if (abs(mx - px) <= abs(my - py) && my > py) {
				blocked = true;
			}
		}
				  break;
		}
	}
			break;
			// level 3 coverage blocks front and sides
	case 3: {
		// check direction player is facing
		switch (facingDirection()) {
		case 'l': {
			if (mx <= px) {
				blocked = true;
			}
		}
				  break;
		case 'r': {
			if (mx >= px) {
				blocked = true;
			}
		}
				  break;
		case 'u': {
			if (my <= py) {
				blocked = true;
			}
		}
				  break;
		case 'd': {
			if (my >= py) {
				blocked = true;
			}
		}
				  break;
		}
	}
			break;
	}

	/*if (blocked) {
		successfulBlock();
	}*/

	return blocked;
}
int Adventurer::blockedDamageReduction() {
	return m_shield->getDefense();
}
void Adventurer::successfulBlock() {
	// play shield hit sound effect
	playShieldHit(m_shield->getName());

	// decrease shield's durability by half the value of how much the shield blocks by
	if (m_shield->getDurability() > 0) {
		m_shield->setDurability(m_shield->getDurability() - m_shield->getDefense() / 2);
	}
	// if shield breaks, remove from the player
	if (m_shield->getDurability() <= 0) {
		shieldBroken();
	}
}


Spellcaster::Spellcaster() : Player(7000, std::make_shared<ArcaneStaff>(0, 0)) {
	setStr(1);
	setInt(3);
	setArmor(1);
	m_spell = std::make_shared<WhirlwindSpell>(0, 0);
	setHasActiveItem(true);
	setActiveItem(m_spell);

	setName(SPELLCASTER);
	setImageName("Spellcaster_48x48.png");
}
Spellcaster::~Spellcaster() {
	if (m_spell)
		m_spell.reset();
}

void Spellcaster::useActiveItem() {
	if (m_mp > 4) {
		m_spell->useItem(*m_dungeon);

		m_mp -= 4;
	}

	// If the player is dual wielding, then they have to drop their second weapon
	if (isDualWielding())
		dropStoredWeapon();
	
}
void Spellcaster::equipActiveItem(std::shared_ptr<Objects> active) {
	if (active->isSpell()) {
		std::shared_ptr<Spell> spell = std::dynamic_pointer_cast<Spell>(active);
		m_spell.reset();
		m_spell = spell;
		setActiveItem(m_spell);
		spell.reset();
	}
}
void Spellcaster::successfulAttack(Actors &a) {
	if (m_mp < m_maxMP)
		m_mp++;
}


Spelunker::Spelunker() : Player(100, std::make_shared<Whip>(0, 0)) {
	setArmor(1);
	setHasActiveItem(true);
	m_rocks.emplace_back(std::make_shared<Rocks>(0, 0));
	m_rocks.emplace_back(std::make_shared<Rocks>(0, 0));
	m_rocks.emplace_back(std::make_shared<Rocks>(0, 0));
	setActiveItem(m_rocks[0]);
	addSecondaryWeapon(std::make_shared<Mattock>(0, 0));

	setName(SPELUNKER);
	setImageName("Spelunker_48x48.png");
}
Spelunker::~Spelunker() {
	for (auto &it : m_rocks)
		it.reset();
}

void Spelunker::useActiveItem() {

	if (m_rocks.empty())
		return;

	m_rocks[0]->useItem(*m_dungeon);
	m_rocks.erase(m_rocks.begin());

	if (m_rocks.empty())
		setHasActiveItem(false);
	
}
void Spelunker::equipActiveItem(std::shared_ptr<Objects> active) {
	if (active->getName() == ROCKS) {
		std::shared_ptr<Rocks> rock = std::dynamic_pointer_cast<Rocks>(active);
		m_rocks.push_back(rock);
		rock.reset();

		setHasActiveItem(true);
	}
}


Acrobat::Acrobat() : Player(80, std::make_shared<Zweihander>(0, 0)) {
	m_stamina = 30;
	m_maxStamina = m_stamina;

	setHasActiveItem(true);
	m_item = std::make_shared<Mobility>();
	setActiveItem(m_item);

	setName(ACROBAT);
	setImageName("OutsideMan2.png");
}
Acrobat::~Acrobat() {
	if (m_item)
		m_item.reset();
}

void Acrobat::useActiveItem() {
	if (m_stamina > 4) {
		m_item->useItem(*m_dungeon);

		m_stamina -= 4;
	}
}
void Acrobat::equipActiveItem(std::shared_ptr<Objects> active) {
	return;
}
void Acrobat::successfulAttack(Actors &a) {
	if (m_stamina < m_maxStamina)
		m_stamina++;
}


TheMadman::TheMadman() : Player(50, std::make_shared<Hands>()) {
	setStr(1);
	setVision(9);

	setHasActiveItem(true);
	setActiveItem(std::make_shared<Teleporter>());
	m_item = std::make_shared<Teleporter>();

	setName(MADMAN);
	setImageName("Madman_48x48.png");
}
TheMadman::~TheMadman() {
	if (m_item)
		m_item.reset();
}

void TheMadman::useActiveItem() {
	m_item->useItem(*m_dungeon);
}
void TheMadman::equipActiveItem(std::shared_ptr<Objects> active) {
	return;
}


// ==========================================
//				:::: NPCS ::::
// ==========================================
NPC::NPC(Dungeon *dungeon, int x, int y, std::string name, std::string image) : Actors(x, y, 100, 5, 5, 5, std::make_shared<ShortSword>(), name) {
	m_dungeon = dungeon;
	(*dungeon)[y*dungeon->getCols() + x].npc = true;
	(*dungeon)[y*dungeon->getCols() + x].wall = true; // so they can't be moved

	setImageName(image);
}

void NPC::talk() {
	// if this is the first time the player has interacted with this NPC
	if (!m_interacted) {
		addInitialDialogue();
		playDialogue();
		m_interacted = true;
	}
	// else if player has interacted but NPC has not been satisfied
	else if (m_interacted) {
		checkSatisfaction();

		if (!m_satisfied) {
			addInteractedDialogue();
			playDialogue();
		}
		else if (m_satisfied && !m_rewardGiven) {
			addSatisfiedDialogue();
			playDialogue();
			reward();
			m_rewardGiven = true;
		}
		else if (m_satisfied && m_rewardGiven) {
			addFinalDialogue();
			playDialogue();
		}
	}

}

inline void NPC::playDialogue() { m_dungeon->playNPCDialogue(*this, m_dialogue); };


//		CREATURE LOVER (Lionel)
CreatureLover::CreatureLover(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, CREATURE_LOVER, "Dead_Mage_48x48.png") {
	m_wantedCreature = GOO_SACK;
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	addDialogue("Oh... hello.");
	addDialogue("I love " + m_wantedCreature + "s.");
	addDialogue("Those " + m_wantedCreature + "s are just adorable little things!");
	addDialogue("Can you bring me one?");

	setInteractionLimit(5);
}

void CreatureLover::checkSatisfaction() {
	int cols = m_dungeon->getCols();

	std::string monsterName;

	for (int y = getPosY() - 1; y < getPosY() + 2; y++) {
		for (int x = getPosX() - 1; x < getPosX() + 2; x++) {

			if (m_dungeon->enemy(x, y)) {
				int pos = m_dungeon->findMonster(x, y);

				if (pos != -1) {
					monsterName = m_dungeon->monsterAt(pos)->getName();

					if (monsterName == m_wantedCreature) {
						playSound("Creature_Lover_Talk.mp3");

						m_dungeon->addSprite(x, y, 1, m_dungeon->monsterAt(pos)->getImageName());
						m_dungeon->monsterAt(pos)->setDestroyed(true);
						
						setSatisfaction(true);
						return;
					}
				}
			}
		}
	}
}
void CreatureLover::reward() {
	int x = m_dungeon->getPlayer()->getPosX();
	int y = m_dungeon->getPlayer()->getPosY();

	if (m_dungeon->itemObject(x, y))
		m_dungeon->itemHash(x, y);

	m_dungeon->createItem(rollPassive(m_dungeon, x, y, Rarity::RARE, true));
}

void CreatureLover::addInteractedDialogue() {
	m_dialogue.clear();

	switch (getInteractionStage()) {
	case 1: addDialogue(m_wantedCreature + "s, " + m_wantedCreature + "s, " + m_wantedCreature + "s!!!"); break;
	case 2: addDialogue("Please, bring me a " + m_wantedCreature + "."); break;
	case 3: addDialogue("Surely an adventurer like yourself must know what a " + m_wantedCreature + " is."); break;
	case 4: 
		addDialogue("I have other hobbies too.");
		addDialogue("...");
		addDialogue("I do.");
		break;
	case 5: addDialogue("Run along now, and don't forget to bring me what I asked."); break;
	}

	// if npc still has new things to say, increment interaction stage
	if (getInteractionStage() < getInteractionLimit())
		incInteractionStage();
	
}
void CreatureLover::addSatisfiedDialogue() {
	m_dialogue.clear();

	addDialogue("?!");
	addDialogue("A " + m_wantedCreature + "! " + "I can't believe it!");
	addDialogue("You deserve something for this.");
}
void CreatureLover::addFinalDialogue() {
	m_dialogue.clear();

	addDialogue("...So adorable!");
}

//		MEMORIZER
Memorizer::Memorizer(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, MEMORIZER, "Orc_48x48.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	addDialogue("HeHeHe");
	addDialogue("Wanna play a game?");
	addDialogue(NPC_PROMPT);

	setInteractionLimit(1);

	switch (randInt(2)) {
	case 0: m_topic = "Gold"; break;
	case 1: m_topic = "HP"; break;
	}

	// Choices for the player when prompted
	addChoice("Why not.");
	addChoice("No thanks.");
}

void Memorizer::useResponse(int index) {
	m_dialogue.clear();

	switch (m_promptStage) {
	case 1:
		switch (index) {
			// Yes
		case 0: {
			m_promptChoices.clear();

			addDialogue("Hope you've been keeping track.");
			addDialogue("How much " + m_topic + " do you currently have?");
			addDialogue(NPC_PROMPT);

			std::string count1, count2;
			if (m_topic == "Gold") {
				m_correctChoice = std::to_string(m_dungeon->getPlayer()->getMoney());
				switch (randInt(2)) {
				case 0:
					count1 = std::to_string(m_dungeon->getPlayer()->getMoney());
					count2 = std::to_string(m_dungeon->getPlayer()->getMoney() + 10);
					break;
				case 1:
					count1 = std::to_string(m_dungeon->getPlayer()->getMoney() + 10);
					count2 = std::to_string(m_dungeon->getPlayer()->getMoney());
					break;
				}
			}
			else if (m_topic == "HP") {
				m_correctChoice = std::to_string(m_dungeon->getPlayer()->getHP());
				switch (randInt(2)) {
				case 0:
					count1 = std::to_string(m_dungeon->getPlayer()->getHP());
					count2 = std::to_string(m_dungeon->getPlayer()->getHP() + 2);
					break;
				case 1:
					count1 = std::to_string(m_dungeon->getPlayer()->getHP() + 2);
					count2 = std::to_string(m_dungeon->getPlayer()->getHP());
					break;
				}
				
			}
			addChoice(count1);
			addChoice(count2);
			m_promptStage = 2;

			break;
		}
			// No
		case 1:
			addDialogue("Suit yourself.");
			break;
		}
		break;
	case 2:
		if (getChoices()[index] == m_correctChoice) {
			addDialogue("Haha! Very well.");
			addDialogue("Claim your prize.");
			setSatisfaction(true);
			reward();
			rewardWasGiven();
		}
		else {
			addDialogue("Seems you need to work on your memory a bit more.");
			addDialogue("I've got nothing for you.");
			incInteractionStage();
		}
		m_promptChoices.clear();
	}

}
void Memorizer::checkSatisfaction() {
	return;
}
void Memorizer::reward() {
	int x = m_dungeon->getPlayer()->getPosX();
	int y = m_dungeon->getPlayer()->getPosY();

	if (m_dungeon->itemObject(x, y))
		m_dungeon->itemHash(x, y);

	m_dungeon->createItem(rollWeapon(m_dungeon, x, y, Rarity::UNCOMMON, true));
}

void Memorizer::addInteractedDialogue() {
	m_dialogue.clear();
	m_promptChoices.clear();

	switch (getInteractionStage()) {
	case 1: {
		addDialogue("Oh, change your mind?");
		addDialogue(NPC_PROMPT);

		addChoice("Fine.");
		addChoice("No, didn't mean to talk to you again.");
		break;
	}
	case 2:
		addDialogue("I've got nothing for you.");
		break;
	}

	// if npc still has new things to say, increment interaction stage
	/*if (getInteractionStage() < getInteractionLimit())
		incInteractionStage();*/
}
void Memorizer::addSatisfiedDialogue() {
	m_dialogue.clear();
	return;
}
void Memorizer::addFinalDialogue() {
	m_dialogue.clear();

	addDialogue("Until we meet again.");
}

//		INJURED EXPLORER
InjuredExplorer::InjuredExplorer(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, INJURED_EXPLORER, "Orc_48x48.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	addDialogue("... Oh!");
	addDialogue("Thank heavens someone came by. My leg, it's not doing so good.");
	addDialogue("I could really use some help, do you have anything to help me out?");
	addDialogue(NPC_PROMPT);

	setInteractionLimit(1);

	// Choices for the player when prompted
	if (playerHasHealingItem()) {
		addChoice("Here.");
		m_promptStage++;
	}

	addChoice("I wish I could help.");
}

void InjuredExplorer::useResponse(int index) {
	m_dialogue.clear();

	switch (m_promptStage) {
		// No healing item
	case 1:
		addDialogue("It looks likes this is my end then.");		
		break;
		// Has healing item
	case 2:
		switch (index) {
			// Choose item to give
		case 0: {
			m_promptChoices.clear();

			addDialogue("You're a life saver!");
			addDialogue(NPC_PROMPT);

			addChoice("I changed my mind.");

			for (unsigned int i = 0; i < m_dungeon->getPlayer()->itemCount(); i++) {
				if (m_dungeon->getPlayer()->itemAt(i)->isHealingItem())
					addChoice("Give " + m_dungeon->getPlayer()->itemAt(i)->getName());
			}

			m_healingItemCount = (int)m_promptChoices.size() - 1;
			
			m_promptStage = 3;

			break;
		}
			// No
		case 1:
			addDialogue("I knew it, I'm gonna die in here.");
			addDialogue("Just leave me.");

			//m_promptStage = 4;
			break;
		}

		break;
		// Give item
	case 3:
		switch (index) {
		case 0:
			addDialogue("Is this some sort of game to you?");
			addDialogue("Toying with a man's life will bring you misery.");
			break;
		case 1: 
		case 2: 
		case 3: 
		case 4: 
		case 5:
			setSatisfaction(true);
			reward();
			rewardWasGiven();

			int removeIndex = -1;
			for (unsigned int i = 0; i < m_dungeon->getPlayer()->itemCount(); i++) {
				if (m_dungeon->getPlayer()->itemAt(i)->isHealingItem())
					removeIndex++;

				if (removeIndex == index - 1) {
					removeIndex = i;
					break;
				}
			}

			m_dungeon->getPlayer()->removeItem(removeIndex);

			addDialogue("...!");
			addDialogue("I'm in your debt!");
			addDialogue("Take this, you might need it.");
			addDialogue("Maybe we'll meet again, stranger.");

			break;
		}
	}
}

void InjuredExplorer::checkSatisfaction() {
	return;
}
void InjuredExplorer::reward() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_dungeon->itemObject(px, py))
		m_dungeon->itemHash(px, py);

	m_dungeon->createItem(rollPassive(m_dungeon, px, py, Rarity::RARE, true));
}
void InjuredExplorer::addInteractedDialogue() {
	m_dialogue.clear();
	m_promptChoices.clear();
	m_promptStage = 1;

	addDialogue("You came back?");
	addDialogue(NPC_PROMPT);

	if (playerHasHealingItem()) {
		addChoice("I do have something.");
		m_promptStage = 2;
	}

	addChoice("Leave.");
}
void InjuredExplorer::addSatisfiedDialogue() {
	m_dialogue.clear();

	addDialogue("");
}
void InjuredExplorer::addFinalDialogue() {
	m_dialogue.clear();

	addDialogue("I'll hang out here until things get a little better.");
}
bool InjuredExplorer::playerHasHealingItem() const {
	for (unsigned int i = 0; i < m_dungeon->getPlayer()->itemCount(); i++) {
		if (m_dungeon->getPlayer()->itemAt(i)->isHealingItem())
			return true;
	}

	return false;
}

//		SHOPKEEPER
Shopkeeper::Shopkeeper(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, SHOPKEEPER, "Shopkeeper_48x48.png") {
	setSprite(dungeon->createSprite(x, y - 1, 1, getImageName())); // y - 1 because shopkeeper is behind the counter

	addDialogue("Huh. Hello.");
	addDialogue("Wasn't expecting to see someone like you down here.");
	addDialogue("Don't get too comfy. Things are not what they seem.");
	addDialogue("I've sealed the doors off so no monsters can get in.");
	addDialogue("Please, have a look around.");

	setInteractionLimit(5);
}

void Shopkeeper::checkSatisfaction() {
	return;
}
void Shopkeeper::reward() {
	return;
}

void Shopkeeper::addInteractedDialogue() {
	m_dialogue.clear();

	switch (getInteractionStage()) {
	case 1:
		addDialogue("This shop was originally set up by my great grandfather and a close friend.");
		addDialogue("Of course, my own father took over, and now I.");
		break;
	case 2: addDialogue("It's not often I get to speak with another person these days."); break;
	case 3: addDialogue("The shop's not in its best condition, but it don't need to be."); break;
	case 4:
		addDialogue("My father is still alive. Although he's too old to keep running the place.");
		addDialogue("'sides, I can manage.");
		break;
	case 5:
		switch (randInt(8)) {
		case 0: addDialogue("Still looking?"); break;
		case 1: addDialogue("Back there? Just some extra supplies."); break;
		case 2: addDialogue("I won't entertain any offers. Everything's already discounted."); break;
		case 3: 
			addDialogue("It gets lonely, but eventually you get used to it.");
			addDialogue("Just like everything else.");
			break;
		case 4: addDialogue("I certainly ain't in a rush."); break;
		case 5: addDialogue("No, nothing's changed in the last 20 seconds."); break;
		case 6:
			addDialogue("I was a local champion at darts.");
			addDialogue("Never did get me anywhere.");
			break;
		case 7: addDialogue("Hard choices to make huh? I get it."); break;
		}
		break;
	}

	// if npc still has new things to say, increment interaction stage
	if (getInteractionStage() < getInteractionLimit())
		incInteractionStage();
}
void Shopkeeper::addSatisfiedDialogue() {
	return;
}
void Shopkeeper::addFinalDialogue() {
	return;
}

//		BLACKSMITH
Blacksmith::Blacksmith(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, BLACKSMITH, "Shopkeeper_48x48.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Blacksmith::useResponse(int index) {
	m_dialogue.clear();

	switch (m_promptStage) {
	case 1: {

		switch (index) {
			// Improve Weapon
		case 0: {
			improveWeapon();
			break;
		}
			// Buy something
		case 1: {
			buyItem();
			break;
		}
			// Mingle
		case 2: {
			mingle();
			break;
		}
			// Walk away
		case 3: {
			addDialogue("Good luck out there.");
			break;
		}
		}
		break;
	}
	case 2:	
		break;
	}
}
void Blacksmith::improveWeapon() {

	// Can't improve your weapon if you don't have one
	if (m_dungeon->getPlayer()->getWeapon()->getName() == HANDS) {
		addDialogue("You okay? You don't have anything for me to work with.");

		return;
	}

	if (m_dungeon->getPlayer()->getMoney() >= m_improveCost) {
		addDialogue("You got it.");

		playHitSmasher();
		m_dungeon->getPlayer()->getWeapon()->increaseSharpnessBy(20);
		m_dungeon->getPlayer()->setMoney(m_dungeon->getPlayer()->getMoney() - m_improveCost);
	}
	else {
		playCrowSound();
		addDialogue("Looks like your pockets are a little light.");
	}
}
void Blacksmith::buyItem() {
	addDialogue("Nothing for sale this time.");
}
void Blacksmith::mingle() {
	addDialogue("What's it look like? Workin.");
}

void Blacksmith::checkSatisfaction() {
	return;
}
void Blacksmith::reward() {
	return;
}

void Blacksmith::addInitialDialogue() {
	// 'arcan' is supposed to be a term for those that enter the ruins looking for the magic.
	addDialogue("Greetings arcan.");
	addDialogue("Weapon need a little sharpening?");
	addDialogue(NPC_PROMPT);

	// Choices for the player when prompted
	int playerWeaponSharpness = m_dungeon->getPlayer()->getWeapon()->getSharpness();
	m_improveCost = (playerWeaponSharpness + 1) * 60;

	addChoice("Can you improve this weapon? (" + std::to_string(m_improveCost) + " Gold)");
	addChoice("What do you have for sale?");
	addChoice("What's new?");
	addChoice("Just passing by.");

	setInteractionLimit(5);
}
void Blacksmith::addInteractedDialogue() {
	m_dialogue.clear();
	m_promptChoices.clear();

	// 'arcan' is supposed to be a term for those that enter the ruins looking for the magic.
	addDialogue("Back again?");
	addDialogue(NPC_PROMPT);

	// Choices for the player when prompted
	int playerWeaponSharpness = m_dungeon->getPlayer()->getWeapon()->getSharpness();
	m_improveCost = (playerWeaponSharpness + 1) * 40;

	addChoice("Sharpen this weapon please. (" + std::to_string(m_improveCost) + " Gold)");
	addChoice("What do you have for sale?");
	addChoice("Any stories?");
	addChoice("Sorry to bother.");
}
void Blacksmith::addSatisfiedDialogue() {
	m_dialogue.clear();
	return;
}
void Blacksmith::addFinalDialogue() {
	return;
}

//		ENCHANTER
Enchanter::Enchanter(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, ENCHANTER, "Shopkeeper_48x48.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Enchanter::useResponse(int index) {
	m_dialogue.clear();
	m_promptChoices.clear();

	switch (m_promptStage) {
		// Choice menu
	case 1: {

		switch (index) {
			// Pick Imbuement
		case 0: {
			imbueWeapon();
			break;
		}
			// Buy something
		case 1: {
			buyItem();
			break;
		}
			// Mingle
		case 2: {
			mingle();
			break;
		}
			// Walk away
		case 3: {
			addDialogue("We shall meet again.");
			break;
		}
		}
		break;
	}
		// Imbuing weapon
	case 2:
		switch (index) {
			// Burning
		case 0: {
			int cost = determineCost(ImbuementType::BURNING);
			if (m_dungeon->getPlayer()->getMoney() >= cost) {
				playHitSmasher();
				addDialogue("Right away.");

				m_dungeon->getPlayer()->getWeapon()->setImbuement(ImbuementType::BURNING, 10);
				m_dungeon->getPlayer()->setMoney(m_dungeon->getPlayer()->getMoney() - cost);
			}
			else {
				addDialogue("What, you don't have enough!");
			}
			
			break;
		}
			// Poison
		case 1: {
			int cost = determineCost(ImbuementType::POISONING);
			if (m_dungeon->getPlayer()->getMoney() >= cost) {
				playHitSmasher();
				addDialogue("Absolutely.");

				m_dungeon->getPlayer()->getWeapon()->setImbuement(ImbuementType::POISONING, 10);
				m_dungeon->getPlayer()->setMoney(m_dungeon->getPlayer()->getMoney() - cost);
			}
			else {
				addDialogue("Come back when you have enough money.");
			}

			break;
		}
			// Freeze
		case 2: {
			int cost = determineCost(ImbuementType::FREEZING);
			if (m_dungeon->getPlayer()->getMoney() >= cost) {
				playHitSmasher();
				addDialogue("Good choice.");

				m_dungeon->getPlayer()->getWeapon()->setImbuement(ImbuementType::FREEZING, 10);
				m_dungeon->getPlayer()->setMoney(m_dungeon->getPlayer()->getMoney() - cost);
			}
			else {
				addDialogue("If you want the best, you need to pay up.");
			}

			break;
		}
			// Nevermind
		case 3: {
			addDialogue("Make up your mind.");
		}
		}

		m_promptStage = 1;
		break;
	}
}
void Enchanter::imbueWeapon() {
	addDialogue("Certainly, if you have the coin.");
	addDialogue(NPC_PROMPT);

	int igniteCost = determineCost(ImbuementType::BURNING);
	addChoice("Imbue fire. (" + std::to_string(igniteCost) + " Gold)");

	int poisonCost = determineCost(ImbuementType::POISONING);
	addChoice("Imbue poison. (" + std::to_string(poisonCost) + " Gold)");

	int freezeCost = determineCost(ImbuementType::FREEZING);
	addChoice("Imbue freeze. (" + std::to_string(freezeCost) + " Gold)");

	addChoice("Nevermind.");
		
	m_promptStage = 2;
}
int Enchanter::determineCost(ImbuementType type) {
	int imbuementLevel = m_dungeon->getPlayer()->getWeapon()->getImbuementLevel(type);

	// Weapon cannot be imbued past 50% chance
	if (imbuementLevel == 50)
		return -1;

	imbuementLevel /= 10;

	// If weapon has not been imbued with this type yet, then the price is slightly higher
	if (imbuementLevel == 0) {
		return 100;
	}

	switch (type) {
	case ImbuementType::BURNING:
		return imbuementLevel * 40;
	case ImbuementType::POISONING:
		return imbuementLevel * 45;
	case ImbuementType::FREEZING:
		return imbuementLevel * 50;
	default:
		return 0;
	}
}
void Enchanter::buyItem() {

}
void Enchanter::mingle() {

}

void Enchanter::checkSatisfaction() {
	return;
}
void Enchanter::reward() {

}

void Enchanter::addInitialDialogue() {
	addDialogue("Welcome traveler...");
	addDialogue("Are you in need of my services?");
	addDialogue(NPC_PROMPT);

	addChoice("Enchant my weapon.");
	addChoice("What do you have for sale?");
	addChoice("Any news?");
	addChoice("Goodbye.");

	setInteractionLimit(5);
}
void Enchanter::addInteractedDialogue() {
	m_dialogue.clear();
	m_promptChoices.clear();

	addDialogue("I knew you'd be back.");
	addDialogue(NPC_PROMPT);

	addChoice("Enchant my weapon.");
	addChoice("Let me see what you have.");
	addChoice("How's business?");
	addChoice("Later.");

	setInteractionLimit(5);
}
void Enchanter::addSatisfiedDialogue() {
	m_dialogue.clear();
	return;
}
void Enchanter::addFinalDialogue() {
	m_dialogue.clear();
	m_promptChoices.clear();

	
}


//		TRADER
Trader::Trader(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, TRADER, "Spellcaster_48x48.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Trader::useResponse(int index) {
	m_dialogue.clear();
	m_promptChoices.clear();

	switch (m_promptStage) {
		// Choice menu
	case 1: {

		switch (index) {
			// Start trade
		case 0: {
			startTrade();
			break;
		}
		//	// Mingle
		//case 1: {
		//	mingle();
		//	break;
		//}
			// Walk away
		case 1: {
			addDialogue("Tread carefully arcan.");
			break;
		}
		}
		break;
	}
		// Trade menu
	case 2:
		switch (index) {
		case 0:
		case 1:
		case 2:
			makeTrade(index);
			break;
		default:
			addDialogue("You'll come around.");
			m_promptStage = 1;
			break;
		}

		break;
	}
}
void Trader::startTrade() {
	m_dialogue.clear();
	m_promptChoices.clear();

	addDialogue("You're gonna jump at the opportunity!");
	addDialogue(NPC_PROMPT);

	addChoice("Trade Passive.");
	addChoice("Trade Item.");
	addChoice("Trade Weapon.");

	addChoice("No way, dude.");

	m_promptStage = 2;
}
void Trader::makeTrade(int index) {
	m_dialogue.clear();
	m_promptChoices.clear();

	bool successfulTrade = false;
	switch (index) {
		// Trade Passive
	case 0:
		if (m_passiveTraded) {
			addDialogue("Sorry, this was a one-time deal.");
			addDialogue("I hope you enjoy it. Haha!");
		}
		else if (m_dungeon->getPlayer()->passiveCount() == 0) {
			addDialogue("What kind of fool do you take me for.");
			addDialogue("You've got nothing of the sort.");
		}
		else {
			addDialogue("Haha! You won't regret it.");
			tradePassive();
			successfulTrade = true;
		}
		break;
		// Trade Inventory Item
	case 1:
		if (m_itemTraded) {
			addDialogue("Sorry, this was a one-time deal.");
			addDialogue("I hope you enjoy it. Haha!");
		}
		else if (!m_dungeon->getPlayer()->hasItems()) {
			addDialogue("What kind of fool do you take me for.");
			addDialogue("You've got nothing of the sort.");
		}
		else {
			addDialogue("You know what they say: low-risk, no reward.");
			tradeItem();
			successfulTrade = true;
		}
		break;
		// Trade Weapon
	case 2:
		if (m_weaponTraded) {
			addDialogue("Sorry, this was a one-time deal.");
			addDialogue("I hope you enjoy it. Haha!");
		}
		else if (m_dungeon->getPlayer()->getWeapon()->getName() == HANDS) {
			addDialogue("I would gladly chop your arms off. Hahaha!");
			addDialogue("But I don't think you'd like that.");
		}
		else {
			addDialogue("A risktaker, I like it.");
			tradeWeapon();
			successfulTrade = true;
		}
		break;
	default:
		break;
	}

	// Loop dialogue if no trade was made
	if (!successfulTrade) {
		addDialogue(NPC_PROMPT);

		addChoice("Trade Passive.");
		addChoice("Trade Item.");
		addChoice("Trade Weapon.");

		addChoice("No way, dude.");
	}
}
void Trader::tradePassive() {
	int index = randInt(m_dungeon->getPlayer()->passiveCount());
	m_dungeon->getPlayer()->removePassive(index);

	m_dungeon->getPlayer()->equipPassive(rollPassive(m_dungeon, 0, 0, Rarity::ALL, false));
	
	m_passiveTraded = true;
	m_promptStage = 1;
}
void Trader::tradeItem() {
	int index = randInt(m_dungeon->getPlayer()->itemCount());
	m_dungeon->getPlayer()->removeItem(index);

	m_dungeon->getPlayer()->addItem(rollItem(m_dungeon, 0, 0, Rarity::RARE, true, false));

	m_itemTraded = true;
	m_promptStage = 1;
}
void Trader::tradeWeapon() {
	m_dungeon->getPlayer()->tradeWeapon(rollWeapon(m_dungeon, 0, 0, Rarity::RARE, true));

	m_weaponTraded = true;
	m_promptStage = 1;
}
void Trader::mingle() {

}

void Trader::checkSatisfaction() {
	return;
}
void Trader::reward() {

}

void Trader::addInitialDialogue() {
	addDialogue("You there.");
	addDialogue("I've got just the deal for you.");
	addDialogue(NPC_PROMPT);

	addChoice("What deal?");
	addChoice("No way, dude.");

	setInteractionLimit(5);
}
void Trader::addInteractedDialogue() {
	m_dialogue.clear();
	m_promptChoices.clear();

	addDialogue("They always come back.");
	addDialogue(NPC_PROMPT);

	addChoice("Fine, I'll take a look.");
	addChoice("Still no.");
}
void Trader::addSatisfiedDialogue() {
	m_dialogue.clear();
	return;
}
void Trader::addFinalDialogue() {

}


//		TUTORIAL NPCS
OutsideMan1::OutsideMan1(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, OUTSIDE_MAN1, "Archer_48x48.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	addDialogue("Another sucker.");
	addDialogue("Can't say I'm surprised.");
	addDialogue("You should really turn back. But if you don't, good luck.");
	addDialogue("You're gonna need it.");

	setInteractionLimit(5);
}

void OutsideMan1::addInteractedDialogue() {
	m_dialogue.clear();

	switch (getInteractionStage()) {
	case 1:
		addDialogue("Well, make your choice.");
		break;
	case 2: addDialogue("You don't need me to tell you the story here."); break;
	case 3:
		addDialogue("My story ain't that interesting..");
		addDialogue("But if you must know, I lost my partner to this place.");
		break;
	case 4:
		addDialogue("The strangeness of this place sometimes allows people to survive death.");
		addDialogue("It seems for my partner they weren't so lucky.");
		addDialogue("But I wait anyway.");
		break;
	case 5:
		switch (randInt(8)) {
		case 0: addDialogue("Nothing else to say to you."); break;
		case 1: addDialogue("My name? Of no importance."); break;
		case 2: addDialogue("You want some candy? *laughs*"); break;
		case 3:
			addDialogue("I am now exhausted in more ways than one.");
			break;
		case 4: addDialogue("Get outta my face."); break;
		case 5: addDialogue("You'll learn your lesson."); break;
		case 6:
			addDialogue("*whistles*");
			break;
		case 7: addDialogue("Looks like everyday does bring something new."); break;
		}
		break;
	}

	// if npc still has new things to say, increment interaction stage
	if (getInteractionStage() < getInteractionLimit())
		incInteractionStage();
}

OutsideMan2::OutsideMan2(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, OUTSIDE_MAN2, "OutsideMan2.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	addDialogue("I know it! The next time's going to be it for sure!");
	addDialogue("I'm gonna be rich!");
	addDialogue("You there! You're looking at the guy who's going to win it all!");
	addDialogue(NPC_PROMPT); // Indicates that the npc will prompt the user

	setInteractionLimit(5);

	// Choices for the player when prompted
	addChoice("Winning? What are you..?");
	addChoice("I don't believe you.");
}

void OutsideMan2::addInteractedDialogue() {
	m_dialogue.clear();

	switch (getInteractionStage()) {
	case 1:
		addDialogue("I knew I'd make it!");
		addDialogue("Momma always said I was a winna!");
		break;
	case 2:
		addDialogue("Glory favors the bold!");
		addDialogue("You could learn a thing or two from me!");
		break;
	case 3:
		addDialogue("You want to take part in my fortune?! Toooo bad!");
		break;
	case 4:
		addDialogue("HaHA!");
		addDialogue("HAHAHA!");
		break;
	case 5:
		switch (randInt(5)) {
		case 0: addDialogue("I've gotta start making plans soon! They won't make themselves!"); break;
		case 1: addDialogue("Too bad for my cousin, they weren't so lucky!"); break;
		case 2: addDialogue("Nobody saw this coming, not even me!"); break;
		case 3:
			addDialogue("Maybe you could be next! But not here you won't be!");
			break;
		case 4: addDialogue("No point in sticking around!"); break;
		}
		break;
	}

	// if npc still has new things to say, increment interaction stage
	if (getInteractionStage() < getInteractionLimit())
		incInteractionStage();
}
void OutsideMan2::useResponse(int index) {
	m_dialogue.clear();

	switch (index) {
	case 0:
		addDialogue("YEEEEHAAAWWW!");
		break;
	case 1:
		addDialogue("HaHAHA! More for me!");
		break;
	}

}

OutsideWoman1::OutsideWoman1(Dungeon *dungeon, int x, int y) : NPC(dungeon, x, y, OUTSIDE_WOMAN1, ".png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	addDialogue("");
	addDialogue("");
	addDialogue("");
	addDialogue("");

	setInteractionLimit(1);
}

void OutsideWoman1::addInteractedDialogue() {
	m_dialogue.clear();

	switch (getInteractionStage()) {
	case 1:
		addDialogue("");
		addDialogue("");
		break;
	case 2: addDialogue(""); break;
	case 3: addDialogue(""); break;
	case 4:
		addDialogue("");
		addDialogue("");
		break;
	case 5:
		switch (randInt(8)) {
		case 0: addDialogue(""); break;
		case 1: addDialogue(""); break;
		case 2: addDialogue(""); break;
		case 3:
			addDialogue("");
			addDialogue("");
			break;
		case 4: addDialogue(""); break;
		case 5: addDialogue(""); break;
		case 6:
			addDialogue("");
			addDialogue("");
			break;
		case 7: addDialogue(""); break;
		}
		break;
	}

	// if npc still has new things to say, increment interaction stage
	if (getInteractionStage() < getInteractionLimit())
		incInteractionStage();
}



// =============================================
//				:::: MONSTERS ::::
// =============================================
Monster::Monster(Dungeon *dungeon, int x, int y, int hp, int armor, int str, int dex, std::string name)
	: Actors(x, y, hp, armor, str, dex, name) {
	setMonsterFlag(true);
	m_dungeon = dungeon;
	m_damageType = DamageType::NORMAL;

	(*dungeon)[y*dungeon->getCols() + x].enemy = true;
}
Monster::Monster(Dungeon *dungeon, int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name)
	: Actors(x, y, hp, armor, str, dex, wep, name) {
	setMonsterFlag(true);
	m_dungeon = dungeon;
	m_damageType = DamageType::NORMAL;

	(*dungeon)[y*dungeon->getCols() + x].enemy = true;
}
Monster::~Monster() {

}

void Monster::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].enemy = false;
	(*m_dungeon)[y*cols + x].enemy = true;

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);

	getSprite()->setLocalZOrder(y + Z_ACTOR);
}
void Monster::moveCheck() {
	if (isDead() || m_destroyed || isStunned() || isFrozen())
		return;

	if (isPossessed()) {

		char move;
		switch (randInt(4)) {
		case 0: move = 'r'; break;
		case 1: move = 'l'; break;
		case 2: move = 'd'; break;
		case 3: move = 'u'; break;
		}

		attemptMove(move);
		return;
	}

	if (m_chases) {

		// If they fell for a decoy, then attempt a greedy chase toward the decoy
		int x, y;
		if (m_dungeon->checkDecoys(getPosX(), getPosY(), x, y)) {
			attemptGreedyChase(true, x, y);
			return;
		}

		// If player is invisible and there are no decoys, then do nothing
		if (m_dungeon->getPlayer()->isInvisible())
			return;
	}

	move();
};
void Monster::attack(Player &p) {
	int mx = getPosX();
	int my = getPosY();

	int px = p.getPosX();
	int py = p.getPosY();

	int damage;
	int monsterPoints = getDex();
	int playerPoints = p.getDex() + p.getWeapon()->getDexBonus();

	// if the player successfully shielded the attack, increase the player's damage reduction and check if
	// monster is stunnable and set their stun status
	if (p.canBlock() && p.didBlock(mx, my)) {
		if (m_hasWeapon)
			damage = std::max(0, 1 + randInt(getStr() + getWeapon()->getDamage()) - (p.getArmor() + p.blockedDamageReduction()));
		else
			damage = std::max(0, 1 + randInt(getStr()) - (p.getArmor() + p.blockedDamageReduction()));

		if (p.activeHasAbility())
			p.useActiveAbility(*this);	

		// handles shield durability change
		p.successfulBlock();

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}

		return;
	}

	// Monster's attack roll is successful
	if ((monsterPoints > 0 ? randInt(monsterPoints) : 0) >= (playerPoints > 0 ? randInt(playerPoints) : 0)) {

		if (m_hasWeapon)
			damage = std::max(1, getWeapon()->getDamage() + randInt(getStr()) - p.getArmor());
		else
			damage = std::max(1, 1 + randInt(std::max(1, getStr())) - p.getArmor());

		damagePlayer(damage, m_damageType);
		
		// Extra effects that can occur after a successful attack
		extraAttackEffects();
	}
	// else attack roll was unsuccessful
	else {
		;
	}
}
void Monster::damagePlayer(int damage, DamageType type) {
	m_dungeon->damagePlayer(damage, type);
	m_dungeon->getPlayer()->reactToDamage(*this);
}
void Monster::death() {
	int cols = m_dungeon->getCols();

	int x = getPosX();
	int y = getPosY();

	(*m_dungeon)[y*cols + x].enemy = false;
	spriteCleanup();

	if (emitsLight())
		m_dungeon->removeLightSource(x, y, getName());

	extraDeathEffects();

	// If they were destroyed, then don't drop anything
	if (m_destroyed)
		return;

	if (m_gold > 0) {
		int px = m_dungeon->getPlayer()->getPosX();
		int py = m_dungeon->getPlayer()->getPosY();
		playGoldDropSound(m_gold, px, py, x, y);

		m_dungeon->addGold(x, y, m_gold * m_dungeon->getPlayer()->getMoneyMultiplier() + m_dungeon->getPlayer()->getMoneyBonus());
	}

	// If Spelunker, then all monsters have a chance to drop a Rock on death
	if (m_dungeon->getPlayer()->getName() == SPELUNKER) {
		if (1 + randInt(100) + m_dungeon->getPlayer()->getLuck() > 90) {

			if (m_dungeon->itemObject(x, y))
				m_dungeon->itemHash(x, y);
			
			(*m_dungeon)[y*cols + x].object = std::make_shared<Rocks>(x, y);
			(*m_dungeon)[y*cols + x].object->setSprite(m_dungeon->createSprite(x, y, -1, (*m_dungeon)[y*cols + x].object->getImageName()));
			(*m_dungeon)[y*cols + x].item = true;
			m_dungeon->addItem((*m_dungeon)[y*cols + x].object);
		}
	}

	if (m_dungeon->getPlayer()->hasScavenger()) {
		
		if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 95) {

			if (m_dungeon->itemObject(x, y))
				m_dungeon->itemHash(x, y);

			std::shared_ptr<Objects> item = rollItem(m_dungeon, x, y, Rarity::COMMON, true);
			item.reset();
		}
	}

	// Check if monster does anything else after death
	deathDrops();
}
void Monster::extraDeathEffects() {
	if (findAffliction(INCENDIARY) != -1) {
		m_dungeon->addTrap(std::make_shared<ActiveFireBomb>(*m_dungeon, getPosX(), getPosY(), 1));
	}
}
void Monster::spriteCleanup() {
	m_dungeon->queueRemoveSprite(getSprite());
}

bool Monster::chase(char &best, bool all, bool diagonalsOnly) {
	int x = getPosX();
	int y = getPosY();

	Coords start(x, y);
	Coords end(m_dungeon->getPlayer()->getPosX(), m_dungeon->getPlayer()->getPosY());

	std::priority_queue<std::pair<int, Coords>> pq;
	std::vector<Coords> segments;

	pq.push(std::make_pair(0, start));

	// Maps ending points to their origin points (i.e. left Coord cameFrom right Coord)
	std::unordered_map<Coords, Coords> cameFrom;
	Coords blank; // empty
	cameFrom.insert(std::make_pair(start, blank));

	// Maps Coords to a cost, which is calculated
	// based on how far that Coord is from the end Coord.
	std::unordered_map<Coords, int> costSoFar;
	costSoFar.insert(std::make_pair(start, 0));

	Coords current;
	int cost, priority;
	
	int shortest = 0;
	int currentRange = 0;
	bool firstDiagonalMove = true; // Used only when diagonalsOnly is true

	// _____ Begin A* algorithm _____
	while (!pq.empty()) {
		current = pq.top().second;
		pq.pop();

		if (current == end)
			break;

		if (!diagonalsOnly || !firstDiagonalMove)
			getCoordsAdjacentTo(*m_dungeon, segments, current.x, current.y, all);
		else {
			getDiagonalCoordsAdjacentTo(*m_dungeon, segments, current.x, current.y);
			firstDiagonalMove = false;
		}

		for (int i = 0; i < (int)segments.size(); i++) {
			/* Calculate the cost of this path:
			* cost = cost of the existing path +
			*			the cost of the path we're currently presented with */
			cost = 0;
			auto existingCost = costSoFar.find(current);
			
			if (existingCost != costSoFar.end())
				cost = existingCost->second;
			cost += calculateDistanceBetween(current, segments[i]);

			auto currentCost = costSoFar.find(segments[i]);

			if (currentCost == costSoFar.end() || cost < currentCost->second) {
				if (currentCost == costSoFar.end())
					costSoFar.insert(std::make_pair(segments[i], cost));
				else
					currentCost->second = cost;

				priority = -1 * (cost + calculateDistanceBetween(segments[i], end)); // Multiplying by -1 makes this a min. priority queue

				if (m_dungeon->enemy(segments[i].x, segments[i].y))
					priority--; // Helps monsters to prioritize routes without enemies in the way to prevent them from not moving sometimes

				pq.push(std::make_pair(priority, segments[i]));

				auto it = cameFrom.find(segments[i]);
				if (it == cameFrom.end())
					cameFrom.insert(std::make_pair(segments[i], current));				
				else
					it->second = current;
			}
			else if (currentCost != costSoFar.end() && cost == currentCost->second) {

				// This allows the monster to choose paths nondeterministically
				if (randReal(1, 100) > 50) {
					auto it = cameFrom.find(segments[i]);
					if (it == cameFrom.end())
						cameFrom.insert(std::make_pair(segments[i], current));
					else
						it->second = current;
				}
			}
		}
	}
	// _____ End A* algorithm _____

	if (current != end)
		return false;

	Coords currentEnd = end, newEnd;

	while (newEnd != start) {
		newEnd = cameFrom.find(currentEnd)->second;

		if (newEnd == start)
			break;

		currentEnd = newEnd;
	}

	int n = currentEnd.x;
	int m = currentEnd.y;

	if (x - 1 == n && y == m)
		best = 'l';
	else if (x + 1 == n && y == m)
		best = 'r';
	else if (x == n && y - 1 == m)
		best = 'u';
	else if (x == n && y + 1 == m)
		best = 'd';
	else if (x - 1 == n && y - 1 == m)
		best = '1';
	else if (x + 1 == n && y - 1 == m)
		best = '2';
	else if (x - 1 == n && y + 1 == m)
		best = '3';
	else
		best = '4';

	return true;
}
bool Monster::attemptChase(Dungeon &dungeon, int &shortest, int currentDist, int origdist, int x, int y, char &first_move, char &optimal_move) {
	int cols = dungeon.getCols();

	// if enemy is immediately adjacent to the player, return to attack
	if (currentDist == origdist && playerIsAdjacent()) {

		if (optimal_move == '0')
			optimal_move = getFacingDirectionRelativeTo(getPosX(), getPosY(), m_dungeon->getPlayer()->getPosX(), m_dungeon->getPlayer()->getPosY());	

		dungeon[y*cols + x].marked = false;
		return true;
	}

	// mark the current tile as visited
	dungeon[y*cols + x].marked = true;

	// player was not found within origdist # of steps OR took more steps than the current shortest path found
	if (currentDist < 0 || currentDist < shortest) {
		dungeon[y*cols + x].marked = false;
		return false;
	}

	if (dungeon.hero(x, y)) {
		if (currentDist > shortest) {
			shortest = currentDist;
			optimal_move = first_move;
		}
		else if (currentDist == shortest) {
			// randomly select optimal path if multiple optimal paths found
			if (randInt(2) == 1)
				optimal_move = first_move;
		}
		dungeon[y*cols + x].marked = false;
		return true;
	}

	if (!dungeon.marked(x + 1, y) && (!dungeon.wall(x + 1, y) || dungeon.hero(x + 1, y))) {
		if (currentDist == origdist)
			first_move = 'r';
		attemptChase(dungeon, shortest, currentDist - 1, origdist, x + 1, y, first_move, optimal_move);
	}

	if (!dungeon.marked(x - 1, y) && (!dungeon.wall(x - 1, y) || dungeon.hero(x - 1, y))) {
		if (currentDist == origdist)
			first_move = 'l';
		attemptChase(dungeon, shortest, currentDist - 1, origdist, x - 1, y, first_move, optimal_move);
	}

	if (!dungeon.marked(x, y + 1) && (!dungeon.wall(x, y + 1) || dungeon.hero(x, y + 1))) {
		if (currentDist == origdist)
			first_move = 'd';
		attemptChase(dungeon, shortest, currentDist - 1, origdist, x, y + 1, first_move, optimal_move);
	}

	if (!dungeon.marked(x, y - 1) && (!dungeon.wall(x, y - 1) || dungeon.hero(x, y - 1))) {
		if (currentDist == origdist)
			first_move = 'u';
		attemptChase(dungeon, shortest, currentDist - 1, origdist, x, y - 1, first_move, optimal_move);
	}

	// unmark the tile as visited when backtracking
	dungeon[y*cols + x].marked = false;

	if (optimal_move != '0')
		return true;

	return false;
}
bool Monster::attemptAllChase(Dungeon &dungeon, int &shortest, int currentDist, int origDist, int x, int y, char &first_move, char &optimal_move) {
	int cols = dungeon.getCols();

	// if enemy is immediately adjacent to the player, return to attack
	if (currentDist == origDist && first_move != '1' && first_move != '2' && first_move != '3' && first_move != '4' &&
		playerIsAdjacent(false)) {

		if (optimal_move == '0')
			optimal_move = getFacingDirectionRelativeTo(getPosX(), getPosY(), m_dungeon->getPlayer()->getPosX(), m_dungeon->getPlayer()->getPosY());

		dungeon[y*cols + x].marked = false;
		return true;
	}
	else if (currentDist == origDist && first_move != 'l' && first_move != 'r' && first_move != 'u' && first_move != 'd' &&
		playerIsAdjacent(true)) {

		if (optimal_move == '0')
			optimal_move = getFacingDirectionRelativeTo(getPosX(), getPosY(), m_dungeon->getPlayer()->getPosX(), m_dungeon->getPlayer()->getPosY());

		dungeon[y*cols + x].marked = false;
		return true;
	}

	// mark the current tile as visited
	dungeon[y*cols + x].marked = true;

	// player was not found within origdist # of steps OR took more steps than the current shortest path found
	if (currentDist < 0 || currentDist < shortest) {
		dungeon[y*cols + x].marked = false;
		return false;
	}

	// if player is found, tell monster to advance
	if (dungeon.hero(x, y)) {
		if (currentDist > shortest) {
			shortest = currentDist;
			optimal_move = first_move;
		}
		else if (currentDist == shortest) {
			// randomly select optimal path if multiple optimal paths found
			if (randInt(2) == 1)
				optimal_move = first_move;
		}
		dungeon[y*cols + x].marked = false;
		return true;
	}

	// Diagonal checking
	if (!dungeon.marked(x + 1, y + 1) && (!dungeon.wall(x + 1, y + 1) || dungeon.hero(x + 1, y + 1))) {
		if (currentDist == origDist)
			first_move = '4';
		attemptAllChase(dungeon, shortest, currentDist - 1, origDist, x + 1, y + 1, first_move, optimal_move);
	}

	if (!dungeon.marked(x - 1, y + 1) && (!dungeon.wall(x - 1, y + 1) || dungeon.hero(x - 1, y + 1))) {
		if (currentDist == origDist)
			first_move = '3';
		attemptAllChase(dungeon, shortest, currentDist - 1, origDist, x - 1, y + 1, first_move, optimal_move);
	}

	if (!dungeon.marked(x + 1, y - 1) && (!dungeon.wall(x + 1, y - 1) || dungeon.hero(x + 1, y - 1))) {
		if (currentDist == origDist)
			first_move = '2';
		attemptAllChase(dungeon, shortest, currentDist - 1, origDist, x + 1, y - 1, first_move, optimal_move);
	}

	if (!dungeon.marked(x - 1, y - 1) && (!dungeon.wall(x - 1, y - 1) || dungeon.hero(x - 1, y - 1))) {
		if (currentDist == origDist)
			first_move = '1';
		attemptAllChase(dungeon, shortest, currentDist - 1, origDist, x - 1, y - 1, first_move, optimal_move);
	}

	// Normal checking
	if (!dungeon.marked(x + 1, y) && (!dungeon.wall(x + 1, y) || dungeon.hero(x + 1, y))) {
		if (currentDist == origDist)
			first_move = 'r';
		attemptAllChase(dungeon, shortest, currentDist - 1, origDist, x + 1, y, first_move, optimal_move);
	}

	if (!dungeon.marked(x - 1, y) && (!dungeon.wall(x - 1, y) || dungeon.hero(x - 1, y))) {
		if (currentDist == origDist)
			first_move = 'l';
		attemptAllChase(dungeon, shortest, currentDist - 1, origDist, x - 1, y, first_move, optimal_move);
	}

	if (!dungeon.marked(x, y + 1) && (!dungeon.wall(x, y + 1) || dungeon.hero(x, y + 1))) {
		if (currentDist == origDist)
			first_move = 'd';
		attemptAllChase(dungeon, shortest, currentDist - 1, origDist, x, y + 1, first_move, optimal_move);
	}

	if (!dungeon.marked(x, y - 1) && (!dungeon.wall(x, y - 1) || dungeon.hero(x, y - 1))) {
		if (currentDist == origDist)
			first_move = 'u';
		attemptAllChase(dungeon, shortest, currentDist - 1, origDist, x, y - 1, first_move, optimal_move);
	}

	// Unmark the tile as visited when backtracking
	dungeon[y*cols + x].marked = false;

	if (optimal_move != '0')
		return true;
	return false;
}
void Monster::attemptGreedyChase(bool diagonals, int x, int y) {
	int sx, sy, ex, ey, xDiff, yDiff;
	sx = getPosX();
	sy = getPosY();
	ex = (x == -1 ? m_dungeon->getPlayer()->getPosX() : x);
	ey = (y == -1 ? m_dungeon->getPlayer()->getPosY() : y);

	xDiff = sx - ex;
	yDiff = sy - ey;

	// Same column
	if (xDiff == 0) {
		// Positive y difference means the enemy is below the player
		if (yDiff > 0)
			attemptMove('u'); // move upward		
		else 
			attemptMove('d'); // move downward		
	}
	// Same row
	else if (yDiff == 0) {
		// Negative x difference means the enemy is left of the player
		if (xDiff < 0)
			attemptMove('r'); // move right		
		else 
			attemptMove('l'); // move left		
	}
	else {
		char move;
		// Player is southeast of the monster
		if (xDiff < 0 && yDiff < 0) {
			if (diagonals) {
				if (attemptMove('4'))
					return;		
			}
			
			move = (randInt(2) == 0 ? 'd' : 'r');
			if (!attemptMove(move)) {
				move = (move == 'd' ? 'r' : 'd');
				attemptMove(move);
			}
			
		}
		// Player is northeast of the monster
		else if (xDiff < 0 && yDiff > 0) {
			if (diagonals) {
				if (attemptMove('2'))
					return;			
			}
			
			move = (randInt(2) == 0 ? 'u' : 'r');
			if (!attemptMove(move)) {
				move = (move == 'u' ? 'r' : 'u');
				attemptMove(move);
			}
			
		}
		// Player is southwest of the monster
		else if (xDiff > 0 && yDiff < 0) {
			if (diagonals) {
				if (attemptMove('3'))
					return;
			}

			move = (randInt(2) == 0 ? 'd' : 'l');
			if (!attemptMove(move)) {
				move = (move == 'd' ? 'l' : 'd');
				attemptMove(move);
			}
		}
		// Player is northwest of the monster
		else /*(xDiff > 0 && yDiff > 0)*/ {
			if (diagonals) {
				if (attemptMove('1'))
					return;
			}

			move = (randInt(2) == 0 ? 'u' : 'l');
			if (!attemptMove(move)) {
				move = (move == 'u' ? 'l' : 'u');
				attemptMove(move);
			}
		}

	}
}
void Monster::attemptDiagonalGreedyChase() {
	int sx, sy, ex, ey, xDiff, yDiff;
	sx = getPosX();
	sy = getPosY();
	ex = m_dungeon->getPlayer()->getPosX();
	ey = m_dungeon->getPlayer()->getPosY();

	xDiff = sx - ex;
	yDiff = sy - ey;

	// Same column
	if (xDiff == 0) {
		// Positive y difference means the enemy is below the player
		if (yDiff > 0)
			attemptMove('u'); // move upward		
		else
			attemptMove('d'); // move downward		
	}
	// Same row
	else if (yDiff == 0) {
		// Negative x difference means the enemy is left of the player
		if (xDiff < 0)
			attemptMove('r'); // move right		
		else
			attemptMove('l'); // move left		
	}
	else {
		char move;
		// Player is southeast of the monster
		if (xDiff < 0 && yDiff < 0) {
			if (attemptMove('4'))
				return;

			move = (randInt(2) == 0 ? 'd' : 'r');
			if (!attemptMove(move)) {
				move = (move == 'd' ? 'r' : 'd');
				attemptMove(move);
			}
		}
		// Player is northeast of the monster
		else if (xDiff < 0 && yDiff > 0) {		
			if (attemptMove('2'))
				return;		

			move = (randInt(2) == 0 ? 'u' : 'r');
			if (!attemptMove(move)) {
				move = (move == 'u' ? 'r' : 'u');
				attemptMove(move);
			}
		}
		// Player is southwest of the monster
		else if (xDiff > 0 && yDiff < 0) {
			if (attemptMove('3'))
				return;
			
			move = (randInt(2) == 0 ? 'd' : 'l');
			if (!attemptMove(move)) {
				move = (move == 'd' ? 'l' : 'd');
				attemptMove(move);
			}
		}
		// Player is northwest of the monster
		else /*(xDiff > 0 && yDiff > 0)*/ {			
				if (attemptMove('1'))
					return;
			
			move = (randInt(2) == 0 ? 'u' : 'l');
			if (!attemptMove(move)) {
				move = (move == 'u' ? 'l' : 'u');
				attemptMove(move);
			}
		}
	}
}
char Monster::attemptDryRunGreedyChase(bool diagonals) {
	int sx, sy, ex, ey, xDiff, yDiff;
	sx = getPosX();
	sy = getPosY();
	ex = m_dungeon->getPlayer()->getPosX();
	ey = m_dungeon->getPlayer()->getPosY();

	xDiff = sx - ex;
	yDiff = sy - ey;

	// Same column
	if (xDiff == 0) {
		// Positive y difference means the enemy is below the player
		if (yDiff > 0) {
			attemptDryRunMove('u');
			return 'u';
		}
		else {
			attemptDryRunMove('d');
			return 'd';
		}
	}
	// Same row
	else if (yDiff == 0) {
		// Negative x difference means the enemy is left of the player
		if (xDiff < 0) {
			attemptDryRunMove('r');
			return 'r';
		}
		else {
			attemptDryRunMove('l');
			return 'l';
		}
	}
	else {
		char move;
		// Player is southeast of the monster
		if (xDiff < 0 && yDiff < 0) {
			if (diagonals) {
				if (attemptDryRunMove('4'))
					return '4';
			}

			move = (randInt(2) == 0 ? 'd' : 'r');
			if (!attemptDryRunMove(move)) {
				move = (move == 'd' ? 'r' : 'd');
				attemptDryRunMove(move);			
			}

			return move;
		}
		// Player is northeast of the monster
		else if (xDiff < 0 && yDiff > 0) {
			if (diagonals) {
				if (attemptDryRunMove('2'))
					return '2';
			}

			move = (randInt(2) == 0 ? 'u' : 'r');
			if (!attemptDryRunMove(move)) {
				move = (move == 'u' ? 'r' : 'u');
				attemptDryRunMove(move);				
			}

			return move;
		}
		// Player is southwest of the monster
		else if (xDiff > 0 && yDiff < 0) {
			if (diagonals) {
				if (attemptDryRunMove('3'))
					return '3';
			}

			move = (randInt(2) == 0 ? 'd' : 'l');
			if (!attemptDryRunMove(move)) {
				move = (move == 'd' ? 'l' : 'd');
				attemptDryRunMove(move);
			}

			return move;
		}
		// Player is northwest of the monster
		else /*(xDiff > 0 && yDiff > 0)*/ {
			if (diagonals) {
				if (attemptDryRunMove('1'))
					return '1';
			}

			move = (randInt(2) == 0 ? 'u' : 'l');
			if (!attemptDryRunMove(move)) {
				move = (move == 'u' ? 'l' : 'u');
				attemptDryRunMove(move);
			}

			return move;
		}

	}
}
bool Monster::attemptDryRunMove(char move) {
	int x = getPosX();
	int y = getPosY();

	// Key:
	// 1 u 2
	// l _ r
	// 3 d 4

	int n, m;
	setDirectionalOffsets(move, n, m);

	// If confused, switch their move to the opposite
	if (isConfused()) {
		switch (move) {
		case 'l': n = 1; m = 0; break;
		case 'r': n = -1; m = 0; break;
		case 'u': n = 0; m = 1; break;
		case 'd': n = 0; m = -1; break;
		case '1': n = 1; m = 1; break;
		case '2': n = -1; m = 1; break;
		case '3': n = 1; m = -1; break;
		case '4': n = -1; m = -1; break;
		}
	}

	if (m_dungeon->hero(x + n, y + m))
		return true;
	
	if (m_dungeon->wall(x + n, y + m) && !isEthereal())
		return false;

	if (m_dungeon->trap(x + n, y + m)) {
		std::vector<int> indexes = m_dungeon->findTraps(x + n, y + m);
		for (unsigned int i = 0; i < indexes.size(); i++) {
			if (m_dungeon->trapAt(indexes[i])->isLethal() && m_smart)
				return false;
		}
	}

	if (!m_dungeon->enemy(x + n, y + m) || isSpirit())
		return true;
	
	return false;
}

bool Monster::attemptMove(char move) {
	int x = getPosX();
	int y = getPosY();

	// Key:
	// 1 u 2
	// l _ r
	// 3 d 4

	int n = 0, m = 0;
	setDirectionalOffsets(move, n, m);

	// If confused, switch their move to the opposite
	if (isConfused()) {
		switch (move) {
		case 'l': n = 1; m = 0; break;
		case 'r': n = -1; m = 0; break;
		case 'u': n = 0; m = 1; break;
		case 'd': n = 0; m = -1; break;
		case '1': n = 1; m = 1; break;
		case '2': n = -1; m = 1; break;
		case '3': n = 1; m = -1; break;
		case '4': n = -1; m = -1; break;
		}
	}

	if (m_dungeon->hero(x + n, y + m)) {
		attack(*m_dungeon->getPlayer());
		return true;
	}

	if (m_dungeon->wall(x + n, y + m) && !isEthereal())
		return false;

	if (m_dungeon->trap(x + n, y + m)) {
		std::vector<int> indexes = m_dungeon->findTraps(x + n, y + m);
		for (unsigned int i = 0; i < indexes.size(); i++) {
			if (m_dungeon->trapAt(indexes[i])->isLethal() && m_smart)
				return false;		
		}
	}

	if (!m_dungeon->enemy(x + n, y + m) || isSpirit()) {
		moveTo(x + n, y + m);

		return true;
	}

	return false;
}
char Monster::moveMonsterRandomly(bool diagonals) {
	int x = getPosX();
	int y = getPosY();

	std::vector<std::pair<int, int>> coords;
	coords.push_back(std::make_pair(x + 1, y));
	coords.push_back(std::make_pair(x - 1, y));
	coords.push_back(std::make_pair(x, y - 1));
	coords.push_back(std::make_pair(x, y + 1));

	if (diagonals) {
		coords.push_back(std::make_pair(x + 1, y + 1));
		coords.push_back(std::make_pair(x - 1, y + 1));
		coords.push_back(std::make_pair(x + 1, y - 1));
		coords.push_back(std::make_pair(x - 1, y - 1));
	}

	while (!coords.empty()) {
		int index = randInt((int)coords.size());
		int n = coords[index].first;
		int m = coords[index].second;

		if (!(m_dungeon->wall(n, m) || m_dungeon->enemy(n, m) || m_dungeon->hero(n, m))) {

			if (m_dungeon->trap(n, m) && !isFlying())
				return '-';
			
			moveTo(n, m);
			if (x == n)
				return m == y - 1 ? 'u' : 'd';
			else if (y == m)
				return n == x - 1 ? 'l' : 'r';
			else if (n == x - 1)
				return m == y - 1 ? '1' : '3';
			else /*n == x + 1*/
				return m == y - 1 ? '2' : '4';
		}

		coords.erase(coords.begin() + index);
	}

	return '-';
}

bool Monster::playerInRange(int range) const {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	// if enemy and player are too far separated, then do not attempt to recurse
	if (abs(px - mx) > range || abs(py - my) > range)
		return false;

	int steps = 0;

	while (px != mx) {
		if (px > mx)
			mx++;
		else
			mx--;

		steps++;
	}

	while (py != my) {
		if (py > my)
			my++;
		else
			my--;

		steps++;
	}

	return (steps <= range ? true : false);
}
bool Monster::playerInDiagonalRange(int range) const {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	// if monster is on the same column/row as player, then just do regular range check
	if (px == mx || py == my)
		return playerInRange(range);

	int steps = 0;

	int xMax = std::max(mx, px);
	int xMin = std::min(mx, px);
	int yMax = std::max(my, py);
	int yMin = std::min(my, py);

	// if column difference is greater
	if (xMax - xMin >= yMax - yMin) {
		while (xMax != xMin) {
			xMax--;
			if (yMax != yMin)
				yMax--;

			steps++;
		}
	}
	else if (xMax - xMin < yMax - yMin) {
		while (yMax != yMin) {
			yMax--;
			if (xMax != xMin)
				xMax--;

			steps++;
		}
	}

	return (steps <= range ? true : false);
}
bool Monster::playerIsAdjacent(bool diagonals) const {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	if (diagonals)
		return abs(px - x) <= 1 && abs(py - y) <= 1;

	return abs(px - x) + abs(py - y) == 1;
}


//		BREAKABLES
Breakables::Breakables(Dungeon *dungeon, int x, int y, int hp, int armor, std::string name, std::string image) : Monster(dungeon, x, y, hp, armor, 0, 0, name) {
	setImageName(image);
}
SturdyBreakables::SturdyBreakables(Dungeon *dungeon, int x, int y, int hp, int armor, std::string name, std::string image)
	: Breakables(dungeon, x, y, hp, armor, name, image) {
	setSturdy(true);
}

WeakCrate::WeakCrate(Dungeon *dungeon, int x, int y) : SturdyBreakables(dungeon, x, y, 1, 1, WEAK_CRATE, "Large_Crate0001.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
void WeakCrate::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Large_Crate%04d.png", 7);
	std::function<void(Dungeon&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	m_dungeon->runAnimationWithCallback(frames, 30, x, y, 1, callback, "Large_Crate0007.png");
}

WeakBarrel::WeakBarrel(Dungeon *dungeon, int x, int y) : SturdyBreakables(dungeon, x, y, 1, 1, WEAK_BARREL, "Barrel0001.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
void WeakBarrel::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Barrel%04d.png", 7);
	std::function<void(Dungeon&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	m_dungeon->runAnimationWithCallback(frames, 30, x, y, 1, callback, "Barrel0007.png");
}

SmallPot::SmallPot(Dungeon *dungeon, int x, int y) : SturdyBreakables(dungeon, x, y, 1, 1, WEAK_POT, "Ceramic_Pot0001.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
void SmallPot::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Ceramic_Pot%04d.png", 7);
	std::function<void(Dungeon&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	m_dungeon->runAnimationWithCallback(frames, 30, x, y, 1, callback, "Ceramic_Pot0007.png");
}

LargePot::LargePot(Dungeon *dungeon, int x, int y) : SturdyBreakables(dungeon, x, y, 1, 1, WEAK_LARGE_POT, "Large_Ceramic_Pot0001.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
void LargePot::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Large_Ceramic_Pot%04d.png", 7);
	std::function<void(Dungeon&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	m_dungeon->runAnimationWithCallback(frames, 30, x, y, 1, callback, "Large_Ceramic_Pot0007.png");
}

Sign::Sign(Dungeon *dungeon, int x, int y) : SturdyBreakables(dungeon, x, y, 1, 1, WEAK_SIGN, "Large_Sign0001.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
void Sign::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Large_Sign%04d.png", 6);
	std::function<void(Dungeon&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	m_dungeon->runAnimationWithCallback(frames, 30, x, y, 1, callback, "Large_Sign0006.png");
}

ArrowSign::ArrowSign(Dungeon *dungeon, int x, int y) : SturdyBreakables(dungeon, x, y, 1, 1, WEAK_ARROW_SIGN, "Large_Arrow_Sign0001.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
void ArrowSign::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Large_Arrow_Sign%04d.png", 6);
	std::function<void(Dungeon&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	m_dungeon->runAnimationWithCallback(frames, 30, x, y, 1, callback, "Large_Arrow_Sign0006.png");
}

ExplosiveBarrel::ExplosiveBarrel(Dungeon *dungeon, int x, int y) : Breakables(dungeon, x, y, 1, 1, EXPLOSIVE_BARREL, "Barrel0001.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
void ExplosiveBarrel::deathDrops() {
	int cols = m_dungeon->getCols();
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Barrel%04d.png", 7);
	std::function<void(Dungeon&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	m_dungeon->runAnimationWithCallback(frames, 30, x, y, 1, callback, "Barrel0007.png");

	playSound("Explosion.mp3");

	// explosion animation
	frames = getAnimationFrameVector("Explosion%04d.png", 8);
	m_dungeon->runSingleAnimation(frames, 24, x, y, 2);

	// flash floor tiles
	flashFloor(*m_dungeon, x, y, false);

	int range = 1;

	// If player is caught in the explosion
	if (abs(px - x) <= range && abs(py - y) <= range)
		m_dungeon->damagePlayer(10, DamageType::EXPLOSIVE);

	// Destroy stuff
	for (int i = x - range; i < x + range + 1; i++) {
		for (int j = y - range; j < y + range + 1; j++) {

			if (m_dungeon->withinBounds(i, j)) {

				if (m_dungeon->wall(i, j))
					m_dungeon->destroyWall(i, j);

				if (m_dungeon->gold(i, j) != 0)
					m_dungeon->removeGold(i, j);

				if (m_dungeon->exit(i, j))
					continue;

				if (m_dungeon->trap(i, j)) {
					std::vector<int> indexes = m_dungeon->findTraps(i, j);
					for (int pos = 0; pos < (int)indexes.size(); pos++) {

						if (m_dungeon->trapAt(indexes.at(pos))->isDestructible())
							m_dungeon->trapAt(indexes.at(pos))->destroyTrap();

						else if (m_dungeon->trapAt(indexes.at(pos))->isExplosive())
							m_dungeon->trapAt(indexes.at(pos))->explode();
					}
				}
			}
		}
	}

	// Find any monsters caught in the blast
	int mx, my;
	for (unsigned i = 0; i < m_dungeon->monsterCount(); i++) {
		mx = m_dungeon->monsterAt(i)->getPosX();
		my = m_dungeon->monsterAt(i)->getPosY();

		if (abs(mx - x) <= range && abs(my - y) <= range) {
			int damage = 10 + (m_dungeon->getPlayer()->explosionImmune() ? 10 : 0);
			m_dungeon->damageMonster(i, damage, DamageType::EXPLOSIVE);
		}
	}
}

CharredWood::CharredWood(Dungeon *dungeon, int x, int y) : Breakables(dungeon, x, y, 1, 1, CHARRED_WOOD, "Barrel0001.png") {
	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
void CharredWood::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	char move = m_dungeon->getPlayer()->facingDirection();

	int n = 0, m = 0;

	switch (move) {
	case 'l': n = -1; m = -1; break;
	case 'r': n = 1; m = -1; break;
	case 'u': n = -1; m = -1; break;
	case 'd': n = -1; m = 1; break;
	}

	// 3 rows/columns
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {

			if (m_dungeon->withinBounds(x + n, y + m)) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);

				if (m_dungeon->enemy(x + n, y + m)) {
					int pos = m_dungeon->findMonster(x + n, y + m);
					if (pos != -1) {
						if (m_dungeon->monsterAt(pos)->canBeBurned() || m_dungeon->getPlayer()->hasAfflictionOverride())
							m_dungeon->giveAffliction(pos, std::make_shared<Confusion>(6));
					}
				}
			}

			switch (move) {
			case 'l': m++; break;
			case 'r': m++; break;
			case 'u': n++; break;
			case 'd': n++; break;
			}

		}

		switch (move) {
		case 'l': n--; m = -1; break;
		case 'r': n++; m = -1; break;
		case 'u': n = -1; m--; break;
		case 'd': n = -1; m++; break;
		}
	}
}


//				:::: SPECIAL ::::
ForgottenSpirit::ForgottenSpirit(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 1, 1, 1, 1, FORGOTTEN_SPIRIT) {
	setFlying(true);
	setEthereal(true);

	setImageName("Old_Orc.png");
	setSprite(dungeon->createSprite(x, y, 5, getImageName()));
	getSprite()->setOpacity(180);

	(*dungeon)[y*dungeon->getCols() + x].enemy = false;
}

void ForgottenSpirit::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].spirit = false;
	(*m_dungeon)[y*cols + x].spirit = true;

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	getSprite()->setLocalZOrder(y + Z_ACTOR + 2);
	setPosX(x); setPosY(y);
}
void ForgottenSpirit::move() {

	if (m_turns > 0) {
		m_turns--;
		return;
	}

	int x = getPosX();
	int y = getPosY();
	
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	// Check if player moved on top of the spirit
	if (x == px && y == py) {
		attack(*m_dungeon->getPlayer());
		return;
	}

	attemptGreedyChase();
	m_turns = 4;
}
void ForgottenSpirit::attack(Player &p) {
	playSound("Devils_Gift.mp3");

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
	m_dungeon->runSingleAnimation(frames, 120, px, py, 2);

	moveTo(px, py);

	p.getSprite()->setVisible(false);
	p.setSuperDead(true);
}

Ghost::Ghost(Dungeon *dungeon, int x, int y, int range) : Monster(dungeon, x, y, 1, 0, 1, 1, GHOST), m_range(range) {
	setChasesPlayer(true);
	setEthereal(true);
	setFlying(true);
	setImageName("Crying_Ghost_48x48.png");
	setGold(10);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Ghost::move() {
	if (playerInRange(m_range))
		attemptGreedyChase();
}
void Ghost::attack(Player &p) {
	// sound effect
	playSound("Possessed.mp3");

	// Possess the player
	p.addAffliction(std::make_shared<Possessed>(10));

	setDestroyed(true); // Destroy the ghost so that no rewards are dropped
}

Flare::Flare(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 1, 0, 1, 1, FLARE) {
	setChasesPlayer(true);
	setFlying(true);
	setImageName("Spinner_Buddy_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Flare::move() {
	if (playerInRange(m_range)) {
		char move;
		if (chase(move))
			attemptMove(move);
	}
}
void Flare::attack(Player &p) {
	moveTo(p.getPosX(), p.getPosY());

	if (p.canBeBurned())
		p.addAffliction(std::make_shared<Burn>(p, 8));
	
	setDestroyed(true);
}
void Flare::reactToDamage() {
	if (playerIsAdjacent())
		if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() < 50)
			if (m_dungeon->getPlayer()->canBeBurned())
				m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 8));
}

MonsterSegment::MonsterSegment(Dungeon *dungeon, int x, int y, int sx, int sy, std::string name, std::string image) : Monster(dungeon, x, y, 1000, 1, 1, 1, name), m_parentX(sx), m_parentY(sy) {
	setImageName(image);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
bool MonsterSegment::canBeDamaged(DamageType type) {
	m_damaged = true;

	return true;
}

//				:::: FLOOR I ::::
Seeker::Seeker(Dungeon *dungeon, int x, int y, int range) : Monster(dungeon, x, y, 8 + randInt(3), 0, 5, 1, SEEKER), m_range(range), m_step(false) {
	setChasesPlayer(true);
	setSturdy(false);
	setImageName("Seeker_48x48.png");
	setGold(1);

	setHasAnimation(true);
	setAnimationFrames("Skeleton_Idle_%04d.png");
	setAnimationFrameCount(4);
	setFrameInterval(8);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector(getAnimationFrames(), getAnimationFrameCount());
	setSprite(dungeon->runAnimationForever(frames, getFrameInterval(), x, y, y + Z_ACTOR));
}
Seeker::Seeker(Dungeon *dungeon, int x, int y, int range, std::string name, std::string image) : Monster(dungeon, x, y, 10 + randInt(5), 1, 4, 1, name), m_range(range), m_step(false) {
	setChasesPlayer(true);
	setSmart(true);
	setImageName(image);
}

void Seeker::move() {
	char best;

	// if seeker is on move cooldown, take them off cooldown
	if (!m_step) {
		m_step = !m_step;
		return;
	}

	if (playerInRange(m_range)) {
		// if viable path is found
		if (chase(best)) {
			attemptMove(best);

			m_step = !m_step;
		}
	}
}

Goblin::Goblin(Dungeon *dungeon, int x, int y, int range) : Monster(dungeon, x, y, randInt(5) + 10, 1, 5, 1, GOBLIN), m_range(range) {
	setChasesPlayer(true);
	setSturdy(false);
	setImageName("Goblin_48x48.png");
	setGold(3);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Goblin::move() {
	if (playerInRange(m_range)) {
		char best;
		if (chase(best))
			attemptMove(best);
	}
}
void Goblin::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	if (1 + randInt(100) + m_dungeon->getPlayer()->getLuck() > 95) {
		if (m_dungeon->itemObject(x, y))
			m_dungeon->itemHash(x, y);

		m_dungeon->createItem(std::make_shared<BoneAxe>(x, y));
	}
}

Wanderer::Wanderer(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 3, 1, WANDERER) {
	setFlying(true);
	setImageName("Wanderer_48x48.png");
	setGold(1);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
Wanderer::Wanderer(Dungeon *dungeon, int x, int y, std::string name, std::string image, int hp, int armor, int str, int dex)
	: Monster(dungeon, x, y, hp, armor, str, dex, name) {
	setFlying(true);
	setImageName(image);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Wanderer::move() {
	int x = getPosX();
	int y = getPosY();

	int n = -1 + randInt(3);
	int m = -1 + randInt(3);

	// if the randomly selected move is not a wall or enemy, move them
	if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m))) {

		if (!m_dungeon->hero(x + n, y + m))
			moveTo(x + n, y + m);
		else
			attack(*m_dungeon->getPlayer());
	}
}
void Wanderer::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// 80% drop chance
	if (1 + randInt(100) + m_dungeon->getPlayer()->getLuck() > 20) {
		if (m_dungeon->itemObject(x, y))
			m_dungeon->itemHash(x, y);

		m_dungeon->createItem(std::make_shared<HeartPod>(x, y));
	}
}

SleepingWanderer::SleepingWanderer(Dungeon *dungeon, int x, int y) : Wanderer(dungeon, x, y, SLEEPING_WANDERER, "Pink_Wanderer_48x48.png", 10, 1, 4, 1) {
	setChasesPlayer(true);
	setGold(3);
}

void SleepingWanderer::move() {
	// If provoked, attempt to chase the player
	if (m_provoked) {
		if (playerInDiagonalRange(m_range))
			attemptGreedyChase(true);
		else
			m_provoked = false;

		return;
	}

	// if player is too close, then wake them up
	if (playerInRange(m_provocationRange))
		m_provoked = true;
}
void SleepingWanderer::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	if (1 + randInt(100) + m_dungeon->getPlayer()->getLuck() > 95) {

		if (m_dungeon->itemObject(x, y))
			m_dungeon->itemHash(x, y);

		m_dungeon->createItem(std::make_shared<BatWing>(x, y));
	}
}

ProvocableWanderer::ProvocableWanderer(Dungeon *dungeon, int x, int y) : Wanderer(dungeon, x, y, PROVOCABLE_WANDERER, "Orange_Wanderer_48x48.png", 12 + randInt(4), 1, 5, 1) {
	setGold(8);
	m_prevHP = getHP();
}

void ProvocableWanderer::move() {
	int x = getPosX();
	int y = getPosY();

	if (m_prevHP > getHP())
		m_provoked = true;

	// If provoked, then try to attack the player
	if (m_provoked) {

		if (playerInRange(1))
			attack(*m_dungeon->getPlayer());

		m_provoked = false;
		m_prevHP = getHP();

		return;
	}

	if (m_turns > 0) {
		m_turns--;
		return;
	}

	int n = -1 + randInt(3);
	int m = -1 + randInt(3);

	while (n == 0 && m == 0 || n != 0 && m != 0) {
		n = -1 + randInt(3);
		m = -1 + randInt(3);
	}

	// if the randomly selected move is a free space, move them
	if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {
		moveTo(x + n, y + m);

		m_turns = 2;
	}
}
void ProvocableWanderer::attack(Player &p) {
	damagePlayer(std::max(1, getStr() + randInt(4) - p.getArmor()), getDamageType());
}
void ProvocableWanderer::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// 100% drop chance
	if (1 + randInt(100) + m_dungeon->getPlayer()->getLuck() > 0) {
		if (m_dungeon->itemObject(x, y))
			m_dungeon->itemHash(x, y);

		m_dungeon->createItem(std::make_shared<HeartPod>(x, y));
	}
}

Roundabout::Roundabout(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 9 + randInt(2), 1, 5, 1, ROUNDABOUT) {
	switch (randInt(4)) {
	case 0: m_dir = 'l'; break;
	case 1: m_dir = 'r'; break;
	case 2: m_dir = 'u'; break;
	case 3: m_dir = 'd'; break;
	default: break;
	}

	setImageName("Roundabout_48x48.png");
	setGold(2);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
Roundabout::Roundabout(Dungeon *dungeon, int x, int y, std::string name, std::string image, int hp, int armor, int str, int dex) : Monster(dungeon, x, y, hp, armor, str, dex, name) {
	int n = randInt(4);

	switch (n) {
	case 0: m_dir = 'l'; break;
	case 1: m_dir = 'r'; break;
	case 2: m_dir = 'u'; break;
	case 3: m_dir = 'd'; break;
	default: break;
	}

	setImageName(image);
}

void Roundabout::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	bool lethal = false, isLava = false;

	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	// if there's a trap and it's lethal, mark it
	if (m_dungeon->trap(x + n, y + m)) {
		std::vector<int> indexes = m_dungeon->findTraps(x + n, y + m);
		for (unsigned int i = 0; i < indexes.size(); i++) {
			if (m_dungeon->trapAt(indexes[i])->isLethal()) {
				lethal = true;
				isLava = m_dungeon->trapAt(indexes[i])->getName() == LAVA;
			}
		}
	}

	// if trap is lava, but they are immune to lava (fire roundabout), move them
	if (isLava && lavaImmune()) {
		moveTo(x + n, y + m);

		return;
	}

	//	if tile to left of the roundabout is not free, check above
	if (m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m) || lethal) {

		// check direction to the "right" of the roundabout, relative to their movement
		switch (m_dir) {
		case 'l': n = 0; m = -1; break;
		case 'r': n = 0; m = 1; break;
		case 'u': n = 1; m = 0; break;
		case 'd': n = -1; m = 0; break;
		}

		//	if player is in the way, attack them
		if (m_dungeon->hero(x + n, y + m))
			attack(*m_dungeon->getPlayer());

		//	if tile above is a wall, then set direction to right
		if (m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) {
			switch (m_dir) {
			case 'l': setDirection('r'); break;
			case 'r': setDirection('l'); break;
			case 'u': setDirection('d'); break;
			case 'd': setDirection('u'); break;
			}
		}
		//	otherwise direction is set up
		else {
			switch (m_dir) {
			case 'l': setDirection('u'); break;
			case 'r': setDirection('d'); break;
			case 'u': setDirection('r'); break;
			case 'd': setDirection('l'); break;
			}
		}

	}
	else {
		moveTo(x + n, y + m);
	}

	specialMove();
}
void Roundabout::attack(Player &p) {
	int px = p.getPosX();
	int py = p.getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.canBlock() && p.didBlock(getPosX(), getPosY())) {
		damage = std::max(0, p.blockedDamageReduction() - damage);
		p.successfulBlock();

		if (p.activeHasAbility())
			p.useActiveAbility(*this);

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	damagePlayer(damage, DamageType::NORMAL);
}

GooSack::GooSack(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 7 + randInt(5), 0, 3, 1, GOO_SACK) {
	setImageName("Goo_Sack_48x48.png");
	setGold(2);

	setHasAnimation(true);
	setAnimationFrames("Goo_Sack_%04d.png");
	setAnimationFrameCount(4);
	setFrameInterval(4);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector(getAnimationFrames(), getAnimationFrameCount());
	setSprite(dungeon->runAnimationForever(frames, getFrameInterval(), x, y, y + Z_ACTOR));
}

void GooSack::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// if not primed but player is nearby, then get primed
	if (!m_primed && playerIsAdjacent(true)) {
		// sound effect for priming
		playSound("Goo_Sack_Primed.mp3");

		m_primed = true;
		m_x = px;
		m_y = py;

		return;
	}
	else if (!m_primed && !playerIsAdjacent(true))
		return;


	// otherwise if player is near the goo sack and it is primed, attack them
	if (m_primed && playerIsAdjacent(true))
		attack(*m_dungeon->getPlayer());

	else if (abs(px - x) <= 2 && abs(py - y) <= 2 && !(m_dungeon->wall(m_x, m_y) || m_dungeon->enemy(m_x, m_y) || m_dungeon->hero(m_x, m_y)))
		moveTo(m_x, m_y);


	m_primed = false;
}
void GooSack::attack(Player &p) {
	int px = p.getPosX();
	int py = p.getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.canBlock() && p.didBlock(getPosX(), getPosY())) {
		damage = std::max(0, p.blockedDamageReduction() - damage);
		p.successfulBlock(); // calls shield durability change

		if (p.activeHasAbility()) {
			p.useActiveAbility(*this);
		}

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}
	
	damagePlayer(damage, getDamageType());
}

Broundabout::Broundabout(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 6 + randInt(6), 0, 3, 1, BROUNDABOUT) {
	int n = randInt(4);

	switch (n) {
	case 0: m_dir = 'l'; break;
	case 1: m_dir = 'r'; break;
	case 2: m_dir = 'u'; break;
	case 3: m_dir = 'd'; break;
	default: break;
	}

	setGold(3);

	setHasAnimation(true);
	setAnimationFrames("Broundabout_%04d.png");
	setAnimationFrameCount(4);
	setFrameInterval(8);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector(getAnimationFrames(), getAnimationFrameCount());
	setSprite(dungeon->runAnimationForever(frames, getFrameInterval(), x, y, y + Z_ACTOR));
}

void Broundabout::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();
	int x = getPosX();
	int y = getPosY();

	bool wall, enemy, hero;

	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	wall = m_dungeon->wall(x + n, y + m);
	enemy = m_dungeon->enemy(x + n, y + m);
	hero = m_dungeon->hero(x + n, y + m);

	// if not primed but player is nearby, then get primed
	if (!m_primed && (abs(px - x) == 0 && abs(py - y) <= 1 || abs(px - x) <= 1 && abs(py - y) == 0)) {
		// sound effect for priming
		playSound("Goo_Sack_Primed.mp3");

		m_primed = true;
		m_x = px;
		m_y = py;

		return;
	}

	// otherwise if player is near the broundabout and it is primed, attack them
	if (m_primed && (abs(px - x) == 0 && abs(py - y) <= 1 || abs(px - x) <= 1 && abs(py - y) == 0))
		attack(*m_dungeon->getPlayer());

	// else if primed, but player moved away, let the broundabout jump towards where the player was
	else if (m_primed && !m_dungeon->enemy(m_x, m_y) && (px != m_x || py != m_y) && abs(px - x) <= 2 && abs(py - y) <= 2)
		moveTo(m_x, m_y);

	// if nothing is in the way, keep moving
	else if (!(wall || enemy || hero))
		moveTo(x + n, y + m);

	// if there's a non-player object in the way, reverse movement
	else if ((wall || enemy) && !hero) {
		switch (m_dir) {
		case 'l': m_dir = 'r'; break;
		case 'r': m_dir = 'l'; break;
		case 'u': m_dir = 'd'; break;
		case 'd': m_dir = 'u'; break;
		}
	}

	m_primed = false;
}
void Broundabout::attack(Player &p) {
	int px = p.getPosX();
	int py = p.getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.canBlock() && p.didBlock(getPosX(), getPosY())) {

		if (p.activeHasAbility())
			p.useActiveAbility(*this);

		damage = std::max(0, damage - p.blockedDamageReduction());
		p.successfulBlock();

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	damagePlayer(damage, getDamageType());
}

Rat::Rat(Dungeon *dungeon, int x, int y, int range) : Monster(dungeon, x, y, 6 + randInt(3), 0, 2, 1, RAT), m_range(range) {
	setChasesPlayer(true);
	setImageName("Small_Dry_Demon.png");
	setGold(2);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Rat::move() {
	int cols = m_dungeon->getCols();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (playerInRange(m_range)) {
		// If the player didn't move since their last turn, chase the player
		if (px == m_prevPlayerX && py == m_prevPlayerY)
			attemptGreedyChase();

		// Otherwise the rat runs away
		else
			run();
	}

	m_prevPlayerX = px;
	m_prevPlayerY = py;
}
void Rat::run() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	int n, m;
	int fx = x, fy = y; // final x and y coordinates for the rat
	for (n = -1, m = 0; n < 2; n += 2) {
		// if space is free and distance is farthest, set new move
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}
	}

	for (n = 0, m = -1; m < 2; m += 2) {
		// if space is free and distance is farthest, set new move
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}
	}

	moveTo(fx, fy);
}

Spider::Spider(Dungeon *dungeon, int x, int y, int webCount) : Monster(dungeon, x, y, 4 + randInt(4), 1, 4, 1, SPIDER), m_webCount(webCount) {
	setImageName("SpiderIdle0001.png");
	setGold(4);

	setHasAnimation(true);
	setAnimationFrames("SpiderIdle%04d.png");
	setAnimationFrameCount(5);
	setFrameInterval(8);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector(getAnimationFrames(), getAnimationFrameCount());
	setSprite(dungeon->runAnimationForever(frames, getFrameInterval(), x, y, y + Z_ACTOR));

	createWebs(x, y);
}
Spider::Spider(Dungeon *dungeon, int x, int y, int webCount, std::string name, std::string image, int hp, int armor, int str, int dex)
	: Monster(dungeon, x, y, hp, armor, str, dex, name), m_webCount(webCount) {
	setImageName(image);

	setHasAnimation(true);
	setAnimationFrames("SpiderIdle%04d.png");
	setAnimationFrameCount(5);
	setFrameInterval(8);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector(getAnimationFrames(), getAnimationFrameCount());
	setSprite(dungeon->runAnimationForever(frames, getFrameInterval(), x, y, y + Z_ACTOR));

	createWebs(x, y);
}

void Spider::move() {
	int cols = m_dungeon->getCols();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// If the player was captured, chase them down
	if (m_captured) {
		attemptGreedyChase(true);
		return;
	}

	// If player is standing on one of the spider's webs, then we will begin the chase
	for (unsigned int i = 0; i < m_webs.size(); i++) {
		if (m_webs[i].first == px && m_webs[i].second == py) {
			m_captured = true;
			m_webs.erase(m_webs.begin() + i);
			return;
		}
	}

	// Check if player is adjacent
	if (getName() != SHOOTING_SPIDER) {

		if (!m_primed && !m_wait) {
			if (playerIsAdjacent(true)) {
				playSound("Spider_Primed.mp3");
				m_primed = true;
			}
		}
		else if (m_primed && !m_wait) {
			// If player is still adjacent, attack them
			if (playerIsAdjacent(true))
				attack(*m_dungeon->getPlayer());

			m_primed = false;
			m_wait = true;
		}
		else if (m_wait)
			m_wait = false;
	}

	// Special actions priority over creating new webs
	if (specialMove())
		return;

	// Create new webs if needed
	if ((int)m_webs.size() < m_webCount) {
		if (!m_dungeon->trap(x, y)) {
			m_webs.push_back(std::make_pair(x, y));
			m_dungeon->addTrap(std::make_shared<Web>(*m_dungeon, x, y));
		}
		else {
			moveMonsterRandomly();
		}
	}
}
void Spider::attack(Player &p) {
	int px = p.getPosX();
	int py = p.getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.canBlock() && p.didBlock(mx, my)) {
		damage = std::max(0, damage - p.blockedDamageReduction());
		p.successfulBlock(); // calls shield durability change

		if (p.activeHasAbility())
			p.useActiveAbility(*this);

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	damagePlayer(damage, getDamageType());

	m_captured = false;
	m_primed = false;
}
void Spider::createWebs(int x, int y) {
	std::vector<std::pair<int, int>> coords;

	// Create vector of (x, y) pairs to choose from
	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {
			coords.push_back(std::make_pair(i, j));
		}
	}

	int n, m;
	// Try to create N webs in random locations around the spider
	for (int i = 0; i < m_webCount; i++) {

		if ((int)m_webs.size() >= m_webCount)
			break;

		int index = randInt((int)coords.size());
		std::pair<int, int> coord = coords[index];
		n = coord.first;
		m = coord.second;
		if (!(m_dungeon->wall(n, m) || m_dungeon->trap(n, m))) {
			m_webs.push_back(std::make_pair(n, m));
			m_dungeon->addTrap(std::make_shared<Web>(*m_dungeon, n, m));

			coords.erase(coords.begin() + index);
		}
	}
}

ShootingSpider::ShootingSpider(Dungeon *dungeon, int x, int y, int webCount, int range)
	: Spider(dungeon, x, y, webCount, SHOOTING_SPIDER, "SpiderIdle0001.png", 7 + randInt(7), 1, 6, 1), m_range(range) {
	m_turns = 2;
	m_maxTurns = m_turns;
	setGold(12);
}

bool ShootingSpider::specialMove() {

	// If spider is primed, then they will attempt to web the player from a distance
	if (m_primed) {

		if (hasLineOfSight(*m_dungeon, *m_dungeon->getPlayer(), getPosX(), getPosY())) {
			// web player
			playSound("Grass2.mp3");

			m_dungeon->getPlayer()->addAffliction(std::make_shared<Stun>(3));
			setCapture(true);
			m_turns = 0; // This prevents the spider from stun-locking the player
		}
		else {
			m_turns = 1; // Forces spider on cooldown for 1 turn before trying to shoot again
		}

		m_primed = false;
		return true;
	}

	// If not off cooldown yet, keep cooling down
	if (m_turns < m_maxTurns) {
		m_turns++;
		return false;
	}

	// If player is in range, then get primed
	if (playerInRange(m_range) && hasLineOfSight(*m_dungeon, *m_dungeon->getPlayer(), getPosX(), getPosY())) {
		playSound("Spider_Primed.mp3");

		m_primed = true;
		return true;
	}

	return false;
}

PouncingSpider::PouncingSpider(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 6 + randInt(6), 2, 7, 2, POUNCING_SPIDER), m_x(x), m_y(y) {
	setHasAnimation(true);
	setAnimationFrames("RedSpiderIdle%04d.png");
	setAnimationFrameCount(5);
	setFrameInterval(8);

	setImageName("RedSpiderIdle0001.png");
	setGold(10);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector(getAnimationFrames(), getAnimationFrameCount());
	setSprite(dungeon->runAnimationForever(frames, getFrameInterval(), x, y, y + Z_ACTOR));
}

void PouncingSpider::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// If primed, check if player is still within their region and attack them if so
	if (m_primed) {
		if (boundaryCheck()) {
			attack(*m_dungeon->getPlayer());

			// Jump on top of player
			moveTo(px, py, 0.05f);
		}

		m_turns = 0;
		m_primed = false;
		return;
	}

	// If not off cooldown yet, keep cooling down
	if (m_turns < m_maxTurns) {

		// If the spider just pounced the player, try jumping back to their original spot
		if (m_turns == 0) {
			if (!(m_dungeon->enemy(m_x, m_y) || m_dungeon->wall(m_x, m_y) || m_dungeon->hero(m_x, m_y))) {
				moveTo(m_x, m_y, 0.05f);
			}
			// If the space was occupied, then the spider's current position becomes
			// their new home position.
			else {
				m_x = x;
				m_y = y;
			}
		}

		m_turns++;
		return;
	}

	// Check if player is in their territory
	if (boundaryCheck()) {
		playSound("Spider_Primed.mp3");

		m_primed = true;
		return;
	}
}
bool PouncingSpider::boundaryCheck() const {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// Check if player is in their territory
	for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 2; j < x + 3; j++) {

			// Boundary and corner check
			if (m_dungeon->withinBounds(j, i) && !(j == x && i == y) &&
				!(i == y - 2 && j == x - 2) && !(i == y - 2 && j == x + 2) && !(i == y + 2 && j == x - 2) && !(i == y + 2 && j == x + 2)) {

				if (px == j && py == i)
					return true;
			}
		}
	}

	return false;
}


//				:::: FLOOR II ::::
DeadSeeker::DeadSeeker(Dungeon *dungeon, int x, int y, int range) : Seeker(dungeon, x, y, range, DEAD_SEEKER, "Dead_Seeker_48x48.png") {
	setGold(5);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

RabidWanderer::RabidWanderer(Dungeon *dungeon, int x, int y) : Wanderer(dungeon, x, y, RABID_WANDERER, "Green_Wanderer_48x48.png", 8 + randInt(4), 2, 6, 1) {
	setGold(5);

}

void RabidWanderer::move() {
	int x = getPosX();
	int y = getPosY();

	int n = -1 + randInt(3);
	int m = -1 + randInt(3);

	// If it's not ready to move, get ready to move
	if (!m_turn) {
		m_turn = true;
		return;
	}

	// if the randomly selected move is not a wall or enemy, move them
	if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m))) {
		if (!m_dungeon->hero(x + n, y + m))
			moveTo(x + n, y + m);
		else
			attack(*m_dungeon->getPlayer());
	}

	m_turn = false;
}
void RabidWanderer::extraAttackEffects() {
	if (m_dungeon->getPlayer()->canBePoisoned()) {
		int roll = randReal(1, 100) + m_dungeon->getPlayer()->getLuck();

		if (roll < 70)
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 8, 5, 1, 1));
	}
}
void RabidWanderer::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// 100% drop chance
	if (1 + randInt(100) + m_dungeon->getPlayer()->getLuck() > 0) {
		if (m_dungeon->itemObject(x, y))
			m_dungeon->itemHash(x, y);

		m_dungeon->createItem(std::make_shared<HeartPod>(x, y));
	}
}

PoisonBubble::PoisonBubble(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 1, 1, 1, 1, POISON_BUBBLE) {
	setImageName("Green_Orb_32x32.png");

	setSprite(dungeon->createSprite(x, y, 2, getImageName()));
	getSprite()->setScale(2.1f * GLOBAL_SPRITE_SCALE);
}

void PoisonBubble::move() {
	return;
}
void PoisonBubble::deathDrops() {
	int cols = m_dungeon->getCols();
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	playSound("Poison_Bomb_Explosion.mp3");

	// Add effect to show poison
	poisonCloud(*m_dungeon, x, y, 0.8f, cocos2d::Color3B(35, 140, 35));

	int range = 1;

	for (int i = x - range; i < x + range + 1; i++) {
		for (int j = y - range; j < y + range + 1; j++) {

			if (!m_dungeon->withinBounds(i, j))
				continue;

			if (m_dungeon->hero(i, j)) {
				if (m_dungeon->getPlayer()->canBePoisoned())
					m_dungeon->getPlayer()->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 8, 4, 1, 1));
			}

			if (m_dungeon->trap(i, j)) {
				std::vector<int> indexes = m_dungeon->findTraps(i, j);
				for (int i = 0; i < (int)indexes.size(); i++)
					if (m_dungeon->trapAt(indexes.at(i))->canBePoisoned())
						m_dungeon->trapAt(indexes.at(i))->poison();
			}
		}
	}

	// Find any monsters
	int mx, my;
	for (unsigned i = 0; i < m_dungeon->monsterCount(); i++) {
		mx = m_dungeon->monsterAt(i)->getPosX();
		my = m_dungeon->monsterAt(i)->getPosY();

		if (abs(mx - x) <= range && abs(my - y) <= range) {
			if (m_dungeon->monsterAt(i)->canBePoisoned() || m_dungeon->getPlayer()->hasAfflictionOverride()) {
				int turns = 8 + (m_dungeon->getPlayer()->hasHarshAfflictions() ? 4 : 0);
				m_dungeon->giveAffliction(i, std::make_shared<Poison>(*m_dungeon->getPlayer(), turns, 4, 1, 1));
			}
		}
	}
}

Piranha::Piranha(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 2, 1, 8, 3, PIRANHA) {
	setGold(7);
	setImageName("Red_Fish_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}

void Piranha::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].enemy = false;
	(*m_dungeon)[y*cols + x].enemy = true;

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);

	getSprite()->setLocalZOrder(y + Z_TRAP_BOTTOM);
}
void Piranha::move() {
	// If player is in the water with the piranha
	//   Chase them using greedy chase
	// Otherwise move around in the body of water randomly

	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_dungeon->trap(px, py)) {
		std::vector<int> indexes = m_dungeon->findTraps(px, py);
		for (int i = 0; i < (int)indexes.size(); i++) {
			if (m_dungeon->trapAt(indexes[i])->getName() == WATER) {
				char move = attemptDryRunGreedyChase(true);

				int n, m;
				setDirectionalOffsets(move, n, m);

				if (m_dungeon->trap(x + n, y + m)) {
					std::vector<int> idxs = m_dungeon->findTraps(x + n, y + m);
					for (int pos = 0; pos < (int)idxs.size(); pos++) {
						if (m_dungeon->trapAt(idxs[pos])->getName() == WATER) {

							if (playerIsAdjacent(true))
								attack(*m_dungeon->getPlayer());
							else if (!(m_dungeon->enemy(x + n, y + m)))
								moveTo(x + n, y + m);

							return;
						}
					}
				}
			}
		}
	}

	attemptMoveInWater();
}
void Piranha::attemptMoveInWater() {
	int x = getPosX();
	int y = getPosY();

	std::vector<std::pair<int, int>> coords;
	coords.push_back(std::make_pair(x + 1, y));
	coords.push_back(std::make_pair(x - 1, y));
	coords.push_back(std::make_pair(x, y - 1));
	coords.push_back(std::make_pair(x, y + 1));

	while (!coords.empty()) {
		int index = randInt((int)coords.size());
		int n = coords[index].first;
		int m = coords[index].second;

		if (m_dungeon->trap(n, m)) {
			std::vector<int> idxs = m_dungeon->findTraps(n, m);
			for (int pos = 0; pos < (int)idxs.size(); pos++) {
				if (m_dungeon->trapAt(idxs[pos])->getName() == WATER) {

					if (!m_dungeon->enemy(n, m)) {
						moveTo(n, m);
						return;
					}
				}
			}
		}

		coords.erase(coords.begin() + index);
	}
}

AngledBouncer::AngledBouncer(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 5, 3, ANGLED_BOUNCER) {

	switch (1 + randInt(4)) {
	case 1: m_dir = '1'; break;
	case 2: m_dir = '2'; break;
	case 3: m_dir = '3'; break;
	case 4: m_dir = '4'; break;
	}

	setGold(7);
	setFlying(true);
	setImageName("Cheese_Wedge_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void AngledBouncer::move() {
	int x = getPosX();
	int y = getPosY();

	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {
		moveTo(x + n, y + m);
	}
	else {
		if (m_dungeon->hero(x + n, y + m))
			attack(*m_dungeon->getPlayer());

		changeDirection();
	}
}
void AngledBouncer::changeDirection() {
	std::vector<char> moves;

	switch (m_dir) {
	case '1':
		moves.push_back('2');
		moves.push_back('3');
		moves.push_back('4');
		break;
	case '2':
		moves.push_back('1');
		moves.push_back('3');
		moves.push_back('4');
		break;
	case '3':
		moves.push_back('1');
		moves.push_back('2');
		moves.push_back('4');
		break;
	case '4':
		moves.push_back('1');
		moves.push_back('2');
		moves.push_back('3');
		break;
	}

	int x = getPosX();
	int y = getPosY();

	int n, m;

	while (!moves.empty()) {
		int index = randInt(moves.size());
		setDirectionalOffsets(moves[index], n, m);

		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {
			m_dir = moves[index];
			moveTo(x + n, y + m);
			return;
		}

		moves.erase(moves.begin() + index);
	}
}

Toad::Toad(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 7 + randInt(4), 1, 3, 2, TOAD) {
	setImageName("Green_Slime_48x48.png");
	setGold(3);
	setCanBePoisoned(false);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Toad::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	switch (m_moveType) {
	case 1:
		if (playerIsAdjacent())
			attack(*m_dungeon->getPlayer());

		m_facing = moveMonsterRandomly();
		break;
	case 2:
	case 3:
	case 4:
		break;
	case 5:
	case 6:
		if (playerIsAdjacent())
			attack(*m_dungeon->getPlayer());

		m_facing = moveMonsterRandomly();
		break;
	case 7:
		break;
	case 8: {

		if (m_facing != '-') {

			int n, m;
			setDirectionalOffsets(m_facing, n, m);

			// If the space is free, try to spit a poison puddle
			if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->trap(x + n, y + m))) {

				// 50% chance to spit out a poison puddle
				if (1 + randInt(100) > 50)
					m_dungeon->addTrap(std::make_shared<PoisonPuddle>(*m_dungeon, x + n, y + m, 8));
			}
		}
	}
			break;
	}

	if (m_moveType < 8)
		m_moveType++;
	else
		m_moveType = 1;
}

WaterSpirit::WaterSpirit(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 12, 1, 3, 1, WATER_SPIRIT) {
	m_maxWait = m_wait;
	setCanBeBurned(false);
	setChasesPlayer(true);
	setFlying(true);
	setImageName("Crying_Ghost_48x48.png");
	setGold(5);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void WaterSpirit::move() {
	douseTiles();

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	if (playerInRange(m_range)) {
		char best;
		if (chase(best))
			attemptMove(best);
	}

	douseTiles();

	m_wait = m_maxWait;
}
void WaterSpirit::douseTiles() {
	int x = getPosX();
	int y = getPosY();

	if (!m_dungeon->trap(x, y)) {
		m_dungeon->addTrap(std::make_shared<Water>(*m_dungeon, x, y));
	}
	else {
		std::vector<int> indexes = m_dungeon->findTraps(x, y);
		for (int i = 0; i < (int)indexes.size(); i++) {
			if (m_dungeon->trapAt(indexes[i])->canBeDoused())
				m_dungeon->trapAt(indexes[i])->douse();

			if (m_dungeon->trapAt(indexes[i])->getName() != WATER &&
				m_dungeon->trapAt(indexes[i])->getName() != POISON_WATER &&
				m_dungeon->trapAt(indexes[i])->getName() != PUDDLE &&
				m_dungeon->trapAt(indexes[i])->getName() != POISON_PUDDLE &&
				m_dungeon->trapAt(indexes[i])->getName() != FROZEN_PUDDLE)
				m_dungeon->addTrap(std::make_shared<Water>(*m_dungeon, x, y));
		}
	}
}
void WaterSpirit::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {
			if (!m_dungeon->withinBounds(i, j) || randReal(1, 100) > 15)
				continue;

			if (!(m_dungeon->wall(i, j) || m_dungeon->trap(i, j))) {
				m_dungeon->addTrap(std::make_shared<Puddle>(*m_dungeon, i, j));
			}
			else if (m_dungeon->trap(i, j)) {
				std::vector<int> indexes = m_dungeon->findTraps(i, j);
				for (int pos = 0; pos < (int)indexes.size(); pos++) {
					if (m_dungeon->trapAt(indexes[pos])->canBeDoused())
						m_dungeon->trapAt(indexes[pos])->douse();

					if (m_dungeon->trapAt(indexes[pos])->getName() != WATER &&
						m_dungeon->trapAt(indexes[pos])->getName() != POISON_WATER &&
						m_dungeon->trapAt(indexes[pos])->getName() != PUDDLE &&
						m_dungeon->trapAt(indexes[pos])->getName() != POISON_PUDDLE &&
						m_dungeon->trapAt(indexes[pos])->getName() != FROZEN_PUDDLE)
						m_dungeon->addTrap(std::make_shared<Puddle>(*m_dungeon, i, j));
				}
			}
		}
	}
}

Firefly::Firefly(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 2, 0, 1, 2, FIREFLY) {
	m_maxWait = m_wait;
	setGold(7);
	setFlying(true);
	setEmitsLight(true);
	setImageName("Cheese_Wedge_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Firefly::move() {
	if (m_wait > 0) {
		m_wait--;
		return;
	}

	moveMonsterRandomly(true);

	m_wait = m_maxWait;
}

TriHorn::TriHorn(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 2, 0, 1, 2, TRI_HORN) {
	m_maxWait = m_wait;
	setGold(7);
	setImageName("Cheese_Wedge_48x48.png");

	switch (1 + randInt(4)) {
	case 1: m_dir = 'l'; break;
	case 2: m_dir = 'r'; break;
	case 3: m_dir = 'u'; break;
	case 4: m_dir = 'd'; break;
	}

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void TriHorn::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_primed) {
		int n, m;
		setDirectionalOffsets(m_dir, n, m);
		bool playerAttacked = false;

		auto frames = getAnimationFrameVector("frame%04d.png", 63);		

		if (n == 0) {

			m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);
			m_dungeon->runSingleAnimation(frames, 120, x + 1, y + m, 2);
			m_dungeon->runSingleAnimation(frames, 120, x - 1, y + m, 2);

			if (m_dungeon->hero(x + n, y + m))
				playerAttacked = true;
			
			if (m_dungeon->hero(x + 1, y + m))
				playerAttacked = true;
			
			if (m_dungeon->hero(x - 1, y + m))
				playerAttacked = true;

			incrementDirectionalOffsets(m_dir, n, m);
			m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);

			if (m_dungeon->hero(x + n, y + m))
				playerAttacked = true;			
		}
		else {

			m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);
			m_dungeon->runSingleAnimation(frames, 120, x + n, y + 1, 2);
			m_dungeon->runSingleAnimation(frames, 120, x + n, y - 1, 2);

			if (m_dungeon->hero(x + n, y + m))
				playerAttacked = true;
			
			if (m_dungeon->hero(x + n, y + 1))
				playerAttacked = true;
			
			if (m_dungeon->hero(x + n, y - 1))
				playerAttacked = true;

			incrementDirectionalOffsets(m_dir, n, m);
			m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);

			if (m_dungeon->hero(x + n, y + m))
				playerAttacked = true;		
		}

		if (playerAttacked)
			attack(*m_dungeon->getPlayer());

		m_primed = false;
		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerInRange(m_range)) {
		m_dir = getCardinalFacingDirectionRelativeTo(x, y, px, py);
		m_primed = true;
		return;
	}

	m_dir = moveMonsterRandomly();
	m_wait = m_maxWait;
}

TumbleShroom::TumbleShroom(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 0, 1, 2, TUMBLE_SHROOM) {
	setGold(7);
	setImageName("Tusked_Demon.png");
	setCanBePoisoned(false);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void TumbleShroom::move() {
	if (m_primed) {
		if (m_primedWait > 0)
			m_primedWait--;
		else {
			releaseSpores();
			m_primed = false;
		}

		return;
	}

	if (m_flipped) {
		if (m_flippedTurns > 0)
			m_flippedTurns--;
		else {
			m_flipped = false;
			getSprite()->setRotation(0);
		}

		return;
	}

	if (playerInRange(m_range)) {
		char best;
		if (chase(best))
			attemptMove(best);
	}
}
void TumbleShroom::moveToWithFlip(int x, int y, float rotation) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].enemy = false;
	(*m_dungeon)[y*cols + x].enemy = true;

	auto flip = cocos2d::RotateBy::create(0.1f, rotation);

	float fx, fy;
	m_dungeon->transformDungeonToSpriteCoordinates(x, y, fx, fy);
	auto move = cocos2d::MoveTo::create(0.1f, cocos2d::Vec2(fx, fy));

	cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	v.pushBack(flip);
	v.pushBack(move);
	m_dungeon->queueCustomSpawnActions(getSprite(), v);

	//m_dungeon->queueMoveSprite(getSprite(), x, y);
	setPosX(x); setPosY(y);

	getSprite()->setLocalZOrder(y + Z_ACTOR);
}
void TumbleShroom::reactToDamage() {
	int x = getPosX();
	int y = getPosY();

	char move = m_dungeon->getPlayer()->facingDirection();

	int n, m;
	setDirectionalOffsets(move, n, m);

	int range = 2;
	int currentRange = 1;
	while (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && currentRange < range) {
		incrementDirectionalOffsets(move, n, m);
		currentRange++;
	}

	if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && currentRange == 2) {
		m_primed = true;
		m_primedWait = 1;

		moveToWithFlip(x + n, y + m, 360);
	}
	else if ((m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && currentRange == 2) {
		decrementDirectionalOffsets(move, n, m);

		m_flipped = true;
		m_flippedTurns = 3;

		moveToWithFlip(x + n, y + m, 180);
	}
	else {
		m_primed = true;
		m_primedWait = 1;
	}

}
void TumbleShroom::releaseSpores() {
	int x = getPosX();
	int y = getPosY();

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (i == x && j == y)
				continue;

			if (!m_dungeon->wall(i, j))
				m_dungeon->addTrap(std::make_shared<Spores>(*m_dungeon, i, j));
		}
	}
}

Wriggler::Wriggler(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 10, 1, WRIGGLER) {
	m_isHead = true;
	setGold(1);
	setHeavy(true);
	//setImageName("Small_Dry_Demon.png");

	//setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	addSegments();
}
Wriggler::Wriggler(Dungeon *dungeon, int x, int y, std::string image, bool isHead, Wriggler *prev)
	: Monster(dungeon, x, y, 10, 1, 10, 1, WRIGGLER), m_isHead(isHead), m_prev(prev) {
	setGold(1);
	setHeavy(true);
	setImageName(image);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Wriggler::addSegments() {
	int x = getPosX();
	int y = getPosY();

	int cols = m_dungeon->getCols();
	std::vector<_Tile> dummyMap = m_dungeon->getDungeon();
	dummyMap[y*cols + x].marked = true;

	std::vector<std::pair<int, int>> coords;
	coords.push_back(std::make_pair(x - 1, y));
	coords.push_back(std::make_pair(x + 1, y));
	coords.push_back(std::make_pair(x, y - 1));
	coords.push_back(std::make_pair(x, y + 1));

	std::vector<std::pair<int, int>> bodyCoords;

	int currentLength = 1;
	while (!coords.empty() && currentLength < m_length) {
		int index = randInt(coords.size());
		int n = coords[index].first;
		int m = coords[index].second;

		if (!(m_dungeon->wall(n, m) || m_dungeon->enemy(n, m) || m_dungeon->hero(n, m) || m_dungeon->trap(n, m) || dummyMap[m*cols + n].marked)) {
			coords.clear();
			bodyCoords.push_back(std::make_pair(n, m));

			dummyMap[m*cols + n].marked = true;

			if (!dummyMap[(m)*cols + (n - 1)].marked)
				coords.push_back(std::make_pair(n - 1, m));

			if (!dummyMap[(m)*cols + (n + 1)].marked)
				coords.push_back(std::make_pair(n + 1, m));

			if (!dummyMap[(m - 1)*cols + (n)].marked)
				coords.push_back(std::make_pair(n, m - 1));

			if (!dummyMap[(m + 1)*cols + (n)].marked)
				coords.push_back(std::make_pair(n, m + 1));

			currentLength++;
		}
		else
			coords.erase(coords.begin() + index);

	}

	if (bodyCoords.empty()) {
		setDestroyed(true);
		return;
	}

	int prevX = x, prevY = y;
	Wriggler *prev = nullptr;
	for (int i = 0; i < (int)bodyCoords.size(); i++) {
		int n = bodyCoords[i].first;
		int m = bodyCoords[i].second;
		std::string image = "Bone_32x32.png";

		std::shared_ptr<Wriggler> monsterSegment = std::make_shared<Wriggler>(m_dungeon, n, m, image, i == bodyCoords.size() - 1, prev);
		m_dungeon->addMonster(monsterSegment);

		if (i == 0) {
			m_dir = getCardinalFacingDirectionRelativeTo(n, m, x, y);
		
			switch (m_dir) {
			case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
			case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
			case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
			case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
			}
			setSprite(m_dungeon->createSprite(x, y, y + Z_ACTOR, image));

			m_next = monsterSegment.get();
			monsterSegment->m_prev = this;
		}

		if (i == bodyCoords.size() - 1) {
			monsterSegment->m_dir = getCardinalFacingDirectionRelativeTo(prevX, prevY, n, m);

			switch (monsterSegment->m_dir) {
			case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
			case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
			case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
			case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
			}
			m_dungeon->queueRemoveSprite(monsterSegment->getSprite());
			monsterSegment->setSprite(m_dungeon->createSprite(n, m, 1, image));
		}
		
		if (prev)
			prev->m_next = monsterSegment.get();

		prev = monsterSegment.get();
		monsterSegment.reset();
		
		prevX = n;
		prevY = m;
	}
}
void Wriggler::move() {
	if (m_dir == '-')
		return;

	int x = getPosX();
	int y = getPosY();

	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	if (m_primed) {	
		if (m_dungeon->hero(x + n, y + m))
			attack(*m_dungeon->getPlayer());
		
		m_primed = false;
		return;
	}

	if (m_dungeon->hero(x + n, y + m)) {
		playSound("Goo_Sack_Primed.mp3");

		m_primed = true;
		return;
	}
}
void Wriggler::extraDeathEffects() {
	if (m_prev) {
		if (m_prev->isDead()) {
			
			m_prev = nullptr;
		}
		else
			m_prev->convertToHead();
	}
	
	if (m_next) {
		if (m_next->isDead()) {
			
			m_next = nullptr;
		}
		else
			m_next->convertToHead();
	}
}
void Wriggler::convertToHead() {
	int x = getPosX();
	int y = getPosY();

	if (!m_isHead) {
		
		if (!m_prev && !m_next || (m_prev && m_prev->isDead()) && (m_next && m_next->isDead())) {
			setDead(true);
			return;
		}
		else if (!m_prev || m_prev && m_prev->isDead()) {
			m_dir = getCardinalFacingDirectionRelativeTo(m_next->getPosX(), m_next->getPosY(), x, y);
		}
		else if (!m_next || m_next && m_next->isDead()) {
			m_dir = getCardinalFacingDirectionRelativeTo(m_prev->getPosX(), m_prev->getPosY(), x, y);
		}

		m_dungeon->queueRemoveSprite(getSprite());
		std::string image;
		switch (m_dir) {
		case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
		case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
		case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
		case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
		}
		setSprite(m_dungeon->createSprite(x, y, y + Z_ACTOR, image));
		m_isHead = true;
	}
	else {
		setDead(true);
	}
}

BarbedCaterpillar::BarbedCaterpillar(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 0, 1, 2, BARBED_CATERPILLAR) {
	m_maxWait = m_wait;
	setGold(7);
	setImageName("Tusked_Demon.png");
	setCanBePoisoned(false);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	if (randReal(1, 100) > 90) {
		m_willBeGood = true;
		m_caterpillarTurns = 15 + randInt(10);
		m_cocoonTurns = 5 + randInt(5);
	}
	else {
		m_willBeGood = false;
		m_caterpillarTurns = -1;
		m_cocoonTurns = -1;
	}
}

void BarbedCaterpillar::move() {
	if (m_caterpillarTurns > 0 || m_caterpillarTurns == -1) {
		if (m_wait > 0) {
			m_wait--;
			return;
		}

		if (playerInRange(m_range))
			attemptGreedyChase();
		else if (randReal(1, 100) > 50)
			moveMonsterRandomly();

		if (m_caterpillarTurns > 0)
			m_caterpillarTurns--;

		m_wait = m_maxWait;
		return;
	}

	if (!m_cocoon && !m_transformed && m_caterpillarTurns == 0) {	
		m_dungeon->queueRemoveSprite(getSprite());
		setSprite(m_dungeon->createSprite(getPosX(), getPosY(), 1, "Cheese_Wedge_48x48.png"));
		m_cocoon = true;
		return;
	}

	if (m_cocoon) {
		m_cocoonTurns--;
		if (m_cocoonTurns == 0) {
			m_dungeon->queueRemoveSprite(getSprite());
			setSprite(m_dungeon->createSprite(getPosX(), getPosY(), 1, "Wanderer_48x48.png"));

			m_cocoon = false;
			m_transformed = true;
			setFlying(true);
		}
	}

	int x = getPosX();
	int y = getPosY();

	if (m_transformed) {
		char move = attemptDryRunGreedyChase(true);

		int n, m;
		setDirectionalOffsets(move, n, m);

		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {
			moveTo(x + n, y + m);
			return;
		}

		if (m_dungeon->hero(x + n, y + m)) {
			moveTo(x + n, y + m);
			m_dungeon->getPlayer()->addAffliction(std::make_shared<HealOverTime>(10));
			setDestroyed(true);
		}
	}
}
void BarbedCaterpillar::extraAttackEffects() {
	if (m_dungeon->getPlayer()->canBePoisoned())
		m_dungeon->getPlayer()->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 8, 3, 1, 1));
}
void BarbedCaterpillar::reactToDamage() {
	double roll = randReal(1, 100) + m_dungeon->getPlayer()->getLuck();

	if (roll < 50 && m_dungeon->getPlayer()->canBePoisoned())
		m_dungeon->getPlayer()->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 8, 3, 1, 1));
}


//				:::: FLOOR III ::::
CrystalTurtle::CrystalTurtle(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 12 + randInt(5), 1, 5, 1, CRYSTAL_TURTLE), m_maxWait(m_wait) {
	m_prevHP = getHP();
	setGold(5);
	setChasesPlayer(true);
	setImageName("Masked_Orc.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void CrystalTurtle::move() {

	if (!m_hit && m_prevHP > getHP()) {
		m_prevHP = getHP();
		m_hit = true;
		return;
	}

	if (m_hit) {
		playSound("Dagger_Swipe1.mp3", *m_dungeon->getPlayer(), getPosX(), getPosY());
		if (playerIsAdjacent(true)) {
			attack(*m_dungeon->getPlayer());
		}

		m_hit = false;
		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerIsAdjacent())
		attack(*m_dungeon->getPlayer());
	else
		attemptGreedyChase();


	m_wait = m_maxWait;
}

CrystalHedgehog::CrystalHedgehog(Dungeon *dungeon, int x, int y, char dir) : Monster(dungeon, x, y, 2, 1, 5, 3, CRYSTAL_HEDGEHOG) {
	setGold(5);
	setImageName("Masked_Orc.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	if (dir == '-') {
		switch (randInt(4)) {
		case 0: m_dir = 'l'; break;
		case 1: m_dir = 'r'; break;
		case 2: m_dir = 'u'; break;
		case 3: m_dir = 'd'; break;
		}
	}
	else
		m_dir = dir;

	m_maxWait = m_wait;
}

void CrystalHedgehog::move() {
	int x = getPosX();
	int y = getPosY();

	int n, m;
	switch (m_dir) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	if (m_wait > 0)
		m_wait--;

	if (m_wait == 0) {
		playSound("Dagger_Swipe1.mp3", *m_dungeon->getPlayer(), x, y);

		if (playerIsAdjacent(true))
			attack(*m_dungeon->getPlayer());

		m_wait = m_maxWait;
		return;
	}

	if (m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) {

		char dir = m_dir;
		while (dir == m_dir) {
			switch (randInt(4)) {
			case 0: dir = 'l'; break;
			case 1: dir = 'r'; break;
			case 2: dir = 'u'; break;
			case 3: dir = 'd'; break;
			}
		}

		m_dir = dir;
		return;
	}

	if (m_dungeon->hero(x + n, y + m)) {
		attack(*m_dungeon->getPlayer());
		return;
	}

	moveTo(x + n, y + m);
}

CrystalShooter::CrystalShooter(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 2, 1, 5, 3, CRYSTAL_SHOOTER) {
	m_maxWait = m_wait;
	setGold(5);
	setImageName("Masked_Orc.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void CrystalShooter::move() {
	int cols = m_dungeon->getCols();

	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	playSound("Dagger_Swipe1.mp3", *m_dungeon->getPlayer(), x, y);

	int n, m;
	int range = 6;
	int currentRange = 1;

	int k = 1; // Number of directions
	while (k <= 8) {

		switch (k) {
		case 1: n = -1; m = 0; break;
		case 2: n = 1; m = 0; break;
		case 3: n = 0; m = -1; break;
		case 4: n = 0; m = 1; break;
		case 5: n = -1; m = -1; break;
		case 6: n = 1; m = -1; break;
		case 7: n = -1; m = 1; break;
		case 8: n = 1; m = 1; break;
		}

		while (!m_dungeon->wall(x + n, y + m) && currentRange <= range) {

			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);

			if (m_dungeon->hero(x + n, y + m)) {
				damagePlayer(getStr(), getDamageType());
				break;
			}

			switch (k) {
			case 1: n--; m = 0; break;
			case 2: n++; m = 0; break;
			case 3: n = 0; m--; break;
			case 4: n = 0; m++; break;
			case 5: n--; m--; break;
			case 6: n++; m--; break;
			case 7: n--; m++; break;
			case 8: n++; m++; break;
			}

			currentRange++;
		}

		k++;
		currentRange = 1;
	}

	m_wait = m_maxWait;
}

CrystalBeetle::CrystalBeetle(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 2, 1, 5, 3, CRYSTAL_BEETLE) {
	m_maxWait = m_wait;
	setGold(5);
	setImageName("Masked_Orc.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	switch (randInt(4)) {
	case 0: m_dir = 'l'; break;
	case 1: m_dir = 'r'; break;
	case 2: m_dir = 'u'; break;
	case 3: m_dir = 'd'; break;
	}
}

bool CrystalBeetle::canBeDamaged(DamageType type) {
	if (type == DamageType::NORMAL || type == DamageType::PIERCING) {
		char move = m_dungeon->getPlayer()->facingDirection();

		bool flag = true;
		switch (m_dir) {
		case 'l':
		case 'r':
		case 'u':
		case 'd': flag = (move != m_dir); break;
		default: return true;
		}

		if (!flag) {
			if (m_backShield > 0) {
				playHitSmasher();
				m_backShield--;
			}
		}

		return flag;
	}
	else
		return true;
}
void CrystalBeetle::move() {

	if (m_backShield == 0) {
		if (playerIsAdjacent(true)) {
			m_dungeon->damagePlayer(getStr() + 10, getDamageType());
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Bleed>(5));
		}
	}

	int x = getPosX();
	int y = getPosY();

	int n = 0, m = 0;
	setDirectionalOffsets(m_dir, n, m);

	if (m_dungeon->hero(x + n, y + m))
		damagePlayer(getStr(), getDamageType());

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	char best;

	if (playerInRange(m_range)) {
		bool success = chase(best);
		if (success) {
			m_dir = best;
			attemptMove(best);

			switch (m_dir) {
			case 'l': n = -1; m = 0; break;
			case 'r': n = 1; m = 0; break;
			case 'u': n = 0; m = -1; break;
			case 'd': n = 0; m = 1; break;
			}

			if (m_dungeon->hero(getPosX() + n, getPosY() + m))
				damagePlayer(getStr(), getDamageType());
		}
	}

	m_wait = m_maxWait;
}

CrystalWarrior::CrystalWarrior(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 12, 3, 10, 1, CRYSTAL_WARRIOR) {
	m_maxWait = m_wait;
	setGold(5);
	setHeavy(true);
	setImageName("Masked_Orc.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	switch (randInt(4)) {
	case 0: m_dir = 'l'; break;
	case 1: m_dir = 'r'; break;
	case 2: m_dir = 'u'; break;
	case 3: m_dir = 'd'; break;
	}
}

bool CrystalWarrior::canBeDamaged(DamageType type) {
	if (type == DamageType::NORMAL || type == DamageType::PIERCING) {
		char move = m_dungeon->getPlayer()->facingDirection();

		if (directionIsOppositeTo(m_dir, move)) {
			if (m_shieldStrength > 0) {
				playHitSmasher();
				m_shieldStrength--;
				m_primed = true;

				if (m_shieldStrength == 0) {
					playSound("Freeze_Spell1.mp3");

					// Chance for shattered shield crystals to damage player
					if (playerIsAdjacent() && randReal(1, 100) + m_dungeon->getPlayer()->getLuck() < 50)
						m_dungeon->damagePlayer(10, DamageType::PIERCING);

					// Remove shield sprite
				}

				return false;
			}
		}

		return true;
	}
	else
		return true;
}
void CrystalWarrior::move() {
	if (m_primed) {
		if (playerIsAdjacent(true))
			attack(*m_dungeon->getPlayer());

		m_primed = false;
		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	m_dir = attemptDryRunGreedyChase();
	attemptGreedyChase();

	m_wait = m_maxWait;
}

Rabbit::Rabbit(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 2, 1, 5, 3, RABBIT) {
	m_maxRunSteps = m_runSteps;
	setGold(5);
	setImageName("Small_Red_Demon.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Rabbit::move() {

	if (!playerInRange(m_range)) {
		if (m_beingChased) {
			if (m_runSteps == 0) {
				m_beingChased = false;
				m_runSteps = m_maxRunSteps;
				return;
			}

			m_runSteps--;

			run();
		}

		return;
	}

	m_beingChased = true;

	run();
}
void Rabbit::run() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	int n, m;
	int fx = x, fy = y; // final x and y coordinates
	for (n = -1, m = 0; n < 2; n += 2) {
		// if space is free and distance is farthest, set new move
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}
	}

	for (n = 0, m = -1; m < 2; m += 2) {
		// if space is free and distance is farthest, set new move
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}
	}

	moveTo(fx, fy);
}

Bombee::Bombee(Dungeon *dungeon, int x, int y, int range) : Monster(dungeon, x, y, 1, 0, 1, 1, BOMBEE), m_fuse(3), m_fused(false), m_aggroRange(range) {
	setChasesPlayer(true);
	setImageName("Bombee_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
Bombee::Bombee(Dungeon *dungeon, int x, int y, int range, std::string name, std::string image, int hp, int armor, int str, int dex)
	: Monster(dungeon, x, y, hp, armor, str, dex, name), m_fuse(3), m_fused(false), m_aggroRange(range) {
	setChasesPlayer(true);
	setImageName(image);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Bombee::move() {
	if (playerInRange(m_aggroRange)) {
		char best;
		if (chase(best))
			attemptMove(best);
	}
}
void Bombee::attack(Player &p) {
	damagePlayer(2, getDamageType());
}
void Bombee::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	m_dungeon->addTrap(std::make_shared<ActiveMegaBomb>(*m_dungeon, x, y));
}

BenevolentBark::BenevolentBark(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 20, 1, 20, 1, BENEVOLENT_BARK) {
	setGold(7);
	setHeavy(true);
	setImageName("Small_Red_Demon.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void BenevolentBark::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (!m_attackMode) {
		if (m_dungeon->getPlayer()->didAttack() && playerInBufferedLinearRange(m_range, 3, x, y, px, py)) {
			m_attackMode = true;
			return;
		}
	}

	if (m_attackMode) {
		if (playerIsAdjacent()) {
			attack(*m_dungeon->getPlayer());
			m_attackMode = false;
			return;
		}
	}

	if (playerInRange(m_range)) {
		if (playerInRange(m_stopRange) && !m_attackMode)
			return;

		attemptGreedyChase();
	}
}

Tick::Tick(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 2, 1, 5, 3, TICK) {
	setGold(7);
	setImageName("Small_Red_Demon.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Tick::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_primed) {

		if (m_firstMove)
			m_firstMove = false;

		if (playerIsAdjacent(true)) {
			playSound("Grass2.mp3");

			damagePlayer(1, getDamageType());
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Ticked>());

			moveTo(px, py);
			setDestroyed(true);
		}
		else {
			moveTo(m_px, m_py);
			m_wait = m_maxWait;
		}

		m_primed = false;
		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerIsAdjacent(true)) {
		playSound("Spider_Primed.mp3");

		m_primed = true;
		m_px = px;
		m_py = py;
		return;
	}

	if (!m_firstMove) {
		moveMonsterRandomly();
		m_wait = m_maxWait;
	}
}

ExoticFeline::ExoticFeline(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 20, 1, 15, 2, EXOTIC_FELINE) {
	setGold(7);
	setChasesPlayer(true);
	setImageName("Small_Red_Demon.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	switch (1 + randInt(4)) {
	case 1: m_dir = 'l'; break;
	case 2: m_dir = 'r'; break;
	case 3: m_dir = 'u'; break;
	case 4: m_dir = 'd'; break;
	}
}

void ExoticFeline::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_primed) {
		char best;
		for (int i = 0; i < 3; i++) {

			if (m_staggered) {
				m_staggered = false;
				continue;
			}

			if (playerIsAdjacent()) {
				char dir = getCardinalFacingDirectionRelativeTo(getPosX(), getPosY(), px, py);
				if (m_dir == dir)
					attack(*m_dungeon->getPlayer());
				else
					m_dir = dir;
			}
			else if (playerInRange(m_approachRange)) {
				if (chase(best))
					attemptMove(best);
			}
		}

		m_primed = false;
		return;
	}

	if (playerInRange(m_attackRange)) {
		playSound("Goo_Sack_Primed.mp3");
		m_primed = true;
		return;
	}

	if (playerInRange(m_approachRange)) {
		cloak();

		char best;
		if (chase(best))
			attemptMove(best);

		return;
	}

	if (playerInRange(m_preyRange))
		cloak();
	
}
void ExoticFeline::reactToDamage() {
	uncloak();

	char move = m_dungeon->getPlayer()->facingDirection();
	if (!directionIsOppositeTo(move, m_dir))
		m_staggered = true;
}
void ExoticFeline::cloak() {
	m_dungeon->queueCustomAction(getSprite(), cocos2d::FadeTo::create(0.3f, 40));
}
void ExoticFeline::uncloak() {
	m_dungeon->queueCustomAction(getSprite(), cocos2d::FadeTo::create(0.3f, 255));
}


//				:::: FLOOR IV ::::
FlameWanderer::FlameWanderer(Dungeon *dungeon, int x, int y) : Wanderer(dungeon, x, y, FLAME_WANDERER, "Red_Wanderer_48x48.png", 12 + randInt(5), 2, 8, 1) {
	setGold(6);
	setCanBeBurned(false);
	setDamageType(DamageType::FIRE);
}

void FlameWanderer::move() {

	if (!m_turn) {
		m_turn = true;
		return;
	}

	if (playerIsAdjacent())
		attack(*m_dungeon->getPlayer());
	else
		moveMonsterRandomly();

	m_turn = false;
}
void FlameWanderer::extraAttackEffects() {
	if (m_dungeon->getPlayer()->canBeBurned()) {
		int roll = randReal(1, 100) + m_dungeon->getPlayer()->getLuck();

		if (roll < 70)
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 4));
	}
}

CharredBombee::CharredBombee(Dungeon *dungeon, int x, int y, int range) : Bombee(dungeon, x, y, range, CHARRED_BOMBEE, "Bombee_48x48.png", 2, 2, 2, 1) {
	setCanBeBurned(false);
	setLavaImmunity(true);
}

void CharredBombee::attack(Player &p) {
	damagePlayer(5, getDamageType());
}

FireRoundabout::FireRoundabout(Dungeon *dungeon, int x, int y) : Roundabout(dungeon, x, y, FIRE_ROUNDABOUT, "Fire_Roundabout_48x48.png", 9 + randInt(2), 1, 5, 1) {
	setLavaImmunity(true);
	setCanBeBurned(false);
	setGold(3);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void FireRoundabout::specialMove() {
	if (1 + randInt(100) > 90)
		m_dungeon->addTrap(std::make_shared<Ember>(*m_dungeon, getPosX(), getPosY()));
}

ItemThief::ItemThief(Dungeon *dungeon, int x, int y, int range) : Monster(dungeon, x, y, 5, 1, 2, 2, ITEM_THIEF), m_range(range) {
	setChasesPlayer(true);
	setSmart(true);
	setSturdy(false);
	setLavaImmunity(true);
	setImageName("Demon_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void ItemThief::move() {
	// if they haven't stolen yet, continue chasing
	if (!m_stole) {
		char best;

		// range is multiplied by 2 since they can move diagonally
		if (playerInDiagonalRange(m_range)) {
			if (chase(best, true))
				attemptMove(best);
		}

		// play sound effect when they steal
		if (m_stole) {
			playSound("Item_Thief_Stole1.mp3");
			playSound("Female_Hey2.mp3");
		}
	}
	// otherwise run away with the player's item!
	else {
		run();
	}
}
void ItemThief::run() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	bool lethal = false;

	// try diagonals first
	int n, m;
	int fx = x, fy = y; // final x and y coordinates for the thief to use
	for (m = -1; m < 2; m++) {
		for (n = -1; n < 2; n++) {

			if (m_dungeon->trap(x + n, y + m)) {
				std::vector<int> indexes = m_dungeon->findTraps(x + n, y + m);
				for (unsigned int i = 0; i < indexes.size(); i++) {
					if (m_dungeon->trapAt(indexes[i])->isLethal())
						lethal = true;
				}
			}

			// if space is free, does not contain a lethal trap, and distance is farthest, set new move
			if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && !lethal && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
				fx = x + n;
				fy = y + m;
			}
		}
	}

	moveTo(fx, fy);
}
void ItemThief::attack(Player &p) {
	// if the player has items, steal one randomly
	if (p.hasItems()) {
		int n = randInt(p.itemCount());
		m_stolenItem = p.itemAt(n);
		p.removeItem(n);
		m_stole = true;
	}
	// else if player has money, steal a random amount
	else if (p.getMoney() > 0) {
		m_stolenGold = 25 + randInt(10);

		if (p.getMoney() >= m_stolenGold)
			p.setMoney(p.getMoney() - m_stolenGold);
		else {
			m_stolenGold = p.getMoney();
			p.setMoney(0);
		}

		m_stole = true;
	}
	// otherwise just attack the player if they have no items or gold
	else {
		int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

		// if player was blocking and blocked successfully
		if (p.canBlock() && p.didBlock(getPosX(), getPosY())) {
			damage = std::max(0, damage - p.blockedDamageReduction());
			p.successfulBlock(); // calls shield durability change
		}
		
		damagePlayer(damage, getDamageType());
	}
}
void ItemThief::deathDrops() {
	int cols = m_dungeon->getCols();
	int x = getPosX();
	int y = getPosY();

	playSound("Item_Thief_Death2.mp3");

	if (m_stolenItem != nullptr) {

		if (m_dungeon->itemObject(x, y))
			m_dungeon->itemHash(x, y);

		m_stolenItem->setPosX(x); m_stolenItem->setPosY(y);
		m_dungeon->createItem(m_stolenItem);
	}

	else if (m_stolenGold > 0) {
		playSound("Coin_Drop5.mp3");
		m_dungeon->addGold(x, y, m_stolenGold);
	}
}

Zapper::Zapper(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10 + randInt(3), 1, 4, 1, ZAPPER), m_cooldown(true), m_cardinalAttack(randInt(2)) {
	// Create sprites
	int n = 0, m = 0;
	for (int i = 0; i < 8; i++) {
		switch (i) {
			// cardinals
		case 0: n = x; m = y - 1; break; // top
		case 1: n = x - 1; m = y; break; // left
		case 2: n = x + 1; m = y; break; // right
		case 3: n = x; m = y + 1; break; // bottom

			// diagonals
		case 4: n = x - 1; m = y - 1; break; // topleft
		case 5: n = x + 1; m = y - 1; break; // topright
		case 6: n = x - 1; m = y + 1; break; // bottomleft
		case 7: n = x + 1; m = y + 1; break; // bottomright
		}

		m_sparks.insert(std::make_pair(i, dungeon->createSprite(n, m, m + Z_PROJECTILE, "Spark_48x48.png")));
		m_sparks[i]->setScale(0.75f);
		m_sparks[i]->setVisible(false);
	}

	setImageName("Zapper_48x48.png");
	setExtraSpritesFlag(true);
	setEmitsLight(true);
	setGold(2);
	setDamageType(DamageType::LIGHTNING);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
	//dungeon.addLightSource(x, y, 2, getName());
}

void Zapper::spriteCleanup() {
	m_dungeon->queueRemoveSprite(getSprite());

	if (!m_sparks.empty()) {
		for (int i = 0; i < 8; i++)
			m_dungeon->queueRemoveSprite(m_sparks[i]);
	}
}

void Zapper::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].enemy = false;
	(*m_dungeon)[y*cols + x].enemy = true;

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	getSprite()->setLocalZOrder(y + Z_ACTOR);
	setPosX(x); setPosY(y);

	moveSprites(x, y);
}
void Zapper::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	//	if zapper is on cooldown, reset cooldown and switch attack direction
	if (m_cooldown) {
		if (m_cardinalAttack) {
			// Hide cardinal sparks
			for (int i = 0; i < 4; i++)
				m_sparks[i]->setVisible(false);
		}
		else {
			// Hide diagonal sparks
			for (int i = 4; i < 8; i++)
				m_sparks[i]->setVisible(false);
		}

		m_cooldown = !m_cooldown;
		m_cardinalAttack = !m_cardinalAttack;
	}
	//	else throw out sparks and attack player if they were in the way
	else {
		if (m_cardinalAttack) {
			// show cardinal sparks
			for (int i = 0; i < 4; i++)
				m_sparks[i]->setVisible(true);

			if ((px == mx + 1 && py == my) || (px == mx - 1 && py == my) || (px == mx && py == my + 1) || (px == mx && py == my - 1))
				attack(*m_dungeon->getPlayer());
		}
		else {
			// show diagonal sparks
			for (int i = 4; i < 8; i++)
				m_sparks[i]->setVisible(true);

			if ((px == mx - 1 && py == my - 1) || (px == mx + 1 && py == my - 1) || (px == mx - 1 && py == my + 1) || (px == mx + 1 && py == my + 1))
				attack(*m_dungeon->getPlayer());
		}

		m_cooldown = !m_cooldown;
	}
}
void Zapper::attack(Player &p) {
	playSound("Shock5.mp3");

	int damage = 1 + randInt(getStr());
	damagePlayer(damage, getDamageType());
}

void Zapper::moveSprites(int x, int y) {
	float fx, fy;
	for (int i = 0; i < 8; i++) {
		switch (i) {
			// cardinals
		case 0: m_dungeon->transformDungeonToSpriteCoordinates(x, y - 1, fx, fy); break; // top
		case 1: m_dungeon->transformDungeonToSpriteCoordinates(x - 1, y, fx, fy); break; // left
		case 2: m_dungeon->transformDungeonToSpriteCoordinates(x + 1, y, fx, fy); break; // right
		case 3: m_dungeon->transformDungeonToSpriteCoordinates(x, y + 1, fx, fy); break; // bottom

			// diagonals
		case 4: m_dungeon->transformDungeonToSpriteCoordinates(x - 1, y - 1, fx, fy); break; // topleft
		case 5: m_dungeon->transformDungeonToSpriteCoordinates(x + 1, y - 1, fx, fy); break; // topright
		case 6: m_dungeon->transformDungeonToSpriteCoordinates(x - 1, y + 1, fx, fy); break; // bottomleft
		case 7: m_dungeon->transformDungeonToSpriteCoordinates(x + 1, y + 1, fx, fy); break; // bottomright
		}

		m_sparks[i]->setPosition(fx, fy);
		m_sparks[i]->setLocalZOrder(fy + Z_PROJECTILE);
		m_sparks[i]->setScale(0.75f);
		m_sparks[i]->setVisible(false);
	}
}
void Zapper::setSpriteColor(cocos2d::Color3B color) {
	for (int i = 0; i < 8; i++)
		m_sparks[i]->setColor(color);
}
void Zapper::addLightEmitters(std::vector<std::pair<int, int>> &lightEmitters) {
	lightEmitters.push_back(std::make_pair(getPosX(), getPosY()));
	int x, y;

	// Add the lightning sparks, if they are visible
	for (unsigned int i = 0; i < m_sparks.size(); i++) {
		if (m_sparks.at(i)->isVisible()) {
			cocos2d::Vec2 pos = m_sparks.at(i)->getPosition();
			m_dungeon->transformSpriteToDungeonCoordinates(pos.x, pos.y, x, y);

			lightEmitters.push_back(std::make_pair(x, y));
		}
	}
}

Spinner::Spinner(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10 + randInt(3), 1, 3, 1, SPINNER) {
	setCanBeBurned(false);

	bool dir = randInt(2);

	m_clockwise = dir;
	m_angle = 1 + randInt(8);

	auto inner = dungeon->createSprite(x, y, 0, "Spinner_Buddy_48x48.png");
	auto outer = dungeon->createSprite(x, y, 0, "Spinner_Buddy_48x48.png");

	m_innerFire = std::make_shared<Objects>();
	m_innerFire->setSprite(inner);
	m_outerFire = std::make_shared<Objects>();
	m_outerFire->setSprite(outer);

	setInitialFirePosition(x, y);

	setImageName("Spinner_48x48.png");
	setExtraSpritesFlag(true);
	setEmitsLight(true);
	setGold(2);
	setDamageType(DamageType::FIRE);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
	//dungeon.addLightSource(x, y, 2, getName());
}

void Spinner::spriteCleanup() {
	m_dungeon->queueRemoveSprite(getSprite());

	m_dungeon->queueRemoveSprite(m_innerFire->getSprite());
	m_dungeon->queueRemoveSprite(m_outerFire->getSprite());
}

void Spinner::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].enemy = false;
	(*m_dungeon)[y*cols + x].enemy = true;

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	getSprite()->setLocalZOrder(y + Z_ACTOR);
	setPosX(x); setPosY(y);

	setInitialFirePosition(x, y);
}
void Spinner::move() {
	int rows = m_dungeon->getRows();
	int cols = m_dungeon->getCols();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	// reset projectiles to visible
	m_innerFire->getSprite()->setVisible(true);
	m_outerFire->getSprite()->setVisible(true);

	// 8 1 2 
	// 7 X 3
	// 6 5 4
	// moves spinner -to- position getAngle(), it is not where it currently is

	if (m_clockwise) {
		switch (m_angle) {
		case 1:
			setFirePosition('r');

			if (my == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 2:
			setFirePosition('r');

			if (my == 1 || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 3:
			setFirePosition('d');

			// if spinner is on the left or right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 4:
			setFirePosition('d');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 5:
			setFirePosition('l');

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 6:
			setFirePosition('l');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (my == rows - 2 || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 7:
			setFirePosition('u');

			// if spinner is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 8:
			setFirePosition('u');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		default:
			break;
		}

		if (m_angle == 8)
			m_angle = 1;
		else
			m_angle++;
	}
	// else counterclockwise
	else {
		switch (m_angle) {
		case 1:
			setFirePosition('l');

			if (my == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 2:
			setFirePosition('u');

			// if spinner is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 3:
			setFirePosition('u');

			// if spinner is on the right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 4:
			setFirePosition('r');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 5:
			setFirePosition('r');

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 6:
			setFirePosition('d');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == rows - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 7:
			setFirePosition('d');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 8:
			setFirePosition('l');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		default:
			break;
		}

		if (m_angle == 1)
			m_angle = 8;
		else
			m_angle--;
	}

	if (playerWasHit(*m_dungeon->getPlayer()))
		attack(*m_dungeon->getPlayer());

}
void Spinner::attack(Player &p) {
	playSound("Fire3.mp3");

	damagePlayer(getStr(), getDamageType());

	// if actor is still alive and can be burned, try to burn them
	if (p.getHP() > 0 && p.canBeBurned()) {

		// chance to burn
		int roll = randInt(100) + p.getLuck();

		// failed the save roll
		if (roll < 60)
			p.addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 5));
	}
}

void Spinner::setInitialFirePosition(int x, int y) {

	/* 8 1 2
	*  7 X 3
	*  6 5 4
	*  moves firebar -to- position getAngle(), it is not where it currently is
	*/

	if (m_clockwise) {
		switch (m_angle) {
		case 1:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y - 2);
			break;
		case 2:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y - 2);
			break;
		case 3:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y - 2);
			break;
		case 4:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y);
			break;
		case 5:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y + 2);
			break;
		case 6:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y + 2);
			break;
		case 7:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y + 2);
			break;
		case 8:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y);
			break;
		}
	}
	else {
		switch (m_angle) {
		case 1:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y - 2);
			break;
		case 2:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y);
			break;
		case 3:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y + 2);
			break;
		case 4:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y + 2);
			break;
		case 5:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y + 2);
			break;
		case 6:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y);
			break;
		case 7:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y - 2);
			break;
		case 8:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y - 2);
			break;
		}
	}

	m_dungeon->queueMoveSprite(m_innerFire->getSprite(), m_innerFire->getPosX(), m_innerFire->getPosY());
	m_innerFire->getSprite()->setLocalZOrder(m_innerFire->getPosY() + Z_ACTOR);
	m_dungeon->queueMoveSprite(m_outerFire->getSprite(), m_outerFire->getPosX(), m_outerFire->getPosY());
	m_outerFire->getSprite()->setLocalZOrder(m_outerFire->getPosY() + Z_ACTOR);

	setSpriteVisibility(false);
}
void Spinner::setFirePosition(char move) {
	switch (move) {
	case 'l':
		m_innerFire->setPosX(m_innerFire->getPosX() - 1);
		m_outerFire->setPosX(m_outerFire->getPosX() - 2);
		break;
	case 'r':
		m_innerFire->setPosX(m_innerFire->getPosX() + 1);
		m_outerFire->setPosX(m_outerFire->getPosX() + 2);
		break;
	case 'u':
		m_innerFire->setPosY(m_innerFire->getPosY() - 1);
		m_outerFire->setPosY(m_outerFire->getPosY() - 2);
		break;
	case 'd':
		m_innerFire->setPosY(m_innerFire->getPosY() + 1);
		m_outerFire->setPosY(m_outerFire->getPosY() + 2);
		break;
	}

	m_dungeon->queueMoveSprite(m_innerFire->getSprite(), m_innerFire->getPosX(), m_innerFire->getPosY());
	m_dungeon->queueMoveSprite(m_outerFire->getSprite(), m_outerFire->getPosX(), m_outerFire->getPosY());
}

bool Spinner::playerWasHit(const Player &p) const {
	if ((m_innerFire->getPosX() == p.getPosX() && m_innerFire->getPosY() == p.getPosY()) ||
		(m_outerFire->getPosX() == p.getPosX() && m_outerFire->getPosY() == p.getPosY()))
		return true;

	return false;
}

void Spinner::setSpriteVisibility(bool visible) {
	m_innerFire->getSprite()->setVisible(visible);
	m_outerFire->getSprite()->setVisible(visible);
}
void Spinner::setSpriteColor(cocos2d::Color3B color) {
	m_innerFire->getSprite()->setColor(color);
	m_outerFire->getSprite()->setColor(color);
}
void Spinner::addLightEmitters(std::vector<std::pair<int, int>> &lightEmitters) {
	lightEmitters.push_back(std::make_pair(getPosX(), getPosY()));

	// Add the spinner sprites too
	/*lightEmitters.push_back(std::make_pair(m_innerFire->getPosX(), m_innerFire->getPosY()));
	lightEmitters.push_back(std::make_pair(m_outerFire->getPosX(), m_outerFire->getPosY()));*/
}

Charger::Charger(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 5 + randInt(5), 1, 3, 1, CHARGER) {
	setChasesPlayer(true);
	setImageName("Demonic_Goblin_48x48.png");
	setGold(5);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Charger::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	int steps = 5;

	if (m_primed) {
		int n, m;
		setDirectionalOffsets(m_direction, n, m);

		while (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m)) && steps > 0) {
			incrementDirectionalOffsets(m_direction, n, m);

			steps--;
		}

		if (m_dungeon->wall(x + n, y + m)) {

			// Destroy this wall if it's a breakable wall
			if (m_dungeon->wallObject(x + n, y + m) && m_dungeon->wallObject(x + n, y + m)->isDestructible())
				m_dungeon->destroyWall(x + n, y + m);

			addAffliction(std::make_shared<Stun>(2));
		}

		else if (m_dungeon->hero(x + n, y + m))
			attack(*m_dungeon->getPlayer());

		
		decrementDirectionalOffsets(m_direction, n, m);
		moveTo(x + n, y + m);		

		m_primed = false;
		m_turns = 2;

		return;
	}

	// If charger has LOS with the player, then get primed
	if (playerInLinearRange(m_losRange, x, y, px, py) && hasLineOfSight(*m_dungeon, x, y, px, py)) {
		playSound("Goo_Sack_Primed.mp3");

		if (px == x && py > y) m_direction = 'd';
		else if (px == x && py < y) m_direction = 'u';
		else if (px > x && py == y) m_direction = 'r';
		else if (px < x && py == y) m_direction = 'l';

		m_primed = true;
	}
	// Otherwise if the charger has no LOS and still has turns to wait before moving, decrement turns left
	else if (m_turns > 0) {
		m_turns--;
	}
	// Otherwise attempt to move toward the player
	else if (playerInRange(m_range)) {
		char best;
		if (chase(best))
			attemptMove(best);

		m_turns = 2;
	}
}
void Charger::attack(Player &p) {
	int px = p.getPosX();
	int py = p.getPosY();

	int steps = 3;

	int n, m;
	setDirectionalOffsets(m_direction, n, m);

	while (!(m_dungeon->wall(px + n, py + m) || m_dungeon->enemy(px + n, py + m)) && steps > 0 && !p.isHeavy()) {
		incrementDirectionalOffsets(m_direction, n, m);

		steps--;
	}

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// If player was knocked into a wall, they take additional damage
	if (m_dungeon->wall(px + n, py + m) && steps > 0 && !m_dungeon->getPlayer()->isHeavy())
		damage += 5;

	damagePlayer(damage, getDamageType());

	decrementDirectionalOffsets(m_direction, n, m);
	p.moveTo(px + n, py + m);

	if (m_dungeon->trap(px + n, py + m))
		m_dungeon->trapEncounter(px + n, py + m);
}

Serpent::Serpent(Dungeon *dungeon, int &x, int &y, int turns) : Monster(dungeon, x, y, 8 + randInt(3), 1, 6, 2, SERPENT), m_turns(turns), m_maxTurns(turns) {
	setImageName("Red_Fish_48x48.png");
	setGold(7);
	setChasesPlayer(true);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	// Set the tail position, if no space is valid then
	// the Serpent's x and y gets re-rolled and we try again
	while (!setTailPosition()) {
		rerollMonsterPosition(x, y);
		setPosX(x);
		setPosY(y);
	}

	addSegments();
}
Serpent::Serpent(Dungeon *dungeon, int &x, int &y, int turns, std::string name, std::string image, int hp, int armor, int str, int dex)
	: Monster(dungeon, x, y, hp, armor, str, dex, name), m_turns(turns), m_maxTurns(turns) {
	setImageName(image);
	setChasesPlayer(true);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	// Set the tail position, if no space is valid then
	// the Serpent's x and y gets re-rolled and we try again
	while (!setTailPosition()) {
		rerollMonsterPosition(x, y);
		setPosX(x);
		setPosY(y);
	}

	addSegments();
}

bool Serpent::setTailPosition() {
	int x = getPosX();
	int y = getPosY();

	std::vector<std::pair<int, int>> coords;
	coords.push_back(std::make_pair(x - 1, y));
	coords.push_back(std::make_pair(x + 1, y));
	coords.push_back(std::make_pair(x, y - 1));
	coords.push_back(std::make_pair(x, y + 1));

	int index;
	int n, m;
	while (!coords.empty()) {
		index = randInt((int)coords.size());
		n = coords[index].first; m = coords[index].second;

		// If this space is free, then set the tail here
		if (!(m_dungeon->wall(n, m) || m_dungeon->enemy(n, m) || m_dungeon->hero(n, m))) {
			m_tailX = n;
			m_tailY = m;
			return true;
		}

		coords.erase(coords.begin() + index);
	}

	return false;
}
void Serpent::rerollMonsterPosition(int &x, int &y) {
	int rows = m_dungeon->getRows();
	int cols = m_dungeon->getCols();

	x = 1 + randInt(cols - 2);
	y = 1 + randInt(rows - 2);

	while (m_dungeon->noSpawn(x, y) || m_dungeon->wall(x, y) || m_dungeon->enemy(x, y) || m_dungeon->hero(x, y)) {
		x = 1 + randInt(cols - 2);
		y = 1 + randInt(rows - 2);
	}
}

void Serpent::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].enemy = false;
	(*m_dungeon)[y*cols + x].enemy = true;

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	getSprite()->setLocalZOrder(y + Z_ACTOR);

	moveSegments(x, y);

	setPosX(x); setPosY(y);
}
void Serpent::move() {
	int cols = m_dungeon->getCols();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	if (m_turns > 0) {
		m_turns--;
		return;
	}

	if (playerInRange(m_range)) {
		char best;
		bool flipped = false; // Flag for indicating if the serpent did a head/tail swap move

		if (chase(best)) {
			if (!attemptMove(best)) {
				int n, m;
				setDirectionalOffsets(best, n, m);

				// Check if this move went on top of its tail, if so then the serpent can perform this action
				if (x + n == m_tail->getPosX() && y + m == m_tail->getPosY()) {
					setPosX(x + n); setPosY(y + m);
					m_dungeon->queueMoveSprite(getSprite(), x + n, y + m);
					flipped = true;

					// No need to update the enemy flags since we're simply switching the head and tail
				}
			}
		}
		else {
			attemptGreedyChase();
		}

		// Check if serpent moved, if so, update tail coordinates to where the head was
		if (x != getPosX() || y != getPosY()) {

			// If the serpent didn't flip its head and tail, then update location flags
			if (!flipped) {
				(*m_dungeon)[m_tail->getPosY()*cols + m_tail->getPosX()].enemy = false;
				(*m_dungeon)[y*cols + x].enemy = true;
			}

			m_dungeon->queueMoveSprite(m_tail->getSprite(), x, y);
			m_tail->setPosX(x);
			m_tail->setPosY(y);
			m_tail->setParentX(getPosX());
			m_tail->setParentY(getPosY());
		}

	}

	m_turns = m_maxTurns;
}
void Serpent::extraAttackEffects() {
	int roll = randReal(1, 100) + m_dungeon->getPlayer()->getLuck();

	if (roll < 50)
		m_dungeon->getPlayer()->addAffliction(std::make_shared<Bleed>());
}
void Serpent::extraDeathEffects() {
	// Destroy the tail upon death
	m_tail->setDestroyed(true);
}

void Serpent::moveSegments(int x, int y) {

	// x and y is where the parent (head) is moving to

	// If the parent's previous position is not its current position, then move the tail
	if (x != getPosX() || y != getPosY()) {
		int cols = m_dungeon->getCols();

		// If the parent's new position did not switch with its tail, then update enemy flag
		if (x != m_tail->getPosX() || y != m_tail->getPosY())
			(*m_dungeon)[m_tail->getPosY()*cols + m_tail->getPosX()].enemy = false;

		(*m_dungeon)[y*cols + x].enemy = true;

		m_dungeon->queueMoveSprite(m_tail->getSprite(), getPosX(), getPosY(), 0.10f);
		getSprite()->setLocalZOrder(getPosY() + Z_ACTOR);

		m_tail->setPosX(getPosX());
		m_tail->setPosY(getPosY());

		m_tail->setParentX(x);
		m_tail->setParentY(y);
	}
}
void Serpent::addSegments() {
	std::shared_ptr<MonsterSegment> segment = std::make_shared<MonsterSegment>(m_dungeon, m_tailX, m_tailY, getPosX(), getPosY(), SERPENT_TAIL, "Bone_32x32.png");
	m_dungeon->addMonster(segment);
	m_tail = segment;
}

ArmoredSerpent::ArmoredSerpent(Dungeon *dungeon, int x, int y, int turns)
	: Serpent(dungeon, x, y, turns, ARMORED_SERPENT, "Red_Fish_48x48.png", 10 + randInt(10), 2, 8, 1) {
	setGold(15);
}

bool ArmoredSerpent::canBeDamaged(DamageType type) {
	if (type == DamageType::EXPLOSIVE || type == DamageType::MAGICAL)
		return true;

	return false;
}

CombustionGolem::CombustionGolem(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 12, 1, 6, 2, COMBUSTION_GOLEM) {
	m_maxWait = m_wait;
	setGold(7);
	setCanBeBurned(false);
	setLavaImmunity(true);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

bool CombustionGolem::canBeDamaged(DamageType type) {
	if (type != DamageType::EXPLOSIVE)
		return true;

	return false;
}
void CombustionGolem::move() {
	if (m_eruptCooldown > 0)
		m_eruptCooldown--;

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	m_wait = m_maxWait;

	if (playerIsAdjacent()) {
		attack(*m_dungeon->getPlayer());
		return;
	}

	if (m_eruptCooldown == 0 && playerInRange(m_attackRange)) {
		playSound("Earthquake_Spell2.mp3");

		m_dungeon->addTrap(std::make_shared<MiniEruption>(*m_dungeon, getPosX(), getPosY()));
		m_eruptCooldown = 7;
		return;
	}

	if (playerInRange(m_moveRange)) {
		attemptGreedyChase();
		return;
	}
}
void CombustionGolem::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	m_dungeon->addTrap(std::make_shared<Combustion>(*m_dungeon, x, y));
}

OldSmokey::OldSmokey(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 120, 2, 0, 0, OLD_SMOKEY) {
	m_maxWait = m_wait;
	setGold(1);
	setSmart(true);
	setImageName("Tusked_Demon.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void OldSmokey::move() {

	if (randReal(1, 100) > 95) {
		// Spawn smoke
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	moveMonsterRandomly();

	m_wait = m_maxWait;
}
void OldSmokey::deathDrops() {
	m_dungeon->getPlayer()->decreaseFavorBy(2);
}


//				:::: FLOOR V ::::
Puff::Puff(Dungeon *dungeon, int x, int y, int turns, int pushDist, int range)
	: Monster(dungeon, x, y, 5 + randInt(5), 1, 1, 1, PUFF), m_turns(turns), m_maxTurns(turns), m_pushDist(pushDist), m_range(range) {
	setFlying(true);
	setImageName("Small_White_Ghost.png");
	setGold(4);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
Puff::Puff(Dungeon *dungeon, int x, int y, int turns, int pushDist, int range, std::string name, std::string image, int hp, int armor, int str, int dex)
	: Monster(dungeon, x, y, hp, armor, str, dex, name), m_turns(turns), m_maxTurns(turns), m_pushDist(pushDist), m_range(range) {
	setFlying(true);
	setImageName(image);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Puff::move() {
	int rows = m_dungeon->getRows();
	int cols = m_dungeon->getCols();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	if (m_turns > 0) {
		m_turns--;
		return;
	}

	// play wind sound if player is close enough
	float range = abs(px - x) + abs(py - y);
	if (range <= 10) {
		float volume = exp(-(range / 1.5));
		cocos2d::experimental::AudioEngine::play2d("Wind_Spell1.mp3", false, volume);
	}

	// Flags for indicating that a path is blocked
	bool l_wall, r_wall, u_wall, d_wall, tl_wall, tr_wall, bl_wall, br_wall;
	l_wall = r_wall = u_wall = d_wall = tl_wall = tr_wall = bl_wall = br_wall = false;

	char move = '-';
	int k = 1;
	int ax = x, ay = y;
	while (k <= m_pushDist) {
		// If player is adjacent, push them
		if (!u_wall && y - k > 0) {
			if (m_dungeon->wall(x, y - k)) {
				u_wall = true;
			}
			else if (m_dungeon->hero(x, y - k) || m_dungeon->enemy(x, y - k)) {
				move = 'u';
				ax = x;
				ay = y - k;
				m_dungeon->linearActorPush(ax, ay, m_pushDist - k + 1, m_range, move);
			}
		}
		if (!d_wall && y + k < rows) {
			if (m_dungeon->wall(x, y + k)) {
				d_wall = true;
			}
			else if (m_dungeon->hero(x, y + k) || m_dungeon->enemy(x, y + k)) {
				move = 'd';
				ax = x;
				ay = y + k;
				m_dungeon->linearActorPush(ax, ay, m_pushDist - k + 1, m_range, move);
			}
		}
		if (!l_wall && x - k > 0) {
			if (m_dungeon->wall(x - k, y)) {
				l_wall = true;
			}
			else if (m_dungeon->hero(x - k, y) || m_dungeon->enemy(x - k, y)) {
				move = 'l';
				ax = x - k;
				ay = y;
				m_dungeon->linearActorPush(ax, ay, m_pushDist - k + 1, m_range, move);
			}

		}
		if (!r_wall && x + k < cols) {
			if (m_dungeon->wall(x + k, y)) {
				r_wall = true;
			}
			else if (m_dungeon->hero(x + k, y) || m_dungeon->enemy(x + k, y)) {
				move = 'r';
				ax = x + k;
				ay = y;
				m_dungeon->linearActorPush(ax, ay, m_pushDist - k + 1, m_range, move);
			}

		}

		if (pushesAll()) {
			if (!tl_wall && x - k > 0 && y - k > 0) {
				if (m_dungeon->wall(x - k, y - k)) {
					tl_wall = true;
				}
				else if (m_dungeon->hero(x - k, y - k) || m_dungeon->enemy(x - k, y - k)) {
					move = '1';
					ax = x - k;
					ay = y - k;
					m_dungeon->linearActorPush(ax, ay, m_pushDist - k + 1, m_range, move);
				}
			}
			if (!tr_wall && x + k > 0 && y - k > 0) {
				if (m_dungeon->wall(x + k, y - k)) {
					tr_wall = true;
				}
				else if (m_dungeon->hero(x + k, y - k) || m_dungeon->enemy(x + k, y - k)) {
					move = '2';
					ax = x + k;
					ay = y - k;
					m_dungeon->linearActorPush(ax, ay, m_pushDist - k + 1, m_range, move);
				}
			}
			if (!bl_wall && x - k > 0 && y + k > 0) {
				if (m_dungeon->wall(x - k, y + k)) {
					bl_wall = true;
				}
				else if (m_dungeon->hero(x - k, y + k) || m_dungeon->enemy(x - k, y + k)) {
					move = '3';
					ax = x - k;
					ay = y + k;
					m_dungeon->linearActorPush(ax, ay, m_pushDist - k + 1, m_range, move);
				}
			}
			if (!br_wall && x + k > 0 && y + k > 0) {
				if (m_dungeon->wall(x + k, y + k)) {
					br_wall = true;
				}
				else if (m_dungeon->hero(x + k, y + k) || m_dungeon->enemy(x + k, y + k)) {
					move = '4';
					ax = x + k;
					ay = y + k;
					m_dungeon->linearActorPush(ax, ay, m_pushDist - k + 1, m_range, move);
				}
			}
		}

		k++;
	}

	m_turns = m_maxTurns;
}

GustyPuff::GustyPuff(Dungeon *dungeon, int x, int y) : Puff(dungeon, x, y, 3, 1, 1, GUSTY_PUFF, "Small_White_Ghost.png", 8 + randInt(6), 1, 1, 1) {
	setGold(6);
}

StrongGustyPuff::StrongGustyPuff(Dungeon *dungeon, int x, int y) : Puff(dungeon, x, y, 4, 2, 2, STRONG_GUSTY_PUFF, "Tribal_Orc_48x48.png", 12 + randInt(6), 1, 1, 2) {
	setGold(12);
}

InvertedPuff::InvertedPuff(Dungeon *dungeon, int x, int y, int turns, int pullDist, int range)
	: Monster(dungeon, x, y, 4 + randInt(6), 1, 1, 1, INVERTED_PUFF), m_turns(turns), m_maxTurns(turns), m_pullDist(pullDist), m_range(range) {
	setFlying(true);
	setImageName("Small_White_Ghost.png");
	setGold(4);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
InvertedPuff::InvertedPuff(Dungeon *dungeon, int x, int y, int turns, int pullDist, int range, std::string name, std::string image, int hp, int armor, int str, int dex)
	: Monster(dungeon, x, y, hp, armor, str, dex, name), m_turns(turns), m_maxTurns(turns), m_pullDist(pullDist), m_range(range) {
	setFlying(true);
	setImageName(image);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void InvertedPuff::move() {
	int rows = m_dungeon->getRows();
	int cols = m_dungeon->getCols();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	if (m_turns > 0) {
		m_turns--;
		return;
	}

	// play wind sound if player is close enough
	float range = abs(px - x) + abs(py - y);
	if (range <= 10) {
		float volume = exp(-(range / 1.5));
		cocos2d::experimental::AudioEngine::play2d("Wind_Spell2.mp3", false, volume);
	}

	// Flags for indicating that a path is blocked
	bool l_wall, r_wall, u_wall, d_wall, tl_wall, tr_wall, bl_wall, br_wall;
	l_wall = r_wall = u_wall = d_wall = tl_wall = tr_wall = bl_wall = br_wall = false;

	char move = '-';
	int k = m_pullDist;
	int ax = x, ay = y;
	while (k > 0) {
		// If player is adjacent, push them
		if (!u_wall && y - k > 0) {
			if (m_dungeon->wall(x, y - k))
				u_wall = true;

			else if (m_dungeon->hero(x, y - k) || m_dungeon->enemy(x, y - k)) {
				move = 'd';
				ax = x;
				ay = y - k;
				m_dungeon->linearActorPush(ax, ay, m_pullDist, m_range, move, true);
			}
		}
		if (!d_wall && y + k < rows) {
			if (m_dungeon->wall(x, y + k))
				d_wall = true;

			else if (m_dungeon->hero(x, y + k) || m_dungeon->enemy(x, y + k)) {
				move = 'u';
				ax = x;
				ay = y + k;
				m_dungeon->linearActorPush(ax, ay, m_pullDist, m_range, move, true);
			}
		}
		if (!l_wall && x - k > 0) {
			if (m_dungeon->wall(x - k, y))
				l_wall = true;

			else if (m_dungeon->hero(x - k, y) || m_dungeon->enemy(x - k, y)) {
				move = 'r';
				ax = x - k;
				ay = y;
				m_dungeon->linearActorPush(ax, ay, m_pullDist, m_range, move, true);
			}

		}
		if (!r_wall && x + k < cols) {
			if (m_dungeon->wall(x + k, y))
				r_wall = true;

			else if (m_dungeon->hero(x + k, y) || m_dungeon->enemy(x + k, y)) {
				move = 'l';
				ax = x + k;
				ay = y;
				m_dungeon->linearActorPush(ax, ay, m_pullDist, m_range, move, true);
			}

		}

		if (pullsAll()) {
			if (!tl_wall && x - k > 0 && y - k > 0) {
				if (m_dungeon->wall(x - k, y - k))
					tl_wall = true;

				else if (m_dungeon->hero(x - k, y - k) || m_dungeon->enemy(x - k, y - k)) {
					move = '4';
					ax = x - k;
					ay = y - k;
					m_dungeon->linearActorPush(ax, ay, m_pullDist, m_range, move, true);
				}
			}
			if (!tr_wall && x + k > 0 && y - k > 0) {
				if (m_dungeon->wall(x + k, y - k))
					tr_wall = true;

				else if (m_dungeon->hero(x + k, y - k) || m_dungeon->enemy(x + k, y - k)) {
					move = '3';
					ax = x + k;
					ay = y - k;
					m_dungeon->linearActorPush(ax, ay, m_pullDist, m_range, move, true);
				}
			}
			if (!bl_wall && x - k > 0 && y + k > 0) {
				if (m_dungeon->wall(x - k, y + k))
					bl_wall = true;

				else if (m_dungeon->hero(x - k, y + k) || m_dungeon->enemy(x - k, y + k)) {
					move = '2';
					ax = x - k;
					ay = y + k;
					m_dungeon->linearActorPush(ax, ay, m_pullDist, m_range, move, true);
				}
			}
			if (!br_wall && x + k > 0 && y + k > 0) {
				if (m_dungeon->wall(x + k, y + k))
					br_wall = true;

				else if (m_dungeon->hero(x + k, y + k) || m_dungeon->enemy(x + k, y + k)) {
					move = '1';
					ax = x + k;
					ay = y + k;
					m_dungeon->linearActorPush(ax, ay, m_pullDist, m_range, move, true);
				}
			}
		}

		k--;
	}

	// Perform any special moves unique to this puff, if any
	specialMove();

	m_turns = m_maxTurns;
}

SpikedInvertedPuff::SpikedInvertedPuff(Dungeon *dungeon, int x, int y) : InvertedPuff(dungeon, x, y, 4, 2, 1, SPIKED_INVERTED_PUFF, "White_Ghost.png", 8 + randInt(6), 2, 5, 1) {
	setGold(10);
}
SpikedInvertedPuff::SpikedInvertedPuff(Dungeon *dungeon, int x, int y, int turns, int pullDist, int range, std::string name, std::string image, int hp, int armor, int str, int dex)
	: InvertedPuff(dungeon, x, y, 4, 2, range, name, image, hp, armor, str, dex) {

}

void SpikedInvertedPuff::attack(Player &p) {
	int damage = std::max(1, randInt(getStr()));
	damagePlayer(damage, getDamageType());
}
void SpikedInvertedPuff::specialMove() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// If the player was sucked into the puff, damage them
	if (px == x && abs(py - y) == 1 || py == y && abs(px - x) == 1)
		attack(*m_dungeon->getPlayer());

}

GustySpikedInvertedPuff::GustySpikedInvertedPuff(Dungeon *dungeon, int x, int y) : SpikedInvertedPuff(dungeon, x, y, 4, 2, 1, GUSTY_SPIKED_INVERTED_PUFF, "White_Ghost.png", 8 + randInt(6), 2, 7, 1) {
	setGold(12);
}

void GustySpikedInvertedPuff::specialMove() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// If the player was sucked into the puff, damage them
	if (abs(px - x) <= 1 && abs(py - y) <= 1) {
		attack(*m_dungeon->getPlayer());
	}
}

JeweledScarab::JeweledScarab(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 2, 0, 5, 3, JEWELED_SCARAB) {
	setFlying(true);
	setImageName("Tusked_Demon.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void JeweledScarab::move() {
	int x = getPosX();
	int y = getPosY();

	bool moved = false;

	std::vector<std::pair<int, int>> coords;
	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {

			// Skips anything that is not a knight style move
			if (!m_dungeon->withinBounds(i, j) || abs(i - x) + abs(j - y) != 3)
				continue;

			coords.push_back(std::make_pair(i, j));
		}
	}

	coordCheck(coords, moved);

	if (moved)
		return;

	// Diagonals next
	coords.push_back(std::make_pair(x - 1, y - 1));
	coords.push_back(std::make_pair(x - 1, y + 1));
	coords.push_back(std::make_pair(x + 1, y - 1));
	coords.push_back(std::make_pair(x + 1, y + 1));

	coordCheck(coords, moved);

	if (moved)
		return;

	// Cardinals last
	coords.push_back(std::make_pair(x - 1, y));
	coords.push_back(std::make_pair(x - 1, y));
	coords.push_back(std::make_pair(x, y - 1));
	coords.push_back(std::make_pair(x, y + 1));

	coordCheck(coords, moved);
}
void JeweledScarab::coordCheck(std::vector<std::pair<int, int>> &coords, bool &moved) {
	for (unsigned int i = 0; i < coords.size(); i++) {
		int index = randInt((int)coords.size());
		int n = coords[index].first;
		int m = coords[index].second;

		if (m_dungeon->hero(n, m)) {
			damagePlayer(getStr(), getDamageType());

			moveTo(n, m, 0.05f);
			setDestroyed(true);

			moved = true;
			return;
		}

		if (!(m_dungeon->enemy(n, m) || m_dungeon->wall(n, m))) {
			moveTo(n, m);

			moved = true;
			return;
		}

		coords.erase(coords.begin() + index);
	}
}
void JeweledScarab::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->itemObject(x, y))
		m_dungeon->itemHash(x, y);

	m_dungeon->createItem(rollPassive(m_dungeon, x, y, Rarity::ALL, true));
}

ArmoredBeetle::ArmoredBeetle(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 3, 5, 0, ARMORED_BEETLE) {
	m_wait = 3;
	m_maxWait = m_wait;
	setGold(5);
	setImageName("Tusked_Demon.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

bool ArmoredBeetle::canBeDamaged(DamageType type) {
	switch (type) {
	case DamageType::NORMAL: return false;
	}

	return true;
}
void ArmoredBeetle::move() {
	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerIsAdjacent())
		attack(*m_dungeon->getPlayer());
	else
		moveMonsterRandomly();

	m_wait = m_maxWait;
}

SpectralSword::SpectralSword(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 20, 1, 10, 5, SPECTRAL_SWORD) {
	setEthereal(true);
	setFlying(true);
	setImageName("Swift_Sword_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

bool SpectralSword::canBeDamaged(DamageType type) {
	if (type == DamageType::EXPLOSIVE || type == DamageType::MAGICAL)
		return true;

	m_attacked = true;
	m_dir = getDirectionOppositeTo(m_dungeon->getPlayer()->facingDirection());
	return false;
}
void SpectralSword::move() {
	int x = getPosX();
	int y = getPosY();

	if (m_attacked) {

		if (!m_primed) {
			m_primed = true;
			return;
		}

		playMiss();

		int n, m;
		setDirectionalOffsets(m_dir, n, m);

		auto frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);

		// Player attacked to the side, so check above and below
		if (m == 0) {
			m_dungeon->runSingleAnimation(frames, 120, x + n, y + 1, 2);
			m_dungeon->runSingleAnimation(frames, 120, x + n, y - 1, 2);

			if (m_dungeon->hero(x + n, y + m) || m_dungeon->hero(x + n, y + 1) || m_dungeon->hero(x + n, y - 1))
				attack(*m_dungeon->getPlayer());
		}
		// Else player attacked above or below, so check the sides
		else {
			m_dungeon->runSingleAnimation(frames, 120, x + 1, y + m, 2);
			m_dungeon->runSingleAnimation(frames, 120, x - 1, y + m, 2);

			if (m_dungeon->hero(x + n, y + m) || m_dungeon->hero(x + 1, y + m) || m_dungeon->hero(x - 1, y + m))
				attack(*m_dungeon->getPlayer());
		}

		setDestroyed(true);
		return;
	}

	attemptGreedyChase();
}
void SpectralSword::reactToDamage() {
	m_attacked = true;
	m_dir = getDirectionOppositeTo(m_dungeon->getPlayer()->facingDirection());
}

SandCentipede::SandCentipede(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 1, 1, 1, 2, SAND_CENTIPEDE) {
	m_wait = 2;
	m_maxWait = m_wait;
	setGold(5);
	setImageName("Skull_Decor_16x16.png");
	setHeavy(true);

	setSprite(dungeon->createSprite(x, y, y + Z_TRAP_BOTTOM + 1, getImageName()));
	(*dungeon)[y*dungeon->getCols() + x].enemy = false;
	(*dungeon)[y*dungeon->getCols() + x].underEnemy = true;
}

void SandCentipede::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].underEnemy = false;
	(*m_dungeon)[y*cols + x].underEnemy = true;

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);
	getSprite()->setLocalZOrder(y + Z_TRAP_BOTTOM + 1);
}
bool SandCentipede::canBeDamaged(DamageType type) {
	if (m_wait < m_maxWait || type == DamageType::MAGICAL)
		return true;

	return false;
}
bool SandCentipede::canBeHit() {
	if (m_wait < m_maxWait)
		return true;

	return false;
}
bool SandCentipede::isUnderground() {
	if (m_wait < m_maxWait)
		return false;

	return true;
}
void SandCentipede::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (!m_primed && x == px && y == py)
		m_primed = true;

	if (m_wait > 0 && m_wait < m_maxWait) {
		m_wait--;
		return;
	}

	if (m_wait == 0) {
		m_dungeon->queueRemoveSprite(getSprite());
		setSprite(m_dungeon->createSprite(x, y, y + Z_TRAP_BOTTOM + 1, "Skull_Decor_16x16.png"));

		m_wait = m_maxWait;
		m_primed = false;
		(*m_dungeon)[y*m_dungeon->getCols() + x].enemy = false;
	}

	if (m_primed) {

		if (m_dungeon->enemy(x, y) || m_dungeon->wall(x, y)) {
			m_primed = false;
			return;
		}

		m_dungeon->queueRemoveSprite(getSprite());
		setSprite(m_dungeon->createSprite(x, y, y + Z_ACTOR, "Cheese_Wedge_48x48.png"));

		if (x == px && y == py)
			damagePlayer(getStr(), getDamageType());

		(*m_dungeon)[y*m_dungeon->getCols() + x].enemy = true;
		m_wait--;
		return;
	}

	/*if (playerIsAdjacent() && !m_dungeon->underEnemy(px, py)) {
		moveTo(px, py);

		m_primed = true;
		return;
	}*/

	/*if (playerInRange(m_range)) {
		char best;
		if (chase(best)) {
			int n, m;
			setDirectionalOffsets(best, n, m);

			if (!m_dungeon->underEnemy(x + n, y + m))
				moveTo(x + n, y + m);
		}
	}*/

	if (m_dungeon->trap(px, py)) {
		std::vector<int> indexes = m_dungeon->findTraps(px, py);
		for (unsigned int i = 0; i < indexes.size(); i++) {
			if (m_dungeon->trapAt(indexes[i])->getName() == SAND) {
				char move = attemptDryRunGreedyChase();

				int n, m;
				setDirectionalOffsets(move, n, m);

				if (m_dungeon->trap(x + n, y + m)) {
					std::vector<int> idxs = m_dungeon->findTraps(x + n, y + m);
					for (int pos = 0; pos < (int)idxs.size(); pos++) {
						if (m_dungeon->trapAt(idxs[pos])->getName() == SAND) {

							if (!(m_dungeon->underEnemy(x + n, y + m)))
								moveTo(x + n, y + m);

							if (m_dungeon->hero(x + n, y + m))
								m_primed = true;

							return;
						}
					}
				}
			}
		}
	}
}
void SandCentipede::extraDeathEffects() {
	(*m_dungeon)[getPosY()*m_dungeon->getCols() + getPosX()].underEnemy = false;
}

SandBeaver::SandBeaver(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 8, 2, SAND_BEAVER) {
	m_maxWait = m_wait;
	setGold(5);
	setImageName("Tusked_Demon.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void SandBeaver::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (!m_primed && playerInBufferedLinearRange(m_attackRange, 1, x, y, px, py) && hasLineOfSight(*m_dungeon, x, y, px, py)) {
		playSound("Goo_Sack_Primed.mp3");

		m_primed = true;
		m_dir = getCardinalFacingDirectionRelativeTo(x, y, px, py);
		return;
	}

	if (m_primed) {
		playSound("Wind_Spell1.mp3", *m_dungeon->getPlayer(), x, y);

		auto frames = getAnimationFrameVector("frame%04d.png", 63);

		int n, m;
		setDirectionalOffsets(m_dir, n, m);

		m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);
		if (m_dungeon->hero(x + n, y + m) || m_dungeon->enemy(x + n, y + m))
			m_dungeon->linearActorPush(x + n, y + m, 3, 3, m_dir);

		if (m_dungeon->hero(x + n, y + m))
			attack(*m_dungeon->getPlayer());

		if (m_dungeon->trap(x + n, y + m)) {
			std::vector<int> indexes = m_dungeon->findTraps(x, y);
			for (int i = 0; i < (int)indexes.size(); i++) {
				if (m_dungeon->trapAt(indexes.at(i))->getName() == SAND) {
					m_dungeon->getPlayer()->addAffliction(std::make_shared<Blindness>(10));
					/*auto smoke = cocos2d::ParticleSmoke::createWithTotalParticles(40);
					m_dungeon->m_scene->addChild(smoke, 10);

					float _x, _y;
					m_dungeon->transformDungeonToSpriteCoordinates(x, y, _x, _y);
					smoke->setSourcePosition(cocos2d::Vec2(_x, _y));*/
				}
			}
		}

		switch (m_dir) {
		case 'l': n = -2; m = -1; break;
		case 'r': n = 2; m = -1; break;
		case 'u': n = -1; m = -2; break;
		case 'd': n = -1; m = 2; break;
		}

		// 2 rows/columns
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 3; j++) {

				if (m_dungeon->withinBounds(x + n, y + m)) {

					m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, 2);

					if (m_dungeon->hero(x + n, y + m) || m_dungeon->enemy(x + n, y + m))
						m_dungeon->linearActorPush(x + n, y + m, 3 - (j + 1) + 1, 3, getFacingDirectionRelativeTo(x, y, x + n, y + m));
					
				}

				switch (m_dir) {
				case 'l': m++; break;
				case 'r': m++; break;
				case 'u': n++; break;
				case 'd': n++; break;
				}

			}

			switch (m_dir) {
			case 'l': n--; m = -1; break;
			case 'r': n++; m = -1; break;
			case 'u': n = -1; m--; break;
			case 'd': n = -1; m++; break;
			}
		}

		m_primed = false;
		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerInRange(m_chaseRange)) {
		attemptGreedyChase();

		m_wait = m_maxWait;
		return;
	}

	if (playerInRange(m_moveRange)) {
		moveMonsterRandomly();
		m_wait = m_maxWait;
	}
}

SandAlbatross::SandAlbatross(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 8, 2, SAND_ALBATROSS) {
	m_maxWait = m_wait;
	setGold(5);
	setFlying(true);
	setImageName("Tusked_Demon.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void SandAlbatross::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (m_primed) {
		m_primed = false;
		m_wait = m_maxWait;

		if (playerIsAdjacent()) {
			attack(*m_dungeon->getPlayer());

			return;
		}

		int n, m;
		setDirectionalOffsets(m_dir, n, m);

		m_dungeon->addTrap(std::make_shared<DustDevil>(*m_dungeon, x + n, y + m, m_dir));

		return;
	}

	if (m_moveTurns == 0) {
		if (playerInBufferedLinearRange(m_range, 1, getPosX(), getPosY(), px, py))
			m_primed = true;

		m_moveTurns = 2;
		return;
	}

	if (m_moveTurns > 0 && playerInRange(m_range)) {
		randReal(1, 100) > 50 ? run() : moveSideways();
		m_dir = getCardinalFacingDirectionRelativeTo(getPosX(), getPosY(), px, py);

		m_moveTurns--;
	}
}
void SandAlbatross::run() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int n, m;
	int fx = x, fy = y; // final x and y coordinates

	for (n = -1, m = 0; n < 2; n += 2) {
		// if space is free and distance is farthest, set new move
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}
	}

	for (n = 0, m = -1; m < 2; m += 2) {
		// if space is free and distance is farthest, set new move
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}
	}

	moveTo(fx, fy);
}
void SandAlbatross::moveSideways() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int n, m;
	char move = getFacingDirectionRelativeTo(x, y, px, py);
	switch (move) {
	case 'l':
	case 'r': move = randInt(2) == 0 ? 'u' : 'd'; break;
	case 'u':
	case 'd': move = randInt(2) == 0 ? 'l' : 'r'; break;
	case '1': move = randInt(2) == 0 ? 'l' : 'u'; break;
	case '2': move = randInt(2) == 0 ? 'r' : 'u'; break;
	case '3': move = randInt(2) == 0 ? 'l' : 'd'; break;
	case '4': move = randInt(2) == 0 ? 'r' : 'd'; break;
	}

	setDirectionalOffsets(move, n, m);

	if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)))
		moveTo(x + n, y + m);

}

Archer::Archer(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, randInt(5) + 9, 2, 3, 2, std::make_shared<WoodBow>(0, 0), ARCHER) {
	setImageName("Archer_48x48.png");
	setGold(4);
	setSturdy(false);
	setHasWeapon(true);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Archer::move() {
	int cols = m_dungeon->getCols();

	int x = m_dungeon->getPlayer()->getPosX();
	int y = m_dungeon->getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	//	if archer is primed, check to attack and return
	if (m_primed) {
		// if player is in sight, shoot them
		if (x - mx == 0 || y - my == 0) {
			playSound("Bow_Release.mp3");

			attack(*m_dungeon->getPlayer());
		}
		// otherwise relax their aim
		else {
			;
		}

		m_primed = false;

		return;
	}


	// if archer has dagger out and is adjacent to player, attack player
	if (getWeapon()->getName() == BRONZE_DAGGER && playerIsAdjacent()) {
		// player archer dagger swipe sound effect
		playArcherDaggerSwipe();

		attack(*m_dungeon->getPlayer());
		return;
	}
	// else if dagger is out and player moved away, switch back to bow
	else if (getWeapon()->getName() == BRONZE_DAGGER && !playerIsAdjacent()) {
		playSound("Bow_Primed.mp3");

		setWeapon(std::make_shared<WoodBow>(0, 0));
		return;
	}

	// if player is invisible, move randomly
	if (m_dungeon->getPlayer()->isInvisible()) {
		moveMonsterRandomly();
		return;
	}

	// if player is not in the archer's sights, move archer
	if (x != mx && y != my) {
		moveMonsterRandomly();

		return;
	}

	// if player and archer are on the same column
	if (x == mx) {
		//	if player is within range and not adjacent, get primed
		if (abs(y - my) > 1 && abs(y - my) <= m_range) {
			if (hasLineOfSight(*m_dungeon, mx, my, x, y)) {
				playSound("Bow_Drawn.mp3");

				m_primed = true;
			}
			else {
				moveMonsterRandomly();

				return;
			}
		}
		// swap weapon to dagger
		else if (abs(y - my) == 1) {
			playSound("Archer_Dagger_Switch2.mp3");

			setWeapon(std::make_shared<BronzeDagger>(0, 0));
		}
		//	just move archer
		else {
			moveMonsterRandomly();

			return;
		}
	}
	// if player and archer are on the same row
	else if (y == my) {
		//	if player is within range and not adjacent, get primed
		if (abs(x - mx) > 1 && abs(x - mx) <= m_range) {
			if (hasLineOfSight(*m_dungeon, mx, my, x, y)) {
				playSound("Bow_Drawn.mp3");

				m_primed = true;
			}
			else {
				moveMonsterRandomly();

				return;
			}
		}
		// swap weapon to dagger
		else if (abs(x - mx) == 1) {
			playSound("Archer_Dagger_Switch2.mp3");

			setWeapon(std::make_shared<BronzeDagger>(0, 0));
		}
		// just move archer
		else {
			moveMonsterRandomly();
		}
	}

}
void Archer::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// 5% drop chance
	if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 95) {
		if (m_dungeon->itemObject(x, y))
			m_dungeon->itemHash(x, y);

		m_dungeon->createItem(std::make_shared<BronzeDagger>(x, y));
	}
}


//				:::: FLOOR VI ::::
Wisp::Wisp(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 1, 2, WISP) {
	m_maxWait = m_wait;
	setGold(5);
	setImageName("Spinner_Buddy_48x48.png");
	setFlying(true);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Wisp::move() {
	if (m_wait > 0) {
		m_wait--;
		moveMonsterRandomly();
		return;
	}

	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (playerInRange(m_range) || playerInDiagonalRange(m_range)) {
		char dir = getFacingDirectionRelativeTo(x, y, px, py);
		int n, m;
		setDirectionalOffsets(dir, n, m);

		char helixDir1, helixDir2;
		switch (dir) {
		case 'l': helixDir1 = '1'; helixDir2 = '3'; break;
		case 'r': helixDir1 = '2'; helixDir2 = '4'; break;
		case 'u': helixDir1 = '1'; helixDir2 = '2'; break;
		case 'd': helixDir1 = '3'; helixDir2 = '4'; break;
		case '1': helixDir1 = 'l'; helixDir2 = 'u'; break;
		case '2': helixDir1 = 'r'; helixDir2 = 'u'; break;
		case '3': helixDir1 = 'l'; helixDir2 = 'd'; break;
		case '4': helixDir1 = 'r'; helixDir2 = 'd'; break;
		}

		m_dungeon->addTrap(std::make_shared<EnergyHelix>(*m_dungeon, x + n, y + m, helixDir1, helixDir2));
		m_dungeon->addTrap(std::make_shared<EnergyHelix>(*m_dungeon, x + n, y + m, helixDir2, helixDir1));
	}

	m_wait = m_maxWait;
}

LightningWisp::LightningWisp(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 1, 2, LIGHTNING_WISP) {
	m_cardinal = 1 + randInt(2) == 1 ? true : false;
	m_maxWait = m_wait;
	setGold(5);
	setImageName("Spinner_Buddy_48x48.png");
	setFlying(true);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void LightningWisp::move() {
	if (m_wait > 0) {
		m_wait--;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	std::vector<std::pair<char, std::pair<int, int>>> coords;
	if (m_cardinal) {
		coords.push_back(std::make_pair('l', std::make_pair(x - 1, y)));
		coords.push_back(std::make_pair('r', std::make_pair(x + 1, y)));
		coords.push_back(std::make_pair('u', std::make_pair(x, y - 1)));
		coords.push_back(std::make_pair('d', std::make_pair(x, y + 1)));
	}
	else {
		coords.push_back(std::make_pair('4', std::make_pair(x + 1, y + 1)));
		coords.push_back(std::make_pair('3', std::make_pair(x - 1, y + 1)));
		coords.push_back(std::make_pair('2', std::make_pair(x + 1, y - 1)));
		coords.push_back(std::make_pair('1', std::make_pair(x - 1, y - 1)));
	}

	m_cardinal = !m_cardinal;

	while (!coords.empty()) {
		int index = randInt((int)coords.size());
		char dir = coords[index].first;
		int n = coords[index].second.first;
		int m = coords[index].second.second;

		setDirectionalOffsets(dir, n, m);

		if (!(m_dungeon->enemy(x + n, y + m) || m_dungeon->wall(x + n, y + m))) {
			if (m_dungeon->hero(x + n, y + m))
				damagePlayer(5, getDamageType());

			incrementDirectionalOffsets(dir, n, m);
			if (!(m_dungeon->enemy(x + n, y + m) || m_dungeon->wall(x + n, y + m))) {
				if (m_dungeon->hero(x + n, y + m))
					damagePlayer(5, getDamageType());

				moveTo(x + n, y + m);
				break;
			}

			decrementDirectionalOffsets(dir, n, m);
			moveTo(x + n, y + m);
			break;
		}

		coords.erase(coords.begin() + index);
	}

	m_wait = m_maxWait;
}

Grabber::Grabber(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 1, 2, GRABBER) {
	m_maxWait = m_wait;
	setGold(5);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Grabber::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_capturedPlayer) {
		if (m_grabTurns > 0) {
			m_dungeon->linearActorPush(px, py, 1, 1, m_dir, true);

			if (playerIsAdjacent())
				attack(*m_dungeon->getPlayer());

			m_grabTurns--;
			return;
		}

		m_capturedPlayer = false;
		m_grabTurns = 3;
		m_wait = m_maxWait;
		return;
	}

	if (m_primed) {
		m_primed = false;

		if (hasLineOfSight(*m_dungeon, *m_dungeon->getPlayer(), x, y)) {

			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			m_dungeon->runSingleAnimation(frames, 120, x, y, 2);

			if (m_dungeon->getPlayer()->canBlock() && m_dungeon->getPlayer()->didBlock(x, y)) {
				playHitSmasher();

				m_dungeon->getPlayer()->successfulBlock();
				m_wait = m_maxWait;
				return;
			}

			playGotHit();

			m_dungeon->getPlayer()->addAffliction(std::make_shared<Stuck>(3));
			m_capturedPlayer = true;
			m_dir = getFacingDirectionRelativeTo(px, py, x, y); // x/px and y/py are in switched positions because linearActorPush works in a weird way
			return;
		}

		m_wait = m_maxWait;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerInBufferedLinearRange(m_range, 1, x, y, px, py)) {
		int n = 0, m = 0;
		if (x != px && y != py) {
			if (x > px)
				n = -1;
			else if (x < px)
				n = 1;
			else if (y > py)
				m = -1;
			else
				m = 1;
		}

		if (isMovementAction(m_dungeon->getPlayer()->getAction()) && hasLineOfSight(*m_dungeon, *m_dungeon->getPlayer(), x + n, y + m)) {
			m_primed = true;
			return;
		}
	}

	if (playerInRange(m_range)) {
		char best;
		if (chase(best))
			attemptMove(best);
	}

	m_wait = m_maxWait;
}

EvilShadow::EvilShadow(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 1, 0, 20, 0, EVIL_SHADOW) {
	setGold(5);
	setImageName("Poison_Puddle.png");
	setDamageType(DamageType::MAGICAL);

	setSprite(dungeon->createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));

	// Trusts that EvilShadow is spawned underneath another enemy
	(*dungeon)[y*dungeon->getCols() + x].underEnemy = true;
}

void EvilShadow::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].underEnemy = false;
	(*m_dungeon)[y*cols + x].underEnemy = true;

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);
	getSprite()->setLocalZOrder(y + Z_TRAP_BOTTOM);
}
bool EvilShadow::canBeDamaged(DamageType type) {
	if (m_hittable)
		return true;

	return false;
}
bool EvilShadow::canBeHit() {
	if (m_hittable)
		return true;

	return false;
}
bool EvilShadow::isUnderground() {
	if (m_hittable)
		return false;

	return true;
}
void EvilShadow::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (!m_revealed) {
		if (!m_dungeon->enemy(x, y))
			m_revealed = true;

		return;
	}

	// Player didn't kill the shadow, so it deals unavoidable damage
	if (m_hittable) {
		m_dungeon->damagePlayer(getStr(), getDamageType());
		setDestroyed(true);
		return;
	}

	if (m_primed) {

		if (m_dungeon->enemy(x, y) || m_dungeon->wall(x, y)) {
			m_primed = false;
			return;
		}

		m_dungeon->queueRemoveSprite(getSprite());
		setSprite(m_dungeon->createSprite(x, y, y + Z_ACTOR, "Cheese_Wedge_48x48.png"));

		if (x == px && y == py) {
			m_dungeon->damagePlayer(getStr(), getDamageType());
			setDestroyed(true);
			return;
		}

		(*m_dungeon)[y*m_dungeon->getCols() + x].enemy = true;
		m_hittable = true;
		return;
	}

	if (playerInRange(m_range)) {
		char best;
		if (chase(best)) {
			int n, m;
			setDirectionalOffsets(best, n, m);

			if (!m_dungeon->underEnemy(x + n, y + m))
				moveTo(x + n, y + m);
		}
	}

	if (playerIsAdjacent() && !m_dungeon->enemy(px, py)) {
		m_primed = true;
		return;
	}
}

CrawlingSpine::CrawlingSpine(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 1, 2, CRAWLING_SPINE) {
	m_maxWait = m_wait;
	setGold(7);
	setImageName("Cheese_Wedge_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void CrawlingSpine::move() {
	if (m_primed) {
		m_dungeon->addTrap(std::make_shared<Goop>(*m_dungeon, m_px, m_py));
		m_primed = false;

		m_goopCooldown = 4;
		return;
	}

	if (playerInRange(m_range)) {
		if (m_wait > 0)
			m_wait--;

		if (m_wait == 0) {
			attemptGreedyChase();
			m_wait = m_maxWait;
		}
	}

	if (m_goopCooldown > 0) {
		m_goopCooldown--;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	for (int i = x - 3; i < x + 4; i++) {
		for (int j = y - 3; j < y + 4; j++) {
			if (!m_dungeon->withinBounds(i, j))
				continue;

			if (m_dungeon->hero(i, j)) {
				m_primed = true;
				m_px = i;
				m_py = j;
				return;
			}
		}
	}
}

ConstructorDemon::ConstructorDemon(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 1, 1, 2, CONSTRUCTOR_DEMON) {
	setGold(7);
	setImageName("Cheese_Wedge_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void ConstructorDemon::move() {
	if (m_cooldown > 0) {
		m_cooldown--;
		return;
	}

	if (!playerInRange(m_actionRange))
		return;

	if (!m_primed) {
		if (playerInRange(m_range))
			run();
		else
			moveSideways();

		m_primed = true;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int n, m;
	char move = getFacingDirectionRelativeTo(x, y, px, py);
	setDirectionalOffsets(move, n, m);

	if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {
		m_dungeon->addTrap(std::make_shared<DirtMound>(*m_dungeon, x + n, y + m));
		m_cooldown = 4;
	}

	m_primed = false;
}
void ConstructorDemon::run() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int n, m;
	int fx = x, fy = y; // final x and y coordinates

	for (n = -1, m = 0; n < 2; n += 2) {
		// if space is free and distance is farthest, set new move
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}
	}

	for (n = 0, m = -1; m < 2; m += 2) {
		// if space is free and distance is farthest, set new move
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}
	}

	moveTo(fx, fy);
}
void ConstructorDemon::moveSideways() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int n, m;
	char move = getFacingDirectionRelativeTo(x, y, px, py);
	switch (move) {
	case 'l':
	case 'r': move = randInt(2) == 0 ? 'u' : 'd'; break;
	case 'u':
	case 'd': move = randInt(2) == 0 ? 'l' : 'r'; break;
	case '1': move = randInt(2) == 0 ? 'l' : 'u'; break;
	case '2': move = randInt(2) == 0 ? 'r' : 'u'; break;
	case '3': move = randInt(2) == 0 ? 'l' : 'd'; break;
	case '4': move = randInt(2) == 0 ? 'r' : 'd'; break;
	}

	setDirectionalOffsets(move, n, m);

	if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)))
		moveTo(x + n, y + m);

}

Howler::Howler(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 5, 2, HOWLER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Howler::move() {

}

FacelessHorror::FacelessHorror(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 5, 2, FACELESS_HORROR) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	switch (randInt(4)) {
	case 0: m_dir = 'l'; break;
	case 1: m_dir = 'r'; break;
	case 2: m_dir = 'u'; break;
	case 3: m_dir = 'd'; break;
	}
}

void FacelessHorror::move() {
	int x = getPosX();
	int y = getPosY();

	if (m_primed) {
		if (m_primedWait > 0) {
			m_primedWait--;
		}
		else {
			playSound("Spider_Primed.mp3");
			m_primed = false;

			int n, m;
			char dir;
			for (int i = 0; i < 8; i++) {
				switch (i + 1) {
				case 1: n = -1; m = 0; dir = 'l'; break;
				case 2: n = 1; m = 0; dir = 'r'; break;
				case 3: n = 0; m = -1; dir = 'u'; break;
				case 4: n = 0; m = 1; dir = 'd'; break;
				case 5: n = -1; m = -1; dir = '1'; break;
				case 6: n = 1; m = -1; dir = '2'; break;
				case 7: n = -1; m = 1; dir = '3'; break;
				case 8: n = 1; m = 1; dir = '4'; break;
				}

				m_dungeon->addTrap(std::make_shared<FacelessMask>(*m_dungeon, x + n, y + m, dir));
			}
		}

		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	char move = m_dungeon->getPlayer()->facingDirection();
	if (hasLineOfSight(*m_dungeon, m_dir, *m_dungeon->getPlayer(), x, y) && directionIsOppositeTo(m_dir, move)) {
		playSound("Goo_Sack_Primed.mp3");

		m_primed = true;
		m_primedWait = 1;
		return;
	}

	if (playerInRange(m_range)) {
		m_dir = attemptDryRunGreedyChase();
		attemptMove(m_dir);
	}

	m_wait = m_maxWait;
}

ToweringBrute::ToweringBrute(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 20, 2, 15, 1, TOWERING_BRUTE) {
	m_maxWait = m_wait;
	setGold(10);
	setHeavy(true);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void ToweringBrute::move() {
	if (m_wait > 0) {
		m_wait--;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	char dir = attemptDryRunGreedyChase();

	int n, m;
	setDirectionalOffsets(dir, n, m);

	if (m_dungeon->wallObject(x + n, y + m) && m_dungeon->wallObject(x + n, y + m)->isDestructible()) {
		m_dungeon->destroyWall(x + n, y + m);
	}
	else if (m_dungeon->enemy(x + n, y + m)) {
		int pos = m_dungeon->findMonster(x + n, y + m);
		if (pos != -1) {
			moveTo(x + n, y + m);
			m_dungeon->monsterAt(pos)->moveTo(x, y);

			(*m_dungeon)[y*m_dungeon->getCols() + x].enemy = true;
			(*m_dungeon)[(y + m)*m_dungeon->getCols() + (x + n)].enemy = true;
		}
	}
	else {
		attemptMove(dir);
	}

	m_wait = m_maxWait;
}

SkyCrasher::SkyCrasher(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 20, 2, SKY_CRASHER) {
	m_maxWait = m_wait;
	setGold(10);
	setHeavy(true);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

bool SkyCrasher::canBeHit() {
	if (m_primed)
		return false;

	return true;
}
void SkyCrasher::move() {
	int x = getPosX();
	int y = getPosY();

	if (m_primed) {
		playSound("Earthquake_Spell2.mp3");

		m_primed = false;
		m_wait = m_maxWait;

		while ((x != m_x || y != m_y) && m_dungeon->enemy(m_x, m_y))
			setLandingPosition(true);

		if (m_dungeon->hero(m_x, m_y)) {
			damagePlayer(getStr(), DamageType::NORMAL);
			moveTo(m_x, m_y, 0.05f);
			moveTo(x, y, 0.05f);
		}
		else {
			if (!m_dungeon->boundary(m_x, m_y) && m_dungeon->wallObject(m_x, m_y))
				m_dungeon->destroyWall(m_x, m_y);

			moveTo(m_x, m_y);
		}

		getSprite()->setLocalZOrder(1);
		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerInRange(m_moveRange))
		getPrimedTowardsPlayer();
}
void SkyCrasher::getPrimedTowardsPlayer() {
	int x = getPosX();
	int y = getPosY();

	m_primed = true;

	m_dungeon->queueMoveSprite(getSprite(), x * 1.0f, y - 1.5f);
	getSprite()->setLocalZOrder(3);

	setLandingPosition();
}
void SkyCrasher::setLandingPosition(bool reposition) {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (!reposition && playerInRectangularRange(*m_dungeon->getPlayer(), 4, 3, x, y)) {
		m_x = px;
		m_y = py;
		return;
	}

	int range = 4;

	if (reposition)
		range = std::min(4, std::max(abs(x - px), abs(y - py)));

	int n, m;
	char dir = getCardinalFacingDirectionRelativeTo(x, y, px, py);
	setDirectionalOffsets(dir, n, m, range);

	bool switched = false;
	if (x == px || y == py)
		switched = true;

	while (range > 0 || !switched) {

		if (!(m_dungeon->hero(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->boundary(x + n, y + m))) {
			if (!switched) {
				switched = true;

				x += n;
				y += m;

				switch (dir) {
				case 'l':
				case 'r': dir = y < py ? 'd' : 'u'; break;
				case 'u':
				case 'd': dir = x < px ? 'r' : 'l'; break;
				}

				range = std::min(3, std::max(abs(x - px), abs(y - py)));
				setDirectionalOffsets(dir, n, m, range);
				continue;
			}
			else
				break;
		}
		else
			decrementDirectionalOffsets(dir, n, m);

		range--;
	}

	m_x = x + n;
	m_y = y + m;
}

SteamBot::SteamBot(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 5, 2, STEAM_BOT) {
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void SteamBot::move() {
	int x = getPosX();
	int y = getPosY();

	switch (m_stage) {
	case 1:
	case 2: {
		if (playerInRange(m_range)) {
			char best;
			if (chase(best))
				attemptMove(best);
		}

		break;
	}
	case 3: moveMonsterRandomly(); break;
	case 4: break;
	case 5: {
		for (int i = x - 1; i < x + 2; i++) {
			for (int j = y - 1; j < y + 2; j++) {
				if (i == x && j == y)
					continue;

				auto frames = getAnimationFrameVector("frame%04d.png", 63);
				m_dungeon->runSingleAnimation(frames, 120, i, j, 2);
				
				if (m_dungeon->hero(i, j))
					attack(*m_dungeon->getPlayer());

				// Small chance to ignite objects
				if (m_dungeon->trap(i, j) && randReal(1, 100) > 95) {
					auto indexes = m_dungeon->findTraps(i, j);
					for (int i = 0; i < (int)indexes.size(); i++) {
						if (m_dungeon->trapAt(indexes[i])->canBeIgnited())
							m_dungeon->trapAt(indexes[i])->ignite();
					}
				}
			}
		}
		break;
	}
	}

	if (m_stage == m_stageMax)
		m_stage = 1;
	else
		m_stage++;
}

Watcher::Watcher(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 40, 3, 10, 3, WATCHER) {
	m_maxWait = m_wait;
	setGold(50);
	setImageName("Demonic_Goblin_48x48.png");
	setHeavy(true);
	setFlying(true);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Watcher::move() {
	if (m_wait > 0) {
		m_wait--;
		return;
	}


	char best;
	if (playerInRange(m_range)) {
		if (chase(best))
			attemptMove(best);
	}

	if (playerInRange(m_range)) {
		if (chase(best))
			attemptMove(best);
	}

	m_wait = m_maxWait;
}

//				:::: FLOOR VII ::::
Pikeman::Pikeman(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 12 + randInt(5), 3, 2, 2, std::make_shared<Pike>(0, 0), PIKEMAN), m_alerted(false) {
	setImageName("Knight_Level1_48x48.png");
	setGold(5);
	setSturdy(false);
	setHasWeapon(true);

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Pikeman::move() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// if player is invisible, just move them randomly
	if (m_dungeon->getPlayer()->isInvisible()) {
		step();
		return;
	}

	if (m_alerted) {
		int n, m;
		setDirectionalOffsets(m_direction, n, m);

		if (m_dungeon->hero(x + n, y + m)) {
			attack(*m_dungeon->getPlayer());
			m_alerted = false;
		}
		else if (m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m))
			m_alerted = false;
		else {
			if (m_dungeon->hero(x + 2 * n, y + 2 * m))
				attack(*m_dungeon->getPlayer());

			moveTo(x + n, y + m);
		}

		return;
	}

	// If player is not in the knight's sights, move knight
	if (x != px && y != py) {
		step();
		return;
	}

	// Same column
	if (x == px) {

		if (hasLineOfSight(*m_dungeon, x, y, px, py)) {
			m_direction = py > y ? 'd' : 'u';
			m_alerted = true;
		}
		else {
			step();
			return;
		}
	}
	// Same row
	else if (y == py) {

		if (hasLineOfSight(*m_dungeon, x, y, px, py)) {
			m_direction = px > x ? 'r' : 'l';
			m_alerted = true;
		}
		else {
			step();
			return;
		}
	}
}
void Pikeman::step() {
	//	50% chance to not move
	int check;
	if (check = randInt(100), check >= 50)
		return;

	// else move the knight in a random direction
	moveMonsterRandomly();
}
void Pikeman::deathDrops() {
	int x = getPosX();
	int y = getPosY();

	// 10% drop chance
	if (1 + randInt(100) + m_dungeon->getPlayer()->getLuck() > 90) {
		if (m_dungeon->itemObject(x, y))
			m_dungeon->itemHash(x, y);

		m_dungeon->createItem(std::make_shared<Pike>(x, y));
	}
}

Shrinekeeper::Shrinekeeper(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 5, 2, SHRINEKEEPER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Shrinekeeper::move() {
	if (m_wait == 0) {
		m_wait = m_maxWait;
		teleportAway();
		return;
	}

	if (m_wait < m_maxWait) {
		m_wait--;
		return;
	}

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_primed) {
		int ex, ey;
		if (canTeleportAdjacentToPlayer(ex, ey)) {
			playSound("Teleport_Spell.mp3", *m_dungeon->getPlayer(), getPosX(), getPosY());
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			m_dungeon->runSingleAnimation(frames, 120, ex, ey, 2);

			m_dungeon->queueCustomAction(getSprite(), cocos2d::Hide::create());
			moveTo(ex, ey);
			m_dungeon->queueCustomAction(getSprite(), cocos2d::Show::create());

			frames = getAnimationFrameVector("ResizedAttack%04d.png", 4);
			m_dungeon->runSingleAnimation(frames, 30, m_px, m_py, 2);

			if (px == m_px && py == m_py)
				attack(*m_dungeon->getPlayer());

		}
		else {
			teleportAway();
		}

		m_primed = false;
		m_wait--;
		return;
	}

	int cols = m_dungeon->getCols();
	int x = getPosX();
	int y = getPosY();

	if (playerInRange(m_moveRange)) {
		if (playerInRange(m_attackRange)) {
			playSound("Teleport_Spell.mp3", *m_dungeon->getPlayer(), getPosX(), getPosY());
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			m_dungeon->runSingleAnimation(frames, 120, x, y, 2);

			getSprite()->setVisible(false);
			(*m_dungeon)[y*cols + x].enemy = false;
			m_primed = true;
			m_px = px;
			m_py = py;
			return;
		}

		char best;
		if (chase(best))
			attemptMove(best);
	}
}
bool Shrinekeeper::canTeleportAdjacentToPlayer(int &ex, int &ey) const {
	int x = m_px;
	int y = m_py;

	std::vector<std::pair<int, int>> coords;
	coords.push_back(std::make_pair(x + 1, y));
	coords.push_back(std::make_pair(x - 1, y));
	coords.push_back(std::make_pair(x, y - 1));
	coords.push_back(std::make_pair(x, y + 1));

	while (!coords.empty()) {
		int index = randInt((int)coords.size());
		int n = coords[index].first;
		int m = coords[index].second;

		if (!(m_dungeon->wall(n, m) || m_dungeon->enemy(n, m) || m_dungeon->hero(n, m))) {
			ex = n;
			ey = m;
			return true;
		}

		coords.erase(coords.begin() + index);
	}

	return false;
}
void Shrinekeeper::teleportAway() {
	int x = m_dungeon->getPlayer()->getPosX();
	int y = m_dungeon->getPlayer()->getPosY();

	std::vector<std::pair<int, int>> coords;

	for (int i = x - m_teleportRadius; i < x + m_teleportRadius + 1; i++) {
		for (int j = y - m_teleportRadius; j < y + m_teleportRadius + 1; j++) {

			// Ignore all but the outer ring
			if (i != x - m_teleportRadius && i != x + m_teleportRadius - 1 && j != y - m_teleportRadius && j != y + m_teleportRadius - 1)
				continue;

			// Boundary check
			if (m_dungeon->withinBounds(i, j)) {
				if (!(m_dungeon->wall(i, j) || m_dungeon->enemy(i, j) || m_dungeon->trap(i, j)))
					coords.push_back(std::make_pair(i, j));
			}
		}
	}

	while (!coords.empty()) {
		int index = randInt((int)coords.size());
		int n = coords[index].first;
		int m = coords[index].second;

		playSound("Teleport_Spell.mp3", *m_dungeon->getPlayer(), getPosX(), getPosY());
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, n, m, 2);

		m_dungeon->queueCustomAction(getSprite(), cocos2d::Hide::create());
		moveTo(n, m);
		m_dungeon->queueCustomAction(getSprite(), cocos2d::Show::create());

		return;
	}

	int cols = m_dungeon->getCols();
	int rows = m_dungeon->getRows();

	int _x = 1 + randInt(cols - 1);
	int _y = 1 + randInt(rows - 1);

	while (m_dungeon->enemy(_x, _y) || m_dungeon->wall(_x, _y) || m_dungeon->trap(_x, _y) || m_dungeon->hero(_x, _y)) {
		_x = 1 + randInt(cols - 1);
		_y = 1 + randInt(rows - 1);
	}

	playSound("Teleport_Spell.mp3", *m_dungeon->getPlayer(), getPosX(), getPosY());
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
	m_dungeon->runSingleAnimation(frames, 120, _x, _y, 2);

	m_dungeon->queueCustomAction(getSprite(), cocos2d::Hide::create());
	moveTo(_x, _y);
	m_dungeon->queueCustomAction(getSprite(), cocos2d::Show::create());
}

Swapper::Swapper(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 5, 2, SWAPPER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Swapper::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_primed) {
		swap();
		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	m_wait = m_maxWait;

	if (playerInRange(m_swapRange)) {
		m_primed = true;
		m_px = px;
		m_py = py;
		return;
	}

	if (playerInRange(m_moveRange)) {
		char best;
		if (chase(best))
			attemptMove(best);
	}
}
void Swapper::swap() {
	int x = getPosX();
	int y = getPosY();

	m_primed = false;

	playSound("Teleport_Spell.mp3", *m_dungeon->getPlayer(), x, y);
	auto frames = getAnimationFrameVector("frame%04d.png", 63);
	m_dungeon->runSingleAnimation(frames, 120, x, y, 2);
	m_dungeon->runSingleAnimation(frames, 120, m_px, m_py, 2);

	if (m_dungeon->itemObject(x, y) || m_dungeon->itemObject(m_px, m_py)) {
		std::shared_ptr<Objects> item1 = m_dungeon->itemObject(x, y);
		if (item1) {
			m_dungeon->removeItem(x, y);
			item1->setPosX(m_px); item1->setPosY(m_py);
			
			m_dungeon->createItem(item1);
		}

		std::shared_ptr<Objects> item2 = m_dungeon->itemObject(m_px, m_py);
		if (item2) {
			m_dungeon->removeItem(m_px, m_py);
			item2->setPosX(x); item2->setPosY(y);
			
			m_dungeon->createItem(item2);
		}
	}

	if (m_dungeon->gold(x, y) > 0 || m_dungeon->gold(m_px, m_py) > 0) {
		int prevGold = m_dungeon->gold(x, y);

		m_dungeon->removeGold(x, y);
		m_dungeon->addGold(x, y, m_dungeon->gold(m_px, m_py));

		m_dungeon->removeGold(m_px, m_py);
		m_dungeon->addGold(m_px, m_py, prevGold);
	}

	if (m_dungeon->hero(m_px, m_py)) {
		moveTo(m_px, m_py);
		m_dungeon->getPlayer()->moveTo(x, y);
		
		damagePlayer(5, DamageType::MAGICAL);
		
		return;
	}

	if (m_dungeon->enemy(m_px, m_py)) {
		int pos = m_dungeon->findMonster(m_px, m_py);
		if (pos != -1) {
			moveTo(m_px, m_py);
			m_dungeon->monsterAt(pos)->moveTo(x, y);

			(*m_dungeon)[y*m_dungeon->getCols() + x].enemy = true;
			(*m_dungeon)[m_py*m_dungeon->getCols() + m_px].enemy = true;
		}
		return;
	}

	if (!(m_dungeon->hero(m_px, m_py) || m_dungeon->enemy(m_px, m_py))) {
		if (playerInRange(m_moveRange)) {
			char best;
			if (chase(best))
				attemptMove(best);
		}
	}
}

ShieldMaster::ShieldMaster(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 5, 2, SHIELD_MASTER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Knight_Level1_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	switch (randInt(4)) {
	case 0: m_exposedDir = 'l'; break;
	case 1: m_exposedDir = 'r'; break;
	case 2: m_exposedDir = 'u'; break;
	case 3: m_exposedDir = 'd'; break;
	}

	m_clockwise = randInt(2) == 0 ? true : false;

	if (m_exposedDir != 'l') m_shields.push_back(std::make_pair('l', dungeon->createMiscSprite(x - 0.5f, y * 1.0f, y + Z_ACTOR, "Water_Orb_32x32.png")));
	if (m_exposedDir != 'r') m_shields.push_back(std::make_pair('r', dungeon->createMiscSprite(x + 0.5f, y * 1.0f, y + Z_ACTOR, "Water_Orb_32x32.png")));
	if (m_exposedDir != 'u') m_shields.push_back(std::make_pair('u', dungeon->createMiscSprite(x * 1.0f, y - 0.5f, y + Z_ACTOR - 1, "Water_Orb_32x32.png")));
	if (m_exposedDir != 'd') m_shields.push_back(std::make_pair('d', dungeon->createMiscSprite(x * 1.0f, y + 0.5f, y + Z_ACTOR + 1, "Water_Orb_32x32.png")));
}

bool ShieldMaster::canBeDamaged(DamageType type) {
	if (directionIsOppositeTo(m_dungeon->getPlayer()->facingDirection(), m_exposedDir) ||
		type == DamageType::PIERCING || type == DamageType::EXPLOSIVE)
		return true;

	return false;
}
void ShieldMaster::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	(*m_dungeon)[getPosY()*cols + getPosX()].enemy = false;
	(*m_dungeon)[y*cols + x].enemy = true;

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);
	getSprite()->setLocalZOrder(y + Z_ACTOR);

	moveShieldsTo(x, y, time);
}
void ShieldMaster::moveShieldsTo(int x, int y, float time) {
	/*cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	for (auto &it : m_shields) {
		switch (it.first) {
		case 'l': v.pushBack(m_dungeon->getMoveAction(it.second, x - 0.5f, y * 1.0f, time)); break;
		case 'r': v.pushBack(m_dungeon->getMoveAction(it.second, x + 0.5f, y * 1.0f, time)); break;
		case 'u': v.pushBack(m_dungeon->getMoveAction(it.second, x * 1.0f, y - 0.5f, time)); break;
		case 'd': v.pushBack(m_dungeon->getMoveAction(it.second, x * 1.0f, y + 0.5f, time)); break;
		}
	}

	m_dungeon->queueCustomSpawnActions(getSprite(), v);*/

	/*for (auto &it : m_shields) {
		switch (it.first) {
		case 'l': m_dungeon->queueMoveSprite(it.second, x - 0.5f, y * 1.0f, time); break;
		case 'r': m_dungeon->queueMoveSprite(it.second, x + 0.5f, y * 1.0f, time); break;
		case 'u': m_dungeon->queueMoveSprite(it.second, x * 1.0f, y - 0.5f, time); break;
		case 'd': m_dungeon->queueMoveSprite(it.second, x * 1.0f, y + 0.5f, time); break;
		}
	}*/

	rotateShields(x, y, time);
}
void ShieldMaster::rotateShields(int x, int y, float time) {
	if (m_clockwise) {
		for (auto &it : m_shields) {
			char dir = it.first;
			switch (dir) {
			case 'l': m_dungeon->queueMoveSprite(it.second, x * 1.0f, y - 0.5f, time); it.second->setLocalZOrder(y + Z_ACTOR - 1); it.first = 'u'; break;
			case 'r': m_dungeon->queueMoveSprite(it.second, x * 1.0f, y + 0.5f, time); it.second->setLocalZOrder(y + Z_ACTOR + 1); it.first = 'd'; break;
			case 'u': m_dungeon->queueMoveSprite(it.second, x + 0.5f, y * 1.0f, time); it.second->setLocalZOrder(y + Z_ACTOR); it.first = 'r'; break;
			case 'd': m_dungeon->queueMoveSprite(it.second, x - 0.5f, y * 1.0f, time); it.second->setLocalZOrder(y + Z_ACTOR); it.first = 'l'; break;
			}
		}

		switch (m_exposedDir) {
		case 'l': m_exposedDir = 'u'; break;
		case 'r': m_exposedDir = 'd'; break;
		case 'u': m_exposedDir = 'r'; break;
		case 'd': m_exposedDir = 'l'; break;
		}
	}
	else {
		for (auto &it : m_shields) {
			char dir = it.first;
			switch (dir) {
			case 'l': m_dungeon->queueMoveSprite(it.second, x * 1.0f, y + 0.5f, time); it.second->setLocalZOrder(y + Z_ACTOR + 1); it.first = 'd'; break;
			case 'r': m_dungeon->queueMoveSprite(it.second, x * 1.0f, y - 0.5f, time); it.second->setLocalZOrder(y + Z_ACTOR - 1); it.first = 'u'; break;
			case 'u': m_dungeon->queueMoveSprite(it.second, x - 0.5f, y * 1.0f, time); it.second->setLocalZOrder(y + Z_ACTOR); it.first = 'l'; break;
			case 'd': m_dungeon->queueMoveSprite(it.second, x + 0.5f, y * 1.0f, time); it.second->setLocalZOrder(y + Z_ACTOR); it.first = 'r'; break;
			}
		}

		switch (m_exposedDir) {
		case 'l': m_exposedDir = 'd'; break;
		case 'r': m_exposedDir = 'u'; break;
		case 'u': m_exposedDir = 'l'; break;
		case 'd': m_exposedDir = 'r'; break;
		}
	}
}
void ShieldMaster::move() {
	int x = getPosX();
	int y = getPosY();

	if (m_wait > 0) {
		rotateShields(x, y);

		m_wait--;
		return;
	}

	if (playerInRange(m_range)) {
		char best;
		if (chase(best))
			attemptMove(best);
	}

	if (x == getPosX() && y == getPosY())
		rotateShields(x, y);

	m_wait = m_maxWait;
}
void ShieldMaster::spriteCleanup() {
	m_dungeon->queueRemoveSprite(getSprite());

	for (auto &it : m_shields) {
		if (it.second) {
			it.second->setVisible(false);
			//m_dungeon->queueRemoveSprite(it.second);
			//it.second = nullptr;
		}
	}

	m_shields.clear();
}

PseudoDoppel::PseudoDoppel(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 5, 2, PSEUDO_DOPPEL) {
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void PseudoDoppel::move() {
	int x = getPosX();
	int y = getPosY();

	if (m_mirror) {
		if (isMovementAction(m_dungeon->getPlayer()->getAction()))
			attemptMove(getDirectionOppositeTo(m_dungeon->getPlayer()->facingDirection()));

		m_mirror = false;
		return;
	}

	moveMonsterRandomly();
	m_mirror = true;
}

Electromagnetizer::Electromagnetizer(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 5, 2, ELECTROMAGNETIZER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Electromagnetizer::extraAttackEffects() {
	if (m_lightStolen == 0 && m_dungeon->getPlayer()->getVision() > 0) {
		m_lightStolen += m_dungeon->getPlayer()->getVision();
		m_dungeon->getPlayer()->setVision(0);
	}
}
void Electromagnetizer::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_primed) {
		m_primed = false;

		if (playerInLinearRange(2, x, y, px, py) && hasLineOfSight(*m_dungeon, *m_dungeon->getPlayer(), x, y))
			attack(*m_dungeon->getPlayer());
			
		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerInLinearRange(2, x, y, px, py) && hasLineOfSight(*m_dungeon, *m_dungeon->getPlayer(), x, y)) {
		playSound("Goo_Sack_Primed.mp3");
		m_primed = true;
		return;
	}
	else if (playerInRange(m_range)) {
		char move;
		if (chase(move))
			attemptMove(move);
	}

	m_wait = m_maxWait;
}
void Electromagnetizer::extraDeathEffects() {
	if (m_lightStolen > 0) {
		m_dungeon->getPlayer()->setVision(m_dungeon->getPlayer()->getVision() + m_lightStolen);
	}
}

BladePsychic::BladePsychic(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 2, 5, 1, BLADE_PSYCHIC) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void BladePsychic::move() {
	if (m_bladeWait > 0) {
		m_bladeWait--;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (playerIsAdjacent())
		attack(*m_dungeon->getPlayer());
	else if (m_bladeWait == 0 && !playerIsAdjacent(true) &&
		playerInRectangularRange(*m_dungeon->getPlayer(), m_range, m_range, x, y)) {
		char dir = getFacingDirectionRelativeTo(x, y, px, py);
		int n, m;
		setDirectionalOffsets(dir, n, m);

		m_dungeon->addTrap(std::make_shared<PsychicSlash>(*m_dungeon, x + n, y + m, dir));
		m_bladeWait = m_maxBladeWait;
	}
	else if (playerInRange(m_moveRange)) {
		char move;
		if (chase(move))
			attemptMove(move);
	}

	m_wait = m_maxWait;
}

DashMaster::DashMaster(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 2, 10, 1, DASH_MASTER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void DashMaster::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_primedTurns > 0) {
		m_primedTurns--;

		char dir = getCardinalFacingDirectionRelativeTo(x, y, px, py);
		int n, m;
		setDirectionalOffsets(dir, n, m);

		if (m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) {
			if (chase(dir))
				setDirectionalOffsets(dir, n, m);
		}

		int range = 3;
		int currentRange = 0;

		while (currentRange < range) {
			currentRange++;

			if (m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m))
				break;		
			
			if (m_dungeon->hero(x + n, y + m)) {
				attack(*m_dungeon->getPlayer());
				break;
			}

			incrementDirectionalOffsets(dir, n, m);
		}

		decrementDirectionalOffsets(dir, n, m);
		moveTo(x + n, y + m);
		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerInRectangularRange(*m_dungeon->getPlayer(), m_range, m_range, x, y)) {
		m_primedTurns = 3;
	}

	m_wait = m_maxWait;
}

AcidicBeast::AcidicBeast(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 2, 10, 1, ACIDIC_BEAST) {
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));

	switch (randInt(4)) {
	case 0: m_dir1 = 'l'; break;
	case 1: m_dir1 = 'r'; break;
	case 2: m_dir1 = 'u'; break;
	case 3: m_dir1 = 'd'; break;
	}

	switch (m_dir1) {
	case 'l': m_dir2 = randInt(2) == 1 ? 'u' : 'd'; break;
	case 'r': m_dir2 = randInt(2) == 1 ? 'd' : 'u'; break;
	case 'u': m_dir2 = randInt(2) == 1 ? 'l' : 'r'; break;
	case 'd': m_dir2 = randInt(2) == 1 ? 'r' : 'l'; break;
	}
}

bool AcidicBeast::canBeDamaged(DamageType type) {
	if (type == DamageType::ACIDIC)
		return false;

	return true;
}
void AcidicBeast::move() {
	int x = getPosX();
	int y = getPosY();

	if (randReal(1, 100) > 50)
		m_dungeon->addTrap(std::make_shared<Acid>(*m_dungeon, x, y));
	
	int n, m;
	if (m_switch)
		setDirectionalOffsets(m_dir2, n, m);	
	else
		setDirectionalOffsets(m_dir1, n, m);
	
	m_switch = !m_switch;

	if (m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m))
		changeDirections();

	else if (m_dungeon->hero(x + n, y + m)) {
		changeDirections();
		attack(*m_dungeon->getPlayer());
	}
	else
		moveTo(x + n, y + m);
}
void AcidicBeast::changeDirections() {
	switch (m_dir1) {
	case 'l': m_dir1 = 'r'; break;
	case 'r': m_dir1 = 'l'; break;
	case 'u': m_dir1 = 'd'; break;
	case 'd': m_dir1 = 'u'; break;
	}

	switch (m_dir1) {
	case 'l': m_dir2 = randInt(2) == 1 ? 'u' : 'd'; break;
	case 'r': m_dir2 = randInt(2) == 1 ? 'd' : 'u'; break;
	case 'u': m_dir2 = randInt(2) == 1 ? 'l' : 'r'; break;
	case 'd': m_dir2 = randInt(2) == 1 ? 'r' : 'l'; break;
	}
}

DarkCanine::DarkCanine(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 20, 2, 10, 1, DARK_CANINE) {
	m_maxWait = m_wait;
	setCanBePoisoned(false);
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void DarkCanine::extraAttackEffects() {

	// Only latches onto the player if it's cardinal to player
	if (playerIsAdjacent()) {
		int x = getPosX();
		int y = getPosY();

		int px = m_dungeon->getPlayer()->getPosX();
		int py = m_dungeon->getPlayer()->getPosY();

		char dir = getCardinalFacingDirectionRelativeTo(x, y, px, py);
		float n, m;
		switch (dir) {
		case 'l': n = -0.5f; m = 0; break;
		case 'r': n = 0.5f; m = 0; break;
		case 'u': n = 0; m = -0.5f; break;
		case 'd': n = 0; m = 0.5f; break;
		}

		m_dungeon->queueMoveSprite(getSprite(), x + n, y + m);
		m_latchedTurns = 3;
		m_dungeon->getPlayer()->addAffliction(std::make_shared<Stuck>(1));
	}

	if (m_dungeon->getPlayer()->canBePoisoned()) {
		if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() < 60)
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 8, 3, 2, 2));
	}

	if (m_dungeon->getPlayer()->canBeBled()) {
		if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() < 60)
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Bleed>(8));
	}
}
void DarkCanine::move() {
	if (m_latchedTurns > 0) {
		m_dungeon->getPlayer()->addAffliction(std::make_shared<Stuck>(1));
		return;
	}
	
	if (m_wait > 0) {
		m_wait--;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	if (playerIsAdjacent(true)) {
		attack(*m_dungeon->getPlayer());
	}

	else if (playerInDiagonalRange(m_range)) {
		char move;
		if (chase(move, false, true)) {
			if (attemptDryRunMove(move)) {
				int n, m;
				if (!isConfused())
					setDirectionalOffsets(move, n, m, 2);
				else
					setDirectionalOffsets(move, n, m, 2, true);

				if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m)))
					moveTo(x + n, y + m);
				else
					attemptMove(move);
			}
		}
	}

	m_wait = m_maxWait;
}
void DarkCanine::reactToDamage() {
	if (m_dungeon->getPlayer()->hasSteelPunch() || m_dungeon->getPlayer()->getStr() > 10)
		m_latchedTurns = 0;

	if (m_latchedTurns > 0)
		m_latchedTurns--;

	if (m_latchedTurns == 0) {
		int x = getPosX();
		int y = getPosY();

		int px = m_dungeon->getPlayer()->getPosX();
		int py = m_dungeon->getPlayer()->getPosY();

		int n = 0, m = 0;
		char dir = getCardinalFacingDirectionRelativeTo(px, py, x, y);
		setDirectionalOffsets(dir, n, m);

		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)))
			moveTo(x + n, y + m);
		else
			moveTo(x, y);

		addAffliction(std::make_shared<Stun>(1));
	}
}


//				:::: FLOOR VIII ::::
AbyssSummoner::AbyssSummoner(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 10, 2, 5, 2, ABYSS_SUMMONER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}
AbyssSummoner::~AbyssSummoner() {
	if (m_maw) {
		m_maw->destroyTrap();
		m_maw.reset();
		m_maw = nullptr;
	}
}

void AbyssSummoner::move() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_summoning) {
		if (m_summonTurns > 0) {
			m_summonTurns--;
			return;
		}

		m_maw.reset();
		m_maw = nullptr;
		m_summoning = false;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (randReal(1, 100) > 50)
		moveMonsterRandomly();

	if (playerInRange(m_range)) {
		playSound("Possessed.mp3");

		m_summoning = true;

		m_maw = std::make_shared<AbyssalMaw>(*m_dungeon, px, py);
		m_dungeon->addTrap(m_maw);
	}

	m_wait = m_maxWait;
}

MagicalBerserker::MagicalBerserker(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 20, 1, 1, 2, MAGICAL_BERSERKER) {
	setGold(7);
	setImageName("Cheese_Wedge_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void MagicalBerserker::move() {

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	switch (m_stage) {
	case 1: 
	case 2:
		attemptGreedyChase(); break;
	case 3: 
		attemptGreedyChase();
		attemptGreedyChase();
		break;
	}

	switch (m_stage) {
	case 1: m_wait = 1; break;
	case 2:
	case 3: m_wait = 0; break;
	}
}
void MagicalBerserker::reactToDamage() {
	m_stage++;
	if (m_stage > 3)
		setDead(true);
	else
		teleportAway();
}
void MagicalBerserker::teleportAway() {
	int x = m_dungeon->getPlayer()->getPosX();
	int y = m_dungeon->getPlayer()->getPosY();

	std::vector<std::pair<int, int>> coords;

	for (int i = x - m_teleportRadius; i < x + m_teleportRadius + 1; i++) {
		for (int j = y - m_teleportRadius; j < y + m_teleportRadius + 1; j++) {

			// Ignore all but the outer ring
			if (i != x - m_teleportRadius && i != x + m_teleportRadius - 1 && j != y - m_teleportRadius && j != y + m_teleportRadius - 1)
				continue;

			// Boundary check
			if (m_dungeon->withinBounds(i, j)) {
				if (!(m_dungeon->wall(i, j) || m_dungeon->enemy(i, j) || m_dungeon->trap(i, j)))
					coords.push_back(std::make_pair(i, j));
			}
		}
	}

	while (!coords.empty()) {
		int index = randInt((int)coords.size());
		int n = coords[index].first;
		int m = coords[index].second;

		playSound("Teleport_Spell.mp3", *m_dungeon->getPlayer(), getPosX(), getPosY());
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, n, m, 2);

		m_dungeon->queueCustomAction(getSprite(), cocos2d::Hide::create());
		moveTo(n, m);
		m_dungeon->queueCustomAction(getSprite(), cocos2d::Show::create());

		return;
	}

	int cols = m_dungeon->getCols();
	int rows = m_dungeon->getRows();

	int _x = 1 + randInt(cols - 1);
	int _y = 1 + randInt(rows - 1);

	while (m_dungeon->enemy(_x, _y) || m_dungeon->wall(_x, _y) || m_dungeon->trap(_x, _y) || m_dungeon->hero(_x, _y)) {
		_x = 1 + randInt(cols - 1);
		_y = 1 + randInt(rows - 1);
	}

	playSound("Teleport_Spell.mp3", *m_dungeon->getPlayer(), getPosX(), getPosY());
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
	m_dungeon->runSingleAnimation(frames, 120, _x, _y, 2);

	m_dungeon->queueCustomAction(getSprite(), cocos2d::Hide::create());
	moveTo(_x, _y);
	m_dungeon->queueCustomAction(getSprite(), cocos2d::Show::create());
}

Disabler::Disabler(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 1, 8, 1, DISABLER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void Disabler::move() {
	int x = getPosX();
	int y = getPosY();

	if (m_castWait > 0)
		m_castWait--;

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (playerInRange(m_range)) {
		char move;
		if (chase(move))
			attemptMove(move);

		if (m_castWait == 0) {
			castField();
			m_castWait = m_maxCastWait;
		}
	}

	m_wait = m_maxWait;
}
void Disabler::castField() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	std::string pattern;
	switch (randInt(3)) {
	case 0: pattern = "Diamond"; break;
	case 1: pattern = "Cross Box"; break;
	case 2: pattern = "Ring"; break;
	}

	for (int i = px - 2; i < px + 3; i++) {
		for (int j = py - 2; j < py + 3; j++) {
			if (!m_dungeon->withinBounds(i, j))
				continue;

			if (pattern == "Diamond") {
				if (!((abs(i - px) <= 1 && abs(j - py) <= 1) || (abs(i - px) == 2 && j == py) || (i == px && abs(j - py) == 2)))
					continue;
			}

			if (pattern == "Cross Box") {
				if (!((abs(i - px) == 2 && abs(j - py) == 2) || (abs(i - px) <= 1 && abs(j - py) <= 1)))
					continue;
			}

			if (pattern == "Ring") {
				if (i != px - 2 && i != px + 2 && j != py - 2 && j != py + 2)
					continue;
			}

			m_dungeon->addTrap(std::make_shared<DisablingField>(*m_dungeon, i, j));
		}
	}
}

IncendiaryInfuser::IncendiaryInfuser(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 1, 8, 1, INCENDIARY_INFUSER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

bool IncendiaryInfuser::canBeDamaged(DamageType type) {
	if (type == DamageType::EXPLOSIVE)
		return false;

	return true;
}
void IncendiaryInfuser::move() {
	if (m_castWait > 0)
		m_castWait--;

	if (m_wait > 0)
		m_wait--;

	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_castWait == 0) {
		m_castWait = m_maxCastWait;

		for (unsigned int i = 0; i < m_dungeon->monsterCount(); i++) {
			int mx = m_dungeon->monsterAt(i)->getPosX();
			int my = m_dungeon->monsterAt(i)->getPosY();

			if (abs(mx - px) <= m_range && abs(my - py) <= m_range) {
				if (randReal(1, 100) < 3)
					m_dungeon->giveAffliction(i, std::make_shared<Incendiary>(10));				
			}
		}

		return;
	}

	if (m_wait == 0) {
		m_wait = m_maxWait;

		moveMonsterRandomly();
	}
}

LightningStriker::LightningStriker(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 1, 8, 1, LIGHTNING_STRIKER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void LightningStriker::move() {
	if (m_castTurns > 0) {
		m_castTurns--;
		if (m_castTurns == 0)
			radiusCheck();

		m_castCooldown = 3;
		return;
	}

	if (m_castCooldown > 0)
		m_castCooldown--;

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (m_castCooldown == 0 && playerInRectangularRange(*m_dungeon->getPlayer(), m_castRange, m_castRange, getPosX(), getPosY())) {
		m_castTurns = m_maxCastTurns;
		return;
	}

	if (playerInRange(m_moveRange)) {
		char move;
		if (chase(move))
			attemptMove(move);
	}

	m_wait = m_maxWait;
}
void LightningStriker::radiusCheck() {
	int x = getPosX();
	int y = getPosY();

	playSound("Shock5.mp3");

	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {

			if (m_dungeon->withinBounds(i, j) &&
				!(i == x - 2 && j == y - 2) && !(i == x - 2 && j == y + 2) && !(i == x + 2 && j == y - 2) && !(i == x + 2 && j == y + 2)) {

				if (m_dungeon->hero(i, j))
					damagePlayer(15, DamageType::LIGHTNING);
				

			}
		}
	}
}

FlameArchaic::FlameArchaic(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 1, 8, 1, FLAME_ARCHAIC) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void FlameArchaic::move() {
	if (m_castTurns > 0) {
		m_castTurns--;
		if (m_castTurns == 0) {
			if (hasLineOfSight(*m_dungeon, *m_dungeon->getPlayer(), getPosX(), getPosY()))
				castFireBlast();
			else
				castFirePillars();
		}

		m_castCooldown = 4;
		return;
	}

	if (m_castCooldown > 0)
		m_castCooldown--;

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (m_castCooldown == 0 && playerInRange(m_castRange)) {
		m_castTurns = m_maxCastTurns;
		return;
	}

	if (playerInRange(m_moveRange)) {
		char move;
		if (chase(move))
			attemptMove(move);
	}

	m_wait = m_maxWait;
}
void FlameArchaic::castFireBlast() {
	playSound("Fireblast_Spell1.mp3");

	int x = getPosX();
	int y = getPosY();

	int n, m;
	char dir = getCardinalFacingDirectionRelativeTo(x, y, m_dungeon->getPlayer()->getPosX(), m_dungeon->getPlayer()->getPosY());
	setDirectionalOffsets(dir, n, m);

	while (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		incrementDirectionalOffsets(dir, n, m);
	}

	// play fire blast explosion sound effect
	playSound("Fireblast_Spell2.mp3");
	auto frames = getAnimationFrameVector("frame%04d.png", 63);

	x += n;
	y += m;

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			// fire explosion animation
			m_dungeon->runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);

			if (m_dungeon->hero(i, j)) {
				m_dungeon->damagePlayer(10, DamageType::FIRE);

				if (m_dungeon->getPlayer()->canBeBurned())
					m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 5));
			}

			if (m_dungeon->enemy(i, j)) {

				int pos = m_dungeon->findMonster(i, j);
				if (pos != -1) {
					m_dungeon->damageMonster(pos, getStr(), DamageType::FIRE);

					// If they can be burned, roll for a high chance to burn
					if (m_dungeon->monsterAt(pos)->canBeBurned() || m_dungeon->getPlayer()->hasAfflictionOverride()) {
						int turns = 5 + m_dungeon->getPlayer()->getInt() + (m_dungeon->getPlayer()->hasHarshAfflictions() ? 6 : 0);
						m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), turns));
					}
				}
			}

			if (m_dungeon->trap(i, j)) {
				std::vector<int> indexes = m_dungeon->findTraps(i, j);
				for (int pos = 0; pos < (int)indexes.size(); pos++)
					if (m_dungeon->trapAt(indexes.at(pos))->canBeIgnited())
						m_dungeon->trapAt(indexes.at(pos))->ignite();
			}
		}
	}
}
void FlameArchaic::castFirePillars() {
	m_dungeon->addTrap(std::make_shared<FlameArchaicFirePillars>(*m_dungeon, getPosX(), getPosY()));
}

MasterConjurer::MasterConjurer(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 1, 8, 1, MASTER_CONJURER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void MasterConjurer::move() {
	if (m_castTurns > 0) {
		m_castTurns--;
		if (m_castTurns == 0)
			summonRandomMonster();

		m_castCooldown = 15;
		return;
	}

	if (m_castCooldown > 0)
		m_castCooldown--;

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (m_castCooldown == 0) {
		m_castTurns = m_maxCastTurns;
		return;
	}

	if (playerInRange(m_range)) {
		char move;
		if (chase(move))
			attemptMove(move);
	}

	m_wait = m_maxWait;
}
void MasterConjurer::summonRandomMonster() {
	int x = getPosX();
	int y = getPosY();

	std::vector<std::pair<int, int>> coords;

	for (int i = x - m_summonRadius; i < x + m_summonRadius + 1; i++) {
		for (int j = y - m_summonRadius; j < y + m_summonRadius + 1; j++) {
			if (m_dungeon->withinBounds(i, j)) {
				if (!(m_dungeon->wall(i, j) || m_dungeon->enemy(i, j) || m_dungeon->hero(i, j) || m_dungeon->trap(i, j)))
					coords.push_back(std::make_pair(i, j));
			}
		}
	}

	while (!coords.empty()) {
		int index = randInt((int)coords.size());
		int n = coords[index].first;
		int m = coords[index].second;

		playSound("Teleport_Spell.mp3", *m_dungeon->getPlayer(), x, y);
		auto frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, n, m, m + Z_PROJECTILE);

		switch (1 + randInt(6)) {
		case 1: m_dungeon->addMonster(rollMonster(FIRST_FLOOR, m_dungeon, n, m)); break;
		case 2: m_dungeon->addMonster(rollMonster(SECOND_FLOOR, m_dungeon, n, m)); break;
		case 3: m_dungeon->addMonster(rollMonster(THIRD_FLOOR, m_dungeon, n, m)); break;
		case 4: m_dungeon->addMonster(rollMonster(FOURTH_FLOOR, m_dungeon, n, m)); break;
		case 5: m_dungeon->addMonster(rollMonster(FIFTH_FLOOR, m_dungeon, n, m)); break;
		case 6: m_dungeon->addMonster(rollMonster(SIXTH_FLOOR, m_dungeon, n, m)); break;
		}

		return;
	}
}
void MasterConjurer::reactToDamage() {
	if (m_castTurns > 0)
		m_castTurns = 0;
}

AdvancedRockSummoner::AdvancedRockSummoner(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 1, 8, 1, ADVANCED_ROCK_SUMMONER) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

bool AdvancedRockSummoner::canBeDamaged(DamageType type) {
	if (type == DamageType::CRUSHING)
		return false;

	return true;
}
void AdvancedRockSummoner::move() {
	if (m_castTurns > 0) {
		m_castTurns--;
		if (m_castTurns == 0)
			summonRock();

		m_castCooldown = 6;
		return;
	}

	if (m_castCooldown > 0)
		m_castCooldown--;

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (m_castCooldown == 0 && playerInRange(m_castRange)) {
		m_castTurns = m_maxCastTurns;
		return;
	}

	if (playerInRange(m_moveRange)) {
		char move;
		if (chase(move))
			attemptMove(move);
	}

	m_wait = m_maxWait;
}
void AdvancedRockSummoner::summonRock() {
	m_dungeon->addTrap(std::make_shared<MegaRock>(*m_dungeon, m_dungeon->getPlayer()->getPosX(), m_dungeon->getPlayer()->getPosY()));
}
void AdvancedRockSummoner::reactToDamage() {
	if (m_castTurns > 0)
		m_castTurns = 0;
}

AscendedShot::AscendedShot(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 21, 1, 8, 1, ASCENDED_SHOT) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

bool AscendedShot::canBeDamaged(DamageType type) {
	if (type == DamageType::MAGICAL)
		return true;

	return false;
}
void AscendedShot::move() {
	if (m_castTurns > 0) {
		m_castTurns--;
		if (m_castTurns == 0 || m_castTurns == 1)
			fireShot();

		m_castCooldown = 6;
		return;
	}

	if (m_castCooldown > 0)
		m_castCooldown--;

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (m_castCooldown == 0 && playerInRange(m_castRange)) {
		m_castTurns = m_maxCastTurns;
		return;
	}

	if (playerInRange(m_moveRange)) {
		char move;
		if (chase(move))
			attemptMove(move);
	}

	m_wait = m_maxWait;
}
void AscendedShot::fireShot() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	char dir = getFacingDirectionRelativeTo(x, y, px, py);
	int n, m;
	setDirectionalOffsets(dir, n, m);

	m_dungeon->addTrap(std::make_shared<ReflectiveShot>(*m_dungeon, x + n, y + m, dir, this));
}

RoyalSwordsman::RoyalSwordsman(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 25, 2, 8, 2, ROYAL_SWORDSMAN) {
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

bool RoyalSwordsman::canBeDamaged(DamageType type) {
	if (m_stage == 3)
		return true;

	if (m_wait == 0) {
		m_primed = true;
		m_stage++;

		if (m_stage == 1)
			m_wait = 1;
		else if (m_stage == 2)
			m_wait = 2;
		
	}

	return false;
}
void RoyalSwordsman::move() {
	if (m_cooldown > 0) {
		m_cooldown--;

		if (m_cooldown == 0)
			m_stage = 0;

		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (!m_primed)
		m_stage = 0;

	switch (m_stage) {
	case 0: {
		if (playerInRange(m_range)) {
			char move;
			if (chase(move))
				attemptMove(move);
		}
		break;
	}
	case 1: singleDash(); break;
	case 2: doubleDash(); m_stage++; return; // Prevents m_primed from being unflagged
	case 3: singleDash(true); m_cooldown = m_maxCooldown; break;
	default: break;
	}

	m_primed = false;
}
void RoyalSwordsman::singleDash(bool diagonals) {
	playMiss();

	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	char dir;
	
	if (!diagonals)
		dir = getCardinalFacingDirectionRelativeTo(x, y, px, py);
	else
		dir = getFacingDirectionRelativeTo(x, y, px, py);

	int n, m;
	setDirectionalOffsets(dir, n, m);

	if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {
		moveTo(x + n, y + m);
	}
	else if (m_dungeon->hero(x + n, y + m)) {
		attack(*m_dungeon->getPlayer());
	}
}
void RoyalSwordsman::doubleDash() {
	playMiss();

	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	char dir = getFacingDirectionRelativeTo(x, y, px, py);
	int n, m;
	setDirectionalOffsets(dir, n, m);

	int range = 2;
	int currentRange = 0;
	while (currentRange < range) {
		currentRange++;

		if (m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) {
			decrementDirectionalOffsets(dir, n, m);
			break;
		}
		
		if (m_dungeon->hero(x + n, y + m)) {
			attack(*m_dungeon->getPlayer());
			decrementDirectionalOffsets(dir, n, m);
			break;
		}

		if (currentRange == range)
			break;
		
		incrementDirectionalOffsets(dir, n, m);
	}

	moveTo(x + n, y + m);
}

LightEntity::LightEntity(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 1, 8, 2, LIGHT_ENTITY) {
	m_movesLeft = m_maxMoves;
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void LightEntity::move() {
	if (m_movesLeft > 0) {
		m_movesLeft--;

		if (randReal(1, 100) > 60)
			moveMonsterRandomly(true);

		return;
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	releaseBeams();

	m_movesLeft = m_maxMoves;
	m_wait = m_maxWait;
}
void LightEntity::releaseBeams() {
	int x = getPosX();
	int y = getPosY();

	std::vector<Coords> coords;
	getCoordsAdjacentTo(*m_dungeon, coords, x, y, true);

	for (int i = 0; i < 3 && !coords.empty(); i++) {
		int index = randInt((int)coords.size());
		int _x = coords[index].x;
		int _y = coords[index].y;

		m_dungeon->addTrap(std::make_shared<LightBeam>(*m_dungeon, _x, _y, getFacingDirectionRelativeTo(x, y, _x, _y)));

		coords.erase(coords.begin() + index);
	}
}

DarkEntity::DarkEntity(Dungeon *dungeon, int x, int y) : Monster(dungeon, x, y, 15, 1, 8, 1, DARK_ENTITY) {
	m_maxWait = m_wait;
	setGold(10);
	setImageName("Demonic_Goblin_48x48.png");

	setSprite(dungeon->createSprite(x, y, y + Z_ACTOR, getImageName()));
}

void DarkEntity::move() {
	if (m_size == 0) {
		setHP(0);
		return;
	}

	// If not maximum size, seek darkness
	//    If in darkness, increase size and damage
	//    return
	// 
	// If maximum size, then
	//    Begin chasing player and set a flag indicating this
	//
	// For every turn in bright enough light, it reduces in size
	//    Each reduction in size reduces its damage
	//
	// When size reaches 0, it dies
}



// ================================
//		  :::: SMASHER ::::
// ================================

Smasher::Smasher(Dungeon *dungeon) 
	: Monster(dungeon, BOSSCOLS / 2, 3, 500, 0, 4, 1, "Smasher"), 
	m_moveActive(false), m_moveEnding(true), m_frenzy(false) {
	setFlying(true);

	setSprite(dungeon->createSprite(getPosX(), getPosY(), 3, "C_Wall_Terrain1_48x48.png"));
	getSprite()->setColor(cocos2d::Color3B(55, 30, 40));
	getSprite()->setScale(3.0f * GLOBAL_SPRITE_SCALE);

	int x = getPosX();
	int y = getPosY();
	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {
			(*dungeon)[j*dungeon->getCols() + i].upper = SMASHER;
			(*dungeon)[j*dungeon->getCols() + i].enemy = true;
		}
	}
}

void Smasher::moveTo(int x, int y, float time) {
	int cols = m_dungeon->getCols();

	for (int i = getPosX() - 1; i < getPosX() + 2; i++) {
		for (int j = getPosY() - 1; j < getPosY() + 2; j++) {
			(*m_dungeon)[j*cols + i].upper = SPACE;
			(*m_dungeon)[j*cols + i].enemy = false;
		}
	}

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {
			(*m_dungeon)[j*cols + i].upper = SMASHER;
			(*m_dungeon)[j*cols + i].enemy = true;
		}
	}

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);

	setPosX(x); setPosY(y);
	getSprite()->setLocalZOrder(y + Z_ACTOR);
}
void Smasher::move() {
	//	check if smasher is currently executing a move
	if (m_moveActive) {
		switch (m_moveType) {
		case 1: move1(); break;
		case 2: move2(); break;
		case 3: move3(); break;
		case 4: move4(); break;
		case 5: move5(); break;
		default: break;
		}
	}
	// if smasher has finished a move and hasn't come off cooldown,
	// then continue the cooldown process
	else if (!m_moveEnding) {
		switch (m_moveType) {
		case 1: resetUpward(); break;
		case 2: {
			switch (m_move) {
			case 'u': resetDownward(); break;
			case 'l': resetRightward(); break;
			case 'r': resetLeftward(); break;
			}

			break;
		}
		case 3: break;
		case 4: break;
		case 5: break;
		default: break;
		}
	}
	// smasher is not executing a move, so randomly select a new one
	else {
		pickMove();
	}
}

void Smasher::pickMove() { // select a move to use
	int col = getPosX();
	int row = getPosY();
	int randmove = 1 + randInt(100);


	// 12% chance : smash attack
	if (randmove > 0 && randmove <= 12) {
		m_moveActive = true;
		m_moveType = 1;
		
		move1();
	}
	// 15% chance : avalanche attack
	else if (randmove > 12 && randmove <= 27) {
		m_moveActive = true;
		m_moveType = 2;
		
		move2();
	}
	// 3% chance : archer/enemy spawning, max 3 at once
	else if (randmove > 27 && randmove <= 30) {
		if (m_dungeon->monsterCount() < 3)
			move3();
		else
			moveSmasher();
	}
	else if (randmove == 0) {
		move4();
	}
	else if (randmove == 0) {
		move5();
	}
	// 70% chance : random movement
	else if (randmove > 30) {
		moveSmasher();
	}

}
void Smasher::move1() { // smashes!
	int x = getPosX();
	int y = getPosY();

	//	if wall immediately beneath smasher is an unbreakable wall, reset position to top
	if (m_dungeon->boundary(x, y + 2)) {
		m_moveEnding = false;
		m_moveActive = false;
		resetUpward();

		return;
	}

	moveSmasher('d');
}
void Smasher::move2() { // starts avalanche
	int x = getPosX();
	int y = getPosY();

	// if this is the beginning of the move, then select a direction to move in
	if (m_move == '0') {
		switch (randInt(3)) {
		case 0: m_move = 'u'; break;
		case 1: m_move = 'l'; break;
		case 2: m_move = 'r'; break;
		}
	}

	//	move is upward
	if (m_move == 'u') {
		//	if wall immediately above smasher is an unbreakable wall, make rocks fall
		if (m_dungeon->boundary(x, y - 2)) {
			playSound("Rock_Slide.mp3");

			m_moveEnding = false;
			m_moveActive = false;
			
			avalanche();
			resetDownward();

			return;
		}

		moveSmasher('u');
	}
	//	move is to the left
	else if (m_move == 'l') {
		//	if wall immediately to the left of smasher is an unbreakable wall, make rocks fall
		if (m_dungeon->boundary(x - 2, y)) {
			playSound("Rock_Slide.mp3");

			m_moveEnding = false;
			m_moveActive = false;
			
			avalanche();
			resetRightward();		

			return;
		}

		moveSmasher('l');
	}
	//	move is to the right
	else if (m_move == 'r') {
		//	if wall immediately to the right of smasher is an unbreakable wall, make rocks fall
		if (m_dungeon->boundary(x + 2, y)) {
			playSound("Rock_Slide.mp3");

			m_moveEnding = false;
			m_moveActive = false;
			
			avalanche();
			resetLeftward();

			return;
		}

		moveSmasher('r');
	}

}
void Smasher::avalanche() {
	int x, y, speed, spikecount;
	spikecount = 15 + randInt(8);

	for (int i = 0; i < spikecount; i++) {
		x = 1 + randInt(BOSSCOLS - 2);
		y = 1 + randInt(5);
		speed = 1 + randInt(3);

		while (m_dungeon->trap(x, y) || m_dungeon->wall(x, y)) {
			x = 1 + randInt(BOSSCOLS - 2);
			y = 1 + randInt(5);
		}

		m_dungeon->addTrap(std::make_shared<FallingSpike>(*m_dungeon, x, y, speed));
	}
}
void Smasher::move3() {	// spawn two enemies
	int cols = m_dungeon->getCols();
	int rows = m_dungeon->getRows();
	
	int n = 2;
	int x, y;

	playSound("Smasher_Spawns.mp3");

	if (m_dungeon->monsterCount() <= 2) {
		while (n > 0) {
			x = 1 + randInt(cols - 2);
			y = rows - 2 - randInt(10);

			while (m_dungeon->enemy(x, y) || m_dungeon->hero(x, y)) {
				x = 1 + randInt(cols - 2);
				y = rows - 2 - randInt(10);
			}

			m_dungeon->addMonster(std::make_shared<AngledBouncer>(m_dungeon, x, y));
			n--;
		}
	}
}
void Smasher::move4() { // earthquake

}
void Smasher::move5() { // projectile bouncers

}

void Smasher::moveSmasher(char dir) { // pick a random direction to move in
	int x = getPosX();
	int y = getPosY();

	char move;

	// Pick random direction
	if (dir == '-') {
		switch (randInt(4)) {
		case 0: move = 'l'; break;
		case 1: move = 'r'; break;
		case 2: move = 'u'; break;
		case 3: move = 'd'; break;
		default: break;
		}
	}
	else
		move = dir;
	
	int n = 0, m = 0;
	int p = n, q = m;
	switch (move) {
	case 'l': n = -2; m = -1; break;
	case 'r': n = 2; m = -1; break;
	case 'u': n = -1; m = -2; break;
	case 'd': n = -1; m = 2; break;
	default: break;
	}

	// If wall immediately adjacent to smasher is an unbreakable wall, do nothing
	if (m_dungeon->boundary(x + n, y + m))
		return;

	int range = 2;
	int currentRange = 1;

	int k = 1; // 3 rows/columns to check
	while (!m_dungeon->boundary(x + n, y + m) && currentRange <= range) {

		p = n; q = m;

		while (k <= 3) {

			if (m_dungeon->hero(x + n, y + m) || m_dungeon->enemy(x + n, y + m))
				m_dungeon->linearActorPush(x + n, y + m, range - currentRange + 1, 1, move);
			
			switch (move) {
			case 'l': m++; break;
			case 'r': m++; break;
			case 'u': n++; break;
			case 'd': n++; break;
			}

			k++;
		}

		switch (move) {
		case 'l': n--; m = -1; break;
		case 'r': n++; m = -1; break;
		case 'u': n = -1; m--; break;
		case 'd': n = -1; m++; break;
		default: break;
		}

		k = 1;
		currentRange++;
	}

	// Adjust coordinates since the value of n/m is not equal to number of units to move Smasher
	switch (move) {
	case 'l': p = n + 2; q = 0; break;
	case 'r': p = n - 2; q = 0; break;
	case 'u': p = 0; q = m + 2; break;
	case 'd': p = 0; q = m - 2; break;
	}

	// Move Smasher
	switch (move) {
	case 'l': moveTo(x + p, y); break;
	case 'r': moveTo(x + p, y); break;
	case 'u': moveTo(x, y + q); break;
	case 'd': moveTo(x, y + q); break;
	}

	// Check if anything was crushed
	if (m_dungeon->boundary(x + n, y + m)) {
		x = getPosX();
		y = getPosY();

		playSound("Smasher_Smash.mp3");

		switch (move) {
		case 'l': n = -1; m = -1; break;
		case 'r': n = 1; m = -1; break;
		case 'u': n = -1; m = -1; break;
		case 'd': n = -1; m = 1; break;
		}

		for (int i = 0; i < 3; i++) {

			if (m_dungeon->hero(x + n, y + m))
				attack(*m_dungeon->getPlayer());
			
			if (m_dungeon->enemy(x + n, y + m)) {
				int pos = m_dungeon->findMonster(x + n, y + m);
				if (pos != -1)
					m_dungeon->monsterAt(pos)->setDestroyed(true);				
			}

			switch (move) {
			case 'l': n = -1; m++; break;
			case 'r': n = 1; m++; break;
			case 'u': n++; m = -1; break;
			case 'd': n++; m = 1; break;
			}

		}
	}

}

void Smasher::resetLeftward() {

	int col = getPosX();
	int row = getPosY();
	int rand = randInt(3);

	//	if smasher has reset far enough to the left, end cooldown
	if (col < BOSSCOLS / 1.5 - rand) {
		m_moveEnding = true;
		m_move = '0';

		return;
	}

	moveSmasher('l');
}
void Smasher::resetRightward() {

	int col = getPosX();
	int row = getPosY();
	int rand = randInt(3);

	//	if smasher has reset far enough to the right, end cooldown
	if (col > BOSSCOLS / 3 + rand) {
		m_moveEnding = true;
		m_move = '0';

		return;
	}

	moveSmasher('r');
}
void Smasher::resetUpward() {
	int col = getPosX();
	int row = getPosY();
	int rand = randInt(3);

	if (row < 6 + rand) {
		m_moveEnding = true;

		return;
	}

	moveSmasher('u');
}
void Smasher::resetDownward() {
	int col = getPosX();
	int row = getPosY();
	int rand = randInt(3);

	if (row > 6 + rand) {
		m_moveEnding = true;
		m_move = '0';

		return;
	}

	moveSmasher('d');
}

void Smasher::attack(Player &p) {
	playSound("Smasher_HitBy1.mp3");

	int damage = 10 + randInt(3);
	m_dungeon->damagePlayer(damage, getDamageType());

	// stun player after being crushed
	p.addAffliction(std::make_shared<Stun>(2));
}
void Smasher::deathDrops() {
	int cols = m_dungeon->getCols();

	int x = getPosX();
	int y = getPosY();

	// pause current music, play victory music, and then resume music once finished
	cocos2d::experimental::AudioEngine::pauseAll();
	int id = playMusic("Victory! All Clear.mp3", false);

	cocos2d::experimental::AudioEngine::setFinishCallback(id,
		[](int id, std::string music) {
		cocos2d::experimental::AudioEngine::resumeAll();
	});

	std::shared_ptr<Objects> idol = std::make_shared<Idol>(cols / 2, 3);
	(*m_dungeon)[idol->getPosY()*cols + idol->getPosX()].object = idol;
	(*m_dungeon)[idol->getPosY()*cols + idol->getPosX()].item = true;
	(*m_dungeon)[idol->getPosY()*cols + idol->getPosX()].object->setSprite(m_dungeon->createSprite(idol->getPosX(), idol->getPosY(), -1, "Glass_Toy_48x48.png"));
	m_dungeon->addItem((*m_dungeon)[idol->getPosY()*cols + idol->getPosX()].object);

	for (int i = y - 1; i < y + 2; i++) {
		for (int j = x - 1; j < x + 2; j++) {
			(*m_dungeon)[i*cols + j].upper = SPACE;
			(*m_dungeon)[i*cols + j].enemy = false;
		}
	}
}


