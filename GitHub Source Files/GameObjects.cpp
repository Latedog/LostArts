#include "cocos2d.h"
#include "AudioEngine.h"
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
Objects::Objects() : m_x(0), m_y(0), m_item(""), m_destructible(false) {

}
Objects::Objects(string status, bool destructible) : m_item(status), m_destructible(destructible) {

}
Objects::Objects(std::string item, std::string image, bool destructible) : m_item(item), m_image(image), m_destructible(destructible) {

}
Objects::Objects(int x, int y, string item, bool destructible) : m_x(x), m_y(y), m_item(item), m_destructible(destructible) {

}
Objects::Objects(int x, int y, std::string item, std::string image, bool destructible) : m_x(x), m_y(y), m_item(item), m_image(image), m_destructible(destructible) {

}
Objects::~Objects() {

}

std::string Objects::getItem() const {
	return m_item;
}
std::string Objects::getImageName() const {
	return m_image;
}
cocos2d::Sprite* Objects::getSprite() {
	return m_sprite;
}
int Objects::getPosX() const {
	return m_x;
}
int Objects::getPosY() const {
	return m_y;
}

void Objects::setSprite(cocos2d::Sprite* sprite) {
	m_sprite = sprite;
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

bool Objects::isDestructible() const {
	return m_destructible;
}
void Objects::setDestructible(bool destructible) {
	m_destructible = destructible;
}


//		IDOL
Idol::Idol() : Objects(randInt(68) + 1, randInt(16) + 1, "Idol") {

}
Idol::~Idol() {

}

//		STAIRS
Stairs::Stairs() : Objects(randInt(68) + 1, randInt(16) + 1, "stairs") {

}

//		DOORS
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

//		BUTTON
Button::Button() : Objects(randInt(68) + 1, randInt(16) + 1, "button") {

}


//	:::: DROPS ::::
//Drops::Drops(int x, int y, std::string item, bool forPlayer) : Objects(x, y, item), m_forPlayer(forPlayer) {
//
//}
Drops::Drops(int x, int y, std::string item, std::string image, bool forPlayer) : Objects(x, y, item, image, false), m_forPlayer(forPlayer) {
	
}
void Drops::changeStats(Drops &drop, Player &p) {
	drop.useItem(p);
}
void Drops::useItem(Drops &drop, Dungeon &dungeon) {
	drop.useItem(dungeon);
}

bool Drops::forPlayer() const {
	return m_forPlayer;
}

Gold::Gold(int x, int y) : Drops(x, y, "Gold", "") {

}

HeartPod::HeartPod() : Drops(randInt(68) + 1, randInt(16) + 1, HEART_POD, "Heart_Pod_48x48.png") {

}
HeartPod::HeartPod(int x, int y) : Drops(x, y, HEART_POD, "Heart_Pod_48x48.png") {

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

LifePotion::LifePotion() : Drops(randInt(68) + 1, randInt(16) + 1, LIFEPOT, "Life_Potion_48x48.png") {
	
}
void LifePotion::useItem(Player &p) {
	// sound effect
	cocos2d::experimental::AudioEngine::play2d("Life_Potion_Used.mp3", false, 1.0f);

	if (p.getHP() < p.getMaxHP()) {			// if player hp is less than their max
		if (p.getHP() + 10 <= p.getMaxHP()) // if player hp + effect of lifepot will refill less than or equal to max
			p.setHP(p.getHP() + 10);
		else	// just set player hp to max
			p.setHP(p.getMaxHP());

		//cout << "You feel replenished." << endl;
		//text.push_back("You feel replenished.\n");
	}
	
	//cout << "Lively as ever..." << endl;
	//text.push_back("Lively as ever...\n");
}

ArmorDrop::ArmorDrop() : Drops(randInt(68) + 1, randInt(16) + 1, ARMOR, "Armor_48x48.png") {

}
void ArmorDrop::useItem(Player &p) {
	cocos2d::experimental::AudioEngine::play2d("Armor_Use.mp3", false, 1.0f);

	p.setArmor(p.getArmor() + 1);
	//cout << "You feel ready for battle." << endl;
	//text.push_back("You feel ready for battle.\n");
}

ShieldRepair::ShieldRepair() : Drops(randInt(68) + 1, randInt(16) + 1, SHIELD_REPAIR, "Shield_Repair_48x48.png") {

}
void ShieldRepair::useItem(Player &p) {
	p.repairShield(10);
}

StatPotion::StatPotion() : Drops(randInt(68) + 1, randInt(16) + 1, STATPOT, "Stat_Potion_48x48.png") {

}
void StatPotion::useItem(Player &p) {
	// sound effect
	cocos2d::experimental::AudioEngine::play2d("Potion_Used.mp3", false, 1.0f);

	p.setDex(p.getDex() + 1);
	p.setStr(p.getStr() + 1);
	p.setMaxHP(p.getMaxHP() + 5);
	//cout << "You feel stronger." << endl;
	//text.push_back("You feel stronger.\n");
}

DizzyElixir::DizzyElixir(int x, int y) : Drops(x, y, DIZZY_ELIXIR, "Dizzy_Elixir_48x48.png") {

}
void DizzyElixir::useItem(Player &p) {
	// sound effect
	cocos2d::experimental::AudioEngine::play2d("Potion_Used.mp3", false, 1.0f);

	p.addAffliction(std::make_shared<Confusion>(20 + p.getInt()));
}

Bomb::Bomb() : Drops(randInt(68) + 1, randInt(16) + 1, BOMB, "Bomb_48x48.png", false), m_fuse(3), m_lit(false) {

}
Bomb::Bomb(std::string type, std::string image) : Drops(randInt(68) + 1, randInt(16) + 1, type, image, false), m_fuse(3), m_lit(false) {

}

void Bomb::useItem(Dungeon &dungeon) {

	if (!isLit()) {
		cocos2d::experimental::AudioEngine::play2d("Bomb_Placed.mp3", false, 1.0f);

		// starts fuse timer
		lightBomb();

		dungeon.getDungeon()[dungeon.getPlayer().getPosY() * dungeon.getCols() + dungeon.getPlayer().getPosX()].extra = LITBOMB;
		setPosX(dungeon.getPlayer().getPosX());
		setPosY(dungeon.getPlayer().getPosY());

		dungeon.getActives().emplace_back(new Bomb(*this));

		// add bomb sprite
		dungeon.addSprite(dungeon.projectile_sprites, dungeon.getRows(), dungeon.getPlayer().getPosX(), dungeon.getPlayer().getPosY(), -1, "Bomb_48x48.png");
	}
	else if (isLit()) {
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

MegaBomb::MegaBomb() : Bomb("Mega Bomb", "Bomb_48x48.png") {

}


//		SKELETON KEY
SkeletonKey::SkeletonKey() : Drops(randInt(68) + 1, randInt(16) + 1, SKELETON_KEY, "Skeleton_Key_48x48.png", false) {

}

void SkeletonKey::useItem(Dungeon &dungeon) {
	// if player is standing over the infinity box, open it
	if (dungeon.getDungeon()[dungeon.getPlayer().getPosY()*dungeon.getCols() + dungeon.getPlayer().getPosX()].item_name == INFINITY_BOX) {
		InfinityBox infbox;
		infbox.open(dungeon.getDungeon()[dungeon.getPlayer().getPosY()*dungeon.getCols() + dungeon.getPlayer().getPosX()]);
	}
}


//		FREEZE SPELL
FreezeSpell::FreezeSpell(int x, int y) : Drops (x, y, FREEZE_SPELL, "Freeze_Spell_48x48.png", false) {

}

void FreezeSpell::useItem(Dungeon &dungeon) {
	// play freeze sound effect
	cocos2d::experimental::AudioEngine::play2d("Freeze_Spell1.mp3", false, 1.0f);
	cocos2d::experimental::AudioEngine::play2d("Freeze_Spell2.mp3", false, 1.0f);

	// add a freeze(stun) effect to all
	for (int i = 0; i < dungeon.getMonsters().size(); i++) {
		dungeon.getMonsters()[i]->addAffliction(std::make_shared<Freeze>(22));
		dungeon.getMonsters()[i]->setFrozen(true);
	}
}

//		EARTHQUAKE SPELL
EarthquakeSpell::EarthquakeSpell(int x, int y) : Drops(x, y, EARTHQUAKE_SPELL, "Earthquake_Spell_48x48.png", false) {

}

void EarthquakeSpell::useItem(Dungeon &dungeon) {
	// play earthquake sound
	cocos2d::experimental::AudioEngine::play2d("Earthquake_Spell1.mp3", false, 1.0f);
	cocos2d::experimental::AudioEngine::play2d("Earthquake_Spell2.mp3", false, 1.0f);
	cocos2d::experimental::AudioEngine::play2d("Earthquake_Explosion1.mp3", false, 1.0f);
	cocos2d::experimental::AudioEngine::play2d("Earthquake_Explosion2.mp3", false, 1.0f);

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
	int mx, my;

	// find any monsters caught in the quake
	for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		
		if (abs(mx - px) <= 2 && abs(my - py) <= 2) {
			dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - 10);

			if (dungeon.getMonsters().at(i)->getHP() <= 0) {
				dungeon.monsterDeath(dungeon.getDungeon(), rows, cols, dungeon.getActives(), dungeon.getMonsters(), i);
			}
			// if monster is still alive, stun them
			else if (dungeon.getMonsters().at(i)->getHP() > 0 && dungeon.getMonsters().at(i)->canBeStunned()) {
				dungeon.getMonsters().at(i)->addAffliction(std::make_shared<Stun>(5));
			}
		}
		
	}

	//	destroy any nearby walls
	for (int j = py - 2; j < py + 3; j++) {
		for (int k = px - 2; k < px + 3; k++) {
			if (j != -1 && j != rows && !(k == -1 && j <= 0) && !(k == cols && j >= rows - 1)) { // boundary check

				if (dungeon.getDungeon()[j*cols + k].top == WALL) {
					dungeon.getDungeon()[j*cols + k].top = SPACE;
					dungeon.getDungeon()[j*cols + k].bottom = SPACE;
					dungeon.getDungeon()[j*cols + k].wall = false;

					// call remove sprite
					dungeon.removeSprite(dungeon.wall_sprites, rows, k, j);
						
				}

				//// destroy any gold in the way
				//if (dungeon.getDungeon()[j*cols + k].gold != 0) {
				//	dungeon.getDungeon()[j*cols + k].gold = 0;

				//	dungeon.removeSprite(dungeon.money_sprites, rows, k, j);
				//}

				// destroy any destructible traps
				if (dungeon[j*cols + k].trap) {
					int pos = dungeon.findTrap(k, j, dungeon.getTraps()); // traps in here
					int pos2 = dungeon.findTrap(k, j, dungeon.getActives()); // traps in there

					if (pos != -1) {
						dungeon.getDungeon()[j*cols + k].trap = false;
						dungeon.getDungeon()[j*cols + k].trap_name = "";

						dungeon.queueRemoveSprite(k, j, dungeon.getTraps().at(pos)->getSprite());
						dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);						
					}
					if (pos2 != -1) {
						dungeon.getDungeon()[j*cols + k].trap = false;
						dungeon.getDungeon()[j*cols + k].trap_name = "";

						dungeon.queueRemoveSprite(k, j, dungeon.getActives().at(pos2)->getSprite());
						dungeon.getActives().erase(dungeon.getActives().begin() + pos2);
					}

					dungeon.getDungeon()[j*cols + k].wall = false; // traps could be classified as walls
				}
			}
		}
	}
	
}

//		FIRE BLAST SPELL
FireBlastSpell::FireBlastSpell(int x, int y) : Drops(x, y, FIREBLAST_SPELL, "Fireblast_Spell_48x48.png", false) {

}

void FireBlastSpell::useItem(Dungeon &dungeon) {
	// play fire blast sound effect
	cocos2d::experimental::AudioEngine::play2d("Fireblast_Spell1.mp3", false, 1.0f);

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	
	// figure out how which direction to iterate
	int n, m; // n : x, m : y
	char playerFacing = dungeon.getPlayer().facingDirection();
	switch (playerFacing) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0;  m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	// while there's no wall or enemy, let the fireball keep traveling
	while (!(dungeon.getDungeon()[(py + m)*cols + px + n].wall || dungeon.getDungeon()[(py + m)*cols + px + n].enemy)) {
		switch (playerFacing) {
		case 'l': n--; m = 0; break;
		case 'r': n++; m = 0; break;
		case 'u': n = 0;  m--; break;
		case 'd': n = 0; m++; break;
		}
	}

	// play fire blast explosion sound effect
	cocos2d::experimental::AudioEngine::play2d("Fireblast_Spell2.mp3", false, 1.0f);

	// find any monsters caught in the blast
	int mx, my;
	for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		if (abs(mx - (px + n)) <= 1 && abs(my - (py + m)) <= 1) {
			dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - 5);

			auto tintRed = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 60, 60));
			auto tintNormal = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 255, 255));
			dungeon.getMonsters().at(i)->getSprite()->runAction(cocos2d::Blink::create(0.2, 5));
			dungeon.getMonsters().at(i)->getSprite()->runAction(cocos2d::Sequence::createWithTwoActions(tintRed, tintNormal));

			//  check if monster died
			if (dungeon.getMonsters().at(i)->getHP() <= 0) {
				dungeon.monsterDeath(dungeon.getDungeon(), rows, cols, dungeon.getActives(), dungeon.getMonsters(), i);
			}
			// otherwise burn the monster if it can be burned
			else if (dungeon.getMonsters().at(i)->getHP() > 0 && dungeon.getMonsters().at(i)->canBeBurned()) {
				dungeon.getMonsters().at(i)->addAffliction(std::make_shared<Burn>(5));
			}
		}

	}
}

