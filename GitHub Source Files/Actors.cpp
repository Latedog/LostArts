#include "AudioEngine.h"
#include "GUI.h"
#include "global.h"
#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include "utilities.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>

using std::string;
using std::to_string;
// remove later
using std::cout;
using std::endl;
using std::setw;

//		ACTOR FUNCTIONS
Actors::Actors() : m_x(0), m_y(0), m_hp(0), m_armor(0), m_str(0), m_dex(0), m_wep(std::make_shared<ShortSword>()) {

}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, string name, bool burnable, bool bleedable, bool healable, bool stunnable, bool freezable)
	: m_x(x), m_y(y), m_maxhp(hp), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_name(name),
	m_burnable(burnable), m_bleedable(bleedable), m_healable(healable), m_stunnable(stunnable), m_freezable(freezable) {

	m_frozen = m_burned = m_bled = m_healed = m_poisoned = m_stunned = false;
}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep)
	: m_x(x), m_y(y), m_maxhp(hp), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_wep(wep),
	m_burnable(true), m_bleedable(true), m_healable(true), m_stunnable(true), m_freezable(true) {

	m_frozen = m_burned = m_bled = m_healed = m_poisoned = m_stunned = false;
}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, string name)
	: m_x(x), m_y(y), m_maxhp(hp), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_wep(wep), m_name(name),
	m_burnable(true), m_bleedable(true), m_healable(true), m_stunnable(true), m_freezable(true) {

	m_frozen = m_burned = m_bled = m_healed = m_poisoned = m_stunned = false;
}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, std::string name, bool burnable, bool bleedable, bool healable, bool stunnable, bool freezable)
	: m_x(x), m_y(y), m_maxhp(hp), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_wep(wep), m_name(name),
	m_burnable(burnable), m_bleedable(bleedable), m_healable(healable), m_stunnable(stunnable), m_freezable(freezable) {
	
	m_frozen = m_burned = m_bled = m_healed = m_poisoned = m_stunned = false;
}
Actors::~Actors() {

}

int Actors::getPosX() const {
	return m_x;
}
int Actors::getPosY() const {
	return m_y;
}
int Actors::getMaxHP() const {
	return m_maxhp;
}
int Actors::getHP() const {
	return m_hp;
}
int Actors::getArmor() const {
	return m_armor;
}
int Actors::getStr() const {
	return m_str;
}
int Actors::getDex() const {
	return m_dex;
}
int Actors::getInt() const {
	return m_int;
}
int Actors::getLuck() const {
	return m_luck;
}
std::shared_ptr<Weapon>& Actors::getWeapon() {
	return m_wep;
}
std::string Actors::getName() const {
	return m_name;
}
std::vector<std::shared_ptr<Afflictions>>& Actors::getAfflictions() {
	return m_afflictions;
}
cocos2d::Sprite* Actors::getSprite() {
	return m_sprite;
}
std::string Actors::getImageName() const {
	return m_image;
}

void Actors::setPosX(int x) {
	m_x = x;
}
void Actors::setPosY(int y) {
	m_y = y;
}
void Actors::setrandPosX(int maxcols) {
	m_x = randInt(maxcols - 2) + 1;
}
void Actors::setrandPosY(int maxrows) {
	m_y = randInt(maxrows - 2) + 1;
}
void Actors::setMaxHP(int maxhp) {
	m_maxhp = maxhp;
}
void Actors::setHP(int hp) {
	m_hp = hp;
}
void Actors::setArmor(int armor) {
	m_armor = armor;
}
void Actors::setStr(int str) {
	m_str = str;
}
void Actors::setDex(int dex) {
	m_dex = dex;
}
void Actors::setInt(int intellect) {
	m_int = intellect;
}
void Actors::setLuck(int luck) {
	m_luck = luck;
}
void Actors::setWeapon(std::shared_ptr<Weapon> wep) {
	m_wep = wep;
}
void Actors::setName(std::string name) {
	m_name = name;
}
void Actors::addAffliction(std::shared_ptr<Afflictions> affliction) {
	
	int index = findAffliction(affliction->getName());

	// if affliction wasn't found, then add new affliction
	if (index == -1) {
		m_afflictions.push_back(affliction);

		return;
	}

	// otherwise, add more turns remaining to the existing affliction
	getAfflictions().at(index)->setTurnsLeft(getAfflictions().at(index)->getTurnsLeft() + affliction->getTurnsLeft());

}
void Actors::setSprite(cocos2d::Sprite* sprite) {
	m_sprite = sprite;
}
void Actors::setImageName(std::string image) {
	m_image = image;
}

bool Actors::isPlayer() const {
	return m_isPlayer;
}
void Actors::setPlayerFlag(bool player) {
	m_isPlayer = player;
}

bool Actors::isMonster() const {
	return m_isMonster;
}
void Actors::setMonsterFlag(bool monster) {
	m_isMonster = monster;
}

void Actors::checkAfflictions() {
	int pos;

	// turn sprite upright
	if (getSprite() != nullptr)
		getSprite()->setRotation(0);

	for (int i = 0; i < m_afflictions.size(); i++) {
		m_afflictions[i]->afflict(*(m_afflictions[i]), *this);

		if (m_afflictions[i]->isExhausted()) {
			pos = i;
			m_afflictions.erase(m_afflictions.begin() + pos);

			i--; // prevents skipping of any afflictions
		}
	}
}
int Actors::findAffliction(std::string name) {
	for (int i = 0; i < m_afflictions.size(); i++) {
		if (m_afflictions.at(i)->getName() == name) {
			return i;
		}
	}
	return -1;
}
void Actors::removeAffliction(std::string name) {
	int pos = findAffliction(name);

	if (pos == -1)
		return;

	m_afflictions.at(pos)->setTurnsLeft(0); // set turns to zero and let the affliction do the rest
}

bool Actors::lavaImmune() const {
	return m_lavaImmune;
}
void Actors::setLavaImmunity(bool immune) {
	m_lavaImmune = immune;
}

bool Actors::isFlying() const {
	return m_flying;
}
void Actors::setFlying(bool flying) {
	m_flying = flying;
}

bool Actors::isSturdy() const {
	return m_sturdy;
}
void Actors::setSturdy(bool sturdy) {
	m_sturdy = sturdy;
}

bool Actors::hasBloodlust() const {
	return m_bloodlust;
}
void Actors::setBloodlust(bool lust) {
	m_bloodlust = lust;
}

bool Actors::canBeStunned() const {
	return m_stunnable;
}
bool Actors::canBeBurned() const {
	return m_burnable;
}
bool Actors::canBeBled() const {
	return m_bleedable;
}
bool Actors::canBeHealed() const {
	return m_healable;
}
bool Actors::canBeFrozen() const {
	return m_freezable;
}
bool Actors::canBePoisoned() const {
	return m_poisonable;
}
void Actors::setCanBeStunned(bool stunnable) {
	m_stunnable = stunnable;
}
void Actors::setCanBeBurned(bool burnable) {
	m_burnable = burnable;
}
void Actors::setCanBeBled(bool bleedable) {
	m_bleedable = bleedable;
}
void Actors::setCanBeHealed(bool healable) {
	m_healable = healable;
}
void Actors::setCanBeFrozen(bool freezable) {
	m_freezable = freezable;
}
void Actors::setCanBePoisoned(bool poisonable) {
	m_poisonable = poisonable;
}

bool Actors::isBurned() const {
	return m_burned;
}
void Actors::setBurned(bool burned) {
	m_burned = burned;
}

bool Actors::isBled() const {
	return m_bled;
}
void Actors::setBleed(bool bled) {
	m_bled = bled;
}

bool Actors::isStunned() const {
	return m_stunned;
}
void Actors::setStunned(bool stun) {
	m_stunned = stun;
}

bool Actors::isFrozen() const {
	return m_frozen;
}
void Actors::setFrozen(bool freeze) {
	m_frozen = freeze;
}

bool Actors::isPoisoned() const {
	return m_poisoned;
}
void Actors::setPoisoned(int poisoned) {
	m_poisoned = poisoned;
}

bool Actors::isInvisible() const {
	return m_invisible;
}
void Actors::setInvisible(int invisible) {
	m_invisible = invisible;
}

bool Actors::isEthereal() const {
	return m_ethereal;
}
void Actors::setEthereal(int ethereal) {
	m_ethereal = ethereal;
}

bool Actors::isConfused() const {
	return m_confused;
}
void Actors::setConfused(int confused) {
	m_confused = confused;
}

bool Actors::isBuffed() const {
	return m_buffed;
}
void Actors::setBuffed(int buffed) {
	m_buffed = buffed;
}

bool Actors::isInvulnerable() const {
	return m_invulnerable;
}
void Actors::setInvulnerable(bool invulnerable) {
	m_invulnerable = invulnerable;
}

bool Actors::isStuck() const {
	return m_stuck;
}
void Actors::setStuck(int stuck) {
	m_stuck = stuck;
}


// ================================================
//				:::: THE PLAYERS ::::
// ================================================
Player::Player() : Actors(randInt(MAXCOLS) + 1, randInt(MAXROWS) + 1, 100, 2, 2, 2, std::make_shared<ShortSword>(), "Player") {
	m_hasShield = false;
	m_blocking = false;
	m_maxhp = 100;
	m_winner = false;

	setPlayerFlag(true);
	setMonsterFlag(false);
}
Player::~Player() {

}

void Player::attack(std::vector<std::shared_ptr<Monster>> &monsters, std::vector<std::shared_ptr<Objects>> &actives, int pos, std::vector<std::string> &text) {
	//text.push_back("You swing your " + getWeapon()->getAction() + " and... ");

	int playerPoints = getDex() + getWeapon()->getDexBonus();
	int monsterPoints = monsters.at(pos)->getDex();

	// attack roll successful
	if (randInt(playerPoints) >= randInt(monsterPoints)) {
		int damage = 1 + randInt(getStr() + getWeapon()->getDmg()) - monsters.at(pos)->getArmor();
		if (damage > 0) {
			string monster = monsters.at(pos)->getName();

			// play hit sound effect
			if (monster == "Smasher") {
				std::string sound;
				int n = randInt(5) + 1;
				switch (n) {
				case 1: sound = "Metal_Hit1.mp3"; break;
				case 2: sound = "Metal_Hit2.mp3"; break;
				case 3: sound = "Metal_Hit3.mp3"; break;
				case 4: sound = "Metal_Hit4.mp3"; break;
				case 5: sound = "Metal_Hit5.mp3"; break;
				}
				cocos2d::experimental::AudioEngine::play2d(sound, false, 1.0f);
			}
			else {
				cocos2d::experimental::AudioEngine::play2d("Enemy_Hit.mp3", false, 1.0f);
			}

			monsters.at(pos)->setHP(monsters.at(pos)->getHP() - damage);

			//text.push_back("deal " + to_string(damage) + " damage to the " + monsters.at(pos)->getName() + "!\n");
			//text.push_back("The " + monster + " has " + to_string(monsters.at(pos)->getHP()) + " HP remaining.\n\n");


			// :::: Check for affliction/ability of weapon ::::

			
			if (getWeapon()->hasBleed()) {

				int bleedchance = 1 + randInt(5);
				// 40% chance to bleed
				if (bleedchance > 3) {
					monsters.at(pos)->addAffliction(std::make_shared<Bleed>());
					monsters.at(pos)->setBleed(true);

					//text.push_back("The " + monster + " was cut deep!\n");
				}
			}
		}
		else {
			int n = randInt(5) + 1;
			std::string sound;
			switch (n) {
			case 1: sound = "Slash1.mp3"; break;
			case 2: sound = "Slash2.mp3"; break;
			case 3: sound = "Slash3.mp3"; break;
			case 4: sound = "Slash4.mp3"; break;
			case 5: sound = "Slash5.mp3"; break;
			}
			cocos2d::experimental::AudioEngine::play2d(sound, false, 1.0f);

			text.push_back("your attack fails!\n");
		}
	}
	// attack roll failed
	else {
		int n = randInt(5) + 1;
		std::string sound;
		switch (n) {
		case 1: sound = "Slash1.mp3"; break;
		case 2: sound = "Slash2.mp3"; break;
		case 3: sound = "Slash3.mp3"; break;
		case 4: sound = "Slash4.mp3"; break;
		case 5: sound = "Slash5.mp3"; break;
		}
		cocos2d::experimental::AudioEngine::play2d(sound, false, 1.0f);

		text.push_back("miss.\n");
	}
}
void Player::attack(Dungeon &dungeon, Actors &a) {
	int playerPoints = getDex() + getWeapon()->getDexBonus();
	int monsterPoints = a.getDex();

	// if player is invisible, they lose invisibility when attacking
	if (isInvisible()) {
		removeAffliction("invisibility");
	}

	// if monster is frozen, attack is automatically successful albeit the ice increases the enemy's armor
	if (a.isFrozen()) {
		// play enemy hit
		playEnemyHit();

		// increase money bonus on successful hit
		increaseMoneyBonus();
		
		// if player can lifesteal, roll
		if (canLifesteal()) {
			rollHeal();
		}

		// damage the monster or otherwise
		int damage = std::max(1, getStr() + getWeapon()->getDmg() - (2 * a.getArmor()));

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
		if (getWeapon()->hasAbility()) {
			getWeapon()->useAbility(*getWeapon(), dungeon, a);
		}

		// tint monster sprite red and flash
		runMonsterDamageBlink(a.getSprite());

		return;
	}

	// if monster is stunned, attack is automatically successful
	if (a.isStunned()) {
		// play enemy hit
		playEnemyHit();

		// increase money bonus on successful hit
		increaseMoneyBonus();

		// if player can lifesteal, roll
		if (canLifesteal()) {
			rollHeal();
		}

		// damage the monster or otherwise
		int damage = std::max(1, getStr() + getWeapon()->getDmg() - (1 * a.getArmor()));

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
		if (getWeapon()->hasAbility()) {
			getWeapon()->useAbility(*getWeapon(), dungeon, a);
		}

		// tint monster sprite red and flash
		runMonsterDamageBlink(a.getSprite());

		return;
	}

	// attack roll successful
	if ((playerPoints > 0 ? randInt(playerPoints) : 0) >= (monsterPoints > 0 ? randInt(monsterPoints) : 0)) {
		
		string monster = a.getName();

		// play hit sound effect
		if (monster == "Smasher") {
			playHitSmasher();
		}
		else {
			playEnemyHit();
		}

		// increase money bonus on successful hit
		increaseMoneyBonus();

		// if player can lifesteal, roll
		if (canLifesteal()) {
			rollHeal();
		}

		// :::: Check for affliction/ability of weapon ::::
		if (getWeapon()->hasAbility()) {
			getWeapon()->useAbility(*getWeapon(), dungeon, a);
		}

		int damage = std::max(1, 1 + randInt(getStr() + getWeapon()->getDmg()) - a.getArmor());

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
		if (!a.isSturdy()) {
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
				dungeon.queueMoveSprite(a.getSprite(), facingDirection());
			}

			// in any case, stun them for a turn
			a.addAffliction(std::make_shared<Stun>(1));
		}
	}
	// attack roll failed
	else {
		playMiss();
	}
}

