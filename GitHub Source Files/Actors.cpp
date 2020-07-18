#include "AudioEngine.h"
#include "GUI.h"
#include "global.h"
#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include "Afflictions.h"
#include "FX.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <functional>

using std::to_string;

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
	
}

void Actors::checkAfflictions() {
	int pos;

	// turn sprite upright
	if (getSprite() != nullptr)
		getSprite()->setRotation(0);

	for (int i = 0; i < (int)m_afflictions.size(); i++) {
		m_afflictions[i]->afflict(*this);

		if (m_afflictions[i]->isExhausted()) {
			pos = i;
			m_afflictions.erase(m_afflictions.begin() + pos);

			i--; // prevents skipping of any afflictions
		}
	}
}
int Actors::findAffliction(std::string name) {
	for (int i = 0; i < (int)m_afflictions.size(); i++) {
		if (m_afflictions.at(i)->getName() == name) {
			return i;
		}
	}
	return -1;
}
void Actors::addAffliction(std::shared_ptr<Afflictions> affliction) {

	if (isPlayer()) {

		// If player has bonus roll, and it's a certain negative type, then try to save the roll
		if (affliction->getName() == BURN || affliction->getName() == POISON || affliction->getName() == FREEZE || affliction->getName() == POSSESSED) {
			Player &p = dynamic_cast<Player&>(*this);
			if (p.hasBonusRoll() && 1 + randInt(100) + p.getLuck() > 60)
				return;
		}
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

	// otherwise, add more turns remaining to the existing affliction
	m_afflictions.at(index)->setTurnsLeft(m_afflictions.at(index)->getTurnsLeft() + affliction->getTurnsLeft());

}
void Actors::removeAffliction(std::string name) {
	int pos = findAffliction(name);

	if (pos == -1)
		return;

	m_afflictions.at(pos)->setTurnsLeft(0); // set turns to zero and let the affliction do the rest
}


// =================================================
//				:::: THE PLAYERS ::::
// =================================================
Player::Player(int hp, std::shared_ptr<Weapon> weapon) : Actors(0, 0, hp, 2, 2, 2, weapon, "Player"), m_maxhp(hp) {	
	setPlayerFlag(true);
	setMonsterFlag(false);
}
Player::~Player() {

}

void Player::moveTo(Dungeon &dungeon, int x, int y, float time) {
	int cols = dungeon.getCols();

	dungeon[getPosY()*cols + getPosX()].hero = false;
	dungeon[y*cols + x].hero = true;
	setPosX(x); setPosY(y);

	// queue player move
	dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY(), time);
}
void Player::move(Dungeon &dungeon, char move) {
	int cols = dungeon.getCols();
	int rows = dungeon.getRows();

	int x = getPosX();
	int y = getPosY();

	bool boundary, wall, item, trap, enemy, npc, spirit;
	std::string wall_type;

	// reset player's blocking stance
	setBlock(false);

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
	setAction(move);

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
	if (!isEthereal() && dungeon[y * cols + x].wall && (dungeon[y*cols + x].wall_type != DOOR_HORIZONTAL_OPEN && dungeon[y*cols + x].wall_type != DOOR_VERTICAL_OPEN)) {
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
			getWeapon()->usePattern(dungeon, moveUsed);
		

		if (!moveUsed) {

			// if it was a movement action
			if (move == 'l' || move == 'r' || move == 'u' || move == 'd') {
				// determines which way to move
				int n = 0, m = 0;

				switch (move) {
				case 'l': n = -1; m = 0; break;
				case 'r': n = 1; m = 0; break;
				case 'u': n = 0; m = -1; break;
				case 'd': n = 0; m = 1; break;
				}
				
				wall_type = dungeon[(y + m)*cols + (x + n)].wall_type;
				boundary = dungeon[(y + m)*cols + (x + n)].boundary;
				wall = (isEthereal() ? false : dungeon[(y + m)*cols + (x + n)].wall);
				item = dungeon[(y + m)*cols + (x + n)].item;
				trap = dungeon[(y + m)*cols + (x + n)].trap;
				enemy = dungeon[(y + m)*cols + (x + n)].enemy;
				npc = dungeon[(y + m)*cols + (x + n)].npc;
				spirit = dungeon[(y + m)*cols + (x + n)].spirit;

				if (npc) {
					dungeon.interactWithNPC(x + n, y + m);
				}
				else if (spirit && !wall) {
					playSound("Devils_Gift.mp3");

					cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
					dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

					moveTo(dungeon, x + n, y + m);

					getSprite()->setVisible(false);
					setSuperDead(true);
					return;
				}
				else if (!(wall || enemy || boundary)) {
					dungeon[y*cols + x].hero = false;
					dungeon[(y + m)*cols + (x + n)].hero = true;
					setPosX(x + n); setPosY(y + m);

					// Pick up any gold/money that was on the ground
					if (dungeon[getPosY()*cols + getPosX()].gold > 0) {
						dungeon.goldPickup(getPosX(), getPosY());

						if (hasGoldInvulnerability()) {
							setInvulnerable(true);
							addAffliction(std::make_shared<Invulnerability>(1));
						}
					}
					
					// For Spelunker
					if (item && dungeon[(y + m)*cols + (x + n)].object->getName() == ROCKS && getName() == SPELUNKER) {
						playSound(dungeon[(y + m)*cols + (x + n)].object->getSoundName());

						equipActiveItem(dungeon, dungeon[(y + m)*cols + (x + n)].object);
						dungeon[(y + m)*cols + (x + n)].object = nullptr;
						dungeon[(y + m)*cols + (x + n)].item_name = EMPTY;
						dungeon[(y + m)*cols + (x + n)].item = false;

						dungeon.removeSprite(dungeon.item_sprites, x + n, y + m);

						moveTo(dungeon, x + n, y + m);
					}

					if (trap) {
						// queue player move
						dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY());

						if (!dungeon.specialTrapCheck(x + n, y + m))
							dungeon.trapEncounter(x + n, y + m);
						
					}
					
					if (dungeon.isShop()) {

						if (dungeon[(y + m)*cols + (x + n)].shop_action == PURCHASE) {
							//auto scene = dynamic_cast<ShopScene*>(dungeon.m_scene);

							dungeon.constructShopHUD(x + n, y + m);
						}
						else if (dungeon[(y + m)*cols + (x + n)].shop_action != PURCHASE) {
							dungeon.deconstructShopHUD();
						}
					}
				}
				else if (wall && !boundary) {

					if (item && dungeon[(y + m)*cols + (x + n)].object->isChest()) {
						// If it's a chest, open it
						if (dungeon[(y + m)*cols + (x + n)].object->isChest()) {
							std::shared_ptr<Chests> chest = std::dynamic_pointer_cast<Chests>(dungeon[(y + m)*cols + (x + n)].object);					
							chest->attemptOpen(dungeon);
							chest.reset();
						}
					}				

					// Wall breaking ability
					else if (wall_type == REG_WALL && canBreakWalls()) {
						dungeon.damagePlayer(10);
						dungeon[(y + m)*cols + (x + n)].wall = false;
						dungeon.removeSprite(dungeon.wall_sprites, x + n, y + m);
					}

					// For traps that act as walls that can be interacted with (such as Braziers)
					else if (trap) {
						dungeon.trapEncounter(x + n, y + m, true);
					}

					else if (wall_type == DOOR_HORIZONTAL || wall_type == DOOR_VERTICAL) {
						dungeon.openDoor(x + n, y + m);
					}
					else if (wall_type == DOOR_HORIZONTAL_OPEN || wall_type == DOOR_VERTICAL_OPEN) {
						dungeon[y*cols + x].hero = false;
						dungeon[(y + m)*cols + (x + n)].hero = true;
						setPosX(x + n); setPosY(y + m);
					}
					else if (wall_type == DOOR_HORIZONTAL_LOCKED || wall_type == DOOR_VERTICAL_LOCKED) {
						// play locked door sound
						playSound("Locked.mp3");
					}
				}
				else if (enemy) {
					dungeon.fight(x + n, y + m);
				}
			}

			// Collect/Interact
			else if (move == 'e') {
				int currentLevel = dungeon.getLevel();
				
				// do this if item needs to be purchased
				std::string shop_action = dungeon[y*cols + x].shop_action;
				if (shop_action == "purchase")
					dungeon.purchaseItem(x, y);
				else
					dungeon.collectItem(x, y);

				if (dungeon.getLevel() > currentLevel)
					return;
			}

			// Spacebar item
			else if (move == 'b') {
				useActiveItem(dungeon);
			}

			// Switch weapon
			else if (move == 'w') {
				switchWeapon();
			}

			// Check if player is idling over a trap
			if (!(move == 'l' || move == 'r' || move == 'u' || move == 'd')) {
				if (dungeon[getPosY()*cols + getPosX()].trap)
					dungeon.trapEncounter(getPosX(), getPosY());
			}

		}

	}

	// Queue player move
	if (getPosX() != x || getPosY() != y) {
		dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY());
		playFootstepSound();
	}

	// Pick up any gold/money that was on the ground
	if (dungeon[getPosY()*cols + getPosX()].gold > 0) {
		dungeon.goldPickup(getPosX(), getPosY());

		if (hasGoldInvulnerability()) {
			setInvulnerable(true);
			addAffliction(std::make_shared<Invulnerability>(1));
		}
	}

}
void Player::attack(Dungeon &dungeon, Actors &a) {
	int playerPoints = getDex() + getWeapon()->getDexBonus();
	int monsterPoints = a.getDex();

	// If the player has no weapon equipped (i.e. their hands), do nothing
	if (getWeapon()->getName() == HANDS)
		return;

	// if player is invisible, they lose invisibility when attacking
	if (isInvisible())
		removeAffliction(INVISIBILITY);
	
	// if monster is frozen, attack is automatically successful albeit the ice increases the enemy's armor
	if (a.isFrozen()) {
		// play enemy hit
		playEnemyHit();

		// Check additional effects of attacking successfully
		checkExtraAttackEffects(dungeon, a);

		// damage the monster or otherwise
		int weaponDamage = getWeapon()->getDmg() * (m_dualWield ? 2 : 1);
		int damage = std::max(1, weaponDamage + (1 + randInt(std::max(1, getStr()))) - (2 * a.getArmor()));
		//int damage = std::max(1, getStr() + getWeapon()->getDmg() - (2 * a.getArmor()));

		// Sharpness bonus
		damage += getWeapon()->getSharpnessBonus();

		// bonus damage from bloodlust, if they have bloodlust
		if (hasBloodlust()) {
			float percent = getHP() / (float)getMaxHP();
			if (percent > 0.25f && percent <= 0.4f) damage += 1;
			else if (percent > 0.15f && percent <= 0.25f) damage += 2;
			else if (percent > 0.10f && percent <= 0.15f) damage += 3;
			else if (percent > 0.05f && percent <= 0.10f) damage += 5;
			else if (percent > 0.00f && percent <= 0.05f) damage += 8;
		}

		a.setHP(a.getHP() - damage);

		// :::: Check for affliction/ability of weapon ::::
		if (getWeapon()->hasAbility())
			getWeapon()->useAbility(dungeon, a);
		
		// Imbuements
		if (getWeapon()->isImbued())
			getWeapon()->useImbuement(dungeon, a);

		// tint monster sprite red and flash
		runMonsterDamageBlink(a.getSprite());

		// additional effects as a result of attacking successfully
		successfulAttack(dungeon, a);

		return;
	}

	// if monster is stunned, attack is automatically successful
	if (a.isStunned()) {
		// play enemy hit
		playEnemyHit();

		// Check additional effects of attacking successfully
		checkExtraAttackEffects(dungeon, a);

		// damage the monster or otherwise
		int weaponDamage = getWeapon()->getDmg() * (m_dualWield ? 2 : 1);
		int damage = std::max(1, weaponDamage + (1 + randInt(std::max(1, getStr()))) - a.getArmor());
		//int damage = std::max(1, getStr() + getWeapon()->getDmg() - (1 * a.getArmor()));

		// Sharpness bonus
		damage += getWeapon()->getSharpnessBonus();

		// bonus damage from bloodlust, if they have bloodlust
		if (hasBloodlust()) {
			float percent = getHP() / (float)getMaxHP();
			if (percent > 0.25f && percent <= 0.4f) damage += 1;
			else if (percent > 0.15f && percent <= 0.25f) damage += 2;
			else if (percent > 0.10f && percent <= 0.15f) damage += 3;
			else if (percent > 0.05f && percent <= 0.10f) damage += 5;
			else if (percent > 0.00f && percent <= 0.05f) damage += 8;
		}

		a.setHP(a.getHP() - damage);

		// :::: Check for affliction/ability of weapon ::::
		if (getWeapon()->hasAbility())
			getWeapon()->useAbility(dungeon, a);
		
		// Imbuements
		if (getWeapon()->isImbued())
			getWeapon()->useImbuement(dungeon, a);

		// tint monster sprite red and flash
		runMonsterDamageBlink(a.getSprite());

		// additional effects as a result of attacking successfully
		successfulAttack(dungeon, a);

		return;
	}

	// attack roll successful
	if ((playerPoints > 0 ? randInt(playerPoints) : 0) >= (monsterPoints > 0 ? randInt(monsterPoints) : 0)) {
		
		std::string monster = a.getName();

		// play hit sound effect
		if (monster == "Smasher")
			playHitSmasher();		
		else
			playEnemyHit();
		

		// Check additional effects of attacking successfully
		checkExtraAttackEffects(dungeon, a);

		// Check for affliction/ability of weapon
		if (getWeapon()->hasAbility())
			getWeapon()->useAbility(dungeon, a);
		
		// Imbuements
		if (getWeapon()->isImbued())
			getWeapon()->useImbuement(dungeon, a);


		/** Damage is calculated by the following:
		* 
		*   Base damage: Weapon's damage
		*   Player Str increases max damage.
		*   Max damage:  Base + Str
		*
		*   Damage = Base + (Value from 1 to Str) - Enemy's armor
		*   Dual wielding doubles a weapon's damage.
		*/
		int weaponDamage = getWeapon()->getDmg() * (m_dualWield ? 2 : 1);
		int damage = std::max(1, weaponDamage + (1 + randInt(std::max(1, getStr()))) - a.getArmor());

		// Sharpness bonus
		damage += getWeapon()->getSharpnessBonus();

		// bonus damage from bloodlust, if they have bloodlust
		if (hasBloodlust()) {
			float percent = getHP() / (float)getMaxHP();
			if (percent > 0.25f && percent <= 0.4f) damage += 1;
			else if (percent > 0.15f && percent <= 0.25f) damage += 2;
			else if (percent > 0.10f && percent <= 0.15f) damage += 3;
			else if (percent > 0.05f && percent <= 0.10f) damage += 5;
			else if (percent > 0.00f && percent <= 0.05f) damage += 8;
		}

		// damage the monster
		a.setHP(a.getHP() - damage);

		// tint monster sprite red and flash
		runMonsterDamage(a.getSprite());

		// check sturdiness of monster (if hitting them results in them getting knocked back)
		if ((!a.isSturdy() || hasSteelPunch()) && !a.isDead()) {
			int cols = dungeon.getCols();
			bool wall, enemy;
			int ax = a.getPosX();
			int ay = a.getPosY();

			int n = 0, m = 0;
			knockbackDirection(n, m, ax, ay);

			wall = dungeon[(ay + m)*cols + (ax + n)].wall;
			enemy = dungeon[(ay + m)*cols + (ax + n)].enemy;
			if (!(wall || enemy)) {
				a.moveTo(dungeon, ax + n, ay + m);

				// if knocking them back was successful, stun them for a turn
				a.addAffliction(std::make_shared<Stun>(1));
			}

		}

		// additional effects as a result of attacking successfully that are unique to a player
		successfulAttack(dungeon, a);

	}
	// attack roll failed
	else {
		playMiss();
	}
}
void Player::checkExtraAttackEffects(Dungeon &dungeon, Actors &a) {
	// increase money bonus on successful hit
	increaseMoneyBonus();

	// Can lifesteal
	if (canLifesteal())
		rollHeal();

	// Can poison enemies
	if (hasPoisonTouch() && (a.canBePoisoned() || hasAfflictionOverride())) {
		if (1 + randInt(100) + getLuck() > 95) {
			int turns = 5 + (hasHarshAfflictions() ? 5 : 0);
			a.addAffliction(std::make_shared<Poison>(turns, 2, 1, 1));
		}
	}

	// Can ignite enemies
	if (hasFireTouch() && (a.canBeBurned() || hasAfflictionOverride())) {
		if (1 + randInt(100) + getLuck() > 95) {
			int turns = 5 + (hasHarshAfflictions() ? 5 : 0);
			a.addAffliction(std::make_shared<Burn>(turns));
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
		chainLightning(dungeon, a);
	

	// Check if weapon is metered
	if (getWeapon()->hasMeter())
		getWeapon()->increaseCharge();

}
void Player::botchedAttack(Dungeon &dungeon, Actors &a) {
	// Metal sound effect
	playHitSmasher();

	// check sturdiness of monster (if hitting them results in them getting knocked back)
	if (!a.isSturdy() && !a.isDead()) {
		int cols = dungeon.getCols();
		bool wall, enemy;
		int ax = a.getPosX();
		int ay = a.getPosY();

		int n = 0, m = 0;
		switch (facingDirection()) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(ay + m)*cols + (ax + n)].wall;
		enemy = dungeon[(ay + m)*cols + (ax + n)].enemy;
		if (!(wall || enemy)) {
			dungeon[(ay)*cols + (ax)].enemy = false;
			dungeon[(ay + m)*cols + (ax + n)].enemy = true;
			a.setPosX(ax + n); a.setPosY(ay + m);
			dungeon.queueMoveSprite(a.getSprite(), ax + n, ay + m);

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
		else{
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
void Player::chainLightning(Dungeon &dungeon, Actors &a) {
	int ax, ay;
	int cols = dungeon.getCols();

	// Use dummy map so we don't have to unmark tiles at the end
	std::vector<_Tile> dummyMap = dungeon.getDungeon();
	
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
				if (dungeon[j*cols + i].enemy) {
					int pos = dungeon.findMonster(i, j);
					if (pos != -1)
						st.push(dungeon.getMonsters().at(pos));				
				}
			}
		}

		// Now damage the current actor
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, ax, ay, 2);

		actor->setHP(actor->getHP() - (a.getStr() + a.getInt()));
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
void Player::storeWeapon(Dungeon &dungeon, std::shared_ptr<Weapon> weapon, bool shop) {

	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// If the player is empty-handed because they threw a weapon or it broke, then directly equip the new weapon
	if (getWeapon()->getName() == HANDS) {
		setWeapon(weapon);

		dungeon[y*cols + x].object = nullptr;
		dungeon[y*cols + x].item_name = EMPTY;
		dungeon[y*cols + x].item = false;
		dungeon.removeSprite(dungeon.item_sprites, x, y - shop);

		return;
	}

	// Check for dual wielding capability
	if (weapon->canDualWield() && weapon->getName() == getWeapon()->getName()) {
		m_dualWield = true;
	}

	// If this is the first time a player is picking up a new weapon, simply add it to their inventory
	if (m_storedWeapon == nullptr) {
		m_storedWeapon = weapon;

		dungeon[y*cols + x].object = nullptr;
		dungeon[y*cols + x].item_name = EMPTY;
		dungeon[y*cols + x].item = false;
		dungeon.removeSprite(dungeon.item_sprites, x, y - shop);

		return;
	}

	// Otherwise we drop the player's current weapon and switch it with the new one
	dropWeapon(dungeon, shop);
	setWeapon(weapon);

	if (getWeapon()->hasBonus())
		getWeapon()->applyBonus(*this);
}
void Player::dropWeapon(Dungeon &dungeon, bool shop) {
	int cols = dungeon.getCols();
	
	int x = getPosX();
	int y = getPosY();

	// Unapply any previous bonuses
	if (getWeapon()->hasBonus())
		getWeapon()->unapplyBonus(*this);

	std::shared_ptr<Objects> oldWeapon = getWeapon();

	dungeon[y*cols + x].object = oldWeapon;
	dungeon[y*cols + x].item_name = oldWeapon->getName();
	dungeon[y*cols + x].item = true;

	dungeon.removeSprite(dungeon.item_sprites, x, y - shop);
	dungeon.addSprite(dungeon.item_sprites, x, y, -1, oldWeapon->getImageName());
}
void Player::dropStoredWeapon(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// Remove dual wielding, if applicable
	if (m_dualWield)
		m_dualWield = false;

	std::shared_ptr<Objects> storedWeapon = m_storedWeapon;

	if (dungeon[y*cols + x].item)
		dungeon.itemHash(x, y);

	dungeon[y*cols + x].object = storedWeapon;
	dungeon[y*cols + x].item_name = storedWeapon->getName();
	dungeon[y*cols + x].item = true;

	dungeon.addSprite(dungeon.item_sprites, x, y, -1, storedWeapon->getImageName());

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
void Player::throwWeaponTo(Dungeon &dungeon, int x, int y) {
	int cols = dungeon.getCols();

	// Unapply any previous bonuses
	if (getWeapon()->hasBonus())
		getWeapon()->unapplyBonus(*this);

	// Remove dual wielding, if applicable
	if (m_dualWield)
		m_dualWield = false;

	std::shared_ptr<Objects> oldWeapon = getWeapon();

	if (dungeon[y*cols + x].item)
		dungeon.itemHash(x, y);

	dungeon[y*cols + x].object = oldWeapon;
	dungeon[y*cols + x].item_name = oldWeapon->getName();
	dungeon[y*cols + x].item = true;

	dungeon.addSprite(dungeon.item_sprites, x, y, -1, oldWeapon->getImageName());

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
				m_items.at(i)->increaseCountBy(drop->getCount());

				itemAdded = true;
				return;
			}
		}
	}

	if ((int)m_items.size() < m_maxiteminv) {
		playSound(drop->getSoundName());
		m_items.push_back(drop);
		itemAdded = true;
	}
}
void Player::use(Dungeon &dungeon, int index) {
	// if item is meant for player, do this
	if (m_items.at(index)->forPlayer())
		m_items.at(index)->useItem(*this);

	// otherwise use it on the dungeon/surroundings
	else
		m_items.at(index)->useItem(dungeon);
	

	if (m_items.at(index)->getName() != SKELETON_KEY) {

		// Resonant Spells passive: 50% base chance to keep the spell
		if (m_resonantSpells && m_items.at(index)->isSpell()) {
			if (1 + randInt(100) + getLuck() > 50)
				return;
		}

		if (m_items.at(index)->canStack() || hasFatStacks()) {
			m_items.at(index)->decreaseCount();
			//std::shared_ptr<Stackable> currentStackable = std::dynamic_pointer_cast<Stackable>(m_items.at(index));

			if (m_items.at(index)->getCount() == 0)
				m_items.erase(m_items.begin() + index);

			//currentStackable.reset();
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

void Player::equipPassive(Dungeon &dungeon, std::shared_ptr<Passive> passive) {
	m_passives.push_back(passive);
	passive->activate(dungeon);
}
void Player::removePassive(int index) {
	m_passives.at(index).reset();
	m_passives.erase(m_passives.begin() + index);
}

void Player::equipTrinket(Dungeon &dungeon, std::shared_ptr<Trinket> trinket, bool shop) {
	// temp pickup sound
	playSound("Potion_Pickup.mp3");
	
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	if (hasTrinket()) {
		getTrinket()->unapply(dungeon, *this);
		swapTrinket(dungeon, trinket);

		return;
	}
	
	dungeon[y*cols + x].item_name = EMPTY;
	dungeon[y*cols + x].object = nullptr;
	dungeon[y*cols + x].item = false;
	dungeon.removeSprite(dungeon.item_sprites, x, y - shop);

	setTrinketFlag(true);
	m_trinket = trinket;
	m_trinket->apply(dungeon, *this);
}
void Player::swapTrinket(Dungeon& dungeon, std::shared_ptr<Trinket> trinket, bool shop) {
	int cols = dungeon.getCols();
	int rows = dungeon.getRows();
	int x = this->getPosX();
	int y = this->getPosY();

	// play trinket drop sound
	playSound("Wood_Shield_Drop.mp3");

	std::shared_ptr<Objects> oldTrinket = getTrinket();

	dungeon[y*cols + x].object = oldTrinket;
	dungeon[y*cols + x].item_name = oldTrinket->getName();
	dungeon[y*cols + x].item = true;

	dungeon.removeSprite(dungeon.item_sprites, x, y - shop);
	dungeon.addSprite(dungeon.item_sprites, x, y, -1, oldTrinket->getImageName());

	// add new trinket
	setTrinketFlag(true);
	m_trinket = trinket;
	m_trinket->apply(dungeon, *this);
}
void Player::equipRelic(Dungeon &dungeon, std::shared_ptr<Trinket> relic) {
	playSound("Potion_Pickup.mp3");

	setTrinketFlag(true);
	m_trinket = relic;
	m_trinket->apply(dungeon, *this);
}
void Player::removeRelic(Dungeon &dungeon) {
	getTrinket()->unapply(dungeon, *this);
	m_trinket.reset();
	m_trinket = nullptr;
	setTrinketFlag(false);
}

void Player::setItemToFront(int index) {
	std::shared_ptr<Drops> temp = getItems()[0];
	getItems()[0] = getItems()[index];
	getItems()[index] = temp;
}

void Player::increaseFavorBy(int favor) {

	while (favor > 0 && m_favor < 20) {
		m_favor++;

		switch (m_favor) {
		case -15: m_timerReduction += 0.10f; break; // Level timer is restored
		case -11: 
		case -7: 
		case -3: 
		case 0: 
		case 4: setLuck(getLuck() + 5); break;
		case 8: setVision(getVision() + 1); break;
		case 12: // Free passive
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
		case 19: 
		case 15: setStr(getStr() - 1); setDex(getDex() - 1); setInt(getInt() - 1); setMaxHP(getMaxHP() / 1.05); break;
		case 11: 
		case 7: setVision(getVision() - 1); break;
		case 3: setLuck(getLuck() - 5); break;
		case 0: break;
		case -4: break; // Chests more likely to be rigged
		case -8: break; // Prices become more expensive
		case -12: break; // NPCs less likely to spawn
		case -16: m_timerReduction -= 0.10f; break; // Level timer becomes faster
		case -20: m_spiritActive = true; break; // Evil spirit chases player permanently
		default: break;
		}

		favor--;
	}
}

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


Adventurer::Adventurer() : Player(100, std::make_shared<ShortSword>()) {
	//setStr(1000);
	setName(ADVENTURER);
	setImageName("Player1_48x48.png");
	
	m_shield = std::make_shared<WoodShield>();
	setHasActiveItem(true);
	setActiveItem(m_shield);
	addItem(std::make_shared<Bomb>());
	addItem(std::make_shared<Matches>());

	if (getWeapon()->hasBonus())
		getWeapon()->applyBonus(*this);
}
void Adventurer::useActiveItem(Dungeon &dungeon) {

	if (m_shield == nullptr)
		return;

	// play setting up shield stance sound effect
	playSound("Shield_Stance.mp3");

	m_blocking = true;
	showShieldBlock(dungeon, *this);

	// If the player is dual wielding, then they have to drop their second weapon
	if (isDualWielding()) {
		dropStoredWeapon(dungeon);
	}
}
void Adventurer::equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop) {
	if (active->isShield()) {
		std::shared_ptr<Shield> shield = std::dynamic_pointer_cast<Shield>(active);
		equipShield(dungeon, shield, shop);
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
void Adventurer::useActiveAbility(Dungeon &dungeon, Actors &a) {
	m_shield->useAbility(dungeon, a);
}

void Adventurer::equipShield(Dungeon &dungeon, std::shared_ptr<Shield> shield, bool shop) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// If player has a shield, then drop and replace with new one
	if (m_shield != nullptr) {
		dungeon.addSprite(dungeon.item_sprites, x, y, -1, m_shield->getImageName()); // old sprite first

		// handles the placement of shield on the ground
		dropShield(dungeon.getDungeon(), cols);
		dungeon.removeSprite(dungeon.item_sprites, x, y - shop);

		m_shield = shield;
		setActiveItem(m_shield);

		return;
	}

	dungeon[y*cols + x].item_name = EMPTY;
	dungeon[y*cols + x].object = nullptr;
	dungeon[y*cols + x].item = false;
	dungeon.removeSprite(dungeon.item_sprites, x, y - shop);

	setHasActiveItem(true);
	m_shield = shield;
	setActiveItem(m_shield);

	// play shield equip sound
	//playShieldEquip(getPlayerShield()->getName());
}
void Adventurer::dropShield(std::vector<_Tile> &dungeon, const int cols) {
	int x = getPosX();
	int y = getPosY();

	// play shield drop sound
	playSound("Wood_Shield_Drop.mp3");

	std::shared_ptr<Objects> oldShield = m_shield;

	dungeon[y*cols + x].object = oldShield;
	dungeon[y*cols + x].item_name = oldShield->getName();
	dungeon[y*cols + x].item = true;
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


Spellcaster::Spellcaster() : Player(70, std::make_shared<ArcaneStaff>()) {
	setStr(1);
	setInt(3);
	setArmor(1);
	m_spell = std::make_shared<WindSpell>();
	setHasActiveItem(true);
	setActiveItem(m_spell);

	setName(SPELLCASTER);
	setImageName("Spellcaster_48x48.png");
}

void Spellcaster::useActiveItem(Dungeon &dungeon) {
	if (m_mp > 4) {
		if (m_spell->forPlayer())
			m_spell->useItem(*this);
		else
			m_spell->useItem(dungeon);

		m_mp -= 4;
	}

	// If the player is dual wielding, then they have to drop their second weapon
	if (isDualWielding()) {
		dropStoredWeapon(dungeon);
	}
}
void Spellcaster::equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop) {
	if (active->isSpell()) {
		std::shared_ptr<Spell> spell = std::dynamic_pointer_cast<Spell>(active);
		m_spell.reset();
		m_spell = spell;
		setActiveItem(m_spell);
		spell.reset();
	}
}
void Spellcaster::successfulAttack(Dungeon &dungeon, Actors &a) {
	if (m_mp < m_maxMP)
		m_mp++;
}


Spelunker::Spelunker() : Player(100, std::make_shared<Whip>()) {
	setArmor(1);
	setHasActiveItem(true);
	m_rocks.emplace_back(std::make_shared<Rocks>());
	m_rocks.emplace_back(std::make_shared<Rocks>());
	m_rocks.emplace_back(std::make_shared<Rocks>());
	setActiveItem(m_rocks[0]);
	addSecondaryWeapon(std::make_shared<Mattock>());

	setName(SPELUNKER);
	setImageName("Spelunker_48x48.png");
}

void Spelunker::useActiveItem(Dungeon &dungeon) {

	if (m_rocks.empty())
		return;

	m_rocks[0]->useItem(dungeon);
	m_rocks.erase(m_rocks.begin());

	if (m_rocks.empty()) {
		setHasActiveItem(false);
	}
}
void Spelunker::equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop) {
	if (active->getName() == ROCKS) {
		std::shared_ptr<Rocks> rock = std::dynamic_pointer_cast<Rocks>(active);
		m_rocks.push_back(rock);
		rock.reset();

		setHasActiveItem(true);
	}
}


Acrobat::Acrobat() : Player(80, std::make_shared<Zweihander>()) {
	m_stamina = 30;
	m_maxStamina = m_stamina;

	setHasActiveItem(true);
	m_item = std::make_shared<Mobility>();
	setActiveItem(m_item);

	setName(ACROBAT);
	setImageName("OutsideMan2.png");
}

void Acrobat::useActiveItem(Dungeon &dungeon) {
	if (m_stamina > 4) {
		m_item->useItem(dungeon);

		m_stamina -= 4;
	}
}
void Acrobat::equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop) {
	return;
}
void Acrobat::successfulAttack(Dungeon &dungeon, Actors &a) {
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

void TheMadman::useActiveItem(Dungeon &dungeon) {
	m_item->useItem(dungeon);
}
void TheMadman::equipActiveItem(Dungeon &dungeon, std::shared_ptr<Objects> active, bool shop) {
	return;
}


// ==========================================
//				:::: NPCS ::::
// ==========================================
NPC::NPC(int x, int y, std::string name, std::string image) : Actors(x, y, 100, 5, 5, 5, std::make_shared<ShortSword>(), name) {
	setImageName(image);
}

void NPC::talk(Dungeon& dungeon) {
	// if this is the first time the player has interacted with this NPC
	if (!m_interacted) {
		addInitialDialogue();
		playDialogue(dungeon);
		m_interacted = true;
	}
	// else if player has interacted but NPC has not be satisfied
	else if (m_interacted) {
		checkSatisfaction(dungeon);

		if (!m_satisfied) {
			addInteractedDialogue();
			playDialogue(dungeon);
		}
		else if (m_satisfied && !m_rewardGiven) {
			addSatisfiedDialogue();
			playDialogue(dungeon);
			reward(dungeon);
			m_rewardGiven = true;
		}
		else if (m_satisfied && m_rewardGiven) {
			addFinalDialogue();
			playDialogue(dungeon);
		}
	}

}

inline void NPC::playDialogue(Dungeon &dungeon) { dungeon.playNPCDialogue(*this, m_dialogue); };


//		CREATURE LOVER (Lionel)
CreatureLover::CreatureLover(int x, int y) : NPC(x, y, CREATURE_LOVER, "Dead_Mage_48x48.png") {
	m_wantedCreature = GOO_SACK;

	addDialogue("Oh... hello.");
	addDialogue("I love " + m_wantedCreature + "s.");
	addDialogue("Those " + m_wantedCreature + "s are just adorable little things!");
	addDialogue("Can you bring me one?");

	setInteractionLimit(5);
}

void CreatureLover::checkSatisfaction(Dungeon& dungeon) {
	int cols = dungeon.getCols();

	std::string monsterName;

	for (int y = getPosY() - 1; y < getPosY() + 2; y++) {
		for (int x = getPosX() - 1; x < getPosX() + 2; x++) {

			if (dungeon[y*cols + x].enemy) {
				int pos = dungeon.findMonster(x, y);

				if (pos != -1) {
					monsterName = dungeon.getMonsters()[pos]->getName();

					if (monsterName == m_wantedCreature) {
						playSound("Creature_Lover_Talk.mp3");

						dungeon[y*cols + x].enemy = false;
						m_creature = dungeon.getMonsters()[pos]; // give creature lover the monster
						dungeon.misc_sprites.push_back(m_creature->getSprite()); // Add to misc sprites for lighting purposes

						dungeon.getMonsters().erase(dungeon.getMonsters().begin() + pos);
						setSatisfaction(true);
						return;
					}
				}
			}
		}
	}
}
void CreatureLover::reward(Dungeon& dungeon) {
	int cols = dungeon.getCols();
	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	if (dungeon[y*cols + x].item) {
		dungeon.itemHash(x, y);
	}

	switch (1 + randInt(5)) {
	case 1: dungeon[y*cols + x].object = std::make_shared<MagicEssence>(); break;
	case 2:	dungeon[y*cols + x].object = std::make_shared<FireTouch>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<PotionAlchemy>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<LuckUp>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<Heavy>(); break;
	default: break;
	}

	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
	dungeon[y*cols + x].item = true;
	dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
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
Memorizer::Memorizer(Dungeon *dungeon, int x, int y) : NPC(x, y, MEMORIZER, "Orc_48x48.png") {
	m_dungeon = dungeon;

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
			reward(*m_dungeon);
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
void Memorizer::checkSatisfaction(Dungeon& dungeon) {
	return;
}
void Memorizer::reward(Dungeon& dungeon) {
	int cols = dungeon.getCols();
	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	if (dungeon[y*cols + x].item) {
		dungeon.itemHash(x, y);
	}

	switch (1 + randInt(5)) {
	case 1: dungeon[y*cols + x].object = std::make_shared<IronShield>(); break;
	case 2:	dungeon[y*cols + x].object = std::make_shared<Katana>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<FrostShield>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<ArmorDrop>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<GoldenLongSword>(); break;
	default: break;
	}

	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
	dungeon[y*cols + x].item = true;
	dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
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

//		SHOPKEEPER
Shopkeeper::Shopkeeper(int x, int y) : NPC(x, y, SHOPKEEPER, "Shopkeeper_48x48.png") {

	addDialogue("Huh. Hello.");
	addDialogue("Wasn't expecting to see someone like you down here.");
	addDialogue("Don't get too comfy. Things are not what they seem.");
	addDialogue("I've sealed the doors off so no monsters can get in.");
	addDialogue("Please, have a look around.");

	setInteractionLimit(5);
}

void Shopkeeper::checkSatisfaction(Dungeon& dungeon) {
	return;
}
void Shopkeeper::reward(Dungeon& dungeon) {
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
Blacksmith::Blacksmith(Dungeon *dungeon, int x, int y) : NPC(x, y, BLACKSMITH, "Shopkeeper_48x48.png") {
	m_dungeon = dungeon;
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
		m_dungeon->getPlayerVector().at(0)->getWeapon()->increaseSharpnessBy(20);
		m_dungeon->getPlayerVector().at(0)->setMoney(m_dungeon->getPlayerVector().at(0)->getMoney() - m_improveCost);
	}
	else {
		playCrowSound();
		addDialogue("Looks like your pockets are a little light.");
	}
}
void Blacksmith::buyItem() {

}
void Blacksmith::mingle() {

}

void Blacksmith::checkSatisfaction(Dungeon& dungeon) {
	return;
}
void Blacksmith::reward(Dungeon& dungeon) {
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
Enchanter::Enchanter(Dungeon *dungeon, int x, int y) : NPC(x, y, ENCHANTER, "Shopkeeper_48x48.png") {
	m_dungeon = dungeon;
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
			int cost = determineCost(Weapon::ImbuementType::BURNING);
			if (m_dungeon->getPlayer()->getMoney() >= cost) {
				playHitSmasher();
				addDialogue("Right away.");

				m_dungeon->getPlayer()->getWeapon()->setImbuement(Weapon::ImbuementType::BURNING, 10);
				m_dungeon->getPlayer()->setMoney(m_dungeon->getPlayer()->getMoney() - cost);
			}
			else {
				addDialogue("What, you don't have enough!");
			}
			
			break;
		}
			// Poison
		case 1: {
			int cost = determineCost(Weapon::ImbuementType::POISONING);
			if (m_dungeon->getPlayer()->getMoney() >= cost) {
				playHitSmasher();
				addDialogue("Absolutely.");

				m_dungeon->getPlayer()->getWeapon()->setImbuement(Weapon::ImbuementType::POISONING, 10);
				m_dungeon->getPlayer()->setMoney(m_dungeon->getPlayer()->getMoney() - cost);
			}
			else {
				addDialogue("Come back when you have enough money.");
			}

			break;
		}
			// Freeze
		case 2: {
			int cost = determineCost(Weapon::ImbuementType::FREEZING);
			if (m_dungeon->getPlayer()->getMoney() >= cost) {
				playHitSmasher();
				addDialogue("Good choice.");

				m_dungeon->getPlayer()->getWeapon()->setImbuement(Weapon::ImbuementType::FREEZING, 10);
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

	int igniteCost = determineCost(Weapon::ImbuementType::BURNING);
	addChoice("Imbue fire. (" + std::to_string(igniteCost) + " Gold)");

	int poisonCost = determineCost(Weapon::ImbuementType::POISONING);
	addChoice("Imbue poison. (" + std::to_string(poisonCost) + " Gold)");

	int freezeCost = determineCost(Weapon::ImbuementType::FREEZING);
	addChoice("Imbue freeze. (" + std::to_string(freezeCost) + " Gold)");

	addChoice("Nevermind.");
		
	m_promptStage = 2;
}
int Enchanter::determineCost(Weapon::ImbuementType type) {
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
	case Weapon::ImbuementType::BURNING:
		return imbuementLevel * 40;
	case Weapon::ImbuementType::POISONING:
		return imbuementLevel * 45;
	case Weapon::ImbuementType::FREEZING:
		return imbuementLevel * 50;
	default:
		return 0;
	}
}
void Enchanter::buyItem() {

}
void Enchanter::mingle() {

}

void Enchanter::checkSatisfaction(Dungeon& dungeon) {
	return;
}
void Enchanter::reward(Dungeon& dungeon) {

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
Trader::Trader(Dungeon *dungeon, int x, int y) : NPC(x, y, TRADER, "Spellcaster_48x48.png") {
	m_dungeon = dungeon;
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
		else if (m_dungeon->getPlayer()->getPassives().empty()) {
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
		else if (m_dungeon->getPlayer()->getItems().empty()) {
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
	int index = randInt(m_dungeon->getPlayer()->getPassives().size());
	m_dungeon->getPlayer()->removePassive(index);

	std::shared_ptr<Passive> passive(nullptr);
	switch (randInt(5)) {
	case 0: passive = std::make_shared<FireTouch>(); break;
	case 1: passive = std::make_shared<PoisonTouch>(); break;
	case 2: passive = std::make_shared<FrostTouch>(); break;
	case 3: passive = std::make_shared<PotionAlchemy>(); break;
	case 4: passive = std::make_shared<Berserk>(); break;
	}
	m_dungeon->getPlayer()->equipPassive(*m_dungeon, passive);
	
	m_passiveTraded = true;
	m_promptStage = 1;
}
void Trader::tradeItem() {
	int index = randInt(m_dungeon->getPlayer()->getItems().size());
	m_dungeon->getPlayer()->removeItem(index);

	std::shared_ptr<Drops> item(nullptr);
	switch (randInt(5)) {
	case 0: item = std::make_shared<Bomb>(); break;
	case 1: item = std::make_shared<SmokeBomb>(); break;
	case 2: item = std::make_shared<BearTrap>(); break;
	case 3: item = std::make_shared<RottenMeat>(); break;
	case 4: item = std::make_shared<Matches>(); break;
	}
	m_dungeon->getPlayer()->addItem(item);

	m_itemTraded = true;
	m_promptStage = 1;
}
void Trader::tradeWeapon() {
	std::shared_ptr<Weapon> item(nullptr);
	switch (randInt(5)) {
	case 0: item = std::make_shared<EnchantedPike>(); break;
	case 1: item = std::make_shared<RadiantPike>(); break;
	case 2: item = std::make_shared<CarbonFiberPike>(); break;
	case 3: item = std::make_shared<LuckyPike>(); break;
	case 4: item = std::make_shared<WoodBow>(); break;
	}
	m_dungeon->getPlayer()->tradeWeapon(item);

	m_weaponTraded = true;
	m_promptStage = 1;
}
void Trader::mingle() {

}

void Trader::checkSatisfaction(Dungeon& dungeon) {
	return;
}
void Trader::reward(Dungeon& dungeon) {

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
OutsideMan1::OutsideMan1(int x, int y) : NPC(x, y, OUTSIDE_MAN1, "Archer_48x48.png") {

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

OutsideMan2::OutsideMan2(int x, int y) : NPC(x, y, OUTSIDE_MAN2, "OutsideMan2.png") {

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

OutsideWoman1::OutsideWoman1(int x, int y) : NPC(x, y, OUTSIDE_WOMAN1, ".png") {

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



// ============================================
//				:::: MONSTERS ::::
// ============================================
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, std::string name)
	: Actors(x, y, hp, armor, str, dex, name) {
	setMonsterFlag(true);
}
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name)
	: Actors(x, y, hp, armor, str, dex, wep, name) {
	setMonsterFlag(true);
}
Monster::~Monster() {

}

void Monster::moveTo(Dungeon &dungeon, int x, int y, float time) {
	int cols = dungeon.getCols();

	dungeon[getPosY()*cols + getPosX()].enemy = false;
	dungeon[y*cols + x].enemy = true;

	dungeon.queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);
}
inline void Monster::moveCheck(Dungeon& dungeon) {
	if (isDead())
		return;

	move(dungeon);
}
void Monster::attack(Dungeon &dungeon, Player &p) {
	std::string monster = getName();
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
			damage = std::max(0, 1 + randInt(getStr() + getWeapon()->getDmg()) - (p.getArmor() + p.blockedDamageReduction()));
		else
			damage = std::max(0, 1 + randInt(getStr()) - (p.getArmor() + p.blockedDamageReduction()));

		if (p.activeHasAbility())
			p.useActiveAbility(dungeon, *this);	

		// handles shield durability change
		p.successfulBlock();

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}

		return;
	}

	// if monster's attack roll is successful
	if ((monsterPoints > 0 ? randInt(monsterPoints) : 0) >= (playerPoints > 0 ? randInt(playerPoints) : 0)) {

		if (m_hasWeapon)
			damage = std::max(1, getWeapon()->getDmg() + randInt(getStr()) - p.getArmor());
		else
			damage = std::max(1, 1 + randInt(getStr()) - p.getArmor());

		// Damage the player
		dungeon.damagePlayer(damage);
		
		// Extra effects that can occur after a successful attack
		extraAttackEffects(dungeon);
	}
	// else attack roll was unsuccessful
	else {
		;
	}
}
void Monster::death(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	dungeon[y*cols + x].enemy = false;
	spriteCleanup(dungeon);

	// If multisegmented, remove their segments
	if (isMultiSegmented())
		removeSegments(dungeon);

	// If they were destroyed, then don't drop anything
	if (m_destroyed)
		return;

	if (m_gold > 0) {
		int px = dungeon.getPlayer()->getPosX();
		int py = dungeon.getPlayer()->getPosY();
		playGoldDropSound(m_gold, px, py, x, y);

		dungeon[y*cols + x].gold += m_gold * dungeon.getPlayer()->getMoneyMultiplier() + dungeon.getPlayer()->getMoneyBonus();
		dungeon.addGoldSprite(x, y);
	}

	// If Spelunker, then all monsters have a chance to drop a Rock on death
	if (dungeon.getPlayer()->getName() == SPELUNKER) {
		if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 85) {

			if (dungeon[y*cols + x].item)
				dungeon.itemHash(x, y);
			
			dungeon[y*cols + x].object = std::make_shared<Rocks>(x, y);
			dungeon[y*cols + x].item_name = ROCKS;
			dungeon[y*cols + x].item = true;
			dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
		}
	}

	// check if monster drops extra stuff upon death
	deathDrops(dungeon);
	
}
void Monster::spriteCleanup(Dungeon &dungeon) {
	dungeon.queueRemoveSprite(getSprite());
}

bool Monster::attemptChase(std::vector<_Tile> &dungeon, int cols, int &shortest, int smelldist, int origdist, int x, int y, char &first_move, char &optimal_move) {

	// if enemy is immediately adjacent to the player, return to attack
	if (smelldist == origdist && \
		(dungeon[(y - 1)*cols + x].hero || dungeon[(y + 1)*cols + x].hero || dungeon[y*cols + x - 1].hero || dungeon[y*cols + x + 1].hero)) {
		dungeon[y*cols + x].marked = false;
		return true;
	}

	// mark the current tile as visited
	dungeon[y*cols + x].marked = true;

	// player was not found within origdist # of steps OR took more steps than the current shortest path found
	if (smelldist < 0 || smelldist < shortest) {
		dungeon[y*cols + x].marked = false;
		return false;
	}

	if (dungeon[y*cols + x].hero) {
		if (smelldist > shortest) {
			shortest = smelldist;
			optimal_move = first_move;
		}
		else if (smelldist == shortest) {
			// randomly select optimal path if multiple optimal paths found
			if (randInt(2) == 1)
				optimal_move = first_move;
		}
		dungeon[y*cols + x].marked = false;
		return true; // if player is found, tell goblin to advance
	}

	if (!dungeon[y*cols + x + 1].marked && (!dungeon[y*cols + x + 1].wall || dungeon[y*cols + x + 1].hero)) {
		if (smelldist == origdist)
			first_move = 'r';
		attemptChase(dungeon, cols, shortest, smelldist - 1, origdist, x + 1, y, first_move, optimal_move);
	}

	if (!dungeon[y*cols + x - 1].marked && (!dungeon[y*cols + x - 1].wall || dungeon[y*cols + x - 1].hero)) {
		if (smelldist == origdist)
			first_move = 'l';
		attemptChase(dungeon, cols, shortest, smelldist - 1, origdist, x - 1, y, first_move, optimal_move);
	}

	if (!dungeon[(y + 1)*cols + x].marked && (!dungeon[(y + 1)*cols + x].wall || dungeon[(y + 1)*cols + x].hero)) {
		if (smelldist == origdist)
			first_move = 'd';
		attemptChase(dungeon, cols, shortest, smelldist - 1, origdist, x, y + 1, first_move, optimal_move);
	}

	if (!dungeon[(y - 1)*cols + x].marked && (!dungeon[(y - 1)*cols + x].wall || dungeon[(y - 1)*cols + x].hero)) {
		if (smelldist == origdist)
			first_move = 'u';
		attemptChase(dungeon, cols, shortest, smelldist - 1, origdist, x, y - 1, first_move, optimal_move);
	}

	// unmark the tile as visited when backtracking
	dungeon[y*cols + x].marked = false;

	if (optimal_move != '0')
		return true;
	return false;
}
bool Monster::attemptSmartChase(Dungeon &dungeon, int cols, int &shortest, int currentDist, int origDist, int x, int y, char &first_move, char &optimal_move) {
	// if enemy is immediately adjacent to the player, return to attack
	if (currentDist == origDist && first_move != '1' && first_move != '2' && first_move != '3' && first_move != '4' &&
		(dungeon[(y - 1)*cols + x].hero || dungeon[(y + 1)*cols + x].hero || dungeon[y*cols + x - 1].hero || dungeon[y*cols + x + 1].hero)) {
		dungeon[y*cols + x].marked = false;
		return true;
	}
	else if (currentDist == origDist && first_move != 'l' && first_move != 'r' && first_move != 'u' && first_move != 'd' &&
		(dungeon[(y - 1)*cols + (x - 1)].hero || dungeon[(y + 1)*cols + (x + 1)].hero || dungeon[(y - 1)*cols + x + 1].hero || dungeon[(y + 1)*cols + x - 1].hero)) {
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
	if (dungeon[y*cols + x].hero) {
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
	if (!dungeon[(y + 1)*cols + x + 1].marked && (!dungeon[(y + 1)*cols + x + 1].wall || dungeon[(y + 1)*cols + x + 1].hero)) {
		if (currentDist == origDist)
			first_move = '4';
		attemptSmartChase(dungeon, cols, shortest, currentDist - 1, origDist, x + 1, y + 1, first_move, optimal_move);
	}

	if (!dungeon[(y + 1)*cols + x - 1].marked && (!dungeon[(y + 1)*cols + x - 1].wall || dungeon[(y + 1)*cols + x - 1].hero)) {
		if (currentDist == origDist)
			first_move = '3';
		attemptSmartChase(dungeon, cols, shortest, currentDist - 1, origDist, x - 1, y + 1, first_move, optimal_move);
	}

	if (!dungeon[(y - 1)*cols + (x + 1)].marked && (!dungeon[(y - 1)*cols + (x + 1)].wall || dungeon[(y - 1)*cols + (x + 1)].hero)) {
		if (currentDist == origDist)
			first_move = '2';
		attemptSmartChase(dungeon, cols, shortest, currentDist - 1, origDist, x + 1, y - 1, first_move, optimal_move);
	}

	if (!dungeon[(y - 1)*cols + (x - 1)].marked && (!dungeon[(y - 1)*cols + (x - 1)].wall || dungeon[(y - 1)*cols + (x - 1)].hero)) {
		if (currentDist == origDist)
			first_move = '1';
		attemptSmartChase(dungeon, cols, shortest, currentDist - 1, origDist, x - 1, y - 1, first_move, optimal_move);
	}

	//// if diagonal move was found successful, return early since a diagonal move is optimal by default
	//if (optimal_move != '0') {
	//	dungeon[y*cols + x].marked = false;
	//	return true;
	//}

	// Normal checking
	if (!dungeon[y*cols + x + 1].marked && (!dungeon[y*cols + x + 1].wall || dungeon[y*cols + x + 1].hero)) {
		if (currentDist == origDist)
			first_move = 'r';
		attemptSmartChase(dungeon, cols, shortest, currentDist - 1, origDist, x + 1, y, first_move, optimal_move);
	}

	if (!dungeon[y*cols + x - 1].marked && (!dungeon[y*cols + x - 1].wall || dungeon[y*cols + x - 1].hero)) {
		if (currentDist == origDist)
			first_move = 'l';
		attemptSmartChase(dungeon, cols, shortest, currentDist - 1, origDist, x - 1, y, first_move, optimal_move);
	}

	if (!dungeon[(y + 1)*cols + x].marked && (!dungeon[(y + 1)*cols + x].wall || dungeon[(y + 1)*cols + x].hero)) {
		if (currentDist == origDist)
			first_move = 'd';
		attemptSmartChase(dungeon, cols, shortest, currentDist - 1, origDist, x, y + 1, first_move, optimal_move);
	}

	if (!dungeon[(y - 1)*cols + x].marked && (!dungeon[(y - 1)*cols + x].wall || dungeon[(y - 1)*cols + x].hero)) {
		if (currentDist == origDist)
			first_move = 'u';
		attemptSmartChase(dungeon, cols, shortest, currentDist - 1, origDist, x, y - 1, first_move, optimal_move);
	}

	// unmark the tile as visited when backtracking
	dungeon[y*cols + x].marked = false;

	if (optimal_move != '0')
		return true;
	return false;
}
void Monster::attemptGreedyChase(Dungeon &dungeon, bool diagonals, int x, int y) {
	int sx, sy, ex, ey, xDiff, yDiff;
	sx = getPosX();
	sy = getPosY();
	ex = (x == -1 ? dungeon.getPlayer()->getPosX() : x);
	ey = (y == -1 ? dungeon.getPlayer()->getPosY() : y);

	xDiff = sx - ex;
	yDiff = sy - ey;

	// Same column
	if (xDiff == 0) {
		// Positive y difference means the enemy is below the player
		if (yDiff > 0) {
			attemptMove(dungeon, 'u'); // move upward
		}
		else {
			attemptMove(dungeon, 'd'); // move downward
		}
	}
	// Same row
	else if (yDiff == 0) {
		// Negative x difference means the enemy is left of the player
		if (xDiff < 0) {
			attemptMove(dungeon, 'r'); // move right
		}
		else {
			attemptMove(dungeon, 'l'); // move left
		}
	}
	else {
		char move;
		// Player is southeast of the monster
		if (xDiff < 0 && yDiff < 0) {
			if (diagonals) {
				if (attemptMove(dungeon, '4'))
					return;		
			}
			
			move = (randInt(2) == 0 ? 'd' : 'r');
			if (!attemptMove(dungeon, move)) {
				move = (move == 'd' ? 'r' : 'd');
				attemptMove(dungeon, move);
			}
			
		}
		// Player is northeast of the monster
		else if (xDiff < 0 && yDiff > 0) {
			if (diagonals) {
				if (attemptMove(dungeon, '2'))
					return;			
			}
			
			move = (randInt(2) == 0 ? 'u' : 'r');
			if (!attemptMove(dungeon, move)) {
				move = (move == 'u' ? 'r' : 'u');
				attemptMove(dungeon, move);
			}
			
		}
		// Player is southwest of the monster
		else if (xDiff > 0 && yDiff < 0) {
			if (diagonals) {
				if (attemptMove(dungeon, '3'))
					return;
			}

			move = (randInt(2) == 0 ? 'd' : 'l');
			if (!attemptMove(dungeon, move)) {
				move = (move == 'd' ? 'l' : 'd');
				attemptMove(dungeon, move);
			}
		}
		// Player is northwest of the monster
		else /*(xDiff > 0 && yDiff > 0)*/ {
			if (diagonals) {
				if (attemptMove(dungeon, '1'))
					return;
			}

			move = (randInt(2) == 0 ? 'u' : 'l');
			if (!attemptMove(dungeon, move)) {
				move = (move == 'u' ? 'l' : 'u');
				attemptMove(dungeon, move);
			}
		}

	}
}

bool Monster::playerInRange(const Player& p, int range) {
	int px = p.getPosX();
	int py = p.getPosY();

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
bool Monster::playerInDiagonalRange(const Player& p, int range) {
	int px = p.getPosX();
	int py = p.getPosY();

	int mx = getPosX();
	int my = getPosY();

	// if monster is on the same column/row as player, then just do regular range check
	if (px == mx || py == my)
		return playerInRange(p, range);

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
bool Monster::playerIsAdjacent(const Player &p, bool diagonals) {

	int px = p.getPosX();
	int py = p.getPosY();

	int x = getPosX();
	int y = getPosY();

	if (diagonals)
		return abs(px - x) <= 1 && abs(py - y) <= 1;

	return abs(px - x) + abs(py - y) == 1;
}
bool Monster::attemptMove(Dungeon &dungeon, char move) {
	int cols = dungeon.getCols();
	int x = getPosX(); int y = getPosY();

	int n = 0, m = 0;

	// Key: (for moveSprite 'move' parameter)
	// 1 u 2
	// l _ r
	// 3 d 4

	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	case '1': n = -1; m = -1; break;
	case '2': n = 1; m = -1; break;
	case '3': n = -1; m = 1; break;
	case '4': n = 1; m = 1; break;
	}

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

	if (dungeon[(y + m)*cols + (x + n)].hero) {
		attack(dungeon, *dungeon.getPlayerVector().at(0));
		return true;
	}

	if (dungeon[(y + m)*cols + (x + n)].wall && !isEthereal())
		return false;

	bool trap = dungeon[(y + m)*cols + (x + n)].trap;
	if (trap) {
		int pos = dungeon.findTrap(x + n, y + m);

		if (pos != -1) {
			bool lethal = dungeon.getTraps()[pos]->isLethal();

			// if trap would do damage to them and they are a smart enemy, they do nothing
			if (lethal && isSmart())
				return false;
		}
	}

	if (!dungeon[(y + m)*cols + (x + n)].enemy || isSpirit()) {
		moveTo(dungeon, x + n, y + m);

		return true;
	}

	return false;
}
bool Monster::moveWithSuccessfulChase(Dungeon &dungeon, char move) {
	int cols = dungeon.getCols();
	int x = getPosX(); int y = getPosY();

	int n = 0, m = 0;

	// Key: (for moveSprite 'move' parameter)
	// 1 u 2
	// l _ r
	// 3 d 4

	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	case '1': n = -1; m = -1; break;
	case '2': n = 1; m = -1; break;
	case '3': n = -1; m = 1; break;
	case '4': n = 1; m = 1; break;
	default: attack(dungeon, *dungeon.getPlayerVector().at(0)); return true;
	}

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
		default: attack(dungeon, *dungeon.getPlayerVector().at(0)); return true;
		}
	}

	if (dungeon[(y + m)*cols + (x + n)].wall)
		return false;

	bool trap = dungeon[(y + m)*cols + (x + n)].trap;
	if (trap) {
		int pos = dungeon.findTrap(x + n, y + m);

		if (pos != -1) {
			bool lethal = dungeon.getTraps()[pos]->isLethal();

			// if trap would do damage to them and they are a smart enemy, they do nothing
			if (lethal && isSmart())
				return false;
		}
	}

	if (!dungeon[(y + m)*cols + (x + n)].enemy) {
		moveTo(dungeon, x + n, y + m);

		return true;
	}

	return false;
}

char Monster::moveMonsterRandomly(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int mx = getPosX();
	int my = getPosY();

	int n = randInt(2);
	int m = 0;
	char move;

	bool leftwall, rightwall, upwall, downwall;
	leftwall = dungeon[my*cols + mx - 1].wall;
	rightwall = dungeon[my*cols + mx + 1].wall;
	upwall = dungeon[(my - 1)*cols + mx].wall;
	downwall = dungeon[(my + 1)*cols + mx].wall;

	bool leftenemy, rightenemy, upenemy, downenemy;
	leftenemy = dungeon[my*cols + mx - 1].enemy;
	rightenemy = dungeon[my*cols + mx + 1].enemy;
	upenemy = dungeon[(my - 1)*cols + mx].enemy;
	downenemy = dungeon[(my + 1)*cols + mx].enemy;

	bool lefttrap, righttrap, uptrap, downtrap;
	lefttrap = dungeon[my*cols + mx - 1].trap;
	righttrap = dungeon[my*cols + mx + 1].trap;
	uptrap = dungeon[(my - 1)*cols + mx].trap;
	downtrap = dungeon[(my + 1)*cols + mx].trap;

	bool lefthero, righthero, uphero, downhero;
	lefthero = dungeon[my*cols + mx - 1].hero;
	righthero = dungeon[my*cols + mx + 1].hero;
	uphero = dungeon[(my - 1)*cols + mx].hero;
	downhero = dungeon[(my + 1)*cols + mx].hero;

	//  if monster is surrounded completely, do nothing (such as goblin or wanderer blocking an archer's path)
	if ((leftwall || leftenemy || lefttrap || lefthero) && (rightwall || rightenemy || righttrap || righthero) &&
		(downwall || downenemy || downtrap || downhero) && (upwall || upenemy || uptrap || uphero)) {
		return '-';
	}
	//	if monster is surrounded by walls on left and right
	else if ((leftwall || leftenemy || lefttrap || lefthero) && (rightwall || rightenemy || righttrap || righthero)) {
		move = 'v';
	}
	//	if monster is surrounded by walls on top and bottom
	else if ((downwall || downenemy || downtrap || downhero) && (upwall || upenemy || uptrap || uphero)) {
		move = 'h';
	}
	//	else pick a random direction
	else {
		move = (n == 0 ? 'h' : 'v');
	}

	char upper;
	bool wall, enemy, trap, hero;

	//	move is horizontal
	if (move == 'h') {
		n = -1 + randInt(3);

		upper = dungeon[my*cols + mx + n].upper;
		wall = dungeon[my*cols + mx + n].wall;
		enemy = dungeon[my*cols + mx + n].enemy;
		trap = dungeon[my*cols + mx + n].trap;
		hero = dungeon[my*cols + mx + n].hero;

		while (n == 0 || wall || enemy || trap || hero || upper != SPACE) {
			n = -1 + randInt(3);

			upper = dungeon[my*cols + mx + n].upper;
			wall = dungeon[my*cols + mx + n].wall;
			enemy = dungeon[my*cols + mx + n].enemy;
			trap = dungeon[my*cols + mx + n].trap;
			hero = dungeon[my*cols + mx + n].hero;
		}

		moveTo(dungeon, mx + n, my);

		return n == -1 ? 'l' : 'r';
	}
	//	move is vertical
	else /*if (move == 'v')*/ {
		m = -1 + randInt(3);

		upper = dungeon[(my + m)*cols + mx].upper;
		wall = dungeon[(my + m)*cols + mx].wall;
		enemy = dungeon[(my + m)*cols + mx].enemy;
		trap = dungeon[(my + m)*cols + mx].trap;
		hero = dungeon[(my + m)*cols + mx].hero;

		while (m == 0 || wall || enemy || trap || hero || upper != SPACE) {
			m = -1 + randInt(3);

			upper = dungeon[(my + m)*cols + mx].upper;
			wall = dungeon[(my + m)*cols + mx].wall;
			enemy = dungeon[(my + m)*cols + mx].enemy;
			trap = dungeon[(my + m)*cols + mx].trap;
			hero = dungeon[(my + m)*cols + mx].hero;
		}

		moveTo(dungeon, mx, my + m);

		return m == -1 ? 'u' : 'd';
	}
}
bool Monster::wallCollision(Dungeon &dungeon, char direction, int p_move, int m_move) {
	int cols = dungeon.getCols();

	// if they're one space apart, return false
	if (p_move - m_move == 1)
		return false;

	// p_move and m_move are the x/y position of the player and monster, respectively
	// direction: the axis for the enemy to check for walls along
	// wallCollision() is only called when an enemy has a potential line of sight on the player

	// Since player and monster are on the same row/column, set otherdir to the complement of p_move/m_move
	int otherdir;

	if (direction == 'x') {
		otherdir = dungeon.getPlayerVector()[0]->getPosY();
	}
	else {
		otherdir = dungeon.getPlayerVector()[0]->getPosX();
	}

	bool wall;

	// if player is below or to the right of the enemy, let monster check moving in the direction opposite (otherdir)
	if (p_move > m_move) {
		p_move--;
		while (p_move - m_move != 0) {
			if (direction == 'x') {

				wall = dungeon[otherdir*cols + p_move].wall;
				if (wall)
					return true;
				else
					p_move--;
			}
			else if (direction == 'y') {

				wall = dungeon[(p_move)*cols + otherdir].wall;
				if (wall)
					return true;
				else
					p_move--;
			}
		}
	}
	else {
		m_move--;
		while (m_move - p_move != 0) {
			if (direction == 'x') {

				wall = dungeon[otherdir*cols + m_move].wall;
				if (wall)
					return true;
				else
					m_move--;
			}
			else if (direction == 'y') {

				wall = dungeon[(m_move)*cols + otherdir].wall;
				if (wall)
					return true;
				else
					m_move--;
			}
		}
	}
	return false;
}
bool Monster::clearLineOfSight(Dungeon &dungeon, const Player &p) {
	int cols = dungeon.getCols();
	int px = p.getPosX();
	int py = p.getPosY();

	int x = getPosX();
	int y = getPosY();

	bool enemy, wall, hero = false;

	//int n, m;
	char dir;

	dir = (px > x && py == y ? 'r' : px < x && py == y ? 'l' : py > y && px == x ? 'd' : 'u');

	switch (dir) {
	case 'l': {
		x--;
		wall = dungeon[y*cols + x].wall;
		enemy = dungeon[y*cols + x].enemy;
		hero = dungeon[y*cols + x].hero;
		while (!(wall || enemy) && px != x) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				return true;
			}

			x--;
			wall = dungeon[y*cols + x].wall;
			enemy = dungeon[y*cols + x].enemy;
			hero = dungeon[y*cols + x].hero;
		}
		break;
	}
	case 'r': {
		x++;
		wall = dungeon[y*cols + x].wall;
		enemy = dungeon[y*cols + x].enemy;
		hero = dungeon[y*cols + x].hero;
		while (!(wall || enemy) && px != x) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				return true;
			}

			x++;
			wall = dungeon[y*cols + x].wall;
			enemy = dungeon[y*cols + x].enemy;
			hero = dungeon[y*cols + x].hero;
		}
		break;
	}
	case 'u': {
		y--;
		wall = dungeon[y*cols + x].wall;
		enemy = dungeon[y*cols + x].enemy;
		hero = dungeon[y*cols + x].hero;
		while (!(wall || enemy) && py != y) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				return true;
			}

			y--;
			wall = dungeon[y*cols + x].wall;
			enemy = dungeon[y*cols + x].enemy;
			hero = dungeon[y*cols + x].hero;
		}
		break;
	}
	case 'd': {
		y++;
		wall = dungeon[y*cols + x].wall;
		enemy = dungeon[y*cols + x].enemy;
		hero = dungeon[y*cols + x].hero;
		while (!(wall || enemy) && py != y) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				return true;
			}

			y++;
			wall = dungeon[y*cols + x].wall;
			enemy = dungeon[y*cols + x].enemy;
			hero = dungeon[y*cols + x].hero;
		}
		break;
	}
	}
	
	if (hero)
		return true;

	return false;
}


