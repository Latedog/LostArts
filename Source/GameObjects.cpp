#include "cocos2d.h"
#include "AudioEngine.h"
#include "global.h"
#include "FX.h"
#include "GameUtils.h"
#include "GameObjects.h"
#include "Actors.h"
#include "Dungeon.h"
#include "Afflictions.h"
#include <string>
#include <vector>
#include <iostream>
#include <memory>


//		OBJECT FUNCTIONS
Objects::Objects() : m_x(0), m_y(0), m_name("") {

}
Objects::Objects(int x, int y, std::string name) : m_x(x), m_y(y), m_name(name) {

}
Objects::Objects(int x, int y, std::string name, std::string image) : m_x(x), m_y(y), m_name(name), m_image(image) {

}
Objects::~Objects() {
	
}

Coords Objects::getPos() const {
	return Coords(m_x, m_y);
}
void Objects::setPos(const Coords &pos) {
	m_x = pos.x;
	m_y = pos.y;
}

//		IDOL
Idol::Idol(int x, int y) : Objects(x, y, "Idol") {

}

//		GOLD
Gold::Gold(Dungeon &dungeon, int x, int y, std::string image) : Objects(x, y, GOLD) {
	setSprite(dungeon.createSprite(x, y, y + Z_MONEY, image));
}

//		STAIRS
Stairs::Stairs(Dungeon &dungeon, int x, int y, bool locked) : Traps(&dungeon, x, y, STAIRCASE, "Stairs_48x48.png", 0), m_locked(locked) {
	dungeon.tileAt(getPos()).exit = true;
	dungeon.tileAt(getPos()).trap = true;

	if (!locked)
		setSprite(dungeon.createSprite(x, y, y + Z_EXIT, getImageName()));
	else
		setSprite(dungeon.createSprite(x, y, y + Z_EXIT, "Locked_Stairs_48x48.png"));
}

void Stairs::interact() {
	if (m_locked) {
		playSound("Locked.mp3");
		return;
	}

	m_dungeon->advanceLevel();
}


//		WALLS
Wall::Wall(Dungeon *dungeon, int x, int y, std::string type) : Objects(x, y, type) {
	m_dungeon = dungeon;

	m_dungeon->tileAt(getPos()).wall = true;

	if (!isDoor())
		setSprite(m_dungeon->createWallSprite(x, y, y + Z_WALL, type));
}
void Wall::destroy() {
	int x = getPosX();
	int y = getPosY();

	Coords currentPos = getPos();

	m_dungeon->tileAt(currentPos).wall = false;
	m_dungeon->tileAt(currentPos).wallObject.reset();
	m_dungeon->tileAt(currentPos).wallObject = nullptr;

	m_dungeon->queueRemoveSprite(getSprite());
	m_dungeon->removeWall(x, y);
}

Boundary::Boundary(Dungeon *dungeon, int x, int y) : Wall(dungeon, x, y, UNB_WALL) {
	dungeon->tileAt(getPos()).boundary = true;
}

RegularWall::RegularWall(Dungeon *dungeon, int x, int y) : Wall(dungeon, x, y, REG_WALL) {

}

UnbreakableWall::UnbreakableWall(Dungeon *dungeon, int x, int y) : Wall(dungeon, x, y, UNB_WALL) {

}

Fountain::Fountain(Dungeon *dungeon, int x, int y) : Wall(dungeon, x, y, FOUNTAIN) {

}

//		DOORS
Door::Door(Dungeon *dungeon, int x, int y, std::string type) : Wall(dungeon, x, y, type), m_open(false), m_hold(true), m_lock(false) {
	
}
void Door::interact() {
	Coords currentPos = getPos();

	int x = getPosX();
	int y = getPosY();

	if (!(m_open || m_lock)) {
		playSound("Door_Opened.mp3");

		getSprite()->setVisible(false);

		toggleOpen();
		m_dungeon->tileAt(currentPos).wall = false;
	}
	else if (m_open) {
		m_dungeon->getPlayer()->moveTo(x, y);
	}
	else if (m_lock) {
		playSound("Locked.mp3");
	}
}

RegularDoor::RegularDoor(Dungeon *dungeon, int x, int y, std::string type) : Door(dungeon, x, y, type) {
	if (type == DOOR_VERTICAL) 
		setSprite(m_dungeon->createSprite(x, y, y + Z_WALL, "Door_Vertical_Closed_48x48.png"));	
	else if (type == DOOR_HORIZONTAL)
		setSprite(m_dungeon->createSprite(x, y, y + Z_WALL, "Door_Horizontal_Closed_48x48.png"));
}

BlastDoor::BlastDoor(Dungeon *dungeon, int x, int y, std::string type) : Door(dungeon, x, y, type) {
	if (type == DOOR_VERTICAL)
		setSprite(m_dungeon->createSprite(x, y, y + Z_WALL, "Door_Vertical_Closed_48x48.png"));
	else if (type == DOOR_HORIZONTAL)
		setSprite(m_dungeon->createSprite(x, y, y + Z_WALL, "Door_Horizontal_Closed_48x48.png"));
}

//		BUTTON
Button::Button(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, BUTTON_UNPRESSED, "Button_Unpressed_48x48.png", 0) {
	dungeon.tileAt(getPos()).trap = true;
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}


//	:::: DROPS ::::
Drops::Drops(int x, int y, std::string name, std::string image) : Objects(x, y, name, image) {
	
}

AutoUsable::AutoUsable(int x, int y, std::string name, std::string image) : Drops(x, y, name, image) {

}

HeartPod::HeartPod(int x, int y) : AutoUsable(x, y, HEART_POD, "Heart_Pod_48x48.png") {
	setSoundName("Heart_Pod_Pickup2.mp3");
}
void HeartPod::useItem(Dungeon &dungeon, Player &player) {
	player.increaseStatBy(StatType::HP, 1);
}

ShieldRepair::ShieldRepair(int x, int y) : AutoUsable(x, y, SHIELD_REPAIR, "Shield_Repair_48x48.png") {
	setSoundName("Armor_Pickup.mp3");
}
void ShieldRepair::useItem(Dungeon &dungeon, Player &player) {
	player.restoreActive(10);
}

Usable::Usable(int x, int y, std::string name, std::string image) : Drops(x, y, name, image) {

}

LifePotion::LifePotion(int x, int y) : Usable(x, y, LIFEPOT, "Life_Potion_48x48.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A red elixir which seems to keep well. Gives you life.");
}
void LifePotion::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	int healing = 15 + (dungeon.getPlayer()->hasPotentPotions() ? 8 : 0);
	player.increaseStatBy(StatType::HP, healing);

	used();
}

BigLifePotion::BigLifePotion(int x, int y) : Usable(x, y, BIG_LIFEPOT, "Big_Health_Potion_48x48.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("The alchemist's improved formula. Gives you more life.");
}
void BigLifePotion::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	int healing = 25 + (player.hasPotentPotions() ? 13 : 0);
	player.increaseStatBy(StatType::HP, healing);

	used();
}

SteadyLifePotion::SteadyLifePotion(int x, int y) : Usable(x, y, STEADY_LIFEPOT, "Wine_Bottle_48x48.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A slow, but powerful acting mixture to heal your wounds.");
}
void SteadyLifePotion::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	float duration = 15 + (player.hasPotentPotions() ? 10 : 0);
	float interval = 0.5f;
	player.addAffliction(std::make_shared<TimedHeal>(duration, interval));

	used();
}

HalfLifePotion::HalfLifePotion(int x, int y) : Usable(x, y, HALF_LIFE_POTION, "Star_Potion_32x32.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A powerful potion, but only restores up to half of\nyour maximum health.");
}
void HalfLifePotion::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	int healing = (player.getMaxHP() / 2) + (player.hasPotentPotions() ? 10 : 0);

	// If player hp is less than half their max, then restore it to the healing value
	if (player.getHP() <= player.getMaxHP() / 2) 
		player.setHP(healing);

	used();
}

SoulPotion::SoulPotion(int x, int y) : Usable(x, y, SOUL_POTION, "Gold_Coin_48x48.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Converts your currency into health. The mages of old\nknew of methods to trade greed for well-being.");
}
void SoulPotion::useItem(Dungeon &dungeon, Player &player) {
	// Every 50 gold = 5% of health refilled

	playSound("Life_Potion_Used.mp3");

	if (player.getHP() == player.getMaxHP())
		return;

	float healing = 0;

	int i;
	for (i = player.getMoney(); i >= 50; i -= 50) {
		healing += 0.05f;

		// Stop if this would heal the player fully already
		if (player.getHP() + player.getMaxHP() * healing >= player.getMaxHP())
			break;
	}

	int amount = player.getMaxHP() * healing;
	if (player.getHP() + amount < player.getMaxHP())
		player.setHP(player.getHP() + amount);
	else
		player.setHP(player.getMaxHP());

	player.setMoney(i);

	used();
}

BinaryLifePotion::BinaryLifePotion(int x, int y) : Usable(x, y, BINARY_LIFE_POTION, "Star_Potion_32x32.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A failed experiment led to this potion's creation.\nOnly restores half of your missing health.");
}
void BinaryLifePotion::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	int healing = (player.getMaxHP() - player.getHP()) / 2;
	player.increaseStatBy(StatType::HP, healing);

	used();
}

StatPotion::StatPotion(int x, int y) : Usable(x, y, STATPOT, "Stat_Potion_48x48.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Increases your combat abilities, but strangely it removes\n the taste from your mouth.");
}
void StatPotion::useItem(Dungeon &dungeon, Player &player) {
	playSound("Potion_Used.mp3");

	player.setMaxHP(player.getMaxHP() + 5);

	int turns = 30 + (player.hasPotentPotions() ? 20 : 0);
	int stats = 2 + (player.hasPotentPotions() ? 1 : 0);
	player.addAffliction(std::make_shared<Buff>(turns, stats, stats, stats));

	used();
}

RottenApple::RottenApple(int x, int y) : Usable(x, y, ROTTEN_APPLE, "Apple_Worm_48x48.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Uhh... it might not be the best idea to consume this.");
}
void RottenApple::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	player.setHP(player.getMaxHP());
	player.addAffliction(std::make_shared<Poison>(*dungeon.getPlayer(), 40, 5, 2, 2));

	used();
}

Antidote::Antidote(int x, int y) : Usable(x, y, ANTIDOTE, "Star_Potion_32x32.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Puts an end to what would otherwise be your end.\n Cures poison.");
}
void Antidote::useItem(Dungeon &dungeon, Player &player) {
	playSound("Relief_Female.mp3");

	if (player.isPoisoned())
		player.removeAffliction(POISON);

	used();
}

ArmorDrop::ArmorDrop(int x, int y) : Usable(x, y, ARMOR, "Armor_48x48.png") {	
	setSoundName("Armor_Pickup.mp3");

	setDescription("Just what a lost adventurer needs to help protect\nthemselves. A little more armor to keep you in one piece.");
}
void ArmorDrop::useItem(Dungeon &dungeon, Player &player) {
	playSound("Armor_Use.mp3");

	player.increaseStatBy(StatType::ARMOR, 1);

	used();
}

DizzyElixir::DizzyElixir(int x, int y) : Usable(x, y, DIZZY_ELIXIR, "Dizzy_Elixir_48x48.png") {	
	setSoundName("Potion_Pickup2.mp3");

	setDescription("A strange concoction that makes you feel good, but...");
}
void DizzyElixir::useItem(Dungeon &dungeon, Player &player) {
	playSound("Potion_Used.mp3");

	int dexBoost = 3 + (player.hasPotentPotions() ? 2 : 0);
	int turns = 20 + player.getInt();
	player.addAffliction(std::make_shared<Confusion>(turns));
	player.addAffliction(std::make_shared<Buff>(turns + 10, 0, dexBoost, 0));

	used();
}

PoisonBomb::PoisonBomb(int x, int y) : Usable(x, y, POISON_BOMB, "Poison_Bomb_48x48.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("WARNING: Do NOT ingest. Handle with care!");
}
void PoisonBomb::useItem(Dungeon &dungeon, Player &player) {
	playSound("Bomb_Placed.mp3");

	int px = player.getPosX();
	int py = player.getPosY();

	setPosX(px);
	setPosY(py);

	if (dungeon.trap(px, py)) {
		std::vector<int> indexes = dungeon.findTraps(px, py);
		for (size_t i = 0; i < indexes.size(); ++i)
			if (dungeon.trapAt(indexes.at(i))->canBePoisoned())
				dungeon.trapAt(indexes.at(i))->poison();
	}

	// add poison cloud bomb to the traps vector, as it only goes off when something walks over it
	dungeon.addTrap(std::make_shared<ActivePoisonBomb>(dungeon, getPosX(), getPosY()));

	used();
}

RottenMeat::RottenMeat(int x, int y) : Usable(x, y, ROTTEN_MEAT, "Honeycomb_48x48.png") {
	setSoundName("Potion_Pickup2.mp3");

	setDescription("Definitely do not eat this. The beasts of this\nplace will probably enjoy this more than you.");
}
void RottenMeat::useItem(Dungeon &dungeon, Player &player) {
	playSound("Bomb_Placed.mp3");

	setPosX(player.getPosX());
	setPosY(player.getPosY());

	std::shared_ptr<RottingDecoy> decoy = std::make_shared<RottingDecoy>(dungeon, getPosX(), getPosY());
	dungeon.addTrap(decoy);
	dungeon.addDecoy(decoy);

	used();
}

MattockDust::MattockDust(int x, int y) : Usable(x, y, MATTOCK_DUST, "Silver_Kettle_48x48.png") {
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Now you've done it.");
}
void MattockDust::useItem(Dungeon &dungeon, Player &player) {
	playSound("Bomb_Placed.mp3");

	int px = player.getPosX();
	int py = player.getPosY();

	for (int i = px - 1; i < px + 2; i++) {
		for (int j = py - 1; j < py + 2; j++) {

			if (i == px && j == py)
				continue;

			if (dungeon.enemy(i, j)) {
				int pos = dungeon.findMonster(i, j);
				if (pos != -1)
					dungeon.giveAffliction(pos, std::make_shared<Confusion>(5));
			}
		}
	}

	player.addAffliction(std::make_shared<Confusion>(5));

	used();
}

Teleport::Teleport(int x, int y) : Usable(x, y, TELEPORT, "Teleport_Scroll_48x48.png") {
	setSoundName("TurningPage3.mp3");

	setDescription("Wait. Where am I?");
}
void Teleport::useItem(Dungeon &dungeon, Player &player) {
	playSound("Teleport_Spell.mp3");

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	Coords playerCoords = player.getPos();

	int x = randInt(1, cols - 2);
	int y = randInt(1, rows - 2);

	while (dungeon.tileIsBlocked(x, y) || dungeon.trap(x, y)) {
		x = randInt(1, cols - 2);
		y = randInt(1, rows - 2);
	}

	Coords newPos(x, y);

	dungeon.tileAt(playerCoords).hero = false;
	dungeon.tileAt(newPos).hero = true;

	player.setPos(newPos);

	dungeon.teleportSprite(player.getSprite(), newPos, getHalfTileYOffset());

	// if player has enough intellect, give them invulnerability for a few turns
	if (player.getInt() >= 5)
		player.addAffliction(std::make_shared<Invulnerability>(3));

	used();
}

SmokeBomb::SmokeBomb(int x, int y) : Usable(x, y, SMOKE_BOMB, "Pumpkin_32x32.png") {
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Toss this to unleash a cloud of smoke.");
}
void SmokeBomb::useItem(Dungeon &dungeon, Player &player) {
	playSound("Fireblast_Spell1.mp3");

	char move = player.facingDirection();

	int x = player.getPosX();
	int y = player.getPosY();

	int n = 0, m = 0;
	setDirectionalOffsets(move, n, m);

	int range = 5;

	while (!(dungeon.wall(x + n, y + m) || dungeon.enemy(x + n, y + m)) && range > 0) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		incrementDirectionalOffsets(move, n, m);

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
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);

			if (dungeon.enemy(i, j)) {
				int pos = dungeon.findMonster(i, j);
				if (pos != -1)
					dungeon.giveAffliction(pos, std::make_shared<Possessed>(8));
			}
		}
	}

	used();
}

WildMushroom::WildMushroom(int x, int y) : Usable(x, y, WILD_MUSHROOM, "Cheese_Wedge_48x48.png") {
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("An unidentified mushroom. Unfortunately, your nature\nknow-how is quite weak, so it's uncertain if this is\nsafe to eat.");
}
void WildMushroom::useItem(Dungeon &dungeon, Player &player) {
	playSound("Bomb_Placed.mp3");

	if (randReal(1, 100) + player.getLuck() <= 50) {
		player.addAffliction(std::make_shared<Poison>(*dungeon.getPlayer(), 10, 5, 1, 1));
	}
	else {
		if (randReal(1, 100) + player.getLuck() > 95)
			player.increaseStatBy(StatType::STRENGTH, 1);	
		else
			player.increaseStatBy(StatType::HP, player.getMaxHP() * 0.1);		
	}

	used();
}

MagmaHeart::MagmaHeart(int x, int y) : Usable(x, y, MAGMA_HEART, "Cheese_Wedge_48x48.png") {
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("A strange object filled with molten energy. Its glow\nprovides you with a calming warmth.");
}
void MagmaHeart::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	player.setDelayedHealing(0.25f);

	used();
}

CactusWater::CactusWater(int x, int y) : Usable(x, y, CACTUS_WATER, "Cheese_Wedge_48x48.png") {	
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Prickly! Some much needed sustenance in this turbulent\nenvironment.");
}
void CactusWater::pickupEffect(Dungeon &dungeon) {
	dungeon.damagePlayer(2, DamageType::NORMAL);
}
void CactusWater::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	player.increaseStatBy(StatType::HP, player.getMaxHP() * 0.05f);
	player.increaseStatBy(StatType::STRENGTH, 1);

	used();
}

SuperRoot::SuperRoot(int x, int y) : Usable(x, y, SUPER_ROOT, "Cheese_Wedge_48x48.png") {	
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("This incredible root provides immunity to\nafflictions both mental and physical.");
}
void SuperRoot::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	float duration = 30;
	float interval = 1.0f;
	player.addAffliction(std::make_shared<AfflictionImmunity>(duration, interval));

	used();
}

RPGInABottle::RPGInABottle(int x, int y) : Usable(x, y, RPG_IN_A_BOTTLE, "Wine_Bottle_48x48.png") {	
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("A peculiar item. Scribbled on the side, it\nsays the user will gain \"experience\" for a short\ntime. Whatever that means.");
}
void RPGInABottle::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	float duration = 20;
	float interval = 1.0f;
	player.addAffliction(std::make_shared<ExperienceGain>(duration, interval));

	used();
}

MonsterTransform::MonsterTransform(int x, int y) : Usable(x, y, MONSTER_TRANSFORM, "Wine_Bottle_48x48.png") {
	setSoundName("Bomb_Pickup2.mp3");
}
void MonsterTransform::useItem(Dungeon &dungeon, Player &player) {
	playSound("Life_Potion_Used.mp3");

	Coords start = player.getPos();

	int radius = 4;
	int level = getMonsterLevel(dungeon.getLevel());

	for (int i = dungeon.monsterCount() - 1; i >= 0 ; --i) {
		Coords monsterPos = dungeon.monsterAt(i)->getPos();
		if (positionInRectangularRange(start, monsterPos, radius, radius)) {
			dungeon.monsterAt(i)->setDestroyed(true);
			dungeon.monsterDeath(i);
			dungeon.addMonster(rollMonster(level, &dungeon, monsterPos.x, monsterPos.y));
		}
	}

	used();
}
int MonsterTransform::getMonsterLevel(int currentLevel) const {
	switch (currentLevel) {
	case NINTH_FLOOR:
		if (randReal(1, 100) > 50)
			return NINTH_FLOOR;
	case EIGHTH_FLOOR:
		if (randReal(1, 100) > 50)
			return EIGHTH_FLOOR;
	case SEVENTH_FLOOR:
		if (randReal(1, 100) > 50)
			return SEVENTH_FLOOR;
	case SIXTH_FLOOR:
		if (randReal(1, 100) > 50)
			return SIXTH_FLOOR;
	case FIFTH_FLOOR:
		if (randReal(1, 100) > 50)
			return FIFTH_FLOOR;
	case FOURTH_FLOOR:
		if (randReal(1, 100) > 50)
			return FOURTH_FLOOR;
	case THIRD_FLOOR:
		if (randReal(1, 100) > 50)
			return THIRD_FLOOR;
	case SECOND_FLOOR:
		if (randReal(1, 100) > 50)
			return SECOND_FLOOR;
	case FIRST_FLOOR:
	default:
		return FIRST_FLOOR;
	}
}

SleddingScarf::SleddingScarf(int x, int y) : Usable(x, y, SLEDDING_SCARF, "Wine_Bottle_48x48.png") {

}
void SleddingScarf::useItem(Dungeon &dungeon, Player &player) {
	playSound("Bomb_Pickup2.mp3");

	dungeon.factoryTileCreation();

	used();
}

Banana::Banana(int x, int y) : Usable(x, y, BANANA, "Cheese_Wedge_48x48.png") {

}
void Banana::useItem(Dungeon &dungeon, Player &player) {
	playSound("Bomb_Pickup2.mp3");

	switch (m_uses) {
	case 0:
		// Change sprite to peeled banana
		// getSprite()->setSpriteFrame("");
		setName(PEELED_BANANA);

		++m_uses;
		return;
	case 1:
		// Change sprite to half eaten banana
		// getSprite()->setSpriteFrame("");
		setName(HALF_EATEN_BANANA);

		player.increaseStatBy(StatType::HP, player.getMaxHP() * 0.03f);
		++m_uses;
		return;
	case 2:
		// Change sprite to banana peel
		// getSprite()->setSpriteFrame("");
		setName(BANANA_PEEL);

		player.increaseStatBy(StatType::HP, player.getMaxHP() * 0.03f);
		player.addAffliction(std::make_shared<Buff>(50, 2, 0, 0));
		++m_uses;
		return;
	case 3:
		throwBananaPeel(dungeon, player);
		used();
		return;
	default: return;
	}
}
void Banana::throwBananaPeel(Dungeon &dungeon, Player &player) {
	char move = player.facingDirection();

	int x = player.getPosX();
	int y = player.getPosY();

	int n = 0, m = 0;
	setDirectionalOffsets(move, n, m);

	int range = 6;
	int currentRange = 0;

	while (currentRange < range) {
		++currentRange;

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		if (dungeon.tileIsBlocked(x + n, y + m)) {
			decrementDirectionalOffsets(move, n, m);
			break;
		}

		incrementDirectionalOffsets(move, n, m);
	}

	dungeon.addTrap(std::make_shared<BananaPeel>(dungeon, x + n, y + m));
}
void Banana::resetItemToOriginalState() {
	// Change sprite to banana
	//getSprite()->setSpriteFrame("");
	setName(BANANA);
	m_uses = 0;
	undoUse();
};

BananaBunch::BananaBunch(int x, int y) : Usable(x, y, BANANA_BUNCH, "Cheese_Wedge_48x48.png") {
	m_quantity = randInt(3, 5);
}
void BananaBunch::useItem(Dungeon &dungeon, Player &player) {
	if (m_quantity > 0) {
		playSound("Bomb_Pickup2.mp3");

		player.increaseStatBy(StatType::HP, player.getMaxHP() * 0.05f);
		player.addAffliction(std::make_shared<Buff>(50, 2, 0, 0));
		Banana::throwBananaPeel(dungeon, player);

		--m_quantity;
	}

	if (m_quantity == 0)
		used();
}
void BananaBunch::resetItemToOriginalState() {
	m_quantity = randInt(3, 5);
	undoUse();
}


// STACKABLES
Stackable::Stackable(int x, int y, std::string name, std::string image, int count) : Usable(x, y, name, image) {
	setCanStack(true);

	m_count = count;
}

Bomb::Bomb(int x, int y) : Stackable(x, y, BOMB, "Bomb_48x48.png", 1) {
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Makes things go boom.");
}
void Bomb::useItem(Dungeon &dungeon, Player &player) {
	playSound("Bomb_Placed.mp3");

	setPosX(player.getPosX());
	setPosY(player.getPosY());

	dungeon.addTrap(std::make_shared<ActiveBomb>(dungeon, getPosX(), getPosY()));

	used();
}

BearTrap::BearTrap(int x, int y) : Stackable(x, y, BEAR_TRAP, "Blue_Toy_32x32.png", 3) {
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("A standard contraption that can immobilize even\nthe largest of foes.");
}
void BearTrap::useItem(Dungeon &dungeon, Player &player) {
	playSound("Bomb_Placed.mp3");

	setPosX(player.getPosX());
	setPosY(player.getPosY());

	// Add to the traps vector, as it only goes off when something walks over it
	dungeon.addTrap(std::make_shared<SetBearTrap>(dungeon, getPosX(), getPosY()));

	used();
}

Matches::Matches(int x, int y) : Stackable(x, y, MATCHES, "CeilingSpike_48x48.png", 3) {
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("Light a few places or light a few faces.");
}
void Matches::useItem(Dungeon &dungeon, Player &player) {
	playSound("Fireblast_Spell2.mp3");

	int x = player.getPosX();
	int y = player.getPosY();

	char move = player.facingDirection();

	int n = 0, m = 0;
	setDirectionalOffsets(move, n, m);

	if (dungeon.trap(x + n, y + m)) {
		std::vector<int> indexes = dungeon.findTraps(x + n, y + m);
		for (int i = 0; i < (int)indexes.size(); i++)
			if (dungeon.trapAt(indexes[i])->canBeIgnited())
				dungeon.trapAt(indexes[i])->ignite();
	}

	if (dungeon.enemy(x + n, y + m)) {
		int pos = dungeon.findMonster(x + n, y + m);
		if (pos != -1) {
			if (dungeon.monsterAt(pos)->canBeBurned() || player.hasAfflictionOverride()) {
				int turns = 5 + (player.hasHarshAfflictions() ? 5 : 0);
				dungeon.giveAffliction(pos, std::make_shared<Burn>(*dungeon.getPlayer(), turns));
			}
		}
	}

	// Create ember if there is nothing else here
	if (!(dungeon.wall(x + n, y + m)))
		dungeon.addTrap(std::make_shared<Ember>(dungeon, x + n, y + m, 4 + randInt(0, 4)));

	used();
}

Firecrackers::Firecrackers(int x, int y) : Stackable(x, y, FIRECRACKERS, "Bright_Star_48x48.png", 5) {
	setSoundName("Bomb_Pickup2.mp3");

	setDescription("You might be able to delay your demise by just\na little using these. Small chance to ignite. Throwable.");
}
void Firecrackers::useItem(Dungeon &dungeon, Player &player) {
	used();

	playSound("Fireblast_Spell1.mp3");

	char move = player.facingDirection();

	int x = player.getPosX();
	int y = player.getPosY();

	int n = 0, m = 0;
	setDirectionalOffsets(move, n, m);

	int range = 10;

	while (!dungeon.wall(x + n, y + m) && range > 0) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		if (dungeon.enemy(x + n, y + m)) {
			int pos = dungeon.findMonster(x + n, y + m);
			if (pos != -1) {

				/*cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x + n, y + m, 2);*/

				if (dungeon.monsterAt(pos)->canBeStunned()) {
					int turns = 2 + (player.hasHarshAfflictions() ? 1 : 0);
					dungeon.giveAffliction(pos, std::make_shared<Stun>(turns));
				}

				if (dungeon.monsterAt(pos)->canBeBurned() || player.hasAfflictionOverride()) {
					if (randReal(1, 100) + player.getLuck() > 90) {
						int turns = 5 + (player.hasHarshAfflictions() ? 2 : 0);
						dungeon.giveAffliction(pos, std::make_shared<Burn>(*dungeon.getPlayer(), turns));
					}
				}

				dungeon.damageMonster(pos, 1, DamageType::NORMAL);
				return;
			}
		}

		incrementDirectionalOffsets(move, n, m);

		range--;
	}

	if (dungeon.wall(x + n, y + m) && dungeon.trap(x + n, y + m)) {
		std::vector<int> indexes = dungeon.findTraps(x + n, y + m);
		for (int pos = 0; pos < (int)indexes.size(); pos++)
			if (dungeon.trapAt(indexes.at(pos))->canBeIgnited() && randReal(1, 100) + player.getLuck() > 90)
				dungeon.trapAt(indexes.at(pos))->ignite();
	}
}


//		SKELETON KEY
SkeletonKey::SkeletonKey(int x, int y) : Usable(x, y, SKELETON_KEY, "Skeleton_Key_48x48.png") {
	setSoundName("Key_Pickup.mp3");

	setDescription("A mysterious key. What could this be for?");
}
void SkeletonKey::useItem(Dungeon &dungeon, Player &player) {

}


//		SPECIAL
Teleporter::Teleporter(int x, int y) : Usable(x, y, TELEPORTER, "Teleport_Scroll_48x48.png") {
	setSoundName("TurningPage3.mp3");

	setDescription("This is going to be fun.");
}
void Teleporter::useItem(Dungeon &dungeon, Player &player) {
	int x = player.getPosX();
	int y = player.getPosY();

	char move = player.facingDirection();

	int teleportDistance = 5 + randInt(0, 3);

	int n, m;
	setDirectionalOffsets(move, n, m, teleportDistance);

	// Check boundaries, if not within boundaries, they die instantly
	if (!dungeon.withinBounds(x + n, y + m)) {
		playSound("Fireblast_Spell1.mp3");
		playSound("Explosion.mp3");

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x, y, y + Z_PROJECTILE);

		player.setHP(0);
		dungeon.queueCustomAction(player.getSprite(), cocos2d::Hide::create());
		//dungeon.getPlayer()->getSprite()->setVisible(false);
		return;
	}

	if (!(dungeon.wall(x + n, y + m) || dungeon.enemy(x + n, y + m))) {
		playSound("Teleport_Spell.mp3");

		player.moveTo(x + n, y + m, 0.01f);

		if (dungeon.trap(x + n, y + m))
			dungeon.trapEncounter(x + n, y + m);

		return;
	}

	// Enemies die instantly if they teleport on top of the player
	if (dungeon.enemy(x + n, y + m)) {
		playSound("Teleport_Spell.mp3");

		int pos = dungeon.findMonster(x + n, y + m);
		if (pos != -1) {
			dungeon.damageMonster(pos, 1000, DamageType::MAGICAL);
			player.moveTo(x + n, y + m, 0.01f);

			if (dungeon.trap(x + n, y + m))
				dungeon.trapEncounter(x + n, y + m);
		}
		return;
	}

	// Look for free space on spaces adjacent to teleport location
	if (dungeon.wall(x + n, y + m)) {
		x += n;
		y += m;

		std::vector<Coords> coords;
		coords.push_back(Coords(x - 1, y));
		coords.push_back(Coords(x + 1, y));
		coords.push_back(Coords(x, y - 1));
		coords.push_back(Coords(x, y + 1));

		for (int i = 0; i < 4; i++) {

			if (!dungeon.withinBounds(coords[i].x, coords[i].y))
				continue;

			if (!(dungeon.wall(coords[i].x, coords[i].y) || dungeon.enemy(coords[i].x, coords[i].y))) {
				playSound("Teleport_Spell.mp3");
				player.moveTo(coords[i].x, coords[i].y, 0.01f);

				if (dungeon.trap(coords[i].x, coords[i].y))
					dungeon.trapEncounter(coords[i].x, coords[i].y);

				return;
			}

			if (dungeon.wall(coords[i].x, coords[i].y))
				continue;

			if (dungeon.enemy(coords[i].x, coords[i].y)) {
				playSound("Teleport_Spell.mp3");

				int pos = dungeon.findMonster(coords[i].x, coords[i].y);
				if (pos != -1) {
					dungeon.damageMonster(pos, 1000, DamageType::MAGICAL);
					player.moveTo(coords[i].x, coords[i].y, 0.01f);

					if (dungeon.trap(coords[i].x, coords[i].y))
						dungeon.trapEncounter(coords[i].x, coords[i].y);
				}
				return;
			}

		}

		playSound("Fireblast_Spell1.mp3");
		playSound("Explosion.mp3");

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x, y, y + Z_PROJECTILE);

		player.setHP(0);
		dungeon.queueCustomAction(player.getSprite(), cocos2d::Hide::create());
		return;
	}
}

Rocks::Rocks(int x, int y) : Usable(x, y, ROCKS, "Ruby_48x48.png") {	
	setSoundName("Bomb_Pickup2.mp3");

	m_durability = 3 + randInt(0, 1);

	setDescription("If it works, it works. Throwable.");
}
void Rocks::useItem(Dungeon &dungeon, Player &player) {
	playMiss();

	char move = player.facingDirection();

	int x = player.getPosX();
	int y = player.getPosY();

	int n = 0, m = 0;
	int p = n, q = m;

	setDirectionalOffsets(move, n, m);

	int range = 7;

	while (!dungeon.wall(x + n, y + m) && range > 0) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		if (dungeon.enemy(x + n, y + m)) {
			int pos = dungeon.findMonster(x + n, y + m);
			if (pos != -1) {

				if (dungeon.monsterAt(pos)->canBeStunned())
					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

				// Attempt knockback on enemy
				if (!dungeon.monsterAt(pos)->isHeavy())
					dungeon.linearActorPush(x + n, y + m, 2, 2, move);

				dungeon.damageMonster(pos, 1, DamageType::NORMAL);
				break;
			}
		}

		p = n, q = m;

		incrementDirectionalOffsets(move, n, m);

		range--;
	}

	p += x;
	q += y;

	m_durability--;
	if (m_durability == 0)
		return;

	if (dungeon.itemObject(p, q))
		dungeon.itemHash(p, q);

	Coords newPos(p, q);
	setPos(newPos);

	dungeon.tileAt(newPos).item = true;
	dungeon.tileAt(newPos).object = std::make_shared<Rocks>(*this);
	dungeon.tileAt(newPos).object->setSprite(dungeon.createSprite(newPos, q + Z_ITEM, dungeon.tileAt(newPos).object->getImageName()));
	dungeon.addItem(dungeon.tileAt(newPos).object);
}

Mobility::Mobility(int x, int y) : Usable(x, y, MOBILITY, "candy1.png") {
	setDescription("The Acrobat is highly skilled and flexible\nwhich allows them to execute incredible tactics\nsuch as rolling and backflips.");
}
void Mobility::useItem(Dungeon &dungeon, Player &player) {
	playMiss();

	char move = player.facingDirection();

	int x = player.getPosX();
	int y = player.getPosY();

	int n = 0, m = 0;
	int p = n, q = m;
	setDirectionalOffsets(move, n, m);

	int range = 3;
	int dist = 1;
	while (!dungeon.wall(x + n, y + m) && dist <= range) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + Z_PROJECTILE);

		if (!dungeon.enemy(x + n, y + m)) {
			p = n;
			q = m;
		}

		incrementDirectionalOffsets(move, n, m);

		dist++;
	}

	player.setInvulnerable(true);
	player.addAffliction(std::make_shared<Invulnerability>(1));
	player.moveTo(x + p, y + q, 0.05f);
}


