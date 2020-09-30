#include "global.h"
#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include "utilities.h"
#include <random>
#include <iostream>
#include <string>

using namespace std;


//		ACTOR FUNCTIONS
Actors::Actors() : m_x(0), m_y(0), m_hp(0), m_armor(0), m_str(0), m_dex(0), m_wep(ShortSword()) {

}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, Weapon wep)
	: m_x(x), m_y(y), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_wep(wep) {

}
Actors::~Actors() {

}

int Actors::getPosX() const {
	return m_x;
}
int Actors::getPosY() const {
	return m_y;
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
Weapon Actors::getWeapon() const {
	return m_wep;
}

void Actors::setPosX(int x) {
	m_x = x;
}
void Actors::setPosY(int y) {
	m_y = y;
}
void Actors::setrandPosX() {
	m_x = randInt(68) + 1;
}
void Actors::setrandPosY() {
	m_y = randInt(16) + 1;
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
void Actors::setWeapon(Weapon wep) {
	m_wep = wep;
}


//		PLAYER FUNCTIONS
Player::Player() : Actors(randInt(68) + 1, randInt(16) + 1, 20, 2, 2, 2, ShortSword()) {
	m_invsize = 0;
	m_iteminvsize = 0;
	m_maxhp = 20;
	m_winner = false;
}
Player::~Player() {

}

void Player::attack(vector<shared_ptr<Monster>> monsters, int pos, vector<string> &text) {
	//cout << "You swing your " << getWeapon().getAction() << " and... ";
	text.push_back("You swing your " + getWeapon().getAction() + " and... ");

	int playerPoints = getDex() + getArmor();
	int monsterPoints = monsters.at(pos)->getDex() + monsters.at(pos)->getArmor();

	if (randInt(playerPoints) >= randInt(monsterPoints)) {
		int damage = randInt(getStr() + getWeapon().getDmg());
		if (damage > 0) {
			monsters.at(pos)->setHP(monsters.at(pos)->getHP() - damage);
			//cout << "deal " << damage << " damage to the " << monsters.at(pos)->getName() << "!" << endl;
			//cout << "The " << monsters.at(pos)->getName() << " has " << monsters.at(pos)->getHP() << " HP remaining.\n" << endl;
			text.push_back("deal " + to_string(damage) + " damage to the " + monsters.at(pos)->getName() + "!\n");
			text.push_back("The " + monsters.at(pos)->getName() + " has " + to_string(monsters.at(pos)->getHP()) + " HP remaining.\n\n");
		}
		else {
			//cout << "your attack fails!" << endl;
			text.push_back("your attack fails!\n");
		}
	}
	else {
		//cout << "miss.\n" << endl;
		text.push_back("miss.\n");
	}
}
void Player::showInventory() {
	//rollHeal();

	cout << "Inventory:" << endl;
	if (m_invsize + m_iteminvsize == 0)
		cout << "You have no items." << endl;
	else {
		unsigned i, j;
		char c = 97;

		if (m_invsize != 0) {
			for (i = 0; i < m_invsize; i++) {
				c += i;
				cout << c << ". " << m_inv.at(i).getAction() << endl;
			}
			c++;
		}

		for (j = 0; j < m_iteminvsize; j++) {
			c += j;
			cout << c << ". " << m_iteminv.at(j).getItem() << endl;
		}
	}
	cout << endl;	
}
void Player::showWeapons(vector<string> &text) {
	rollHeal(text);

	cout << "Weapons:" << endl;
	//text.push_back("Weapons:");
	
	char c = 97;
	for (unsigned i = 0; i < m_invsize; i++) {
		c += i;
		cout << c << ". " << m_inv.at(i).getAction() << endl;
		//text.push_back(c + ". " + m_inv.at(i).getAction() + '\n');
	}
	
	cout << endl;
}
void Player::showItems(vector<string> &text) {
	rollHeal(text);

	cout << "Items:" << endl;
	//text.push_back("Items:");

	char c = 97;
	for (unsigned i = 0; i < m_iteminvsize; i++) {
		c += i;
		cout << c << ". " << m_iteminv.at(i).getItem() << endl;
		//text.push_back(c + ". " + m_iteminv.at(i).getItem() + '\n');
	}

	cout << endl;
}
int Player::getInventorySize() const {
	return m_invsize;
}
int Player::getItemInvSize() const {
	return m_iteminvsize;
}
void Player::addWeapon(Weapon w) {
	m_inv.push_back(w);
	m_invsize++;
}
void Player::addItem(Drops drop) {
	m_iteminv.push_back(drop);
	m_iteminvsize++;
}
void Player::wield(vector<string> &text) {
	char c;
	if (m_invsize == 0)
		cout << "You have no weapons to swap to.\n";
		//text.push_back("You have no weapons to swap to.\n");
	else {
		cout << "Choose an item to wield.\n" << endl;
		//text.push_back("Choose an item to wield.\n");
		showWeapons(text);

		c = getCharacter();

		if (c >= 'a' && c < 'a' + m_invsize) {
			Weapon old = getWeapon();
			setWeapon(m_inv[c-97]);	// switch to new weapon
			cout << "You've switched to your " << getWeapon().getAction() << ".\n";
			//text.push_back("You've switched to your " + getWeapon().getAction() + ".\n");
			m_inv.erase(m_inv.begin() + (c - 97));			// remove item just equipped
			m_inv.push_back(old);		// push old weapon into inventory
		}
		else
			cout << "You still hold your " << getWeapon().getAction() << ".\n";
			//text.push_back("You still hold your " + getWeapon().getAction() + ".\n");
	}
	c = getCharacter();
}
void Player::use(std::vector<std::shared_ptr<Objects>> &active, Tile &tile, vector<string> &text) {
	char c;
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
				tile.bottom = LITBOMB;
				bomb.lightBomb();
				bomb.setPosX(getPosX());
				bomb.setPosY(getPosY());

				active.emplace_back(new Bomb(bomb));
				cout << "The bomb was placed.\n";
			}
			m_iteminv.erase(m_iteminv.begin() + (c - 97));	// remove item just used
			m_iteminvsize--;
		}
		else
			cout << "You decide not to use anything." << endl;
	}
	c = getCharacter();
}
void Player::setMaxHP(int maxhp) {
	m_maxhp = maxhp;
}
int Player::getMaxHP() const {
	return m_maxhp;
}
void Player::rollHeal(vector<string> &text) {
	if (getHP() > 0 && getHP() < getMaxHP()) {
		if (randInt(10) + 1 > 9) {
			// 10% chance to heal the player if below max hp
			setHP(getHP() + 1);
			//cout << "You feel refreshed.\n" << endl;
			text.push_back("You feel refreshed.\n");
		}
	}
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
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, Weapon wep, string name)
	: Actors(x, y, hp, armor, str, dex, wep), m_name(name) {

}
Monster::~Monster(){

}