//		BREAKABLES
Breakables::Breakables(int x, int y, int hp, int armor, std::string name, std::string image) : Monster(x, y, hp, armor, 0, 0, name) {
	setImageName(image);
}
SturdyBreakables::SturdyBreakables(int x, int y, int hp, int armor, std::string name, std::string image) 
	: Breakables(x, y, hp, armor, name, image) {
	setSturdy(true);
}

WeakCrate::WeakCrate(int x, int y) : SturdyBreakables(x, y, 1, 1, WEAK_CRATE, "Large_Crate0001.png") {

}
void WeakCrate::deathDrops(Dungeon &dungeon) {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Large_Crate%04d.png", 7);
	std::function<void(Dungeon&, std::vector<cocos2d::Sprite*>&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	dungeon.runAnimationWithCallback(frames, 30, x, y, 1, callback, "Large_Crate0007.png");
}

WeakBarrel::WeakBarrel(int x, int y) : SturdyBreakables(x, y, 1, 1, WEAK_BARREL, "Barrel0001.png") {

}
void WeakBarrel::deathDrops(Dungeon &dungeon) {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Barrel%04d.png", 7);
	std::function<void(Dungeon&, std::vector<cocos2d::Sprite*>&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	dungeon.runAnimationWithCallback(frames, 30, x, y, 1, callback, "Barrel0007.png");
}

SmallPot::SmallPot(int x, int y) : SturdyBreakables(x, y, 1, 1, WEAK_POT, "Ceramic_Pot0001.png") {

}
void SmallPot::deathDrops(Dungeon &dungeon) {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Ceramic_Pot%04d.png", 7);
	std::function<void(Dungeon&, std::vector<cocos2d::Sprite*>&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	dungeon.runAnimationWithCallback(frames, 30, x, y, 1, callback, "Ceramic_Pot0007.png");
}

LargePot::LargePot(int x, int y) : SturdyBreakables(x, y, 1, 1, WEAK_LARGE_POT, "Large_Ceramic_Pot0001.png") {

}
void LargePot::deathDrops(Dungeon &dungeon) {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Large_Ceramic_Pot%04d.png", 7);
	std::function<void(Dungeon&, std::vector<cocos2d::Sprite*>&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	dungeon.runAnimationWithCallback(frames, 30, x, y, 1, callback, "Large_Ceramic_Pot0007.png");
}

Sign::Sign(int x, int y) : SturdyBreakables(x, y, 1, 1, WEAK_SIGN, "Large_Sign0001.png") {

}
void Sign::deathDrops(Dungeon &dungeon) {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Large_Sign%04d.png", 6);
	std::function<void(Dungeon&, std::vector<cocos2d::Sprite*>&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	dungeon.runAnimationWithCallback(frames, 30, x, y, 1, callback, "Large_Sign0006.png");
}

ArrowSign::ArrowSign(int x, int y) : SturdyBreakables(x, y, 1, 1, WEAK_ARROW_SIGN, "Large_Arrow_Sign0001.png") {

}
void ArrowSign::deathDrops(Dungeon &dungeon) {
	int x = getPosX();
	int y = getPosY();

	// Animation of the crate breaking
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Large_Arrow_Sign%04d.png", 6);
	std::function<void(Dungeon&, std::vector<cocos2d::Sprite*>&, int, int, int, std::string)> callback = &Dungeon::addSprite;
	dungeon.runAnimationWithCallback(frames, 30, x, y, 1, callback, "Large_Arrow_Sign0006.png");
}


//		FORGOTTEN SPIRIT
ForgottenSpirit::ForgottenSpirit(Dungeon &dungeon, int x, int y) : Monster(x, y, 1, 1, 1, 1, FORGOTTEN_SPIRIT) {
	setFlying(true);
	setEthereal(true);

	setImageName("Old_Orc.png");
	setSprite(dungeon.createSprite(x, y, 4, getImageName()));
	getSprite()->setOpacity(180);
}

void ForgottenSpirit::moveTo(Dungeon &dungeon, int x, int y, float time) {
	int cols = dungeon.getCols();

	dungeon[getPosY()*cols + getPosX()].spirit = false;
	dungeon[y*cols + x].spirit = true;

	dungeon.queueMoveSprite(getSprite(), x, y, time);

	setPosX(x); setPosY(y);
}
void ForgottenSpirit::move(Dungeon &dungeon) {

	if (m_turns > 0) {
		m_turns--;
		return;
	}

	int x = getPosX();
	int y = getPosY();
	
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	// Check if player moved on top of the spirit
	if (x == px && y == py) {
		attack(dungeon, *dungeon.getPlayer());
		return;
	}

	attemptGreedyChase(dungeon);
	m_turns = 4;
}
void ForgottenSpirit::attack(Dungeon &dungeon, Player &p) {
	playSound("Devils_Gift.mp3");

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
	dungeon.runSingleAnimation(frames, 120, px, py, 2);

	moveTo(dungeon, px, py);

	p.getSprite()->setVisible(false);
	p.setSuperDead(true);
}


//		MONSTER SEGMENT
MonsterSegment::MonsterSegment(int x, int y, int sx, int sy, std::string name, std::string image) : Monster(x, y, 1000, 1, 1, 1, name), m_parentX(sx), m_parentY(sy) {
	setImageName(image);
}


//		GOBLIN FUNCTIONS
Goblin::Goblin(int x, int y, int smelldist) : Monster(x, y, randInt(5) + 10, 1, 5, 1, GOBLIN), m_smelldist(smelldist) {
	setChasesPlayer(true);
	setSturdy(false);
	setImageName("Goblin_48x48.png");
	setGold(3);
}

void Goblin::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	char first, best;
	first = best = '0';
	int shortest = 0;

	if (playerInRange(*dungeon.getPlayer(), m_smelldist)) {
		// if viable path is found
		if (attemptChase(dungeon.getDungeon(), cols, shortest, m_smelldist, m_smelldist, getPosX(), getPosY(), first, best)) {
			moveWithSuccessfulChase(dungeon, best);
		}
	}
}
void Goblin::deathDrops(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	bool item = dungeon[y*cols + x].item;
	// 15% drop chance
	if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 90) {
		if (item)
			dungeon.itemHash(x, y);
		
		dungeon[y*cols + x].object = std::make_shared<BoneAxe>(x, y);
		dungeon[y*cols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
	}
}


//		WANDERER FUNCTIONS
Wanderer::Wanderer(int x, int y) : Monster(x, y, 10, 1, 3, 1, WANDERER) {
	setFlying(true);
	setImageName("Wanderer_48x48.png");
	setGold(1);
}
Wanderer::Wanderer(int x, int y, std::string name, std::string image, int hp, int armor, int str, int dex)
	: Monster(x, y, hp, armor, str, dex, name) {
	setFlying(true);
	setImageName(image);
}

void Wanderer::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int mx = getPosX();
	int my = getPosY();

	int n = -1 + randInt(3);
	int m = -1 + randInt(3);
	bool wall, enemy, hero;

	wall = dungeon[(my + m)*cols + mx + n].wall;
	enemy = dungeon[(my + m)*cols + mx + n].enemy;
	hero = dungeon[(my + m)*cols + mx + n].hero;

	// if the randomly selected move is not a wall or enemy, move them
	if (!(wall || enemy)) {

		// if it's not the player, then move them to that space
		if (!hero) {
			moveTo(dungeon, mx + n, my + m);
		}
		// otherwise attack the player
		else {
			attack(dungeon, *dungeon.getPlayerVector().at(0));
		}
	}
}
void Wanderer::deathDrops(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon[y*cols + x].item;

	// 80% drop chance
	if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 20) {
		if (item) {
			dungeon.itemHash(x, y);
		}
		dungeon[y*cols + x].object = std::make_shared<HeartPod>(x, y);
		dungeon[y*cols + x].item_name = HEART_POD;
		dungeon[y*cols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
	}
}

RabidWanderer::RabidWanderer(int x, int y) : Wanderer(x, y, RABID_WANDERER, "Green_Wanderer_48x48.png", 8 + randInt(4), 2, 2, 1) {
	setGold(5);
}

void RabidWanderer::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int mx = getPosX();
	int my = getPosY();

	int n = -1 + randInt(3);
	int m = -1 + randInt(3);

	// If it's not ready to move, get ready to move
	if (!m_turn) {
		m_turn = true;
		return;
	}

	bool wall, enemy, hero;

	wall = dungeon[(my + m)*cols + mx + n].wall;
	enemy = dungeon[(my + m)*cols + mx + n].enemy;
	hero = dungeon[(my + m)*cols + mx + n].hero;

	// if the randomly selected move is not a wall or enemy, move them
	if (!(wall || enemy)) {

		// if it's not the player, then move them to that space
		if (!hero) {
			moveTo(dungeon, mx + n, my + m);
		}
		// otherwise attack the player
		else {
			attack(dungeon, *dungeon.getPlayerVector().at(0));
		}
	}

	m_turn = false;
}
void RabidWanderer::attack(Dungeon &dungeon, Player &p) {
	// playSound();

	dungeon.damagePlayer(getStr() + randInt(3));

	// if actor is still alive and can be poisoned, try to poison them
	if (p.getHP() > 0 && p.canBePoisoned()) {

		// chance to poison
		int roll = 1 + randInt(100) + p.getLuck();

		// 30% chance to be poisoned
		if (roll < 70) {
			p.addAffliction(std::make_shared<Poison>(8, 5, 1, 1));
		}
	}
}
void RabidWanderer::deathDrops(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon[y*cols + x].item;

	// 100% drop chance
	if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 0) {
		if (item) {
			dungeon.itemHash(x, y);
		}
		dungeon[y*cols + x].object = std::make_shared<HeartPod>(x, y);
		dungeon[y*cols + x].item_name = HEART_POD;
		dungeon[y*cols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
	}
}

SleepingWanderer::SleepingWanderer(int x, int y) : Wanderer(x, y, SLEEPING_WANDERER, "Pink_Wanderer_48x48.png", 10, 1, 4, 1) {
	setChasesPlayer(true);
	setGold(3);
}

void SleepingWanderer::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int mx = getPosX();
	int my = getPosY();

	int n = -1 + randInt(3);
	int m = -1 + randInt(3);
	bool wall, enemy, hero;

	wall = dungeon[(my + m)*cols + mx + n].wall;
	enemy = dungeon[(my + m)*cols + mx + n].enemy;
	hero = dungeon[(my + m)*cols + mx + n].hero;

	// If provoked, attempt to chase the player
	if (m_provoked) {
		char first, best;
		first = best = '0';
		int shortest = 0;

		if (playerInDiagonalRange(*dungeon.getPlayer(), m_range)) {
			attemptGreedyChase(dungeon, true);
			/*if (attemptSmartChase(dungeon, cols, shortest, m_range, m_range, getPosX(), getPosY(), first, best)) {
				moveWithSuccessfulChase(dungeon, best);
			}*/
		}
		else {
			m_provoked = false;
		}

		return;
	}

	// if player is too close, then wake them up
	if (playerInRange(*dungeon.getPlayer(), m_provocationRange)) {
		m_provoked = true;
	}
}
void SleepingWanderer::deathDrops(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon[y*cols + x].item;

	if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 95) {

		if (item)
			dungeon.itemHash(x, y);
		
		dungeon[y*cols + x].object = std::make_shared<BatWing>(x, y);
		dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
		dungeon[y*cols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
	}
}

ProvocableWanderer::ProvocableWanderer(int x, int y) : Wanderer(x, y, PROVOCABLE_WANDERER, "Orange_Wanderer_48x48.png", 12 + randInt(4), 1, 5, 1) {
	setGold(8);
	m_prevHP = getHP();
}

void ProvocableWanderer::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int mx = getPosX();
	int my = getPosY();

	if (m_prevHP > getHP())
		m_provoked = true;

	// If provoked, then try to attack the player
	if (m_provoked) {

		if (playerInRange(*dungeon.getPlayer(), 1)) {
			attack(dungeon, *dungeon.getPlayerVector().at(0));
		}

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

	bool wall, enemy, hero;

	wall = dungeon[(my + m)*cols + mx + n].wall;
	enemy = dungeon[(my + m)*cols + mx + n].enemy;
	hero = dungeon[(my + m)*cols + mx + n].hero;

	// if the randomly selected move is a free space, move them
	if (!(wall || enemy || hero)) {
		moveTo(dungeon, mx + n, my + m);
		
		m_turns = 2;
	}
}
void ProvocableWanderer::attack(Dungeon &dungeon, Player &p) {
	dungeon.damagePlayer(std::max(1, getStr() + randInt(4) - p.getArmor()));
}
void ProvocableWanderer::deathDrops(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon[y*cols + x].item;

	// 100% drop chance
	if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 0) {
		if (item) {
			dungeon.itemHash(x, y);
		}
		dungeon[y*cols + x].object = std::make_shared<HeartPod>(x, y);
		dungeon[y*cols + x].item_name = HEART_POD;
		dungeon[y*cols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
	}
}

FlameWanderer::FlameWanderer(int x, int y) : Wanderer(x, y, FLAME_WANDERER, "Red_Wanderer_48x48.png", 12 + randInt(5), 2, 3, 1) {
	setGold(6);
	setCanBeBurned(false);
}

void FlameWanderer::move(Dungeon &dungeon) {

	if (!m_turn) {
		m_turn = true;
		return;
	}

	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	if (playerIsAdjacent(*dungeon.getPlayer()))
		attack(dungeon, *dungeon.getPlayerVector()[0]);
	else
		moveMonsterRandomly(dungeon);

	m_turn = false;
}
void FlameWanderer::attack(Dungeon &dungeon, Player &p) {
	dungeon.damagePlayer(getStr() + randInt(6));

	// if actor is still alive and can be poisoned, try to poison them
	if (p.getHP() > 0 && p.canBeBurned()) {
		int roll = 1 + randInt(100) + p.getLuck();

		// 30% chance to be burned
		if (roll < 70)
			p.addAffliction(std::make_shared<Burn>(4));	
	}
}


//		ARCHER FUNCTIONS
Archer::Archer(int x, int y, int range) : Monster(x, y, randInt(5) + 9, 2, 3, 2, std::make_shared<WoodBow>(), ARCHER), m_primed(false), m_range(range) {
	setImageName("Archer_48x48.png");
	setGold(4);
	setSturdy(false);
	setHasWeapon(true);
}

void Archer::move(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	//	if archer is primed, check to attack and return
	if (m_primed) {
		// if player is in sight, shoot them
		if (x - mx == 0 || y - my == 0) {
			playSound("Bow_Release.mp3");

			attack(dungeon, *dungeon.getPlayerVector().at(0));
		}
		// otherwise relax their aim
		else {
			;
		}

		m_primed = false;

		return;
	}


	// if archer has dagger out and is adjacent to player, attack player
	if (getWeapon()->getName() == BRONZE_DAGGER && playerIsAdjacent(*dungeon.getPlayer())) {
		// player archer dagger swipe sound effect
		playArcherDaggerSwipe();

		attack(dungeon, *dungeon.getPlayerVector().at(0));
		return;
	}
	// else if dagger is out and player moved away, switch back to bow
	else if (getWeapon()->getName() == BRONZE_DAGGER && !playerIsAdjacent(*dungeon.getPlayer())) {
		playSound("Bow_Primed.mp3");

		setWeapon(std::make_shared<WoodBow>());
		return;
	}

	// if player is invisible, move randomly
	if (dungeon.getPlayer()->isInvisible()) {
		moveMonsterRandomly(dungeon);
		return;
	}

	// if player is not in the archer's sights, move archer
	if (x != mx && y != my) {
		moveMonsterRandomly(dungeon);

		return;
	}

	// if player and archer are on the same column
	if (x == mx) {
		//	if player is within range and not adjacent, get primed
		if (abs(y - my) > 1 && abs(y - my) <= m_range) {
			if (clearLineOfSight(dungeon, *dungeon.getPlayerVector()[0])) {
				playSound("Bow_Drawn.mp3");

				m_primed = true;
			}
			else {
				moveMonsterRandomly(dungeon);

				return;
			}
		}
		// swap weapon to dagger
		else if (abs(y - my) == 1) {
			playSound("Archer_Dagger_Switch2.mp3");

			setWeapon(std::make_shared<BronzeDagger>());
		}
		//	just move archer
		else {
			moveMonsterRandomly(dungeon);

			return;
		}
	}
	// if player and archer are on the same row
	else if (y == my) {
		//	if player is within range and not adjacent, get primed
		if (abs(x - mx) > 1 && abs(x - mx) <= m_range) {
			if (clearLineOfSight(dungeon, *dungeon.getPlayerVector()[0])) {
				playSound("Bow_Drawn.mp3");

				m_primed = true;
			}
			else {
				moveMonsterRandomly(dungeon);

				return;
			}
		}
		// swap weapon to dagger
		else if (abs(x - mx) == 1) {
			playSound("Archer_Dagger_Switch2.mp3");

			setWeapon(std::make_shared<BronzeDagger>());
		}
		// just move archer
		else {
			moveMonsterRandomly(dungeon);
		}
	}

}
void Archer::deathDrops(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon[y*cols + x].item;

	// 15% drop chance
	if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 85) {
		if (item) {
			dungeon.itemHash(x, y);
		}
		dungeon[y*cols + x].object = std::make_shared<BronzeDagger>();
		dungeon[y*cols + x].item_name = BRONZE_DAGGER;
		dungeon[y*cols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
	}
}


//		ZAPPER
Zapper::Zapper(Dungeon &dungeon, int x, int y, int rows) : Monster(x, y, 10 + randInt(3), 1, 4, 1, ZAPPER), m_cooldown(true), m_attack(randInt(2)) {

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
		sparks.insert(std::make_pair(i, dungeon.createSprite(n, m, 0, "Spark_48x48.png")));
		sparks[i]->setScale(0.75f);
		sparks[i]->setVisible(false);
	}

	setImageName("Zapper_48x48.png");
	setExtraSpritesFlag(true);
	setEmitsLight(true);
	setGold(2);
}

