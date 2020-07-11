#include "cocos2d.h"
#include "AudioEngine.h"
#include "global.h"
#include "GUI.h"
#include "GameObjects.h"
#include "Actors.h"
#include "Dungeon.h"
#include "Afflictions.h"
#include "FX.h"
#include <string>
#include <iostream>
#include <memory>

using std::string;


//		OBJECT FUNCTIONS
Objects::Objects() : m_x(0), m_y(0), m_name("") {

}
Objects::Objects(std::string status) : m_name(status) {

}
Objects::Objects(std::string item, std::string image) : m_name(item), m_image(image) {

}
Objects::Objects(int x, int y, string item) : m_x(x), m_y(y), m_name(item) {

}
Objects::Objects(int x, int y, std::string item, std::string image) : m_x(x), m_y(y), m_name(item), m_image(image) {

}
Objects::~Objects() {

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

void Objects::setDestructible(bool destructible) {
	m_destructible = destructible;
}
void Objects::setWeaponFlag(bool weapon) {
	m_isWeapon = weapon;
}
void Objects::setShieldFlag(bool shield) {
	m_isShield = shield;
}
void Objects::setItemFlag(bool item) {
	m_isItem = item;
}
void Objects::setAutoFlag(bool autoUse) {
	m_autoUse = autoUse;
}
void Objects::setChestFlag(bool chest) {
	m_isChest = chest;
}
void Objects::setTrinketFlag(bool trinket) {
	m_isTrinket = trinket;
}

void Objects::setDescription(std::string description) {
	m_description = description;
}

void Objects::setSoundName(std::string sound) {
	m_sound = sound;
}
void Objects::setExtraSpritesFlag(bool extras) {
	m_hasExtraSprites = extras;
}
void Objects::setEmitsLight(bool emits) {
	m_emitsLight = emits;
}


//		IDOL
Idol::Idol() : Objects(randInt(68) + 1, randInt(16) + 1, "Idol") {

}

//		STAIRS
Stairs::Stairs(int x, int y) : Traps(x, y, STAIRCASE, "Stairs_48x48.png", 0) {

}

//		DOORS
Door::Door(int x, int y) : Objects(x, y, DOOR), m_open(false), m_hold(true), m_lock(false) {

}


//		BUTTON
Button::Button(int x, int y) : Traps(x, y, BUTTON_UNPRESSED, "Button_Unpressed_48x48.png", 0) {

}


//	:::: DROPS ::::

Drops::Drops(int x, int y, std::string item, std::string image) : Objects(x, y, item, image) {
	
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
	}
}

LifePotion::LifePotion(int x, int y) : Drops(x, y, LIFEPOT, "Life_Potion_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A red elixir which seems to keep well. Gives you life.");
}
void LifePotion::useItem(Player &p) {
	// sound effect
	playSound("Life_Potion_Used.mp3");

	int healing = 15 + (p.hasPotentPotions() ? 8 : 0);
	if (p.getHP() < p.getMaxHP()) {			// if player hp is less than their max
		if (p.getHP() + healing <= p.getMaxHP()) // if player hp + effect of lifepot will refill less than or equal to max
			p.setHP(p.getHP() + healing);
		else	// just set player hp to max
			p.setHP(p.getMaxHP());
	}

}

BigLifePotion::BigLifePotion(int x, int y) : Drops(x, y, BIG_LIFEPOT, "Big_Health_Potion_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("The alchemist's improved formula. Gives you more life.");
}
void BigLifePotion::useItem(Player &p) {
	// sound effect
	playSound("Life_Potion_Used.mp3");

	int healing = 25 + (p.hasPotentPotions() ? 13 : 0);
	if (p.getHP() < p.getMaxHP()) {			// if player hp is less than their max
		if (p.getHP() + healing <= p.getMaxHP()) // if player hp + effect of lifepot will refill less than or equal to max
			p.setHP(p.getHP() + healing);
		else	// just set player hp to max
			p.setHP(p.getMaxHP());
	}

}

SteadyLifePotion::SteadyLifePotion(int x, int y) : Drops(x, y, STEADY_LIFEPOT, "Wine_Bottle_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A slow, but powerful acting mixture to heal your wounds.");
}
void SteadyLifePotion::useItem(Player &p) {
	// sound effect
	playSound("Life_Potion_Used.mp3");

	int turns = 25 + (p.hasPotentPotions() ? 10 : 0);
	p.addAffliction(std::make_shared<HealOverTime>(turns));
}

HalfLifePotion::HalfLifePotion(int x, int y) : Drops(x, y, HALF_LIFE_POTION, "Star_Potion_32x32.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A powerful potion, but only restores up to half of\nyour maximum health.");
}
void HalfLifePotion::useItem(Player &p) {
	// sound effect
	playSound("Life_Potion_Used.mp3");

	int healing = (p.getMaxHP() / 2) + (p.hasPotentPotions() ? 10 : 0);

	// If player hp is less than half their max, then restore it to the healing value
	if (p.getHP() <= p.getMaxHP() / 2) {
		p.setHP(healing);
	}
}

SoulPotion::SoulPotion(int x, int y) : Drops(x, y, SOUL_POTION, "Gold_Coin_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Converts your currency into health. The mages of old\nknew of methods to trade greed for well-being.");
}
void SoulPotion::useItem(Player &p) {
	// Every 50 gold = 5% of health refilled

	playSound("Life_Potion_Used.mp3");

	if (p.getHP() == p.getMaxHP())
		return;

	float healing = 0;

	int i;
	for (i = p.getMoney(); i >= 50; i -= 50) {
		healing += 0.05f;

		// Stop if this would heal the player fully already
		if (p.getHP() + p.getMaxHP() * healing >= p.getMaxHP())
			break;
	}

	int amount = p.getMaxHP() * healing;
	if (p.getHP() + amount < p.getMaxHP())
		p.setHP(p.getHP() + amount);
	else
		p.setHP(p.getMaxHP());

	p.setMoney(i);
}

BinaryLifePotion::BinaryLifePotion(int x, int y) : Drops(x, y, BINARY_LIFE_POTION, "Star_Potion_32x32.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A failed experiment led to this potion's creation.\nOnly restores half of your missing health.");
}
void BinaryLifePotion::useItem(Player &p) {
	playSound("Life_Potion_Used.mp3");

	int healing = (p.getMaxHP() - p.getHP()) / 2;
	if (p.getHP() + healing < p.getMaxHP())
		p.setHP(p.getHP() + healing);
	else
		p.setHP(p.getMaxHP());
}

StatPotion::StatPotion(int x, int y) : Drops(x, y, STATPOT, "Stat_Potion_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Increases your combat abilities, but strangely it removes\n the taste from your mouth.");
}
void StatPotion::useItem(Player &p) {
	// sound effect
	playSound("Potion_Used.mp3");

	p.setMaxHP(p.getMaxHP() + 5);

	int turns = 30 + (p.hasPotentPotions() ? 20 : 0);
	int stats = 2 + (p.hasPotentPotions() ? 1 : 0);
	p.addAffliction(std::make_shared<Buff>(turns, stats, stats, stats));
}

RottenApple::RottenApple(int x, int y) : Drops(x, y, ROTTEN_APPLE, "Apple_Worm_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Uhh... it might not be the best idea to consume this.");
}
void RottenApple::useItem(Player &p) {
	// sound effect
	playSound("Life_Potion_Used.mp3");

	p.setHP(p.getMaxHP());
	p.addAffliction(std::make_shared<Poison>(40, 5, 2, 2));
}

Antidote::Antidote(int x, int y) : Drops(x, y, ANTIDOTE, "Star_Potion_32x32.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Puts an end to what would otherwise be your end.\n Cures poison.");
}
void Antidote::useItem(Player &p) {
	playSound("Relief_Female.mp3");

	if (p.isPoisoned())
		p.removeAffliction(POISON);
}

ArmorDrop::ArmorDrop(int x, int y) : Drops(x, y, ARMOR, "Armor_48x48.png") {
	setItemFlag(true);
	setSoundName("Armor_Pickup.mp3");

	setDescription("Just what a lost adventurer needs to help protect\nthemselves. A little more armor to keep you in one piece.");
}
void ArmorDrop::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setArmor(p.getArmor() + 1);
}

ShieldRepair::ShieldRepair(int x, int y) : Drops(x, y, SHIELD_REPAIR, "Shield_Repair_48x48.png") {
	setAutoFlag(true);
	setSoundName("Armor_Pickup.mp3");
}
void ShieldRepair::useItem(Player &p) {
	p.restoreActive(10);
}

DizzyElixir::DizzyElixir(int x, int y) : Drops(x, y, DIZZY_ELIXIR, "Dizzy_Elixir_48x48.png") {
	setItemFlag(true);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A strange concoction that makes you feel good, but...");
}
void DizzyElixir::useItem(Player &p) {
	// sound effect
	playSound("Potion_Used.mp3");

	int dexBoost = 3 + (p.hasPotentPotions() ? 2 : 0);
	p.addAffliction(std::make_shared<Confusion>(20 + p.getInt(), dexBoost));
}

PoisonCloud::PoisonCloud(int x, int y) : Drops(x, y, POISON_CLOUD, "Poison_Bomb_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("WARNING: Do NOT ingest. Handle with care!");
}
void PoisonCloud::useItem(Dungeon &dungeon) {
	playSound("Bomb_Placed.mp3");

	setPosX(dungeon.getPlayer()->getPosX());
	setPosY(dungeon.getPlayer()->getPosY());

	dungeon[getPosY()*dungeon.getCols() + getPosX()].trap = true;

	// add poison cloud bomb to the traps vector, as it only goes off when something walks over it
	std::shared_ptr<PoisonBomb> bomb = std::make_shared<PoisonBomb>(getPosX(), getPosY(), 10);
	dungeon.getTraps().push_back(bomb);

	// add bomb sprite
	dungeon.getTraps().back()->setSprite(dungeon.createSprite(dungeon.getPlayer()->getPosX(), dungeon.getPlayer()->getPosY(), -1, bomb->getImageName()));
}

RottenMeat::RottenMeat(int x, int y) : Drops(x, y, ROTTEN_MEAT, "Honeycomb_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Definitely do not eat this. The beasts of this\nplace will probably enjoy this more than you.");
}
void RottenMeat::useItem(Dungeon &dungeon) {
	playSound("Bomb_Placed.mp3");

	setPosX(dungeon.getPlayer()->getPosX());
	setPosY(dungeon.getPlayer()->getPosY());

	dungeon[getPosY()*dungeon.getCols() + getPosX()].trap = true;

	std::shared_ptr<RottingDecoy> decoy = std::make_shared<RottingDecoy>(getPosX(), getPosY());
	dungeon.getTraps().push_back(decoy);
	dungeon.addDecoy(decoy);

	// add sprite
	dungeon.getTraps().back()->setSprite(dungeon.createSprite(getPosX(), getPosY(), -1, decoy->getImageName()));
}

MattockDust::MattockDust(int x, int y) : Drops(x, y, MATTOCK_DUST, "Silver_Kettle_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Now you've done it.");
}
void MattockDust::useItem(Dungeon &dungeon) {
	playSound("Bomb_Placed.mp3");

	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	for (int i = px - 1; i < px + 2; i++) {
		for (int j = py - 1; j < py + 2; j++) {
			
			if (i == px && j == py)
				continue;

			if (dungeon[j*cols + i].enemy) {
				int pos = dungeon.findMonster(i, j);
				if (pos != -1)
					dungeon.giveAffliction(pos, std::make_shared<Confusion>(5, 0));			
			}
		}
	}

	dungeon.getPlayerVector()[0]->addAffliction(std::make_shared<Confusion>(5, 0));
}

Teleport::Teleport(int x, int y) : Drops(x, y, TELEPORT, "Teleport_Scroll_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("TurningPage3.mp3");

	setDescription("Wait. Where am I?");
}
void Teleport::useItem(Dungeon &dungeon) {
	// play teleport sound
	playSound("Teleport_Spell.mp3");

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = 1 + randInt(cols - 2);
	int y = 1 + randInt(rows - 2);

	bool enemy, wall, trap;
	enemy = dungeon[y*cols + x].enemy;
	wall = dungeon[y*cols + x].wall;
	trap = dungeon[y*cols + x].trap;

	while (enemy || wall || trap) {
		x = 1 + randInt(cols - 2);
		y = 1 + randInt(rows - 2);

		enemy = dungeon[y*cols + x].enemy;
		wall = dungeon[y*cols + x].wall;
		trap = dungeon[y*cols + x].trap;
	}

	dungeon[dungeon.getPlayer()->getPosY()*cols + dungeon.getPlayer()->getPosX()].hero = false;
	dungeon[y*cols + x].hero = true;
	dungeon.getPlayerVector()[0]->setPosX(x);
	dungeon.getPlayerVector()[0]->setPosY(y);

	dungeon.teleportSprite(dungeon.getPlayer()->getSprite(), x, y);

	// if player has enough intellect, give them invulnerability for a few turns
	if (dungeon.getPlayer()->getInt() >= 5) {
		dungeon.getPlayerVector()[0]->addAffliction(std::make_shared<Invulnerability>(3));
	}
}

SmokeBomb::SmokeBomb(int x, int y) : Drops(x, y, SMOKE_BOMB, "Pumpkin_32x32.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Toss this to unleash a cloud of smoke.");
}
void SmokeBomb::useItem(Dungeon &dungeon) {
	playSound("Fireblast_Spell1.mp3");

	int cols = dungeon.getCols();
	bool wall, enemy;
	char move = dungeon.getPlayer()->facingDirection();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int n = 0, m = 0;

	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;

	int range = 5;

	while (!(wall || enemy) && range > 0) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

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

	// Reassign for less work
	x = x + n;
	y = y + m;

	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {

			// Boundary and corner check
			if (!dungeon.withinBounds(i, j) || (i == x + 2 && j == y - 2) || (i == x + 2 && j == y + 2) || (i == x - 2 && j == y - 2) || (i == x - 2 && j == y + 2))
				continue;

			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, i, j, 2);

			if (dungeon[j*cols + i].enemy) {
				int pos = dungeon.findMonster(i, j);
				if (pos != -1)
					dungeon.giveAffliction(pos, std::make_shared<Possessed>(8));				
			}
			
		}
	}
}

// STACKABLES
Stackable::Stackable(int x, int y, std::string name, std::string image, int count) : Drops(x, y, name, image), m_count(count) {
	setCanStack(true);
}

Bomb::Bomb(int x, int y) : Stackable(x, y, BOMB, "Bomb_48x48.png", 1) {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Makes things go boom.");
}
void Bomb::useItem(Dungeon &dungeon) {
	playSound("Bomb_Placed.mp3");

	// Decrease count
	decreaseCount();

	setPosX(dungeon.getPlayer()->getPosX());
	setPosY(dungeon.getPlayer()->getPosY());

	dungeon.getTraps().push_back(std::make_shared<ActiveBomb>(getPosX(), getPosY()));

	// add bomb sprite
	dungeon.getTraps().back()->setSprite(dungeon.createSprite(dungeon.getPlayer()->getPosX(), dungeon.getPlayer()->getPosY(), -1, "Bomb_48x48.png"));
}

BearTrap::BearTrap(int x, int y) : Stackable(x, y, BEAR_TRAP, "Blue_Toy_32x32.png", 3) {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("A standard contraption that can immobilize even\nthe largest of foes.");
}
void BearTrap::useItem(Dungeon &dungeon) {
	playSound("Bomb_Placed.mp3");

	// Decrease count
	decreaseCount();

	setPosX(dungeon.getPlayer()->getPosX());
	setPosY(dungeon.getPlayer()->getPosY());

	dungeon[getPosY()*dungeon.getCols() + getPosX()].trap = true;

	// Add to the traps vector, as it only goes off when something walks over it
	std::shared_ptr<SetBearTrap> trap = std::make_shared<SetBearTrap>(getPosX(), getPosY());
	dungeon.getTraps().push_back(trap);

	// add sprite
	dungeon.getTraps().back()->setSprite(dungeon.createSprite(dungeon.getPlayer()->getPosX(), dungeon.getPlayer()->getPosY(), -1, trap->getImageName()));
}

Matches::Matches(int x, int y) : Stackable(x, y, MATCHES, "CeilingSpike_48x48.png", 3) {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Light a few places or light a few faces.");
}
void Matches::useItem(Dungeon &dungeon) {
	playSound("Fireblast_Spell2.mp3");

	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	char move = dungeon.getPlayer()->facingDirection();

	int n = 0, m = 0;
	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	// Decrease count
	decreaseCount();

	if (dungeon[(y + m)*cols + (x + n)].trap) {
		int pos = dungeon.findTrap(x + n, y + m);
		if (pos != -1) {
			if (dungeon.getTraps().at(pos)->canBeIgnited())
				dungeon.getTraps().at(pos)->ignite(dungeon);
		}
	}

	if (dungeon[(y + m)*cols + (x + n)].enemy) {
		int pos = dungeon.findMonster(x + n, y + m);
		if (pos != -1) {
			if (dungeon.getMonsters().at(pos)->canBeBurned())
				dungeon.giveAffliction(pos, std::make_shared<Burn>(8));
		}
	}

	// Create ember if there is nothing else here
	if (!(dungeon[(y + m)*cols + (x + n)].wall || dungeon[(y + m)*cols + (x + n)].enemy)) {
		std::shared_ptr<Traps> ember = std::make_shared<Ember>(x + n, y + m, 4 + randInt(4));
		dungeon.getTraps().push_back(ember);

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Fire%04d.png", 8);
		ember->setSprite(dungeon.runAnimationForever(frames, 24, x + n, y + m, 2));
		ember->getSprite()->setScale(0.75f * GLOBAL_SPRITE_SCALE);

		dungeon[(y + m)*cols + (x + n)].trap = true;
		dungeon.addLightSource(x + n, y + m, 3);
	}
}

Firecrackers::Firecrackers(int x, int y) : Stackable(x, y, FIRECRACKERS, "Bright_Star_48x48.png", 5) {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("You might be able to delay your demise by just\na little using these. Throwable.");
}
void Firecrackers::useItem(Dungeon &dungeon) {
	playSound("Fireblast_Spell1.mp3");

	int cols = dungeon.getCols();
	bool wall, enemy;
	char move = dungeon.getPlayer()->facingDirection();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int n = 0, m = 0;

	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	// Decrease count
	decreaseCount();

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;

	int range = 10;

	while (!wall && range > 0) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

		if (enemy) {
			int pos = dungeon.findMonster(x + n, y + m);
			if (pos != -1) {

				/*cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);*/

				if (dungeon.getMonsters().at(pos)->canBeStunned()) {
					dungeon.giveAffliction(pos, std::make_shared<Stun>(2));
				}

				if (dungeon.getMonsters().at(pos)->canBeBurned()) {
					if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 90)
						dungeon.giveAffliction(pos, std::make_shared<Burn>(5));
				}

				dungeon.damageMonster(pos, 1);
				return;
			}
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

	if (wall && dungeon[(y + m)*cols + (x + n)].trap) {
		int pos = dungeon.findTrap(x + n, y + m);
		if (pos != -1) {
			if (dungeon.getTraps().at(pos)->canBeIgnited() && 1 + randInt(100) + dungeon.getPlayer()->getLuck() > 90)
				dungeon.getTraps().at(pos)->ignite(dungeon);
		}
	}
}


//		SKELETON KEY
SkeletonKey::SkeletonKey(int x, int y) : Drops(x, y, SKELETON_KEY, "Skeleton_Key_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Key_Pickup.mp3");

	setDescription("A mysterious key. What could this be for?");
}
void SkeletonKey::useItem(Dungeon &dungeon) {
	
}


//		SPECIAL
Teleporter::Teleporter(int x, int y) : Drops(x, y, TELEPORTER, "Teleport_Scroll_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("TurningPage3.mp3");

	setDescription("This is going to be fun.");
}
void Teleporter::useItem(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	bool wall, enemy;
	char move = dungeon.getPlayer()->facingDirection();

	int teleportDistance = 5 + randInt(4);

	int n, m; // n : x, m : y
	switch (move) {
	case 'l': n = -teleportDistance; m = 0; break;
	case 'r': n = teleportDistance; m = 0; break;
	case 'u': n = 0; m = -teleportDistance; break;
	case 'd': n = 0; m = teleportDistance; break;
	}

	// Check boundaries, if not within boundaries, they die instantly
	if (!dungeon.withinBounds(x + n, y + m)) {
		playSound("Fireblast_Spell1.mp3");
		playSound("Explosion.mp3");

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x, y, 2);

		dungeon.getPlayerVector().at(0)->setHP(0);
		dungeon.getPlayer()->getSprite()->setVisible(false);
		return;
	}

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;

	if (!(wall || enemy)) {
		playSound("Teleport_Spell.mp3");

		dungeon.getPlayerVector()[0]->moveTo(dungeon, x + n, y + m, 0.01f);

		if (dungeon[(y + m)*cols + (x + n)].trap)
			dungeon.trapEncounter(x + n, y + m);

		return;
	}

	// Enemies die instantly if they teleport on top of the player
	if (enemy) {
		playSound("Teleport_Spell.mp3");

		int pos = dungeon.findMonster(x + n, y + m);
		if (pos != -1) {
			dungeon.damageMonster(pos, 1000);
			dungeon.getPlayerVector()[0]->moveTo(dungeon, x + n, y + m, 0.01f);

			if (dungeon[(y + m)*cols + (x + n)].trap)
				dungeon.trapEncounter(x + n, y + m);
		}
		return;
	}

	// Look for free space on spaces adjacent to teleport location
	if (wall) {
		x += n;
		y += m;

		std::vector<std::pair<int, int>> coords;
		coords.push_back(std::make_pair(x - 1, y));
		coords.push_back(std::make_pair(x + 1, y));
		coords.push_back(std::make_pair(x, y - 1));
		coords.push_back(std::make_pair(x, y + 1));

		for (int i = 0; i < 4; i++) {

			if (!dungeon.withinBounds(coords[i].first, coords[i].second))
				continue;

			wall = dungeon[coords[i].second*cols + coords[i].first].wall;
			enemy = dungeon[coords[i].second*cols + coords[i].first].enemy;

			if (!(wall || enemy)) {
				playSound("Teleport_Spell.mp3");
				dungeon.getPlayerVector()[0]->moveTo(dungeon, coords[i].first, coords[i].second, 0.01f);

				if (dungeon[coords[i].second*cols + coords[i].first].trap)
					dungeon.trapEncounter(coords[i].first, coords[i].second);

				return;
			}
			
			if (wall)
				continue;

			if (enemy) {
				playSound("Teleport_Spell.mp3");

				int pos = dungeon.findMonster(coords[i].first, coords[i].second);
				if (pos != -1) {
					dungeon.damageMonster(pos, 1000);
					dungeon.getPlayerVector()[0]->moveTo(dungeon, coords[i].first, coords[i].second, 0.01f);

					if (dungeon[coords[i].second*cols + coords[i].first].trap)
						dungeon.trapEncounter(coords[i].first, coords[i].second);
				}
				return;
			}
			
		}

		playSound("Fireblast_Spell1.mp3");
		playSound("Explosion.mp3");

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x, y, 2);

		dungeon.getPlayerVector().at(0)->setHP(0);
		dungeon.getPlayer()->getSprite()->setVisible(false);
		return;
	}
}

Rocks::Rocks(int x, int y) : Drops(x, y, ROCKS, "Ruby_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Bomb_Pickup2.mp3");

	m_durability = 3 + randInt(2);

	setDescription("If it works, it works. Throwable.");
}
void Rocks::useItem(Dungeon &dungeon) {
	playMiss();

	int cols = dungeon.getCols();
	bool wall, enemy;
	char move = dungeon.getPlayer()->facingDirection();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int n = 0, m = 0;

	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;

	int p = n, q = m;
	int range = 7;

	while (!wall && range > 0) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

		if (enemy) {
			int pos = dungeon.findMonster(x + n, y + m);
			if (pos != -1) {

				if (dungeon.getMonsters().at(pos)->canBeStunned())
					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));				

				// Attempt knockback on enemy
				if (!dungeon.getMonsters().at(pos)->isHeavy()) {

					dungeon.linearActorPush(x + n, y + m, 2, move);

					/*int dist = 1;
					while (dist <= 2) {
						switch (move) {
						case 'l': n--; m = 0; break;
						case 'r': n++; m = 0; break;
						case 'u': n = 0; m--; break;
						case 'd': n = 0; m++; break;
						}

						wall = dungeon[(y + m)*cols + (x + n)].wall;
						enemy = dungeon[(y + m)*cols + (x + n)].enemy;

						if (wall)
							break;

						if (!(wall || enemy)) {							
							dungeon.getMonsters().at(pos)->moveTo(dungeon, x + n, y + m);
							break;
						}

						dist++;
					}*/
				}

				dungeon.damageMonster(pos, 1);
				break;
			}
		}

		p = n, q = m;

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

	p += x;
	q += y;

	m_durability--;
	if (m_durability == 0)
		return;

	if (dungeon[q*cols + p].item)
		dungeon.itemHash(p, q);

	dungeon[q*cols + p].item = true;
	dungeon[q*cols + p].object = std::make_shared<Rocks>(*this);
	dungeon.addSprite(dungeon.item_sprites, p, q, -1, dungeon[q*cols + p].object->getImageName());
}