//		WIND SPELL
WindSpell::WindSpell(int x, int y) : Drops(x, y, WIND_SPELL, "Wind_Spell_48x48.png", false) {

}

void WindSpell::useItem(Dungeon &dungeon) {
	// play wind sound
	cocos2d::experimental::AudioEngine::play2d("Wind_Spell1.mp3", false, 1.0f);
	cocos2d::experimental::AudioEngine::play2d("Wind_Spell2.mp3", false, 1.0f);

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
	int mx, my, tempx, tempy;

	// find any monsters caught in the whirlwind
	char move;
	int tries = 5; // for breaking out of the loop if too many tries
	bool dead = false; // for stunning only monsters that are alive
	int oldSize = dungeon.getMonsters().size();

	for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
		dead = false;
		tries = 5;
		oldSize = dungeon.getMonsters().size();
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		// if there any monsters within a 2 block radius, push them away
		while (abs(mx - px) <= 3 && abs(my - py) <= 3 && tries > 0) {

			// if monster is to the left of the player and equally as far to the side or less as they are to above or below the player
			if (mx < px && (my - py == 0 || abs(mx - px) <= abs(my - py))) {
				move = 'l';
			}
			// if monster is to the right of the player and equally as far to the side or less as they are to above or below the player
			else if (mx > px && (my - py == 0 || abs(mx - px) <= abs(my - py))) {
				move = 'r';
			}
			// if monster is above the player and equally as far above/below or less as they are to the side of the player
			else if (my < py && (mx - px == 0 || abs(my - py) <= abs(mx - px))) {
				move = 'u';
			}
			// if monster is below the player and equally as far above/below or less as they are to the side of the player
			else if (my > py && (mx - px == 0 || abs(my - py) <= abs(mx - px))) {
				move = 'd';
			}

			// push the monster
			dungeon.pushMonster(dungeon, mx, my, move);

			// check if monster died
			if (dungeon.getMonsters().size() < oldSize) {
				dead = true;
				break;
			}

			// check if there was a trap at this position
			if (dungeon.getDungeon()[dungeon.getMonsters().at(i)->getPosY()*cols + dungeon.getMonsters().at(i)->getPosX()].trap) {
				tempx = dungeon.getMonsters().at(i)->getPosX();
				tempy = dungeon.getMonsters().at(i)->getPosY();
				dungeon.singleMonsterTrapEncounter(dungeon, dungeon.findMonster(tempx, tempy, dungeon.getMonsters()));

				// check if monster died
				if (dungeon.getMonsters().size() < oldSize) {
					dead = true;
					break;
				}
			}

			
			mx = dungeon.getMonsters().at(i)->getPosX();
			my = dungeon.getMonsters().at(i)->getPosY();
			tries--;
		}

		// stun them for one turn at the end
		if (!dead) {
			if (dungeon.getMonsters().at(i)->canBeStunned()) {
				dungeon.getMonsters().at(i)->addAffliction(std::make_shared<Stun>(1));
			}
		}
		else {
			i--; // decrement since monster died to not skip monsters
		}
	}
}

//		INVISIBILITY SPELL
InvisibilitySpell::InvisibilitySpell(int x, int y) : Drops(x, y, INVISIBILITY_SPELL, "Invisibility_Spell_48x48.png") {

}

void InvisibilitySpell::useItem(Player &p) {
	// play invisibility sound effect
	cocos2d::experimental::AudioEngine::play2d("Invisibility_Spell.mp3", false, 1.0f);

	p.addAffliction(std::make_shared<Invisibility>(25));
}

//		ETHEREAL SPELL
EtherealSpell::EtherealSpell(int x, int y) : Drops(x, y, ETHEREAL_SPELL, "Ethereal_Spell_48x48.png") {

}

