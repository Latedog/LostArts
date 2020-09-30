#include "cocos2d.h"
#include "global.h"
#include "GameUtils.h"
#include "GameObjects.h"
#include "Dungeon.h"
#include "Actors.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <fstream>

std::string GAME_TEXT_FILE = EN_US_FILE;
float GLOBAL_SPRITE_SCALE = 1.0f;

std::unordered_map<std::string, bool> LootTable::m_passivesObtained;
std::unordered_map<std::string, bool> LootTable::m_relicsObtained;
std::unordered_map<std::string, bool> NPCTable::m_NPCsEncountered;

unsigned int LootTable::m_commonPassiveCount = 0;
unsigned int LootTable::m_uncommonPassiveCount = 0;
unsigned int LootTable::m_rarePassiveCount = 0;
unsigned int LootTable::m_mythicalPassiveCount = 0;
unsigned int LootTable::m_legendaryPassiveCount = 0;

std::map<std::string, void*> GameTimers::m_gameTimers;

rapidjson::Document TextUtils::jsonTree;

void GameTable::initializeTables() {
	LootTable::initializeLootTable();
	NPCTable::initializeNPCTable();
}

std::shared_ptr<Drops> rollItem(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough, bool create) {

	if (rarity == Rarity::ALL)
		rarity = rollRarity();
	
	std::shared_ptr<Drops> item(nullptr);

	switch (rarity) {
	case Rarity::LEGENDARY: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {

			break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {

			break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 80) {
			switch (randInt(1, 3)) {
			case 1: item = std::make_shared<SoulPotion>(x, y); break;
			case 2:	item = std::make_shared<HalfLifePotion>(x, y); break;
			case 3: item = std::make_shared<ArmorDrop>(x, y); break;
			//case 4: item = std::make_shared<ShieldRepair>(x, y); break;
			//case 5: item = std::make_shared<Poison>(x, y); break;
			//case 6: item = std::make_shared<SmokeBomb>(x, y); break;
			//case 7: item = std::make_shared<StatPotion>(x, y); break;
			//case 8: item = std::make_shared<RottenMeat>(x, y); break;
			//case 9: item = std::make_shared<Matches>(x, y); break;
			//case 10: item = std::make_shared<Firecrackers>(x, y); break;
			//case 11: item = std::make_shared<BearTrap>(x, y); break;
			default: break;
			}

			break;
		}
	}
	case Rarity::UNCOMMON: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 70) {
			switch (randInt(1, 6)) {
			case 1: item = std::make_shared<BigLifePotion>(x, y); break;
			case 2:	item = std::make_shared<PoisonBomb>(x, y); break;
			case 3: item = std::make_shared<RottenApple>(x, y); break;
			case 4: item = std::make_shared<BinaryLifePotion>(x, y); break;
			case 5: item = std::make_shared<Teleport>(x, y); break;
			case 6: item = std::make_shared<SteadyLifePotion>(x, y); break;
			//case 7: item = std::make_shared<StatPotion>(x, y); break;
			//case 8: item = std::make_shared<RottenMeat>(x, y); break;
			//case 9: item = std::make_shared<Matches>(x, y); break;
			//case 10: item = std::make_shared<Firecrackers>(x, y); break;
			//case 11: item = std::make_shared<BearTrap>(x, y); break;
			default: break;
			}

			break;
		}
	}
	case Rarity::COMMON: {
		switch (randInt(1, 11)) {
		case 1: item = std::make_shared<Bomb>(x, y); break;
		case 2:	item = std::make_shared<Matches>(x, y); break;
		case 3: item = std::make_shared<Firecrackers>(x, y); break;
		case 4: item = std::make_shared<SmokeBomb>(x, y); break;
		case 5: item = std::make_shared<BearTrap>(x, y); break;
		case 6: item = std::make_shared<RottenMeat>(x, y); break;
		case 7: item = std::make_shared<StatPotion>(x, y); break;
		case 8: item = std::make_shared<LifePotion>(x, y); break;
		case 9: item = std::make_shared<Antidote>(x, y); break;
		case 10: {
			if (dungeon->getPlayer()->getName() == ADVENTURER)
				item = std::make_shared<ShieldRepair>(x, y);
			else
				item = std::make_shared<Bomb>(x, y);

			break;
		}
		case 11: item = std::make_shared<DizzyElixir>(x, y); break;
		default: break;
		}
	}
	default: break;
	}

	if (create) {
		Coords pos(x, y);

		dungeon->tileAt(pos).object = item;
		dungeon->tileAt(pos).object->setSprite(dungeon->createSprite(pos, pos.y + Z_ITEM, dungeon->tileAt(pos).object->getImageName()));
		dungeon->tileAt(pos).item = true;
		dungeon->addItem(dungeon->tileAt(pos).object);
	}

	return item;
}
std::shared_ptr<Spell> rollSpell(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough) {
	if (rarity == Rarity::ALL)
		rarity = rollRarity();

	std::shared_ptr<Spell> item(nullptr);

	switch (rarity) {
	case Rarity::LEGENDARY: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {
			;
			//break;
		}
	}
	case Rarity::MYTHICAL: {
		
	}
	case Rarity::RARE: {
		
	}
	case Rarity::UNCOMMON: {
		
	}
	case Rarity::COMMON: {
		switch (randInt(1, 15)) {
		case 1: item = std::make_shared<FreezeSpell>(x, y); break;
		case 2:	item = std::make_shared<IceShardSpell>(x, y); break;
		case 3: item = std::make_shared<HailStormSpell>(x, y); break;
		case 4: item = std::make_shared<EarthquakeSpell>(x, y); break;
		case 5: item = std::make_shared<RockSummonSpell>(x, y); break;
		case 6: item = std::make_shared<ShockwaveSpell>(x, y); break;
		case 7: item = std::make_shared<FireBlastSpell>(x, y); break;
		case 8: item = std::make_shared<FireCascadeSpell>(x, y); break;
		case 9: item = std::make_shared<FireExplosionSpell>(x, y); break;
		case 10: item = std::make_shared<WhirlwindSpell>(x, y); break;
		case 11: item = std::make_shared<WindVortexSpell>(x, y); break;
		case 12: item = std::make_shared<WindBlastSpell>(x, y); break;
		case 13: item = std::make_shared<ThunderCloudSpell>(x, y); break;
		case 14: item = std::make_shared<InvisibilitySpell>(x, y); break;
		case 15: item = std::make_shared<EtherealSpell>(x, y); break;
		default: break;
		}
	}
	default: break;
	}

	return item;
}
std::shared_ptr<Passive> rollPassive(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough) {
	bool allowAll = false;
	if (rarity == Rarity::ALL) {
		allowAll = true;
		rarity = rollRarity();
	}

	Rarity actualRarity;

	std::shared_ptr<Passive> item(nullptr);

	switch (rarity) {
	case Rarity::LEGENDARY: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {

			break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {
			switch (randInt(1, 5)) {
			case 1: item = std::make_shared<LavaImmune>(x, y); break;
			case 2:	item = std::make_shared<RainbowTouch>(x, y); break;
			case 3: item = std::make_shared<SoulSplit>(x, y); break;
			case 4: item = std::make_shared<ResonantSpells>(x, y); break;
			case 5: item = std::make_shared<Flying>(x, y); break;
			//case 6: item = std::make_shared<SmokeBomb>(x, y); break;
			//case 7: item = std::make_shared<StatPotion>(x, y); break;
			//case 8: item = std::make_shared<RottenMeat>(x, y); break;
			//case 9: item = std::make_shared<Matches>(x, y); break;
			//case 10: item = std::make_shared<Firecrackers>(x, y); break;
			//case 11: item = std::make_shared<BearTrap>(x, y); break;
			default: break;
			}

			actualRarity = Rarity::MYTHICAL;
			break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 80) {
			switch (randInt(1, 8)) {
			case 1: item = std::make_shared<BrickBreaker>(x, y); break;
			case 2:	item = std::make_shared<SteelPunch>(x, y); break;
			case 3: item = std::make_shared<Heavy>(x, y); break;
			case 4: item = std::make_shared<IronCleats>(x, y); break;
			case 5: item = std::make_shared<FireImmune>(x, y); break;
			case 6: item = std::make_shared<PoisonImmune>(x, y); break;
			case 7: item = std::make_shared<FrostTouch>(x, y); break;
			case 8: item = std::make_shared<BombImmune>(x, y); break;
			//case 9: item = std::make_shared<BrickBreaker>(x, y); break;
			//case 10: item = std::make_shared<Firecrackers>(x, y); break;
			//case 11: item = std::make_shared<BearTrap>(x, y); break;
			default: break;
			}

			actualRarity = Rarity::RARE;
			break;
		}
	}
	case Rarity::UNCOMMON: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {
			switch (randInt(1, 10)) {
			case 1: item = std::make_shared<CheapShops>(x, y); break;
			case 2:	item = std::make_shared<TrapIllumination>(x, y); break;
			case 3: item = std::make_shared<ItemIllumination>(x, y); break;
			case 4: item = std::make_shared<MonsterIllumination>(x, y); break;
			case 5: item = std::make_shared<Berserk>(x, y); break;
			case 6: item = std::make_shared<LifeSteal>(x, y); break;
			case 7: item = std::make_shared<FireTouch>(x, y); break;
			case 8: item = std::make_shared<PoisonTouch>(x, y); break;
			case 9: item = std::make_shared<PotionAlchemy>(x, y); break;
			case 10: item = std::make_shared<FatStacks>(x, y); break;
			//case 11: item = std::make_shared<BearTrap>(x, y); break;
			default: break;
			}

			actualRarity = Rarity::UNCOMMON;
			break;
		}
	}
	case Rarity::COMMON: {
		switch (randInt(1, 5)) {
		case 1: item = std::make_shared<LifeElixir>(x, y); break;
		case 2:	item = std::make_shared<MagicEssence>(x, y); break;
		case 3: item = std::make_shared<LuckUp>(x, y); break;
		case 4: item = std::make_shared<BetterRates>(x, y); break;
		case 5: item = std::make_shared<SummonNPCs>(x, y); break;
		//case 6: item = std::make_shared<SmokeBomb>(x, y); break;
		//case 7: item = std::make_shared<StatPotion>(x, y); break;
		//case 8: item = std::make_shared<RottenMeat>(x, y); break;
		//case 9: item = std::make_shared<Matches>(x, y); break;
		//case 10: item = std::make_shared<Firecrackers>(x, y); break;
		//case 11: item = std::make_shared<BearTrap>(x, y); break;
		default: break;
		}

		actualRarity = Rarity::COMMON;
		break;
	}
	default: break;
	}

	if (LootTable::allPassivesSeen()) {
		item.reset();
		return rollStatPassive(dungeon, x, y);
	}

	if (LootTable::passiveWasAcquired(item->getName())) {
		if (LootTable::allPassivesOfRaritySeen(actualRarity))
			rarity = increaseRarityLevel(actualRarity);
		
		else if (allowAll)
			rarity = Rarity::ALL;

		item.reset();
		item = rollPassive(dungeon, x, y, rarity, fallThrough);
	}

	LootTable::acquiredPassive(item->getName(), actualRarity);
	return item;
}
std::shared_ptr<Passive> rollStatPassive(Dungeon *dungeon, int x, int y) {
	std::shared_ptr<Passive> item(nullptr);

	switch (randInt(1, 4)) {
	case 1: item = std::make_shared<LifeElixir>(x, y); break;
	case 2:	item = std::make_shared<MagicEssence>(x, y); break;
	case 3: item = std::make_shared<LuckUp>(x, y); break;
	case 4: item = std::make_shared<BatWing>(x, y); break;
	//case 5: item = std::make_shared<SummonNPCs>(x, y); break;
	default: break;
	}

	return item;
}
std::shared_ptr<Relic> rollRelic(Dungeon *dungeon, Rarity rarity, bool fallThrough) {
	if (rarity == Rarity::ALL)
		rarity = rollRarity();

	std::shared_ptr<Relic> item(nullptr);

	switch (rarity) {
	case Rarity::LEGENDARY: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {

			//break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {

			//break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {

			//break;
		}
	}
	case Rarity::UNCOMMON: {
		if (!fallThrough || fallThrough && randInt(1, 100) > 50) {

			//break;
		}
	}
	case Rarity::COMMON: {
		switch (randInt(1, 7)) {
		case 1: item = std::make_shared<CursedStrength>(); break;
		case 2: item = std::make_shared<BrightStar>(); break;
		case 3: item = std::make_shared<DarkStar>(); break;
		case 4: item = std::make_shared<Riches>(); break;
		case 5: item = std::make_shared<SuperMagicEssence>(); break;
		case 6: item = std::make_shared<MatrixVision>(); break;
		case 7: item = std::make_shared<Protection>(); break;
		}
	}
	default: break;
	}

	if (LootTable::relicWasAcquired(item->getName())) {
		item.reset();
		item = rollRelic(dungeon, rarity, fallThrough);
	}

	return item;
}
std::shared_ptr<Chests> rollChest(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough) {
	if (rarity == Rarity::ALL)
		rarity = rollRarity();

	std::shared_ptr<Chests> item(nullptr);

	switch (rarity) {
	case Rarity::LEGENDARY: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 50) {

			//break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 80) {
			switch (randInt(1, 1)) {
			case 1: item = std::make_shared<GoldenChest>(dungeon, x, y); break;
			default: break;
			}
			break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 70) {
			switch (randInt(1, 3)) {
			case 1: item = std::make_shared<HauntedChest>(dungeon, x, y); break;
			case 2: item = std::make_shared<ExplodingChest>(dungeon, x, y); break;
			case 3: item = std::make_shared<TeleportingChest>(dungeon, x, y); break;
			default: break;
			}
			break;
		}
	}
	case Rarity::UNCOMMON: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 50) {
			switch (randInt(1, 2)) {
			case 1: item = std::make_shared<SilverChest>(dungeon, x, y); break;
			case 2: item = std::make_shared<LifeChest>(dungeon, x, y); break;
			default: break;
			}
			break;
		}
	}
	case Rarity::COMMON: {
		switch (randInt(1, 2)) {
		case 1: item = std::make_shared<TreasureChest>(dungeon, x, y); break;
		case 2:	item = std::make_shared<BrownChest>(dungeon, x, y); break;
			//case 9: item = std::make_shared<Matches>(x, y); break;
			//case 10: item = std::make_shared<Firecrackers>(x, y); break;
			//case 11: item = std::make_shared<BearTrap>(x, y); break;
		default: break;
		}
	}
	default: break;
	}

	return item;
}
std::shared_ptr<Weapon> rollWeapon(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough) {
	if (rarity == Rarity::ALL)
		rarity = rollRarity();

	std::shared_ptr<Weapon> item(nullptr);

	switch (rarity) {
	case Rarity::LEGENDARY: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 99.5) {

			break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 90) {
			switch (randInt(1, 8)) {
			case 1: item = std::make_shared<SuperiorNunchuks>(x, y); break;
			case 2:	item = std::make_shared<SuperiorJian>(x, y); break;
			case 3: item = std::make_shared<SuperiorHammer>(x, y); break;
			case 4: item = std::make_shared<SuperiorKatana>(x, y); break;
			case 5: item = std::make_shared<SuperiorEstoc>(x, y); break;
			case 6: item = std::make_shared<SuperiorBoStaff>(x, y); break;
			case 7: item = std::make_shared<SuperiorZweihander>(x, y); break;
			case 8: item = std::make_shared<Pistol>(x, y); break;
			//case 9: item = std::make_shared<Claw>(x, y); break;
			//case 10: item = std::make_shared<Mattock>(x, y); break;
			//case 11: item = std::make_shared<BearTrap>(x, y); break;
			default: break;
			}
			break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 70) {
			switch (randInt(1, 10)) {
			case 1: item = std::make_shared<GreaterNunchuks>(x, y); break;
			case 2:	item = std::make_shared<GreaterJian>(x, y); break;
			case 3: item = std::make_shared<GreaterHammer>(x, y); break;
			case 4: item = std::make_shared<GreaterKatana>(x, y); break;
			case 5: item = std::make_shared<GreaterEstoc>(x, y); break;
			case 6: item = std::make_shared<GreaterBoStaff>(x, y); break;
			case 7: item = std::make_shared<GreaterZweihander>(x, y); break;
			case 8: item = std::make_shared<WoodBow>(x, y); break;
			case 9: item = std::make_shared<Claw>(x, y); break;
			case 10: item = std::make_shared<Mattock>(x, y); break;
			//case 11: item = std::make_shared<BearTrap>(x, y); break;
			default: break;
			}
			break;
		}
	}
	case Rarity::UNCOMMON: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 50) {
			switch (randInt(1, 11)) {
			case 1: item = std::make_shared<CarbonFiberPike>(x, y); break;
			case 2:	item = std::make_shared<EnchantedPike>(x, y); break;
			case 3: item = std::make_shared<LuckyPike>(x, y); break;
			case 4: item = std::make_shared<RadiantPike>(x, y); break;
			case 5: item = std::make_shared<Katana>(x, y); break;
			case 6: item = std::make_shared<Hammer>(x, y); break;
			case 7: item = std::make_shared<Estoc>(x, y); break;
			case 8: item = std::make_shared<Jian>(x, y); break;
			case 9: item = std::make_shared<Boomerang>(x, y); break;
			case 10: item = std::make_shared<Zweihander>(x, y); break;
			case 11: item = std::make_shared<ArcaneStaff>(x, y); break;
			default: break;
			}
			break;
		}
	}
	case Rarity::COMMON: {
		switch (randInt(1, 8)) {
		case 1: item = std::make_shared<CarbonFiberShortSword>(x, y); break;
		case 2:	item = std::make_shared<EnchantedShortSword>(x, y); break;
		case 3: item = std::make_shared<LuckyShortSword>(x, y); break;
		case 4: item = std::make_shared<RadiantShortSword>(x, y); break;
		case 5: item = std::make_shared<Pike>(x, y); break;
		case 6: item = std::make_shared<RustyCutlass>(x, y); break;
		case 7: item = std::make_shared<BoStaff>(x, y); break;
		case 8: item = std::make_shared<Nunchuks>(x, y); break;
		//case 9: item = std::make_shared<Matches>(x, y); break;
		//case 10: item = std::make_shared<Firecrackers>(x, y); break;
		//case 11: item = std::make_shared<BearTrap>(x, y); break;
		default: break;
		}
	}
	default: break;
	}

	return item;
}
std::shared_ptr<Shield> rollShield(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough) {
	if (rarity == Rarity::ALL)
		rarity = rollRarity();

	std::shared_ptr<Shield> item(nullptr);

	switch (rarity) {
	case Rarity::LEGENDARY: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 50) {

			//break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 50) {

			//break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 50) {

			//break;
		}
	}
	case Rarity::UNCOMMON: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 50) {

			//break;
		}
	}
	case Rarity::COMMON: {
		switch (randInt(1, 5)) {
		case 1: item = std::make_shared<WoodShield>(x, y); break;
		case 2:	item = std::make_shared<IronShield>(x, y); break;
		case 3: item = std::make_shared<FrostShield>(x, y); break;
		case 4: item = std::make_shared<ThornedShield>(x, y); break;
		case 5: item = std::make_shared<ReflectShield>(x, y); break;
		//case 6: item = std::make_shared<SuperiorBoStaff>(x, y); break;
		//case 7: item = std::make_shared<SuperiorZweihander>(x, y); break;
		//case 8: item = std::make_shared<Pistol>(x, y); break;
		default: break;
		}
	}
	default: break;
	}

	return item;
}