// =============================================
//				:::: PASSIVES ::::
// =============================================
Passive::Passive(int x, int y, std::string name, std::string image) : Drops(x, y, name, image) {

}
void Passive::activate(Dungeon &dungeon) {
	if (forPlayer())
		useItem(*dungeon.getPlayerVector()[0]);
	else
		useItem(dungeon);
}

BatWing::BatWing(int x, int y) : Passive(x, y, BATWING, "Angels_Brace.png") {
	setDescription("A reminder that agility keeps you alive. Boosts dexterity.");
}
void BatWing::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setDex(p.getDex() + 1);
}

LifeElixir::LifeElixir(int x, int y) : Passive(x, y, LIFE_ELIXIR, "Heart_Potion_48x48.png") {
	setDescription("An incredible concoction that invigorates you down to\nyour soul. Increases your maximum health.");
}
void LifeElixir::useItem(Player &p) {
	// sound effect
	playSound("Life_Potion_Used.mp3");

	p.setMaxHP(p.getMaxHP() + 25);
}

MagicEssence::MagicEssence(int x, int y) : Passive(x, y, MAGIC_ESSENCE, "Blue_Flash_48x48.png") {
	setDescription("In older times, the Spellmasters would quaff these to\nincrease their magical prowess in a pinch.");
}
void MagicEssence::useItem(Player &p) {
	// sound effect
	playSound("Life_Potion_Used.mp3");

	p.setInt(p.getInt() + 1);
}

Flying::Flying(int x, int y) : Passive(x, y, FLYING, "Angels_Brace.png") {
	setDescription("Only the most powerful mages could master such a\ntechnique. Years would be spent to get to this level of\nsophistication.");
}
void Flying::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setFlying(true);
}

SteelPunch::SteelPunch(int x, int y) : Passive(x, y, STEEL_PUNCH, "Cheese_Wedge_48x48.png") {
	setDescription("You have acquired the secret of the Steel Punch!");
}
void SteelPunch::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setSteelPunch(true);
}

IronCleats::IronCleats(int x, int y) : Passive(x, y, IRON_CLEATS, "Cheese_Wedge_48x48.png") {
	setDescription("A set of well-crafted iron boots. Prevents spike damage.");
}
void IronCleats::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setSpikeImmunity(true);
}

PoisonTouch::PoisonTouch(int x, int y) : Passive(x, y, POISON_TOUCH, "Cheese_Wedge_48x48.png") {
	setDescription("Chance to poison enemies when attacking.");
}
void PoisonTouch::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setPoisonTouch(true);
}

FireTouch::FireTouch(int x, int y) : Passive(x, y, FIRE_TOUCH, "Cheese_Wedge_48x48.png") {
	setDescription("Chance to ignite enemies when attacking.");
}
void FireTouch::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setFireTouch(true);
}

FrostTouch::FrostTouch(int x, int y) : Passive(x, y, FROST_TOUCH, "Cheese_Wedge_48x48.png") {
	setDescription("Chance to freeze enemies when attacking.");
}
void FrostTouch::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setFrostTouch(true);
}

RainbowTouch::RainbowTouch(int x, int y) : Passive(x, y, RAINBOW_TOUCH, "Cheese_Wedge_48x48.png") {
	setDescription("Chance to poison, ignite, or freeze enemies.");
}
void RainbowTouch::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setPoisonTouch(true);
	p.setFireTouch(true);
	p.setFrostTouch(true);
}

PoisonImmune::PoisonImmune(int x, int y) : Passive(x, y, POISON_IMMUNE, "Cheese_Wedge_48x48.png") {
	setDescription("The antibodies flowing within you are tuned far beyond\n the standard.");
}
void PoisonImmune::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCanBePoisoned(false);
}

FireImmune::FireImmune(int x, int y) : Passive(x, y, VULCAN_RUNE, "Vulcan_Rune_48x48.png") {
	setDescription("Some say this rune was forged by a legendary\n blacksmith that then hid their wares deep\n inside a volcano.");
}
void FireImmune::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCanBeBurned(false);
}

LavaImmune::LavaImmune(int x, int y) : Passive(x, y, VULCAN_RUNE, "Vulcan_Rune_48x48.png") {
	setDescription("Some say this rune was forged by a legendary\n blacksmith that then hid their wares deep\n inside a volcano. Provides lava resistance.");
}
void LavaImmune::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setLavaImmunity(true);
}

BombImmune::BombImmune(int x, int y) : Passive(x, y, BOMB_IMMUNE, "Cheese_Wedge_48x48.png") {
	setDescription("The art of bomb-making become second nature to you.\nImmunity to explosions and increases bomb damage.");
}
void BombImmune::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setExplosionImmune(true);
}

PotionAlchemy::PotionAlchemy(int x, int y) : Passive(x, y, POTION_ALCHEMY, "Cheese_Wedge_48x48.png") {
	setDescription("Your knowledge of potions allows you to make them more potent.");
}
void PotionAlchemy::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setPotentPotions(true);
}

SoulSplit::SoulSplit(int x, int y) : Passive(x, y, SOUL_SPLIT, "Cheese_Wedge_48x48.png") {
	setDescription("Greed has manifested itself into a tangible being.");
}
void SoulSplit::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setSoulSplit(true);
}

LuckUp::LuckUp(int x, int y) : Passive(x, y, LUCKY_PIG, "Lucky_Pig_48x48.png") {
	setDescription("A lucky pig's head. But not so much for the pig.");
}
void LuckUp::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setLuck(p.getLuck() + 15);
}

Berserk::Berserk(int x, int y) : Passive(x, y, BLOODLUST, "Whiskey_48x48.png") {
	setDescription("You should really watch your temper, you're turning red!");
}
void Berserk::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setBloodlust(true);
}

LifeSteal::LifeSteal(int x, int y) : Passive(x, y, BLOODRITE, "Bloody_Apple_32x32.png") {
	setDescription("Suddenly, blood sounds particularly delicious...");
}
void LifeSteal::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setLifesteal(true);
}

Heavy::Heavy(int x, int y) : Passive(x, y, HEAVY, "Cheese_Wedge_48x48.png") {
	setDescription("Your steps have become incredibly solid.");
}
void Heavy::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setHeavy(true);
}

BrickBreaker::BrickBreaker(int x, int y) : Passive(x, y, BRICK_BREAKER, "Cheese_Wedge_48x48.png") {
	setDescription("Claustrophobia is setting in, but smashing down these\nwalls takes a toll on your well-being.");
}
void BrickBreaker::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCanBreakWalls(true);
}

SummonNPCs::SummonNPCs(int x, int y) : Passive(x, y, SUMMON_NPCS, "Cheese_Wedge_48x48.png") {
	setDescription("Your cries do not go unheard. Others have a greater\nchance of finding you.");
}
void SummonNPCs::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCharismaNPC(true);
}

CheapShops::CheapShops(int x, int y) : Passive(x, y, CHEAP_SHOPS, "Cheese_Wedge_48x48.png") {
	setDescription("Your way of words convinces those around you\n to give you a better deal.");
}
void CheapShops::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCheapShops(true);
}

BetterRates::BetterRates(int x, int y) : Passive(x, y, BETTER_RATES, "Cheese_Wedge_48x48.png") {
	setDescription("An overwhelming feeling of relaxation overcomes you.");
}
void BetterRates::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setBetterRates(true);
}

TrapIllumination::TrapIllumination(int x, int y) : Passive(x, y, TRAP_ILLUMINATION, "Cheese_Wedge_48x48.png") {
	setDescription("Suddenly, the dangers of this dungeon are made more\napparent.");
}
void TrapIllumination::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setTrapIllumination(true);
}

ItemIllumination::ItemIllumination(int x, int y) : Passive(x, y, ITEM_ILLUMINATION, "Cheese_Wedge_48x48.png") {
	setDescription("The treasures of this place are revealed.");
}
void ItemIllumination::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setItemIllumination(true);
}

MonsterIllumination::MonsterIllumination(int x, int y) : Passive(x, y, MONSTER_ILLUMINATION, "Cheese_Wedge_48x48.png") {
	setDescription("The monsters of this place are revealed.");
}
void MonsterIllumination::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setMonsterIllumination(true);
}

ResonantSpells::ResonantSpells(int x, int y) : Passive(x, y, RESONANT_SPELLS, "Cheese_Wedge_48x48.png") {
	setDescription("Somehow the magic of this place has begun\nto stick with you. Spells have a chance to\nstay on use.");
}
void ResonantSpells::useItem(Player &p) {
	playSound("Armor_Use.mp3");

	p.setResonantSpells(true);
}



// =============================================
//				:::: SPELLS ::::
// =============================================
Spell::Spell(int x, int y, int damage, std::string item, std::string image) 
	: Drops(x, y, item, image), m_damage(damage) {

}

//		FREEZE SPELL
FreezeSpell::FreezeSpell(int x, int y) : Spell(x, y, 0, FREEZE_SPELL, "Freeze_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("A spellbook that has imagery of ice crystals.");
}
void FreezeSpell::useItem(Dungeon &dungeon) {
	// play freeze sound effect
	playSound("Freeze_Spell1.mp3");
	playSound("Freeze_Spell2.mp3");

	int range = 4 + dungeon.getPlayer()->getInt();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int mx, my;
	// add a freeze(stun) effect to all
	for (unsigned int i = 0; i < dungeon.getMonsters().size(); i++) {
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		if (abs(x - mx) <= range && abs(y - my) <= range) {
			dungeon.giveAffliction(i, std::make_shared<Freeze>(18 + dungeon.getPlayer()->getInt()));
			dungeon.getMonsters()[i]->setFrozen(true);
		}
	}
}

IceShardSpell::IceShardSpell(int x, int y) : Spell(x, y, 0, ICE_SHARD_SPELL, "Freeze_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("Summon a wave of icicles to pierce and freeze enemies.");
}
void IceShardSpell::useItem(Dungeon &dungeon) {
	playSound("Freeze_Spell2.mp3");

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// Create an Ice Shards trap
	int shardLimit = dungeon.getPlayer()->getInt() > 5 ? 4 : 3;
	dungeon.getTraps().push_back(std::make_shared<IceShards>(x, y, shardLimit));
	dungeon.getTraps().back()->setSprite(dungeon.createSprite(x, y, -1, dungeon.getTraps().back()->getImageName()));
}

HailStormSpell::HailStormSpell(int x, int y) : Spell(x, y, 0, HAIL_STORM_SPELL, "Freeze_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("Rain down frozen death.");
}
void HailStormSpell::useItem(Dungeon &dungeon) {
	playSound("Freeze_Spell2.mp3");

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// Create a HailStorm trap
	int shardLimit = dungeon.getPlayer()->getInt() > 5 ? 4 : 3;
	dungeon.getTraps().push_back(std::make_shared<HailStorm>(x, y, dungeon.getPlayer()->facingDirection(), shardLimit));
	//dungeon.getTraps().back()->setSprite(dungeon.createSprite(x, y, -1, dungeon.getTraps().back()->getImageName()));
}

//		EARTHQUAKE SPELL
EarthquakeSpell::EarthquakeSpell(int x, int y) : Spell(x, y, 6, EARTHQUAKE_SPELL, "Earthquake_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("A spellbook that cracks the earth.");
}
void EarthquakeSpell::useItem(Dungeon &dungeon) {
	// play earthquake sound
	playSound("Earthquake_Spell1.mp3");
	playSound("Earthquake_Spell2.mp3");
	playSound("Earthquake_Explosion1.mp3");
	playSound("Earthquake_Explosion2.mp3");

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();
	int mx, my;

	// find any monsters caught in the quake
	for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		
		if (abs(mx - px) <= 2 && abs(my - py) <= 2) {
			dungeon.damageMonster(i, getDamage() + dungeon.getPlayer()->getInt());

			// Stun monster as well
			if (dungeon.getMonsters().at(i)->canBeStunned())
				dungeon.giveAffliction(i, std::make_shared<Stun>(4 + dungeon.getPlayer()->getInt()));		
		}
		
	}

	//	destroy any nearby walls
	for (int j = py - 2; j < py + 3; j++) {
		for (int k = px - 2; k < px + 3; k++) {
			if (j != -1 && j != rows && !(k == -1 && j <= 0) && !(k == cols && j >= rows - 1)) { // boundary check

				if (dungeon[j*cols + k].wall_type == REG_WALL) {
					dungeon[j*cols + k].wall = false;

					// call remove sprite
					dungeon.removeSprite(dungeon.wall_sprites, k, j);						
				}

				/*
				// destroy any gold in the way
				if (dungeon[j*cols + k].gold != 0) {
					dungeon[j*cols + k].gold = 0;

					dungeon.removeSprite(dungeon.money_sprites, rows, k, j);
				}
				*/

				// Don't destroy the exit!
				if (dungeon[j*cols + k].exit)
					continue;
						
				// Destroy any traps
				if (dungeon[j*cols + k].trap) {

					int pos = dungeon.findTrap(k, j); // traps in here

					if (pos == -1)
						continue;

					// if the trap is destructible, destroy it
					if (dungeon.getTraps()[pos]->isDestructible())
						dungeon.getTraps()[pos]->destroyTrap(dungeon);

					// if the trap is not destructible, but the player has enough intellect, destroy it too
					else if (!dungeon.getTraps()[pos]->isDestructible() && dungeon.getPlayer()->getInt() >= 5)
						dungeon.getTraps()[pos]->destroyTrap(dungeon);

					if (dungeon.getTraps().at(pos)->isExplosive()) {
						std::shared_ptr<ActiveBomb> bomb = std::dynamic_pointer_cast<ActiveBomb>(dungeon.getTraps().at(pos));
						bomb->explosion(dungeon, *dungeon.getPlayerVector()[0]);
						bomb.reset();
					}				
				}

			}
		}
	}
	
}

RockSummonSpell::RockSummonSpell(int x, int y) : Spell(x, y, 6, ROCK_SUMMON_SPELL, "Earthquake_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("Summon a formation of magical stones that you can\nsend barreling into enemies.");
}
void RockSummonSpell::useItem(Dungeon &dungeon) {
	playSound("Earthquake_Spell1.mp3");

	int cols = dungeon.getCols();

	// Get the rocks to spawn; More intellect = More rocks
	setCoords(dungeon);

	int x, y;
	for (unsigned int i = 0; i < coords.size(); i++) {
		x = coords[i].first;
		y = coords[i].second;

		if (dungeon[y*cols + x].enemy) {
			int pos = dungeon.findMonster(x, y);
			if (pos != -1) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x, y, 2);

				dungeon.damageMonster(pos, 1000);
			}
		}
		else if (dungeon[y*cols + x].wall && dungeon[y*cols + x].wall_type == REG_WALL) {
			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x, y, 2);

			dungeon[y*cols + x].wall = false;
			dungeon.removeSprite(dungeon.wall_sprites, x, y);
		}
		else if (!dungeon[y*cols + x].wall) {
			// Makes the rock interactable
			dungeon[y*cols + x].wall = true;
			dungeon[y*cols + x].trap = true;

			// Create RockSummon trap
			dungeon.getTraps().push_back(std::make_shared<RockSummon>(x, y));
			dungeon.getTraps().back()->setSprite(dungeon.createSprite(x, y, -1, dungeon.getTraps().back()->getImageName()));
		}
	}
}
void RockSummonSpell::setCoords(Dungeon &dungeon) {
	coords.clear(); // Resonant Spells can cause this to be called more than once

	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int intellect = dungeon.getPlayer()->getInt();

	int n = 0, m = 0;
	switch (dungeon.getPlayer()->facingDirection()) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	// Summon 3 in front of player
	if (intellect <= 5) {
		// Player attacked to the side, so check above and below
		if (m == 0) {
			coords.push_back(std::make_pair(x + n, y + m));
			coords.push_back(std::make_pair(x + n, y + 1));
			coords.push_back(std::make_pair(x + n, y - 1));
		}
		// Else player attacked above or below, so check the sides
		else {
			coords.push_back(std::make_pair(x + n, y + m));
			coords.push_back(std::make_pair(x + 1, y + m));
			coords.push_back(std::make_pair(x - 1, y + m));
		}
	}
	if (intellect > 5 && intellect <= 10) {
		for (int i = x - 1; i < x + 2; i++) {
			for (int j = y - 1; j < y + 2; j++) {

				if (i == x && j == y)
					continue;

				// Attacked up or down
				if (n == 0) {

					// Attacked below
					if (m == 1) {
						if (j < y)
							continue;
					}
					// Attacked above
					else {
						if (j > y)
							continue;
					}
				}

				// Attacked to the side
				else if (m == 0) {
					// Attacked left
					if (n == -1) {
						if (i > x)
							continue;
					}
					// Attacked right
					else {
						if (i < x)
							continue;
					}
				}

				coords.push_back(std::make_pair(i, j));
			}
		}
	}
	else if (intellect > 10) {
		for (int i = x - 1; i < x + 2; i++) {
			for (int j = y - 1; j < y + 2; j++) {

				if (i == x && j == y)
					continue;

				coords.push_back(std::make_pair(i, j));
			}
		}
	}
}

ShockwaveSpell::ShockwaveSpell(int x, int y) : Spell(x, y, 6, SHOCKWAVE_SPELL, "Earthquake_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("The ground reverberates with energy upon passing\nglance of this relic.");
}
void ShockwaveSpell::useItem(Dungeon &dungeon) {
	playSound("Earthquake_Spell1.mp3");
	playSound("Earthquake_Spell2.mp3");

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();
	char move = dungeon.getPlayer()->facingDirection();

	int n, m;
	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	// fire explosion animation
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
	dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

	if (dungeon[(y + m)*cols + (x + n)].enemy) {
		int pos = dungeon.findMonster(x + n, y + m);
		if (pos != -1) {
			if (dungeon.getMonsters().at(pos)->canBeStunned())
				dungeon.giveAffliction(pos, std::make_shared<Stun>(2));

			dungeon.damageMonster(pos, getDamage());
		}
	}

	// Create Shockwave trap
	int waveLimit = dungeon.getPlayer()->getInt() > 5 ? 7 : 5;
	dungeon.getTraps().push_back(std::make_shared<Shockwaves>(x, y, move, waveLimit));
	//dungeon.getTraps().back()->setSprite(dungeon.createSprite(x, y, -1, dungeon.getTraps().back()->getImageName()));
}

//		FIRE BLAST SPELL
FireBlastSpell::FireBlastSpell(int x, int y) : Spell(x, y, 5, FIREBLAST_SPELL, "Fireblast_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("A fiery spell that launches a projectile in the\n direction you're facing.");
}
void FireBlastSpell::useItem(Dungeon &dungeon) {
	// play fire blast sound effect
	playSound("Fireblast_Spell1.mp3");

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	
	// figure out how which direction to iterate
	int n, m; // n : x, m : y
	char playerFacing = dungeon.getPlayer()->facingDirection();
	switch (playerFacing) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	// while there's no wall or enemy, let the fireball keep traveling
	while (!(dungeon[(py + m)*cols + px + n].wall || dungeon[(py + m)*cols + px + n].enemy)) {

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, px + n, py + m, 2);

		switch (playerFacing) {
		case 'l': n--; m = 0; break;
		case 'r': n++; m = 0; break;
		case 'u': n = 0; m--; break;
		case 'd': n = 0; m++; break;
		}
	}

	// play fire blast explosion sound effect
	playSound("Fireblast_Spell2.mp3");

	//// fire explosion animation
	//cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
	//dungeon.runSingleAnimation(frames, 120, px + n, py + m, 2);

	//// find any monsters caught in the blast
	//int mx, my;
	//for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
	//	mx = dungeon.getMonsters().at(i)->getPosX();
	//	my = dungeon.getMonsters().at(i)->getPosY();

	//	if (abs(mx - (px + n)) <= 1 && abs(my - (py + m)) <= 1) {
	//		dungeon.damageMonster(i, getDamage() + dungeon.getPlayer()->getInt());

	//		//  check if monster died
	//		if (dungeon.getMonsters().at(i)->getHP() <= 0) {
	//			dungeon.monsterDeath(i);
	//		}
	//		// otherwise burn the monster if it can be burned
	//		else if (dungeon.getMonsters().at(i)->getHP() > 0 && dungeon.getMonsters().at(i)->canBeBurned()) {
	//			dungeon.getMonsters().at(i)->addAffliction(std::make_shared<Burn>(5 + (dungeon.getPlayer()->getInt() / 2)));
	//		}
	//	}
	//}

	px += n;
	py += m;

	for (int i = px - 1; i < px + 2; i++) {
		for (int j = py - 1; j < py + 2; j++) {

			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, i, j, 2);

			if (dungeon[j*cols + i].enemy) {

				int pos = dungeon.findMonster(i, j);
				if (pos != -1) {
					dungeon.damageMonster(pos, getDamage() + dungeon.getPlayer()->getInt());

					// If they can be burned, roll for a high chance to burn
					if (dungeon.getMonsters().at(pos)->canBeBurned()) {
						dungeon.giveAffliction(pos, std::make_shared<Burn>(5 + dungeon.getPlayer()->getInt()));					
					}
				}
			}

			if (dungeon[j*cols + i].trap) {

				int pos = dungeon.findTrap(i, j);
				if (pos != -1) {
					if (dungeon.getTraps().at(pos)->canBeIgnited())
						dungeon.getTraps().at(pos)->ignite(dungeon);
				}
			}
		}
	}
}

FireCascadeSpell::FireCascadeSpell(int x, int y) : Spell(x, y, 5, FIRE_CASCADE_SPELL, "Fireblast_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("Releases pillars of fire that cascade outward.");
}
void FireCascadeSpell::useItem(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int pos;

	playSound("Fireblast_Spell1.mp3");

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (i == x && j == y)
				continue;

			if (!dungeon[j*cols + i].wall) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, i, j, 2);
			}

			if (dungeon[j*cols + i].enemy) {

				pos = dungeon.findMonster(i, j);
				if (pos != -1) {

					// If they can be burned, burn them
					if (dungeon.getMonsters().at(pos)->canBeBurned()) {
						dungeon.giveAffliction(pos, std::make_shared<Burn>(10 + dungeon.getPlayer()->getInt()));
					}
				}
			}

			if (dungeon[j*cols + i].trap) {

				pos = dungeon.findTrap(i, j);
				if (pos != -1) {
					if (dungeon.getTraps().at(pos)->canBeIgnited())
						dungeon.getTraps().at(pos)->ignite(dungeon);
				}
			}
		}
	}

	// Create fire pillar trap
	int pillarLimit = dungeon.getPlayer()->getInt() > 5 ? 2 : 1;
	dungeon.getTraps().push_back(std::make_shared<FirePillars>(x, y, pillarLimit));
}

FireExplosionSpell::FireExplosionSpell(int x, int y) : Spell(x, y, 5, FIRE_EXPLOSION_SPELL, "Fireblast_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("Unleash your wrath. High chance to burn.");
}
void FireExplosionSpell::useItem(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int pos;

	playSound("Fireblast_Spell1.mp3");
	playSound("Explosion.mp3");

	// Damage any monsters in the blast radius
	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {

			// Boundary and corner check
			if (!dungeon.withinBounds(i, j) || (i == x + 2 && j == y - 2) || (i == x + 2 && j == y + 2) || (i == x - 2 && j == y - 2) || (i == x - 2 && j == y + 2))
				continue;

			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, i, j, 2);
			
			if (dungeon[j*cols + i].enemy) {

				pos = dungeon.findMonster(i, j);
				if (pos != -1) {
					dungeon.damageMonster(pos, getDamage() + dungeon.getPlayer()->getInt());

					// If they can be burned, roll for a high chance to burn
					if (dungeon.getMonsters().at(pos)->canBeBurned()) {
						if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 50 - dungeon.getPlayer()->getInt()) {
							dungeon.giveAffliction(pos, std::make_shared<Burn>(5 + dungeon.getPlayer()->getInt()));
						}
					}
				}
			}

			if (dungeon[j*cols + i].trap) {

				pos = dungeon.findTrap(i, j);
				if (pos != -1) {
					if (dungeon.getTraps().at(pos)->canBeIgnited())
						dungeon.getTraps().at(pos)->ignite(dungeon);
				}
			}
		}
	}

	// Chance to burn the player as well
	if (dungeon.getPlayer()->canBeBurned()) {

		// Add their Intellect stat because it should be less likely to get burned with the more Int they have
		if (1 + randInt(100) - dungeon.getPlayer()->getLuck() > 60 + dungeon.getPlayer()->getInt()) {
			dungeon.getPlayerVector().at(0)->addAffliction(std::make_shared<Burn>(5));
		}
	}
}