/*
void Player::showInventory() {
	cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n";
	cout << setw(50) << "Inventory:" << endl;
	if (m_invsize + m_iteminvsize == 0)
		cout << setw(55) << "You have no items." << endl;
	else {
		unsigned i = 0, j;
		char c = 97;

		if (m_invsize != 0) {
			for (i = 0; i < m_invsize; i++) {
				c += i;
				cout << setw(38) << c << ". " << m_inv.at(i).getAction() << endl;
				c = 97;
			}
		}

		if (m_iteminvsize != 0) {
			for (j = 0; j < m_iteminvsize; j++) {
				c += i + j;
				cout << setw(38) << c << ". " << m_iteminv.at(j).getItem() << endl;
				c = 97;
			}
		}
	}
	cout << endl;	
}
void Player::showWeapons(std::vector<std::string> &text) {
	cout << "Weapons:" << endl;
	
	char c = 97;
	for (unsigned i = 0; i < m_invsize; i++) {
		c += i;
		cout << c << ". " << m_inv.at(i).getAction() << endl;
		c = 97;
	}
	
	cout << endl;
}
void Player::showItems(std::vector<std::string> &text) {
	cout << "Items:" << endl;

	char c = 97;
	for (unsigned i = 0; i < m_iteminvsize; i++) {
		c += i;
		cout << c << ". " << m_iteminv.at(i).getItem() << endl;
		c = 97;
	}

	cout << endl;
}
*/

int Player::getMoney() const {
	return m_money;
}
void Player::setMoney(int money) {
	m_money = money;
}
int Player::getMoneyMultiplier() const {
	return m_moneyMultiplier;
}
void Player::setMoneyMultiplier(int multiplier) {
	m_moneyMultiplier = multiplier;
}
float Player::getMoneyBonus() const {
	return m_moneyBonus;
}
void Player::setMoneyBonus(float bonus) {
	m_moneyBonus = bonus;
}
int Player::getMaxMoneyBonus() const {
	return m_maxMoneyBonus;
}
void Player::increaseMoneyBonus() {
	if (getMoneyBonus() < getMaxMoneyBonus())
		setMoneyBonus(getMoneyBonus() + 0.20f);
}
void Player::decreaseMoneyBonus() {
	// decrease money bonus when hit
	if (getMoneyBonus() - 0.6f > 0)
		setMoneyBonus(getMoneyBonus() - 0.6f);
	else if (getMoneyBonus() - 0.6f < 0) {
		setMoneyBonus(0);
	}
}
std::vector<std::shared_ptr<Weapon>>& Player::getWeapons() {
	return m_weapons;
}
std::vector<std::shared_ptr<Drops>>& Player::getItems() {
	return m_items;
}
int Player::getMaxWeaponInvSize() const {
	return m_maxwepinv;
}
int Player::getMaxItemInvSize() const {
	return m_maxiteminv;
}

bool Player::hasShield() const {
	return m_hasShield;
}
void Player::setShieldPossession(bool possesses) {
	m_hasShield = possesses;
}
std::shared_ptr<Shield>& Player::getShield() {
	return m_shield;
}
std::shared_ptr<Shield> Player::getPlayerShield() const {
	return m_shield;
}
void Player::equipShield(Dungeon &dungeon, std::shared_ptr<Shield> shield, bool shop) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// if player has a shield, then drop and replace with new one
	if (hasShield()) {
		dungeon.addSprite(dungeon.item_sprites, rows, x, y, -1, getShield()->getImageName()); // old sprite first

		// handles the placement of shield on the ground
		dropShield(dungeon.getDungeon(), rows, cols);
		dungeon.removeSprite(dungeon.item_sprites, rows, x, y - shop);

		m_shield = shield;
		m_hasShield = true;

		return;
	}

	dungeon[y*cols + x].item_name = EMPTY;
	dungeon[y*cols + x].object = nullptr;
	dungeon[y*cols + x].item = false;
	dungeon.removeSprite(dungeon.item_sprites, rows, x, y - shop);

	m_shield = shield;
	m_hasShield = true;

	// play shield equip sound
	//playShieldEquip(getPlayerShield()->getItem());
}
void Player::dropShield(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols) {
	int x = getPosX();
	int y = getPosY();

	// play shield drop sound
	cocos2d::experimental::AudioEngine::play2d("Wood_Shield_Drop.mp3", false, 1.0f);

	//getShield()->setPosX(x);
	//getShield()->setPosY(y);

	std::shared_ptr<Objects> oldShield = getShield();

	dungeon[y*maxcols + x].object = oldShield;
	dungeon[y*maxcols + x].item_name = oldShield->getItem();
	dungeon[y*maxcols + x].item = true;

	m_hasShield = false;
	m_shield = std::make_shared<Shield>();
}
void Player::shieldBroken() {
	// play shield broken sound effect
	playShieldBroken(getPlayerShield()->getItem());

	m_hasShield = false;
	m_shield = std::make_shared<Shield>();
}
void Player::repairShield(int repairs) {
	// if repairs do not exceed the shield's durability, then just repair it as normal
	if (getShield()->getDurability() + repairs < getShield()->getMaxDurability()) {
		getShield()->setDurability(getShield()->getDurability() + repairs);
	}
	// if repairs exceeds but does not equal the shield's max durability, then set to max
	else if (getShield()->getDurability() + repairs > getShield()->getMaxDurability()) {
		getShield()->setDurability(getShield()->getMaxDurability());
	}
	// else if equal, then do nothing
	else {
		;
	}
}
bool Player::isBlocking() const {
	return m_blocking;
}
void Player::setBlock(bool blocking) {
	m_blocking = blocking;
}
void Player::blocked() {
	// play shield hit sound effect
	playShieldHit(getPlayerShield()->getItem());

	// decrease shield's durability by half the value of how much the shield blocks by
	if (getShield()->getDurability() > 0) {
		getShield()->setDurability(getShield()->getDurability() - getShield()->getDefense() / 2);
	}
	// if shield breaks, remove from the player
	if (getShield()->getDurability() <= 0) {
		shieldBroken();
	}
}
bool Player::shieldCoverage(int mx, int my) {
	int px = getPosX();
	int py = getPosY();
	bool blocked = false;

	// check shield's coverage
	switch (getShield()->getCoverage()) {
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

	return blocked;
}
char Player::facingDirection() const {
	return m_facing;
}
void Player::setFacingDirection(char facing) {
	m_facing = facing;
}
char Player::getAction() const {
	return m_action;
}
void Player::setAction(char action) {
	m_action = action;
}

void Player::addWeapon(std::shared_ptr<Weapon> weapon) {
	m_weapons.push_back(weapon);
}
void Player::addItem(std::shared_ptr<Drops> drop) {
	m_items.push_back(drop);
}
void Player::wield(int index) {
	std::shared_ptr<Weapon> old = getWeapon();

	// apply and unapply any previous bonuses
	if (old->hasBonus()) {
		old->unapplyBonus(*this);
	}
	//if (m_weapons[index]->hasBonus()) {
	//	m_weapons[index]->applyBonus(*this);
	//}

	setWeapon(m_weapons[index]);				// switch to new weapon
	m_weapons.erase(m_weapons.begin() + index);		// remove item just equipped
	m_weapons.push_back(old);					// push old weapon into inventory
}
void Player::use(Dungeon &dungeon, _Tile &tile, int index) {
	// if item is meant for player, do this
	if (m_items.at(index)->forPlayer()) {
		m_items.at(index)->changeStats(*m_items.at(index), *this);
	}
	// otherwise use it on the dungeon/surroundings
	else {
		m_items.at(index)->useItem(*m_items.at(index), dungeon);
	}

	if (m_items.at(index)->getItem() != SKELETON_KEY)
		m_items.erase(m_items.begin() + index);	// remove item just used
}

bool Player::canLifesteal() const {
	return m_lifesteal;
}
void Player::setLifesteal(bool steal) {
	m_lifesteal = steal;
}
void Player::rollHeal() {
	if (getHP() > 0 && getHP() + 2 < getMaxHP()) {
		if (randInt(100) + 1 > 90) {
			// 15% chance to heal the player if below max hp
			setHP(getHP() + 2);
		}
	}
}

bool Player::hasTrinket() const {
	return m_hasTrinket;
}
void Player::setTrinketFlag(bool hasTrinket) {
	m_hasTrinket = hasTrinket;
}
std::shared_ptr<Trinket>& Player::getTrinket() {
	return m_trinket;
}
void Player::equipTrinket(Dungeon &dungeon, std::shared_ptr<Trinket> trinket, bool shop) {
	// temp pickup sound
	cocos2d::experimental::AudioEngine::play2d("Potion_Pickup.mp3", false, 1.0f);
	
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
	dungeon.removeSprite(dungeon.item_sprites, rows, x, y - shop);

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
	cocos2d::experimental::AudioEngine::play2d("Wood_Shield_Drop.mp3", false, 1.0f);

	std::shared_ptr<Objects> oldTrinket = getTrinket();

	dungeon.getDungeon()[y*cols + x].object = oldTrinket;
	dungeon.getDungeon()[y*cols + x].item_name = oldTrinket->getItem();
	dungeon.getDungeon()[y*cols + x].item = true;

	dungeon.removeSprite(dungeon.item_sprites, rows, x, y - shop);
	dungeon.addSprite(dungeon.item_sprites, dungeon.getRows(), x, y, -1, oldTrinket->getImageName());

	// add new trinket
	setTrinketFlag(true);
	m_trinket = trinket;
	m_trinket->apply(dungeon, *this);
}

void Player::setItemToFront(int index) {
	std::shared_ptr<Drops> temp = getItems()[0];
	getItems()[0] = getItems()[index];
	getItems()[index] = temp;
}

int Player::getVision() const {
	return m_vision;
}
void Player::setVision(int vision) {
	m_vision = vision;
}

bool Player::hasSkeletonKey() const {
	for (int i = 0; i < m_items.size(); i++) {
		if (m_items.at(i)->getItem() == SKELETON_KEY)
			return true;
	}
	return false;
}
void Player::checkKeyConditions(std::vector<std::string> &text) {
	// if player hp is less than a specified threshold, and player was hit after picking up the key, break it
	if (this->getHP() < 8 && this->getHP() < this->keyHP()) {
		for (int i = 0; i < m_items.size(); i++) {
			if (m_items.at(i)->getItem() ==	SKELETON_KEY) {
				// play key broken sound effect
				cocos2d::experimental::AudioEngine::play2d("Skeleton_Key_Broken.mp3", false, 1.0f);

				//text.push_back("The mysterious trinket shatters.\n");
				m_items.erase(m_items.begin() + i);	// remove the key
				break;
			}
		}
	}
}
int Player::keyHP() const {
	return m_keyhp;
}
void Player::setKeyHP(int keyhp) {
	m_keyhp = keyhp;
}

void Player::setWin(bool win) {
	m_winner = win;
}
bool Player::getWin() const {
	return m_winner;
}
string Player::getDeath() const {
	return m_death;
}
void Player::setDeath(string m) {
	m_death = m;
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
		playDialogue(dungeon);
		m_interacted = true;
	}
	// else if player has interacted but NPC has not be satisfied
	else if (m_interacted) {
		checkSatisfaction(dungeon);
		if (!m_satisfied) {
			addInteractedDialogue(m_dialogue);
			playDialogue(dungeon);
		}
		else if (m_satisfied && !m_rewardGiven) {
			addSatisfiedDialogue(m_dialogue);
			playDialogue(dungeon);
			reward(dungeon);
			m_rewardGiven = true;
		}
		else if (m_satisfied && m_rewardGiven) {
			addFinalDialogue(m_dialogue);
			playDialogue(dungeon);
		}
	}

}