Rarity rollRarity() {
	Rarity rarity;

	double roll = randReal(1, 100);

	if (roll >= 1 && roll <= 50)
		rarity = Rarity::COMMON;	
	else if (roll > 50 && roll <= 70)
		rarity = Rarity::UNCOMMON;	
	else if (roll > 70 && roll <= 85)
		rarity = Rarity::RARE;
	else if (roll > 85 && roll <= 99.5)
		rarity = Rarity::MYTHICAL;	
	else 
		rarity = Rarity::LEGENDARY;
	

	return rarity;
}
Rarity increaseRarityLevel(Rarity rarity) {
	switch (rarity) {
	case Rarity::MYTHICAL: return Rarity::LEGENDARY;
	case Rarity::RARE: return Rarity::MYTHICAL;
	case Rarity::UNCOMMON: return Rarity::RARE;
	case Rarity::COMMON: return Rarity::UNCOMMON;
	default: return Rarity::COMMON;
	}
}

void LootTable::initializeLootTable() {
	m_passivesObtained.insert(std::make_pair(BAT_WING, false));
	m_passivesObtained.insert(std::make_pair(LIFE_ELIXIR, false));
	m_passivesObtained.insert(std::make_pair(MAGIC_ESSENCE, false));
	m_passivesObtained.insert(std::make_pair(FLYING, false));
	m_passivesObtained.insert(std::make_pair(STEEL_PUNCH, false));
	m_passivesObtained.insert(std::make_pair(IRON_CLEATS, false));
	m_passivesObtained.insert(std::make_pair(LUCKY_PIG, false));
	m_passivesObtained.insert(std::make_pair(LIFE_STEAL, false));
	m_passivesObtained.insert(std::make_pair(BERSERK, false));
	m_passivesObtained.insert(std::make_pair(VULCAN_RUNE, false));
	m_passivesObtained.insert(std::make_pair(POISON_TOUCH, false));
	m_passivesObtained.insert(std::make_pair(FIRE_TOUCH, false));
	m_passivesObtained.insert(std::make_pair(FROST_TOUCH, false));
	m_passivesObtained.insert(std::make_pair(RAINBOW_TOUCH, false));
	m_passivesObtained.insert(std::make_pair(POISON_IMMUNE, false));
	m_passivesObtained.insert(std::make_pair(FIRE_IMMUNE, false));
	m_passivesObtained.insert(std::make_pair(BOMB_IMMUNE, false));
	m_passivesObtained.insert(std::make_pair(POTION_ALCHEMY, false));
	m_passivesObtained.insert(std::make_pair(SOUL_SPLIT, false));
	m_passivesObtained.insert(std::make_pair(HEAVY, false));
	m_passivesObtained.insert(std::make_pair(BRICK_BREAKER, false));
	m_passivesObtained.insert(std::make_pair(SUMMON_NPCS, false));
	m_passivesObtained.insert(std::make_pair(CHEAP_SHOPS, false));
	m_passivesObtained.insert(std::make_pair(BETTER_RATES, false));
	m_passivesObtained.insert(std::make_pair(TRAP_ILLUMINATION, false));
	m_passivesObtained.insert(std::make_pair(ITEM_ILLUMINATION, false));
	m_passivesObtained.insert(std::make_pair(MONSTER_ILLUMINATION, false));
	m_passivesObtained.insert(std::make_pair(RESONANT_SPELLS, false));
	m_passivesObtained.insert(std::make_pair(FAT_STACKS, false));
	m_passivesObtained.insert(std::make_pair(SCAVENGER, false));

	m_commonPassiveCount = 0;
	m_uncommonPassiveCount = 0;
	m_rarePassiveCount = 0;
	m_mythicalPassiveCount = 0;
	m_legendaryPassiveCount = 0;

	m_relicsObtained.insert(std::make_pair(CURSED_STRENGTH, false));
	m_relicsObtained.insert(std::make_pair(BRIGHT_STAR, false));
	m_relicsObtained.insert(std::make_pair(DARK_STAR, false));
	m_relicsObtained.insert(std::make_pair(RICHES, false));
	m_relicsObtained.insert(std::make_pair(SUPER_MAGIC_ESSENCE, false));
	m_relicsObtained.insert(std::make_pair(MATRIX_VISION, false));
	m_relicsObtained.insert(std::make_pair(PROTECTION, false));
}

