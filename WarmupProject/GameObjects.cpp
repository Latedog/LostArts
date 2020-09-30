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
Objects::Objects(int x, int y, string item) : m_x(x), m_y(y), m_item(item) {

}
Objects::~Objects() {

}

string Objects::getItem() const {
	return m_item;
}
int Objects::getPosX() const {
	return m_x;
}
int Objects::getPosY() const {
	return m_y;
}
void Objects::setPosX(int x) {
	m_x = x;
}
void Objects::setPosY(int y) {
	m_y = y;
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
Idol::Idol() : Objects(randInt(68) + 1, randInt(16) + 1, "Idol") {

}
Idol::~Idol() {

}


//		STAIRS FUNCTIONS
Stairs::Stairs() : Objects(randInt(68) + 1, randInt(16) + 1, "stairs") {

}


//		DROP FUNCTIONS
Drops::Drops(int x, int y, string item) : Objects(x, y, item) {
	
}
void Drops::changeStats(Drops &drop, Player &p) {
	drop.useItem(p);
}

HeartPod::HeartPod() : Drops(randInt(68) + 1, randInt(16) + 1, "Heart Pod") {

}
HeartPod::HeartPod(int x, int y) : Drops(x, y, "Heart Pod") {

}
void HeartPod::useItem(Player &p) {
	if (p.getHP() < p.getMaxHP()) {			// if player hp is less than their max
		if (p.getHP() + 2 <= p.getMaxHP()) // if player hp + effect of lifepot will refill less than or equal to max
			p.setHP(p.getHP() + 2);
		else	// just set player hp to max
			p.setHP(p.getMaxHP());

		cout << "You feel a little healthier.\n" << endl;
		//text.push_back("You feel a little healthier.\n");
	}
	else
		cout << "Lively as ever..." << endl;
		//text.push_back("Lively as ever...\n");
}

LifePotion::LifePotion() : Drops(randInt(68) + 1, randInt(16) + 1, "Life Potion") {
	
}
void LifePotion::useItem(Player &p) {
	if (p.getHP() < p.getMaxHP()) {			// if player hp is less than their max
		if (p.getHP() + 10 <= p.getMaxHP()) // if player hp + effect of lifepot will refill less than or equal to max
			p.setHP(p.getHP() + 10);
		else	// just set player hp to max
			p.setHP(p.getMaxHP());

		cout << "You feel replenished." << endl;
		//text.push_back("You feel replenished.\n");
	}
	else
		cout << "Lively as ever..." << endl;
		//text.push_back("Lively as ever...\n");
}

ArmorDrop::ArmorDrop() : Drops(randInt(68) + 1, randInt(16) + 1, "Armor") {

}
void ArmorDrop::useItem(Player &p) {
	p.setArmor(p.getArmor() + 1);
	cout << "You feel ready for battle." << endl;
	//text.push_back("You feel ready for battle.\n");
}

StatPotion::StatPotion() : Drops(randInt(68) + 1, randInt(16) + 1, "Stat Potion") {

}
void StatPotion::useItem(Player &p) {
	p.setDex(p.getDex() + 1);
	p.setStr(p.getStr() + 1);
	p.setMaxHP(p.getMaxHP() + 5);
	cout << "You feel stronger." << endl;
	//text.push_back("You feel stronger.\n");
}

Bomb::Bomb() : Drops(randInt(68) + 1, randInt(16) + 1, "Bomb"), m_fuse(3), m_lit(false) {

}
void Bomb::useItem(Player &p) {
	if (isLit()) {
		m_fuse--;
	}
}
int Bomb::getFuse() const {
	return m_fuse;
}
void Bomb::setFuse(int fuse) {
	m_fuse = fuse;

}
void Bomb::lightBomb() {
	m_lit = true;
}
bool Bomb::isLit() const {
	return m_lit;
}


//		CHESTS FUNCTIONS
Chests::Chests() : Drops(randInt(68) + 1, randInt(16) + 1, "Chest") {

}

BrownChest::BrownChest() : Drops(randInt(68) + 1, randInt(16) + 1, "Brown Chest") {

}
void BrownChest::open(Tile &tile, std::vector<std::string> &text) {
	int n = 1 + randInt(4);
	switch (n) {
	case 1:
		tile.bottom = LIFEPOT; // life potion
		//cout << " and find a health potion!" << endl;
		text.push_back(" and find a health potion!\n");
		break;
	case 2:
		tile.bottom = ARMOR; // armor
		//cout << " and find Armor!\n" << endl;
		text.push_back(" and find Armor!\n");
		break;
	case 3:
		tile.bottom = STATPOT; // stat potion
		//cout << " and find a Stat Potion!\n" << endl;
		text.push_back(" and find a Stat Potion!\n");
		break;
	case 4:
		tile.bottom = BOMB; // stat potion
		//cout << " and find a Bomb!\n" << endl;
		text.push_back(" and find a Bomb!\n");
		break;
	default:
		break;
	}
}

SilverChest::SilverChest() : Drops(randInt(68) + 1, randInt(16) + 1, "Silver Chest") {

}
void SilverChest::open(Tile & tile, std::vector<std::string> &text){
	int n = randInt(3) + 1;
	switch (n) {
	case 1:
		tile.bottom = LIFEPOT; // life potion
		cout << " and find a health potion!" << endl;
		break;
	case 2:
		tile.bottom = ARMOR; // armor
		cout << " and find Armor!" << endl;
		break;
	case 3:
		tile.bottom = STATPOT; // stat potion
		cout << " and find a Stat Potion!" << endl;
		break;
	default:
		break;
	}
}

GoldenChest::GoldenChest() : Drops(randInt(68) + 1, randInt(16) + 1, "Golden Chest") {

}
void GoldenChest::open(Tile & tile, std::vector<std::string> &text) {

}


//		WEAPON FUNCTIONS
Weapon::Weapon() {

}
Weapon::Weapon(int x, int y, string action, int dexbonus, int dmg) : Objects(x, y, action), m_action(action), m_dexbonus(dexbonus), m_dmg(dmg) {

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
RustyCutlass::RustyCutlass() : Weapon(randInt(68) + 1, randInt(16) + 1, "Rusty Cutlass", 2, 1) {

}
RustyCutlass::RustyCutlass(int x, int y) : Weapon(x, y, "Rusty Cutlass", 2, 1) {

}


//		BONE AXE 
BoneAxe::BoneAxe(int x, int y) : Weapon(x, y, "Bone Axe", 1, 3) {

}
BoneAxe::~BoneAxe() {

}


//		WOOD BOW
WoodBow::WoodBow() : Weapon(randInt(68) + 1, randInt(16) + 1, "Wood Bow", 2, 2) {

}


//		BRONZE DAGGER
BronzeDagger::BronzeDagger() : Weapon(randInt(68) + 1, randInt(16) + 1, "Bronze Dagger", 2, 1) {

}


//
//
//		BOSS WEAPONS
SmashersFists::SmashersFists() : Weapon(1,1, "his body", 5, 0) {

}


//		TRAPS
Traps::Traps(int x, int y, string name, int damage) : Objects (x, y, name), m_trapdmg(damage) {

}
int Traps::getDmg() const {
	return m_trapdmg;
}
void Traps::setDmg(int damage) {
	m_trapdmg = damage;
}

//		FALLING SPIKES
FallingSpike::FallingSpike(int x, int y, int speed) : Traps(x, y, "falling spike", 3), m_speed(speed) {

}
int FallingSpike::getSpeed() const {
	return m_speed;
}
void FallingSpike::setSpeed(int speed) {
	m_speed = speed;
}

//		SPIKE TRAPS
SpikeTrap::SpikeTrap() : Traps(1 + randInt(BOSSCOLS - 2), BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2), "spike trap", 5), m_cyclespeed(3 + randInt(3)) {

}
SpikeTrap::SpikeTrap(int x, int y, int speed) : Traps(x, y, "spike trap", 3), m_cyclespeed(speed), m_countdown(speed) {

}

int SpikeTrap::getSpeed() const {
	return m_cyclespeed;
}
void SpikeTrap::setSpeed(int speed) {
	m_cyclespeed = speed;
}
int SpikeTrap::getCountdown() const {
	return m_countdown;
}
void SpikeTrap::setCountdown(int count) {
	m_countdown = count;
}