void Zapper::spriteCleanup(Dungeon &dungeon) {
	dungeon.queueRemoveSprite(getSprite());

	if (!sparks.empty()) {
		for (int i = 0; i < 8; i++) {
			dungeon.queueRemoveSprite(sparks[i]);
		}
	}
}

void Zapper::moveTo(Dungeon &dungeon, int x, int y, float time) {
	int cols = dungeon.getCols();

	dungeon[getPosY()*cols + getPosX()].enemy = false;
	dungeon[y*cols + x].enemy = true;

	dungeon.queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);

	moveSprites(x, y, dungeon.getRows());
}
void Zapper::move(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();


	//	if zapper is on cooldown, reset cooldown and switch attack direction
	if (onCooldown()) {
		if (cardinalAttack()) {
			// hide cardinal sparks
			for (int i = 0; i < 4; i++) {
				getSparks()[i]->setVisible(false);
			}
		}
		else {
			// hide diagonal sparks
			for (int i = 4; i < 8; i++) {
				getSparks()[i]->setVisible(false);
			}
		}
		setCooldown();
		swapDirection();
	}
	//	else throw out sparks and attack player if they were in the way
	else {
		if (cardinalAttack()) {
			// show cardinal sparks
			for (int i = 0; i < 4; i++) {
				getSparks()[i]->setVisible(true);
			}

			if ((px == mx + 1 && py == my) || (px == mx - 1 && py == my) || (px == mx && py == my + 1) || (px == mx && py == my - 1)) {
				attack(dungeon, *dungeon.getPlayerVector()[0]);
			}
		}
		else {
			// show diagonal sparks
			for (int i = 4; i < 8; i++) {
				getSparks()[i]->setVisible(true);
			}

			if ((px == mx - 1 && py == my - 1) || (px == mx + 1 && py == my - 1) || (px == mx - 1 && py == my + 1) || (px == mx + 1 && py == my + 1)) {
				attack(dungeon, *dungeon.getPlayerVector()[0]);
			}
		}
		setCooldown();
	}
}
void Zapper::attack(Dungeon &dungeon, Player &p) {
	playSound("Shock5.mp3");

	int damage = 1 + randInt(getStr());
	dungeon.damagePlayer(damage);
}