// =============================================
//				:::: SPELLS ::::
// =============================================
Spell::Spell(int x, int y, int damage, std::string item, std::string image)
	: Usable(x, y, item, image), m_damage(damage) {

}

//		FREEZE SPELL
FreezeSpell::FreezeSpell(int x, int y) : Spell(x, y, 0, FREEZE_SPELL, "Freeze_Spell_48x48.png") {
	setSoundName("Book_Pickup.mp3");

	setDescription("A spellbook that has imagery of ice crystals.");
}
void FreezeSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Freeze_Spell1.mp3");
	playSound("Freeze_Spell2.mp3");

	int range = 4 + player.getInt();

	int x = player.getPosX();
	int y = player.getPosY();

	int mx, my;
	// add a freeze(stun) effect to nearby enemies
	for (unsigned int i = 0; i < dungeon.monsterCount(); i++) {
		mx = dungeon.monsterAt(i)->getPosX();
		my = dungeon.monsterAt(i)->getPosY();

		if (abs(x - mx) <= range && abs(y - my) <= range && (dungeon.monsterAt(i)->canBeFrozen() || player.hasAfflictionOverride())) {
			int turns = 18 + player.getInt() + (player.hasHarshAfflictions() ? 6 : 0);
			dungeon.giveAffliction(i, std::make_shared<Freeze>(turns));
			dungeon.monsterAt(i)->setFrozen(true);
		}
	}

	used();
}

IceShardSpell::IceShardSpell(int x, int y) : Spell(x, y, 0, ICE_SHARD_SPELL, "Freeze_Spell_48x48.png") {
	setSoundName("Book_Pickup.mp3");

	setDescription("Summon a wave of icicles to pierce and freeze enemies.");
}
void IceShardSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Freeze_Spell2.mp3");

	int x = player.getPosX();
	int y = player.getPosY();

	// Create an Ice Shards trap
	int shardLimit = player.getInt() > 5 ? 4 : 3;
	dungeon.addTrap(std::make_shared<IceShards>(dungeon, x, y, shardLimit));

	used();
}

HailStormSpell::HailStormSpell(int x, int y) : Spell(x, y, 0, HAIL_STORM_SPELL, "Freeze_Spell_48x48.png") {
	setSoundName("Book_Pickup.mp3");

	setDescription("Rain down frozen death.");
}
void HailStormSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Freeze_Spell2.mp3");

	int x = player.getPosX();
	int y = player.getPosY();

	// Create a HailStorm trap
	int shardLimit = player.getInt() > 5 ? 4 : 3;
	dungeon.addTrap(std::make_shared<HailStorm>(dungeon, x, y, player.facingDirection(), shardLimit));

	used();
}

//		EARTHQUAKE SPELL
EarthquakeSpell::EarthquakeSpell(int x, int y) : Spell(x, y, 6, EARTHQUAKE_SPELL, "Earthquake_Spell_48x48.png") {
	setSoundName("Book_Pickup.mp3");

	setDescription("A spellbook that cracks the earth.");
}
void EarthquakeSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Earthquake_Spell1.mp3");
	playSound("Earthquake_Spell2.mp3");
	playSound("Earthquake_Explosion1.mp3");
	playSound("Earthquake_Explosion2.mp3");

	int px = player.getPosX();
	int py = player.getPosY();
	int mx, my;

	// find any monsters caught in the quake
	for (unsigned i = 0; i < dungeon.monsterCount(); i++) {
		mx = dungeon.monsterAt(i)->getPosX();
		my = dungeon.monsterAt(i)->getPosY();

		if (abs(mx - px) <= 2 && abs(my - py) <= 2) {
			dungeon.damageMonster(i, getDamage() + player.getInt(), DamageType::MAGICAL);

			// Stun monster as well
			if (dungeon.monsterAt(i)->canBeStunned())
				dungeon.giveAffliction(i, std::make_shared<Stun>(4 + player.getInt()));
		}

	}

	//	destroy any nearby walls
	for (int j = py - 2; j < py + 3; j++) {
		for (int k = px - 2; k < px + 3; k++) {

			if (dungeon.withinBounds(k, j)) {

				if (dungeon.wall(k, j))
					dungeon.destroyWall(k, j);

				if (dungeon.exit(k, j))
					continue;

				if (dungeon.trap(k, j)) {

					std::vector<int> indexes = dungeon.findTraps(k, j);
					for (int i = 0; i < (int)indexes.size(); i++) {

						if (dungeon.trapAt(indexes.at(i))->isDestructible() ||
							!dungeon.trapAt(indexes.at(i))->isDestructible() && player.getInt() >= 5)
							dungeon.trapAt(indexes.at(i))->destroyTrap();

						else if (dungeon.trapAt(indexes.at(i))->isExplosive())
							dungeon.trapAt(indexes.at(i))->explode();
					}
				}

			}
		}
	}

	used();
}

RockSummonSpell::RockSummonSpell(int x, int y) : Spell(x, y, 6, ROCK_SUMMON_SPELL, "Earthquake_Spell_48x48.png") {
	setSoundName("Book_Pickup.mp3");

	setDescription("Summon a formation of magical stones that you can\nsend barreling into enemies.");
}
void RockSummonSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Earthquake_Spell1.mp3");

	// Get the rocks to spawn; More intellect = More rocks
	setCoords(dungeon, player);

	int x, y;
	for (unsigned int i = 0; i < coords.size(); i++) {
		x = coords[i].x;
		y = coords[i].y;

		if (dungeon.enemy(x, y)) {
			int pos = dungeon.findMonster(x, y);
			if (pos != -1) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x, y, y + Z_PROJECTILE);

				dungeon.damageMonster(pos, 1000, DamageType::MAGICAL);
			}
		}
		else if (dungeon.wallObject(x, y)) {
			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x, y, y + Z_PROJECTILE);

			dungeon.destroyWall(x, y);
		}
		else if (!dungeon.wall(x, y)) {
			// Create RockSummon trap
			dungeon.addTrap(std::make_shared<RockSummon>(dungeon, x, y));
		}
	}

	used();
}
void RockSummonSpell::setCoords(Dungeon &dungeon, Player &player) {
	coords.clear(); // Resonant Spells can cause this to be called more than once

	int x = player.getPosX();
	int y = player.getPosY();

	int intellect = player.getInt();

	int n = 0, m = 0;
	setDirectionalOffsets(player.facingDirection(), n, m);

	// Summon 3 in front of player
	if (intellect <= 5) {
		// Player attacked to the side, so check above and below
		if (m == 0) {
			coords.push_back(Coords(x + n, y + m));
			coords.push_back(Coords(x + n, y + 1));
			coords.push_back(Coords(x + n, y - 1));
		}
		// Else player attacked above or below, so check the sides
		else {
			coords.push_back(Coords(x + n, y + m));
			coords.push_back(Coords(x + 1, y + m));
			coords.push_back(Coords(x - 1, y + m));
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

				coords.push_back(Coords(i, j));
			}
		}
	}
	else if (intellect > 10) {
		for (int i = x - 1; i < x + 2; i++) {
			for (int j = y - 1; j < y + 2; j++) {

				if (i == x && j == y)
					continue;

				coords.push_back(Coords(i, j));
			}
		}
	}
}

ShockwaveSpell::ShockwaveSpell(int x, int y) : Spell(x, y, 6, SHOCKWAVE_SPELL, "Earthquake_Spell_48x48.png") {
	setSoundName("Book_Pickup.mp3");

	setDescription("The ground reverberates with energy upon passing\nglance of this relic.");
}
void ShockwaveSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Earthquake_Spell1.mp3");
	playSound("Earthquake_Spell2.mp3");

	int x = player.getPosX();
	int y = player.getPosY();
	char move = player.facingDirection();

	int n, m;
	setDirectionalOffsets(move, n, m);

	// fire explosion animation
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
	dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

	if (dungeon.enemy(x + n, y + m)) {
		int pos = dungeon.findMonster(x + n, y + m);
		if (pos != -1) {
			if (dungeon.monsterAt(pos)->canBeStunned())
				dungeon.giveAffliction(pos, std::make_shared<Stun>(2));

			dungeon.damageMonster(pos, getDamage(), DamageType::MAGICAL);
		}
	}

	if (dungeon.underEnemy(x + n, y + m)) {
		int pos = dungeon.findUndergroundMonster(x + n, y + m);
		if (pos != -1) {
			if (dungeon.monsterAt(pos)->canBeStunned())
				dungeon.giveAffliction(pos, std::make_shared<Stun>(2));

			dungeon.damageMonster(pos, getDamage(), DamageType::MAGICAL);
		}
	}

	// Create Shockwave trap
	int waveLimit = player.getInt() > 5 ? 7 : 5;
	dungeon.addTrap(std::make_shared<Shockwaves>(dungeon, x, y, move, waveLimit));

	used();
}

//		FIRE BLAST SPELL
FireBlastSpell::FireBlastSpell(int x, int y) : Spell(x, y, 5, FIREBLAST_SPELL, "Fireblast_Spell_48x48.png") {
	setSoundName("Book_Pickup.mp3");

	setDescription("A fiery spell that launches a projectile in the\n direction you're facing.");
}
void FireBlastSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Fireblast_Spell1.mp3");

	int px = player.getPosX();
	int py = player.getPosY();

	int n, m; // n : x, m : y
	char playerFacing = player.facingDirection();
	setDirectionalOffsets(playerFacing, n, m);

	while (!(dungeon.wall(px + n, py + m) || dungeon.enemy(px + n, py + m))) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, px + n, py + m, py + m + Z_PROJECTILE);

		incrementDirectionalOffsets(playerFacing, n, m);
	}

	// play fire blast explosion sound effect
	playSound("Fireblast_Spell2.mp3");

	px += n;
	py += m;

	for (int i = px - 1; i < px + 2; i++) {
		for (int j = py - 1; j < py + 2; j++) {

			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);

			if (dungeon.hero(i, j))
				dungeon.damagePlayer(getDamage(), DamageType::FIRE);

			if (dungeon.enemy(i, j)) {

				int pos = dungeon.findMonster(i, j);
				if (pos != -1) {
					dungeon.damageMonster(pos, getDamage() + player.getInt(), DamageType::FIRE);

					// If they can be burned, roll for a high chance to burn
					if (dungeon.monsterAt(pos)->canBeBurned() || player.hasAfflictionOverride()) {
						int turns = 5 + player.getInt() + (player.hasHarshAfflictions() ? 6 : 0);
						dungeon.giveAffliction(pos, std::make_shared<Burn>(*dungeon.getPlayer(), turns));
					}
				}
			}

			if (dungeon.trap(i, j)) {
				std::vector<int> indexes = dungeon.findTraps(i, j);
				for (int pos = 0; pos < (int)indexes.size(); pos++)
					if (dungeon.trapAt(indexes.at(pos))->canBeIgnited())
						dungeon.trapAt(indexes.at(pos))->ignite();
			}
		}
	}

	used();
}

FireCascadeSpell::FireCascadeSpell(int x, int y) : Spell(x, y, 5, FIRE_CASCADE_SPELL, "Fireblast_Spell_48x48.png") {
	setSoundName("Book_Pickup.mp3");

	setDescription("Releases pillars of fire that cascade outward.");
}
void FireCascadeSpell::useItem(Dungeon &dungeon, Player &player) {
	int x = player.getPosX();
	int y = player.getPosY();

	playSound("Fireblast_Spell1.mp3");

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (i == x && j == y)
				continue;

			if (!dungeon.wall(i, j)) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);
			}

			if (dungeon.enemy(i, j)) {

				int pos = dungeon.findMonster(i, j);
				if (pos != -1) {

					// If they can be burned, burn them
					if (dungeon.monsterAt(pos)->canBeBurned() || player.hasAfflictionOverride()) {
						int turns = 10 + player.getInt() + (player.hasHarshAfflictions() ? 6 : 0);
						dungeon.giveAffliction(pos, std::make_shared<Burn>(*dungeon.getPlayer(), turns));
					}
				}
			}

			if (dungeon.trap(i, j)) {
				std::vector<int> indexes = dungeon.findTraps(i, j);
				for (int pos = 0; pos < (int)indexes.size(); pos++)
					if (dungeon.trapAt(indexes.at(pos))->canBeIgnited())
						dungeon.trapAt(indexes.at(pos))->ignite();
			}
		}
	}

	// Create fire pillar trap
	int pillarLimit = player.getInt() > 5 ? 2 : 1;
	dungeon.addTrap(std::make_shared<FirePillars>(dungeon, x, y, pillarLimit));

	used();
}

FireExplosionSpell::FireExplosionSpell(int x, int y) : Spell(x, y, 5, FIRE_EXPLOSION_SPELL, "Fireblast_Spell_48x48.png") {
	setSoundName("Book_Pickup.mp3");

	setDescription("Unleash your wrath. High chance to burn.");
}
void FireExplosionSpell::useItem(Dungeon &dungeon, Player &player) {
	int x = player.getPosX();
	int y = player.getPosY();

	playSound("Fireblast_Spell1.mp3");
	playSound("Explosion.mp3");

	// Damage any monsters in the blast radius
	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {

			// Boundary and corner check
			if (!dungeon.withinBounds(i, j) || (i == x + 2 && j == y - 2) || (i == x + 2 && j == y + 2) || (i == x - 2 && j == y - 2) || (i == x - 2 && j == y + 2))
				continue;

			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);

			if (dungeon.enemy(i, j)) {

				int pos = dungeon.findMonster(i, j);
				if (pos != -1) {
					dungeon.damageMonster(pos, getDamage() + player.getInt(), DamageType::MAGICAL);

					// If they can be burned, roll for a high chance to burn
					if (dungeon.monsterAt(pos)->canBeBurned() || player.hasAfflictionOverride()) {
						if (randReal(1, 100) + player.getLuck() > 50 - player.getInt()) {
							int turns = 5 + player.getInt() + (player.hasHarshAfflictions() ? 6 : 0);
							dungeon.giveAffliction(pos, std::make_shared<Burn>(*dungeon.getPlayer(), turns));
						}
					}
				}
			}

			if (dungeon.trap(i, j)) {
				std::vector<int> indexes = dungeon.findTraps(i, j);
				for (int pos = 0; pos < (int)indexes.size(); pos++)
					if (dungeon.trapAt(indexes.at(pos))->canBeIgnited())
						dungeon.trapAt(indexes.at(pos))->ignite();
			}
		}
	}

	// Chance to burn the player as well
	if (player.canBeBurned()) {

		// Add their Intellect stat because it should be less likely to get burned with the more Int they have
		if (randReal(1, 100) - player.getLuck() > 60 + player.getInt())
			player.addAffliction(std::make_shared<Burn>(*dungeon.getPlayer(), 5));	
	}

	used();
}

//		WIND SPELL
WhirlwindSpell::WhirlwindSpell(int x, int y) : Spell(x, y, 0, WHIRLWIND_SPELL, "Wind_Spell_48x48.png") {	
	setSoundName("Book_Pickup.mp3");

	setDescription("A light breeze runs by when you touch the cover.");
}
void WhirlwindSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Wind_Spell1.mp3");
	playSound("Wind_Spell2.mp3");

	// if player is on fire, roll for a chance to remove it
	if (player.isBurned()) {
		double roll = randReal(1, 100) + 2 * player.getInt();

		if (roll > 50)
			player.removeAffliction(BURN);
	}

	int px = player.getPosX();
	int py = player.getPosY();
	int mx, my;

	// find any monsters caught in the whirlwind
	char move;
	int tries = 5; // for breaking out of the loop if too many tries
	int radius = 3;

	for (unsigned int i = 0; i < dungeon.monsterCount(); i++) {
		tries = 5 + (player.getInt() >= 10 ? 3 : player.getInt() >= 5 ? 1 : 0);
		radius = 3 + (player.getInt() >= 10 ? 2 : player.getInt() >= 5 ? 1 : 0);

		mx = dungeon.monsterAt(i)->getPosX();
		my = dungeon.monsterAt(i)->getPosY();

		if (abs(mx - px) > radius && abs(my - py) > radius || dungeon.monsterAt(i)->isHeavy())
			continue;

		// if there any monsters within a 3 block radius, push them away
		while (abs(mx - px) <= radius && abs(my - py) <= radius && tries > 0) {

			// if monster is to the left of the player and equally as far to the side or less as they are to above or below the player
			if (mx < px && (my - py == 0 || abs(mx - px) <= abs(my - py)))
				move = 'l';

			// if monster is to the right of the player and equally as far to the side or less as they are to above or below the player
			else if (mx > px && (my - py == 0 || abs(mx - px) <= abs(my - py)))
				move = 'r';

			// if monster is above the player and equally as far above/below or less as they are to the side of the player
			else if (my < py && (mx - px == 0 || abs(my - py) <= abs(mx - px)))
				move = 'u';

			// if monster is below the player and equally as far above/below or less as they are to the side of the player
			else if (my > py && (mx - px == 0 || abs(my - py) <= abs(mx - px)))
				move = 'd';

			dungeon.pushMonster(mx, my, move);

			if (dungeon.trap(mx, my))
				dungeon.singleMonsterTrapEncounter(dungeon.findMonster(mx, my));

			if (dungeon.monsterAt(i)->isDead())
				break;

			mx = dungeon.monsterAt(i)->getPosX();
			my = dungeon.monsterAt(i)->getPosY();
			tries--;
		}

		if (dungeon.monsterAt(i)->canBeStunned()) {
			// if player has at least 5 intellect, stun is increased to three turns
			int turns = (player.getInt() >= 5 ? 3 : 1);
			dungeon.giveAffliction(i, std::make_shared<Stun>(turns));
		}
	}

	used();
}

WindBlastSpell::WindBlastSpell(int x, int y) : Spell(x, y, 0, WIND_BLAST_SPELL, "Wind_Spell_48x48.png") {	
	setSoundName("Book_Pickup.mp3");

	setDescription("Call forth a powerful gust of wind to knock\nenemies backward.");
}
void WindBlastSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Wind_Spell2.mp3");

	int x = player.getPosX();
	int y = player.getPosY();

	char move = player.facingDirection();

	int n, m; // n : x, m : y
	switch (move) {
	case 'l': n = -1; m = -1; break;
	case 'r': n = 1; m = -1; break;
	case 'u': n = -1; m = -1; break;
	case 'd': n = -1; m = 1; break;
	}

	int range = 6;
	int currentRange = 1;

	int k = 1; // 3 rows/columns to check
	while (k <= 3) {

		while (!dungeon.wall(x + n, y + m) && currentRange <= range) {

			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

			if (dungeon.enemy(x + n, y + m)) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1)
					dungeon.giveAffliction(pos, std::make_shared<Stun>(2 + player.getInt()));

				dungeon.linearActorPush(x + n, y + m, range - currentRange + 1, range, move);
				break;
			}

			switch (move) {
			case 'l': n--; break;
			case 'r': n++; break;
			case 'u': m--; break;
			case 'd': m++; break;
			}

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
	}

	used();
}

WindVortexSpell::WindVortexSpell(int x, int y) : Spell(x, y, 0, WIND_VORTEX_SPELL, "Wind_Spell_48x48.png") {	
	setSoundName("Book_Pickup.mp3");

	setDescription("Let loose the eye of the storm.");
}
void WindVortexSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Wind_Spell2.mp3");

	int x = player.getPosX();
	int y = player.getPosY();

	// Create WindVortex trap
	int turnLimit = 10 + player.getInt();
	dungeon.addTrap(std::make_shared<WindVortex>(dungeon, x, y, turnLimit));

	used();
}

//		THUNDER CLOUD
ThunderCloudSpell::ThunderCloudSpell(int x, int y) : Spell(x, y, 0, THUNDER_CLOUD_SPELL, "Wind_Spell_48x48.png") {	
	setSoundName("Book_Pickup.mp3");

	setDescription("Send forth a miniature thunderstorm.");
}
void ThunderCloudSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Wind_Spell2.mp3");

	int x = player.getPosX();
	int y = player.getPosY();
	char move = player.facingDirection();

	int n = 0, m = 0;
	setDirectionalOffsets(move, n, m);

	// Create ThunderCloud trap
	int turnLimit = 15 + player.getInt();
	dungeon.addTrap(std::make_shared<ThunderCloud>(dungeon, x + n, y + m, move, turnLimit));

	used();
}


//		INVISIBILITY SPELL
InvisibilitySpell::InvisibilitySpell(int x, int y) : Spell(x, y, 0, INVISIBILITY_SPELL, "Invisibility_Spell_48x48.png") {	
	setSoundName("Book_Pickup.mp3");

	setDescription("Now you see me. Now you");
}
void InvisibilitySpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Invisibility_Spell.mp3");

	player.addAffliction(std::make_shared<Invisibility>(25 + player.getInt()));

	used();
}

//		ETHEREAL SPELL
EtherealSpell::EtherealSpell(int x, int y) : Spell(x, y, 0, ETHEREAL_SPELL, "Ethereal_Spell_48x48.png") {	
	setSoundName("Book_Pickup.mp3");

	setDescription("Now you see me. Now you.. still do?");
}
void EtherealSpell::useItem(Dungeon &dungeon, Player &player) {
	playSound("Ethereal_Spell.mp3");

	player.addAffliction(std::make_shared<Ethereality>(15 + 2 * player.getInt()));

	used();
}



// =============================================
//				:::: PASSIVES ::::
// =============================================
Passive::Passive(int x, int y, std::string name, std::string image) : Objects(x, y, name, image) {

}

BatWing::BatWing(int x, int y) : Passive(x, y, BAT_WING, "Angels_Brace.png") {
	setDescription("A reminder that agility keeps you alive. Boosts dexterity.");
}
void BatWing::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setDex(p.getDex() + 1);
}
void BatWing::unapply(Player &p) {
	p.setDex(p.getDex() - 1);
}

LifeElixir::LifeElixir(int x, int y) : Passive(x, y, LIFE_ELIXIR, "Heart_Potion_48x48.png") {
	setDescription("An incredible concoction that invigorates you down to\nyour soul. Increases your maximum health.");
}
void LifeElixir::apply(Player &p) {
	// sound effect
	playSound("Life_Potion_Used.mp3");

	p.setMaxHP(p.getMaxHP() + 25);
}
void LifeElixir::unapply(Player &p) {
	p.setMaxHP(p.getMaxHP() - 25);
	if (p.getHP() > p.getMaxHP())
		p.setHP(p.getMaxHP());
}

MagicEssence::MagicEssence(int x, int y) : Passive(x, y, MAGIC_ESSENCE, "Cheese_Wedge_48x48.png") {
	setDescription("In older times, the Spellmasters would wear these to\nincrease their magical prowess in a pinch.");
}
void MagicEssence::apply(Player &p) {
	// sound effect
	playSound("Life_Potion_Used.mp3");

	p.setInt(p.getInt() + 1);
}
void MagicEssence::unapply(Player &p) {
	p.setInt(p.getInt() - 1);
}

Flying::Flying(int x, int y) : Passive(x, y, FLYING, "Angels_Brace.png") {
	setDescription("Only the most powerful mages could master such a\ntechnique. Years would be spent to get to this level of\nsophistication.");
}
void Flying::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setFlying(true);
}
void Flying::unapply(Player &p) {
	p.setFlying(false);
}

SteelPunch::SteelPunch(int x, int y) : Passive(x, y, STEEL_PUNCH, "Cheese_Wedge_48x48.png") {
	setDescription("You have acquired the secret of the Steel Punch!");
}
void SteelPunch::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setSteelPunch(true);
}
void SteelPunch::unapply(Player &p) {
	p.setSteelPunch(false);
}

IronCleats::IronCleats(int x, int y) : Passive(x, y, IRON_CLEATS, "Cheese_Wedge_48x48.png") {
	setDescription("A set of well-crafted iron boots. Prevents spike damage.");
}
void IronCleats::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setSpikeImmunity(true);
}
void IronCleats::unapply(Player &p) {
	p.setSpikeImmunity(false);
}

PoisonTouch::PoisonTouch(int x, int y) : Passive(x, y, POISON_TOUCH, "Cheese_Wedge_48x48.png") {
	setDescription("Chance to poison enemies when attacking.");
}
void PoisonTouch::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setPoisonTouch(true);
}
void PoisonTouch::unapply(Player &p) {
	p.setPoisonTouch(false);
}

FireTouch::FireTouch(int x, int y) : Passive(x, y, FIRE_TOUCH, "Cheese_Wedge_48x48.png") {
	setDescription("Chance to ignite enemies when attacking.");
}
void FireTouch::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setFireTouch(true);
}
void FireTouch::unapply(Player &p) {
	p.setFireTouch(false);
}

FrostTouch::FrostTouch(int x, int y) : Passive(x, y, FROST_TOUCH, "Cheese_Wedge_48x48.png") {
	setDescription("Chance to freeze enemies when attacking.");
}
void FrostTouch::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setFrostTouch(true);
}
void FrostTouch::unapply(Player &p) {
	p.setFrostTouch(false);
}

RainbowTouch::RainbowTouch(int x, int y) : Passive(x, y, RAINBOW_TOUCH, "Cheese_Wedge_48x48.png") {
	setDescription("Chance to poison, ignite, or freeze enemies.");
}
void RainbowTouch::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setPoisonTouch(true);
	p.setFireTouch(true);
	p.setFrostTouch(true);
}
void RainbowTouch::unapply(Player &p) {
	p.setPoisonTouch(false);
	p.setFireTouch(false);
	p.setFrostTouch(false);
}

PoisonImmune::PoisonImmune(int x, int y) : Passive(x, y, POISON_IMMUNE, "Cheese_Wedge_48x48.png") {
	setDescription("The antibodies flowing within you are tuned far beyond\nthe standard.");
}
void PoisonImmune::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCanBePoisoned(false);
}
void PoisonImmune::unapply(Player &p) {
	p.setCanBePoisoned(true);
}

FireImmune::FireImmune(int x, int y) : Passive(x, y, FIRE_IMMUNE, "Vulcan_Rune_48x48.png") {
	setDescription("Thicker skin? No. Magical forces prevent you\nfrom experiencing burns.");
}
void FireImmune::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCanBeBurned(false);
}
void FireImmune::unapply(Player &p) {
	p.setCanBeBurned(true);
}

LavaImmune::LavaImmune(int x, int y) : Passive(x, y, VULCAN_RUNE, "Vulcan_Rune_48x48.png") {
	setDescription("Some say this rune was forged by a legendary\nblacksmith that then hid their wares deep\ninside a volcano. Provides lava resistance.");
}
void LavaImmune::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setLavaImmunity(true);
}
void LavaImmune::unapply(Player &p) {
	p.setLavaImmunity(false);
}

BombImmune::BombImmune(int x, int y) : Passive(x, y, BOMB_IMMUNE, "Cheese_Wedge_48x48.png") {
	setDescription("The art of bomb-making becomes second nature to you.\nImmunity to explosions and increases bomb damage.");
}
void BombImmune::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setExplosionImmune(true);
}
void BombImmune::unapply(Player &p) {
	p.setExplosionImmune(false);
}

PotionAlchemy::PotionAlchemy(int x, int y) : Passive(x, y, POTION_ALCHEMY, "Cheese_Wedge_48x48.png") {
	setDescription("Your knowledge of potions allows you to make them more potent.");
}
void PotionAlchemy::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setPotentPotions(true);
}
void PotionAlchemy::unapply(Player &p) {
	p.setPotentPotions(false);
}

SoulSplit::SoulSplit(int x, int y) : Passive(x, y, SOUL_SPLIT, "Cheese_Wedge_48x48.png") {
	setDescription("Greed has manifested itself into a tangible being.");
}
void SoulSplit::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setSoulSplit(true);
}
void SoulSplit::unapply(Player &p) {
	p.setSoulSplit(false);
}

LuckUp::LuckUp(int x, int y) : Passive(x, y, LUCKY_PIG, "Lucky_Pig_48x48.png") {
	setDescription("A lucky pig's head. But not so much for the pig.");
}
void LuckUp::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setLuck(p.getLuck() + 15);
}
void LuckUp::unapply(Player &p) {
	p.setLuck(p.getLuck() - 15);
}

Berserk::Berserk(int x, int y) : Passive(x, y, BERSERK, "Whiskey_48x48.png") {
	setDescription("You should really watch your temper, you're turning red!");
}
void Berserk::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setBloodlust(true);
}
void Berserk::unapply(Player &p) {
	p.setBloodlust(false);
}

LifeSteal::LifeSteal(int x, int y) : Passive(x, y, LIFE_STEAL, "Bloody_Apple_32x32.png") {
	setDescription("Suddenly, blood sounds particularly delicious...");
}
void LifeSteal::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setLifesteal(true);
}
void LifeSteal::unapply(Player &p) {
	p.setLifesteal(false);
}

Heavy::Heavy(int x, int y) : Passive(x, y, HEAVY, "Cheese_Wedge_48x48.png") {
	setDescription("Your steps have become incredibly solid.");
}
void Heavy::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setHeavy(true);
}
void Heavy::unapply(Player &p) {
	p.setHeavy(false);
}

BrickBreaker::BrickBreaker(int x, int y) : Passive(x, y, BRICK_BREAKER, "Cheese_Wedge_48x48.png") {
	setDescription("Claustrophobia is setting in, but smashing down these\nwalls takes a toll on your well-being.");
}
void BrickBreaker::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCanBreakWalls(true);
}
void BrickBreaker::unapply(Player &p) {
	p.setCanBreakWalls(false);
}

SummonNPCs::SummonNPCs(int x, int y) : Passive(x, y, SUMMON_NPCS, "Cheese_Wedge_48x48.png") {
	setDescription("Your cries do not go unheard. Others have a greater\nchance of finding you.");
}
void SummonNPCs::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCharismaNPC(true);
}
void SummonNPCs::unapply(Player &p) {
	p.setCharismaNPC(false);
}

CheapShops::CheapShops(int x, int y) : Passive(x, y, CHEAP_SHOPS, "Cheese_Wedge_48x48.png") {
	setDescription("Your way of words convinces those around you\nto give you a better deal.");
}
void CheapShops::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setCheapShops(true);
}
void CheapShops::unapply(Player &p) {
	p.setCheapShops(false);
}

BetterRates::BetterRates(int x, int y) : Passive(x, y, BETTER_RATES, "Cheese_Wedge_48x48.png") {
	setDescription("An overwhelming feeling of relaxation overcomes you.");
}
void BetterRates::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setBetterRates(true);
}
void BetterRates::unapply(Player &p) {
	p.setBetterRates(false);
}

TrapIllumination::TrapIllumination(int x, int y) : Passive(x, y, TRAP_ILLUMINATION, "Cheese_Wedge_48x48.png") {
	setDescription("Suddenly, the dangers of this dungeon are made more\napparent.");
}
void TrapIllumination::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setTrapIllumination(true);
}
void TrapIllumination::unapply(Player &p) {
	p.setTrapIllumination(false);
}

ItemIllumination::ItemIllumination(int x, int y) : Passive(x, y, ITEM_ILLUMINATION, "Cheese_Wedge_48x48.png") {
	setDescription("The treasures of this place are revealed.");
}
void ItemIllumination::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setItemIllumination(true);
}
void ItemIllumination::unapply(Player &p) {
	p.setItemIllumination(false);
}

MonsterIllumination::MonsterIllumination(int x, int y) : Passive(x, y, MONSTER_ILLUMINATION, "Cheese_Wedge_48x48.png") {
	setDescription("The monsters of this place are revealed.");
}
void MonsterIllumination::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setMonsterIllumination(true);
}
void MonsterIllumination::unapply(Player &p) {
	p.setMonsterIllumination(false);
}

ResonantSpells::ResonantSpells(int x, int y) : Passive(x, y, RESONANT_SPELLS, "Cheese_Wedge_48x48.png") {
	setDescription("Somehow the magic of this place has begun\nto stick with you. Spells have a chance to\nstay on use.");
}
void ResonantSpells::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setResonantSpells(true);
}
void ResonantSpells::unapply(Player &p) {
	p.setResonantSpells(false);
}

FatStacks::FatStacks(int x, int y) : Passive(x, y, FAT_STACKS, "Cheese_Wedge_48x48.png") {
	setDescription("Suddenly your pockets are deep, very deep.\nAll items are now stackable.");
}
void FatStacks::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setFatStacks(true);
}
void FatStacks::unapply(Player &p) {
	p.setFatStacks(false);
}

Scavenger::Scavenger(int x, int y) : Passive(x, y, SCAVENGER, "Cheese_Wedge_48x48.png") {
	setDescription("Often times survival comes down to how crafty you can\nbe. All enemies have a small chance of dropping loot.");
}
void Scavenger::apply(Player &p) {
	playSound("Armor_Use.mp3");

	p.setScavenger(true);
}
void Scavenger::unapply(Player &p) {
	p.setScavenger(false);
}


//				:::: RELICS ::::
Relic::Relic(int x, int y, std::string name, std::string image) : Objects(x, y, name, image) {
	
}


CursedStrength::CursedStrength(int x, int y) : Relic(x, y, CURSED_STRENGTH, "Blue_Toy_32x32.png") {
	setDescription("Have the gods favored you, or was it a farce?\nGrants immense strength--at a cost.");
}
void CursedStrength::apply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 3:
		p.setStr(p.getStr() + 3);
	case 2:
		p.setArmor(p.getArmor() + 2);
	case 1:
		p.setSlow(true);
		p.setStr(p.getStr() + 8);
		break;
	}
}
void CursedStrength::unapply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 4: 
		p.setSlow(false);
	case 3:
		p.setStr(p.getStr() - 3);
	case 2:
		p.setArmor(p.getArmor() - 2);
	case 1:
		p.setSlow(false);
		p.setStr(p.getStr() - 8);
		break;
	}
}
void CursedStrength::upgrade(Dungeon &dungeon, Player &p) {
	if (getLevel() == 4)
		return;

	switch (getLevel()) {
	case 1:
		p.setArmor(p.getArmor() + 2);
		break;
	case 2:
		p.setStr(p.getStr() + 3);
		break;
	case 3:
		p.setSlow(false);
		break;
	}

	increaseLevel();
}