void NPC::playDialogue(Dungeon& dungeon) {
	auto scene = dynamic_cast<Level1Scene*>(dungeon.m_scene);
	scene->kbListener->setEnabled(false);
	scene->m_hud->NPCInteraction(scene->kbListener, dungeon, m_dialogue);
}
void NPC::setDialogue(std::vector<std::string> dialogue) {
	m_dialogue = dialogue;
}
void NPC::setSatisfaction(bool satisfied) {
	m_satisfied = satisfied;
}


//		CREATURE LOVER (Lionel)
CreatureLover::CreatureLover(int x, int y) : NPC(x, y, CREATURE_LOVER, "Dead_Mage_48x48.png") {
	std::vector<std::string> dialogue;

	m_wantedCreature = GOO_SACK;

	dialogue.push_back("Oh... hello.");
	dialogue.push_back("I love " + m_wantedCreature + "s.");
	dialogue.push_back("Those " + m_wantedCreature + "s are just adorable little things!");
	dialogue.push_back("Can you bring me one?");

	setDialogue(dialogue);
}

void CreatureLover::checkSatisfaction(Dungeon& dungeon) {
	int cols = dungeon.getCols();

	bool enemy;
	std::string monsterName;

	for (int y = getPosY() - 1; y < getPosY() + 2; y++) {
		for (int x = getPosX() - 1; x < getPosX() + 2; x++) {

			if (dungeon[y*cols + x].enemy) {
				int pos = dungeon.findMonster(x, y);

				if (pos != -1) {
					monsterName = dungeon.getMonsters()[pos]->getName();

					if (monsterName == m_wantedCreature) {
						cocos2d::experimental::AudioEngine::play2d("Creature_Lover_Talk.mp3", false, 1.0f);

						dungeon[y*cols + x].enemy = false;
						m_creature = dungeon.getMonsters()[pos]; // give creature lover the monster
						m_creature->getSprite()->setColor(cocos2d::Color3B(120, 120, 120));

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
	int x = dungeon.getPlayer().getPosX();
	int y = dungeon.getPlayer().getPosY();

	if (dungeon[y*cols + x].item) {
		dungeon.itemHash(dungeon.getDungeon(), x, y);
	}

	switch (1 + randInt(5)) {
	case 1: dungeon[y*cols + x].object = std::make_shared<IronShield>(); break;
	case 2:	dungeon[y*cols + x].object = std::make_shared<Katana>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<FrostShield>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<ArmorDrop>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<GoldenLongSword>(); break;
	default: break;
	}

	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getItem();
	dungeon[y*cols + x].item = true;
	dungeon.addSprite(dungeon.item_sprites, dungeon.getRows(), x, y, -1, dungeon[y*cols + x].object->getImageName());
}

void CreatureLover::addInteractedDialogue(std::vector<std::string> &dialogue) {
	dialogue.clear();

	dialogue.push_back(m_wantedCreature + "s, " + m_wantedCreature + "s, " + m_wantedCreature + "s!!!");
}
void CreatureLover::addSatisfiedDialogue(std::vector<std::string> &dialogue) {
	dialogue.clear();

	dialogue.push_back("?!");
	dialogue.push_back("A " + m_wantedCreature + "! " + "I can't believe it!");
	dialogue.push_back("You deserve something for this.");
}
void CreatureLover::addFinalDialogue(std::vector<std::string> &dialogue) {
	dialogue.clear();

	dialogue.push_back("...So adorable!");
}



// ============================================
//				:::: MONSTERS ::::
// ============================================
Monster::Monster() {

}
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, string name, bool burnable, bool bleedable, bool healable, bool stunnable, bool freezable)
	: Actors(x, y, hp, armor, str, dex, wep, name, burnable, bleedable, healable, stunnable, freezable) {
	setMonsterFlag(true);
}
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, string name, bool burnable, bool bleedable, bool healable, bool stunnable, bool freezable) 
	: Actors(x, y, hp, armor, str, dex, name, burnable, bleedable, healable, stunnable, freezable) {
	setMonsterFlag(true);
}
Monster::~Monster() {

}

void Monster::move(Monster &m, Dungeon &dungeon) {
	m.move(dungeon);
}
void Monster::encounter(Player &p, Monster &m, std::vector<std::string> &text) {
	m.attack(p, text);
}
void Monster::encounter(Dungeon &dungeon, Player &p, Monster &m) {
	m.attack(dungeon, p);
}
void Monster::encounter(Monster &m1, Monster &m2) {
	m1.attack(m2);
}
void Monster::attack(Player &p, std::vector<std::string> &text) {
	string weapon = this->getWeapon()->getAction();
	/*if (weapon == "Wood Bow") {
		text.push_back("The " + this->getName() + " fires their " + weapon + " at you... ");
	}
	else {
		text.push_back("The " + this->getName() + " slashes their " + weapon + " at you... ");
	}*/

	std::string monster = this->getName();
	int mx = this->getPosX();
	int my = this->getPosY();
	int px = p.getPosX();
	int py = p.getPosY();

	int damage;
	int monsterPoints = getDex();
	int playerPoints = p.getDex() + p.getWeapon()->getDexBonus();

	bool blocked = false;
	// check if player is blocking
	if (p.isBlocking()) {
		// check shield's coverage
		switch (p.getShield()->getCoverage()) {
		// level 1 coverage blocks only in the front
		case 1: {
			// check direction player is facing
			switch (p.facingDirection()) {
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
			switch (p.facingDirection()) {
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
			switch (p.facingDirection()) {
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
	}

	// if the player successfully shielded the attack, increase the player's damage reduction and check if
	// monster is stunnable and set their stun status
	if (blocked) {
		damage = std::max(0, 1 + randInt(getStr() + getWeapon()->getDmg()) - (p.getArmor() + p.getShield()->getDefense()));

		// handles shield durability change
		p.blocked();

		/*if (p.getShield()->hasAbility()) {
			p.getPlayerShield()->useAbility(*this);
		}*/

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my <= 1)) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}

		return;
	}
	
	// if monster's attack roll is successful
	if ((monsterPoints > 0 ? randInt(monsterPoints) : 0) >= (playerPoints > 0 ? randInt(playerPoints) : 0)) {

		damage = std::max(1, 1 + randInt(getStr() + getWeapon()->getDmg()) - p.getArmor());

		// play got hit sound effect
		playGotHit();

		p.setHP(p.getHP() - damage);
		//text.push_back("and hits for " + to_string(damage) + " damage!");
		//text.push_back((damage >= 3 ? " Ouch!\n" : "\n"));
	}
	// else attack roll was unsuccessful
	else {
		;// text.push_back("and misses!\n");
	}
}
void Monster::attack(Dungeon &dungeon, Player &p) {
	//string weapon = this->getWeapon()->getAction();

	std::string monster = this->getName();
	int mx = this->getPosX();
	int my = this->getPosY();
	int px = p.getPosX();
	int py = p.getPosY();

	int damage;
	int monsterPoints = getDex();
	int playerPoints = p.getDex() + p.getWeapon()->getDexBonus();

	bool blocked = false;
	// check if player is blocking
	if (p.isBlocking()) {
		// check shield's coverage
		switch (p.getShield()->getCoverage()) {
			// level 1 coverage blocks only in the front
		case 1: {
			// check direction player is facing
			switch (p.facingDirection()) {
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
			switch (p.facingDirection()) {
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
			switch (p.facingDirection()) {
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
	}

	// if the player successfully shielded the attack, increase the player's damage reduction and check if
	// monster is stunnable and set their stun status
	if (blocked) {
		damage = std::max(0, 1 + randInt(getStr() + getWeapon()->getDmg()) - (p.getArmor() + p.getShield()->getDefense()));

		// handles shield durability change
		p.blocked();

		if (p.getShield()->hasAbility()) {
			p.getPlayerShield()->useAbility(dungeon, *this);
		}

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my <= 1)) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}

		return;
	}

	// if monster's attack roll is successful
	if ((monsterPoints > 0 ? randInt(monsterPoints) : 0) >= (playerPoints > 0 ? randInt(playerPoints) : 0)) {

		damage = std::max(1, 1 + randInt(getStr() + getWeapon()->getDmg()) - p.getArmor());

		// play got hit sound effect
		playGotHit();

		p.setHP(p.getHP() - damage);
	}
	// else attack roll was unsuccessful
	else {
		;
	}
}
void Monster::death(Monster &m, Dungeon &dungeon) {
	m.death(dungeon);
}
void Monster::destroy(Dungeon &dungeon) {
	// destroyMonster is used to eliminate monsters without having them drop anything

	int x = getPosX();
	int y = getPosY();

	int pos = dungeon.findMonster(x, y);

	if (pos == -1)
		return;

	dungeon[y*dungeon.getCols() + x].top = SPACE;
	dungeon[y*dungeon.getCols() + x].enemy = false;

	dungeon.queueRemoveSprite(getSprite());
	dungeon.getMonsters().erase(dungeon.getMonsters().begin() + pos);
}

bool Monster::attemptChase(std::vector<_Tile> &dungeon, int maxcols, int &shortest, int smelldist, int origdist, int x, int y, char &first_move, char &optimal_move) {

	// if enemy is immediately adjacent to the player, return to attack
	if (smelldist == origdist && \
		(dungeon[(y - 1)*maxcols + x].hero || dungeon[(y + 1)*maxcols + x].hero || dungeon[y*maxcols + x - 1].hero || dungeon[y*maxcols + x + 1].hero)) {
		dungeon[y*maxcols + x].marked = false;
		return true;
	}

	// mark the current tile as visited
	dungeon[y*maxcols + x].marked = true;

	// player was not found within origdist # of steps OR took more steps than the current shortest path found
	if (smelldist < 0 || smelldist < shortest) {
		dungeon[y*maxcols + x].marked = false;
		return false;
	}

	if (dungeon[y*maxcols + x].hero) {
		if (smelldist > shortest) {
			shortest = smelldist;
			optimal_move = first_move;
		}
		else if (smelldist == shortest) {
			// randomly select optimal path if multiple optimal paths found
			if (randInt(2) == 1)
				optimal_move = first_move;
		}
		dungeon[y*maxcols + x].marked = false;
		return true; // if player is found, tell goblin to advance
	}

	if (!dungeon[y*maxcols + x + 1].marked && (!dungeon[y*maxcols + x + 1].wall || dungeon[y*maxcols + x + 1].hero)) {
		if (smelldist == origdist)
			first_move = 'r';
		attemptChase(dungeon, maxcols, shortest, smelldist - 1, origdist, x + 1, y, first_move, optimal_move);
	}

	if (!dungeon[y*maxcols + x - 1].marked && (!dungeon[y*maxcols + x - 1].wall || dungeon[y*maxcols + x - 1].hero)) {
		if (smelldist == origdist)
			first_move = 'l';
		attemptChase(dungeon, maxcols, shortest, smelldist - 1, origdist, x - 1, y, first_move, optimal_move);
	}

	if (!dungeon[(y + 1)*maxcols + x].marked && (!dungeon[(y + 1)*maxcols + x].wall || dungeon[(y + 1)*maxcols + x].hero)) {
		if (smelldist == origdist)
			first_move = 'd';
		attemptChase(dungeon, maxcols, shortest, smelldist - 1, origdist, x, y + 1, first_move, optimal_move);
	}

	if (!dungeon[(y - 1)*maxcols + x].marked && (!dungeon[(y - 1)*maxcols + x].wall || dungeon[(y - 1)*maxcols + x].hero)) {
		if (smelldist == origdist)
			first_move = 'u';
		attemptChase(dungeon, maxcols, shortest, smelldist - 1, origdist, x, y - 1, first_move, optimal_move);
	}

	// unmark the tile as visited when backtracking
	dungeon[y*maxcols + x].marked = false;

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
bool Monster::playerInRange(Player p, int range) {
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
bool Monster::playerInDiagonalRange(Player p, int range) {
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
void Monster::moveWithSuccessfulChase(Dungeon &dungeon, char move) {
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
	default: encounter(dungeon, dungeon.getPlayerVector().at(0), *this); return;
	}

	bool trap = dungeon.getDungeon()[(y + m)*cols + (x + n)].trap;
	if (trap) {
		int pos = dungeon.findTrap(x + n, y + m);

		if (pos != -1) {
			bool lethal = dungeon.getTraps()[pos]->isLethal();

			// if trap would do damage to them and they are a smart enemy, they do nothing
			if (lethal && isSmart())
				return;
		}
	}

	if (!dungeon.getDungeon()[(y + m)*cols + (x + n)].enemy) {
		dungeon.getDungeon()[y*cols + x].top = SPACE;
		dungeon.getDungeon()[y*cols + x].enemy = false;
		dungeon.getDungeon()[(y + m)*cols + (x + n)].enemy = true;

		dungeon.queueMoveSprite(getSprite(), move);
		setPosX(x + n); setPosY(y + m);
	}

}
void Monster::moveMonsterRandomly(Dungeon &dungeon) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int mx = getPosX();
	int my = getPosY();

	int n = randInt(2);
	int m = 0;
	char move;

	bool leftwall, rightwall, upwall, downwall;
	leftwall = dungeon.getDungeon()[my*maxcols + mx - 1].wall;
	rightwall = dungeon.getDungeon()[my*maxcols + mx + 1].wall;
	upwall = dungeon.getDungeon()[(my - 1)*maxcols + mx].wall;
	downwall = dungeon.getDungeon()[(my + 1)*maxcols + mx].wall;

	bool leftenemy, rightenemy, upenemy, downenemy;
	leftenemy = dungeon.getDungeon()[my*maxcols + mx - 1].enemy;
	rightenemy = dungeon.getDungeon()[my*maxcols + mx + 1].enemy;
	upenemy = dungeon.getDungeon()[(my - 1)*maxcols + mx].enemy;
	downenemy = dungeon.getDungeon()[(my + 1)*maxcols + mx].enemy;

	bool lefttrap, righttrap, uptrap, downtrap;
	lefttrap = dungeon.getDungeon()[my*maxcols + mx - 1].trap;
	righttrap = dungeon.getDungeon()[my*maxcols + mx + 1].trap;
	uptrap = dungeon.getDungeon()[(my - 1)*maxcols + mx].trap;
	downtrap = dungeon.getDungeon()[(my + 1)*maxcols + mx].trap;

	//  if monster is surrounded completely, do nothing (such as goblin or wanderer blocking an archer's path)
	if ((leftwall || leftenemy || lefttrap) && (rightwall || rightenemy || righttrap) &&
		(downwall || downenemy || downtrap) && (upwall || upenemy || uptrap)) {
		return;
	}
	//	if monster is surrounded by walls on left and right
	else if ((leftwall || leftenemy || lefttrap) && (rightwall || rightenemy || righttrap)) {
		move = 'v';
	}
	//	if monster is surrounded by walls on top and bottom
	else if ((downwall || downenemy || downtrap) && (upwall || upenemy || uptrap)) {
		move = 'h';
	}
	//	else pick a random direction
	else {
		move = (n == 0 ? 'h' : 'v');
	}

	char top, upper;
	bool wall, enemy, trap;

	//	move is horizontal
	if (move == 'h') {
		n = -1 + randInt(3);

		upper = dungeon.getDungeon()[my*maxcols + mx + n].upper;
		wall = dungeon.getDungeon()[my*maxcols + mx + n].wall;
		enemy = dungeon.getDungeon()[my*maxcols + mx + n].enemy;
		trap = dungeon.getDungeon()[my*maxcols + mx + n].trap;

		while (n == 0 || wall || enemy || trap || upper != SPACE) {
			n = -1 + randInt(3);

			upper = dungeon.getDungeon()[my*maxcols + mx + n].upper;
			wall = dungeon.getDungeon()[my*maxcols + mx + n].wall;
			enemy = dungeon.getDungeon()[my*maxcols + mx + n].enemy;
			trap = dungeon.getDungeon()[my*maxcols + mx + n].trap;
		}

		dungeon.getDungeon()[my*maxcols + mx + n].enemy = true;
		dungeon.getDungeon()[my*maxcols + mx].top = SPACE;
		dungeon.getDungeon()[my*maxcols + mx].enemy = false;

		//moveSprite(monster_sprites, maxrows, mx, my, n == -1 ? 'l' : 'r');
		dungeon.queueMoveSprite(getSprite(), n == -1 ? 'l' : 'r');
		setPosX(mx + n);
	}
	//	move is vertical
	else if (move == 'v') {
		m = -1 + randInt(3);

		upper = dungeon.getDungeon()[(my + m)*maxcols + mx].upper;
		wall = dungeon.getDungeon()[(my + m)*maxcols + mx].wall;
		enemy = dungeon.getDungeon()[(my + m)*maxcols + mx].enemy;
		trap = dungeon.getDungeon()[(my + m)*maxcols + mx].trap;

		while (m == 0 || wall || enemy || trap || upper != SPACE) {
			m = -1 + randInt(3);

			upper = dungeon.getDungeon()[(my + m)*maxcols + mx].upper;
			wall = dungeon.getDungeon()[(my + m)*maxcols + mx].wall;
			enemy = dungeon.getDungeon()[(my + m)*maxcols + mx].enemy;
			trap = dungeon.getDungeon()[(my + m)*maxcols + mx].trap;
		}

		dungeon.getDungeon()[(my + m)*maxcols + mx].enemy = true;
		dungeon.getDungeon()[my*maxcols + mx].top = SPACE;
		dungeon.getDungeon()[my*maxcols + mx].enemy = false;

		dungeon.queueMoveSprite(getSprite(), m == -1 ? 'u' : 'd');
		setPosY(my + m);
	}
}
bool Monster::wallCollision(Dungeon &dungeon, char direction, int p_move, int m_move) {
	int maxcols = dungeon.getCols();

	// if they're one space apart, return false
	if (p_move - m_move == 1)
		return false;

	// p_move and m_move are the x/y position of the player and monster, respectively
	// direction: the axis for the enemy to check for walls along
	// wallCollision() is only called when an enemy has a potential line of sight on the player

	// Since player and monster are on the same row/column, set otherdir to the complement of p_move/m_move
	int otherdir;

	if (direction == 'x') {
		otherdir = dungeon.getPlayerVector().at(0).getPosY();
	}
	else {
		otherdir = dungeon.getPlayerVector().at(0).getPosX();
	}

	char top;
	bool wall;

	// if player is below or to the right of the enemy, let monster check moving in the direction opposite (otherdir)
	if (p_move > m_move) {
		p_move--;
		while (p_move - m_move != 0) {
			if (direction == 'x') {

				wall = dungeon[otherdir*maxcols + p_move].wall;
				if (wall)
					return true;
				else
					p_move--;
			}
			else if (direction == 'y') {

				wall = dungeon[(p_move)*maxcols + otherdir].wall;
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

				wall = dungeon[otherdir*maxcols + m_move].wall;
				if (wall)
					return true;
				else
					m_move--;
			}
			else if (direction == 'y') {

				wall = dungeon[(m_move)*maxcols + otherdir].wall;
				if (wall)
					return true;
				else
					m_move--;
			}
		}
	}
	return false;
}
bool Monster::clearLineOfSight(Dungeon &dungeon, Player &p) {
	int cols = dungeon.getCols();
	int px = p.getPosX();
	int py = p.getPosY();

	int x = getPosX();
	int y = getPosY();

	bool enemy, wall, hero = false;

	int n, m;
	char dir;

	dir = (px > x && py == y ? 'r' : px < x && py == y ? 'l' : py > y && px == x ? 'd' : 'u');

	switch (dir) {
	case 'l': {
		x--;
		wall = dungeon.getDungeon()[y*cols + x].wall;
		enemy = dungeon.getDungeon()[y*cols + x].enemy;
		hero = dungeon.getDungeon()[y*cols + x].hero;
		while (!(wall || enemy) && px != x) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				return true;
			}

			x--;
			wall = dungeon.getDungeon()[y*cols + x].wall;
			enemy = dungeon.getDungeon()[y*cols + x].enemy;
			hero = dungeon.getDungeon()[y*cols + x].hero;
		}
		break;
	}
	case 'r': {
		x++;
		wall = dungeon.getDungeon()[y*cols + x].wall;
		enemy = dungeon.getDungeon()[y*cols + x].enemy;
		hero = dungeon.getDungeon()[y*cols + x].hero;
		while (!(wall || enemy) && px != x) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				return true;
			}

			x++;
			wall = dungeon.getDungeon()[y*cols + x].wall;
			enemy = dungeon.getDungeon()[y*cols + x].enemy;
			hero = dungeon.getDungeon()[y*cols + x].hero;
		}
		break;
	}
	case 'u': {
		y--;
		wall = dungeon.getDungeon()[y*cols + x].wall;
		enemy = dungeon.getDungeon()[y*cols + x].enemy;
		hero = dungeon.getDungeon()[y*cols + x].hero;
		while (!(wall || enemy) && py != y) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				return true;
			}

			y--;
			wall = dungeon.getDungeon()[y*cols + x].wall;
			enemy = dungeon.getDungeon()[y*cols + x].enemy;
			hero = dungeon.getDungeon()[y*cols + x].hero;
		}
		break;
	}
	case 'd': {
		y++;
		wall = dungeon.getDungeon()[y*cols + x].wall;
		enemy = dungeon.getDungeon()[y*cols + x].enemy;
		hero = dungeon.getDungeon()[y*cols + x].hero;
		while (!(wall || enemy) && py != y) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				return true;
			}

			y++;
			wall = dungeon.getDungeon()[y*cols + x].wall;
			enemy = dungeon.getDungeon()[y*cols + x].enemy;
			hero = dungeon.getDungeon()[y*cols + x].hero;
		}
		break;
	}
	}
	
	if (hero)
		return true;

	return false;
}

