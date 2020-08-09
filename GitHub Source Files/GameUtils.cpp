#include "cocos2d.h"
#include "global.h"
#include "GameUtils.h"
#include "GameObjects.h"
#include "Dungeon.h"
#include "Actors.h"
#include <string>
#include <memory>
#include <unordered_map>

std::unordered_map<std::string, bool> LootTable::m_passivesObtained;
std::unordered_map<std::string, bool> LootTable::m_relicsObtained;
std::unordered_map<std::string, bool> NPCTable::m_NPCsEncountered;

unsigned int LootTable::m_commonPassiveCount = 0;
unsigned int LootTable::m_uncommonPassiveCount = 0;
unsigned int LootTable::m_rarePassiveCount = 0;
unsigned int LootTable::m_mythicalPassiveCount = 0;
unsigned int LootTable::m_legendaryPassiveCount = 0;


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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 80) {
			switch (1 + randInt(3)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 70) {
			switch (1 + randInt(5)) {
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
		switch (1 + randInt(11)) {
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
		int cols = dungeon->getCols();

		(*dungeon)[y*cols + x].object = item;
		(*dungeon)[y*cols + x].object->setSprite(dungeon->createSprite(x, y, -1, (*dungeon)[y*cols + x].object->getImageName()));
		(*dungeon)[y*cols + x].item = true;
		dungeon->addItem((*dungeon)[y*cols + x].object);
	}

	return item;
}
std::shared_ptr<Spell> rollSpell(Dungeon *dungeon, int x, int y, Rarity rarity, bool fallThrough) {
	if (rarity == Rarity::ALL)
		rarity = rollRarity();

	std::shared_ptr<Spell> item(nullptr);

	switch (rarity) {
	case Rarity::LEGENDARY: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {
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
		switch (1 + randInt(15)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {
			switch (1 + randInt(5)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 80) {
			switch (1 + randInt(8)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {
			switch (1 + randInt(10)) {
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
		switch (1 + randInt(5)) {
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

	switch (1 + randInt(4)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			//break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			//break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			//break;
		}
	}
	case Rarity::UNCOMMON: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			//break;
		}
	}
	case Rarity::COMMON: {
		switch (1 + randInt(7)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			//break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 80) {
			switch (1 + randInt(1)) {
			case 1: item = std::make_shared<GoldenChest>(dungeon, x, y); break;
			default: break;
			}
			break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && randReal(1, 100) > 70) {
			switch (1 + randInt(3)) {
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
			switch (1 + randInt(2)) {
			case 1: item = std::make_shared<SilverChest>(dungeon, x, y); break;
			case 2: item = std::make_shared<LifeChest>(dungeon, x, y); break;
			default: break;
			}
			break;
		}
	}
	case Rarity::COMMON: {
		switch (1 + randInt(2)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 98) {

			break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 90) {
			switch (1 + randInt(8)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 70) {
			switch (1 + randInt(10)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {
			switch (1 + randInt(11)) {
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
		switch (1 + randInt(8)) {
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
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			//break;
		}
	}
	case Rarity::MYTHICAL: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			//break;
		}
	}
	case Rarity::RARE: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			//break;
		}
	}
	case Rarity::UNCOMMON: {
		if (!fallThrough || fallThrough && 1 + randInt(100) > 50) {

			//break;
		}
	}
	case Rarity::COMMON: {
		switch (1 + randInt(5)) {
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
	}
}

void LootTable::initializeLootTable() {
	m_passivesObtained.insert(std::make_pair(BATWING, false));
	m_passivesObtained.insert(std::make_pair(LIFE_ELIXIR, false));
	m_passivesObtained.insert(std::make_pair(MAGIC_ESSENCE, false));
	m_passivesObtained.insert(std::make_pair(FLYING, false));
	m_passivesObtained.insert(std::make_pair(STEEL_PUNCH, false));
	m_passivesObtained.insert(std::make_pair(IRON_CLEATS, false));
	m_passivesObtained.insert(std::make_pair(LUCKY_PIG, false));
	m_passivesObtained.insert(std::make_pair(LIFESTEAL, false));
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

	switch (randInt(5)) {
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

std::shared_ptr<Monster> rollMonster(int level, Dungeon *dungeon, int x, int y) {
	std::shared_ptr<Monster> monster(nullptr);

	switch (level) {
	case FIRST_FLOOR: {
		switch (1 + randInt(12)) {
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
		switch (1 + randInt(7)) {
		case 1:	monster = std::make_shared<RabidWanderer>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<Toad>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<PoisonBubble>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<Piranha>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<WaterSpirit>(dungeon, x, y);	break;
		case 6:	monster = std::make_shared<AngledBouncer>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<Firefly>(dungeon, x, y); break;
		//case 8:	monster = std::make_shared<PouncingSpider>(dungeon, x, y); break;
		//case 9:	monster = std::make_shared<Goblin>(dungeon, x, y); break;
		//case 10: monster = std::make_shared<Wanderer>(dungeon, x, y); break;
		//case 11: monster = std::make_shared<SleepingWanderer>(dungeon, x, y); break;
		//case 12: monster = std::make_shared<ProvocableWanderer>(dungeon, x, y);	break;
		}

		break;
	}
	case THIRD_FLOOR: {
		switch (1 + randInt(7)) {
		case 1:	monster = std::make_shared<DeadSeeker>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<CrystalTurtle>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<CrystalHedgehog>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<CrystalShooter>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<CrystalBeetle>(dungeon, x, y); break;
		case 6:	monster = std::make_shared<Rabbit>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<Bombee>(dungeon, x, y); break;
		//case 8:	monster = std::make_shared<>(dungeon, x, y); break;
		//case 9:	monster = std::make_shared<>(dungeon, x, y); break;
		//case 10: monster = std::make_shared<>(dungeon, x, y); break;
		//case 11: monster = std::make_shared<>(dungeon, x, y); break;
		//case 12: monster = std::make_shared<>(dungeon, x, y);	break;
		}

		break;
	}
	case FOURTH_FLOOR: {
		switch (1 + randInt(10)) {
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
		switch (1 + randInt(10)) {
		case 1:	monster = std::make_shared<Puff>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<GustyPuff>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<StrongGustyPuff>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<InvertedPuff>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<SpikedInvertedPuff>(dungeon, x, y); break;
		case 6:	monster = std::make_shared<GustySpikedInvertedPuff>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<ArmoredBeetle>(dungeon, x, y); break;
		case 8:	monster = std::make_shared<SpectralSword>(dungeon, x, y); break;
		case 9: monster = std::make_shared<SandCentipede>(dungeon, x, y); break;
		case 10: monster = std::make_shared<Archer>(dungeon, x, y); break;
			//case 11: monster = std::make_shared<>(dungeon, x, y); break;
			//case 12: monster = std::make_shared<>(dungeon, x, y);	break;
		}

		break;
	}
	case SIXTH_FLOOR: {
		switch (1 + randInt(7)) {
		case 1:	monster = std::make_shared<Wisp>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<LightningWisp>(dungeon, x, y); break;
		case 3:	monster = std::make_shared<Grabber>(dungeon, x, y); break;
		case 4:	monster = std::make_shared<EvilShadow>(dungeon, x, y); break;
		case 5:	monster = std::make_shared<CrawlingSpine>(dungeon, x, y); break;
		case 6:	monster = std::make_shared<ConstructorDemon>(dungeon, x, y); break;
		case 7:	monster = std::make_shared<Howler>(dungeon, x, y); break;
		//case 8: monster = std::make_shared<>(dungeon, x, y); break;
		//case 9: monster = std::make_shared<>(dungeon, x, y); break;
		//case 10: monster = std::make_shared<>(dungeon, x, y); break;
		//case 11: monster = std::make_shared<>(dungeon, x, y); break;
		//case 12: monster = std::make_shared<>(dungeon, x, y);	break;
		}

		break;
	}
	case SEVENTH_FLOOR: {
		switch (1 + randInt(2)) {
		case 1:	monster = std::make_shared<Pikeman>(dungeon, x, y); break;
		case 2:	monster = std::make_shared<Shrinekeeper>(dungeon, x, y); break;
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
		}

		break;
	}
	case EIGHTH_FLOOR: {
		switch (1 + randInt(2)) {
		case 1: monster = std::make_shared<AbyssSummoner>(dungeon, x, y); break;
		case 2: monster = std::make_shared<MagicalBerserker>(dungeon, x, y); break;
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
		}

		break;
	}
	case NINTH_FLOOR: {
		switch (1 + randInt(10)) {
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
		}

		break;
	}
	}

	return monster;
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
void setDirectionalOffsets(char move, int &n, int &m) {
	// Key:
	// 1 u 2
	// l _ r
	// 3 d 4

	switch (move) {
	case 'l': n = -1; m = 0; break;
	case 'r': n = 1; m = 0; break;
	case 'u': n = 0; m = -1; break;
	case 'd': n = 0; m = 1; break;
	case '1': n = -1; m = -1; break;
	case '2': n = 1; m = -1; break;
	case '3': n = -1; m = 1; break;
	case '4': n = 1; m = 1; break;
	default: n = 0; m = 0; break;
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
char getFacingDirectionRelativeTo(int sx, int sy, int ex, int ey) {
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
bool isMovementAction(char move) {
	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd': return true;
	}

	return false;
}

bool hasLineOfSight(const Dungeon &dungeon, char dir, int sx, int sy, int ex, int ey) {
	int n, m;
	setDirectionalOffsets(dir, n, m);

	while (!(dungeon.wall(sx + n, sy + m) || dungeon.enemy(sx + n, sy + m) || dungeon.hero(sx + n, sy + m)))
		incrementDirectionalOffsets(dir, n, m);
	
	if (sx + n == ex && sy + m == ey)
		return true;

	return false;
}
bool hasLineOfSight(const Dungeon &dungeon, int sx, int sy, int ex, int ey) {
	if (sx == ex) {
		return hasLineOfSight(dungeon, sy < ey ? 'd' : 'u', sx, sy, ex, ey);
	}
	else if (sy == ey) {
		return hasLineOfSight(dungeon, sx < ex ? 'r' : 'l', sx, sy, ex, ey);
	}

	return false;
}
bool hasLineOfSight(const Dungeon &dungeon, const Player &p, int sx, int sy) {
	return hasLineOfSight(dungeon, sx, sy, p.getPosX(), p.getPosY());
}
bool playerInLinearRange(int range, int sx, int sy, int ex, int ey) {
	return ((abs(sx - ex) == 0 && abs(sy - ey) <= range) || (abs(sy - ey) == 0 && abs(sx - ex) <= range));
}
bool playerInBufferedLinearRange(int range, int buffer, int sx, int sy, int ex, int ey) {
	return ((abs(sx - ex) <= buffer && abs(sy - ey) <= range) || (abs(sy - ey) <= buffer && abs(sx - ex) <= range));
}
bool playerIsAdjacent(const Player &p, int x, int y, bool diagonals) {
	int px = p.getPosX();
	int py = p.getPosY();

	if (diagonals)
		return abs(px - x) <= 1 && abs(py - y) <= 1;

	return abs(px - x) + abs(py - y) == 1;
}