void Monster::encounter(Player &p, Monster &m, std::vector<std::string> &text) {
	m.attack(p, text);
}
void Monster::attack(Player &p, vector<string> &text) {
	string weapon = this->getWeapon().getAction();
	if (weapon == "Wood Bow")
		//cout << "The " << this->getName() << " fires their " << weapon << " at you... ";
		text.push_back("The " + this->getName() + " fires their " + weapon + " at you... ");
	else
		//cout << "The " << this->getName() << " slashes their " << weapon << " at you... ";
		text.push_back("The " + this->getName() + " slashes their " + weapon + " at you... ");

	int monsterPoints = getDex() + getArmor();
	int playerPoints = p.getDex() + p.getArmor();

	if (randInt(monsterPoints) >= randInt(playerPoints)) {
		int damage = randInt(getStr() + getWeapon().getDmg());
		p.setHP(p.getHP() - damage);
		if (damage > 0) {
			//cout << "and hits for " << damage << " damage!";
			//cout << (damage >= 3 ? " Ouch!\n" : "\n");
			text.push_back("and hits for " + to_string(damage) + " damage!");
			text.push_back((damage >= 3 ? " Ouch!\n" : "\n"));
		}
		else {
			//cout << "and fumbles their attack." << endl;
			text.push_back("and fumbles their attack.\n");
		}
		cout << endl;
	}
	else {
		//cout << "and misses!" << endl;
		text.push_back("and misses!\n");
	}
}
string Monster::getName() {
	return m_name;
}


//		GOBLIN FUNCTIONS
Goblin::Goblin(int smelldist) : Monster(randInt(68) + 1, randInt(16) + 1, randInt(5) + 16, 1, 3, 1, ShortSword(), "goblin"), m_smelldist(smelldist) {
	
}
Goblin::~Goblin() {

}

int Goblin::getSmellDistance() const {
	return m_smelldist;
}


//		WANDERER FUNCTIONS
Wanderer::Wanderer() : Monster(randInt(68) + 1, randInt(16) + 1, 8, 2, 4, 0, ShortSword(), "wanderer") {

}
Wanderer::~Wanderer(){

}


//		ARCHER FUNCTIONS
Archer::Archer() : Monster(randInt(68) + 1, randInt(16) + 1, randInt(5) + 7, 2, 3, 1, WoodBow(), "archer"), m_primed(false) {

}

bool Archer::isPrimed() const {
	return m_primed;
}
void Archer::prime(bool p) {
	m_primed = p;
}



//
//
//		BOSS FIGHT FUNCTIONS

Smasher::Smasher() : Monster(BOSSCOLS / 2, 3, 100, 2, 4, 0, SmashersFists(), "Smasher"), m_moveActive(false), m_moveEnding(true) {

}
void Smasher::attack(Player &p, vector<string> &text) {
	string weapon = this->getWeapon().getAction();
	
	int monsterPoints = getDex() + getArmor();
	int playerPoints = p.getDex() + p.getArmor();

	int damage = 4+randInt(2);
	p.setHP(p.getHP() - damage);
	
	//cout << this->getName() << " smashes you for " << damage << " damage!\n";
	text.push_back(this->getName() + " smashes you for " + to_string(damage) + " damage!\n");
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

//		SMASHER SPIKES
