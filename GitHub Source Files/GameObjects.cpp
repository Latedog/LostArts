#include "cocos2d.h"
#include "AudioEngine.h"
#include "global.h"
#include "GameObjects.h"
#include "Actors.h"
#include "utilities.h"
#include "Dungeon.h"
#include <string>
#include <iostream>
#include <memory>

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
void Objects::setImageName(std::string image) {
	m_image = image;
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
bool Objects::isWeapon() const {
	return m_isWeapon;
}
void Objects::setWeaponFlag(bool weapon) {
	m_isWeapon = weapon;
}
bool Objects::isShield() const {
	return m_isShield;
}
void Objects::setShieldFlag(bool shield) {
	m_isShield = shield;
}
bool Objects::isItem() const {
	return m_isItem;
}
void Objects::setItemFlag(bool item) {
	m_isItem = item;
}
bool Objects::isAutoUse() const {
	return m_autoUse;
}
void Objects::setAutoFlag(bool autoUse) {
	m_autoUse = autoUse;
}
bool Objects::isChest() const {
	return m_isChest;
}
void Objects::setChestFlag(bool chest) {
	m_isChest = chest;
}
bool Objects::isTrinket() const {
	return m_isTrinket;
}
void Objects::setTrinketFlag(bool trinket) {
	m_isTrinket = trinket;
}

std::string Objects::getDescription() const {
	return m_description;
}
void Objects::setDescription(std::string description) {
	m_description = description;
}

std::string Objects::getSoundName() const {
	return m_sound;
}
void Objects::setSoundName(std::string sound) {
	m_sound = sound;
}
bool Objects::hasExtraSprites() {
	return m_hasExtraSprites;
}
void Objects::setExtraSpritesFlag(bool extras) {
	m_hasExtraSprites = extras;
}
bool Objects::emitsLight() const {
	return m_emitsLight;
}
void Objects::setEmitsLight(bool emits) {
	m_emitsLight = emits;
}


//		IDOL
Idol::Idol() : Objects(randInt(68) + 1, randInt(16) + 1, "Idol") {

}
Idol::~Idol() {

}

//		STAIRS
Stairs::Stairs(int x, int y) : Traps(x, y, STAIRCASE, "Stairs_48x48.png", 0, false) {

}

//		DOORS
Door::Door(int x, int y) : Objects(x, y, DOOR), m_open(false), m_hold(true), m_lock(false) {

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
Button::Button(int x, int y) : Traps(x, y, BUTTON_UNPRESSED, "Button_Unpressed_48x48.png", 0, false) {

}


//	:::: DROPS ::::

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


HeartPod::HeartPod(int x, int y) : Drops(x, y, HEART_POD, "Heart_Pod_48x48.png") {
	setAutoFlag(true);
	setSoundName("Heart_Pod_Pickup2.mp3");
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

LifePotion::LifePotion(int x, int y) : Drops(x, y, LIFEPOT, "Life_Potion_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A red elixir which seems to keep well. Gives you life.");
}
void LifePotion::useItem(Player &p) {
	// sound effect
	cocos2d::experimental::AudioEngine::play2d("Life_Potion_Used.mp3", false, 1.0f);

	if (p.getHP() < p.getMaxHP()) {			// if player hp is less than their max
		if (p.getHP() + 15 <= p.getMaxHP()) // if player hp + effect of lifepot will refill less than or equal to max
			p.setHP(p.getHP() + 15);
		else	// just set player hp to max
			p.setHP(p.getMaxHP());

		//cout << "You feel replenished." << endl;
		//text.push_back("You feel replenished.\n");
	}
	
	//cout << "Lively as ever..." << endl;
	//text.push_back("Lively as ever...\n");
}

BigLifePotion::BigLifePotion(int x, int y) : Drops(x, y, BIG_LIFEPOT, "Big_Health_Potion_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("The alchemist's improved formula. Gives you more life.");
}
void BigLifePotion::useItem(Player &p) {
	// sound effect
	cocos2d::experimental::AudioEngine::play2d("Life_Potion_Used.mp3", false, 1.0f);

	if (p.getHP() < p.getMaxHP()) {			// if player hp is less than their max
		if (p.getHP() + 25 <= p.getMaxHP()) // if player hp + effect of lifepot will refill less than or equal to max
			p.setHP(p.getHP() + 25);
		else	// just set player hp to max
			p.setHP(p.getMaxHP());

	}

}

StatPotion::StatPotion(int x, int y) : Drops(x, y, STATPOT, "Stat_Potion_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Increases your combat abilities, but strangely it removes\n the taste from your mouth.");
}
void StatPotion::useItem(Player &p) {
	// sound effect
	cocos2d::experimental::AudioEngine::play2d("Potion_Used.mp3", false, 1.0f);

	p.setMaxHP(p.getMaxHP() + 5);
	p.addAffliction(std::make_shared<Buff>(30, 2, 2, 2));
}

ArmorDrop::ArmorDrop(int x, int y) : Drops(x, y, ARMOR, "Armor_48x48.png") {
	setItemFlag(true);
	setSoundName("Armor_Pickup.mp3");

	setDescription("Just what a lost adventurer needs to help protect themselves.\n A little more armor to keep you in one piece.");
}
void ArmorDrop::useItem(Player &p) {
	cocos2d::experimental::AudioEngine::play2d("Armor_Use.mp3", false, 1.0f);

	p.setArmor(p.getArmor() + 1);
	//cout << "You feel ready for battle." << endl;
	//text.push_back("You feel ready for battle.\n");
}

ShieldRepair::ShieldRepair(int x, int y) : Drops(x, y, SHIELD_REPAIR, "Shield_Repair_48x48.png") {
	setAutoFlag(true);
	setSoundName("Armor_Pickup.mp3");
}
void ShieldRepair::useItem(Player &p) {
	p.repairShield(10);
}

DizzyElixir::DizzyElixir(int x, int y) : Drops(x, y, DIZZY_ELIXIR, "Dizzy_Elixir_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A strange concoction that makes you feel good, but...");
}
void DizzyElixir::useItem(Player &p) {
	// sound effect
	cocos2d::experimental::AudioEngine::play2d("Potion_Used.mp3", false, 1.0f);

	p.addAffliction(std::make_shared<Confusion>(20 + p.getInt()));
}

Bomb::Bomb(int x, int y) : Drops(x, y, BOMB, "Bomb_48x48.png", false) {
	setItemFlag(true);
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Makes things go boom.");
}
Bomb::Bomb(std::string type, std::string image) : Drops(randInt(68) + 1, randInt(16) + 1, type, image, false) {
	setItemFlag(true);
	setSoundName("Bomb_Pickup2.mp3");
}

void Bomb::useItem(Dungeon &dungeon) {
	cocos2d::experimental::AudioEngine::play2d("Bomb_Placed.mp3", false, 1.0f);

	dungeon.getDungeon()[dungeon.getPlayer().getPosY() * dungeon.getCols() + dungeon.getPlayer().getPosX()].extra = LITBOMB;
	setPosX(dungeon.getPlayer().getPosX());
	setPosY(dungeon.getPlayer().getPosY());

	//dungeon.getActives().emplace_back(new Bomb(*this));
	dungeon.getTraps().push_back(std::make_shared<ActiveBomb>(getPosX(), getPosY()));

	// add bomb sprite
	dungeon.getTraps().back()->setSprite(dungeon.createSprite(dungeon.getPlayer().getPosX(), dungeon.getPlayer().getPosY(), -1, "Bomb_48x48.png"));
}

MegaBomb::MegaBomb() : Bomb("Mega Bomb", "Bomb_48x48.png") {

}

PoisonCloud::PoisonCloud(int x, int y) : Drops(x, y, POISON_CLOUD, "Poison_Bomb_48x48.png", false) {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("WARNING: Do NOT ingest. Handle with care!");
}
void PoisonCloud::useItem(Dungeon &dungeon) {
	cocos2d::experimental::AudioEngine::play2d("Bomb_Placed.mp3", false, 1.0f);

	dungeon[dungeon.getPlayer().getPosY() * dungeon.getCols() + dungeon.getPlayer().getPosX()].extra = LITBOMB;
	setPosX(dungeon.getPlayer().getPosX());
	setPosY(dungeon.getPlayer().getPosY());

	dungeon[getPosY()*dungeon.getCols() + getPosX()].trap = true;
	//dungeon.getDungeon()[getPosX()*dungeon.getCols() + getPosY()].trap_name = POISON_CLOUD;

	// add poison cloud bomb to the traps vector, as it only goes off when something walks over it
	std::shared_ptr<PoisonBomb> bomb = std::make_shared<PoisonBomb>(getPosX(), getPosY(), 10);
	dungeon.getTraps().push_back(bomb);

	// add bomb sprite
	dungeon.getTraps().back()->setSprite(dungeon.createSprite(dungeon.getPlayer().getPosX(), dungeon.getPlayer().getPosY(), -1, bomb->getImageName()));
}


//		SKELETON KEY
SkeletonKey::SkeletonKey(int x, int y) : Drops(x, y, SKELETON_KEY, "Skeleton_Key_48x48.png", false) {
	setItemFlag(true);
	setSoundName("Key_Pickup.mp3");

	setDescription("A mysterious key. What could this be for?");
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
	setItemFlag(true);
	setSoundName("Book_Pickup.mp3");

	setDescription("A spellbook that has imagery of ice crystals.");
}

void FreezeSpell::useItem(Dungeon &dungeon) {
	// play freeze sound effect
	cocos2d::experimental::AudioEngine::play2d("Freeze_Spell1.mp3", false, 1.0f);
	cocos2d::experimental::AudioEngine::play2d("Freeze_Spell2.mp3", false, 1.0f);

	// add a freeze(stun) effect to all
	for (int i = 0; i < dungeon.getMonsters().size(); i++) {
		dungeon.getMonsters()[i]->setHP(dungeon.getMonsters()[i]->getHP() + dungeon.getPlayer().getInt()); // damage if player has any intellect

		dungeon.getMonsters()[i]->addAffliction(std::make_shared<Freeze>(22));
		dungeon.getMonsters()[i]->setFrozen(true);
	}
}
int FreezeSpell::getDamage() const {
	return m_damage;
}

//		EARTHQUAKE SPELL
EarthquakeSpell::EarthquakeSpell(int x, int y) : Drops(x, y, EARTHQUAKE_SPELL, "Earthquake_Spell_48x48.png", false) {
	setItemFlag(true);
	setSoundName("Book_Pickup.mp3");

	setDescription("A spellbook that cracks the earth.");
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
			dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - (getDamage() + dungeon.getPlayer().getInt()));

			if (dungeon.getMonsters().at(i)->getHP() <= 0) {
				dungeon.monsterDeath(i);
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

				/*
				// destroy any gold in the way
				if (dungeon.getDungeon()[j*cols + k].gold != 0) {
					dungeon.getDungeon()[j*cols + k].gold = 0;

					dungeon.removeSprite(dungeon.money_sprites, rows, k, j);
				}
				*/

				// Don't destroy the exit!!
				if (dungeon.getDungeon()[j*cols + k].exit) {
					continue;
				}

				
				// destroy any traps
				if (dungeon.getDungeon()[j*cols + k].trap) {

					int pos = dungeon.findTrap(k, j); // traps in here

					// if the trap is destructible, destroy it
					if (dungeon.getTraps()[pos]->isDestructible()) {
						if (pos != -1) {
							dungeon.getDungeon()[j*cols + k].trap = false;
							dungeon.getDungeon()[j*cols + k].trap_name = "";

							dungeon.queueRemoveSprite(dungeon.getTraps().at(pos)->getSprite());
							dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);

							dungeon.getDungeon()[j*cols + k].wall = false; // traps could be classified as walls
						}
					}

					if (dungeon.getTraps().at(pos)->isExplosive()) {
						std::shared_ptr<ActiveBomb> bomb = std::dynamic_pointer_cast<ActiveBomb>(dungeon.getTraps().at(pos));
						bomb->explosion(dungeon, dungeon.getPlayerVector()[0]);
						bomb.reset();
					}

					// if the trap is not destructible, but the player has enough intellect, destroy it too
					else if (!dungeon.getTraps()[pos]->isDestructible() && dungeon.getPlayer().getInt() >= 5) {
						
						if (pos != -1) {
							dungeon.getDungeon()[j*cols + k].trap = false;
							dungeon.getDungeon()[j*cols + k].trap_name = "";

							dungeon.queueRemoveSprite(dungeon.getTraps().at(pos)->getSprite());
							dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
						}

						dungeon.getDungeon()[j*cols + k].wall = false; // traps could be classified as walls
						
					}
				}

			}
		}
	}
	
}
int EarthquakeSpell::getDamage() const {
	return m_damage;
}

//		FIRE BLAST SPELL
FireBlastSpell::FireBlastSpell(int x, int y) : Drops(x, y, FIREBLAST_SPELL, "Fireblast_Spell_48x48.png", false) {
	setItemFlag(true);
	setSoundName("Book_Pickup.mp3");

	setDescription("A fiery spell that launches a projectile in the\n direction you're facing.");
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
			dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - (getDamage() + dungeon.getPlayer().getInt()));

			runMonsterDamage(dungeon.getMonsters().at(i)->getSprite());
			/*auto tintRed = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 60, 60));
			auto tintNormal = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 255, 255));
			dungeon.getMonsters().at(i)->getSprite()->runAction(cocos2d::Blink::create(0.2, 5));
			dungeon.getMonsters().at(i)->getSprite()->runAction(cocos2d::Sequence::createWithTwoActions(tintRed, tintNormal));*/

			//  check if monster died
			if (dungeon.getMonsters().at(i)->getHP() <= 0) {
				dungeon.monsterDeath(i);
			}
			// otherwise burn the monster if it can be burned
			else if (dungeon.getMonsters().at(i)->getHP() > 0 && dungeon.getMonsters().at(i)->canBeBurned()) {
				dungeon.getMonsters().at(i)->addAffliction(std::make_shared<Burn>(5 + (dungeon.getPlayer().getInt() / 2)));
			}
		}

	}
}
int FireBlastSpell::getDamage() const {
	return m_damage;
}

//		WIND SPELL
WindSpell::WindSpell(int x, int y) : Drops(x, y, WIND_SPELL, "Wind_Spell_48x48.png", false) {
	setItemFlag(true);
	setSoundName("Book_Pickup.mp3");

	setDescription("A strong gust of wind runs by when you touch the cover.");
}