void EtherealSpell::useItem(Player &p) {
	// play ethereality sound effect
	cocos2d::experimental::AudioEngine::play2d("Ethereal_Spell.mp3", false, 1.0f);

	p.addAffliction(std::make_shared<Ethereality>(15));
}

//		TELEPORT SPELL
Teleport::Teleport(int x, int y) : Drops(x, y, TELEPORT, "Teleport_Scroll_48x48.png", false) {

}

void Teleport::useItem(Dungeon &dungeon) {
	// play teleport sound
	cocos2d::experimental::AudioEngine::play2d("Teleport_Spell.mp3", false, 1.0f);

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = 1 + randInt(cols - 2);
	int y = 1 + randInt(rows - 2);

	bool enemy, wall, trap;
	enemy = dungeon.getDungeon()[y*cols + x].enemy;
	wall = dungeon.getDungeon()[y*cols + x].wall;
	trap = dungeon.getDungeon()[y*cols + x].trap;

	while (enemy || wall || trap) {
		x = 1 + randInt(cols - 2);
		y = 1 + randInt(rows - 2);

		enemy = dungeon.getDungeon()[y*cols + x].enemy;
		wall = dungeon.getDungeon()[y*cols + x].wall;
		trap = dungeon.getDungeon()[y*cols + x].trap;
	}

	dungeon.getDungeon()[dungeon.getPlayer().getPosY()*cols + dungeon.getPlayer().getPosX()].hero = false;
	dungeon.getDungeon()[y*cols + x].hero = true;
	dungeon.getPlayerVector().at(0).setPosX(x);
	dungeon.getPlayerVector().at(0).setPosY(y);

	dungeon.teleportSprite(dungeon.getPlayer().getSprite(), x, y);
}



//		CHESTS
Chests::Chests(std::string chest, std::string image) : Drops(randInt(68) + 1, randInt(16) + 1, chest, image, false) {

}

BrownChest::BrownChest() : Chests(BROWN_CHEST, "Brown_Chest_48x48.png") {

}
std::string BrownChest::open(_Tile &tile, std::vector<std::string> &text) {
	int n = 1 + randInt(3);
	switch (n) {
	case 1:
		tile.item_name = LIFEPOT; // life potion
		//text.push_back(" and find a health potion!\n");
		return "Life_Potion_48x48.png";
		break;
	case 2:
		tile.item_name = ARMOR; // armor
		//text.push_back(" and find Armor!\n");
		return "Armor_48x48.png";
		break;
	//case 3:
	//	tile.bottom = STATPOT; // stat potion
	//	text.push_back(" and find a Stat Potion!\n");
	//	break;
	case 3:
		tile.item_name = BOMB; // stat potion
		//text.push_back(" and find a Bomb!\n");
		return "Bomb_48x48.png";
		break;
	default:
		break;
	}
}

SilverChest::SilverChest() : Chests(SILVER_CHEST, "Silver_Chest_48x48.png") {

}
std::string SilverChest::open(_Tile &tile, std::vector<std::string> &text){
	int n = 1 + randInt(3);
	switch (n) {
	//case 1:
	//	tile.bottom = LIFEPOT; // life potion
	//	text.push_back(" and find a health potion!\n");
	//	break;
	case 1:
		tile.item_name = ARMOR; // armor
		//text.push_back(" and find Armor!\n");
		return "Armor_48x48.png";
		break;
	case 2:
		tile.item_name = STATPOT; // stat potion
		//text.push_back(" and find a Stat Potion!\n");
		return "Stat_Potion_48x48.png";
		break;
	case 3:
		tile.item_name = BOMB; // bomb
		//text.push_back(" and find a Bomb!\n");
		return "Bomb_48x48.png";
		break;
	default:
		break;
	}
}

GoldenChest::GoldenChest() : Chests(GOLDEN_CHEST, "Golden_Chest_48x48.png") {

}
std::string GoldenChest::open(_Tile &tile, std::vector<std::string> &text) {
	int n = 1 + randInt(4);
	switch (n) {
	case 1:
		tile.item_name = LIFEPOT; // life potion
		//text.push_back(" and find a health potion!\n");
		return "Life_Potion_48x48.png";
		break;
	case 2:
		tile.item_name = ARMOR; // armor
		//text.push_back(" and find Armor!\n");
		return "Armor_48x48.png";
		break;
	case 3:
		tile.item_name = STATPOT; // stat potion
		//text.push_back(" and find a Stat Potion!\n");
		return "Stat_Potion_48x48.png";
		break;
	case 4:
		tile.item_name = BOMB; // bomb
		//text.push_back(" and find a Bomb!\n");
		return "Bomb_48x48.png";
		break;
	default:
		break;
	}
}

InfinityBox::InfinityBox() : Chests(INFINITY_BOX, "Golden_Chest_48x48.png") {

}
std::string InfinityBox::open(_Tile &tile) {
	int n = 1 + randInt(4);
	switch (n) {
	case 1:
		tile.item_name = LIFEPOT; // life potion
		//text.push_back("It's a Legendary Crossbow!\n");
		break;
	case 2:
		tile.item_name = ARMOR; // armor
		//text.push_back("It's the Rune of Overwhelming Power!\n");
		break;
	case 3:
		tile.item_name = STATPOT; // stat potion
		//text.push_back("It's a Spell of Destruction!\n");
		break;
	case 4:
		tile.item_name = BOMB; // bomb
		//text.push_back("It's a Diamond Long Sword!\n");
		break;
	default:
		break;
	}
	return "";
}


//	:::: WEAPONS ::::
Weapon::Weapon() {

}
Weapon::Weapon(int x, int y, std::string action, int dexbonus, int dmg, int range, bool ability)
	: Objects(x, y, action), m_action(action), m_dexbonus(dexbonus), m_dmg(dmg), m_range(range), m_hasAbility(ability) {
	m_bleed = m_burn = m_poison = false;
}
Weapon::Weapon(int x, int y, std::string action, std::string image, int dexbonus, int dmg, int range, bool ability)
	: Objects(x, y, action, image), m_action(action), m_dexbonus(dexbonus), m_dmg(dmg), m_range(range), m_hasAbility(ability) {
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

void Weapon::useAbility(Weapon &weapon, Dungeon &dungeon, Actors &a) {
	weapon.useAbility(dungeon, a);
}
void Weapon::useAbility(Dungeon &dungeon, Actors &a) {
	;
}

bool Weapon::hasAbility() const {
	return m_hasAbility;
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

//		SHORT SWORD
ShortSword::ShortSword() : Weapon(randInt(68) + 1, randInt(16) + 1, SHORT_SWORD, "Short_Sword_48x48.png", 1, 3, 1) {

}

//		RUSTY CUTLASS
RustyCutlass::RustyCutlass() : Weapon(randInt(68) + 1, randInt(16) + 1, RUSTY_CUTLASS, "Rusty_Broadsword_48x48.png", 2, 2, 1) {

}
RustyCutlass::RustyCutlass(int x, int y) : Weapon(x, y, RUSTY_CUTLASS, "Rusty_Broadsword_48x48.png", 2, 1, 1) {

}

//		BONE AXE 
BoneAxe::BoneAxe(int x, int y) : Weapon(x, y, BONE_AXE, "Bone_Axe_48x48.png", 0, 4, 1) {

}

//		WOOD BOW
WoodBow::WoodBow() : Weapon(randInt(68) + 1, randInt(16) + 1, WOOD_BOW, "Wood_Bow_48x48.png", 2, 2, 8) {

}

//		BRONZE DAGGER
BronzeDagger::BronzeDagger() : Weapon(randInt(68) + 1, randInt(16) + 1, BRONZE_DAGGER, "Bronze_Dagger_48x48.png", 2, 2, 1, true) {
	m_bleedChance = 40;
}

void BronzeDagger::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBled()) {
		int roll = randInt(100) + a.getLuck();

		// 40% chance to bleed
		if (roll < getBleedChance()) {
			// play bleed sound effect
			cocos2d::experimental::AudioEngine::play2d("Dagger_Slice_Bleed.mp3", false, 0.7f);

			a.addAffliction(std::make_shared<Bleed>());
			a.setBleed(true);
		}
	}
}

int BronzeDagger::getBleedChance() const {
	return m_bleedChance;
}

//		IRON LANCE
IronLance::IronLance() : Weapon(randInt(68) + 1, randInt(16) + 1, IRON_LANCE, "Iron_Lance_48x48.png", 2, 2, 2) {

}

///		BOSS WEAPONS
SmashersFists::SmashersFists() : Weapon(1, 1, "his body", 5, 0, 1) {

}