//		WIND SPELL
WindSpell::WindSpell(int x, int y) : Spell(x, y, 0, WHIRLWIND_SPELL, "Wind_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("A light breeze runs by when you touch the cover.");
}
void WindSpell::useItem(Dungeon &dungeon) {
	// play wind sound
	playSound("Wind_Spell1.mp3");
	playSound("Wind_Spell2.mp3");

	// if player is on fire, roll for a chance to remove it
	if (dungeon.getPlayer()->isBurned()) {
		int roll = 1 + randInt(100) + 2*dungeon.getPlayer()->getInt();

		if (roll > 50) {
			dungeon.getPlayerVector()[0]->removeAffliction(BURN);
		}
	}

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();
	int mx, my, tempx, tempy;

	// find any monsters caught in the whirlwind
	char move;
	int tries = 5; // for breaking out of the loop if too many tries
	bool dead = false; // for stunning only monsters that are alive
	int radius = 3;
	unsigned int oldSize = dungeon.getMonsters().size();

	for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
		dead = false;
		tries = 5 + (dungeon.getPlayer()->getInt() >= 10 ? 3 : dungeon.getPlayer()->getInt() >= 5 ? 1 : 0);
		radius = 3 + (dungeon.getPlayer()->getInt() >= 10 ? 2 : dungeon.getPlayer()->getInt() >= 5 ? 1 : 0);
		oldSize = dungeon.getMonsters().size();
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		if (abs(mx - px) > radius && abs(my - py) > radius)
			continue;

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
			dungeon.pushMonster(mx, my, move);

			// check if monster died
			if (dungeon.getMonsters().size() < oldSize) {
				dead = true;
				break;
			}

			// check if there was a trap at this position
			if (dungeon[dungeon.getMonsters().at(i)->getPosY()*cols + dungeon.getMonsters().at(i)->getPosX()].trap) {
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
				int turns = (dungeon.getPlayer()->getInt() >= 5 ? 3 : 1);
				dungeon.giveAffliction(i, std::make_shared<Stun>(turns));
			}
		}
		else {
			i--; // decrement to not skip monsters since monster died
		}
	}
}

WindBlastSpell::WindBlastSpell(int x, int y) : Spell(x, y, 0, WIND_BLAST_SPELL, "Wind_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("Call forth a powerful gust of wind to knock\nenemies backward.");
}
void WindBlastSpell::useItem(Dungeon &dungeon) {
	playSound("Wind_Spell2.mp3");

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	bool wall, enemy;
	char move = dungeon.getPlayer()->facingDirection();

	int n, m; // n : x, m : y
	switch (move) {
	case 'l': n = -1; m = -1; break;
	case 'r': n = 1; m = -1; break;
	case 'u': n = -1; m = -1; break;
	case 'd': n = -1; m = 1; break;
	}

	int range = 6;
	int currentRange = 1;

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;

	int k = 1; // 3 rows/columns to check
	while (k <= 3) {

		while (!wall && currentRange <= range) {

			cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

			if (enemy) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1)
					dungeon.giveAffliction(pos, std::make_shared<Stun>(2 + dungeon.getPlayer()->getInt()));

				dungeon.linearActorPush(x + n, y + m, range - currentRange + 1, move);
				break;		
			}

			switch (move) {
			case 'l': n--; break;
			case 'r': n++; break;
			case 'u': m--; break;
			case 'd': m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			currentRange++;
		}

		switch (move) {
		case 'l': n = -1; m++; break;
		case 'r': n = 1; m++; break;
		case 'u': n++; m = -1; break;
		case 'd': n++; m = 1; break;
		}

		k++;
		currentRange = 1;
		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;
	}
}

WindVortexSpell::WindVortexSpell(int x, int y) : Spell(x, y, 0, WIND_VORTEX_SPELL, "Wind_Spell_48x48.png") {
	setItemFlag(true);
	setForPlayer(false);
	setSoundName("Book_Pickup.mp3");

	setDescription("Let loose the eye of the storm.");
}
void WindVortexSpell::useItem(Dungeon &dungeon) {
	playSound("Wind_Spell2.mp3");

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// Create WindVortex trap
	int turnLimit = 10 + dungeon.getPlayer()->getInt();
	dungeon.getTraps().push_back(std::make_shared<WindVortex>(x, y, turnLimit));
	dungeon.getTraps().back()->setSprite(dungeon.createSprite(x, y, -1, dungeon.getTraps().back()->getImageName()));
}

//		INVISIBILITY SPELL
InvisibilitySpell::InvisibilitySpell(int x, int y) : Spell(x, y, 0, INVISIBILITY_SPELL, "Invisibility_Spell_48x48.png") {
	setItemFlag(true);
	setSoundName("Book_Pickup.mp3");

	setDescription("Now you see me. Now you");
}
void InvisibilitySpell::useItem(Player &p) {
	// play invisibility sound effect
	playSound("Invisibility_Spell.mp3");

	p.addAffliction(std::make_shared<Invisibility>(25 + p.getInt()));
}

//		ETHEREAL SPELL
EtherealSpell::EtherealSpell(int x, int y) : Spell(x, y, 0, ETHEREAL_SPELL, "Ethereal_Spell_48x48.png") {
	setItemFlag(true);
	setSoundName("Book_Pickup.mp3");

	setDescription("Now you see me. Now you.. still do?");
}
void EtherealSpell::useItem(Player &p) {
	// play ethereality sound effect
	playSound("Ethereal_Spell.mp3");

	p.addAffliction(std::make_shared<Ethereality>(15 + 2*p.getInt()));
}



//					:::: TRINKETS ::::
Trinket::Trinket(int x, int y, std::string name, std::string image) : Objects(x, y, name, image) {
	
}


CursedStrength::CursedStrength(int x, int y) : Trinket(x, y, CURSED_STRENGTH, "Blue_Toy_32x32.png") {
	setDescription("Have the gods favored you, or was it a farce?\nGrants immense strength--at a cost.");
}
void CursedStrength::apply(Dungeon &dungeon, Player &p) {
	p.setSlow(true);
	p.setStr(p.getStr() + 8);
}
void CursedStrength::unapply(Dungeon &dungeon, Player &p) {
	p.setSlow(false);
	p.setStr(p.getStr() - 8);
}

BrightStar::BrightStar(int x, int y) : Trinket(x, y, BRIGHT_STAR, "Fireflies_Jar_48x48.png") {
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

DarkStar::DarkStar(int x, int y) : Trinket(x, y, DARK_STAR, "Lightbulb_32x32.png") {
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

GoldPot::GoldPot(int x, int y) : Trinket(x, y, GOLD_POT, "Golden_Ring_32x32.png") {
	setDescription("A strong sense of greed fills your mind with this ring\n in your possession.");
}
void GoldPot::apply(Dungeon &dungeon, Player &p) {
	p.setMoneyBonus(p.getMoneyBonus() + 2);
}
void GoldPot::unapply(Dungeon &dungeon, Player &p) {
	p.setMoneyBonus(p.getMoneyBonus() - 2);
}

// Deprecated
LuckyPig::LuckyPig(int x, int y) : Trinket(x, y, LUCKY_PIG, "Lucky_Pig_48x48.png") {
	setDescription("A lucky pig's head. But not so much for the pig.");
}
void LuckyPig::apply(Dungeon &dungeon, Player &p) {
	p.setLuck(p.getLuck() + 15);
}
void LuckyPig::unapply(Dungeon &dungeon, Player &p) {
	p.setLuck(p.getLuck() - 15);
}

RingOfCasting::RingOfCasting(int x, int y) : Trinket(x, y, RING_OF_CASTING, "Wizards_Hat_48x48.png") {
	setDescription("This hat makes you feel more capable when handling spells.");
}
void RingOfCasting::apply(Dungeon &dungeon, Player &p) {
	p.setInt(p.getInt() + 5);
}
void RingOfCasting::unapply(Dungeon &dungeon, Player &p) {
	p.setInt(p.getInt() - 5);
}

VulcanRune::VulcanRune(int x, int y) : Trinket(x, y, VULCAN_RUNE, "Vulcan_Rune_48x48.png") {
	setDescription("Some say this rune was forged by a legendary\n blacksmith that then hid their wares deep\n inside a volcano.");
}
void VulcanRune::apply(Dungeon &dungeon, Player &p) {
	//p.setLavaImmunity(true);
	p.setCanBeBurned(false);
}
void VulcanRune::unapply(Dungeon &dungeon, Player &p) {
	//p.setLavaImmunity(false);
	p.setCanBeBurned(true);
}

Bloodrite::Bloodrite(int x, int y) : Trinket(x, y, BLOODRITE, "Bloody_Apple_32x32.png") {
	setDescription("Suddenly, blood sounds particularly delicious...");
}
void Bloodrite::apply(Dungeon &dungeon, Player &p) {
	p.setLifesteal(true);
}
void Bloodrite::unapply(Dungeon &dungeon, Player &p) {
	p.setLifesteal(false);
}

Bloodlust::Bloodlust(int x, int y) : Trinket(x, y, BLOODLUST, "Whiskey_48x48.png") {
	setDescription("You should really watch your temper, you're turning red!");
}
void Bloodlust::apply(Dungeon &dungeon, Player &p) {
	p.setBloodlust(true);
}
void Bloodlust::unapply(Dungeon &dungeon, Player &p) {
	p.setBloodlust(false);

	// resets redness
	p.getSprite()->setColor(cocos2d::Color3B(255, 255, 255));
}

BatFang::BatFang(int x, int y) : Trinket(x, y, BATFANG, "Whiskey_48x48.png") {
	setDescription("The fang of a wild Wanderer. It's sharp!");
}
void BatFang::apply(Dungeon &dungeon, Player &p) {
	p.setToxic(true);
}
void BatFang::unapply(Dungeon &dungeon, Player &p) {
	p.setToxic(false);
}



//		CHESTS
Chests::Chests(int x, int y, std::string chest, std::string image) : Drops(x, y, chest, image) {
	setForPlayer(false);
	setSoundName("ChestOpening1.mp3");
}

LifeChest::LifeChest(int x, int y) : Chests(x, y, LIFE_CHEST, "Pink_Chest_48x48.png") {

}
void LifeChest::open(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	playSound(getSoundName());

	dungeon.removeSprite(dungeon.item_sprites, x, y);

	switch (1 + randInt(7)) {
	case 1: dungeon[y*cols + x].object = std::make_shared<LifePotion>(); break;
	case 2:	dungeon[y*cols + x].object = std::make_shared<BigLifePotion>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<SteadyLifePotion>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<HalfLifePotion>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<BinaryLifePotion>(); break;
	case 6: dungeon[y*cols + x].object = std::make_shared<RottenApple>(); break;
	case 7: dungeon[y*cols + x].object = std::make_shared<SoulPotion>(); break;
	default: break;
	}

	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
	dungeon[y*cols + x].wall = false;

	int z = dungeon.isShop() ? 2 : -1;
	dungeon.addSprite(dungeon.item_sprites, x, y, z, dungeon[y*cols + x].object->getImageName());
}

BrownChest::BrownChest(int x, int y) : Chests(x, y, BROWN_CHEST, "Brown_Chest_48x48.png") {

}
void BrownChest::open(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	playSound(getSoundName());

	dungeon.removeSprite(dungeon.item_sprites, x, y);

	switch (1 + randInt(11)) {
	case 1: dungeon[y*cols + x].object = std::make_shared<LifePotion>(); break;
	case 2:	dungeon[y*cols + x].object = std::make_shared<DizzyElixir>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<Bomb>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<ShieldRepair>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<PoisonCloud>(); break;
	case 6: dungeon[y*cols + x].object = std::make_shared<SmokeBomb>(); break;
	case 7: dungeon[y*cols + x].object = std::make_shared<StatPotion>(); break;
	case 8: dungeon[y*cols + x].object = std::make_shared<RottenMeat>(); break;
	case 9: dungeon[y*cols + x].object = std::make_shared<Matches>(); break;
	case 10: dungeon[y*cols + x].object = std::make_shared<Firecrackers>(); break;
	case 11: dungeon[y*cols + x].object = std::make_shared<BearTrap>(); break;
	default: break;
	}

	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
	dungeon[y*cols + x].wall = false;
	
	int z = dungeon.isShop() ? 2 : -1;
	dungeon.addSprite(dungeon.item_sprites, x, y, z, dungeon[y*cols + x].object->getImageName());
}

SilverChest::SilverChest(int x, int y) : Chests(x, y, SILVER_CHEST, "Silver_Chest_48x48.png") {

}
void SilverChest::open(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	playSound(getSoundName());

	dungeon.removeSprite(dungeon.item_sprites, x, y);

	switch (1 + randInt(14)) {
	case 1: dungeon[y*cols + x].object = std::make_shared<InvisibilitySpell>(); break;
	case 2:	dungeon[y*cols + x].object = std::make_shared<FireBlastSpell>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<PoisonTouch>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<FireTouch>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<Antidote>(); break;
	case 6:	dungeon[y*cols + x].object = std::make_shared<BearTrap>(); break;
	case 7:	dungeon[y*cols + x].object = std::make_shared<BigLifePotion>(); break;
	case 8:	dungeon[y*cols + x].object = std::make_shared<IceShardSpell>(); break;
	case 9: dungeon[y*cols + x].object = std::make_shared<EtherealSpell>(); break;
	case 10: dungeon[y*cols + x].object = std::make_shared<FreezeSpell>(); break;
	case 11: dungeon[y*cols + x].object = std::make_shared<HailStormSpell>(); break;
	case 12: dungeon[y*cols + x].object = std::make_shared<WindBlastSpell>(); break;
	case 13: dungeon[y*cols + x].object = std::make_shared<WindVortexSpell>(); break;
	case 14: dungeon[y*cols + x].object = std::make_shared<FrostTouch>(); break;
	default: break;
	}
	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
	dungeon[y*cols + x].wall = false;

	int z = dungeon.isShop() ? 2 : -1;
	dungeon.addSprite(dungeon.item_sprites, x, y, z, dungeon[y*cols + x].object->getImageName());
}

GoldenChest::GoldenChest(int x, int y) : Chests(x, y, GOLDEN_CHEST, "Golden_Chest_48x48.png") {

}
void GoldenChest::open(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	playSound(getSoundName());

	dungeon.removeSprite(dungeon.item_sprites, x, y);

	int n = 1 + randInt(17);
	switch (n) {
	case 1: dungeon[y*cols + x].object = std::make_shared<SteelPunch>(); break;
	case 2: dungeon[y*cols + x].object = std::make_shared<IronCleats>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<Claw>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<SuperiorHammer>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<ResonantSpells>(); break;
	case 6: dungeon[y*cols + x].object = std::make_shared<RainbowTouch>(); break;
	case 7: dungeon[y*cols + x].object = std::make_shared<LifeSteal>(); break;
	case 8: dungeon[y*cols + x].object = std::make_shared<PotionAlchemy>(); break;
	case 9: dungeon[y*cols + x].object = std::make_shared<BrickBreaker>(); break;
	case 10: dungeon[y*cols + x].object = std::make_shared<Pistol>(); break;
	case 11: dungeon[y*cols + x].object = std::make_shared<LavaImmune>(); break;
	case 12: dungeon[y*cols + x].object = std::make_shared<SuperiorEstoc>(); break;
	case 13: dungeon[y*cols + x].object = std::make_shared<SuperiorNunchuks>(); break;
	case 14: dungeon[y*cols + x].object = std::make_shared<SuperiorJian>(); break;
	case 15: dungeon[y*cols + x].object = std::make_shared<SuperiorBoStaff>(); break;
	case 16: dungeon[y*cols + x].object = std::make_shared<SuperiorKatana>(); break;
	case 17: dungeon[y*cols + x].object = std::make_shared<SuperiorZweihander>(); break;
	case 18: dungeon[y*cols + x].object = std::make_shared<VulcanBow>(); break;
	}

	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
	dungeon[y*cols + x].wall = false;
	
	int z = dungeon.isShop() ? 2 : -1;
	dungeon.addSprite(dungeon.item_sprites, x, y, z, dungeon[y*cols + x].object->getImageName());
}

HauntedChest::HauntedChest(int x, int y) : Chests(x, y, HAUNTED_CHEST, "Golden_Chest_48x48.png") {

}
void HauntedChest::open(Dungeon &dungeon) {
	// Summon ghosts
	playSound("Devils_Gift.mp3");

	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	std::vector<std::pair<int, int>> coords;
	for (int i = x - 3; i < x + 4; i++) {
		for (int j = y - 3; j < y + 4; j++) {

			// Ignore all but the outer ring
			if (!dungeon.withinBounds(i, j) || (abs(i - x) <= 2 || abs(j - y) <= 2))
				continue;

			coords.push_back(std::make_pair(i, j));
		}
	}

	bool wall, enemy;
	int n = 3; // Number of ghosts to spawn
	while (!coords.empty() && n > 0) {
		int index = randInt((int)coords.size());
		std::pair<int, int> pair = coords[index];
		coords.erase(coords.begin() + index);

		wall = dungeon[pair.second*cols + pair.first].wall;
		enemy = dungeon[pair.second*cols + pair.first].enemy;

		if (!(wall || enemy)) {
			std::shared_ptr<Monster> m = std::make_shared<Ghost>(pair.first, pair.second);
			m->setSprite(dungeon.createSprite(pair.first, pair.second, 1, m->getImageName()));
			dungeon.getMonsters().push_back(m);
			dungeon[pair.second*cols + pair.first].enemy = true;

			n--;
		}
	}

	playSound(getSoundName());

	dungeon.removeSprite(dungeon.item_sprites, x, y);

	n = 1 + randInt(9);
	switch (n) {
	case 1: dungeon[y*cols + x].object = std::make_shared<SteelPunch>(); break;
	case 2: dungeon[y*cols + x].object = std::make_shared<GreaterNunchuks>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<FrostShield>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<GreaterHammer>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<GreaterBoStaff>(); break;
	case 6: dungeon[y*cols + x].object = std::make_shared<RainbowTouch>(); break;
	case 7: dungeon[y*cols + x].object = std::make_shared<LifeSteal>(); break;
	case 8: dungeon[y*cols + x].object = std::make_shared<Mattock>(); break;
	case 9: dungeon[y*cols + x].object = std::make_shared<GreaterKatana>(); break;
	}

	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
	dungeon[y*cols + x].wall = false;
	
	int z = dungeon.isShop() ? 2 : -1;
	dungeon.addSprite(dungeon.item_sprites, x, y, z, dungeon[y*cols + x].object->getImageName());
}

TeleportingChest::TeleportingChest(int x, int y) : Chests(x, y, TELEPORTING_CHEST, "Golden_Chest_48x48.png") {
	m_teleports = 1 + randInt(2);
}
void TeleportingChest::open(Dungeon &dungeon) {

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// Teleport if there are still teleports remaining
	if (m_teleports > 0) {
		playSound("Teleport_Spell.mp3");

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x, y, 2);

		dungeon[y*cols + x].item = false;
		dungeon[y*cols + x].wall = false;

		int _x = 1 + randInt(cols - 1);
		int _y = 1 + randInt(rows - 1);

		while (dungeon[_y*cols + _x].enemy || dungeon[_y*cols + _x].wall || dungeon[_y*cols + _x].trap || dungeon[_y*cols + _x].item || dungeon[_y*cols + _x].hero) {
			_x = 1 + randInt(cols - 1);
			_y = 1 + randInt(rows - 1);
		}

		dungeon[_y*cols + _x].item = true;
		dungeon[_y*cols + _x].wall = true;
		dungeon[_y*cols + _x].object = dungeon[y*cols + x].object; // Assign chest to new position

		dungeon[y*cols + x].object.reset();
		dungeon[y*cols + x].object = nullptr;
		dungeon.moveSprite(dungeon.item_sprites, x, y, _x, _y);

		setPosX(_x); setPosY(_y);

		m_teleports--;
		return;
	}

	playSound(getSoundName());

	dungeon.removeSprite(dungeon.item_sprites, x, y);

	int n = 1 + randInt(9);
	switch (n) {
	case 1: dungeon[y*cols + x].object = std::make_shared<SteelPunch>(); break;
	case 2: dungeon[y*cols + x].object = std::make_shared<IronCleats>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<FrostShield>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<VulcanHammer>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<ResonantSpells>(); break;
	case 6: dungeon[y*cols + x].object = std::make_shared<RainbowTouch>(); break;
	case 7: dungeon[y*cols + x].object = std::make_shared<LifeSteal>(); break;
	case 8: dungeon[y*cols + x].object = std::make_shared<PotionAlchemy>(); break;
	case 9: dungeon[y*cols + x].object = std::make_shared<BrickBreaker>(); break;
	}

	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
	dungeon[y*cols + x].wall = false;

	int z = dungeon.isShop() ? 2 : -1;
	dungeon.addSprite(dungeon.item_sprites, x, y, z, dungeon[y*cols + x].object->getImageName());
}

RiggedChest::RiggedChest(int x, int y, std::string name, std::string image, int turns) : Chests(x, y, name, image), m_turns(turns) {

}

ExplodingChest::ExplodingChest(int x, int y) : RiggedChest(x, y, EXPLODING_CHEST, "Golden_Chest_48x48.png", 1) {
	
}
void ExplodingChest::open(Dungeon &dungeon) {
	
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	playSound(getSoundName());

	dungeon.removeSprite(dungeon.item_sprites, x, y);

	std::shared_ptr<Traps> trap = std::make_shared<ActiveBomb>(x, y, 1);
	dungeon.getTraps().push_back(trap);
	trap->setSprite(dungeon.createSprite(x, y, 0, trap->getImageName()));

	int n = 1 + randInt(10);
	switch (n) {
	case 1: dungeon[y*cols + x].object = std::make_shared<TrapIllumination>(); break;
	case 2: dungeon[y*cols + x].object = std::make_shared<ItemIllumination>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<Berserk>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<GreaterJian>(); break;
	case 5: dungeon[y*cols + x].object = std::make_shared<WindVortexSpell>(); break;
	case 6: dungeon[y*cols + x].object = std::make_shared<RainbowTouch>(); break;
	case 7: dungeon[y*cols + x].object = std::make_shared<LifeSteal>(); break;
	case 8: dungeon[y*cols + x].object = std::make_shared<MagicEssence>(); break;
	case 9: dungeon[y*cols + x].object = std::make_shared<BetterRates>(); break;
	case 10: dungeon[y*cols + x].object = std::make_shared<MonsterIllumination>(); break;
	}

	dungeon[y*cols + x].item_name = dungeon[y*cols + x].object->getName();
	dungeon[y*cols + x].wall = false;

	int z = dungeon.isShop() ? 2 : -1;
	dungeon.addSprite(dungeon.item_sprites, x, y, z, dungeon[y*cols + x].object->getImageName());
}

InfinityBox::InfinityBox(int x, int y) : Chests(x, y, INFINITY_BOX, "Golden_Chest_48x48.png") {

}
void InfinityBox::open(Dungeon &dungeon) {
	int cols = dungeon.getCols();
	int x = getPosX();
	int y = getPosY();

	playSound(getSoundName());

	dungeon.removeSprite(dungeon.item_sprites, x, y);

	int n = 1 + randInt(4);
	switch (n) {
	case 1: dungeon[y*cols + x].object = std::make_shared<SteelPunch>(); break;
	case 2: dungeon[y*cols + x].object = std::make_shared<VulcanSword>(); break;
	case 3: dungeon[y*cols + x].object = std::make_shared<FrostShield>(); break;
	case 4: dungeon[y*cols + x].object = std::make_shared<VulcanHammer>(); break;
	default:
		break;
	}
	
	dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
}


//			:::: WEAPONS ::::
Weapon::Weapon(int x, int y, std::string name, std::string image, int dmg, int dexbonus)
	: Objects(x, y, name, image), m_dmg(dmg), m_dexbonus(dexbonus) {
	
}

Hands::Hands(int x, int y) : Weapon(x, y, HANDS, "Rainbow_Lollipop_48x48.png", 0, 0) {
	setDescription("It's a little dangerous to go empty-handed.");
}

//		BASIC WEAPONS
ShortSword::ShortSword(int x, int y) : Weapon(x, y, SHORT_SWORD, "Short_Sword_48x48.png", 3, 1) {
	setSoundName("Weapon_Pickup2.mp3");

	setDescription("Your trusty short sword. It's kept exceptionally clean.");
}

RadiantShortSword::RadiantShortSword(int x, int y) : Weapon(x, y, RADIANT_SHORT_SWORD, "Golden_Shortsword_48x48.png", 3, 1) {
	setSoundName("Weapon_Pickup.mp3");
	setHasBonus(true);

	setDescription("Gives light.");
}
void RadiantShortSword::applyBonus(Actors &a) {
	if (a.isPlayer()) {
		Player &p = dynamic_cast<Player&>(a);
		p.setVision(p.getVision() + 1);
	}
}
void RadiantShortSword::unapplyBonus(Actors &a) {
	if (a.isPlayer()) {
		Player &p = dynamic_cast<Player&>(a);
		p.setVision(p.getVision() - 1);
	}
}

CarbonFiberShortSword::CarbonFiberShortSword(int x, int y) : Weapon(x, y, CARBON_FIBER_SHORT_SWORD, "Carbon_Fiber_Short_Sword_48x48.png", 3, 1) {
	setSoundName("Weapon_Pickup.mp3");
	setHasBonus(true);

	setDescription("A standard weapon. Strong and lightweight.");
}
void CarbonFiberShortSword::applyBonus(Actors &a) {
	a.setDex(a.getDex() + 2);
}
void CarbonFiberShortSword::unapplyBonus(Actors &a) {
	a.setDex(a.getDex() - 2);
}

EnchantedShortSword::EnchantedShortSword(int x, int y) : Weapon(x, y, ENCHANTED_SHORT_SWORD, "Enchanted_Short_Sword_48x48.png", 3, 1) {
	setSoundName("Weapon_Pickup.mp3");
	setHasBonus(true);

	setDescription("You feel a faint energy channeling through.");
}
void EnchantedShortSword::applyBonus(Actors &a) {
	a.setInt(a.getInt() + 2);
}
void EnchantedShortSword::unapplyBonus(Actors &a) {
	a.setInt(a.getInt() - 2);
}

LuckyShortSword::LuckyShortSword(int x, int y) : Weapon(x, y, LUCKY_SHORT_SWORD, "Short_Sword_48x48.png", 3, 1) {
	setSoundName("Weapon_Pickup.mp3");
	setHasBonus(true);

	setDescription("You feel lucky while holding this.");
}
void LuckyShortSword::applyBonus(Actors &a) {
	a.setLuck(a.getLuck() + 10);
}
void LuckyShortSword::unapplyBonus(Actors &a) {
	a.setLuck(a.getLuck() - 10);
}

BloodShortSword::BloodShortSword(int x, int y) : Weapon(x, y, BLOOD_SHORT_SWORD, "Blood_Shortsword_48x48.png", 3, 1) {
	setSoundName("Weapon_Pickup2.mp3");
	setHasAbility(true);

	setDescription("This sword feels alive. But using it\nmakes you feel healthier.");
}
void BloodShortSword::useAbility(Dungeon &dungeon, Actors &a) {
	if (dungeon.getPlayer()->getHP() > 0 && dungeon.getPlayer()->getHP() < dungeon.getPlayer()->getMaxHP()) {
		if (randInt(100) + 1 > 90) {
			// 10% chance to heal the player if below max hp
			dungeon.getPlayerVector()[0]->setHP(dungeon.getPlayer()->getHP() + 2);
		}
	}
}

GoldenShortSword::GoldenShortSword(int x, int y) : Weapon(x, y, GOLDEN_SHORT_SWORD, "Golden_Shortsword_48x48.png", 2, 1) {
	setSoundName("Weapon_Pickup2.mp3");
	setHasAbility(true);

	setDescription("This sword is particularly bright when near lots of coins.");
}
void GoldenShortSword::useAbility(Dungeon &dungeon, Actors &a) {
	// + 1 damage for every X coins
	setDamage(getDmg() + dungeon.getPlayer()->getMoney() / 25);
}

BronzeDagger::BronzeDagger(int x, int y) : Weapon(x, y, BRONZE_DAGGER, "Bronze_Dagger_48x48.png", 2, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setHasAbility(true);

	setDescription("It won't hurt as much, but it'll make them bleed.");
}
void BronzeDagger::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBled()) {
		int roll = 1 + randInt(100) + a.getLuck();

		// 40% chance to bleed
		if (roll > m_bleedChance) {
			// play bleed sound effect
			playSound("Dagger_Slice_Bleed.mp3");

			a.addAffliction(std::make_shared<Bleed>());
		}
	}
}

