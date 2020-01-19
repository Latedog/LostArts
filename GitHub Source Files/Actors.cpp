#include "AudioEngine.h"
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
//Weapon Actors::getWeapon() const {
//	return m_wep;
//}
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


//		PLAYER FUNCTIONS
Player::Player() : Actors(randInt(68) + 1, randInt(16) + 1, 400, 2, 2, 2, std::make_shared<ShortSword>(), "Player") {
	m_hasShield = false;
	m_blocking = false;
	m_invsize = 0;
	m_iteminvsize = 0;
	m_maxhp = 100;
	m_winner = false;
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

		// damage the monster or otherwise
		int damage = std::max(1, getStr() + getWeapon()->getDmg() - (2 * a.getArmor()));
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

		// damage the monster or otherwise
		int damage = std::max(1, getStr() + getWeapon()->getDmg() - (1 * a.getArmor()));
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
	if (randInt(playerPoints) >= randInt(monsterPoints)) {
		int damage = std::max(1, 1 + randInt(getStr() + getWeapon()->getDmg()) - a.getArmor());
		
		string monster = a.getName();

		// play hit sound effect
		if (monster == "Smasher") {
			playHitSmasher();
		}
		else {
			playEnemyHit();
		}

		// damage the monster or otherwise
		a.setHP(a.getHP() - damage);

		// :::: Check for affliction/ability of weapon ::::
		if (getWeapon()->hasAbility()) {
			getWeapon()->useAbility(*getWeapon(), dungeon, a);
		}

		// tint monster sprite red and flash
		runMonsterDamage(a.getSprite());
	}
	// attack roll failed
	else {
		playMiss();
	}
}

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

