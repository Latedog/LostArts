#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include "utilities.h"
#include <random>
#include <iostream>

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
	m_maxhp = 20;
	m_winner = false;
}
Player::~Player() {

}

void Player::attack(Goblin &g) {
	cout << "You swing your " << getWeapon().getAction() << " and... ";
	
	int playerPoints = getDex() + getArmor();
	int goblinPoints = g.getDex() + g.getArmor();

	if (randInt(playerPoints) >= randInt(goblinPoints)) {
		int damage = randInt(getStr() + getWeapon().getDmg());
		if (damage > 0) {
			g.setHP(g.getHP() - damage);
			cout << "deal " << damage << " damage to the goblin!" << endl;
			cout << "Goblin has " << g.getHP() << " HP remaining.\n" << endl;
		}
		else {
			cout << "your attack fails!" << endl;
		}
	}
	else {
		cout << "miss.\n" << endl;
	}
}
void Player::showInventory() {
	rollHeal();

	cout << "Inventory:" << endl;
	if (m_invsize == 0)
		cout << "You have no items." << endl;
	else {
		char c = 97;
		for (int i = 0; i < m_invsize; i++) {
			c += i;
			cout << c << ". " << m_inv.at(i).getAction() << endl;
		}
	}
	cout << endl;	
}
std::vector<Weapon> Player::getInventory() {
	return m_inv;
}
int Player::getInventorySize() const {
	return m_invsize;
}
void Player::addInventory(Weapon w) {
	m_inv.push_back(w);
	m_invsize++;
}
void Player::wield() {

	if (m_invsize == 0)
		cout << "You have no items swap to.\n";
	else {
		cout << "Choose an item to wield.\n" << endl;
		showInventory();

		char c = getCharacter();

		if (c >= 'a' && c < 'a' + m_invsize) {
			Weapon old = getWeapon();
			setWeapon(m_inv[c-97]);	// switch to new weapon
			cout << "You've switched to your " << getWeapon().getAction() << ".\n";
			m_inv.erase(m_inv.begin() + (c - 97));			// remove item just equipped
			m_inv.push_back(old);		// push old weapon into inventory
		}
		else
			cout << "You still hold your " << getWeapon().getAction() << ".\n";
	}
}
int Player::getMaxHP() const {
	return m_maxhp;
}
void Player::rollHeal() {
	if (getHP() > 0 && getHP() < getMaxHP()) {
		if (randInt(10) + 1 > 9) {
			// 10% chance to heal the player if below max hp
			setHP(getHP() + 1);
			cout << "You feel refreshed.\n" << endl;
		}
	}
}
void Player::setWin(bool win) {
	m_winner = win;
}
bool Player::getWin() const {
	return m_winner;
}

//		MONSTER FUNCTIONS
Monster::Monster() {

}
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, Weapon wep)
	: Actors(x, y, hp, armor, str, dex, wep) {

}
Monster::~Monster(){

}


//		GOBLIN FUNCTIONS
Goblin::Goblin(int smelldist) : Monster(randInt(68) + 1, randInt(16) + 1, randInt(5) + 16, 1, 3, 1, ShortSword()) {
	m_smelldist = smelldist;
}
Goblin::~Goblin() {

}

void Goblin::attack(Player &p) {
	cout << "The goblin swings their " << getWeapon().getAction() << " at you... ";

	int goblinPoints = getDex() + getArmor();
	int playerPoints = p.getDex() + p.getArmor();

	if (randInt(goblinPoints) >= randInt(playerPoints)) {
		int damage = randInt(getStr() + getWeapon().getDmg());
		p.setHP(p.getHP() - damage);
		if (damage > 0) {
			cout << "and hits for " << damage << " damage!";
			cout << (damage >= 3 ? " Ouch!\n" : "\n");
		}
		else {
			cout << "fumbles their attack." << endl;
		}
		cout << endl;
	}
	else {
		cout << "and misses!" << endl;
	}
}
int Goblin::getSmellDistance() const {
	return m_smelldist;
}