RustyCutlass::RustyCutlass(int x, int y) : Weapon(x, y, RUSTY_CUTLASS, "Rusty_Broadsword_48x48.png", 2, 2) {
	setSoundName("Weapon_Pickup2.mp3");
	setAttackPattern(true);

	m_durability = 15 + randInt(6);

	setDescription("A rusty blade. It's seen better days.");
}
void RustyCutlass::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int n = 0, m = 0;
	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	if (dungeon[y*cols + x].enemy) {
		dungeon.fight(x, y);

		// 1 Turn of invulnerability on attack attempt
		dungeon.getPlayerVector().at(0)->addAffliction(std::make_shared<Invulnerability>(1));

		m_durability--;

		if (m_durability == 0) {
			playSound("Crumble_Breaking.mp3");

			dungeon.getPlayerVector().at(0)->getWeapon().reset();
			dungeon.getPlayerVector().at(0)->setWeapon(std::make_shared<Hands>());
		}
	}

}

BoneAxe::BoneAxe(int x, int y) : Weapon(x, y, BONE_AXE, "Bone_Axe_48x48.png", 4, 0) {
	setSoundName("Weapon_Pickup2.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("A primitive item, but it gets the job done.\nThrowable.");
}
void BoneAxe::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(dungeon.getPlayer()->getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	// if it was a movement action
	if (action == 'l' || action == 'r' || action == 'u' || action == 'd') {

		bool wall, enemy;
		int range = 1;

		// determines which way to move
		int n = 0, m = 0;

		int p = n, q = m; // For easier remembering of previous values

		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;

		while (!wall && range <= 12) {

			if (enemy) {
				playSound("Metal_Hit8.mp3");

				setDamage(getDmg() + 1); // damage boosted
				dungeon.fight(x + n, y + m);
				setDamage(getDmg() - 1); // remove damage boost

				moveUsed = true;
				break;
			}

			p = n; q = m;

			switch (move) {
			case 'l': n--; m = 0; break;
			case 'r': n++; m = 0; break;
			case 'u': n = 0; m--; break;
			case 'd': n = 0; m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			range++;
		}

		// Throw the mattock head on the space in front of the enemy/wall/as far as it could go
		dungeon.getPlayerVector()[0]->throwWeaponTo(dungeon, x + p, y + q);	
	}

	// effect to remove wind up effect
	untint(dungeon.getPlayer()->getSprite());

	moveUsed = true;
	m_woundUp = false;
}

Katana::Katana(int x, int y) : Weapon(x, y, KATANA, "Katana_48x48.png", 2, 3) {
	setSoundName("Weapon_Pickup2.mp3");
	setAttackPattern(true);

	setDescription("Way of the Samurai. Measure once. Cut twice.");
}
Katana::Katana(int x, int y, std::string name, std::string image, int dmg, int dex) 
	: Weapon(x, y, name, image, dmg, dex) {
	setSoundName("Weapon_Pickup2.mp3");
	setAttackPattern(true);
}
void Katana::usePattern(Dungeon &dungeon, bool &moveUsed) {
	
	int cols = dungeon.getCols();

	bool wall, enemy;
	char move = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// If there was an attempted hit, then check to do special ability
	if (m_hit) {
		if (!m_waitOver)
			m_waitOver = true;
		else {
			playMiss();

			// Attack anything there
			for (unsigned int i = 0; i < coords.size(); i++) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, coords[i].first, coords[i].second, 2);

				if (dungeon[coords[i].second*cols + coords[i].first].enemy)
					dungeon.fight(coords[i].first, coords[i].second);
			}
			m_hit = false;
			m_waitOver = false;
		}
	}

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
		if (enemy) {
			dungeon.fight(x + n, y + m);
			moveUsed = true;

			if (!m_hit) {
				m_hit = true;

				setCoords(dungeon);
			}
		}
	}
}
void Katana::setCoords(Dungeon &dungeon) {
	coords.clear();

	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int n = 0, m = 0;
	switch (dungeon.getPlayer()->getAction()) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (i == x && j == y)
				continue;

			// Attacked up or down
			if (n == 0) {

				// Attacked below
				if (m == 1) {
					if (j <= y)
						continue;
				}
				// Attacked above
				else {
					if (j >= y)
						continue;
				}
			}

			// Attacked to the side
			else if (m == 0) {
				// Attacked left
				if (n == -1) {
					if (i >= x)
						continue;
				}
				// Attacked right
				else {
					if (i <= x)
						continue;
				}
			}

			coords.push_back(std::make_pair(i, j));
		}
	}
}

GreaterKatana::GreaterKatana(int x, int y) : Katana(x, y, GREATER_KATANA, "Katana_48x48.png", 2, 3) {
	setDescription("Way of the Samurai. Measure once. Cut twice.");
}
void GreaterKatana::setCoords(Dungeon &dungeon) {
	coords.clear();

	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int n = 0, m = 0;
	switch (dungeon.getPlayer()->getAction()) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (i == x && j == y)
				continue;

			// Attacked up or down
			if (n == 0) {

				// Attacked below
				if (m == 1) {
					if (j < y)
						continue;
				}
				// Attacked above
				else {
					if (j > y)
						continue;
				}
			}

			// Attacked to the side
			else if (m == 0) {
				// Attacked left
				if (n == -1) {
					if (i > x)
						continue;
				}
				// Attacked right
				else {
					if (i < x)
						continue;
				}
			}

			coords.push_back(std::make_pair(i, j));
		}
	}
}

SuperiorKatana::SuperiorKatana(int x, int y) : Katana(x, y, SUPERIOR_KATANA, "Katana_48x48.png", 2, 3) {
	setDescription("Way of the Samurai. Measure once. Cut twice.");
}
void SuperiorKatana::setCoords(Dungeon &dungeon) {
	coords.clear();

	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int n = 0, m = 0;
	switch (dungeon.getPlayer()->getAction()) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (i == x && j == y)
				continue;

			coords.push_back(std::make_pair(i, j));
		}
	}
}

VulcanSword::VulcanSword(int x, int y) : Weapon(x, y, VULCAN_SWORD, "Vulcan_Sword_48x48.png", 3, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setHasAbility(true);

	setDescription("The blade is imbued with tiny embers that could melt\n through anything.");
}
void VulcanSword::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBurned()) {
		int roll = 1 + randInt(100) - a.getLuck();

		// 5% chance to burn
		if (roll < m_burnChance) {
			// play burned sound effect
			cocos2d::experimental::AudioEngine::play2d("Fire2.mp3", false, 0.7f);

			a.addAffliction(std::make_shared<Burn>(5));
		}
	}
}



//		LONG WEAPONS
LongWeapon::LongWeapon(int x, int y, int range, int dmg, int dex, int intellect, std::string name, std::string image) 
	: Weapon(x, y, name, image, dmg, dex), m_range(range) {
	setAttackPattern(true);
}
void LongWeapon::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	bool wall, enemy;
	char move = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int range = getRange();

	// Check if weapon can be cast
	checkSpecial(dungeon, moveUsed);
	if (moveUsed)
		return;

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
				dungeon.fight(x + n, y + m);
				moveUsed = true;
				
				// If this weapon doesn't pierce, then stop and return
				if (!hasPiercing())
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

Pike::Pike(int x, int y) : LongWeapon(x, y, 2, 2, 2, 0, PIKE, "Iron_Lance_48x48.png") {
	setSoundName("Weapon_Pickup.mp3");

	setDescription("Provides you with a bit more but much needed reach.");
}

RadiantPike::RadiantPike(int x, int y) : LongWeapon(x, y, 2, 2, 2, 0, RADIANT_PIKE, "Iron_Longsword_48x48.png") {
	setSoundName("Weapon_Pickup.mp3");
	setHasBonus(true);

	setDescription("Gives light.");
}
void RadiantPike::applyBonus(Actors &a) {
	if (a.isPlayer()) {
		Player &p = dynamic_cast<Player&>(a);
		p.setVision(p.getVision() + 1);
	}
}
void RadiantPike::unapplyBonus(Actors &a) {
	if (a.isPlayer()) {
		Player &p = dynamic_cast<Player&>(a);
		p.setVision(p.getVision() - 1);
	}
}

CarbonFiberPike::CarbonFiberPike(int x, int y) : LongWeapon(x, y, 2, 2, 2, 0, CARBON_FIBER_PIKE, "Iron_Longsword_48x48.png") {
	setSoundName("Weapon_Pickup.mp3");
	setHasBonus(true);

	setDescription("Strong and lightweight.");
}
void CarbonFiberPike::applyBonus(Actors &a) {
	a.setDex(a.getDex() + 2);
}
void CarbonFiberPike::unapplyBonus(Actors &a) {
	a.setDex(a.getDex() - 2);
}

EnchantedPike::EnchantedPike(int x, int y) : LongWeapon(x, y, 2, 2, 2, 0, ENCHANTED_PIKE, "Iron_Longsword_48x48.png") {
	setSoundName("Weapon_Pickup.mp3");
	setHasBonus(true);

	setDescription("You feel a faint energy channeling through.");
}
void EnchantedPike::applyBonus(Actors &a) {
	a.setInt(a.getInt() + 2);
}
void EnchantedPike::unapplyBonus(Actors &a) {
	a.setInt(a.getInt() - 2);
}

LuckyPike::LuckyPike(int x, int y) : LongWeapon(x, y, 2, 2, 2, 0, LUCKY_PIKE, "Iron_Longsword_48x48.png") {
	setSoundName("Weapon_Pickup.mp3");
	setHasBonus(true);

	setDescription("You feel lucky while holding this.");
}
void LuckyPike::applyBonus(Actors &a) {
	a.setLuck(a.getLuck() + 10);
}
void LuckyPike::unapplyBonus(Actors &a) {
	a.setLuck(a.getLuck() - 10);
}


//		PIERCING WEAPONS
PiercingWeapon::PiercingWeapon(int x, int y, int range, int dmg, int dex, int intellect, std::string name, std::string image) 
	: LongWeapon(x, y, range, dmg, dex, intellect, name, image) {

}

GoldenLongSword::GoldenLongSword(int x, int y) : PiercingWeapon(x, y, 2, 2, 1, 0, GOLDEN_LONG_SWORD, "Golden_Longsword_48x48.png") {
	setSoundName("Weapon_Pickup2.mp3");
	setHasAbility(true);

	setDescription("This long sword is particularly bright when near lots of coins.");
}
void GoldenLongSword::useAbility(Dungeon &dungeon, Actors &a) {
	// + 1 damage for every X coins
	setDamage(2 + dungeon.getPlayer()->getMoney() / 25);
}

IronLongSword::IronLongSword(int x, int y) : PiercingWeapon(x, y, 2, 4, 1, 0, IRON_LONG_SWORD, "Iron_Longsword_48x48.png") {
	setSoundName("Weapon_Pickup2.mp3");

	setDescription("Your reach is quite excellent with this in hand. \nPierces enemies.");
}


//		BOWS
Bow::Bow(int x, int y, std::string name, std::string image, int damage, int dexbonus, int range) 
	: Weapon(x, y, name, image, damage, dexbonus), m_range(range) {
	setAttackPattern(true);
}
void Bow::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	bool wall, enemy;
	char move = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int range = getRange();

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

WoodBow::WoodBow(int x, int y) : Bow(x, y, WOOD_BOW, "Wood_Bow_32x32.png", 2, 2, 4) {
	setDescription("A classic long ranged weapon, but ineffective\n at close range.");
}

IronBow::IronBow(int x, int y) : Bow(x, y, IRON_BOW, "Reinforced_Bow_48x48.png", 3, 2, 4) {
	setDescription("This bow has been reinforced yet feels lighter.\n Not effective at close range.");
}

VulcanBow::VulcanBow(int x, int y) : Bow(x, y, VULCAN_BOW, "Vulcan_Bow_48x48.png", 3, 2, 5) {
	setHasAbility(true);

	setDescription("The legendary weapon smith's craftsmanship cannot\n go unnoticed. Doubtless that this bow will deliver.");
}
void VulcanBow::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBurned()) {
		int roll = randInt(100) - a.getLuck();

		// 5% chance to burn
		if (roll < m_burnChance) {
			// play burned sound effect
			playSound("Fire2.mp3");

			a.addAffliction(std::make_shared<Burn>(5));
		}
	}
}



//		 SPECIALTY WEAPONS
Estoc::Estoc(int x, int y) : Weapon(x, y, ESTOC, "Long_Sword_48x48.png", 2, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("A long pointed weapon suitable for lunging.\nDeals bonus damage the farther you lunge.");
}
Estoc::Estoc(int x, int y, std::string name, std::string image, int dmg, int dex) 
	: Weapon(x, y, name, image, dmg, dex) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);
}
void Estoc::usePattern(Dungeon &dungeon, bool &moveUsed) {

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if hammer is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(dungeon.getPlayer()->getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	// if it was a movement action
	if (action == 'l' || action == 'r' || action == 'u' || action == 'd') {
		bool wall, enemy;
		bool firstHit = true;
		int range = 1;
		int pos;

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

		while (!wall && range <= m_limit) {

			if (enemy) {

				int damageBoost = 0;
				switch (range) {
				case 1: break;
				case 2: damageBoost = 2; break;
				case 3: damageBoost = 4; break;
				case 4: damageBoost = 6; break;
				}

				pos = dungeon.findMonster(x + n, y + m);

				// Superior effect
				if (isSuperior() && pos != -1)
					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

				setDamage(getDmg() + damageBoost); // damage boosted
				dungeon.fight(x + n, y + m);
				setDamage(getDmg() - damageBoost); // remove damage boost

				moveUsed = true;

				// If player did not lunge, then no piercing should be done
				if (range == 1)
					break;

				// If player already attacked, then this was the piercing attack
				if (!firstHit)
					break;

				// If this is the first hit, then move the player just behind and try to do piercing once
				if (firstHit) {

					// If player lunged, move them
					if (range != 1) {
						int p = x + n + (n < 0 ? 1 : n > 0 ? -1 : 0);
						int q = y + m + (m < 0 ? 1 : m > 0 ? -1 : 0);
						dungeon.getPlayerVector()[0]->moveTo(dungeon, p, q);

						if (dungeon[(q)*cols + (p)].trap) {
							dungeon.trapEncounter(p, q);
						}
					}

					range -= 1; // Decrease range by one to maintain the same damage boost on any piercing done
					firstHit = false;

					// Piercing if Greater + Superior only.
					if (!canPierce())
						break;
				}

			}
			else if (!enemy && !firstHit && !isSuperior())
				break;

			switch (move) {
			case 'l': n--; m = 0; break;
			case 'r': n++; m = 0; break;
			case 'u': n = 0; m--; break;
			case 'd': n = 0; m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			range++;
		}


	}

	// effect to remove wind up effect
	untint(dungeon.getPlayer()->getSprite());

	moveUsed = true;
	m_woundUp = false;
}

GreaterEstoc::GreaterEstoc(int x, int y) : Estoc(x, y, GREATER_ESTOC, "Long_Sword_48x48.png", 2, 2) {
	//m_limit = 4;
	setDescription("A long pointed weapon suitable for lunging.\nDeals bonus damage the farther you lunge.\nPierces.");
}

SuperiorEstoc::SuperiorEstoc(int x, int y) : Estoc(x, y, SUPERIOR_ESTOC, "Long_Sword_48x48.png", 3, 2) {
	//m_limit = 4;
	setDescription("A long pointed weapon suitable for lunging.\nDeals bonus damage the farther you lunge.\nThis weapon is a force to be reckoned with.");
}

Zweihander::Zweihander(int x, int y) : Weapon(x, y, ZWEIHANDER, "Long_Sword_48x48.png", 2, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	//setCast(true);

	setDescription("Provides excellent reach laterally.");
}
Zweihander::Zweihander(int x, int y, std::string name, std::string image, int dmg, int dex) 
	: Weapon(x, y, name, image, dmg, dex) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
}
void Zweihander::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// Check if player used weapon cast
	checkSpecial(dungeon, moveUsed);
	if (moveUsed)
		return;

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

		// Player attacked to the side, so check above and below
		if (m == 0) {

			if (dungeon[(y + m)*cols + (x + n)].enemy) {
				dungeon.fight(x + n, y + m);
				moveUsed = true;
			}
			if (dungeon[(y + 1)*cols + (x + n)].enemy) {
				dungeon.fight(x + n, y + 1);
				moveUsed = true;
			}
			if (dungeon[(y - 1)*cols + (x + n)].enemy) {
				dungeon.fight(x + n, y - 1);
				moveUsed = true;
			}
		}
		// Else player attacked above or below, so check the sides
		else {

			if (dungeon[(y + m)*cols + (x + n)].enemy) {			
				dungeon.fight(x + n, y + m);
				moveUsed = true;
			}
			if (dungeon[(y + m)*cols + (x + 1)].enemy) {
				dungeon.fight(x + 1, y + m);
				moveUsed = true;
			}
			if (dungeon[(y + m)*cols + (x - 1)].enemy) {
				dungeon.fight(x - 1, y + m);
				moveUsed = true;
			}
		}

		// Superior effect
		if (!moveUsed && isSuperior()) {
			bool wall, enemy;
			int range = 2;
			int pos = -1;

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;

			while (!wall && range > 0) {

				if (enemy) {
					setDamage(getDmg() + 2);
					dungeon.fight(x + n, y + m);
					setDamage(getDmg() - 2);

					pos = dungeon.findMonster(x + n, y + m);
					moveUsed = true;
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

				if (moveUsed)
					break;
			}

			// Knock the monster back one tile
			/*if (pos != -1) {
				if (!(wall || enemy))
					dungeon.getMonsters().at(pos)->moveTo(dungeon, x + n, y + m);
			}*/
		}
	}
}

GreaterZweihander::GreaterZweihander(int x, int y) : Zweihander(x, y, GREATER_ZWEIHANDER, "Long_Sword_48x48.png", 2, 2) {
	setCast(true);

	setDescription("Provides excellent reach both laterally and literally.");
}
void GreaterZweihander::checkSpecial(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(dungeon.getPlayer()->getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	// if it was a movement action
	if (action == 'l' || action == 'r' || action == 'u' || action == 'd') {

		bool wall, enemy;
		int range = 2;
		int pos = -1;

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
				setDamage(getDmg() + 2);
				dungeon.fight(x + n, y + m);
				setDamage(getDmg() - 2);

				pos = dungeon.findMonster(x + n, y + m);
				moveUsed = true;
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

			if (moveUsed)
				break;
		}

		// Knock the monster back one tile
		/*if (pos != -1) {
			if (!(wall || enemy))
				dungeon.getMonsters().at(pos)->moveTo(dungeon, x + n, y + m);
		}*/
	}

	// effect to remove wind up effect
	untint(dungeon.getPlayer()->getSprite());

	moveUsed = true;
	m_woundUp = false;
}

SuperiorZweihander::SuperiorZweihander(int x, int y) : Zweihander(x, y, SUPERIOR_ZWEIHANDER, "Long_Sword_48x48.png", 3, 2) {
	setDescription("Provides excellent reach both laterally and literally\nwith incredible ease.");
}