int Player::getMoney() const {
	return m_money;
}
void Player::setMoney(int money) {
	m_money = money;
}
//std::vector<Weapon> Player::getWeaponInv() const {
//	return m_inv;
//}
std::vector<std::shared_ptr<Weapon>>& Player::getWeapons() {
	return m_weapons;
}
//std::vector<Drops> Player::getItemInv() const {
//	return m_iteminv;
//}
std::vector<std::shared_ptr<Drops>>& Player::getItems() {
	return m_items;
}
//int Player::getInventorySize() const {
//	return m_items.size() + m_weapons.size();
//}
//int Player::getItemInvSize() const {
//	return m_iteminvsize;
//}
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
Shield& Player::getShield() {
	return m_shield;
}
Shield Player::getPlayerShield() const {
	return m_shield;
}
void Player::equipShield(Shield shield) {
	// play shield equip sound
	playShieldEquip(getPlayerShield().getItem());

	m_shield = shield;
	m_hasShield = true;
}
void Player::dropShield(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols) {
	int x = this->getPosX();
	int y = this->getPosY();

	/*if (dungeon[y*maxcols + x].item) {
		itemHash(dungeon, maxrows, maxcols, x, y);
	}*/

	// play shield drop sound
	cocos2d::experimental::AudioEngine::play2d("Wood_Shield_Drop.mp3", false, 1.0f);

	getShield().setPosX(x);
	getShield().setPosY(y);

	//char type;
	std::string type;
	std::string shield = getPlayerShield().getItem();
	std::shared_ptr<Objects> oldShield(nullptr);
	if (shield == "Wood Shield") {
		oldShield = std::make_shared<WoodShield>(getPlayerShield());
		type = WOOD_SHIELD;
	}
	else if (shield == "Iron Shield") {
		oldShield = std::make_shared<IronShield>(getPlayerShield());
		type = IRON_SHIELD;
	}

	dungeon[y*maxcols + x].object = oldShield;
	//dungeon[y*maxcols + x].bottom = type;
	dungeon[y*maxcols + x].item_name = type;
	dungeon[y*maxcols + x].item = true;

	m_hasShield = false;
	m_shield = Shield();
}
void Player::shieldBroken() {
	// play shield broken sound effect
	playShieldBroken(getPlayerShield().getItem());

	m_hasShield = false;
	m_shield = Shield();
}
void Player::repairShield(int repairs) {
	// if repairs do not exceed the shield's durability, then just repair it as normal
	if (getShield().getDurability() + repairs < getShield().getMaxDurability()) {
		getShield().setDurability(getShield().getDurability() + repairs);
	}
	// if repairs exceeds but does not equal the shield's max durability, then set to max
	else if (getShield().getDurability() + repairs > getShield().getMaxDurability()) {
		getShield().setDurability(getShield().getMaxDurability());
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
	playShieldHit(getPlayerShield().getItem());

	// decrease shield's durability by half the value of how much the shield blocks by
	if (getShield().getDurability() > 0) {
		getShield().setDurability(getShield().getDurability() - getShield().getDefense() / 2);
	}
	// if shield breaks, remove from the player
	if (getShield().getDurability() <= 0) {
		shieldBroken();
	}
}
bool Player::shieldCoverage(int mx, int my) {
	int px = getPosX();
	int py = getPosY();
	bool blocked = false;

	// check shield's coverage
	switch (getShield().getCoverage()) {
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

//void Player::addWeapon(Weapon w) {
//	m_inv.push_back(w);
//	m_invsize++;
//}
void Player::addWeapon(std::shared_ptr<Weapon> weapon) {
	m_weapons.push_back(weapon);
}
//void Player::addItem(Drops drop) {
//	m_iteminv.push_back(drop);
//	m_iteminvsize++;
//}
void Player::addItem(std::shared_ptr<Drops> drop) {
	m_items.push_back(drop);
}
/*
void Player::wield(std::vector<std::string> &text) {
	unsigned char c;
	if (m_invsize == 0)
		cout << "You have no weapons to swap to.\n";
	else {
		cout << "Choose an item to wield.\n" << endl;
		showWeapons(text);

		c = getCharacter();

		if (c >= 'a' && c < 'a' + m_invsize) {
			std::shared_ptr<Weapon> old = getWeapon();
			setWeapon(m_inv[c-97]);	// switch to new weapon
			cout << "You've switched to your " << getWeapon()->getAction() << ".\n";
			m_weapons.erase(m_weapons.begin() + (c - 97));			// remove item just equipped
			m_weapons.push_back(old);		// push old weapon into inventory
		}
		else
			cout << "You still hold your " << getWeapon()->getAction() << ".\n";
	}
	c = getCharacter();
}
void Player::wield() {
	unsigned char c;
	if (m_invsize == 0)
		;// cout << "You have no weapons to swap to.\n";
	else {
		//cout << "Choose an item to wield.\n" << endl;

		int i = 0;
		while (c = getCharacter(), c != '\r' || c != '\n' || c != 27) { // 27 is the esc char
			if (c == ARROW_LEFT && i > 0)
				i--;
			else if (c == ARROW_RIGHT && i < m_invsize-1)
				i++;
		}

		if (c != 27) {
			Weapon old = getWeapon();
			setWeapon(m_inv[i]);				// switch to new weapon
			m_inv.erase(m_inv.begin() + i);		// remove item just equipped
			m_inv.push_back(old);				// push old weapon into inventory
			//cout << "You've switched to your " << getWeapon().getAction() << ".\n";
		}
		else
			;//cout << "You still hold your " << getWeapon().getAction() << ".\n";
	}
}
*/
void Player::wield(int index) {
	std::shared_ptr<Weapon> old = getWeapon();
	setWeapon(m_weapons[index]);				// switch to new weapon
	m_weapons.erase(m_weapons.begin() + index);		// remove item just equipped
	m_weapons.push_back(old);					// push old weapon into inventory
}
/*
void Player::use(std::vector<std::shared_ptr<Objects>> &active, _Tile &tile, std::vector<std::string> &text) {
	unsigned char c;
	if (m_iteminvsize == 0)
		cout << "You have no items to use.\n";
	else {
		cout << "Choose an item to use.\n" << endl;
		showItems(text);

		c = getCharacter();

		if (c >= 'a' && c < 'a' + m_iteminvsize) {
			string item = m_iteminv.at(c - 97).getItem();
			if (item == "Life Potion") {
				LifePotion lp;
				lp.changeStats(lp, *this);
			}
			else if (item == "Armor") {
				ArmorDrop armor;
				armor.changeStats(armor, *this);
			}
			else if (item == "Stat Potion") {
				StatPotion sp;
				sp.changeStats(sp, *this);
			}
			else if (item == "Bomb") {
				Bomb bomb;
				tile.traptile = LITBOMB;
				bomb.lightBomb();
				bomb.setPosX(getPosX());
				bomb.setPosY(getPosY());

				active.emplace_back(new Bomb(bomb));
				cout << "The bomb was placed.\n";
			}
			else if (item == "Mysterious Trinket") {
				if (tile.bottom == INFINITY_BOX) {
					cout << "The box was unlocked!\n";

					InfinityBox infbox;
					infbox.open(tile, text);
				}
				else {
					cout << "Nothing happened.\n";
					c = getCharacter();
					return;
				}
			}

			m_iteminv.erase(m_iteminv.begin() + (c - 97));	// remove item just used
			m_iteminvsize--;
		}
		else
			cout << "You decide not to use anything." << endl;
	}
	c = getCharacter();
}
*/
void Player::use(Dungeon &dungeon, _Tile &tile, int index) {
	// if item is meant for player, do this
	if (m_items.at(index)->forPlayer()) {
		m_items.at(index)->changeStats(*m_items.at(index), *this);
	}
	// otherwise use it on the dungeon/surroundings
	else {
		m_items.at(index)->useItem(*m_items.at(index), dungeon);
	}

	m_items.erase(m_items.begin() + index);	// remove item just used
}
void Player::rollHeal(std::vector<std::string> &text) {
	if (getHP() > 0 && getHP() < getMaxHP()) {
		if (randInt(50) + 1 > 49) {
			// 2% chance to heal the player if below max hp
			setHP(getHP() + 1);
			text.push_back("You feel refreshed.\n");
		}
	}
}

bool Player::hasSkeletonKey() const {
	for (int i = 0; i < m_iteminvsize; i++) {
		if (m_iteminv.at(i).getItem() == "Mysterious Trinket")
			return true;
	}
	return false;
}
void Player::checkKeyConditions(std::vector<std::string> &text) {
	// if player hp is less than a specified threshold, and player was hit after picking up the key, break it
	if (this->getHP() < 8 && this->getHP() < this->keyHP()) {
		for (int i = 0; i < m_iteminvsize; i++) {
			if (m_iteminv.at(i).getItem() == "Mysterious Trinket") {
				// play key broken sound effect
				cocos2d::experimental::AudioEngine::play2d("Skeleton_Key_Broken.mp3", false, 1.0f);

				text.push_back("The mysterious trinket shatters.\n");
				m_iteminv.erase(m_iteminv.begin() + i);	// remove the key
				m_iteminvsize--;
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


//		MONSTER FUNCTIONS
Monster::Monster() {

}
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, std::shared_ptr<Weapon> wep, string name, bool burnable, bool bleedable, bool healable, bool stunnable, bool freezable)
	: Actors(x, y, hp, armor, str, dex, wep, name, burnable, bleedable, healable, stunnable, freezable) {
	
}
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, string name, bool burnable, bool bleedable, bool healable, bool stunnable, bool freezable) 
	: Actors(x, y, hp, armor, str, dex, name, burnable, bleedable, healable, stunnable, freezable) {
	
}
Monster::~Monster(){

}

void Monster::encounter(Player &p, Monster &m, std::vector<std::string> &text) {
	m.attack(p, text);
}
void Monster::encounter(Monster &m1, Monster &m2) {
	m1.attack(m2);
}
void Monster::attack(Player &p, std::vector<std::string> &text) {
	string weapon = this->getWeapon()->getAction();
	if (weapon == "Wood Bow") {
		text.push_back("The " + this->getName() + " fires their " + weapon + " at you... ");
	}
	else {
		text.push_back("The " + this->getName() + " slashes their " + weapon + " at you... ");
	}

	std::string monster = this->getName();
	int mx = this->getPosX();
	int my = this->getPosY();
	int px = p.getPosX();
	int py = p.getPosY();

	int monsterPoints = getDex();
	int playerPoints = p.getDex() + p.getWeapon()->getDexBonus();

	bool blocked = false;
	// check if player is blocking
	if (p.isBlocking()) {
		// check shield's coverage
		switch (p.getShield().getCoverage()) {
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
	
	// if monster's attack roll is successful
	if (randInt(monsterPoints) >= randInt(playerPoints)) {
		int damage = std::max(1, 1 + randInt(getStr() + getWeapon()->getDmg()) - p.getArmor());

		// if the player successfully shielded the attack, increase the player's damage reduction and check if
		// monster is stunnable and set their stun status
		if (blocked) {
			damage = std::max(0, 1 + randInt(getStr() + getWeapon()->getDmg()) - (p.getArmor() + p.getShield().getDefense()));

			p.blocked();

			// set stun status
			if (monster == "goblin" || monster == "seeker" || monster == "mounted knight" || monster == "wanderer") {
				addAffliction(std::make_shared<Stun>());
			}
			else if (monster == "archer") {
				Archer *archer = dynamic_cast<Archer*>(this);

				if (archer->getWeapon()->getAction() == "Bronze Dagger") {
					addAffliction(std::make_shared<Stun>());
				}
			}
		}

		if (damage > 0) {
			// play got hit sound effect
			playGotHit();

			p.setHP(p.getHP() - damage);
			//text.push_back("and hits for " + to_string(damage) + " damage!");
			//text.push_back((damage >= 3 ? " Ouch!\n" : "\n"));
		}
		else {
			text.push_back("but you brush it off!\n");
		}
		
	}
	// else attack roll was unsuccessful
	else {
		text.push_back("and misses!\n");
	}
}
void Monster::attack(Monster &m) {
	;
}

bool Monster::isFlying() const {
	return m_flying;
}
void Monster::setFlying(bool flying) {
	m_flying = flying;
}

bool Monster::chasesPlayer() const {
	return m_chases;
}
void Monster::setChasesPlayer(bool chases) {
	m_chases = chases;
}

//							(	...	bool burnable, bool bleedable, bool healable, bool stunnable, bool freezable)

//		GOBLIN FUNCTIONS
Goblin::Goblin(int smelldist) : Monster(randInt(68) + 1, randInt(16) + 1, randInt(5) + 10, 1, 3, 1, std::make_shared<ShortSword>(), "goblin"), m_smelldist(smelldist) {
	setChasesPlayer(true);
}

int Goblin::getSmellDistance() const {
	return m_smelldist;
}


//		WANDERER FUNCTIONS
Wanderer::Wanderer() : Monster(randInt(68) + 1, randInt(16) + 1, 10, 1, 3, 1, std::make_shared<ShortSword>(), "wanderer") {
	setFlying(true);
}


//		ARCHER FUNCTIONS
Archer::Archer() : Monster(randInt(68) + 1, randInt(16) + 1, randInt(5) + 9, 2, 3, 2, std::make_shared<WoodBow>(), "archer"), m_primed(false) {

}

bool Archer::isPrimed() const {
	return m_primed;
}
void Archer::prime(bool p) {
	m_primed = p;
}


//		ZAPPER
Zapper::Zapper() : Monster(randInt(68) + 1, randInt(16) + 1, 10 + randInt(3), 1, 4, 1, "zapper"), m_cooldown(true), m_attack(randInt(2)) {
	for (int i = 0; i < 8; i++) {
		cocos2d::Sprite* spark = cocos2d::Sprite::createWithSpriteFrameName("Spark_48x48.png");
		sparks.insert(std::pair<int, cocos2d::Sprite*>(i, spark));
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


//		SPINNER
Spinner::Spinner() : Monster(randInt(68) + 1, randInt(16) + 1, 10 + randInt(3), 1, 3, 1, "spinner", false) {
	bool dir = randInt(2);

	m_clockwise = dir;
	m_angle = 1 + randInt(8);

	inner = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	outer = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
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

void Spinner::setInitialFirePosition(int x, int y) {

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


//		BOMBEE
Bombee::Bombee() : Monster(randInt(68) + 1, randInt(16) + 1, 1, 0, 1, 1, "bombee"), m_fuse(3), m_fused(false), m_aggroRange(11) {
	setChasesPlayer(true);
}

void Bombee::attack(Player &p, std::vector<std::string> &text) {
	playGotHit();

	//text.push_back("The " + this->getName() + " hits you for 1 damage.\n");
	p.setHP(p.getHP() - 1);
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
MountedKnight::MountedKnight() : Monster(randInt(68) + 1, randInt(16) + 1, 12 + randInt(5), 3, 2, 2, std::make_shared<IronLance>(), "mounted knight"), m_alerted(false) {

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
Roundabout::Roundabout() : Monster(randInt(68) + 1, randInt(16) + 1, 9 + randInt(2), 1, 5, 1, "roundabout") {
	int n = randInt(4);

	switch (n) {
	case 0: m_direction = 'l'; break;
	case 1: m_direction = 'r'; break;
	case 2: m_direction = 'u'; break;
	case 3: m_direction = 'd'; break;
	default: break;
	}
}

void Roundabout::attack(Player &p, std::vector<std::string> &text) {
	int damage = std::max(1, 1 + randInt(getStr() - p.getArmor()));

	// if player was blocking and blocked successfully
	if (p.isBlocking() && p.shieldCoverage(getPosX(), getPosY())) {
		damage = std::max(0, p.getShield().getDefense() - damage);
		p.blocked(); // calls shield durability change
	}

	if (damage > 0) {
		playGotHit();

		p.setHP(p.getHP() - (damage - p.getArmor()));
		//text.push_back("The " + this->getName() + " hits you for " + to_string(damage - p.getArmor()) + " damage!\n");
	}
	else {
		;//text.push_back("The " + this->getName() + "bumps into you.\n");
	}
}
char Roundabout::getDirection() const {
	return m_direction;
}
void Roundabout::setDirection(char dir) {
	m_direction = dir;
}


//		SEEKER
Seeker::Seeker(int range) : Monster(randInt(68) + 1, randInt(16) + 1, 8 + randInt(3), 0, 3, 1, std::make_shared<ShortSword>(), "seeker"), m_range(range), m_step(false) {
	setChasesPlayer(true);
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




//
//
//		BOSS FIGHT FUNCTIONS

Smasher::Smasher() : Monster(BOSSCOLS / 2, 3, 500, 0, 4, 1, std::make_shared<SmashersFists>(), "Smasher"), m_moveActive(false), m_moveEnding(true), m_frenzy(false) {

}

void Smasher::attack(Player &p, std::vector<std::string> &text) {
	cocos2d::experimental::AudioEngine::play2d("Smasher_HitBy1.mp3", false, 1.0f);

	string weapon = this->getWeapon()->getAction();
	
	int damage = 4+randInt(2);
	p.setHP(p.getHP() - damage);
	
	text.push_back(this->getName() + " smashes you for " + to_string(damage) + " damage!\n");
}
void Smasher::attack(Monster &m) {
	cocos2d::experimental::AudioEngine::play2d("Smasher_HitBy1.mp3", false, 1.0f);

	int damage = 100;
	m.setHP(m.getHP() - damage);

	//text.push_back(this->getName() + " smashes you for " + to_string(damage) + " damage!\n");
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
}
void playShieldBroken(std::string shield) {
	if (shield == "Wood Shield") {
		cocos2d::experimental::AudioEngine::play2d("Wood_Shield_Broken.mp3", false, 1.0f);
	}
	else if (shield == "Iron Shield") {
		cocos2d::experimental::AudioEngine::play2d("Iron_Shield_Broken.mp3", false, 1.0f);
	}
}