void WindSpell::useItem(Dungeon &dungeon) {
	// play wind sound
	cocos2d::experimental::AudioEngine::play2d("Wind_Spell1.mp3", false, 1.0f);
	cocos2d::experimental::AudioEngine::play2d("Wind_Spell2.mp3", false, 1.0f);

	// if player is on fire, roll for a chance to remove it
	if (dungeon.getPlayer().isBurned()) {
		int roll = 1 + randInt(100) + 2*dungeon.getPlayer().getInt();

		if (roll > 50) {
			dungeon.getPlayerVector()[0].removeAffliction("burn");
		}
	}

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
	int mx, my, tempx, tempy;

	// find any monsters caught in the whirlwind
	char move;
	int tries = 5; // for breaking out of the loop if too many tries
	bool dead = false; // for stunning only monsters that are alive
	int radius = 3;
	int oldSize = dungeon.getMonsters().size();

	for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
		dead = false;
		tries = 5 + (dungeon.getPlayer().getInt() >= 10 ? 3 : dungeon.getPlayer().getInt() >= 5 ? 1 : 0);
		radius = 3 + (dungeon.getPlayer().getInt() >= 10 ? 2 : dungeon.getPlayer().getInt() >= 5 ? 1 : 0);
		oldSize = dungeon.getMonsters().size();
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		// if there any monsters within a 3 block radius, push them away
		while (abs(mx - px) <= radius && abs(my - py) <= radius && tries > 0) {

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
				
				dungeon.singleMonsterTrapEncounter(dungeon.findMonster(tempx, tempy));

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
				// if player has at least 5 intellect, stun is increased to three turns
				int turns = (dungeon.getPlayer().getInt() >= 5 ? 3 : 1);
				dungeon.getMonsters().at(i)->addAffliction(std::make_shared<Stun>(turns));
			}
		}
		else {
			i--; // decrement to not skip monsters since monster died
		}
	}
}
int WindSpell::getDamage() const {
	return m_damage;
}

//		INVISIBILITY SPELL
InvisibilitySpell::InvisibilitySpell(int x, int y) : Drops(x, y, INVISIBILITY_SPELL, "Invisibility_Spell_48x48.png") {
	setItemFlag(true);
	setSoundName("Book_Pickup.mp3");

	setDescription("Now you see me. Now you");
}

void InvisibilitySpell::useItem(Player &p) {
	// play invisibility sound effect
	cocos2d::experimental::AudioEngine::play2d("Invisibility_Spell.mp3", false, 1.0f);

	p.addAffliction(std::make_shared<Invisibility>(25 + p.getInt()));
}

//		ETHEREAL SPELL
EtherealSpell::EtherealSpell(int x, int y) : Drops(x, y, ETHEREAL_SPELL, "Ethereal_Spell_48x48.png") {
	setItemFlag(true);
	setSoundName("Book_Pickup.mp3");

	setDescription("Now you see me. Now you.. still do?");
}

void EtherealSpell::useItem(Player &p) {
	// play ethereality sound effect
	cocos2d::experimental::AudioEngine::play2d("Ethereal_Spell.mp3", false, 1.0f);

	p.addAffliction(std::make_shared<Ethereality>(15 + 2*p.getInt()));
}

//		TELEPORT SPELL
Teleport::Teleport(int x, int y) : Drops(x, y, TELEPORT, "Teleport_Scroll_48x48.png", false) {
	setItemFlag(true);
	setSoundName("TurningPage3.mp3");

	setDescription("Wait. Where am I?");
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

	// if player has enough intellect, give them invulnerability for a few turns
	if (dungeon.getPlayer().getInt() >= 5) {
		dungeon.getPlayerVector()[0].addAffliction(std::make_shared<Invulnerability>(3));
	}
}



//					:::: TRINKETS ::::
Trinket::Trinket(int x, int y, std::string name, std::string image, bool destructible) : Objects(x, y, name, image, destructible) {
	setTrinketFlag(true);
}

void Trinket::apply(Trinket &trinket, Dungeon &dungeon, Player &p) {
	trinket.apply(dungeon, p);
}


DarkStar::DarkStar(int x, int y) : Trinket(x, y, DARK_STAR, "Lightbulb_32x32.png", false) {
	setDescription("The darkness brings you strength.");
}
void DarkStar::apply(Dungeon &dungeon, Player &p) {
	p.setVision(p.getVision() - 2);
	p.setStr(p.getStr() + 3);
}
void DarkStar::unapply(Dungeon &dungeon, Player &p) {
	p.setVision(p.getVision() + 2);
	p.setStr(p.getStr() - 3);
}

LuckyPig::LuckyPig(int x, int y) : Trinket(x, y, LUCKY_PIG, "Lucky_Pig_48x48.png", false) {
	setDescription("A lucky pig's head. But not so much for the pig.");
}
void LuckyPig::apply(Dungeon &dungeon, Player &p) {
	p.setLuck(p.getLuck() + 15);
}
void LuckyPig::unapply(Dungeon &dungeon, Player &p) {
	p.setLuck(p.getLuck() - 15);
}

GoldPot::GoldPot(int x, int y) : Trinket(x, y, GOLD_POT, "Golden_Ring_32x32.png", false) {
	setDescription("A strong sense of greed fills your mind with this ring in\n your possession.");
}
void GoldPot::apply(Dungeon &dungeon, Player &p) {
	p.setMoneyBonus(p.getMoneyBonus() + 2);
}
void GoldPot::unapply(Dungeon &dungeon, Player &p) {
	p.setMoneyBonus(p.getMoneyBonus() - 2);
}

RingOfCasting::RingOfCasting(int x, int y) : Trinket(x, y, RING_OF_CASTING, "Wizards_Hat_48x48.png", false) {
	setDescription("This hat makes you feel more capable when handling spells.");
}
void RingOfCasting::apply(Dungeon &dungeon, Player &p) {
	p.setInt(p.getInt() + 5);
}
void RingOfCasting::unapply(Dungeon &dungeon, Player &p) {
	p.setInt(p.getInt() - 5);
}

VulcanRune::VulcanRune(int x, int y) : Trinket(x, y, VULCAN_RUNE, "Vulcan_Rune_48x48.png", false) {
	setDescription("Some say this rune was forged by a legendary blacksmith\n that then hid their wares deep inside a volcano.");
}
void VulcanRune::apply(Dungeon &dungeon, Player &p) {
	//p.setLavaImmunity(true);
	p.setCanBeBurned(false);
}
void VulcanRune::unapply(Dungeon &dungeon, Player &p) {
	//p.setLavaImmunity(false);
	p.setCanBeBurned(true);
}

BrightStar::BrightStar(int x, int y) : Trinket(x, y, BRIGHT_STAR, "Fireflies_Jar_48x48.png", false) {
	setDescription("A small light in this dark place.");
}
void BrightStar::apply(Dungeon &dungeon, Player &p) {
	p.setVision(p.getVision() + 2);
	p.setMaxHP(p.getMaxHP() + 10);
}
void BrightStar::unapply(Dungeon &dungeon, Player &p) {
	p.setVision(p.getVision() - 2);
	p.setMaxHP(p.getMaxHP() - 10);

	if (p.getHP() > p.getMaxHP())
		p.setHP(p.getMaxHP());
}

Bloodrite::Bloodrite(int x, int y) : Trinket(x, y, BLOODRITE, "Bloody_Apple_32x32.png", false) {
	setDescription("Suddenly, blood sounds particularly delicious...");
}
void Bloodrite::apply(Dungeon &dungeon, Player &p) {
	p.setLifesteal(true);
}
void Bloodrite::unapply(Dungeon &dungeon, Player &p) {
	p.setLifesteal(false);
}

Bloodlust::Bloodlust(int x, int y) : Trinket(x, y, BLOODLUST, "Whiskey_48x48.png", false) {
	setDescription("You should really watch your temper, you're glowing red!");
}
void Bloodlust::apply(Dungeon &dungeon, Player &p) {
	p.setBloodlust(true);
}
void Bloodlust::unapply(Dungeon &dungeon, Player &p) {
	p.setBloodlust(false);

	// resets redness
	p.getSprite()->setColor(cocos2d::Color3B(255, 255, 255));
}



//		CHESTS
Chests::Chests(std::string chest, std::string image) : Drops(randInt(68) + 1, randInt(16) + 1, chest, image, false) {
	setChestFlag(true);
	setSoundName("ChestOpening1.mp3");
}
std::string Chests::open(Chests &chest, _Tile &tile) {
	return chest.open(tile);
}

BrownChest::BrownChest() : Chests(BROWN_CHEST, "Brown_Chest_48x48.png") {

}
std::string BrownChest::open(_Tile &tile) {
	switch (1 + randInt(7)) {
	case 1: tile.object = std::make_shared<LifePotion>(); break;
	case 2:	tile.object = std::make_shared<DizzyElixir>(); break;
	case 3: tile.object = std::make_shared<Bomb>(); break;
	case 4: tile.object = std::make_shared<ShieldRepair>(); break;
	case 5: tile.object = std::make_shared<PoisonCloud>(); break;
	case 6: tile.object = std::make_shared<WindSpell>(); break;
	case 7: tile.object = std::make_shared<StatPotion>(); break;
	default: break;
	}
	tile.item_name = tile.object->getItem();
	return tile.object->getImageName();
}

SilverChest::SilverChest() : Chests(SILVER_CHEST, "Silver_Chest_48x48.png") {

}
std::string SilverChest::open(_Tile &tile){
	switch (1 + randInt(9)) {
	case 1: tile.object = std::make_shared<InvisibilitySpell>(); break;
	case 2:	tile.object = std::make_shared<FireBlastSpell>(); break;
	case 3: tile.object = std::make_shared<Bomb>(); break;
	case 4: tile.object = std::make_shared<ArmorDrop>(); break;
	case 5: tile.object = std::make_shared<ArcaneStaff>(); break;
	case 6:	tile.object = std::make_shared<BrightStar>(); break;
	case 7:	tile.object = std::make_shared<BigLifePotion>(); break;
	case 8:	tile.object = std::make_shared<Bloodlust>(); break;
	case 9: tile.object = std::make_shared<EtherealSpell>(); break;
	default: break;
	}
	tile.item_name = tile.object->getItem();

	return tile.object->getImageName();
}

GoldenChest::GoldenChest() : Chests(GOLDEN_CHEST, "Golden_Chest_48x48.png") {

}
std::string GoldenChest::open(_Tile &tile) {
	int n = 1 + randInt(9);
	switch (n) {
	case 1: tile.object = std::make_shared<LuckyPig>(); break;
	case 2: tile.object = std::make_shared<VulcanSword>(); break;
	case 3: tile.object = std::make_shared<FrostShield>(); break;
	case 4: tile.object = std::make_shared<VulcanHammer>(); break;
	case 5: tile.object = std::make_shared<IronBow>(); break;
	case 6: tile.object = std::make_shared<GoldPot>(); break;
	case 7: tile.object = std::make_shared<Teleport>(); break;
	case 8: tile.object = std::make_shared<GoldenLongSword>(); break;
	case 9: tile.object = std::make_shared<VulcanRune>(); break;
	}

	tile.item_name = tile.object->getItem();
	return tile.object->getImageName();
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
	setWeaponFlag(true);
}
Weapon::Weapon(int x, int y, std::string action, std::string image, int dexbonus, int dmg, int range, bool ability)
	: Objects(x, y, action, image), m_action(action), m_dexbonus(dexbonus), m_dmg(dmg), m_range(range), m_hasAbility(ability) {
	m_bleed = m_burn = m_poison = false;
	setWeaponFlag(true);
}

void Weapon::useAbility(Weapon &weapon, Dungeon &dungeon, Actors &a) {
	weapon.useAbility(dungeon, a);
}
void Weapon::usePattern(Weapon &weapon, Dungeon &dungeon, bool &moveUsed) {
	weapon.usePattern(dungeon, moveUsed);
}

bool Weapon::hasBonus() const {
	return m_hasBonus;
}
void Weapon::setHasBonus(bool bonus) {
	m_hasBonus = bonus;
}
bool Weapon::hasAbility() const {
	return m_hasAbility;
}
void Weapon::setHasAbility(bool ability) {
	m_hasAbility = ability;
}
bool Weapon::hasAttackPattern() const {
	return m_hasPattern;
}
void Weapon::setAttackPattern(bool pattern) {
	m_hasPattern = pattern;
}
bool Weapon::canBeCast() const {
	return m_canBeCast;
}
void Weapon::setCast(bool cast) {
	m_canBeCast = cast;
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
void Weapon::setDamage(int damage) {
	m_dmg = damage;
}

//		SHORT SWORD
ShortSword::ShortSword() : Weapon(randInt(68) + 1, randInt(16) + 1, SHORT_SWORD, "Short_Sword_48x48.png", 1, 3, 1) {
	setSoundName("Weapon_Pickup2.mp3");

	setDescription("Your trusty short sword. It's kept exceptionally clean.");
}

BloodShortSword::BloodShortSword() : Weapon(0, 0, BLOOD_SHORT_SWORD, "Blood_Shortsword_48x48.png", 1, 3, 1) {
	setSoundName("Weapon_Pickup2.mp3");
	setHasAbility(true);

	setDescription("This sword feels alive. But you don't mind because using it\n makes you feel healthier.");
}
void BloodShortSword::useAbility(Dungeon &dungeon, Actors &a) {
	if (dungeon.getPlayer().getHP() > 0 && dungeon.getPlayer().getHP() < dungeon.getPlayer().getMaxHP()) {
		if (randInt(100) + 1 > 90) {
			// 10% chance to heal the player if below max hp
			dungeon.getPlayerVector()[0].setHP(dungeon.getPlayer().getHP() + 2);
		}
	}
}

GoldenShortSword::GoldenShortSword() : Weapon(0, 0, GOLDEN_SHORT_SWORD, "Golden_Shortsword_48x48.png", 1, 2, 1) {
	setSoundName("Weapon_Pickup2.mp3");

	setDescription("This sword is particularly bright when near lots of coins.");
}
void GoldenShortSword::useAbility(Dungeon &dungeon, Actors &a) {
	// + 1 damage for every X coins
	setDamage(2 + dungeon.getPlayer().getMoney() / 25);
}

GoldenLongSword::GoldenLongSword() : Weapon(0, 0, GOLDEN_LONG_SWORD, "Golden_Longsword_48x48.png", 1, 2, 2, true) {
	setSoundName("Weapon_Pickup2.mp3");
	setAttackPattern(true);

	setDescription("This long sword is particularly bright when near lots of coins.");
}
void GoldenLongSword::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	bool wall, item, trap, enemy;
	char move = dungeon.getPlayer().getAction();

	int x = dungeon.getPlayer().getPosX();
	int y = dungeon.getPlayer().getPosY();

	// if weapon has a range of 2, check for monsters to attack

	bool far_enemy, close_enemy;

	// if it was a movement action
	if (move == 'l' || move == 'r' || move == 'u' || move == 'd') {
		// determines which way to move
		int n = 0, m = 0;

		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		close_enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		far_enemy = (y == 1 && move == 'u' || y == rows - 2 && move == 'd' ? close_enemy : dungeon[(y + 2 * m)*cols + (x + 2 * n)].enemy);

		if (close_enemy) {
			dungeon.fight(x + n, y + m);
			moveUsed = true;
		}
		if (far_enemy && !wall && (y + 2 * m)*cols + (x + 2 * n) > 0 && (y + 2 * m)*cols + (x + 2 * n) < rows*cols) {
			dungeon.fight(x + 2 * n, y + 2 * m);
			moveUsed = true;
		}
	}
}
void GoldenLongSword::useAbility(Dungeon &dungeon, Actors &a) {
	// + 1 damage for every X coins
	setDamage(2 + dungeon.getPlayer().getMoney() / 25);
}