//	:::: SHIELDS ::::
Shield::Shield() : Objects("No shield") {

}
Shield::Shield(int defense, int durability, int coverage, std::string type, std::string image)
	: Objects(type, image), m_defense(defense), m_durability(durability), m_coverage(coverage) {
	m_max_durability = durability;
}
Shield::Shield(int x, int y, int defense, int durability, int coverage, std::string type, std::string image)
	: Objects(x, y, type, image), m_defense(defense), m_durability(durability), m_coverage(coverage) {
	m_max_durability = durability;
}
Shield::Shield(int defense, int durability, int max_durability, int coverage, std::string type, std::string image)
	: Objects(type, image), m_defense(defense), m_durability(durability), m_coverage(coverage) {
	m_max_durability = max_durability;
}

int Shield::getDefense() const {
	return m_defense;
}
void Shield::setDefense(int defense) {
	m_defense = defense;
}
int Shield::getDurability() const {
	return m_durability;
}
void Shield::setDurability(int durability) {
	m_durability = durability;
}
int Shield::getMaxDurability() const {
	return m_max_durability;
}
void Shield::setMaxDurability(int durability) {
	m_max_durability = durability;
}
int Shield::getCoverage() const {
	return m_coverage;
}
void Shield::setCoverage(int coverage) {
	m_coverage = coverage;
}

//						Shield(defense, durability, coverage, name)
WoodShield::WoodShield() : Shield(2, 10, 1, WOOD_SHIELD, "Wood_Shield_48x48.png") {

}
WoodShield::WoodShield(Shield const shield)
	: Shield(shield.getDefense(), shield.getDurability(), shield.getMaxDurability(), shield.getCoverage(), shield.getItem(), shield.getImageName()) {
}
WoodShield::WoodShield(int x, int y) : Shield(x, y, 2, 10, 1, WOOD_SHIELD, "Wood_Shield_48x48.png") {

}

IronShield::IronShield() : Shield(4, 30, 1, IRON_SHIELD, "Iron_Shield_48x48.png") {

}
IronShield::IronShield(Shield const shield)
	: Shield(shield.getDefense(), shield.getDurability(), shield.getMaxDurability(), shield.getCoverage(), shield.getItem(), shield.getImageName()) {
}
IronShield::IronShield(int x, int y) : Shield(x, y, 4, 30, 1, IRON_SHIELD, "Iron_Shield_48x48.png") {

}



//	:::: TRAPS ::::
Traps::Traps(int x, int y, std::string name, int damage, bool destructible) : Objects (x, y, name, destructible), m_trapdmg(damage) {
	
}
int Traps::getDmg() const {
	return m_trapdmg;
}
void Traps::setDmg(int damage) {
	m_trapdmg = damage;
}


//		PIT
Pit::Pit() : Traps(1 + randInt(68), 1 + randInt(16), "pit", 1000) {

}
Pit::Pit(int x, int y) : Traps(x, y, "pit", 1000) {

}

void Pit::fall(Actors &a) {
	// play falling sound effect
	cocos2d::experimental::AudioEngine::play2d("Female_Falling_Scream_License.mp3", false, 1.0f);

	// instant death from falling
	a.setHP(0);
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
	m_deactive = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Deactive_48x48.png");
	m_primed = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Primed_48x48.png");
	m_active = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Active_48x48.png");

	setSprite(m_deactive);
}
SpikeTrap::SpikeTrap(int x, int y, int speed) : Traps(x, y, "spike trap", 3), m_cyclespeed(speed), m_countdown(speed) {
	m_deactive = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Deactive_48x48.png");
	m_primed = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Primed_48x48.png");
	m_active = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Active_48x48.png");

	setSprite(m_deactive);
	setSpriteVisibility(true, false, false);
}