BrightStar::BrightStar(int x, int y) : Relic(x, y, BRIGHT_STAR, "Fireflies_Jar_48x48.png") {
	setDescription("A small light in this dark place.");
}
void BrightStar::apply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 3:
		p.setVision(p.getVision() + 1);
	case 2:
		p.setGoldIllumination(true);
	case 1:
		p.setVision(p.getVision() + 1);
		break;
	}
}
void BrightStar::unapply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 4:
		p.setLuck(p.getLuck() - 25);
		p.decreaseFavorBy(5);
	case 3:
		p.setVision(p.getVision() - 1);
	case 2:
		p.setGoldIllumination(false);
	case 1:
		p.setVision(p.getVision() - 1);
		break;
	}
}
void BrightStar::upgrade(Dungeon &dungeon, Player &p) {
	if (getLevel() == 4)
		return;

	switch (getLevel()) {
	case 1:
		p.setGoldIllumination(true);
		break;
	case 2:
		p.setVision(p.getVision() + 1);
		break;
	case 3:
		p.setLuck(p.getLuck() + 25);
		p.increaseFavorBy(5);
		break;
	}

	increaseLevel();
}

DarkStar::DarkStar(int x, int y) : Relic(x, y, DARK_STAR, "Lightbulb_32x32.png") {
	setDescription("The darkness brings you strength.");
}
void DarkStar::apply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 3:
		p.setVision(p.getVision() - 1);
		p.setStr(p.getStr() + 2);
		p.setDex(p.getDex() + 2);
	case 2:
		p.setDex(p.getDex() + 2);
	case 1:
		p.setVision(p.getVision() - 1);
		p.setStr(p.getStr() + 3);
		break;
	}
}
void DarkStar::unapply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 4:
		p.setChainLightning(false);
		p.setVision(p.getVision() - 1);
	case 3:
		p.setVision(p.getVision() + 1);
		p.setStr(p.getStr() - 2);
		p.setDex(p.getDex() - 2);
	case 2:
		p.setDex(p.getDex() - 2);
	case 1:
		p.setVision(p.getVision() + 1);
		p.setStr(p.getStr() - 3);
		break;
	}
}
void DarkStar::upgrade(Dungeon &dungeon, Player &p) {
	if (getLevel() == 4)
		return;

	switch (getLevel()) {
	case 1:
		p.setDex(p.getDex() + 2);
		break;
	case 2:
		p.setVision(p.getVision() - 1);
		p.setStr(p.getStr() + 2);
		p.setDex(p.getDex() + 2);
		break;
	case 3:
		p.setChainLightning(true);
		p.setVision(p.getVision() + 1);
		break;
	}

	increaseLevel();
}

Riches::Riches(int x, int y) : Relic(x, y, RICHES, "Golden_Ring_32x32.png") {
	setDescription("A strong sense of greed fills your mind with this ring\nin your possession.");
}
void Riches::apply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 3:
		p.setMoneyMultiplier(p.getMoneyMultiplier() + 1);
	case 2:
		p.setMoneyMultiplier(p.getMoneyMultiplier() + 1);
	case 1:
		p.setMoneyBonus(p.getMoneyBonus() + 3);
		break;
	}
}
void Riches::unapply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 4:
		p.setGoldInvulnerability(false);
	case 3:
		p.setMoneyMultiplier(p.getMoneyMultiplier() - 1);
	case 2:
		p.setMoneyMultiplier(p.getMoneyMultiplier() - 1);
	case 1:
		p.setMoneyBonus(p.getMoneyBonus() - 3);
		break;
	}
}
void Riches::upgrade(Dungeon &dungeon, Player &p) {
	if (getLevel() == 4)
		return;

	switch (getLevel()) {
	case 1:
		p.setMoneyMultiplier(p.getMoneyMultiplier() + 1);
		break;
	case 2:
		p.setMoneyMultiplier(p.getMoneyMultiplier() + 1);
		break;
	case 3:
		p.setGoldInvulnerability(true);
		break;
	}

	increaseLevel();
}

MatrixVision::MatrixVision(int x, int y) : Relic(x, y, MATRIX_VISION, "Water_Orb_32x32.png") {
	setDescription("Do your eyes deceive you or can you suddenly\nglimpse tears in the fabric of space-time?");
}
void MatrixVision::apply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 3:
		p.setTimerReduction(p.getTimerReduction() + 0.05f);
	case 2:
		p.setCripplingBlows(true);
	case 1:
		p.setTimerReduction(p.getTimerReduction() + 0.05f);
		break;
	}
}
void MatrixVision::unapply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 4:
		p.setMatrixVision(false);
	case 3:
		p.setTimerReduction(p.getTimerReduction() - 0.05f);
	case 2:
		p.setCripplingBlows(false);
	case 1:
		p.setTimerReduction(p.getTimerReduction() - 0.05f);
		break;
	}
}
void MatrixVision::upgrade(Dungeon &dungeon, Player &p) {
	if (getLevel() == 4)
		return;

	switch (getLevel()) {
	case 1:
		p.setCripplingBlows(true);
		break;
	case 2:
		p.setTimerReduction(p.getTimerReduction() + 0.05f);
		break;
	case 3:
		p.setMatrixVision(true);
		break;
	}

	increaseLevel();
}

SuperMagicEssence::SuperMagicEssence(int x, int y) : Relic(x, y, SUPER_MAGIC_ESSENCE, "Wizards_Hat_48x48.png") {
	setDescription("The control of elemental power was long sought after.\nOnly the greatest could achieve totally mastery.");
}
void SuperMagicEssence::apply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 3:
		p.setInt(p.getInt() + 1);
		p.setSpellRetaliation(true);
	case 2:
		p.setInt(p.getInt() + 2);
		p.setHarshAfflictions(true);
	case 1:
		p.setInt(p.getInt() + 3);
		break;
	}
}
void SuperMagicEssence::unapply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 4:
		p.setAfflictionOverride(false);
	case 3:
		p.setInt(p.getInt() - 1);
		p.setSpellRetaliation(false);
	case 2:
		p.setInt(p.getInt() - 2);
		p.setHarshAfflictions(false);
	case 1:
		p.setInt(p.getInt() - 3);
		break;
	}
}
void SuperMagicEssence::upgrade(Dungeon &dungeon, Player &p) {
	if (getLevel() == 4)
		return;

	switch (getLevel()) {
	case 1:
		p.setInt(p.getInt() + 2);
		p.setHarshAfflictions(true);
		break;
	case 2:
		p.setInt(p.getInt() + 1);
		p.setSpellRetaliation(true);
		break;
	case 3:
		p.setAfflictionOverride(true);
		break;
	}

	increaseLevel();
}

Protection::Protection(int x, int y) : Relic(x, y, PROTECTION, "Waffles_48x48.png") {
	setDescription("Somehow, you feel safer with this close to your chest.\nGrants impressive protective abilities.");
}
void Protection::apply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 3:
		p.setArmor(p.getArmor() + 2);
		p.setBonusRoll(true);
	case 2:
		p.setLuck(p.getLuck() + 5);
		p.setMaxHP(p.getMaxHP() * 1.15);
	case 1:
		p.setArmor(p.getArmor() + 1);
		p.setMaxHP(p.getMaxHP() * 1.1);
		break;
	}
}
void Protection::unapply(Dungeon &dungeon, Player &p) {
	switch (getLevel()) {
	case 4:
		p.setMaxHP(p.getMaxHP() / 1.1);
		p.setFragileRetaliation(false);
	case 3:
		p.setArmor(p.getArmor() - 2);
		p.setBonusRoll(false);
	case 2:
		p.setLuck(p.getLuck() - 5);
		p.setMaxHP(p.getMaxHP() / 1.15);
	case 1:
		p.setArmor(p.getArmor() - 1);
		p.setMaxHP(p.getMaxHP() / 1.1);
		break;
	}

	if (p.getHP() > p.getMaxHP())
		p.setHP(p.getMaxHP());
}
void Protection::upgrade(Dungeon &dungeon, Player &p) {
	if (getLevel() == 4)
		return;

	switch (getLevel()) {
	case 1:
		p.setLuck(p.getLuck() + 5);
		p.setMaxHP(p.getMaxHP() * 1.15);
		break;
	case 2:
		p.setArmor(p.getArmor() + 2);
		p.setBonusRoll(true);
		break;
	case 3:
		p.setMaxHP(p.getMaxHP() * 1.1);
		p.setFragileRetaliation(true);
		break;
	}

	increaseLevel();
}



//		CHESTS
Chests::Chests(Dungeon *dungeon, int x, int y, std::string chest, std::string image) : Objects(x, y, chest, image) {
	m_dungeon = dungeon;

	setSoundName("ChestOpening1.mp3");

	setSprite(dungeon->createSprite(x, y, y + Z_CHEST, getImageName()));
	dungeon->tileAt(getPos()).item = true;
	dungeon->tileAt(getPos()).wall = true;
}
Chests::~Chests() {
	if (m_item)
		m_item.reset();
}

void Chests::open(Dungeon &dungeon) {
	int x = getPosX();
	int y = getPosY();
	Coords currentPos = getPos();

	playSound(getSoundName());

	dungeon.removeItem(x, y);

	dungeon.tileAt(currentPos).object = m_item;
	dungeon.tileAt(currentPos).wall = false;
	dungeon.tileAt(currentPos).item = true;

	int z = dungeon.isShop() ? 2 : -1;
	dungeon.tileAt(currentPos).object->setSprite(dungeon.createSprite(currentPos, y + z, dungeon.tileAt(currentPos).object->getImageName()));
	dungeon.addItem(dungeon.tileAt(currentPos).object);

	openEffect(dungeon);
}
void Chests::openEffect(Dungeon &dungeon) {
	// If player has rigged chests from negative favor, roll to rig the chest
	if (dungeon.getPlayer()->getFavor() <= -4 && randReal(1, 100) > 80) {
		int x = getPosX();
		int y = getPosY();

		dungeon.addTrap(std::make_shared<ActiveBomb>(dungeon, x, y, 1));
	}
}

TreasureChest::TreasureChest(Dungeon *dungeon, int x, int y) : Chests(dungeon, x, y, TREASURE_CHEST, "Brown_Chest_48x48.png") {

}
void TreasureChest::open(Dungeon &dungeon) {
	int x = getPosX();
	int y = getPosY();
	Coords currentPos = getPos();

	playSound(getSoundName());

	dungeon.removeItem(x, y);

	int goldAmount = 20 + randInt(0, 20 + dungeon.getPlayer()->getLuck());
	dungeon.addGold(x, y, goldAmount);

	dungeon.tileAt(currentPos).wall = false;
}

LifeChest::LifeChest(Dungeon *dungeon, int x, int y) : Chests(dungeon, x, y, LIFE_CHEST, "Pink_Chest_48x48.png") {
	switch (randInt(1, 7)) {
	case 1: m_item = std::make_shared<LifePotion>(x, y); break;
	case 2:	m_item = std::make_shared<BigLifePotion>(x, y); break;
	case 3: m_item = std::make_shared<SteadyLifePotion>(x, y); break;
	case 4: m_item = std::make_shared<HalfLifePotion>(x, y); break;
	case 5: m_item = std::make_shared<BinaryLifePotion>(x, y); break;
	case 6: m_item = std::make_shared<RottenApple>(x, y); break;
	case 7: m_item = std::make_shared<SoulPotion>(x, y); break;
	default: break;
	}
}

BrownChest::BrownChest(Dungeon *dungeon, int x, int y) : Chests(dungeon, x, y, BROWN_CHEST, "Brown_Chest_48x48.png") {
	switch (randInt(1, 1)) {
	case 1: m_item = rollItem(dungeon, x, y, Rarity::COMMON, false, false); break;
	}
}

SilverChest::SilverChest(Dungeon *dungeon, int x, int y) : Chests(dungeon, x, y, SILVER_CHEST, "Silver_Chest_48x48.png") {
	switch (randInt(1, 4)) {
	case 1: m_item = rollPassive(dungeon, x, y, Rarity::UNCOMMON, true); break;
	case 2: m_item = rollWeapon(dungeon, x, y, Rarity::UNCOMMON, false); break;
	case 3: m_item = rollSpell(dungeon, x, y, Rarity::COMMON); break;
	case 4: m_item = rollItem(dungeon, x, y, Rarity::UNCOMMON, false, false); break;
	}
}

GoldenChest::GoldenChest(Dungeon *dungeon, int x, int y) : Chests(dungeon, x, y, GOLDEN_CHEST, "Golden_Chest_48x48.png") {
	switch (randInt(1, 2)) {
	case 1: m_item = rollPassive(dungeon, x, y, Rarity::RARE, false); break;
	case 2: m_item = rollWeapon(dungeon, x, y, Rarity::MYTHICAL, false); break;
	}
}

HauntedChest::HauntedChest(Dungeon *dungeon, int x, int y) : Chests(dungeon, x, y, HAUNTED_CHEST, "Golden_Chest_48x48.png") {
	switch (randInt(1, 3)) {
	case 1: m_item = rollPassive(dungeon, x, y, Rarity::RARE, true); break;
	case 2: m_item = rollWeapon(dungeon, x, y, Rarity::UNCOMMON, true); break;
	case 3: m_item = rollSpell(dungeon, x, y, Rarity::COMMON, true); break;
	}
}
void HauntedChest::openEffect(Dungeon &dungeon) {
	// Summon ghosts
	playSound("Devils_Gift.mp3");

	int x = getPosX();
	int y = getPosY();

	std::vector<Coords> coords;
	for (int i = x - 3; i < x + 4; i++) {
		for (int j = y - 3; j < y + 4; j++) {

			// Ignore all but the outer ring
			if (!dungeon.withinBounds(i, j) || (abs(i - x) <= 2 || abs(j - y) <= 2))
				continue;

			coords.push_back(Coords(i, j));
		}
	}

	int n = 3; // Number of ghosts to spawn
	while (!coords.empty() && n > 0) {
		int index = randInt(0, static_cast<int>(coords.size()) - 1);
		Coords pair = coords[index];
		coords.erase(coords.begin() + index);

		if (!dungeon.enemy(pair.x, pair.y)) {
			dungeon.addMonster(std::make_shared<Ghost>(&dungeon, pair.x, pair.y));

			n--;
		}
	}
}

TeleportingChest::TeleportingChest(Dungeon *dungeon, int x, int y) : Chests(dungeon, x, y, TELEPORTING_CHEST, "Golden_Chest_48x48.png") {
	m_teleports = randInt(1, 2);

	switch (randInt(1, 3)) {
	case 1: m_item = rollPassive(dungeon, x, y, Rarity::RARE, true); break;
	case 2: m_item = rollWeapon(dungeon, x, y, Rarity::RARE, true); break;
	case 3: m_item = rollSpell(dungeon, x, y, Rarity::COMMON, true); break;
	}
}
void TeleportingChest::attemptOpen(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	Coords currentPos = getPos();

	// Teleport if there are still teleports remaining
	if (m_teleports > 0) {
		playSound("Teleport_Spell.mp3");

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, currentPos, currentPos.y + Z_PROJECTILE);

		dungeon.tileAt(currentPos).item = false;
		dungeon.tileAt(currentPos).wall = false;

		int _x = randInt(1, cols - 2);
		int _y = randInt(1, rows - 2);

		while (dungeon.tileIsBlocked(_x, _y) || dungeon.trap(_x, _y) || dungeon.itemObject(_x, _y)) {
			_x = randInt(1, cols - 2);
			_y = randInt(1, rows - 2);
		}

		Coords newPos(_x, _y);

		dungeon.tileAt(newPos).item = true;
		dungeon.tileAt(newPos).wall = true;
		dungeon.tileAt(newPos).object = dungeon.tileAt(currentPos).object; // Assign chest to new position

		dungeon.tileAt(currentPos).object.reset();
		dungeon.tileAt(currentPos).object = nullptr;
		dungeon.queueMoveSprite(getSprite(), newPos, 0.0f);

		setPos(newPos);
		m_item->setPos(newPos);

		m_teleports--;
		return;
	}

	open(dungeon);
}

RiggedChest::RiggedChest(Dungeon *dungeon, int x, int y, std::string name, std::string image, int turns) : Chests(dungeon, x, y, name, image), m_turns(turns) {

}

ExplodingChest::ExplodingChest(Dungeon *dungeon, int x, int y) : RiggedChest(dungeon, x, y, EXPLODING_CHEST, "Golden_Chest_48x48.png", 1) {
	switch (randInt(1, 3)) {
	case 1: m_item = rollPassive(dungeon, x, y, Rarity::RARE, true); break;
	case 2: m_item = rollWeapon(dungeon, x, y, Rarity::UNCOMMON, true); break;
	case 3: m_item = rollSpell(dungeon, x, y, Rarity::COMMON, true); break;
	}
}
void ExplodingChest::openEffect(Dungeon &dungeon) {	
	int x = getPosX();
	int y = getPosY();

	dungeon.addTrap(std::make_shared<ActiveBomb>(dungeon, x, y, 1));
}

InfinityBox::InfinityBox(Dungeon *dungeon, int x, int y) : Chests(dungeon, x, y, INFINITY_BOX, "Golden_Chest_48x48.png") {
	int n = randInt(1, 4);
	switch (n) {
	case 1: m_item = std::make_shared<SteelPunch>(x, y); break;
	case 2: m_item = std::make_shared<VulcanSword>(x, y); break;
	case 3: m_item = std::make_shared<FrostShield>(x, y); break;
	case 4: m_item = std::make_shared<VulcanHammer>(x, y); break;
	default: break;
	}
}


//			:::: WEAPONS ::::
Weapon::Weapon(int x, int y, std::string name, std::string image, int dmg, int dexbonus)
	: Objects(x, y, name, image), m_dmg(dmg), m_dexbonus(dexbonus) {
	m_damageType = DamageType::NORMAL;
}
Weapon::Weapon(const Weapon &other) : Objects(other.getPosX(), other.getPosY(), other.getName(), other.getImageName()) {
	m_hasBonus = other.m_hasBonus;
	m_hasAbility = other.m_hasAbility;
	m_hasPattern = other.m_hasPattern;
	m_canBeCast = other.m_canBeCast;

	m_imbued = other.m_imbued;
	m_imbuement = other.m_imbuement;

	m_dmg = other.m_dmg;
	m_damageType = other.m_damageType;
	m_sharpness = other.m_sharpness;

	m_dexbonus = other.m_dexbonus;
}

int Weapon::getSharpnessBonus() {
	int damageBonus;

	if (m_sharpness == 0)
		return 0;

	damageBonus = (m_sharpness + 10) / 10;
	m_sharpness--;

	return damageBonus;
}

void Weapon::setImbuement(ImbuementType imbuement, int chance) {
	switch (imbuement) {
	case ImbuementType::BURNING:
		m_imbuement.burn = true;
		m_imbuement.burnChance += chance;
		break;
	case ImbuementType::POISONING:
		m_imbuement.poison = true;
		m_imbuement.poisonChance += chance;
		break;
	case ImbuementType::FREEZING:
		m_imbuement.freeze = true;
		m_imbuement.freezeChance += chance;
		break;
	}

	m_imbued = true;
}
int Weapon::getImbuementLevel(ImbuementType imbuement) const {
	switch (imbuement) {
	case ImbuementType::BURNING:
		return m_imbuement.burnChance;
	case ImbuementType::POISONING:
		return m_imbuement.poisonChance;
	case ImbuementType::FREEZING:
		return m_imbuement.freezeChance;
	default: return 0;
	}
}
void Weapon::useImbuement(Dungeon &dungeon, Actors &a) {

	if (m_imbuement.burn) {
		if (a.canBeBurned() || dungeon.getPlayer()->hasAfflictionOverride()) {
			if (randReal(1, 100) + dungeon.getPlayer()->getLuck() <= m_imbuement.burnChance) {
				int turns = 5 + (dungeon.getPlayer()->hasHarshAfflictions() ? 5 : 0);
				a.addAffliction(std::make_shared<Burn>(*dungeon.getPlayer(), turns));
			}
		}
	}

	if (m_imbuement.poison) {
		if (a.canBePoisoned() || dungeon.getPlayer()->hasAfflictionOverride()) {
			if (randReal(1, 100) + dungeon.getPlayer()->getLuck() <= m_imbuement.poisonChance) {
				int turns = 5 + (dungeon.getPlayer()->hasHarshAfflictions() ? 5 : 0);
				a.addAffliction(std::make_shared<Poison>(*dungeon.getPlayer(), turns, 3, 1, 1));
			}
		}
	}

	if (m_imbuement.freeze) {
		if (a.canBeFrozen() || dungeon.getPlayer()->hasAfflictionOverride()) {
			if (randReal(1, 100) + dungeon.getPlayer()->getLuck() <= m_imbuement.freezeChance) {
				int turns = 5 + (dungeon.getPlayer()->hasHarshAfflictions() ? 5 : 0);
				a.addAffliction(std::make_shared<Freeze>(turns));
				a.setFrozen(true);
			}
		}
	}
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


BronzeDagger::BronzeDagger(int x, int y) : Weapon(x, y, BRONZE_DAGGER, "Bronze_Dagger_48x48.png", 2, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setHasAbility(true);

	setDescription("It won't hurt as much, but it'll make them bleed.");
}
void BronzeDagger::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBled()) {
		if (randReal(1, 100) + dungeon.getPlayer()->getLuck() > m_bleedChance) {
			playSound("Dagger_Slice_Bleed.mp3");

			a.addAffliction(std::make_shared<Bleed>());
		}
	}
}

RustyCutlass::RustyCutlass(int x, int y) : Weapon(x, y, RUSTY_CUTLASS, "Rusty_Broadsword_48x48.png", 2, 2) {
	setSoundName("Weapon_Pickup2.mp3");
	setAttackPattern(true);

	m_durability = 15 + randInt(0, 5);

	setDescription("A rusty blade. It's seen better days.");
}
void RustyCutlass::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = dungeon.getPlayer()->facingDirection();
	char action = dungeon.getPlayer()->getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	int n = 0, m = 0;
	setDirectionalOffsets(move, n, m);

	if (dungeon.enemy(x + n, y + m)) {
		player.fightMonsterAt(Coords(x + n, y + m), getDamageType());

		// 1 Turn of invulnerability on attack attempt
		player.addAffliction(std::make_shared<Invulnerability>(1));

		m_durability--;

		if (m_durability == 0) {
			playSound("Crumble_Breaking.mp3");

			player.getWeapon().reset();
			player.setWeapon(std::make_shared<Hands>());
		}
	}
}

BoneAxe::BoneAxe(int x, int y) : Weapon(x, y, BONE_AXE, "Bone_Axe_48x48.png", 4, 0) {
	setSoundName("Weapon_Pickup2.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("A primitive item, but it gets the job done.\nThrowable.");
}
void BoneAxe::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(player.getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	// if it was a movement action
	if (isMovementAction(action)) {
		int range = 1;

		int n = 0, m = 0;
		int p = n, q = m; // For easier remembering of previous values
		setDirectionalOffsets(move, n, m);

		while (!dungeon.wall(x + n, y + m) && range <= 12) {

			if (dungeon.enemy(x + n, y + m)) {
				//playSound("Metal_Hit8.mp3");

				setDamage(getDamage() + 1); // damage boosted
				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
				setDamage(getDamage() - 1); // remove damage boost

				moveUsed = true;
				break;
			}

			p = n; q = m;

			incrementDirectionalOffsets(move, n, m);

			range++;
		}

		// Throw on the space in front of the enemy/wall/as far as it could go
		//playSound("Metal_Hit8.mp3");
		player.throwWeaponTo(x + p, y + q);
	}

	// effect to remove wind up effect
	untint(player.getSprite());

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

void Katana::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// If there was an attempted hit, then check to do special ability
	if (m_hit) {
		if (!m_waitOver)
			m_waitOver = true;
		else {
			playMiss();

			// Attack anything there
			for (unsigned int i = 0; i < coords.size(); i++) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, coords[i].x, coords[i].y, coords[i].y + Z_PROJECTILE);

				if (dungeon.enemy(coords[i].x, coords[i].y))
					player.fightMonsterAt(coords[i], getDamageType());
			}
			m_hit = false;
			m_waitOver = false;
		}
	}

	// if it was a movement action
	if (isMovementAction(move)) {

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		if (dungeon.enemy(x + n, y + m)) {
			player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
			moveUsed = true;

			if (!m_hit) {
				m_hit = true;

				setCoords(dungeon, player);
			}
		}
	}
}
void Katana::setCoords(Dungeon &dungeon, Player &player) {
	coords.clear();

	int x = player.getPosX();
	int y = player.getPosY();

	int n = 0, m = 0;
	setDirectionalOffsets(player.getAction(), n, m);

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

			coords.push_back(Coords(i, j));
		}
	}
}

GreaterKatana::GreaterKatana(int x, int y) : Katana(x, y, GREATER_KATANA, "Katana_48x48.png", 2, 3) {
	setDescription("Way of the Samurai. Measure once. Cut twice.");
}
void GreaterKatana::setCoords(Dungeon &dungeon, Player &player) {
	coords.clear();

	int x = player.getPosX();
	int y = player.getPosY();

	int n = 0, m = 0;
	setDirectionalOffsets(player.getAction(), n, m);

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

			coords.push_back(Coords(i, j));
		}
	}
}

SuperiorKatana::SuperiorKatana(int x, int y) : Katana(x, y, SUPERIOR_KATANA, "Katana_48x48.png", 2, 3) {
	setDescription("Way of the Samurai. Measure once. Cut twice.");
}
void SuperiorKatana::setCoords(Dungeon &dungeon, Player &player) {
	coords.clear();

	int x = player.getPosX();
	int y = player.getPosY();

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (i == x && j == y)
				continue;

			coords.push_back(Coords(i, j));
		}
	}
}

VulcanSword::VulcanSword(int x, int y) : Weapon(x, y, VULCAN_SWORD, "Vulcan_Sword_48x48.png", 3, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setHasAbility(true);

	setDescription("The blade is imbued with tiny embers that could melt\nthrough anything.");
}
void VulcanSword::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBurned()) {
		// 5% chance to burn
		if (randReal(1, 100) + dungeon.getPlayer()->getLuck() < m_burnChance) {
			playSound("Fire2.mp3");

			a.addAffliction(std::make_shared<Burn>(*dungeon.getPlayer(), 5));
		}
	}
}



//		LONG WEAPONS
LongWeapon::LongWeapon(int x, int y, int range, int dmg, int dex, int intellect, std::string name, std::string image) 
	: Weapon(x, y, name, image, dmg, dex), m_range(range) {
	setAttackPattern(true);
}
void LongWeapon::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	int range = getRange();

	// Check if weapon can be cast
	checkSpecial(dungeon, moveUsed);
	if (moveUsed)
		return;

	if (isMovementAction(move)) {

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		while (!dungeon.wall(x + n, y + m) && range > 0) {

			if (dungeon.enemy(x + n, y + m)) {
				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
				moveUsed = true;

				// If this weapon doesn't pierce, then stop and return
				if (!hasPiercing())
					return;
			}

			incrementDirectionalOffsets(move, n, m);

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
	a.increaseStatBy(StatType::INTELLECT, 2);
}
void EnchantedPike::unapplyBonus(Actors &a) {
	a.decreaseStatBy(StatType::INTELLECT, 2);
}

LuckyPike::LuckyPike(int x, int y) : LongWeapon(x, y, 2, 2, 2, 0, LUCKY_PIKE, "Iron_Longsword_48x48.png") {
	setSoundName("Weapon_Pickup.mp3");
	setHasBonus(true);

	setDescription("You feel lucky while holding this.");
}
void LuckyPike::applyBonus(Actors &a) {
	a.increaseStatBy(StatType::LUCK, 10);
}
void LuckyPike::unapplyBonus(Actors &a) {
	a.decreaseStatBy(StatType::LUCK, 10);
}


//		PIERCING WEAPONS
PiercingWeapon::PiercingWeapon(int x, int y, int range, int dmg, int dex, int intellect, std::string name, std::string image) 
	: LongWeapon(x, y, range, dmg, dex, intellect, name, image) {

}




//		BOWS
Bow::Bow(int x, int y, std::string name, std::string image, int damage, int dexbonus, int range) 
	: Weapon(x, y, name, image, damage, dexbonus), m_range(range) {
	setAttackPattern(true);
	setCast(true);
}

void Bow::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	// Check if player used weapon cast
	checkSpecial(dungeon, player, moveUsed);
	if (moveUsed)
		return;

	char move = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	if (isMovementAction(move)) {

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		if (dungeon.enemy(x + n, y + m)) {
			// if player is adjacent to the monster, then the bow's damage is reduced because you melee them
			setDamage(getDamage() - 2);
			player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
			moveUsed = true;
			setDamage(getDamage() + 2);
		}
	}
}
void Bow::checkSpecial(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char action = player.getAction();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(player.getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	char move = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	int range = getRange();

	// if it was a movement action
	if (isMovementAction(move)) {

		int n = 0, m = 0;
		switch (move) {
		case 'l': n = -1; m = 0; break;
		case 'r': n = 1; m = 0; break;
		case 'u': n = 0; m = -1; break;
		case 'd': n = 0; m = 1; break;
		}

		while (!dungeon.wall(x + n, y + m) && range > 0) {

			if (dungeon.enemy(x + n, y + m)) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1) {
					cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("ResizedAttack%04d.png", 4);
					dungeon.runSingleAnimation(frames, 30, x + n, y + m, 2);

					playEnemyHit();

					dungeon.damageMonster(pos, getDamage(), DamageType::PIERCING);
				}

				break;
			}

			switch (move) {
			case 'l': n--; m = 0; break;
			case 'r': n++; m = 0; break;
			case 'u': n = 0; m--; break;
			case 'd': n = 0; m++; break;
			}

			range--;
		}
	}

	// effect to remove wind up effect
	untint(player.getSprite());

	moveUsed = true;
	m_woundUp = false;
}

WoodBow::WoodBow(int x, int y) : Bow(x, y, WOOD_BOW, "Wood_Bow_32x32.png", 4, 2, 6) {
	setDescription("A classic long ranged weapon, but ineffective\n at close range.");
}

IronBow::IronBow(int x, int y) : Bow(x, y, IRON_BOW, "Reinforced_Bow_48x48.png", 5, 2, 6) {
	setDescription("This bow has been reinforced yet feels lighter.\n Not effective at close range.");
}