bool LootTable::allPassivesSeen() {
	for (auto &it : m_passivesObtained)
		if (!it.second)
			return false;

	return true;
}
bool LootTable::allPassivesOfRaritySeen(Rarity rarity) {
	switch (rarity) {
	case Rarity::LEGENDARY: return m_legendaryPassiveCount == 0;
	case Rarity::MYTHICAL: return m_mythicalPassiveCount == 5;
	case Rarity::RARE: return m_rarePassiveCount == 8;
	case Rarity::UNCOMMON: return m_uncommonPassiveCount == 10;
	case Rarity::COMMON: return m_commonPassiveCount == 5;
	default: return true;
	}
}
void LootTable::updatePassiveRarityCount(Rarity rarity) {
	switch (rarity) {
	case Rarity::LEGENDARY: m_legendaryPassiveCount++; break;
	case Rarity::MYTHICAL: m_mythicalPassiveCount++; break;
	case Rarity::RARE: m_rarePassiveCount++; break;
	case Rarity::UNCOMMON: m_uncommonPassiveCount++; break;
	case Rarity::COMMON: m_commonPassiveCount++; break;
	}
}
bool LootTable::passiveWasAcquired(std::string passive) {
	if (!m_passivesObtained.find(passive)->second)
		return false;

	return true;
}
void LootTable::acquiredPassive(std::string passive, Rarity rarity) {
	auto it = m_passivesObtained.find(passive);
	if (it != m_passivesObtained.end() && !it->second) {
		it->second = true;
		LootTable::updatePassiveRarityCount(rarity);
	}
}

