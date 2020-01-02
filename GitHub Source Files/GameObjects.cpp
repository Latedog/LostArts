#include "global.h"
#include "GameObjects.h"
#include "Actors.h"
#include "utilities.h"
#include "Dungeon.h"
#include <string>
#include <iostream>


using std::string;
using std::cout;
using std::endl;


//		OBJECT FUNCTIONS
Objects::Objects() {

}
Objects::Objects(string status) : m_item(status){

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
void Objects::setrandPosX(int maxrows) {
	m_x = randInt(maxrows - 2) + 1;
}
void Objects::setrandPosY(int maxcols) {
	m_y = randInt(maxcols - 2) + 1;
}


//		IDOL FUNCTIONS
Idol::Idol() : Objects(randInt(68) + 1, randInt(16) + 1, "Idol") {

}
Idol::~Idol() {

}


//		STAIRS FUNCTIONS
Stairs::Stairs() : Objects(randInt(68) + 1, randInt(16) + 1, "stairs") {

}


//		DOOR FUNCTIONS
Door::Door() : Objects(randInt(68) + 1, randInt(16) + 1, "door"), m_open(false), m_hold(true), m_lock(false) {

}

bool Door::isOpen() const {
	return m_open;
}
void Door::toggleOpen() {
	m_open = !m_open;
}
bool Door::isHeld() const {
	return m_hold;
}
void Door::toggleHeld() {
	m_hold = !m_hold;
}
bool Door::isLocked() const {
	return m_lock;
}
void Door::toggleLock() {
	m_lock = !m_lock;
}


//		BUTTON FUNCTIONS
Button::Button() : Objects(randInt(68) + 1, randInt(16) + 1, "button") {

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
		if (p.getHP() + 1 <= p.getMaxHP()) // if player hp + effect of heart pod will refill less than or equal to max
			p.setHP(p.getHP() + 1);
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
Bomb::Bomb(string type) : Drops(randInt(68) + 1, randInt(16) + 1, type), m_fuse(3), m_lit(false) {

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

MegaBomb::MegaBomb() : Bomb("Mega Bomb") {

}


//		SKELETON KEY
SkeletonKey::SkeletonKey() : Drops(randInt(68) + 1, randInt(16) + 1, "Mysterious Trinket") {

}



//		CHESTS FUNCTIONS
Chests::Chests(string chest) : Drops(randInt(68) + 1, randInt(16) + 1, chest) {

}

BrownChest::BrownChest() : Chests("Brown Chest") {

}
std::string BrownChest::open(_Tile &tile, std::vector<std::string> &text) {
	int n = 1 + randInt(3);
	switch (n) {
	case 1:
		tile.bottom = LIFEPOT; // life potion
		text.push_back(" and find a health potion!\n");
		return "Life_Potion_48x48.png";
		break;
	case 2:
		tile.bottom = ARMOR; // armor
		text.push_back(" and find Armor!\n");
		return "Armor_48x48.png";
		break;
	//case 3:
	//	tile.bottom = STATPOT; // stat potion
	//	text.push_back(" and find a Stat Potion!\n");
	//	break;
	case 3:
		tile.bottom = BOMB; // stat potion
		text.push_back(" and find a Bomb!\n");
		return "Bomb_48x48.png";
		break;
	default:
		break;
	}
}

SilverChest::SilverChest() : Chests("Silver Chest") {

}
std::string SilverChest::open(_Tile &tile, std::vector<std::string> &text){
	int n = 1 + randInt(3);
	switch (n) {
	//case 1:
	//	tile.bottom = LIFEPOT; // life potion
	//	text.push_back(" and find a health potion!\n");
	//	break;
	case 1:
		tile.bottom = ARMOR; // armor
		text.push_back(" and find Armor!\n");
		return "Armor_48x48.png";
		break;
	case 2:
		tile.bottom = STATPOT; // stat potion
		text.push_back(" and find a Stat Potion!\n");
		return "Stat_Potion_48x48.png";
		break;
	case 3:
		tile.bottom = BOMB; // bomb
		text.push_back(" and find a Bomb!\n");
		return "Bomb_48x48.png";
		break;
	default:
		break;
	}
}

GoldenChest::GoldenChest() : Chests("Golden Chest") {

}
std::string GoldenChest::open(_Tile &tile, std::vector<std::string> &text) {
	int n = 1 + randInt(4);
	switch (n) {
	case 1:
		tile.bottom = LIFEPOT; // life potion
		text.push_back(" and find a health potion!\n");
		return "Life_Potion_48x48.png";
		break;
	case 2:
		tile.bottom = ARMOR; // armor
		text.push_back(" and find Armor!\n");
		return "Armor_48x48.png";
		break;
	case 3:
		tile.bottom = STATPOT; // stat potion
		text.push_back(" and find a Stat Potion!\n");
		return "Stat_Potion_48x48.png";
		break;
	case 4:
		tile.bottom = BOMB; // bomb
		text.push_back(" and find a Bomb!\n");
		return "Bomb_48x48.png";
		break;
	default:
		break;
	}
}

InfinityBox::InfinityBox() : Chests("Infinity Box") {

}
std::string InfinityBox::open(_Tile &tile, std::vector<std::string> &text) {
	int n = 1 + randInt(4);
	switch (n) {
	case 1:
		tile.bottom = LIFEPOT; // life potion
		text.push_back("It's a Legendary Crossbow!\n");
		break;
	case 2:
		tile.bottom = ARMOR; // armor
		text.push_back("It's the Rune of Overwhelming Power!\n");
		break;
	case 3:
		tile.bottom = STATPOT; // stat potion
		text.push_back("It's a Spell of Destruction!\n");
		break;
	case 4:
		tile.bottom = BOMB; // bomb
		text.push_back("It's a Diamond Long Sword!\n");
		break;
	default:
		break;
	}
	return "";
}


//		WEAPON FUNCTIONS
Weapon::Weapon() {

}
Weapon::Weapon(int x, int y, string action, int dexbonus, int dmg, int range) : Objects(x, y, action), m_action(action), m_dexbonus(dexbonus), m_dmg(dmg), m_range(range) {
	m_bleed = m_burn = m_poison = false;
}
Weapon::Weapon(int x, int y, string action, int dexbonus, int dmg, int range, string affliction) : Objects(x, y, action), m_action(action), m_dexbonus(dexbonus), m_dmg(dmg), m_range(range) {
	m_bleed = m_burn = m_poison = false;
	if (affliction == "bleed")
		m_bleed = true;
	if (affliction == "burn")
		m_burn = true;
	if (affliction == "poison")
		m_poison = true;
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
int Weapon::getRange() const {
	return m_range;
}
bool Weapon::hasBleed() const {
	return m_bleed;
}
bool Weapon::hasBurn() const {
	return m_burn;
}
bool Weapon::hasPoison() const {
	return m_poison;
}


//		SHORT SWORD FUNCTIONS
ShortSword::ShortSword() : Weapon(randInt(68) + 1, randInt(16) + 1, "Short Sword", 1, 2, 1) {

}
ShortSword::~ShortSword() {

}


//		RUSTY CUTLASS
RustyCutlass::RustyCutlass() : Weapon(randInt(68) + 1, randInt(16) + 1, "Rusty Cutlass", 2, 1, 1) {

}
RustyCutlass::RustyCutlass(int x, int y) : Weapon(x, y, "Rusty Cutlass", 2, 1, 1) {

}


//		BONE AXE 
BoneAxe::BoneAxe(int x, int y) : Weapon(x, y, "Bone Axe", 1, 3, 1) {

}


//		WOOD BOW
WoodBow::WoodBow() : Weapon(randInt(68) + 1, randInt(16) + 1, "Wood Bow", 2, 2, 8) {

}


//		BRONZE DAGGER
BronzeDagger::BronzeDagger() : Weapon(randInt(68) + 1, randInt(16) + 1, "Bronze Dagger", 2, 1, 1, "bleed") {

}


//		IRON LANCE
IronLance::IronLance() : Weapon(randInt(68) + 1, randInt(16) + 1, "Iron Lance", 2, 1, 2) {

}


//
//
//		BOSS WEAPONS
SmashersFists::SmashersFists() : Weapon(1,1, "his body", 5, 0, 1) {

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

//		TRIGGERED SPIKES
TriggerSpike::TriggerSpike(int x, int y) : Traps(x, y, "trigger spike trap", 5), m_triggered(false) {

}

bool TriggerSpike::isTriggered() const {
	return m_triggered;
}
void TriggerSpike::toggleTrigger() {
	m_triggered = !m_triggered;
}


//		PLAYER/MONSTER STATUS
//

//		BURN
Burn::Burn() : Objects("burn"), m_maxburn(4), m_burn(4) {

}

void Burn::burn(Actors &a, std::vector<std::string> &text) {
	a.setHP(a.getHP() - 1);
	m_burn--;
	text.push_back("The flame burns...\n");

	if (m_burn == 0)
		a.toggleBurn();
}
int Burn::maxBurns() const {
	return m_maxburn;
}
int Burn::burnsLeft() const {
	return m_burn;
}
void Burn::setBurn(int burn) {
	m_burn = burn;
}


//		HEAL
HealOverTime::HealOverTime() : Objects("heal"), m_heals(20) {

}

void HealOverTime::heal(Player &p, std::vector<std::string> &text) {
	if (p.getHP() < p.getMaxHP())
		p.setHP(p.getHP() + 1);
	m_heals--;
}
int HealOverTime::healsLeft() const {
	return m_heals;
}
void HealOverTime::setHeals(int heals) {
	m_heals = heals;
}


//		BLEED
Bleed::Bleed() : Objects("bleed"), m_maxbleed(5), m_bleed(5), m_maxwait(3), m_wait(3) {

}

void Bleed::bleed(Actors &a, std::vector<std::string> &text) {
	if (m_wait == 0) {
		a.setHP(a.getHP() - 1);
		a.setBleed(a.bleedLeft() - 1);
		m_bleed--;
		text.push_back("Blood drips from the " + a.getName() + "...\n");

		if (m_bleed == 0) {
			a.toggleBleed();
		}
		setWait(maxWait());
	}
	else
		setWait(getWait() - 1);
}
int Bleed::maxBleed() const {
	return m_maxbleed;
}
int Bleed::bleedLeft() const {
	return m_bleed;
}
void Bleed::setBleed(int bleed) {
	m_bleed = bleed;
}
int Bleed::maxWait() const {
	return m_maxwait;
}
int Bleed::getWait() const {
	return m_wait;
}
void Bleed::setWait(int wait) {
	m_wait = wait;
}