VulcanBow::VulcanBow(int x, int y) : Bow(x, y, VULCAN_BOW, "Vulcan_Bow_48x48.png", 6, 2, 8) {
	setHasAbility(true);

	setDescription("The legendary weapon smith's craftsmanship cannot\n go unnoticed. Doubtless that this bow will deliver.");
}
void VulcanBow::useAbility(Dungeon &dungeon, Actors &a) {
	if (a.canBeBurned()) {
		// 5% chance to burn
		if (randReal(1, 100) + dungeon.getPlayer()->getLuck() < m_burnChance) {
			// play burned sound effect
			playSound("Fire2.mp3");

			a.addAffliction(std::make_shared<Burn>(*dungeon.getPlayer(), 5));
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
void Estoc::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(player.getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	// if it was a movement action
	if (!player.isStuck() && isMovementAction(action)) {
		bool firstHit = true;
		int range = 1;
		int pos;

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		while (!dungeon.wall(x + n, y + m) && range <= m_limit) {

			if (dungeon.enemy(x + n, y + m)) {

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

				setDamage(getDamage() + damageBoost); // damage boosted
				player.fightMonsterAt(Coords(x + n, y + m), DamageType::PIERCING);
				setDamage(getDamage() - damageBoost); // remove damage boost

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
						player.moveTo(p, q);

						if (dungeon.trap(p, q))
							dungeon.trapEncounter(p, q);

					}

					range -= 1; // Decrease range by one to maintain the same damage boost on any piercing done
					firstHit = false;

					// Piercing if Greater + Superior only.
					if (!canPierce())
						break;
				}

			}
			else if (!dungeon.enemy(x + n, y + m) && !firstHit && !isSuperior())
				break;

			incrementDirectionalOffsets(move, n, m);

			range++;
		}

	}

	// effect to remove wind up effect
	untint(player.getSprite());

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
void Zweihander::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// Check if player used weapon cast
	checkSpecial(dungeon, player, moveUsed);
	if (moveUsed)
		return;

	// if it was a movement action
	if (isMovementAction(move)) {

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		// Player attacked to the side, so check above and below
		if (m == 0) {

			if (dungeon.enemy(x + n, y + m)) {
				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
				moveUsed = true;
			}
			if (dungeon.enemy(x + n, y + 1)) {
				player.fightMonsterAt(Coords(x + n, y + 1), getDamageType());
				moveUsed = true;
			}
			if (dungeon.enemy(x + n, y - 1)) {
				player.fightMonsterAt(Coords(x + n, y - 1), getDamageType());
				moveUsed = true;
			}
		}
		// Else player attacked above or below, so check the sides
		else {

			if (dungeon.enemy(x + n, y + m)) {
				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
				moveUsed = true;
			}
			if (dungeon.enemy(x + 1, y + m)) {
				player.fightMonsterAt(Coords(x + 1, y + m), getDamageType());
				moveUsed = true;
			}
			if (dungeon.enemy(x - 1, y + m)) {
				player.fightMonsterAt(Coords(x - 1, y + m), getDamageType());
				moveUsed = true;
			}
		}

		// Superior effect
		if (!moveUsed && isSuperior()) {
			int range = 2;
			int pos = -1;

			while (!dungeon.wall(x + n, y + m) && range > 0) {

				if (dungeon.enemy(x + n, y + m)) {
					setDamage(getDamage() + 2);
					player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
					setDamage(getDamage() - 2);

					pos = dungeon.findMonster(x + n, y + m);
					moveUsed = true;
				}

				incrementDirectionalOffsets(move, n, m);

				range--;

				if (moveUsed)
					break;
			}

			// Knock the monster back one tile
			/*if (pos != -1) {
				if (!(wall || enemy))
					dungeon.monsterAt(pos)->moveTo(x + n, y + m);
			}*/
		}
	}
}

GreaterZweihander::GreaterZweihander(int x, int y) : Zweihander(x, y, GREATER_ZWEIHANDER, "Long_Sword_48x48.png", 2, 2) {
	setCast(true);

	setDescription("Provides excellent reach both laterally and literally.");
}
void GreaterZweihander::checkSpecial(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(player.getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	if (isMovementAction(action)) {

		int range = 2;
		int pos = -1;

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		while (!dungeon.wall(x + n, y + m) && range > 0) {

			if (dungeon.enemy(x + n, y + m)) {
				setDamage(getDamage() + 2);
				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
				setDamage(getDamage() - 2);

				pos = dungeon.findMonster(x + n, y + m);
				moveUsed = true;
			}

			incrementDirectionalOffsets(move, n, m);

			range--;

			if (moveUsed)
				break;
		}

		// Knock the monster back one tile
		/*if (pos != -1) {
			if (!(wall || enemy))
				dungeon.monsterAt(pos)->moveTo(x + n, y + m);
		}*/
	}

	// effect to remove wind up effect
	untint(player.getSprite());

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

	setDescription("Tools such as this were popular among bandits.\nGrab enemies and pull them in close for fatal damage.");
}
void Claw::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(player.getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	if (isMovementAction(action)) {

		int range = 1;
		int pos = -1;

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		// Find an enemy to hook
		while (!dungeon.wall(x + n, y + m) && range <= 4) {

			if (dungeon.enemy(x + n, y + m)) {
				playSound("Dagger_Slice_Bleed.mp3", 0.7f);

				pos = dungeon.findMonster(x + n, y + m);
				moveUsed = true;
				break;
			}

			incrementDirectionalOffsets(move, n, m);
			range++;
		}

		// Now pull them in
		if (moveUsed && pos != -1) {

			// Reversed movement since we're now moving toward the player
			switch (move) {
			case 'l': n++; m = 0; break;
			case 'r': n--; m = 0; break;
			case 'u': n = 0; m++; break;
			case 'd': n = 0; m--; break;
			}

			// If enemy was farther than one tile away, pull them toward the player
			while (!dungeon.hero(x + n, y + m) && range > 1) {

				// Move the monster toward the player
				dungeon.monsterAt(pos)->moveTo(x + n, y + m, 0.045f);

				// And check for traps along the way
				if (dungeon.trap(x + n, y + m)) {
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

				range--;
			}

			// Stun monster for one turn at the end
			if (pos != -1) {
				if (dungeon.monsterAt(pos)->canBeStunned())
					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
			}
		}
	}

	// effect to remove wind up effect
	untint(player.getSprite());

	moveUsed = true;
	m_woundUp = false;
}
void Claw::useAbility(Dungeon &dungeon, Actors &a) {

	// if weapon was primed, apply stun
	if (m_woundUp && a.canBeStunned()) {
		a.addAffliction(std::make_shared<Stun>(1));
	}

	if (a.canBeBled()) {
		// 10% chance to bleed
		if (randReal(1, 100) - dungeon.getPlayer()->getLuck() <= m_bleedChance) {
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
void Hammer::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(player.getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	if (isMovementAction(action)) {

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		if (dungeon.enemy(x + n, y + m)) {
			// play fire blast explosion sound effect
			playSound("Fireblast_Spell2.mp3");

			int damageBoost = 3;
			if (TStun()) damageBoost = 4;
			else if (boxStun()) damageBoost = 5;

			setDamage(getDamage() + damageBoost); // damage boosted
			int pos = dungeon.findMonster(x + n, y + m);
			if (pos != -1 && dungeon.monsterAt(pos)->canBeStunned())
				dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

			player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
			setDamage(getDamage() - damageBoost); // remove damage boost

			// Greater effect
			if (TStun())
				TPattern(dungeon, player);

			// Superior effect
			if (boxStun())
				boxPattern(dungeon, player);
		}
	}

	// effect to remove wind up effect
	untint(player.getSprite());

	moveUsed = true;
	m_woundUp = false;
}
void Hammer::TPattern(Dungeon &dungeon, Player &player) {
	int x = player.getPosX();
	int y = player.getPosY();

	char move = player.facingDirection();

	int n = 0, m = 0;
	setDirectionalOffsets(move, n, m);

	setDamage(getDamage() + 1);

	std::vector<Coords> coords;
	// Player attacked to the side, so check above and below
	if (m == 0) {

		// Attacked left, so check one beyond that
		if (n == -1)
			coords.push_back(Coords(x + n - 1, y + m));

		// Attacked right, so check one beyond that
		if (n == 1)
			coords.push_back(Coords(x + n + 1, y + m));

		coords.push_back(Coords(x + n, y + 1));
		coords.push_back(Coords(x + n, y - 1));
	}
	// Else player attacked above or below, so check the sides
	else {
		// Attacked upward
		if (m == -1)
			coords.push_back(Coords(x + n, y + m - 1));

		// Attacked downward
		if (m == 1)
			coords.push_back(Coords(x + n, y + m + 1));

		coords.push_back(Coords(x + 1, y + m));
		coords.push_back(Coords(x - 1, y + m));
	}

	for (unsigned int i = 0; i < coords.size(); i++) {
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		dungeon.runSingleAnimation(frames, 120, coords[i].x, coords[i].y, coords[i].y + Z_PROJECTILE);

		if (dungeon.enemy(coords[i].x, coords[i].y)) {
			int pos = dungeon.findMonster(coords[i].x, coords[i].y);
			if (pos != -1) {
				if (dungeon.monsterAt(pos)->canBeStunned() && randReal(1, 100) + dungeon.getPlayer()->getLuck() > 50)
					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

				player.fightMonsterAt(coords[i], getDamageType());
			}
		}
	}

	setDamage(getDamage() - 1);
}
void Hammer::boxPattern(Dungeon &dungeon, Player &player) {
	int x = player.getPosX();
	int y = player.getPosY();

	char move = player.facingDirection();

	int n, m; // n : x, m : y
	switch (move) {
	case 'l': n = -1; m = -1; break;
	case 'r': n = 1; m = -1; break;
	case 'u': n = -1; m = -1; break;
	case 'd': n = -1; m = 1; break;
	}

	int range = 2;
	int currentRange = 1;

	int k = 1; // 3 rows/columns to check
	while (k <= 3) {

		while (!dungeon.wall(x + n, y + m) && currentRange <= range) {

			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

			if (dungeon.enemy(x + n, y + m)) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1) {
					if (dungeon.monsterAt(pos)->canBeStunned())
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

					dungeon.damageMonster(pos, getDamage(), DamageType::NORMAL);
				}
			}

			switch (move) {
			case 'l': n--; break;
			case 'r': n++; break;
			case 'u': m--; break;
			case 'd': m++; break;
			}

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
	if (a.canBeStunned() && m_woundUp)
		a.addAffliction(std::make_shared<Stun>(1));
	
	if (a.canBeBurned()) {
		// 5% chance to burn
		if (randReal(1, 100) - dungeon.getPlayer()->getLuck() < m_burnChance) {
			// play burned sound effect
			playSound("Fire2.mp3", 0.7f);

			a.addAffliction(std::make_shared<Burn>(*dungeon.getPlayer(), 5));
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
		a.increaseStatBy(StatType::INTELLECT, 3);
		m_bonusApplied = true;
	}
}
void ArcaneStaff::unapplyBonus(Actors &a) {
	if (m_bonusApplied) {
		a.decreaseStatBy(StatType::INTELLECT, 3);
		m_bonusApplied = false;
		m_isCast = false; // allows casting if weapon was switched
	}
}
void ArcaneStaff::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// if player didn't cast, then return
	if (!m_isCast && action != WIND_UP) {
		m_isCast = false;
		return;
	}

	// if staff is not cast and player has cast, then do so and return
	if (!m_isCast && action == WIND_UP) {
		// effect to show wind up
		tintStaffCast(player.getSprite());

		playSound("Staff_Cast1.mp3");

		// boost int
		giveBonus(player);

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
	case 'd': unapplyBonus(player); m_isCast = false; break;
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
void BoStaff::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	int range = 1;
	int pos;

	if (isMovementAction(move)) {

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		while (!dungeon.wall(x + n, y + m) && range > 0) {

			if (dungeon.enemy(x + n, y + m)) {
				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
				moveUsed = true;
				pos = dungeon.findMonster(x + n, y + m);

				// Superior Bo Staff knockback
				if (pos != -1 && canPush())
					dungeon.linearActorPush(x + n, y + m, 1, 1, move);
			}

			incrementDirectionalOffsets(move, n, m);

			range--;

			if (moveUsed)
				break;
		}

		// If there was a hit, try to knock the enemy back
		if (moveUsed) {

			// Knock enemy in front of you back if not Superior Bo Staff
			if (!(dungeon.wall(x + n, y + m) || dungeon.enemy(x + n, y + m)) && !canPush()) {
				if (pos != -1)
					dungeon.monsterAt(pos)->moveTo(x + n, y + m);
			}

			// Now check behind the player for an enemy
			switch (move) {
			case 'l': n = 1; m = 0; break;
			case 'r': n = -1; m = 0; break;
			case 'u': n = 0; m = 1; break;
			case 'd': n = 0; m = -1; break;
			}

			// If there's an enemy, hit them
			if (dungeon.enemy(x + n, y + m)) {

				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
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

						dungeon.linearActorPush(x + n, y + m, 1, 1, move);
						return;
					}

					switch (move) {
					case 'l': n++; m = 0; break;
					case 'r': n--; m = 0; break;
					case 'u': n = 0; m++; break;
					case 'd': n = 0; m--; break;
					}

					if (!(dungeon.wall(x + n, y + m) || dungeon.enemy(x + n, y + m)))
						dungeon.monsterAt(pos)->moveTo(x + n, y + m);
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
			if (dungeon.enemy(x + n, y + m)) {

				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
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

						dungeon.linearActorPush(x + n, y + m, 1, 1, move);
						return;
					}

					switch (move) {
					case 'l': n++; m = 0; break;
					case 'r': n--; m = 0; break;
					case 'u': n = 0; m++; break;
					case 'd': n = 0; m--; break;
					}

					if (!(dungeon.wall(x + n, y + m) || dungeon.enemy(x + n, y + m)))
						dungeon.monsterAt(pos)->moveTo(x + n, y + m);
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
void Nunchuks::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	if (isMovementAction(move)) {

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		// If an enemy is hit, consume the action
		if (dungeon.enemy(x + n, y + m)) {
			player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
			moveUsed = true;

			if (!stationaryStun())
				return;
		}

		// Superior effect
		if (!dungeon.enemy(x + n, y + m) && moreStun()) {
			int a, b, c, d;
			switch (move) {
			case 'l': a = -1; b = -1; c = -1; d = 1; break;
			case 'r': a = 1; b = -1; c = 1; d = 1; break;
			case 'u':a = -1; b = -1; c = 1; d = -1; break;
			case 'd': a = -1; b = 1; c = 1; d = 1; break;
			}

			if (dungeon.enemy(x + a, y + b)) {
				int pos = dungeon.findMonster(x + a, y + b);
				if (pos != -1 && dungeon.monsterAt(pos)->canBeStunned()) {
					// Stun sprite effect
					playEnemyHit();
					gotStunned(dungeon.monsterAt(pos)->getSprite());

					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
				}
			}
			if (dungeon.enemy(x + c, y + d)) {
				int pos = dungeon.findMonster(x + c, y + d);
				if (pos != -1 && dungeon.monsterAt(pos)->canBeStunned()) {
					// Stun sprite effect
					playEnemyHit();
					gotStunned(dungeon.monsterAt(pos)->getSprite());

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

		if (dungeon.enemy(x + a, y + b)) {
			int pos = dungeon.findMonster(x + a, y + b);
			if (pos != -1 && dungeon.monsterAt(pos)->canBeStunned()) {
				// Stun sprite effect
				playEnemyHit();
				gotStunned(dungeon.monsterAt(pos)->getSprite());

				dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
			}
		}
		if (dungeon.enemy(x + c, y + d)) {
			int pos = dungeon.findMonster(x + c, y + d);
			if (pos != -1 && dungeon.monsterAt(pos)->canBeStunned()) {
				// Stun sprite effect
				playEnemyHit();
				gotStunned(dungeon.monsterAt(pos)->getSprite());

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

	m_durability = 15 + randInt(0, 10);

	setDescription("The greatest ability of all. Susceptible to breaking.");
}
void Mattock::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	if (isMovementAction(move)) {
		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		if (dungeon.wallObject(x + n, y + m) && dungeon.wallObject(x + n, y + m)->isDestructible()) {

			playSound("Metal_Hit8.mp3");

			dungeon.destroyWall(x + n, y + m);

			m_durability--;

			// If mattock broke, unequip the mattock and drop a mattock head
			if (m_durability == 0) {
				player.setWeapon(std::make_shared<Hands>());

				if (dungeon.itemObject(x, y))
					dungeon.itemHash(x, y);

				Coords pos(x, y);
				
				dungeon.tileAt(pos).item = true;
				dungeon.tileAt(pos).object = std::make_shared<MattockHead>(x, y);
				dungeon.tileAt(pos).object->setSprite(dungeon.createSprite(pos, pos.y + Z_ITEM, dungeon.tileAt(pos).object->getImageName()));
				dungeon.addItem(dungeon.tileAt(pos).object);
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

	m_durability = 5 + randInt(0, 5);

	setDescription("It seems we weren't careful were we. At least you\ncan throw it.");
}
void MattockHead::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// if player didn't prime, then return
	if (!m_woundUp && action != WIND_UP) {
		m_woundUp = false;
		return;
	}

	// if weapon is not primed and player primed, then do so and return
	if (!m_woundUp && action == WIND_UP) {
		// effect to show wind up
		tintItemCast(player.getSprite());

		playSound("FootStepGeneric2.mp3");

		m_woundUp = true;
		moveUsed = true;
		return;
	}

	if (isMovementAction(action)) {
		int range = 1;

		int n = 0, m = 0;
		int p = n, q = m; // For easier remembering of previous values
		setDirectionalOffsets(move, n, m);

		while (!dungeon.wall(x + n, y + m) && range <= 12) {

			if (dungeon.enemy(x + n, y + m)) {
				playSound("Metal_Hit8.mp3");

				setDamage(getDamage() + 4); // damage boosted
				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
				setDamage(getDamage() - 4); // remove damage boost

				moveUsed = true;
				break;
			}

			p = n; q = m;

			incrementDirectionalOffsets(move, n, m);

			range++;
		}

		m_durability--;

		// If durability hits zero, then replace the mattock head with mattock dust
		if (m_durability == 0) {
			player.setWeapon(std::make_shared<Hands>());
			if (dungeon.itemObject(x, y))
				dungeon.itemHash(x, y);

			Coords pos(x, y);

			dungeon.tileAt(pos).item = true;
			dungeon.tileAt(pos).object = std::make_shared<MattockDust>(x, y);
			dungeon.tileAt(pos).object->setSprite(dungeon.createSprite(pos, pos.y + Z_ITEM, dungeon.tileAt(pos).object->getImageName()));
			dungeon.addItem(dungeon.tileAt(pos).object);
		}
		else {
			// Throw the mattock head on the space in front of the enemy/wall/as far as it could go
			player.throwWeaponTo(x + p, y + q);
		}
	}

	// effect to remove wind up effect
	untint(player.getSprite());

	moveUsed = true;
	m_woundUp = false;
}

Pistol::Pistol(int x, int y) : Weapon(x, y, PISTOL, "Vulcan_Dagger_48x48.png", 1, 0) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("Items of this mechanical nature were largely\ndestroyed in the times of magic. It looks like\nthis one survived, but it is severely damaged\nand can only hold two rounds at a time.");
}
void Pistol::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

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
		tintItemCast(player.getSprite());

		playSound("Fireblast_Spell1.mp3");

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		while (!dungeon.wall(x + n, y + m)) {

			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

			if (dungeon.enemy(x + n, y + m)) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1) {
					if (dungeon.monsterAt(pos)->canBeBled())
						dungeon.giveAffliction(pos, std::make_shared<Bleed>());

					dungeon.damageMonster(pos, 6, DamageType::PIERCING);
				}

				moveUsed = true;
				break;
			}

			incrementDirectionalOffsets(move, n, m);
		}
	}

	m_rounds--;
	if (m_rounds == 0) {
		m_reloading = true;

		setCast(false); // Removes label from HUD
	}

	// effect to remove wind up effect
	untint(player.getSprite());

	moveUsed = true;
}

Whip::Whip(int x, int y) : Weapon(x, y, WHIP, "Reinforced_Bow_48x48.png", 2, 2) {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);

	setDescription("The weapon of choice among all archaeological\nadventure lovers. More effective if strikes are\nmaximum range.");
}
void Whip::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	int range = 1;
	int pos;

	if (isMovementAction(move)) {

		int n = 0, m = 0;
		setDirectionalOffsets(move, n, m);

		while (!dungeon.wall(x + n, y + m) && range <= 3) {

			if (dungeon.enemy(x + n, y + m)) {

				// Sweetspot bonus
				if (range == 3)
					setDamage(getDamage() + 1);

				player.fightMonsterAt(Coords(x + n, y + m), getDamageType());

				// Remove sweetspot bonus
				if (range == 3)
					setDamage(getDamage() - 1);

				pos = dungeon.findMonster(x + n, y + m);

				moveUsed = true;
			}

			incrementDirectionalOffsets(move, n, m);

			if (moveUsed)
				break;

			range++;
		}

		// Attempt knockback by one tile if enemy was hit by the sweetspot
		if (moveUsed && range == 3 && !(dungeon.wall(x + n, y + m) || dungeon.enemy(x + n, y + m))) {
			if (pos != -1) {
				if (!dungeon.monsterAt(pos)->isHeavy())
					dungeon.monsterAt(pos)->moveTo(x + n, y + m);
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
void Jian::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// If action was not the special ability, do nothing and return
	if (action != WIND_UP)
		return;

	// Check if player tried to use the special ability
	if (action == WIND_UP) {

		// If not enough charge, then return
		if (getCharge() < getMaxCharge())
			return;

		// effect to show wind up
		tintItemCast(player.getSprite());

		playSound("FootStepGeneric2.mp3");

		for (int i = x - 1; i < x + 2; i++) {
			for (int j = y - 1; j < y + 2; j++) {

				// Ignore player's spot
				if (i == x && j == y)
					continue;

				cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);

				if (dungeon.enemy(i, j)) {

					/*if (canBleed()) {
						int pos = dungeon.findMonster(i, j);
						if (pos != -1)
							if (dungeon.monsterAt(pos)->canBeBled())
								dungeon.giveAffliction(pos, std::make_shared<Bleed>());
					}*/

					float playerHealthPercentage = player.getHP() / (float)player.getMaxHP();

					// If player has at least 90% of their hp remaining, the special ability does extra damage
					if (hasBonusDamage() && playerHealthPercentage >= 0.9f)
						setDamage(getDamage() + 5);

					int pos = dungeon.findMonster(i, j);
					dungeon.damageMonster(pos, getDamage(), DamageType::NORMAL);

					// Do knockback and stun if SuperiorJian
					if (pos != -1 && hasKnockback()) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));

						char move = getFacingDirectionRelativeTo(Coords(x, y), Coords(i, j));
						dungeon.linearActorPush(i, j, 1, 1, move);
					}

					if (hasBonusDamage() && playerHealthPercentage >= 0.9f)
						setDamage(getDamage() - 5);
				}
			}
		}

	}

	resetCharge();
	setCast(false); // Removes label from HUD

	playSound("Wind_Spell2.mp3");

	// effect to remove wind up effect
	untint(player.getSprite());

	moveUsed = true;
}

GreaterJian::GreaterJian(int x, int y) : Jian(x, y, GREATER_JIAN, "Long_Sword_48x48.png", 7, 2, 2) {
	setDescription("This Jian is capable of overflowing with energy\nif the hand behind it is healthy enough. Special deals\nincreased damage if health is high enough.");
}

SuperiorJian::SuperiorJian(int x, int y) : Jian(x, y, SUPERIOR_JIAN, "Long_Sword_48x48.png", 7, 3, 2) {
	setDescription("The ultimate version of the Jian. In addition,\ncan force enemies back and stun them.");
}

Boomerang::Boomerang(int x, int y) : MeterWeapon(x, y, 5, 2, 2, BOOMERANG, "Long_Sword_48x48.png") {
	setSoundName("Weapon_Pickup.mp3");
	setAttackPattern(true);
	setCast(true);

	setDescription("Always comes back because of the aura of this place,\nnot because of your skill. Its magical properties\nallow it to retrieve items from afar.");
}
void Boomerang::usePattern(Dungeon &dungeon, Player &player, bool &moveUsed) {
	char move = player.facingDirection();
	char action = player.getAction();

	int x = player.getPosX();
	int y = player.getPosY();

	// If action was not the special ability, do nothing and return
	if (action != WIND_UP)
		return;

	// Check if player tried to use the special ability
	if (action == WIND_UP) {

		// If not enough charge, then return
		if (getCharge() < getMaxCharge())
			return;

		// effect to show wind up
		tintItemCast(player.getSprite());

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

			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			dungeon.runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

			// Get items, if any
			if (dungeon.itemObject(x + n, y + m)) {

				// If it was a chest, open it
				if (dungeon.itemObject(x + n, y + m)->isChest()) {
					std::shared_ptr<Chests> chest = std::dynamic_pointer_cast<Chests>(dungeon.tileAt(Coords(x + n, y + m)).object);
					chest->attemptOpen(dungeon);
					chest.reset();
				}
				// Otherwise bring it to the player
				else {
					int _x = x, _y = y;
					if (dungeon.itemObject(_x, _y))
						dungeon.itemHash(_x, _y);

					Coords pos(_x, _y);

					dungeon.tileAt(pos).item = true;
					dungeon.tileAt(pos).object = dungeon.tileAt(Coords(x + n, y + m)).object;
					dungeon.tileAt(pos).object->setSprite(dungeon.createSprite(pos, pos.y + Z_ITEM, dungeon.tileAt(pos).object->getImageName()));
					dungeon.addItem(dungeon.tileAt(pos).object);

					dungeon.removeItem(x + n, y + m);
				}
			}

			if (dungeon.enemy(x + n, y + m)) {
				int pos = dungeon.findMonster(x + n, y + m);
				if (pos != -1) {
					dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
					player.fightMonsterAt(Coords(x + n, y + m), getDamageType());
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
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x - 1, y + m, y + m + Z_PROJECTILE);
				dungeon.runSingleAnimation(frames, 120, x + 1, y + m, y + m + Z_PROJECTILE);

				if (dungeon.withinBounds(x - 1, y + m) && dungeon.enemy(x - 1, y + m)) {
					int pos = dungeon.findMonster(x - 1, y + m);
					if (pos != -1) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
						player.fightMonsterAt(Coords(x - 1, y + m), getDamageType());
					}
				}
				if (dungeon.withinBounds(x + 1, y + m) && dungeon.enemy(x + 1, y + m)) {
					int pos = dungeon.findMonster(x + 1, y + m);
					if (pos != -1) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
						player.fightMonsterAt(Coords(x + 1, y + m), getDamageType());
					}
				}
			}
			// Threw it to the sides
			else if (m == 0) {

				cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				dungeon.runSingleAnimation(frames, 120, x + n, y - 1, y - 1 + Z_PROJECTILE);
				dungeon.runSingleAnimation(frames, 120, x + n, y + 1, y + 1 + Z_PROJECTILE);

				if (dungeon.withinBounds(x + n, y - 1) && dungeon.enemy(x + n, y - 1)) {
					int pos = dungeon.findMonster(x + n, y - 1);
					if (pos != -1) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
						player.fightMonsterAt(Coords(x + n, y - 1), getDamageType());
					}
				}
				if (dungeon.withinBounds(x + n, y + 1) && dungeon.enemy(x + n, y + 1)) {
					int pos = dungeon.findMonster(x + n, y + 1);
					if (pos != -1) {
						dungeon.giveAffliction(pos, std::make_shared<Stun>(1));
						player.fightMonsterAt(Coords(x + n, y + 1), getDamageType());
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
	untint(player.getSprite());

	moveUsed = true;
}


//	:::: SHIELDS ::::
Shield::Shield(int x, int y, int defense, int durability, int coverage, std::string type, std::string image)
	: Objects(x, y, type, image), m_defense(defense), m_durability(durability), m_coverage(coverage) {
	m_max_durability = durability;
	
}

WoodShield::WoodShield(int x, int y) : Shield(x, y, 2, 15, 1, WOOD_SHIELD, "Wood_Shield_48x48.png") {
	setDescription("A wood shield. Can block projectiles and close combat\n attacks, but be careful that it doesn't break.");
}

IronShield::IronShield(int x, int y) : Shield(x, y, 4, 40, 1, IRON_SHIELD, "Iron_Shield_48x48.png") {
	setDescription("A sturdier shield, capable of withstanding stronger blows.");
}

ThornedShield::ThornedShield(int x, int y) : Shield(x, y, 3, 40, 2, THORNED_SHIELD, "Thorned_Shield_48x48.png") {
	setDescription("This prickly shield will surely give foes\na piece of your mind.");
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
		playSound("Spike_Hit.mp3");

		// damaged effect
		runMonsterDamage(a.getSprite());

		a.setHP(a.getHP() - 1);
	}
}

FrostShield::FrostShield(int x, int y) : Shield(x, y, 3, 30, 1, FROST_SHIELD, "Frost_Shield_48x48.png") {
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
		playSound("IceBreak7.mp3");

		// add freeze
		a.addAffliction(std::make_shared<Freeze>(3));
	}
}

ReflectShield::ReflectShield(int x, int y) : Shield(x, y, 2, 25, 1, REFLECT_SHIELD, "Holy_Shield_48x48.png") {
	setDescription("A shield with truly superior reflective capabilities.\nCan deflect projectiles back at enemies.");
}
void ReflectShield::useAbility(Dungeon &dungeon, Actors &a) {
	int px = dungeon.getPlayer()->getPosX();
	int py = dungeon.getPlayer()->getPosY();
	int ax = a.getPosX();
	int ay = a.getPosY();

	// if attacker is at a distance to player, it reflects and damages them
	if ((abs(ax - px) > 1 || abs(ay - py) > 1)) {
		// sound effect
		playSound("Enemy_Hit.mp3");

		// damaged effect
		runMonsterDamage(a.getSprite());

		a.setHP(a.getHP() - a.getStr());
	}
}


// ==========================================
//				:::: TRAPS ::::
// ==========================================
Traps::Traps(Dungeon *dungeon, int x, int y, std::string name, std::string image, int damage) : Objects(x, y, name, image), m_damage(damage) {
	m_dungeon = dungeon;

	dungeon->tileAt(getPos()).trap = true;
}

void Traps::moveTo(int x, int y, float time) {
	Coords currentPos = getPos();
	Coords newPos(x, y);

	if (m_dungeon->countTrapNumber(currentPos.x, currentPos.y) <= 1)
		m_dungeon->tileAt(currentPos).trap = false;

	m_dungeon->tileAt(newPos).trap = true;

	m_dungeon->queueMoveSprite(getSprite(), newPos, time);
	setPos(newPos);
}
void Traps::destroyTrap() {
	int x = getPosX();
	int y = getPosY();

	Coords currentPos = getPos();

	if (actsAsWall())
		m_dungeon->tileAt(currentPos).wall = false;

	// Remove any light sources, if could possibly provide light (like Braziers)
	if (isLightSource())
		m_dungeon->removeLightSource(x, y, getName());

	// if there are NOT multiple traps on this location, then unflag this spot
	if (m_dungeon->countTrapNumber(x, y) <= 1)
		m_dungeon->tileAt(currentPos).trap = false;

	spriteCleanup();

	drops();

	setDestroyed(true);
}
void Traps::spriteCleanup() {
	if (getSprite() != nullptr) {
		m_dungeon->queueRemoveSprite(getSprite());
		setSprite(nullptr);
	}
}

void Traps::checkExplosion(int x, int y) {
	if (m_dungeon->trap(x, y)) {
		std::vector<int> indexes = m_dungeon->findTraps(x, y);
		for (size_t i = 0; i < indexes.size(); ++i) {

			if (m_dungeon->trapAt(indexes.at(i))->isDestructible())
				m_dungeon->trapAt(indexes.at(i))->destroyTrap();

			else if (m_dungeon->trapAt(indexes.at(i))->isExplosive())
				m_dungeon->trapAt(indexes.at(i))->explode();
		}
	}
}
void Traps::checkBurn(int x, int y) {
	if (m_dungeon->trap(x, y)) {
		std::vector<int> indexes = m_dungeon->findTraps(x, y);
		for (size_t i = 0; i < indexes.size(); ++i)
			if (m_dungeon->trapAt(indexes.at(i))->canBeIgnited())
				m_dungeon->trapAt(indexes.at(i))->ignite();
	}
}
void Traps::checkPoison(int x, int y) {
	if (m_dungeon->trap(x, y)) {
		std::vector<int> indexes = m_dungeon->findTraps(x, y);
		for (size_t i = 0; i < indexes.size(); ++i)
			if (m_dungeon->trapAt(indexes.at(i))->canBePoisoned())
				m_dungeon->trapAt(indexes.at(i))->poison();
	}
}
void Traps::checkDouse(int x, int y) {
	if (m_dungeon->trap(x, y)) {
		std::vector<int> indexes = m_dungeon->findTraps(x, y);
		for (size_t i = 0; i < indexes.size(); ++i)
			if (m_dungeon->trapAt(indexes.at(i))->canBeDoused())
				m_dungeon->trapAt(indexes.at(i))->douse();
	}
}
void Traps::checkFreeze(int x, int y) {
	if (m_dungeon->trap(x, y)) {
		std::vector<int> indexes = m_dungeon->findTraps(x, y);
		for (size_t i = 0; i < indexes.size(); ++i)
			if (m_dungeon->trapAt(indexes.at(i))->canBeFrozen())
				m_dungeon->trapAt(indexes.at(i))->freeze();
	}
}

//		DEVILS WATER
DevilsWater::DevilsWater(Dungeon *dungeon, int x, int y) : Traps(dungeon, x, y, DEVILS_WATER, "Water_Tile1_48x48.png", 0) {
	setSprite(dungeon->createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}

void DevilsWater::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	checkDouse(x, y);

	if (!m_firstUse && (px != m_x || py != m_y)) {
		m_firstUse = true;
		m_x = px;
		m_y = py;
	}
}
void DevilsWater::trapAction(Actors &a) {
	// TODO: Ensure this works with clones.

	if (!a.isPlayer())
		return;

	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	if (m_dungeon->hero(x, y)) {
		if (a.isBurned()) {
			playSound("Relief_Female.mp3");

			a.setBurned(false);
			a.removeAffliction(BURN);
		}
	}

	SecondFloor *floor = dynamic_cast<SecondFloor*>(m_dungeon);
	if (floor->watersUsed() || floor->getWaterPrompt() || !m_firstUse)
		return;

	playSound("Puddle_Splash.mp3");
	
	floor->devilsWaterPrompt();
	m_firstUse = false;
	m_x = px;
	m_y = py;
}


//		BRAZIER
Brazier::Brazier(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, BRAZIER, "Gold_Goblet_48x48.png", 0) {
	setWallFlag(true);
	setCanBeIgnited(true);
	setDestructible(true);

	setSprite(m_dungeon->createSprite(x, y, y + Z_WALL, getImageName()));

	dungeon.tileAt(getPos()).wall = true;
}
Brazier::~Brazier() {
	if (m_flame != nullptr)
		m_flame->removeFromParent();
}

void Brazier::trapAction(Actors &a) {

	// If not lit or it was tipped, do nothing
	if (!m_lit || m_tipped)
		return;

	// Braziers can be tipped over to leave a 3x3 grid of Embers

	int x = getPosX();
	int y = getPosY();

	playSound("Fireblast_Spell2.mp3");

	m_dungeon->removeLightSource(x, y, getName());
	m_flame->removeFromParent();
	m_flame = nullptr;
	m_tipped = true; // So you can't tip it over multiple times

	char move = m_dungeon->getPlayer()->facingDirection();

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

			if (m_dungeon->withinBounds(x + n, y + m)) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

				if (m_dungeon->enemy(x + n, y + m)) {
					int pos = m_dungeon->findMonster(x + n, y + m);
					if (pos != -1) {
						// Ignite them
						if (m_dungeon->monsterAt(pos)->canBeBurned() || m_dungeon->getPlayer()->hasAfflictionOverride())
							m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), 5));
					}
				}

				checkBurn(x + n, y + m);

				// Create an Ember here if there's no wall
				if (!m_dungeon->wall(x + n, y + m))
					m_dungeon->addTrap(std::make_shared<Ember>(*m_dungeon, x + n, y + m, 5 + randInt(0, 7)));
				
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
void Brazier::ignite() {

	// If already lit, do nothing
	if (m_lit)
		return;

	m_lit = true;

	m_dungeon->addLightSource(getPosX(), getPosY(), 6, getName());

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Fire%04d.png", 8);
	m_flame = m_dungeon->runAnimationForever(frames, 24, getPosX(), getPosY(), getPosY() + Z_PROJECTILE);
	m_flame->setScale(0.6f * GLOBAL_SPRITE_SCALE);

	float x = getPosX();
	float y = getPosY() - 0.35f;
	m_dungeon->queueMoveSprite(m_flame, x, y);
}


//		PIT
Pit::Pit(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, PIT, "Pit_48x48.png", 1000) {
	setLethal(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}

void Pit::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	// Remove any gold on top of the pit
	if (m_dungeon->gold(x, y) > 0) {
		playSound("Falling_In_A_Hole.mp3", m_dungeon->getPlayer()->getPos(), getPos());

		auto scale = cocos2d::ScaleTo::create(0.4f, 0.0f);
		auto fade = cocos2d::FadeOut::create(0.3f);
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(scale);
		v.pushBack(fade);

		m_dungeon->removeGoldWithActions(x, y, v);
	}

	// Remove any items on top of the pit
	if (m_dungeon->item(x, y)) {
		playSound("Falling_In_A_Hole.mp3", m_dungeon->getPlayer()->getPos(), getPos());

		auto scale = cocos2d::ScaleTo::create(0.4f, 0.0f);
		auto fade = cocos2d::FadeOut::create(0.3f);
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(scale);
		v.pushBack(fade);

		m_dungeon->removeItemWithActions(x, y, v);
	}

	if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1)
			trapAction(*m_dungeon->monsterAt(pos));
	}

	if (m_dungeon->hero(x, y))
		trapAction(*m_dungeon->getPlayerAt(x, y));
}
void Pit::trapAction(Actors &a) {
	if (a.isFlying())
		return;

	if (a.isPlayer()) {
		playSound("Female_Falling_Scream_License.mp3");

		// instant death from falling
		a.setHP(0);
		a.setSuperDead(true);

		deathFade(a.getSprite());
	}
	else {
		Monster &m = dynamic_cast<Monster&>(a);

		playMonsterDeathByPit(*m_dungeon->getPlayer(), m);

		float duration = 1.0f;
		auto scale = cocos2d::ScaleTo::create(duration, 0.0f);
		auto fade = cocos2d::FadeOut::create(duration);
		auto move = cocos2d::MoveBy::create(0.5f, cocos2d::Vec2(0, -SPACING_FACTOR / 4));

		cocos2d::Vector<cocos2d::FiniteTimeAction*> actions;
		actions.pushBack(scale);
		actions.pushBack(fade);
		actions.pushBack(move);

		m_dungeon->queueCustomAction(a.getSprite(), cocos2d::Spawn::create(actions));

		// death animation
		/*auto scale = cocos2d::ScaleTo::create(0.4f, 0.0f);
		auto fade = cocos2d::FadeOut::create(0.3f);
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(scale);
		v.pushBack(fade);
		
		m_dungeon->queueCustomAction(a.getSprite(), cocos2d::Spawn::create(v));*/

		m.setDestroyed(true);
	}
}

//		FALLING SPIKES
FallingSpike::FallingSpike(Dungeon &dungeon, int x, int y, int speed) : Traps(&dungeon, x, y, FALLING_SPIKE, "CeilingSpike_48x48.png", 3), m_speed(speed) {
	setSprite(dungeon.createSprite(x, y, 0, getImageName()));
}
FallingSpike::~FallingSpike() {
	if (getSprite() != nullptr)
		getSprite()->removeFromParent();
}

void FallingSpike::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	int range = m_speed; // How many tiles it will travel
	int currentRange = -1;

	while (currentRange < range) {

		currentRange++;

		if (m_dungeon->wall(x, y + currentRange))
			break;
		
		if (m_dungeon->hero(x, y + currentRange)) {
			playSound("Spike_Hit.mp3");

			m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);

			break;
		}
	}

	moveTo(x, y + currentRange);

	if (m_dungeon->wall(x, y + currentRange) || m_dungeon->hero(x, y + currentRange))
		destroyTrap();
}
void FallingSpike::trapAction(Actors &a) {

	if (!a.isPlayer())
		return;

	playSound("Spike_Hit.mp3");
	m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);

	destroyTrap();
}