void SpikeTrap::cycle(Actors &a) {
	int ax = a.getPosX();
	int ay = a.getPosY();

	int tx = getPosX();
	int ty = getPosY();

	// if spiketrap was primed, reset the counter
	if (getCountdown() == 0 && m_active->isVisible()) {
		setSprite(m_deactive);
		setSpriteVisibility(true, false, false);

		setCountdown(getSpeed());

		//return;
	}

	// spiketrap is counting down
	if (getCountdown() > 1) {
		setCountdown(getCountdown() - 1);
	}
	// prime the spiketrap
	else if (getCountdown() == 1) {
		// spiketrap is primed

		// if player is too far, don't play the sound
		if (abs(ax - tx) + abs(ay - ty) < 12) {
			cocos2d::experimental::AudioEngine::play2d("Spiketrap_Primed.mp3", false, (float)exp(-(abs(ax - tx) + abs(ay - ty)) / 2));
		}

		setSprite(m_primed);
		setSpriteVisibility(false, true, false);

		setCountdown(getCountdown() - 1);
	}
	// spiketrap is launched
	else {

		// if player is too far, don't play the sound
		if (abs(ax - tx) + abs(ay - ty) < 12) {
			cocos2d::experimental::AudioEngine::play2d("Spiketrap_Active.mp3", false, (float)exp(-(abs(ax - tx) + abs(ay - ty)) / 2));
		}

		setSprite(m_active);
		setSpriteVisibility(false, false, true);

		// check if actor was hit
		if (ax == tx && ay == ty) {
			cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

			a.setHP(a.getHP() - getDmg());
		}
	}
}
void SpikeTrap::setSpriteVisibility(bool deactive, bool primed, bool active) {
	if (deactive) {
		m_deactive->setVisible(true);
		m_primed->setVisible(false);
		m_active->setVisible(false);
	}
	else if (primed) {
		m_deactive->setVisible(false);
		m_primed->setVisible(true);
		m_active->setVisible(false);
	}
	else if (active) {
		m_deactive->setVisible(false);
		m_primed->setVisible(false);
		m_active->setVisible(true);
	}
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

cocos2d::Sprite* SpikeTrap::getSpriteD() {
	return m_deactive;
}
cocos2d::Sprite* SpikeTrap::getSpriteP() {
	return m_primed;
}
cocos2d::Sprite* SpikeTrap::getSpriteA() {
	return m_active;
}

//		TRIGGERED SPIKES
TriggerSpike::TriggerSpike(int x, int y) : Traps(x, y, "trigger spike trap", 5), m_triggered(false) {
	m_deactive = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Deactive_48x48.png");
	m_primed = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Primed_48x48.png");
	m_active = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Active_48x48.png");

	setSprite(m_deactive);
}

void TriggerSpike::trigger(Actors &a) {
	int ax = a.getPosX();
	int ay = a.getPosY();

	int tx = getPosX();
	int ty = getPosY();

	// if the spike trap was triggered
	if (isTriggered()) {
		// spiketrap is launched
		if (abs(ax - tx) + abs(ay - ty) < 15) {
			cocos2d::experimental::AudioEngine::play2d("Spiketrap_Active.mp3", false, 1.0f);
		}
		
		setSprite(m_active);
		setSpriteVisibility(false, false, true);

		//	check if player was still on top
		if (ax == tx && ay == ty) {
			cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

			a.setHP(a.getHP() - getDmg());
		}

		toggleTrigger();
	}
	else {
		// else if the trap wasn't triggered, check if player is standing on top of it
		if (ax == tx && ay == ty) {
			toggleTrigger();

			// spiketrap is primed
			cocos2d::experimental::AudioEngine::play2d("Spiketrap_Primed.mp3", false, 1.0f);
			
			setSprite(m_primed);
			setSpriteVisibility(false, true, false);
		}
		else {
			// retract spiketrap
			cocos2d::experimental::AudioEngine::play2d("Spiketrap_Deactive.mp3", false, 1.0f);
			
			setSprite(m_deactive);
			setSpriteVisibility(true, false, false);
		}
	}
}
void TriggerSpike::setSpriteVisibility(bool deactive, bool primed, bool active) {
	if (deactive) {
		m_deactive->setVisible(true);
		m_primed->setVisible(false);
		m_active->setVisible(false);
	}
	else if (primed) {
		m_deactive->setVisible(false);
		m_primed->setVisible(true);
		m_active->setVisible(false);
	}
	else if (active) {
		m_deactive->setVisible(false);
		m_primed->setVisible(false);
		m_active->setVisible(true);
	}
}

bool TriggerSpike::isTriggered() const {
	return m_triggered;
}
void TriggerSpike::toggleTrigger() {
	m_triggered = !m_triggered;
}

cocos2d::Sprite* TriggerSpike::getSpriteD() {
	return m_deactive;
}
cocos2d::Sprite* TriggerSpike::getSpriteP() {
	return m_primed;
}
cocos2d::Sprite* TriggerSpike::getSpriteA() {
	return m_active;
}

//		PUDDLES
Puddle::Puddle(int x, int y) : Traps(x, y, "puddle", 0, true) {

}

void Puddle::slip(Actors &a) {
	int roll = randInt(100) + a.getLuck();

	// splash sound effect
	cocos2d::experimental::AudioEngine::play2d("Puddle_Splash.mp3", false, 0.7f);

	// if player was on fire, put it out
	if (a.isBurned()) {
		// play relief sound
		cocos2d::experimental::AudioEngine::play2d("Relief_Female.mp3", false, 0.7f);

		a.setBurned(false);

		int i = a.findAffliction("burn");

		if (i == -1)
			return;

		//a.getAfflictions().at(i)->setExhaustion(true);
		a.getAfflictions().erase(a.getAfflictions().begin() + i);
	}

	// failed the save roll
	if (roll < 40) {

		// if actor can be stunned and they aren't stunned already
		if (a.canBeStunned() && !a.isStunned()) {
			// play slip sound effect
			cocos2d::experimental::AudioEngine::play2d("Puddle_Slip.mp3", false, 1.0f);

			// turn sprite sideways
			a.getSprite()->setRotation(90);
			
			a.setStunned(true);
			a.addAffliction(std::make_shared<Stun>());
		}
		// if actor can be stunned and they're already stunned, add more time onto the stun
		else if (a.canBeStunned() && a.isStunned()) {
			// play slip sound effect
			cocos2d::experimental::AudioEngine::play2d("Puddle_Slip.mp3", false, 1.0f);

			int index = a.findAffliction("stun");

			// if affliction wasn't found, then return
			if (index == -1)
				return;

			// add one more turn onto the stun
			a.getAfflictions().at(index)->setTurnsLeft(a.getAfflictions().at(index)->getTurnsLeft() + 1);
		}

		// else if they're not stunnable, nothing happens
		return;
	}
}

//		FIREBAR
Firebar::Firebar(int x, int y) : Traps(x, y, "firebar", 5) {
	m_clockwise = randInt(2);
	m_angle = 1 + randInt(8);

	m_inner = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	m_outer = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
}
Firebar::Firebar(int x, int y, std::string firebar) : Traps(x, y, firebar, 5) {
	m_clockwise = randInt(2);
	m_angle = 1 + randInt(8);
}

void Firebar::burn(Actors &a) {
	cocos2d::experimental::AudioEngine::play2d("Fire3.mp3", false, 1.0f);

	a.setHP(a.getHP() - this->getDmg());

	// if actor is still alive and can be burned, try to burn them
	if (a.getHP() > 0 && a.canBeBurned()) {

		// if actor isn't yet burned, burn them
		//if (!a.isBurned()) {
			// chance to burn
			int roll = randInt(100) + a.getLuck();

			// failed the save roll
			if (roll < 80) {
				//a.setBurned(true);
				a.addAffliction(std::make_shared<Burn>());
			}
		//}
		//// if they're already burned, add more burn time
		//else if (a.isBurned()) {

		//	int index = a.findAffliction("burn");

		//	// if affliction wasn't found, then return
		//	if (index == -1)
		//		return;

		//	// add three more turns onto the burn
		//	a.getAfflictions().at(index)->setTurnsLeft(a.getAfflictions().at(index)->getTurnsLeft() + 3);
		//}
	}

}

void Firebar::setInitialFirePosition(int x, int y) {

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
void Firebar::setFirePosition(char move) {
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

int Firebar::getAngle() const {
	return m_angle;
}
void Firebar::setAngle(int angle) {
	m_angle = angle;
}
bool Firebar::isClockwise() const {
	return m_clockwise;
}

bool Firebar::playerWasHit(const Actors &a) {
	if ((m_innerFire.getPosX() == a.getPosX() && m_innerFire.getPosY() == a.getPosY()) ||
		(m_outerFire.getPosX() == a.getPosX() && m_outerFire.getPosY() == a.getPosY()))
		return true;

	return false;
}

void Firebar::setSpriteVisibility(bool visible) {
	m_inner->setVisible(visible);
	m_outer->setVisible(visible);
}
cocos2d::Sprite* Firebar::getInner() {
	return m_inner;
}
cocos2d::Sprite* Firebar::getOuter() {
	return m_outer;
}

//		DOUBLE FIREBAR
DoubleFirebar::DoubleFirebar(int x, int y) : Firebar(x, y, "double firebar") {
	m_inner = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	m_innerMirror = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	m_outer = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	m_outerMirror = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
}

void DoubleFirebar::setInitialFirePosition(int x, int y) {
	if (isClockwise()) {
		switch (getAngle()) {
		case 1:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y - 2);

			m_innerFireMirror.setPosX(x + 1); m_innerFireMirror.setPosY(y + 1);
			m_outerFireMirror.setPosX(x + 2); m_outerFireMirror.setPosY(y + 2);
			break;
		case 2:
			m_innerFire.setPosX(x); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x); m_outerFire.setPosY(y - 2);

			m_innerFireMirror.setPosX(x); m_innerFireMirror.setPosY(y + 1);
			m_outerFireMirror.setPosX(x); m_outerFireMirror.setPosY(y + 2);
			break;
		case 3:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y - 2);

			m_innerFireMirror.setPosX(x - 1); m_innerFireMirror.setPosY(y + 1);
			m_outerFireMirror.setPosX(x - 2); m_outerFireMirror.setPosY(y + 2);
			break;
		case 4:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y);

			m_innerFireMirror.setPosX(x - 1); m_innerFireMirror.setPosY(y);
			m_outerFireMirror.setPosX(x - 2); m_outerFireMirror.setPosY(y);
			break;
		case 5:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y + 2);

			m_innerFireMirror.setPosX(x - 1); m_innerFireMirror.setPosY(y - 1);
			m_outerFireMirror.setPosX(x - 2); m_outerFireMirror.setPosY(y - 2);
			break;
		case 6:
			m_innerFire.setPosX(x); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x); m_outerFire.setPosY(y + 2);

			m_innerFireMirror.setPosX(x); m_innerFireMirror.setPosY(y - 1);
			m_outerFireMirror.setPosX(x); m_outerFireMirror.setPosY(y - 2);
			break;
		case 7:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y + 2);

			m_innerFireMirror.setPosX(x + 1); m_innerFireMirror.setPosY(y - 1);
			m_outerFireMirror.setPosX(x + 2); m_outerFireMirror.setPosY(y - 2);
			break;
		case 8:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y);

			m_innerFireMirror.setPosX(x + 1); m_innerFireMirror.setPosY(y);
			m_outerFireMirror.setPosX(x + 2); m_outerFireMirror.setPosY(y);
			break;
		}
	}
	else {
		switch (getAngle()) {
		case 1:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y - 2);

			m_innerFireMirror.setPosX(x - 1); m_innerFireMirror.setPosY(y + 1);
			m_outerFireMirror.setPosX(x - 2); m_outerFireMirror.setPosY(y + 2);
			break;
		case 2:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y);

			m_innerFireMirror.setPosX(x - 1); m_innerFireMirror.setPosY(y);
			m_outerFireMirror.setPosX(x - 2); m_outerFireMirror.setPosY(y);
			break;
		case 3:
			m_innerFire.setPosX(x + 1); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x + 2); m_outerFire.setPosY(y + 2);

			m_innerFireMirror.setPosX(x - 1); m_innerFireMirror.setPosY(y - 1);
			m_outerFireMirror.setPosX(x - 2); m_outerFireMirror.setPosY(y - 2);
			break;
		case 4:
			m_innerFire.setPosX(x); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x); m_outerFire.setPosY(y + 2);

			m_innerFireMirror.setPosX(x); m_innerFireMirror.setPosY(y - 1);
			m_outerFireMirror.setPosX(x); m_outerFireMirror.setPosY(y - 2);
			break;
		case 5:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y + 1);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y + 2);

			m_innerFireMirror.setPosX(x + 1); m_innerFireMirror.setPosY(y - 1);
			m_outerFireMirror.setPosX(x + 2); m_outerFireMirror.setPosY(y - 2);
			break;
		case 6:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y);

			m_innerFireMirror.setPosX(x + 1); m_innerFireMirror.setPosY(y);
			m_outerFireMirror.setPosX(x + 2); m_outerFireMirror.setPosY(y);
			break;
		case 7:
			m_innerFire.setPosX(x - 1); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x - 2); m_outerFire.setPosY(y - 2);

			m_innerFireMirror.setPosX(x + 1); m_innerFireMirror.setPosY(y + 1);
			m_outerFireMirror.setPosX(x + 2); m_outerFireMirror.setPosY(y + 2);
			break;
		case 8:
			m_innerFire.setPosX(x); m_innerFire.setPosY(y - 1);
			m_outerFire.setPosX(x); m_outerFire.setPosY(y - 2);

			m_innerFireMirror.setPosX(x); m_innerFireMirror.setPosY(y + 1);
			m_outerFireMirror.setPosX(x); m_outerFireMirror.setPosY(y + 2);
			break;
		}
	}
}
bool DoubleFirebar::playerWasHit(const Actors &a) {
	if ((m_innerFire.getPosX() == a.getPosX() && m_innerFire.getPosY() == a.getPosY()) ||
		(m_outerFire.getPosX() == a.getPosX() && m_outerFire.getPosY() == a.getPosY()) ||
		(m_innerFireMirror.getPosX() == a.getPosX() && m_innerFireMirror.getPosY() == a.getPosY()) ||
		(m_outerFireMirror.getPosX() == a.getPosX() && m_outerFireMirror.getPosY() == a.getPosY()))
		return true;

	return false;
}
void DoubleFirebar::setFirePosition(char move) {
	switch (move) {
	case 'l':
		m_innerFire.setPosX(m_innerFire.getPosX() - 1);
		m_outerFire.setPosX(m_outerFire.getPosX() - 2);
		m_innerFireMirror.setPosX(m_innerFireMirror.getPosX() + 1);
		m_outerFireMirror.setPosX(m_outerFireMirror.getPosX() + 2);
		break;
	case 'r':
		m_innerFire.setPosX(m_innerFire.getPosX() + 1);
		m_outerFire.setPosX(m_outerFire.getPosX() + 2);
		m_innerFireMirror.setPosX(m_innerFireMirror.getPosX() - 1);
		m_outerFireMirror.setPosX(m_outerFireMirror.getPosX() - 2);
		break;
	case 'u':
		m_innerFire.setPosY(m_innerFire.getPosY() - 1);
		m_outerFire.setPosY(m_outerFire.getPosY() - 2);
		m_innerFireMirror.setPosY(m_innerFireMirror.getPosY() + 1);
		m_outerFireMirror.setPosY(m_outerFireMirror.getPosY() + 2);
		break;
	case 'd':
		m_innerFire.setPosY(m_innerFire.getPosY() + 1);
		m_outerFire.setPosY(m_outerFire.getPosY() + 2);
		m_innerFireMirror.setPosY(m_innerFireMirror.getPosY() - 1);
		m_outerFireMirror.setPosY(m_outerFireMirror.getPosY() - 2);
		break;
	}
}