Katana::Katana() : Weapon(0, 0, KATANA, "Katana_48x48.png", 3, 2, 2) {
	setSoundName("Weapon_Pickup2.mp3");
	setAttackPattern(true);

	setDescription("Measure twice. Cut once. A trustworthy weapon.");
}
void Katana::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	bool wall, item, trap, enemy;
	char move = dungeon.getPlayer().getAction();

	int x = dungeon.getPlayer().getPosX();
	int y = dungeon.getPlayer().getPosY();

	// if weapon has a range of 2, check for monsters to attack

	bool far_enemy, close_enemy;

	// if it was a movement action
	if (move == 'l' || move == 'r' || move == 'u' || move == 'd') {
		// determines which way to move
		int n = 0, m = 0;

		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		close_enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		far_enemy = (y == 1 && move == 'u' || y == rows - 2 && move == 'd' ? close_enemy : dungeon[(y + 2 * m)*cols + (x + 2 * n)].enemy);

		if (close_enemy) {
			dungeon.fight(x + n, y + m);
			moveUsed = true;
		}
		if (far_enemy && !wall && (y + 2 * m)*cols + (x + 2 * n) > 0 && (y + 2 * m)*cols + (x + 2 * n) < rows*cols) {
			dungeon.fight(x + 2 * n, y + 2 * m);
			moveUsed = true;
		}
	}
}

IronLongSword::IronLongSword() : Weapon(0, 0, IRON_LONG_SWORD, "Iron_Longsword_48x48.png", 1, 4, 2) {
	setSoundName("Weapon_Pickup2.mp3");
	setAttackPattern(true);

	setDescription("Your reach is quite excellent with this in hand. Pierces enemies.");
}
void IronLongSword::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	bool wall, item, trap, enemy;
	char move = dungeon.getPlayer().getAction();

	int x = dungeon.getPlayer().getPosX();
	int y = dungeon.getPlayer().getPosY();

	// if weapon has a range of 2, check for monsters to attack

	bool far_enemy, close_enemy;

	// if it was a movement action
	if (move == 'l' || move == 'r' || move == 'u' || move == 'd') {
		// determines which way to move
		int n = 0, m = 0;

		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		close_enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		far_enemy = (y == 1 && move == 'u' || y == rows - 2 && move == 'd' ? close_enemy : dungeon[(y + 2 * m)*cols + (x + 2 * n)].enemy);

		if (close_enemy) {
			dungeon.fight(x + n, y + m);
			moveUsed = true;
		}
		if (far_enemy && !wall && (y + 2 * m)*cols + (x + 2 * n) > 0 && (y + 2 * m)*cols + (x + 2 * n) < rows*cols) {
			dungeon.fight(x + 2 * n, y + 2 * m);
			moveUsed = true;
		}
	}
}

//		RUSTY CUTLASS
RustyCutlass::RustyCutlass() : Weapon(randInt(68) + 1, randInt(16) + 1, RUSTY_CUTLASS, "Rusty_Broadsword_48x48.png", 2, 2, 1) {
	setSoundName("Weapon_Pickup2.mp3");

	setDescription("A rusty sword. It's seen better days.");
}
RustyCutlass::RustyCutlass(int x, int y) : Weapon(x, y, RUSTY_CUTLASS, "Rusty_Broadsword_48x48.png", 2, 2, 1) {
	setSoundName("Weapon_Pickup2.mp3");

	setDescription("A rusty sword. It's seen better days.");
}

//		BONE AXE 
BoneAxe::BoneAxe(int x, int y) : Weapon(x, y, BONE_AXE, "Bone_Axe_48x48.png", 0, 4, 1) {
	setSoundName("Weapon_Pickup2.mp3");

	setDescription("A primitive item, but it gets the job done.");
}


//		BOWS
Bow::Bow(int x, int y, std::string name, std::string image, int damage, int dexbonus, int range) : Weapon(x, y, name, image, dexbonus, damage, range) {
	setAttackPattern(true);
}
void Bow::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	bool wall, item, trap, enemy;
	char move = dungeon.getPlayer().getAction();

	int x = dungeon.getPlayer().getPosX();
	int y = dungeon.getPlayer().getPosY();

	int range = getRange();
	bool far_enemy, close_enemy;

	// if it was a movement action
	if (move == 'l' || move == 'r' || move == 'u' || move == 'd') {
		// determines which way to move
		int n = 0, m = 0;

		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;

		while (!wall && range > 0) {
			if (enemy) {
				// if player is adjacent to the monster, then the bow's damage is reduced because you melee them
				if (abs(x - (x + n)) <= 1 && abs(y - (y + m)) <= 1) {
					setDamage(getDmg() - 1);
					dungeon.fight(x + n, y + m);
					moveUsed = true;
					setDamage(getDmg() + 1);

					return;
				}

				dungeon.fight(x + n, y + m);
				moveUsed = true;
				return;
			}

			switch (move) {
			case 'l': n--; m = 0; break;
			case 'r': n++; m = 0; break;
			case 'u': n = 0; m--; break;
			case 'd': n = 0; m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			range--;
		}
	}
}

WoodBow::WoodBow() : Bow(0, 0, WOOD_BOW, "Wood_Bow_32x32.png", 2, 2, 4) {
	setDescription("A classic long ranged weapon, but ineffective\n at close range.");
}

IronBow::IronBow() : Bow(0, 0, IRON_BOW, "Reinforced_Bow_48x48.png", 3, 2, 4) {
	setDescription("This bow has been reinforced yet feels lighter. Not effective at close range.");
}

VulcanBow::VulcanBow() : Bow(0, 0, VULCAN_BOW, "Vulcan_Bow_48x48.png", 3, 2, 5) {
	setHasAbility(true);

	setDescription("The legendary weapon smith's craftsmanship cannot\n go unnoticed. Doubtless that this bow will deliver.");
}
void VulcanBow::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBurned()) {
		int roll = randInt(100) - a.getLuck();

		// 5% chance to burn
		if (roll < m_burnChance) {
			// play burned sound effect
			cocos2d::experimental::AudioEngine::play2d("Fire2.mp3", false, 0.7f);

			a.addAffliction(std::make_shared<Burn>(5));
		}
	}
}

//		BRONZE DAGGER
BronzeDagger::BronzeDagger() : Weapon(randInt(68) + 1, randInt(16) + 1, BRONZE_DAGGER, "Bronze_Dagger_48x48.png", 2, 2, 1, true) {
	m_bleedChance = 20;
	setSoundName("Weapon_Pickup.mp3");

	setDescription("It won't hurt as much, but it'll make them bleed.");
}

void BronzeDagger::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBled()) {
		int roll = randInt(100) - a.getLuck();

		// 20% chance to bleed
		if (roll < getBleedChance()) {
			// play bleed sound effect
			cocos2d::experimental::AudioEngine::play2d("Dagger_Slice_Bleed.mp3", false, 0.7f);

			a.addAffliction(std::make_shared<Bleed>());
			//a.setBleed(true);
		}
	}
}
int BronzeDagger::getBleedChance() const {
	return m_bleedChance;
}

//		IRON LANCE
IronLance::IronLance() : Weapon(randInt(68) + 1, randInt(16) + 1, IRON_LANCE, "Iron_Lance_48x48.png", 2, 2, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);

	setDescription("Like a true knight, you charge into battle with arms outstretched.");
}
void IronLance::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	bool wall, item, trap, enemy;
	char action = dungeon.getPlayer().getAction();

	int x = dungeon.getPlayer().getPosX();
	int y = dungeon.getPlayer().getPosY();

	// if weapon has a range of 2, check for monsters to attack

	bool far_enemy, close_enemy;

	// if it was a movement action
	if (action == 'l' || action == 'r' || action == 'u' || action == 'd') {
		// determines which way to move
		int n = 0, m = 0;

		switch (action) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		close_enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		far_enemy = (y == 1 && action == 'u' || y == rows - 2 && action == 'd' ? close_enemy : dungeon[(y + 2 * m)*cols + (x + 2 * n)].enemy);

		if (close_enemy) {
			dungeon.fight(x + n, y + m);
			moveUsed = true;
		}
		else if (far_enemy && !wall && (y + 2 * m)*cols + (x + 2 * n) > 0 && (y + 2 * m)*cols + (x + 2 * n) < rows*cols) {
			dungeon.fight(x + 2 * n, y + 2 * m);
			moveUsed = true;
		}
	}
}

VulcanSword::VulcanSword() : Weapon(0, 0, VULCAN_SWORD, "Vulcan_Sword_48x48.png", 2, 3, 1, true) {
	setSoundName("Weapon_Pickup.mp3");

	setDescription("The blade is imbued with tiny embers that could melt\n through anything.");
}
void VulcanSword::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBurned()) {
		int roll = randInt(100) - a.getLuck();

		// 5% chance to burn
		if (roll < m_burnChance) {
			// play burned sound effect
			cocos2d::experimental::AudioEngine::play2d("Fire2.mp3", false, 0.7f);

			a.addAffliction(std::make_shared<Burn>(5));
		}
	}
}

VulcanHammer::VulcanHammer() : Weapon(0, 0, VULCAN_HAMMER, "Vulcan_Hammer_48x48.png", 1, 3, 1, true) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("The hammer that the legendary blacksmith used to\n forge his weapons. Packs one heck of a punch.");
}
void VulcanHammer::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	bool wall, item, trap, enemy;
	char move = dungeon.getPlayer().facingDirection();
	char action = dungeon.getPlayer().getAction();

	int x = dungeon.getPlayer().getPosX();
	int y = dungeon.getPlayer().getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if hammer is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(dungeon.getPlayer().getSprite());

		cocos2d::experimental::AudioEngine::play2d("FootStepGeneric2.mp3", false, 1.0f);

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	// if it was a movement action
	if (action == 'l' || action == 'r' || action == 'u' || action == 'd') {
		// determines which way to move
		int n = 0, m = 0;

		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		if (dungeon[(y + m)*cols + (x + n)].enemy) {
			// play fire blast explosion sound effect
			cocos2d::experimental::AudioEngine::play2d("Fireblast_Spell2.mp3", false, 1.0f);

			setDamage(getDmg() + 2); // damage boosted
			dungeon.fight(x + n, y + m);
			setDamage(getDmg() - 2); // remove damage boost
		}
	}

	// effect to remove wind up effect
	untint(dungeon.getPlayer().getSprite());

	moveUsed = true;
	m_woundUp = false;
}
void VulcanHammer::useAbility(Dungeon &dungeon, Actors &a) {
	// if hammer was primed, apply stun
	if (a.canBeStunned() && m_woundUp) {
		a.addAffliction(std::make_shared<Stun>(1));
	}

	if (a.canBeBurned()) {
		int roll = randInt(100) - a.getLuck();

		// 5% chance to burn
		if (roll < m_burnChance) {
			// play burned sound effect
			cocos2d::experimental::AudioEngine::play2d("Fire2.mp3", false, 0.7f);

			a.addAffliction(std::make_shared<Burn>(5));
		}
	}
}

ArcaneStaff::ArcaneStaff() : Weapon(0, 0, ARCANE_STAFF, "Arcane_Staff_48x48.png", 1, 1, 1, false) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setHasBonus(true);
	setCast(true);

	setDescription("Ooo, sparkly.");
}
void ArcaneStaff::applyBonus(Actors &a) {
	if (!m_bonusApplied) {
		a.setInt(a.getInt() + 3);
		m_bonusApplied = true;
	}
}
void ArcaneStaff::unapplyBonus(Actors &a) {
	if (m_bonusApplied) {
		a.setInt(a.getInt() - 3);
		m_bonusApplied = false;
		m_isCast = false; // allows casting if weapon was switched
	}
}
void ArcaneStaff::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	bool wall, item, trap, enemy;
	char move = dungeon.getPlayer().facingDirection();
	char action = dungeon.getPlayer().getAction();

	int x = dungeon.getPlayer().getPosX();
	int y = dungeon.getPlayer().getPosY();

	// if player didn't cast, then return
	if (!m_isCast && action != WIND_UP) {
		m_isCast = false;
		return;
	}

	// if staff is not cast and player has cast, then do so and return
	if (!m_isCast && action == WIND_UP) {
		// effect to show wind up
		tintStaffCast(dungeon.getPlayer().getSprite());

		cocos2d::experimental::AudioEngine::play2d("Staff_Cast1.mp3", false, 1.0f);

		// boost int
		applyBonus(dungeon.getPlayerVector()[0]);

		m_px = x;
		m_py = y;
		m_isCast = true;
		moveUsed = true;

		return;
	}

	// if player attempted to move, unapply bonus
	switch (action) {
	case 'l':
	case 'r':
	case 'u':
	case 'd': unapplyBonus(dungeon.getPlayerVector()[0]); m_isCast = false; break;
	}

}
void ArcaneStaff::useAbility(Dungeon &dungeon, Actors &a) {
	//// if player moved, unapply bonus
	//if (m_px != dungeon.getPlayer().getPosX() || m_py != dungeon.getPlayer().getPosY())
	//	applyBonus(dungeon.getPlayerVector()[0]);
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
	setShieldFlag(true);
}
Shield::Shield(int x, int y, int defense, int durability, int coverage, std::string type, std::string image)
	: Objects(x, y, type, image), m_defense(defense), m_durability(durability), m_coverage(coverage) {
	m_max_durability = durability;
	setShieldFlag(true);
}
Shield::Shield(int defense, int durability, int max_durability, int coverage, std::string type, std::string image)
	: Objects(type, image), m_defense(defense), m_durability(durability), m_coverage(coverage) {
	m_max_durability = max_durability;
	setShieldFlag(true);
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
bool Shield::hasAbility() const {
	return m_hasAbility;
}
void Shield::setAbility(bool ability) {
	m_hasAbility = ability;
}

//						Shield(defense, durability, coverage, name)
WoodShield::WoodShield() : Shield(2, 15, 1, WOOD_SHIELD, "Wood_Shield_48x48.png") {
	setDescription("A wood shield. Can block projectiles and close combat\n attacks, but be careful that it doesn't break.");
}
WoodShield::WoodShield(Shield const shield)
	: Shield(shield.getDefense(), shield.getDurability(), shield.getMaxDurability(), shield.getCoverage(), shield.getItem(), shield.getImageName()) {
}
WoodShield::WoodShield(int x, int y) : Shield(x, y, 2, 15, 1, WOOD_SHIELD, "Wood_Shield_48x48.png") {

}

IronShield::IronShield() : Shield(4, 40, 1, IRON_SHIELD, "Iron_Shield_48x48.png") {
	setDescription("A sturdier shield, capable of withstanding stronger blows.");
}
IronShield::IronShield(Shield const shield)
	: Shield(shield.getDefense(), shield.getDurability(), shield.getMaxDurability(), shield.getCoverage(), shield.getItem(), shield.getImageName()) {
}
IronShield::IronShield(int x, int y) : Shield(x, y, 4, 40, 1, IRON_SHIELD, "Iron_Shield_48x48.png") {

}

ThornedShield::ThornedShield(int x, int y) : Shield(x, y, 3, 40, 2, THORNED_SHIELD, "Thorned_Shield_48x48.png") {
	setAbility(true);

	setDescription("This prickly shield will surely give foes a piece of your mind.");
}
void ThornedShield::useAbility(Dungeon &dungeon, Actors &a) {
	// spiked sound effect?
	//

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
	int ax = a.getPosX();
	int ay = a.getPosY();

	// if attacker is next to player, it damages them
	if (abs(ax - px) <= 1 && abs(ay - py) <= 1) {
		// sound effect
		cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 0.8f);

		// damaged effect
		runMonsterDamage(a.getSprite());

		a.setHP(a.getHP() - 1);
	}
}