bool Zapper::onCooldown() const {
	return m_cooldown;
}
void Zapper::setCooldown() {
	m_cooldown = !m_cooldown;
}
bool Zapper::cardinalAttack() const {
	return m_attack;
}
void Zapper::swapDirection() {
	m_attack = !m_attack;
}
std::map<int, cocos2d::Sprite*> Zapper::getSparks() {
	return sparks;
}
void Zapper::moveSprites(int x, int y, int rows) {
	// set sprite positions
	for (int i = 0; i < 8; i++) {
		switch (i) {
			// cardinals
		case 0: getSparks()[i]->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET); break; // top
		case 1: getSparks()[i]->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET); break;// left
		case 2: getSparks()[i]->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET); break;// right
		case 3: getSparks()[i]->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;// bottom
			// diagonals
		case 4: getSparks()[i]->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;// topleft
		case 5: getSparks()[i]->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;// topright
		case 6: getSparks()[i]->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;// bottomleft
		case 7: getSparks()[i]->setPosition((x + 1)* SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;// bottomright
		}
		getSparks()[i]->setScale(0.75);
		getSparks()[i]->setVisible(false);
	}
}
void Zapper::setSpriteColor(cocos2d::Color3B color) {
	for (int i = 0; i < 8; i++) {
		getSparks()[i]->setColor(color);
	}
}
void Zapper::addLightEmitters(const Dungeon &dungeon, std::vector<std::pair<int, int>> &lightEmitters) {
	lightEmitters.push_back(std::make_pair(getPosX(), getPosY()));

	// Add the lightning sparks, if they are visible
	for (unsigned int i = 0; i < sparks.size(); i++) {
		if (sparks.at(i)->isVisible()) {
			cocos2d::Vec2 pos = sparks.at(i)->getPosition();
			int x = (pos.x + X_OFFSET) / SPACING_FACTOR;
			int y = dungeon.getRows() - ((pos.y + Y_OFFSET) / SPACING_FACTOR);
			lightEmitters.push_back(std::make_pair(x, y));
		}
	}
}


//		SPINNER
Spinner::Spinner(Dungeon &dungeon, int x, int y, int rows) : Monster(x, y, 10 + randInt(3), 1, 3, 1, SPINNER) {
	setCanBeBurned(false);

	bool dir = randInt(2);

	m_clockwise = dir;
	m_angle = 1 + randInt(8);

	auto inner = dungeon.createSprite(x, y, 0, "Spinner_Buddy_48x48.png");
	auto outer = dungeon.createSprite(x, y, 0, "Spinner_Buddy_48x48.png");

	m_innerFire = std::make_shared<Objects>();
	m_innerFire->setSprite(inner);
	m_outerFire = std::make_shared<Objects>();
	m_outerFire->setSprite(outer);

	setInitialFirePosition(x, y, rows);

	setImageName("Spinner_48x48.png");
	setExtraSpritesFlag(true);
	setEmitsLight(true);
	setGold(2);
}

void Spinner::spriteCleanup(Dungeon &dungeon) {
	dungeon.queueRemoveSprite(getSprite());

	dungeon.queueRemoveSprite(m_innerFire->getSprite());
	dungeon.queueRemoveSprite(m_outerFire->getSprite());
}