bool Monster::chasesPlayer() const {
	return m_chases;
}
void Monster::setChasesPlayer(bool chases) {
	m_chases = chases;
}
bool Monster::isSmart() const {
	return m_smart;
}
void Monster::setSmart(bool smart) {
	m_smart = smart;
}

bool Monster::hasExtraSprites() {
	return m_hasExtraSprites;
}
void Monster::setExtraSpritesFlag(bool extras) {
	m_hasExtraSprites = extras;
}
bool Monster::emitsLight() const {
	return m_emitsLight;
}
void Monster::setEmitsLight(bool emits) {
	m_emitsLight = emits;
}

//							(	...	bool burnable, bool bleedable, bool healable, bool stunnable, bool freezable)

//		GOBLIN FUNCTIONS
Goblin::Goblin(int x, int y, int smelldist) : Monster(x, y, randInt(5) + 10, 1, 3, 1, std::make_shared<ShortSword>(), GOBLIN), m_smelldist(smelldist) {
	setChasesPlayer(true);
	setImageName("Goblin_48x48.png");
}

void Goblin::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	char first, best;
	first = best = '0';
	int shortest = 0;

	if (playerInRange(dungeon.getPlayer(), getSmellDistance())) {
		// if viable path is found
		if (attemptChase(dungeon.getDungeon(), cols, shortest, getSmellDistance(), getSmellDistance(), getPosX(), getPosY(), first, best)) {
			moveWithSuccessfulChase(dungeon, best);
		}
	}
}
void Goblin::death(Dungeon &dungeon) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon.getDungeon()[y*maxcols + x].item;

	// these are up here because of itemHash() changing the x and y values
	dungeon.getDungeon()[y*maxcols + x].top = SPACE;
	dungeon.getDungeon()[y*maxcols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	cocos2d::experimental::AudioEngine::play2d("Coin_Drop3.mp3", false, 1.0f);
	dungeon.getDungeon()[y*maxcols + x].gold += 3 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);

	// 15% drop chance
	if (1 + randInt(100) + dungeon.getPlayer().getLuck() > 85) {
		if (item) {
			dungeon.itemHash(dungeon.getDungeon(), x, y);
		}
		dungeon.getDungeon()[y*maxcols + x].object = std::make_shared<BoneAxe>(x, y);
		dungeon.getDungeon()[y*maxcols + x].item_name = BONE_AXE;
		dungeon.getDungeon()[y*maxcols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, maxrows, x, y, -1, dungeon.getDungeon()[y*maxcols + x].object->getImageName());
	}
}