FrostShield::FrostShield(int x, int y) : Shield(x, y, 3, 30, 1, FROST_SHIELD, "Frost_Shield_48x48.png") {
	setAbility(true);

	setDescription("Magical. Capable of freezing enemies with its strange enchantment.");
}
void FrostShield::useAbility(Dungeon &dungeon, Actors &a) {
	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
	int ax = a.getPosX();
	int ay = a.getPosY();

	// if attacker is next to player, it damages them
	if (abs(ax - px) <= 1 && abs(ay - py) <= 1) {
		// sound effect
		cocos2d::experimental::AudioEngine::play2d("IceBreak7.mp3", false, 0.8f);

		// add freeze
		a.addAffliction(std::make_shared<Freeze>(3));
	}
}

ReflectShield::ReflectShield(int x, int y) : Shield(x, y, 2, 25, 1, REFLECT_SHIELD, "Holy_Shield_48x48.png") {
	setAbility(true);

	setDescription("A shield with truly superior reflective capabilities. Can deflect projectiles back at enemies.");
}
void ReflectShield::useAbility(Dungeon &dungeon, Actors &a) {
	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
	int ax = a.getPosX();
	int ay = a.getPosY();

	// if attacker is at a distance to player, it reflects and damages them
	if ((abs(ax - px) > 1 || abs(ay - py) > 1)) {
		// sound effect
		cocos2d::experimental::AudioEngine::play2d("Enemy_Hit.mp3", false, 0.8f);

		// damaged effect
		runMonsterDamage(a.getSprite());

		a.setHP(a.getHP() - a.getStr());
	}
}
void ReflectShield::checkLineOfFire(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int tx = a.getPosX();
	int ty = a.getPosY();

	bool wall, enemy, hero;

	/*
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
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield()->getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
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
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield()->getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
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
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield()->getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
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
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield()->getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
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
	*/
}


//	:::: TRAPS ::::
Traps::Traps(int x, int y, std::string name, std::string image, int damage, bool destructible) : Objects (x, y, name, image, destructible), m_trapdmg(damage) {
	
}

void Traps::activeTrapAction(Traps &trap, Dungeon &dungeon, Actors &a) {
	trap.activeTrapAction(dungeon, a);
}
void Traps::trapAction(Traps &trap, Dungeon &dungeon, Actors &a) {
	trap.trapAction(dungeon, a);
}
void Traps::trapAction(Dungeon &dungeon, Actors &a) {
	this->trapAction(a);
}

int Traps::getDmg() const {
	return m_trapdmg;
}
void Traps::setDmg(int damage) {
	m_trapdmg = damage;
}

bool Traps::isTemporary() const {
	return m_temp;
}
void Traps::setTemporary(int temp) {
	m_temp = temp;
}

bool Traps::isActive() const {
	return m_active;
}
void Traps::setActive(bool active) {
	m_active = active;
}

bool Traps::actsAsWall() const {
	return m_wall;
}
void Traps::setWallFlag(bool wall) {
	m_wall = wall;
}

bool Traps::isLethal() const {
	return m_lethal;
}
void Traps::setLethal(bool lethal) {
	m_lethal = lethal;
}

bool Traps::isExplosive() const {
	return m_explosive;
}
void Traps::setExplosive(bool explosive) {
	m_explosive = explosive;
}


//		PIT
Pit::Pit() : Traps(1 + randInt(68), 1 + randInt(16), PIT, "Pit_48x48.png", 1000) {
	setLethal(true);
}
Pit::Pit(int x, int y) : Traps(x, y, PIT, "Pit_48x48.png", 1000) {
	setLethal(true);
}

void Pit::trapAction(Dungeon &dungeon, Actors &a) {
	if (a.isPlayer()) {
		// play falling sound effect
		cocos2d::experimental::AudioEngine::play2d("Female_Falling_Scream_License.mp3", false, 1.0f);

		// instant death from falling
		a.setHP(0);

		deathFade(a.getSprite());
	}
	else if (a.isMonster() && !a.isFlying()) {
		Monster &m = dynamic_cast<Monster&>(a);

		// play falling sound effect
		playMonsterDeathByPit(dungeon.getPlayer(), m);

		// death animation
		deathFade(a.getSprite());

		m.destroy(dungeon);
	}
}

//		FALLING SPIKES
FallingSpike::FallingSpike(int x, int y, int speed) : Traps(x, y, FALLING_SPIKE, "CeilingSpike_48x48.png", 3), m_speed(speed) {
	setTemporary(true);
	setActive(true);
}

void FallingSpike::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	int pos = dungeon.findTrap(x, y);

	switch (getSpeed()) {
	case 1:
		//	if tile below spike is the player, hit them
		if (dungeon.getDungeon()[(y + 1)*cols + x].hero) {
			cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

			a.setHP(a.getHP() - getDmg());

			dungeon.removeSprite(dungeon.spike_sprites, rows, x, y);
			//dungeon.removeSprite(getSprite());

			dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
		}
		//	if tile below spike is not a wall
		else if (y + 1 < rows - 1) {
			dungeon.moveSprite(dungeon.spike_sprites, rows, x, y, 'd');
			//dungeon.moveSprite(getSprite(), 'd');

			setPosY(y + 1);

			// check if a spike already exists in this spot, if so make note of it
			if (dungeon.getDungeon()[(y + 1)*cols + x].traptile == c_SPIKE) {
				dungeon.getDungeon()[(y + 1)*cols + x].overlap = true;
				dungeon.getDungeon()[(y + 1)*cols + x].overlap_count++;
			}

			dungeon.getDungeon()[(y + 1)*cols + x].traptile = c_SPIKE;
			dungeon.getDungeon()[(y + 1)*cols + x].trap = true;
		}
		//	else disappear
		else {
			dungeon.removeSprite(dungeon.spike_sprites, rows, x, y);
			//dungeon.removeSprite(getSprite());

			dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
		}
		break;
	case 2:
		//	if player is in the way, hit them
		if (dungeon.getDungeon()[(y + 1)*cols + x].hero || dungeon.getDungeon()[(std::min(y + 2, rows - 1))*cols + x].hero) {
			cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

			a.setHP(a.getHP() - getDmg());

			dungeon.removeSprite(dungeon.spike_sprites, rows, x, y);
			//dungeon.removeSprite(getSprite());

			dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
		}
		//	else if two tiles below spike are not walls
		else if (y + 2 < rows - 1) {
			dungeon.moveSprite(dungeon.spike_sprites, rows, x, y, 'D');
			//dungeon.moveSprite(getSprite(), 'D');

			setPosY(y + 2);

			// check if a spike already exists in this spot, if so make note of it
			if (dungeon.getDungeon()[(y + 2)*cols + x].traptile == c_SPIKE) {
				dungeon.getDungeon()[(y + 2)*cols + x].overlap = true;
				dungeon.getDungeon()[(y + 2)*cols + x].overlap_count++;
			}

			dungeon.getDungeon()[(y + 2)*cols + x].traptile = c_SPIKE;
			dungeon.getDungeon()[(y + 2)*cols + x].trap = true;
		}
		//	else if there is only one tile of space underneath, set spike to bottom
		else if (y + 2 >= rows - 1 && y != rows - 2) {
			dungeon.moveSprite(dungeon.spike_sprites, rows, x, y, 'd');
			//dungeon.moveSprite(getSprite(), 'd');

			setPosY(rows - 2);

			// check if a spike already exists in this spot, if so make note of it
			if (dungeon.getDungeon()[(y + 1)*cols + x].traptile == c_SPIKE) {
				dungeon.getDungeon()[(y + 1)*cols + x].overlap = true;
				dungeon.getDungeon()[(y + 1)*cols + x].overlap_count++;
			}

			dungeon.getDungeon()[(rows - 2)*cols + x].traptile = c_SPIKE;
			dungeon.getDungeon()[(rows - 2)*cols + x].trap = true;
		}
		//	else disappear
		else {
			dungeon.removeSprite(dungeon.spike_sprites, rows, x, y);
			//dungeon.removeSprite(getSprite());

			dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
		}
		break;
	case 3:
		//	if player is below, hit them
		if (dungeon.getDungeon()[(std::min(y + 3, rows - 1))*cols + x].hero || dungeon.getDungeon()[(std::min(y + 2, rows - 1))*cols + x].hero || dungeon.getDungeon()[(y + 1)*cols + x].hero) {
			cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

			a.setHP(a.getHP() - getDmg());

			dungeon.removeSprite(dungeon.spike_sprites, rows, x, y);
			//dungeon.removeSprite(getSprite());

			dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);

		}
		//	if spike does not hit a wall
		else if (y + 3 < rows - 1) {
			dungeon.moveSprite(dungeon.spike_sprites, rows, x, y, 'V');
			//dungeon.moveSprite(getSprite(), 'V');

			setPosY(y + 3);

			// check if a spike already exists in this spot, if so make note of it
			if (dungeon.getDungeon()[(y + 3)*cols + x].traptile == c_SPIKE) {
				dungeon.getDungeon()[(y + 3)*cols + x].overlap = true;
				dungeon.getDungeon()[(y + 3)*cols + x].overlap_count++;
			}

			dungeon.getDungeon()[(y + 3)*cols + x].traptile = c_SPIKE;
			dungeon.getDungeon()[(y + 3)*cols + x].trap = true;
		}
		//	if spike would overshoot, move it to the bottom
		else if (y + 3 >= rows - 1 && y != rows - 2) {
			// if spike is one above the bottom
			if (y == rows - 3) {
				dungeon.moveSprite(dungeon.spike_sprites, rows, x, y, 'd');
				//dungeon.moveSprite(getSprite(), 'd');
			}
			// else spike was two above the bottom
			else {
				dungeon.moveSprite(dungeon.spike_sprites, rows, x, y, 'D');
				//dungeon.moveSprite(getSprite(), 'D');
			}

			setPosY(rows - 2);

			// check if a spike already exists in this spot, if so make note of it
			if (dungeon.getDungeon()[(rows - 2)*cols + x].traptile == c_SPIKE) {
				dungeon.getDungeon()[(rows - 2)*cols + x].overlap = true;
				dungeon.getDungeon()[(rows - 2)*cols + x].overlap_count++;
			}

			dungeon.getDungeon()[(rows - 2)*cols + x].traptile = c_SPIKE;
			dungeon.getDungeon()[(rows - 2)*cols + x].trap = true;
		}
		//	else disappear
		else {
			dungeon.removeSprite(dungeon.spike_sprites, rows, x, y);
			//dungeon.removeSprite(getSprite());

			dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
		}
		break;
	}

	// if there are more than two spikes overlapping, just reduce the overlap count
	if (dungeon.getDungeon()[y*cols + x].overlap && dungeon.getDungeon()[y*cols + x].overlap_count > 1) {
		dungeon.getDungeon()[y*cols + x].overlap_count--;
	}
	// if there is only one overlap, set overlap to false, reduce count to zero,
	// but do not replace the previous tile with a space
	else if (dungeon.getDungeon()[y*cols + x].overlap && dungeon.getDungeon()[y*cols + x].overlap_count == 1) {
		dungeon.getDungeon()[y*cols + x].overlap = false;
		dungeon.getDungeon()[y*cols + x].overlap_count--;
	}
	// else if there is no overlap, then reset the tile to empty projectile space
	else {
		dungeon.getDungeon()[y*cols + x].traptile = SPACE;
		dungeon.getDungeon()[y*cols + x].trap = false;
	}

}
void FallingSpike::trapAction(Dungeon &dungeon, Actors &a) {
	cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

	a.setHP(a.getHP() - getDmg());

	dungeon.getDungeon()[getPosY()*dungeon.getCols() + getPosX()].trap = false;
	dungeon.getDungeon()[getPosY()*dungeon.getCols() + getPosX()].trap_name = "";
	
	dungeon.removeSprite(dungeon.spike_sprites, dungeon.getRows(), getPosX(), getPosY()); // remove the spike sprite
	//dungeon.removeSprite(getSprite());

	dungeon.getTraps().erase(dungeon.getTraps().begin() + dungeon.findTrap(getPosX(), getPosY()));
	//dungeon.getActives().erase(dungeon.getActives().begin() + dungeon.findActive(getPosX(), getPosY()));
}

int FallingSpike::getSpeed() const {
	return m_speed;
}
void FallingSpike::setSpeed(int speed) {
	m_speed = speed;
}

//		SPIKES
Spikes::Spikes(int x, int y) : Traps(x, y, SPIKES, "Spiketrap_Active_48x48.png", 7) {

}

void Spikes::trapAction(Dungeon &dungeon, Actors &a) {
	if (!a.isFlying()) {
		cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

		a.setHP(a.getHP() - getDmg());
	}
}

//		SPIKE TRAPS
SpikeTrap::SpikeTrap() : Traps(1 + randInt(BOSSCOLS - 2), BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2), AUTOSPIKE_DEACTIVE, "Spiketrap_Deactive_48x48.png", 5), m_cyclespeed(3 + randInt(3)) {
	m_deactive = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Deactive_48x48.png");
	m_primed = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Primed_48x48.png");
	m_active = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Active_48x48.png");

	setSprite(m_deactive);
	//setSpriteVisibility(true, false, false);
	setTemporary(false);
	setActive(true);
}
SpikeTrap::SpikeTrap(int x, int y, int speed) : Traps(x, y, AUTOSPIKE_DEACTIVE, "Spiketrap_Deactive_48x48.png", 3), m_cyclespeed(speed), m_countdown(speed) {
	m_deactive = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Deactive_48x48.png");
	m_primed = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Primed_48x48.png");
	m_active = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Active_48x48.png");

	setSprite(m_deactive);
	setSpriteVisibility(true, false, false);
	setTemporary(false);
	setActive(true);
}