void Spinner::moveTo(Dungeon &dungeon, int x, int y, float time) {
	int cols = dungeon.getCols();

	dungeon[getPosY()*cols + getPosX()].enemy = false;
	dungeon[y*cols + x].enemy = true;

	dungeon.queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);

	setInitialFirePosition(x, y, dungeon.getRows());
}
void Spinner::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	// reset projectiles to visible
	m_innerFire->getSprite()->setVisible(true);
	m_outerFire->getSprite()->setVisible(true);

	// 8 1 2 
	// 7 X 3
	// 6 5 4
	// moves spinner -to- position getAngle(), it is not where it currently is

	if (isClockwise()) {
		switch (getAngle()) {
		case 1:
			setFirePosition(dungeon, 'r');

			if (my == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 2:
			setFirePosition(dungeon, 'r');

			if (my == 1 || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 3:
			setFirePosition(dungeon, 'd');

			// if spinner is on the left or right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 4:
			setFirePosition(dungeon, 'd');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 5:
			setFirePosition(dungeon, 'l');

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 6:
			setFirePosition(dungeon, 'l');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (my == rows - 2 || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 7:
			setFirePosition(dungeon, 'u');

			// if spinner is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 8:
			setFirePosition(dungeon, 'u');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		default:
			break;
		}

		if (getAngle() == 8)
			setAngle(1);
		else
			setAngle(getAngle() + 1);
	}
	// else counterclockwise
	else {
		switch (getAngle()) {
		case 1:
			setFirePosition(dungeon, 'l');

			if (my == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 2:
			setFirePosition(dungeon, 'u');

			// if spinner is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 3:
			setFirePosition(dungeon, 'u');

			// if spinner is on the right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 4:
			setFirePosition(dungeon, 'r');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 5:
			setFirePosition(dungeon, 'r');

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 6:
			setFirePosition(dungeon, 'd');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == rows - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 7:
			setFirePosition(dungeon, 'd');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 8:
			setFirePosition(dungeon, 'l');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		default:
			break;
		}

		if (getAngle() == 1)
			setAngle(8);
		else
			setAngle(getAngle() - 1);
	}

	if (playerWasHit(*dungeon.getPlayer())) {
		attack(dungeon, *dungeon.getPlayerVector()[0]);
	}
}
void Spinner::attack(Dungeon &dungeon, Player &p) {
	playSound("Fire3.mp3");

	dungeon.damagePlayer(getStr());

	// if actor is still alive and can be burned, try to burn them
	if (p.getHP() > 0 && p.canBeBurned()) {

		// chance to burn
		int roll = randInt(100) + p.getLuck();

		// failed the save roll
		if (roll < 60) {
			p.addAffliction(std::make_shared<Burn>());
		}
	}
}

void Spinner::setInitialFirePosition(int x, int y, int rows) {

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


	// set sprite positions
	if (isClockwise()) {
		switch (getAngle()) {
		case 1:
			m_innerFire->getSprite()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 2:
			m_innerFire->getSprite()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 3:
			m_innerFire->getSprite()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 4:
			m_innerFire->getSprite()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			break;
		case 5:
			m_innerFire->getSprite()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 6:
			m_innerFire->getSprite()->setPosition((x) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 7:
			m_innerFire->getSprite()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 8:
			m_innerFire->getSprite()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			break;
		}
	}
	else {
		switch (getAngle()) {
		case 1:
			m_innerFire->getSprite()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 2:
			m_innerFire->getSprite()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			break;
		case 3:
			m_innerFire->getSprite()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 4:
			m_innerFire->getSprite()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 5:
			m_innerFire->getSprite()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 6:
			m_innerFire->getSprite()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 7:
			m_innerFire->getSprite()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 8:
			m_innerFire->getSprite()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			m_outerFire->getSprite()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		}
	}
	
	setSpriteVisibility(false);
}
void Spinner::setFirePosition(Dungeon &dungeon, char move) {
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
	
	dungeon.queueMoveSprite(m_innerFire->getSprite(), m_innerFire->getPosX(), m_innerFire->getPosY());
	dungeon.queueMoveSprite(m_outerFire->getSprite(), m_outerFire->getPosX(), m_outerFire->getPosY());
}

bool Spinner::isClockwise() const {
	return m_clockwise;
}
int Spinner::getAngle() const {
	return m_angle;
}
void Spinner::setAngle(int angle) {
	m_angle = angle;
}

bool Spinner::playerWasHit(const Actors &a) {
	if ((m_innerFire->getPosX() == a.getPosX() && m_innerFire->getPosY() == a.getPosY()) ||
		(m_outerFire->getPosX() == a.getPosX() && m_outerFire->getPosY() == a.getPosY()))
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
void Spinner::addLightEmitters(const Dungeon &dungeon, std::vector<std::pair<int, int>> &lightEmitters) {
	lightEmitters.push_back(std::make_pair(getPosX(), getPosY()));

	// Add the spinner sprites too
	/*lightEmitters.push_back(std::make_pair(m_innerFire->getPosX(), m_innerFire->getPosY()));
	lightEmitters.push_back(std::make_pair(m_outerFire->getPosX(), m_outerFire->getPosY()));*/
}


//		BOMBEE
Bombee::Bombee(int x, int y, int range) : Monster(x, y, 1, 0, 1, 1, BOMBEE), m_fuse(3), m_fused(false), m_aggroRange(range) {
	setChasesPlayer(true);
	setImageName("Bombee_48x48.png");
}
Bombee::Bombee(int x, int y, int range, std::string name, std::string image, int hp, int armor, int str, int dex) 
	: Monster(x, y, hp, armor, str, dex, name), m_fuse(3), m_fused(false), m_aggroRange(range) {
	setChasesPlayer(true);
	setImageName(image);
}

void Bombee::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	char first, best;
	first = best = '0';
	int shortest = 0;

	if (playerInRange(*dungeon.getPlayer(), m_aggroRange)) {
		// if viable path is found
		if (attemptChase(dungeon.getDungeon(), cols, shortest, m_aggroRange, m_aggroRange, getPosX(), getPosY(), first, best)) {
			moveWithSuccessfulChase(dungeon, best);
		}
	}
}
void Bombee::attack(Dungeon &dungeon, Player &p) {
	dungeon.damagePlayer(2);
}
void Bombee::deathDrops(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	std::shared_ptr<Traps> megabomb = std::make_shared<ActiveMegaBomb>(x, y);
	dungeon.getTraps().push_back(megabomb);
	megabomb->setSprite(dungeon.createSprite(x, y, -1, megabomb->getImageName()));
}

CharredBombee::CharredBombee(int x, int y, int range) : Bombee(x, y, range, CHARRED_BOMBEE, "Bombee_48x48.png", 2, 2, 2, 1) {
	setCanBeBurned(false);
	setLavaImmunity(true);
}

void CharredBombee::attack(Dungeon &dungeon, Player &p) {
	dungeon.damagePlayer(5);
}


//		MOUNTED KNIGHT
MountedKnight::MountedKnight(int x, int y) : Monster(x, y, 12 + randInt(5), 3, 2, 2, std::make_shared<Pike>(), MOUNTED_KNIGHT), m_alerted(false) {
	setImageName("Knight_Level1_48x48.png");
	setGold(5);
	setSturdy(false);
	setHasWeapon(true);
}

void MountedKnight::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	bool wall, enemy, hero;
	int n, m;

	// if player is invisible, just move them randomly
	if (dungeon.getPlayer()->isInvisible()) {
		step(dungeon);
		return;
	}

	//	if knight was alerted, check to attack, stop, or advance and return
	if (isAlerted()) {
		switch (getDirection()) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		hero = dungeon[(y + m)*cols + (x + n)].hero;

		if (hero) {
			attack(dungeon, *dungeon.getPlayerVector().at(0));
		}
		else if (wall || enemy) {
			toggleAlert();

			return;
		}
		else {
			moveTo(dungeon, x + n, y + m);
		}

		return;
	}

	//	if player is not in the knight's sights, move knight
	if (x != px && y != py) {
		step(dungeon);
		return;
	}

	// if player and knight are on the same column
	if (x == px) {
		//	if player is in line of sight, charge them
		if (!wallCollision(dungeon, 'y', py, y)) {
			setDirection(py - y > 0 ? 'd' : 'u');
			toggleAlert();
		}
		else {
			step(dungeon);
			return;
		}
	}
	// if player and knight are on the same row
	else if (y == py) {
		//	if player is in line of sight, charge them
		if (!wallCollision(dungeon, 'x', px, x)) {
			setDirection(px - x > 0 ? 'r' : 'l');
			toggleAlert();
		}
		else {
			step(dungeon);
			return;
		}
	}
}
void MountedKnight::step(Dungeon &dungeon) {
	//	50% chance to not move
	int check;
	if (check = randInt(100), check >= 50)
		return;

	// else move the knight in a random direction
	moveMonsterRandomly(dungeon);
}
void MountedKnight::deathDrops(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon[y*cols + x].item;

	// 10% drop chance
	if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 90) {
		if (item) {
			dungeon.itemHash(x, y);
		}
		dungeon[y*cols + x].object = std::make_shared<Pike>();
		dungeon[y*cols + x].item_name = IRON_LANCE;
		dungeon[y*cols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
	}

}

bool MountedKnight::isAlerted() const {
	return m_alerted;
}
void MountedKnight::toggleAlert() {
	m_alerted = !m_alerted;
}
char MountedKnight::getDirection() const {
	return m_direction;
}
void MountedKnight::setDirection(char dir) {
	m_direction = dir;
}


//		ROUNDABOUT
Roundabout::Roundabout(int x, int y) : Monster(x, y, 9 + randInt(2), 1, 5, 1, ROUNDABOUT) {
	int n = randInt(4);

	switch (n) {
	case 0: m_direction = 'l'; break;
	case 1: m_direction = 'r'; break;
	case 2: m_direction = 'u'; break;
	case 3: m_direction = 'd'; break;
	default: break;
	}

	setImageName("Roundabout_48x48.png");
	setGold(2);
}
Roundabout::Roundabout(int x, int y, std::string name, std::string image, int hp, int armor, int str, int dex) : Monster(x, y, hp, armor, str, dex, name) {
	int n = randInt(4);

	switch (n) {
	case 0: m_direction = 'l'; break;
	case 1: m_direction = 'r'; break;
	case 2: m_direction = 'u'; break;
	case 3: m_direction = 'd'; break;
	default: break;
	}

	setImageName(image);
}

void Roundabout::move(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();
	int x = getPosX();
	int y = getPosY();

	bool wall, enemy, trap, hero;
	bool lethal = false, isLava = false;
	int n, m;

	switch (getDirection()) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;
	trap = dungeon[(y + m)*cols + (x + n)].trap;
	hero = dungeon[(y + m)*cols + (x + n)].hero;


	// if there's a trap and it's lethal, mark it
	if (trap) {
		int pos = dungeon.findTrap(x + n, y + m);

		if (pos != -1) {
			lethal = dungeon.getTraps()[pos]->isLethal();
			isLava = dungeon.getTraps()[pos]->getName() == LAVA;
		}
	}

	// if trap is lava, but they are immune to lava (fire roundabout), move them
	if (isLava && lavaImmune()) {
		moveTo(dungeon, x + n, y + m);

		return;
	}

	//	if tile to left of the roundabout is not free, check above
	if (wall || enemy || hero || lethal) {

		// check direction to the "right" of the roundabout, relative to their movement
		switch (getDirection()) {
		case 'l': n = 0; m = -1; break;
		case 'r': n = 0; m = 1; break;
		case 'u': n = 1; m = 0; break;
		case 'd': n = -1; m = 0; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		hero = dungeon[(y + m)*cols + (x + n)].hero;

		//	if player is in the way, attack them
		if (hero) {
			attack(dungeon, *dungeon.getPlayerVector()[0]);
		}

		//	if tile above is a wall, then set direction to right
		if (wall || enemy) {
			switch (getDirection()) {
			case 'l': setDirection('r'); break;
			case 'r': setDirection('l'); break;
			case 'u': setDirection('d'); break;
			case 'd': setDirection('u'); break;
			}
		}
		//	otherwise direction is set up
		else {
			switch (getDirection()) {
			case 'l': setDirection('u'); break;
			case 'r': setDirection('d'); break;
			case 'u': setDirection('r'); break;
			case 'd': setDirection('l'); break;
			}
		}

	}
	else {
		moveTo(dungeon, x + n, y + m);
	}

}
void Roundabout::attack(Dungeon &dungeon, Player &p) {
	int px = p.getPosX();
	int py = p.getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.canBlock() && p.didBlock(getPosX(), getPosY())) {
		damage = std::max(0, p.blockedDamageReduction() - damage);
		p.successfulBlock();

		if (p.activeHasAbility()) {
			p.useActiveAbility(dungeon, *this);
		}

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	if (damage > 0) {
		dungeon.damagePlayer(damage);
	}
}

char Roundabout::getDirection() const {
	return m_direction;
}
void Roundabout::setDirection(char dir) {
	m_direction = dir;
}

//		FIRE ROUNDABOUT
FireRoundabout::FireRoundabout(int x, int y) : Roundabout(x, y, FIRE_ROUNDABOUT, "Fire_Roundabout_48x48.png", 9 + randInt(2), 1, 5, 1) {
	setLavaImmunity(true);
	setCanBeBurned(false);
	setGold(3);
}

void FireRoundabout::move(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();
	int x = getPosX();
	int y = getPosY();

	bool wall, enemy, trap, hero;
	bool lethal = false, isLava = false;
	int n, m;

	switch (getDirection()) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;
	trap = dungeon[(y + m)*cols + (x + n)].trap;
	hero = dungeon[(y + m)*cols + (x + n)].hero;

	
	// if there's a trap and it's lethal, mark it
	if (trap) {
		int pos = dungeon.findTrap(x + n, y + m);

		if (pos != -1) {
			lethal = dungeon.getTraps()[pos]->isLethal();
			isLava = dungeon.getTraps()[pos]->getName() == LAVA;
		}
	}

	// if trap is lava, but they are immune to lava (fire roundabout), move them
	if (isLava && lavaImmune()) {
		moveTo(dungeon, x + n, y + m);

		return;
	}

	//	if tile to left of the roundabout is not free, check above
	if (wall || enemy || hero || lethal) {

		// check direction to the "right" of the roundabout, relative to their movement
		switch (getDirection()) {
		case 'l': n = 0; m = -1; break;
		case 'r': n = 0; m = 1; break;
		case 'u': n = 1; m = 0; break;
		case 'd': n = -1; m = 0; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		hero = dungeon[(y + m)*cols + (x + n)].hero;

		//	if player is in the way, attack them
		if (hero) {
			attack(dungeon, *dungeon.getPlayerVector()[0]);
		}

		//	if tile above is a wall, then set direction to right
		if (wall || enemy) {
			switch (getDirection()) {
			case 'l': setDirection('r'); break;
			case 'r': setDirection('l'); break;
			case 'u': setDirection('d'); break;
			case 'd': setDirection('u'); break;
			}
		}
		//	otherwise direction is set up
		else {
			switch (getDirection()) {
			case 'l': setDirection('u'); break;
			case 'r': setDirection('d'); break;
			case 'u': setDirection('r'); break;
			case 'd': setDirection('l'); break;
			}
		}

	}
	else {
		moveTo(dungeon, x + n, y + m);
	}
	
	// 10% chance to spawn an ember where they're walking
	if (1 + randInt(100) > 90) {
		std::shared_ptr<Traps> ember = std::make_shared<Ember>(getPosX(), getPosY());
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Fire%04d.png", 8);
		ember->setSprite(dungeon.runAnimationForever(frames, 24, getPosX(), getPosY(), 2));
		ember->getSprite()->setScale(0.75f * GLOBAL_SPRITE_SCALE);
		
		dungeon[getPosY() * cols + getPosX()].trap = true;
		dungeon.getTraps().push_back(ember);
		dungeon.addLightSource(getPosX(), getPosY(), 3, ember->getName());
	}
}


//		SEEKER
Seeker::Seeker(int x, int y, int range) : Monster(x, y, 8 + randInt(3), 0, 5, 1, SEEKER), m_range(range), m_step(false) {
	setChasesPlayer(true);
	setSturdy(false);
	setImageName("Seeker_48x48.png");
	setGold(1);

	setHasAnimation(true);
	setAnimationFrames("Skeleton_Idle_%04d.png");
	setAnimationFrameCount(4);
	setFrameInterval(8);
}
Seeker::Seeker(int x, int y, int range, std::string name, std::string image) : Monster(x, y, 10 + randInt(5), 1, 4, 1, name), m_range(range), m_step(false) {
	setChasesPlayer(true);
	setSturdy(false);
	setSmart(true);
	setImageName(image);
}

void Seeker::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	char first, best;
	first = best = '0';
	int shortest = 0;

	// if seeker is on move cooldown, take them off cooldown
	if (!getStep()) {
		toggleStep();
		return;
	}

	if (playerInRange(*dungeon.getPlayer(), m_range)) {
		// if viable path is found
		if (attemptChase(dungeon.getDungeon(), cols, shortest, m_range, m_range, getPosX(), getPosY(), first, best)) {
			moveWithSuccessfulChase(dungeon, best);

			toggleStep();
		}
	}
}

int Seeker::getRange() const {
	return m_range;
}
bool Seeker::getStep() const {
	return m_step;
}
void Seeker::toggleStep() {
	m_step = !m_step;
}

//		DEAD SEEKER
DeadSeeker::DeadSeeker(int x, int y, int range) : Seeker(x, y, range, DEAD_SEEKER, "Dead_Seeker_48x48.png") {
	setGold(3);
}


//		ITEM THIEF
ItemThief::ItemThief(int x, int y, int range) : Monster(x, y, 5, 1, 2, 2, ITEM_THIEF), m_range(range) {
	setChasesPlayer(true);
	setSmart(true);
	setSturdy(false);
	setLavaImmunity(true);
	setImageName("Demon_48x48.png");
}

void ItemThief::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	// if they haven't stolen yet, continue chasing
	if (!m_stole) {
		char first, best;
		first = best = '0';
		int shortest = 0;

		// range is multiplied by 2 since they can move diagonally
		if (playerInDiagonalRange(*dungeon.getPlayer(), getRange())) {
			// if viable path is found
			if (attemptSmartChase(dungeon, cols, shortest, getRange(), getRange(), getPosX(), getPosY(), first, best)) {
				moveWithSuccessfulChase(dungeon, best);
			}
		}

		// play sound effect when they steal
		if (m_stole) {
			playSound("Item_Thief_Stole1.mp3");
			playSound("Female_Hey2.mp3");
		}
	}
	// otherwise run away with the player's item!
	else {
		run(dungeon);
	}
}
void ItemThief::run(Dungeon& dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	bool wall, enemy, trap, lethal = false;

	// try diagonals first
	int n, m;
	int fx = x, fy = y; // final x and y coordinates for the thief to use
	for (m = -1; m < 2; m++) {
		for (n = -1; n < 2; n++) {
			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			trap = dungeon[(y + m)*cols + (x + n)].trap;

			if (trap) {
				int pos = dungeon.findTrap(x + n, y + m);

				if (pos != -1) {
					lethal = dungeon.getTraps()[pos]->isLethal();
				}
			}

			// if space is free, does not contain a lethal trap, and distance is farthest, set new move
			if (!(wall || enemy) && !lethal && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
				fx = x + n;
				fy = y + m;
			}
		}
	}

	dungeon[y*cols + x].enemy = false;
	dungeon[fy*cols + fx].enemy = true;

	dungeon.queueMoveSprite(getSprite(), fx, fy);
	setPosX(fx); setPosY(fy);
}
void ItemThief::attack(Dungeon &dungeon, Player &p) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	// if the player has items, steal one randomly
	if (p.getItems().size() > 0) {
		int n = randInt(p.getItems().size());
		m_stolenItem = p.getItems().at(n);
		p.getItems().erase(p.getItems().begin() + n);
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

		if (damage > 0) {
			dungeon.damagePlayer(damage);
		}
		
	}
}
void ItemThief::death(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	playSound("Item_Thief_Death2.mp3");

	dungeon[y*cols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	if (m_stolenItem != nullptr) {

		if (dungeon[y*cols + x].item)
			dungeon.itemHash(x, y);
		
		dungeon[y*cols + x].object = m_stolenItem;
		dungeon[y*cols + x].item_name = m_stolenItem->getName();
		dungeon[y*cols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
	}

	else if (m_stolenGold > 0) {
		playSound("Coin_Drop5.mp3");
		dungeon[y*cols + x].gold += m_stolenGold;
		dungeon.addGoldSprite(x, y);
	}
}

int ItemThief::getRange() const {
	return m_range;
}


//		GOO SACK
GooSack::GooSack(int x, int y) : Monster(x, y, 7 + randInt(5), 0, 3, 1, GOO_SACK) {
	setImageName("Goo_Sack_48x48.png");
	setGold(2);

	setHasAnimation(true);
	setAnimationFrames("Goo_Sack_%04d.png");
	setAnimationFrameCount(4);
	setFrameInterval(4);
}

void GooSack::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// if not primed but player is nearby, then get primed
	if (!m_primed && playerIsAdjacent(*dungeon.getPlayer(), true)) {
		// sound effect for priming
		playSound("Goo_Sack_Primed.mp3");

		m_primed = true;
		m_x = px;
		m_y = py;

		return;
	}
	else if (!m_primed && !playerIsAdjacent(*dungeon.getPlayer(), true)) {
		return;
	}

	// otherwise if player is near the goo sack and it is primed, attack them
	if (m_primed && playerIsAdjacent(*dungeon.getPlayer(), true)) {
		attack(dungeon, *dungeon.getPlayerVector()[0]);
	}
	else if (abs(px - x) <= 2 && abs(py - y) <= 2 && !(dungeon[m_y*cols + m_x].wall || dungeon[m_y*cols + m_x].enemy || dungeon[m_y*cols + m_x].hero)) {
		moveTo(dungeon, m_x, m_y);
	}

	m_primed = false;
}
void GooSack::attack(Dungeon &dungeon, Player &p) {
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.canBlock() && p.didBlock(getPosX(), getPosY())) {
		damage = std::max(0, p.blockedDamageReduction() - damage);
		p.successfulBlock(); // calls shield durability change

		if (p.activeHasAbility()) {
			p.useActiveAbility(dungeon, *this);
		}

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	if (damage > 0) {
		dungeon.damagePlayer(damage);
	}

}


//		BROUNDABOUT
Broundabout::Broundabout(int x, int y) : Roundabout(x, y, BROUNDABOUT, "Brown_Slime_48x48.png", 6 + randInt(6), 0, 3, 1) {
	int n = randInt(4);

	switch (n) {
	case 0: setDirection('l'); break;
	case 1: setDirection('r'); break;
	case 2: setDirection('u'); break;
	case 3: setDirection('d'); break;
	default: break;
	}

	setGold(3);

	setHasAnimation(true);
	setAnimationFrames("Broundabout_%04d.png");
	setAnimationFrameCount(4);
	setFrameInterval(8);
}

void Broundabout::move(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();
	int x = getPosX();
	int y = getPosY();

	bool wall, enemy, hero;
	int n, m;
	
	switch (getDirection()) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;
	hero = dungeon[(y + m)*cols + (x + n)].hero;

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
	if (m_primed && (abs(px - x) == 0 && abs(py - y) <= 1 || abs(px - x) <= 1 && abs(py - y) == 0)) {
		attack(dungeon, *dungeon.getPlayerVector()[0]);
	}
	// else if primed, but player moved away, let the broundabout jump towards where the player was
	else if (m_primed && (px != m_x || py != m_y) && abs(px - x) <= 2 && abs(py - y) <= 2) {
		moveTo(dungeon, m_x, m_y);
	}
	// if nothing is in the way, keep moving
	else if (!(wall || enemy || hero)) {
		moveTo(dungeon, x + n, y + m);
	}
	// if there's a non-player object in the way, reverse movement
	else if ((wall || enemy) && !hero) {
		switch (getDirection()) {
		case 'l': setDirection('r'); break;
		case 'r': setDirection('l'); break;
		case 'u': setDirection('d'); break;
		case 'd': setDirection('u'); break;
		}
	}
	
	m_primed = false;
}
void Broundabout::attack(Dungeon &dungeon, Player &p) {
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.canBlock() && p.didBlock(getPosX(), getPosY())) {

		if (p.activeHasAbility()) {
			p.useActiveAbility(dungeon, *this);
		}

		damage = std::max(0, damage - p.blockedDamageReduction());
		p.successfulBlock();

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	if (damage > 0) {
		dungeon.damagePlayer(damage);
	}
}


//		CHARGER
Charger::Charger(int x, int y) : Monster(x, y, 5 + randInt(5), 1, 3, 1, CHARGER) {
	setChasesPlayer(true);
	setImageName("Demonic_Goblin_48x48.png");
	setGold(5);
}

void Charger::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	int steps = 5;

	bool wall, enemy, trap, hero;

	if (m_primed) {
		int n, m;
		switch (m_direction) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		trap = dungeon[(y + m)*cols + (x + n)].trap;
		hero = dungeon[(y + m)*cols + (x + n)].hero;

		while (!(wall || enemy || hero) && steps > 0) {

			setPosX(x + n);
			setPosY(y + m);

			switch (m_direction) {
			case 'l': n--; break;
			case 'r': n++; break;
			case 'u': m--; break;
			case 'd': m++; break;
			}		

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			hero = dungeon[(y + m)*cols + (x + n)].hero;
			trap = dungeon[(y + m)*cols + (x + n)].trap;

			steps--;
		}

		if (wall) {
			// destroy this wall if it's a breakable wall
			if (dungeon[(y + m)*cols + (x + n)].wall_type == REG_WALL) {
				dungeon[(y + m)*cols + (x + n)].wall = false;

				// call remove sprite
				dungeon.removeSprite(dungeon.wall_sprites, x + n, y + m);
			}

			addAffliction(std::make_shared<Stun>(2));
		}

		else if (hero) {
			attack(dungeon, *dungeon.getPlayerVector().at(0));
		}

		dungeon[y*cols + x].enemy = false;
		dungeon[getPosY()*cols + getPosX()].enemy = true;
		dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY());

		if (trap) {
			// Check if monster is on top of a trap
			int pos = dungeon.findMonster(getPosX(), getPosY());
			if (pos == -1)
				return;
			dungeon.singleMonsterTrapEncounter(pos);
		}

		m_primed = false;
		m_turns = 2;

		return;
	}

	// If charger has LOS with the player, then get primed
	if ((px == x && abs(py - y) <= m_losRange || py == y && abs(px - x) <= m_losRange) && clearLineOfSight(dungeon, *dungeon.getPlayerVector()[0])) {
		// sound effect for priming
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
	else if (playerInRange(*dungeon.getPlayer(), m_range)) {
		char first, best;
		first = best = '0';
		int shortest = 0;

		// if viable path is found
		if (attemptChase(dungeon.getDungeon(), cols, shortest, m_range, m_range, x, y, first, best)) {
			moveWithSuccessfulChase(dungeon, best);
		}

		m_turns = 2;
	}
}
void Charger::attack(Dungeon &dungeon, Player &p) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int steps = 3;

	bool wall, enemy, trap;

	int n, m;
	switch (m_direction) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	wall = dungeon[(py + m)*cols + (px + n)].wall;
	enemy = dungeon[(py + m)*cols + (px + n)].enemy;
	trap = dungeon[(py + m)*cols + (px + n)].trap;

	while (!(wall || enemy) && steps > 0 && !dungeon.getPlayer()->isHeavy()) {

		dungeon.getPlayerVector()[0]->setPosX(px + n);
		dungeon.getPlayerVector()[0]->setPosY(py + m);

		switch (m_direction) {
		case 'l': n--; break;
		case 'r': n++; break;
		case 'u': m--; break;
		case 'd': m++; break;
		}

		wall = dungeon[(py + m)*cols + (px + n)].wall;
		enemy = dungeon[(py + m)*cols + (px + n)].enemy;
		trap = dungeon[(py + m)*cols + (px + n)].trap;

		steps--;
	}

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// If player was knocked into a wall, they take additional damage
	if (wall && steps > 0 && !dungeon.getPlayer()->isHeavy())
		damage += 5;

	dungeon.damagePlayer(damage);

	dungeon[py*cols + px].hero = false;
	dungeon[dungeon.getPlayer()->getPosY()*cols + dungeon.getPlayer()->getPosX()].hero = true;
	dungeon.queueMoveSprite(dungeon.getPlayer()->getSprite(), dungeon.getPlayer()->getPosX(), dungeon.getPlayer()->getPosY());

	if (dungeon[getPosY()*cols + getPosX()].trap) {
		dungeon.trapEncounter(dungeon.getPlayer()->getPosX(), dungeon.getPlayer()->getPosY());
	}
}


//		RAT
Rat::Rat(int x, int y, int range) : Monster(x, y, 6 + randInt(3), 0, 2, 1, RAT), m_range(range) {
	setChasesPlayer(true);
	setImageName("Small_Dry_Demon.png");
	setGold(2);
}

void Rat::move(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	if (playerInRange(*dungeon.getPlayer(), m_range)) {
		// If the player didn't move since their last turn, chase the player
		if (px == m_prevPlayerX && py == m_prevPlayerY) {
			attemptGreedyChase(dungeon);
		}
		// Otherwise the rat runs away
		else {
			run(dungeon);
		}
	}

	m_prevPlayerX = px;
	m_prevPlayerY = py;
}
void Rat::run(Dungeon& dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	bool wall, enemy;

	int n, m;
	int fx = x, fy = y; // final x and y coordinates for the rat
	for (n = -1, m = 0; n < 2; n += 2) {	
		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;

		// if space is free and distance is farthest, set new move
		if (!(wall || enemy) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}	
	}

	for (n = 0, m = -1; m < 2; m += 2) {
		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;

		// if space is free and distance is farthest, set new move
		if (!(wall || enemy) && abs(px - (x + n)) + abs(py - (y + m)) > abs(px - fx) + abs(py - fy)) {
			fx = x + n;
			fy = y + m;
		}
	}

	moveTo(dungeon, fx, fy);
}


//		TOAD
Toad::Toad(int x, int y) : Monster(x, y, 7 + randInt(4), 1, 3, 2, TOAD) {
	setImageName("Green_Slime_48x48.png");
	setGold(3);
	setCanBePoisoned(false);
}

void Toad::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	switch (m_moveType) {
	case 1:
		// If player is adjacent, attack them
		if (dungeon[(y - 1)*cols + x].hero || dungeon[(y + 1)*cols + x].hero || dungeon[y*cols + x - 1].hero || dungeon[y*cols + x + 1].hero) {
			attack(dungeon, *dungeon.getPlayerVector().at(0));
		}
		m_facing = moveMonsterRandomly(dungeon);
		break;
	case 2:
	case 3:
	case 4:
		break;
	case 5:
	case 6:
		// If player is adjacent, attack them
		if (dungeon[(y - 1)*cols + x].hero || dungeon[(y + 1)*cols + x].hero || dungeon[y*cols + x - 1].hero || dungeon[y*cols + x + 1].hero) {
			attack(dungeon, *dungeon.getPlayerVector().at(0));
		}
		m_facing = moveMonsterRandomly(dungeon);
		break;
	case 7:
		break;
	case 8: {
		
		if (m_facing != '-') {

			int n, m;
			switch (m_facing) {
			case 'l': n = -1; m = 0; break;
			case 'r': n = 1; m = 0; break;
			case 'u': n = 0; m = -1; break;
			case 'd': n = 0; m = 1; break;
			}

			// If the space is free, try to spit a poison puddle
			if (!(dungeon[(y + m)*cols + (x + n)].wall || dungeon[(y + m)*cols + (x + n)].trap)) {

				// 50% chance to spit out a poison puddle
				if (1 + randInt(100) > 50) {
					std::shared_ptr<Traps> puddle = std::make_shared<PoisonPuddle>(dungeon, x + n, y + m, 8);
					//puddle->setSprite(dungeon.createSprite(x + n, y + m, 0, puddle->getImageName()));
					dungeon[(y + m)*cols + (x + n)].trap = true;
					dungeon.getTraps().push_back(puddle);
				}
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


//		SPIDER
Spider::Spider(Dungeon &dungeon, int x, int y, int webCount) : Monster(x, y, 4 + randInt(4), 1, 4, 1, SPIDER), m_webCount(webCount) {
	setImageName("SpiderIdle0001.png");
	setGold(4);

	setHasAnimation(true);
	setAnimationFrames("SpiderIdle%04d.png");
	setAnimationFrameCount(5);
	setFrameInterval(8);



	createWebs(dungeon, x, y);
}
Spider::Spider(Dungeon &dungeon, int x, int y, int webCount, std::string name, std::string image, int hp, int armor, int str, int dex) 
	: Monster(x, y, hp, armor, str, dex, name), m_webCount(webCount) {
	setImageName(image);

	setHasAnimation(true);
	setAnimationFrames("SpiderIdle%04d.png");
	setAnimationFrameCount(5);
	setFrameInterval(8);

	createWebs(dungeon, x, y);
}

void Spider::move(Dungeon &dungeon) {
	
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// If the player was captured, chase them down
	if (m_captured) {
		attemptGreedyChase(dungeon, true);
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
			if (playerIsAdjacent(*dungeon.getPlayer(), true)) {
				playSound("Spider_Primed.mp3");
				m_primed = true;
			}
		}
		else if (m_primed && !m_wait) {
			// If player is still adjacent, attack them
			if (playerIsAdjacent(*dungeon.getPlayer(), true))
				attack(dungeon, *dungeon.getPlayerVector()[0]);

			m_primed = false;
			m_wait = true;
		}
		else if (m_wait)
			m_wait = false;
	}

	// Special actions priority over creating new webs
	if (specialMove(dungeon))
		return;

	// Create new webs if needed
	if ((int)m_webs.size() < m_webCount) {
		if (!dungeon[y*cols + x].trap) {
			m_webs.push_back(std::make_pair(x, y));

			dungeon[y*cols + x].trap = true;
			std::shared_ptr<Web> web = std::make_shared<Web>(dungeon, x, y);			
			dungeon.getTraps().push_back(web);
		}
		else {
			moveMonsterRandomly(dungeon);
		}
	}
}
void Spider::attack(Dungeon &dungeon, Player &p) {
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
			p.useActiveAbility(dungeon, *this);

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my) <= 1) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	if (damage > 0)
		dungeon.damagePlayer(damage);
	

	m_captured = false;
	m_primed = false;
}
void Spider::createWebs(Dungeon &dungeon, int x, int y) {
	int cols = dungeon.getCols();
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
		if (!(dungeon[m*cols + n].wall || dungeon[m*cols + n].trap)) {
			m_webs.push_back(std::make_pair(n, m));

			dungeon[m*cols + n].trap = true;
			std::shared_ptr<Web> web = std::make_shared<Web>(dungeon, n, m);
			dungeon.getTraps().push_back(web);
			dungeon[m*cols + n].trap_name = web->getName();

			coords.erase(coords.begin() + index);
		}
	}
}

ShootingSpider::ShootingSpider(Dungeon &dungeon, int x, int y, int webCount, int range) 
	: Spider(dungeon, x, y, webCount, SHOOTING_SPIDER, "SpiderIdle0001.png", 7 + randInt(7), 1, 6, 1), m_range(range) {
	m_turns = 2;
	m_maxTurns = m_turns;
	setGold(12);
}

bool ShootingSpider::specialMove(Dungeon &dungeon) {

	// If spider is primed, then they will attempt to web the player from a distance
	if (m_primed) {

		if (clearLineOfSight(dungeon, *dungeon.getPlayer())) {
			// web player
			playSound("Grass2.mp3");
			
			dungeon.getPlayerVector()[0]->addAffliction(std::make_shared<Stun>(3));
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
	if (playerInRange(*dungeon.getPlayer(), m_range) && clearLineOfSight(dungeon, *dungeon.getPlayer())) {
		playSound("Spider_Primed.mp3");

		m_primed = true;
		return true;
	}

	return false;
}

PouncingSpider::PouncingSpider(int x, int y) : Monster(x, y, 6 + randInt(6), 2, 7, 2, POUNCING_SPIDER), m_x(x), m_y(y) {
	setHasAnimation(true);
	setAnimationFrames("RedSpiderIdle%04d.png");
	setAnimationFrameCount(5);
	setFrameInterval(8);

	setImageName("RedSpiderIdle0001.png");
	setGold(10);
}

void PouncingSpider::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// If primed, check if player is still within their region and attack them if so
	if (m_primed) {
		if (boundaryCheck(dungeon)) {
			attack(dungeon, *dungeon.getPlayer());

			// Jump on top of player
			moveTo(dungeon, px, py, 0.05f);
		}

		m_turns = 0;
		m_primed = false;
		return;
	}

	// If not off cooldown yet, keep cooling down
	if (m_turns < m_maxTurns) {

		// If the spider just pounced the player, try jumping back to their original spot
		if (m_turns == 0) {
			if (!(dungeon[m_y*cols + m_x].enemy || dungeon[m_y*cols + m_x].wall || dungeon[m_y*cols + m_x].hero)) {
				moveTo(dungeon, m_x, m_y, 0.05f);
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
	if (boundaryCheck(dungeon)) {
		playSound("Spider_Primed.mp3");

		m_primed = true;
		return;
	}
}
bool PouncingSpider::boundaryCheck(const Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// Check if player is in their territory
	for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 2; j < x + 3; j++) {

			// Boundary and corner check
			if (dungeon.withinBounds(j, i) && !(j == x && i == y) &&
				!(i == y - 2 && j == x - 2) && !(i == y - 2 && j == x + 2) && !(i == y + 2 && j == x - 2) && !(i == y + 2 && j == x + 2)) {

				if (px == j && py == i)
					return true;			
			}
		}
	}

	return false;
}


//		GHOST
Ghost::Ghost(int x, int y, int range) : Monster(x, y, 1, 0, 1, 1, GHOST), m_range(range) {
	setChasesPlayer(true);
	setEthereal(true);
	setFlying(true);
	setImageName("Crying_Ghost_48x48.png");
	setGold(10);
}

void Ghost::move(Dungeon &dungeon) {
	if (playerInRange(*dungeon.getPlayer(), m_range))
		attemptGreedyChase(dungeon);

}
void Ghost::attack(Dungeon &dungeon, Player &p) {
	// sound effect
	playSound("Possessed.mp3");

	// Possess the player
	p.addAffliction(std::make_shared<Possessed>(10));

	setDestroyed(true); // Destroy the ghost so that no rewards are dropped
}


//		PUFF
Puff::Puff(int x, int y, int turns, int pushDist) : Monster(x, y, 5 + randInt(5), 1, 1, 1, PUFF), m_turns(turns), m_maxTurns(turns), m_pushDist(pushDist) {
	setFlying(true);
	setImageName("Small_White_Ghost.png");
	setGold(4);
}
Puff::Puff(int x, int y, int turns, int pushDist, std::string name, std::string image, int hp, int armor, int str, int dex)
	: Monster(x, y, hp, armor, str, dex, name), m_turns(turns), m_maxTurns(turns), m_pushDist(pushDist) {
	setFlying(true);
	setImageName(image);
}

void Puff::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

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
			if (dungeon[(y - k)*cols + x].wall) {
				u_wall = true;
			}
			else if (dungeon[(y - k)*cols + x].hero || dungeon[(y - k)*cols + x].enemy) {
				move = 'u';
				ax = x;
				ay = y - k;
				dungeon.linearActorPush(ax, ay, m_pushDist - k + 1, move);
			}	
		}
		if (!d_wall && y + k < rows) {
			if (dungeon[(y + k)*cols + x].wall) {
				d_wall = true;
			}
			else if (dungeon[(y + k)*cols + x].hero || dungeon[(y + k)*cols + x].enemy) {
				move = 'd';
				ax = x;
				ay = y + k;
				dungeon.linearActorPush(ax, ay, m_pushDist - k + 1, move);
			}
		}
		if (!l_wall && x - k > 0) {
			if (dungeon[y*cols + (x - k)].wall) {
				l_wall = true;
			}
			else if (dungeon[y*cols + (x - k)].hero || dungeon[y*cols + (x - k)].enemy) {
				move = 'l';
				ax = x - k;
				ay = y;
				dungeon.linearActorPush(ax, ay, m_pushDist - k + 1, move);
			}
			
		}
		if (!r_wall && x + k < cols) {
			if (dungeon[y*cols + (x + k)].wall) {
				r_wall = true;
			}
			else if (dungeon[y*cols + (x + k)].hero || dungeon[y*cols + (x + k)].enemy) {
				move = 'r';
				ax = x + k;
				ay = y;
				dungeon.linearActorPush(ax, ay, m_pushDist - k + 1, move);
			}
			
		}

		if (pushesAll()) {
			if (!tl_wall && x - k > 0 && y - k > 0) {
				if (dungeon[(y - k)*cols + (x - k)].wall) {
					tl_wall = true;
				}
				else if (dungeon[(y - k)*cols + (x - k)].hero || dungeon[(y - k)*cols + (x - k)].enemy) {
					move = '1';
					ax = x - k;
					ay = y - k;
					dungeon.linearActorPush(ax, ay, m_pushDist - k + 1, move);
				}			
			}
			if (!tr_wall && x + k > 0 && y - k > 0) {
				if (dungeon[(y - k)*cols + (x + k)].wall) {
					tr_wall = true;
				}
				else if (dungeon[(y - k)*cols + (x + k)].hero || dungeon[(y - k)*cols + (x + k)].enemy) {
					move = '2';
					ax = x + k;
					ay = y - k;
					dungeon.linearActorPush(ax, ay, m_pushDist - k + 1, move);
				}			
			}
			if (!bl_wall && x - k > 0 && y + k > 0) {
				if (dungeon[(y + k)*cols + (x - k)].wall) {
					bl_wall = true;
				}
				else if (dungeon[(y + k)*cols + (x - k)].hero || dungeon[(y + k)*cols + (x - k)].enemy) {
					move = '3';
					ax = x - k;
					ay = y + k;
					dungeon.linearActorPush(ax, ay, m_pushDist - k + 1, move);
				}				
			}
			if (!br_wall && x + k > 0 && y + k > 0) {
				if (dungeon[(y + k)*cols + (x + k)].wall) {
					br_wall = true;
				}
				else if (dungeon[(y + k)*cols + (x + k)].hero || dungeon[(y + k)*cols + (x + k)].enemy) {
					move = '4';
					ax = x + k;
					ay = y + k;
					dungeon.linearActorPush(ax, ay, m_pushDist - k + 1, move);
				}			
			}
		}

		k++;
	}

	m_turns = m_maxTurns;
}