bool LootTable::relicWasAcquired(std::string relic) {
	size_t pos = relic.find(" +");
	if (pos != std::string::npos)
		relic.erase(pos);

	if (!m_relicsObtained.find(relic)->second)
		return false;

	return true;
}
void LootTable::acquiredRelic(std::string relic) {
	size_t pos = relic.find(" +");
	if (pos != std::string::npos)
		relic.erase(pos);

	auto it = m_relicsObtained.find(relic);
	if (it != m_relicsObtained.end())
		it->second = true;
}


std::shared_ptr<NPC> rollNPC(Dungeon *dungeon, int x, int y) {

	if (NPCTable::allNPCsSeen())
		return nullptr;

	std::shared_ptr<NPC> npc(nullptr);

	switch (randInt(0, 4)) {
	case 0: npc = std::make_shared<CreatureLover>(dungeon, x, y); break;
	case 1: npc = std::make_shared<Memorizer>(dungeon, x, y); break;
	case 2: npc = std::make_shared<Enchanter>(dungeon, x, y); break;
	case 3: npc = std::make_shared<Blacksmith>(dungeon, x, y); break;
	case 4: npc = std::make_shared<Trader>(dungeon, x, y); break;
	}

	if (NPCTable::NPCWasSeen(npc->getName())) {
		npc.reset();
		npc = rollNPC(dungeon, x, y);
	}

	NPCTable::encounteredNPC(npc->getName());
	return npc;
}

void NPCTable::initializeNPCTable() {
	m_NPCsEncountered.insert(std::make_pair(CREATURE_LOVER, false));
	m_NPCsEncountered.insert(std::make_pair(MEMORIZER, false));
	m_NPCsEncountered.insert(std::make_pair(BLACKSMITH, false));
	m_NPCsEncountered.insert(std::make_pair(ENCHANTER, false));
	m_NPCsEncountered.insert(std::make_pair(TRADER, false));
	//m_NPCsEncountered.insert(std::make_pair(TRADER, false));
}

bool NPCTable::allNPCsSeen() {
	for (auto &it : m_NPCsEncountered)
		if (!it.second)
			return false;

	return true;
}
bool NPCTable::NPCWasSeen(std::string name) {
	if (!m_NPCsEncountered.find(name)->second)
		return false;

	return true;
}
void NPCTable::encounteredNPC(std::string name) {
	auto it = m_NPCsEncountered.find(name);
	if (it != m_NPCsEncountered.end())
		it->second = true;
}