Claw::Claw(int x, int y) : Weapon(x, y, CLAW, "Swift_Sword_48x48.png", 2, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setHasAbility(true);
	setCast(true);

	setDescription("Provides excellent reach both laterally and literally.");
}
void Claw::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(dungeon.getPlayer()->getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	// if it was a movement action
	if (action == 'l' || action == 'r' || action == 'u' || action == 'd') {

		bool wall, enemy;
		int range = 1;
		int pos = -1;

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

		// Find an enemy to hook
		while (!wall && range <= 4) {

			if (enemy) {
				cocos2d::experimental::AudioEngine::play2d("Dagger_Slice_Bleed.mp3", false, 0.7f);
				
				pos = dungeon.findMonster(x + n, y + m);
				moveUsed = true;
				break;
			}

			switch (move) {
			case 'l': n--; m = 0; break;
			case 'r': n++; m = 0; break;
			case 'u': n = 0; m--; break;
			case 'd': n = 0; m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			range++;
		}

		// Now pull them in
		if (moveUsed && pos != -1) {

			bool trap, hero;

			// Reversed movement since we're now moving toward the player
			switch (move) {
			case 'l': n++; m = 0; break;
			case 'r': n--; m = 0; break;
			case 'u': n = 0; m++; break;
			case 'd': n = 0; m--; break;
			}

			trap = dungeon[(y + m)*cols + (x + n)].trap;
			hero = dungeon[(y + m)*cols + (x + n)].hero;

			// If enemy was farther than one tile away, pull them toward the player
			while (!hero && range > 1) {

				// Move the monster toward the player
				dungeon.getMonsters().at(pos)->moveTo(dungeon, x + n, y + m, 0.045f);

				// And check for traps along the way
				if (trap) {
					dungeon.singleMonsterTrapEncounter(pos);

					// Now try to refresh the monster index since they might have died to a trap
					pos = dungeon.findMonster(x + n, y + m);
					if (pos == -1)
						break;
				}

				switch (move) {
				case 'l': n++; m = 0; break;
				case 'r': n--; m = 0; break;
				case 'u': n = 0; m++; break;
				case 'd': n = 0; m--; break;
				}

				trap = dungeon[(y + m)*cols + (x + n)].trap;
				hero = dungeon[(y + m)*cols + (x + n)].hero;
				range--;
			}

			// Stun monster for one turn at the end
			if (pos != -1) {
				if (dungeon.getMonsters().at(pos)->canBeStunned()) {
					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
				}
			}
		}
	}

	// effect to remove wind up effect
	untint(dungeon.getPlayer()->getSprite());

	moveUsed = true;
	m_woundUp = false;
}
void Claw::useAbility(Dungeon &dungeon, Actors &a) {

	// if weapon was primed, apply stun
	if (m_woundUp && a.canBeStunned()) {
		a.addAffliction(std::make_shared<Stun>(1));
	}

	if (a.canBeBled()) {
		int roll = 1 + randInt(100) - a.getLuck();

		// 10% chance to bleed
		if (roll <= m_bleedChance) {
			playSound("Dagger_Slice_Bleed.mp3");

			a.addAffliction(std::make_shared<Bleed>());
		}
	}
}

Hammer::Hammer(int x, int y) : Weapon(x, y, HAMMER, "Vulcan_Hammer_48x48.png", 3, 1) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("Smash your enemies into the ground.");
}
Hammer::Hammer(int x, int y, std::string name, std::string image, int dmg, int dex) 
	: Weapon(x, y, name, image, dmg, dex) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);
}
void Hammer::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(dungeon.getPlayer()->getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	// if it was a movement action
	if (action == 'l' || action == 'r' || action == 'u' || action == 'd') {
		
		int n = 0, m = 0;
		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		if (dungeon[(y + m)*cols + (x + n)].enemy) {
			// play fire blast explosion sound effect
			playSound("Fireblast_Spell2.mp3");

			int damageBoost = 3;
			if (TStun()) damageBoost = 4;
			else if (boxStun()) damageBoost = 5;

			setDamage(getDmg() + damageBoost); // damage boosted
			int pos = dungeon.findMonster(x + n, y + m);
			if (pos != -1 && dungeon.getMonsters().at(pos)->canBeStunned())
				dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

			dungeon.fight(x + n, y + m);
			setDamage(getDmg() - damageBoost); // remove damage boost

			// Greater effect
			if (TStun())
				TPattern(dungeon);

			// Superior effect
			if (boxStun())
				boxPattern(dungeon);
		}
	}

	// effect to remove wind up effect
	untint(dungeon.getPlayer()->getSprite());

	moveUsed = true;
	m_woundUp = false;
}
void Hammer::TPattern(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	char move = dungeon.getPlayer()->facingDirection();

	int n = 0, m = 0;
	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	setDamage(getDmg() + 1);

	std::vector<std::pair<int, int>> coords;
	// Player attacked to the side, so check above and below
	if (m == 0) {

		// Attacked left, so check one beyond that
		if (n == -1)
			coords.push_back(std::make_pair(x + n - 1, y + m));
		
		// Attacked right, so check one beyond that
		if (n == 1) 
			coords.push_back(std::make_pair(x + n + 1, y + m));
		
		coords.push_back(std::make_pair(x + n, y + 1));
		coords.push_back(std::make_pair(x + n, y - 1));	
	}
	// Else player attacked above or below, so check the sides
	else {
		// Attacked upward
		if (m == -1)
			coords.push_back(std::make_pair(x + n, y + m - 1));
		
		// Attacked downward
		if (m == 1)
			coords.push_back(std::make_pair(x + n, y + m + 1));
		
		coords.push_back(std::make_pair(x + 1, y + m));
		coords.push_back(std::make_pair(x - 1, y + m));	
	}

	for (unsigned int i = 0; i < coords.size(); i++) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, coords[i].first, coords[i].second, 2);

		if (dungeon[coords[i].second*cols + coords[i].first].enemy) {
			int pos = dungeon.findMonster(coords[i].first, coords[i].second);
			if (pos != -1) {
				if (dungeon.getMonsters().at(pos)->canBeStunned() && 1 + randInt(100) + dungeon.getPlayer()->getLuck() > 50)
					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

				dungeon.fight(coords[i].first, coords[i].second);
			}
		}
	}

	setDamage(getDmg() - 1);
}
void Hammer::boxPattern(Dungeon &dungeon) {
	
	int cols = dungeon.getCols();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	bool wall, enemy;
	char move = dungeon.getPlayer()->facingDirection();

	int n, m; // n : x, m : y
	switch (move) {
	case 'l': n = -1; m = -1; break;
	case 'r': n = 1; m = -1; break;
	case 'u': n = -1; m = -1; break;
	case 'd': n = -1; m = 1; break;
	}

	int range = 2;
	int currentRange = 1;

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;

	int k = 1; // 3 rows/columns to check
	while (k <= 3) {

		while (!wall && currentRange <= range) {

			cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

			if (enemy) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1) {
					if (dungeon.getMonsters().at(pos)->canBeStunned())
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

					dungeon.damageMonster(pos, getDmg());
				}
			}

			switch (move) {
			case 'l': n--; break;
			case 'r': n++; break;
			case 'u': m--; break;
			case 'd': m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			currentRange++;
		}

		switch (move) {
		case 'l': n = -1; m++; break;
		case 'r': n = 1; m++; break;
		case 'u': n++; m = -1; break;
		case 'd': n++; m = 1; break;
		}

		k++;
		currentRange = 1;
		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;
	}
}

GreaterHammer::GreaterHammer(int x, int y) : Hammer(x, y, GREATER_HAMMER, "Vulcan_Hammer_48x48.png", 3, 1) {
	setDescription("Smash your enemies into the ground harder.");
}

SuperiorHammer::SuperiorHammer(int x, int y) : Hammer(x, y, SUPERIOR_HAMMER, "Vulcan_Hammer_48x48.png", 3, 1) {
	setDescription("Smash your enemies into the ground with all your might.");
}

VulcanHammer::VulcanHammer(int x, int y) : Hammer(x, y, VULCAN_HAMMER, "Vulcan_Hammer_48x48.png", 3, 1) {
	setSoundName("Weapon_Pickup.mp3");
	setHasAbility(true);

	setDescription("The hammer that the legendary blacksmith used to\n forge his weapons. Packs one heck of a punch.");
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
			playSound("Fire2.mp3", 0.7f);

			a.addAffliction(std::make_shared<Burn>(5));
		}
	}
}

ArcaneStaff::ArcaneStaff(int x, int y) : Weapon(x, y, ARCANE_STAFF, "Arcane_Staff_48x48.png", 1, 1) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setHasBonus(true);
	setCast(true);

	setDescription("Ooo, sparkly.");
}
void ArcaneStaff::giveBonus(Actors &a) {
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
	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// if player didn't cast, then return
	if (!m_isCast && action != WIND_UP) {
		m_isCast = false;
		return;
	}

	// if staff is not cast and player has cast, then do so and return
	if (!m_isCast && action == WIND_UP) {
		// effect to show wind up
		tintStaffCast(dungeon.getPlayer()->getSprite());

		playSound("Staff_Cast1.mp3");

		// boost int
		giveBonus(*dungeon.getPlayerVector()[0]);

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
	case 'd': unapplyBonus(*dungeon.getPlayerVector()[0]); m_isCast = false; break;
	}

}

BoStaff::BoStaff(int x, int y) : Weapon(x, y, BO_STAFF, "Long_Sword_48x48.png", 3, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);

	setDescription("A classic weapon in ancient times.\nProtects your backside.");
}
BoStaff::BoStaff(int x, int y, std::string name, std::string image, int dmg, int dex) 
	: Weapon(x, y, name, image, dmg, dex) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
}
void BoStaff::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	bool wall, enemy;
	char move = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int range = 1;
	int pos;

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
				dungeon.fight(x + n, y + m);
				moveUsed = true;
				pos = dungeon.findMonster(x + n, y + m);

				// Superior Bo Staff knockback
				if (pos != -1 && canPush()) {
					dungeon.linearActorPush(x + n, y + m, 1, move);
				}
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

			if (moveUsed)
				break;
		}

		// If there was a hit, try to knock the enemy back
		if (moveUsed) {

			// Knock enemy in front of you back if not Superior Bo Staff
			if (!(wall || enemy) && !canPush()) {
				if (pos != -1)
					dungeon.getMonsters().at(pos)->moveTo(dungeon, x + n, y + m);
			}

			// Now check behind the player for an enemy
			switch (move) {
			case 'l': n = 1; m = 0; break;
			case 'r': n = -1; m = 0; break;
			case 'u': n = 0; m = 1; break;
			case 'd': n = 0; m = -1; break;
			}

			// If there's an enemy, hit them
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			if (enemy) {
				
				dungeon.fight(x + n, y + m);
				pos = dungeon.findMonster(x + n, y + m);

				// Now try to knock them back as well
				if (pos != -1) {

					// Superior Bo Staff knockback
					if (canPush()) {
						switch (move) {
						case 'l': move = 'r'; break;
						case 'r': move = 'l'; break;
						case 'u': move = 'd'; break;
						case 'd': move = 'u'; break;
						}

						dungeon.linearActorPush(x + n, y + m, 1, move);
						return;
					}
			
					switch (move) {
					case 'l': n++; m = 0; break;
					case 'r': n--; m = 0; break;
					case 'u': n = 0; m++; break;
					case 'd': n = 0; m--; break;
					}

					wall = dungeon[(y + m)*cols + (x + n)].wall;
					enemy = dungeon[(y + m)*cols + (x + n)].enemy;
				
					if (!(wall || enemy))
						dungeon.getMonsters().at(pos)->moveTo(dungeon, x + n, y + m);
				}
			}

			return;
		}
		// Greater/Superior Bo Staff effect
		else if (canBackHit()) {
			// Now check behind the player for an enemy
			switch (move) {
			case 'l': n = 1; m = 0; break;
			case 'r': n = -1; m = 0; break;
			case 'u': n = 0; m = 1; break;
			case 'd': n = 0; m = -1; break;
			}

			// If there's an enemy, hit them
			if (dungeon[(y + m)*cols + (x + n)].enemy) {

				dungeon.fight(x + n, y + m);
				pos = dungeon.findMonster(x + n, y + m);

				// Now try to knock them back as well
				if (pos != -1) {

					// Superior Bo Staff knockback
					if (canPush()) {
						switch (move) {
						case 'l': move = 'r'; break;
						case 'r': move = 'l'; break;
						case 'u': move = 'd'; break;
						case 'd': move = 'u'; break;
						}

						dungeon.linearActorPush(x + n, y + m, 1, move);					
						return;
					}

					switch (move) {
					case 'l': n++; m = 0; break;
					case 'r': n--; m = 0; break;
					case 'u': n = 0; m++; break;
					case 'd': n = 0; m--; break;
					}

					wall = dungeon[(y + m)*cols + (x + n)].wall;
					enemy = dungeon[(y + m)*cols + (x + n)].enemy;

					if (!(wall || enemy))
						dungeon.getMonsters().at(pos)->moveTo(dungeon, x + n, y + m);
				}
			}
		}
	}
}

GreaterBoStaff::GreaterBoStaff(int x, int y) : BoStaff(x, y, GREATER_BO_STAFF, "Long_Sword_48x48.png", 3, 2) {
	setDescription("A classic weapon in ancient times. Your\nskill has seemed to increase.");
}

SuperiorBoStaff::SuperiorBoStaff(int x, int y) : BoStaff(x, y, SUPERIOR_BO_STAFF, "Long_Sword_48x48.png", 4, 2) {
	setDescription("A classic weapon in ancient times. The\npower emanating from this weapon is immense.");
}

Nunchuks::Nunchuks(int x, int y) : Weapon(x, y, NUNCHUKS, "Blood_Staff_48x48.png", 2, 3) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);

	setDescription("Another classic weapon in ancient times. Smack\naround some foes to your sides.");
}
Nunchuks::Nunchuks(int x, int y, std::string name, std::string image, int dmg, int dex) 
	: Weapon(x, y, name, image, dmg, dex) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
}
void Nunchuks::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	bool wall, enemy;
	char move = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

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

		// If an enemy is hit, consume the action
		if (enemy) {
			dungeon.fight(x + n, y + m);
			moveUsed = true;

			if (!stationaryStun())
				return;
		}

		// Superior effect
		if (!enemy && moreStun()) {
			int a, b, c, d;
			switch (move) {
			case 'l': a = -1; b = -1; c = -1; d = 1; break;
			case 'r': a = 1; b = -1; c = 1; d = 1; break;
			case 'u':a = -1; b = -1; c = 1; d = -1; break;
			case 'd': a = -1; b = 1; c = 1; d = 1; break;
			}

			if (dungeon[(y + b)*cols + (x + a)].enemy) {
				int pos = dungeon.findMonster(x + a, y + b);
				if (pos != -1 && dungeon.getMonsters().at(pos)->canBeStunned()) {
					// Stun sprite effect
					playEnemyHit();
					gotStunned(dungeon.getMonsters().at(pos)->getSprite());

					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
				}
			}
			if (dungeon[(y + d)*cols + (x + c)].enemy) {
				int pos = dungeon.findMonster(x + c, y + d);
				if (pos != -1 && dungeon.getMonsters().at(pos)->canBeStunned()) {
					// Stun sprite effect
					playEnemyHit();
					gotStunned(dungeon.getMonsters().at(pos)->getSprite());

					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
				}
			}
		}
		
		// Now check for enemies next to the player to stun
		int a, b, c, d;
		switch (move) {
		case 'l': 
		case 'r': a = 0; b = -1; c = 0; d = 1; break;
		case 'u': 
		case 'd': a = -1; b = 0; c = 1; d = 0; break;
		}

		if (dungeon[(y + b)*cols + (x + a)].enemy) {
			int pos = dungeon.findMonster(x + a, y + b);
			if (pos != -1 && dungeon.getMonsters().at(pos)->canBeStunned()) {
				// Stun sprite effect
				playEnemyHit();
				gotStunned(dungeon.getMonsters().at(pos)->getSprite());

				dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
			}
		}
		if (dungeon[(y + d)*cols + (x + c)].enemy) {
			int pos = dungeon.findMonster(x + c, y + d);
			if (pos != -1 && dungeon.getMonsters().at(pos)->canBeStunned()) {
				// Stun sprite effect
				playEnemyHit();
				gotStunned(dungeon.getMonsters().at(pos)->getSprite());

				dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
			}
		}

	}
}

GreaterNunchuks::GreaterNunchuks(int x, int y) : Nunchuks(x, y, GREATER_NUNCHUKS, "Blood_Staff_48x48.png", 2, 3) {
	setDescription("Another classic weapon in ancient times.");
}

SuperiorNunchuks::SuperiorNunchuks(int x, int y) : Nunchuks(x, y, SUPERIOR_NUNCHUKS, "Blood_Staff_48x48.png", 2, 3) {
	setDescription("Those who mastered this weapon were nearly\nuntouchable. Whoever left this behind, you have\nacquired some of its might.");
}

Mattock::Mattock(int x, int y) : Weapon(x, y, MATTOCK, "Vulcan_Axe_48x48.png", 2, 1) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);

	m_durability = 15 + randInt(11);

	setDescription("The greatest ability of all. Susceptible to breaking.");
}
void Mattock::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	bool wall;
	char move = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

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
		if (wall && dungeon[(y + m)*cols + (x + n)].wall_type == REG_WALL) {
			playSound("Metal_Hit8.mp3");

			dungeon[(y + m)*cols + (x + n)].wall = false;
			dungeon.removeSprite(dungeon.wall_sprites, x + n, y + m);

			m_durability--;

			// If mattock broke, unequip the mattock and drop a mattock head
			if (m_durability == 0) {
				dungeon.getPlayerVector()[0]->setWeapon(std::make_shared<Hands>());
				if (dungeon[y*cols + x].item)
					dungeon.itemHash(x, y);

				dungeon[y*cols + x].item = true;
				dungeon[y*cols + x].object = std::make_shared<MattockHead>();
				dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
			}

			moveUsed = true;
			return;
		}
	}
}

MattockHead::MattockHead(int x, int y) : Weapon(x, y, MATTOCK_HEAD, "Vulcan_Dagger_48x48.png", 1, 1) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);

	m_durability = 5 + randInt(6);

	setDescription("It seems we weren't careful were we. At least you\ncan throw it.");
}
void MattockHead::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(dungeon.getPlayer()->getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	// if it was a movement action
	if (action == 'l' || action == 'r' || action == 'u' || action == 'd') {

		bool wall, enemy;
		int range = 1;

		// determines which way to move
		int n = 0, m = 0;

		int p = n, q = m; // For easier remembering of previous values

		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}	

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;

		while (!wall && range <= 12) {

			if (enemy) {
				playSound("Metal_Hit8.mp3");

				setDamage(getDmg() + 4); // damage boosted
				dungeon.fight(x + n, y + m);
				setDamage(getDmg() - 4); // remove damage boost

				moveUsed = true;
				break;
			}

			p = n; q = m;

			switch (move) {
			case 'l': n--; m = 0; break;
			case 'r': n++; m = 0; break;
			case 'u': n = 0; m--; break;
			case 'd': n = 0; m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
			range++;
		}

		m_durability--;
		
		// If durability hits zero, then replace the mattock head with mattock dust
		if (m_durability == 0) {
			dungeon.getPlayerVector()[0]->setWeapon(std::make_shared<Hands>());
			if (dungeon[y*cols + x].item)
				dungeon.itemHash(x, y);

			dungeon[y*cols + x].item = true;
			dungeon[y*cols + x].object = std::make_shared<MattockDust>();
			dungeon.addSprite(dungeon.item_sprites, x, y, -1, dungeon[y*cols + x].object->getImageName());
		}
		else {
			// Throw the mattock head on the space in front of the enemy/wall/as far as it could go
			dungeon.getPlayerVector()[0]->throwWeaponTo(dungeon, x + p, y + q);
		}
	}

	// effect to remove wind up effect
	untint(dungeon.getPlayer()->getSprite());

	moveUsed = true;
	m_woundUp = false;
}

Pistol::Pistol(int x, int y) : Weapon(x, y, PISTOL, "Vulcan_Dagger_48x48.png", 1, 0) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("Items of this mechanical nature were largely\ndestroyed in the times of magic. It looks like\nthis one survived, but it is severely damaged\nand can only hold two rounds at a time.");
}
void Pistol::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// If action was not the special ability, do nothing and return
	if (action != WIND_UP)
		return;

	// Check if player tried to use the special ability
	if (action == WIND_UP) {

		// If reloading, then return
		if (m_reloading) {
			if (m_rounds < 2) {
				playSound("Metal_Hit8.mp3");

				m_rounds++;

				// Show special button when loaded
				if (m_rounds == 2)
					setCast(true);

				return;
			}
			m_reloading = false;
		}

		// effect to show wind up
		tintItemCast(dungeon.getPlayer()->getSprite());

		playSound("Fireblast_Spell1.mp3");

		bool wall, enemy;
		int n = 0, m = 0;
		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;

		while (!wall) {

			cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

			if (enemy) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1) {
					if (dungeon.getMonsters().at(pos)->canBeBled())
						dungeon.giveAffliction(pos, std::make_shared<Bleed>());

					dungeon.damageMonster(pos, 6);
				}			

				moveUsed = true;
				break;
			}

			switch (move) {
			case 'l': n--; m = 0; break;
			case 'r': n++; m = 0; break;
			case 'u': n = 0; m--; break;
			case 'd': n = 0; m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		}
	}

	m_rounds--;
	if (m_rounds == 0) {
		m_reloading = true;

		setCast(false); // Removes label from HUD
	}

	// effect to remove wind up effect
	untint(dungeon.getPlayer()->getSprite());

	moveUsed = true;
}

Whip::Whip(int x, int y) : Weapon(x, y, WHIP, "Reinforced_Bow_48x48.png", 2, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);

	setDescription("The weapon of choice among all archaeological\nadventure lovers. More effective if strikes are\nmaximum range.");
}
void Whip::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();
	bool wall, enemy;
	char move = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	int range = 1;
	int pos;

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

		while (!wall && range <= 3) {

			if (enemy) {

				// Sweetspot bonus
				if (range == 3)
					setDamage(getDmg() + 1);

				dungeon.fight(x + n, y + m);

				// Remove sweetspot bonus
				if (range == 3)
					setDamage(getDmg() - 1);

				pos = dungeon.findMonster(x + n, y + m);
				
				moveUsed = true;
			}

			switch (move) {
			case 'l': n--; m = 0; break;
			case 'r': n++; m = 0; break;
			case 'u': n = 0; m--; break;
			case 'd': n = 0; m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;

			if (moveUsed)
				break;

			range++;
		}

		// Attempt knockback by one tile if enemy was hit by the sweetspot
		if (moveUsed && range == 3 && !(wall || enemy)) {
			if (pos != -1) {
				if (!dungeon.getMonsters().at(pos)->isHeavy())
					dungeon.getMonsters().at(pos)->moveTo(dungeon, x + n, y + m);
			}
		}
	}
}


//		METERED WEAPONS
MeterWeapon::MeterWeapon(int x, int y, int charge, int dmg, int dex, std::string name, std::string image) 
	: Weapon(x, y, name, image, dmg, dex), m_charge(charge), m_maxCharge(charge) {

}

void MeterWeapon::increaseCharge() {
	if (m_charge < m_maxCharge)
		m_charge++;

	// Show special button when charged
	if (m_charge == m_maxCharge)
		setCast(true);
}

Jian::Jian(int x, int y) : MeterWeapon(x, y, 6, 2, 2, JIAN, "Long_Sword_48x48.png") {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("Yet another classic weapon in ancient times.");
}
Jian::Jian(int x, int y, std::string name, std::string image, int charge, int dmg, int dex) 
	: MeterWeapon(x, y, charge, dmg, dex, name, image) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);
}
void Jian::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// If action was not the special ability, do nothing and return
	if (action != WIND_UP)
		return;

	// Check if player tried to use the special ability
	if (action == WIND_UP) {

		// If not enough charge, then return
		if (getCharge() < getMaxCharge())
			return;

		// effect to show wind up
		tintItemCast(dungeon.getPlayer()->getSprite());

		playSound("FootStepGeneric2.mp3");

		for (int i = x - 1; i < x + 2; i++) {
			for (int j = y - 1; j < y + 2; j++) {

				// Ignore player's spot
				if (i == x && j == y)
					continue;

				cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, i, j, 2);

				if (dungeon[j*cols + i].enemy) {

					/*if (canBleed()) {
						int pos = dungeon.findMonster(i, j);
						if (pos != -1)
							if (dungeon.getMonsters().at(pos)->canBeBled())
								dungeon.giveAffliction(pos, std::make_shared<Bleed>());
					}*/

					float playerHealthPercentage = dungeon.getPlayer()->getHP() / (float)dungeon.getPlayer()->getMaxHP();

					// If player has at least 90% of their hp remaining, the special ability does extra damage
					if (hasBonusDamage() && playerHealthPercentage >= 0.9f)
						setDamage(getDmg() + 5);
					
					int pos = dungeon.findMonster(i, j);
					dungeon.damageMonster(pos, getDmg());

					// Do knockback and stun if SuperiorJian
					if (pos != -1 && hasKnockback()) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

						char move = dungeon.getDirectionRelativeTo(x, y, i, j);
						dungeon.linearActorPush(i, j, 1, move);
					}				

					if (hasBonusDamage() && playerHealthPercentage >= 0.9f)
						setDamage(getDmg() - 5);				
				}
			}
		}
		
	}

	resetCharge();
	setCast(false); // Removes label from HUD

	playSound("Wind_Spell2.mp3");

	// effect to remove wind up effect
	untint(dungeon.getPlayer()->getSprite());

	moveUsed = true;
}

GreaterJian::GreaterJian(int x, int y) : Jian(x, y, GREATER_JIAN, "Long_Sword_48x48.png", 7, 2, 2) {
	setDescription("Yet another classic weapon in ancient times.");
}

SuperiorJian::SuperiorJian(int x, int y) : Jian(x, y, SUPERIOR_JIAN, "Long_Sword_48x48.png", 8, 3, 2) {
	setDescription("Yet another classic weapon in ancient times.");
}

Boomerang::Boomerang(int x, int y) : MeterWeapon(x, y, 5, 2, 2, BOOMERANG, "Long_Sword_48x48.png") {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("Always comes back because of the aura of this place,\nnot because of your skill.");
}
void Boomerang::usePattern(Dungeon &dungeon, bool &moveUsed) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = dungeon.getPlayer()->getPosX();
	int y = dungeon.getPlayer()->getPosY();

	// If action was not the special ability, do nothing and return
	if (action != WIND_UP)
		return;

	// Check if player tried to use the special ability
	if (action == WIND_UP) {

		// If not enough charge, then return
		if (getCharge() < getMaxCharge())
			return;

		// effect to show wind up
		tintItemCast(dungeon.getPlayer()->getSprite());

		playSound("FootStepGeneric2.mp3");

		int n = 0, m = 0;

		switch (move) {
		case 'l': n = -4; m = 0; break;
		case 'r': n = 4; m = 0; break;
		case 'u': n = 0; m = -4; break;
		case 'd': n = 0; m = 4; break;
		}

		// The space 4 away from the direction the player is facing
		if (dungeon.withinBounds(x + n, y + m)) {

			cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

			// Get items, if any
			if (dungeon[(y + m)*cols + (x + n)].item) {

				// If it was a chest, open it
				if (dungeon[(y + m)*cols + (x + n)].object->isChest()) {
					std::shared_ptr<Chests> chest = std::dynamic_pointer_cast<Chests>(dungeon[(y + m)*cols + (x + n)].object);
					chest->open(dungeon);
					chest.reset();
				}
				// Otherwise bring it to the player
				else {
					int _x = x, _y = y;
					if (dungeon[_y*cols + _x].item)
						dungeon.itemHash(_x, _y);

					dungeon[(y + m)*cols + (x + n)].item = false;
					dungeon[_y*cols + _x].item = true;
					dungeon[_y*cols + _x].object = dungeon[(y + m)*cols + (x + n)].object;
					dungeon[(y + m)*cols + (x + n)].object.reset();
					dungeon[(y + m)*cols + (x + n)].object = nullptr;
					dungeon.removeSprite(dungeon.item_sprites, x + n, y + m);					
					dungeon.addSprite(dungeon.item_sprites, _x, _y, -1, dungeon[_y*cols + _x].object->getImageName());
				}
			}

			if (dungeon[(y + m)*cols + (x + n)].enemy) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1) {
					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
					dungeon.fight(x + n, y + m);
				}
			}
		}

		// Reset to one
		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		// 6 other spaces to check
		for (int i = 0; i < 3; i++) {

			// Threw it above or below
			if (n == 0) {

				cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x - 1, y + m, 2);
				dungeon.runSingleAnimation(frames, 120, x + 1, y + m, 2);

				if (dungeon.withinBounds(x - 1, y + m) && dungeon[(y + m)*cols + (x - 1)].enemy) {
					int pos = dungeon.findMonster(x - 1, y + m);
					if (pos != -1) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
						dungeon.fight(x - 1, y + m);
					}
				}
				if (dungeon.withinBounds(x + 1, y + m) && dungeon[(y + m)*cols + (x + 1)].enemy) {
					int pos = dungeon.findMonster(x + 1, y + m);
					if (pos != -1) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
						dungeon.fight(x + 1, y + m);
					}
				}
			}
			// Threw it to the sides
			else if (m == 0) {

				cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x + n, y - 1, 2);
				dungeon.runSingleAnimation(frames, 120, x + n, y + 1, 2);

				if (dungeon.withinBounds(x + n, y - 1) && dungeon[(y - 1)*cols + (x + n)].enemy) {
					int pos = dungeon.findMonster(x + n, y - 1);
					if (pos != -1) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
						dungeon.fight(x + n, y - 1);
					}
				}
				if (dungeon.withinBounds(x + n, y + 1) && dungeon[(y + 1)*cols + (x + n)].enemy) {
					int pos = dungeon.findMonster(x + n, y + 1);
					if (pos != -1) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
						dungeon.fight(x + n, y + 1);
					}
				}
			}

			switch (move) {
			case 'l': n--; m = 0; break;
			case 'r': n++; m = 0; break;
			case 'u': n = 0; m--; break;
			case 'd': n = 0; m++; break;
			}
		}
	}

	resetCharge();
	setCast(false); // Removes label from HUD

	playSound("Wind_Spell2.mp3");

	// effect to remove wind up effect
	untint(dungeon.getPlayer()->getSprite());

	moveUsed = true;
}