GustyPuff::GustyPuff(int x, int y) : Puff(x, y, 3, 1, GUSTY_PUFF, "Small_White_Ghost.png", 8 + randInt(6), 1, 1, 1) {
	setGold(6);
}

StrongGustyPuff::StrongGustyPuff(int x, int y) : Puff(x, y, 4, 2, STRONG_GUSTY_PUFF, "Tribal_Orc_48x48.png", 12 + randInt(6), 1, 1, 2) {
	setGold(12);
}


//		INVERTED PUFF
InvertedPuff::InvertedPuff(int x, int y, int turns, int pullDist) : Monster(x, y, 4 + randInt(6), 1, 1, 1, INVERTED_PUFF), m_turns(turns), m_maxTurns(turns), m_pullDist(pullDist) {
	setFlying(true);
	setImageName("Small_White_Ghost.png");
	setGold(4);
}
InvertedPuff::InvertedPuff(int x, int y, int turns, int pullDist, std::string name, std::string image, int hp, int armor, int str, int dex)
	: Monster(x, y, hp, armor, str, dex, name), m_turns(turns), m_maxTurns(turns), m_pullDist(pullDist) {
	setFlying(true);
	setImageName(image);
}

void InvertedPuff::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

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
			if (dungeon[(y - k)*cols + x].wall) {
				u_wall = true;
			}
			else if (dungeon[(y - k)*cols + x].hero || dungeon[(y - k)*cols + x].enemy) {
				move = 'd';
				ax = x;
				ay = y - k;
				dungeon.linearActorPush(ax, ay, m_pullDist - 1, move, true);
			}
		}
		if (!d_wall && y + k < rows) {
			if (dungeon[(y + k)*cols + x].wall) {
				d_wall = true;
			}
			else if (dungeon[(y + k)*cols + x].hero || dungeon[(y + k)*cols + x].enemy) {
				move = 'u';
				ax = x;
				ay = y + k;
				dungeon.linearActorPush(ax, ay, m_pullDist - 1, move, true);
			}
		}
		if (!l_wall && x - k > 0) {
			if (dungeon[y*cols + (x - k)].wall) {
				l_wall = true;
			}
			else if (dungeon[y*cols + (x - k)].hero || dungeon[y*cols + (x - k)].enemy) {
				move = 'r';
				ax = x - k;
				ay = y;
				dungeon.linearActorPush(ax, ay, m_pullDist - 1, move, true);
			}

		}
		if (!r_wall && x + k < cols) {
			if (dungeon[y*cols + (x + k)].wall) {
				r_wall = true;
			}
			else if (dungeon[y*cols + (x + k)].hero || dungeon[y*cols + (x + k)].enemy) {
				move = 'l';
				ax = x + k;
				ay = y;
				dungeon.linearActorPush(ax, ay, m_pullDist - 1, move, true);
			}

		}

		if (pullsAll()) {
			if (!tl_wall && x - k > 0 && y - k > 0) {
				if (dungeon[(y - k)*cols + (x - k)].wall) {
					tl_wall = true;
				}
				else if (dungeon[(y - k)*cols + (x - k)].hero || dungeon[(y - k)*cols + (x - k)].enemy) {
					move = '4';
					ax = x - k;
					ay = y - k;
					dungeon.linearActorPush(ax, ay, m_pullDist - 1, move, true);
				}
			}
			if (!tr_wall && x + k > 0 && y - k > 0) {
				if (dungeon[(y - k)*cols + (x + k)].wall) {
					tr_wall = true;
				}
				else if (dungeon[(y - k)*cols + (x + k)].hero || dungeon[(y - k)*cols + (x + k)].enemy) {
					move = '3';
					ax = x + k;
					ay = y - k;
					dungeon.linearActorPush(ax, ay, m_pullDist - 1, move, true);
				}
			}
			if (!bl_wall && x - k > 0 && y + k > 0) {
				if (dungeon[(y + k)*cols + (x - k)].wall) {
					bl_wall = true;
				}
				else if (dungeon[(y + k)*cols + (x - k)].hero || dungeon[(y + k)*cols + (x - k)].enemy) {
					move = '2';
					ax = x - k;
					ay = y + k;
					dungeon.linearActorPush(ax, ay, m_pullDist - 1, move, true);
				}
			}
			if (!br_wall && x + k > 0 && y + k > 0) {
				if (dungeon[(y + k)*cols + (x + k)].wall) {
					br_wall = true;
				}
				else if (dungeon[(y + k)*cols + (x + k)].hero || dungeon[(y + k)*cols + (x + k)].enemy) {
					move = '1';
					ax = x + k;
					ay = y + k;
					dungeon.linearActorPush(ax, ay, m_pullDist - 1, move, true);
				}
			}
		}

		k--;
	}

	// Perform any special moves unique to this puff, if any
	specialMove(dungeon);

	m_turns = m_maxTurns;
}