std::shared_ptr<Monster> createMonsterByName(std::string name, Dungeon *dungeon, int x, int y) {
	std::shared_ptr<Monster> monster(nullptr);

	// BREAKABLES
	if (name == WEAK_CRATE) monster = std::make_shared<WeakCrate>(dungeon, x, y);
	else if (name == WEAK_BARREL) monster = std::make_shared<WeakBarrel>(dungeon, x, y);
	else if (name == EXPLOSIVE_BARREL) monster = std::make_shared<ExplosiveBarrel>(dungeon, x, y);
	else if (name == WEAK_LARGE_POT) monster = std::make_shared<LargePot>(dungeon, x, y);


	// SPECIAL
	else if (name == GHOST) monster = std::make_shared<Ghost>(dungeon, x, y);
	else if (name == FLARE) monster = std::make_shared<Flare>(dungeon, x, y);
	else if (name == WATCHER) monster = std::make_shared<Watcher>(dungeon, x, y);
	else if (name == FORGOTTEN_SPIRIT) monster = std::make_shared<ForgottenSpirit>(dungeon, x, y);


	// FIRST_FLOOR
	else if (name == SEEKER) monster = std::make_shared<Seeker>(dungeon, x, y);
	else if (name == GOO_SACK) monster = std::make_shared<GooSack>(dungeon, x, y);
	else if (name == ROUNDABOUT) monster = std::make_shared<Roundabout>(dungeon, x, y);
	else if (name == BROUNDABOUT) monster = std::make_shared<Broundabout>(dungeon, x, y);
	else if (name == RAT) monster = std::make_shared<Rat>(dungeon, x, y);
	else if (name == SPIDER) monster = std::make_shared<Spider>(dungeon, x, y);
	else if (name == SHOOTING_SPIDER) monster = std::make_shared<ShootingSpider>(dungeon, x, y);
	else if (name == POUNCING_SPIDER) monster = std::make_shared<PouncingSpider>(dungeon, x, y);
	else if (name == GOBLIN) monster = std::make_shared<Goblin>(dungeon, x, y);
	else if (name == WANDERER) monster = std::make_shared<Wanderer>(dungeon, x, y);
	else if (name == SLEEPING_WANDERER) monster = std::make_shared<SleepingWanderer>(dungeon, x, y);
	else if (name == PROVOCABLE_WANDERER) monster = std::make_shared<ProvocableWanderer>(dungeon, x, y);
	
	
	// SECOND_FLOOR:
	else if (name == RABID_WANDERER) monster = std::make_shared<RabidWanderer>(dungeon, x, y);
	else if (name == TOAD) monster = std::make_shared<Toad>(dungeon, x, y);
	else if (name == POISON_BUBBLE) monster = std::make_shared<PoisonBubble>(dungeon, x, y);
	else if (name == PIRANHA) monster = std::make_shared<Piranha>(dungeon, x, y);
	else if (name == WATER_SPIRIT) monster = std::make_shared<WaterSpirit>(dungeon, x, y);
	else if (name == ANGLED_BOUNCER) monster = std::make_shared<AngledBouncer>(dungeon, x, y);
	else if (name == FIREFLY) monster = std::make_shared<Firefly>(dungeon, x, y);
	else if (name == DEAD_SEEKER) monster = std::make_shared<DeadSeeker>(dungeon, x, y);
	else if (name == TRI_HORN) monster = std::make_shared<TriHorn>(dungeon, x, y);
	else if (name == TUMBLE_SHROOM) monster = std::make_shared<TumbleShroom>(dungeon, x, y);
	else if (name == WRIGGLER) monster = Wriggler::create(dungeon, Coords(x, y));
	else if (name == BARBED_CATERPILLAR) monster = std::make_shared<BarbedCaterpillar>(dungeon, x, y);
	

	// THIRD_FLOOR:
	else if (name == CRYSTAL_WARRIOR) monster = std::make_shared<CrystalWarrior>(dungeon, x, y);
	else if (name == CRYSTAL_TURTLE) monster = std::make_shared<CrystalTurtle>(dungeon, x, y);
	else if (name == CRYSTAL_HEDGEHOG) monster = std::make_shared<CrystalHedgehog>(dungeon, x, y);
	else if (name == CRYSTAL_SHOOTER) monster = std::make_shared<CrystalShooter>(dungeon, x, y);
	else if (name == CRYSTAL_BEETLE) monster = std::make_shared<CrystalBeetle>(dungeon, x, y);
	else if (name == RABBIT) monster = std::make_shared<Rabbit>(dungeon, x, y);
	else if (name == BOMBEE) monster = std::make_shared<Bombee>(dungeon, x, y);
	else if (name == TICK) monster = std::make_shared<Tick>(dungeon, x, y);
	else if (name == BENEVOLENT_BARK) monster = std::make_shared<BenevolentBark>(dungeon, x, y);
	else if (name == EXOTIC_FELINE) monster = std::make_shared<ExoticFeline>(dungeon, x, y);
	//else if (name == ) monster = std::make_shared<>(dungeon, x, y);
	//else if (name == ) monster = std::make_shared<>(dungeon, x, y);
	

	// FOURTH_FLOOR:
	else if (name == FLAME_WANDERER) monster = std::make_shared<FlameWanderer>(dungeon, x, y);
	else if (name == ZAPPER) monster = std::make_shared<Zapper>(dungeon, x, y);
	else if (name == SPINNER) monster = std::make_shared<Spinner>(dungeon, x, y);
	else if (name == CHARRED_BOMBEE) monster = std::make_shared<CharredBombee>(dungeon, x, y);
	else if (name == FIRE_ROUNDABOUT) monster = std::make_shared<FireRoundabout>(dungeon, x, y);
	else if (name == ITEM_THIEF) monster = std::make_shared<ItemThief>(dungeon, x, y);
	else if (name == CHARGER) monster = std::make_shared<Charger>(dungeon, x, y);
	else if (name == SERPENT) monster = std::make_shared<Serpent>(dungeon, x, y);
	else if (name == ARMORED_SERPENT) monster = std::make_shared<ArmoredSerpent>(dungeon, x, y);
	else if (name == COMBUSTION_GOLEM) monster = std::make_shared<CombustionGolem>(dungeon, x, y);
	else if (name == OLD_SMOKEY) monster = std::make_shared<OldSmokey>(dungeon, x, y);
	//else if (name == ) monster = std::make_shared<>(dungeon, x, y);
	
	
	// FIFTH_FLOOR:
	else if (name == PUFF) monster = std::make_shared<Puff>(dungeon, x, y);
	else if (name == GUSTY_PUFF) monster = std::make_shared<GustyPuff>(dungeon, x, y);
	else if (name == STRONG_GUSTY_PUFF) monster = std::make_shared<StrongGustyPuff>(dungeon, x, y);
	else if (name == INVERTED_PUFF) monster = std::make_shared<InvertedPuff>(dungeon, x, y);
	else if (name == SPIKED_INVERTED_PUFF) monster = std::make_shared<SpikedInvertedPuff>(dungeon, x, y);
	else if (name == GUSTY_SPIKED_INVERTED_PUFF) monster = std::make_shared<GustySpikedInvertedPuff>(dungeon, x, y);
	else if (name == ARMORED_BEETLE) monster = std::make_shared<ArmoredBeetle>(dungeon, x, y);
	else if (name == SPECTRAL_SWORD) monster = std::make_shared<SpectralSword>(dungeon, x, y);
	else if (name == SAND_CENTIPEDE) monster = std::make_shared<SandCentipede>(dungeon, x, y);
	else if (name == ARCHER) monster = std::make_shared<Archer>(dungeon, x, y);
	else if (name == SAND_BEAVER) monster = std::make_shared<SandBeaver>(dungeon, x, y);
	else if (name == SAND_ALBATROSS) monster = std::make_shared<SandAlbatross>(dungeon, x, y);

	
	// SIXTH_FLOOR:	
	else if (name == WISP) monster = std::make_shared<Wisp>(dungeon, x, y);
	else if (name == LIGHTNING_WISP) monster = std::make_shared<LightningWisp>(dungeon, x, y);
	else if (name == GRABBER) monster = std::make_shared<Grabber>(dungeon, x, y);
	else if (name == EVIL_SHADOW) monster = std::make_shared<EvilShadow>(dungeon, x, y);
	else if (name == CRAWLING_SPINE) monster = std::make_shared<CrawlingSpine>(dungeon, x, y);
	else if (name == CONSTRUCTOR_DEMON) monster = std::make_shared<ConstructorDemon>(dungeon, x, y);
	else if (name == HOWLER) monster = std::make_shared<Howler>(dungeon, x, y);
	else if (name == FACELESS_HORROR) monster = std::make_shared<FacelessHorror>(dungeon, x, y);
	else if (name == TOWERING_BRUTE) monster = std::make_shared<ToweringBrute>(dungeon, x, y);
	else if (name == SKY_CRASHER) monster = std::make_shared<SkyCrasher>(dungeon, x, y);
	else if (name == STEAM_BOT) monster = std::make_shared<SteamBot>(dungeon, x, y);
	//else if (name == ) monster = std::make_shared<>(dungeon, x, y);
	
	
	// SEVENTH_FLOOR:
	else if (name == PIKEMAN) monster = std::make_shared<Pikeman>(dungeon, x, y);
	else if (name == SHRINEKEEPER) monster = std::make_shared<Shrinekeeper>(dungeon, x, y);
	else if (name == SWAPPER) monster = std::make_shared<Swapper>(dungeon, x, y);
	else if (name == SHIELD_MASTER) monster = std::make_shared<ShieldMaster>(dungeon, x, y);
	else if (name == PSEUDO_DOPPEL) monster = std::make_shared<PseudoDoppel>(dungeon, x, y);
	else if (name == ELECTROMAGNETIZER) monster = std::make_shared<Electromagnetizer>(dungeon, x, y);
	else if (name == BLADE_PSYCHIC) monster = std::make_shared<BladePsychic>(dungeon, x, y);
	else if (name == DASH_MASTER) monster = std::make_shared<DashMaster>(dungeon, x, y);
	else if (name == ACIDIC_BEAST) monster = std::make_shared<AcidicBeast>(dungeon, x, y);
	else if (name == DARK_CANINE) monster = std::make_shared<DarkCanine>(dungeon, x, y);
	//else if (name == ) monster = std::make_shared<>(dungeon, x, y);
	//else if (name == ) monster = std::make_shared<>(dungeon, x, y);

	
	// EIGHTH_FLOOR:	
	else if (name == ABYSS_SUMMONER) monster = std::make_shared<AbyssSummoner>(dungeon, x, y);
	else if (name == MAGICAL_BERSERKER) monster = std::make_shared<MagicalBerserker>(dungeon, x, y);
	else if (name == DISABLER) monster = std::make_shared<Disabler>(dungeon, x, y);
	else if (name == INCENDIARY_INFUSER) monster = std::make_shared<IncendiaryInfuser>(dungeon, x, y);
	else if (name == LIGHTNING_STRIKER) monster = std::make_shared<LightningStriker>(dungeon, x, y);
	else if (name == FLAME_ARCHAIC) monster = std::make_shared<FlameArchaic>(dungeon, x, y);
	else if (name == ADVANCED_ROCK_SUMMONER) monster = std::make_shared<AdvancedRockSummoner>(dungeon, x, y);
	else if (name == MASTER_CONJURER) monster = std::make_shared<MasterConjurer>(dungeon, x, y);
	else if (name == ASCENDED_SHOT) monster = std::make_shared<AscendedShot>(dungeon, x, y);
	else if (name == ROYAL_SWORDSMAN) monster = std::make_shared<RoyalSwordsman>(dungeon, x, y);
	else if (name == LIGHT_ENTITY) monster = std::make_shared<LightEntity>(dungeon, x, y);
	//else if (name == DARK_ENTITY) monster = std::make_shared<DarkEntity>(dungeon, x, y);

	return monster;
}
std::shared_ptr<Monster> rollMonster(int level, Dungeon *dungeon, int x, int y) {
	std::shared_ptr<Monster> monster(nullptr);

	switch (level) {
	case FIRST_FLOOR: {
		switch (randInt(1, 12)) {
		case 1:	monster = std::make_shared<Seeker>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<GooSack>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<Roundabout>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<Broundabout>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<Rat>(dungeon, x, y);	break;
		case 6:	monster = std::make_shared<Spider>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<ShootingSpider>(dungeon, x, y); break;
		case 8:	monster = std::make_shared<PouncingSpider>(dungeon, x, y); break;
		case 9:	monster = std::make_shared<Goblin>(dungeon, x, y); break;
		case 10: monster = std::make_shared<Wanderer>(dungeon, x, y); break;
		case 11: monster = std::make_shared<SleepingWanderer>(dungeon, x, y); break;
		case 12: monster = std::make_shared<ProvocableWanderer>(dungeon, x, y);	break;
		}

		break;
	}
	case SECOND_FLOOR: {
		switch (randInt(1, 12)) {
		case 1:	monster = std::make_shared<RabidWanderer>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<Toad>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<PoisonBubble>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<Piranha>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<WaterSpirit>(dungeon, x, y);	break;
		case 6:	monster = std::make_shared<AngledBouncer>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<Firefly>(dungeon, x, y); break;
		case 8:	monster = std::make_shared<DeadSeeker>(dungeon, x, y); break;
		case 9:	monster = std::make_shared<TriHorn>(dungeon, x, y); break;
		case 10: monster = std::make_shared<TumbleShroom>(dungeon, x, y); break;
		case 11: monster = Wriggler::create(dungeon, Coords(x, y));; break;
		case 12: monster = std::make_shared<BarbedCaterpillar>(dungeon, x, y); break;
		}

		break;
	}
	case THIRD_FLOOR: {
		switch (randInt(1, 10)) {
		case 1:	monster = std::make_shared<CrystalWarrior>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<CrystalTurtle>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<CrystalHedgehog>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<CrystalShooter>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<CrystalBeetle>(dungeon, x, y); break;
		case 6:	monster = std::make_shared<Rabbit>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<Bombee>(dungeon, x, y); break;
		case 8:	monster = std::make_shared<Tick>(dungeon, x, y); break;
		case 9:	monster = std::make_shared<BenevolentBark>(dungeon, x, y); break;
		case 10: monster = std::make_shared<ExoticFeline>(dungeon, x, y); break;
		//case 11: monster = std::make_shared<>(dungeon, x, y); break;
		//case 12: monster = std::make_shared<>(dungeon, x, y);	break;
		}

		break;
	}
	case FOURTH_FLOOR: {
		switch (randInt(1, 10)) {
		case 1:	monster = std::make_shared<FlameWanderer>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<Zapper>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<Spinner>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<CharredBombee>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<FireRoundabout>(dungeon, x, y); break;
		case 6:	monster = std::make_shared<ItemThief>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<Charger>(dungeon, x, y); break;
		case 8:	monster = std::make_shared<Serpent>(dungeon, x, y); break;
		case 9: monster = std::make_shared<ArmoredSerpent>(dungeon, x, y); break;
		case 10: monster = std::make_shared<CombustionGolem>(dungeon, x, y); break;
		//case 11: monster = std::make_shared<OldSmokey>(dungeon, x, y); break;
		//case 12: monster = std::make_shared<>(dungeon, x, y);	break;
		}

		break;
	}
	case FIFTH_FLOOR: {
		switch (randInt(1, 12)) {
		case 1:	monster = std::make_shared<Puff>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<GustyPuff>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<StrongGustyPuff>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<InvertedPuff>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<SpikedInvertedPuff>(dungeon, x, y); break;
		case 6:	monster = std::make_shared<GustySpikedInvertedPuff>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<ArmoredBeetle>(dungeon, x, y); break;
		case 8:	monster = std::make_shared<SpectralSword>(dungeon, x, y); break;
		case 9: monster = std::make_shared<SpectralSword>(dungeon, x, y); break; // Removed SandCentipede temporarily
		case 10: monster = std::make_shared<Archer>(dungeon, x, y); break;
		case 11: monster = std::make_shared<SandBeaver>(dungeon, x, y); break;
		case 12: monster = std::make_shared<SandAlbatross>(dungeon, x, y);	break;
		}

		break;
	}
	case SIXTH_FLOOR: {
		switch (randInt(1, 11)) {
		case 1:	monster = std::make_shared<Wisp>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<LightningWisp>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<Grabber>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<EvilShadow>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<CrawlingSpine>(dungeon, x, y); break;
		case 6:	monster = std::make_shared<ConstructorDemon>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<Howler>(dungeon, x, y); break;
		case 8: monster = std::make_shared<FacelessHorror>(dungeon, x, y); break;
		case 9: monster = std::make_shared<ToweringBrute>(dungeon, x, y); break;
		case 10: monster = std::make_shared<SkyCrasher>(dungeon, x, y); break;
		case 11: monster = std::make_shared<SteamBot>(dungeon, x, y); break;
		//case 12: monster = std::make_shared<>(dungeon, x, y);	break;
		}

		break;
	}
	case SEVENTH_FLOOR: {
		switch (randInt(1, 10)) {
		case 1:	monster = std::make_shared<Pikeman>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<Shrinekeeper>(dungeon, x, y); break;
		case 3: monster = std::make_shared<Swapper>(dungeon, x, y); break;
		case 4: monster = std::make_shared<ShieldMaster>(dungeon, x, y); break;
		case 5: monster = std::make_shared<PseudoDoppel>(dungeon, x, y); break;
		case 6: monster = std::make_shared<Electromagnetizer>(dungeon, x, y); break;
		case 7: monster = std::make_shared<BladePsychic>(dungeon, x, y); break;
		case 8: monster = std::make_shared<DashMaster>(dungeon, x, y); break;
		case 9: monster = std::make_shared<AcidicBeast>(dungeon, x, y); break;
		case 10: monster = std::make_shared<DarkCanine>(dungeon, x, y); break;
		//case 11: monster = std::make_shared<>(dungeon, x, y); break;
		//case 12: monster = std::make_shared<>(dungeon, x, y);	break;
		}

		break;
	}
	case EIGHTH_FLOOR: {
		switch (randInt(1, 11)) {
		case 1: monster = std::make_shared<AbyssSummoner>(dungeon, x, y); break;
		case 2: monster = std::make_shared<MagicalBerserker>(dungeon, x, y); break;
		case 3: monster = std::make_shared<Disabler>(dungeon, x, y); break;
		case 4: monster = std::make_shared<IncendiaryInfuser>(dungeon, x, y); break;
		case 5: monster = std::make_shared<LightningStriker>(dungeon, x, y); break;
		case 6: monster = std::make_shared<FlameArchaic>(dungeon, x, y); break;
		case 7: monster = std::make_shared<AdvancedRockSummoner>(dungeon, x, y); break;
		case 8: monster = std::make_shared<MasterConjurer>(dungeon, x, y); break;
		case 9: monster = std::make_shared<AscendedShot>(dungeon, x, y); break;
		case 10: monster = std::make_shared<RoyalSwordsman>(dungeon, x, y); break;
		case 11: monster = std::make_shared<LightEntity>(dungeon, x, y); break;
		//case 12: monster = std::make_shared<>(dungeon, x, y);	break;
		}

		break;
	}
	case NINTH_FLOOR: {
		switch (randInt(1, 10)) {
		case 1:
		//case 1: monster = std::make_shared<>(dungeon, x, y); break;
		//case 2: monster = std::make_shared<>(dungeon, x, y); break;
		//case 3: monster = std::make_shared<>(dungeon, x, y); break;
		//case 4: monster = std::make_shared<>(dungeon, x, y); break;
		//case 5: monster = std::make_shared<>(dungeon, x, y); break;
		//case 6: monster = std::make_shared<>(dungeon, x, y); break;
		//case 7: monster = std::make_shared<>(dungeon, x, y); break;
		//case 8: monster = std::make_shared<>(dungeon, x, y); break;
		//case 9: monster = std::make_shared<>(dungeon, x, y); break;
		//case 10: monster = std::make_shared<>(dungeon, x, y); break;
		//case 11: monster = std::make_shared<>(dungeon, x, y); break;
		//case 12: monster = std::make_shared<>(dungeon, x, y);	break;
		default: break;
		}

		break;
	}
	}

	return monster;
}

