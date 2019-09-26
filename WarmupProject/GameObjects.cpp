#include "global.h"
#include "GameObjects.h"
#include "Actors.h"
#include "utilities.h"
#include "Dungeon.h"
#include <string>
#include <iostream>

using namespace std;


//		OBJECT FUNCTIONS
Objects::Objects() {

}
Objects::Objects(int x, int y) : m_x(x), m_y(y) {

}
Objects::~Objects() {

}

int Objects::getPosX() const {
	return m_x;
}
int Objects::getPosY() const {
	return m_y;
}
void Objects::setrandPosX()
{
	m_x = randInt(68) + 1;
}
void Objects::setrandPosY()
{
	m_y = randInt(16) + 1;
}


//		IDOL FUNCTIONS
Idol::Idol() : Objects(randInt(68) + 1, randInt(16) + 1) {

}
Idol::~Idol() {

}


//		DROP FUNCTIONS
Drops::Drops(int x, int y, string item) : Objects(x, y) {
	m_item = item;
}
string Drops::getItem() const {
	return m_item;
}

LifePotion::LifePotion() : Drops(randInt(68) + 1, randInt(16) + 1, "Life Potion") {
	
}
void LifePotion::restoreHP(Player &p) {
	p.setHP(p.getHP() + 10);
	cout << "You feel replenished." << endl;
}

ArmorDrop::ArmorDrop() : Drops(randInt(68) + 1, randInt(16) + 1, "Armor") {

}
void ArmorDrop::increaseArmor(Player &p) {
	p.setArmor(p.getArmor() + 1);
	cout << "You feel ready for battle." << endl;
}

StatPotion::StatPotion() : Drops(randInt(68) + 1, randInt(16) + 1, "Stat Potion") {

}
void StatPotion::buffStats(Player &p) {
	p.setDex(p.getDex() + 1);
	p.setStr(p.getStr() + 2);
	cout << "You feel stronger." << endl;
}

Chest::Chest() : Drops(randInt(68) + 1, randInt(16) + 1, "Chest") {

}
void Chest::open(Tile &tile) {
	int n = randInt(3) + 1;
	switch (n) {
	case 1:
		tile.bottom = LIFEPOT; // life potion
		cout << " and find a health potion!" << endl;
		break;
	case 2:
		tile.bottom = ARMOR; // armor
		cout << " and find armor!" << endl;
		break;
	case 3:
		tile.bottom = STATPOT; // stat potion
		cout << " and find a stat potion!" << endl;
		break;
	default:
		break;
	}
}


//		WEAPON FUNCTIONS
Weapon::Weapon() {

}
Weapon::Weapon(int x, int y, string action, int dexbonus, int dmg) : Objects(x, y), m_action(action), m_dexbonus(dexbonus), m_dmg(dmg) {

}
Weapon::~Weapon() {

}

string Weapon::getAction() const {
	return m_action;
}
int Weapon::getDexBonus() const {
	return m_dexbonus;
}
int Weapon::getDmg() const {
	return m_dmg;
}


//		SHORT SWORD FUNCTIONS
ShortSword::ShortSword() : Weapon(randInt(68) + 1, randInt(16) + 1, "Short Sword", 1, 2) {

}
ShortSword::~ShortSword() {

}


//		RUSTY CUTLASS
RustyCutlass::RustyCutlass(int x, int y) : Weapon(x, y, "Rusty Cutlass", 2, 1) {

}


//		BONE AXE 
BoneAxe::BoneAxe(int x, int y) : Weapon(x, y, "Bone Axe", 1, 3) {

}
BoneAxe::~BoneAxe() {

}