void SpikeTrap::activeTrapAction(Dungeon &dungeon, Actors &a) {
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

		// if they're standing on top of it and not flying, hit them
		if (ax == tx && ay == ty && !a.isFlying()) {
			cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

			a.setHP(a.getHP() - getDmg());
		}
	}

	// assigns spike to traps vector for lighting purposes
	int pos = dungeon.findTrap(tx, ty);
	dungeon.getTraps().at(pos) = std::make_shared<SpikeTrap>(*this);
}
void SpikeTrap::trapAction(Dungeon &dungeon, Actors &a) {
	if (getCountdown() == 0 && !a.isFlying()) {
		cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

		a.setHP(a.getHP() - getDmg());
	}
}
/*
void SpikeTrap::trapAction(Actors &a) {
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
*/
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
	else {
		m_deactive->setVisible(false);
		m_primed->setVisible(false);
		m_active->setVisible(false);
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
TriggerSpike::TriggerSpike(int x, int y) : Traps(x, y, TRIGGERSPIKE_DEACTIVE, "Spiketrap_Deactive_48x48.png", 5), m_triggered(false) {
	m_deactive = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Deactive_48x48.png");
	m_primed = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Primed_48x48.png");
	m_active = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Active_48x48.png");

	setSprite(m_deactive);
	setTemporary(false);
	setActive(true);
}

void TriggerSpike::activeTrapAction(Dungeon &dungeon, Actors &a) {
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
		if (ax == tx && ay == ty && !a.isFlying()) {
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
void TriggerSpike::trapAction(Dungeon &dungeon, Actors &a) {
	if (m_active->isVisible() && !a.isFlying()) {
		cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

		a.setHP(a.getHP() - getDmg());
	}
}
void TriggerSpike::trapAction(Actors &a) {
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
	else {
		m_deactive->setVisible(false);
		m_primed->setVisible(false);
		m_active->setVisible(false);
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
Puddle::Puddle(int x, int y) : Traps(x, y, PUDDLE, "Puddle.png", 0, true) {
	setTemporary(true);
	setActive(false);
}

void Puddle::trapAction(Dungeon &dungeon, Actors &a) {
	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	// if actor is flying, return
	if (a.isFlying())
		return;

	int roll = randInt(100) + a.getLuck();

	// if monster, adjust sound based on monster's position
	if (a.isMonster()) {
		float volume = exp(-(abs(px - a.getPosX()) + abs(py - a.getPosY()) / 2));
		cocos2d::experimental::AudioEngine::play2d("Puddle_Splash.mp3", false, volume);
	}
	else {
		// splash sound effect
		cocos2d::experimental::AudioEngine::play2d("Puddle_Splash.mp3", false, 0.7f);
	}

	dungeon.getDungeon()[getPosY()*dungeon.getCols() + getPosX()].trap = false;
	dungeon.getDungeon()[getPosY()*dungeon.getCols() + getPosX()].trap_name = "";
	dungeon.queueRemoveSprite(getSprite());
	dungeon.getTraps().erase(dungeon.getTraps().begin() + dungeon.findTrap(getPosX(), getPosY()));

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
		if (a.canBeStunned()) {
			// play slip sound effect
			cocos2d::experimental::AudioEngine::play2d("Puddle_Slip.mp3", false, exp(-(abs(px - a.getPosX()) + abs(py - a.getPosY()))));

			// turn sprite sideways
			a.getSprite()->setRotation(90);
			
			a.setStunned(true);
			a.addAffliction(std::make_shared<Stun>());
		}

		// else if they're not stunnable, nothing happens
		return;
	}
	
}

//		FIREBAR
Firebar::Firebar(int x, int y, int rows) : Traps(x, y, FIREBAR, "Firebar_Totem_48x48.png", 5) {
	m_clockwise = randInt(2);
	m_angle = 1 + randInt(8);

	m_inner = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	m_outer = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");

	setInitialFirePosition(x, y, rows);
	setTemporary(false);
	setActive(true);
	setWallFlag(true);
	setExtraSpritesFlag(true);
	setEmitsLight(true);
}
Firebar::Firebar(int x, int y, std::string firebar) : Traps(x, y, firebar, "Firebar_Totem_48x48.png", 5) {
	m_clockwise = randInt(2);
	m_angle = 1 + randInt(8);

	setTemporary(false);
	setActive(true);
	setWallFlag(true);
	setExtraSpritesFlag(true);
	setEmitsLight(true);
}
Firebar::~Firebar() {
	if (getInner() != nullptr)
		getInner()->removeFromParent();

	if (getOuter() != nullptr)
		getOuter()->removeFromParent();
}

void Firebar::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	int mx = getPosX();
	int my = getPosY();

	// reset projectiles to visible
	getInner()->setVisible(true);
	getOuter()->setVisible(true);

	/* 8 1 2
	*  7 X 3
	*  6 5 4
	*  moves firebar -to- position getAngle(), it is not where it currently is
	*  HOWEVER, the firebar's sprite begins at the sprite's current angle, so the sprite is at the angle it currently is
	*/

	if (isClockwise()) {
		switch (getAngle()) {
		case 1:
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');

			setFirePosition('r');

			if (my == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 2:
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');

			setFirePosition('r');
			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 3:
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');

			setFirePosition('d');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 4:
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');

			setFirePosition('d');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 5:
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');

			setFirePosition('l');
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 6:
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');

			setFirePosition('l');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (my == maxrows - 2 || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 7:
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');

			setFirePosition('u');

			// if firebar is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 8:
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');

			setFirePosition('u');

			// if firebar is on the left or top edge boundary, hide the projectiles
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
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');

			setFirePosition('l');
			if (my == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 2:
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');

			setFirePosition('u');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 3:
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');

			setFirePosition('u');

			// if firebar is on the right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 4:
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');

			setFirePosition('r');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 5:
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');

			setFirePosition('r');
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 6:
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');

			setFirePosition('d');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == maxrows - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 7:
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');

			setFirePosition('d');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		case 8:
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');

			setFirePosition('l');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			break;
		default: break;
		}

		if (getAngle() == 1)
			setAngle(8);
		else
			setAngle(getAngle() - 1);
	}

	if (playerWasHit(dungeon.getPlayerVector().at(0))) {
		trapAction(dungeon.getPlayerVector().at(0));
	}
}
void Firebar::trapAction(Actors &a) {
	if(a.getName() == PLAYER)
		cocos2d::experimental::AudioEngine::play2d("Fire3.mp3", false, 1.0f);

	a.setHP(a.getHP() - this->getDmg());

	// if actor is still alive and can be burned, try to burn them
	if (a.getHP() > 0 && a.canBeBurned()) {

		int roll = randInt(100) + a.getLuck();

		// failed the save roll
		if (roll < 80) {
			a.addAffliction(std::make_shared<Burn>());
		}
	}

}

void Firebar::setInitialFirePosition(int x, int y, int rows) {

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

void Firebar::setSpriteColor(cocos2d::Color3B color) {
	getInner()->setColor(color);
	getOuter()->setColor(color);
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
DoubleFirebar::DoubleFirebar(int x, int y, int rows) : Firebar(x, y, DOUBLE_FIREBAR) {
	m_inner = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	m_innerMirror = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	m_outer = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	m_outerMirror = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");

	setInitialFirePosition(x, y, rows);
}
DoubleFirebar::~DoubleFirebar() {
	if (getInner() != nullptr)
		getInner()->removeFromParent();

	if (getOuter() != nullptr)
		getOuter()->removeFromParent();

	if (getInnerMirror() != nullptr)
		getInnerMirror()->removeFromParent();

	if (getOuterMirror() != nullptr)
		getOuterMirror()->removeFromParent();
}

void DoubleFirebar::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	int mx = getPosX();
	int my = getPosY();

	// reset projectiles to visible
	setSpriteVisibility(true);

	/* 8 1 2
	*  7 X 3
	*  6 5 4
	*  moves firebar -to- position getAngle(), it is not where it currently is
	*
	*/

	if (isClockwise()) {
		switch (getAngle()) {
		case 1: {
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');
			setFirePosition('r');

			if (my == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			// opposite fire buddies
			dungeon.queueMoveSprite(getInnerMirror(), 'l');
			dungeon.queueMoveSprite(getOuterMirror(), 'L');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}

		}
				break;
		case 2: {
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');

			setFirePosition('r');
			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'l');
			dungeon.queueMoveSprite(getOuterMirror(), 'L');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (my == maxrows - 2 || mx == 1) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 3: {
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');

			setFirePosition('d');
			// if firebar is on the left or right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'u');
			dungeon.queueMoveSprite(getOuterMirror(), 'U');

			// if firebar is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 4: {
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');

			setFirePosition('d');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'u');
			dungeon.queueMoveSprite(getOuterMirror(), 'U');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 5: {
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');

			setFirePosition('l');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}
			// opposite fire
			dungeon.queueMoveSprite(getInnerMirror(), 'r');
			dungeon.queueMoveSprite(getOuterMirror(), 'R');

			if (my == 1) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 6: {
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');
			setFirePosition('l');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (my == maxrows - 2 || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'r');
			dungeon.queueMoveSprite(getOuterMirror(), 'R');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 7: {
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');
			setFirePosition('u');

			// if firebar is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'd');
			dungeon.queueMoveSprite(getOuterMirror(), 'D');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 8: {
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');
			setFirePosition('u');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'd');
			dungeon.queueMoveSprite(getOuterMirror(), 'D');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		default: break;
		}

		if (getAngle() == 8)
			setAngle(1);
		else
			setAngle(getAngle() + 1);
	}
	// else counterclockwise
	else {
		switch (getAngle()) {
		case 1: {
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');
			setFirePosition('l');

			if (my == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'r');
			dungeon.queueMoveSprite(getOuterMirror(), 'R');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 2: {
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');
			setFirePosition('u');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'd');
			dungeon.queueMoveSprite(getOuterMirror(), 'D');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == maxrows - 2) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 3: {
			dungeon.queueMoveSprite(getInner(), 'u');
			dungeon.queueMoveSprite(getOuter(), 'U');
			setFirePosition('u');

			// if firebar is on the right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'd');
			dungeon.queueMoveSprite(getOuterMirror(), 'D');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 4: {
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');
			setFirePosition('r');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'l');
			dungeon.queueMoveSprite(getOuterMirror(), 'L');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 5: {
			dungeon.queueMoveSprite(getInner(), 'r');
			dungeon.queueMoveSprite(getOuter(), 'R');
			setFirePosition('r');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'l');
			dungeon.queueMoveSprite(getOuterMirror(), 'L');


			if (my == 1) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 6: {
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');
			setFirePosition('d');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == maxrows - 2) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'u');
			dungeon.queueMoveSprite(getOuterMirror(), 'U');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 7: {
			dungeon.queueMoveSprite(getInner(), 'd');
			dungeon.queueMoveSprite(getOuter(), 'D');
			setFirePosition('d');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'u');
			dungeon.queueMoveSprite(getOuterMirror(), 'U');

			// if firebar is on the right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		case 8: {
			dungeon.queueMoveSprite(getInner(), 'l');
			dungeon.queueMoveSprite(getOuter(), 'L');
			setFirePosition('l');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				getInner()->setVisible(false);
				getOuter()->setVisible(false);
			}

			// opposite
			dungeon.queueMoveSprite(getInnerMirror(), 'r');
			dungeon.queueMoveSprite(getOuterMirror(), 'R');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				getInnerMirror()->setVisible(false);
				getOuterMirror()->setVisible(false);
			}
		}
				break;
		default: break;
		}

		if (getAngle() == 1)
			setAngle(8);
		else
			setAngle(getAngle() - 1);
	}

	if (playerWasHit(dungeon.getPlayerVector().at(0))) {
		trapAction(dungeon.getPlayerVector().at(0));
	}
}

void DoubleFirebar::setInitialFirePosition(int x, int y, int rows) {
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

	// set sprite positions
	if (isClockwise()) {
		switch (getAngle()) {
		case 1:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 2:
			getInner()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 3:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 4:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 5:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 6:
			getInner()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 7:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 8:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			break;
		}
	}
	else {
		switch (getAngle()) {
		case 1:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 2:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 3:
			getInner()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 4:
			getInner()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 5:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 6:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
			break;
		case 7:
			getInner()->setPosition((x - 1) * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition((x - 2) * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x + 1) * SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x + 2) * SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		case 8:
			getInner()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 1))*SPACING_FACTOR - Y_OFFSET);
			getOuter()->setPosition(x * SPACING_FACTOR - X_OFFSET, (rows - (y - 2))*SPACING_FACTOR - Y_OFFSET);
			// opposite
			getInnerMirror()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 1))*SPACING_FACTOR - Y_OFFSET);
			getOuterMirror()->setPosition((x)* SPACING_FACTOR - X_OFFSET, (rows - (y + 2))*SPACING_FACTOR - Y_OFFSET);
			break;
		}
	}

	setSpriteVisibility(false);
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

void DoubleFirebar::setSpriteColor(cocos2d::Color3B color) {
	getInner()->setColor(color);
	getInnerMirror()->setColor(color);
	getOuter()->setColor(color);
	getOuterMirror()->setColor(color);
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
Lava::Lava(int x, int y) : Traps(x, y, "Lava", "Lava_Tile1_48x48.png", 8) {
	setTemporary(false);
	setActive(false);
	setLethal(true);
	//setEmitsLight(true);
}

void Lava::trapAction(Actors &a) {
	if (!a.isFlying()) {
		// lava sound
		if (a.getName() == PLAYER)
			cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);
	}

	// if not immune to lava or flying, then burn them
	if (!(a.lavaImmune() || a.isFlying())) {
		a.setHP(a.getHP() - getDmg());

		// if actor is still alive and can be burned
		if (a.getHP() > 0 && a.canBeBurned()) {

			// chance to burn
			int roll = randInt(100) - a.getLuck();

			// failed the save roll
			if (roll < 100) {
				a.setBurned(true);
				a.addAffliction(std::make_shared<Burn>());
			}
		}
	}
}

//		SPRINGS
Spring::Spring(int x, int y, bool trigger, char move) : Traps(x, y, SPRING, "Spring_Arrow_Left_48x48.png", 0, true) {
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
	setTemporary(false);
	setActive(trigger); // if it's a trigger, then it's active

	// set image name
	std::string image;
	switch (m_dir) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	case '1': image = "Spring_Arrow_UpRight_48x48.png"; break;
	case '2': image = "Spring_Arrow_UpLeft_48x48.png"; break;
	case '3': image = "Spring_Arrow_DownLeft_48x48.png"; break;
	case '4': image = "Spring_Arrow_DownRight_48x48.png"; break;
	default: image = "cheese.png"; break;
	}
	setImageName(image);
}
Spring::Spring(int x, int y, bool trigger, bool known, bool cardinal) : Traps(x, y, SPRING, "Spring_Arrow_Left_48x48.png", 0, true) {
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
	setTemporary(false);
	setActive(trigger); // if it's a trigger, then it's active

	setImage();
	//// set image name
	//std::string image;
	//switch (getDirection()) {
	//case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	//case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	//case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	//case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	//case '1': image = "Spring_Arrow_UpRight_48x48.png"; break;
	//case '2': image = "Spring_Arrow_UpLeft_48x48.png"; break;
	//case '3': image = "Spring_Arrow_DownLeft_48x48.png"; break;
	//case '4': image = "Spring_Arrow_DownRight_48x48.png"; break;
	//case '#': image = "Spring_Arrow_DownRight_48x48.png"; break;
	//case '+': image = "Spring_Arrow_DownRight_48x48.png"; break;
	//case 'x': image = "Spring_Arrow_DownRight_48x48.png"; break;
	//default: image = "cheese.png"; break;
	//}
	//setImageName(image);
}

