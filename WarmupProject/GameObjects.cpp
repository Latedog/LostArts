#include "GameObjects.h"
#include "utilities.h"
#include <string>

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


//		DROP FUNCTION
Drops::Drops() {

}
void Drops::changeHP() {

}
void changeArmor();
void changeDex();
void changeStr();


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