int Goblin::getSmellDistance() const {
	return m_smelldist;
}


//		WANDERER FUNCTIONS
Wanderer::Wanderer(int x, int y) : Monster(x, y, 10, 1, 3, 1, std::make_shared<ShortSword>(), WANDERER) {
	setFlying(true);
	setImageName("Wanderer_48x48.png");
}

void Wanderer::move(Dungeon &dungeon) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int mx = getPosX();
	int my = getPosY();

	int n = -1 + randInt(3);
	int m = -1 + randInt(3);
	bool wall, enemy, hero;


	wall = dungeon.getDungeon()[(my + m)*maxcols + mx + n].wall;
	enemy = dungeon.getDungeon()[(my + m)*maxcols + mx + n].enemy;
	hero = dungeon.getDungeon()[(my + m)*maxcols + mx + n].hero;

	// if the randomly selected move is not a wall or enemy, move them
	if (!(wall || enemy)) {

		// if it's not the player, then move them to that space
		if (!hero) {
			dungeon.getDungeon()[(my + m)*maxcols + mx + n].top = c_WANDERER;
			dungeon.getDungeon()[(my + m)*maxcols + mx + n].enemy = true;
			dungeon.getDungeon()[my*maxcols + mx].top = SPACE;
			dungeon.getDungeon()[my*maxcols + mx].enemy = false;

			// Key: (for moveSprite 'move' parameter)
			// 1 u 2
			// l _ r
			// 3 d 4

			// n is x, m is y
			if (n == 0 && m != 0) {
				//moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? 'u' : 'd');
				dungeon.queueMoveSprite(getSprite(), m == -1 ? 'u' : 'd');
			}
			else if (n != 0 && m == 0) {
				//moveSprite(monster_sprites, maxrows, mx, my, n == -1 ? 'l' : 'r');
				dungeon.queueMoveSprite(getSprite(), n == -1 ? 'l' : 'r');
			}
			// wanderer moved diagonally
			else if (n != 0 && m != 0) {
				// moved left and ?
				if (n == -1) {
					//moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? '1' : '3');
					dungeon.queueMoveSprite(getSprite(), m == -1 ? '1' : '3');
				}
				// moved right and ?
				else if (n == 1) {
					//moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? '2' : '4');
					dungeon.queueMoveSprite(getSprite(), m == -1 ? '2' : '4');
				}
			}

			setPosX(mx + n);
			setPosY(my + m);
		}
		// otherwise attack the player
		else {
			encounter(dungeon.getPlayerVector().at(0), *this, dungeon.dungeonText);
		}
	}
}
void Wanderer::death(Dungeon &dungeon) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon.getDungeon()[y*maxcols + x].item;

	// these are up here because of itemHash() changing the x and y values
	dungeon.getDungeon()[y*maxcols + x].top = SPACE;
	dungeon.getDungeon()[y*maxcols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	cocos2d::experimental::AudioEngine::play2d("Coin_Drop1.mp3", false, 1.0f);
	dungeon.getDungeon()[y*maxcols + x].gold += 1 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);

	// 80% drop chance
	if (1 + randInt(100) + dungeon.getPlayer().getLuck() > 20) {
		if (item) {
			dungeon.itemHash(dungeon.getDungeon(), x, y);
		}
		dungeon.getDungeon()[y*maxcols + x].object = std::make_shared<HeartPod>(x, y);
		dungeon.getDungeon()[y*maxcols + x].item_name = HEART_POD;
		dungeon.getDungeon()[y*maxcols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, maxrows, x, y, -1, dungeon.getDungeon()[y*maxcols + x].object->getImageName());
	}
}


//		ARCHER FUNCTIONS
Archer::Archer(int x, int y) : Monster(x, y, randInt(5) + 9, 2, 3, 2, std::make_shared<WoodBow>(), ARCHER), m_primed(false) {
	setImageName("Archer_48x48.png");
}

void Archer::move(Dungeon &dungeon) {
	int maxcols = dungeon.getCols();

	int x = dungeon.getPlayer().getPosX();
	int y = dungeon.getPlayer().getPosY();

	int mx = getPosX();
	int my = getPosY();

	//	if archer is primed, check to attack and return
	if (isPrimed()) {
		// if player is in sight, shoot them
		if (x - mx == 0 || y - my == 0) {
			cocos2d::experimental::AudioEngine::play2d("Bow_Release.mp3", false, 0.8f);

			encounter(dungeon, dungeon.getPlayerVector().at(0), *this);
		}
		// otherwise relax their aim
		else {
			;
			//dungeonText.push_back("The archer eases their aim.\n");
		}

		prime(false);

		return;
	}


	// if archer has dagger out and is adjacent to player, attack player
	if (getWeapon()->getAction() == BRONZE_DAGGER && \
		(dungeon.getDungeon()[(my - 1)*maxcols + mx].hero || dungeon.getDungeon()[(my + 1)*maxcols + mx].hero || dungeon.getDungeon()[my*maxcols + mx - 1].hero || dungeon.getDungeon()[my*maxcols + mx + 1].hero)) {
		// player archer dagger swipe sound effect
		playArcherDaggerSwipe();

		encounter(dungeon, dungeon.getPlayerVector().at(0), *this);
		return;
	}
	// else if dagger is out and player moved away, switch back to bow
	else if (getWeapon()->getAction() == BRONZE_DAGGER && \
		(!dungeon.getDungeon()[(my - 1)*maxcols + mx].hero && !dungeon.getDungeon()[(my + 1)*maxcols + mx].hero && !dungeon.getDungeon()[my*maxcols + mx - 1].hero && !dungeon.getDungeon()[my*maxcols + mx + 1].hero)) {
		cocos2d::experimental::AudioEngine::play2d("Bow_Primed.mp3", false, 0.8f);

		setWeapon(std::make_shared<WoodBow>());
		//dungeonText.push_back("The archer switches back to their Wood Bow.\n");
		return;
	}

	// if player is invisible, move randomly
	if (dungeon.getPlayer().isInvisible()) {
		moveMonsterRandomly(dungeon);
		return;
	}

	// if player is not in the archer's sights, move archer
	if (x - mx != 0 && y - my != 0) {
		moveMonsterRandomly(dungeon);

		return;
	}

	// if player and archer are on the same column
	if (x - mx == 0) {
		//	if player is within range and not adjacent, get primed
		if (abs(y - my) > 1 && abs(y - my) <= 15) {
			if (clearLineOfSight(dungeon, dungeon.getPlayerVector()[0])) { //(!wallCollision(dungeon, 'y', y, my)) {
				cocos2d::experimental::AudioEngine::play2d("Bow_Drawn.mp3", false, 0.8f);

				prime(true);

				//dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveMonsterRandomly(dungeon);

				return;
			}
		}
		// swap weapon to dagger
		else if (abs(y - my) == 1) {
			cocos2d::experimental::AudioEngine::play2d("Archer_Dagger_Switch2.mp3", false, 0.8f);

			setWeapon(std::make_shared<BronzeDagger>());
			//dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
		}
		//	just move archer
		else {
			moveMonsterRandomly(dungeon);

			return;
		}
	}
	// if player and archer are on the same row
	else if (y - my == 0) {
		//	if player is within range and not adjacent, get primed
		if (abs(x - mx) > 1 && abs(x - mx) <= 15) {
			if (clearLineOfSight(dungeon, dungeon.getPlayerVector()[0])) { //(!wallCollision(dungeon, 'x', x, mx)) {
				cocos2d::experimental::AudioEngine::play2d("Bow_Drawn.mp3", false, 0.8f);

				prime(true);

				//dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveMonsterRandomly(dungeon);

				return;
			}
		}
		// swap weapon to dagger
		else if (abs(x - mx) == 1) {
			cocos2d::experimental::AudioEngine::play2d("Archer_Dagger_Switch2.mp3", false, 0.8f);

			setWeapon(std::make_shared<BronzeDagger>());
			//dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
		}
		// just move archer
		else {
			moveMonsterRandomly(dungeon);
		}
	}

}
void Archer::death(Dungeon &dungeon) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon.getDungeon()[y*maxcols + x].item;

	// these are up here because of itemHash() changing the x and y values
	dungeon.getDungeon()[y*maxcols + x].top = SPACE;
	dungeon.getDungeon()[y*maxcols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	cocos2d::experimental::AudioEngine::play2d("Coin_Drop3.mp3", false, 1.0f);
	dungeon.getDungeon()[y*maxcols + x].gold += 3 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);

	// 15% drop chance
	if (1 + randInt(100) + dungeon.getPlayer().getLuck() > 85) {
		if (item) {
			dungeon.itemHash(dungeon.getDungeon(), x, y);
		}
		dungeon.getDungeon()[y*maxcols + x].object = std::make_shared<BronzeDagger>();
		dungeon.getDungeon()[y*maxcols + x].item_name = BRONZE_DAGGER;
		dungeon.getDungeon()[y*maxcols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, maxrows, x, y, -1, dungeon.getDungeon()[y*maxcols + x].object->getImageName());
	}
}

bool Archer::isPrimed() const {
	return m_primed;
}
void Archer::prime(bool p) {
	m_primed = p;
}


//		ZAPPER
Zapper::Zapper(int x, int y, int rows) : Monster(x, y, 10 + randInt(3), 1, 4, 1, ZAPPER), m_cooldown(true), m_attack(randInt(2)) {
	for (int i = 0; i < 8; i++) {
		cocos2d::Sprite* spark = cocos2d::Sprite::createWithSpriteFrameName("Spark_48x48.png");
		sparks.insert(std::pair<int, cocos2d::Sprite*>(i, spark));
	}

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

	setImageName("Zapper_48x48.png");
	setExtraSpritesFlag(true);
	setEmitsLight(true);
}
Zapper::~Zapper() {
	if (!sparks.empty()) {
		for (int i = 0; i < 8; i++) {
			getSparks()[i]->removeFromParent();
		}
	}
}