void Spring::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int x = a.getPosX();
	int y = a.getPosY();

	int tx = getPosX();
	int ty = getPosY();

	int cols = dungeon.getCols();

	// if it's not a trigger, return
	if (!isTrigger())
		return;


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

	wall = dungeon.getDungeon()[(y + m)*cols + x + n].wall;
	enemy = dungeon.getDungeon()[(y + m)*cols + x + n].enemy;
	hero = dungeon.getDungeon()[(y + m)*cols + x + n].hero;

	// if space is free, move the actor there
	if (a.getName() == "Player") {
		if (!(wall || enemy)) {
			// play trigger sound effect
			cocos2d::experimental::AudioEngine::play2d("Spring_Bounce.mp3", false, (float)exp(-(abs(x - tx) + abs(y - ty)) / 2));

			dungeon.getDungeon()[y*cols + x].hero = false;
			dungeon.getDungeon()[(y + m)*cols + x + n].hero = true;
			a.setPosX(x + n); a.setPosY(y + m);

			// queue player move
			dungeon.queueMoveSprite(a.getSprite(), x + n, y + m);

			// check if there was a trap at this position
			if (dungeon.getDungeon()[(y + m)*cols + x + n].trap) {
				dungeon.trapEncounter(x + n, y + m);
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

			dungeon.queueMoveSprite(a.getSprite(), x + n, y + m);

			// check if there was a trap at this position
			if (dungeon.getDungeon()[(y + m)*cols + x + n].trap) {
				//dungeon.singleMonsterTrapEncounter(dungeon, dungeon.findMonster(a.getPosX(), a.getPosY()));
				dungeon.singleMonsterTrapEncounter(dungeon.findMonster(a.getPosX(), a.getPosY()));
			}
		}
	}
}
void Spring::trapAction(Dungeon &dungeon, Actors &a) {
	int x = a.getPosX();
	int y = a.getPosY();

	int tx = getPosX();
	int ty = getPosY();

	int cols = dungeon.getCols();

	// if actor is flying, return
	if (a.isFlying())
		return;

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


	int n = 0, m = 0; // n : x, m : y
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

	wall = dungeon.getDungeon()[(y + m)*cols + x + n].wall;
	enemy = dungeon.getDungeon()[(y + m)*cols + x + n].enemy;
	hero = dungeon.getDungeon()[(y + m)*cols + x + n].hero;

	// if space is free, move the actor there
	if (a.getName() == "Player") {
		if (!(wall || enemy)) {
			// play trigger sound effect
			cocos2d::experimental::AudioEngine::play2d("Spring_Bounce.mp3", false, (float)exp(-(abs(x - tx) + abs(y - ty)) / 2));

			dungeon.getDungeon()[y*cols + x].hero = false;
			dungeon.getDungeon()[(y + m)*cols + x + n].hero = true;
			a.setPosX(x + n); a.setPosY(y + m);

			// queue player move
			dungeon.queueMoveSprite(a.getSprite(), x + n, y + m);

			// check if there was a trap at this position
			if (dungeon.getDungeon()[(y + m)*cols + x + n].trap) {
				dungeon.trapEncounter(x + n, y + m);
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

			dungeon.queueMoveSprite(a.getSprite(), x + n, y + m);

			// check if there was a trap at this position
			if (dungeon.getDungeon()[(y + m)*cols + x + n].trap) {
				dungeon.singleMonsterTrapEncounter(dungeon.findMonster(a.getPosX(), a.getPosY()));
			}
		}
	}
}

char Spring::getDirection() const {
	return m_dir;
}
void Spring::setDirection(char dir) {
	m_dir = dir;
}
bool Spring::isTrigger() const {
	return m_isTrigger;
}
bool Spring::triggered() const {
	return m_triggered;
}
void Spring::setImage() {
	// set image name
	std::string image;
	switch (getDirection()) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	case '1': image = "Spring_Arrow_UpRight_48x48.png"; break;
	case '2': image = "Spring_Arrow_UpLeft_48x48.png"; break;
	case '3': image = "Spring_Arrow_DownLeft_48x48.png"; break;
	case '4': image = "Spring_Arrow_DownRight_48x48.png"; break;
	case '#': image = "Spring_Arrow_DownRight_48x48.png"; break;
	case '+': image = "Spring_Arrow_DownRight_48x48.png"; break;
	case 'x': image = "Spring_Arrow_DownRight_48x48.png"; break;
	default: image = "cheese.png"; break;
	}
	setImageName(image);
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
bool Spring::isOpposite(Spring other) const {
	char dir1 = getDirection();
	char dir2 = other.getDirection();

	switch (dir1) {
	case 'l': return dir2 == 'r';
	case 'r': return dir2 == 'l';
	case 'u': return dir2 == 'd';
	case 'd': return dir2 == 'u';
	case '1': return dir2 == '3';
	case '2': return dir2 == '4';
	case '3': return dir2 == '1';
	case '4': return dir2 == '2';
	}
}
void Spring::oppositeSprings(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	int n = 0, m = 0;
	char dir = getDirection();
	switch (dir) {
	case 'l': n = -1, m = 0; break;
	case 'r': n = 1, m = 0; break;
	case 'u': n = 0, m = -1; break;
	case 'd': n = 0, m = 1; break;
	case '1': n = 1, m = -1; break;	// Q1
	case '2': n = -1, m = -1; break;// Q2
	case '3': n = -1, m = 1; break;	// Q3
	case '4': n = 1, m = 1; break;	// Q4
	}

	bool isTrap = dungeon[(y + m)*cols + (x + n)].trap;
	if (isTrap) {

		int pos = dungeon.findTrap(x + n, y + m);
		if (pos != -1 && dungeon.getTraps().at(pos)->getItem() == SPRING) {

			// while the two springs point toward each other, reroll this spring's direction
			std::shared_ptr<Spring> spring = std::dynamic_pointer_cast<Spring>(dungeon.getTraps().at(pos));
			while (spring->isOpposite(*this)) {

				switch (1 + randInt(8)) {
				case 1: dir = 'l'; n = -1, m = 0; break;
				case 2: dir = 'r'; n = 1, m = 0; break;
				case 3: dir = 'u'; n = 0, m = -1; break;
				case 4: dir = 'd'; n = 0, m = 1; break;
				case 5: dir = '1'; n = 1, m = -1; break;	// Q1
				case 6: dir = '2'; n = -1, m = -1; break;	// Q2
				case 7: dir = '3'; n = -1, m = 1; break;	// Q3
				case 8: dir = '4'; n = 1, m = 1; break;		// Q4
				}

				setDirection(dir);
				setImage();
			}

			// if spring had to be turned, recursively check for more springs in the new direction it's facing
			if (dir != getDirection()) {
				oppositeSprings(dungeon);
			}
		}
	}
}

//		TURRETS
Turret::Turret(int x, int y, char dir, int range) : Traps(x, y, TURRET, "Spring_Arrow_Left_48x48.png", 4), m_dir(dir), m_range(range), m_triggered(false) {
	setTemporary(false);
	setActive(true);
	setWallFlag(true);

	// set proper image name
	std::string image;
	switch (dir) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	}
	setImageName(image);
}

void Turret::activeTrapAction(Dungeon &dungeon, Actors &a) {
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
void Turret::trapAction(Dungeon &dungeon, Actors &a) {
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
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield()->getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
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
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield()->getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
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
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield()->getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
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
					damage = std::max(0, dungeon.getPlayerVector().at(0).getShield()->getDefense() - damage);
					dungeon.getPlayerVector().at(0).blocked();
				}
				dungeon.getPlayerVector().at(0).setHP(dungeon.getPlayerVector().at(0).getHP() - damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
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
MovingBlock::MovingBlock(int x, int y, char pattern, int spaces) : Traps(x, y, MOVING_BLOCK, "Breakable_Crate_48x48.png", 10), m_pattern(pattern), m_spaces(spaces) {
	m_counter = spaces;
	m_dir = (randInt(2) == 0 ? -1 : 1);

	setTemporary(false);
	setActive(true);
	setWallFlag(true);
}

void MovingBlock::activeTrapAction(Dungeon &dungeon, Actors &a) {
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

			// check if there was a monster, if so, push them
			if (dungeon.getDungeon()[a.getPosY()*cols + (a.getPosX() + m_dir)].enemy) {
				int mx = a.getPosX() + m_dir;
				int my = a.getPosY();
				int pos = dungeon.findMonster(mx, my);

				if (pos != -1) {
					int oldSize = dungeon.getMonsters().size();
					dungeon.pushMonster(dungeon, mx, my, m_dir == 1 ? 'r' : 'l');

					// check if new position has a trap
					if (dungeon.getMonsters().size() == oldSize && dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
						dungeon.singleMonsterTrapEncounter(pos);
					}
				}
			}

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
				dungeon.queueMoveSprite(a.getSprite(), a.getPosX(), a.getPosY());

				// check if there was a trap at this position
				if (dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].trap) {
					dungeon.trapEncounter(a.getPosX(), a.getPosY());
				}
			}
		}

		if (enemy) {
			int pos = dungeon.findMonster(tx + m_dir, ty);

			if (pos != -1) {

				// check for other monsters to push
				if (dungeon[ty*cols + tx + m_dir + m_dir].enemy) {
					int mx = tx + m_dir + m_dir;
					int my = ty;
					int n = dungeon.findMonster(mx, my);

					if (n != -1) {
						int oldSize = dungeon.getMonsters().size();
						dungeon.pushMonster(dungeon, mx, my, m_dir == 1 ? 'r' : 'l');

						// check if new position has a trap
						if (dungeon.getMonsters().size() == oldSize && dungeon[dungeon.getMonsters().at(n)->getPosY()*cols + dungeon.getMonsters().at(n)->getPosX()].trap) {
							dungeon.singleMonsterTrapEncounter(n);
						}
					}
				}

				// check if player is in the way
				if (dungeon[ty*cols + tx + m_dir + m_dir].hero) {
					dungeon.pushPlayer(m_dir == 1 ? 'r' : 'l');
				}

				dungeon.getMonsters().at(pos)->setPosX(tx + m_dir + m_dir);
				dungeon.getDungeon()[ty*cols + tx + m_dir].enemy = false;
				dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].enemy = true;
				dungeon.queueMoveSprite(dungeon.getMonsters().at(pos)->getSprite(), dungeon.getMonsters().at(pos)->getPosX(), dungeon.getMonsters().at(pos)->getPosY());

				// :::: Check trap first, because if monster dies, it will not exist to check for traps ::::
				// check if there was a trap at this position
				int oldSize = dungeon.getMonsters().size();
				if (dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
					dungeon.singleMonsterTrapEncounter(pos);
				}

				// check if there was a wall at this position, if so, just kill the monster outright
				if (dungeon.getMonsters().size() == oldSize && dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].wall) {
					// play bone crunch sound
					playBoneCrunch((float)exp(-(abs(ax - dungeon.getMonsters().at(pos)->getPosX()) + abs(ay - dungeon.getMonsters().at(pos)->getPosY())) / 2));

					dungeon.destroyMonster(pos);
				}
			}
		}

		setPosX(tx + m_dir);
		dungeon.getDungeon()[ty*cols + tx].wall = false;
		dungeon.getDungeon()[ty*cols + tx + m_dir].wall = true;
		dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY());
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

			// check if there was a monster, if so, push them
			if (dungeon.getDungeon()[(a.getPosY() + m_dir)*cols + a.getPosX()].enemy) {
				int mx = a.getPosX();
				int my = a.getPosY() + m_dir;
				int pos = dungeon.findMonster(mx, my);

				if (pos != -1) {
					int oldSize = dungeon.getMonsters().size();
					dungeon.pushMonster(dungeon, mx, my, m_dir == 1 ? 'd' : 'u');

					// check if new position has a trap
					if (dungeon.getMonsters().size() == oldSize && dungeon[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
						dungeon.singleMonsterTrapEncounter(pos);
					}
				}
			}

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
				dungeon.queueMoveSprite(a.getSprite(), a.getPosX(), a.getPosY());

				// check if there was a trap at this position
				if (dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].trap) {
					dungeon.trapEncounter(a.getPosX(), a.getPosY());
				}
			}
		}

		if (enemy) {
			int pos = dungeon.findMonster(tx, ty + m_dir);

			if (pos != -1) {

				// check for other monsters to push
				if (dungeon[(ty + 2 * m_dir)*cols + tx].enemy) {
					int mx = tx;
					int my = ty + m_dir + m_dir;
					int n = dungeon.findMonster(mx, my);

					if (n != -1) {
						int oldSize = dungeon.getMonsters().size();
						dungeon.pushMonster(dungeon, mx, my, m_dir == 1 ? 'd' : 'u');

						// check if new position has a trap
						if (dungeon.getMonsters().size() == oldSize && dungeon[dungeon.getMonsters().at(n)->getPosY()*cols + dungeon.getMonsters().at(n)->getPosX()].trap) {
							dungeon.singleMonsterTrapEncounter(n);
						}
					}
				}

				// check if player is in the way
				if (dungeon[(ty + 2 * m_dir)*cols + tx].hero) {
					dungeon.pushPlayer(m_dir == 1 ? 'd' : 'u');
				}

				dungeon.getMonsters().at(pos)->setPosY(ty + m_dir + m_dir);
				dungeon.getDungeon()[(ty + m_dir)*cols + tx].enemy = false;
				dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].enemy = true;
				dungeon.queueMoveSprite(dungeon.getMonsters().at(pos)->getSprite(), dungeon.getMonsters().at(pos)->getPosX(), dungeon.getMonsters().at(pos)->getPosY());

				// :::: Check trap first, because if monster dies, it will not exist to check for traps ::::
				// check if there was a trap at this position
				int oldSize = dungeon.getMonsters().size();
				if (dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
					dungeon.singleMonsterTrapEncounter(pos);
				}

				// check if there was a wall at this position, if so, just kill the monster outright
				if (dungeon.getMonsters().size() == oldSize && dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].wall) {
					// play bone crunch sound
					playBoneCrunch((float)exp(-(abs(ax - dungeon.getMonsters().at(pos)->getPosX()) + abs(ay - dungeon.getMonsters().at(pos)->getPosY())) / 2));

					dungeon.destroyMonster(pos);
				}
			}
		}

		setPosY(ty + m_dir);
		dungeon.getDungeon()[(ty)*cols + tx].wall = false;
		dungeon.getDungeon()[(ty + m_dir)*cols + tx].wall = true;
		dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY());
		setCounter(getCounter() - 1);

		break;
	}
	}
}
void MovingBlock::trapAction(Dungeon &dungeon, Actors &a) {
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
			// check if there was a monster, if so, push them
			else if (dungeon.getDungeon()[a.getPosY()*cols + (a.getPosX() + m_dir)].enemy) {
				int mx = a.getPosX() + m_dir;
				int my = a.getPosY();
				int pos = dungeon.findMonster(mx, my);

				if (pos != -1) {
					int oldSize = dungeon.getMonsters().size();
					dungeon.pushMonster(dungeon, mx, my, m_dir == 1 ? 'r' : 'l');

					// check if new position has a trap
					if (dungeon.getMonsters().size() == oldSize && dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
						dungeon.singleMonsterTrapEncounter(pos);
					}
				}
			}
			else {
				a.setPosX(tx + m_dir + m_dir);
				dungeon.getDungeon()[ay*cols + ax].hero = false;
				dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].hero = true;
				dungeon.queueMoveSprite(a.getSprite(), a.getPosX(), a.getPosY());

				// check if there was a trap at this position
				if (dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].trap) {
					dungeon.trapEncounter(a.getPosX(), a.getPosY());
				}
			}
		}

		if (enemy) {
			int pos = dungeon.findMonster(tx + m_dir, ty);

			if (pos != -1) {

				// check for other monsters to push
				if (dungeon[ty*cols + tx + m_dir + m_dir].enemy) {
					int mx = tx + m_dir + m_dir;
					int my = ty;
					int n = dungeon.findMonster(mx, my);

					if (n != -1) {
						int oldSize = dungeon.getMonsters().size();
						dungeon.pushMonster(dungeon, mx, my, m_dir == 1 ? 'r' : 'l');

						// check if new position has a trap
						if (dungeon.getMonsters().size() == oldSize && dungeon[dungeon.getMonsters().at(n)->getPosY()*cols + dungeon.getMonsters().at(n)->getPosX()].trap) {
							dungeon.singleMonsterTrapEncounter(n);
						}
					}
				}

				dungeon.getMonsters().at(pos)->setPosX(tx + m_dir + m_dir);
				dungeon.getDungeon()[ty*cols + tx + m_dir].enemy = false;
				dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].enemy = true;
				dungeon.queueMoveSprite(dungeon.getMonsters().at(pos)->getSprite(), dungeon.getMonsters().at(pos)->getPosX(), dungeon.getMonsters().at(pos)->getPosY());

				// :::: Check trap first, because if monster dies, it will not exist to check for traps ::::
				// check if there was a trap at this position
				int oldSize = dungeon.getMonsters().size();
				if (dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
					dungeon.singleMonsterTrapEncounter(pos);
				}

				// check if there was a wall at this position, if so, just kill the monster outright
				if (dungeon.getMonsters().size() == oldSize && dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].wall) {
					// play bone crunch sound
					playBoneCrunch((float)exp(-(abs(ax - dungeon.getMonsters().at(pos)->getPosX()) + abs(ay - dungeon.getMonsters().at(pos)->getPosY())) / 2));

					dungeon.destroyMonster(pos);
				}
			}
		}

		setPosX(tx + m_dir);
		dungeon.getDungeon()[ty*cols + tx].wall = false;
		dungeon.getDungeon()[ty*cols + tx + m_dir].wall = true;
		dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY());
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
			// check if there was a monster, if so, push them
			else if (dungeon.getDungeon()[(a.getPosY() + m_dir)*cols + a.getPosX()].enemy) {
				int mx = a.getPosX();
				int my = a.getPosY() + m_dir;
				int pos = dungeon.findMonster(mx, my);

				if (pos != -1) {
					int oldSize = dungeon.getMonsters().size();
					dungeon.pushMonster(dungeon, mx, my, m_dir == 1 ? 'd' : 'u');

					// check if new position has a trap
					if (dungeon.getMonsters().size() == oldSize && dungeon[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
						dungeon.singleMonsterTrapEncounter(pos);
					}
				}
			}
			else {
				a.setPosY(ty + m_dir + m_dir);
				dungeon.getDungeon()[ay*cols + ax].hero = false;
				dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].hero = true;
				dungeon.queueMoveSprite(a.getSprite(), a.getPosX(), a.getPosY());

				// check if there was a trap at this position
				if (dungeon.getDungeon()[a.getPosY()*cols + a.getPosX()].trap) {
					dungeon.trapEncounter(a.getPosX(), a.getPosY());
				}
			}
		}

		if (enemy) {
			int pos = dungeon.findMonster(tx, ty + m_dir);

			if (pos != -1) {

				// check for other monsters to push
				if (dungeon.getDungeon()[(ty + 2*m_dir)*cols + tx].enemy) {
					int mx = tx;
					int my = ty + m_dir + m_dir;
					int n = dungeon.findMonster(mx, my);

					if (n != -1) {
						int oldSize = dungeon.getMonsters().size();
						dungeon.pushMonster(dungeon, mx, my, m_dir == 1 ? 'd' : 'u');

						// check if new position has a trap
						if (dungeon.getMonsters().size() == oldSize && dungeon[dungeon.getMonsters().at(n)->getPosY()*cols + dungeon.getMonsters().at(n)->getPosX()].trap) {
							dungeon.singleMonsterTrapEncounter(n);
						}
					}
				}

				dungeon.getMonsters().at(pos)->setPosY(ty + m_dir + m_dir);
				dungeon.getDungeon()[(ty + m_dir)*cols + tx].enemy = false;
				dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].enemy = true;
				dungeon.queueMoveSprite(dungeon.getMonsters().at(pos)->getSprite(), dungeon.getMonsters().at(pos)->getPosX(), dungeon.getMonsters().at(pos)->getPosY());

				// :::: Check trap first, because if monster dies, it will not exist to check for traps ::::
				// check if there was a trap at this position
				int oldSize = dungeon.getMonsters().size();
				if (dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].trap) {
					dungeon.singleMonsterTrapEncounter(pos);
				}

				// check if there was a wall at this position, if so, just kill the monster outright
				if (dungeon.getMonsters().size() == oldSize && dungeon.getDungeon()[dungeon.getMonsters().at(pos)->getPosY()*cols + dungeon.getMonsters().at(pos)->getPosX()].wall) {
					// play bone crunch sound
					playBoneCrunch((float)exp(-(abs(ax - dungeon.getMonsters().at(pos)->getPosX()) + abs(ay - dungeon.getMonsters().at(pos)->getPosY())) / 2));

					dungeon.destroyMonster(pos);
				}

			}
		}

		setPosY(ty + m_dir);
		dungeon.getDungeon()[(ty)*cols + tx].wall = false;
		dungeon.getDungeon()[(ty + m_dir)*cols + tx].wall = true;
		dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY());
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