//	:::: SHIELDS ::::
Shield::Shield() : Objects("No shield") {

}
Shield::Shield(int x, int y, int defense, int durability, int coverage, std::string type, std::string image)
	: Objects(x, y, type, image), m_defense(defense), m_durability(durability), m_coverage(coverage) {
	m_max_durability = durability;
	
}


//						Shield(defense, durability, coverage, name)
WoodShield::WoodShield(int x, int y) : Shield(x, y, 2, 15, 1, WOOD_SHIELD, "Wood_Shield_48x48.png") {
	setDescription("A wood shield. Can block projectiles and close combat\n attacks, but be careful that it doesn't break.");
}

IronShield::IronShield(int x, int y) : Shield(x, y, 4, 40, 1, IRON_SHIELD, "Iron_Shield_48x48.png") {
	setDescription("A sturdier shield, capable of withstanding stronger blows.");
}

ThornedShield::ThornedShield(int x, int y) : Shield(x, y, 3, 40, 2, THORNED_SHIELD, "Thorned_Shield_48x48.png") {
	setAbility(true);

	setDescription("This prickly shield will surely give foes a piece of your mind.");
}
void ThornedShield::useAbility(Dungeon &dungeon, Actors &a) {
	// spiked sound effect?
	//

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();
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

	setDescription("Magical. Capable of freezing enemies with\n its strange yet impressive enchantment.");
}
void FrostShield::useAbility(Dungeon &dungeon, Actors &a) {
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();
	int ax = a.getPosX();
	int ay = a.getPosY();

	// if attacker is next to player, it freezes them
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
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();
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


//	:::: TRAPS ::::
Traps::Traps(int x, int y, std::string name, std::string image, int damage) : Objects(x, y, name, image), m_trapdmg(damage) {
	
}

void Traps::destroyTrap(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	if (actsAsWall())
		dungeon[y*cols + x].wall = false;

	// Remove any light sources, if could possibly provide light (like Braziers)
	if (isLightSource())
		dungeon.removeLightSource(x, y);

	// if there are NOT multiple traps on this location, then unflag this spot
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon[y*cols + x].trap = false;

	if (getSprite() != nullptr) {
		dungeon.queueRemoveSprite(getSprite());
		setSprite(nullptr);
	}

	setDestroyed(true);
}


//		BRAZIER
Brazier::Brazier(int x, int y) : Traps(x, y, BRAZIER, "Gold_Goblet_48x48.png", 0) {
	setWallFlag(true);
	setCanBeIgnited(true);
	setDestructible(true);
}
Brazier::~Brazier() {
	if (m_flame != nullptr)
		m_flame->removeFromParent();
}

void Brazier::trapAction(Dungeon &dungeon, Actors &a) {

	// If not lit or it was tipped, do nothing
	if (!m_lit || m_tipped)
		return;

	// Braziers can be tipped over to leave a 3x3 grid of Embers

	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	playSound("Fireblast_Spell2.mp3");

	dungeon.removeLightSource(x, y);
	m_flame->removeFromParent();
	m_flame = nullptr;
	m_tipped = true; // So you can't tip it over multiple times

	char move = dungeon.getPlayer()->facingDirection();

	int n = 0, m = 0;

	switch (move) {
	case 'l': n = -1; m = -1; getSprite()->setRotation(270); break;
	case 'r': n = 1; m = -1; getSprite()->setRotation(90); break;
	case 'u': n = -1; m = -1; getSprite()->setRotation(90); break;
	case 'd': n = -1; m = 1; getSprite()->setRotation(180); break;
	}

	// 3 rows/columns
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {

			if (dungeon.withinBounds(x + n, y + m)) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

				if (dungeon[(y + m)*cols + (x + n)].enemy) {
					int pos = dungeon.findMonster(x + n, y + m);
					if (pos != -1) {
						// Ignite them
						if (dungeon.getMonsters().at(pos)->canBeBurned())
							dungeon.giveAffliction(pos, std::make_shared<Burn>(5));						
					}
				}

				if (dungeon[(y + m)*cols + (x + n)].trap) {
					int pos = dungeon.findTrap(x + n, y + m);
					if (pos != -1) {
						if (dungeon.getTraps().at(pos)->canBeIgnited())
							dungeon.getTraps().at(pos)->ignite(dungeon);
					}
				}

				// Create an Ember here if there's no wall
				if (!dungeon[(y + m)*cols + (x + n)].wall) {
					std::shared_ptr<Traps> ember = std::make_shared<Ember>(x + n, y + m, 5 + randInt(8));
					dungeon.getTraps().push_back(ember);

					cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Fire%04d.png", 8);
					ember->setSprite(dungeon.runAnimationForever(frames, 24, x + n, y + m, 2));
					ember->getSprite()->setScale(0.75f * GLOBAL_SPRITE_SCALE);

					//ember->setSprite(dungeon.createSprite(x + n, y + m, -1, ember->getImageName()));

					dungeon[(y + m)*cols + (x + n)].trap = true;
					dungeon.addLightSource(x + n, y + m, 3);
				}
			}

			switch (move) {
			case 'l': m++; break;
			case 'r': m++; break;
			case 'u': n++; break;
			case 'd': n++; break;
			}

		}

		switch (move) {
		case 'l': n--; m = -1; break;
		case 'r': n++; m = -1; break;
		case 'u': n = -1; m--; break;
		case 'd': n = -1; m++; break;
		}
	}
}
void Brazier::ignite(Dungeon &dungeon) {

	// If already lit, do nothing
	if (m_lit)
		return;

	m_lit = true;

	dungeon.addLightSource(getPosX(), getPosY(), 6);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Fire%04d.png", 8);
	m_flame = dungeon.runAnimationForever(frames, 24, getPosX(), getPosY(), 2);
	m_flame->setScale(0.6f * GLOBAL_SPRITE_SCALE);

	float x = getPosX();
	float y = getPosY() - 0.35f;
	dungeon.queueMoveSprite(m_flame, x, y);
}


//		PIT
Pit::Pit(int x, int y) : Traps(x, y, PIT, "Pit_48x48.png", 1000) {
	setLethal(true);
}

void Pit::trapAction(Dungeon &dungeon, Actors &a) {
	if (a.isPlayer() && !a.isFlying()) {
		// play falling sound effect
		playSound("Female_Falling_Scream_License.mp3");

		// instant death from falling
		a.setHP(0);

		deathFade(a.getSprite());
	}
	else if (a.isMonster() && !a.isFlying()) {
		Monster &m = dynamic_cast<Monster&>(a);

		// play falling sound effect
		playMonsterDeathByPit(*dungeon.getPlayer(), m);

		// death animation
		deathFade(a.getSprite());

		m.setDestroyed(true);
	}
}

//		FALLING SPIKES
FallingSpike::FallingSpike(int x, int y, int speed) : Traps(x, y, FALLING_SPIKE, "CeilingSpike_48x48.png", 3), m_speed(speed) {
	setTemporary(true);
	setActive(true);
}
FallingSpike::~FallingSpike() {
	if (getSprite() != nullptr)
		getSprite()->removeFromParent();
}

void FallingSpike::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();
	int pos = dungeon.findTrap(x, y);

	int range = m_speed; // How many tiles it will travel
	int dist = 1; // Starting point

	bool wall = dungeon[(y + dist)*cols + (x)].wall;
	bool hero = dungeon[(y + dist)*cols + (x)].hero;

	while (!wall && dist <= range) {

		if (hero) {
			playSound("Spike_Hit.mp3");

			dungeon.damagePlayer(getDmg());

			dungeon.queueMoveSprite(getSprite(), x, y + dist);
			destroyTrap(dungeon);
			return;
		}

		dist++;

		wall = dungeon[(y + dist)*cols + (x)].wall;
		hero = dungeon[(y + dist)*cols + (x)].hero;
	}

	if (dist == 4)
		dist--;

	if (!dungeon[(y + dist)*cols + (x)].wall) {
		if (dungeon.countTrapNumber(x, y) <= 1)
			dungeon[y*cols + x].trap = false;

		setPosY(y + dist);
		dungeon.queueMoveSprite(getSprite(), x, y + dist);
		dungeon[(y + dist)*cols + x].trap = true;
	}
	else
		destroyTrap(dungeon);

}
void FallingSpike::trapAction(Dungeon &dungeon, Actors &a) {

	if (!a.isPlayer())
		return;

	playSound("Spike_Hit.mp3");
	dungeon.damagePlayer(getDmg());

	destroyTrap(dungeon);
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

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	// Is player
	if (a.isPlayer()) {
		// If not no flying or spike immunity, damage them
		if (!(a.isFlying() || dungeon.getPlayer()->spikeImmunity())) {
			playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY());

			dungeon.damagePlayer(getDmg());
		}
		else if (dungeon.getPlayer()->spikeImmunity()) {
			playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY()); // Replace with appropriate sound
		}

		return;
	}

	// If not flying
	if (!a.isFlying()) {
		playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY());

		a.setHP(a.getHP() - getDmg());
	}
}

//		SPIKE TRAPS
SpikeTrap::SpikeTrap() : Traps(1 + randInt(BOSSCOLS - 2), BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2), AUTOSPIKE_DEACTIVE, "Spiketrap_Deactive_48x48.png", 5), m_cyclespeed(3 + randInt(3)) {
	m_deactive = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Deactive_48x48.png");
	m_primed = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Primed_48x48.png");
	m_active = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Active_48x48.png");

	m_deactive->setScale(GLOBAL_SPRITE_SCALE);
	m_primed->setScale(GLOBAL_SPRITE_SCALE);
	m_active->setScale(GLOBAL_SPRITE_SCALE);

	setSprite(m_deactive);
	//setSpriteVisibility(true, false, false);
	setTemporary(false);
	setActive(true);
}
SpikeTrap::SpikeTrap(int x, int y, int speed) : Traps(x, y, AUTOSPIKE_DEACTIVE, "Spiketrap_Deactive_48x48.png", 3), m_cyclespeed(speed), m_countdown(speed) {
	m_deactive = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Deactive_48x48.png");
	m_primed = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Primed_48x48.png");
	m_active = cocos2d::Sprite::createWithSpriteFrameName("Spiketrap_Active_48x48.png");

	m_deactive->setScale(GLOBAL_SPRITE_SCALE);
	m_primed->setScale(GLOBAL_SPRITE_SCALE);
	m_active->setScale(GLOBAL_SPRITE_SCALE);

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

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

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
		playSound("Spiketrap_Primed.mp3", px, py, tx, ty);

		setSprite(m_primed);
		setSpriteVisibility(false, true, false);

		setCountdown(getCountdown() - 1);
	}
	// spiketrap is launched
	else {
		playSound("Spiketrap_Active.mp3", px, py, tx, ty);

		setSprite(m_active);
		setSpriteVisibility(false, false, true);

		// if they're standing on top of it and not flying, hit them
		if (ax == tx && ay == ty && !a.isFlying()) {

			int px = dungeon.getPlayer()->getPosX();
			int py = dungeon.getPlayer()->getPosY();

			// Is player
			if (a.isPlayer()) {
				// If no flying or spike immunity, damage them
				if (!(a.isFlying() || dungeon.getPlayer()->spikeImmunity())) {
					playSound("Spike_Hit.mp3", px, py, tx, ty);

					dungeon.damagePlayer(getDmg());
				}
				else if (dungeon.getPlayer()->spikeImmunity()) {
					playSound("Spike_Hit.mp3", px, py, tx, ty); // Replace with appropriate sound
				}

				return;
			}

			playSound("Spike_Hit.mp3");

			a.setHP(a.getHP() - getDmg());
		}
	}

	// assigns spike to traps vector for lighting purposes
	int pos = dungeon.findTrap(tx, ty);
	dungeon.getTraps().at(pos) = std::make_shared<SpikeTrap>(*this);
}
void SpikeTrap::trapAction(Dungeon &dungeon, Actors &a) {

	if (m_countdown == 0 && !a.isFlying()) {

		int px = dungeon.getPlayer()->getPosX();
		int py = dungeon.getPlayer()->getPosY();

		// Is player
		if (a.isPlayer()) {
			// If not no flying or spike immunity, damage them
			if (!(a.isFlying() || dungeon.getPlayer()->spikeImmunity())) {
				playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY());

				dungeon.damagePlayer(getDmg());
			}
			else if (dungeon.getPlayer()->spikeImmunity()) {
				playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY()); // Replace with appropriate sound
			}

			return;
		}
		
		playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY());

		a.setHP(a.getHP() - getDmg());
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

	m_deactive->setScale(GLOBAL_SPRITE_SCALE);
	m_primed->setScale(GLOBAL_SPRITE_SCALE);
	m_active->setScale(GLOBAL_SPRITE_SCALE);

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
		playSound("Spiketrap_Active.mp3");

		setSprite(m_active);
		setSpriteVisibility(false, false, true);

		//	check if player was still on top
		if (ax == tx && ay == ty && !a.isFlying()) {

			int px = dungeon.getPlayer()->getPosX();
			int py = dungeon.getPlayer()->getPosY();

			// Is player
			if (a.isPlayer()) {
				// If no flying or spike immunity, damage them
				if (!(a.isFlying() || dungeon.getPlayer()->spikeImmunity())) {
					playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY());

					dungeon.damagePlayer(getDmg());
				}
				else if (dungeon.getPlayer()->spikeImmunity()) {
					playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY()); // Replace with appropriate sound
				}

				return;
			}

			playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY());

			a.setHP(a.getHP() - getDmg());
		}

		toggleTrigger();
	}
	else {
		// else if the trap wasn't triggered, check if player is standing on top of it
		if (ax == tx && ay == ty) {
			toggleTrigger();

			// spiketrap is primed
			playSound("Spiketrap_Primed.mp3");

			setSprite(m_primed);
			setSpriteVisibility(false, true, false);
		}
		else {
			// retract spiketrap
			playSound("Spiketrap_Deactive.mp3");

			setSprite(m_deactive);
			setSpriteVisibility(true, false, false);
		}
	}
}
void TriggerSpike::trapAction(Dungeon &dungeon, Actors &a) {

	if (m_active->isVisible() && !a.isFlying()) {

		int px = dungeon.getPlayer()->getPosX();
		int py = dungeon.getPlayer()->getPosY();

		// Is player
		if (a.isPlayer()) {
			// If no flying or spike immunity, damage them
			if (!(a.isFlying() || dungeon.getPlayer()->spikeImmunity())) {
				playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY());

				dungeon.damagePlayer(getDmg());
			}
			else if (dungeon.getPlayer()->spikeImmunity()) {
				playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY()); // Replace with appropriate sound
			}

			return;
		}

		playSound("Spike_Hit.mp3", px, py, getPosX(), getPosY());

		a.setHP(a.getHP() - getDmg());
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
Puddle::Puddle(int x, int y, int turns) : Traps(x, y, PUDDLE, "Puddle.png", 0), m_turns(turns) {
	setTemporary(true);
	setDestructible(true);

	if (turns == -1)
		setActive(false);
	else
		setActive(true);
}
Puddle::Puddle(int x, int y, int turns, std::string name, std::string image) : Traps(x, y, name, image, 0), m_turns(turns) {
	setImageName(image);
	setTemporary(true);
	setDestructible(true);

	if (turns == -1)
		setActive(false);
	else
		setActive(true);
}

void Puddle::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// Does not dissipate
	if (m_turns == -1)
		return;

	if (m_turns > 0)
		m_turns--;
	else {
		dungeon[y*cols + x].trap = false;
		dungeon[y*cols + x].trap_name = "";
		dungeon.queueRemoveSprite(getSprite());
		setDestroyed(true);
	}
}
void Puddle::trapAction(Dungeon &dungeon, Actors &a) {

	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	// if actor is flying, return
	if (a.isFlying())
		return;

	int roll = 1 + randInt(100) + a.getLuck();

	playSound("Puddle_Splash.mp3", px, py, x, y);

	destroyTrap(dungeon);

	// if player was on fire, put it out
	if (a.isBurned()) {
		// play relief sound
		playSound("Relief_Female.mp3");

		a.setBurned(false);
		a.removeAffliction(BURN);
	}

	// failed the save roll
	if (roll < 40) {

		// if actor can be stunned and they aren't stunned already
		if (a.canBeStunned()) {
			// play slip sound effect
			playSound("Puddle_Slip.mp3", px, py, x, y);

			// turn sprite sideways
			a.getSprite()->setRotation(90);
			
			a.addAffliction(std::make_shared<Stun>());
		}

		// special happenings as a result of walking on the puddle
		specialAction(dungeon, a);

		// else if they're not stunnable, nothing happens
		return;
	}
}

PoisonPuddle::PoisonPuddle(int x, int y, int turns) : Puddle(x, y, turns, POISON_PUDDLE, "Poison_Puddle.png") {

}

void PoisonPuddle::specialAction(Dungeon &dungeon, Actors &a) {
	// if actor can be poisoned, attempt to poison
	if (a.canBePoisoned()) {

		// 40% chance to be poisoned
		if (1 + randInt(100) + a.getLuck() < 60) {
			// play poison sound effect
			//cocos2d::experimental::AudioEngine::play2d("Puddle_Slip.mp3", false, exp(-(abs(px - a.getPosX()) + abs(py - a.getPosY()))));

			a.addAffliction(std::make_shared<Poison>(5, 3, 1, 1));
		}
	}
}

//		FIREBAR
Firebar::Firebar(int x, int y, int rows) : Traps(x, y, FIREBAR, "Firebar_Totem_48x48.png", 5) {
	m_clockwise = randInt(2);
	m_angle = 1 + randInt(8);

	m_inner = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");
	m_outer = cocos2d::Sprite::createWithSpriteFrameName("Spinner_Buddy_48x48.png");

	m_inner->setScale(GLOBAL_SPRITE_SCALE);
	m_outer->setScale(GLOBAL_SPRITE_SCALE);

	m_innerFire = std::make_shared<Objects>();
	m_outerFire = std::make_shared<Objects>();

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

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

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

	if (playerWasHit(*dungeon.getPlayerVector().at(0))) {
		playSound("Fire3.mp3");

		dungeon.damagePlayer(getDmg());

		if (dungeon.getPlayer()->canBeBurned() && 1 + randInt(100) + dungeon.getPlayer()->getLuck() < 80)
			a.addAffliction(std::make_shared<Burn>());
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
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y - 2);
			break;
		case 2:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y - 2);
			break;
		case 3:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y - 2);
			break;
		case 4:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y);
			break;
		case 5:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y + 2);
			break;
		case 6:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y + 2);
			break;
		case 7:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y + 2);
			break;
		case 8:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y);
			break;
		}
	}
	else {
		switch (m_angle) {
		case 1:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y - 2);
			break;
		case 2:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y);
			break;
		case 3:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y + 2);
			break;
		case 4:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y + 2);
			break;
		case 5:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y + 2);
			break;
		case 6:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y);
			break;
		case 7:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y - 2);
			break;
		case 8:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y - 2);
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
		m_innerFire->setPosX(m_innerFire->getPosX() - 1);
		m_outerFire->setPosX(m_outerFire->getPosX() - 2);
		break;
	case 'r':
		m_innerFire->setPosX(m_innerFire->getPosX() + 1);
		m_outerFire->setPosX(m_outerFire->getPosX() + 2);
		break;
	case 'u':
		m_innerFire->setPosY(m_innerFire->getPosY() - 1);
		m_outerFire->setPosY(m_outerFire->getPosY() - 2);
		break;
	case 'd':
		m_innerFire->setPosY(m_innerFire->getPosY() + 1);
		m_outerFire->setPosY(m_outerFire->getPosY() + 2);
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
	if ((m_innerFire->getPosX() == a.getPosX() && m_innerFire->getPosY() == a.getPosY()) ||
		(m_outerFire->getPosX() == a.getPosX() && m_outerFire->getPosY() == a.getPosY()))
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

	m_inner->setScale(GLOBAL_SPRITE_SCALE);
	m_outer->setScale(GLOBAL_SPRITE_SCALE);
	m_innerMirror->setScale(GLOBAL_SPRITE_SCALE);
	m_outerMirror->setScale(GLOBAL_SPRITE_SCALE);

	m_innerFire = std::make_shared<Objects>();
	m_outerFire = std::make_shared<Objects>();
	m_innerFireMirror = std::make_shared<Objects>();
	m_outerFireMirror = std::make_shared<Objects>();

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

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

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

	if (playerWasHit(*dungeon.getPlayerVector().at(0))) {
		playSound("Fire3.mp3");

		dungeon.damagePlayer(getDmg());

		if (dungeon.getPlayer()->canBeBurned() && 1 + randInt(100) + dungeon.getPlayer()->getLuck() < 80)
			a.addAffliction(std::make_shared<Burn>());
	}
}