void Zapper::move(Dungeon &dungeon) {
	int maxcols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	int mx = getPosX();
	int my = getPosY();


	//	if zapper is on cooldown, reset cooldown and switch attack direction
	if (onCooldown()) {
		if (cardinalAttack()) {
			//dungeon.getDungeon()[my*maxcols + mx + 1].projectile = dungeon.getDungeon()[my*maxcols + mx - 1].projectile = dungeon.getDungeon()[(my - 1)*maxcols + mx].projectile = dungeon.getDungeon()[(my + 1)*maxcols + mx].projectile = SPACE;
			// hide cardinal sparks
			for (int i = 0; i < 4; i++) {
				getSparks()[i]->setVisible(false);
			}
		}
		else {
			//dungeon.getDungeon()[(my - 1)*maxcols + mx - 1].projectile = dungeon.getDungeon()[(my - 1)*maxcols + mx + 1].projectile = dungeon.getDungeon()[(my + 1)*maxcols + mx - 1].projectile = dungeon.getDungeon()[(my + 1)*maxcols + mx + 1].projectile = SPACE;
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
			//dungeon.getDungeon()[my*maxcols + mx + 1].projectile = dungeon.getDungeon()[my*maxcols + mx - 1].projectile = dungeon.getDungeon()[(my - 1)*maxcols + mx].projectile = dungeon.getDungeon()[(my + 1)*maxcols + mx].projectile = SPARK;

			// show cardinal sparks
			for (int i = 0; i < 4; i++) {
				getSparks()[i]->setVisible(true);
			}

			if ((px == mx + 1 && py == my) || (px == mx - 1 && py == my) || (px == mx && py == my + 1) || (px == mx && py == my - 1)) {
				encounter(dungeon.getPlayerVector()[0], *this, dungeon.dungeonText);
			}
		}
		else {
			//dungeon.getDungeon()[(my - 1)*maxcols + mx - 1].projectile = dungeon.getDungeon()[(my - 1)*maxcols + mx + 1].projectile = dungeon.getDungeon()[(my + 1)*maxcols + mx - 1].projectile = dungeon.getDungeon()[(my + 1)*maxcols + mx + 1].projectile = SPARK;

			// show diagonal sparks
			for (int i = 4; i < 8; i++) {
				getSparks()[i]->setVisible(true);
			}

			if ((px == mx - 1 && py == my - 1) || (px == mx + 1 && py == my - 1) || (px == mx - 1 && py == my + 1) || (px == mx + 1 && py == my + 1)) {
				encounter(dungeon.getPlayerVector()[0], *this, dungeon.dungeonText);
			}
		}
		setCooldown();
	}
}
void Zapper::attack(Player &p, std::vector<std::string> &text) {
	cocos2d::experimental::AudioEngine::play2d("Shock5.mp3", false, 1.0f);

	//text.push_back("The " + this->getName() + " shocks you... ");

	int damage = 1 + randInt(getStr());
	p.setHP(p.getHP() - damage);
	//text.push_back("and does " + to_string(damage) + " damage!");
	//text.push_back((damage >= 3 ? " Ouch!\n" : "\n"));
}
void Zapper::death(Dungeon &dungeon) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	dungeon.getDungeon()[y*maxcols + x].top = SPACE;
	dungeon.getDungeon()[y*maxcols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	for (int i = y - 1; i < y + 2; i++) {
		for (int j = x - 1; j < x + 2; j++) {
			dungeon.getDungeon()[i*maxcols + j].projectile = SPACE;
			//dungeon.removeAssociatedSprite(dungeon.zapper_sprites, maxrows, x, y, j, i);
		}
	}
	cocos2d::experimental::AudioEngine::play2d("Coin_Drop2.mp3", false, 1.0f);
	dungeon.getDungeon()[y*maxcols + x].gold += 2 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);
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


//		SPINNER
Spinner::Spinner(int x, int y, int rows) : Monster(x, y, 10 + randInt(3), 1, 3, 1, SPINNER, false) {
	setCanBeBurned(false);

	bool dir = randInt(2);

	m_clockwise = dir;
	m_angle = 1 + randInt(8);

	inner = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	outer = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");

	setInitialFirePosition(x, y, rows);

	setImageName("Spinner_48x48.png");
	setExtraSpritesFlag(true);
	setEmitsLight(true);
}
Spinner::~Spinner() {
	if (getInner() != nullptr)
		getInner()->removeFromParent();

	if (getOuter() != nullptr)
		getOuter()->removeFromParent();
}

void Spinner::move(Dungeon &dungeon) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	int mx = getPosX();
	int my = getPosY();

	// reset projectiles to visible
	getInner()->setVisible(true);
	getOuter()->setVisible(true);

	// 8 1 2 
	// 7 X 3
	// 6 5 4
	// moves spinner -to- position getAngle(), it is not where it currently is

	if (isClockwise()) {
		switch (getAngle()) {
		case 1:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 1, my - 1, 'r');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 2, my - 2, 'R');
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');
			setFirePosition('r');

			if (my == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 2:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx, my - 1, 'r');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx, my - 2, 'R');
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');
			setFirePosition('r');

			if (my == 1 || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 3:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 1, my - 1, 'd');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 2, my - 2, 'D');
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');
			setFirePosition('d');

			// if spinner is on the left or right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 4:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 1, my, 'd');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 2, my, 'D');
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');
			setFirePosition('d');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 5:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 1, my + 1, 'l');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 2, my + 2, 'L');
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');
			setFirePosition('l');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 6:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx, my + 1, 'l');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx, my + 2, 'L');
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');
			setFirePosition('l');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (my == maxrows - 2 || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 7:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 1, my + 1, 'u');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 2, my + 2, 'U');
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');
			setFirePosition('u');

			// if spinner is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 8:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 1, my, 'u');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 2, my, 'U');
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');
			setFirePosition('u');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
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
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 1, my - 1, 'l');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 2, my - 2, 'L');
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');
			setFirePosition('l');

			if (my == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 2:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 1, my, 'u');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 2, my, 'U');
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');
			setFirePosition('u');

			// if spinner is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 3:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 1, my + 1, 'u');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx + 2, my + 2, 'U');
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');
			setFirePosition('u');

			// if spinner is on the right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 4:

			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx, my + 1, 'r');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx, my + 2, 'R');
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');
			setFirePosition('r');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 5:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 1, my + 1, 'r');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 2, my + 2, 'R');
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');
			setFirePosition('r');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 6:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 1, my, 'd');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 2, my, 'D');
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');
			setFirePosition('d');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == maxrows - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 7:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 1, my - 1, 'd');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx - 2, my - 2, 'D');
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');
			setFirePosition('d');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 8:
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx, my - 1, 'l');
			//dungeon.moveAssociatedSprite(dungeon.spinner_sprites, maxrows, mx, my, mx, my - 2, 'L');
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');
			setFirePosition('l');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
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

	if (playerWasHit(dungeon.getPlayer())) {
		encounter(dungeon.getPlayerVector()[0], *this, dungeon.dungeonText);
	}
}
void Spinner::attack(Player &p, std::vector<std::string> &text) {
	cocos2d::experimental::AudioEngine::play2d("Fire3.mp3", false, 1.0f);

	//text.push_back("The " + this->getName() + " hits you for 3 damage.\n");
	p.setHP(p.getHP() - getStr());

	// if actor is still alive and can be burned, try to burn them
	if (p.getHP() > 0 && p.canBeBurned()) {

		// chance to burn
		int roll = randInt(100) + p.getLuck();

		// failed the save roll
		if (roll < 60) {
			//p.setBurned(true);
			p.addAffliction(std::make_shared<Burn>());
		}
	}
}
void Spinner::death(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	dungeon.getDungeon()[y*cols + x].top = SPACE;
	dungeon.getDungeon()[y*cols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	/*for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 2; j < x + 3; j++) {
			dungeon.removeAssociatedSprite(dungeon.spinner_sprites, rows, x, y, j, i);
		}
	}*/
	cocos2d::experimental::AudioEngine::play2d("Coin_Drop2.mp3", false, 1.0f);
	dungeon.getDungeon()[y*cols + x].gold += 2 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);
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
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y - 2);
			break;
		case 2:
			m_innerFire.setPosX(x); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x); m_outerFire.setPosY(y - 2);
			break;
		case 3:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y - 2);
			break;
		case 4:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y);
			break;
		case 5:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y + 2);
			break;
		case 6:
			m_innerFire.setPosX(x); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x); m_outerFire.setPosY(y + 2);
			break;
		case 7:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y + 2);
			break;
		case 8:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y);
			break;
		}
	}
	else {
		switch (m_angle) {
		case 1:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y - 2);
			break;
		case 2:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y);
			break;
		case 3:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y + 2);
			break;
		case 4:
			m_innerFire.setPosX(x); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x); m_outerFire.setPosY(y + 2);
			break;
		case 5:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y + 2);
			break;
		case 6:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y);
			break;
		case 7:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y - 2);
			break;
		case 8:
			m_innerFire.setPosX(x); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x); m_outerFire.setPosY(y - 2);
			break;
		}
	}


	// set sprite positions
	if (isClockwise()) {
		switch (getAngle()) {
		case 1:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 2:
			getInner()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 3:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 4:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			break;
		case 5:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 6:
			getInner()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 7:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 8:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			break;
		}
	}
	else {
		switch (getAngle()) {
		case 1:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 2:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			break;
		case 3:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 4:
			getInner()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 5:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 6:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 7:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 8:
			getInner()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		}
	}
	
	setSpriteVisibility(false);
}
void Spinner::setFirePosition(char move) {
	switch (move) {
	case 'l':
		m_innerFire.setPosX(m_innerFire.getPosX() - 1);
		m_outerFire.setPosX(m_outerFire.getPosX() - 2);
		break;
	case 'r':
		m_innerFire.setPosX(m_innerFire.getPosX() + 1);
		m_outerFire.setPosX(m_outerFire.getPosX() + 2);
		break;
	case 'u':
		m_innerFire.setPosY(m_innerFire.getPosY() - 1);
		m_outerFire.setPosY(m_outerFire.getPosY() - 2);
		break;
	case 'd':
		m_innerFire.setPosY(m_innerFire.getPosY() + 1);
		m_outerFire.setPosY(m_outerFire.getPosY() + 2);
		break;
	}
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
	if ((m_innerFire.getPosX() == a.getPosX() && m_innerFire.getPosY() == a.getPosY()) ||
		(m_outerFire.getPosX() == a.getPosX() && m_outerFire.getPosY() == a.getPosY()))
		return true;

	return false;
}

cocos2d::Sprite* Spinner::getInner() const {
	return inner;
}
cocos2d::Sprite* Spinner::getOuter() const {
	return outer;
}
void Spinner::setSpriteVisibility(bool visible) {
	getInner()->setVisible(visible);
	getOuter()->setVisible(visible);
}
void Spinner::setSpriteColor(cocos2d::Color3B color) {
	getInner()->setColor(color);
	getOuter()->setColor(color);
}


//		BOMBEE
Bombee::Bombee(int x, int y) : Monster(x, y, 1, 0, 1, 1, BOMBEE), m_fuse(3), m_fused(false), m_aggroRange(11) {
	setChasesPlayer(true);
	setImageName("Bombee_48x48.png");
}

void Bombee::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	char first, best;
	first = best = '0';
	int shortest = 0;

	if (playerInRange(dungeon.getPlayer(), getRange())) {
		// if viable path is found
		if (attemptChase(dungeon.getDungeon(), cols, shortest, getRange(), getRange(), getPosX(), getPosY(), first, best)) {
			moveWithSuccessfulChase(dungeon, best);
		}
	}
}
void Bombee::attack(Dungeon &dungeon, Player &p) {
	playGotHit();

	p.setHP(p.getHP() - 1);
}
void Bombee::death(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int rows = dungeon.getRows();

	int x = getPosX();
	int y = getPosY();

	dungeon.getDungeon()[y*cols + x].top = SPACE;
	dungeon.getDungeon()[y*cols + x].enemy = false;
	dungeon.getDungeon()[y*cols + x].extra = c_BOMBEE_ACTIVE;
	dungeon.queueRemoveSprite(getSprite());

	/*std::shared_ptr<MegaBomb> megabomb = std::make_shared<MegaBomb>();
	megabomb->lightBomb();
	megabomb->setPosX(getPosX());
	megabomb->setPosY(getPosY());*/

	std::shared_ptr<Traps> megabomb = std::make_shared<ActiveMegaBomb>(x, y);
	dungeon.getTraps().push_back(megabomb);
	megabomb->setSprite(dungeon.createSprite(x, y, -1, megabomb->getImageName()));
}