//		ACTIVE BOMB
ActiveBomb::ActiveBomb(int x, int y, int timer) : Traps(x, y, ACTIVE_BOMB, "Bomb_48x48.png", 10), m_timer(timer) {
	m_fuseID = cocos2d::experimental::AudioEngine::play2d("Bomb_Fuse2.mp3", false, 0.8f);
}
ActiveBomb::ActiveBomb(int x, int y, std::string type, std::string image, int damage, int timer) : Traps(x, y, type, image, damage), m_timer(timer) {
	m_fuseID = cocos2d::experimental::AudioEngine::play2d("Bomb_Fuse2.mp3", false, 0.8f);
}

void ActiveBomb::activeTrapAction(Dungeon &dungeon, Actors &a) {
	if (getTimer() > 0) {
		setTimer(getTimer() - 1);
	}
	else {
		// play explosion sound effect
		cocos2d::experimental::AudioEngine::play2d("Explosion.mp3", false, 1.0f);

		explosion(dungeon, a);

		dungeon.getDungeon()[getPosY()*dungeon.getCols() + getPosX()].extra = SPACE;
		dungeon.removeSprite(getSprite());
		dungeon.getTraps().erase(dungeon.getTraps().begin() + dungeon.findTrap(getPosX(), getPosY(), true));
	}
}
void ActiveBomb::explosion(Dungeon &dungeon, Actors &a) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	int x = getPosX();
	int y = getPosY();
	
	int mx, my;

	// stop fuse sound
	cocos2d::experimental::AudioEngine::stop(m_fuseID);

	// check if player was hit or if there is anything to destroy
	if (getItem() == ACTIVE_MEGA_BOMB) {
		// flash floor tiles
		flashFloor(dungeon, x, y, true);

		//	if player is caught in the explosion
		if (abs(px - x) <= 2 && abs(py - y) <= 2) {
			dungeon.getPlayerVector()[0].setHP(dungeon.getPlayerVector()[0].getHP() - 20);

			if (dungeon.getPlayerVector()[0].getHP() <= 0)
				dungeon.getPlayerVector()[0].setDeath(getItem());
		}

		//	destroy stuff
		for (int j = y - 2; j < y + 3; j++) {
			for (int k = x - 2; k < x + 3; k++) {
				if (j != -1 && j != maxrows && !(k == -1 && j <= 0) && !(k == maxcols && j >= maxrows - 1)) { // boundary check

					// destroy any walls in the way
					if (dungeon.getDungeon()[j*maxcols + k].top == WALL) {
						dungeon.getDungeon()[j*maxcols + k].top = SPACE;
						dungeon.getDungeon()[j*maxcols + k].bottom = SPACE;
						dungeon.getDungeon()[j*maxcols + k].wall = false;

						// call remove sprite
						dungeon.removeSprite(dungeon.wall_sprites, maxrows, k, j);
					}

					// destroy any gold in the way
					if (dungeon.getDungeon()[j*maxcols + k].gold != 0) {
						dungeon.getDungeon()[j*maxcols + k].gold = 0;

						dungeon.removeSprite(dungeon.money_sprites, maxrows, k, j);
					}

					// Don't destroy the exit!!
					if (dungeon.getDungeon()[j*maxcols + k].exit) {
						continue;
					}

					// destroy any destructible traps
					if (dungeon.getDungeon()[j*maxcols + k].trap) {
						int n = dungeon.findTrap(k, j);

						if (n != -1) {
							if (dungeon.getTraps().at(n)->isDestructible()) {
								dungeon.getDungeon()[j*maxcols + k].trap = false;
								dungeon.getDungeon()[j*maxcols + k].trap_name = "";

								dungeon.removeSprite(dungeon.getTraps().at(n)->getSprite());
								dungeon.m_trapIndexes.push_back(std::make_pair(k, j));
							}
							else if (dungeon.getTraps().at(n)->isExplosive()) {
								std::shared_ptr<ActiveBomb> bomb = std::dynamic_pointer_cast<ActiveBomb>(dungeon.getTraps().at(n));
								bomb->explosion(dungeon, a);
								bomb.reset();
							}
						}
					}
				}
			}
		}
	}
	// else is regular bomb
	else {
		// flash floor tiles
		flashFloor(dungeon, x, y, false);

		//	if player is caught in the explosion
		if (abs(px - x) <= 1 && abs(py - y) <= 1) {
			dungeon.getPlayerVector()[0].setHP(dungeon.getPlayer().getHP() - 10);

			if (dungeon.getPlayer().getHP() <= 0)
				dungeon.getPlayerVector()[0].setDeath(getItem());
		}

		//	destroy stuff
		for (int j = y - 1; j < y + 2; j++) {
			for (int k = x - 1; k < x + 2; k++) {

				// destroy walls
				if (dungeon.getDungeon()[j*maxcols + k].top == WALL) {
					dungeon.getDungeon()[j*maxcols + k].top = SPACE;
					dungeon.getDungeon()[j*maxcols + k].bottom = SPACE;
					dungeon.getDungeon()[j*maxcols + k].wall = false;

					// call remove sprite twice to remove top and bottom walls
					dungeon.removeSprite(dungeon.wall_sprites, maxrows, k, j);
				}

				// destroy any gold
				if (dungeon.getDungeon()[j*maxcols + k].gold != 0) {
					dungeon.getDungeon()[j*maxcols + k].gold = 0;

					dungeon.removeSprite(dungeon.money_sprites, maxrows, k, j);
				}

				// Don't destroy the exit!!
				if (dungeon.getDungeon()[j*maxcols + k].exit) {
					continue;
				}

				// destroy any destructible traps
				if (dungeon.getDungeon()[j*maxcols + k].trap) {
					int n = dungeon.findTrap(k, j);

					if (n != -1) {
						if (dungeon.getTraps().at(n)->isDestructible()) {
							dungeon.getDungeon()[j*maxcols + k].trap = false;
							dungeon.getDungeon()[j*maxcols + k].trap_name = "";

							dungeon.removeSprite(dungeon.getTraps().at(n)->getSprite());
							dungeon.m_trapIndexes.push_back(std::make_pair(k, j));
						}
						else if (dungeon.getTraps().at(n)->isExplosive()) {
							std::shared_ptr<ActiveBomb> bomb = std::dynamic_pointer_cast<ActiveBomb>(dungeon.getTraps().at(n));
							bomb->explosion(dungeon, a);
							bomb.reset();
						}
					}
				}
			}
		}
	}


	// find any monsters caught in the blast
	int pos = dungeon.findTrap(x, y); // finds the bomb

	for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		if (getItem() == ACTIVE_MEGA_BOMB) {
			if (abs(mx - x) <= 2 && abs(my - y) <= 2) {
				dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - 20);

				if (dungeon.getMonsters().at(i)->getHP() <= 0) {
					dungeon.monsterDeath(i);
				}
			}
		}
		// else is a regular bomb
		else if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - 10);

			if (dungeon.getMonsters().at(i)->getHP() <= 0) {
				dungeon.monsterDeath(i);
			}
		}
	}
}

int ActiveBomb::getTimer() const {
	return m_timer;
}
void ActiveBomb::setTimer(int timer) {
	m_timer = timer;
}

//		ACTIVE MEGA BOMB
ActiveMegaBomb::ActiveMegaBomb(int x, int y) : ActiveBomb(x, y, ACTIVE_MEGA_BOMB, "Bomb_48x48.png", 20) {

}

void ActiveMegaBomb::activeTrapAction(Dungeon &dungeon, Actors &a) {
	if (getTimer() > 0) {
		setTimer(getTimer() - 1);
	}
	else {
		// play explosion sound effect
		cocos2d::experimental::AudioEngine::play2d("Mega_Explosion.mp3", false, 1.0);

		explosion(dungeon, a);

		dungeon.getDungeon()[getPosY()*dungeon.getCols() + getPosX()].extra = SPACE;
		dungeon.removeSprite(getSprite());
		dungeon.getTraps().erase(dungeon.getTraps().begin() + dungeon.findTrap(getPosX(), getPosY(), true));
	}
}