void DoubleFirebar::setInitialFirePosition(int x, int y, int rows) {
	if (isClockwise()) {
		switch (getAngle()) {
		case 1:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y - 2);

			m_innerFireMirror->setPosX(x + 1); m_innerFireMirror->setPosY(y + 1);
			m_outerFireMirror->setPosX(x + 2); m_outerFireMirror->setPosY(y + 2);
			break;
		case 2:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y - 2);

			m_innerFireMirror->setPosX(x); m_innerFireMirror->setPosY(y + 1);
			m_outerFireMirror->setPosX(x); m_outerFireMirror->setPosY(y + 2);
			break;
		case 3:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y - 2);

			m_innerFireMirror->setPosX(x - 1); m_innerFireMirror->setPosY(y + 1);
			m_outerFireMirror->setPosX(x - 2); m_outerFireMirror->setPosY(y + 2);
			break;
		case 4:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y);

			m_innerFireMirror->setPosX(x - 1); m_innerFireMirror->setPosY(y);
			m_outerFireMirror->setPosX(x - 2); m_outerFireMirror->setPosY(y);
			break;
		case 5:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y + 2);

			m_innerFireMirror->setPosX(x - 1); m_innerFireMirror->setPosY(y - 1);
			m_outerFireMirror->setPosX(x - 2); m_outerFireMirror->setPosY(y - 2);
			break;
		case 6:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y + 2);

			m_innerFireMirror->setPosX(x); m_innerFireMirror->setPosY(y - 1);
			m_outerFireMirror->setPosX(x); m_outerFireMirror->setPosY(y - 2);
			break;
		case 7:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y + 2);

			m_innerFireMirror->setPosX(x + 1); m_innerFireMirror->setPosY(y - 1);
			m_outerFireMirror->setPosX(x + 2); m_outerFireMirror->setPosY(y - 2);
			break;
		case 8:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y);

			m_innerFireMirror->setPosX(x + 1); m_innerFireMirror->setPosY(y);
			m_outerFireMirror->setPosX(x + 2); m_outerFireMirror->setPosY(y);
			break;
		}
	}
	else {
		switch (getAngle()) {
		case 1:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y - 2);

			m_innerFireMirror->setPosX(x - 1); m_innerFireMirror->setPosY(y + 1);
			m_outerFireMirror->setPosX(x - 2); m_outerFireMirror->setPosY(y + 2);
			break;
		case 2:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y);

			m_innerFireMirror->setPosX(x - 1); m_innerFireMirror->setPosY(y);
			m_outerFireMirror->setPosX(x - 2); m_outerFireMirror->setPosY(y);
			break;
		case 3:
			m_innerFire->setPosX(x + 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x + 2); m_outerFire->setPosY(y + 2);

			m_innerFireMirror->setPosX(x - 1); m_innerFireMirror->setPosY(y - 1);
			m_outerFireMirror->setPosX(x - 2); m_outerFireMirror->setPosY(y - 2);
			break;
		case 4:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y + 2);

			m_innerFireMirror->setPosX(x); m_innerFireMirror->setPosY(y - 1);
			m_outerFireMirror->setPosX(x); m_outerFireMirror->setPosY(y - 2);
			break;
		case 5:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y + 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y + 2);

			m_innerFireMirror->setPosX(x + 1); m_innerFireMirror->setPosY(y - 1);
			m_outerFireMirror->setPosX(x + 2); m_outerFireMirror->setPosY(y - 2);
			break;
		case 6:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y);

			m_innerFireMirror->setPosX(x + 1); m_innerFireMirror->setPosY(y);
			m_outerFireMirror->setPosX(x + 2); m_outerFireMirror->setPosY(y);
			break;
		case 7:
			m_innerFire->setPosX(x - 1); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x - 2); m_outerFire->setPosY(y - 2);

			m_innerFireMirror->setPosX(x + 1); m_innerFireMirror->setPosY(y + 1);
			m_outerFireMirror->setPosX(x + 2); m_outerFireMirror->setPosY(y + 2);
			break;
		case 8:
			m_innerFire->setPosX(x); m_innerFire->setPosY(y - 1);
			m_outerFire->setPosX(x); m_outerFire->setPosY(y - 2);

			m_innerFireMirror->setPosX(x); m_innerFireMirror->setPosY(y + 1);
			m_outerFireMirror->setPosX(x); m_outerFireMirror->setPosY(y + 2);
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
	if ((m_innerFire->getPosX() == a.getPosX() && m_innerFire->getPosY() == a.getPosY()) ||
		(m_outerFire->getPosX() == a.getPosX() && m_outerFire->getPosY() == a.getPosY()) ||
		(m_innerFireMirror->getPosX() == a.getPosX() && m_innerFireMirror->getPosY() == a.getPosY()) ||
		(m_outerFireMirror->getPosX() == a.getPosX() && m_outerFireMirror->getPosY() == a.getPosY()))
		return true;

	return false;
}
void DoubleFirebar::setFirePosition(char move) {
	switch (move) {
	case 'l':
		m_innerFire->setPosX(m_innerFire->getPosX() - 1);
		m_outerFire->setPosX(m_outerFire->getPosX() - 2);
		m_innerFireMirror->setPosX(m_innerFireMirror->getPosX() + 1);
		m_outerFireMirror->setPosX(m_outerFireMirror->getPosX() + 2);
		break;
	case 'r':
		m_innerFire->setPosX(m_innerFire->getPosX() + 1);
		m_outerFire->setPosX(m_outerFire->getPosX() + 2);
		m_innerFireMirror->setPosX(m_innerFireMirror->getPosX() - 1);
		m_outerFireMirror->setPosX(m_outerFireMirror->getPosX() - 2);
		break;
	case 'u':
		m_innerFire->setPosY(m_innerFire->getPosY() - 1);
		m_outerFire->setPosY(m_outerFire->getPosY() - 2);
		m_innerFireMirror->setPosY(m_innerFireMirror->getPosY() + 1);
		m_outerFireMirror->setPosY(m_outerFireMirror->getPosY() + 2);
		break;
	case 'd':
		m_innerFire->setPosY(m_innerFire->getPosY() + 1);
		m_outerFire->setPosY(m_outerFire->getPosY() + 2);
		m_innerFireMirror->setPosY(m_innerFireMirror->getPosY() - 1);
		m_outerFireMirror->setPosY(m_outerFireMirror->getPosY() - 2);
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
}

void Lava::trapAction(Dungeon &dungeon, Actors &a) {
	if (!a.isFlying()) {
		// lava sound
		if (a.isPlayer())
			playSound("Fire4.mp3");
	}

	// if not immune to lava or flying, then burn them
	if (!(a.lavaImmune() || a.isFlying())) {

		if (a.isPlayer())
			dungeon.damagePlayer(getDmg());
		else
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
Spring::Spring(int x, int y, bool trigger, char move) : Traps(x, y, SPRING, "Spring_Arrow_Left_48x48.png", 0) {
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
	setDestructible(true);

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
Spring::Spring(int x, int y, bool trigger, bool known, bool cardinal) : Traps(x, y, SPRING, "Spring_Arrow_Left_48x48.png", 0) {
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
	setDestructible(true);

	setImage();
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


	// check if player is on top and not flying or heavy
	if (!(tx == x && ty == y && (!a.isFlying() || !a.isHeavy()))) {
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
	if (a.isPlayer()) {
		if (!(wall || enemy)) {
			// play trigger sound effect
			cocos2d::experimental::AudioEngine::play2d("Spring_Bounce.mp3", false, (float)exp(-(abs(x - tx) + abs(y - ty)) / 2));

			dungeon[y*cols + x].hero = false;
			dungeon[(y + m)*cols + x + n].hero = true;
			a.setPosX(x + n); a.setPosY(y + m);

			// queue player move
			dungeon.queueMoveSprite(a.getSprite(), x + n, y + m);

			// check if there was a trap at this position
			if (dungeon[(y + m)*cols + x + n].trap) {
				dungeon.trapEncounter(x + n, y + m);
			}
		}
	}
	// else it was a monster
	else {
		if (!(wall || enemy || hero)) {
			// play trigger sound effect
			cocos2d::experimental::AudioEngine::play2d("Spring_Bounce.mp3", false, (float)pow((abs(dungeon.getPlayer()->getPosX() - tx) + abs(dungeon.getPlayer()->getPosY() - ty)), -1));

			dungeon[y*cols + x].enemy = false;
			dungeon[(y + m)*cols + x + n].enemy = true;
			a.setPosX(x + n); a.setPosY(y + m);

			dungeon.queueMoveSprite(a.getSprite(), x + n, y + m);

			Monster &monster = dynamic_cast<Monster&>(a);
			if (monster.isMultiSegmented())
				monster.moveSegments(dungeon, x, y);

			// check if there was a trap at this position
			if (dungeon[(y + m)*cols + x + n].trap) {
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
	if (a.isFlying() || a.isHeavy())
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

	wall = dungeon[(y + m)*cols + x + n].wall;
	enemy = dungeon[(y + m)*cols + x + n].enemy;
	hero = dungeon[(y + m)*cols + x + n].hero;

	// if space is free, move the actor there
	if (a.isPlayer()) {
		if (!(wall || enemy)) {
			// play trigger sound effect
			int px = dungeon.getPlayer()->getPosX();
			int py = dungeon.getPlayer()->getPosY();
			playSound("Spring_Bounce.mp3", px, py, getPosX(), getPosY());

			dungeon[y*cols + x].hero = false;
			dungeon[(y + m)*cols + x + n].hero = true;
			a.setPosX(x + n); a.setPosY(y + m);

			// queue player move
			dungeon.queueMoveSprite(a.getSprite(), x + n, y + m);

			// check if there was a trap at this position
			if (dungeon[(y + m)*cols + x + n].trap) {
				dungeon.trapEncounter(x + n, y + m);
			}
		}
	}
	// else it was a monster
	else {
		if (!(wall || enemy || hero)) {
			// play trigger sound effect
			int px = dungeon.getPlayer()->getPosX();
			int py = dungeon.getPlayer()->getPosY();
			playSound("Spring_Bounce.mp3", px, py, getPosX(), getPosY());

			dungeon[y*cols + x].enemy = false;
			dungeon[(y + m)*cols + x + n].enemy = true;
			a.setPosX(x + n); a.setPosY(y + m);

			dungeon.queueMoveSprite(a.getSprite(), x + n, y + m);

			Monster &monster = dynamic_cast<Monster&>(a);
			if (monster.isMultiSegmented())
				monster.moveSegments(dungeon, x, y);

			// check if there was a trap at this position
			if (dungeon[(y + m)*cols + x + n].trap) {
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
bool Spring::isOpposite(const Spring &other) const {
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
		if (pos != -1 && (dungeon.getTraps().at(pos)->getName() == SPRING || dungeon[(y + m)*cols + (x + n)].trap_name == SPRING)) {

			// while the two springs point toward each other, reroll this spring's direction
			std::shared_ptr<Spring> spring = std::dynamic_pointer_cast<Spring>(dungeon.getTraps().at(pos));
			while (spring->isOpposite(*this)) {

				switch (1 + randInt(8)) {
				case 1: dir = 'l'; break;
				case 2: dir = 'r'; break;
				case 3: dir = 'u'; break;
				case 4: dir = 'd'; break;
				case 5: dir = '1'; break;	// Q1
				case 6: dir = '2'; break;	// Q2
				case 7: dir = '3'; break;	// Q3
				case 8: dir = '4'; break;	// Q4
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
			if (ay == ty && tx - ax >= 1 && tx - ax <= m_range && !dungeon.wallCollision('x', ax, tx)) {
				// play trigger sound effect
				playSound("Turret_Trigger.mp3");

				std::string image = "Spring_Arrow_Left_Red_48x48.png";
				getSprite()->removeFromParent();
				setSprite(dungeon.createSprite(tx, ty, 1, image));

				m_triggered = true;
				return;
			}
			break;
		}
		case 'r': {
			// if turret and actor are on the same column, and the actor is within shooting range, and there aren't any walls in the way, get triggered
			if (ay == ty && ax - tx >= 1 && ax - tx <= m_range && !dungeon.wallCollision('x', ax, tx)) {
				// play trigger sound effect
				playSound("Turret_Trigger.mp3");

				std::string image = "Spring_Arrow_Right_Red_48x48.png";
				getSprite()->removeFromParent();
				setSprite(dungeon.createSprite(tx, ty, 1, image));

				m_triggered = true;
				return;
			}
			break;
		}
		case 'u': {
			// if turret and actor are on the same row, and the actor is within shooting range, and there aren't any walls in the way, get triggered
			if (ax == tx && ty - ay >= 1 && ty - ay <= m_range && !dungeon.wallCollision('y', ay, ty)) {
				// play trigger sound effect
				playSound("Turret_Trigger.mp3");

				std::string image = "Spring_Arrow_Up_Red_48x48.png";
				getSprite()->removeFromParent();
				setSprite(dungeon.createSprite(tx, ty, 1, image));

				m_triggered = true;
				return;
			}
			break;
		}
		case 'd': {
			// if turret and actor are on the same row, and the actor is within shooting range, and there aren't any walls in the way, get triggered
			if (ax == tx && ay - ty >= 1 && ay - ty <= m_range && !dungeon.wallCollision('y', ay, ty)) {
				// play trigger sound effect
				playSound("Turret_Trigger.mp3");

				std::string image = "Spring_Arrow_Down_Red_48x48.png";
				getSprite()->removeFromParent();
				setSprite(dungeon.createSprite(tx, ty, 1, image));

				m_triggered = true;
				return;
			}
			break;
		}
		}

		return;
	}

	// play shoot sound effect
	playSound("Gunshot1.mp3");

	// else if it is triggered, shoot in the proper direction
	switch (m_dir) {
	case 'l':
	case 'r':
	case 'u':
	case 'd':
		// if turret and actor are on the same row, and there aren't any walls in the way, shoot them
		checkLineOfFire(dungeon);
		break;
	}

	std::string image;
	switch (m_dir) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	}
	getSprite()->removeFromParent();
	setSprite(dungeon.createSprite(tx, ty, 1, image));

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
		wall = dungeon[ty*cols + tx].wall;
		enemy = dungeon[ty*cols + tx].enemy;
		hero = dungeon[ty*cols + tx].hero;
		while (!wall) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				int damage = getDmg();
				// if player was blocking and successfully shielded the shot, reduce the damage taken
				if (dungeon.getPlayer()->canBlock() && dungeon.getPlayer()->didBlock(getPosX(), getPosY())) {
					damage = std::max(0, dungeon.getPlayer()->blockedDamageReduction() - damage);
					dungeon.getPlayerVector()[0]->successfulBlock();
				}
				dungeon.damagePlayer(damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
				dungeon.damageMonster(pos, getDmg());

				return; // remove this if it's a piercing turret (laser)
			}
			tx--;
			wall = dungeon[ty*cols + tx].wall;
			enemy = dungeon[ty*cols + tx].enemy;
			hero = dungeon[ty*cols + tx].hero;
		}
		break;
	}
	case 'r': {
		tx++;
		wall = dungeon[ty*cols + tx].wall;
		enemy = dungeon[ty*cols + tx].enemy;
		hero = dungeon[ty*cols + tx].hero;
		while (!wall) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				int damage = getDmg();
				// if player was blocking and successfully shielded the shot, reduce the damage taken
				if (dungeon.getPlayer()->canBlock() && dungeon.getPlayer()->didBlock(getPosX(), getPosY())) {
					damage = std::max(0, dungeon.getPlayer()->blockedDamageReduction() - damage);
					dungeon.getPlayerVector()[0]->successfulBlock();
				}
				dungeon.damagePlayer(damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
				dungeon.damageMonster(pos, getDmg());

				return; // remove this if it's a piercing turret (laser)
			}
			tx++;
			wall = dungeon[ty*cols + tx].wall;
			enemy = dungeon[ty*cols + tx].enemy;
			hero = dungeon[ty*cols + tx].hero;
		}
		break;
	}
	case 'u': {
		ty--;
		wall = dungeon[ty*cols + tx].wall;
		enemy = dungeon[ty*cols + tx].enemy;
		hero = dungeon[ty*cols + tx].hero;
		while (!wall) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				int damage = getDmg();
				// if player was blocking and successfully shielded the shot, reduce the damage taken
				if (dungeon.getPlayer()->canBlock() && dungeon.getPlayer()->didBlock(getPosX(), getPosY())) {
					damage = std::max(0, dungeon.getPlayer()->blockedDamageReduction() - damage);
					dungeon.getPlayerVector()[0]->successfulBlock();
				}
				dungeon.damagePlayer(damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
				dungeon.damageMonster(pos, getDmg());

				return; // remove this if it's a piercing turret (laser)
			}
			ty--;
			wall = dungeon[ty*cols + tx].wall;
			enemy = dungeon[ty*cols + tx].enemy;
			hero = dungeon[ty*cols + tx].hero;
		}
		break;
	}
	case 'd': {
		ty++;
		wall = dungeon[ty*cols + tx].wall;
		enemy = dungeon[ty*cols + tx].enemy;
		hero = dungeon[ty*cols + tx].hero;
		while (!wall) {
			// if turret and actor are on the same column, and there aren't any walls in the way, shoot them
			if (hero) {
				int damage = getDmg();
				// if player was blocking and successfully shielded the shot, reduce the damage taken
				if (dungeon.getPlayer()->canBlock() && dungeon.getPlayer()->didBlock(getPosX(), getPosY())) {
					damage = std::max(0, dungeon.getPlayer()->blockedDamageReduction() - damage);
					dungeon.getPlayerVector()[0]->successfulBlock();
				}
				dungeon.damagePlayer(damage);

				return;
			}
			if (enemy) {
				int pos = dungeon.findMonster(tx, ty);
				dungeon.damageMonster(pos, getDmg());

				return; // remove this if it's a piercing turret (laser)
			}
			ty++;
			wall = dungeon[ty*cols + tx].wall;
			enemy = dungeon[ty*cols + tx].enemy;
			hero = dungeon[ty*cols + tx].hero;
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

		if (m_counter == 0) {
			resetCounter();
			flip();
		}

		wall = dungeon[ty*cols + tx + m_dir].wall;
		enemy = dungeon[ty*cols + tx + m_dir].enemy;
		hero = dungeon[ty*cols + tx + m_dir].hero;

		if (wall) {
			resetCounter();
			flip();
			return;
		}

		if (dungeon[ty*cols + (tx + m_dir + m_dir)].wall) {

			if (hero) {
				playBoneCrunch();

				dungeon.damagePlayer(1000);
			}

			if (enemy) {
				int pos = dungeon.findMonster(tx + m_dir, ty);
				if (pos != -1) {
					playSound("Bone_Crack1.mp3", dungeon.getPlayer()->getPosX(), dungeon.getPlayer()->getPosY(), tx, ty);

					dungeon.getMonsters().at(pos)->setDestroyed(true);
				}
			}
		}

		else if (enemy || hero) {
			char move = m_dir == 1 ? 'r' : 'l';
			dungeon.linearActorPush(tx + m_dir, ty, 1, move, false, true);
		}

		setPosX(tx + m_dir);
		dungeon[ty*cols + tx].wall = false;
		dungeon[ty*cols + tx + m_dir].wall = true;
		dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY());
		setCounter(getCounter() - 1);

		break;
	}
	case 'v': {

		if (m_counter == 0) {
			resetCounter();
			flip();
		}

		wall = dungeon[(ty + m_dir)*cols + tx].wall;
		enemy = dungeon[(ty + m_dir)*cols + tx].enemy;
		hero = dungeon[(ty + m_dir)*cols + tx].hero;

		if (wall) {
			resetCounter();
			flip();
			return;
		}

		if (dungeon[(ty + m_dir + m_dir)*cols + tx].wall) {

			if (hero) {
				playBoneCrunch();

				dungeon.damagePlayer(1000);
			}

			if (enemy) {
				int pos = dungeon.findMonster(tx, ty + m_dir);
				if (pos != -1) {
					playSound("Bone_Crack1.mp3", dungeon.getPlayer()->getPosX(), dungeon.getPlayer()->getPosY(), tx, ty);

					dungeon.getMonsters().at(pos)->setDestroyed(true);
				}
			}
		}

		else if (enemy || hero) {
			char move = m_dir == 1 ? 'd' : 'u';
			dungeon.linearActorPush(tx, ty + m_dir, 1, move, false, true);
		}	

		setPosY(ty + m_dir);
		dungeon[(ty)*cols + tx].wall = false;
		dungeon[(ty + m_dir)*cols + tx].wall = true;
		dungeon.queueMoveSprite(getSprite(), getPosX(), getPosY());
		setCounter(getCounter() - 1);

		break;
	}
	}
}

//		ACTIVE BOMB
ActiveBomb::ActiveBomb(int x, int y, int timer) : Traps(x, y, ACTIVE_BOMB, "Bomb_48x48.png", 10), m_timer(timer) {
	m_fuseID = playSoundWithID("Bomb_Fuse2.mp3");
}
ActiveBomb::ActiveBomb(int x, int y, std::string type, std::string image, int damage, int timer) : Traps(x, y, type, image, damage), m_timer(timer) {
	m_fuseID = playSoundWithID("Bomb_Fuse2.mp3");
}

void ActiveBomb::activeTrapAction(Dungeon &dungeon, Actors &a) {
	if (getTimer() > 0) {
		setTimer(getTimer() - 1);
	}
	else {
		// play explosion sound effect
		playSound("Explosion.mp3");

		explosion(dungeon, a);

		destroyTrap(dungeon);
	}
}
void ActiveBomb::explosion(Dungeon &dungeon, Actors &a) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();
	
	int mx, my;

	// stop fuse sound
	cocos2d::experimental::AudioEngine::stop(m_fuseID);

	// explosion animation
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("Explosion%04d.png", 8);
	dungeon.runSingleAnimation(frames, 24, x, y, 2);

	// check if player was hit or if there is anything to destroy
	if (getName() == ACTIVE_MEGA_BOMB) {
		// flash floor tiles
		flashFloor(dungeon, x, y, true);

		//	if player is caught in the explosion and is not immune to explosions
		if (abs(px - x) <= 2 && abs(py - y) <= 2 && !dungeon.getPlayer()->explosionImmune()) {
			dungeon.damagePlayer(20);

			if (dungeon.getPlayerVector()[0]->getHP() <= 0)
				dungeon.getPlayerVector()[0]->setDeath(getName());
		}

		//	destroy stuff
		for (int j = y - 2; j < y + 3; j++) {
			for (int k = x - 2; k < x + 3; k++) {
				if (j != -1 && j != maxrows && !(k == -1 && j <= 0) && !(k == maxcols && j >= maxrows - 1)) { // boundary check

					// destroy any walls in the way
					if (dungeon[j*maxcols + k].wall_type == REG_WALL) {
						dungeon[j*maxcols + k].wall = false;

						// call remove sprite
						dungeon.removeSprite(dungeon.wall_sprites, k, j);
					}

					// destroy any gold in the way
					if (dungeon[j*maxcols + k].gold != 0) {
						dungeon[j*maxcols + k].gold = 0;

						dungeon.removeSprite(dungeon.money_sprites, k, j);
					}

					// Don't destroy the exit!!
					if (dungeon[j*maxcols + k].exit) {
						continue;
					}

					// destroy any destructible traps
					if (dungeon[j*maxcols + k].trap) {
						int n = dungeon.findTrap(k, j);

						if (n != -1) {
							if (dungeon.getTraps().at(n)->isDestructible()) {
								dungeon.getTraps().at(n)->destroyTrap(dungeon);
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

		//	if player is caught in the explosion and not immune to explosions
		if (abs(px - x) <= 1 && abs(py - y) <= 1 && !dungeon.getPlayer()->explosionImmune()) {
			dungeon.damagePlayer(10);

			if (dungeon.getPlayer()->getHP() <= 0)
				dungeon.getPlayerVector()[0]->setDeath(getName());
		}

		//	destroy stuff
		for (int j = y - 1; j < y + 2; j++) {
			for (int k = x - 1; k < x + 2; k++) {

				// destroy walls
				if (dungeon[j*maxcols + k].wall_type == REG_WALL) {
					dungeon[j*maxcols + k].wall = false;

					// call remove sprite twice to remove top and bottom walls
					dungeon.removeSprite(dungeon.wall_sprites, k, j);
				}

				// destroy any gold
				if (dungeon[j*maxcols + k].gold != 0) {
					dungeon[j*maxcols + k].gold = 0;

					dungeon.removeSprite(dungeon.money_sprites, k, j);
				}

				// Don't destroy the exit!!
				if (dungeon[j*maxcols + k].exit) {
					continue;
				}

				// destroy any destructible traps
				if (dungeon[j*maxcols + k].trap) {
					int n = dungeon.findTrap(k, j);

					if (n != -1) {
						if (dungeon.getTraps().at(n)->isDestructible()) {
							dungeon.getTraps().at(n)->destroyTrap(dungeon);
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

		if (getName() == ACTIVE_MEGA_BOMB) {
			if (abs(mx - x) <= 2 && abs(my - y) <= 2) {
				int damage = 20 + (dungeon.getPlayer()->explosionImmune() ? 10 : 0);
				dungeon.damageMonster(i, damage);

				if (dungeon.getMonsters().at(i)->getHP() <= 0) {
					dungeon.monsterDeath(i);
				}
			}
		}
		// else is a regular bomb
		else if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			int damage = 10 + (dungeon.getPlayer()->explosionImmune() ? 5 : 0);
			dungeon.damageMonster(i, damage);

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
		playSound("Mega_Explosion.mp3");

		explosion(dungeon, a);

		destroyTrap(dungeon);
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
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	if (!m_triggered)
		return;
	

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

		setDestroyed(true);
		return;
	}
	m_turns--;
}
void PoisonBomb::trapAction(Dungeon &dungeon, Actors &a) {

	if (!m_set) {
		m_set = true;
		return;
	}

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
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	// poison sound effect
	playSound("Poison_Bomb_Explosion.mp3");


	// if bomb was set off by Actor a (bombs can set this off too)
	if (ax == x && ay == y) {
		// the enemy that sets the bomb off receives the most damage
		if (a.isPlayer())
			dungeon.damagePlayer(getDmg());
		else
			a.setHP(a.getHP() - getDmg());

		if (a.canBePoisoned()) {
			a.addAffliction(std::make_shared<Poison>(8, 4, 1, 1));
		}
	}

	radiusCheck(dungeon, a);
	m_blown = true; // flag so that damage is not repeatedly dealt after initial explosion

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
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();


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
						dungeon.getPlayerVector()[0]->setHP(dungeon.getPlayer()->getHP() - (getDmg() - damageReduction));

						if (dungeon.getPlayer()->canBePoisoned()) {
							dungeon.getPlayerVector()[0]->addAffliction(std::make_shared<Poison>(8, 4, 1, 1));
						}
					}
					// otherwise add a weaker poison
					else if (dungeon.getPlayer()->canBePoisoned()) {
						dungeon.getPlayerVector()[0]->addAffliction(std::make_shared<Poison>(4, 4, 1, 1));
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
								dungeon.damageMonster(pos, getDmg() - damageReduction);

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
SetBearTrap::SetBearTrap(int x, int y) : Traps(x, y, BEAR_TRAP, "Blue_Toy_32x32.png", 4) {
	
}

void SetBearTrap::trapAction(Dungeon &dungeon, Actors &a) {

	if (!m_set) {
		m_set = true;
		return;
	}

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
	if (ax == x && ay == y) {
		// metal close sound effect
		playSound("Metal_Hit8.mp3", dungeon.getPlayer()->getPosX(), dungeon.getPlayer()->getPosY(), x, y);

		a.setHP(a.getHP() - getDmg());
		a.addAffliction(std::make_shared<Bleed>(5));
		a.addAffliction(std::make_shared<Cripple>(15));

		destroyTrap(dungeon);
	}
}

//		CRUMBLE FLOOR
CrumbleFloor::CrumbleFloor(int x, int y, int strength) : Traps(x, y, CRUMBLE_FLOOR, "Crumble_Floor1_48x48.png", 0), m_strength(strength) {

}
CrumbleFloor::CrumbleFloor(int x, int y, int strength, std::string name, std::string image) : Traps(x, y, name, image, 0), m_strength(strength) {

}

void CrumbleFloor::activeTrapAction(Dungeon &dungeon, Actors &a) {
	//int ax = a.getPosX();
	//int ay = a.getPosY();
	int x = getPosX();
	int y = getPosY();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	if (!m_crossed || m_triggerer == nullptr)
		return;

	int tx = m_triggerer->getPosX();
	int ty = m_triggerer->getPosY();
	
	if (m_crossed && !(tx == x && ty == y)) {
		m_crossed = false;
		m_strength--;
		m_triggerer = nullptr;

		// crumbling sound effect
		playCrumble(exp(-(abs(px - x) + abs(py - y))));

		// if strength is exhausted, this trap is replaced by a pit
		if (m_strength == 0 && !(tx == x && ty == y)) {
			// play fade animation
			fadeOut(getSprite());

			// breaking & crumbling sound effect
			playSound("Crumble_Breaking.mp3", px, py, getPosX(), getPosY());

			setDestroyed(true);

			if (this->getName() == CRUMBLE_FLOOR) {
				std::shared_ptr<Traps> pit = std::make_shared<Pit>(x, y);
				pit->setSprite(dungeon.createSprite(x, y, -4, pit->getImageName()));
				dungeon.getTraps().push_back(pit);
			}
			else if (this->getName() == CRUMBLE_LAVA) {
				std::shared_ptr<Traps> lava = std::make_shared<Lava>(x, y);
				lava->setSprite(dungeon.createSprite(x, y, -4, lava->getImageName()));
				dungeon.getTraps().push_back(lava);
			}
		}
		else if (m_strength == 1 && !(tx == x && ty == y)) {
			getSprite()->removeFromParent();
			setSprite(dungeon.createSprite(x, y, -4, "Crumble_Floor3_48x48.png"));
		}
		else if (m_strength == 2 && !(tx == x && ty == y)) {
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
		//activeTrapAction(dungeon, a);
		return;
	}

	// if player just walked on it, then flag it for next turn
	if (ax == x && ay == y && !a.isFlying()) {
		m_crossed = true;
		m_triggerer = &a;
	}
}


CrumbleLava::CrumbleLava(int x, int y, int strength) : CrumbleFloor(x, y, strength, CRUMBLE_LAVA, "Crumble_Floor1_48x48.png") {

}

//		EMBER
Ember::Ember(int x, int y, int turns) : Traps(x, y, EMBER, "Spinner_Buddy_48x48.png", 2), m_turns(turns) {

}
Ember::~Ember() {
	if (getSprite() != nullptr)
		getSprite()->removeFromParent();
}

void Ember::activeTrapAction(Dungeon &dungeon, Actors &a) {
	if (isDestroyed())
		return;

	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	if (dungeon[y*cols + x].enemy) {
		playSound("Fire3.mp3", px, py, x, y);

		int pos = dungeon.findMonster(x, y);
		if (pos != -1) {
			// 50% chance to burn
			if (dungeon.getMonsters().at(pos)->canBeBurned() && 1 + randInt(100) + dungeon.getPlayer()->getLuck() > 50)
				dungeon.giveAffliction(pos, std::make_shared<Burn>(4));		
		}

		// if there are NOT multiple traps on this location, then unflag this spot
		if (dungeon.countTrapNumber(x, y) <= 1)
			dungeon[y*cols + x].trap = false;

		dungeon.removeLightSource(x, y);
		setDestroyed(true);
	}

	if (m_turns > 0) {
		m_turns--;
		return;
	}

	destroyTrap(dungeon);
}
void Ember::trapAction(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int ax = a.getPosX();
	int ay = a.getPosY();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	if (isDestroyed())
		return;

	if (ax == x && ay == y) {
		playSound("Fire3.mp3", px, py, x, y);

		// 50% chance to burn
		if (a.canBeBurned()) {
			a.setHP(a.getHP() - getDmg());

			// If it's the player, then it's less chance to be burned
			int luck = dungeon.getPlayer()->getLuck() * (a.isPlayer() ? -1 : 1);
			if (1 + randInt(100) + luck > 50)
				a.addAffliction(std::make_shared<Burn>(4));
		}

		destroyTrap(dungeon);
	}
}

//		WEB
Web::Web(int x, int y, int stickiness) : Traps(x, y, WEB, "Spider_Web.png", 0), m_stickiness(stickiness) {
	setDestructible(true);
	setCanBeIgnited(true);
}

void Web::trapAction(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int ax = a.getPosX();
	int ay = a.getPosY();

	if (isDestroyed())
		return;

	if (a.isPlayer() && abs(ax - x) + abs(ay - y) == 0) {
		playSound("Grass2.mp3");

		// Stun actor for one turn
		if (a.canBeStunned())
			a.addAffliction(std::make_shared<Stun>(m_stickiness));
		
		destroyTrap(dungeon);
	}
}
void Web::ignite(Dungeon &dungeon) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	if (isDestroyed())
		return;

	destroyTrap(dungeon);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
	dungeon.runSingleAnimation(frames, 120, x, y, 2);

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (dungeon[j*cols + i].trap) {
				int pos = dungeon.findTrap(i, j);
				if (pos != -1) {
					if (dungeon.getTraps().at(pos)->canBeIgnited()) {
						cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
						dungeon.runSingleAnimation(frames, 120, i, j, 2);

						dungeon.getTraps().at(pos)->ignite(dungeon);
					}
				}
			}

			if (dungeon[j*cols + i].enemy) {
				int pos = dungeon.findMonster(i, j);
				if (pos != -1) {
					if (dungeon.getMonsters().at(pos)->canBeBurned()) {
						cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
						dungeon.runSingleAnimation(frames, 120, i, j, 2);

						dungeon.giveAffliction(pos, std::make_shared<Burn>(8));
					}
				}
			}
		}
	}

	// Create an ember where it was
	std::shared_ptr<Traps> ember = std::make_shared<Ember>(x, y, 2 + randInt(3));
	dungeon.getTraps().push_back(ember);

	frames = dungeon.getAnimation("Fire%04d.png", 8);
	ember->setSprite(dungeon.runAnimationForever(frames, 24, x, y, 2));
	ember->getSprite()->setScale(0.75f * GLOBAL_SPRITE_SCALE);
	//ember->setSprite(dungeon.createSprite(x, y, -1, ember->getImageName()));

	dungeon[(y)*cols + (x)].trap = true;
	dungeon.addLightSource(x, y, 3);
}

//		DECOY
Decoy::Decoy(int x, int y, int range, int turns, std::string name, std::string image) : Traps(x, y, name, image, 0), m_attractRange(range), m_turns(turns) {

}

//		ROTTING DECOY
RottingDecoy::RottingDecoy(int x, int y, int bites) : Decoy(x, y, bites, 8, ROTTING_DECOY, "Honeycomb_48x48.png") {

}

void RottingDecoy::activeTrapAction(Dungeon &dungeon, Actors &a) {

	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// Find any monsters adjacent and poison them
	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (isDestroyed())
				return;

			int pos = dungeon.findMonster(i, j);
			if (pos != -1) {
				trapAction(dungeon, *dungeon.getMonsters().at(pos));
			}
		}
	}
}
void RottingDecoy::trapAction(Dungeon &dungeon, Actors &a) {

	// Does nothing to the player
	if (a.isPlayer())
		return;

	if (a.canBePoisoned()) {
		a.addAffliction(std::make_shared<Poison>(2, 2, 1, 1));	
	}

	decTurns();
	if (getTurns() == 0) {
		destroyTrap(dungeon);
	}
}

//		FIRE PILLARS
FirePillars::FirePillars(int x, int y, int limit) : Traps(x, y, FIRE_PILLARS, "Fireblast_Spell_48x48.png", 0), m_limit(limit) {

}
void FirePillars::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	// m_ring value of -1 is to absorb one turn before acting since the first
	// ring of fire is launched when FireCascade is used.
	if (m_ring == -1) {
		m_ring++;
		return;
	}

	// If it traveled to the third ring, then stop
	if (m_ring > m_limit) {
		setDestroyed(true);
		return;
	}

	playSound("Fireblast_Spell1.mp3");

	// Find any monsters caught in the fire
	for (int i = x - (2 + m_ring); i < x + (3 + m_ring); i++) {
		for (int j = y - (2 + m_ring); j < y + (3 + m_ring); j++) {

			// Ignore all but the outer rings
			if (i != x - (2 + m_ring) && i != x + (3 + m_ring) - 1 && j != y - (2 + m_ring) && j != y + (3 + m_ring) - 1)
				continue;

			// Boundary check
			if (dungeon.withinBounds(i, j)) {

				if (!dungeon[j*cols + i].wall) {
					// fire explosion animation
					cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
					dungeon.runSingleAnimation(frames, 120, i, j, 2);
				}

				if (dungeon[j*cols + i].enemy) {

					int pos = dungeon.findMonster(i, j);
					if (pos != -1) {

						// If they can be burned
						if (dungeon.getMonsters().at(pos)->canBeBurned()) {
							dungeon.giveAffliction(pos, std::make_shared<Burn>(10 + dungeon.getPlayer()->getInt()));
						}
					}
				}

				if (dungeon[j*cols + i].trap) {

					int pos = dungeon.findTrap(i, j);
					if (pos != -1) {
						if (dungeon.getTraps().at(pos)->canBeIgnited())
							dungeon.getTraps().at(pos)->ignite(dungeon);
					}
				}
			}
		}
	}

	m_ring++;
}

//		ICE SHARDS
IceShards::IceShards(int x, int y, int limit) : Traps(x, y, ICE_SHARDS, "Sapphire_48x48.png", 4), m_limit(limit) {

}
void IceShards::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	if (m_shards == 0) {
		m_shards++;
		return;
	}

	if (m_shards > m_limit) {
		destroyTrap(dungeon);
		/*setDestroyed(true);
		dungeon.queueRemoveSprite(getSprite());*/
		return;
	}

	playSound("Freeze_Spell1.mp3");

	int n, m;
	bool wall, enemy;
	
	int k = 1; // Number of directions
	while (k <= 8) {

		switch (k) {
		case 1: n = -1; m = 0; break;
		case 2: n = 1; m = 0; break;
		case 3: n = 0; m = -1; break;
		case 4: n = 0; m = 1; break;
		case 5: n = -1; m = -1; break;
		case 6: n = 1; m = -1; break;
		case 7: n = -1; m = 1; break;
		case 8: n = 1; m = 1; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;

		while (!wall) {

			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

			if (enemy) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1) {

					// Attempt freeze
					if (dungeon.getMonsters().at(pos)->canBeFrozen()) {
						if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 40 - dungeon.getPlayer()->getInt())
							dungeon.giveAffliction(pos, std::make_shared<Freeze>(3 + dungeon.getPlayer()->getInt()));
					}
					
					dungeon.damageMonster(pos, getDmg() + dungeon.getPlayer()->getInt());
				}
				break;
			}

			switch (k) {
			case 1: n--; m = 0; break;
			case 2: n++; m = 0; break;
			case 3: n = 0; m--; break;
			case 4: n = 0; m++; break;
			case 5: n--; m--; break;
			case 6: n++; m--; break;
			case 7: n--; m++; break;
			case 8: n++; m++; break;
			}

			wall = dungeon[(y + m)*cols + (x + n)].wall;
			enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		}

		k++;
	}

	m_shards++;
}

//		HAIL STORM
HailStorm::HailStorm(int x, int y, char dir, int limit) : Traps(x, y, HAIL_STORM, "Sapphire_48x48.png", 4), m_dir(dir), m_limit(limit) {

}
void HailStorm::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();

	if (m_turns == 0) {
		m_turns++;
		return;
	}

	if (m_turns > m_limit) {
		setDestroyed(true);
		return;
	}

	playSound("Freeze_Spell1.mp3");

	int n, m;
	bool enemy;

	switch (m_dir) {
	case 'l': n = -1; m = -2; break;
	case 'r': n = 1; m = -2; break;
	case 'u': n = -2; m = -1; break;
	case 'd': n = -2; m = 1; break;
	}

	// 3 rows/columns
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 5; j++) {
			
			// 40% base chance to strike this spot
			if (dungeon.withinBounds(x + n, y + m) && 1 + randInt(100) + dungeon.getPlayer()->getLuck() > 40 - dungeon.getPlayer()->getInt()) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

				enemy = dungeon[(y + m)*cols + (x + n)].enemy;

				if (enemy) {
					int pos = dungeon.findMonster(x + n, y + m);
					if (pos != -1) {

						// Attempt freeze
						if (dungeon.getMonsters().at(pos)->canBeFrozen()) {
							if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 25 - dungeon.getPlayer()->getInt())
								dungeon.giveAffliction(pos, std::make_shared<Freeze>(3 + dungeon.getPlayer()->getInt()));
						}

						dungeon.damageMonster(pos, getDmg() + dungeon.getPlayer()->getInt());
					}
				}
			}

			switch (m_dir) {
			case 'l': m++; break;
			case 'r': m++; break;
			case 'u': n++; break;
			case 'd': n++; break;
			}

		}

		switch (m_dir) {
		case 'l': n--; m = -2; break;
		case 'r': n++; m = -2; break;
		case 'u': n = -2; m--; break;
		case 'd': n = -2; m++; break;
		}
	}

	m_turns++;
}

//		SHOCKWAVES
Shockwaves::Shockwaves(int x, int y, char dir, int limit) : Traps(x, y, SHOCKWAVES, "Sapphire_48x48.png", 6), m_dir(dir), m_limit(limit) {

}
void Shockwaves::activeTrapAction(Dungeon &dungeon, Actors &a) {
	
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	if (m_waves > m_limit) {
		setDestroyed(true);
		return;
	}

	int n, m;
	bool wall, enemy, trap;

	// These values determine how many tiles from the center line the ripple covers.
	// For instance, a value of 2 for the frontWave and 1 for the rearWave indicates
	// that the frontWave spans 5 tiles and the rearWave spans 3 tiles.
	int frontWave, rearWave;

	switch (m_waves) {
	case 1: m_waves++; return; // The first wave is activated when the Shockwave spell is first used
	case 2: frontWave = 1; rearWave = 0; break;
	case 3: frontWave = 1; rearWave = 1; break;
	case 4: frontWave = 2; rearWave = 1; break;
	case 5: frontWave = 2; rearWave = 2; break;
	default: frontWave = 2; rearWave = 1; break;
	}

	// Determines the row/column to act on
	switch (m_dir) {
	case 'l': n = -m_waves; m = -frontWave; break;
	case 'r': n = m_waves; m = -frontWave; break;
	case 'u': n = -frontWave; m = -m_waves; break;
	case 'd': n = -frontWave; m = m_waves; break;
	}

	// Front wave
	for (int i = 0; i < 2 * frontWave + 1; i++) {

		// Boundary check
		if (!dungeon.withinBounds(x + n, y + m))
			continue;

		//playSound("Earthquake_Spell2.mp3");

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		trap = dungeon[(y + m)*cols + (x + n)].trap;

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

		if (enemy) {
			int pos = dungeon.findMonster(x + n, y + m);
			if (pos != -1) {
				if (dungeon.getMonsters().at(pos)->canBeStunned())
					dungeon.giveAffliction(pos, std::make_shared<Stun>(2));
				
				dungeon.damageMonster(pos, getDmg());
			}
		}

		// 50% base chance to destroy walls
		if (wall && dungeon[(y + m)*cols + (x + n)].wall_type == REG_WALL) {
			if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 50 - dungeon.getPlayer()->getInt()) {
				dungeon[(y + m)*cols + (x + n)].wall = false;
				dungeon.removeSprite(dungeon.wall_sprites, x + n, y + m);
			}
		}

		// 50% base chance to destroy destructible traps
		if (trap) {
			int pos = dungeon.findTrap(x + n, y + m);

			if (pos != -1) {
				if (dungeon.getTraps().at(pos)->isDestructible()) {
					if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 50 - dungeon.getPlayer()->getInt()) {
						dungeon.getTraps().at(pos)->destroyTrap(dungeon);
					}
				}
				else if (dungeon.getTraps().at(pos)->isExplosive()) {
					std::shared_ptr<ActiveBomb> bomb = std::dynamic_pointer_cast<ActiveBomb>(dungeon.getTraps().at(pos));
					bomb->explosion(dungeon, a);
					bomb.reset();
				}
			}
		}

		switch (m_dir) {
		case 'l': m++; break;
		case 'r': m++; break;
		case 'u': n++; break;
		case 'd': n++; break;
		}

	}

	// Subtract one for the rear wave
	switch (m_dir) {
	case 'l': n = -(m_waves - 1); m = -rearWave; break;
	case 'r': n = m_waves - 1; m = -rearWave; break;
	case 'u': n = -rearWave; m = -(m_waves - 1); break;
	case 'd': n = -rearWave; m = m_waves - 1; break;
	}

	// Rear wave
	for (int i = 0; i < 2 * rearWave + 1; i++) {

		// Boundary check
		if (!dungeon.withinBounds(x + n, y + m))
			continue;

		playSound("Earthquake_Spell2.mp3");

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		trap = dungeon[(y + m)*cols + (x + n)].trap;

		if (enemy) {
			int pos = dungeon.findMonster(x + n, y + m);
			if (pos != -1) {
				if (dungeon.getMonsters().at(pos)->canBeStunned())
					dungeon.giveAffliction(pos, std::make_shared<Stun>(2));

				dungeon.damageMonster(pos, getDmg() / 2);
			}
		}

		// 50% base chance to destroy walls
		if (wall && dungeon[(y + m)*cols + (x + n)].wall_type == REG_WALL) {
			if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 50 - dungeon.getPlayer()->getInt()) {
				dungeon[(y + m)*cols + (x + n)].wall = false;
				dungeon.removeSprite(dungeon.wall_sprites, x + n, y + m);
			}
		}

		// 50% base chance to destroy destructible traps
		if (trap) {
			int pos = dungeon.findTrap(x + n, y + m);

			if (pos != -1) {
				if (dungeon.getTraps().at(pos)->isDestructible()) {
					if (1 + randInt(100) + dungeon.getPlayer()->getLuck() > 50 - dungeon.getPlayer()->getInt()) {
						dungeon.getTraps().at(pos)->destroyTrap(dungeon);
					}
				}
				else if (dungeon.getTraps().at(pos)->isExplosive()) {
					std::shared_ptr<ActiveBomb> bomb = std::dynamic_pointer_cast<ActiveBomb>(dungeon.getTraps().at(pos));
					bomb->explosion(dungeon, a);
					bomb.reset();
				}
			}
		}

		switch (m_dir) {
		case 'l': m++; break;
		case 'r': m++; break;
		case 'u': n++; break;
		case 'd': n++; break;
		}
	}

	m_waves++;
}

//		ROCK SUMMON
RockSummon::RockSummon(int x, int y) : Traps(x, y, ROCK_SUMMON, "Diamond_48x48.png", 8) {
	setWallFlag(true);
}
void RockSummon::trapAction(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	// Only the player can interact with this
	if (!a.isPlayer())
		return;

	bool wall, enemy, trap;
	char move = dungeon.getPlayer()->facingDirection();
	
	int n = 0, m = 0;
	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	wall = dungeon[(y + m)*cols + (x + n)].wall;
	enemy = dungeon[(y + m)*cols + (x + n)].enemy;
	trap = dungeon[(y + m)*cols + (x + n)].trap;

	while (!wall) {

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

		if (enemy) {
			playSound("Earthquake_Spell1.mp3");

			int pos = dungeon.findMonster(x + n, y + m);
			if (pos != -1) {
				if (dungeon.getMonsters().at(pos)->canBeStunned())
					dungeon.giveAffliction(pos, std::make_shared<Stun>(4));

				dungeon.damageMonster(pos, getDmg());

				dungeon[y*cols + x].wall = false; // Unmark its spawn location as a wall
				setDestroyed(true);
				dungeon.queueRemoveSprite(getSprite());
				return;
			}
		}

		switch (move) {
		case 'l': n--; m = 0; break;
		case 'r': n++; m = 0; break;
		case 'u': n = 0; m--; break;
		case 'd': n = 0; m++; break;
		}

		wall = dungeon[(y + m)*cols + (x + n)].wall;
		enemy = dungeon[(y + m)*cols + (x + n)].enemy;
		trap = dungeon[(y + m)*cols + (x + n)].trap;
	}
	
	playSound("Earthquake_Spell1.mp3");

	// fire explosion animation
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = dungeon.getAnimation("frame%04d.png", 63);
	dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);

	if (wall && dungeon[(y + m)*cols + (x + n)].wall_type == REG_WALL) {
		dungeon[(y + m)*cols + (x + n)].wall = false;
		dungeon.removeSprite(dungeon.wall_sprites, x + n, y + m);
	}
	else if (wall && trap) {
		int pos = dungeon.findTrap(x + n, y + m);
		if (pos != -1) {
			dungeon.getTraps().at(pos)->trapAction(dungeon, *dungeon.getPlayerVector().at(0));
		}
	}

	destroyTrap(dungeon);
}