SpikedInvertedPuff::SpikedInvertedPuff(int x, int y) : InvertedPuff(x, y, 4, 2, SPIKED_INVERTED_PUFF, "White_Ghost.png", 8 + randInt(6), 2, 5, 1) {
	setGold(10);
}
SpikedInvertedPuff::SpikedInvertedPuff(int x, int y, int turns, int pullDist, std::string name, std::string image, int hp, int armor, int str, int dex)
	: InvertedPuff(x, y, 4, 2, name, image, hp, armor, str, dex) {

}

void SpikedInvertedPuff::attack(Dungeon &dungeon, Player &p) {
	int damage = std::max(1, randInt(getStr()));
	dungeon.damagePlayer(damage);
}
void SpikedInvertedPuff::specialMove(Dungeon &dungeon) {
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// If the player was sucked into the puff, damage them
	if (px == x && abs(py - y) == 1 || py == y && abs(px - x) == 1) {
		attack(dungeon, *dungeon.getPlayerVector()[0]);
	}
}

GustySpikedInvertedPuff::GustySpikedInvertedPuff(int x, int y) : SpikedInvertedPuff(x, y, 4, 2, GUSTY_SPIKED_INVERTED_PUFF, "White_Ghost.png", 8 + randInt(6), 2, 7, 1) {
	setGold(12);
}

void GustySpikedInvertedPuff::specialMove(Dungeon &dungeon) {
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	// If the player was sucked into the puff, damage them
	if (abs(px - x) <= 1 && abs(py - y) <= 1) {
		attack(dungeon, *dungeon.getPlayerVector()[0]);
	}
}


//		SERPENT
Serpent::Serpent(Dungeon &dungeon, int &x, int &y, int turns) : Monster(x, y, 8 + randInt(3), 1, 6, 2, SERPENT), m_turns(turns), m_maxTurns(turns) {
	setImageName("Red_Fish_48x48.png");
	setGold(7);
	setChasesPlayer(true);
	setMultiSegmented(true);

	// Set the tail position, if no space is valid then
	// the Serpent's x and y gets re-rolled and we try again
	while (!setTailPosition(dungeon)) {
		rerollMonsterPosition(dungeon, x, y);
		setPosX(x);
		setPosY(y);
	}

	addSegments(dungeon);
}
Serpent::Serpent(Dungeon &dungeon, int &x, int &y, int turns, std::string name, std::string image, int hp, int armor, int str, int dex) 
	: Monster(x, y, hp, armor, str, dex, name), m_turns(turns), m_maxTurns(turns) {
	setImageName(image);
	setChasesPlayer(true);
	setMultiSegmented(true);

	// Set the tail position, if no space is valid then
	// the Serpent's x and y gets re-rolled and we try again
	while (!setTailPosition(dungeon)) {
		rerollMonsterPosition(dungeon, x, y);
		setPosX(x);
		setPosY(y);
	}
}

bool Serpent::setTailPosition(Dungeon &dungeon) {
	int cols = dungeon.getCols();

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
		if (!(dungeon[m*cols + n].wall || dungeon[m*cols + n].enemy || dungeon[m*cols + n].hero)) {
			m_tailX = n;
			m_tailY = m;
			return true;
		}

		coords.erase(coords.begin() + index);
	}

	return false;
}
void Serpent::rerollMonsterPosition(Dungeon &dungeon, int &x, int &y) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	bool wall, enemy, hero, noSpawn;

	x = 1 + randInt(cols - 2);
	y = 1 + randInt(rows - 2);

	noSpawn = dungeon[y*cols + x].noSpawn;
	wall = dungeon[y*cols + x].wall;
	enemy = dungeon[y*cols + x].enemy;
	hero = dungeon[y*cols + x].hero;

	while (noSpawn || wall || enemy || hero) { // while monster position clashes with wall, player, or idol, reroll its position
		x = 1 + randInt(cols - 2);
		y = 1 + randInt(rows - 2);

		noSpawn = dungeon[y*cols + x].noSpawn;
		wall = dungeon[y*cols + x].wall;
		enemy = dungeon[y*cols + x].enemy;
		hero = dungeon[y*cols + x].hero;
	}
}

void Serpent::moveTo(Dungeon &dungeon, int x, int y, float time) {
	int cols = dungeon.getCols();

	dungeon[getPosY()*cols + getPosX()].enemy = false;
	dungeon[y*cols + x].enemy = true;

	dungeon.queueMoveSprite(getSprite(), x, y, time);

	moveSegments(dungeon, x, y);

	setPosX(x); setPosY(y);
}
void Serpent::move(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();

	if (m_turns > 0) {
		m_turns--;
		return;
	}

	if (playerInRange(*dungeon.getPlayer(), m_range)) {
		char first, best;
		first = best = '0';
		int shortest = 0;
		bool flipped = false; // Flag for indicating if the serpent did a head/tail swap move

		if (attemptChase(dungeon.getDungeon(), cols, shortest, m_range, m_range, x, y, first, best)) {
			if (!moveWithSuccessfulChase(dungeon, best)) {
				int n, m;
				switch (best) {
				case 'l': n = -1; m = 0; break;
				case 'r': n = 1; m = 0; break;
				case 'u': n = 0; m = -1; break;
				case 'd': n = 0; m = 1; break;
				}

				// Check if this move went on top of its tail, if so then the serpent can perform this action
				if (x + n == m_tail->getPosX() && y + m == m_tail->getPosY()) {
					setPosX(x + n); setPosY(y + m);
					dungeon.queueMoveSprite(getSprite(), x + n, y + m);
					flipped = true;

						// No need to update the enemy flags since we're simply switching the head and tail
				}
			}
		}
		else {
			attemptGreedyChase(dungeon);
		}

		// Check if serpent moved, if so, update tail coordinates to where the head was
		if (x != getPosX() || y != getPosY()) {

			// If the serpent didn't flip its head and tail, then update location flags
			if (!flipped) {
				dungeon[m_tail->getPosY()*cols + m_tail->getPosX()].enemy = false;
				dungeon[y*cols + x].enemy = true;
			}

			dungeon.queueMoveSprite(m_tail->getSprite(), x, y);
			m_tail->setPosX(x);
			m_tail->setPosY(y);
			m_tail->setParentX(getPosX());
			m_tail->setParentY(getPosY());
		}
		
	}

	m_turns = m_maxTurns;
}
void Serpent::extraAttackEffects(Dungeon& dungeon) {
	int roll = 1 + randInt(100) + dungeon.getPlayer()->getLuck();

	if (roll < 50)
		dungeon.getPlayerVector()[0]->addAffliction(std::make_shared<Bleed>());
	
}

void Serpent::moveSegments(Dungeon &dungeon, int x, int y) {

	// x and y is where the parent (head) is moving to

	// If the parent's previous position is not its current position, then move the tail
	if (x != getPosX() || y != getPosY()) {
		int cols = dungeon.getCols();

		// If the parent's new position did not switch with its tail, then update enemy flag
		if (x != m_tail->getPosX() || y != m_tail->getPosY())
			dungeon[m_tail->getPosY()*cols + m_tail->getPosX()].enemy = false;

		dungeon[y*cols + x].enemy = true;

		dungeon.queueMoveSprite(m_tail->getSprite(), getPosX(), getPosY(), 0.05f);
		m_tail->setPosX(getPosX());
		m_tail->setPosY(getPosY());

		m_tail->setParentX(x);
		m_tail->setParentY(y);
	}
}
void Serpent::addSegments(Dungeon &dungeon) {
	std::shared_ptr<MonsterSegment> segment = std::make_shared<MonsterSegment>(m_tailX, m_tailY, getPosX(), getPosY(), SERPENT_TAIL, "Bone_32x32.png");
	dungeon.getMonsters().emplace_back(segment);
	m_tail = segment;

	dungeon.getDungeon()[m_tailY*dungeon.getCols() + m_tailX].enemy = true;
}
void Serpent::removeSegments(Dungeon &dungeon) {
	// Destroy the tail upon death
	m_tail->setDestroyed(true);
}

ArmoredSerpent::ArmoredSerpent(Dungeon &dungeon, int x, int y, int turns) 
	: Serpent(dungeon, x, y, turns, ARMORED_SERPENT, "Red_Fish_48x48.png", 10 + randInt(10), 2, 8, 1) {
	setGold(15);
}


//		





// ================================
//		  :::: SMASHER ::::
// ================================

Smasher::Smasher(FirstBoss *dungeon) 
	: Monster(BOSSCOLS / 2, 3, 500, 0, 4, 1, "Smasher"), 
	m_dungeon(dungeon), m_moveActive(false), m_moveEnding(true), m_frenzy(false) {

}