//		SPIKES
Spikes::Spikes(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, SPIKES, "Spiketrap_Active_48x48.png", 7) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}

void Spikes::trapAction(Actors &a) {
	if (a.isFlying())
		return;

	int x = getPosX();
	int y = getPosY();

	if (a.isPlayer()) {
		if (!m_dungeon->getPlayer()->spikeImmunity()) {
			playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), getPos());

			m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);
		}
		else if (m_dungeon->getPlayer()->spikeImmunity()) {
			playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), getPos()); // Replace with appropriate sound
		}

		return;
	}
	
	playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), getPos());

	m_dungeon->damageMonster(a.getPos(), getDamage(), DamageType::NORMAL);
}

//		SPIKE TRAPS
SpikeTrap::SpikeTrap(Dungeon &dungeon, int x, int y, int speed) : Traps(&dungeon, x, y, AUTOSPIKE_DEACTIVE, "Spiketrap_Deactive_48x48.png", 3), m_cyclespeed(speed), m_countdown(speed) {
	m_deactive = dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, "Spiketrap_Deactive_48x48.png");
	m_primed = dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, "Spiketrap_Primed_48x48.png");
	m_active = dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, "Spiketrap_Active_48x48.png");

	setSprite(m_deactive);
	setSpriteVisibility(false, false, false);
}
SpikeTrap::~SpikeTrap() {
	if (m_deactive != nullptr)
		m_deactive->removeFromParent();

	if (m_primed != nullptr)
		m_primed->removeFromParent();

	if (m_active != nullptr)
		m_active->removeFromParent();
}

void SpikeTrap::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	// if spiketrap was primed, reset the counter
	if (m_countdown == 0 && m_active->isVisible()) {
		setSprite(m_deactive);
		setSpriteVisibility(true, false, false);

		m_countdown = m_cyclespeed;

		//return;
	}

	// spiketrap is counting down
	if (m_countdown > 1) {
		m_countdown--;
	}
	// prime the spiketrap
	else if (m_countdown == 1) {
		// spiketrap is primed
		playSound("Spiketrap_Primed.mp3", m_dungeon->getPlayer()->getPos(), getPos());

		setSprite(m_primed);
		setSpriteVisibility(false, true, false);

		m_countdown--;
	}
	// spiketrap is launched
	else {
		playSound("Spiketrap_Active.mp3", m_dungeon->getPlayer()->getPos(), getPos());

		setSprite(m_active);
		setSpriteVisibility(false, false, true);

		if (m_dungeon->hero(x, y)) {

			if (!(m_dungeon->getPlayerAt(x, y)->isFlying() || m_dungeon->getPlayerAt(x, y)->spikeImmunity())) {
				playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), getPos());

				m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);
			}
			else if (m_dungeon->getPlayerAt(x, y)->spikeImmunity()) {
				playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), getPos()); // Replace with appropriate sound
			}
		}

		if (m_dungeon->enemy(x, y)) {
			int pos = m_dungeon->findMonster(x, y);
			if (pos != -1)
				if (!m_dungeon->monsterAt(pos)->isFlying())
					m_dungeon->damageMonster(pos, getDamage(), DamageType::NORMAL);
		}
	}

}
void SpikeTrap::trapAction(Actors &a) {
	int x = getPosX();
	int y = getPosY();

	if (m_countdown == 0 && !a.isFlying()) {
		if (a.isPlayer()) {
			if (!(a.isFlying() || m_dungeon->getPlayerAt(x, y)->spikeImmunity())) {
				playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), getPos());

				m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);
			}
			else if (m_dungeon->getPlayerAt(x, y)->spikeImmunity()) {
				playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), getPos()); // Replace with appropriate sound
			}

			return;
		}
		else {
			playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), getPos());

			int pos = m_dungeon->findMonster(x, y);
			if (pos != -1)			
				m_dungeon->damageMonster(pos, getDamage(), DamageType::NORMAL);
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
	else {
		m_deactive->setVisible(false);
		m_primed->setVisible(false);
		m_active->setVisible(false);
	}
}

//		TRIGGERED SPIKES
TriggerSpike::TriggerSpike(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, TRIGGERSPIKE_DEACTIVE, "Spiketrap_Deactive_48x48.png", 5), m_triggered(false) {
	m_deactive = dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, "Spiketrap_Deactive_48x48.png");
	m_primed = dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, "Spiketrap_Primed_48x48.png");
	m_active = dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, "Spiketrap_Active_48x48.png");

	setSprite(m_deactive);
	setSpriteVisibility(false, false, false);
}
TriggerSpike::~TriggerSpike() {
	if (m_deactive != nullptr)
		m_deactive->removeFromParent();

	if (m_primed != nullptr)
		m_primed->removeFromParent();

	if (m_active != nullptr)
		m_active->removeFromParent();
}

void TriggerSpike::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	// if the spike trap was triggered
	if (m_triggered) {
		playSound("Spiketrap_Active.mp3", m_dungeon->getPlayer()->getPos(), getPos());

		setSprite(m_active);
		setSpriteVisibility(false, false, true);

		if (m_dungeon->hero(x, y))
			trapAction(*m_dungeon->getPlayerAt(x, y));
		
		if (m_dungeon->enemy(x, y)) {
			int pos = m_dungeon->findMonster(x, y);
			if (pos != -1)
				trapAction(*m_dungeon->monsterAt(pos));
		}

		toggleTrigger();
	}
	else {
		// else if the trap wasn't triggered, check if player is standing on top of it
		if (m_dungeon->hero(x, y) || m_dungeon->enemy(x, y)) {

			if (m_dungeon->hero(x, y)) {
				if (m_dungeon->getPlayerAt(x, y)->isFlying())
					return;
			}

			if (m_dungeon->enemy(x, y)) {
				int pos = m_dungeon->findMonster(getPos());
				if (pos != -1) {
					if (m_dungeon->monsterAt(pos)->isFlying())
						return;
				}
			}

			toggleTrigger();

			// spiketrap is primed
			playSound("Spiketrap_Primed.mp3", m_dungeon->getPlayer()->getPos(), getPos());

			setSprite(m_primed);
			setSpriteVisibility(false, true, false);
		}
		else {
			// retract spiketrap
			//playSound("Spiketrap_Deactive.mp3"); // This sound does not exist

			setSprite(m_deactive);
			setSpriteVisibility(true, false, false);
		}
	}
}
void TriggerSpike::trapAction(Actors &a) {
	Coords coords = getPos();

	if (m_active->isVisible() && !a.isFlying()) {
		if (a.isPlayer()) {
			// If no flying or spike immunity, damage them
			if (!(a.isFlying() || m_dungeon->getPlayerAt(coords)->spikeImmunity())) {
				playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), coords);

				m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);
			}
			else if (m_dungeon->getPlayerAt(coords)->spikeImmunity()) {
				playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), coords); // Replace with appropriate sound
			}

			return;
		}
		else {
			playSound("Spike_Hit.mp3", m_dungeon->getPlayer()->getPos(), coords);

			int pos = m_dungeon->findMonster(coords);
			if (pos != -1)
				m_dungeon->damageMonster(pos, getDamage(), DamageType::NORMAL);
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

//		PUDDLES
Puddle::Puddle(Dungeon &dungeon, int x, int y, int turns) : Traps(&dungeon, x, y, PUDDLE, "Puddle.png", 0), m_turns(turns) {
	setDestructible(true);
	setCanBeFrozen(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
}
Puddle::Puddle(Dungeon &dungeon, int x, int y, int turns, std::string name, std::string image) : Traps(&dungeon, x, y, name, image, 0), m_turns(turns) {
	setImageName(image);
	setDestructible(true);
}

void Puddle::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->trap(x, y)) {
		std::vector<int> indexes = m_dungeon->findTraps(x, y);
		for (int i = 0; i < static_cast<int>(indexes.size()); i++) {
			if (m_dungeon->trapAt(indexes.at(i))->canBeDoused()) {
				m_dungeon->trapAt(indexes.at(i))->douse();

				destroyTrap();
				return;
			}
		}
	}

	// Does not dissipate
	if (m_turns == -1)
		return;

	if (m_turns > 0)
		m_turns--;
	else
		destroyTrap();

}
void Puddle::trapAction(Actors &a) {
	int x = getPosX();
	int y = getPosY();

	if (a.isFlying())
		return;

	playSound("Puddle_Splash.mp3", m_dungeon->getPlayer()->getPos(), getPos());

	destroyTrap();

	if (a.isBurned()) {
		playSound("Relief_Female.mp3");

		a.setBurned(false);
		a.removeAffliction(BURN);
	}

	if (randReal(1, 100) - m_dungeon->getPlayer()->getLuck() < 40) {

		if (a.canBeStunned()) {
			playSound("Puddle_Slip.mp3", m_dungeon->getPlayer()->getPos(), getPos());

			// Turn sprite sideways
			a.getSprite()->setRotation(90);
			a.addAffliction(std::make_shared<Slipped>());
			
			a.addAffliction(std::make_shared<Stun>());
		}

		specialAction(a);

		return;
	}
}
void Puddle::freeze() {
	if (isDestroyed())
		return;

	m_dungeon->addTrap(std::make_shared<FrozenPuddle>(*m_dungeon, getPosX(), getPosY()));

	destroyTrap();
}
void Puddle::spriteCleanup() {
	if (getSprite() != nullptr) {
		int x = getPosX();
		int y = getPosY();

		if (m_dungeon->enemy(x, y) || m_dungeon->hero(x, y)) {
			m_dungeon->queueRemoveSprite(getSprite());
			setSprite(nullptr);

			return;
		}

		auto fade = cocos2d::FadeOut::create(0.2f);
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(fade);

		m_dungeon->queueRemoveSpriteWithMultipleActions(getSprite(), v);
		setSprite(nullptr);
	}
}

PoisonPuddle::PoisonPuddle(Dungeon &dungeon, int x, int y, int turns) : Puddle(dungeon, x, y, turns, POISON_PUDDLE, "Poison_Puddle.png") {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
}

void PoisonPuddle::specialAction(Actors &a) {
	// if actor can be poisoned, attempt to poison
	if (a.canBePoisoned()) {
		// 40% chance to be poisoned
		if (randReal(1, 100) + a.getLuck() < 60)
			a.addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 5, 3, 1, 1));		
	}
}

FrozenPuddle::FrozenPuddle(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, FROZEN_PUDDLE, "Puddle.png", 0) {
	setDestructible(true);
	setCanBeIgnited(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
}

void FrozenPuddle::activeTrapAction() {
	if (m_turns > 0) {
		m_turns--;
		return;
	}

	ignite();
}
void FrozenPuddle::trapAction(Actors &a) {
	int x = getPosX();
	int y = getPosY();

	if (a.isFlying())
		return;

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	playCrumble(exp(-(abs(px - x) + abs(py - y))) / 3);

	if (randReal(1, 100) < 90) {

		if (a.canBeStunned()) {
			a.getSprite()->setRotation(90);
			a.addAffliction(std::make_shared<Slipped>());

			a.addAffliction(std::make_shared<Stun>(1));
		}
	}
}
void FrozenPuddle::ignite() {
	if (isDestroyed())
		return;

	m_dungeon->addTrap(std::make_shared<Puddle>(*m_dungeon, getPosX(), getPosY()));

	destroyTrap();
}

//		WATER
Water::Water(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, WATER, "Water_Tile1_48x48.png", 0) {
	setCanBePoisoned(true);
	setCanBeFrozen(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
	getSprite()->setOpacity(170);

	if (randReal(1, 100) > 90)
		dungeon.addMonster(std::make_shared<Piranha>(&dungeon, x, y));
}
Water::Water(Dungeon &dungeon, int x, int y, std::string name, std::string image) : Traps(&dungeon, x, y, name, image, 0) {
	
}

void Water::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	checkDouse(x, y);

	specialActiveAction();

	if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1) {
			if (!m_dungeon->monsterAt(pos)->isFlying() && m_dungeon->monsterAt(pos)->isBurned()) {
				m_dungeon->monsterAt(pos)->setBurned(false);
				m_dungeon->monsterAt(pos)->removeAffliction(BURN);

				m_dungeon->giveAffliction(pos, std::make_shared<Wet>(3));
			}
		}
	}

	if (m_dungeon->hero(x, y)) {	
		if (!m_dungeon->getPlayer()->isFlying() && m_dungeon->getPlayer()->isBurned()) {
			playSound("Relief_Female.mp3");

			m_dungeon->getPlayer()->setBurned(false);
			m_dungeon->getPlayer()->removeAffliction(BURN);

			m_dungeon->getPlayer()->addAffliction(std::make_shared<Wet>(3));
		}	
	}
}
void Water::trapAction(Actors &a) {
	if (a.isFlying())
		return;

	int x = getPosX();
	int y = getPosY();

	if (a.isPlayer())
		playSound("Puddle_Splash.mp3", 0.5f);

	if (a.isPoisoned() && canBePoisoned()) {
		poison();
		return;
	}

	if (a.isBurned()) {
		if (a.isPlayer())
			playSound("Relief_Female.mp3");

		a.setBurned(false);
		a.removeAffliction(BURN);
	}
	
	specialAction(a);

	a.addAffliction(std::make_shared<Wet>(3));
}
void Water::poison() {
	if (isDestroyed())
		return;

	m_dungeon->addTrap(std::make_shared<PoisonWater>(*m_dungeon, getPosX(), getPosY()));

	destroyTrap();
}
void Water::freeze() {
	if (isDestroyed())
		return;

	m_dungeon->addTrap(std::make_shared<Ice>(*m_dungeon, getPosX(), getPosY()));

	destroyTrap();
}

PoisonWater::PoisonWater(Dungeon &dungeon, int x, int y) : Water(dungeon, x, y, POISON_WATER, "PoisonWater_48x48.png") {
	setCanBeFrozen(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
	getSprite()->setOpacity(170);
}

void PoisonWater::specialActiveAction() {

	if (m_wait) {
		m_wait = false;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	std::vector<Coords> coords;
	coords.push_back(Coords(x - 1, y));
	coords.push_back(Coords(x + 1, y));
	coords.push_back(Coords(x, y - 1));
	coords.push_back(Coords(x, y + 1));

	while (!coords.empty()) {
		int index = randInt(0, static_cast<int>(coords.size()) - 1);
		int n = coords[index].x;
		int m = coords[index].y;

		checkPoison(n, m);

		coords.erase(coords.begin() + index);
	}

	m_wait = true;
}
void PoisonWater::specialAction(Actors &a) {
	if (a.canBePoisoned())
		a.addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 6, 3, 1, 1));	
}

Ice::Ice(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, ICE, "Puddle.png", 0) {
	setCanBeIgnited(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
}

void Ice::trapAction(Actors &a) {
	int x = getPosX();
	int y = getPosY();

	if (a.isFlying())
		return;

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	playCrumble(exp(-(abs(px - x) + abs(py - y))) / 3);

	if (randReal(1, 100) < 90) {

		if (a.canBeStunned()) {
			a.getSprite()->setRotation(90);
			a.addAffliction(std::make_shared<Slipped>(0));

			a.addAffliction(std::make_shared<Stun>(1));
		}
	}
}
void Ice::ignite() {
	if (isDestroyed())
		return;

	m_dungeon->addTrap(std::make_shared<Water>(*m_dungeon, getPosX(), getPosY()));

	destroyTrap();
}

//		POISON MISTER
PoisonMister::PoisonMister(Dungeon &dungeon, int x, int y, int wait) : Traps(&dungeon, x, y, POISON_MISTER, "Cheese_Wedge_48x48.png", 0), m_wait(wait), m_maxWait(m_wait) {
	setSprite(dungeon.createSprite(x, y, y + Z_FLOATERS, getImageName()));
}

void PoisonMister::activeTrapAction() {
	if (m_wait > 0) {
		m_wait--;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	playSound("Bomb_Fuse2.mp3", m_dungeon->getPlayer()->getPos(), getPos());

	m_dungeon->addTrap(std::make_shared<PoisonCloud>(*m_dungeon, x, y, 5));

	m_wait = m_maxWait;
}

//		STALACTITE
Stalactite::Stalactite(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, STALACTITE, "Cheese_Wedge_48x48.png", 8) {
	m_stalactiteY = y - 9;
	setSprite(dungeon.createSprite(x, m_stalactiteY, m_stalactiteY + Z_FLOATERS + 3, getImageName()));
	getSprite()->setVisible(false);
	dungeon.addSprite(x, y, y + Z_TRAP_MIDDLE, "candy1.png");
}

void Stalactite::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();
	Coords start(getPos());

	Coords playerCoords = m_dungeon->getPlayerCoordsClosestTo(start);

	if (!m_activated && start == playerCoords) {
		playSound("Button_Pressed.mp3");

		getSprite()->setVisible(true);
		m_activated = true;
		return;
	}

	if (m_activated) {
		if (m_wait > 0) {
			m_wait--;
			m_dungeon->queueMoveSprite(getSprite(), x, m_stalactiteY += 3);

			return;
		}

		m_dungeon->queueMoveSprite(getSprite(), x, m_stalactiteY += 3);

		if (m_dungeon->hero(x, y))
			m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);

		if (m_dungeon->enemy(x, y))
			m_dungeon->damageMonster(start, getDamage(), DamageType::NORMAL);

		destroyTrap();
	}
}

//		GIANT CRYSTAL
GiantCrystal::GiantCrystal(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, GIANT_CRYSTAL, "Cheese_Wedge_48x48.png", 0) {
	setWallFlag(true);
	setDestructible(true);
	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	dungeon.tileAt(getPos()).wall = true;
}

void GiantCrystal::drops() {
	m_dungeon->addTrap(std::make_shared<BrokenCrystals>(*m_dungeon, getPosX(), getPosY()));
}
void GiantCrystal::trapAction(Actors &a) {
	playHitSmasher();

	if (a.getStr() >= m_strengthCutoff) {
		m_strength--;
	}

	if (m_strength == 0) {
		destroyTrap();
	}
}

BrokenCrystals::BrokenCrystals(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, BROKEN_CRYSTALS, "Cheese_Wedge_48x48.png", 5) {
	setDestructible(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
}

void BrokenCrystals::trapAction(Actors &a) {
	playSound("Skeleton_Key_Broken.mp3", m_dungeon->getPlayer()->getPos(), a.getPos());

	if (a.isPlayer()) {
		m_dungeon->damagePlayer(getDamage(), DamageType::PIERCING);
		if (a.canBeBled())
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Bleed>(6));
	}

	int pos = m_dungeon->findMonster(a.getPos());
	if (pos != -1) {
		m_dungeon->damageMonster(pos, getDamage(), DamageType::NORMAL);
		if (a.canBeBled())
			m_dungeon->giveAffliction(pos, std::make_shared<Bleed>(6));
	}

	destroyTrap();
}

//		TREE
Tree::Tree(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, TREE, "Cheese_Wedge_48x48.png", 0) {
	setWallFlag(true);
	setCanBeIgnited(true);
	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	createRoots();

	dungeon.tileAt(getPos()).wall = true;
}

void Tree::createRoots() {
	int x = getPosX();
	int y = getPosY();
	
	std::vector<Coords> coords;

	// Create vector of (x, y) pairs to choose from
	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {
			coords.push_back(Coords(i, j));
		}
	}

	int n, m;
	// Try to create N roots in random locations around the tree
	for (int i = 0; i < 3; i++) {

		if (static_cast<int>(m_roots.size()) >= 3)
			break;

		int index = randInt(0, static_cast<int>(coords.size()) - 1);
		Coords coord = coords[index];
		n = coord.x;
		m = coord.y;

		if (!m_dungeon->withinBounds(n, m)) {
			coords.erase(coords.begin() + index);
			continue;
		}

		if (!(m_dungeon->wall(n, m) || m_dungeon->trap(n, m))) {
			m_roots.push_back(Coords(n, m));

			m_dungeon->tileAt(Coords(n, m)).trap = true;
			std::shared_ptr<TreeRoot> root = std::make_shared<TreeRoot>(*m_dungeon, n, m);
			m_dungeon->addTrap(root);

			coords.erase(coords.begin() + index);
		}
	}
}

void Tree::activeTrapAction() {
	if (!m_lit)
		return;

	int x = getPosX();
	int y = getPosY();

	if (m_litTurns == 0) {
		m_dungeon->removeLightSource(x, y, getName());
		destroyTrap();
		return;
	}

	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {

			if (!m_dungeon->withinBounds(i, j) || i == x && j == y)
				continue;

			if (randReal(1, 100) > 90)
				checkBurn(i, j);

			if (m_dungeon->hero(i, j)) {
				if (randReal(1, 100) - m_dungeon->getPlayer()->getLuck() > 85 && m_dungeon->getPlayer()->canBeBurned())
					m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 6));
				
			}

			if (m_dungeon->enemy(i, j)) {
				int pos = m_dungeon->findMonster(i, j);
				if (pos != -1) 
					if (randReal(1, 100) > 85 && m_dungeon->monsterAt(pos)->canBeBurned())
						m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), 6));
									
			}
		}
	}

	m_litTurns--;
}
void Tree::trapAction(Actors &a) {
	// Shake tree
	playSound("Grass2.mp3");
}
void Tree::ignite() {
	// If already lit, do nothing
	if (m_lit)
		return;

	m_lit = true;

	m_dungeon->addLightSource(getPosX(), getPosY(), 6, getName());

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Fire%04d.png", 8);
	cocos2d::Sprite* m_flame = m_dungeon->runAnimationForever(frames, 24, getPosX(), getPosY(), 2);
	m_flame->setScale(0.6f * GLOBAL_SPRITE_SCALE);
	m_flames.push_back(m_flame);

	float x = getPosX();
	float y = getPosY() - 0.35f;
	m_dungeon->queueMoveSprite(m_flame, x, y, 0.0f);
}
void Tree::spriteCleanup() {
	m_dungeon->queueRemoveSprite(getSprite());
	setSprite(nullptr);

	for (auto &it : m_flames)
		m_dungeon->queueRemoveSprite(it);
}

TreeRoot::TreeRoot(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, TREE_ROOT, "Bone_32x32.png", 0) {
	setCanBeIgnited(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_TOP, getImageName()));
}

void TreeRoot::activeTrapAction() {
	if (!m_lit)
		return;

	int x = getPosX();
	int y = getPosY();

	if (m_litTurns == 0) {
		m_dungeon->removeLightSource(x, y, getName());
		destroyTrap();
		return;
	}

	checkBurn(x, y);

	if (m_dungeon->hero(x, y)) {
		if (m_dungeon->getPlayer()->canBeBurned())
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 6));
	}

	if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1)
			if (m_dungeon->monsterAt(pos)->canBeBurned())
				m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), 6));
	}

	m_litTurns--;
}
void TreeRoot::trapAction(Actors &a) {
	if (a.isFlying())
		return;

	if (randReal(1, 100) + a.getLuck() <= 10) {

		if (a.canBeStunned()) {
			playMiss();
			a.getSprite()->setRotation(90);
			a.addAffliction(std::make_shared<Slipped>());

			a.addAffliction(std::make_shared<Stun>(1));
		}
	}
}
void TreeRoot::ignite() {
	// If already lit, do nothing
	if (m_lit)
		return;

	m_lit = true;

	m_dungeon->addLightSource(getPosX(), getPosY(), 3, getName());

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Fire%04d.png", 8);
	m_flame = m_dungeon->runAnimationForever(frames, 24, getPosX(), getPosY(), 2);
	m_flame->setScale(0.6f * GLOBAL_SPRITE_SCALE);

	float x = getPosX();
	float y = getPosY() - 0.35f;
	m_dungeon->queueMoveSprite(m_flame, x, y, 0.0f);
}
void TreeRoot::spriteCleanup() {
	m_dungeon->queueRemoveSprite(getSprite());
	setSprite(nullptr);

	if (m_flame != nullptr)
		m_dungeon->queueRemoveSprite(m_flame);
}

MalevolentPlant::MalevolentPlant(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, MALEVOLENT_PLANT, "Cheese_Wedge_48x48.png", 4) {
	setCanBeIgnited(true);

	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	setWallFlag(true);
	dungeon.tileAt(getPos()).wall = true;
}

void MalevolentPlant::activeTrapAction() {
	if (randReal(1, 100) > 90) {
		m_primed = true;
		return;
	}

	if (!m_primed)
		return;

	Coords start(getPos());
	Coords playerCoords = m_dungeon->getPlayerCoordsClosestTo(start);

	if (positionIsAdjacent(start, playerCoords, true))
		m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);
	
	m_primed = false;
}
void MalevolentPlant::trapAction(Actors &a) {
	playSound("Grass2.mp3");
	destroyTrap();
}
void MalevolentPlant::ignite() {
	int x = getPosX();
	int y = getPosY();

	if (isDestroyed())
		return;

	destroyTrap();

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
	m_dungeon->runSingleAnimation(frames, 120, x, y, y + Z_PROJECTILE);

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (m_dungeon->enemy(i, j) || m_dungeon->trap(i, j))
				m_dungeon->runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);

			checkBurn(i, j);

			if (m_dungeon->enemy(i, j)) {
				int pos = m_dungeon->findMonster(i, j);
				if (pos != -1) {
					if (m_dungeon->monsterAt(pos)->canBeBurned())
						m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), 8));					
				}
			}
		}
	}

	// Create an ember where it was
	m_dungeon->addTrap(std::make_shared<Ember>(*m_dungeon, x, y, 6 + randInt(0, 2)));
}

//		FIREBAR
Firebar::Firebar(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, FIREBAR, "Firebar_Totem_48x48.png", 5) {
	m_clockwise = randInt(0, 1);
	m_angle = randInt(1, 8);

	auto frames = getAnimationFrameVector("Fire%04d.png", 8);

	auto inner = dungeon.runAnimationForever(frames, 24, x, y, y + Z_PROJECTILE - 1);
	inner->setScale(0.75f * GLOBAL_SPRITE_SCALE);
	auto outer = dungeon.runAnimationForever(frames, 24, x, y, y + Z_PROJECTILE - 1);
	outer->setScale(0.75f * GLOBAL_SPRITE_SCALE);

	m_innerFire = std::make_shared<Objects>();
	m_innerFire->setSprite(inner);
	m_outerFire = std::make_shared<Objects>();
	m_outerFire->setSprite(outer);

	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	setInitialFirePosition(x, y);

	setWallFlag(true);
	setExtraSpritesFlag(true);
	setEmitsLight(true);

	dungeon.tileAt(getPos()).wall = true;
}
Firebar::Firebar(Dungeon &dungeon, int x, int y, std::string firebar) : Traps(&dungeon, x, y, firebar, "Firebar_Totem_48x48.png", 5) {
	m_clockwise = randInt(0, 1);
	m_angle = randInt(1, 8);

	setWallFlag(true);
	setExtraSpritesFlag(true);
	setEmitsLight(true);

	dungeon.tileAt(getPos()).wall = true;
}