int Bombee::getFuse() const {
	return m_fuse;
}
void Bombee::setFuse() {
	m_fuse--;
}
bool Bombee::isFused() const {
	return m_fused;
}
int Bombee::getRange() const {
	return m_aggroRange;
}


//		MOUNTED KNIGHT
MountedKnight::MountedKnight(int x, int y) : Monster(x, y, 12 + randInt(5), 3, 2, 2, std::make_shared<IronLance>(), MOUNTED_KNIGHT), m_alerted(false) {
	setImageName("Knight_Level1_48x48.png");
}

void MountedKnight::move(Dungeon &dungeon) {
	int maxrows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	int x = getPosX();
	int y = getPosY();

	bool wall, enemy, hero;
	int n, m;

	// flags
	/*bool leftwall, rightwall, upwall, downwall;
	leftwall = dungeon.getDungeon()[my*maxcols + mx - 1].wall;
	rightwall = dungeon.getDungeon()[my*maxcols + mx + 1].wall;
	upwall = dungeon.getDungeon()[(my - 1)*maxcols + mx].wall;
	downwall = dungeon.getDungeon()[(my + 1)*maxcols + mx].wall;

	bool leftenemy, rightenemy, upenemy, downenemy;
	leftenemy = dungeon.getDungeon()[my*maxcols + mx - 1].enemy;
	rightenemy = dungeon.getDungeon()[my*maxcols + mx + 1].enemy;
	upenemy = dungeon.getDungeon()[(my - 1)*maxcols + mx].enemy;
	downenemy = dungeon.getDungeon()[(my + 1)*maxcols + mx].enemy;

	bool lefthero, righthero, uphero, downhero;
	lefthero = dungeon.getDungeon()[my*maxcols + mx - 1].hero;
	righthero = dungeon.getDungeon()[my*maxcols + mx + 1].hero;
	uphero = dungeon.getDungeon()[(my - 1)*maxcols + mx].hero;
	downhero = dungeon.getDungeon()[(my + 1)*maxcols + mx].hero;*/

	// if player is invisible, just move them randomly
	if (dungeon.getPlayer().isInvisible()) {
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
			encounter(dungeon, dungeon.getPlayerVector().at(0), *this);
		}
		else if (wall || enemy) {
			toggleAlert();

			return;
		}
		else {
			dungeon.getDungeon()[y*cols + x].enemy = false;
			dungeon.getDungeon()[(y + m)*cols + (x + n)].enemy = true;

			dungeon.queueMoveSprite(getSprite(), getDirection());
			setPosX(x + n); setPosY(y + m);
		}


		/*
		char direction = getDirection();
		if (direction == 'l') {
			if (lefthero) {
				encounter(dungeon, dungeon.getPlayerVector().at(0), *this);
			}
			else if (leftwall || leftenemy) {
				toggleAlert();

				return;
			}
			else {
				dungeon.getDungeon()[my*maxcols + mx].top = SPACE;
				dungeon.getDungeon()[my*maxcols + mx].enemy = false;
				dungeon.getDungeon()[my*maxcols + mx - 1].top = c_MOUNTED_KNIGHT;
				dungeon.getDungeon()[my*maxcols + mx - 1].enemy = true;

				dungeon.queueMoveSprite(getSprite(), direction);
				setPosX(mx - 1);
			}
		}
		else if (direction == 'r') {
			if (righthero) {
				encounter(dungeon, dungeon.getPlayerVector().at(0), *this);
			}
			else if (rightwall || rightenemy) {
				toggleAlert();

				return;
			}
			else {
				dungeon.getDungeon()[my*maxcols + mx].top = SPACE;
				dungeon.getDungeon()[my*maxcols + mx].enemy = false;
				dungeon.getDungeon()[my*maxcols + mx + 1].top = c_MOUNTED_KNIGHT;
				dungeon.getDungeon()[my*maxcols + mx + 1].enemy = true;

				dungeon.queueMoveSprite(getSprite(), direction);
				setPosX(mx + 1);
			}
		}
		else if (direction == 'u') {
			if (uphero) {
				//encounter(dungeon.getPlayerVector().at(0), *this, dungeon.dungeonText);
				encounter(dungeon, dungeon.getPlayerVector().at(0), *this);
			}
			else if (upwall || upenemy) {
				toggleAlert();

				return;
			}
			else {
				dungeon.getDungeon()[my*maxcols + mx].top = SPACE;
				dungeon.getDungeon()[my*maxcols + mx].enemy = false;
				dungeon.getDungeon()[(my - 1)*maxcols + mx].top = c_MOUNTED_KNIGHT;
				dungeon.getDungeon()[(my - 1)*maxcols + mx].enemy = true;

				dungeon.queueMoveSprite(getSprite(), direction);
				setPosY(my - 1);
			}
		}
		else if (direction == 'd') {
			if (downhero) {
				//encounter(dungeon.getPlayerVector().at(0), *this, dungeon.dungeonText);
				encounter(dungeon, dungeon.getPlayerVector().at(0), *this);
			}
			else if (downwall || downenemy) {
				toggleAlert();

				return;
			}
			else {
				dungeon.getDungeon()[my*maxcols + mx].top = SPACE;
				dungeon.getDungeon()[my*maxcols + mx].enemy = false;
				dungeon.getDungeon()[(my + 1)*maxcols + mx].top = c_MOUNTED_KNIGHT;
				dungeon.getDungeon()[(my + 1)*maxcols + mx].enemy = true;

				dungeon.queueMoveSprite(getSprite(), direction);
				setPosY(my + 1);
			}
		}
		*/
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

			//dungeonText.push_back("The " + monsters.at(pos)->getName() + " charges toward you!\n");
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

			//dungeonText.push_back("The " + monsters.at(pos)->getName() + " charges toward you!\n");
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
void MountedKnight::death(Dungeon &dungeon) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	bool item = dungeon.getDungeon()[y*maxcols + x].item;

	// these are up here because of itemHash() changing the x and y values
	dungeon.getDungeon()[y*maxcols + x].top = SPACE;
	dungeon.getDungeon()[y*maxcols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());


	cocos2d::experimental::AudioEngine::play2d("Coin_Drop5.mp3", false, 1.0f);
	dungeon.getDungeon()[y*maxcols + x].gold += 5 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);

	// 10% drop chance
	if (1 + randInt(100) + dungeon.getPlayer().getLuck() > 90) {
		if (item) {
			dungeon.itemHash(dungeon.getDungeon(), x, y);
		}
		dungeon.getDungeon()[y*maxcols + x].object = std::make_shared<IronLance>();
		dungeon.getDungeon()[y*maxcols + x].item_name = IRON_LANCE;
		dungeon.getDungeon()[y*maxcols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, maxrows, x, y, -1, dungeon.getDungeon()[y*maxcols + x].object->getImageName());
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

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
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
			isLava = dungeon.getTraps()[pos]->getItem() == LAVA;
		}
	}

	// if trap is lava, but they are immune to lava (fire roundabout), move them
	if (isLava && lavaImmune()) {
		dungeon.getDungeon()[y*cols + x].enemy = false;
		dungeon.getDungeon()[(y + m)*cols + (x + n)].enemy = true;

		dungeon.queueMoveSprite(getSprite(), getDirection());
		setPosX(x + n); setPosY(y + m);

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
			encounter(dungeon.getPlayerVector()[0], *this, dungeon.dungeonText);
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
		dungeon[y*cols + x].enemy = false;
		dungeon[(y + m)*cols + (x + n)].enemy = true;

		dungeon.queueMoveSprite(getSprite(), getDirection());
		setPosX(x + n); setPosY(y + m);
	}

}
void Roundabout::attack(Player &p, std::vector<std::string> &text) {
	int px = p.getPosX();
	int py = p.getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.isBlocking() && p.shieldCoverage(getPosX(), getPosY())) {
		damage = std::max(0, p.getShield()->getDefense() - damage);
		p.blocked(); // calls shield durability change

		/*if (p.getShield()->hasAbility()) {
			p.getPlayerShield()->useAbility(dungeon, *this);
		}*/

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my <= 1)) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	if (damage > 0) {
		playGotHit();

		p.setHP(p.getHP() - damage);
		//text.push_back("The " + this->getName() + " hits you for " + to_string(damage - p.getArmor()) + " damage!\n");
	}
	else {
		;//text.push_back("The " + this->getName() + "bumps into you.\n");
	}
}
void Roundabout::death(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	dungeon[y*cols + x].top = SPACE;
	dungeon[y*cols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	cocos2d::experimental::AudioEngine::play2d("Coin_Drop2.mp3", false, 1.0f);
	dungeon[y*dungeon.getCols() + x].gold += 2 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);
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
}

void FireRoundabout::move(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
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
			isLava = dungeon.getTraps()[pos]->getItem() == LAVA;
		}
	}

	// if trap is lava, but they are immune to lava (fire roundabout), move them
	if (isLava && lavaImmune()) {
		dungeon.getDungeon()[y*cols + x].enemy = false;
		dungeon.getDungeon()[(y + m)*cols + (x + n)].enemy = true;

		dungeon.queueMoveSprite(getSprite(), getDirection());
		setPosX(x + n); setPosY(y + m);

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
			encounter(dungeon.getPlayerVector()[0], *this, dungeon.dungeonText);
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
		dungeon[y*cols + x].enemy = false;
		dungeon[(y + m)*cols + (x + n)].enemy = true;

		dungeon.queueMoveSprite(getSprite(), getDirection());
		setPosX(x + n); setPosY(y + m);
	}
	
}
void FireRoundabout::death(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	dungeon[y*cols + x].top = SPACE;
	dungeon[y*cols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	cocos2d::experimental::AudioEngine::play2d("Coin_Drop3.mp3", false, 1.0f);
	dungeon[y*dungeon.getCols() + x].gold += 3 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);
}


//		SEEKER
Seeker::Seeker(int x, int y, int range) : Monster(x, y, 8 + randInt(3), 0, 3, 1, std::make_shared<ShortSword>(), SEEKER), m_range(range), m_step(false) {
	setChasesPlayer(true);
	setImageName("Seeker_48x48.png");
}
Seeker::Seeker(int x, int y, int range, std::string name, std::string image) : Monster(x, y, 10 + randInt(5), 1, 4, 1, std::make_shared<ShortSword>(), name), m_range(range), m_step(false) {
	setChasesPlayer(true);
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

	if (playerInRange(dungeon.getPlayer(), getRange())) {
		// if viable path is found
		if (attemptChase(dungeon.getDungeon(), cols, shortest, getRange(), getRange(), getPosX(), getPosY(), first, best)) {

			// if the seeker can move, move them
			//if (getStep()) {
				moveWithSuccessfulChase(dungeon, best);
			//}

			toggleStep();
		}
	}
}
void Seeker::death(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	dungeon.getDungeon()[y*cols + x].top = SPACE;
	dungeon.getDungeon()[y*cols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	cocos2d::experimental::AudioEngine::play2d("Coin_Drop1.mp3", false, 1.0f);
	dungeon.getDungeon()[y*dungeon.getCols() + x].gold += 1 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);
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
	
}
void DeadSeeker::death(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	dungeon[y*cols + x].top = SPACE;
	dungeon[y*cols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	cocos2d::experimental::AudioEngine::play2d("Coin_Drop3.mp3", false, 1.0f);
	dungeon[y*cols + x].gold += 3 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);
}