void DoubleFirebar::setSpriteVisibility(bool visible) {
	m_inner->setVisible(visible);
	m_innerMirror->setVisible(visible);
	m_outer->setVisible(visible);
	m_outerMirror->setVisible(visible);
}
cocos2d::Sprite* DoubleFirebar::getInner() {
	return m_inner;
}
cocos2d::Sprite* DoubleFirebar::getInnerMirror() {
	return m_innerMirror;
}
cocos2d::Sprite* DoubleFirebar::getOuter() {
	return m_outer;
}
cocos2d::Sprite* DoubleFirebar::getOuterMirror() {
	return m_outerMirror;
}

//		LAVA
Lava::Lava(int x, int y) : Traps(x, y, "Lava", 8) {

}

void Lava::burn(Actors &a) {
	// lava sound
	cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

	a.setHP(a.getHP() - getDmg());

	
	// if actor is still alive and can be burned, try to burn them
	if (a.getHP() > 0 && a.canBeBurned()) {

		// if actor isn't yet burned, attempt roll to burn them
		if (!a.isBurned()) {
			// chance to burn
			int roll = randInt(100) - a.getLuck();

			// failed the save roll
			if (roll < 100) {
				a.setBurned(true);
				a.addAffliction(std::make_shared<Burn>());
			}
		}
		// if they're already burned, add more burn time
		else if (a.isBurned()) {

			int index = a.findAffliction("burn");

			// if affliction wasn't found, then return
			if (index == -1)
				return;

			// add three more turns onto the burn
			a.getAfflictions().at(index)->setTurnsLeft(a.getAfflictions().at(index)->getTurnsLeft() + 3);
		}
	}
	
}

//		SPRINGS
Spring::Spring(int x, int y, bool trigger, char move) : Traps(x, y, "Spring", 0, true) {
	/* Move key:
	*  2 u 1
	*  l x r
	*  3 d 4
	*/

	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd':
	case '1':
	case '2':
	case '3':
	case '4': m_dir = move; break;
	default: {
		// choose random direction
		switch (1 + randInt(8)) {
		case 1: m_dir = 'l'; break;
		case 2: m_dir = 'r'; break;
		case 3: m_dir = 'u'; break;
		case 4: m_dir = 'd'; break;
		case 5: m_dir = '1'; break;
		case 6: m_dir = '2'; break;
		case 7: m_dir = '3'; break;
		case 8: m_dir = '4'; break;
		}
	}
	}
	
	m_isTrigger = trigger;
	m_triggered = false;
	m_multidirectional = false;
}
Spring::Spring(int x, int y, bool trigger, bool known, bool cardinal) : Traps(x, y, "Spring", 0, true) {
	/* Move key:
	*  2 u 1
	*  l x r
	*  3 d 4
	*/

	// multidirectional
	if (!known) {
		m_any = true;
		m_dir = '#'; // used for sprite assigning
	}
	// cardinal
	else if (known && cardinal) {
		m_any = false;
		m_cardinal = true;
		m_dir = '+';
	}
	// diagonal
	else if (known && !cardinal) {
		m_any = false;
		m_cardinal = false;
		m_dir = 'x';
	}

	m_isTrigger = trigger;
	m_triggered = false;
	m_multidirectional = true;
}

void Spring::trigger(Dungeon &dungeon, Actors &a) {
	int x = a.getPosX();
	int y = a.getPosY();

	int tx = getPosX();
	int ty = getPosY();

	int cols = dungeon.getCols();

	// check if player is on top
	if (!(tx == x && ty == y)) {
		m_triggered = false;

		return;
	}

	// check if trigger spring trap
	if (isTrigger()) {

		// if actor is on the same tile as the spring, trigger it
		if (tx == x && ty == y && !triggered()) {
			// play trigger sound effect
			cocos2d::experimental::AudioEngine::play2d("Spring_Trigger.mp3", false, 1.0f);

			m_triggered = true;

			return;
		}
		// if something triggered the spring but stepped off, return and reset the trap to not triggered
		else if (!(tx == x && ty == y) && triggered()) {
			m_triggered = false;

			return;
		}
		// if actor is on the same tile on the spring and it is triggered, continue and reset the trap to not triggered
		else if (tx == x && ty == y && triggered()) {
			m_triggered = false;
		}

	}


	int n = 0, m = 0; //n : x, m : y
	bool wall, enemy, hero;

	// any direction
	if (isMultiDirectional() && isAny()) {
		// choose random direction
		switch (1 + randInt(8)) {
		case 1: n = -1, m = 0; break;
		case 2: n = 1, m = 0; break;
		case 3: n = 0, m = -1; break;
		case 4: n = 0, m = 1; break;
		case 5: n = 1, m = -1; break;
		case 6: n = -1, m = -1; break;
		case 7: n = -1, m = 1; break;
		case 8: n = 1, m = 1; break;
		}
	}
	// multi, but not any direction
	else if (isMultiDirectional() && !isAny()) {
		// cardinal directions only
		if (isCardinal()) {
			switch (1 + randInt(4)) {
			case 1: n = -1, m = 0; break;	// L
			case 2: n = 1, m = 0; break;	// R
			case 3: n = 0, m = -1; break;	// U
			case 4: n = 0, m = 1; break;	// D
			}
		}
		// diagonal directions only
		else {
			switch (1 + randInt(4)) {
			case 1: n = 1, m = -1; break;	// Q1
			case 2: n = -1, m = -1; break;	// Q2
			case 3: n = -1, m = 1; break;	// Q3
			case 4: n = 1, m = 1; break;	// Q4
			}
		}
	}
	// single direction
	else {
		switch (getDirection()) {
		case 'l': n = -1, m = 0; break;
		case 'r': n = 1, m = 0; break;
		case 'u': n = 0, m = -1; break;
		case 'd': n = 0, m = 1; break;
		case '1': n = 1, m = -1; break;	// Q1
		case '2': n = -1, m = -1; break;// Q2
		case '3': n = -1, m = 1; break;	// Q3
		case '4': n = 1, m = 1; break;	// Q4
		}
	}

	wall = dungeon[(y + m)*cols + x + n].wall;
	enemy = dungeon[(y + m)*cols + x + n].enemy;
	hero = dungeon[(y + m)*cols + x + n].hero;

	// if space is free, move the actor there
	if (a.getName() == "Player") {
		if (!(wall || enemy)) {
			// play trigger sound effect
			cocos2d::experimental::AudioEngine::play2d("Spring_Bounce.mp3", false, (float)exp(-(abs(x - tx) + abs(y - ty)) / 2));

			dungeon.getDungeon()[y*cols + x].hero = false;
			dungeon.getDungeon()[(y + m)*cols + x + n].hero = true;
			a.setPosX(x + n); a.setPosY(y + m);

			// queue player move
			dungeon.queueMoveSprite(a.getSprite(), dungeon.getRows(), x + n, y + m);

			// check if there was a trap at this position
			if (dungeon.getDungeon()[(y + m)*cols + x + n].trap) {
				dungeon.trapEncounter(dungeon, x + n, y + m);
			}
		}
	}
	// else it was a monster
	else {
		if (!(wall || enemy || hero)) {
			// play trigger sound effect
			cocos2d::experimental::AudioEngine::play2d("Spring_Bounce.mp3", false, (float)pow((abs(dungeon.getPlayer().getPosX() - tx) + abs(dungeon.getPlayer().getPosY() - ty)), -1));

			dungeon.getDungeon()[y*cols + x].enemy = false;
			dungeon.getDungeon()[(y + m)*cols + x + n].enemy = true;
			a.setPosX(x + n); a.setPosY(y + m);

			dungeon.queueMoveSprite(a.getSprite(), dungeon.getRows(), x + n, y + m);

			// check if there was a trap at this position
			if (dungeon.getDungeon()[(y + m)*cols + x + n].trap) {
				dungeon.singleMonsterTrapEncounter(dungeon, dungeon.findMonster(a.getPosX(), a.getPosY(), dungeon.getMonsters()));
			}
		}
	}
}