void Firebar::activeTrapAction() {
	int rows = m_dungeon->getRows();
	int cols = m_dungeon->getCols();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	if (m_innerActive > 0) {
		m_innerActive--;
		if (m_innerActive == 0) {
			m_dungeon->queueCustomAction(m_innerFire->getSprite(), cocos2d::ScaleTo::create(0.05f, 0.75f * GLOBAL_SPRITE_SCALE));
		}
	}

	if (m_outerActive > 0) {
		m_outerActive--;
		if (m_outerActive == 0) {
			m_dungeon->queueCustomAction(m_outerFire->getSprite(), cocos2d::ScaleTo::create(0.05f, 0.75f * GLOBAL_SPRITE_SCALE));
		}
	}

	// reset projectiles to visible
	m_innerFire->getSprite()->setVisible(true);
	m_outerFire->getSprite()->setVisible(true);

	/* 8 1 2
	*  7 X 3
	*  6 5 4
	*  moves firebar -to- position m_angle, it is not where it currently is
	*  HOWEVER, the firebar's sprite begins at the sprite's current angle, so the sprite is at the angle it currently is
	*/

	if (m_clockwise) {
		switch (m_angle) {
		case 1:
			setFirePosition('r');

			if (my == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 2:
			setFirePosition('r');

			if (my == 1 || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 3:
			setFirePosition('d');

			// if spinner is on the left or right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 4:
			setFirePosition('d');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 5:
			setFirePosition('l');

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 6:
			setFirePosition('l');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (my == rows - 2 || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 7:
			setFirePosition('u');

			// if spinner is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 8:
			setFirePosition('u');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		default:
			break;
		}

		if (m_angle == 8)
			m_angle = 1;
		else
			m_angle++;
	}
	// else counterclockwise
	else {
		switch (m_angle) {
		case 1:
			setFirePosition('l');

			if (my == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 2:
			setFirePosition('u');

			// if spinner is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 3:
			setFirePosition('u');

			// if spinner is on the right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 4:
			setFirePosition('r');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 5:
			setFirePosition('r');

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 6:
			setFirePosition('d');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == rows - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 7:
			setFirePosition('d');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		case 8:
			setFirePosition('l');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			break;
		default:
			break;
		}

		if (m_angle == 1)
			m_angle = 8;
		else
			m_angle--;
	}

	if (playerWasHit()) {
		playSound("Fire3.mp3");

		m_dungeon->damagePlayer(getDamage(), DamageType::FIRE);

		if (m_dungeon->getPlayer()->canBeBurned() && randReal(1, 100) + m_dungeon->getPlayer()->getLuck() < 80)
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 5));
	}
}

void Firebar::setInitialFirePosition(int x, int y) {

	/* 8 1 2
	*  7 X 3
	*  6 5 4
	*  moves firebar -to- position m_angle, it is not where it currently is
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

	m_dungeon->queueMoveSprite(m_innerFire->getSprite(), m_innerFire->getPosX(), m_innerFire->getPosY());
	m_dungeon->queueMoveSprite(m_outerFire->getSprite(), m_outerFire->getPosX(), m_outerFire->getPosY());

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

	m_dungeon->queueMoveSprite(m_innerFire->getSprite(), m_innerFire->getPosX(), m_innerFire->getPosY());
	m_dungeon->queueMoveSprite(m_outerFire->getSprite(), m_outerFire->getPosX(), m_outerFire->getPosY());

}

bool Firebar::playerWasHit() {
	std::vector<Coords> coords = m_dungeon->getCoordsOfAllPlayers();

	for (auto &it : coords) {
		if (m_innerFire->getPosX() == it.x && m_innerFire->getPosY() == it.y) {
			m_dungeon->queueCustomAction(m_innerFire->getSprite(), cocos2d::ScaleTo::create(0.1f, 0.25f * GLOBAL_SPRITE_SCALE));
			m_innerActive = 3;

			return true;
		}

		if (m_outerFire->getPosX() == it.x && m_outerFire->getPosY() == it.y) {
			m_dungeon->queueCustomAction(m_outerFire->getSprite(), cocos2d::ScaleTo::create(0.1f, 0.25f * GLOBAL_SPRITE_SCALE));
			m_outerActive = 3;

			return true;
		}
	}

	return false;
}

void Firebar::setSpriteColor(cocos2d::Color3B color) {
	m_innerFire->getSprite()->setColor(color);
	m_outerFire->getSprite()->setColor(color);
}
void Firebar::setSpriteVisibility(bool visible) {
	m_innerFire->getSprite()->setVisible(visible);
	m_outerFire->getSprite()->setVisible(visible);
}

void Firebar::spriteCleanup() {
	if (getSprite() != nullptr) {
		m_dungeon->queueRemoveSprite(getSprite());
		setSprite(nullptr);
	}

	if (m_innerFire != nullptr) {
		m_dungeon->queueRemoveSprite(m_innerFire->getSprite());
	}

	if (m_outerFire != nullptr) {
		m_dungeon->queueRemoveSprite(m_outerFire->getSprite());
	}
}

//		DOUBLE FIREBAR
DoubleFirebar::DoubleFirebar(Dungeon &dungeon, int x, int y) : Firebar(dungeon, x, y, DOUBLE_FIREBAR) {
	auto frames = getAnimationFrameVector("Fire%04d.png", 8);

	auto inner = dungeon.runAnimationForever(frames, 24, x, y, y + Z_PROJECTILE - 1);
	inner->setScale(0.75f * GLOBAL_SPRITE_SCALE);
	auto outer = dungeon.runAnimationForever(frames, 24, x, y, y + Z_PROJECTILE - 1);
	outer->setScale(0.75f * GLOBAL_SPRITE_SCALE);

	auto innerMirror = dungeon.runAnimationForever(frames, 24, x, y, y + Z_PROJECTILE - 1);
	innerMirror->setScale(0.75f * GLOBAL_SPRITE_SCALE);
	auto outerMirror = dungeon.runAnimationForever(frames, 24, x, y, y + Z_PROJECTILE - 1);
	outerMirror->setScale(0.75f * GLOBAL_SPRITE_SCALE);

	m_innerFire = std::make_shared<Objects>();
	m_innerFire->setSprite(inner);
	m_outerFire = std::make_shared<Objects>();
	m_outerFire->setSprite(outer);
	m_innerFireMirror = std::make_shared<Objects>();
	m_innerFireMirror->setSprite(innerMirror);
	m_outerFireMirror = std::make_shared<Objects>();
	m_outerFireMirror->setSprite(outerMirror);

	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	setInitialFirePosition(x, y);
}

void DoubleFirebar::activeTrapAction() {
	int rows = m_dungeon->getRows();
	int cols = m_dungeon->getCols();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int mx = getPosX();
	int my = getPosY();

	if (m_innerActive > 0) {
		m_innerActive--;
		if (m_innerActive == 0) {
			m_dungeon->queueCustomAction(m_innerFire->getSprite(), cocos2d::ScaleTo::create(0.05f, 0.75f * GLOBAL_SPRITE_SCALE));
		}
	}

	if (m_innerMirrorActive > 0) {
		m_innerMirrorActive--;
		if (m_innerMirrorActive == 0) {
			m_dungeon->queueCustomAction(m_innerFireMirror->getSprite(), cocos2d::ScaleTo::create(0.05f, 0.75f * GLOBAL_SPRITE_SCALE));
		}
	}

	if (m_outerActive > 0) {
		m_outerActive--;
		if (m_outerActive == 0) {
			m_dungeon->queueCustomAction(m_outerFire->getSprite(), cocos2d::ScaleTo::create(0.05f, 0.75f * GLOBAL_SPRITE_SCALE));
		}
	}

	if (m_outerMirrorActive > 0) {
		m_outerMirrorActive--;
		if (m_outerMirrorActive == 0) {
			m_dungeon->queueCustomAction(m_outerFireMirror->getSprite(), cocos2d::ScaleTo::create(0.05f, 0.75f * GLOBAL_SPRITE_SCALE));
		}
	}

	// reset projectiles to visible
	setSpriteVisibility(true);

	/* 8 1 2
	*  7 X 3
	*  6 5 4
	*  moves firebar -to- position m_angle, it is not where it currently is
	*
	*/

	if (m_clockwise) {
		switch (m_angle) {
		case 1: {
			setFirePosition('r');

			if (my == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}

		}
				break;
		case 2: {
			setFirePosition('r');
			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}


			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (my == rows - 2 || mx == 1) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 3: {
			setFirePosition('d');
			// if firebar is on the left or right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 4: {
			setFirePosition('d');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 5: {
			setFirePosition('l');

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}
			if (my == 1) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 6: {
			setFirePosition('l');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (my == rows - 2 || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == cols - 2) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 7: {
			setFirePosition('u');

			// if firebar is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 8: {
			setFirePosition('u');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		default: break;
		}

		if (m_angle == 8)
			m_angle = 1;
		else
			m_angle++;
	}
	// else counterclockwise
	else {
		switch (m_angle) {
		case 1: {
			setFirePosition('l');

			if (my == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 2: {
			setFirePosition('u');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == rows - 2) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 3: {
			setFirePosition('u');

			// if firebar is on the right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 4: {
			setFirePosition('r');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 5: {
			setFirePosition('r');

			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2))) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			if (my == 1) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 6: {
			setFirePosition('d');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == rows - 2) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == cols - 2) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 7: {
			setFirePosition('d');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the right edge boundary, hide the projectiles
			if (mx == cols - 2) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		case 8: {
			setFirePosition('l');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				m_innerFire->getSprite()->setVisible(false);
				m_outerFire->getSprite()->setVisible(false);
			}

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != rows - 2 && !(my == rows - 3 && mx == cols - 2)) || mx == cols - 2) {
				m_innerFireMirror->getSprite()->setVisible(false);
				m_outerFireMirror->getSprite()->setVisible(false);
			}
		}
				break;
		default: break;
		}

		if (m_angle == 1)
			m_angle = 8;
		else
			m_angle--;
	}

	if (playerWasHit()) {
		playSound("Fire3.mp3");

		m_dungeon->damagePlayer(getDamage(), DamageType::FIRE);

		if (m_dungeon->getPlayer()->canBeBurned() && randReal(1, 100) + m_dungeon->getPlayer()->getLuck() < 80)
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 5));
	}
}

void DoubleFirebar::setInitialFirePosition(int x, int y) {
	if (m_clockwise) {
		switch (m_angle) {
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
		switch (m_angle) {
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

	m_dungeon->queueMoveSprite(m_innerFire->getSprite(), m_innerFire->getPosX(), m_innerFire->getPosY());
	m_dungeon->queueMoveSprite(m_outerFire->getSprite(), m_outerFire->getPosX(), m_outerFire->getPosY());
	m_dungeon->queueMoveSprite(m_innerFireMirror->getSprite(), m_innerFireMirror->getPosX(), m_innerFireMirror->getPosY());
	m_dungeon->queueMoveSprite(m_outerFireMirror->getSprite(), m_outerFireMirror->getPosX(), m_outerFireMirror->getPosY());

	setSpriteVisibility(false);
}
bool DoubleFirebar::playerWasHit() {
	std::vector<Coords> coords = m_dungeon->getCoordsOfAllPlayers();

	for (auto &it : coords) {
		if (m_innerFire->getPosX() == it.x && m_innerFire->getPosY() == it.y) {
			m_dungeon->queueCustomAction(m_innerFire->getSprite(), cocos2d::ScaleTo::create(0.1f, 0.25f * GLOBAL_SPRITE_SCALE));
			m_innerActive = 3;

			return true;
		}

		if (m_outerFire->getPosX() == it.x && m_outerFire->getPosY() == it.y) {
			m_dungeon->queueCustomAction(m_outerFire->getSprite(), cocos2d::ScaleTo::create(0.1f, 0.25f * GLOBAL_SPRITE_SCALE));
			m_outerActive = 3;

			return true;
		}

		if (m_innerFireMirror->getPosX() == it.x && m_innerFireMirror->getPosY() == it.y) {
			m_dungeon->queueCustomAction(m_innerFireMirror->getSprite(), cocos2d::ScaleTo::create(0.1f, 0.25f * GLOBAL_SPRITE_SCALE));
			m_innerMirrorActive = 3;

			return true;
		}

		if (m_outerFireMirror->getPosX() == it.x && m_outerFireMirror->getPosY() == it.y) {
			m_dungeon->queueCustomAction(m_outerFireMirror->getSprite(), cocos2d::ScaleTo::create(0.1f, 0.25f * GLOBAL_SPRITE_SCALE));
			m_outerMirrorActive = 3;

			return true;
		}
	}

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

	m_dungeon->queueMoveSprite(m_innerFire->getSprite(), m_innerFire->getPosX(), m_innerFire->getPosY());
	m_dungeon->queueMoveSprite(m_outerFire->getSprite(), m_outerFire->getPosX(), m_outerFire->getPosY());
	m_dungeon->queueMoveSprite(m_innerFireMirror->getSprite(), m_innerFireMirror->getPosX(), m_innerFireMirror->getPosY());
	m_dungeon->queueMoveSprite(m_outerFireMirror->getSprite(), m_outerFireMirror->getPosX(), m_outerFireMirror->getPosY());
}

void DoubleFirebar::setSpriteColor(cocos2d::Color3B color) {
	m_innerFire->getSprite()->setColor(color);
	m_innerFireMirror->getSprite()->setColor(color);
	m_outerFire->getSprite()->setColor(color);
	m_outerFireMirror->getSprite()->setColor(color);
}
void DoubleFirebar::setSpriteVisibility(bool visible) {
	m_innerFire->getSprite()->setVisible(visible);
	m_innerFireMirror->getSprite()->setVisible(visible);
	m_outerFire->getSprite()->setVisible(visible);
	m_outerFireMirror->getSprite()->setVisible(visible);
}

void DoubleFirebar::spriteCleanup() {
	if (getSprite() != nullptr) {
		m_dungeon->queueRemoveSprite(getSprite());
		setSprite(nullptr);
	}

	if (m_innerFire != nullptr) {
		m_dungeon->queueRemoveSprite(m_innerFire->getSprite());
	}

	if (m_outerFire != nullptr) {
		m_dungeon->queueRemoveSprite(m_outerFire->getSprite());
	}

	if (m_innerFireMirror != nullptr) {
		m_dungeon->queueRemoveSprite(m_innerFireMirror->getSprite());
	}

	if (m_outerFireMirror != nullptr) {
		m_dungeon->queueRemoveSprite(m_outerFireMirror->getSprite());
	}
}

//		LAVA
Lava::Lava(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, LAVA, "Lava_Tile1_48x48.png", 8) {
	setCanBeDoused(true);
	setLethal(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}

void Lava::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->hero(x, y))
		trapAction(*m_dungeon->getPlayer());

	if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1)
			trapAction(*m_dungeon->monsterAt(pos));
	}
}
void Lava::trapAction(Actors &a) {
	if (!a.isFlying()) {
		// lava sound
		if (a.isPlayer())
			playSound("Fire4.mp3");
	}

	// if not immune to lava or flying, then burn them
	if (!(a.lavaImmune() || a.isFlying())) {

		if (a.isPlayer())
			m_dungeon->damagePlayer(getDamage(), DamageType::MELTING);
		else
			m_dungeon->damageMonster(a.getPos(), getDamage(), DamageType::MELTING);

		if (a.canBeBurned()) {		
			a.setBurned(true);
			a.addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 8));
		}
	}
}
void Lava::douse() {
	m_dungeon->addTrap(std::make_shared<Laze>(*m_dungeon, getPosX(), getPosY()));

	destroyTrap();
}

Laze::Laze(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, LAVA, "PoisonWater_48x48.png", 4) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}
void Laze::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->hero(x, y)) {
		m_dungeon->damagePlayer(getDamage(), DamageType::POISONOUS);
		if (m_dungeon->getPlayerAt(x, y)->canBePoisoned())
			m_dungeon->getPlayerAt(x, y)->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 8, 4, 1, 1));
	}

	if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1) {
			m_dungeon->damageMonster(pos, getDamage(), DamageType::POISONOUS);
			if (m_dungeon->monsterAt(pos)->canBePoisoned())
				m_dungeon->giveAffliction(pos, std::make_shared<Poison>(*m_dungeon->getPlayer(), 8, 4, 1, 1));
		}
	}

	if (m_turns > 0) {
		m_turns--;
		if (m_turns == 0)
			destroyTrap();
	}
}

//		MAGMA TIDE
MagmaTide::MagmaTide(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, MAGMA_TIDE, "Lava_Tile1_48x48.png", 8) {
	setLethal(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
	//getSprite()->setScale(2.4f);

	m_wait = 8 + randInt(0, 4);
	m_maxWait = m_wait;

	m_stage = randInt(1, 7);
}

void MagmaTide::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_wait > 0)
		m_wait--;

	if (m_wait == 0) {

		if (m_stage == 6)
			m_stage = 1;
		else
			m_stage++;

		float scaleFactor = GLOBAL_SPRITE_SCALE;
		switch (m_stage) {
		case 1: scaleFactor = GLOBAL_SPRITE_SCALE; m_wait = m_maxWait; break;
		case 2: scaleFactor = 2.4f; m_wait = 1; break;
		case 3: scaleFactor = GLOBAL_SPRITE_SCALE; m_wait = 1; break;
		case 4: scaleFactor = 2.4f; m_wait = 1; break;
		case 5: scaleFactor = 4.0f; m_wait = m_maxWait; break;
		case 6: scaleFactor = 2.4f; m_wait = 1; break;
		}

		m_dungeon->queueCustomAction(getSprite(), cocos2d::ScaleTo::create(0.1f, scaleFactor));

		//m_expanded = !m_expanded;

		/*if (m_expanded)
			m_dungeon->queueCustomAction(getSprite(), cocos2d::ScaleTo::create(0.1f, 4.0f));
		else
			m_dungeon->queueCustomAction(getSprite(), cocos2d::ScaleTo::create(0.1f, 2.4f));*/

		//m_wait = m_maxWait;
	}

	int range = 0;

	switch (m_stage) {
	case 1: range = 0; break;
	case 2: range = 1; break;
	case 3: range = 0; break;
	case 4: range = 1; break;
	case 5: range = 2; break;
	case 6: range = 1; break;
	}
	
	//int range = (m_expanded ? 2 : 1);

	for (int i = x - range; i < x + range + 1; i++) {
		for (int j = y - range; j < y + range + 1; j++) {

			if (!m_dungeon->withinBounds(i, j))
				continue;

			if (m_dungeon->enemy(i, j)) {
				int pos = m_dungeon->findMonster(i, j);
				if (pos != -1)
					trapAction(*m_dungeon->monsterAt(pos));				
			}

			if (m_dungeon->hero(i, j))
				trapAction(*m_dungeon->getPlayer());

			checkBurn(i, j);
		}
	}
}
void MagmaTide::trapAction(Actors &a) {
	if (!a.isFlying()) {
		// lava sound
		if (a.isPlayer())
			playSound("Fire4.mp3");
	}

	// if not immune to lava or flying, then burn them
	if (!(a.lavaImmune() || a.isFlying())) {

		if (a.isPlayer())
			m_dungeon->damagePlayer(getDamage(), DamageType::MELTING);
		else
			m_dungeon->damageMonster(a.getPos(), getDamage(), DamageType::MELTING);

		if (a.canBeBurned()) {
			a.setBurned(true);
			a.addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 8));
		}
	}
}

//		MOLTEN PILLAR
MoltenPillar::MoltenPillar(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, MOLTEN_PILLAR, "Ruby_48x48.png", 0) {
	m_maxWait = m_wait;
	setWallFlag(true);
	setDestructible(true);
	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	dungeon.addLightSource(x, y, 4, getName());
	dungeon.tileAt(getPos()).wall = true;
}

void MoltenPillar::activeTrapAction() {

	int x = getPosX();
	int y = getPosY();

	if (m_wait > 0)
		m_wait--;

	if (m_wait == 0) {
		if (m_stage == 2)
			m_stage = 1;
		else
			m_stage++;

		m_wait = m_maxWait;
	}
	else
		return;

	double strength;
	switch (m_stage) {
	case 1: strength = 3; break;
	case 2: strength = 4; break;
	}

	m_dungeon->removeLightSource(x, y, getName());
	m_dungeon->addLightSource(x, y, strength, getName());
}
void MoltenPillar::trapAction(Actors &a) {
	playHitSmasher();

	if (a.getStr() >= m_strengthCutoff) {
		m_strength--;
	}

	if (m_strength == 0) {
		destroyTrap();
	}
}
void MoltenPillar::drops() {
	int x = getPosX();
	int y = getPosY();
	Coords pos = getPos();

	m_dungeon->addTrap(std::make_shared<Lava>(*m_dungeon, x, y));

	if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 98) {
		m_dungeon->tileAt(pos).object = std::make_shared<MagmaHeart>(x, y);
		m_dungeon->tileAt(pos).object->setSprite(m_dungeon->createSprite(pos, pos.y + Z_ITEM, m_dungeon->tileAt(pos).object->getImageName()));
		m_dungeon->tileAt(pos).item = true;
		m_dungeon->addItem(m_dungeon->tileAt(pos).object);
	}
}

//		SPRINGS
Spring::Spring(Dungeon &dungeon, int x, int y, bool trigger, char move) : Traps(&dungeon, x, y, SPRING, "Spring_Arrow_Left_48x48.png", 0) {
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
		switch (randInt(1, 8)) {
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

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
	getSprite()->setScale(0.5f);

	oppositeSprings();
}
Spring::Spring(Dungeon &dungeon, int x, int y, bool trigger, bool known, bool cardinal) : Traps(&dungeon, x, y, SPRING, "Spring_Arrow_Left_48x48.png", 0) {
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
	setDestructible(true);

	setImage();

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
	getSprite()->setScale(0.5f);

	oppositeSprings();
}

void Spring::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->hero(x, y)) {
		trapAction(*m_dungeon->getPlayerAt(x, y));
	}
	else if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1)
			trapAction(*m_dungeon->monsterAt(pos));
	}
}
void Spring::trapAction(Actors &a) {
	int x = a.getPosX();
	int y = a.getPosY();

	int tx = getPosX();
	int ty = getPosY();

	if (a.isFlying() || a.isHeavy())
		return;

	if (!(tx == x && ty == y)) {
		m_triggered = false;

		return;
	}

	// check if trigger spring trap
	if (m_isTrigger) {

		// if actor is on the same tile as the spring, trigger it
		if (tx == x && ty == y && !m_triggered) {
			// play trigger sound effect
			playSound("Spring_Trigger.mp3");

			m_triggered = true;

			return;
		}
		// if something triggered the spring but stepped off, return and reset the trap to not triggered
		else if (!(tx == x && ty == y) && m_triggered) {
			m_triggered = false;

			return;
		}
		// if actor is on the same tile on the spring and it is triggered, continue and reset the trap to not triggered
		else if (tx == x && ty == y && m_triggered) {
			m_triggered = false;
		}

	}


	int n = 0, m = 0; // n : x, m : y
	
	// any direction
	if (isMultiDirectional() && isAny()) {
		// choose random direction
		switch (randInt(1, 8)) {
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
			switch (randInt(1, 4)) {
			case 1: n = -1, m = 0; break;	// L
			case 2: n = 1, m = 0; break;	// R
			case 3: n = 0, m = -1; break;	// U
			case 4: n = 0, m = 1; break;	// D
			}
		}
		// diagonal directions only
		else {
			switch (randInt(1, 4)) {
			case 1: n = 1, m = -1; break;	// Q1
			case 2: n = -1, m = -1; break;	// Q2
			case 3: n = -1, m = 1; break;	// Q3
			case 4: n = 1, m = 1; break;	// Q4
			}
		}
	}
	// single direction
	else {
		switch (m_dir) {
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


	if (a.isPlayer()) {
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {
			playSound("Spring_Bounce.mp3", m_dungeon->getPlayer()->getPos(), getPos());

			a.moveTo(x + n, y + m);

			if (m_dungeon->trap(x + n, y + m))
				m_dungeon->trapEncounter(x + n, y + m);		
		}
	}
	else {
		if (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {
			playSound("Spring_Bounce.mp3", m_dungeon->getPlayer()->getPos(), getPos());

			a.moveTo(x + n, y + m);

			if (m_dungeon->trap(x + n, y + m))
				m_dungeon->singleMonsterTrapEncounter(m_dungeon->findMonster(x + n, y + m));
		}
	}
}

void Spring::setImage() {
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
	case '#': image = "Spring_Arrow_DownRight_48x48.png"; break;
	case '+': image = "Spring_Arrow_DownRight_48x48.png"; break;
	case 'x': image = "Spring_Arrow_DownRight_48x48.png"; break;
	default: image = "cheese.png"; break;
	}
	setImageName(image);
}
bool Spring::isOpposite(const Spring &other) const {
	char dir1 = m_dir;
	char dir2 = other.m_dir;

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

	return false;
}
void Spring::oppositeSprings() {
	int x = getPosX();
	int y = getPosY();

	int n = 0, m = 0;
	char dir = m_dir;
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

	if (m_dungeon->trap(x + n, y + m)) {
		std::vector<int> indexes = m_dungeon->findTraps(x + n, y + m);
		for (int i = 0; i < (int)indexes.size(); i++) {

			if (m_dungeon->trapAt(indexes[i])->getName() == SPRING) {

				// while the two springs point toward each other, reroll this spring's direction
				std::shared_ptr<Spring> spring = std::dynamic_pointer_cast<Spring>(m_dungeon->trapAt(indexes[i]));
				while (spring->isOpposite(*this)) {

					switch (randInt(1, 8)) {
					case 1: dir = 'l'; break;
					case 2: dir = 'r'; break;
					case 3: dir = 'u'; break;
					case 4: dir = 'd'; break;
					case 5: dir = '1'; break;	// Q1
					case 6: dir = '2'; break;	// Q2
					case 7: dir = '3'; break;	// Q3
					case 8: dir = '4'; break;	// Q4
					}

					m_dir = dir;
					setImage();
				}

				// if spring had to be turned, recursively check for more springs in the new direction it's facing
				if (dir != m_dir)
					oppositeSprings();				
			}
		}
	}
}

//		TURRETS
Turret::Turret(Dungeon &dungeon, int x, int y, char dir) : Traps(&dungeon, x, y, TURRET, "Spring_Arrow_Left_48x48.png", 4), m_triggered(false) {
	setWallFlag(true);

	if (dir == '-') {
		switch (randInt(1, 4)) {
		case 1: m_dir = 'l'; break;
		case 2: m_dir = 'r'; break;
		case 3: m_dir = 'u'; break;
		case 4: m_dir = 'd'; break;
		}
	}
	else
		m_dir = dir;

	std::string image;
	switch (m_dir) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	}
	setImageName(image);

	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	dungeon.tileAt(getPos()).wall = true;
}

void Turret::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_cooldown) {
		m_cooldown = false;
		return;
	}

	if (!m_triggered) {
		Coords start(x, y);

		std::vector<Coords> coords = m_dungeon->getCoordsOfAllPlayers();

		for (auto &it : coords) {
			if (hasLineOfSight(*m_dungeon, m_dir, start, it) && positionInLinearRange(start, it, m_range)) {
				playSound("Turret_Trigger.mp3");

				std::string image;
				switch (m_dir) {
				case 'l': image = "Spring_Arrow_Left_Red_48x48.png"; break;
				case 'r': image = "Spring_Arrow_Right_Red_48x48.png"; break;
				case 'u': image = "Spring_Arrow_Up_Red_48x48.png"; break;
				case 'd': image = "Spring_Arrow_Down_Red_48x48.png"; break;
				}

				getSprite()->setSpriteFrame(image);
				m_triggered = true;
			}
		}

		return;
	}

	playSound("Gunshot1.mp3");

	checkLineOfFire();

	std::string image;
	switch (m_dir) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	}
	getSprite()->setSpriteFrame(image);

	m_cooldown = true;
	m_triggered = false;
}
void Turret::checkLineOfFire() {
	int x = getPosX();
	int y = getPosY();

	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	while (!m_dungeon->wall(x + n, y + m)) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		if (m_dungeon->hero(x + n, y + m)) {
			int damage = getDamage();

			if (m_dungeon->getPlayerAt(x + n, y + m)->canBlock() && m_dungeon->getPlayerAt(x + n, y + m)->didBlockAttackAt(x, y, damage)) {
				;
			}

			m_dungeon->damagePlayer(damage, DamageType::PIERCING);

			return;
		}

		if (m_dungeon->enemy(x + n, y + m)) {
			int pos = m_dungeon->findMonster(x + n, y + m);
			m_dungeon->damageMonster(pos, getDamage(), DamageType::PIERCING);

			return; // remove this if it's a piercing turret (laser)
		}

		incrementDirectionalOffsets(m_dir, n, m);
	}
}

//		MOVING BLOCKS
MovingBlock::MovingBlock(Dungeon &dungeon, int x, int y, int spaces) 
	: Traps(&dungeon, x, y, MOVING_BLOCK, "Breakable_Crate_48x48.png", 10), m_maxSpaces(spaces), m_spaces(spaces) {

	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;

	setWallFlag(true);
	dungeon.tileAt(getPos()).wall = true;
}

void MovingBlock::moveTo(int x, int y, float time) {
	Coords currentPos = getPos();
	Coords newPos(x, y);

	m_dungeon->tileAt(currentPos).wall = false;
	m_dungeon->tileAt(newPos).wall = true;

	m_dungeon->queueMoveSprite(getSprite(), newPos, time);
	setPos(newPos);
}
void MovingBlock::activeTrapAction() {
	if (m_spaces == 0) {
		m_spaces = m_maxSpaces;
		setDirection();
	}

	int x = getPosX();
	int y = getPosY();

	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	if (m_dungeon->wall(x + n, y + m)) {
		m_spaces = m_maxSpaces;
		setDirection();
		return;
	}

	else if (m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m)) {
		m_dungeon->linearActorPush(x + n, y + m, 1, 1, m_dir, false, true);
	}

	if (!(m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m)))
		moveTo(x + n, y + m);

	m_spaces--;
}

LinearMovingBlock::LinearMovingBlock(Dungeon &dungeon, int x, int y, char dir, int spaces) : MovingBlock(dungeon, x, y, spaces) {
	if (dir == '-') {
		switch (randInt(1, 4)) {
		case 1: m_dir = 'l'; break;
		case 2: m_dir = 'r'; break;
		case 3: m_dir = 'u'; break;
		case 4: m_dir = 'd'; break;
		}
	}
	else
		m_dir = dir;
}

void LinearMovingBlock::setDirection() {
	switch (m_dir) {
	case 'l': m_dir = 'r'; break;
	case 'r': m_dir = 'l'; break;
	case 'u': m_dir = 'd'; break;
	case 'd': m_dir = 'u'; break;
	}
}

SquareMovingBlock::SquareMovingBlock(Dungeon &dungeon, int x, int y, char dir, int spaces, bool clockwise) : MovingBlock(dungeon, x, y, spaces) {
	if (dir == '-') {
		switch (randInt(1, 4)) {
		case 1: m_dir = 'l'; break;
		case 2: m_dir = 'r'; break;
		case 3: m_dir = 'u'; break;
		case 4: m_dir = 'd'; break;
		}
	}
	else
		m_dir = dir;

	m_clockwise = clockwise;
}

void SquareMovingBlock::setDirection() {
	switch (m_dir) {
	case 'l': m_dir = m_clockwise ? 'u' : 'd'; break;
	case 'r': m_dir = m_clockwise ? 'd' : 'u'; break;
	case 'u': m_dir = m_clockwise ? 'r' : 'l'; break;
	case 'd': m_dir = m_clockwise ? 'l' : 'r'; break;
	}
}

//		ACTIVE BOMB
ActiveBomb::ActiveBomb(Dungeon &dungeon, int x, int y, int timer) : Traps(&dungeon, x, y, ACTIVE_BOMB, "Bomb_48x48.png", 10), m_timer(timer) {
	m_fuseID = playSoundWithID("Bomb_Fuse2.mp3");
	setRange(1);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_TOP, getImageName()));

	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}
ActiveBomb::ActiveBomb(Dungeon &dungeon, int x, int y, std::string type, std::string image, int damage, int timer) : Traps(&dungeon, x, y, type, image, damage), m_timer(timer) {
	m_fuseID = playSoundWithID("Bomb_Fuse2.mp3");
}

void ActiveBomb::activeTrapAction() {
	if (m_timer > 0) {
		m_timer--;
	}
	else {
		// play explosion sound effect
		playSound("Explosion.mp3");

		explosion();

		destroyTrap();
	}
}
void ActiveBomb::explosion() {
	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	int x = getPosX();
	int y = getPosY();
	
	int mx, my;

	// stop fuse sound
	cocos2d::experimental::AudioEngine::stop(m_fuseID);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Explosion%04d.png", 8);
	m_dungeon->runSingleAnimation(frames, 24, x, y, 2);

	flashFloor(*m_dungeon, x, y, m_range == 1 ? false : true);

	if (abs(px - x) <= m_range && abs(py - y) <= m_range)
		m_dungeon->damagePlayer(getDamage(), DamageType::EXPLOSIVE);

	for (int i = x - m_range; i < x + m_range + 1; i++) {
		for (int j = y - m_range; j < y + m_range + 1; j++) {

			if (m_dungeon->withinBounds(i, j)) {

				if (m_dungeon->wall(i, j))
					m_dungeon->destroyWall(i, j);
				
				if (m_dungeon->gold(i, j) != 0)
					m_dungeon->removeGold(i, j);

				if (m_dungeon->exit(i, j))
					continue;
				
				checkExplosion(i, j);

				extraExplosionEffects(i, j);
			}
		}
	}

	// Find any monsters caught in the blast
	for (unsigned i = 0; i < m_dungeon->monsterCount(); i++) {
		mx = m_dungeon->monsterAt(i)->getPosX();
		my = m_dungeon->monsterAt(i)->getPosY();
	
		if (abs(mx - x) <= m_range && abs(my - y) <= m_range) {
			int damage = getDamage() + (m_dungeon->getPlayer()->explosionImmune() ? 10 : 0);
			m_dungeon->damageMonster(i, damage, DamageType::EXPLOSIVE);
		}	
	}
}

//		ACTIVE MEGA BOMB
ActiveMegaBomb::ActiveMegaBomb(Dungeon &dungeon, int x, int y) : ActiveBomb(dungeon, x, y, ACTIVE_MEGA_BOMB, "Bomb_48x48.png", 20) {
	setRange(2);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_TOP, getImageName()));

	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}

void ActiveMegaBomb::activeTrapAction() {
	if (getTimer() > 0) {
		decreaseTimerBy(1);
	}
	else {
		playSound("Mega_Explosion.mp3");

		explosion();

		destroyTrap();
	}
}

ActiveFireBomb::ActiveFireBomb(Dungeon &dungeon, int x, int y, int turns) : ActiveBomb(dungeon, x, y, ACTIVE_FIRE_BOMB, "Bomb_48x48.png", 15, turns) {
	setRange(1);

	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_TOP, getImageName()));
}

void ActiveFireBomb::extraExplosionEffects(int x, int y) {
	if (!m_dungeon->wall(x, y)) {
		if (randReal(1, 100) > 50)
			m_dungeon->addTrap(std::make_shared<Ember>(*m_dungeon, x, y, 5 + randInt(0, 4)));		
	}
}

//		ACTIVE POISON BOMB
ActivePoisonBomb::ActivePoisonBomb(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, ACTIVE_POISON_BOMB, "Green_Orb_32x32.png", 6) {
	m_fuseID = playSoundWithID("Bomb_Fuse2.mp3");

	setExplosive(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
}

void ActivePoisonBomb::trapAction(Actors &a) {
	/*if (!m_set) {
		m_set = true;
		return;
	}*/

	int x = getPosX();
	int y = getPosY();

	int ax = a.getPosX();
	int ay = a.getPosY();

	// if it hasn't been set off yet and something is standing on it
	if (ax == x && ay == y) {
		explosion();

		destroyTrap();
	}
}
void ActivePoisonBomb::explosion() {
	int x = getPosX();
	int y = getPosY();
	
	playSound("Poison_Bomb_Explosion.mp3", m_dungeon->getPlayer()->getPos(), getPos());
	cocos2d::experimental::AudioEngine::stop(m_fuseID);

	// Damage anything that was in the blast radius: damage is reduced depending on how far from the blast they were
	for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 2; j < x + 3; j++) {

			// boundary and corner check
			if (m_dungeon->withinBounds(j, i) &&
				!(i == y - 2 && j == x - 2) && !(i == y - 2 && j == x + 2) && !(i == y + 2 && j == x - 2) && !(i == y + 2 && j == x + 2)) {

				// check if player was hit, and they were not the one that set the bomb off
				if (m_dungeon->hero(j, i)) {

					// if on the outer ring, damage is reduced by 4. if inner ring, damage reduced by 2
					int damageReduction = (abs(x - j) == 2 || abs(y - i) == 2 ? 4 : abs(x - j) == 1 && abs(y - i) == 1 ? 2 : 0);
					m_dungeon->damagePlayer(getDamage() - damageReduction, DamageType::EXPLOSIVE);

					if (m_dungeon->getPlayer()->canBePoisoned())
						m_dungeon->getPlayer()->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 8, 4, 1, 1));

				}

				// check for enemies in the gas cloud
				if (m_dungeon->enemy(j, i)) {
					int pos = m_dungeon->findMonster(j, i);

					if (pos != -1) {
						// if on the outer ring, damage is reduced by 4. if inner ring, damage reduced by 2
						int damageReduction = (abs(x - j) == 2 || abs(y - i) == 2 ? 4 : abs(x - j) == 1 && abs(y - i) == 1 ? 2 : 0);
						m_dungeon->damageMonster(pos, getDamage() - damageReduction, DamageType::POISONOUS);

						if (m_dungeon->monsterAt(pos)->canBePoisoned())
							m_dungeon->monsterAt(pos)->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 8, 4, 1, 1));
					}
				}

				checkPoison(j, i);
			}
		}
	}

	poisonCloud(*m_dungeon, x, y, 0.8f, cocos2d::Color3B(35, 140, 35));
}
void ActivePoisonBomb::explode() {
	explosion();
	destroyTrap();
}
void ActivePoisonBomb::drops() {
	m_dungeon->addTrap(std::make_shared<PoisonCloud>(*m_dungeon, getPosX(), getPosY(), 12));
}

PoisonCloud::PoisonCloud(Dungeon &dungeon, int x, int y, int turns) : Traps(&dungeon, x, y, POISON_CLOUD, "Bomb_48x48.png", 10), m_turns(turns) {
	dungeon.tileAt(getPos()).trap = false;
}

void PoisonCloud::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	poisonCloud(*m_dungeon, x, y, 0.0f, cocos2d::Color3B(35, 140, 35));

	for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 2; j < x + 3; j++) {

			// boundary and corner check
			if (m_dungeon->withinBounds(j, i) &&
				!(i == y - 2 && j == x - 2) && !(i == y - 2 && j == x + 2) && !(i == y + 2 && j == x - 2) && !(i == y + 2 && j == x + 2)) {

				if (m_dungeon->hero(j, i)) {
					if (m_dungeon->getPlayer()->canBePoisoned())
						m_dungeon->getPlayer()->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 4, 4, 1, 1));					
				}

				if (m_dungeon->enemy(j, i)) {
					int pos = m_dungeon->findMonster(j, i);
					if (pos != -1) {
						if (m_dungeon->monsterAt(pos)->canBePoisoned())
							m_dungeon->monsterAt(pos)->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 4, 4, 1, 1));
					}
				}

				checkPoison(j, i);
			}
		}
	}

	m_turns--;
	if (m_turns == 0)
		destroyTrap();
}

//		BEAR TRAP
SetBearTrap::SetBearTrap(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, BEAR_TRAP, "Blue_Toy_32x32.png", 4) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
}

void SetBearTrap::trapAction(Actors &a) {
	if (!m_set) {
		m_set = true;
		return;
	}

	Coords pos = getPos();
	Coords actorPos = a.getPos();

	// if actor is flying, return
	if (a.isFlying())
		return;

	if (pos == actorPos) {
		playSound("Metal_Hit8.mp3", m_dungeon->getPlayer()->getPos(), pos);

		a.decreaseStatBy(StatType::HP, getDamage());
		a.addAffliction(std::make_shared<Bleed>(5));
		a.addAffliction(std::make_shared<Cripple>(15));

		destroyTrap();
	}
}

//		CRUMBLE FLOOR
CrumbleFloor::CrumbleFloor(Dungeon &dungeon, int x, int y, int strength, std::string name, std::string image) : Traps(&dungeon, x, y, name, image, 0), m_strength(strength) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}