//		WIND VORTEX
WindVortex::WindVortex(int x, int y, int limit) : Traps(x, y, WIND_VORTEX, "Emerald_48x48.png", 5), m_limit(limit) {

}
void WindVortex::activeTrapAction(Dungeon &dungeon, Actors &a) {
	int cols = dungeon.getCols();

	int x = getPosX();
	int y = getPosY();

	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();


	if (m_turns == 0) {
		m_turns++;
		return;
	}

	if (m_turns > m_limit) {
		destroyTrap(dungeon);
		return;
	}

	playSound("Wind_Spell1.mp3", px, py, x, y);
	playSound("Wind_Spell2.mp3", px, py, x, y);

	bool wall, enemy, hero;

	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {

			if (!dungeon.withinBounds(i, j))
				continue;

			wall = dungeon[j*cols + i].wall;
			enemy = dungeon[j*cols + i].enemy;
			hero = dungeon[j*cols + i].hero;

			// Inner ring
			if (abs(x - i) <= 1 && abs(y - j) <= 1) {
				if (enemy) {
					// Eye of the storm
					if (i == x && j == y) {
						int pos = dungeon.findMonster(i, j);
						if (pos != -1) {
							dungeon.damageMonster(pos, getDmg() + dungeon.getPlayer()->getInt());
						}
					}
					else {
						int pos = dungeon.findMonster(i, j);
						if (pos != -1) {
							if (dungeon.getMonsters().at(pos)->canBeStunned())
								dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
						}
					}
				}
				// Pull a new enemy into the center, if possible
				else if (!enemy && !hero && i == x && j == y) {
					bool stop = false;

					for (int p = x - 1; p < x + 2; p++) {
						for (int q = y - 1; q < y + 2; q++) {
							if (dungeon[q*cols + p].enemy) {
								int pos = dungeon.findMonster(p, q);
								if (pos != -1) {
									dungeon.getMonsters().at(pos)->moveTo(dungeon, x, y);
									if (dungeon[y*cols + x].trap)
										dungeon.singleMonsterTrapEncounter(pos);

									stop = true;
									break;
								}
							}
						}
						if (stop)
							break;
					}
				}
			}
			// Outer ring
			else {
				if (enemy) {
					char pullDirection;

					// Diagonals
					if (abs(x - i) + abs(y - j) == 2 || abs(x - i) + abs(y - j) == 4) {
						// Top-left
						if (i < x && j < y) {
							pullDirection = '1';
						}
						// Top-right
						else if (i > x && j < y) {
							pullDirection = '2';
						}
						// Bottom-left
						else if (i < x && j > y) {
							pullDirection = '3';
						}
						// Bottom-right
						else {
							pullDirection = '4';
						}
					}

					// Same column
					if (i == x) {
						// Above
						if (j < y)
							pullDirection = 'd';
						else
							pullDirection = 'u';
					}
					// Same row
					else if (j == y) {
						// Left
						if (i < x)
							pullDirection = 'r';
						else
							pullDirection = 'l';
					}
					// Top-left
					else if (i < x && j < y) {
						if (abs(i - x) == 2)
							pullDirection = 'r';
						else
							pullDirection = 'd';
					}
					// Top-right
					else if (i > x && j < y) {
						if (abs(i - x) == 2)
							pullDirection = 'l';
						else
							pullDirection = 'd';
					}
					// Bottom-left
					else if (i < x && j > y) {
						if (abs(i - x) == 2)
							pullDirection = 'r';
						else
							pullDirection = 'u';
					}
					// Bottom-right
					else {
						if (abs(i - x) == 2)
							pullDirection = 'l';
						else
							pullDirection = 'u';
					}

					dungeon.linearActorPush(i, j, 1, pullDirection, true);
				}
			}		
		}
	}

	m_turns++;
}