//		ITEM THIEF
ItemThief::ItemThief(int x, int y, int range) : Monster(x, y, 5, 1, 2, 2, ITEM_THIEF), m_range(range) {
	setChasesPlayer(true);
	setSmart(true);
	setSturdy(false);
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
		if (playerInDiagonalRange(dungeon.getPlayer(), getRange())) {
			// if viable path is found
			if (attemptSmartChase(dungeon, cols, shortest, getRange(), getRange(), getPosX(), getPosY(), first, best)) {
				moveWithSuccessfulChase(dungeon, best);
			}
		}

		// play sound effect when they steal
		if (m_stole) {
			cocos2d::experimental::AudioEngine::play2d("Item_Thief_Stole1.mp3", false, 1.0f);
			cocos2d::experimental::AudioEngine::play2d("Female_Hey2.mp3", false, 1.0f);
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

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

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
		if (p.isBlocking() && p.shieldCoverage(getPosX(), getPosY())) {
			damage = std::max(0, p.getShield()->getDefense() - damage);
			p.blocked(); // calls shield durability change
		}

		if (damage > 0) {
			playGotHit();

			p.setHP(p.getHP() - damage);
		}
		else {
			;
		}
	}
}
void ItemThief::death(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	cocos2d::experimental::AudioEngine::play2d("Item_Thief_Death2.mp3", false, 1.0f);

	dungeon[y*cols + x].top = SPACE;
	dungeon[y*cols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	if (m_stolenItem != nullptr) {
		if (dungeon[y*cols + x].item) {
			dungeon.itemHash(dungeon.getDungeon(), x, y);
		}
		dungeon.getDungeon()[y*cols + x].object = m_stolenItem;
		dungeon.getDungeon()[y*cols + x].item_name = m_stolenItem->getItem();
		dungeon.getDungeon()[y*cols + x].item = true;
		dungeon.addSprite(dungeon.item_sprites, dungeon.getRows(), x, y, -1, dungeon.getDungeon()[y*cols + x].object->getImageName());
	}

	else if (m_stolenGold > 0) {
		cocos2d::experimental::AudioEngine::play2d("Coin_Drop5.mp3", false, 1.0f);
		dungeon[y*cols + x].gold += m_stolenGold;
		dungeon.addGoldSprite(dungeon.getDungeon(), x, y);
	}
}

int ItemThief::getRange() const {
	return m_range;
}


//		GOO SACK
GooSack::GooSack(int x, int y) : Monster(x, y, 7 + randInt(5), 0, 3, 1, GOO_SACK) {
	setImageName("Goo_Sack_48x48.png");
}

void GooSack::move(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	int x = getPosX();
	int y = getPosY();

	// if not primed but player is nearby, then get primed
	if (!m_primed && abs(px - x) <= 1 && abs(py - y) <= 1) {
		// sound effect for priming
		cocos2d::experimental::AudioEngine::play2d("Goo_Sack_Primed.mp3", false, 1.0f);

		m_primed = true;
		m_x = px;
		m_y = py;

		return;
	}
	else if (!m_primed && !(abs(px - x) <= 1 && abs(py - y) <= 1)) {
		return;
	}

	// otherwise if player is near the goo sack and it is primed, attack them
	if (m_primed && abs(px - x) <= 1 && abs(py - y) <= 1) {
		attack(dungeon, dungeon.getPlayerVector()[0]);
	}
	else if (!(dungeon[m_y*cols + m_x].wall || dungeon[m_y*cols + m_x].enemy)) {
		dungeon.getDungeon()[y*cols + x].enemy = false;
		dungeon.getDungeon()[m_y*cols + m_x].enemy = true;

		dungeon.queueMoveSprite(getSprite(), m_x, m_y);
		setPosX(m_x); setPosY(m_y);
	}

	m_primed = false;
}
void GooSack::attack(Dungeon &dungeon, Player &p) {
	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.isBlocking() && p.shieldCoverage(getPosX(), getPosY())) {
		damage = std::max(0, p.getShield()->getDefense() - damage);
		p.blocked(); // calls shield durability change

		if (p.getShield()->hasAbility()) {
			p.getPlayerShield()->useAbility(dungeon, *this);
		}

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my <= 1)) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	if (damage > 0) {
		playGotHit();

		p.setHP(p.getHP() - damage);
	}

}
void GooSack::death(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	dungeon.getDungeon()[y*cols + x].top = SPACE;
	dungeon.getDungeon()[y*cols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	cocos2d::experimental::AudioEngine::play2d("Coin_Drop2.mp3", false, 1.0f);
	dungeon.getDungeon()[y*dungeon.getCols() + x].gold += 2 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);
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
}

void Broundabout::move(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
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
		cocos2d::experimental::AudioEngine::play2d("Goo_Sack_Primed.mp3", false, 1.0f);

		m_primed = true;
		m_x = px;
		m_y = py;

		return;
	}

	// otherwise if player is near the goo sack and it is primed, attack them
	if (m_primed && (abs(px - x) == 0 && abs(py - y) <= 1 || abs(px - x) <= 1 && abs(py - y) == 0)) {
		attack(dungeon, dungeon.getPlayerVector()[0]);
	}
	// else if primed, but player moved away, let the broundabout jump towards where the player was
	else if (m_primed && !(abs(px - x) == 0 && abs(py - y) <= 1 || abs(px - x) <= 1 && abs(py - y) == 0)) {
		dungeon[y*cols + x].enemy = false;
		dungeon[m_y*cols + m_x].enemy = true;

		dungeon.queueMoveSprite(getSprite(), m_x, m_y);
		setPosX(m_x); setPosY(m_y);
	}
	// if nothing is in the way, keep moving
	else if (!(wall || enemy || hero)) {
		dungeon[y*cols + x].enemy = false;
		dungeon[(y + m)*cols + (x + n)].enemy = true;

		dungeon.queueMoveSprite(getSprite(), x + n, y + m);
		setPosX(x + n); setPosY(y + m);
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
	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	int mx = getPosX();
	int my = getPosY();

	int damage = std::max(1, randInt(std::max(1, getStr() - p.getArmor())));

	// if player was blocking and blocked successfully
	if (p.isBlocking() && p.shieldCoverage(getPosX(), getPosY())) {
		damage = std::max(0, p.getShield()->getDefense() - damage);
		p.blocked(); // calls shield durability change

		if (p.getShield()->hasAbility()) {
			p.getPlayerShield()->useAbility(dungeon, *this);
		}

		// if monster is next to player, stun them
		if (abs(px - mx) <= 1 && abs(py - my <= 1)) {
			// stun sprite effect
			gotStunned(this->getSprite());

			addAffliction(std::make_shared<Stun>(1));
		}
	}

	if (damage > 0) {
		playGotHit();

		p.setHP(p.getHP() - damage);
	}
}
void Broundabout::death(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	dungeon[y*cols + x].top = SPACE;
	dungeon[y*cols + x].enemy = false;
	dungeon.queueRemoveSprite(getSprite());

	cocos2d::experimental::AudioEngine::play2d("Coin_Drop3.mp3", false, 1.0f);
	dungeon[y*dungeon.getCols() + x].gold += 3 * dungeon.getPlayer().getMoneyMultiplier() + dungeon.getPlayer().getMoneyBonus();
	dungeon.addGoldSprite(dungeon.getDungeon(), x, y);
}



//
//
//		BOSS FIGHT FUNCTIONS

Smasher::Smasher() : Monster(BOSSCOLS / 2, 3, 500, 0, 4, 1, std::make_shared<SmashersFists>(), "Smasher"), m_moveActive(false), m_moveEnding(true), m_frenzy(false) {

}

void Smasher::attack(Player &p, std::vector<std::string> &text) {
	cocos2d::experimental::AudioEngine::play2d("Smasher_HitBy1.mp3", false, 1.0f);

	//string weapon = this->getWeapon()->getAction();
	
	int damage = 10 + randInt(3);
	p.setHP(p.getHP() - damage);

	// stun player after being crushed
	p.addAffliction(std::make_shared<Stun>(2));
	
	//text.push_back(this->getName() + " smashes you for " + to_string(damage) + " damage!\n");
}
void Smasher::attack(Monster &m) {
	cocos2d::experimental::AudioEngine::play2d("Smasher_HitBy1.mp3", false, 1.0f);

	int damage = 100;
	m.setHP(m.getHP() - damage);

	//text.push_back(this->getName() + " smashes you for " + to_string(damage) + " damage!\n");
}
void Smasher::death(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// pause current music, play victory music, and then resume music once finished
	cocos2d::experimental::AudioEngine::pauseAll();
	int id = cocos2d::experimental::AudioEngine::play2d("Victory! All Clear.mp3", false, 1.0f);

	cocos2d::experimental::AudioEngine::setFinishCallback(id,
		[](int id, std::string music) {
		cocos2d::experimental::AudioEngine::resumeAll();
	});

	dungeon.removeSprite(dungeon.monster_sprites, rows, x, y);

	Idol idol;
	idol.setPosX(BOSSCOLS / 2);
	idol.setPosY(3);
	dungeon.getDungeon()[idol.getPosY()*cols + idol.getPosX()].bottom = IDOL;
	dungeon.getDungeon()[idol.getPosY()*cols + idol.getPosX()].item = true;

	// sprite for idol
	dungeon.addSprite(dungeon.item_sprites, rows, idol.getPosX(), idol.getPosY(), -1, "Freeze_Spell_48x48.png");

	for (int i = y - 1; i < y + 2; i++) {
		for (int j = x - 1; j < x + 2; j++) {
			dungeon.getDungeon()[i*cols + j].upper = SPACE;
			dungeon.getDungeon()[i*cols + j].enemy = false;
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



//		ADDITIONAL FUNCTIONS

void playGotHit() {
	std::string sound;
	int n = randInt(6) + 1;
	switch (n) {
	case 1: sound = "Hit_Female1.mp3"; break;
	case 2: sound = "Hit_Female2.mp3"; break;
	case 3: sound = "Hit_Female3.mp3"; break;
	case 4: sound = "Hit_Female4.mp3"; break;
	case 5: sound = "Hit_Female5.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, 0.6f);
	cocos2d::experimental::AudioEngine::play2d("Player_Hit.mp3", false, 1.0f);
}
void playMiss(float volume) {
	int n = randInt(5) + 1;
	std::string sound;
	switch (n) {
	case 1: sound = "Slash1.mp3"; break;
	case 2: sound = "Slash2.mp3"; break;
	case 3: sound = "Slash3.mp3"; break;
	case 4: sound = "Slash4.mp3"; break;
	case 5: sound = "Slash5.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, volume);
}
void playEnemyHit(float volume) {
	cocos2d::experimental::AudioEngine::play2d("Enemy_Hit.mp3", false, volume);
}
void playHitSmasher(float volume) {
	std::string sound;
	int n = randInt(5) + 1;
	switch (n) {
	case 1: sound = "Metal_Hit1.mp3"; break;
	case 2: sound = "Metal_Hit2.mp3"; break;
	case 3: sound = "Metal_Hit3.mp3"; break;
	case 4: sound = "Metal_Hit4.mp3"; break;
	case 5: sound = "Metal_Hit5.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, volume);
}
void playShieldEquip(std::string shield) {
	if (shield == "Wood Shield") {
		cocos2d::experimental::AudioEngine::play2d("Shield_Pickup.mp3", false, 1.0f);
	}
	else if (shield == "Iron Shield") {
		cocos2d::experimental::AudioEngine::play2d("Iron_Shield_Pickup.mp3", false, 1.0f);
	}
}
void playShieldHit(std::string shield) {
	if (shield == "Wood Shield") {
		cocos2d::experimental::AudioEngine::play2d("Wood_Shield_Impact.mp3", false, 1.0f);
	}
	else if (shield == "Iron Shield") {
		cocos2d::experimental::AudioEngine::play2d("Metal_Hit1.mp3", false, 1.0f);
	}
	else {
		cocos2d::experimental::AudioEngine::play2d("Wood_Shield_Impact.mp3", false, 1.0f);
	}
}
void playShieldBroken(std::string shield) {
	if (shield == "Wood Shield") {
		cocos2d::experimental::AudioEngine::play2d("Wood_Shield_Broken.mp3", false, 1.0f);
	}
	else if (shield == "Iron Shield") {
		cocos2d::experimental::AudioEngine::play2d("Iron_Shield_Broken.mp3", false, 1.0f);
	}
}

void gotStunned(cocos2d::Sprite* sprite) {
	auto tintGray = cocos2d::TintTo::create(0, cocos2d::Color3B(100, 100, 100));
	sprite->runAction(tintGray);
	sprite->runAction(cocos2d::Blink::create(0.2, 3));
}

void bloodlustTint(Actors &a) {
	float percent = a.getHP() / (float)a.getMaxHP();

	int redness = 255; // sprites will have their b and g values reduced to make them appear redder

	if (percent > 0.25f && percent <= 0.4f) redness = 190;
	else if (percent > 0.15f && percent <= 0.25f) redness = 140;
	else if (percent > 0.10f && percent <= 0.15f) redness = 100;
	else if (percent > 0.05f && percent <= 0.10f) redness = 75;
	else if (percent > 0.00f && percent <= 0.05f) redness = 50;

	a.getSprite()->setColor(cocos2d::Color3B(255, redness, redness));
}