void CrumbleFloor::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_crossed) {
		if (!(m_dungeon->enemy(x, y) || m_dungeon->hero(x, y))) {
			weaken();
			m_crossed = false;
		}

		return;
	}

	if (m_dungeon->enemy(x, y) || m_dungeon->hero(x, y)) {
		if (m_dungeon->enemy(x, y)) {
			int pos = m_dungeon->findMonster(x, y);
			if (pos != -1 && !m_dungeon->monsterAt(pos)->isFlying())
				m_crossed = true;
		}

		if (m_dungeon->hero(x, y)) {
			if (!m_dungeon->getPlayerAt(x, y)->isFlying())
				m_crossed = true;
		}

		return;
	}
}
void CrumbleFloor::trapAction(Actors &a) {
	if (m_crossed) {
		weaken();
		return;
	}
}
void CrumbleFloor::weaken() {
	--m_strength;

	playCrumble(exp(-calculateDistanceBetween(getPos(), m_dungeon->getPlayer()->getPos())));

	if (m_strength == 0) {
		playSound("Crumble_Breaking.mp3", m_dungeon->getPlayer()->getPos(), getPos());

		crumble();
	}
	else if (m_strength == 1) {
		getSprite()->setSpriteFrame("Crumble_Floor3_48x48.png");
	}
	else if (m_strength == 2) {
		getSprite()->setSpriteFrame("Crumble_Floor2_48x48.png");
	}
}

CrumblePit::CrumblePit(Dungeon &dungeon, int x, int y, int strength) : CrumbleFloor(dungeon, x, y, strength, CRUMBLE_PIT, "Crumble_Floor1_48x48.png") {

}
void CrumblePit::crumble() {
	m_dungeon->addTrap(std::make_shared<Pit>(*m_dungeon, getPosX(), getPosY()));

	destroyTrap();
}

CrumbleLava::CrumbleLava(Dungeon &dungeon, int x, int y, int strength) : CrumbleFloor(dungeon, x, y, strength, CRUMBLE_LAVA, "Crumble_Floor1_48x48.png") {
	
}
void CrumbleLava::crumble() {
	m_dungeon->addTrap(std::make_shared<Lava>(*m_dungeon, getPosX(), getPosY()));

	destroyTrap();
}

//		EMBER
Ember::Ember(Dungeon &dungeon, int x, int y, int turns) : Traps(&dungeon, x, y, EMBER, "Spinner_Buddy_48x48.png", 2), m_turns(turns) {
	setCanBeDoused(true);

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Fire%04d.png", 8);
	setSprite(dungeon.runAnimationForever(frames, 24, x, y, y + Z_TRAP_TOP));
	getSprite()->setScale(0.75f * GLOBAL_SPRITE_SCALE);

	dungeon.addLightSource(x, y, 3, getName());
}

void Ember::activeTrapAction() {
	if (isDestroyed())
		return;

	int x = getPosX();
	int y = getPosY();

	if (m_turns > 0 && m_dungeon->enemy(x, y)) {

		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1) {		

			// If the monster can't be burned, do nothing
			if (!m_dungeon->monsterAt(pos)->canBeBurned()) {
				m_turns--;
				return;
			}		

			// 50% chance to burn
			if (m_dungeon->monsterAt(pos)->canBeBurned() && randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 50) {
				playSound("Fire3.mp3", m_dungeon->getPlayer()->getPos(), getPos());
				m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), 4));
			}
		}
	}

	if (m_turns > 0) {
		m_turns--;
		return;
	}

	destroyTrap();
}
void Ember::trapAction(Actors &a) {
	Coords start(getPos());

	Coords actorCoords(a.getPos());

	if (isDestroyed())
		return;

	if (start == actorCoords) {

		// 50% chance to burn
		if (a.canBeBurned()) {
			playSound("Fire3.mp3", m_dungeon->getPlayer()->getPos(), start);

			if (a.isPlayer())
				m_dungeon->damagePlayer(getDamage(), DamageType::FIRE);
			else
				m_dungeon->damageMonster(actorCoords, getDamage(), DamageType::FIRE);

			// If it's the player, then it's less chance to be burned
			int luck = m_dungeon->getPlayer()->getLuck() * (a.isPlayer() ? -1 : 1);
			if (randReal(1, 100) + luck > 50)
				a.addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 4));

			destroyTrap();
		}
	}
}
void Ember::douse() {
	if (isDestroyed())
		return;

	destroyTrap();
}
void Ember::spriteCleanup() {
	auto scale = cocos2d::ScaleTo::create(0.1f, 0.0f);
	auto fade = cocos2d::FadeOut::create(0.1f);
	cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	v.pushBack(scale);
	v.pushBack(fade);

	m_dungeon->queueRemoveSpriteWithMultipleActions(getSprite(), v);
}

//		WEB
Web::Web(Dungeon &dungeon, int x, int y, int stickiness) : Traps(&dungeon, x, y, WEB, "Spider_Web.png", 0), m_stickiness(stickiness) {
	setDestructible(true);
	setCanBeIgnited(true);

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_TOP, getImageName()));
}

void Web::trapAction(Actors &a) {
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
		
		destroyTrap();
	}
}
void Web::ignite() {
	int x = getPosX();
	int y = getPosY();

	if (isDestroyed())
		return;

	destroyTrap();

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
	m_dungeon->runSingleAnimation(frames, 120, x, y, y + Z_PROJECTILE);

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (m_dungeon->enemy(i, j) || m_dungeon->trap(i, j))
				m_dungeon->runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);

			checkBurn(i, j);

			if (m_dungeon->enemy(i, j)) {
				int pos = m_dungeon->findMonster(i, j);
				if (pos != -1) {
					if (m_dungeon->monsterAt(pos)->canBeBurned())
						m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), 8));				
				}
			}
		}
	}

	// Create an ember where it was
	m_dungeon->addTrap(std::make_shared<Ember>(*m_dungeon, x, y, 2 + randInt(0, 2)));
}

//		WIND TUNNEL
WindTunnel::WindTunnel(Dungeon &dungeon, int x, int y, char dir) : Traps(&dungeon, x, y, WIND_TUNNEL, "Spider_Web.png", 0), m_dir(dir) {
	setWallFlag(true);

	std::string image;
	switch (dir) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	}
	setImageName(image);

	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));
}

void WindTunnel::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	int n = 0, m = 0;
	setDirectionalOffsets(m_dir, n, m);

	int range = 15;
	int currentRange = 1;

	while (!m_dungeon->wall(x + n, y + m) && currentRange <= range) {

		if (m_dungeon->hero(x + n, y + m) || m_dungeon->enemy(x + n, y + m)) {
			m_dungeon->linearActorPush(x + n, y + m, 1, range, m_dir);
			break;
		}

		incrementDirectionalOffsets(m_dir, n, m);

		currentRange++;
	}
}

Sand::Sand(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, SAND, "Crumble_Floor1_48x48.png", 0) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));

	if (randReal(1, 100) > 90)
		dungeon.addMonster(std::make_shared<SandCentipede>(&dungeon, x, y));
}

void Sand::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (randReal(1, 100) > 70)
		checkDouse(x, y);
}

Quicksand::Quicksand(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, QUICKSAND, "Cheese_Wedge_48x48.png", 0) {
	m_depth = 0;
	m_maxDepth = 3;
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}

void Quicksand::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_capturedActor && m_capturedActor->isDead()) {
		m_capturedActor.reset();
		m_capturedActor = nullptr;
	}
	else if (m_capturedActor && m_capturedActor->getPosX() == x && m_capturedActor->getPosY() == y) {

		if (m_capturedActor->isPlayer()) {

			if (directionIsOppositeTo(m_dungeon->getPlayer()->facingDirection(), m_pDir)) {
				playSound("Grass2.mp3");
				m_dungeon->queueCustomAction(m_capturedActor->getSprite(), cocos2d::ScaleBy::create(0.08f, 1.25f));

				m_depth--;
				if (m_depth == 0) {
					m_capturedActor.reset();
					m_capturedActor = nullptr;
					return;
				}
			}
			else {
				m_depth++;
				m_dungeon->queueCustomAction(m_capturedActor->getSprite(), cocos2d::ScaleBy::create(0.08f, 0.8f));

				if (m_depth == m_maxDepth) {
					playSound("Female_Falling_Scream_License.mp3");
					deathFade(m_capturedActor->getSprite());

					m_capturedActor->setSuperDead(true);
					m_depth = 0;
				}
			}
			
			m_pDir = m_dungeon->getPlayer()->facingDirection();
		}
		else {
			m_depth++;
			m_dungeon->queueCustomAction(m_capturedActor->getSprite(), cocos2d::ScaleBy::create(0.08f, 0.8f));

			if (m_depth == m_maxDepth) {
				deathFade(m_capturedActor->getSprite());
				m_capturedActor->setSuperDead(true);
				m_depth = 0;
			}
		}

		m_capturedActor->addAffliction(std::make_shared<Stun>(1));

		return;
	}
	else if (m_capturedActor && (m_capturedActor->getPosX() != x || m_capturedActor->getPosY() != y)) {
		m_capturedActor.reset();
		m_capturedActor = nullptr;
	}

	if (m_dungeon->hero(x, y) && !m_dungeon->getPlayerAt(x, y)->isFlying()) {
		m_pDir = m_dungeon->getPlayerAt(x, y)->facingDirection();
		m_dungeon->getPlayerAt(x, y)->addAffliction(std::make_shared<Stun>(1));
		m_capturedActor = m_dungeon->getPlayerAt(x, y);

		m_depth++;
		m_dungeon->queueCustomAction(m_capturedActor->getSprite(), cocos2d::ScaleBy::create(0.08f, 0.8f));
	}

	if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1 && !m_dungeon->monsterAt(pos)->isFlying()) {
			m_dungeon->giveAffliction(pos, std::make_shared<Stun>(1));
			m_capturedActor = m_dungeon->monsterAt(pos);

			m_depth++;
			m_dungeon->queueCustomAction(m_capturedActor->getSprite(), cocos2d::ScaleBy::create(0.08f, 0.8f));
		}
	}
}

//		CACTUS
Cactus::Cactus(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, CACTUS, "Cheese_Wedge_48x48.png", 5) {
	setCanBeIgnited(true);
	setWallFlag(true);
	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	dungeon.tileAt(getPos()).wall = true;
}

void Cactus::trapAction(Actors &a) {

	if (!a.isPlayer())
		return;

	m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);

	destroyTrap();
}
void Cactus::ignite() {
	setDestroyed(true); // Prevents CactusWater from dropping

	int x = getPosX();
	int y = getPosY();

	auto frames = getAnimationFrameVector("frame%04d.png", 63);
	m_dungeon->runSingleAnimation(frames, 120, x, y, y + Z_PROJECTILE);

	m_dungeon->addTrap(std::make_shared<Ember>(*m_dungeon, x, y, 6 + randInt(0, 4)));

	destroyTrap();
}
void Cactus::drops() {
	if (isDestroyed())
		return;

	if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 95)
		m_dungeon->createItem(std::make_shared<CactusWater>(getPosX(), getPosY()));
}

MovingTile::MovingTile(Dungeon &dungeon, int x, int y, int spaces) : Traps(&dungeon, x, y, MOVING_TILE, "Cheese_Wedge_48x48.png", 0), m_spaces(spaces), m_maxSpaces(spaces) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));

	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}

void MovingTile::moveTo(int x, int y, float time) {
	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);
}
void MovingTile::activeTrapAction() {
	if (m_spaces == 0) {
		m_spaces = m_maxSpaces;
		setDirection();
	}

	int x = getPosX();
	int y = getPosY();
	Coords pos = getPos();

	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	if (m_dungeon->itemObject(x, y)) {
		Coords newPos(x + n, y + m);

		m_dungeon->itemObject(x, y)->setPos(newPos);
		m_dungeon->tileAt(newPos).object = m_dungeon->itemObject(x, y);
		m_dungeon->tileAt(newPos).item = true;
		m_dungeon->queueMoveSprite(m_dungeon->tileAt(newPos).object->getSprite(), newPos);

		m_dungeon->tileAt(pos).object.reset();
		m_dungeon->tileAt(pos).object = nullptr;
		m_dungeon->tileAt(pos).item = false;
	}

	if (m_dungeon->trap(x, y)) {
		std::vector<int> indexes = m_dungeon->findTraps(x, y);
		for (size_t i = 0; i < indexes.size(); ++i)
			m_dungeon->trapAt(indexes.at(i))->moveTo(x + n, y + m);
	}

	if (m_dungeon->hero(x, y)) {
		if (!(m_dungeon->enemy(x + n, y + m) || m_dungeon->wall(x + n, y + m)))
			m_dungeon->getPlayerAt(x, y)->moveTo(x + n, y + m);
	}

	if (m_dungeon->enemy(x, y)) {
		if (!(m_dungeon->hero(x + n, y + m) || m_dungeon->wall(x + n, y + m))) {
			int pos = m_dungeon->findMonster(x, y);
			if (pos != -1)
				m_dungeon->monsterAt(pos)->moveTo(x + n, y + m);
		}
	}

	moveTo(x + n, y + m);

	m_spaces--;
}

LinearMovingTile::LinearMovingTile(Dungeon &dungeon, int x, int y, char dir, int spaces) : MovingTile(dungeon, x, y, spaces) {
	m_dir = dir;
}

void LinearMovingTile::setDirection() {
	switch (m_dir) {
	case 'l': m_dir = 'r'; break;
	case 'r': m_dir = 'l'; break;
	}
}

SquareMovingTile::SquareMovingTile(Dungeon &dungeon, int x, int y, char dir, int spaces, bool clockwise) 
	: MovingTile(dungeon, x, y, spaces), m_clockwise(clockwise) {
	m_dir = dir;
}

void SquareMovingTile::setDirection() {
	switch (m_dir) {
	case 'l': m_dir = m_clockwise ? 'u' : 'd'; break;
	case 'r': m_dir = m_clockwise ? 'd' : 'u'; break;
	case 'u': m_dir = m_clockwise ? 'r' : 'l'; break;
	case 'd': m_dir = m_clockwise ? 'l' : 'r'; break;
	}
}

LavaGrating::LavaGrating(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, LAVA_GRATING, "Cheese_Wedge_48x48.png", 0) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
	getSprite()->setScale(4.0f);

	m_lava = dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, "Lava_Tile1_48x48.png");
	m_lava->setVisible(false);
	m_lava->setScale(4.0f);

	dungeon.tileAt(getPos()).trap = false;
}

void LavaGrating::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	int bodyCount = 0;
	std::vector<int> monsterIndexes;
	bool heroOnTop = false;
	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {

			if (!m_dungeon->withinBounds(i, j))
				continue;

			if (m_dungeon->hero(i, j)) {
				heroOnTop = true;
				bodyCount++;
			}

			if (m_dungeon->enemy(i, j)) {
				int pos = m_dungeon->findMonster(i, j);
				if (pos != -1) {
					monsterIndexes.push_back(pos);
					bodyCount++;
				}			
			}
		}
	}

	if (bodyCount >= 3) {
		m_lava->setVisible(true);

		for (auto it : monsterIndexes) {
			m_dungeon->damageMonster(it, 8, DamageType::MAGICAL);
			if (m_dungeon->monsterAt(it)->canBeBurned())
				m_dungeon->giveAffliction(it, std::make_shared<Burn>(*m_dungeon->getPlayer(), 6));
		}

		if (heroOnTop) {
			m_dungeon->damagePlayer(8, DamageType::MELTING);
			if (m_dungeon->getPlayer()->canBeBurned())
				m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 6));
		}
	}
	else
		m_lava->setVisible(false);
}

//		LIGHT ABSORBER
LightAbsorber::LightAbsorber(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, LIGHT_ABSORBER, "Cheese_Wedge_48x48.png", 0) {
	setWallFlag(true);

	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	dungeon.tileAt(getPos()).wall = true;
}
void LightAbsorber::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	int px, py;
	Coords pos = getPos();
	Coords playerCoords = m_dungeon->getPlayerCoordsClosestTo(pos);

	//std::vector<Coords> coords = m_dungeon->getCoordsOfAllPlayers();

	//for (auto &it : coords) {
		px = playerCoords.x;
		py = playerCoords.y;

		if (m_activated && positionInRectangularRange(pos, playerCoords, 4, 4)) {

			m_dungeon->getPlayerAt(px, py)->setVision(m_dungeon->getPlayerAt(px, py)->getVision() + m_lightReductionLevel);

			if (positionInRectangularRange(pos, playerCoords, 1, 1))
				m_lightReductionLevel = 4;

			else if (positionInRectangularRange(pos, playerCoords, 2, 2))
				m_lightReductionLevel = 3;

			else if (positionInRectangularRange(pos, playerCoords, 3, 3))
				m_lightReductionLevel = 2;

			else if (positionInRectangularRange(pos, playerCoords, 4, 4))
				m_lightReductionLevel = 1;


			m_dungeon->getPlayerAt(px, py)->setVision(m_dungeon->getPlayerAt(px, py)->getVision() - m_lightReductionLevel);
		}
		else if (m_activated && !positionInRectangularRange(pos, playerCoords, 4, 4)) {
			m_dungeon->getPlayerAt(px, py)->setVision(m_dungeon->getPlayerAt(px, py)->getVision() + m_lightReductionLevel);
			m_lightReductionLevel = 1;
			m_activated = false;
		}
		else if (!m_activated && positionInRectangularRange(pos, playerCoords, 4, 4)) {
			m_dungeon->getPlayerAt(px, py)->setVision(m_dungeon->getPlayerAt(px, py)->getVision() - m_lightReductionLevel);
			m_activated = true;
		}
	//}
}

WatcherStatue::WatcherStatue(Dungeon &dungeon, int x, int y, std::string image, char dir) : Traps(&dungeon, x, y, WATCHER_STATUE, "Cheese_Wedge_48x48.png", 0), m_dir(dir) {
	setWallFlag(true);

	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	dungeon.tileAt(getPos()).wall = true;
}

void WatcherStatue::activeTrapAction() {
	if (m_cooldown > 0) {
		m_cooldown--;
		return;
	}

	if (m_primed) {
		react();
		m_cooldown = 3;
		m_primed = false;
		return;
	}

	Coords start = getPos();

	std::vector<Coords> coords = m_dungeon->getCoordsOfAllPlayers();

	for (auto &it : coords) {
		char move = m_dungeon->getPlayerAt(it)->facingDirection();
		if (hasLineOfSight(*m_dungeon, m_dir, start, it) && directionIsOppositeTo(m_dir, move)) {
			if (!m_primed) {
				playSound("Smasher_Spawns.mp3", m_dungeon->getPlayer()->getPos(), start);

				m_primed = true;
			}

			return;
		}
	}
}

FireballWatcher::FireballWatcher(Dungeon &dungeon, int x, int y, char dir) : WatcherStatue(dungeon, x, y, "Cheese_Wedge_48x48.png", dir) {

}

void FireballWatcher::react() {
	int x = getPosX();
	int y = getPosY();

	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	while (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		incrementDirectionalOffsets(m_dir, n, m);
	}

	// play fire blast explosion sound effect
	playSound("Fireblast_Spell2.mp3");

	x += n;
	y += m;

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			m_dungeon->runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);

			if (m_dungeon->enemy(i, j)) {

				int pos = m_dungeon->findMonster(i, j);
				if (pos != -1) {
					m_dungeon->damageMonster(pos, 10, DamageType::MAGICAL);

					// If they can be burned, roll for a high chance to burn
					if (m_dungeon->monsterAt(pos)->canBeBurned()) {
						int turns = 6 + (m_dungeon->getPlayer()->hasHarshAfflictions() ? 6 : 0);
						m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), turns));
					}
				}
			}

			if (m_dungeon->hero(i, j)) {
				m_dungeon->damagePlayer(10, DamageType::FIRE);
				if (m_dungeon->getPlayerAt(i, j)->canBeBurned())
					m_dungeon->getPlayerAt(i, j)->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 6));
			}

			checkBurn(i, j);
		}
	}
}

FreezeWatcher::FreezeWatcher(Dungeon &dungeon, int x, int y, char dir) : WatcherStatue(dungeon, x, y, "Cheese_Wedge_48x48.png", dir) {

}

void FreezeWatcher::react() {
	int x = getPosX();
	int y = getPosY();
	Coords start(x, y);

	std::vector<Coords> coords = m_dungeon->getCoordsOfAllPlayers();

	for (auto &it : coords) {
		if (hasLineOfSight(*m_dungeon, m_dir, start, it)) {
			if (m_dungeon->getPlayerAt(it.x, it.y)->canBeFrozen())
				m_dungeon->getPlayerAt(it.x, it.y)->addAffliction(std::make_shared<Freeze>(3));

			return;
		}
	}

	/*if (m_dungeon->getPlayer()->canBeFrozen())
		m_dungeon->getPlayer()->addAffliction(std::make_shared<Freeze>(3));*/
}

DartWatcher::DartWatcher(Dungeon &dungeon, int x, int y, char dir) : WatcherStatue(dungeon, x, y, "Cheese_Wedge_48x48.png", dir) {

}

void DartWatcher::react() {
	int x = getPosX();
	int y = getPosY();

	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	while (!(m_dungeon->wall(x + n, y + m) || m_dungeon->enemy(x + n, y + m) || m_dungeon->hero(x + n, y + m))) {

		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		incrementDirectionalOffsets(m_dir, n, m);
	}

	playMiss();

	x += n;
	y += m;

	if (m_dungeon->enemy(x, y)) {

		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1) {
			m_dungeon->damageMonster(pos, 2, DamageType::MAGICAL);

			if (m_dungeon->monsterAt(pos)->canBePoisoned()) {
				int turns = 15;
				m_dungeon->giveAffliction(pos, std::make_shared<Poison>(*m_dungeon->getPlayer(), turns, 3, 2, 2));
			}
		}
	}

	if (m_dungeon->hero(x, y)) {
		int damage = 0;
		if (m_dungeon->getPlayerAt(x, y)->canBlock() &&
			m_dungeon->getPlayerAt(x, y)->didBlockAttackAt(getPosX(), getPosY(), damage))
			return;
		

		m_dungeon->damagePlayer(2, DamageType::NORMAL);
		if (m_dungeon->getPlayerAt(x, y)->canBePoisoned())
			m_dungeon->getPlayerAt(x, y)->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 15, 3, 2, 2));
	}
}

CrumbleWatcher::CrumbleWatcher(Dungeon &dungeon, int x, int y, char dir) : WatcherStatue(dungeon, x, y, "Cheese_Wedge_48x48.png", dir) {

}

void CrumbleWatcher::react() {
	int x = getPosX();
	int y = getPosY();
	Coords start(x, y);

	std::vector<Coords> coords = m_dungeon->getCoordsOfAllPlayers();

	for (auto &it : coords) {
		if (hasLineOfSight(*m_dungeon, m_dir, start, it)) {
			if (!m_dungeon->trap(it.x, it.y))
				m_dungeon->addTrap(std::make_shared<CrumblePit>(*m_dungeon, it.x, it.y, 1));
			
			return;
		}
	}
}

SpawnWatcher::SpawnWatcher(Dungeon &dungeon, int x, int y, char dir) : WatcherStatue(dungeon, x, y, "Cheese_Wedge_48x48.png", dir) {

}

void SpawnWatcher::react() {
	int x = m_dungeon->getPlayer()->getPosX();
	int y = m_dungeon->getPlayer()->getPosY();

	std::vector<Coords> coords;
	for (int i = x - 3; i < x + 4; i++) {
		for (int j = y - 3; j < y + 4; j++) {

			// Ignore all but the outer ring
			if (!m_dungeon->withinBounds(i, j) || (abs(i - x) <= 2 || abs(j - y) <= 2))
				continue;

			coords.push_back(Coords(i, j));
		}
	}

	int n = 3; // Number of enemies to spawn
	while (!coords.empty() && n > 0) {
		int index = randInt(0, static_cast<int>(coords.size()) - 1);
		Coords pos = coords[index];
		coords.erase(coords.begin() + index);

		if (!m_dungeon->enemy(pos.x, pos.y)) {
			m_dungeon->addMonster(std::make_shared<SpectralSword>(m_dungeon, pos.x, pos.y));

			n--;
		}
	}
}

GuardianWatcher::GuardianWatcher(Dungeon &dungeon, int x, int y, char dir) : WatcherStatue(dungeon, x, y, "Cheese_Wedge_48x48.png", dir) {

}

void GuardianWatcher::react() {
	m_dungeon->addMonster(std::make_shared<Watcher>(m_dungeon, getPosX(), getPosY()));

	destroyTrap();
}

FlareCandle::FlareCandle(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, FLARE_CANDLE, "Cheese_Wedge_48x48.png", 0) {
	setCanBeDoused(true);

	auto frames = getAnimationFrameVector("Fire%04d.png", 8);
	setSprite(dungeon.runAnimationForever(frames, 24, x, y, 2));
	getSprite()->setScale(0.35f * GLOBAL_SPRITE_SCALE);
	getSprite()->setColor(cocos2d::Color3B(100, 100, 255));

	dungeon.addLightSource(x, y, y + Z_WALL, getName());
}

void FlareCandle::activeTrapAction() {
	if (randReal(1, 100) > 99) {
		if (!m_dungeon->enemy(getPosX(), getPosY()))
			m_dungeon->addMonster(std::make_shared<Flare>(m_dungeon, getPosX(), getPosY()));
	}
}
void FlareCandle::trapAction(Actors &a) {
	if (!a.isPlayer() || isDestroyed())
		return;

	playMiss();

	destroyTrap();
}
void FlareCandle::douse() {
	if (isDestroyed())
		return;

	destroyTrap();
}
void FlareCandle::spriteCleanup() {
	auto scale = cocos2d::ScaleTo::create(0.1f, 0.0f);
	auto fade = cocos2d::FadeOut::create(0.1f);
	cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	v.pushBack(scale);
	v.pushBack(fade);

	m_dungeon->queueRemoveSpriteWithMultipleActions(getSprite(), v);
}

Acid::Acid(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, ACID, "Cheese_Wedge_48x48.png", 8) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
}

void Acid::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->hero(x, y)) {
		if (!m_dungeon->getPlayer()->isFlying())
			m_dungeon->damagePlayer(getDamage(), DamageType::ACIDIC);
	}
	
	if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1 && !m_dungeon->monsterAt(pos)->isFlying())
			m_dungeon->damageMonster(getPos(), getDamage(), DamageType::ACIDIC);
	}

	if (m_dungeon->trap(x, y)) {
		auto indexes = m_dungeon->findTraps(x, y);
		for (size_t i = 0; i < indexes.size(); ++i) {
			if (m_dungeon->trapAt(indexes[i])->isDestructible())
				m_dungeon->trapAt(indexes[i])->destroyTrap();
		}
	}

	m_turns--;
	if (m_turns == 0)
		destroyTrap();
}
void Acid::trapAction(Actors &a) {
	if (a.isFlying())
		return;

	if (a.isPlayer()) {
		m_dungeon->damagePlayer(getDamage(), DamageType::ACIDIC);
	}
	else {
		m_dungeon->damageMonster(getPos(), getDamage(), DamageType::ACIDIC);
	}
}

DNASplitter::DNASplitter(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, DNA_SPLITTER, "Cheese_Wedge_48x48.png", 0) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}

void DNASplitter::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->hero(x, y))
		trapAction(*m_dungeon->getPlayer());	
}
void DNASplitter::trapAction(Actors &a) {
	if (!a.isPlayer() || isDestroyed())
		return;

	m_dungeon->clonePlayer();

	destroyTrap();
}


//		DECOY
Decoy::Decoy(Dungeon &dungeon, int x, int y, int range, int turns, std::string name, std::string image) : Traps(&dungeon, x, y, name, image, 0), m_attractRange(range), m_turns(turns) {

}

//		ROTTING DECOY
RottingDecoy::RottingDecoy(Dungeon &dungeon, int x, int y, int bites) : Decoy(dungeon, x, y, 8, bites, ROTTING_DECOY, "Honeycomb_48x48.png") {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_TOP, getImageName()));
}

void RottingDecoy::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (isDestroyed())
		return;

	// Find any monsters adjacent and poison them
	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			int pos = m_dungeon->findMonster(i, j);
			if (pos != -1)
				trapAction(*m_dungeon->monsterAt(pos));			
		}
	}
}
void RottingDecoy::trapAction(Actors &a) {

	// Does nothing to the player
	if (a.isPlayer())
		return;

	if (a.canBePoisoned())
		a.addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 2, 2, 1, 1));
	
	decTurns();
	if (getTurns() == 0)
		destroyTrap();
}


Spores::Spores(Dungeon &dungeon, int x, int y, int turns) : Traps(&dungeon, x, y, SPORES, "PoisonWater_48x48.png", 0), m_turns(turns) {
	setSprite(dungeon.createSprite(x, y, y + Z_FLOATERS, getImageName()));
	getSprite()->setOpacity(120);
}

void Spores::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->hero(x, y)) {
		//playSound("Fire3.mp3", *m_dungeon->getPlayer(), x, y);

		if (m_dungeon->getPlayer()->canBePoisoned())
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Poison>(*m_dungeon->getPlayer(), 6, 3, 2, 1));
	}

	if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1) {
			if (m_dungeon->monsterAt(pos)->canBePoisoned())
				m_dungeon->giveAffliction(pos, std::make_shared<Poison>(*m_dungeon->getPlayer(), 6, 3, 2, 1));
		}
	}

	m_turns--;
	if (m_turns == 0)
		destroyTrap();
}
void Spores::spriteCleanup() {
	if (getSprite() != nullptr) {
		auto fade = cocos2d::FadeOut::create(0.2f);
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(fade);

		m_dungeon->queueRemoveSpriteWithMultipleActions(getSprite(), v);
		setSprite(nullptr);
	}
}

DustDevil::DustDevil(Dungeon &dungeon, int x, int y, char dir) : Traps(&dungeon, x, y, DUST_DEVIL, "Firebar_Totem_48x48.png", 8), m_dir(dir) {
	setSprite(dungeon.createSprite(x, y, y + Z_PROJECTILE, getImageName()));
	getSprite()->setOpacity(220);
}

void DustDevil::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();
	
	int n, m;
	setDirectionalOffsets(m_dir, n, m);

	int range = 2;
	int currentRange = 1;
	while (!m_dungeon->wall(x + n, y + m) && currentRange <= range) {
		if (m_dungeon->hero(x + n, y + m)) {
			m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);

			moveTo(x + n, y + m);
			destroyTrap();
			return;
		}

		incrementDirectionalOffsets(m_dir, n, m);
		currentRange++;
	}

	if (currentRange == 3)
		decrementDirectionalOffsets(m_dir, n, m);

	moveTo(x + n, y + m);

	if (m_dungeon->wall(x + n, y + m))
		destroyTrap();
}

EnergyHelix::EnergyHelix(Dungeon &dungeon, int x, int y, char dir, char dir2) : Traps(&dungeon, x, y, ENERGY_HELIX, "Fireblast_Spell_48x48.png", 0), m_dir(dir), m_secondaryDir(dir2) {
	m_maxSpaces = m_spaces;
	m_currentDir = dir;

	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("Fire%04d.png", 8);
	setSprite(dungeon.runAnimationForever(frames, 24, x, y, y + Z_PROJECTILE));
	getSprite()->setScale(0.75f * GLOBAL_SPRITE_SCALE);

	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;

	dungeon.addLightSource(x, y, 3, getName());
}
void EnergyHelix::moveTo(int x, int y, float time) {
	m_dungeon->removeLightSource(getPosX(), getPosY(), getName());
	m_dungeon->addLightSource(x, y, 3, getName());

	m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	setPosX(x); setPosY(y);
}
void EnergyHelix::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->hero(x, y)) {
		playSound("Fire3.mp3", m_dungeon->getPlayer()->getPos(), getPos());

		if (m_dungeon->getPlayer()->canBeBurned())
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 7));

		destroyTrap();
		return;
	}

	if (m_dungeon->wall(x, y)) {
		destroyTrap();
		return;
	}

	if (m_spaces == 0) {
		m_spaces = m_maxSpaces;

		if (m_currentDir == m_dir)
			m_currentDir = m_secondaryDir;
		else
			m_currentDir = m_dir;
	}

	int n, m;
	setDirectionalOffsets(m_currentDir, n, m);

	moveTo(x + n, y + m);

	if (m_dungeon->hero(x + n, y + m)) {
		playSound("Fire3.mp3", m_dungeon->getPlayer()->getPos(), getPos());

		if (m_dungeon->getPlayer()->canBeBurned())
			m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 7));

		destroyTrap();
		return;
	}

	if (m_dungeon->wall(x + n, y + m)) {
		destroyTrap();
		return;
	}

	m_spaces--;
}
void EnergyHelix::spriteCleanup() {
	auto scale = cocos2d::ScaleTo::create(0.1f, 0.0f);
	auto fade = cocos2d::FadeOut::create(0.1f);
	cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	v.pushBack(scale);
	v.pushBack(fade);

	m_dungeon->queueRemoveSpriteWithMultipleActions(getSprite(), v);
}

AbyssalMaw::AbyssalMaw(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, ABYSSAL_MAW, "Puddle.png", 10) {
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
}

void AbyssalMaw::activeTrapAction() {
	if (m_wait > 0) {
		m_wait--;
		return;
	}

	int x = getPosX();
	int y = getPosY();

	playSound("Fireblast_Spell1.mp3");

	auto frames = getAnimationFrameVector("frame%04d.png", 63);
	m_dungeon->runSingleAnimation(frames, 120, x, y, y + Z_PROJECTILE);

	if (m_dungeon->hero(x, y))
		m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);

	destroyTrap();
}

Goop::Goop(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, GOOP, "Puddle.png", 0) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, getImageName()));
}

void Goop::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->hero(x, y))
		trapAction(*m_dungeon->getPlayer());
	else if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1)
			trapAction(*m_dungeon->monsterAt(pos));
	}
}
void Goop::trapAction(Actors &a) {
	if (a.isFlying())
		return;

	playSound("Puddle_Splash.mp3", m_dungeon->getPlayer()->getPos(), getPos());

	if (a.isPlayer())
		a.addAffliction(std::make_shared<Stuck>(2));
	else
		a.addAffliction(std::make_shared<Stun>(2));

	destroyTrap();
}

MiniEruption::MiniEruption(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, MINI_ERUPTION, "Puddle.png", 5) {
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;

	//setSprite(dungeon.createSprite(x, y, -1, getImageName()));
}