char Spring::getDirection() const {
	return m_dir;
}
bool Spring::isTrigger() const {
	return m_isTrigger;
}
bool Spring::triggered() const {
	return m_triggered;
}
bool Spring::isMultiDirectional() const {
	return m_multidirectional;
}
bool Spring::isAny() const {
	return m_any;
}
bool Spring::isCardinal() const {
	return m_cardinal;
}

//		TURRETS
Turret::Turret(int x, int y, char dir, int range) : Traps(x, y, "Turret", 4), m_dir(dir), m_range(range), m_triggered(false) {
	
}

void Turret::shoot(Dungeon &dungeon, Actors &a) {
	int ax = a.getPosX();
	int ay = a.getPosY();

	int tx = getPosX();
	int ty = getPosY();

	// if on cooldown, come off cooldown and return
	if (onCooldown()) {
		m_cooldown = false;
		return;
	}

	// check to get triggered
	if (!isTriggered()) {
		switch (m_dir) {
		case 'l': {
			// if turret and actor are on the same column, and the actor is within shooting range, and there aren't any walls in the way, get triggered
			if (ay == ty && tx - ax >= 1 && tx - ax <= m_range && !dungeon.wallCollision(dungeon.getDungeon(), dungeon.getCols(), 'x', ax, tx)) {
				// play trigger sound effect
				cocos2d::experimental::AudioEngine::play2d("Turret_Trigger.mp3", false, 1.0f);

				m_triggered = true;
				return;
			}
			break;
		}
		case 'r': {
			// if turret and actor are on the same column, and the actor is within shooting range, and there aren't any walls in the way, get triggered
			if (ay == ty && ax - tx >= 1 && ax - tx <= m_range && !dungeon.wallCollision(dungeon.getDungeon(), dungeon.getCols(), 'x', ax, tx)) {
				// play trigger sound effect
				cocos2d::experimental::AudioEngine::play2d("Turret_Trigger.mp3", false, 1.0f);

				m_triggered = true;
				return;
			}
			break;
		}
		case 'u': {
			// if turret and actor are on the same row, and the actor is within shooting range, and there aren't any walls in the way, get triggered
			if (ax == tx && ty - ay >= 1 && ty - ay <= m_range && !dungeon.wallCollision(dungeon.getDungeon(), dungeon.getCols(), 'y', ay, ty)) {
				// play trigger sound effect
				cocos2d::experimental::AudioEngine::play2d("Turret_Trigger.mp3", false, 1.0f);

				m_triggered = true;
				return;
			}
			break;
		}
		case 'd': {
			// if turret and actor are on the same row, and the actor is within shooting range, and there aren't any walls in the way, get triggered
			if (ax == tx && ay - ty >= 1 && ay - ty <= m_range && !dungeon.wallCollision(dungeon.getDungeon(), dungeon.getCols(), 'y', ay, ty)) {
				// play trigger sound effect
				cocos2d::experimental::AudioEngine::play2d("Turret_Trigger.mp3", false, 1.0f);

				m_triggered = true;
				return;
			}
			break;
		}
		}

		return;
	}

	// play shoot sound effect
	cocos2d::experimental::AudioEngine::play2d("Gunshot1.mp3", false, 0.8f);

	// else if it is triggered, shoot in the proper direction
	switch (m_dir) {
	case 'l': {
		//// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
		//if (ay == ty && !dungeon.wallCollision(dungeon.getDungeon(), dungeon.getCols(), 'x', ax, tx)) {
		//	a.setHP(a.getHP() - getDmg());
		//}
		checkLineOfFire(dungeon);
		break;
	}
	case 'r': {
		//// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
		//if (ay == ty && !dungeon.wallCollision(dungeon.getDungeon(), dungeon.getCols(), 'x', ax, tx)) {
		//	a.setHP(a.getHP() - getDmg());
		//}
		checkLineOfFire(dungeon);
		break;
	}
	case 'u': {
		//// if turret and actor are on the same row, and there aren't any walls in the way, shoot them
		//if (ax == tx && !dungeon.wallCollision(dungeon.getDungeon(), dungeon.getCols(), 'y', ay, ty)) {
		//	a.setHP(a.getHP() - getDmg());
		//}
		checkLineOfFire(dungeon);
		break;
	}
	case 'd': {
		//// if turret and actor are on the same row, and there aren't any walls in the way, shoot them
		//if (ax == tx && !dungeon.wallCollision(dungeon.getDungeon(), dungeon.getCols(), 'y', ay, ty)) {
		//	a.setHP(a.getHP() - getDmg());
		//}
		checkLineOfFire(dungeon);
		break;
	}
	}

	m_cooldown = true;
	m_triggered = false;
}
void Turret::checkLineOfFire(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	
	int tx = getPosX();
	int ty = getPosY();

	bool wall, enemy, hero;

	switch (m_dir) {
	case 'l': {
		tx--;
		wall = dungeon.getDungeon()[ty*cols + tx].wall;
		enemy = dungeon.getDungeon()[ty*cols + tx].enemy;
		hero = dungeon.getDungeon()[ty*cols + tx].hero;
		while (!wall) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				int damage = getDmg();
				// if player was blocking and successfully shielded the shot, reduce the damage taken
				if (dungeon.getPlayerVector().at(0).isBlocking() && dungeon.getPlayerVector().at(0).shieldCoverage(getPosX(), getPosY())) {
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield().getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty, dungeon.getMonsters());
				dungeon.getMonsters().at(pos)->setHP(dungeon.getMonsters().at(pos)->getHP() - this->getDmg());

				return; // remove this if it's a piercing turret (laser)
			}
			tx--;
			wall = dungeon.getDungeon()[ty*cols + tx].wall;
			enemy = dungeon.getDungeon()[ty*cols + tx].enemy;
			hero = dungeon.getDungeon()[ty*cols + tx].hero;
		}
		break;
	}
	case 'r': {
		tx++;
		wall = dungeon.getDungeon()[ty*cols + tx].wall;
		enemy = dungeon.getDungeon()[ty*cols + tx].enemy;
		hero = dungeon.getDungeon()[ty*cols + tx].hero;
		while (!wall) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				int damage = getDmg();
				// if player was blocking and successfully shielded the shot, reduce the damage taken
				if (dungeon.getPlayerVector().at(0).isBlocking() && dungeon.getPlayerVector().at(0).shieldCoverage(getPosX(), getPosY())) {
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield().getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty, dungeon.getMonsters());
				dungeon.getMonsters().at(pos)->setHP(dungeon.getMonsters().at(pos)->getHP() - this->getDmg());

				return; // remove this if it's a piercing turret (laser)
			}
			tx++;
			wall = dungeon.getDungeon()[ty*cols + tx].wall;
			enemy = dungeon.getDungeon()[ty*cols + tx].enemy;
			hero = dungeon.getDungeon()[ty*cols + tx].hero;
		}
		break;
	}
	case 'u': {
		ty--;
		wall = dungeon.getDungeon()[ty*cols + tx].wall;
		enemy = dungeon.getDungeon()[ty*cols + tx].enemy;
		hero = dungeon.getDungeon()[ty*cols + tx].hero;
		while (!wall) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				int damage = getDmg();
				// if player was blocking and successfully shielded the shot, reduce the damage taken
				if (dungeon.getPlayerVector().at(0).isBlocking() && dungeon.getPlayerVector().at(0).shieldCoverage(getPosX(), getPosY())) {
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield().getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty, dungeon.getMonsters());
				dungeon.getMonsters().at(pos)->setHP(dungeon.getMonsters().at(pos)->getHP() - this->getDmg());

				return; // remove this if it's a piercing turret (laser)
			}
			ty--;
			wall = dungeon.getDungeon()[ty*cols + tx].wall;
			enemy = dungeon.getDungeon()[ty*cols + tx].enemy;
			hero = dungeon.getDungeon()[ty*cols + tx].hero;
		}
		break;
	}
	case 'd': {
		ty++;
		wall = dungeon.getDungeon()[ty*cols + tx].wall;
		enemy = dungeon.getDungeon()[ty*cols + tx].enemy;
		hero = dungeon.getDungeon()[ty*cols + tx].hero;
		while (!wall) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				int damage = getDmg();
				// if player was blocking and successfully shielded the shot, reduce the damage taken
				if (dungeon.getPlayerVector().at(0).isBlocking() && dungeon.getPlayerVector().at(0).shieldCoverage(getPosX(), getPosY())) {
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield().getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty, dungeon.getMonsters());
				dungeon.getMonsters().at(pos)->setHP(dungeon.getMonsters().at(pos)->getHP() - this->getDmg());

				return; // remove this if it's a piercing turret (laser)
			}
			ty++;
			wall = dungeon.getDungeon()[ty*cols + tx].wall;
			enemy = dungeon.getDungeon()[ty*cols + tx].enemy;
			hero = dungeon.getDungeon()[ty*cols + tx].hero;
		}
		break;
	}
	}
}