void GameTimers::addGameTimer(const std::string &name, void *target) {
	m_gameTimers.insert(std::make_pair(name, target));
}
void GameTimers::removeGameTimer(const std::string &name) {
	auto it = m_gameTimers.find(name);
	if (it != m_gameTimers.end())
		m_gameTimers.erase(it);
}
void GameTimers::resumeAllGameTimers() {
	for (auto &it : m_gameTimers)
		cocos2d::Director::getInstance()->getScheduler()->resumeTarget(it.second);
}
void GameTimers::pauseAllGameTimers() {
	for (auto &it : m_gameTimers)
		cocos2d::Director::getInstance()->getScheduler()->pauseTarget(it.second);	
}
void GameTimers::removeAllGameTimers() {
	for (auto &it : m_gameTimers)
		cocos2d::Director::getInstance()->getScheduler()->unscheduleUpdate(it.second);
	
	m_gameTimers.clear();
}

int calculateDistanceBetween(const Coords &start, const Coords &end) {
	return (abs(start.x - end.x) + abs(start.y - end.y));
}
void getCoordsAdjacentTo(const Dungeon &dungeon, std::vector<Coords> &coords, const Coords &start, bool all) {
	coords.clear();

	int x = start.x;
	int y = start.y;

	if (!(dungeon.wall(x - 1, y)))
		coords.push_back(Coords(x - 1, y));

	if (!(dungeon.wall(x + 1, y)))
		coords.push_back(Coords(x + 1, y));

	if (!(dungeon.wall(x, y - 1)))
		coords.push_back(Coords(x, y - 1));

	if (!(dungeon.wall(x, y + 1)))
		coords.push_back(Coords(x, y + 1));

	if (all) {
		if (!(dungeon.wall(x - 1, y - 1)))
			coords.push_back(Coords(x - 1, y - 1));

		if (!(dungeon.wall(x - 1, y + 1)))
			coords.push_back(Coords(x - 1, y + 1));

		if (!(dungeon.wall(x + 1, y - 1)))
			coords.push_back(Coords(x + 1, y - 1));

		if (!(dungeon.wall(x + 1, y + 1)))
			coords.push_back(Coords(x + 1, y + 1));
	}
}
void getDiagonalCoordsAdjacentTo(const Dungeon &dungeon, std::vector<Coords> &coords, const Coords &start) {
	coords.clear();

	int x = start.x;
	int y = start.y;

	if (!(dungeon.wall(x - 1, y - 1)))
		coords.push_back(Coords(x - 1, y - 1));

	if (!(dungeon.wall(x - 1, y + 1)))
		coords.push_back(Coords(x - 1, y + 1));

	if (!(dungeon.wall(x + 1, y - 1)))
		coords.push_back(Coords(x + 1, y - 1));

	if (!(dungeon.wall(x + 1, y + 1)))
		coords.push_back(Coords(x + 1, y + 1));
}