void MiniEruption::addCoords() {
	m_coords.clear();

	int x = getPosX();
	int y = getPosY();

	m_coords.push_back(Coords(x - m_radius, y));
	m_coords.push_back(Coords(x + m_radius, y));
	m_coords.push_back(Coords(x, y - m_radius));
	m_coords.push_back(Coords(x, y + m_radius));
}
void MiniEruption::activeTrapAction() {
	if (m_turns == -1) {
		m_turns++;
		return;
	}

	if (m_turns > m_maxTurns) {
		setDestroyed(true);
		return;
	}

	playSound("Fireblast_Spell1.mp3", m_dungeon->getPlayer()->getPos(), getPos());
	playSound("Earthquake_Spell2.mp3", m_dungeon->getPlayer()->getPos(), getPos());

	addCoords();

	for (size_t i = 0; i < m_coords.size(); ++i) {
		int x = m_coords[i].x;
		int y = m_coords[i].y;

		if (!m_dungeon->withinBounds(x, y))
			continue;

		if (!m_dungeon->wall(x, y) || m_dungeon->wallObject(x, y) && m_dungeon->wallObject(x, y)->isDestructible()) {
			auto frames = getAnimationFrameVector("frame%04d.png", 63);
			m_dungeon->runSingleAnimation(frames, 120, x, y, y + Z_PROJECTILE);
		}

		if (m_dungeon->wallObject(x, y) && m_dungeon->wallObject(x, y)->isDestructible())
			m_dungeon->destroyWall(x, y);
		
		if (m_dungeon->hero(x, y)) {
			m_dungeon->damagePlayer(getDamage(), DamageType::FIRE);
			if (!m_dungeon->getPlayer()->isFlying() && m_dungeon->getPlayer()->canBeBurned())
				m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 6));
		}

		if (m_dungeon->enemy(x, y)) {
			int pos = m_dungeon->findMonster(x, y);
			if (pos != -1) {
				if (!m_dungeon->monsterAt(pos)->isFlying() && m_dungeon->monsterAt(pos)->canBeBurned())
					m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), 6));
			}
		}
	}

	m_turns++;
	m_radius++;
}

Combustion::Combustion(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, COMBUSTION, "Diamond_48x48.png", 10) {
	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	setWallFlag(true);
	dungeon.tileAt(getPos()).wall = true;
}

void Combustion::activeTrapAction() {
	if (m_countdown > 0) {
		m_countdown--;
		return;
	}

	int x = getPosX();
	int y = getPosY();
	Coords start(getPos());

	playSound("Explosion.mp3");

	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {

			if (m_dungeon->hero(i, j))
				m_dungeon->damagePlayer(getDamage(), DamageType::EXPLOSIVE);

			if (m_dungeon->enemy(i, j))
				m_dungeon->damageMonster(Coords(i, j), getDamage(), DamageType::EXPLOSIVE);
		
			if (m_dungeon->wallObject(i, j) && m_dungeon->wallObject(i, j)->isDestructible())
				m_dungeon->destroyWall(i, j);

			if (m_dungeon->gold(i, j) != 0)
				m_dungeon->removeGold(i, j);

			checkExplosion(i, j);		
		}
	}

	for (int i = x - 3; i < x + 4; i++) {
		for (int j = y - 3; j < y + 4; j++) {
			if (!m_dungeon->withinBounds(i, j))
				continue;

			if (m_dungeon->hero(i, j) || m_dungeon->enemy(i, j))
				m_dungeon->linearActorPush(i, j, 3 - std::max(abs(i - x), abs(j - y)) + 1, 3, getFacingDirectionRelativeTo(start, Coords(i, j)));
		}
	}

	destroyTrap();
}

DirtMound::DirtMound(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, COMBUSTION, "Emerald_48x48.png", 0) {
	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));

	setWallFlag(true);
	dungeon.tileAt(getPos()).wall = true;
}

void DirtMound::trapAction(Actors &a) {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	playCrumble(exp(-(abs(px - x) + abs(py - y))) / 3);

	destroyTrap();
}

FacelessMask::FacelessMask(Dungeon &dungeon, int x, int y, char dir) : Traps(&dungeon, x, y, FACELESS_MASK, "Emerald_48x48.png", 6), m_dir(dir) {
	setSprite(dungeon.createSprite(x, y, y + Z_PROJECTILE, getImageName()));
}

void FacelessMask::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	int n = 0, m = 0;

	int range = m_acceleration;
	int currentRange = -1;

	while (currentRange < range) {
		currentRange++;

		if (m_dungeon->wall(x + n, y + m))
			break;

		if (m_dungeon->hero(x + n, y + m)) {
			m_dungeon->damagePlayer(getDamage(), DamageType::NORMAL);
			break;
		}

		incrementDirectionalOffsets(m_dir, n, m);
	}

	moveTo(x + n, y + m);

	if (m_dungeon->wall(x + n, y + m) || m_dungeon->hero(x + n, y + m))
		destroyTrap();
	
	m_acceleration++;
}

PsychicSlash::PsychicSlash(Dungeon &dungeon, int x, int y, char dir) : Traps(&dungeon, x, y, PSYCHIC_SLASH, "Emerald_48x48.png", 6), m_dir(dir) {
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;

	setCoords();

	for (int i = 0; i < 3; i++) {
		m_sprites.push_back(dungeon.createSprite(m_coords[i].x, m_coords[i].y, y + Z_PROJECTILE, getImageName()));
		m_sprites[i]->setOpacity(170);
	}
}

void PsychicSlash::moveTo(int x, int y, float time) {
	int n, m;
	setDirectionalOffsets(m_dir, n, m, m_acceleration);

	for (int i = 0; i < 3; i++) {
		if (m_coords[i] == getPos()) {
			setPosX(m_coords[i].x + n);
			setPosY(m_coords[i].y + m);
		}

		m_coords[i].x += n;
		m_coords[i].y += m;

		m_dungeon->queueMoveSprite(m_sprites[i], m_coords[i], time);
	}

	//m_dungeon->queueMoveSprite(getSprite(), x, y, time);
	//setPosX(x); setPosY(y);
}
void PsychicSlash::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();
	Coords start(getPos());

	Coords playerCoords = m_dungeon->getPlayerCoordsClosestTo(start);

	if (m_turnsLeft == 0) {
		if (m_acceleration == 2) {
			destroyTrap();
		}
		else {
			m_dir = getFacingDirectionRelativeTo(start, playerCoords);
			setCoords();
			m_turnsLeft = 5;
			m_acceleration++;
		}

		return;
	}

	int n = 0, m = 0;

	int range = m_acceleration;
	int currentRange = -1;
	bool playerWasHit = false, boundaryHit = false;

	while (currentRange < range) {
		currentRange++;

		for (size_t i = 0; i < m_coords.size(); ++i) {
			if (m_dungeon->boundary(m_coords[i].x + n, m_coords[i].y + m)) {
				boundaryHit = true;
				break;
			}

			if (m_dungeon->hero(m_coords[i].x + n, m_coords[i].y + m)) {
				int damage = getDamage();
				if (!(m_dungeon->getPlayerAt(m_coords[i] + Coords(n, m))->canBlock() && m_dungeon->getPlayerAt(m_coords[i] + Coords(n, m))->didBlockAttackAt(m_coords[i].x, m_coords[i].y, damage)))
					m_dungeon->damagePlayer(damage, DamageType::NORMAL);
				
				playerWasHit = true;			
				break;
			}
		}

		if (boundaryHit || playerWasHit)
			break;

		incrementDirectionalOffsets(m_dir, n, m);
	}

	if (!boundaryHit)
		moveTo(x + n, y + m);

	if (boundaryHit || playerWasHit)
		destroyTrap();

	m_turnsLeft--;
}
void PsychicSlash::setCoords() {
	int x = getPosX();
	int y = getPosY();

	m_coords.clear();

	m_coords.push_back(Coords(x, y));

	switch (m_dir) {
	case 'l':
	case 'r':
		m_coords.push_back(Coords(x, y - 1));
		m_coords.push_back(Coords(x, y + 1));
		break;
	case 'u':
	case 'd':
		m_coords.push_back(Coords(x - 1, y));
		m_coords.push_back(Coords(x + 1, y));
		break;
	case '1':
		m_coords.push_back(Coords(x + 1, y));
		m_coords.push_back(Coords(x, y + 1));
		break;
	case '2':
		m_coords.push_back(Coords(x - 1, y));
		m_coords.push_back(Coords(x, y + 1));
		break;
	case '3':
		m_coords.push_back(Coords(x + 1, y));
		m_coords.push_back(Coords(x, y - 1));
		break;
	case '4':
		m_coords.push_back(Coords(x - 1, y));
		m_coords.push_back(Coords(x, y - 1));
		break;
	}
}
void PsychicSlash::spriteCleanup() {
	for (unsigned int i = 0; i < m_sprites.size(); i++) {
		auto fade = cocos2d::FadeOut::create(0.1f);
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(fade);

		m_dungeon->queueRemoveSpriteWithMultipleActions(m_sprites[i], v);
	}
	
	m_sprites.clear();
}

DisablingField::DisablingField(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, DISABLING_FIELD, "Sapphire_48x48.png", 0) {
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;

	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_BOTTOM, getImageName()));
	getSprite()->setOpacity(100);
}

void DisablingField::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->hero(x, y)) {
		playSound("Possessed.mp3");
		tintStunned(m_dungeon->getPlayer()->getSprite());

		m_dungeon->getPlayer()->addAffliction(std::make_shared<Disabled>(1));
	}

	m_turns--;
	if (m_turns == 0)
		destroyTrap();
}

FlameArchaicFirePillars::FlameArchaicFirePillars(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, FLAME_ARCHAIC_FIRE_PILLARS, "Fireblast_Spell_48x48.png", 0) {
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}

void FlameArchaicFirePillars::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

	// If it traveled to the third ring, then stop
	if (m_ring > m_limit) {
		setDestroyed(true);
		return;
	}

	playSound("Fireblast_Spell1.mp3");

	// Find any monsters caught in the fire
	for (int i = x - (1 + m_ring); i < x + (2 + m_ring); i++) {
		for (int j = y - (1 + m_ring); j < y + (2 + m_ring); j++) {

			// Ignore all but the outer rings
			if (i != x - (1 + m_ring) && i != x + (2 + m_ring) - 1 && j != y - (1 + m_ring) && j != y + (2 + m_ring) - 1)
				continue;

			if (m_dungeon->withinBounds(i, j)) {

				if (!m_dungeon->wall(i, j)) {
					cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
					m_dungeon->runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);
				}

				if (m_dungeon->hero(i, j)) {
					if (m_dungeon->getPlayer()->canBeBurned())
						m_dungeon->getPlayer()->addAffliction(std::make_shared<Burn>(*m_dungeon->getPlayer(), 10));
				}

				if (m_dungeon->enemy(i, j)) {

					int pos = m_dungeon->findMonster(i, j);
					if (pos != -1) {

						if (m_dungeon->monsterAt(pos)->canBeBurned()) {
							int turns = 10;
							m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), turns));
						}
					}
				}

				checkBurn(i, j);
			}
		}
	}

	m_ring++;
}

MegaRock::MegaRock(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, MEGA_ROCK, "Diamond_48x48.png", 25) {
	setSprite(dungeon.createSprite((float)x, y - 1.8f, y + Z_FLOATERS, getImageName()));
	getSprite()->setScale(2.4f);
}

void MegaRock::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	m_height--;
	if (m_height == 1)
		m_dungeon->queueMoveSprite(getSprite(), (float)x, y - 0.9f);
	else
		m_dungeon->queueMoveSprite(getSprite(), x, y);

	if (m_height == 0) {
		playSound("Earthquake_Spell1.mp3");
		playSound("Earthquake_Spell2.mp3");

		for (int i = x - 1; i < x + 2; i++) {
			for (int j = y - 1; j < y + 2; j++) {
				if (m_dungeon->hero(i, j)) {
					m_dungeon->damagePlayer(getDamage(), DamageType::CRUSHING);
					if (m_dungeon->getPlayer()->canBeStunned())
						m_dungeon->getPlayer()->addAffliction(std::make_shared<Stun>(1));
				}

				if (m_dungeon->enemy(i, j))
					m_dungeon->damageMonster(Coords(i, j), getDamage(), DamageType::CRUSHING);

				if (m_dungeon->wall(i, j))
					m_dungeon->destroyWall(i, j);
			}
		}

		destroyTrap();
	}
}
void MegaRock::spriteCleanup() {
	auto fade = cocos2d::FadeOut::create(0.1f);
	cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	v.pushBack(fade);

	m_dungeon->queueRemoveSpriteWithMultipleActions(getSprite(), v);
}

ReflectiveShot::ReflectiveShot(Dungeon &dungeon, int x, int y, char dir, Actors *parent)
	: Traps(&dungeon, x, y, REFLECTIVE_SHOT, "Ruby_48x48.png", 10), m_dir(dir), m_parent(parent) {
	setSprite(dungeon.createSprite(x, y, y + Z_PROJECTILE, getImageName()));
	getSprite()->setOpacity(170);
}

void ReflectiveShot::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	int n = 0, m = 0;

	int range = 2;
	int currentRange = -1;

	while (currentRange < range) {
		currentRange++;

		if (m_dungeon->wall(x + n, y + m))
			break;

		if (m_dungeon->hero(x + n, y + m) && !m_deflected) {
			int damage = getDamage();
			if (m_dungeon->getPlayerAt(x + n, y + m)->canBlock() && m_dungeon->getPlayerAt(x + n, y + m)->didBlockAttackAt(x, y, damage))
				;
			else
				m_dungeon->damagePlayer(damage, DamageType::MAGICAL);

			moveTo(x + n, y + m);
			destroyTrap();
			return;
		}

		if (x + n == m_parent->getPosX() && y + m == m_parent->getPosY()) {
			m_dungeon->damageMonster(Coords(x + n, y + m), getDamage(), DamageType::MAGICAL);

			moveTo(x + n, y + m);
			destroyTrap();
			return;
		}

		if (currentRange == range)
			break;

		incrementDirectionalOffsets(m_dir, n, m);
	}

	moveTo(x + n, y + m);

	if (m_dungeon->wall(x + n, y + m))
		destroyTrap();
}
void ReflectiveShot::trapAction(Actors &a) {
	if (!a.isPlayer())
		return;

	playHitSmasher();
	m_dir = m_dungeon->getPlayer()->facingDirection();
	m_deflected = true;
}
void ReflectiveShot::spriteCleanup() {
	auto fade = cocos2d::FadeOut::create(0.1f);
	cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	v.pushBack(fade);

	m_dungeon->queueRemoveSpriteWithMultipleActions(getSprite(), v);
}

LightBeam::LightBeam(Dungeon &dungeon, int x, int y, char dir) : Traps(&dungeon, x, y, LIGHT_BEAM, "Sapphire_48x48.png", 4), m_dir(dir) {
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;

	m_coords.push_back(Coords(x, y));
	m_coords.push_back(Coords(x, y));
	m_coords.push_back(Coords(x, y));

	for (int i = 0; i < 3; i++) {
		m_sprites.push_back(dungeon.createSprite(m_coords[i].x, m_coords[i].y, y + Z_PROJECTILE, getImageName()));
		m_sprites[i]->setOpacity(170);

		m_dirs.push_back(dir);
	}

	/*setSprite(dungeon.createSprite(x, y, y + Z_PROJECTILE, getImageName()));
	getSprite()->setOpacity(170);*/
}

void LightBeam::moveBeam(float time) {
	for (int i = 0; i < 3; i++) {
		m_dungeon->queueMoveSprite(m_sprites[i], m_coords[i].x, m_coords[i].y, time);
	}
}
void LightBeam::activeTrapAction() {
	int n = 0, m = 0;

	int range = m_acceleration;
	int currentRange = -1;
	bool wallHit = false;

	for (int i = 0; i < (int)m_coords.size(); i++) {
		while (currentRange < range) {
			currentRange++;

			if (m_dungeon->wall(m_coords[i].x + n, m_coords[i].y + m)) {
				wallHit = true;

				decrementDirectionalOffsets(m_dirs[i], n, m);
				m_coords[i].x += n;
				m_coords[i].y += m;

				m_dirs[i] = changeTrajectory(m_dirs[i]);
				n = 0;
				m = 0;
			}

			if (m_dungeon->hero(m_coords[i].x + n, m_coords[i].y + m)) {
				m_dungeon->damagePlayer(getDamage(), DamageType::PIERCING);
			}

			if (currentRange == range)
				break;

			incrementDirectionalOffsets(m_dirs[i], n, m);
		}

		m_coords[i].x += n;
		m_coords[i].y += m;

		currentRange = -1;
		n = 0;
		m = 0;

		if (m_initialMove)
			range--;	
	}

	if (m_initialMove)
		m_initialMove = false;

	moveBeam();

	if (wallHit)
		bouncesLeft--;

	if (bouncesLeft == 0)
		destroyTrap();
}
char LightBeam::changeTrajectory(char dir) {
	std::vector<char> moves;

	switch (dir) {
	case 'l': moves.push_back('r'); break;
	case 'r': moves.push_back('l'); break;
	case 'u': moves.push_back('d'); break;
	case 'd': moves.push_back('u'); break;
	case '1':
		moves.push_back('2');
		moves.push_back('3');
		moves.push_back('4');
		break;
	case '2':
		moves.push_back('1');
		moves.push_back('3');
		moves.push_back('4');
		break;
	case '3':
		moves.push_back('1');
		moves.push_back('2');
		moves.push_back('4');
		break;
	case '4':
		moves.push_back('1');
		moves.push_back('2');
		moves.push_back('3');
		break;
	}

	int x = getPosX();
	int y = getPosY();

	int n, m;

	int index = randInt(0, static_cast<int>(moves.size()) - 1);
	setDirectionalOffsets(moves[index], n, m);

	return moves[index];

	//while (!moves.empty()) {
	//	int index = randInt(0, (int)moves.size() - 1);
	//	setDirectionalOffsets(moves[index], n, m);

	//	//if (!m_dungeon->wall(x + n, y + m))
	//		return moves[index];
	//	
	//	moves.erase(moves.begin() + index);
	//}
}
void LightBeam::spriteCleanup() {
	for (unsigned int i = 0; i < m_sprites.size(); i++) {
		auto fade = cocos2d::FadeOut::create(0.1f);
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(fade);

		m_dungeon->queueRemoveSpriteWithMultipleActions(m_sprites[i], v);
	}

	m_sprites.clear();
}


BananaPeel::BananaPeel(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, BANANA_PEEL, "Waffles_48x48.png", 0) {
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_TOP, getImageName()));
}
void BananaPeel::trapAction(Actors &a) {
	if (a.isFlying())
		return;

	playSound("Puddle_Slip.mp3", m_dungeon->getPlayer()->getPos(), getPos());

	a.getSprite()->setRotation(90);

	a.addAffliction(std::make_shared<Stun>(2));
	a.addAffliction(std::make_shared<Slipped>(1));

	destroyTrap();
}

FactoryTile::FactoryTile(Dungeon &dungeon, int x, int y, char dir) : Traps(&dungeon, x, y, FACTORY_TILE, "Fireblast_Spell_48x48.png", 0), m_dir(dir) {
	std::string image;
	switch (m_dir) {
	case 'l': image = "Spring_Arrow_Left_48x48.png"; break;
	case 'r': image = "Spring_Arrow_Right_48x48.png"; break;
	case 'u': image = "Spring_Arrow_Up_48x48.png"; break;
	case 'd': image = "Spring_Arrow_Down_48x48.png"; break;
	}
	setSprite(dungeon.createSprite(x, y, y + Z_TRAP_MIDDLE, image));
}
void FactoryTile::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_dungeon->enemy(x, y)) {
		int pos = m_dungeon->findMonster(x, y);
		if (pos != -1) {
			int n, m;
			setDirectionalOffsets(m_dir, n, m);

			if (!m_dungeon->tileIsBlocked(x + n, y + m)) {
				m_dungeon->giveAffliction(pos, std::make_shared<Stun>(1));
				m_dungeon->monsterAt(pos)->moveTo(x + n, y + m);
			}
		}
	}
}

//		FIRE PILLARS
FirePillars::FirePillars(Dungeon &dungeon, int x, int y, int limit) : Traps(&dungeon, x, y, FIRE_PILLARS, "Fireblast_Spell_48x48.png", 0), m_limit(limit) {
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}
void FirePillars::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	int px = m_dungeon->getPlayer()->getPosX();
	int py = m_dungeon->getPlayer()->getPosY();

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

			if (m_dungeon->withinBounds(i, j)) {

				if (!m_dungeon->wall(i, j)) {
					cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
					m_dungeon->runSingleAnimation(frames, 120, i, j, j + Z_PROJECTILE);
				}

				if (m_dungeon->enemy(i, j)) {

					int pos = m_dungeon->findMonster(i, j);
					if (pos != -1) {

						// If they can be burned
						if (m_dungeon->monsterAt(pos)->canBeBurned() || m_dungeon->getPlayer()->hasAfflictionOverride()) {
							int turns = 10 + m_dungeon->getPlayer()->getInt() + (m_dungeon->getPlayer()->hasHarshAfflictions() ? 6 : 0);
							m_dungeon->giveAffliction(pos, std::make_shared<Burn>(*m_dungeon->getPlayer(), turns));
						}
					}
				}

				checkBurn(i, j);
			}
		}
	}

	m_ring++;
}

//		ICE SHARDS
IceShards::IceShards(Dungeon &dungeon, int x, int y, int limit) : Traps(&dungeon, x, y, ICE_SHARDS, "Sapphire_48x48.png", 4), m_limit(limit) {
	setSprite(dungeon.createSprite(x, y, y + Z_PROJECTILE, getImageName()));

	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}
void IceShards::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_shards == 0) {
		m_shards++;
		return;
	}

	if (m_shards > m_limit) {
		destroyTrap();
		return;
	}

	playSound("Freeze_Spell1.mp3");

	int n, m;

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

		while (!m_dungeon->wall(x + n, y + m)) {

			// fire explosion animation
			cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
			m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

			if (m_dungeon->enemy(x + n, y + m)) {
				int pos = m_dungeon->findMonster(x + n, y + m);
				if (pos != -1) {

					// Attempt freeze
					if (m_dungeon->monsterAt(pos)->canBeFrozen() || m_dungeon->getPlayer()->hasAfflictionOverride()) {
						if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 40 - m_dungeon->getPlayer()->getInt()) {
							int turns = 3 + m_dungeon->getPlayer()->getInt() + (m_dungeon->getPlayer()->hasHarshAfflictions() ? 3 : 0);
							m_dungeon->giveAffliction(pos, std::make_shared<Freeze>(turns));
						}
					}
					
					m_dungeon->damageMonster(pos, getDamage() + m_dungeon->getPlayer()->getInt(), DamageType::PIERCING);
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
		}

		k++;
	}

	m_shards++;
}

//		HAIL STORM
HailStorm::HailStorm(Dungeon &dungeon, int x, int y, char dir, int limit) : Traps(&dungeon, x, y, HAIL_STORM, "Sapphire_48x48.png", 4), m_dir(dir), m_limit(limit) {
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}
void HailStorm::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

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
			if (m_dungeon->withinBounds(x + n, y + m) && randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 40 - m_dungeon->getPlayer()->getInt()) {
				// fire explosion animation
				cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
				m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

				if (m_dungeon->enemy(x + n, y + m)) {
					int pos = m_dungeon->findMonster(x + n, y + m);
					if (pos != -1) {

						// Attempt freeze
						if (m_dungeon->monsterAt(pos)->canBeFrozen() || m_dungeon->getPlayer()->hasAfflictionOverride()) {
							if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 25 - m_dungeon->getPlayer()->getInt()) {
								int turns = 3 + m_dungeon->getPlayer()->getInt() + (m_dungeon->getPlayer()->hasHarshAfflictions() ? 3 : 0);
								m_dungeon->giveAffliction(pos, std::make_shared<Freeze>(turns));
							}
						}

						m_dungeon->damageMonster(pos, getDamage() + m_dungeon->getPlayer()->getInt(), DamageType::MAGICAL);
					}
				}

				checkFreeze(x + n, y + m);
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
Shockwaves::Shockwaves(Dungeon &dungeon, int x, int y, char dir, int limit) : Traps(&dungeon, x, y, SHOCKWAVES, "Sapphire_48x48.png", 6), m_dir(dir), m_limit(limit) {
	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}
void Shockwaves::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_waves > m_limit) {
		setDestroyed(true);
		return;
	}

	int n, m;

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
		if (!m_dungeon->withinBounds(x + n, y + m))
			continue;

		//playSound("Earthquake_Spell2.mp3");

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		if (m_dungeon->enemy(x + n, y + m)) {
			int pos = m_dungeon->findMonster(x + n, y + m);
			if (pos != -1) {
				if (m_dungeon->monsterAt(pos)->canBeStunned())
					m_dungeon->giveAffliction(pos, std::make_shared<Stun>(2));
				
				m_dungeon->damageMonster(pos, getDamage(), DamageType::MAGICAL);
			}
		}

		if (m_dungeon->underEnemy(x + n, y + m)) {
			int pos = m_dungeon->findUndergroundMonster(x + n, y + m);
			if (pos != -1) {
				if (m_dungeon->monsterAt(pos)->canBeStunned())
					m_dungeon->giveAffliction(pos, std::make_shared<Stun>(2));

				m_dungeon->damageMonster(pos, getDamage(), DamageType::MAGICAL);
			}
		}

		// 50% base chance to destroy walls
		if (m_dungeon->wallObject(x + n, y + m) && m_dungeon->wallObject(x + n, y + m)->isDestructible()) {
			if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 50 - m_dungeon->getPlayer()->getInt()) {
				m_dungeon->destroyWall(x + n, y + m);
			}
		}

		// 50% base chance to destroy destructible traps
		if (m_dungeon->trap(x + n, y + m)) {

			std::vector<int> indexes = m_dungeon->findTraps(x, y);
			for (int i = 0; i < (int)indexes.size(); i++) {

				if (m_dungeon->trapAt(indexes.at(i))->isDestructible()) {
					if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 50 - m_dungeon->getPlayer()->getInt())
						m_dungeon->trapAt(indexes.at(i))->destroyTrap();
				}

				else if (m_dungeon->trapAt(indexes.at(i))->isExplosive())
					m_dungeon->trapAt(indexes.at(i))->explode();
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
		if (!m_dungeon->withinBounds(x + n, y + m))
			continue;

		playSound("Earthquake_Spell2.mp3");

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		if (m_dungeon->enemy(x + n, y + m)) {
			int pos = m_dungeon->findMonster(x + n, y + m);
			if (pos != -1) {
				if (m_dungeon->monsterAt(pos)->canBeStunned())
					m_dungeon->giveAffliction(pos, std::make_shared<Stun>(2));

				m_dungeon->damageMonster(pos, getDamage() / 2, DamageType::MAGICAL);
			}
		}

		if (m_dungeon->underEnemy(x + n, y + m)) {
			int pos = m_dungeon->findUndergroundMonster(x + n, y + m);
			if (pos != -1) {
				if (m_dungeon->monsterAt(pos)->canBeStunned())
					m_dungeon->giveAffliction(pos, std::make_shared<Stun>(2));

				m_dungeon->damageMonster(pos, getDamage(), DamageType::MAGICAL);
			}
		}

		// 50% base chance to destroy walls
		if (m_dungeon->wallObject(x + n, y + m) && m_dungeon->wallObject(x + n, y + m)->isDestructible()) {
			if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 50 - m_dungeon->getPlayer()->getInt()) {
				m_dungeon->destroyWall(x + n, y + m);
			}
		}

		// 50% base chance to destroy destructible traps
		if (m_dungeon->trap(x + n, y + m)) {
			std::vector<int> indexes = m_dungeon->findTraps(x, y);
			for (int i = 0; i < (int)indexes.size(); i++) {

				if (m_dungeon->trapAt(indexes.at(i))->isDestructible()) {
					if (randReal(1, 100) + m_dungeon->getPlayer()->getLuck() > 50 - m_dungeon->getPlayer()->getInt())
						m_dungeon->trapAt(indexes.at(i))->destroyTrap();
				}

				else if (m_dungeon->trapAt(indexes.at(i))->isExplosive())
					m_dungeon->trapAt(indexes.at(i))->explode();
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
RockSummon::RockSummon(Dungeon &dungeon, int x, int y) : Traps(&dungeon, x, y, ROCK_SUMMON, "Diamond_48x48.png", 8) {
	setWallFlag(true);

	// Makes the rock interactable
	dungeon.tileAt(getPos()).wall = true;
	dungeon.tileAt(getPos()).trap = true;

	setSprite(dungeon.createSprite(x, y, y + Z_WALL, getImageName()));
}
void RockSummon::trapAction(Actors &a) {
	int x = getPosX();
	int y = getPosY();

	// Only the player can interact with this
	if (!a.isPlayer())
		return;

	char move = m_dungeon->getPlayer()->facingDirection();
	
	int n = 0, m = 0;
	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	}

	while (!m_dungeon->wall(x + n, y + m)) {

		// fire explosion animation
		cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
		m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

		if (m_dungeon->enemy(x + n, y + m)) {
			playSound("Earthquake_Spell1.mp3");

			int pos = m_dungeon->findMonster(x + n, y + m);
			if (pos != -1) {
				if (m_dungeon->monsterAt(pos)->canBeStunned())
					m_dungeon->giveAffliction(pos, std::make_shared<Stun>(4));

				m_dungeon->damageMonster(pos, getDamage(), DamageType::MAGICAL);

				m_dungeon->tileAt(Coords(x, y)).wall = false; // Unmark its spawn location as a wall
				setDestroyed(true);
				m_dungeon->queueRemoveSprite(getSprite());
				return;
			}
		}

		switch (move) {
		case 'l': n--; m = 0; break;
		case 'r': n++; m = 0; break;
		case 'u': n = 0; m--; break;
		case 'd': n = 0; m++; break;
		}
	}
	
	playSound("Earthquake_Spell1.mp3");

	// fire explosion animation
	cocos2d::Vector<cocos2d::SpriteFrame*> frames = getAnimationFrameVector("frame%04d.png", 63);
	m_dungeon->runSingleAnimation(frames, 120, x + n, y + m, y + m + Z_PROJECTILE);

	if (m_dungeon->wallObject(x + n, y + m) && m_dungeon->wallObject(x + n, y + m)->isDestructible())
		m_dungeon->destroyWall(x + n, y + m);

	else if (m_dungeon->wall(x + n, y + m) && m_dungeon->trap(x + n, y + m)) {
		m_dungeon->trapEncounter(x + n, y + m);
	}

	destroyTrap();
}

//		WIND VORTEX
WindVortex::WindVortex(Dungeon &dungeon, int x, int y, int limit) : Traps(&dungeon, x, y, WIND_VORTEX, "Emerald_48x48.png", 5), m_limit(limit) {
	setSprite(dungeon.createSprite(x, y, y + Z_PROJECTILE, getImageName()));

	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}
void WindVortex::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();
	Coords start(getPos());

	if (m_turns == 0) {
		m_turns++;
		return;
	}

	if (m_turns > m_limit) {
		destroyTrap();
		return;
	}

	playSound("Wind_Spell1.mp3", m_dungeon->getPlayer()->getPos(), getPos());
	playSound("Wind_Spell2.mp3", m_dungeon->getPlayer()->getPos(), getPos());

	for (int i = x - 2; i < x + 3; i++) {
		for (int j = y - 2; j < y + 3; j++) {

			if (!m_dungeon->withinBounds(i, j))
				continue;

			// Inner ring
			if (abs(x - i) <= 1 && abs(y - j) <= 1) {
				if (m_dungeon->enemy(i, j)) {
					// Eye of the storm
					if (i == x && j == y) {
						int pos = m_dungeon->findMonster(i, j);
						if (pos != -1) {
							m_dungeon->damageMonster(pos, getDamage() + m_dungeon->getPlayer()->getInt(), DamageType::MAGICAL);
						}
					}
					else {
						int pos = m_dungeon->findMonster(i, j);
						if (pos != -1) {
							if (m_dungeon->monsterAt(pos)->canBeStunned())
								m_dungeon->giveAffliction(pos, std::make_shared<Stun>(1));
						}
					}
				}
				// Pull a new enemy into the center, if possible
				else if (!m_dungeon->enemy(i, j) && !m_dungeon->hero(i, j) && i == x && j == y) {
					bool stop = false;

					for (int p = x - 1; p < x + 2; p++) {
						for (int q = y - 1; q < y + 2; q++) {
							if (m_dungeon->enemy(p, q)) {
								int pos = m_dungeon->findMonster(p, q);
								if (pos != -1) {
									m_dungeon->monsterAt(pos)->moveTo(x, y);
									if (m_dungeon->enemy(x, y))
										m_dungeon->singleMonsterTrapEncounter(pos);

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
				if (m_dungeon->enemy(i, j)) {
					char pullDirection = getFacingDirectionRelativeTo(start, Coords(i, j));
					m_dungeon->linearActorPush(i, j, 1, 1, pullDirection, true);
				}
			}		
		}
	}

	m_turns++;
}

//		THUNDER CLOUD
ThunderCloud::ThunderCloud(Dungeon &dungeon, int x, int y, char dir, int limit) : Traps(&dungeon, x, y, THUNDER_CLOUD, "Emerald_48x48.png", 5), m_dir(dir), m_limit(limit) {
	setSprite(dungeon.createSprite(getPos(), getHalfTileYOffset(), y + Z_FLOATERS, getImageName()));
	dungeon.addLightSource(x, y, 3, getName());

	if (dungeon.countTrapNumber(x, y) <= 1)
		dungeon.tileAt(getPos()).trap = false;
}
void ThunderCloud::activeTrapAction() {
	int x = getPosX();
	int y = getPosY();

	if (m_turns > m_limit) {
		destroyTrap();
		return;
	}

	m_turns++;

	int n = 0, m = 0;
	setDirectionalOffsets(m_dir, n, m);

	// If this is a waiting turn, then attack the current spot
	if (m_wait > 0) {
		n = 0;
		m = 0;
	}
	
	if (m_dungeon->enemy(x + n, y + m)) {
		playSound("Shock5.mp3");

		int pos = m_dungeon->findMonster(x + n, y + m);
		if (pos != -1) {
			m_dungeon->damageMonster(pos, getDamage(), DamageType::LIGHTNING);
			m_dungeon->chainLightning(m_dungeon->monsterAt(pos)->getPos());
		}
	}

	if (m_wait > 0) {
		m_wait--;
		return;
	}

	if (!m_dungeon->boundary(x + n, y + m)) {
		setPosX(x + n); setPosY(y + m);
		m_dungeon->queueMoveSprite(getSprite(), getPosX(), getPosY());
		m_dungeon->removeLightSource(x, y, getName());
		m_dungeon->addLightSource(getPosX(), getPosY(), 3, getName());
	}

	m_wait = 1;
}

