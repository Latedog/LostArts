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
Actors::Actors() : m_x(0), m_y(0), m_hp(0), m_armor(0), m_str(0), m_dex(0), m_wep(ShortSword()) {

}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, string name) : m_x(x), m_y(y), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_name(name), \
	m_burned(false), m_bleed(false), m_poisoned(false) {

}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, Weapon wep)
	: m_x(x), m_y(y), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_wep(wep), m_burned(false), m_bleed(false), m_poisoned(false) {

}
Actors::Actors(int x, int y, int hp, int armor, int str, int dex, Weapon wep, string name)
	: m_x(x), m_y(y), m_hp(hp), m_armor(armor), m_str(str), m_dex(dex), m_wep(wep), m_name(name), m_burned(false), m_bleed(false), m_poisoned(false) {

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
std::string Actors::getName() const {
	return m_name;
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

bool Actors::isBurned() const {
	return m_burned;
}
void Actors::toggleBurn() {
	m_burned = !m_burned;
}
int Actors::burnsLeft() const {
	return m_burncount;
}
void Actors::setBurn(int burn) {
	m_burncount = burn;
}

bool Actors::isBled() const {
	return m_bleed;
}
void Actors::toggleBleed() {
	m_bleed = !m_bleed;
}
int Actors::bleedLeft() const {
	return m_bleedcount;
}
void Actors::setBleed(int bleed) {
	m_bleedcount = bleed;
}


//		PLAYER FUNCTIONS
Player::Player() : Actors(randInt(68) + 1, randInt(16) + 1, 5000, 2, 2, 2, ShortSword()) {
	m_invsize = 0;
	m_iteminvsize = 0;
	m_maxhp = 50;
	m_winner = false;
}
Player::~Player() {

}

void Player::attack(std::vector<std::shared_ptr<Monster>> &monsters, std::vector<std::shared_ptr<Objects>> &actives, int pos, std::vector<std::string> &text) {
	text.push_back("You swing your " + getWeapon().getAction() + " and... ");

	int playerPoints = getDex() + getWeapon().getDexBonus();
	int monsterPoints = monsters.at(pos)->getDex();

	// attack roll successful
	if (randInt(playerPoints) >= randInt(monsterPoints)) {
		int damage = 1 + randInt(getStr() + getWeapon().getDmg()) - monsters.at(pos)->getArmor();
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

			text.push_back("deal " + to_string(damage) + " damage to the " + monsters.at(pos)->getName() + "!\n");
			text.push_back("The " + monster + " has " + to_string(monsters.at(pos)->getHP()) + " HP remaining.\n\n");


			// :::: Check for affliction properties of weapon ::::

			// if weapon can cause bleeding and monster is not already bled
			if (getWeapon().hasBleed() && !monsters.at(pos)->isBled()) {
				int bleedchance = 1 + randInt(5);
				// 40% chance to bleed
				if (bleedchance > 3) {
					Bleed bleed;
					actives.emplace_back(new Bleed());
					monsters.at(pos)->toggleBleed();
					monsters.at(pos)->setBleed(bleed.maxBleed());
					text.push_back("The " + monster + " was cut deep!\n");
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

std::vector<Weapon> Player::getWeaponInv() const {
	return m_inv;
}
std::vector<Drops> Player::getItemInv() const {
	return m_iteminv;
}
int Player::getInventorySize() const {
	return m_invsize;
}
int Player::getItemInvSize() const {
	return m_iteminvsize;
}
int Player::getMaxWeaponInvSize() const {
	return m_maxwepinv;
}
int Player::getMaxItemInvSize() const {
	return m_maxiteminv;
}

void Player::addWeapon(Weapon w) {
	m_inv.push_back(w);
	m_invsize++;
}
void Player::addItem(Drops drop) {
	m_iteminv.push_back(drop);
	m_iteminvsize++;
}
void Player::wield(std::vector<std::string> &text) {
	unsigned char c;
	if (m_invsize == 0)
		cout << "You have no weapons to swap to.\n";
	else {
		cout << "Choose an item to wield.\n" << endl;
		showWeapons(text);

		c = getCharacter();

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
void Player::wield(int index) {
	Weapon old = getWeapon();
	setWeapon(m_inv[index]);				// switch to new weapon
	m_inv.erase(m_inv.begin() + index);		// remove item just equipped
	m_inv.push_back(old);				// push old weapon into inventory
}
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
std::string Player::use(std::vector<std::shared_ptr<Objects>> &active, _Tile &tile, int index) {
	string item = m_iteminv.at(index).getItem();
	string image = "cheese.png";

	if (item == "Life Potion") {
		// sound effect
		cocos2d::experimental::AudioEngine::play2d("Life_Potion_Used.mp3", false, 1.0f);

		LifePotion lp;
		lp.changeStats(lp, *this);
	}
	else if (item == "Armor") {
		cocos2d::experimental::AudioEngine::play2d("Armor_Use.mp3", false, 1.0f);

		ArmorDrop armor;
		armor.changeStats(armor, *this);
	}
	else if (item == "Stat Potion") {
		// sound effect
		cocos2d::experimental::AudioEngine::play2d("Potion_Used.mp3", false, 1.0f);

		StatPotion sp;
		sp.changeStats(sp, *this);
	}
	else if (item == "Bomb") {
		cocos2d::experimental::AudioEngine::play2d("Bomb_Placed.mp3", false, 1.0f);

		Bomb bomb;
		tile.extra = LITBOMB;
		bomb.lightBomb();
		bomb.setPosX(getPosX());
		bomb.setPosY(getPosY());

		active.emplace_back(new Bomb(bomb));

		image = "Bomb_48x48.png";
	}
	else if (item == "Mysterious Trinket") {
		if (tile.bottom == INFINITY_BOX) {
			//cout << "The box was unlocked!\n";

			InfinityBox infbox;
			//infbox.open(tile, text);
		}
		else {
			//cout << "Nothing happened.\n";
			//c = getCharacter();
			return image;
		}
	}
	m_iteminv.erase(m_iteminv.begin() + index);	// remove item just used
	m_iteminvsize--;

	return image;
}
void Player::setMaxHP(int maxhp) {
	m_maxhp = maxhp;
}
int Player::getMaxHP() const {
	return m_maxhp;
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
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, Weapon wep, string name)
	: Actors(x, y, hp, armor, str, dex, wep, name), m_name(name) {
	m_maxhp = hp;
}
Monster::Monster(int x, int y, int hp, int armor, int str, int dex, string name) : Actors(x, y, hp, armor, str, dex, name), m_name(name) {
	m_maxhp = hp;
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
	string weapon = this->getWeapon().getAction();
	if (weapon == "Wood Bow") {
		text.push_back("The " + this->getName() + " fires their " + weapon + " at you... ");
	}
	else
		text.push_back("The " + this->getName() + " slashes their " + weapon + " at you... ");

	int monsterPoints = getDex();
	int playerPoints = p.getDex() + p.getWeapon().getDexBonus();

	if (randInt(monsterPoints) >= randInt(playerPoints)) {
		int damage = 1 + randInt(getStr() + getWeapon().getDmg()) - p.getArmor();
		if (damage > 0) {
			// play got hit sound effect
			std::string sound;
			int n = randInt(6) + 1;
			switch (n) {
			case 1: sound = "Hit_Female1.mp3"; break;
			case 2: sound = "Hit_Female2.mp3"; break;
			case 3: sound = "Hit_Female3.mp3"; break;
			case 4: sound = "Hit_Female4.mp3"; break;
			case 5: sound = "Hit_Female5.mp3"; break;
			}
			cocos2d::experimental::AudioEngine::play2d(sound, false, 1.0f);
			cocos2d::experimental::AudioEngine::play2d("Player_Hit.mp3", false, 1.0f);

			p.setHP(p.getHP() - damage);
			text.push_back("and hits for " + to_string(damage) + " damage!");
			text.push_back((damage >= 3 ? " Ouch!\n" : "\n"));
		}
		else {
			text.push_back("but you brush it off!\n");
		}
		cout << endl;
	}
	else {
		text.push_back("and misses!\n");
	}
}
void Monster::attack(Monster &m) {
	;
}
string Monster::getName() {
	return m_name;
}
void Monster::setMaxHP(int maxhp) {
	m_maxhp = maxhp;
}
int Monster::getMaxHP() const {
	return m_maxhp;
}
cocos2d::Action* Monster::getMove() const {
	return m_move;
}
void Monster::setMove(cocos2d::Action* action) {
	m_move = action;
}


//		GOBLIN FUNCTIONS
Goblin::Goblin(int smelldist) : Monster(randInt(68) + 1, randInt(16) + 1, randInt(5) + 10, 1, 3, 1, ShortSword(), "goblin"), m_smelldist(smelldist) {
	
}
Goblin::~Goblin() {

}

int Goblin::getSmellDistance() const {
	return m_smelldist;
}


//		WANDERER FUNCTIONS
Wanderer::Wanderer() : Monster(randInt(68) + 1, randInt(16) + 1, 8, 2, 3, 1, ShortSword(), "wanderer") {

}
Wanderer::~Wanderer(){

}


//		ARCHER FUNCTIONS
Archer::Archer() : Monster(randInt(68) + 1, randInt(16) + 1, randInt(5) + 7, 2, 3, 2, WoodBow(), "archer"), m_primed(false) {

}

bool Archer::isPrimed() const {
	return m_primed;
}
void Archer::prime(bool p) {
	m_primed = p;
}


//		ZAPPER
Zapper::Zapper() : Monster(randInt(68) + 1, randInt(16) + 1, 8 + randInt(3), 1, 4, 1, "zapper"), m_cooldown(true), m_attack(randInt(2)) {
	for (int i = 0; i < 8; i++) {
		cocos2d::Sprite* spark = cocos2d::Sprite::create("Spark.png");
		sparks.insert(std::pair<int, cocos2d::Sprite*>(i, spark));
	}
}

void Zapper::attack(Player &p, std::vector<std::string> &text) {
	cocos2d::experimental::AudioEngine::play2d("Shock5.mp3", false, 1.0f);

	text.push_back("The " + this->getName() + " shocks you... ");

	int damage = 1 + randInt(getStr());
	p.setHP(p.getHP() - damage);
	text.push_back("and does " + to_string(damage) + " damage!");
	text.push_back((damage >= 3 ? " Ouch!\n" : "\n"));
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
Spinner::Spinner() : Monster(randInt(68) + 1, randInt(16) + 1, 8 + randInt(3), 1, 3, 1, "spinner") {
	bool dir = randInt(2);

	m_clockwise = dir;
	m_angle = 1 + randInt(8);

	inner = cocos2d::Sprite::create("Spinner_Buddy_48x48.png");
	outer = cocos2d::Sprite::create("Spinner_Buddy_48x48.png");
}

void Spinner::attack(Player &p, std::vector<std::string> &text) {
	cocos2d::experimental::AudioEngine::play2d("Fire3.mp3", false, 1.0f);

	text.push_back("The " + this->getName() + " hits you for 3 damage.\n");
	p.setHP(p.getHP() - 3);
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
cocos2d::Sprite* Spinner::getInner() const {
	return inner;
}
cocos2d::Sprite* Spinner::getOuter() const {
	return outer;
}



//		BOMBEE
Bombee::Bombee() : Monster(randInt(68) + 1, randInt(16) + 1, 1, 2, 1, 1, "bombee"), m_fuse(3), m_fused(false), m_aggroRange(13) {

}

void Bombee::attack(Player &p, std::vector<std::string> &text) {
	text.push_back("The " + this->getName() + " hits you for 1 damage.\n");
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
MountedKnight::MountedKnight() : Monster(randInt(68) + 1, randInt(16) + 1, 10 + randInt(5), 3, 2, 2, IronLance(), "mounted knight"), m_alerted(false) {

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
Roundabout::Roundabout() : Monster(randInt(68) + 1, randInt(16) + 1, 7 + randInt(2), 1, 5, 1, "roundabout") {
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
	int damage = 1 + randInt(getStr());

	if (damage - p.getArmor() > 0) {
		p.setHP(p.getHP() - (damage - p.getArmor()));
		text.push_back("The " + this->getName() + " hits you for " + to_string(damage - p.getArmor()) + " damage!\n");
	}
	else {
		text.push_back("The " + this->getName() + "bumps into you.\n");
	}
}
char Roundabout::getDirection() const {
	return m_direction;
}
void Roundabout::setDirection(char dir) {
	m_direction = dir;
}


//		SEEKER
Seeker::Seeker(int range) : Monster(randInt(68) + 1, randInt(16) + 1, 8 + randInt(3), 0, 2, 1, ShortSword(), "seeker"), m_range(range), m_step(false) {

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

Smasher::Smasher() : Monster(BOSSCOLS / 2, 3, 500, 0, 4, 1, SmashersFists(), "Smasher"), m_moveActive(false), m_moveEnding(true), m_frenzy(false) {

}

void Smasher::attack(Player &p, std::vector<std::string> &text) {
	cocos2d::experimental::AudioEngine::play2d("Smasher_HitBy1.mp3", false, 1.0f);

	string weapon = this->getWeapon().getAction();
	
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

//		SMASHER SPIKES