cocos2d::Vec2 getHalfTileYOffset() {
	return cocos2d::Vec2(0, SPACING_FACTOR / 2);
}
cocos2d::Vector<cocos2d::SpriteFrame*> getAnimationFrameVector(const char* format, int count) {
	auto spritecache = cocos2d::SpriteFrameCache::getInstance();
	cocos2d::Vector<cocos2d::SpriteFrame*> animFrames;
	char str[100];
	for (int i = 1; i <= count; i++) {
		sprintf(str, format, i);
		animFrames.pushBack(spritecache->getSpriteFrameByName(str));
	}
	return animFrames;
}
cocos2d::Vector<cocos2d::SpriteFrame*> getAnimationFrameVector(std::string format, int count) {
	return getAnimationFrameVector(format.c_str(), count);
}
void setDirectionalOffsets(char move, int &n, int &m, int offset, bool reversed) {
	// Key:
	// 1 u 2
	// l _ r
	// 3 d 4

	switch (move) {
	case 'l': n = -offset; m = 0; break;
	case 'r': n = offset; m = 0; break;
	case 'u': n = 0; m = -offset; break;
	case 'd': n = 0; m = offset; break;
	case '1': n = -offset; m = -offset; break;
	case '2': n = offset; m = -offset; break;
	case '3': n = -offset; m = offset; break;
	case '4': n = offset; m = offset; break;
	default: n = 0; m = 0; break;
	}

	if (reversed) {
		switch (move) {
		case 'l': n = offset; m = 0; break;
		case 'r': n = -offset; m = 0; break;
		case 'u': n = 0; m = offset; break;
		case 'd': n = 0; m = -offset; break;
		case '1': n = offset; m = offset; break;
		case '2': n = -offset; m = offset; break;
		case '3': n = offset; m = -offset; break;
		case '4': n = -offset; m = -offset; break;
		default: n = 0; m = 0; break;
		}
	}
}
void incrementDirectionalOffsets(char move, int &n, int &m) {
	switch (move) {
	case 'l': n--; m = 0; break;
	case 'r': n++; m = 0; break;
	case 'u': n = 0; m--; break;
	case 'd': n = 0; m++; break;
	case '1': n--; m--; break;
	case '2': n++; m--; break;
	case '3': n--; m++; break;
	case '4': n++; m++; break;
	}
}
void decrementDirectionalOffsets(char move, int &n, int &m) {
	switch (move) {
	case 'l': n++; m = 0; break;
	case 'r': n--; m = 0; break;
	case 'u': n = 0; m++; break;
	case 'd': n = 0; m--; break;
	case '1': n++; m++; break;
	case '2': n--; m++; break;
	case '3': n++; m--; break;
	case '4': n--; m--; break;
	}
}
bool directionIsOppositeTo(const char target, const char other) {
	switch (target) {
	case 'l': return other == 'r';
	case 'r': return other == 'l';
	case 'u': return other == 'd';
	case 'd': return other == 'u';
	case '1': return other == '4';
	case '2': return other == '3';
	case '3': return other == '2';
	case '4': return other == '1';
	}

	return false;
}
char getDirectionOppositeTo(const char other) {
	char opp = '-';
	switch (other) {
	case 'l': opp = 'r'; break;
	case 'r': opp = 'l'; break;
	case 'u': opp = 'd'; break;
	case 'd': opp = 'u'; break;
	case '1': opp = '4'; break;
	case '2': opp = '3'; break;
	case '3': opp = '2'; break;
	case '4': opp = '1'; break;
	}
	return opp;
}
char getFacingDirectionRelativeTo(const Coords &start, const Coords &end) {
	int sx = start.x, sy = start.y;
	int ex = end.x, ey = end.y;

	if (sx == ex) {
		if (sy < ey)
			return 'd';
		else
			return 'u';
	}
	else if (sy == ey) {
		if (sx < ex)
			return 'r';
		else
			return 'l';
	}
	else {
		if (abs(sx - ex) == abs(sy - ey)) {
			if (sx < ex && sy < ey)
				return '4';
			else if (sx > ex && sy < ey)
				return '3';
			else if (sx < ex && sy > ey)
				return '2';
			else
				return '1';
		}
		else {
			if (abs(sx - ex) < abs(sy - ey)) {
				if (sy < ey)
					return 'd';
				else
					return 'u';
			}
			else {
				if (sx < ex)
					return 'r';
				else
					return 'l';
			}
		}
	}
}
char getCardinalFacingDirectionRelativeTo(const Coords &start, const Coords &end) {
	int sx = start.x, sy = start.y;
	int ex = end.x, ey = end.y;

	if (abs(sx - ex) < abs(sy - ey)) {
		if (sy < ey)
			return 'd';
		else
			return 'u';
	}
	else if (abs(sx - ex) > abs(sy - ey)) {
		if (sx < ex)
			return 'r';
		else
			return 'l';
	}
	else {

		if (sx < ex && sy < ey)
			return randInt(0, 1) == 0 ? 'r' : 'd';
		else if (sx > ex && sy < ey)
			return randInt(0, 1) == 0 ? 'l' : 'd';
		else if (sx < ex && sy > ey)
			return randInt(0, 1) == 0 ? 'r' : 'u';
		else
			return randInt(0, 1) == 0 ? 'l' : 'u';
	}
}
bool isMovementAction(char move) {
	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd': return true;
	}

	return false;
}