char Turret::getDirection() const {
	return m_dir;
}
void Turret::setDirection(char dir) {
	m_dir = dir;
}
int Turret::getRange() const {
	return m_range;
}
void Turret::setRange(int range) {
	m_range = range;
}
bool Turret::isTriggered() const {
	return m_triggered;
}
void Turret::setTrigger(bool trigger) {
	m_triggered = trigger;
}
bool Turret::onCooldown() const {
	return m_cooldown;
}
void Turret::setCooldown(bool cooldown) {
	m_cooldown = cooldown;
}

//		MOVING BLOCKS
MovingBlock::MovingBlock(int x, int y, char pattern, int spaces) : Traps(x, y, "Moving Block", 10), m_pattern(pattern), m_spaces(spaces) {
	m_counter = spaces;
	m_dir = (randInt(2) == 0 ? -1 : 1);
}

void MovingBlock::move(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int ax = a.getPosX();
	int ay = a.getPosY();

	int tx = getPosX();
	int ty = getPosY();

	bool wall, enemy, hero;

	switch (getPattern()) {
	case 'h': {
		if (getCounter() == 0) {
			resetCounter();
			flip();
		}

		wall = dungeon.getDungeon()[ty*cols + tx + m_dir].wall;
		enemy = dungeon.getDungeon()[ty*cols + tx + m_dir].enemy;
		hero = dungeon.getDungeon()[ty*cols + tx + m_dir].hero;

		if (wall) {
			resetCounter();
			flip();
			return;
		}

		if (hero) {
			// check if there was a wall at this position, if so, crush player and stun them
			if (dungeon.getDungeon()[a.getPosY()*cols + (a.getPosX() + m_dir)].wall) {
				// play bone crunch sound
				playBoneCrunch();

				a.setHP(a.getHP() - getDmg());
				a.addAffliction(std::make_shared<Stun>());
			}
			else {
				a.setPosX(tx + m_dir + m_dir);
				dungeon.getDungeon()[ay*cols + ax].hero = false;
				dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].hero = true;
				dungeon.queueMoveSprite(a.getSprite(), dungeon.getRows(), a.getPosX(), a.getPosY());

				// check if there was a trap at this position
				if (dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].trap) {
					dungeon.trapEncounter(dungeon, a.getPosX(), a.getPosY());
				}
			}
		}

		if (enemy) {
			int pos = dungeon.findMonster(tx + m_dir, ty, dungeon.getMonsters());

			if (pos != -1) {
				dungeon.getMonsters().at(pos)->setPosX(tx + m_dir + m_dir);
				dungeon.getDungeon()[ty*cols + tx + m_dir].enemy = false;
				dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].enemy = true;
				dungeon.queueMoveSprite(dungeon.getMonsters().at(pos)->getSprite(), dungeon.getRows(), dungeon.getMonsters().at(pos)->getPosX(), dungeon.getMonsters().at(pos)->getPosY());

				// :::: Check trap first, because if monster dies, it will not exist to check for traps ::::
				// check if there was a trap at this position
				if (dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
					dungeon.singleMonsterTrapEncounter(dungeon, pos);
				}

				// check if there was a wall at this position, if so, just kill the monster outright
				if (dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].wall) {
					// play bone crunch sound
					playBoneCrunch((float)exp(-(abs(ax - dungeon.getMonsters().at(pos)->getPosX()) + abs(ay - dungeon.getMonsters().at(pos)->getPosY())) / 2));

					dungeon.destroyMonster(dungeon.getDungeon(), dungeon.getRows(), dungeon.getCols(), dungeon.getMonsters(), pos);
				}
			}
		}

		setPosX(tx + m_dir);
		dungeon.getDungeon()[ty*cols + tx].wall = false;
		dungeon.getDungeon()[ty*cols + tx + m_dir].wall = true;
		dungeon.queueMoveSprite(getSprite(), dungeon.getRows(), getPosX(), getPosY());
		setCounter(getCounter() - 1);

		break;
	}
	case 'v': {
		if (getCounter() == 0) {
			resetCounter();
			flip();
		}

		wall = dungeon.getDungeon()[(ty + m_dir)*cols + tx].wall;
		enemy = dungeon.getDungeon()[(ty + m_dir)*cols + tx].enemy;
		hero = dungeon.getDungeon()[(ty + m_dir)*cols + tx].hero;

		if (wall) {
			resetCounter();
			flip();
			return;
		}

		if (hero) {
			// check if there was a wall at this position, if so, crush player and stun them
			if (dungeon.getDungeon()[(a.getPosY() + m_dir)*cols + a.getPosX()].wall) {
				// play bone crunch sound
				playBoneCrunch();

				a.setHP(a.getHP() - getDmg());
				a.addAffliction(std::make_shared<Stun>());
			}
			else {
				a.setPosY(ty + m_dir + m_dir);
				dungeon.getDungeon()[ay*cols + ax].hero = false;
				dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].hero = true;
				dungeon.queueMoveSprite(a.getSprite(), dungeon.getRows(), a.getPosX(), a.getPosY());

				// check if there was a trap at this position
				if (dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].trap) {
					dungeon.trapEncounter(dungeon, a.getPosX(), a.getPosY());
				}
			}
		}

		if (enemy) {
			int pos = dungeon.findMonster(tx, ty + m_dir, dungeon.getMonsters());

			if (pos != -1) {
				dungeon.getMonsters().at(pos)->setPosY(ty + m_dir + m_dir);
				dungeon.getDungeon()[(ty + m_dir)*cols + tx].enemy = false;
				dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].enemy = true;
				dungeon.queueMoveSprite(dungeon.getMonsters().at(pos)->getSprite(), dungeon.getRows(), dungeon.getMonsters().at(pos)->getPosX(), dungeon.getMonsters().at(pos)->getPosY());

				// :::: Check trap first, because if monster dies, it will not exist to check for traps ::::
				// check if there was a trap at this position
				if (dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
					dungeon.singleMonsterTrapEncounter(dungeon, pos);
				}

				// check if there was a wall at this position, if so, just kill the monster outright
				if (dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].wall) {
					// play bone crunch sound
					playBoneCrunch((float)exp(-(abs(ax - dungeon.getMonsters().at(pos)->getPosX()) + abs(ay - dungeon.getMonsters().at(pos)->getPosY())) / 2));

					dungeon.destroyMonster(dungeon.getDungeon(), dungeon.getRows(), dungeon.getCols(), dungeon.getMonsters(), pos);
				}
			}
		}

		setPosY(ty + m_dir);
		dungeon.getDungeon()[(ty)*cols + tx].wall = false;
		dungeon.getDungeon()[(ty + m_dir)*cols + tx].wall = true;
		dungeon.queueMoveSprite(getSprite(), dungeon.getRows(), getPosX(), getPosY());
		setCounter(getCounter() - 1);

		break;
	}
	}
}

char MovingBlock::getPattern() const {
	return m_pattern;
}
void MovingBlock::setPattern(char pattern) {
	m_pattern = pattern;
}
int MovingBlock::getSpaces() const {
	return m_spaces;
}
int MovingBlock::getCounter() const {
	return m_counter;
}
void MovingBlock::setCounter(int count) {
	m_counter = count;
}
void MovingBlock::resetCounter() {
	m_counter = m_spaces;
}
void MovingBlock::flip() {
	m_dir = -m_dir;
}
bool MovingBlock::turn() const {
	return m_turn;
}
void MovingBlock::toggleTurn() {
	m_turn = !m_turn;
}



//		SOUND EFFECTS
void playBoneCrunch(float volume) {
	switch (randInt(1)) {
	case 0:	cocos2d::experimental::AudioEngine::play2d("Bone_Crack1.mp3", false, volume);
	case 1: cocos2d::experimental::AudioEngine::play2d("Bone_Crack2.mp3", false, volume);
	}
}