//		POISON BOMB
PoisonBomb::PoisonBomb(int x, int y, int turns) : ActiveBomb(x, y, POISON_BOMB, "Green_Orb_32x32.png", 6, 0) {
	m_turns = turns;
	m_blown = false;
	setTemporary(true);
	setActive(false);
	setExplosive(true);
}

void PoisonBomb::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int ax = a.getPosX();
	int ay = a.getPosY();
	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	if (!m_triggered) {
		return;
	}

	// add effect to show poison
	poisonCloud(dungeon, x, y, 0.0f, cocos2d::Color3B(35, 140, 35));

	// damage anything else that's in the cloud
	radiusCheck(dungeon, a);

	// if actor is in range, and turns are left on the poison cloud, add poison
	if (abs(ax - x) <= 2 && abs(ay - y) <=2 && !(ay == y - 2 && ax == x - 2) && !(ay == y - 2 && ax == x + 2) && !(ay == y + 2 && ax == x - 2) && !(ay == y + 2 && ax == x + 2) && m_turns > 0) {

		if (a.canBePoisoned()) {
			a.addAffliction(std::make_shared<Poison>(4, 4, 1, 1));
		}
	}


	// if turns have run out, remove the gas cloud
	else if (m_turns == 0) {
		// remove poison cloud effect
		//poisonCloud(dungeon, x, y, 0.2f, cocos2d::Color3B(200, 200, 200));

		dungeon.getTraps().erase(dungeon.getTraps().begin() + dungeon.findTrap(x, y));
		return;
	}
	m_turns--;
}
void PoisonBomb::trapAction(Dungeon &dungeon, Actors &a) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();
	int ax = a.getPosX();
	int ay = a.getPosY();

	// if it hasn't been set off yet and something is standing on it
	if (!m_triggered && ax == x && ay == y) {
		explosion(dungeon, a);

		return;
	}
	else if (m_triggered) {
		activeTrapAction(dungeon, a);
	}
}
void PoisonBomb::explosion(Dungeon &dungeon, Actors &a) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();
	int ax = a.getPosX();
	int ay = a.getPosY();
	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();

	// poison sound effect
	cocos2d::experimental::AudioEngine::play2d("Poison_Bomb_Explosion.mp3", false, 1.0f);


	// if bomb was set off by Actor a (bombs can set this off too)
	if (ax == x && ay == y) {
		// the enemy that sets the bomb off receives the most damage
		a.setHP(a.getHP() - getDmg());

		if (a.canBePoisoned()) {
			a.addAffliction(std::make_shared<Poison>(8, 4, 1, 1));
		}
	}

	radiusCheck(dungeon, a);
	m_blown = true; // flag so that damage is not repeatedly dealt after initial explosion

	/*
	int pos, mx, my;
	bool enemy;
	// hurt anyone that was in the blast radius: damage is reduced depending on how far from the blast you were
	for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 2; j < x + 3; j++) {
			if (i != -1 && i != rows && !(j == -1 && i <= 0) && !(j == cols && i >= rows - 1) && !(j == x && i == y) &&
				!(i == y - 2 && j == x - 2) && !(i == y - 2 && j == x + 2) && !(i == y + 2 && j == x - 2) && !(i == y + 2 && j == x + 2)) { // boundary check

				// check if player was hit, and they were not the one that set the bomb off
				if (j == px && i == py && !(px == a.getPosX() && py == a.getPosY())) {
					
					// if on the outer ring, damage is reduced by 4. if inner ring, damage reduced by 2
					int damageReduction = (abs(px - j) == 2 || abs(py - i) == 2 ? 4 : 2);
					dungeon.getPlayerVector()[0].setHP(dungeon.getPlayer().getHP() - (getDmg() + damageReduction));

					if (dungeon.getPlayer().canBePoisoned()) {
						dungeon.getPlayerVector()[0].addAffliction(std::make_shared<Poison>(8, 4, 1, 1));
					}
				}

				if (dungeon[i*cols + j].enemy) {
					pos = dungeon.findMonster(j, i);

					if (pos != -1) {
						mx = dungeon.getMonsters()[pos]->getPosX();
						my = dungeon.getMonsters()[pos]->getPosY();

						// check if any monsters were hit, and they were not the one that the bomb off
						if (j == mx && i == my && !(mx == a.getPosX() && my == a.getPosY())) {

							// if on the outer ring, damage is reduced by 4. if inner ring, damage reduced by 2
							int damageReduction = (abs(mx - j) == 2 || abs(my - i) == 2 ? 4 : 2);
							dungeon.getMonsters()[pos]->setHP(dungeon.getMonsters()[pos]->getHP() - (getDmg() + damageReduction));

							if (dungeon.getMonsters()[pos]->canBePoisoned()) {
								dungeon.getMonsters()[pos]->addAffliction(std::make_shared<Poison>(8, 4, 1, 1));
							}
						}
					}
				}
			}
		}
	}
	*/

	dungeon[y*cols + x].trap_name = "";
	dungeon[y*cols + x].trap = false;
	m_triggered = true;

	// add effect to show poison
	poisonCloud(dungeon, x, y, 0.8f, cocos2d::Color3B(35, 140, 35));
	getSprite()->setVisible(false); // hide the sprite because the trap still needs to exist
}
void PoisonBomb::radiusCheck(Dungeon &dungeon, Actors &a) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();
	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();


	int pos, mx, my;
	
	// hurt anyone that was in the blast radius: damage is reduced depending on how far from the blast you were
	for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 2; j < x + 3; j++) {

			// boundary and corner check
			if (i != -1 && i != rows && !(j == -1 && i <= 0) && !(j == cols && i >= rows - 1) && !(j == x && i == y) &&
				!(i == y - 2 && j == x - 2) && !(i == y - 2 && j == x + 2) && !(i == y + 2 && j == x - 2) && !(i == y + 2 && j == x + 2)) { 

				// check if player was hit, and they were not the one that set the bomb off
				if (j == px && i == py && !(px == a.getPosX() && py == a.getPosY())) {

					// if bomb just exploded
					if (!m_blown) {
						// if on the outer ring, damage is reduced by 4. if inner ring, damage reduced by 2
						int damageReduction = (abs(px - j) == 2 || abs(py - i) == 2 ? 4 : 2);
						dungeon.getPlayerVector()[0].setHP(dungeon.getPlayer().getHP() - (getDmg() - damageReduction));

						if (dungeon.getPlayer().canBePoisoned()) {
							dungeon.getPlayerVector()[0].addAffliction(std::make_shared<Poison>(8, 4, 1, 1));
						}
					}
					// otherwise add a weaker poison
					else if (dungeon.getPlayer().canBePoisoned()) {
						dungeon.getPlayerVector()[0].addAffliction(std::make_shared<Poison>(4, 4, 1, 1));
					}
				}

				// check for enemies in the gas cloud
				if (dungeon[i*cols + j].enemy) {
					pos = dungeon.findMonster(j, i);

					if (pos != -1) {
						mx = dungeon.getMonsters()[pos]->getPosX();
						my = dungeon.getMonsters()[pos]->getPosY();

						// check if any monsters were hit, and they were not the one that the bomb off
						if (j == mx && i == my && !(mx == a.getPosX() && my == a.getPosY())) {

							// if bomb just exploded
							if (!m_blown) {
								// if on the outer ring, damage is reduced by 4. if inner ring, damage reduced by 2
								int damageReduction = (abs(mx - j) == 2 || abs(my - i) == 2 ? 4 : 2);
								dungeon.getMonsters()[pos]->setHP(dungeon.getMonsters()[pos]->getHP() - (getDmg() - damageReduction));

								if (dungeon.getMonsters()[pos]->canBePoisoned()) {
									dungeon.getMonsters()[pos]->addAffliction(std::make_shared<Poison>(8, 4, 1, 1));
								}
							}
							// else add a weaker poison
							else if (dungeon.getMonsters()[pos]->canBePoisoned()) {
								dungeon.getMonsters()[pos]->addAffliction(std::make_shared<Poison>(4, 4, 1, 1));
							}
						}
					}
				}
			}
		}
	}
}

//		BEAR TRAP
BearTrap::BearTrap(int x, int y) : Traps(x, y, BEAR_TRAP, "Bear_Trap_48x48.png", 4, true) {
	
}

void BearTrap::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int ax = a.getPosX();
	int ay = a.getPosY();

	if (!m_triggered) {
		return;
	}

	// if actor died, remove the trap
	if (a.getHP() <= 0) {
		dungeon.removeSprite(getSprite());
		dungeon.getTraps().erase(dungeon.getTraps().begin() + dungeon.findTrap(x, y));

		return;
	}

	if (m_wait == 0) {
		a.addAffliction(std::make_shared<Stun>(1));
		m_wait = 1;
	}
	else {
		m_wait--;
	}
}
void BearTrap::trapAction(Dungeon &dungeon, Actors &a) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();
	int ax = a.getPosX();
	int ay = a.getPosY();

	// if actor is flying, return
	if (a.isFlying())
		return;

	// if it hasn't been set off yet and something is standing on it
	if (!m_triggered && ax == x && ay == y) {
		// metal close sound effect
		//

		a.setHP(a.getHP() - getDmg());
		a.addAffliction(std::make_shared<Bleed>(3));
		//a.addAffliction(std::make_shared<Stuck>(1000));
		m_triggered = true;

		// if actor died, remove the trap
		if (a.getHP() <= 0) {
			dungeon.removeSprite(getSprite());
			dungeon.getTraps().erase(dungeon.getTraps().begin() + dungeon.findTrap(x, y));
		}

		return;
	}
	else if (m_triggered) {
		activeTrapAction(dungeon, a);
	}
}

//		CRUMBLE FLOOR
CrumbleFloor::CrumbleFloor(int x, int y, int strength) : Traps(x, y, CRUMBLE_FLOOR, "Crumble_Floor1_48x48.png", 0, false), m_strength(strength) {

}
CrumbleFloor::CrumbleFloor(int x, int y, int strength, std::string name, std::string image) : Traps(x, y, name, image, 0, false), m_strength(strength) {

}

void CrumbleFloor::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int ax = a.getPosX();
	int ay = a.getPosY();
	int x = getPosX();
	int y = getPosY();
	int px = dungeon.getPlayer().getPosX();
	int py = dungeon.getPlayer().getPosY();
	
	if (m_crossed && !(ax == x && ay == y)) {
		m_crossed = false;
		m_strength--;

		// crumbling sound effect
		playCrumble(exp(-(abs(px - x) + abs(py - y))));

		// if strength is exhausted, this trap is replaced by a pit
		if (m_strength == 0 && !(ax == x && ay == y)) {
			// play fade animation
			fadeOut(getSprite());

			// breaking & crumbling sound effect
			cocos2d::experimental::AudioEngine::play2d("Crumble_Breaking.mp3", false, exp(-(abs(px - x) + abs(py - y))));

			dungeon.m_trapIndexes.push_back(std::make_pair(x, y)); // queue trap for removal

			if (this->getItem() == CRUMBLE_FLOOR) {
				std::shared_ptr<Traps> pit = std::make_shared<Pit>(x, y);
				pit->setSprite(dungeon.createSprite(x, y, -4, pit->getImageName()));
				dungeon.getTraps().push_back(pit);
			}
			else if (this->getItem() == CRUMBLE_LAVA) {
				std::shared_ptr<Traps> lava = std::make_shared<Lava>(x, y);
				lava->setSprite(dungeon.createSprite(x, y, -4, lava->getImageName()));
				dungeon.getTraps().push_back(lava);
			}
		}
		else if (m_strength == 1 && !(ax == x && ay == y)) {
			getSprite()->removeFromParent();
			setSprite(dungeon.createSprite(x, y, -4, "Crumble_Floor3_48x48.png"));
		}
		else if (m_strength == 2 && !(ax == x && ay == y)) {
			getSprite()->removeFromParent();
			setSprite(dungeon.createSprite(x, y, -4, "Crumble_Floor2_48x48.png"));
		}
	}
}
void CrumbleFloor::trapAction(Dungeon &dungeon, Actors &a) {
	int ax = a.getPosX();
	int ay = a.getPosY();
	int x = getPosX();
	int y = getPosY();

	// if tile was walked over, reduce strength of the floor
	if (m_crossed) {
		m_strength--;
	}

	// if player just walked on it, then flag it for next turn
	if (ax == x && ay == y && !a.isFlying()) {
		m_crossed = true;
	}
}


CrumbleLava::CrumbleLava(int x, int y, int strength) : CrumbleFloor(x, y, strength, CRUMBLE_LAVA, "Crumble_Floor1_48x48.png") {

}


//		SOUND EFFECTS
void playBoneCrunch(float volume) {
	switch (randInt(1)) {
	case 0:	cocos2d::experimental::AudioEngine::play2d("Bone_Crack1.mp3", false, volume);
	case 1: cocos2d::experimental::AudioEngine::play2d("Bone_Crack2.mp3", false, volume);
	}
}
void playCrumble(float volume) {
	std::string sound;
	switch (randInt(4)) {
	case 0:	sound = "RockFootStep1.mp3"; break;
	case 1: sound = "RockFootStep2.mp3"; break;
	case 2: sound = "RockFootStep3.mp3"; break;
	case 3: sound = "RockFootStep4.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, volume);
}

void poisonCloud(Dungeon &dungeon, int x, int y, int time, cocos2d::Color3B color) {
	int cols = dungeon.getCols();
	int rows = dungeon.getRows();

	for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 2; j < x + 3; j++) {
			if (i != -1 && i != rows && !(j == -1 && i <= 0) && !(j == cols && i >= rows - 1) &&
				!(i == y - 2 && j == x - 2) && !(i == y - 2 && j == x + 2) && !(i == y + 2 && j == x - 2) && !(i == y + 2 && j == x + 2)) { // boundary check
				auto tintGreen = cocos2d::TintTo::create(time, color);
				dungeon[i*cols + j].floor->runAction(tintGreen);
			}
		}
	}
}
void fadeOut(cocos2d::Sprite* sprite) {
	auto fade = cocos2d::FadeOut::create(0.5);
	sprite->runAction(fade);
}
void flashFloor(Dungeon &dungeon, int x, int y, bool mega) {
	int cols = dungeon.getCols();
	int rows = dungeon.getRows();

	for (int i = y - (1 + mega); i < y + (2 + mega); i++) {
		for (int j = x - (1 + mega); j < x + (2 + mega); j++) {
			if (i != -1 && i != rows && !(j == -1 && i <= 0) && !(j == cols && i >= rows - 1)) { // boundary check
				auto tintWhite = cocos2d::TintTo::create(0.08f, cocos2d::Color3B(183, 255, 255));
				auto untint = cocos2d::TintTo::create(0.08f, cocos2d::Color3B(200, 200, 200));
				auto tinting = cocos2d::Sequence::createWithTwoActions(tintWhite, untint);
				dungeon[i*cols + j].floor->runAction(tinting);
				dungeon[i*cols + j].floor->runAction(cocos2d::Blink::create(0.08f, 1));
			}
		}
	}
}