bool hasLineOfSight(const Dungeon &dungeon, char dir, const Coords &start, const Coords &end) {
	int n, m;
	setDirectionalOffsets(dir, n, m);

	while (!dungeon.tileIsBlocked(start.x + n, start.y + m))
		incrementDirectionalOffsets(dir, n, m);

	Coords finalPos = start + Coords(n, m);
	if (finalPos == end)
		return true;

	return false;
}

bool hasLineOfSight(const Dungeon &dungeon, const Coords &start, const Coords &end) {
	if (start.x == end.x) {
		return hasLineOfSight(dungeon, start.y < end.y ? 'd' : 'u', start, end);
	}
	else if (start.y == end.y) {
		return hasLineOfSight(dungeon, start.x < end.x ? 'r' : 'l', start, end);
	}

	return false;
}

bool positionInRange(const Coords &start, const Coords &end, int range) {
	return calculateDistanceBetween(start, end) <= range;
}

bool positionInLinearRange(const Coords &start, const Coords &end, int range) {
	return positionInBufferedLinearRange(start, end, range, 0);
}
bool positionInBufferedLinearRange(const Coords &start, const Coords &end, int range, int buffer) {
	return ((abs(start.x - end.x) <= buffer && abs(start.y - end.y) <= range) || (abs(start.y - end.y) <= buffer && abs(start.x - end.x) <= range));
}

bool positionIsAdjacent(const Coords &start, const Coords &end, bool diagonals) {
	if (diagonals)
		return abs(start.x - end.x) <= 1 && abs(start.y - end.y) <= 1;

	return abs(start.x - end.x) + abs(start.y - end.y) == 1;
}

bool positionInRectangularRange(const Coords &start, const Coords &end, int length, int width) {
	if (length == width)
		return abs(start.x - end.x) <= length && abs(start.y - end.y) <= length;

	return (abs(start.x - end.x) <= length && abs(start.y - end.y) <= width) || (abs(start.x - end.x) <= width && abs(start.y - end.y) <= length);
}


void TextUtils::initJsonText() {
	std::ifstream ifs(GAME_TEXT_FILE);
	std::string content((std::istreambuf_iterator<char>(ifs)),
		std::istreambuf_iterator<char>());

	jsonTree.Parse(content.c_str());
}

std::string fetchMenuText(const std::string &menuType, const std::string &id) {
	auto menuObj = TextUtils::jsonTree.FindMember("Menus");
	if (menuObj != TextUtils::jsonTree.MemberEnd()) {
		auto obj = menuObj->value.GetObjectW();
		auto it = obj.FindMember(menuType.c_str());
		if (it != obj.MemberEnd()) {
			auto a = it->value.GetArray();
			for (auto it = a.Begin(); it != a.End(); it++) {
				auto menuItem = it->GetObjectW();
				auto menuText = menuItem.FindMember(id.c_str());
				if (menuText != menuItem.MemberEnd())
					return menuText->value.GetString();
			}
		}
	}

	return "MENU TEXT NOT FOUND";
}
std::string fetchPromptText(const std::string &promptType, const std::string &id) {
	auto menuObj = TextUtils::jsonTree.FindMember("Prompts");
	if (menuObj != TextUtils::jsonTree.MemberEnd()) {
		auto obj = menuObj->value.GetObjectW();
		auto it = obj.FindMember(promptType.c_str());
		if (it != obj.MemberEnd()) {
			auto a = it->value.GetArray();
			for (auto it = a.Begin(); it != a.End(); it++) {
				auto menuItem = it->GetObjectW();
				auto menuText = menuItem.FindMember(id.c_str());
				if (menuText != menuItem.MemberEnd())
					return menuText->value.GetString();
			}
		}
	}

	return "PROMPT TEXT NOT FOUND";
}
void fetchNPCDialogue(const std::string &name, const std::string &id, std::vector<std::string> &dialogue) {
	auto it = TextUtils::jsonTree.FindMember("NPC");
	if (it != TextUtils::jsonTree.MemberEnd()) {
		auto npcObj = it->value.GetObjectW();
		auto npcIt = npcObj.FindMember(name.c_str());
		if (npcIt != npcObj.MemberEnd()) {
			auto npcTextObj = npcIt->value.GetObjectW();
			auto npcTextIt = npcTextObj.FindMember(id.c_str());
			if (npcTextIt != npcTextObj.MemberEnd()) {
				auto a = npcTextIt->value.GetArray();
				for (auto it = a.Begin(); it != a.End(); it++) {
					dialogue.push_back(it->GetString());
				}
			}
		}
	}
}
void fetchItemInfo(const std::string &type, const std::string &id, std::string &name, std::string &desc) {
	auto it = TextUtils::jsonTree.FindMember("Drops");
	if (it != TextUtils::jsonTree.MemberEnd()) {
		auto itemObj = it->value.GetObjectW();
		auto itemIt = itemObj.FindMember(type.c_str());
		if (itemIt != itemObj.MemberEnd()) {
			auto itemTextObj = itemIt->value.GetObjectW();
			auto itemTextIt = itemTextObj.FindMember(id.c_str());
			if (itemTextIt != itemTextObj.MemberEnd()) {
				auto a = itemTextIt->value.GetArray();
				name = a[0].GetObjectW().FindMember("name")->value.GetString();
				desc = a[1].GetObjectW().FindMember("desc")->value.GetString();
				return;
			}
		}
	}

	name = "ITEM NOT FOUND";
	desc = id;
}
std::string fetchItemName(const std::string &type, const std::string &id) {
	std::string name, dummy;
	fetchItemInfo(type, id, name, dummy);
	return name;
}

void getNPCDialogue(const std::string &filename, const std::string &id, std::vector<std::string> &dialogue) {
	std::fstream in;
	in.open(filename);

	if (!in)
		return;

	std::string line;
	while (getline(in, line)) {
		if (line == id) {
			while (getline(in, line), line != "#")
				dialogue.push_back(line);

			break;
		}
	}

	in.close();
}
void getItemInfo(const std::string &filename, const std::string &id, std::string &name, std::string &desc) {
	name = "ITEM NAME NOT FOUND";

	std::fstream in;
	in.open(filename);

	if (!in)
		return;

	/** Information is organized in the file as the following:
	*   English_Item_Name
	*   Localized_Item_Name
	*   Localized_Item_Desc */

	std::string line;
	while (getline(in, line)) {
		if (line == id) {
			getline(in, name);
			getline(in, line);
			getline(in, desc);

			break;
		}
	}

	in.close();
}
std::string getItemName(const std::string &filename, const std::string &id) {
	std::string name, dummy;
	getItemInfo(filename, id, name, dummy);
	return name;
}

void formatItemDescriptionForDisplay(std::string &desc) {
	int maxLineLength = 55; // Max number of chars allow per line. May need to be dynamically adjusted.

	if (static_cast<int>(desc.size()) > maxLineLength) {
		int lineBreaks = desc.size() / maxLineLength;
		size_t pos = maxLineLength - 1;
		while (lineBreaks > 0) {
			while (desc[pos] != ' ')
				pos--;

			if (lineBreaks == 1 && static_cast<int>(desc.substr(pos).size()) > maxLineLength)
				lineBreaks++;

			desc.replace(pos, 1, "\n");

			pos += maxLineLength + 1;
			lineBreaks--;
		}
	}
}
void replaceTextVariableWith(std::vector<std::string> &text, const std::string &varText, const std::string &actualText) {
	int size = text.size();
	for (int i = 0; i < size; i++) {
		if (text[i].find(varText) == std::string::npos)
			continue;

		size_t startPos = text[i].find("[");
		while (startPos != std::string::npos) {
			size_t endPos = text[i].find("]");
			if (endPos == std::string::npos)
				break;

			text[i].replace(startPos, endPos - startPos + 1, actualText);

			startPos = text[i].find("[");
		}
	}
}


void SaveManager::init() {

}
void SaveManager::createNewSaveFile() {

}
void SaveManager::loadSaveFile(const std::string &file) {

}
void SaveManager::overwriteSaveFile(const std::string &file) {

}
void SaveManager::deleteSaveFile(const std::string &file) {

}