void Smasher::move(Dungeon &dungeon) {
	//	check if smasher is currently executing a move
	if (isActive()) {
		switch (getMove()) {
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
	else if (!isEnded()) {
		switch (getMove()) {
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
		setActive(true);
		setMove(1);
		
		move1();
	}
	// 15% chance : avalanche attack
	else if (randmove > 12 && randmove <= 27) {
		setActive(true);
		setMove(2);
		
		move2();
	}
	// 3% chance : archer/enemy spawning, max 3 at once
	else if (randmove > 27 && randmove <= 30) {
		if (m_dungeon->getMonsters().size() < 3)
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
	int col = getPosX();
	int row = getPosY();

	//	if wall immediately beneath smasher is an unbreakable wall, reset position to top
	if (m_dungeon->getDungeon()[(row + 2)*BOSSCOLS + col].wall_type == UNB_WALL) {
		setEnded(false);
		setActive(false);
		resetUpward();

		return;
	}

	moveSmasherD();
}
void Smasher::move2() { // starts avalanche
	int col = getPosX();
	int row = getPosY();

	int n = randInt(3);

	// if this is the beginning of the move, then select a direction to move in
	if (m_move == '0') {
		switch (n) {
		case 0: m_move = 'u'; break;
		case 1: m_move = 'l'; break;
		case 2: m_move = 'r'; break;
		}
	}

	//	move is upward
	if (m_move == 'u') {
		//	if wall immediately above smasher is an unbreakable wall, make rocks fall
		if (m_dungeon->getDungeon()[(row - 2)*BOSSCOLS + col].wall_type == UNB_WALL) {
			cocos2d::experimental::AudioEngine::play2d("Rock_Slide.mp3", false, 1.0f);

			setEnded(false);
			setActive(false);
			
			avalanche();
			resetDownward();

			return;
		}

		moveSmasherU();
	}
	//	move is to the left
	else if (m_move == 'l') {
		//	if wall immediately to the left of smasher is an unbreakable wall, make rocks fall
		if (m_dungeon->getDungeon()[(row)*BOSSCOLS + col - 2].wall_type == UNB_WALL) {
			cocos2d::experimental::AudioEngine::play2d("Rock_Slide.mp3", false, 1.0f);

			setEnded(false);
			setActive(false);
			
			avalanche();
			resetRightward();
			;

			return;
		}

		moveSmasherL();
	}
	//	move is to the right
	else if (m_move == 'r') {
		//	if wall immediately to the right of smasher is an unbreakable wall, make rocks fall
		if (m_dungeon->getDungeon()[(row)*BOSSCOLS + col + 2].wall_type == UNB_WALL) {
			cocos2d::experimental::AudioEngine::play2d("Rock_Slide.mp3", false, 1.0f);

			setEnded(false);
			setActive(false);
			
			avalanche();
			resetLeftward();
			;

			return;
		}

		moveSmasherR();
	}

}
void Smasher::avalanche() {
	int col = getPosX();
	int row = getPosY();
	int x, y, speed, spikecount;
	bool trap;
	spikecount = 15 + randInt(8);
	std::shared_ptr<FallingSpike> spike = nullptr;

	for (int i = 0; i < spikecount; i++) {
		x = 1 + randInt(BOSSCOLS - 2);
		y = 1 + randInt(5);
		speed = 1 + randInt(3);
		trap = m_dungeon->getDungeon()[y*BOSSCOLS + x].trap;
		spike = std::make_shared<FallingSpike>(x, y, speed);

		while (trap || x == col || x == col - 1 || x == col + 1) { // while spike position clashes with anything
			x = 1 + randInt(BOSSCOLS - 2);
			y = 1 + randInt(5);
			spike->setPosX(x);	// reroll it
			spike->setPosY(y);

			trap = m_dungeon->getDungeon()[spike->getPosY()*BOSSCOLS + spike->getPosX()].trap;
		}

		// add spike sprite
		spike->setSprite(m_dungeon->createSprite(spike->getPosX(), spike->getPosY(), 0, "CeilingSpike_48x48.png"));

		m_dungeon->getTraps().push_back(spike);
		m_dungeon->getDungeon()[spike->getPosY()*BOSSCOLS + spike->getPosX()].trap = true;
		
		spike.reset();
	}
}
void Smasher::move3() {	// spawn two archers
	int n = 2;
	int x, y;
	bool hero, enemy;

	if (m_dungeon->getMonsters().size() <= 2) {
		while (n > 0) {
			x = 1 + randInt(BOSSCOLS - 2);
			y = BOSSROWS - 2 - randInt(10);

			std::shared_ptr<Monster> archer = std::make_shared<Archer>(x, y);

			enemy = m_dungeon->getDungeon()[y*BOSSCOLS + x].enemy;
			hero = m_dungeon->getDungeon()[y*BOSSCOLS + x].hero;
			while (enemy || hero) {
				x = 1 + randInt(BOSSCOLS - 2);
				y = BOSSROWS - 2 - randInt(10);
				archer->setPosX(x);
				archer->setPosY(y);

				enemy = m_dungeon->getDungeon()[archer->getPosY()*BOSSCOLS + archer->getPosX()].enemy;
				hero = m_dungeon->getDungeon()[archer->getPosY()*BOSSCOLS + archer->getPosX()].hero;
			}

			archer->setSprite(m_dungeon->createSprite(x, y, 1, "Archer_48x48.png"));

			m_dungeon->getMonsters().emplace_back(archer);
			m_dungeon->getDungeon()[y*BOSSCOLS + x].enemy = true;
			n--;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Spawns.mp3", false, 1.0f);
	}
}
void Smasher::move4() { // earthquake

}
void Smasher::move5() { // projectile bouncers

}

void Smasher::moveSmasher() { // pick a random direction to move in
	int n = randInt(4);

	switch (n) {
	case 0: moveSmasherL(); break;
	case 1: moveSmasherR(); break;
	case 2: moveSmasherU(); break;
	case 3: moveSmasherD(); break;
	default: break;
	}
}
void Smasher::moveSmasherL() {
	int col = getPosX();
	int row = getPosY();
	int i, j;

	//	if wall immediately to left smasher is an unbreakable wall, do nothing
	if (m_dungeon->getDungeon()[row*BOSSCOLS + col - 2].wall_type == UNB_WALL) {
		return;
	}

	//  if there is only one space to the left of the smasher
	else if (m_dungeon->getDungeon()[row*BOSSCOLS + col - 3].wall_type == UNB_WALL) {
		// right side of smasher is set to spaces
		for (i = col + 1, j = row - 1; j < row + 2; j++) {
			m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SPACE;
			m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = false;
		}

		//	smasher is pushed to left wall
		for (i = col - 2, j = row - 1; j < row + 2; j++) {
			//	if player is in the way, crush them
			if (m_dungeon->getDungeon()[j*BOSSCOLS + i].hero) {
				attack(*m_dungeon, *m_dungeon->getPlayerVector()[0]);
			}
			// if there are any monsters in the way, crush them
			if (m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy && m_dungeon->getDungeon()[j*BOSSCOLS + i].upper != SMASHER) {
				int pos = m_dungeon->findMonster(i, j);
				attack(*(m_dungeon->getMonsters().at(pos)));

				// check if monster died
				if (m_dungeon->getMonsters().at(pos)->getHP() <= 0) {
					m_dungeon->monsterDeath(pos);
				}
			}

			m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SMASHER;
			m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = true;
		}

		playSound("Smasher_Smash.mp3");
		
		setPosX(col - 1);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

	//	if there are only two spaces to the left of the smasher
	else if (m_dungeon->getDungeon()[row*BOSSCOLS + col - 4].wall_type == UNB_WALL) {
		// right 2 layers of smasher is set to spaces
		for (i = col + 1; i > col - 1; i--) {
			for (j = row - 1; j < row + 2; j++) {
				m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SPACE;
				m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	smasher is pushed to left wall
		for (i = col - 2; i > col - 4; i--) {
			for (j = row - 1; j < row + 2; j++) {
				//	if player is in the way, crush them
				if (m_dungeon->getDungeon()[j*BOSSCOLS + i].hero) {
					m_dungeon->getPlayerVector()[0]->setPosX(col - 3);
					m_dungeon->getDungeon()[j*BOSSCOLS + i].hero = false;
					m_dungeon->getDungeon()[m_dungeon->getPlayerVector()[0]->getPosY()*BOSSCOLS + m_dungeon->getPlayerVector()[0]->getPosX()].hero = true;
					attack(*m_dungeon, *m_dungeon->getPlayerVector()[0]);
				}

				// if there are any monsters in the way, crush them
				if (m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy && m_dungeon->getDungeon()[j*BOSSCOLS + i].upper != SMASHER) {
					int pos = m_dungeon->findMonster(i, j);
					attack(*(m_dungeon->getMonsters().at(pos)));

					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != col - 3 && m_dungeon->getDungeon()[j*BOSSCOLS + (i - 1)].enemy) {
						int n = m_dungeon->findMonster(i - 1, j);

						m_dungeon->monsterDeath(n);
					}

					// move the monster and sprite
					m_dungeon->pushMonster(m_dungeon->getDungeon(), i, j, 'X', col - 3, 0);

					// check if monster died
					if (m_dungeon->getMonsters().at(pos)->getHP() <= 0) {
						m_dungeon->monsterDeath(pos);
					}
				}

				m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SMASHER;
				m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = true;

			}
		}


		playSound("Smasher_Smash.mp3");
		//m_dungeon->queueMoveSprite(getSprite(), 'L');
		setPosX(col - 2);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

	//	if there are at least 3 spaces to left of the smasher
	else if (m_dungeon->getDungeon()[row*BOSSCOLS + col - 4].wall_type != UNB_WALL) {
		// right 2 layers of smasher is set to spaces
		for (i = col + 1; i > col - 1; i--) {
			for (j = row - 1; j < row + 2; j++) {
				m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SPACE;
				m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	left two layers set to smasher
		for (i = col - 2; i > col - 4; i--) {
			for (j = row - 1; j < row + 2; j++) {
				// if player is in the way, move them to the side
				if (m_dungeon->getDungeon()[j*BOSSCOLS + i].hero) {
					// if there's another enemy, push them
					if (m_dungeon->getDungeon()[j*BOSSCOLS + i - 1].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), i - 1, j, 'l');
					}
					if (m_dungeon->getDungeon()[j*BOSSCOLS + i - 2].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), i - 2, j, 'l');
					}

					m_dungeon->getPlayerVector()[0]->setPosX(col - 4);
					m_dungeon->getDungeon()[j*BOSSCOLS + i].hero = false;
					m_dungeon->getDungeon()[m_dungeon->getPlayerVector()[0]->getPosY()*BOSSCOLS + m_dungeon->getPlayerVector()[0]->getPosX()].hero = true;
				}

				// if there are any monsters in the way, move them
				if (m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy && m_dungeon->getDungeon()[j*BOSSCOLS + i].upper != SMASHER) {
					// if player is in the way, push them
					if (m_dungeon->getDungeon()[j*BOSSCOLS + i - 1].hero || m_dungeon->getDungeon()[j*BOSSCOLS + i - 2].hero) {
						m_dungeon->pushPlayer('l');
					}
					// if there's another enemy, push them
					if (m_dungeon->getDungeon()[j*BOSSCOLS + i - 1].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), i - 1, j, 'l');
					}
					if (i != col - 3 && m_dungeon->getDungeon()[j*BOSSCOLS + i - 2].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), i - 2, j, 'l');
					}

					m_dungeon->pushMonster(m_dungeon->getDungeon(), i, j, 'X', col - 4, 0);
				}

				m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SMASHER;
				m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = true;
			}
		}
		//m_dungeon->queueMoveSprite(getSprite(), 'L');
		setPosX(col - 2);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

}
void Smasher::moveSmasherR() {
	int col = getPosX();
	int row = getPosY();
	int i, j;

	//	if wall immediately to right of smasher is an unbreakable wall, do nothing
	if (m_dungeon->getDungeon()[row*BOSSCOLS + col + 2].wall_type == UNB_WALL) {
		return;
	}

	//  if there is only one space to the right of the smasher
	else if (m_dungeon->getDungeon()[row*BOSSCOLS + col + 3].wall_type == UNB_WALL) {
		// left side of smasher is set to spaces
		for (i = col - 1, j = row - 1; j < row + 2; j++) {
			m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SPACE;
			m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = false;
		}

		//	smasher is pushed to right wall
		for (i = col + 2, j = row - 1; j < row + 2; j++) {
			//	if player is in the way, crush them
			if (m_dungeon->getDungeon()[j*BOSSCOLS + i].hero) {
				attack(*m_dungeon, *m_dungeon->getPlayerVector()[0]);
			}
			// if there are any monsters in the way, crush them
			if (m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy && m_dungeon->getDungeon()[j*BOSSCOLS + i].upper != SMASHER) {
				int pos = m_dungeon->findMonster(i, j);
				attack(*(m_dungeon->getMonsters().at(pos)));

				// check if monster died
				if (m_dungeon->getMonsters().at(pos)->getHP() <= 0) {
					m_dungeon->monsterDeath(pos);
				}
			}

			m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SMASHER;
			m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = true;
		}
		
		playSound("Smasher_Smash.mp3");
		//m_dungeon->queueMoveSprite(getSprite(), 'r');
		setPosX(col + 1);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

	//	if there are only two spaces to the right of the smasher
	else if (m_dungeon->getDungeon()[row*BOSSCOLS + col + 4].wall_type == UNB_WALL) {
		// left two layers of smasher is set to spaces
		for (i = col - 1; i < col + 1; i++) {
			for (j = row - 1; j < row + 2; j++) {
				m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SPACE;
				m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	smasher is pushed to right wall
		for (i = col + 2; i < col + 4; i++) {
			for (j = row - 1; j < row + 2; j++) {
				//	if player is in the way, crush them
				if (m_dungeon->getDungeon()[j*BOSSCOLS + i].hero) {
					m_dungeon->getPlayerVector()[0]->setPosX(col + 3);
					m_dungeon->getDungeon()[j*BOSSCOLS + i].hero = false;
					m_dungeon->getDungeon()[m_dungeon->getPlayerVector()[0]->getPosY()*BOSSCOLS + m_dungeon->getPlayerVector()[0]->getPosX()].hero = true;
					attack(*m_dungeon, *m_dungeon->getPlayerVector()[0]);
				}

				// if there are any monsters in the way, crush them
				if (m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy && m_dungeon->getDungeon()[j*BOSSCOLS + i].upper != SMASHER) {
					// crush the monster
					int pos = m_dungeon->findMonster(i, j);
					attack(*(m_dungeon->getMonsters().at(pos)));

					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != col + 3 && m_dungeon->getDungeon()[j*BOSSCOLS + (i + 1)].enemy) {
						int n = m_dungeon->findMonster(i + 1, j);

						m_dungeon->monsterDeath(n);
					}

					// move the sprite
					m_dungeon->pushMonster(m_dungeon->getDungeon(), i, j, 'X', col + 3, 0);

					// check if monster died (should be dead)
					if (m_dungeon->getMonsters().at(pos)->getHP() <= 0) {
						m_dungeon->monsterDeath(pos);
					}
				}

				m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SMASHER;
				m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = true;

			}
		}
		
		playSound("Smasher_Smash.mp3");
		//m_dungeon->queueMoveSprite(getSprite(), 'R');
		setPosX(col + 2);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

	//	if there are at least 3 spaces to right of the smasher
	else if (m_dungeon->getDungeon()[row*BOSSCOLS + col + 4].wall_type != UNB_WALL) {
		// left two layers of smasher is set to spaces
		for (i = col - 1; i < col + 1; i++) {
			for (j = row - 1; j < row + 2; j++) {
				m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SPACE;
				m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	right two layers set to smasher
		for (i = col + 2; i < col + 4; i++) {
			for (j = row - 1; j < row + 2; j++) {
				// if player is in the way, move them to the side
				if (m_dungeon->getDungeon()[j*BOSSCOLS + i].hero) {
					// if there's another enemy, push them
					if (m_dungeon->getDungeon()[j*BOSSCOLS + i + 1].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), i + 1, j, 'r');
					}
					if (m_dungeon->getDungeon()[j*BOSSCOLS + i + 2].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), i + 2, j, 'r');
					}

					m_dungeon->getPlayerVector()[0]->setPosX(col + 4);
					m_dungeon->getDungeon()[j*BOSSCOLS + i].hero = false;
					m_dungeon->getDungeon()[m_dungeon->getPlayerVector()[0]->getPosY()*BOSSCOLS + m_dungeon->getPlayerVector()[0]->getPosX()].hero = true;
				}

				// if there are any monsters in the way, move them
				if (m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy && m_dungeon->getDungeon()[j*BOSSCOLS + i].upper != SMASHER) {
					// if player is in the way, push them
					if (m_dungeon->getDungeon()[j*BOSSCOLS + i + 1].hero || m_dungeon->getDungeon()[j*BOSSCOLS + i + 2].hero) {
						m_dungeon->pushPlayer('r');
					}
					// if there's another enemy, push them
					if (m_dungeon->getDungeon()[j*BOSSCOLS + i + 1].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), i + 1, j, 'r');
					}
					if (i != col + 3 && m_dungeon->getDungeon()[j*BOSSCOLS + i + 2].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), i + 2, j, 'r');
					}

					m_dungeon->pushMonster(m_dungeon->getDungeon(), i, j, 'X', col + 4, 0);
				}

				m_dungeon->getDungeon()[j*BOSSCOLS + i].upper = SMASHER;
				m_dungeon->getDungeon()[j*BOSSCOLS + i].enemy = true;
			}
		}

		//m_dungeon->queueMoveSprite(getSprite(), 'R');
		setPosX(col + 2);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

}
void Smasher::moveSmasherU() {
	int col = getPosX();
	int row = getPosY();
	int i, j;

	//	if wall immediately above smasher is an unbreakable wall, do nothing
	if (m_dungeon->getDungeon()[(row - 2)*BOSSCOLS + col].wall_type == UNB_WALL) {
		return;
	}

	//  if there is only one space above the smasher
	else if (m_dungeon->getDungeon()[(row - 3)*BOSSCOLS + col].wall_type == UNB_WALL) {
		// bottom layer of smasher is set to spaces
		for (i = row + 1, j = col - 1; j < col + 2; j++) {
			m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SPACE;
			m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = false;
		}

		//	smasher is pushed to upper layer
		for (i = row - 2, j = col - 1; j < col + 2; j++) {
			//	if player is in the way, crush them
			if (m_dungeon->getDungeon()[i*BOSSCOLS + j].hero) {
				attack(*m_dungeon, *m_dungeon->getPlayerVector()[0]);
			}
			// if there are any monsters in the way, crush them
			if (m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy && m_dungeon->getDungeon()[i*BOSSCOLS + j].upper != SMASHER) {
				// find and crush the monster
				int pos = m_dungeon->findMonster(j, i);
				attack(*(m_dungeon->getMonsters().at(pos)));

				// check if monster died
				if (m_dungeon->getMonsters().at(pos)->getHP() <= 0) {
					m_dungeon->monsterDeath(pos);
				}
			}

			m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SMASHER;
			m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = true;
		}
		
		playSound("Smasher_Smash.mp3");
		//m_dungeon->queueMoveSprite(getSprite(), 'u');
		setPosY(row - 1);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

	//	if there are only two spaces above the smasher
	else if (m_dungeon->getDungeon()[(row - 4)*BOSSCOLS + col].wall_type == UNB_WALL) {
		// bottom 2 layers of smasher is set to spaces
		for (i = row + 1; i > row - 1; i--) {
			for (j = col - 1; j < col + 2; j++) {
				m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SPACE;
				m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	smasher is pushed to ceiling
		for (i = row - 2; i > row - 4; i--) {
			for (j = col - 1; j < col + 2; j++) {
				//	if player is in the way, crush them
				if (m_dungeon->getDungeon()[i*BOSSCOLS + j].hero) {
					m_dungeon->getPlayerVector()[0]->setPosY(row - 3);
					m_dungeon->getDungeon()[i*BOSSCOLS + j].hero = false;
					m_dungeon->getDungeon()[m_dungeon->getPlayerVector()[0]->getPosY()*BOSSCOLS + m_dungeon->getPlayerVector()[0]->getPosX()].hero = true;
					attack(*m_dungeon, *m_dungeon->getPlayerVector()[0]);
				}

				// if there are any monsters in the way, crush them
				if (m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy && m_dungeon->getDungeon()[i*BOSSCOLS + j].upper != SMASHER) {
					// find and crush the monster
					int pos = m_dungeon->findMonster(j, i);
					attack(*(m_dungeon->getMonsters().at(pos)));

					// if there are two enemies stacked, then kill the enemy closest to the wall first
					// :::: This is for the purpose of looking more natural on screen.
					if (i != 1 && m_dungeon->getDungeon()[(i - 1)*BOSSCOLS + j].enemy) {
						int n = m_dungeon->findMonster(j, i - 1);

						m_dungeon->monsterDeath(n);
					}

					// move the monster
					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i, 'Y', 0, row - 3);

					// check if monster died
					if (m_dungeon->getMonsters().at(pos)->getHP() <= 0) {
						m_dungeon->monsterDeath(pos);
					}

				}

				m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SMASHER;
				m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = true;

			}
		}
		
		playSound("Smasher_Smash.mp3");
		//m_dungeon->queueMoveSprite(getSprite(), 'U');
		setPosY(row - 2);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

	//	if there are at least 3 spaces above the smasher
	else if (m_dungeon->getDungeon()[(row - 4)*BOSSCOLS + col].wall_type != UNB_WALL) {
		// bottom 2 layers of smasher is set to spaces
		for (i = row + 1; i > row - 1; i--) {
			for (j = col - 1; j < col + 2; j++) {
				m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SPACE;
				m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	top two layers above smasher set to smasher
		for (i = row - 2; i > row - 4; i--) {
			for (j = col - 1; j < col + 2; j++) {
				// if player is in the way, move them above
				if (m_dungeon->getDungeon()[i*BOSSCOLS + j].hero) {
					// if there's another enemy, push them
					if (m_dungeon->getDungeon()[(i - 1)*BOSSCOLS + j].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 1, 'u');
					}
					if (m_dungeon->getDungeon()[(i - 2)*BOSSCOLS + j].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 2, 'u');
					}

					m_dungeon->getPlayerVector()[0]->setPosY(row - 4);
					m_dungeon->getDungeon()[i*BOSSCOLS + j].hero = false;
					m_dungeon->getDungeon()[m_dungeon->getPlayerVector()[0]->getPosY()*BOSSCOLS + m_dungeon->getPlayerVector()[0]->getPosX()].hero = true;
				}

				// if there are any monsters in the way, move them
				if (m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy && m_dungeon->getDungeon()[i*BOSSCOLS + j].upper != SMASHER) {
					// if player is in the way, push them
					if (m_dungeon->getDungeon()[(i - 1)*BOSSCOLS + j].hero || m_dungeon->getDungeon()[(i - 2)*BOSSCOLS + j].hero) {
						m_dungeon->pushPlayer('u');
					}
					// if there's another enemy, push them
					if (m_dungeon->getDungeon()[(i - 1)*BOSSCOLS + j].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 1, 'u');
					}
					if (i != 1 && m_dungeon->getDungeon()[(i - 2)*BOSSCOLS + j].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 2, 'u');
					}

					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i, 'Y', 0, row - 4);
				}

				m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SMASHER;
				m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = true;
			}
		}
		
		//m_dungeon->queueMoveSprite(getSprite(), 'U');
		setPosY(row - 2);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

}
void Smasher::moveSmasherD() {
	int col = getPosX();
	int row = getPosY();
	int i, j;

	//	if wall immediately beneath smasher is an unbreakable wall, do nothing
	if (m_dungeon->getDungeon()[(row + 2)*BOSSCOLS + col].wall_type == UNB_WALL) {
		return;
	}

	// if there is only one space underneath the smasher
	else if (m_dungeon->getDungeon()[(row + 3)*BOSSCOLS + col].wall_type == UNB_WALL) {
		// top layer of smasher is set to spaces
		for (i = row - 1, j = col - 1; j < col + 2; j++) {
			m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SPACE;
			m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = false;
		}

		//	smasher is pushed to bottom layer
		for (i = row + 2, j = col - 1; j < col + 2; j++) {
			//	if player is in the way, crush them
			if (m_dungeon->getDungeon()[i*BOSSCOLS + j].hero) {
				attack(*m_dungeon, *m_dungeon->getPlayerVector()[0]);
			}

			// if there are any monsters in the way, crush them
			if (m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy && m_dungeon->getDungeon()[i*BOSSCOLS + j].upper != SMASHER) {

				int pos = m_dungeon->findMonster(j, i);
				attack(*(m_dungeon->getMonsters().at(pos)));

				// check if monster died
				if (m_dungeon->getMonsters().at(pos)->getHP() <= 0) {
					m_dungeon->monsterDeath(pos);
				}
			}

			m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SMASHER;
			m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = true;
		}
		
		playSound("Smasher_Smash.mp3");
		//m_dungeon->queueMoveSprite(getSprite(), 'd');
		setPosY(row + 1);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

	//	if there are only two spaces underneath the smasher
	else if (m_dungeon->getDungeon()[(row + 4)*BOSSCOLS + col].wall_type == UNB_WALL) {
		// top 2 layers of smasher is set to spaces
		for (i = row - 1; i < row + 1; i++) {
			for (j = col - 1; j < col + 2; j++) {
				m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SPACE;
				m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	smasher is pushed to bottom layer
		for (i = row + 2; i < row + 4; i++) {
			for (j = col - 1; j < col + 2; j++) {
				//	if player is in the way, crush them
				if (m_dungeon->getDungeon()[i*BOSSCOLS + j].hero) {
					m_dungeon->getPlayerVector()[0]->setPosY(BOSSROWS - 2);
					m_dungeon->getDungeon()[i*BOSSCOLS + j].hero = false;
					m_dungeon->getDungeon()[m_dungeon->getPlayerVector()[0]->getPosY()*BOSSCOLS + m_dungeon->getPlayerVector()[0]->getPosX()].hero = true;
					attack(*m_dungeon, *m_dungeon->getPlayerVector()[0]);
				}

				// if there are any monsters in the way, crush them dead
				if (m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy && m_dungeon->getDungeon()[i*BOSSCOLS + j].upper != SMASHER) {
					// smasher attacking a monster is a formality at this point
					int pos = m_dungeon->findMonster(j, i);
					attack(*(m_dungeon->getMonsters().at(pos)));

					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != row + 3 && m_dungeon->getDungeon()[(i + 1)*BOSSCOLS + j].enemy) {
						int n = m_dungeon->findMonster(j, i + 1);

						m_dungeon->monsterDeath(n);
					}

					// NOTE: this must be after the attack, otherwise pos is then incorrect
					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i, 'Y', 0, BOSSROWS - 2);

					// check if monster died (monster will be dead)
					if (m_dungeon->getMonsters().at(pos)->getHP() <= 0) {
						m_dungeon->monsterDeath(pos);
					}

				}

				m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SMASHER;
				m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = true;
			}
		}
		
		playSound("Smasher_Smash.mp3");
		//m_dungeon->queueMoveSprite(getSprite(), 'D');
		setPosY(row + 2);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

	//	if there are at least 3 spaces underneath the smasher
	else if (m_dungeon->getDungeon()[(row + 4)*BOSSCOLS + col].wall_type != UNB_WALL) {
		// top 2 layers of smasher is set to spaces
		for (i = row - 1; i < row + 1; i++) {
			for (j = col - 1; j < col + 2; j++) {
				m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SPACE;
				m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	bottom two beneath smasher set to smasher
		for (i = row + 2; i < row + 4; i++) {
			for (j = col - 1; j < col + 2; j++) {
				// if player is in the way, move them below
				if (m_dungeon->getDungeon()[i*BOSSCOLS + j].hero) {
					// if there's another enemy, push them
					if (m_dungeon->getDungeon()[(i + 1)*BOSSCOLS + j].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i + 1, 'd');
					}
					if (m_dungeon->getDungeon()[(i + 2)*BOSSCOLS + j].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i + 2, 'd');
					}

					m_dungeon->getPlayerVector()[0]->setPosY(row + 4);
					m_dungeon->getDungeon()[i*BOSSCOLS + j].hero = false;
					m_dungeon->getDungeon()[m_dungeon->getPlayerVector()[0]->getPosY()*BOSSCOLS + m_dungeon->getPlayerVector()[0]->getPosX()].hero = true;
				}

				// if there are any monsters in the way, move them
				if (m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy) {// && m_dungeon->getDungeon()[i*BOSSCOLS + j].upper != SMASHER) {
					// if player is in the way, push them
					if (m_dungeon->getDungeon()[(i + 1)*BOSSCOLS + j].hero || m_dungeon->getDungeon()[(i + 2)*BOSSCOLS + j].hero) {
						m_dungeon->pushPlayer('d');
					}

					// if there's another enemy, push them
					if (m_dungeon->getDungeon()[(i + 1)*BOSSCOLS + j].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i + 1, 'd');
					}
					if (i != row + 3 && m_dungeon->getDungeon()[(i + 2)*BOSSCOLS + j].enemy) {
						m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i + 2, 'd');
					}

					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i, 'Y', 0, row + 4);
				}

				m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SMASHER;
				m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = true;
			}
		}
	
		setPosY(row + 2);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
	}

}

void Smasher::resetLeftward() {

	int col = getPosX();
	int row = getPosY();
	int rand = randInt(3);

	//	if smasher has reset far enough to the left, end cooldown
	if (col < BOSSCOLS / 1.5 - rand) {
		setEnded(true);
		m_move = '0';

		return;
	}

	moveSmasherL();
}
void Smasher::resetRightward() {

	int col = getPosX();
	int row = getPosY();
	int rand = randInt(3);

	//	if smasher has reset far enough to the right, end cooldown
	if (col > BOSSCOLS / 3 + rand) {
		setEnded(true);
		m_move = '0';

		return;
	}

	moveSmasherR();
	
}
void Smasher::resetUpward() {
	int col = getPosX();
	int row = getPosY();
	int i, j;
	int rand = randInt(3);

	if (row < 6 + rand) {
		setEnded(true);

		return;
	}

	//	bottom 3 layers set to space
	for (i = row + 1; i > row - 2; i--) {
		for (j = col - 1; j < col + 2; j++) {
			m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SPACE;
			m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = false;
		}
	}

	//  three layers above smasher set to smasher
	for (i = row - 2; i > row - 5; i--) {
		for (j = col - 1; j < col + 2; j++) {
			// if player is in the way, move them above
			if (m_dungeon->getDungeon()[i*BOSSCOLS + j].hero) {
				// if there's another enemy, push them
				if (m_dungeon->getDungeon()[(i - 1)*BOSSCOLS + j].enemy) {
					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 1, 'u');
				}
				if (m_dungeon->getDungeon()[(std::max(i - 2, 1))*BOSSCOLS + j].enemy) {
					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 2, 'u');
				}
				if (m_dungeon->getDungeon()[(std::max(i - 3, 1))*BOSSCOLS + j].enemy) {
					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 3, 'u');
				}

				m_dungeon->getPlayerVector()[0]->setPosY(row - 5);
				m_dungeon->getDungeon()[i*BOSSCOLS + j].hero = false;
				m_dungeon->getDungeon()[m_dungeon->getPlayerVector()[0]->getPosY()*BOSSCOLS + m_dungeon->getPlayerVector()[0]->getPosX()].hero = true;
			}
			// if there are any monsters in the way, move them
			if (m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy && m_dungeon->getDungeon()[i*BOSSCOLS + j].upper != SMASHER) {
				// if player is in the way, push them
				if (m_dungeon->getDungeon()[(i - 1)*BOSSCOLS + j].hero || m_dungeon->getDungeon()[(i - 2)*BOSSCOLS + j].hero) {
					m_dungeon->pushPlayer('d');
				}
				// if there's another enemy, push them
				if (m_dungeon->getDungeon()[(i - 1)*BOSSCOLS + j].enemy) {
					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 1, 'u');
				}
				if (m_dungeon->getDungeon()[(std::max(i - 2, 1))*BOSSCOLS + j].enemy) {
					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 2, 'u');
				}
				if (m_dungeon->getDungeon()[(std::max(i - 3, 1))*BOSSCOLS + j].enemy) {
					m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i - 3, 'u');
				}

				m_dungeon->pushMonster(m_dungeon->getDungeon(), j, i, 'Y', 0, row - 5);
			}

			m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SMASHER;
			m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = true;
		}
	}

	setPosY(row - 3);
	m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
}
void Smasher::resetDownward() {
	int col = getPosX();
	int row = getPosY();
	int i, j;
	int rand = randInt(3);

	if (row > 6 + rand) {
		setEnded(true);
		m_move = '0';

		return;
	}

	//	top 2 layers set to space
	for (i = row - 1; i < row + 1; i++) {
		for (j = col - 1; j < col + 2; j++) {
			m_dungeon->getDungeon()[i*BOSSCOLS + j].upper = SPACE;
			m_dungeon->getDungeon()[i*BOSSCOLS + j].enemy = false;
		}
	}

	moveSmasherD();
}

void Smasher::attack(Dungeon &dungeon, Player &p) {
	playSound("Smasher_HitBy1.mp3");

	int damage = 10 + randInt(3);
	dungeon.damagePlayer(damage);

	// stun player after being crushed
	p.addAffliction(std::make_shared<Stun>(2));
}
void Smasher::attack(Monster &m) {
	playSound("Smasher_HitBy1.mp3");

	m.setHP(0);
}
void Smasher::death(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// pause current music, play victory music, and then resume music once finished
	cocos2d::experimental::AudioEngine::pauseAll();
	int id = playMusic("Victory! All Clear.mp3", false);

	cocos2d::experimental::AudioEngine::setFinishCallback(id,
		[](int id, std::string music) {
		cocos2d::experimental::AudioEngine::resumeAll();
	});

	dungeon.queueRemoveSprite(getSprite());

	std::shared_ptr<Objects> idol = std::make_shared<Idol>(cols / 2, 3);
	dungeon[idol->getPosY()*cols + idol->getPosX()].object = idol;
	dungeon[idol->getPosY()*cols + idol->getPosX()].item = true;

	// sprite for idol
	dungeon.addSprite(dungeon.item_sprites, idol->getPosX(), idol->getPosY(), -1, "Glass_Toy_48x48.png");

	for (int i = y - 1; i < y + 2; i++) {
		for (int j = x - 1; j < x + 2; j++) {
			dungeon[i*cols + j].upper = SPACE;
			dungeon[i*cols + j].enemy = false;
		}
	}
}

bool Smasher::isActive() const {
	return m_moveActive;
}
void Smasher::setActive(bool status) {
	m_moveActive = status;
}
bool Smasher::isEnded() const {
	return m_moveEnding;
}
void Smasher::setEnded(bool status) {
	m_moveEnding = status;
}
int Smasher::getMove() const {
	return m_moveType;
}
void Smasher::setMove(int move) {
	m_moveType = move;
}
bool Smasher::isFrenzied() const {
	return m_frenzy;
}
void Smasher::startFrenzy() {
	m_frenzy = true;
}

