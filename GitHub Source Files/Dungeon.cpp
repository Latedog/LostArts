#include "AudioEngine.h"
#include "GUI.h"
#include "global.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include "Actors.h"
#include "utilities.h"
#include <iostream>
#include <memory>

using cocos2d::Vec2;
using std::shared_ptr;
using std::make_shared;
using std::dynamic_pointer_cast;
using std::string;
using std::to_string;
using std::cout;
using std::endl;


Dungeon::Dungeon() : m_level(1) {
	int rows = getRows();
	int cols = getCols();

	_Tile *tile;
	int i, j;
	
	//	initialize entire dungeon to blank space
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			tile = new _Tile;
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = SPACE;
			tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = false;
			tile->gold = 0;
			tile->marked = false;
			tile->object = nullptr;

			m_maze.push_back(*tile);
			delete tile;
		}
	}

	for (i = 0; i < rows; i++) { //	initialize top and bottom of dungeon to be unbreakable walls
		for (j = 0; j < cols; j++) {
			m_maze[i*cols + j].top = UNBREAKABLE_WALL;
			m_maze[i*cols + j].bottom = UNBREAKABLE_WALL;
			m_maze[i*cols + j].traptile = UNBREAKABLE_WALL;

			m_maze[i*cols + j].wall = true;
		}
		i += (rows - 2);
	}

	for (i = 0; i < rows - 1; i++){
		//	initialize edges of dungeon to be walls
		m_maze[i*cols].top = m_maze[i*cols].bottom = m_maze[i*cols].traptile = UNBREAKABLE_WALL;
		m_maze[i*cols + cols - 1].top = m_maze[i*cols + cols -1].bottom = m_maze[i*cols + cols - 1].traptile = UNBREAKABLE_WALL;
		m_maze[i*cols].wall = true;
		m_maze[i*cols + cols - 1].wall = true;
	}

	// RANDOMLY GENERATES LEVEL CHUNKS
	std::vector<std::vector<std::vector<char>>> b;
	std::vector<char> v, finalvec;

	// chooses layer for the special chunks to be on
	specialChunkLayer1 = 1 + randInt(4);
	specialChunkLayer2 = 1 + randInt(4);
	layer = 1;

	int count = 0;
	while (count < (rows - 2) / 4) {
		if (count == 0) {
			v = topChunks(b, cols);
			layer++;
		}
		else if (count == 1 || count == 2) {
			v = middleChunks(b, cols);
			layer++;
		}
		else if (count == 3) {
			v = bottomChunks(b, cols);
			layer++;
		}

		for (i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i) == '#' ? WALL : v.at(i));

		b.clear();
		count++;
	}

	/*v1 = topChunks(b1, cols); layer++;
	v2 = middleChunks(b2, cols); layer++;
	v3 = middleChunks(b3, cols); layer++;
	v4 = bottomChunks(b4, cols);

	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v1.at(i) == '#' ? WALL : v1.at(i));


	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v2.at(i) == '#' ? WALL : v2.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v3.at(i) == '#' ? WALL : v3.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v4.at(i) == '#' ? WALL : v4.at(i));*/
	
	bool wall, enemy, _trap, _item, hero;

	count = 0;
	char object;
	std::shared_ptr<Monster> monster(nullptr);
	for (i = 1; i < rows - 1; i++) {
		for (j = 1; j < cols - 1; j++) {
			m_maze[i*cols + j].top = finalvec.at(count);
			m_maze[i*cols + j].bottom = finalvec.at(count);
			if (m_maze[i*cols + j].top == WALL || m_maze[i*cols + j].top == UNBREAKABLE_WALL) {
				m_maze[i*cols + j].traptile = finalvec.at(count);
				m_maze[i*cols + j].wall = true;
			}

			//	if the object was an item or enemy
			if (m_maze[i*cols + j].bottom != WALL && m_maze[i*cols + j].bottom != SPACE) {

				object = m_maze[i*cols + j].bottom;

				switch (object) {
				case c_BROWN_CHEST: {
					std::shared_ptr<Objects> objects = std::make_shared<BrownChest>();
					m_maze[i*cols + j].object = objects;
					m_maze[i*cols + j].top = SPACE;
					m_maze[i*cols + j].item_name = BROWN_CHEST;
					m_maze[i*cols + j].item = true;
					break;
				}
				case c_SILVER_CHEST: {
					std::shared_ptr<Objects> objects = std::make_shared<SilverChest>();
					m_maze[i*cols + j].object = objects;
					m_maze[i*cols + j].top = SPACE;
					m_maze[i*cols + j].item_name = SILVER_CHEST;
					m_maze[i*cols + j].item = true;
					break;
				}
				case WANDERER: {
					m_maze[i*cols + j].bottom = SPACE;
					monster = std::make_shared<Wanderer>();
					monster->setPosX(j);
					monster->setPosY(i);
					m_monsters.emplace_back(monster);
					m_maze[i*cols + j].enemy = true;
					break;
				}
				case GOBLIN: {
					m_maze[i*cols + j].bottom = SPACE;
					monster = std::make_shared<Goblin>(12);
					monster->setPosX(j);
					monster->setPosY(i);
					m_monsters.emplace_back(monster);
					m_maze[i*cols + j].enemy = true;
					break;
				}
				default:
					break;
				}
			}
			count++;
		}
	}
	// END LEVEL GENERATION


	Player p;
	player.push_back(p);

	char toptile, bottomtile, traptile;

	wall = m_maze[player.at(0).getPosY()*cols + player.at(0).getPosX()].wall;
	enemy = m_maze[player.at(0).getPosY()*cols + player.at(0).getPosX()].enemy;
	_trap = m_maze[player.at(0).getPosY()*cols + player.at(0).getPosX()].trap;

	while (wall || enemy || _trap || player.at(0).getPosX() > 4 && player.at(0).getPosX() < 65) {
		player.at(0).setrandPosX(cols);
		player.at(0).setrandPosY(rows);

		// while player is not spawned along left side or right side, reroll
		while (player.at(0).getPosX() > 4 && player.at(0).getPosX() < 65) {
			player.at(0).setrandPosX(cols);
		}

		wall = m_maze[player.at(0).getPosY()*cols + player.at(0).getPosX()].wall;
		enemy = m_maze[player.at(0).getPosY()*cols + player.at(0).getPosX()].enemy;
		_trap = m_maze[player.at(0).getPosY()*cols + player.at(0).getPosX()].trap;
	}

	m_maze[player.at(0).getPosY()*cols + player.at(0).getPosX()].hero = true;
	player.at(0).addItem(std::make_shared<InvisibilitySpell>());
	player.at(0).addItem(std::make_shared<Teleport>());
	player.at(0).addItem(std::make_shared<DizzyElixir>());
	player.at(0).addItem(std::make_shared<WindSpell>());
	player.at(0).addWeapon(std::make_shared<IronLance>());
	player.at(0).addWeapon(std::make_shared<BronzeDagger>());
	player.at(0).equipShield(WoodShield());


	std::shared_ptr<Objects> stairs = std::make_shared<Stairs>();
	traptile = m_maze[stairs->getPosY()*cols + stairs->getPosX()].traptile;
	
	///	while stairs do not spawn on the side opposite from the player
	//while (traptile != SPACE || ((player.at(0).getPosX() < 5 && stairs->getPosX() < 65) || (player.at(0).getPosX() > 64 && stairs->getPosX() > 4))) {
	while (traptile != SPACE || ((player.at(0).getPosX() < 5 && stairs->getPosX() >= 5) || (player.at(0).getPosX() > 64 && stairs->getPosX() < 64))) {
		stairs->setrandPosX(cols);
		stairs->setrandPosY(rows);

		traptile = m_maze[stairs->getPosY()*cols + stairs->getPosX()].traptile;
	}
	m_maze[stairs->getPosY()*cols + stairs->getPosX()].traptile = STAIRS;
	m_maze[stairs->getPosY()*cols + stairs->getPosX()].trap = true;
	m_traps.emplace_back(stairs);


	int n;

	// BEGIN MONSTER GENERATION
	char monstertype;
	for (int i = 0; i < 9; i++) {

		// number of each enemy to spawn
		switch (i) {
		case 0: n = 3 + randInt(2); break; // Goblin
		case 1: n = 8 + randInt(2); break; // Wanderer
		case 2: n = 3 + randInt(5); break; // Archer
		case 3: n = 5 + randInt(4); break; // Zapper
		case 4: n = 3 + randInt(6); break; // Spinner
		case 5: n = 1; break;			   // Bombee
		case 6: n = 5 + randInt(3); break; // Roundabout
		case 7: n = 0 + randInt(3); break; // Mounted Knight
		case 8: n = 2 + randInt(4); break; // Seeker
		}

		while (n > 0) {

			// the type of enemy
			switch (i) {
			case 0:
				monster = std::make_shared<Goblin>(10);
				monstertype = GOBLIN; break;
			case 1: 
				monster = std::make_shared<Wanderer>();
				monstertype = WANDERER; break;
			case 2:
				monster = std::make_shared<Archer>();
				monstertype = ARCHER; break;
			case 3: 
				monster = std::make_shared<Zapper>();
				monstertype = ZAPPER; break;
			case 4: 
				monster = std::make_shared<Spinner>();
				monstertype = SPINNER; break;
			case 5: 
				monster = std::make_shared<Bombee>();
				monstertype = BOMBEE; break;
			case 6: 
				monster = std::make_shared<Roundabout>();
				monstertype = ROUNDABOUT; break;
			case 7: 
				monster = std::make_shared<MountedKnight>();
				monstertype = MOUNTED_KNIGHT; break;
			case 8: 
				monster = std::make_shared<Seeker>(10);
				monstertype = SEEKER; break;
			}

			wall = m_maze[monster->getPosY()*cols + monster->getPosX()].wall;
			enemy = m_maze[monster->getPosY()*cols + monster->getPosX()].enemy;
			hero = m_maze[monster->getPosY()*cols + monster->getPosX()].hero;

			while (wall || enemy || hero) { // while monster position clashes with wall, player, or idol, reroll its position
				monster->setrandPosX(cols);
				monster->setrandPosY(rows);

				wall = m_maze[monster->getPosY()*cols + monster->getPosX()].wall;
				enemy = m_maze[monster->getPosY()*cols + monster->getPosX()].enemy;
				hero = m_maze[monster->getPosY()*cols + monster->getPosX()].hero;
			}
			m_monsters.emplace_back(monster);

			m_maze[monster->getPosY()*cols + monster->getPosX()].top = monstertype;
			m_maze[monster->getPosY()*cols + monster->getPosX()].enemy = true;

			if (monstertype == SPINNER) {
				std::shared_ptr<Spinner> spinner = std::dynamic_pointer_cast<Spinner>(monster);
				spinner->setInitialFirePosition(monster->getPosX(), monster->getPosY());
				spinner.reset();
			}

			n--;
			monster.reset();
		}
	}

	/// END MONSTER GENERATION

	
	//	TRAP GENERATION
	char traptype;
	std::string trap_type = "";
	std::shared_ptr<Objects> trap(nullptr);
	
	for (int i = 0; i < 7; i++) { // i < number_of_different_trap_types

		// number of each trap to spawn
		switch (i) {
		case 0: n = 2 + randInt(5); break; // Pit
		case 1: n = 3 + randInt(3); break; // Firebars
		case 2: n = 2 + randInt(2); break; // Double Firebars
		case 3: n = 15 + randInt(5); break; // Puddles
		case 4: n = 10 + randInt(10); break; // Springs
		case 5: n = 5 + randInt(3); break; // Turrets
		case 6: n = 2 + randInt(3); break; // Moving Blocks
		case 7: n = 3 + randInt(3); break; // 
		case 8: n = 4 + randInt(4); break; // 
		default: n = 0; break;
		}

		while (n > 0) {
			// type of item to spawn
			switch (i) {
			case 0:
				trap = std::make_shared<Pit>();
				trap_type = PIT; break;
			case 1:
				trap = std::make_shared<Firebar>(1 + randInt(cols - 2), 1 + randInt(rows - 2));
				trap_type = FIREBAR; break;
			case 2:
				trap = std::make_shared<DoubleFirebar>(1 + randInt(cols - 2), 1 + randInt(rows - 2));
				trap_type = DOUBLE_FIREBAR; break;
			case 3:
				trap = std::make_shared<Puddle>(1 + randInt(cols - 2), 1 + randInt(rows - 2));
				trap_type = PUDDLE; break;
			case 4:
				char move;
				switch (1 + randInt(8)) {
				case 1: move = 'l'; break;	// L
				case 2: move = 'r'; break;	// R
				case 3: move = 'u'; break;	// U
				case 4: move = 'd'; break;	// D
				case 5: move = '1'; break;	// Q1
				case 6: move = '2'; break;	// Q2
				case 7: move = '3'; break;	// Q3
				case 8: move = '4'; break;	// Q4
				}
				//									(int x, int y, bool trigger, char direction = random)
				trap = std::make_shared<Spring>(1 + randInt(cols - 2), 1 + randInt(rows - 2), false);
				//											(int x, int y, bool trigger, bool known, bool cardinal)
				//trap = std::make_shared<Spring>(1 + randInt(cols - 2), 1 + randInt(rows - 2), true, true, true);
				trap_type = SPRING; break;
			case 5:
				//char move;
				switch (1 + randInt(4)) {
				case 1: move = 'l'; break;	// L
				case 2: move = 'r'; break;	// R
				case 3: move = 'u'; break;	// U
				case 4: move = 'd'; break;	// D
				}
				trap = std::make_shared<Turret>(1 + randInt(cols - 2), 1 + randInt(rows - 2), move);
				trap_type = TURRET; break;
			case 6:
				trap = std::make_shared<MovingBlock>(1 + randInt(cols - 2), 1 + randInt(rows - 2), (randInt(2) == 0 ? 'h' : 'v'));
				trap_type = MOVING_BLOCK; break;
			case 7:
				//item = make_shared<MountedKnight>();
				//itemtype = MOUNTED_KNIGHT; break;
			case 8:
				//item = std::make_shared<InfinityBox>();
				break;//itemtype = INFINITY_BOX; break;
			}


			//traptile = m_maze[trap->getPosY()*cols + trap->getPosX()].traptile;
			_trap = m_maze[trap->getPosY()*cols + trap->getPosX()].trap;
			wall = m_maze[trap->getPosY()*cols + trap->getPosX()].wall;
			enemy = m_maze[trap->getPosY()*cols + trap->getPosX()].enemy;
			hero = m_maze[trap->getPosY()*cols + trap->getPosX()].hero;
			_item = m_maze[trap->getPosY()*cols + trap->getPosX()].item;

			// while item position clashes with wall, player, etc., then reroll its position
			while (_trap || wall || enemy || hero || _item) {
				trap->setrandPosX(cols);
				trap->setrandPosY(rows);

				//traptile = m_maze[trap->getPosY()*cols + trap->getPosX()].traptile;
				_trap = m_maze[trap->getPosY()*cols + trap->getPosX()].trap;
				wall = m_maze[trap->getPosY()*cols + trap->getPosX()].wall;
				enemy = m_maze[trap->getPosY()*cols + trap->getPosX()].enemy;
				hero = m_maze[trap->getPosY()*cols + trap->getPosX()].hero;
				_item = m_maze[trap->getPosY()*cols + trap->getPosX()].item;
			}
			m_actives.emplace_back(trap);
			m_traps.emplace_back(trap);

			//m_maze[trap->getPosY()*cols + trap->getPosX()].traptile = traptype;
			m_maze[trap->getPosY()*cols + trap->getPosX()].trap_name = trap_type;
			m_maze[trap->getPosY()*cols + trap->getPosX()].trap = true;

			if (trap_type == FIREBAR) {
				m_maze[trap->getPosY()*cols + trap->getPosX()].wall = true;
				std::shared_ptr<Firebar> firebar = std::dynamic_pointer_cast<Firebar>(trap);
				firebar->setInitialFirePosition(trap->getPosX(), trap->getPosY());
				firebar.reset();
			}
			else if (trap_type == DOUBLE_FIREBAR) {
				m_maze[trap->getPosY()*cols + trap->getPosX()].wall = true;
				std::shared_ptr<DoubleFirebar> firebar = std::dynamic_pointer_cast<DoubleFirebar>(trap);
				firebar->setInitialFirePosition(trap->getPosX(), trap->getPosY());
				firebar.reset();
			}
			else if (trap_type == TURRET) {
				m_maze[trap->getPosY()*cols + trap->getPosX()].wall = true;
			}
			else if (trap_type == MOVING_BLOCK) {
				m_maze[trap->getPosY()*cols + trap->getPosX()].wall = true;
				//m_maze[trap->getPosY()*cols + trap->getPosX()].trap = false;
			}

			n--;
			trap.reset();
		}
	}
	/// END TRAP GENERATION


	//	ITEM GENERATION
	char itemtype;
	std::string item_name = "";
	std::shared_ptr<Objects> item(nullptr);

	for (int i = 0; i < 14; i++) { // i < number_of_different_items

		// number of each item to spawn
		switch (i) {
		case 0: n = 1; break; // Life Potion
		case 1: n = 1; break; // Bomb
		case 2: n = 1; break; // Brown Chest
		case 3: n = 1; break; // Rusty Cutlass
		case 4: n = 1; break; // Skeleton Key
		case 5: n = 1; break; // Wood Shield
		case 6: n = 2; break; // Freeze Spell
		case 7: n = 3 + randInt(3); break; // Earthquake Spell
		case 8: n = 4 + randInt(4); break; // 
		case 9: n = 4 + randInt(4); break; // 
		case 10: n = 4 + randInt(4); break; // 
		case 11: n = 4 + randInt(4); break; // 
		case 12: n = 5; break;
		case 13: n = 5; break;
		default: n = 0; break;
		}

		while (n > 0) {
			// type of item to spawn
			switch (i) {
			case 0:
				item = std::make_shared<LifePotion>();
				//itemtype = LIFEPOT;
				item_name = item->getItem(); break;
			case 1:
				item = std::make_shared<Bomb>();
				//itemtype = BOMB;
				item_name = item->getItem(); break;
			case 2:
				item = std::make_shared<BrownChest>();
				//itemtype = BROWN_CHEST;
				item_name = item->getItem(); break;
			case 3:
				item = std::make_shared<RustyCutlass>();
				//itemtype = CUTLASS;
				item_name = item->getItem(); break;
			case 4:
				item = std::make_shared<SkeletonKey>();
				//itemtype = SKELETON_KEY;
				item_name = item->getItem(); break;
			case 5:
				item = std::make_shared<WoodShield>(randInt(cols - 2) + 1, randInt(rows - 2) + 1);
				//itemtype = WOOD_SHIELD;
				item_name = item->getItem(); break;
			case 6:
				item = std::make_shared<FreezeSpell>(randInt(cols - 2) + 1, randInt(rows - 2) + 1);
				itemtype = '-';
				item_name = item->getItem(); break;
			case 7:
				item = std::make_shared<EarthquakeSpell>(randInt(cols - 2) + 1, randInt(rows - 2) + 1);
				itemtype = '-';
				item_name = item->getItem(); break;
			case 8:
				item = std::make_shared<FireBlastSpell>(randInt(cols - 2) + 1, randInt(rows - 2) + 1);
				itemtype = '-';
				item_name = item->getItem(); break;
			case 9:
				item = std::make_shared<WindSpell>(randInt(cols - 2) + 1, randInt(rows - 2) + 1);
				itemtype = '-';
				item_name = item->getItem(); break;
			case 10:
				item = std::make_shared<InvisibilitySpell>(randInt(cols - 2) + 1, randInt(rows - 2) + 1);
				itemtype = '-';
				item_name = item->getItem(); break;
			case 11:
				item = std::make_shared<EtherealSpell>(randInt(cols - 2) + 1, randInt(rows - 2) + 1);
				itemtype = '-';
				item_name = item->getItem(); break;
			case 12:
				item = std::make_shared<Teleport>(randInt(cols - 2) + 1, randInt(rows - 2) + 1);
				itemtype = '-';
				item_name = item->getItem(); break;
			case 13:
				item = std::make_shared<DizzyElixir>(randInt(cols - 2) + 1, randInt(rows - 2) + 1);
				itemtype = '-';
				item_name = item->getItem(); break;
			}

			_item = m_maze[item->getPosY()*cols + item->getPosX()].item;
			wall = m_maze[item->getPosY()*cols + item->getPosX()].wall;
			_trap = m_maze[item->getPosY()*cols + item->getPosX()].trap;

			// while item position clashes with wall, player, etc., then reroll its position
			while (_item || wall || _trap) {
				item->setrandPosX(cols);
				item->setrandPosY(rows);

				_item = m_maze[item->getPosY()*cols + item->getPosX()].item;
				wall = m_maze[item->getPosY()*cols + item->getPosX()].wall;
				_trap = m_maze[item->getPosY()*cols + item->getPosX()].trap;
			}
			m_maze[item->getPosY()*cols + item->getPosX()].object = item;
			//m_maze[item->getPosY()*cols + item->getPosX()].bottom = itemtype;
			m_maze[item->getPosY()*cols + item->getPosX()].item_name = item_name;
			m_maze[item->getPosY()*cols + item->getPosX()].item = true;
			n--;
			item.reset();
		}
	}
	/// END ITEM GENERATION

}
Dungeon::Dungeon(int level) : m_level(level) {
	Player p;
	player.push_back(p);
}
Dungeon& Dungeon::operator=(Dungeon const &dungeon) {
	for (unsigned i = 0; i < dungeon.getRows()*dungeon.getCols(); i++) {
		m_maze[i] = dungeon.m_maze[i];
	}
	player = dungeon.player;
	//dungeonText = dungeon.dungeonText;

	m_monsters = dungeon.m_monsters;
	m_actives = dungeon.m_actives;
	m_traps = dungeon.m_traps;

	m_level = dungeon.m_level;
	layer = dungeon.layer;
	specialChunkLayer1 = dungeon.specialChunkLayer1;
	specialChunkLayer2 = dungeon.specialChunkLayer2;

	m_spawn.clear();

	return *this;
}
_Tile& Dungeon::operator[](int index) {
	return this->getDungeon()[index];
}
Dungeon::~Dungeon() {
	
}

void Dungeon::peekDungeon(int x, int y, char move) {
	int rows = getRows();
	int cols = getCols();

	char top, projectile, bottom, traptile;
	bool wall, item, trap, enemy;

	int initHP = player.at(0).getHP(); // used for checking if player took damage ever
	
	// reset player's blocking stance
	player.at(0).setBlock(false);

	// set player's facing direction
	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd':
		player.at(0).setFacingDirection(move); break;
	}

	// check for any afflictions
	player.at(0).checkAfflictions();

	// if player is confused, switch their movement direction
	if (player.at(0).isConfused()) {
		switch (move) {
		case 'l': move = 'r'; break;
		case 'r': move = 'l'; break;
		case 'u': move = 'd'; break;
		case 'd': move = 'u'; break;
		}
	}

	// if player was ethereal and is on top of a wall, they die instantly
	if (!player.at(0).isEthereal() && m_maze[player.at(0).getPosY() * cols + player.at(0).getPosX()].wall) {
		cocos2d::experimental::AudioEngine::play2d("Female_Falling_Scream_License.mp3", false, 1.0f);
		deathFade(player.at(0).getSprite());
		player.at(0).setHP(0);
		return;
	}

	// if player is not immobilized, move them
	if (!(player.at(0).isStunned() || player.at(0).isFrozen())) {

		// move player
		int weprange = player.at(0).getWeapon()->getRange();
		bool move_used = false;
		bool far_enemy, close_enemy;

		// if weapon has a range of 2, check for monsters to attack
		if (weprange == 2) {
			if (move == 'l') {
				//top = m_maze[y*cols + x - 1].top;
				wall = m_maze[y*cols + x - 1].wall;
				close_enemy = m_maze[y*cols + x - 1].enemy;
				far_enemy = m_maze[y*cols + x - 2].enemy;

				if (close_enemy) {
					fight(*this, x - 1, y);
					move_used = true;
				}
				else if (far_enemy && !wall && y*cols + x - 2 > 0) {
					fight(*this, x - 2, y);
					move_used = true;
				}
			}
			else if (move == 'r') {
				//top = m_maze[y*cols + x + 1].top;
				wall = m_maze[y*cols + x + 1].wall;
				close_enemy = m_maze[y*cols + x + 1].enemy;
				far_enemy = m_maze[y*cols + x + 2].enemy;

				if (close_enemy) {
					fight(*this, x + 1, y);
					move_used = true;
				}
				else if (far_enemy && !wall && y*cols + x + 2 < rows * cols) {
					fight(*this, x + 2, y);
					move_used = true;
				}
			}
			else if (move == 'u') {
				//top = m_maze[(y - 1)*cols + x].top;
				wall = m_maze[(y - 1)*cols + x].wall;
				close_enemy = m_maze[(y - 1)*cols + x].enemy;

				// if player is along top edge, assign far_enemy to close_enemy instead
				far_enemy = (y == 1 ? close_enemy : m_maze[(y - 2)*cols + x].enemy);

				if (close_enemy) {
					fight(*this, x, y - 1);
					move_used = true;
				}
				else if (far_enemy && !wall && (y - 2)*cols + x > 0) {
					fight(*this, x, y - 2);
					move_used = true;
				}
			}
			else if (move == 'd') {
				//top = m_maze[(y + 1)*cols + x].top;
				wall = m_maze[(y + 1)*cols + x].wall;
				close_enemy = m_maze[(y + 1)*cols + x].enemy;
				far_enemy = (y == rows - 2 ? close_enemy : m_maze[(y + 2)*cols + x].enemy);

				if (close_enemy) {
					fight(*this, x, y + 1);
					move_used = true;
				}
				else if (far_enemy && !wall && (y + 2)*cols + x < rows * cols) {
					fight(*this, x, y + 2);
					move_used = true;
				}
			}
		}

		if (!move_used) {

			if (move == 'l') { // moved left
				top = m_maze[y*cols + x - 1].top;
				bottom = m_maze[y*cols + x - 1].bottom;
				traptile = m_maze[y*cols + x - 1].trap;

				wall = (player.at(0).isEthereal() ? false : m_maze[y*cols + x - 1].wall);
				item = m_maze[y*cols + x - 1].item;
				trap = m_maze[y*cols + x - 1].trap;
				enemy = m_maze[y*cols + x - 1].enemy;

				if (!(wall || enemy)) {
					// move character to the left
					m_maze[y*cols + x].hero = false;
					m_maze[y*cols + x - 1].hero = true;
					player.at(0).setPosX(x - 1);

					if (trap) {
						// queue player move
						queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY());

						trapEncounter(*this, x - 1, y);
					}
					if (item) {
						foundItem(m_maze, cols, x - 1, y);
					}
				}
				else if (enemy) {	// monster encountered
					fight(*this, x - 1, y);
				}
			}

			else if (move == 'r') { // moved right
				top = m_maze[y*cols + x + 1].top;
				bottom = m_maze[y*cols + x + 1].bottom;
				traptile = m_maze[y*cols + x + 1].trap;

				wall = (player.at(0).isEthereal() ? false : m_maze[y*cols + x + 1].wall);
				item = m_maze[y*cols + x + 1].item;
				trap = m_maze[y*cols + x + 1].trap;
				enemy = m_maze[y*cols + x + 1].enemy;

				if (!(wall || enemy)) {
					// move character to the left
					m_maze[y*cols + x].hero = false;
					m_maze[y*cols + x + 1].hero = true;
					player.at(0).setPosX(x + 1);

					if (trap) {
						// queue player move
						queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY());

						trapEncounter(*this, x + 1, y);
					}
					if (item) {
						foundItem(m_maze, cols, x + 1, y);
					}
				}
				else if (enemy) {
					fight(*this, x + 1, y);
				}
			}

			else if (move == 'u') { // moved up
				top = m_maze[(y - 1)*cols + x].top;
				bottom = m_maze[(y - 1)*cols + x].bottom;
				traptile = m_maze[(y - 1)*cols + x].traptile;

				wall = (player.at(0).isEthereal() ? false : m_maze[(y - 1)*cols + x].wall);
				item = m_maze[(y - 1)*cols + x].item;
				trap = m_maze[(y - 1)*cols + x].trap;
				enemy = m_maze[(y - 1)*cols + x].enemy;

				if (!(wall || enemy)) {
					m_maze[y*cols + x].hero = false;
					m_maze[(y - 1)*cols + x].hero = true;
					player.at(0).setPosY(y - 1);

					if (trap) {
						// queue player move
						queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY());
						
						trapEncounter(*this, x, y - 1);
					}
					if (item) {
						foundItem(m_maze, cols, x, y - 1);
					}
				}
				else if (enemy) {
					fight(*this, x, y - 1);
				}
			}

			else if (move == 'd') { // moved down
				top = m_maze[(y + 1)*cols + x].top;
				bottom = m_maze[(y + 1)*cols + x].bottom;
				traptile = m_maze[(y + 1)*cols + x].traptile;

				wall = (player.at(0).isEthereal() ? false : m_maze[(y + 1)*cols + x].wall);
				item = m_maze[(y + 1)*cols + x].item;
				trap = m_maze[(y + 1)*cols + x].trap;
				enemy = m_maze[(y + 1)*cols + x].enemy;

				if (!(wall || enemy)) {
					m_maze[y*cols + x].hero = false;
					m_maze[(y + 1)*cols + x].hero = true;
					player.at(0).setPosY(y + 1);

					if (trap) {
						// queue player move
						queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY());

						trapEncounter(*this, x, y + 1);
					}
					if (item) {
						foundItem(m_maze, cols, x, y + 1);
					}
				}
				else if (enemy) {
					fight(*this, x, y + 1);
				}
			}

			else if (move == 'e') {
				collectItem(m_maze, rows, cols, x, y);

				if (m_level > 1)
					return;
			}

			else if (move == 'b') {
				if (player.at(0).hasShield()) {
					// play setting up shield stance sound effect
					cocos2d::experimental::AudioEngine::play2d("Shield_Stance.mp3", false, 1.0f);

					player.at(0).setBlock(true);
				}
			}

			else if (move == 'w') {
				//showDungeon();
				//player.at(0).wield(dungeonText);
				//player.at(0).wield();
			}

			else if (move == 'c') {
				//showDungeon();
				//player.at(0).use(m_actives, m_maze[y*cols + x], index);
			}
		}

	}

	// queue player move
	if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
		queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// pick up any gold/money that was on the ground
	if (m_maze[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
		goldPickup(m_maze, rows, cols, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// update sprite lighting
	updateLighting(*this, getMonsters(), getTraps());

	// check active items in play
	checkActive(m_maze, rows, cols, m_actives, m_monsters);

	/*
	//	if player is dead then return
	if (player.at(0).getHP() <= 0) {
		//m_maze[y*cols + x].top = 'X'; // change player icon to X to indicate death

		//clearScreen();
		//showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}*/

	// if all monsters are dead?
	if (m_monsters.empty()) {
		//clearScreen();
		//showDungeon();
		//if (!dungeonText.empty())
			//showText();

		return;
	}

	// :::: Move monsters ::::
	int mx, my, shortest, mInitHP;
	char first, best, prev;
	std::string monster;

	for (unsigned i = 0; i < m_monsters.size(); i++) {
		mInitHP = getMonsters().at(i)->getHP();
		monster = m_monsters.at(i)->getName();
		mx = m_monsters.at(i)->getPosX();
		my = m_monsters.at(i)->getPosY();
		first = best = prev = '0';
		shortest = 0;

		// check for any afflictions
		m_monsters.at(i)->checkAfflictions();

		// check if an affliction killed the monster
		if (m_monsters.at(i)->getHP() <= 0) {
			monsterDeath(m_maze, rows, cols, m_actives, m_monsters, i);
			i--;
			continue;
		}

		// if not stunned, use turn to move
		if (!(getMonsters().at(i)->isStunned() || getMonsters().at(i)->isFrozen())) {

			// if player is invisible, check if the monster chases them
			if (player.at(0).isInvisible()) {
				if (getMonsters().at(i)->chasesPlayer()) {
					continue;
				}
			}

			if (monster == "goblin") {
				//	allows use of goblin's smell distance function
				std::shared_ptr<Goblin> goblin = std::dynamic_pointer_cast<Goblin>(m_monsters.at(i));

				if (playerInRange(mx, my, i, goblin->getSmellDistance())) {
					// if viable path is found
					if (attemptChase(m_maze, cols, i, shortest, goblin->getSmellDistance(), goblin->getSmellDistance(), mx, my, first, best, prev)) {
						moveWithSuccessfulChase(m_maze, rows, cols, mx, my, i, best, m_monsters);
					}
				}
				goblin.reset();
			}

			else if (monster == "wanderer") {
				moveWanderers(m_maze, rows, cols, mx, my, i, m_monsters);
			}

			else if (monster == "archer") {
				checkArchers(m_maze, rows, cols, player.at(0).getPosX(), player.at(0).getPosY(), i, m_monsters);
			}

			else if (monster == "zapper") {
				moveZappers(m_maze, cols, mx, my, i, m_monsters);
			}

			else if (monster == "spinner") {
				moveSpinners(m_maze, rows, cols, mx, my, i, m_monsters);
			}

			else if (monster == "bombee") {
				std::shared_ptr<Bombee> bombee = std::dynamic_pointer_cast<Bombee>(m_monsters.at(i));
				if (playerInRange(mx, my, i, bombee->getRange())) {
					// if viable path is found
					if (attemptChase(m_maze, cols, i, shortest, bombee->getRange(), bombee->getRange(), mx, my, first, best, prev)) {
						moveWithSuccessfulChase(m_maze, rows, cols, mx, my, i, best, m_monsters);
					}
				}
				bombee.reset();
			}

			else if (monster == "roundabout") {
				moveRoundabouts(m_maze, rows, cols, mx, my, i, m_monsters);
			}

			else if (monster == "mounted knight") {
				checkMountedKnights(m_maze, rows, cols, player.at(0).getPosX(), player.at(0).getPosY(), i, m_monsters);
			}

			else if (monster == "seeker") {
				std::shared_ptr<Seeker> seeker = std::dynamic_pointer_cast<Seeker>(m_monsters.at(i));

				if (playerInRange(mx, my, i, seeker->getRange())) {
					// if viable path is found
					if (attemptChase(m_maze, cols, i, shortest, seeker->getRange(), seeker->getRange(), mx, my, first, best, prev)) {
						moveSeekers(m_maze, rows, cols, mx, my, i, best, m_monsters);
					}
				}
				seeker.reset();
			}

			// flash monster sprite if damage was taken
			if (getMonsters().at(i)->getHP() < mInitHP) {
				runMonsterDamage(getMonsters().at(i)->getSprite());
			}

			//	if player is dead then break
			if (player.at(0).getHP() <= 0) {
				player.at(0).setDeath(m_monsters.at(i)->getName());
				break;
			}
		}
	}

	// check monsters on traps
	monsterTrapEncounter(*this);

	// check if player is holding the skeleton key
	if (player.at(0).hasSkeletonKey()) {
		player.at(0).checkKeyConditions(dungeonText);
	}

	// flash player sprite if took damage
	if (player.at(0).getHP() < initHP) {
		// if player is invisible, they lose invisibility when damaged
		if (player.at(0).isInvisible()) {
			player.at(0).removeAffliction("invisibility");
		}

		runPlayerDamage(player.at(0).getSprite());
	}

	// run actions
	runSpawn();

	//clearScreen();
	//showDungeon();
	//if (!dungeonText.empty())
		//showText();
}

void Dungeon::checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters) {
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int x, y;
	int pos;

	if (!actives.empty()) {
		for (unsigned i = 0; i < actives.size(); i++) {
			if (actives.at(i)->getItem() == "Bomb") {
				std::shared_ptr<Bomb> bomb = std::dynamic_pointer_cast<Bomb>(actives.at(i));
				if (bomb->getFuse() > 0) {
					bomb->setFuse(bomb->getFuse() - 1);
					actives.at(i) = bomb;
				}
				else {
					// play explosion sound effect
					cocos2d::experimental::AudioEngine::play2d("Explosion.mp3", false, 1.0f);

					//dungeonText.push_back("A bomb exploded!\n");
					//explosion(dungeon, maxrows, maxcols, actives, monsters, bomb->getPosX(), bomb->getPosY(), i);
					explosion(*this, bomb->getPosX(), bomb->getPosY());
					dungeon[bomb->getPosY()*maxcols + bomb->getPosX()].extra = SPACE;

					// remove bomb sprite
					removeSprite(projectile_sprites, maxrows, bomb->getPosX(), bomb->getPosY());

					// set i to pos because otherwise the iterator is destroyed
					pos = i;
					actives.erase(actives.begin() + pos);
					i--;
				}
				bomb.reset();
			}
			else if (actives.at(i)->getItem() == "Mega Bomb") {
				std::shared_ptr<MegaBomb> megabomb = std::dynamic_pointer_cast<MegaBomb>(actives.at(i));
				if (megabomb->getFuse() > 0) {
					megabomb->setFuse(megabomb->getFuse() - 1);
					actives.at(i) = megabomb;
				}
				else {
					// play explosion sound effect
					cocos2d::experimental::AudioEngine::play2d("Mega_Explosion.mp3", false, 1.0);

					//dungeonText.push_back("BOOM!\n");
					//explosion(dungeon, maxrows, maxcols, actives, monsters, megabomb->getPosX(), megabomb->getPosY(), i);
					explosion(*this, megabomb->getPosX(), megabomb->getPosY());
					dungeon[megabomb->getPosY()*maxcols + megabomb->getPosX()].extra = SPACE;
					removeSprite(projectile_sprites, maxrows, megabomb->getPosX(), megabomb->getPosY());

					// set i to pos because otherwise the iterator is destroyed
					pos = i;
					actives.erase(actives.begin() + pos);
					i--;
				}
				megabomb.reset();
			}
			else if (actives.at(i)->getItem() == "falling spike") {
				std::shared_ptr<FallingSpike> spike = std::dynamic_pointer_cast<FallingSpike>(actives.at(i));
				x = spike->getPosX();
				y = spike->getPosY();

				switch (spike->getSpeed()) {
				case 1:
					//	if tile below spike is the player, hit them
					if (dungeon[(y + 1)*maxcols + x].hero) {
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);

						dungeonText.push_back("A speed 1 spike smacked you on the head for " + to_string(spike->getDmg()) + " damage!\n");
						player.at(0).setHP(player.at(0).getHP() - spike->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							dungeon[py*maxcols + px].top = 'X';
						}
					}
					//	if tile below spike is not a wall
					else if (y + 1 < maxrows - 1) {
						moveSprite(spike_sprites, maxrows, x, y, 'd');

						spike->setPosY(y + 1);
						//actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(y + 1)*maxcols + x].traptile == SPIKE) {
							dungeon[(y + 1)*maxcols + x].overlap = true;
							dungeon[(y + 1)*maxcols + x].overlap_count++;
						}

						dungeon[(y + 1)*maxcols + x].traptile = SPIKE;
						dungeon[(y + 1)*maxcols + x].trap = true;
					}
					//	else disappear
					else {
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);
						// subtract one from the increment so as to not skip any spikes
						i--;
					}
					break;
				case 2:
					//	if player is in the way, hit them
					if (dungeon[(y + 1)*maxcols + x].hero || dungeon[(y + 2)*maxcols + x].hero) {
						removeSprite(spike_sprites, maxrows, x, y);
						
						pos = i;
						actives.erase(actives.begin() + pos);

						dungeonText.push_back("A speed 2 spike smacked you on the head for " + to_string(spike->getDmg()) + "!\n");
						player.at(0).setHP(player.at(0).getHP() - spike->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							dungeon[py*maxcols + px].top = 'X';
						}
						i--;
					}
					//	else if two tiles below spike are not walls
					else if (y + 2 < maxrows - 1) {
						moveSprite(spike_sprites, maxrows, x, y, 'D');

						spike->setPosY(y + 2);
						actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(y + 2)*maxcols + x].traptile == SPIKE) {
							dungeon[(y + 2)*maxcols + x].overlap = true;
							dungeon[(y + 2)*maxcols + x].overlap_count++;
						}

						dungeon[(y + 2)*maxcols + x].traptile = SPIKE;
						dungeon[(y + 2)*maxcols + x].trap = true;
					}
					//	else if there is only one tile of space underneath, set spike to bottom
					else if (y + 2 >= maxrows - 1 && y != maxrows - 2) {
						moveSprite(spike_sprites, maxrows, x, y, 'd');

						spike->setPosY(maxrows - 2);
						actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(y + 1)*maxcols + x].traptile == SPIKE) {
							dungeon[(y + 1)*maxcols + x].overlap = true;
							dungeon[(y + 1)*maxcols + x].overlap_count++;
						}

						dungeon[(maxrows - 2)*maxcols + x].traptile = SPIKE;
						dungeon[(maxrows - 2)*maxcols + x].trap = true;
					}
					//	else disappear
					else {
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);
						// subtract one from the increment so as to not skip any spikes
						i--;
					}
					break;
				case 3:
					//	if player is below, hit them
					if (dungeon[(y + 3)*maxcols + x].hero || dungeon[(y + 2)*maxcols + x].hero || dungeon[(y + 1)*maxcols + x].hero) {
						removeSprite(spike_sprites, maxrows, x, y);
						
						pos = i;
						actives.erase(actives.begin() + pos);

						dungeonText.push_back("A speed 3 spike smacked you on the head for " + to_string(spike->getDmg()) + "!\n");
						player.at(0).setHP(player.at(0).getHP() - spike->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							dungeon[py*maxcols + px].top = 'X';
						}
						i--;
					}
					//	if spike does not hit a wall
					else if (y + 3 < maxrows - 1) {
						moveSprite(spike_sprites, maxrows, x, y, 'V');

						spike->setPosY(y + 3);
						actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(y + 3)*maxcols + x].traptile == SPIKE) {
							dungeon[(y + 3)*maxcols + x].overlap = true;
							dungeon[(y + 3)*maxcols + x].overlap_count++;
						}

						dungeon[(y + 3)*maxcols + x].traptile = SPIKE;
						dungeon[(y + 3)*maxcols + x].trap = true;
					}
					//	if spike would overshoot, move it to the bottom
					else if (y + 3 >= maxrows - 1 && y != maxrows - 2) {
						// if spike is one above the bottom
						if (y == maxrows - 3) {
							moveSprite(spike_sprites, maxrows, x, y, 'd');
						}
						// else spike was two above the bottom
						else {
							moveSprite(spike_sprites, maxrows, x, y, 'D');
						}

						spike->setPosY(maxrows - 2);
						actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(maxrows - 2)*maxcols + x].traptile == SPIKE) {
							dungeon[(maxrows - 2)*maxcols + x].overlap = true;
							dungeon[(maxrows - 2)*maxcols + x].overlap_count++;
						}

						dungeon[(maxrows - 2)*maxcols + x].traptile = SPIKE;
						dungeon[(maxrows - 2)*maxcols + x].trap = true;
					}
					//	else disappear
					else {
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);
						// subtract one from the increment so as to not skip any spikes
						i--;
					}
					break;
				default:
					break;
				}

				// if there are more than two spikes overlapping, just reduce the overlap count
				if (dungeon[y*maxcols + x].overlap && dungeon[y*maxcols + x].overlap_count > 1) {
					dungeon[y*maxcols + x].overlap_count--;
				}
				// if there is only one overlap, set overlap to false, reduce count to zero,
				// but do not replace the previous tile with a space
				else if (dungeon[y*maxcols + x].overlap && dungeon[y*maxcols + x].overlap_count == 1) {
					dungeon[y*maxcols + x].overlap = false;
					dungeon[y*maxcols + x].overlap_count--;
				}
				// else if there is no overlap, then reset the tile to empty projectile space
				else {
					dungeon[y*maxcols + x].traptile = SPACE;
					dungeon[y*maxcols + x].trap = false;
				}

				//dungeon[y*maxcols + x].traptile = SPACE;
				//dungeon[y*maxcols + x].trap = false;
				spike.reset();
			}
			else if (actives.at(i)->getItem() == "spike trap") {
				std::shared_ptr<SpikeTrap> spiketrap = std::dynamic_pointer_cast<SpikeTrap>(actives.at(i));
				spiketrap->cycle(player.at(0));

				// assigns spike to traps vector for lighting purposes
				x = spiketrap->getPosX();
				y = spiketrap->getPosY();
				int pos = findTrap(x, y, getTraps());
				getTraps().at(pos) = spiketrap;

				spiketrap.reset();
			}
			else if (actives.at(i)->getItem() == "trigger spike trap") {
				std::shared_ptr<TriggerSpike> ts = std::dynamic_pointer_cast<TriggerSpike>(actives.at(i));
				ts->trigger(player.at(0));
				ts.reset();
			}
			else if (actives.at(i)->getItem() == "firebar") {
				moveFirebar(dungeon, maxrows, maxcols, actives, i);
			}
			else if (actives.at(i)->getItem() == "double firebar") {
				moveDoubleFirebar(dungeon, maxrows, maxcols, actives, i);
			}
			else if (actives.at(i)->getItem() == "Spring") {
				std::shared_ptr<Spring> spring = std::dynamic_pointer_cast<Spring>(actives.at(i));

				// only check if spring is a trigger type
				if (spring->isTrigger()) {
					if (spring->triggered()) {
						spring->trigger(*this, player.at(0));
					}
				}

				spring.reset();
			}
			else if (actives.at(i)->getItem() == "Turret") {
				std::shared_ptr<Turret> turret = std::dynamic_pointer_cast<Turret>(actives.at(i));
				turret->shoot(*this, player.at(0));
				turret.reset();
			}
			else if (actives.at(i)->getItem() == "Moving Block") {
				std::shared_ptr<MovingBlock> block = std::dynamic_pointer_cast<MovingBlock>(actives.at(i));
				block->move(*this, player.at(0));
				block.reset();
			}
			else if (actives.at(i)->getItem() == "door") {
				shared_ptr<Door> door = dynamic_pointer_cast<Door>(actives.at(i));
				x = door->getPosX();
				y = door->getPosY();

				bool enemy, hero;
				enemy = dungeon[y*maxcols + x].enemy;
				hero = dungeon[y*maxcols + x].hero;

				// if door is open, isn't being held, and nothing is in the way
				if (door->isOpen() && !door->isHeld() && !(enemy || hero) && (dungeon[y*maxcols + x].top == DOOR_H_OPEN || dungeon[y*maxcols + x].top == DOOR_V_OPEN)) {
					// close the door
					cocos2d::experimental::AudioEngine::play2d("Door_Closed.mp3", false, 1.0f);

					// replace sprite with closed door
					if (dungeon[y*maxcols + x].traptile == DOOR_H)
						addSprite(door_sprites, maxrows, x, y, -2, "Door_Horizontal_Closed_48x48.png");
					else
						addSprite(door_sprites, maxrows, x, y, -2, "Door_Vertical_Closed_48x48.png");

					// close the door
					dungeon[y*maxcols + x].top = (dungeon[y*maxcols + x].traptile == DOOR_H ? DOOR_H : DOOR_V);
					door->toggleOpen();
					door->toggleHeld();
				}
				// else if door is open and is being held
				else if (door->isOpen() && door->isHeld()) {
					// release hold on the door
					door->toggleHeld();
				}
				door.reset();
			}
		}
	}

	// cleans up any destroyed traps
	trapRemoval(*this);
}
void Dungeon::explosion(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int x, int y, int pos) { // bomb's coordinates
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int mx, my;

	// find any monsters caught in the blast
	for (unsigned i = 0; i < monsters.size(); i++) {
		mx = monsters.at(i)->getPosX();
		my = monsters.at(i)->getPosY();

		if (actives.at(pos)->getItem() == "Mega Bomb") {
			if (abs(mx - x) <= 2 && abs(my - y) <= 2) {
				monsters.at(i)->setHP(monsters.at(i)->getHP() - 20);
				//dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

				if (monsters.at(i)->getHP() <= 0) {
					monsterDeath(dungeon, maxrows, maxcols, actives, monsters, i);
				}
			}
		}
		// else is a regular bomb
		else if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			monsters.at(i)->setHP(monsters.at(i)->getHP() - 10);
			//dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

			if (monsters.at(i)->getHP() <= 0) {
				monsterDeath(dungeon, maxrows, maxcols, actives, monsters, i);
			}
		}
	}

	// check if player was hit or if there are any walls to destroy
	if (actives.at(pos)->getItem() == "Mega Bomb") {

		//	if player is caught in the explosion
		if (abs(px - x) <= 2 && abs(py - y) <= 2) {
			player.at(0).setHP(player.at(0).getHP() - 20);
			//dungeonText.push_back("That's gotta hurt!\n");
			if (player.at(0).getHP() <= 0)
				player.at(0).setDeath(actives.at(pos)->getItem());
		}

		//	destroy any nearby walls
		for (int j = y - 2; j < y + 3; j++) {
			for (int k = x - 2; k < x + 3; k++) {
				if (j != -1 && j != maxrows) { // boundary check
					if (dungeon[j*maxcols + k].top == WALL) {
						dungeon[j*maxcols + k].top = SPACE;
						dungeon[j*maxcols + k].bottom = SPACE;
						dungeon[j*maxcols + k].wall = false;

						// call remove sprite twice to remove top and bottom walls
						removeSprite(wall_sprites, maxrows, k, j);
						//removeSprite(wall_sprites, maxrows, k, j);
					}
					
					// destroy any gold in the way
					if (dungeon[j*maxcols + k].gold != 0) {
						dungeon[j*maxcols + k].gold = 0;

						removeSprite(money_sprites, maxrows, k, j);
					}
				}
			}
		}
	}
	// else is regular bomb
	else {

		//	if player is caught in the explosion
		if (abs(px - x) <= 1 && abs(py - y) <= 1) {
			player.at(0).setHP(player.at(0).getHP() - 10);
			//dungeonText.push_back("That's gotta hurt!\n");
			if (player.at(0).getHP() <= 0)
				player.at(0).setDeath(actives.at(pos)->getItem());
		}
		//	destroy any nearby walls
		for (int j = y - 1; j < y + 2; j++) {
			for (int k = x - 1; k < x + 2; k++) {
				if (dungeon[j*maxcols + k].top == WALL) {
					dungeon[j*maxcols + k].top = SPACE;
					dungeon[j*maxcols + k].bottom = SPACE;
					dungeon[j*maxcols + k].wall = false;

					// call remove sprite twice to remove top and bottom walls
					removeSprite(wall_sprites, maxrows, k, j);
					//removeSprite(wall_sprites, maxrows, k, j);
				}

				// destroy any gold
				if (dungeon[j*maxcols + k].gold != 0) {
					dungeon[j*maxcols + k].gold = 0;

					removeSprite(money_sprites, maxrows, k, j);
				}

				// destroy any destructible traps
				if (dungeon[j*maxcols + k].trap) {
					int pos = this->findTrap(k, j, this->getTraps());
					int pos2 = this->findTrap(k, j, this->getActives());

					if (pos != -1) {
						if (this->getTraps().at(pos)->isDestructible()) {
							dungeon[j*maxcols + k].trap = false;
							dungeon[j*maxcols + k].trap_name = "";
							queueRemoveSprite(k, j, this->getTraps().at(pos)->getSprite());
							this->getTraps().erase(this->getTraps().begin() + pos);
						}
					}
					if (pos2 != -1) {
						if (this->getActives().at(pos)->isDestructible()) {
							// push this trap into the trap removal vector for later
							// removing it now would cause an error since checkActives() is still iterating through
							m_trapIndexes.push_back(std::make_pair(k, j));
						}
					}
				}
			}
		}
	}
}
void Dungeon::explosion(Dungeon &dungeon, int x, int y) { // bomb's coordinates
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int mx, my;

	// find any monsters caught in the blast
	int pos = dungeon.findTrap(x, y, dungeon.getActives()); // finds the bomb
	for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		if (dungeon.getActives().at(pos)->getItem() == "Mega Bomb") {
			if (abs(mx - x) <= 2 && abs(my - y) <= 2) {
				dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - 20);
				//dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

				if (dungeon.getMonsters().at(i)->getHP() <= 0) {
					monsterDeath(dungeon.getDungeon(), maxrows, maxcols, dungeon.getActives(), dungeon.getMonsters(), i);
				}
			}
		}
		// else is a regular bomb
		else if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - 10);
			//dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

			if (dungeon.getMonsters().at(i)->getHP() <= 0) {
				monsterDeath(dungeon.getDungeon(), maxrows, maxcols, dungeon.getActives(), dungeon.getMonsters(), i);
			}
		}
	}

	// check if player was hit or if there are any walls to destroy
	if (dungeon.getActives().at(pos)->getItem() == "Mega Bomb") {

		//	if player is caught in the explosion
		if (abs(px - x) <= 2 && abs(py - y) <= 2) {
			player.at(0).setHP(player.at(0).getHP() - 20);
			//dungeonText.push_back("That's gotta hurt!\n");
			if (player.at(0).getHP() <= 0)
				player.at(0).setDeath(dungeon.getActives().at(pos)->getItem());
		}

		//	destroy any nearby walls
		for (int j = y - 2; j < y + 3; j++) {
			for (int k = x - 2; k < x + 3; k++) {
				if (j != -1 && j != maxrows && !(k == -1 && j <= 0) && !(k == maxcols && j >= maxrows - 1)) { // boundary check
					if (dungeon.getDungeon()[j*maxcols + k].top == WALL) {
						dungeon.getDungeon()[j*maxcols + k].top = SPACE;
						dungeon.getDungeon()[j*maxcols + k].bottom = SPACE;
						dungeon.getDungeon()[j*maxcols + k].wall = false;

						// call remove sprite twice to remove top and bottom walls
						removeSprite(wall_sprites, maxrows, k, j);
						//removeSprite(wall_sprites, maxrows, k, j);
					}

					// destroy any gold in the way
					if (dungeon.getDungeon()[j*maxcols + k].gold != 0) {
						dungeon.getDungeon()[j*maxcols + k].gold = 0;

						removeSprite(money_sprites, maxrows, k, j);
					}

					// destroy any destructible traps
					if (dungeon[j*maxcols + k].trap) {
						int pos = dungeon.findTrap(k, j, dungeon.getTraps());
						int pos2 = dungeon.findTrap(k, j, dungeon.getActives());

						if (pos != -1) {
							if (dungeon.getTraps().at(pos)->isDestructible()) {
								dungeon.getDungeon()[j*maxcols + k].trap = false;
								dungeon.getDungeon()[j*maxcols + k].trap_name = "";

								queueRemoveSprite(k, j, dungeon.getTraps().at(pos)->getSprite());
								dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
							}
						}
						if (pos2 != -1) {
							if (dungeon.getActives().at(pos)->isDestructible()) {
								// push this trap into the trap removal vector for later
								// removing it now would cause an error since checkActives() is still iterating through
								m_trapIndexes.push_back(std::make_pair(k, j));
							}
						}
					}
				}
			}
		}
	}
	// else is regular bomb
	else {

		//	if player is caught in the explosion
		if (abs(px - x) <= 1 && abs(py - y) <= 1) {
			player.at(0).setHP(player.at(0).getHP() - 10);
			//dungeonText.push_back("That's gotta hurt!\n");
			if (player.at(0).getHP() <= 0)
				player.at(0).setDeath(dungeon.getActives().at(pos)->getItem());
		}
		//	destroy any nearby walls
		for (int j = y - 1; j < y + 2; j++) {
			for (int k = x - 1; k < x + 2; k++) {
				if (dungeon.getDungeon()[j*maxcols + k].top == WALL) {
					dungeon.getDungeon()[j*maxcols + k].top = SPACE;
					dungeon.getDungeon()[j*maxcols + k].bottom = SPACE;
					dungeon.getDungeon()[j*maxcols + k].wall = false;

					// call remove sprite twice to remove top and bottom walls
					removeSprite(wall_sprites, maxrows, k, j);
					//removeSprite(wall_sprites, maxrows, k, j);
				}

				// destroy any gold
				if (dungeon.getDungeon()[j*maxcols + k].gold != 0) {
					dungeon.getDungeon()[j*maxcols + k].gold = 0;

					removeSprite(money_sprites, maxrows, k, j);
				}

				// destroy any destructible traps
				if (dungeon[j*maxcols + k].trap) {
					int pos = dungeon.findTrap(k, j, dungeon.getTraps());
					int pos2 = dungeon.findTrap(k, j, dungeon.getActives());

					if (pos != -1) {
						if (dungeon.getTraps().at(pos)->isDestructible()) {
							dungeon.getDungeon()[j*maxcols + k].trap = false;
							dungeon.getDungeon()[j*maxcols + k].trap_name = "";

							queueRemoveSprite(k, j, dungeon.getTraps().at(pos)->getSprite());
							dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
						}
					}
					if (pos2 != -1) {
						if (dungeon.getActives().at(pos)->isDestructible()) {
							// push this trap into the trap removal vector for later
							// removing it now would cause an error since checkActives() is still iterating through
							m_trapIndexes.push_back(std::make_pair(k, j));
						}
					}
				}
			}
		}
	}
}

void Dungeon::trapRemoval(Dungeon &dungeon) {
	// removes any traps that were just destroyed after checkActive

	if (m_trapIndexes.empty())
		return;

	int pos;
	for (auto &it : m_trapIndexes) {
		pos = dungeon.findTrap(it.first, it.second, dungeon.getActives());
		if (pos != -1) {
			dungeon.getActives().erase(dungeon.getActives().begin() + pos);
		}
	}
}
int Dungeon::findTrap(int x, int y, std::vector<std::shared_ptr<Objects>> &actives) {
	for (int i = 0; i < actives.size(); i++) {
		if (actives.at(i)->getPosX() == x && actives.at(i)->getPosY() == y) {
			return i;
		}
	}

	return -1;
}
void Dungeon::trapEncounter(Dungeon &dungeon, int x, int y) { // trap's coordinates
	int i = findTrap(x, y, dungeon.getTraps());

	if (i == -1)
		return;

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	string trap = dungeon.getTraps().at(i)->getItem();

	if (trap == "falling spike") {
		shared_ptr<FallingSpike> spike = dynamic_pointer_cast<FallingSpike>(dungeon.getActives().at(i));

		cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

		// remove the spike sprite
		removeSprite(spike_sprites, rows, x, y);

		dungeonText.push_back("A falling spike hit you for " + to_string(spike->getDmg()) + "!\n");
		player.at(0).setHP(player.at(0).getHP() - spike->getDmg());

		dungeon.getActives().erase(dungeon.getActives().begin() + i);
		spike.reset();
	}
	else if (trap == "spike trap") {
		shared_ptr<SpikeTrap> spiketrap = dynamic_pointer_cast<SpikeTrap>(dungeon.getTraps().at(i));

		if (dungeon[y*cols + x].traptile == SPIKETRAP_ACTIVE) {
			//dungeonText.push_back("You step on a spike trap and take " + to_string(spiketrap->getDmg()) + " damage!\n");
			player.at(0).setHP(player.at(0).getHP() - spiketrap->getDmg());
		}
		spiketrap.reset();
	}
	else if (trap == "trigger spike trap") {
		shared_ptr<TriggerSpike> ts = dynamic_pointer_cast<TriggerSpike>(dungeon.getTraps().at(i));

		if (dungeon[y*cols + x].traptile == TRIGGER_SPIKE_ACTIVE) {
			//dungeonText.push_back("You step on a spike trap and take " + to_string(ts->getDmg()) + " damage!\n");
			player.at(0).setHP(player.at(0).getHP() - ts->getDmg());
		}

		ts.reset();
	}
	else if (trap == "pit") {
		shared_ptr<Pit> pit = dynamic_pointer_cast<Pit>(dungeon.getTraps().at(i));
		pit->fall(player.at(0)); // kills player
		pit.reset();

		// death animation
		deathFade(player.at(0).getSprite());
	}
	else if (trap == "puddle") {
		shared_ptr<Puddle> puddle = dynamic_pointer_cast<Puddle>(dungeon.getTraps().at(i));
		puddle->slip(player.at(0));
		puddle.reset();

		dungeon[y*cols + x].trap = false;
		dungeon[y*cols + x].trap_name = "";
		//actives.erase(actives.begin() + i);
		queueRemoveSprite(x, y, dungeon.getTraps().at(i)->getSprite());
		dungeon.getTraps().erase(dungeon.getTraps().begin() + i);
		//removeSprite(trap_sprites, maxrows, x, y);
	}
	else if (trap == "Lava") {
		shared_ptr<Lava> lava = dynamic_pointer_cast<Lava>(dungeon.getTraps().at(i));
		lava->burn(player.at(0));
		lava.reset();
	}
	else if (trap == "Spring") {
		std::shared_ptr<Spring> spring = std::dynamic_pointer_cast<Spring>(dungeon.getTraps().at(i));
		spring->trigger(*this, player.at(0));
		spring.reset();
	}

}
void Dungeon::moveFirebar(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, int pos) {
	std::shared_ptr<Firebar> firebar = std::dynamic_pointer_cast<Firebar>(actives.at(pos));
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	int mx = actives.at(pos)->getPosX();
	int my = actives.at(pos)->getPosY();

	// reset projectiles to visible
	firebar->getInner()->setVisible(true);
	firebar->getOuter()->setVisible(true);

	/* 8 1 2 
	*  7 X 3
	*  6 5 4
	*  moves firebar -to- position getAngle(), it is not where it currently is
	*  HOWEVER, the firebar's sprite begins at the sprite's current angle, so the sprite is at the angle it currently is
	*/

	if (firebar->isClockwise()) {
		switch (firebar->getAngle()) {
		case 1:
			queueMoveSprite(firebar->getInner(), 'r');
			queueMoveSprite(firebar->getOuter(), 'R');
			
			firebar->setFirePosition('r');

			if (my == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 2:
			queueMoveSprite(firebar->getInner(), 'r');
			queueMoveSprite(firebar->getOuter(), 'R');
			
			firebar->setFirePosition('r');
			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 3:
			queueMoveSprite(firebar->getInner(), 'd');
			queueMoveSprite(firebar->getOuter(), 'D');
			
			firebar->setFirePosition('d');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 4:
			queueMoveSprite(firebar->getInner(), 'd');
			queueMoveSprite(firebar->getOuter(), 'D');

			firebar->setFirePosition('d');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 5:
			queueMoveSprite(firebar->getInner(), 'l');
			queueMoveSprite(firebar->getOuter(), 'L');

			firebar->setFirePosition('l');
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 6:
			queueMoveSprite(firebar->getInner(), 'l');
			queueMoveSprite(firebar->getOuter(), 'L');

			firebar->setFirePosition('l');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (my == maxrows - 2 || mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 7:
			queueMoveSprite(firebar->getInner(), 'u');
			queueMoveSprite(firebar->getOuter(), 'U');

			firebar->setFirePosition('u');

			// if firebar is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 8:
			queueMoveSprite(firebar->getInner(), 'u');
			queueMoveSprite(firebar->getOuter(), 'U');

			firebar->setFirePosition('u');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		default:
			break;
		}

		if (firebar->getAngle() == 8)
			firebar->setAngle(1);
		else
			firebar->setAngle(firebar->getAngle() + 1);
	}
	// else counterclockwise
	else {
		switch (firebar->getAngle()) {
		case 1:
			queueMoveSprite(firebar->getInner(), 'l');
			queueMoveSprite(firebar->getOuter(), 'L');

			firebar->setFirePosition('l');
			if (my == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 2:
			queueMoveSprite(firebar->getInner(), 'u');
			queueMoveSprite(firebar->getOuter(), 'U');

			firebar->setFirePosition('u');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 3:
			queueMoveSprite(firebar->getInner(), 'u');
			queueMoveSprite(firebar->getOuter(), 'U');

			firebar->setFirePosition('u');

			// if firebar is on the right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 4:
			queueMoveSprite(firebar->getInner(), 'r');
			queueMoveSprite(firebar->getOuter(), 'R');

			firebar->setFirePosition('r');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 5:
			queueMoveSprite(firebar->getInner(), 'r');
			queueMoveSprite(firebar->getOuter(), 'R');

			firebar->setFirePosition('r');
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 6:
			queueMoveSprite(firebar->getInner(), 'd');
			queueMoveSprite(firebar->getOuter(), 'D');

			firebar->setFirePosition('d');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == maxrows - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 7:
			queueMoveSprite(firebar->getInner(), 'd');
			queueMoveSprite(firebar->getOuter(), 'D');

			firebar->setFirePosition('d');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		case 8:
			queueMoveSprite(firebar->getInner(), 'l');
			queueMoveSprite(firebar->getOuter(), 'L');

			firebar->setFirePosition('l');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			break;
		default: break;
		}

		if (firebar->getAngle() == 1)
			firebar->setAngle(8);
		else
			firebar->setAngle(firebar->getAngle() - 1);
	}

	if (firebar->playerWasHit(player.at(0))) {
		firebar->burn(player.at(0));
	}

	firebar.reset();
}
void Dungeon::moveDoubleFirebar(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, int pos) {
	std::shared_ptr<DoubleFirebar> firebar = std::dynamic_pointer_cast<DoubleFirebar>(actives.at(pos));
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	int mx = actives.at(pos)->getPosX();
	int my = actives.at(pos)->getPosY();

	// reset projectiles to visible
	firebar->setSpriteVisibility(true);

	/* 8 1 2
	*  7 X 3
	*  6 5 4
	*  moves firebar -to- position getAngle(), it is not where it currently is
	*  
	*/

	if (firebar->isClockwise()) {
		switch (firebar->getAngle()) {
		case 1: {
			queueMoveSprite(firebar->getInner(), 'r');
			queueMoveSprite(firebar->getOuter(), 'R');
			firebar->setFirePosition('r');

			if (my == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			// opposite fire buddies
			queueMoveSprite(firebar->getInnerMirror(), 'l');
			queueMoveSprite(firebar->getOuterMirror(), 'L');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}

		}
			break;
		case 2: {
			queueMoveSprite(firebar->getInner(), 'r');
			queueMoveSprite(firebar->getOuter(), 'R');

			firebar->setFirePosition('r');
			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'l');
			queueMoveSprite(firebar->getOuterMirror(), 'L');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (my == maxrows - 2 || mx == 1) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 3: {
			queueMoveSprite(firebar->getInner(), 'd');
			queueMoveSprite(firebar->getOuter(), 'D');

			firebar->setFirePosition('d');
			// if firebar is on the left or right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'u');
			queueMoveSprite(firebar->getOuterMirror(), 'U');

			// if firebar is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 4: {
			queueMoveSprite(firebar->getInner(), 'd');
			queueMoveSprite(firebar->getOuter(), 'D');

			firebar->setFirePosition('d');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'u');
			queueMoveSprite(firebar->getOuterMirror(), 'U');
			
			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 5: {
			queueMoveSprite(firebar->getInner(), 'l');
			queueMoveSprite(firebar->getOuter(), 'L');

			firebar->setFirePosition('l');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}
			// opposite fire
			queueMoveSprite(firebar->getInnerMirror(), 'r');
			queueMoveSprite(firebar->getOuterMirror(), 'R');

			if (my == 1) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 6: {
			queueMoveSprite(firebar->getInner(), 'l');
			queueMoveSprite(firebar->getOuter(), 'L');
			firebar->setFirePosition('l');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (my == maxrows - 2 || mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'r');
			queueMoveSprite(firebar->getOuterMirror(), 'R');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 7: {
			queueMoveSprite(firebar->getInner(), 'u');
			queueMoveSprite(firebar->getOuter(), 'U');
			firebar->setFirePosition('u');

			// if firebar is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'd');
			queueMoveSprite(firebar->getOuterMirror(), 'D');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 8: {
			queueMoveSprite(firebar->getInner(), 'u');
			queueMoveSprite(firebar->getOuter(), 'U');
			firebar->setFirePosition('u');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'd');
			queueMoveSprite(firebar->getOuterMirror(), 'D');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		default: break;
		}

		if (firebar->getAngle() == 8)
			firebar->setAngle(1);
		else
			firebar->setAngle(firebar->getAngle() + 1);
	}
	// else counterclockwise
	else {
		switch (firebar->getAngle()) {
		case 1: {
			queueMoveSprite(firebar->getInner(), 'l');
			queueMoveSprite(firebar->getOuter(), 'L');
			firebar->setFirePosition('l');

			if (my == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'r');
			queueMoveSprite(firebar->getOuterMirror(), 'R');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 2: {
			queueMoveSprite(firebar->getInner(), 'u');
			queueMoveSprite(firebar->getOuter(), 'U');
			firebar->setFirePosition('u');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'd');
			queueMoveSprite(firebar->getOuterMirror(), 'D');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == maxrows - 2) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 3: {
			queueMoveSprite(firebar->getInner(), 'u');
			queueMoveSprite(firebar->getOuter(), 'U');
			firebar->setFirePosition('u');

			// if firebar is on the right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'd');
			queueMoveSprite(firebar->getOuterMirror(), 'D');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 4: {
			queueMoveSprite(firebar->getInner(), 'r');
			queueMoveSprite(firebar->getOuter(), 'R');
			firebar->setFirePosition('r');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'l');
			queueMoveSprite(firebar->getOuterMirror(), 'L');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 5: {
			queueMoveSprite(firebar->getInner(), 'r');
			queueMoveSprite(firebar->getOuter(), 'R');
			firebar->setFirePosition('r');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'l');
			queueMoveSprite(firebar->getOuterMirror(), 'L');


			if (my == 1) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 6: {
			queueMoveSprite(firebar->getInner(), 'd');
			queueMoveSprite(firebar->getOuter(), 'D');
			firebar->setFirePosition('d');

			// if firebar is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == maxrows - 2) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'u');
			queueMoveSprite(firebar->getOuterMirror(), 'U');

			// if firebar is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols - 2) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 7: {
			queueMoveSprite(firebar->getInner(), 'd');
			queueMoveSprite(firebar->getOuter(), 'D');
			firebar->setFirePosition('d');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'u');
			queueMoveSprite(firebar->getOuterMirror(), 'U');

			// if firebar is on the right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		case 8: {
			queueMoveSprite(firebar->getInner(), 'l');
			queueMoveSprite(firebar->getOuter(), 'L');
			firebar->setFirePosition('l');

			// if firebar is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				firebar->getInner()->setVisible(false);
				firebar->getOuter()->setVisible(false);
			}

			// opposite
			queueMoveSprite(firebar->getInnerMirror(), 'r');
			queueMoveSprite(firebar->getOuterMirror(), 'R');

			// if firebar is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				firebar->getInnerMirror()->setVisible(false);
				firebar->getOuterMirror()->setVisible(false);
			}
		}
			break;
		default: break;
		}

		if (firebar->getAngle() == 1)
			firebar->setAngle(8);
		else
			firebar->setAngle(firebar->getAngle() - 1);
	}

	if (firebar->playerWasHit(player.at(0))) {
		firebar->burn(player.at(0));
	}

	firebar.reset();
}
void Dungeon::openDoor(Dungeon &dungeon, int x, int y) {
	int i = findTrap(x, y, dungeon.getActives());

	if (i == -1)
		return;

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	shared_ptr<Door> door = dynamic_pointer_cast<Door>(dungeon.getActives().at(i));

	// if the door isn't locked, open it
	if (!door->isLocked()) {
		// door opening sound
		cocos2d::experimental::AudioEngine::play2d("Door_Opened.mp3", false, 1.0f);

		// remove door sprite
		removeSprite(door_sprites, rows, x, y);

		door->toggleOpen();
		dungeon.getDungeon()[y*cols + x].top = (dungeon.getDungeon()[y*cols + x].top == DOOR_H ? DOOR_H_OPEN : DOOR_V_OPEN);
	}

	door.reset();
}

void Dungeon::monsterTrapEncounter(Dungeon &dungeon) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	int x, y;
	bool trap;
	std::string trap_name;
	//std::string monster;
	
	for (int i = 0; i < dungeon.getMonsters().size(); i++) {
		x = dungeon.getMonsters().at(i)->getPosX();
		y = dungeon.getMonsters().at(i)->getPosY();
		trap = dungeon.getDungeon()[y*cols + x].trap;
		trap_name = dungeon.getDungeon()[y*cols + x].trap_name;
		//monster = dungeon.getMonsters().at(i)->getName();

		if (trap) {
			if (trap_name == PIT) {
				if (!dungeon.getMonsters().at(i)->isFlying()) {
					// play falling sound effect
					playMonsterDeathByPit(player.at(0), *dungeon.getMonsters().at(i));

					// death animation
					auto scale = cocos2d::ScaleTo::create(0.5, 0);
					auto fade = cocos2d::FadeOut::create(0.5);
					dungeon.getMonsters().at(i)->getSprite()->runAction(scale);
					dungeon.getMonsters().at(i)->getSprite()->runAction(fade);

					destroyMonster(dungeon.getDungeon(), rows, cols, dungeon.getMonsters(), i);
				}
			}
			else if (trap_name == SPRING) {
				if (!dungeon.getMonsters().at(i)->isFlying()) {
					std::shared_ptr<Spring> spring = std::dynamic_pointer_cast<Spring>(dungeon.getTraps().at(dungeon.findTrap(x, y, dungeon.getTraps())));
					spring->trigger(*this, *dungeon.getMonsters().at(i));
					spring.reset();
				}
			}
		}
	}
}
void Dungeon::singleMonsterTrapEncounter(Dungeon &dungeon, int pos) {
	int rows = dungeon.getRows();
	int cols = dungeon.getCols();
	int x, y;
	bool trap;
	std::string trap_name;
	
	x = dungeon.getMonsters().at(pos)->getPosX();
	y = dungeon.getMonsters().at(pos)->getPosY();
	trap = dungeon.getDungeon()[y*cols + x].trap;
	trap_name = dungeon.getDungeon()[y*cols + x].trap_name;

	if (trap) {
		if (trap_name == PIT) {
			if (!dungeon.getMonsters().at(pos)->isFlying()) {
				// play falling sound effect
				playMonsterDeathByPit(player.at(0), *dungeon.getMonsters().at(pos));

				// death animation
				auto scale = cocos2d::ScaleTo::create(0.5, 0);
				auto fade = cocos2d::FadeOut::create(0.5);
				dungeon.getMonsters().at(pos)->getSprite()->runAction(scale);
				dungeon.getMonsters().at(pos)->getSprite()->runAction(fade);

				destroyMonster(dungeon.getDungeon(), rows, cols, dungeon.getMonsters(), pos);
			}
		}
		else if (trap_name == SPRING) {
			if (!dungeon.getMonsters().at(pos)->isFlying()) {
				std::shared_ptr<Spring> spring = std::dynamic_pointer_cast<Spring>(dungeon.getTraps().at(dungeon.findTrap(x, y, dungeon.getTraps())));
				spring->trigger(*this, *dungeon.getMonsters().at(pos));
				spring.reset();
			}
		}
	}
}
void Dungeon::destroyMonster(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Monster>> &monsters, int pos) {
	// destroyMonster is used to eliminate monsters without having them drop anything

	int x = monsters.at(pos)->getPosX();
	int y = monsters.at(pos)->getPosY();

	dungeon[y*maxcols + x].top = SPACE;
	dungeon[y*maxcols + x].enemy = false;

	queueRemoveSprite(x, y, monsters);
	monsters.erase(monsters.begin() + pos);
}
void Dungeon::monsterDeath(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int pos) { // monster's coordinates
	std::string monster = monsters.at(pos)->getName();
	int x = monsters.at(pos)->getPosX();
	int y = monsters.at(pos)->getPosY();
	std::string image;
	bool item = dungeon[y*maxcols + x].item;

	// these are up here because of itemHash() changing the x and y values
	dungeon[y*maxcols + x].top = SPACE;
	//enemyOverlap(dungeon, maxrows, maxcols, x, y);
	dungeon[y*maxcols + x].enemy = false;

	// remove the monster sprite from the game
	//removeSprite(monster_sprites, maxrows, x, y);
	queueRemoveSprite(x, y, monsters);

	if (monster == "bombee") {
		shared_ptr<Bombee> bombee = dynamic_pointer_cast<Bombee>(monsters.at(pos));
		dungeon[y*maxcols + x].extra = BOMBEE_ACTIVE;

		MegaBomb megabomb;
		megabomb.lightBomb();
		megabomb.setPosX(bombee->getPosX());
		megabomb.setPosY(bombee->getPosY());

		actives.emplace_back(new MegaBomb(megabomb));
		addSprite(projectile_sprites, maxrows, x, y, -1, "Bomb_48x48.png");

		//dungeonText.push_back("*beep*\n");
		bombee.reset();
	}
	else {
		//dungeonText.push_back("The " + monster + " was slain.\n");

		if (monster == "goblin") {
			// 20% drop chance
			if (randInt(10) + 1 > 8) {
				if (item) {
					itemHash(dungeon, maxrows, maxcols, x, y);
				}
				dungeon[y*maxcols + x].item_name = BONE_AXE;
				dungeon[y*maxcols + x].item = true;
				addSprite(item_sprites, maxrows, x, y, -1, "Bone_Axe_48x48.png");
			}

			cocos2d::experimental::AudioEngine::play2d("Coin_Drop3.mp3", false, 1.0f);
			dungeon[y*maxcols + x].gold += 3;
			addGoldSprite(dungeon, maxrows, maxcols, x, y);
		}
		if (monster == "seeker") {
			cocos2d::experimental::AudioEngine::play2d("Coin_Drop1.mp3", false, 1.0f);
			dungeon[y*maxcols + x].gold += 1;
			addGoldSprite(dungeon, maxrows, maxcols, x, y);
		}
		if (monster == "roundabout") {
			cocos2d::experimental::AudioEngine::play2d("Coin_Drop2.mp3", false, 1.0f);
			dungeon[y*maxcols + x].gold += 2;
			addGoldSprite(dungeon, maxrows, maxcols, x, y);
		}
		else if (monster == "wanderer") {
			// 80% drop chance
			if (randInt(5) + 1 > 1) {
				if (item) {
					itemHash(dungeon, maxrows, maxcols, x, y);
				}
				dungeon[y*maxcols + x].item_name = HEART_POD;
				dungeon[y*maxcols + x].item = true;
				addSprite(item_sprites, maxrows, x, y, -1, "Heart_Pod_48x48.png");
			}

			cocos2d::experimental::AudioEngine::play2d("Coin_Drop1.mp3", false, 1.0f);
			dungeon[y*maxcols + x].gold += 1;
			addGoldSprite(dungeon, maxrows, maxcols, x, y);
		}
		else if (monster == "archer") {
			// 15% drop chance
			if (1 + randInt(20) > 17) {
				if (item) {
					itemHash(dungeon, maxrows, maxcols, x, y);
				}
				dungeon[y*maxcols + x].item_name = BRONZE_DAGGER;
				dungeon[y*maxcols + x].item = true;
				addSprite(item_sprites, maxrows, x, y, -1, "Bronze_Dagger_48x48.png");
			}
			cocos2d::experimental::AudioEngine::play2d("Coin_Drop3.mp3", false, 1.0f);
			dungeon[y*maxcols + x].gold += 3;
			addGoldSprite(dungeon, maxrows, maxcols, x, y);
		}
		else if (monster == "zapper") {
			for (int i = y - 1; i < y + 2; i++) {
				for (int j = x - 1; j < x + 2; j++) {
					dungeon[i*maxcols + j].projectile = SPACE;
					removeAssociatedSprite(zapper_sprites, maxrows, x, y, j, i);
				}
			}
			cocos2d::experimental::AudioEngine::play2d("Coin_Drop2.mp3", false, 1.0f);
			dungeon[y*maxcols + x].gold += 2;
			addGoldSprite(dungeon, maxrows, maxcols, x, y);
		}
		else if (monster == "spinner") {
			for (int i = y - 2; i < y + 3; i++) {
				for (int j = x - 2; j < x + 3; j++) {
					if (i*maxcols + j >= 0 && i*maxcols + j <= (maxrows-1)*(maxcols-1)) {
						dungeon[i*maxcols + j].projectile = SPACE;
						removeAssociatedSprite(spinner_sprites, maxrows, x, y, j, i);
					}
				}
			}
			cocos2d::experimental::AudioEngine::play2d("Coin_Drop2.mp3", false, 1.0f);
			dungeon[y*maxcols + x].gold += 2;
			addGoldSprite(dungeon, maxrows, maxcols, x, y);
		}
		else if (monster == "mounted knight") {
			// 10% drop chance
			if (randInt(10) + 1 == 10) {
				if (item) {
					itemHash(dungeon, maxrows, maxcols, x, y);
				}
				dungeon[y*maxcols + x].item_name = IRON_LANCE;
				dungeon[y*maxcols + x].item = true;
				addSprite(item_sprites, maxrows, x, y, -1, "Iron_Lance_48x48.png");
			}
			cocos2d::experimental::AudioEngine::play2d("Coin_Drop5.mp3", false, 1.0f);
			dungeon[y*maxcols + x].gold += 5;
			addGoldSprite(dungeon, maxrows, maxcols, x, y);
		}
	}
	//dungeon[y*maxcols + x].top = SPACE;
	//dungeon[y*maxcols + x].enemy = false;

	monsters.erase(monsters.begin() + pos);
}

void Dungeon::enemyOverlap(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y) {
	// if there wasn't any overlap, mark the tile as no enemy
	if (!dungeon[y*maxcols + x].enemy_overlap) {
		dungeon[y*maxcols + x].enemy = false;
		if (dungeon[y*maxcols + x].upper == SMASHER)
			dungeon[y*maxcols + x].upper = SPACE;
		else
			dungeon[y*maxcols + x].top = SPACE;
		return;
	}

	// if there was more than one overlap, just decrease the overlap count
	if (dungeon[y*maxcols + x].enemy_overlap && dungeon[y*maxcols + x].enemy_overlap_count > 1) {
		dungeon[y*maxcols + x].enemy_overlap_count--;
	}
	// if there is only one overlap, remove overlap and reduce count, but do not
	// reset the tile to empty space
	else if (dungeon[y*maxcols + x].enemy_overlap && dungeon[y*maxcols + x].enemy_overlap_count == 1) {
		dungeon[y*maxcols + x].enemy_overlap = false;
		dungeon[y*maxcols + x].enemy_overlap_count--;
	}
}
void Dungeon::itemHash(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int &x, int &y) {
	bool item = dungeon[y*maxcols + x].item;
	bool wall = dungeon[y*maxcols + x].wall;

	// if current tile has no item and the position is valid, return to set item here
	if (!(item || wall)) {
		return;
	}

	int n = -1 + randInt(3);
	int m = -1 + randInt(3);
	
	// while hash draws out of bounds, find a new position
	while (dungeon[(y + m)*maxcols + (x + n)].top == UNBREAKABLE_WALL) {
		n = -1 + randInt(3);
		m = -1 + randInt(3);
	}
	
	// hash until we find a valid spot
	itemHash(dungeon, maxrows, maxcols, x += n, y += m);
}

bool Dungeon::monsterHash(Dungeon &dungeon, int &x, int &y, bool &switched, char move) {
	// let starting x and y be the monster's initial position

	int rows = dungeon.getRows();
	int cols = dungeon.getCols();

	bool enemy = dungeon.getDungeon()[y*cols + x].enemy;
	bool wall = dungeon.getDungeon()[y*cols + x].wall;
	bool hero = dungeon.getDungeon()[y*cols + x].hero;

	// if current tile is valid, return to set monster here
	if (!(wall || enemy || hero)) {
		return true;
	}

	switch (move) {
		// 1. if next tile is a wall then switch direction (one time)

		// 2. else if a switch has already been made and a wall is hit, return false

		// 3. otherwise keep looking in the same path

	case 'l':
		if (dungeon.getDungeon()[y*cols + x - 1].wall && !switched) {
			if (dungeon.getDungeon()[(y - 1)*cols + x].wall && dungeon.getDungeon()[(y + 1)*cols + x].wall) {
				return false;
			}
			else if (dungeon.getDungeon()[(y - 1)*cols + x].wall && !dungeon.getDungeon()[(y + 1)*cols + x].wall) {
				move = 'd';
			}
			else if (dungeon.getDungeon()[(y + 1)*cols + x].wall && !dungeon.getDungeon()[(y - 1)*cols + x].wall) {
				move = 'u';
			}
			else {
				move = (randInt(2) == 0 ? 'u' : 'd');
			}
			switched = true;
		}
		else if (dungeon.getDungeon()[y*cols + x - 1].wall && switched) {
			return false;
		}
		break;
	case 'r':
		if (dungeon.getDungeon()[y*cols + x + 1].wall && !switched) {
			if (dungeon.getDungeon()[(y - 1)*cols + x].wall && dungeon.getDungeon()[(y + 1)*cols + x].wall) {
				return false;
			}
			else if (dungeon.getDungeon()[(y - 1)*cols + x].wall && !dungeon.getDungeon()[(y + 1)*cols + x].wall) {
				move = 'd';
			}
			else if (dungeon.getDungeon()[(y + 1)*cols + x].wall && !dungeon.getDungeon()[(y - 1)*cols + x].wall) {
				move = 'u';
			}
			else {
				move = (randInt(2) == 0 ? 'u' : 'd');
			}
			switched = true;
		}
		else if (dungeon.getDungeon()[y*cols + x + 1].wall && switched) {
			return false;
		}
		break;
	case 'u':
		if (dungeon.getDungeon()[(y - 1)*cols + x].wall && !switched) {
			if (dungeon.getDungeon()[y*cols + x - 1].wall && dungeon.getDungeon()[y*cols + x + 1].wall) {
				return false;
			}
			else if (dungeon.getDungeon()[y*cols + x - 1].wall && !dungeon.getDungeon()[y*cols + x + 1].wall) {
				move = 'r';
			}
			else if (dungeon.getDungeon()[y*cols + x + 1].wall && !dungeon.getDungeon()[y*cols + x - 1].wall) {
				move = 'l';
			}
			else {
				move = (randInt(2) == 0 ? 'l' : 'r');
			}
			switched = true;
		}
		else if (dungeon.getDungeon()[(y - 1)*cols + x].wall && switched) {
			return false;
		}
		break;
	case 'd':
		if (dungeon.getDungeon()[(y + 1)*cols + x].wall && !switched) {
			if (dungeon.getDungeon()[y*cols + x - 1].wall && dungeon.getDungeon()[y*cols + x + 1].wall) {
				return false;
			}
			else if (dungeon.getDungeon()[y*cols + x - 1].wall && !dungeon.getDungeon()[y*cols + x + 1].wall) {
				move = 'r';
			}
			else if (dungeon.getDungeon()[y*cols + x + 1].wall && !dungeon.getDungeon()[y*cols + x - 1].wall) {
				move = 'l';
			}
			else {
				move = (randInt(2) == 0 ? 'l' : 'r');
			}
			switched = true;
		}
		else if (dungeon.getDungeon()[(y + 1)*cols + x].wall && switched) {
			return false;
		}
		break;
	}

	if (move == 'l') return monsterHash(dungeon, --x, y, switched, move);
	if (move == 'r') return monsterHash(dungeon, ++x, y, switched, move);
	if (move == 'u') return monsterHash(dungeon, x, --y, switched, move);
	if (move == 'd') return monsterHash(dungeon, x, ++y, switched, move);

	return false;
}
void Dungeon::pushMonster(Dungeon &dungeon, int &mx, int &my, char move, int cx, int cy) {
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	bool b = false;
	int ox = mx;
	int oy = my;

	for (int i = 0; i < dungeon.getMonsters().size(); i++) {
		if (dungeon.getMonsters().at(i)->getPosX() == mx && dungeon.getMonsters().at(i)->getPosY() == my) {
			switch (move) {
			case 'l':
			case 'r':
			case 'u':
			case 'd': {
				if (monsterHash(dungeon, mx, my, b, move)) {
					dungeon[oy*maxcols + ox].top = SPACE;
					dungeon[oy*maxcols + ox].enemy = false;
					//dungeon[my*maxcols + mx].top = ARCHER;
					dungeon[my*maxcols + mx].enemy = true;
					dungeon.getMonsters()[i]->setPosX(mx);
					dungeon.getMonsters()[i]->setPosY(my);


					queueMoveSprite(dungeon.getMonsters().at(i)->getSprite(), maxrows, mx, my, 0.05f);
				}
				else {
					monsterDeath(dungeon.getDungeon(), maxrows, maxcols, dungeon.getActives(), dungeon.getMonsters(), i);
				}
				break;
			}
					  //	CUSTOM MOVEMENT CASE in X direction
			case 'X': {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;

				//dungeon[my*maxcols + cx].top = ARCHER;
				dungeon[my*maxcols + cx].enemy = true;

				dungeon.getMonsters()[i]->setPosX(cx);
				queueMoveSprite(dungeon.getMonsters().at(i)->getSprite(), maxrows, cx, my);
				break;
			}
					  //	CUSTOM MOVEMENT CASE in Y direction
			case 'Y': {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;

				//dungeon[(cy)*maxcols + mx].top = ARCHER;
				dungeon[(cy)*maxcols + mx].enemy = true;

				dungeon.getMonsters()[i]->setPosY(cy);
				queueMoveSprite(dungeon.getMonsters().at(i)->getSprite(), maxrows, mx, cy);
				break;
			}
			}
			return;
		}
	}
}

// Enemies that use chasing algorithm
bool Dungeon::attemptChase(std::vector<_Tile> &dungeon, int maxcols, int pos, int &shortest, int smelldist, int origdist, int x, int y, \
	char &first_move, char &optimal_move, char prev) {
	
	// if enemy is immediately adjacent to the player, return to attack
	if (smelldist == origdist && \
		(dungeon[(y - 1)*maxcols + x].hero || dungeon[(y + 1)*maxcols + x].hero || dungeon[y*maxcols + x - 1].hero || dungeon[y*maxcols + x + 1].hero)) {
		dungeon[y*maxcols + x].marked = false;
		return true;
	}
	
	// mark the current tile as visited
	dungeon[y*maxcols + x].marked = true;

	// player was not found within origdist # of steps OR took more steps than the current shortest path found
	if (smelldist < 0 || smelldist < shortest) {
		dungeon[y*maxcols + x].marked = false;
		return false;
	}

	if (dungeon[y*maxcols + x].hero) {
		if (smelldist > shortest) {
			shortest = smelldist;
			optimal_move = first_move;
		}
		else if (smelldist == shortest) {
			// randomly select optimal path if multiple optimal paths found
			if (randInt(2) == 1)
				optimal_move = first_move;
		}
		dungeon[y*maxcols + x].marked = false;
		return true; // if player is found, tell goblin to advance
	}

	if (!dungeon[y*maxcols + x+1].marked && prev != 'l' && (!dungeon[y*maxcols + x + 1].wall || dungeon[y*maxcols + x + 1].hero)) { //(dungeon[y*maxcols + x+1].top != WALL && dungeon[y*maxcols + x+1].top != UNBREAKABLE_WALL && dungeon[y*maxcols + x + 1].top != LAVA) && !dungeon[y*maxcols + x+1].enemy) {
		if (smelldist == origdist)
			first_move = 'r';
		attemptChase(dungeon, maxcols, pos, shortest, smelldist - 1, origdist, x + 1, y, first_move, optimal_move, 'r');
	}
	
	if (!dungeon[y*maxcols + x-1].marked && prev != 'r' && (!dungeon[y*maxcols + x - 1].wall || dungeon[y*maxcols + x - 1].hero)) { //(dungeon[y*maxcols + x-1].top != WALL && dungeon[y*maxcols + x-1].top != UNBREAKABLE_WALL && dungeon[y*maxcols + x - 1].top != LAVA) && !dungeon[y*maxcols + x-1].enemy) {
		if (smelldist == origdist)
			first_move = 'l';
		attemptChase(dungeon, maxcols, pos, shortest, smelldist - 1, origdist, x - 1, y, first_move, optimal_move, 'l');
	}

	if (!dungeon[(y+1)*maxcols + x].marked && prev != 'u' && (!dungeon[(y + 1)*maxcols + x].wall || dungeon[(y+1)*maxcols + x].hero)) { //(dungeon[(y+1)*maxcols + x].top != WALL && dungeon[(y+1)*maxcols + x].top != UNBREAKABLE_WALL && dungeon[(y + 1)*maxcols + x].top != LAVA) && !dungeon[(y+1)*maxcols + x].enemy) {
		if (smelldist == origdist)
			first_move = 'd';
		attemptChase(dungeon, maxcols, pos, shortest, smelldist - 1, origdist, x, y + 1, first_move, optimal_move, 'd');
	}

	if (!dungeon[(y-1)*maxcols + x].marked && prev != 'd' && (!dungeon[(y - 1)*maxcols + x].wall || dungeon[(y-1)*maxcols + x].hero)) { //(dungeon[(y-1)*maxcols + x].top != WALL && dungeon[(y-1)*maxcols + x].top != UNBREAKABLE_WALL && dungeon[(y - 1)*maxcols + x].top != LAVA) && !dungeon[(y-1)*maxcols + x].enemy) {
		if (smelldist == origdist)
			first_move = 'u';
		attemptChase(dungeon, maxcols, pos, shortest, smelldist - 1, origdist, x, y - 1, first_move, optimal_move, 'u');
	}

	// unmark the tile as visited when backtracking
	dungeon[y*maxcols + x].marked = false;

	if (optimal_move != '0')
		return true;
	return false;
}
bool Dungeon::playerInRange(int mx, int my, int pos, int range) {
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	// if enemy and player are too far separated, then do not attempt to recurse
	if (abs(px - mx) > range || abs(py - my) > range)
		return false;

	int steps = 0;

	while (px != mx) {
		if (px > mx)
			mx++;
		else
			mx--;

		steps++;
	}

	while (py != my) {
		if (py > my)
			my++;
		else
			my--;

		steps++;
	}

	return (steps <= range ? true : false);
}
void Dungeon::moveWithSuccessfulChase(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, char move, std::vector<std::shared_ptr<Monster>> &monsters) {
	std::string monster = monsters.at(pos)->getName();

	switch (move) {
	case 'l':
		if (!dungeon[my*maxcols + mx - 1].enemy) {
			dungeon[my*maxcols + mx - 1].top = GOBLIN;
			if (monster == "bombee")
				dungeon[my*maxcols + mx - 1].top = BOMBEE;
			else if (monster == "seeker")
				dungeon[my*maxcols + mx - 1].top = SEEKER;
			dungeon[my*maxcols + mx - 1].enemy = true;
			dungeon[my*maxcols + mx].top = SPACE;
			dungeon[my*maxcols + mx].enemy = false;

			//moveSprite(monster_sprites, maxrows, mx, my, move); // insert monster sprites vector
			queueMoveSprite(monsters.at(pos), move);
			monsters.at(pos)->setPosX(mx - 1);
		}
		break;
	case 'r':
		if (!dungeon[my*maxcols + mx + 1].enemy) {
			dungeon[my*maxcols + mx + 1].top = GOBLIN;
			if (monster == "bombee")
				dungeon[my*maxcols + mx + 1].top = BOMBEE;
			else if (monster == "seeker")
				dungeon[my*maxcols + mx + 1].top = SEEKER;
			dungeon[my*maxcols + mx + 1].enemy = true;
			dungeon[my*maxcols + mx].top = SPACE;
			dungeon[my*maxcols + mx].enemy = false;

			//moveSprite(monster_sprites, maxrows, mx, my, move);
			queueMoveSprite(monsters.at(pos), move);
			monsters.at(pos)->setPosX(mx + 1);
		}
		break;
	case 'd':
		if (!dungeon[(my + 1)*maxcols + mx].enemy) {
			dungeon[(my + 1)*maxcols + mx].top = GOBLIN;
			if (monster == "bombee")
				dungeon[(my + 1)*maxcols + mx].top = BOMBEE;
			else if (monster == "seeker")
				dungeon[(my + 1)*maxcols + mx].top = SEEKER;
			dungeon[(my + 1)*maxcols + mx].enemy = true;
			dungeon[my*maxcols + mx].top = SPACE;
			dungeon[my*maxcols + mx].enemy = false;

			//moveSprite(monster_sprites, maxrows, mx, my, move);
			queueMoveSprite(monsters.at(pos), move);
			monsters.at(pos)->setPosY(my + 1);
		}
		break;
	case 'u':
		if (!dungeon[(my - 1)*maxcols + mx].enemy) {
			dungeon[(my - 1)*maxcols + mx].top = GOBLIN;
			if (monster == "bombee")
				dungeon[(my - 1)*maxcols + mx].top = BOMBEE;
			else if (monster == "seeker")
				dungeon[(my - 1)*maxcols + mx].top = SEEKER;
			dungeon[(my - 1)*maxcols + mx].enemy = true;
			dungeon[my*maxcols + mx].top = SPACE;
			dungeon[my*maxcols + mx].enemy = false;

			//moveSprite(monster_sprites, maxrows, mx, my, move);
			queueMoveSprite(monsters.at(pos), move);
			monsters.at(pos)->setPosY(my - 1);
		}
		break;
	default:
		monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		break;
	}
}
void Dungeon::moveSeekers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, char move, std::vector<std::shared_ptr<Monster>> &monsters) {
	std::shared_ptr<Seeker> seeker = std::dynamic_pointer_cast<Seeker>(monsters.at(pos));

	if (seeker->getStep())
		moveWithSuccessfulChase(dungeon, maxrows, maxcols, mx, my, pos, move, monsters);

	seeker->toggleStep();
	monsters.at(pos) = seeker;
	seeker.reset();
}

// Enemies that move randomly in some way
void Dungeon::checkArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	shared_ptr<Archer> archer = dynamic_pointer_cast<Archer>(monsters.at(pos));
	int mx = monsters.at(pos)->getPosX();
	int my = monsters.at(pos)->getPosY();

	//	if archer is primed, check to attack and return
	if (archer->isPrimed()) {
		// if player is in sight, shoot them
		if (x - mx == 0 || y - my == 0) {
			cocos2d::experimental::AudioEngine::play2d("Bow_Release.mp3", false, 0.8f);

			monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		}
		// otherwise relax their aim
		else {

			dungeonText.push_back("The archer eases their aim.\n");
		}

		archer->prime(false);
		monsters.at(pos).reset();
		monsters.at(pos) = archer;
		archer.reset();

		return;
	}


	// if archer has dagger out and is adjacent to player, attack player
	if (monsters.at(pos)->getWeapon()->getAction() == "Bronze Dagger" && \
		(dungeon[(my - 1)*maxcols + mx].hero || dungeon[(my + 1)*maxcols + mx].hero || dungeon[my*maxcols + mx - 1].hero || dungeon[my*maxcols + mx + 1].hero)) {
		// player archer dagger swipe sound effect
		playArcherDaggerSwipe();
		
		monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		return;
	}
	// else if dagger is out and player moved away, switch back to bow
	else if (monsters.at(pos)->getWeapon()->getAction() == "Bronze Dagger" && \
		(!dungeon[(my - 1)*maxcols + mx].hero && !dungeon[(my + 1)*maxcols + mx].hero && !dungeon[my*maxcols + mx - 1].hero && !dungeon[my*maxcols + mx + 1].hero)) {
		cocos2d::experimental::AudioEngine::play2d("Bow_Primed.mp3", false, 0.8f);
		
		monsters.at(pos)->setWeapon(std::make_shared<WoodBow>());
		//dungeonText.push_back("The archer switches back to their Wood Bow.\n");
		return;
	}

	// if player is invisible, move randomly
	if (player.at(0).isInvisible()) {
		moveMonsterRandomly(dungeon, maxrows, maxcols, monsters, pos);
		return;
	}

	// if player is not in the archer's sights, move archer
	if (x - mx != 0 && y - my != 0) {
		moveMonsterRandomly(dungeon, maxrows, maxcols, monsters, pos);
		
		return;
	}

	// if player and archer are on the same column
	if (x - mx == 0) {		
		//	if player is within range and not adjacent, get primed
		if (abs(y - my) > 1 && abs(y - my) <= 15) {
			if (!wallCollision(dungeon, maxcols, 'y', y, my)) {
				cocos2d::experimental::AudioEngine::play2d("Bow_Drawn.mp3", false, 0.8f);

				archer->prime(true);
				monsters.at(pos).reset();
				monsters.at(pos) = archer;
				archer.reset();

				dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveMonsterRandomly(dungeon, maxrows, maxcols, monsters, pos);
				
				return;
			}
		}
		// swap weapon to dagger
		else if (abs(y - my) == 1) {
			cocos2d::experimental::AudioEngine::play2d("Archer_Dagger_Switch2.mp3", false, 0.8f);

			monsters.at(pos)->setWeapon(std::make_shared<BronzeDagger>());
			dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
		}
		//	just move archer
		else {
			moveMonsterRandomly(dungeon, maxrows, maxcols, monsters, pos);
			
			return;
		}
	}
	// if player and archer are on the same row
	else if (y - my == 0) {
		//	if player is within range and not adjacent, get primed
		if (abs(x - mx) > 1 && abs(x - mx) <= 15) {
			if (!wallCollision(dungeon, maxcols, 'x', x, mx)) {
				cocos2d::experimental::AudioEngine::play2d("Bow_Drawn.mp3", false, 0.8f);

				archer->prime(true);
				monsters.at(pos).reset();
				monsters.at(pos) = archer;
				archer.reset();

				dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveMonsterRandomly(dungeon, maxrows, maxcols, monsters, pos);
				
				return;
			}
		}
		// swap weapon to dagger
		else if (abs(x - mx) == 1) {
			cocos2d::experimental::AudioEngine::play2d("Archer_Dagger_Switch2.mp3", false, 0.8f);

			monsters.at(pos)->setWeapon(std::make_shared<BronzeDagger>());
			dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
		}
		// just move archer
		else {
			moveMonsterRandomly(dungeon, maxrows, maxcols, monsters, pos);
		}
	}

}
void Dungeon::checkMountedKnights(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	std::shared_ptr<MountedKnight> knight = std::dynamic_pointer_cast<MountedKnight>(monsters.at(pos));
	int mx = monsters.at(pos)->getPosX();
	int my = monsters.at(pos)->getPosY();

	// flags
	bool leftwall, rightwall, upwall, downwall;
	leftwall = dungeon[my*maxcols + mx - 1].wall;
	rightwall = dungeon[my*maxcols + mx + 1].wall;
	upwall = dungeon[(my - 1)*maxcols + mx].wall;
	downwall = dungeon[(my + 1)*maxcols + mx].wall;

	bool leftenemy, rightenemy, upenemy, downenemy;
	leftenemy = dungeon[my*maxcols + mx - 1].enemy;
	rightenemy = dungeon[my*maxcols + mx + 1].enemy;
	upenemy = dungeon[(my - 1)*maxcols + mx].enemy;
	downenemy = dungeon[(my + 1)*maxcols + mx].enemy;

	bool lefthero, righthero, uphero, downhero;
	lefthero = dungeon[my*maxcols + mx - 1].hero;
	righthero = dungeon[my*maxcols + mx + 1].hero;
	uphero = dungeon[(my - 1)*maxcols + mx].hero;
	downhero = dungeon[(my + 1)*maxcols + mx].hero;

	// if player is invisible, just move them randomly
	if (player.at(0).isInvisible()) {
		moveMountedKnights(dungeon, maxrows, maxcols, mx, my, pos, monsters);
		return;
	}

	//	if knight was alerted, check to attack, stop, or advance and return
	if (knight->isAlerted()) {
		char direction = knight->getDirection();
		if (direction == 'l') {
			if (lefthero) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
			else if (leftwall || leftenemy) {
				knight->toggleAlert();
				monsters.at(pos) = knight;
				knight.reset();

				return;
			}
			else {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;
				dungeon[my*maxcols + mx - 1].top = MOUNTED_KNIGHT;
				dungeon[my*maxcols + mx - 1].enemy = true;

				//moveSprite(monster_sprites, maxrows, mx, my, direction);
				queueMoveSprite(monsters.at(pos), direction);
				monsters.at(pos)->setPosX(mx - 1);
			}
		}
		else if (direction == 'r') {
			if (righthero) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
			else if (rightwall || rightenemy) {
				knight->toggleAlert();
				monsters.at(pos) = knight;
				knight.reset();

				return;
			}
			else {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;
				dungeon[my*maxcols + mx + 1].top = MOUNTED_KNIGHT;
				dungeon[my*maxcols + mx + 1].enemy = true;

				//moveSprite(monster_sprites, maxrows, mx, my, direction);
				queueMoveSprite(monsters.at(pos), direction);
				monsters.at(pos)->setPosX(mx + 1);
			}
		}
		else if (direction == 'u') {
			if (uphero) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
			else if (upwall || upenemy) {
				knight->toggleAlert();
				monsters.at(pos) = knight;
				knight.reset();

				return;
			}
			else {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;
				dungeon[(my - 1)*maxcols + mx].top = MOUNTED_KNIGHT;
				dungeon[(my - 1)*maxcols + mx].enemy = true;

				//moveSprite(monster_sprites, maxrows, mx, my, direction);
				queueMoveSprite(monsters.at(pos), direction);
				monsters.at(pos)->setPosY(my - 1);
			}
		}
		else if (direction == 'd') {
			if (downhero) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
			else if (downwall || downenemy) {
				knight->toggleAlert();
				monsters.at(pos) = knight;
				knight.reset();

				return;
			}
			else {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;
				dungeon[(my + 1)*maxcols + mx].top = MOUNTED_KNIGHT;
				dungeon[(my + 1)*maxcols + mx].enemy = true;

				//moveSprite(monster_sprites, maxrows, mx, my, direction);
				queueMoveSprite(monsters.at(pos), direction);
				monsters.at(pos)->setPosY(my + 1);
			}
		}

		return;
	}

	//	if player is not in the knight's sights, move knight
	if (x - mx != 0 && y - my != 0) {
		moveMountedKnights(dungeon, maxrows, maxcols, mx, my, pos, monsters);
		return;
	}

	// if player and knight are on the same column
	if (x - mx == 0) {
		//	if player is in line of sight, charge them
		if (!wallCollision(dungeon, maxcols, 'y', y, my)) {
			knight->setDirection(y - my > 0 ? 'd' : 'u');
			knight->toggleAlert();
			monsters.at(pos) = knight;
			knight.reset();

			dungeonText.push_back("The " + monsters.at(pos)->getName() + " charges toward you!\n");
		}
		else {
			moveMountedKnights(dungeon, maxrows, maxcols, mx, my, pos, monsters);
			return;
		}
	}
	// if player and knight are on the same row
	else if (y - my == 0) {
		//	if player is in line of sight, charge them
		if (!wallCollision(dungeon, maxcols, 'x', x, mx)) {
			knight->setDirection(x - mx > 0 ? 'r' : 'l');
			knight->toggleAlert();
			monsters.at(pos) = knight;

			dungeonText.push_back("The " + monsters.at(pos)->getName() + " charges toward you!\n");
		}
		else {
			moveMountedKnights(dungeon, maxrows, maxcols, mx, my, pos, monsters);
			return;
		}
	}

}
void Dungeon::moveMountedKnights(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	//	50% chance to not move
	int check;
	if (check = randInt(100), check >= 50)
		return;

	// else move the knight in a random direction
	moveMonsterRandomly(dungeon, maxrows, maxcols, monsters, pos);
}
void Dungeon::moveWanderers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	int n = -1 + randInt(3);
	int m = -1 + randInt(3);
	bool wall, enemy, hero;


	//char top = dungeon[(my + m)*maxcols + mx + n].top;
	wall = dungeon[(my + m)*maxcols + mx + n].wall;
	enemy = dungeon[(my + m)*maxcols + mx + n].enemy;
	hero = dungeon[(my + m)*maxcols + mx + n].hero;

	// if the randomly selected move is not a wall or enemy, move them
	if (!(wall || enemy)) {

		// if it's not the player, then move them to that space
		if (!hero) {
			dungeon[(my + m)*maxcols + mx + n].top = WANDERER;
			dungeon[(my + m)*maxcols + mx + n].enemy = true;
			dungeon[my*maxcols + mx].top = SPACE;
			dungeon[my*maxcols + mx].enemy = false;

			// Key: (for moveSprite 'move' parameter)
			// 1 u 2
			// l _ r
			// 3 d 4

			// n is x, m is y
			if (n == 0 && m != 0) {
				//moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? 'u' : 'd');
				queueMoveSprite(monsters.at(pos), m == -1 ? 'u' : 'd');
			}
			else if (n != 0 && m == 0) {
				//moveSprite(monster_sprites, maxrows, mx, my, n == -1 ? 'l' : 'r');
				queueMoveSprite(monsters.at(pos), n == -1 ? 'l' : 'r');
			}
			// wanderer moved diagonally
			else if (n != 0 && m != 0) {
				// moved left and ?
				if (n == -1) {
					//moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? '1' : '3');
					queueMoveSprite(monsters.at(pos), m == -1 ? '1' : '3');
				}
				// moved right and ?
				else if (n == 1) {
					//moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? '2' : '4');
					queueMoveSprite(monsters.at(pos), m == -1 ? '2' : '4');
				}
			}

			monsters.at(pos)->setPosX(mx + n);
			monsters.at(pos)->setPosY(my + m);
		}
		// otherwise attack the player
		else {
			monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		}
	}
}

// Enemies with unique movement
void Dungeon::moveRoundabouts(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	std::shared_ptr<Roundabout> ra = std::dynamic_pointer_cast<Roundabout>(monsters.at(pos));
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	bool leftwall, rightwall, upwall, downwall;
	leftwall = dungeon[my*maxcols + mx - 1].wall;
	rightwall = dungeon[my*maxcols + mx + 1].wall;
	upwall = dungeon[(my - 1)*maxcols + mx].wall;
	downwall = dungeon[(my + 1)*maxcols + mx].wall;

	bool leftenemy, rightenemy, upenemy, downenemy;
	leftenemy = dungeon[my*maxcols + mx - 1].enemy;
	rightenemy = dungeon[my*maxcols + mx + 1].enemy;
	upenemy = dungeon[(my - 1)*maxcols + mx].enemy;
	downenemy = dungeon[(my + 1)*maxcols + mx].enemy;

	bool lefthero, righthero, uphero, downhero;
	lefthero = dungeon[my*maxcols + mx - 1].hero;
	righthero = dungeon[my*maxcols + mx + 1].hero;
	uphero = dungeon[(my - 1)*maxcols + mx].hero;
	downhero = dungeon[(my + 1)*maxcols + mx].hero;

	if (ra->getDirection() == 'l') {

		//	if tile to left of the roundabout is not free, check above
		if (leftwall || leftenemy || lefthero) {
			//	if player is in the way, attack them
			if (lefthero) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}

			//	if tile above is a wall, then set direction to right
			if (upwall || upenemy || uphero) {
				if (uphero) {
					monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
				}
				ra->setDirection('r');
				monsters.at(pos).reset();
				monsters.at(pos) = ra;
			}
			//	otherwise direction is set up
			else {
				ra->setDirection('u');
				monsters.at(pos).reset();
				monsters.at(pos) = ra;
			}
		}
		else {
			dungeon[my*maxcols + mx].top = SPACE;
			dungeon[my*maxcols + mx].enemy = false;
			dungeon[my*maxcols + mx - 1].top = ROUNDABOUT;
			dungeon[my*maxcols + mx - 1].enemy = true;

			//moveSprite(monster_sprites, maxrows, mx, my, ra->getDirection());
			queueMoveSprite(monsters.at(pos), ra->getDirection());
			monsters.at(pos)->setPosX(mx - 1);
		}
	}
	else if (ra->getDirection() == 'r') {
		//	if tile to right of the roundabout is a wall, check below
		if (rightwall || rightenemy || righthero) {
			//	if player is in the way, attack them
			if (righthero) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}

			//	if tile below is a wall, then set direction to left
			if (downwall || downenemy || downhero) {
				if (downhero) {
					monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
				}
				ra->setDirection('l');
				monsters.at(pos).reset();
				monsters.at(pos) = ra;
			}
			//	otherwise direction is set down
			else {
				ra->setDirection('d');
				monsters.at(pos).reset();
				monsters.at(pos) = ra;
			}
		}
		else {
			dungeon[my*maxcols + mx].top = SPACE;
			dungeon[my*maxcols + mx].enemy = false;
			dungeon[my*maxcols + mx + 1].top = ROUNDABOUT;
			dungeon[my*maxcols + mx + 1].enemy = true;

			//moveSprite(monster_sprites, maxrows, mx, my, ra->getDirection());
			queueMoveSprite(monsters.at(pos), ra->getDirection());
			monsters.at(pos)->setPosX(mx + 1);
		}
	}
	else if (ra->getDirection() == 'u') {
		//	if tile above the roundabout is a wall, check right
		if (upwall || upenemy || uphero) {
			//	if player is in the way, attack them
			if (uphero) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}

			//	if tile to the right is a wall, then set direction downward
			if (rightwall || rightenemy || righthero) {
				if (righthero) {
					monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
				}
				ra->setDirection('d');
				monsters.at(pos).reset();
				monsters.at(pos) = ra;
			}
			//	otherwise direction is set right
			else {
				ra->setDirection('r');
				monsters.at(pos).reset();
				monsters.at(pos) = ra;
			}
		}
		else {
			dungeon[my*maxcols + mx].top = SPACE;
			dungeon[my*maxcols + mx].enemy = false;
			dungeon[(my - 1)*maxcols + mx].top = ROUNDABOUT;
			dungeon[(my - 1)*maxcols + mx].enemy = true;

			//moveSprite(monster_sprites, maxrows, mx, my, ra->getDirection());
			queueMoveSprite(monsters.at(pos), ra->getDirection());
			monsters.at(pos)->setPosY(my - 1);
		}
	}
	else if (ra->getDirection() == 'd') {
		//	if tile below the roundabout is a wall, check left
		if (downwall || downenemy || downhero) {
			//	if player is in the way, attack them
			if (downhero) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}

			//	if tile to the left is a wall, then set direction upward
			if (leftwall || leftenemy || lefthero) {
				if (lefthero) {
					monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
				}
				ra->setDirection('u');
				monsters.at(pos).reset();
				monsters.at(pos) = ra;
			}
			//	otherwise direction is set left
			else {
				ra->setDirection('l');
				monsters.at(pos).reset();
				monsters.at(pos) = ra;
			}
		}
		else {
			dungeon[my*maxcols + mx].top = SPACE;
			dungeon[my*maxcols + mx].enemy = false;
			dungeon[(my + 1)*maxcols + mx].top = ROUNDABOUT;
			dungeon[(my + 1)*maxcols + mx].enemy = true;

			//moveSprite(monster_sprites, maxrows, mx, my, ra->getDirection());
			queueMoveSprite(monsters.at(pos), ra->getDirection());
			monsters.at(pos)->setPosY(my + 1);
		}
	}

	ra.reset();
}

// Stationary enemies
void Dungeon::moveZappers(std::vector<_Tile> &dungeon, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	shared_ptr<Zapper> zapper = dynamic_pointer_cast<Zapper>(monsters.at(pos));
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();


	//	if zapper is on cooldown, reset cooldown and switch attack direction
	if (zapper->onCooldown()) {
		if (zapper->cardinalAttack()) {
			dungeon[my*maxcols + mx + 1].projectile = dungeon[my*maxcols + mx - 1].projectile = dungeon[(my - 1)*maxcols + mx].projectile = dungeon[(my + 1)*maxcols + mx].projectile = SPACE;
			// hide cardinal sparks
			for (int i = 0; i < 4; i++) {
				zapper->getSparks()[i]->setVisible(false);
			}
		}
		else {
			dungeon[(my - 1)*maxcols + mx - 1].projectile = dungeon[(my - 1)*maxcols + mx + 1].projectile = dungeon[(my + 1)*maxcols + mx - 1].projectile = dungeon[(my + 1)*maxcols + mx + 1].projectile = SPACE;
			// hide diagonal sparks
			for (int i = 4; i < 8; i++) {
				zapper->getSparks()[i]->setVisible(false);
			}
		}
		zapper->setCooldown();
		zapper->swapDirection();
	}
	//	else throw out sparks and attack player if they were in the way
	else {
		if (zapper->cardinalAttack()) {
			dungeon[my*maxcols + mx + 1].projectile = dungeon[my*maxcols + mx - 1].projectile = dungeon[(my - 1)*maxcols + mx].projectile = dungeon[(my + 1)*maxcols + mx].projectile = SPARK;
			
			// show cardinal sparks
			for (int i = 0; i < 4; i++) {
				zapper->getSparks()[i]->setVisible(true);
			}

			if ((px == mx + 1 && py == my) || (px == mx - 1 && py == my) || (px == mx && py == my + 1) || (px == mx && py == my - 1)) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
		}
		else {
			dungeon[(my-1)*maxcols + mx - 1].projectile = dungeon[(my-1)*maxcols + mx + 1].projectile = dungeon[(my + 1)*maxcols + mx - 1].projectile = dungeon[(my + 1)*maxcols + mx + 1].projectile = SPARK;
			
			// show diagonal sparks
			for (int i = 4; i < 8; i++) {
				zapper->getSparks()[i]->setVisible(true);
			}
			
			if ((px == mx - 1 && py == my-1) || (px == mx + 1 && py == my-1) || (px == mx-1 && py == my + 1) || (px == mx+1 && py == my + 1)) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
		}
		zapper->setCooldown();
	}
	monsters.at(pos).reset();
	monsters.at(pos) = zapper;

	zapper.reset();
}
void Dungeon::moveSpinners(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	std::shared_ptr<Spinner> spinner = std::dynamic_pointer_cast<Spinner>(monsters.at(pos));
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	// reset projectiles to visible
	spinner->getInner()->setVisible(true);
	spinner->getOuter()->setVisible(true);

	// 8 1 2 
	// 7 X 3
	// 6 5 4
	// moves spinner -to- position getAngle(), it is not where it currently is

	if (spinner->isClockwise()) {
		switch (spinner->getAngle()) {
		case 1:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my - 1, 'r');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my - 2, 'R');
			spinner->setFirePosition('r');

			if (my == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 2:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my - 1, 'r');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my - 2, 'R');
			spinner->setFirePosition('r');

			if (my == 1 || mx == maxcols - 2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 3:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my - 1, 'd');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my - 2, 'D');
			spinner->setFirePosition('d');
		
			// if spinner is on the left or right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 4:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my, 'd');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my, 'D');
			spinner->setFirePosition('d');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 5:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my + 1, 'l');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my + 2, 'L');
			spinner->setFirePosition('l');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 6:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my + 1, 'l');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my + 2, 'L');
			spinner->setFirePosition('l');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (my == maxrows - 2 || mx == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 7:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my + 1, 'u');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my + 2, 'U');
			spinner->setFirePosition('u');

			// if spinner is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 8:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my, 'u');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my, 'U');
			spinner->setFirePosition('u');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		default:
			break;
		}

		if (spinner->getAngle() == 8)
			spinner->setAngle(1);
		else
			spinner->setAngle(spinner->getAngle() + 1);
	}
	// else counterclockwise
	else {
		switch (spinner->getAngle()) {
		case 1:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my - 1, 'l');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my - 2, 'L');
			spinner->setFirePosition('l');

			if (my == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 2:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my, 'u');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my, 'U');
			spinner->setFirePosition('u');

			// if spinner is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols-2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 3:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my + 1, 'u');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my + 2, 'U');
			spinner->setFirePosition('u');

			// if spinner is on the right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 4:
		
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my + 1, 'r');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my + 2, 'R');
			spinner->setFirePosition('r');

			// if spinner is on the bottom or right edge boundary, hide the projectiles
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2){
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 5:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my + 1, 'r');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my + 2, 'R');
			spinner->setFirePosition('r');

			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 6:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my, 'd');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my, 'D');
			spinner->setFirePosition('d');

			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == maxrows-2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 7:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my - 1, 'd');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my - 2, 'D');
			spinner->setFirePosition('d');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 8:
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my - 1, 'l');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my - 2, 'L');
			spinner->setFirePosition('l');

			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		default:
			break;
		}
		
		if (spinner->getAngle() == 1)
			spinner->setAngle(8);
		else
			spinner->setAngle(spinner->getAngle() - 1);
	}
	
	if (spinner->playerWasHit(player.at(0))) {
		spinner->encounter(player.at(0), *(spinner), dungeonText);
	}
	spinner.reset();
}

// Helper functions for moving enemies
void Dungeon::moveMonsterRandomly(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Monster>> &monsters, int pos) {
	int mx = monsters.at(pos)->getPosX();
	int my = monsters.at(pos)->getPosY();

	int n = randInt(2);
	int m = 0;
	char move;

	bool leftwall, rightwall, upwall, downwall;
	leftwall = dungeon[my*maxcols + mx - 1].wall;
	rightwall = dungeon[my*maxcols + mx + 1].wall;
	upwall = dungeon[(my - 1)*maxcols + mx].wall;
	downwall = dungeon[(my + 1)*maxcols + mx].wall;

	bool leftenemy, rightenemy, upenemy, downenemy;
	leftenemy = dungeon[my*maxcols + mx - 1].enemy;
	rightenemy = dungeon[my*maxcols + mx + 1].enemy;
	upenemy = dungeon[(my - 1)*maxcols + mx].enemy;
	downenemy = dungeon[(my + 1)*maxcols + mx].enemy;

	bool lefttrap, righttrap, uptrap, downtrap;
	lefttrap = dungeon[my*maxcols + mx - 1].trap;
	righttrap = dungeon[my*maxcols + mx + 1].trap;
	uptrap = dungeon[(my - 1)*maxcols + mx].trap;
	downtrap = dungeon[(my + 1)*maxcols + mx].trap;

	//  if monster is surrounded completely, do nothing (such as goblin or wanderer blocking an archer's path)
	if ((leftwall || leftenemy || lefttrap) && (rightwall || rightenemy || righttrap) &&
		(downwall || downenemy || downtrap) && (upwall || upenemy || uptrap)) {
		return;
	}
	//	if monster is surrounded by walls on left and right
	else if ((leftwall || leftenemy || lefttrap) && (rightwall || rightenemy || righttrap)) {
		move = 'v';
	}
	//	if monster is surrounded by walls on top and bottom
	else if ((downwall || downenemy || downtrap) && (upwall || upenemy || uptrap)) {
		move = 'h';
	}
	//	else pick a random direction
	else {
		move = (n == 0 ? 'h' : 'v');
	}

	char top, upper;
	bool wall, enemy, trap;

	//	move is horizontal
	if (move == 'h') {
		n = -1 + randInt(3);
		//top = dungeon[my*maxcols + mx + n].top;
		upper = dungeon[my*maxcols + mx + n].upper;
		wall = dungeon[my*maxcols + mx + n].wall;
		enemy = dungeon[my*maxcols + mx + n].enemy;
		trap = dungeon[my*maxcols + mx + n].trap;

		while (n == 0 || wall || enemy || trap || upper != SPACE) {
			n = -1 + randInt(3);

			//top = dungeon[my*maxcols + mx + n].top;
			upper = dungeon[my*maxcols + mx + n].upper;
			wall = dungeon[my*maxcols + mx + n].wall;
			enemy = dungeon[my*maxcols + mx + n].enemy;
			trap = dungeon[my*maxcols + mx + n].trap;
		}

		//dungeon[my*maxcols + mx + n].top = ARCHER;
		dungeon[my*maxcols + mx + n].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;

		//moveSprite(monster_sprites, maxrows, mx, my, n == -1 ? 'l' : 'r');
		queueMoveSprite(monsters.at(pos), n == -1 ? 'l' : 'r');
		monsters.at(pos)->setPosX(mx + n);
	}
	//	move is vertical
	else if (move == 'v') {
		m = -1 + randInt(3);
		//top = dungeon[(my + m)*maxcols + mx].top;
		upper = dungeon[(my + m)*maxcols + mx].upper;
		wall = dungeon[(my + m)*maxcols + mx].wall;
		enemy = dungeon[(my + m)*maxcols + mx].enemy;
		trap = dungeon[(my + m)*maxcols + mx].trap;

		while (m == 0 || wall || enemy || trap || upper != SPACE) {
			m = -1 + randInt(3);

			//top = dungeon[(my + m)*maxcols + mx].top;
			upper = dungeon[(my + m)*maxcols + mx].upper;
			wall = dungeon[(my + m)*maxcols + mx].wall;
			enemy = dungeon[(my + m)*maxcols + mx].enemy;
			trap = dungeon[(my + m)*maxcols + mx].trap;
		}

		//dungeon[(my + m)*maxcols + mx].top = ARCHER;
		dungeon[(my + m)*maxcols + mx].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;

		//moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? 'u' : 'd');
		queueMoveSprite(monsters.at(pos), m == -1 ? 'u' : 'd');
		monsters.at(pos)->setPosY(my + m);
	}
}
bool Dungeon::wallCollision(std::vector<_Tile> &dungeon, int maxcols, char direction, int p_move, int m_move) {
	// if they're one space apart, return false
	if (p_move - m_move == 1)
		return false;

	// p_move and m_move are the x/y position of the player and monster, respectively
	// direction: the axis for the enemy to check for walls along
	// wallCollision() is only called when an enemy has a potential line of sight on the player

	// Since player and monster are on the same row/column, set otherdir to the complement of p_move/m_move
	int otherdir;

	if (direction == 'x') {
		otherdir = player.at(0).getPosY();
	}
	else {
		otherdir = player.at(0).getPosX();
	}

	char top;
	bool wall;

	// if player is below or to the right of the enemy, let monster check moving in the direction opposite (otherdir)
	if (p_move > m_move) {
		p_move--;
		while (p_move - m_move != 0) {
			if (direction == 'x') {
				
				wall = dungeon[otherdir*maxcols + p_move].wall;
				if (wall)
					return true;
				else
					p_move--;
			}
			else if (direction == 'y') {
				
				wall = dungeon[(p_move)*maxcols + otherdir].wall;
				if (wall)
					return true;
				else
					p_move--;
			}
		}
	}
	else {
		m_move--;
		while (m_move - p_move != 0) {
			if (direction == 'x') {
				
				wall = dungeon[otherdir*maxcols + m_move].wall;
				if (wall)
					return true;
				else
					m_move--;
			}
			else if (direction == 'y') {
				
				wall = dungeon[(m_move)*maxcols + otherdir].wall;
				if (wall)
					return true;
				else
					m_move--;
			}
		}
	}
	return false;
}

int Dungeon::findMonster(int mx, int my, std::vector<std::shared_ptr<Monster>> monsters) {
	for (int i = 0; i < monsters.size(); i++) {
		if (monsters.at(i)->getPosX() == mx && monsters.at(i)->getPosY() == my) {
			return i;
		}
	}
	return -1;
}
void Dungeon::fight(Dungeon &dungeon, int x, int y) { // monster's coordinates
	int i = findMonster(x, y, dungeon.getMonsters());
	
	if (i == -1)
		return;

	//player.at(0).attack(dungeon.getMonsters(), dungeon.getActives(), i, dungeonText);
	player.at(0).attack(dungeon, *dungeon.getMonsters().at(i));

	if (dungeon.getMonsters().at(i)->getHP() <= 0) {
		monsterDeath(dungeon.getDungeon(), dungeon.getRows(), dungeon.getCols(), dungeon.getActives(), dungeon.getMonsters(), i);
	}
}

// Item collection
void Dungeon::foundItem(std::vector<_Tile> &dungeon, int maxcols, int x, int y) {
	char c = dungeon[y*maxcols + x].bottom;

	//dungeonText.push_back("You've found ");
	//switch (c) {
	//	// WEAPONS
	//case CUTLASS: dungeonText.push_back("a Rusty Cutlass.\n"); break;
	//case BONEAXE: dungeonText.push_back("a Bone Axe.\n"); break;
	//case BRONZE_DAGGER: dungeonText.push_back("a Bronze Dagger.\n"); break;
	//case IRON_LANCE: dungeonText.push_back("an Iron Lance.\n"); break;
	//	// ITEMS
	//case HEART_POD: dungeonText.push_back("a Heart Pod.\n"); break;
	//case LIFEPOT: dungeonText.push_back("a Life Potion.\n"); break;
	//case ARMOR: dungeonText.push_back("some extra Armor.\n"); break;
	//case STATPOT: dungeonText.push_back("a Stat Potion.\n"); break;
	//case BOMB: dungeonText.push_back("a Bomb!\n"); break;
	//case BROWN_CHEST: dungeonText.push_back("a Brown Chest!\n"); break;
	//case SILVER_CHEST: dungeonText.push_back("a Silver Chest!\n"); break;
	//case GOLDEN_CHEST: dungeonText.push_back("a Golden Chest!\n"); break;
	//case SKELETON_KEY: dungeonText.push_back("a Mysterious Trinket.\n"); break;
	//case INFINITY_BOX: dungeonText.push_back("an ornate box, emanating with power.\n"); break;
	//	// OBJECTS
	//case STAIRS: dungeonText.push_back("stairs!\n"); break;
	//case LOCKED_STAIRS: dungeonText.push_back("stairs but it's blocked off...\n"); break;
	//case BUTTON: dungeonText.push_back("a button!?\n"); break;
	//default:
	//	break;
	//}
}
void Dungeon::collectItem(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y) {
	int current_inv_size = player.at(0).getWeapons().size() + player.at(0).getItems().size();
	int current_wep_inv_size = player.at(0).getWeapons().size();
	int current_item_inv_size = player.at(0).getItems().size();
	int max_wep_inv_size = player.at(0).getMaxWeaponInvSize();
	int max_item_inv_size = player.at(0).getMaxItemInvSize();
	std::string image;

	char bottom = dungeon[y*maxcols + x].bottom;
	char traptile = dungeon[y*maxcols + x].traptile;
	std::string item_name = dungeon[y*maxcols + x].item_name;

	//		WEAPONS
	if (item_name == RUSTY_CUTLASS) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(std::make_shared<RustyCutlass>(x, y));

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup2.mp3", false, 1.0f);

			dungeonText.push_back("You pick up the Rusty Cutlass.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (item_name == BONE_AXE) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(std::make_shared<BoneAxe>(x,y)); //adds bone axe to inventory

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup2.mp3", false, 1.0f);

			dungeonText.push_back("You pick up the Bone Axe.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (item_name == BRONZE_DAGGER) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(std::make_shared<BronzeDagger>());

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup.mp3", false, 1.0f);

			dungeonText.push_back("You pick up the Bronze Dagger.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (item_name == IRON_LANCE) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(std::make_shared<IronLance>());

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup.mp3", false, 1.0f);

			dungeonText.push_back("You pick up the Iron Lance.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}

	//		SHIELDS
	else if (item_name == WOOD_SHIELD) {
		// save shield at this spot, overwritten if shield is dropped
		Shield newShield = *(dynamic_pointer_cast<WoodShield>(dungeon[y*maxcols + x].object));
		std::string shieldimage = player.at(0).getShield().getImageName();

		// if player has a shield, then drop and replace with new one
		if (player.at(0).hasShield()) {
			// don't need to hash because the shield is picked up first
			/*if (dungeon[y*maxcols + x].item) {
				itemHash(dungeon, maxrows, maxcols, x, y);
			}*/
			// handles the placement of shield on the ground
			player.at(0).dropShield(dungeon, maxrows, maxcols);
			removeSprite(item_sprites, maxrows, x, y);
			
			player.at(0).equipShield(newShield);
			addSprite(item_sprites, maxrows, x, y, -1, shieldimage);

			return;
		}
		// else just equip the new one
		player.at(0).equipShield(newShield);
		player.at(0).setShieldPossession(true);

		dungeon[y*maxcols + x].item_name = EMPTY;
		dungeon[y*maxcols + x].object = nullptr;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}

	//		DROPS
	else if (item_name == HEART_POD) {
		cocos2d::experimental::AudioEngine::play2d("Heart_Pod_Pickup2.mp3", false, 1.0f);
		
		HeartPod heart;
		heart.changeStats(heart, player.at(0));

		dungeon[y*maxcols + x].item_name = SPACE;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
	else if (item_name == LIFEPOT) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<LifePotion>());

			cocos2d::experimental::AudioEngine::play2d("Potion_Pickup2.mp3", false, 1.0f);

			dungeonText.push_back("You grab the Life Potion!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (item_name == ARMOR) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<ArmorDrop>());

			cocos2d::experimental::AudioEngine::play2d("Armor_Pickup.mp3", false, 1.0f);

			dungeonText.push_back("You grab the Armor!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (item_name == STATPOT) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<StatPotion>());

			cocos2d::experimental::AudioEngine::play2d("Potion_Pickup2.mp3", false, 1.0f);

			//dungeonText.push_back("You grab the Stat Potion!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (item_name == DIZZY_ELIXIR) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<DizzyElixir>());

			cocos2d::experimental::AudioEngine::play2d("Potion_Pickup2.mp3", false, 1.0f);
		}
	}
	else if (item_name == BOMB) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<Bomb>());

			cocos2d::experimental::AudioEngine::play2d("Bomb_Pickup2.mp3", false, 1.0f);

			dungeonText.push_back("You grab the Bomb!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (item_name == SKELETON_KEY) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<SkeletonKey>());

			cocos2d::experimental::AudioEngine::play2d("Key_Pickup.mp3", false, 1.0f);

			//dungeonText.push_back("You pick up the Mysterious Trinket.\n");
			player.at(0).setKeyHP(player.at(0).getHP()); // set key hp (hp at time of pickup)
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (item_name == FREEZE_SPELL) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<FreezeSpell>());

			cocos2d::experimental::AudioEngine::play2d("Book_Pickup.mp3", false, 1.0f);
		}
	}
	else if (item_name == EARTHQUAKE_SPELL) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<EarthquakeSpell>());

			cocos2d::experimental::AudioEngine::play2d("Book_Pickup.mp3", false, 1.0f);
		}
	}
	else if (item_name == FIREBLAST_SPELL) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<FireBlastSpell>());

			cocos2d::experimental::AudioEngine::play2d("Book_Pickup.mp3", false, 1.0f);
		}
	}
	else if (item_name == WIND_SPELL) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<WindSpell>());

			cocos2d::experimental::AudioEngine::play2d("Book_Pickup.mp3", false, 1.0f);
		}
	}
	else if (item_name == INVISIBILITY_SPELL) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<InvisibilitySpell>());

			cocos2d::experimental::AudioEngine::play2d("Book_Pickup.mp3", false, 1.0f);
		}
	}
	else if (item_name == ETHEREAL_SPELL) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<EtherealSpell>());

			cocos2d::experimental::AudioEngine::play2d("Book_Pickup.mp3", false, 1.0f);
		}
	}
	else if (item_name == TELEPORT) {
	if (current_item_inv_size < max_item_inv_size) {
		player.at(0).addItem(std::make_shared<Teleport>());

		cocos2d::experimental::AudioEngine::play2d("Book_Pickup.mp3", false, 1.0f);
	}
	}
	else if (item_name == BROWN_CHEST) {
		BrownChest c;

		cocos2d::experimental::AudioEngine::play2d("ChestOpening1.mp3", false, 1.0f);

		dungeonText.push_back("You open the Brown Chest... ");

		removeSprite(item_sprites, maxrows, x, y);
		image = c.open(dungeon[y*maxcols + x], dungeonText);
		addSprite(item_sprites, maxrows, x, y, -1, image);
	}
	else if (item_name == SILVER_CHEST) {
		SilverChest c;

		cocos2d::experimental::AudioEngine::play2d("ChestOpening1.mp3", false, 1.0f);

		dungeonText.push_back("You open the Silver Chest... ");

		removeSprite(item_sprites, maxrows, x, y);
		image = c.open(dungeon[y*maxcols + x], dungeonText);
		addSprite(item_sprites, maxrows, x, y, -1, image);
	}
	else if (item_name == GOLDEN_CHEST) {
		GoldenChest c;

		cocos2d::experimental::AudioEngine::play2d("ChestOpening1.mp3", false, 1.0f);

		dungeonText.push_back("You open the Golden Chest... ");

		removeSprite(item_sprites, maxrows, x, y);
		image = c.open(dungeon[y*maxcols + x], dungeonText);
		addSprite(item_sprites, maxrows, x, y, -1, image);
	}
	else if (item_name == INFINITY_BOX) {
		dungeonText.push_back("The ornate box is sealed shut.\n");
	}


	//		MISC
	else if (bottom == IDOL) {
		player.at(0).setWin(true);
		dungeon[y*maxcols + x].bottom = SPACE;
	}
	else if (traptile == STAIRS) {
		setLevel(getLevel() + 1);
		//clearScreen();
	}
	else if (traptile == LOCKED_STAIRS) {
		cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);

		dungeonText.push_back("The stairs are locked!\n");
	}
	else if (traptile == BUTTON) {
		dungeonText.push_back("It's a button.\n");
	}
	else {
		cocos2d::experimental::AudioEngine::play2d("Player_Movement.mp3", false, 1.0f);

		dungeonText.push_back("There's nothing in the dirt... \n");
	}

	// if an item was added, remove the sprite that was taken and reset tile item status
	if (current_inv_size < player.at(0).getWeapons().size() + player.at(0).getItems().size()) {
		dungeon[y*maxcols + x].bottom = SPACE;
		dungeon[y*maxcols + x].object = nullptr;
		dungeon[y*maxcols + x].item_name = "";
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
}
void Dungeon::goldPickup(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y) {
	cocos2d::experimental::AudioEngine::play2d("Gold_Pickup2.mp3", false, 1.0f);

	player.at(0).setMoney(player.at(0).getMoney() + dungeon[y*maxcols + x].gold);
	dungeon[y*maxcols + x].gold = 0;
	removeSprite(money_sprites, maxrows, x, y);
}

/*
void Dungeon::showDungeon() {
	//	adjust window to be more centered
	cout << "\n\n\n\n";
	cout << "       ";

	_Tile *tile;
	for (int i = 0; i < MAXROWS; i++) {
		for (int j = 0; j < MAXCOLS; j++) {
			tile = &m_maze[i*MAXCOLS + j];
			if (tile->top == SPACE && tile->projectile == SPACE && tile->traptile != SPACE)
				cout << tile->traptile;
			else if (tile->top == SPACE && tile->projectile != SPACE)
				cout << tile->projectile;
			else if (tile->top == SPACE)
				cout << tile->bottom;
			else
				cout << tile->top;
		}
		cout << endl;
		cout << "       ";
	}
	cout << endl;

	cout << "       ";
	cout << "HP: " << player.at(0).getHP() << ", Armor: " << player.at(0).getArmor() \
		<< ", Strength: " << player.at(0).getStr() << ", Dexterity: " << player.at(0).getDex() \
		<< " | Current Weapon: " << player.at(0).getWeapon().getAction() << endl;
	cout << "       ";
	cout << endl;
}
*/
void Dungeon::showText() {
	for (unsigned i = 0; i < dungeonText.size(); i++) {
		cout << dungeonText.at(i);
	}
	dungeonText.clear();
}

std::vector<Player>& Dungeon::getPlayerVector() {
	return player;
}
Player Dungeon::getPlayer() const {
	return player.at(0);
}
void Dungeon::setPlayer(Player p) {
	player.at(0) = p;
}
int Dungeon::getLevel() const {
	return m_level;
}
void Dungeon::setLevel(int level) {
	m_level = level;
}

std::vector<_Tile>& Dungeon::getDungeon() {
	return m_maze;
}
int Dungeon::getRows() const {
	return m_rows;
}
int Dungeon::getCols() const {
	return m_cols;
}
std::vector<std::shared_ptr<Monster>>& Dungeon::getMonsters() {
	return m_monsters;
}
std::vector<std::shared_ptr<Objects>>& Dungeon::getActives() {
	return m_actives;
}
std::vector<std::shared_ptr<Objects>>& Dungeon::getTraps() {
	return m_traps;
}
void Dungeon::callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index) {
	player.at(0).use(*this, dungeon[y*maxcols + x], index);
}

//		:::: SPRITE SETTING ::::
void Dungeon::setPlayerSprite(cocos2d::Sprite* sprite) {
	player_sprite = sprite;
}
void Dungeon::setMoneySprites(std::vector<cocos2d::Sprite*> sprites) {
	money_sprites = sprites;
}
void Dungeon::setMonsterSprites(std::vector<cocos2d::Sprite*> sprites) {
	monster_sprites = sprites;
}
void Dungeon::setItemSprites(std::vector<cocos2d::Sprite*> sprites) {
	item_sprites = sprites;
}
void Dungeon::setTrapSprites(std::vector<cocos2d::Sprite*> sprites) {
	trap_sprites = sprites;
}
void Dungeon::setProjectileSprites(std::vector<cocos2d::Sprite*> sprites) {
	projectile_sprites = sprites;
}
void Dungeon::setSpikeProjectileSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites) {
	spike_sprites = sprites;
}
void Dungeon::setSpinnerSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites) {
	spinner_sprites = sprites;
}
void Dungeon::setZapperSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites) {
	zapper_sprites = sprites;
}
void Dungeon::setWallSprites(std::vector<cocos2d::Sprite*> sprites) {
	wall_sprites = sprites;
}
void Dungeon::setDoorSprites(std::vector<cocos2d::Sprite*> sprites) {
	door_sprites = sprites;
}
void Dungeon::setFloorSprites(std::vector<cocos2d::Sprite*> sprites) {
	floor_sprites = sprites;
}
void Dungeon::setScene(cocos2d::Scene* scene) {
	m_scene = scene;
}

cocos2d::Sprite* Dungeon::findSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y) {
	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (maxrows - y)*SPACING_FACTOR - Y_OFFSET;

	// used for storing the current sprite's x and y coordinates
	Vec2 point;
	int px, py;

	for (int i = 0; i < sprites.size(); i++) {
		point = sprites[i]->getPosition();
		px = point.x;
		py = point.y;
		if (x_sprite == px && y_sprite == py) {
			return sprites[i];
		}
	}
	return nullptr;
}
void Dungeon::setSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, char move) {
	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (maxrows - y)*SPACING_FACTOR - Y_OFFSET;

	// used for storing the current sprite's x and y coordinates
	Vec2 point;
	int px, py;

	// Key:
	// 1 u 2
	// l _ r
	// 3 d 4

	for (int i = 0; i < sprites.size(); i++) {
		point = sprites[i]->getPosition();
		px = point.x;
		py = point.y;
		if (x_sprite == px && y_sprite == py) {
			switch (move) {
			case 'l': sprites.at(i)->setPosition((x - 1)*SPACING_FACTOR - X_OFFSET, py); break;
			case 'r': sprites.at(i)->setPosition((x + 1)*SPACING_FACTOR - X_OFFSET, py); break;
			case 'u': sprites.at(i)->setPosition(px, (maxrows - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;
			case 'd': sprites.at(i)->setPosition(px, (maxrows - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '1': sprites.at(i)->setPosition((x - 1)*SPACING_FACTOR - X_OFFSET, (maxrows - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '2': sprites.at(i)->setPosition((x + 1)*SPACING_FACTOR - X_OFFSET, (maxrows - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '3': sprites.at(i)->setPosition((x - 1)*SPACING_FACTOR - X_OFFSET, (maxrows - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '4': sprites.at(i)->setPosition((x + 1)*SPACING_FACTOR - X_OFFSET, (maxrows - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;
			}
			return;
		}
	}
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, int x, int y) {
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, (getRows() - y)*SPACING_FACTOR - Y_OFFSET);
}

// Move sprites
void Dungeon::queueMoveSprite(std::shared_ptr<Monster> monster, char move) {
	switch (move) {
	case 'l': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, 0));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case 'L': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-2 * SPACING_FACTOR, 0));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case '<': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-3 * SPACING_FACTOR, 0));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case 'r': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, 0));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case 'R': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(2 * SPACING_FACTOR, 0));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case '>': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(3 * SPACING_FACTOR, 0));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case 'u': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case 'U': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, 2 * SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case '^': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, 3 * SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case 'd': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case 'D': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -2 * SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case 'V': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -3 * SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case '1': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case '2': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case '3': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, -SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	case '4': {
		auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, -SPACING_FACTOR));
		insertActionIntoSpawn(monster->getSprite(), move);
		break;
	}
	}
}
void Dungeon::queueMoveSprite(std::shared_ptr<Monster> monster, int maxrows, int cx, int cy) {
	auto move = cocos2d::MoveTo::create(.1, cocos2d::Vec2(cx * SPACING_FACTOR - X_OFFSET, (maxrows - cy)*SPACING_FACTOR - Y_OFFSET));
	insertActionIntoSpawn(monster->getSprite(), move);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, int maxrows, int cx, int cy, float time, bool front) {
	auto move = cocos2d::MoveTo::create(time, cocos2d::Vec2(cx * SPACING_FACTOR - X_OFFSET, (maxrows - cy)*SPACING_FACTOR - Y_OFFSET));
	insertActionIntoSpawn(sprite, move, front);
}
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, char move, float time) {
	switch (move) {
	case 'l': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-SPACING_FACTOR, 0));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case 'L': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-2 * SPACING_FACTOR, 0));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case '<': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-3 * SPACING_FACTOR, 0));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case 'r': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(SPACING_FACTOR, 0));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case 'R': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(2 * SPACING_FACTOR, 0));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case '>': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(3 * SPACING_FACTOR, 0));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case 'u': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case 'U': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, 2 * SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case '^': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, 3 * SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case 'd': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, -SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case 'D': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, -2 * SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case 'V': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, -3 * SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case '1': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-SPACING_FACTOR, SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case '2': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(SPACING_FACTOR, SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case '3': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-SPACING_FACTOR, -SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	case '4': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(SPACING_FACTOR, -SPACING_FACTOR));
		insertActionIntoSpawn(sprite, move);
		break;
	}
	}
}
void Dungeon::moveSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, char move) {
	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (maxrows - y)*SPACING_FACTOR - Y_OFFSET;

	// used for storing the current sprite's x and y coordinates
	Vec2 point;
	int px, py;

	// Key:
	// 1 u 2
	// l _ r
	// 3 d 4
	// Capital character (L R U D) means moves twice in that direction
	// < > ^ V means moves thrice in that direction

	for (int i = 0; i < sprites.size(); i++) {
		point = sprites[i]->getPosition();
		px = point.x;
		py = point.y;
		if (x_sprite == px && y_sprite == py) {
			switch (move) {
			case 'l': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, 0));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'L': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-2 * SPACING_FACTOR, 0));
				sprites.at(i)->runAction(move);
				break;
			}
			case '<': {
				auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-3 * SPACING_FACTOR, 0));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'r': {
				auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, 0));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'R': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(2 * SPACING_FACTOR, 0));
				move->setTarget(sprites.at(i));
				sprites.at(i)->runAction(move);
				break;
			}
			case '>': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(3 * SPACING_FACTOR, 0));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'u': {
				auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, SPACING_FACTOR));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'U': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, 2 * SPACING_FACTOR));
				sprites.at(i)->runAction(move);
				break;
			}
			case '^': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, 3 * SPACING_FACTOR));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'd': {
				auto move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -SPACING_FACTOR));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'D': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -2 * SPACING_FACTOR));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'V': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -3 * SPACING_FACTOR));
				sprites.at(i)->runAction(move);
				break;
			}
			case '1': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, SPACING_FACTOR));
				sprites.at(i)->runAction(move); break;
			}
			case '2': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, SPACING_FACTOR));
				sprites.at(i)->runAction(move); break;
			}
			case '3': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, -SPACING_FACTOR));
				sprites.at(i)->runAction(move); break;
			}
			case '4': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, -SPACING_FACTOR));
				sprites.at(i)->runAction(move); break;
			}
			}
			return;
		}
	}
}
void Dungeon::moveSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int cx, int cy) {
	// the position of the actual sprite on the screen
	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (maxrows - y)*SPACING_FACTOR - Y_OFFSET;

	// used for storing the current sprite's x and y coordinates
	Vec2 point;
	int px, py;

	for (int i = 0; i < sprites.size(); i++) {
		point = sprites[i]->getPosition();
		px = point.x;
		py = point.y;
		if (x_sprite == px && y_sprite == py) {
			cocos2d::Action* move = cocos2d::MoveTo::create(.1, cocos2d::Vec2(cx * SPACING_FACTOR - X_OFFSET, (maxrows - cy)*SPACING_FACTOR - Y_OFFSET));
			sprites.at(i)->runAction(move);

			return;
		}
	}
}
void Dungeon::moveSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y, char move) {
	// Vec2 contains the coordinates of the sprite we want to move

	// target sprite's position on the display
	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (maxrows - y)*SPACING_FACTOR - Y_OFFSET;

	// used for storing the current sprite's x and y coordinates
	Vec2 point;
	int px, py;

	// Key:
	// 1 u 2
	// l _ r
	// 3 d 4
	// Capital character (L R U D) means moves twice in that direction

	auto it = sprites.find(Vec2(x, y));
	if (it == sprites.end()) {
		int i = 0;
		return;
	}

	switch (move) {
	case 'l': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, 0));
		it->second->runAction(move);
		sprites.insert(std::make_pair(Vec2(x - 1, y), it->second));
		break;
	}
	case 'L': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-2 * SPACING_FACTOR, 0));
		it->second->runAction(move);
		sprites.insert(std::make_pair(Vec2(x + 1, y), it->second));
		break;
	}
	case 'r': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, 0));
		it->second->runAction(move);
		break;
	}
	case 'R': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(2 * SPACING_FACTOR, 0));
		it->second->runAction(move);
		break;
	}
	case 'u': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, SPACING_FACTOR));
		it->second->runAction(move);
		break;
	}
	case 'U': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, 2 * SPACING_FACTOR));
		it->second->runAction(move);
		break;
	}
	case 'd': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -SPACING_FACTOR));
		it->second->runAction(move);
		sprites.insert(std::make_pair(Vec2(x, y + 1), it->second));
		break;
	}
	case 'D': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -2 * SPACING_FACTOR));
		it->second->runAction(move);
		sprites.insert(std::make_pair(Vec2(x, y + 2), it->second));
		break;
	}
	case 'V': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -3 * SPACING_FACTOR));
		it->second->runAction(move);
		sprites.insert(std::make_pair(Vec2(x, y + 3), it->second));
		break;
	}
	case '1': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, SPACING_FACTOR));
		it->second->runAction(move); break;
	}
	case '2': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, SPACING_FACTOR));
		it->second->runAction(move); break;
	}
	case '3': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, -SPACING_FACTOR));
		it->second->runAction(move); break;
	}
	case '4': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, -SPACING_FACTOR));
		it->second->runAction(move); break;
	}
	}
	sprites.erase(it);

	/*
	for (auto &it = sprites.equal_range(Vec2(X, Y)).first; it != sprites.equal_range(Vec2(X, Y)).second; ++it) {
		point = it->second->getPosition();
		px = point.x;
		py = point.y;
		if (x_sprite == px && y_sprite == py) {
			switch (move) {
			case 'l': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, 0));
				it->second->runAction(move);
				break;
			}
			case 'L': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-2 * SPACING_FACTOR, 0));
				it->second->runAction(move);
				break;
			}
			case 'r': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, 0));
				it->second->runAction(move);
				break;
			}
			case 'R': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(2 * SPACING_FACTOR, 0));
				it->second->runAction(move);
				break;
			}
			case 'u': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, SPACING_FACTOR));
				it->second->runAction(move);
				break;
			}
			case 'U': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, 2 * SPACING_FACTOR));
				it->second->runAction(move);
				break;
			}
			case 'd': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -SPACING_FACTOR));
				it->second->runAction(move);
				break;
			}
			case 'D': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -2 * SPACING_FACTOR));
				it->second->runAction(move);
				break;
			}
			case '1': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, SPACING_FACTOR));
				it->second->runAction(move); break;
			}
			case '2': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, SPACING_FACTOR));
				it->second->runAction(move); break;
			}
			case '3': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, -SPACING_FACTOR));
				it->second->runAction(move); break;
			}
			case '4': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, -SPACING_FACTOR));
				it->second->runAction(move); break;
			}
			}
			return;
		}
	}
	*/
}
void Dungeon::moveAssociatedSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int X, int Y, int x, int y, char move) {
	// Vec2 contains the coordinates of the sprite that the sprite we want to move is associated with
	// X and Y are the "parent" sprite's coordinates
	// x and y are the coordinates of the sprite we want to move

	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (maxrows - y)*SPACING_FACTOR - Y_OFFSET;

	// used for storing the current sprite's x and y coordinates
	Vec2 point;
	int px, py;

	// Key:
	// 1 u 2
	// l _ r
	// 3 d 4
	// Capital character (L R U D) means moves twice in that direction

	for (auto &it = sprites.equal_range(Vec2(X, Y)).first; it != sprites.equal_range(Vec2(X, Y)).second; ++it) {
		point = it->second->getPosition();
		px = point.x;
		py = point.y;
		if (x_sprite == px && y_sprite == py) {
			switch (move) {
			case 'l': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, 0));
				it->second->runAction(move);
				break;
			}
			case 'L': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-2 * SPACING_FACTOR, 0));
				it->second->runAction(move);
				break;
			}
			case 'r': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, 0));
				it->second->runAction(move);
				break;
			}
			case 'R': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(2 * SPACING_FACTOR, 0));
				it->second->runAction(move);
				break;
			}
			case 'u': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, SPACING_FACTOR));
				it->second->runAction(move);
				break;
			}
			case 'U': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, 2 * SPACING_FACTOR));
				it->second->runAction(move);
				break;
			}
			case 'd': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -SPACING_FACTOR));
				it->second->runAction(move);
				break;
			}
			case 'D': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -2 * SPACING_FACTOR));
				it->second->runAction(move);
				break;
			}
			case '1': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, SPACING_FACTOR));
				it->second->runAction(move); break;
			}
			case '2': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, SPACING_FACTOR));
				it->second->runAction(move); break;
			}
			case '3': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-SPACING_FACTOR, -SPACING_FACTOR));
				it->second->runAction(move); break;
			}
			case '4': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, -SPACING_FACTOR));
				it->second->runAction(move); break;
			}
			}
			return;
		}
	}
}

// Create new sprites
cocos2d::Sprite* Dungeon::createSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image) {
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithSpriteFrameName(image);
	m_scene->addChild(sprite, z);
	sprite->setPosition(x*SPACING_FACTOR - X_OFFSET, (maxrows - y)*SPACING_FACTOR - Y_OFFSET);
	sprites.push_back(sprite);

	return sprite;
}
void Dungeon::addSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image) {
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithSpriteFrameName(image);
	m_scene->addChild(sprite, z);
	sprite->setPosition(x*SPACING_FACTOR - X_OFFSET, (maxrows - y)*SPACING_FACTOR - Y_OFFSET);
	sprites.push_back(sprite);
}
void Dungeon::addSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y, int z, std::string image) {
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithSpriteFrameName(image);
	m_scene->addChild(sprite, z);
	sprite->setPosition(x*SPACING_FACTOR - X_OFFSET, (maxrows - y)*SPACING_FACTOR - Y_OFFSET);
	sprites.insert(std::pair<cocos2d::Vec2, cocos2d::Sprite*>(Vec2(x, y), sprite));
}
void Dungeon::addGoldSprite(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y) {
	int gold = dungeon[y*maxcols + x].gold;

	// if there's gold here already, remove the sprite before we add a new one
	if (gold != 0) {
		removeSprite(money_sprites, maxrows, x, y);
	}

	std::string image;

	if (gold == 1) image = "Gold_Coin1_48x48.png";
	else if (gold == 2) image = "Gold_Coin2_48x48.png";
	else if (gold == 3) image = "Gold_Coin3_48x48.png";
	else if (gold >= 4 && gold <= 10) image = "Gold_Pile1_48x48.png";
	else if (gold > 10) image = "Gold_Pile2_48x48.png";

	cocos2d::Sprite* money = cocos2d::Sprite::createWithSpriteFrameName(image);
	m_scene->addChild(money, -3);
	money->setPosition(x*SPACING_FACTOR - X_OFFSET, (maxrows - y)*SPACING_FACTOR - Y_OFFSET);
	money_sprites.push_back(money);
}

// Remove sprites
void Dungeon::queueRemoveSprite(int x, int y, std::vector<std::shared_ptr<Monster>> monsters) {
	cocos2d::Sprite* sprite = monsters.at(findMonster(x, y, monsters))->getSprite();
	auto remove = cocos2d::RemoveSelf::create();

	insertActionIntoSpawn(sprite, remove);
}
void Dungeon::queueRemoveSprite(int x, int y, cocos2d::Sprite* sprite) {
	auto remove = cocos2d::RemoveSelf::create();
	insertActionIntoSpawn(sprite, remove);
}
void Dungeon::removeSprite(std::vector<cocos2d::Sprite*> &sprites, int maxrows, int x, int y) {
	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (maxrows - y)*SPACING_FACTOR - Y_OFFSET;

	// used for storing the current sprite's x and y coordinates
	Vec2 point;
	int px, py;

	for (int i = 0; i < sprites.size(); i++) {
		point = sprites.at(i)->getPosition();
		px = point.x;
		py = point.y;
		if (x_sprite == px && y_sprite == py) {
			sprites.at(i)->removeFromParent();
			sprites.erase(sprites.begin() + i);
			return;
		}
	}
}
void Dungeon::removeSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int x, int y) {
	auto it = sprites.find(Vec2(x, y));
	it->second->removeFromParent();
	sprites.erase(it);
}
void Dungeon::removeAssociatedSprite(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> &sprites, int maxrows, int X, int Y, int x, int y) {
	// Vec2 contains the coordinates of the sprite that the sprite we want to delete is associated with
	// X and Y are the "parent" sprite's coordinates
	// x and y are the coordinates of the sprite we want to delete

	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (maxrows - y)*SPACING_FACTOR - Y_OFFSET;

	// used for storing the current sprite's x and y coordinates
	Vec2 point;
	int px, py;

	for (auto &it = sprites.equal_range(Vec2(X, Y)).first; it != sprites.equal_range(Vec2(X, Y)).second; ++it) {
		point = it->second->getPosition();
		px = point.x;
		py = point.y;
		if (x_sprite == px && y_sprite == py) {
			it->second->removeFromParent();
			sprites.erase(it);
			return;
		}
	}
}

// update sprite coloring (lighting)
void Dungeon::updateLighting(Dungeon &dungeon, std::vector<std::shared_ptr<Monster>> &monsters, std::vector<std::shared_ptr<Objects>> &traps) {
	int rows = dungeon.getRows();

	Vec2 pos = player_sprite->getPosition();
	int x = pos.x;
	int y = pos.y;
	int cutoff = 2400;

	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int p_cutoff = 7;

	cocos2d::Sprite* sprite;
	int sx, sy, dist, color;

	//sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR;
	//sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR);

	// monster sprites

	/*
	for (int i = 0; i < monster_sprites.size(); i++) {
		sx = (monster_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = getRows() - ((monster_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - px) + abs(sy - py);

		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (dist > p_cutoff) {
			monster_sprites.at(i)->setColor(cocos2d::Color3B(0, 0, 0));
		}
		else {
			color = (255 * formula);
			monster_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}
	*/

	int mx, my;
	for (int i = 0; i < monsters.size(); i++) {
		mx = monsters.at(i)->getPosX();
		my = monsters.at(i)->getPosY();
		dist = abs(mx - px) + abs(my - py);

		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (dist > p_cutoff) {
			monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(0, 0, 0));
		}
		else {
			color = (255 * formula);
			monsters.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// floor sprites
	for (int i = 0; i < floor_sprites.size(); i++) {
		sx = (floor_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((floor_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (dist > p_cutoff) {
			floor_sprites.at(i)->setColor(cocos2d::Color3B(0, 0, 0));
		}
		else {

			color = (255 * formula);
			floor_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// wall sprites
	for (int i = 0; i < wall_sprites.size(); i++) {
		//sx = wall_sprites.at(i)->getPosition().x;
		//sy = wall_sprites.at(i)->getPosition().y;
		sx = (wall_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((wall_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (dist > p_cutoff) {
			wall_sprites.at(i)->setColor(cocos2d::Color3B(0, 0, 0));
		}
		else {
			color = (255 * formula);
			wall_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// trap sprites
	for (int i = 0; i < traps.size(); i++) {
		sx = (traps.at(i)->getSprite()->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((traps.at(i)->getSprite()->getPosition().y + Y_OFFSET) / SPACING_FACTOR);
		dist = abs(sx - px) + abs(sy - py);

		float formula = (p_cutoff - dist) / (float)p_cutoff;

		// if trap is lava or other light-emitting source, dampen the lighting effect
		if (traps.at(i)->getItem() == "Lava") {
			int cutoff = p_cutoff + 6;
			formula = (cutoff - dist) / (float)(cutoff);
			if (dist >= cutoff) {
				traps.at(i)->getSprite()->setColor(cocos2d::Color3B(19, 19, 19)); // color value calculated by taking (1/cutoff) * 255
			}
			else {
				color = (255 * formula);
				traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}
		else {
			if (dist > p_cutoff) {
				traps.at(i)->getSprite()->setColor(cocos2d::Color3B(0, 0, 0));
			}
			else {
				color = (255 * formula);
				traps.at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}
	}

	/*for (int i = 0; i < trap_sprites.size(); i++) {
		sx = (trap_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = getRows() - ((trap_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);
		dist = abs(sx - px) + abs(sy - py);

		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (dist > p_cutoff) {
			trap_sprites.at(i)->setColor(cocos2d::Color3B(0, 0, 0));
		}
		else {
			color = (255 * formula);
			trap_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}*/

	// item sprites
	for (int i = 0; i < item_sprites.size(); i++) {
		sx = (item_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((item_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (dist > p_cutoff) {
			item_sprites.at(i)->setColor(cocos2d::Color3B(0, 0, 0));
		}
		else {

			color = (255 * formula);
			item_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// money sprites
	for (int i = 0; i < money_sprites.size(); i++) {
		sx = (money_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((money_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (dist > p_cutoff) {
			money_sprites.at(i)->setColor(cocos2d::Color3B(0, 0, 0));
		}
		else {
			color = (255 * formula);
			money_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}
}

// run sprite actions
int Dungeon::spriteFound(cocos2d::Sprite* sprite) {
	for (int i = 0; i < m_seq.size(); i++) {
		if (m_seq[i].first == sprite)
			return i;
	}
	return -1;
}
void Dungeon::insertActionIntoSpawn(cocos2d::Sprite* sprite, cocos2d::FiniteTimeAction* action, bool front) {
	// if sprite does not have any actions associated with it yet, then add it
	//if (m_spawn.find(sprite) == m_spawn.end()) {
	//	cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
	//	v.pushBack(action);

	//	m_spawn.insert(std::make_pair(sprite, v));
	//}
	//// else if the sprite already exists, just add to the vector
	//else {
	//	auto it = m_spawn.find(sprite);
	//	it->second.pushBack(action);
	//}

	//m_seq.push_back(std::make_pair(sprite, action));

	int i = spriteFound(sprite);

	if (i == -1) {
		cocos2d::Vector<cocos2d::FiniteTimeAction*> v;
		v.pushBack(action);
		if (!front) {
			m_seq.push_back(std::make_pair(sprite, v));
		}
		else {
			m_seq.insert(m_seq.begin(), std::make_pair(sprite, v));
		}
	}
	else {
		m_seq.at(i).second.pushBack(action);
	}
}
void Dungeon::runSpawn() {
	//if (!m_spawn.empty()) {
	//	for (auto &it : m_spawn) {
	//		// create a sequence for the given sprite
	//		cocos2d::Sequence* seq = cocos2d::Sequence::create(it.second);

	//		// run the sequence on the sprite
	//		it.first->runAction(seq);
	//	}
	//	m_spawn.clear();
	//}


	if (!m_seq.empty()) {
		for (auto &it : m_seq) {
			// create a sequence for the given sprite
			cocos2d::Sequence* seq = cocos2d::Sequence::create(it.second);

			it.first->runAction(seq);
			//it.first->runAction(it.second);
		}
		m_seq.clear();
	}
}

/// unused
void Dungeon::unmarkTiles() {
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 70; j++) {
			m_maze[i*MAXCOLS + j].marked = false;
		}
	}
}

std::vector<char> Dungeon::topChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one = { {'#', '#', '#', '#'},
								 {'#', '#', '#', '#'},
								 {' ', ' ', ' ', ' '},
								 {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> two = { {'#', '#', '#', '#'},
								 {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> three = { {' ', '#', '#', ' '},
								   {' ', '#', '#', ' '},
								   {' ', '#', '#', ' '},
								   {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> four = { {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {'#', ' ', ' ', '#'} };

	std::vector<std::vector<char>> five = { {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> six = {  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> seven = { {' ', ' ', '#', ' '},
								   {'#', ' ', ' ', ' '},
								   {' ', ' ', '#', ' '},
								   {'#', ' ', '#', ' '} };

	std::vector<std::vector<char>> eight = { {'#', '#', '#', '#'},
								   {' ', ' ', ' ', '#'},
								   {'#', ' ', ' ', ' '},
								   {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> ten = {  {'#', ' ', ' ', '#'},
								  {'#', ' ', ' ', '#'},
								  {'#', ' ', ' ', '#'},
								  {'#', ' ', ' ', '#'} };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#'},
								    {' ', ' ', ' ', ' '},
								    {' ', ' ', ' ', ' '},
								    {'#', '#', '#', '#'} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	c.push_back(eleven);
	
	c = mixChunks(c, maxcols);

	return combineChunks(c);
}
std::vector<char> Dungeon::middleChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one = { {'#', '#', '#', '#'},
								 {'#', ' ', ' ', ' '},
								 {' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', '#'} };

	std::vector<std::vector<char>> two = { {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' '},
								 {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> three = { {'#', ' ', '#', '#'},
								   {' ', ' ', ' ', '#'},
								   {' ', ' ', ' ', '#'},
								   {'#', ' ', '#', '#'} };

	std::vector<std::vector<char>> four = { {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> five = { {'#', ' ', ' ', '#'},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {'#', ' ', ' ', '#'} };

	std::vector<std::vector<char>> six = { {' ', ' ', ' ', ' '},
							     {' ', ' ', ' ', ' '},
							     {' ', ' ', ' ', ' '},
								 {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> seven = { {' ', ' ', '#', '#'},
								   {' ', ' ', ' ', '#'},
								   {'#', '#', ' ', ' '},
								   {'#', '#', ' ', ' '} };

	std::vector<std::vector<char>> eight = { {'#', ' ', '#', '#'},
								  {'#', ' ', '#', '#'},
								  {' ', ' ', ' ', ' '},
								  {'#', ' ', '#', '#'} };

	std::vector<std::vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> ten =  { {' ', ' ', '#', ' '},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '} };


	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	//c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	//c.push_back();

	c = mixChunks(c, maxcols);

	return combineChunks(c);
}
std::vector<char> Dungeon::bottomChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one = { {' ', ' ', ' ', ' '},
								 {' ', '#', '#', ' '},
								 {' ', '#', ' ', ' '},
								 {'#', '#', ' ', '#'} };

	std::vector<std::vector<char>> two = { {' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', '#'},
								 {'#', ' ', ' ', '#'},
								 {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> three = { {'#', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '},
								   {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> four = { {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> five = { {'#', ' ', ' ', '#'},
								  {'#', ' ', ' ', '#'},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', ' ', '#'} };

	std::vector<std::vector<char>> six = {  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> seven = { {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '},
								   {'#', '#', ' ', ' '} };

	std::vector<std::vector<char>> eight = { {' ', ' ', '#', '#'},
								   {' ', ' ', '#', '#'},
								   {' ', '#', '#', '#'},
								   {' ', '#', '#', '#'} };

	std::vector<std::vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> ten = { {' ', ' ', '#', ' '},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> eleven = { {' ', ' ', ' ', '#'},
								    {' ', ' ', ' ', '#'},
								    {' ', '#', ' ', '#'},
								    {' ', '#', '#', '#'} };

	std::vector<std::vector<char>> specialroom2 = { {'#', '#', '#', '#'},
										  {'#', 'W', 'W', '#'},
										  {'#', 'W', 'W', '#'},
										  {'#', '#', '#', '#'} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	c.push_back(eleven);
	//c.push_back(specialroom1);

	c = mixChunks(c, maxcols);

	return combineChunks(c);
}
std::vector<std::vector<std::vector<char>>> Dungeon::mixChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols) {
	std::vector<std::vector<std::vector<char>>> v;
	std::vector<std::vector<char>> specialroom1 = { {'#', '#', '#', '#'},
										  {'#', 'G', ' ', '#'},
										  {'#', c_SILVER_CHEST, c_SILVER_CHEST, '#'},
										  {'#', '#', '#', '#'} };
	std::vector<std::vector<char>> specialroom2 = { {'#', '#', '#', '#'},
										  {'#', 'W', 'W', '#'},
										  {'#', 'W', 'W', '#'},
										  {'#', '#', '#', '#'} };
	int s;

	for (int i = 0; i < (maxcols-2) / 4; i++)
		v.push_back(c[randInt(c.size())]);

	if (layer == specialChunkLayer1) {
		s = randInt((maxcols - 2) / 4 - 1);
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom1);
	}
	if (layer == specialChunkLayer2) {
		s = randInt((maxcols - 2) / 4 - 1);
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom2);
	}

	return v;
}
std::vector<char> Dungeon::combineChunks(std::vector<std::vector<std::vector<char>>> c) {
	std::vector<char> v;
	for (int i = 0; i < 4; i++) {			// iterate thru rows of 2d vector
		for (int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 4; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}


//	:::: FIRST SHOP ::::
FirstShop::FirstShop(Player p) : Dungeon(2) {
	_Tile *tile;
	// initialize tiles
	for (int i = 0; i < m_rows; i++) {
		for (int j = 0; j < m_cols; j++) {
			tile = new _Tile;
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = SPACE;
			tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = false;
			tile->price = tile->gold = 0;
			tile->marked = false;
			tile->object = nullptr;
			tile->item_name = tile->shop_action = "";

			m_shop1.push_back(*tile);
			delete tile;
		}
	}
	

	// get the shop layout
	std::vector<char> v = generate();
	int rows = MAXROWS_SHOP1;
	int cols = MAXCOLS_SHOP1;
	char top, bottom, traptile;

	// begin mapping shop
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			switch (v.at(i*cols + j)) {
			case PLAYER: {
				m_shop1[i*cols + j].hero = true;
				//m_shop1[i*cols + j].bottom = SPACE;
				player.at(0) = p;
				player.at(0).setPosX(j);
				player.at(0).setPosY(i);
				player.at(0).setMoney(100);
				break;
			}
			case '#': {
				m_shop1[i*cols + j].top = m_shop1[i*cols + j].bottom = m_shop1[i*cols + j].traptile = UNBREAKABLE_WALL;
				m_shop1[i*cols + j].wall = true;
				break;
			}
			case DOOR_V: {
				m_shop1[i*cols + j].top = DOOR_V;
				m_shop1[i*cols + j].bottom = SPACE;
				m_shop1[i*cols + j].traptile = DOOR_V;
				m_shop1[i*cols + j].wall = true;
				
				Door door;
				door.setPosX(j);
				door.setPosY(i);
				m_shopActives.emplace_back(new Door(door));
				break;
			}
			case STAIRS: {
				m_shop1[i*cols + j].top = m_shop1[i*cols + j].bottom = SPACE;
				m_shop1[i*cols + j].traptile = STAIRS;
				m_shop1[i*cols + j].trap = true;
				break;
			}
				// purchase spot
			case 'p': {
				m_shop1[i*cols + j].shop_action = PURCHASE;
				break;
			}
				// item price
			case '$': {

				break;
			}
				// shop counter
			case 'c': {
				m_shop1[i*cols + j].shop_action = "countertop";
				m_shop1[i*cols + j].wall = true;
				break;
			}
				// shopkeeper
			case 'k': {
				m_shop1[i*cols + j].shop_action = "shopkeeper";
				m_shop1[i*cols + j].wall = true;
				break;
			}
				// breakable object
			case 'd': {
				m_shop1[i*cols + j].shop_action = "breakable";
				break;
			}
				// secret thing
			case 'b': {
				m_shop1[i*cols + j].shop_action = "secret";
				break;
			}
					  // 1, 2, 3 : hp, shield repair, bombs
			case '1': {
				switch (randInt(2)) {
				case 0: m_shop1[i*cols + j].object = std::make_shared<LifePotion>(); m_shop1[i*cols + j].item_name = LIFEPOT; m_shop1[i*cols + j].price = 25; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<ShieldRepair>(); m_shop1[i*cols + j].item_name = SHIELD_REPAIR; m_shop1[i*cols + j].price = 15; break;
				}
				m_shop1[i*cols + j].shop_action = "shop_item";
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				break;
			}
			case '2': {
				switch (randInt(2)) {
				case 0: m_shop1[i*cols + j].object = std::make_shared<ShieldRepair>(); m_shop1[i*cols + j].item_name = SHIELD_REPAIR; m_shop1[i*cols + j].price = 15; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<Bomb>(); m_shop1[i*cols + j].item_name = BOMB; m_shop1[i*cols + j].price = 20; break;
				}
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
			case '3': {
				switch (randInt(2)) {
				case 0: m_shop1[i*cols + j].object = std::make_shared<LifePotion>(); m_shop1[i*cols + j].item_name = LIFEPOT; m_shop1[i*cols + j].price = 25; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<Bomb>(); m_shop1[i*cols + j].item_name = BOMB; m_shop1[i*cols + j].price = 20; break;
				}
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
					  // 4, 5 : random items
			case '4': {
				switch (randInt(4)) {
				case 0: m_shop1[i*cols + j].object = std::make_shared<BronzeDagger>(); m_shop1[i*cols + j].item_name = BRONZE_DAGGER; m_shop1[i*cols + j].price = 30; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<IronLance>(); m_shop1[i*cols + j].item_name = IRON_LANCE; m_shop1[i*cols + j].price = 50; break;
				case 2: m_shop1[i*cols + j].item_name = WOOD_SHIELD; m_shop1[i*cols + j].price = 28;
					m_shop1[i*cols + j].object = std::make_shared<WoodShield>(j, i); break;
				case 3: m_shop1[i*cols + j].item_name = IRON_SHIELD; m_shop1[i*cols + j].price = 45;
					m_shop1[i*cols + j].object = std::make_shared<IronShield>(j, i); break;
				}
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
			case '5': {
				switch (randInt(2)) {
				case 0: m_shop1[i*cols + j].object = std::make_shared<LifePotion>(); m_shop1[i*cols + j].item_name = LIFEPOT; m_shop1[i*cols + j].price = 25; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<Bomb>(); m_shop1[i*cols + j].item_name = BOMB; m_shop1[i*cols + j].price = 20; break;
				}
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
					  // 6 : random chest
			case '6': {
				int n = 1 + randInt(100);
				
				// 60% chance
				if (n <= 60) { m_shop1[i*cols + j].object = std::make_shared<BrownChest>(); m_shop1[i*cols + j].item_name = BROWN_CHEST; m_shop1[i*cols + j].price = 20; }
				// 35% chance
				else if (n > 60 && n <= 95) { m_shop1[i*cols + j].object = std::make_shared<SilverChest>(); m_shop1[i*cols + j].item_name = SILVER_CHEST; m_shop1[i*cols + j].price = 40; }
				// 5% chance
				else if (n > 95) { m_shop1[i*cols + j].object = std::make_shared<GoldenChest>(); m_shop1[i*cols + j].item_name = GOLDEN_CHEST; m_shop1[i*cols + j].price = 80; }

				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
			}
		}
	}
}
FirstShop& FirstShop::operator=(FirstShop const &dungeon) {
	m_shop1 = dungeon.m_shop1;
	player = dungeon.player;
	m_level = dungeon.m_level;

	m_shopMonsters = dungeon.m_shopMonsters;
	m_shopActives = dungeon.m_shopActives;
	m_traps = dungeon.m_traps;

	return *this;
}

void FirstShop::peekDungeon(int x, int y, char move) {
	char top, projectile, bottom, traptile;
	bool wall, item, trap, enemy;

	int rows = m_rows;
	int cols = m_cols;
	std::string shop_action = m_shop1[y*cols + x].shop_action;

	auto scene = dynamic_cast<Shop1Scene*>(m_scene);

	int initHP = player.at(0).getHP();

	// reset player's blocking stance
	player.at(0).setBlock(false);

	// set player's facing direction
	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd':
		player.at(0).setFacingDirection(move); break;
	}

	// check for any afflictions
	player.at(0).checkAfflictions();

	// :::: Move player ::::
	int weprange = player.at(0).getWeapon()->getRange();
	bool move_used = false;
	bool far_enemy, close_enemy;

	// if weapon has a range of 2, check for monsters to attack
	if (weprange == 2) {

		if (move == 'l') { // moved to the left
			top = m_shop1[y*cols + x - 1].top;
			close_enemy = m_shop1[y*cols + x - 1].enemy;
			far_enemy = m_shop1[y*cols + x - 2].enemy;

			if (close_enemy) {
				fight(*this, x - 1, y);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && y*cols + x - 2 > 0) {	// monster encountered
				fight(*this, x - 2, y);
				move_used = true;
			}
		}
		else if (move == 'r') {
			top = m_shop1[y*cols + x + 1].top;
			close_enemy = m_shop1[y*cols + x + 1].enemy;
			far_enemy = m_shop1[y*cols + x + 2].enemy;

			if (close_enemy) {
				fight(*this, x + 1, y);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && y*cols + x + 2 < rows * cols) {
				fight(*this, x + 2, y);
				move_used = true;
			}
		}
		else if (move == 'u') { // moved up
			top = m_shop1[(y - 1)*cols + x].top;
			close_enemy = m_shop1[(y - 1)*cols + x].enemy;
			// if player is along top edge, assign far_enemy to close_enemy instead
			far_enemy = (y == 1 ? close_enemy : m_shop1[(y - 2)*cols + x].enemy);

			if (close_enemy) {
				fight(*this, x, y - 1);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && (y - 2)*cols + x > 0) {
				fight(*this, x, y - 2);
				move_used = true;
			}
		}
		else if (move == 'd') { // moved down
			top = m_shop1[(y + 1)*cols + x].top;
			close_enemy = m_shop1[(y + 1)*cols + x].enemy;
			far_enemy = (y == rows - 2 ? close_enemy : m_shop1[(y + 2)*cols + x].enemy);

			if (close_enemy) {
				fight(*this, x, y + 1);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && (y + 2)*cols + x < rows * cols) {
				fight(*this, x, y + 2);
				move_used = true;
			}
		}
	}

	// otherwise, check the action taken
	if (!move_used) {

		if (move == 'l') { // moved to the left
			top = m_shop1[y*cols + x - 1].top;
			bottom = m_shop1[y*cols + x - 1].bottom;
			traptile = m_shop1[y*cols + x - 1].trap;

			wall = m_shop1[y*cols + x - 1].wall;
			item = m_shop1[y*cols + x - 1].item;
			trap = m_shop1[y*cols + x - 1].trap;
			enemy = m_shop1[y*cols + x - 1].enemy;

			if (!(wall || enemy)) {
				// move character to the left
				m_shop1[y*cols + x].hero = false;
				m_shop1[y*cols + x - 1].hero = true;
				player.at(0).setPosX(x - 1);

				if (trap) {
					; // check for traps
				}
				if (item) {
					foundItem(m_shop1, cols, x - 1, y);
				}
				if (m_shop1[y*cols + x - 1].shop_action == PURCHASE) {
					// if there's a price already floating, remove before showing the new one
					if (scene->itemprice != nullptr)
						scene->deconstructShopHUD();

					scene->showShopHUD(*this, x - 1, y);
				}
				else if (scene->itemprice != nullptr && m_shop1[y*cols + x - 1].shop_action != PURCHASE) {
					scene->deconstructShopHUD();
				}
			}
			else if (wall) {
				if (top == DOOR_H || top == DOOR_V) {
					openDoor(*this, x - 1, y);
				}
				else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
					m_shop1[y*cols + x].hero = false;
					m_shop1[y*cols + x - 1].hero = true;
					player.at(0).setPosX(x - 1);
				}
				else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
					// play locked door sound
					cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
				}
			}
			else if (enemy) {	// monster encountered
				fight(*this, x - 1, y);
			}
		}

		else if (move == 'r') { // moved to the right
			top = m_shop1[y*cols + x + 1].top;
			bottom = m_shop1[y*cols + x + 1].bottom;
			traptile = m_shop1[y*cols + x + 1].trap;

			wall = m_shop1[y*cols + x + 1].wall;
			item = m_shop1[y*cols + x + 1].item;
			trap = m_shop1[y*cols + x + 1].trap;
			enemy = m_shop1[y*cols + x + 1].enemy;

			if (!(wall || enemy)) {
				// move character to the right
				m_shop1[y*cols + x].hero = false;
				m_shop1[y*cols + x + 1].hero = true;
				player.at(0).setPosX(x + 1);

				if (trap) {
					; // check for traps
				}
				if (item) {
					foundItem(m_shop1, cols, x + 1, y);
				}
				if (m_shop1[y*cols + x + 1].shop_action == PURCHASE) {
					// if there's a price already floating, remove before showing the new one
					if (scene->itemprice != nullptr)
						scene->deconstructShopHUD();

					scene->showShopHUD(*this, x + 1, y);
				}
				else if (scene->itemprice != nullptr && m_shop1[y*cols + x + 1].shop_action != PURCHASE) {
					scene->deconstructShopHUD();
				}
			}
			else if (wall) {
				if (top == DOOR_H || top == DOOR_V) {
					openDoor(*this, x + 1, y);
				}
				else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
					m_shop1[y*cols + x].hero = false;
					m_shop1[y*cols + x + 1].hero = true;
					player.at(0).setPosX(x + 1);
				}
				else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
					// play locked door sound
					cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
				}
			}
			else if (enemy) {
				fight(*this, x + 1, y);
			}
		}

		else if (move == 'u') { // moved up
			top = m_shop1[(y - 1)*cols + x].top;
			bottom = m_shop1[(y - 1)*cols + x].bottom;
			traptile = m_shop1[(y - 1)*cols + x].traptile;

			wall = m_shop1[(y - 1)*cols + x].wall;
			item = m_shop1[(y - 1)*cols + x].item;
			trap = m_shop1[(y - 1)*cols + x].trap;
			enemy = m_shop1[(y - 1)*cols + x].enemy;

			if (!(wall || enemy)) {
				m_shop1[y*cols + x].hero = false;
				m_shop1[(y - 1)*cols + x].hero = true;
				player.at(0).setPosY(y - 1);

				if (trap) {
					;
				}
				if (item) {
					foundItem(m_shop1, cols, x, y - 1);
				}
				if (m_shop1[(y - 1)*cols + x].shop_action == PURCHASE) {
					if (scene->itemprice != nullptr)
						scene->deconstructShopHUD();

					scene->showShopHUD(*this, x, y - 1);
				}
				else if (scene->itemprice != nullptr && m_shop1[(y - 1)*cols + x].shop_action != PURCHASE) {
					scene->deconstructShopHUD();
				}
			}
			else if (wall) {
				if (top == DOOR_H || top == DOOR_V) {
					openDoor(*this, x, y - 1);
				}
				else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
					m_shop1[y*cols + x].hero = false;
					m_shop1[(y - 1)*cols + x].hero = true;
					player.at(0).setPosY(y - 1);
				}
				else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
					// play locked door sound
					cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
				}
			}
			else if (enemy) {
				fight(*this, x, y - 1);
			}
		}

		else if (move == 'd') { // moved down
			top = m_shop1[(y + 1)*cols + x].top;
			bottom = m_shop1[(y + 1)*cols + x].bottom;
			traptile = m_shop1[(y + 1)*cols + x].traptile;

			wall = m_shop1[(y + 1)*cols + x].wall;
			item = m_shop1[(y + 1)*cols + x].item;
			trap = m_shop1[(y + 1)*cols + x].trap;
			enemy = m_shop1[(y + 1)*cols + x].enemy;

			if (!(wall || enemy)) {
				m_shop1[y*cols + x].hero = false;
				m_shop1[(y + 1)*cols + x].hero = true;
				player.at(0).setPosY(y + 1);

				if (trap) {
					;
				}
				if (item) {
					foundItem(m_shop1, cols, x, y + 1);
				}
				if (m_shop1[(y + 1)*cols + x].shop_action == PURCHASE) {
					if (scene->itemprice != nullptr)
						scene->deconstructShopHUD();

					scene->showShopHUD(*this, x, y + 1);
				}
				else if (scene->itemprice != nullptr && m_shop1[(y + 1)*cols + x].shop_action != PURCHASE) {
					scene->deconstructShopHUD();
				}
			}
			else if (wall) {
				if (top == DOOR_H || top == DOOR_V) {
					openDoor(*this, x, y + 1);
				}
				else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
					m_shop1[y*cols + x].hero = false;
					m_shop1[(y + 1)*cols + x].hero = true;
					player.at(0).setPosY(y + 1);
				}
				else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
					// play locked door sound
					cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
				}
			}
			else if (enemy) {
				fight(*this, x, y + 1);
			}
		}

		else if (move == 'e') {
			// do this if item needs to be purchased
			if (shop_action == "purchase") {
				purchaseItem(m_shop1, x, y);
			}
			// else if the item purchased was a chest, do this
			else if (shop_action == "chest purchased") {
				m_shop1[y*cols + x].item_name = "";
				removeSprite(item_sprites, rows, x, y - 1);
				collectItem(m_shop1, rows, cols, x, y);
			}
			// else if there's just an item sitting on the ground, do this
			else {
				collectItem(m_shop1, rows, cols, x, y);
			}

			if (m_level > 2)
				return;
		}

		else if (move == 'b') {
			if (player.at(0).hasShield()) {
				// play setting up shield stance sound effect
				cocos2d::experimental::AudioEngine::play2d("Shield_Stance.mp3", false, 1.0f);

				player.at(0).setBlock(true);
			}
		}
	}

	// queue player move
	queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY());

	// pick up any gold/money that was on the ground
	if (m_shop1[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
		goldPickup(m_shop1, rows, cols, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// check active items in play
	checkActive(m_shop1, rows, cols, m_shopActives, m_shopMonsters);

	//	if player is dead then return
	if (player.at(0).getHP() <= 0) {
		//m_shop1[y*cols + x].top = 'X'; // change player icon to X to indicate death

		//clearScreen();
		//showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}

	// flash player sprite if took damage
	if (player.at(0).getHP() < initHP) {
		auto tintRed = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 70, 70));
		auto tintNormal = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 255, 255));
		player.at(0).getSprite()->runAction(cocos2d::Blink::create(0.2, 4));
		player.at(0).getSprite()->runAction(cocos2d::Sequence::createWithTwoActions(tintRed, tintNormal));
	}
}

void FirstShop::purchaseItem(std::vector<_Tile> &dungeon, int x, int y) {
	int cols = m_cols;
	int rows = m_rows;
	std::string item = dungeon[(y-1)*cols + x].item_name;
	int playerGold = player.at(0).getMoney();
	int price = dungeon[(y-1)*cols + x].price;

	int items = player.at(0).getItems().size();
	int maxItems = player.at(0).getMaxItemInvSize();

	// if player has enough money, let them buy it
	if (playerGold >= price) {
		// purchase sound
		cocos2d::experimental::AudioEngine::play2d("Purchase_Item1.mp3", false, 1.0f);

		player.at(0).setMoney(playerGold - price);
		dungeon[y*cols + x].shop_action = ""; // removes purchase ability
		collectItem(dungeon, rows, cols, x, y - 1);

		// remove HUD after purchasing
		auto scene = dynamic_cast<Shop1Scene*>(m_scene);
		scene->deconstructShopHUD();
	}
	// else play insufficient funds sound
	else {
		cocos2d::experimental::AudioEngine::play2d("Insufficient_Funds.mp3", false, 1.0f);
	}
}
void FirstShop::collectItem(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y) {
	// :::: If coming from purchase item, y is equal to py-1 where py is the player's y position ::::

	// :::: If collectItem is called directly, then there is no y shift ::::

	int current_inv_size = player.at(0).getWeapons().size() + player.at(0).getItems().size();
	int current_wep_inv_size = player.at(0).getWeapons().size();
	int current_item_inv_size = player.at(0).getItems().size();
	int max_wep_inv_size = player.at(0).getMaxWeaponInvSize();
	int max_item_inv_size = player.at(0).getMaxItemInvSize();
	std::string image;

	char bottom = dungeon[y*maxcols + x].bottom;
	char traptile = dungeon[y*maxcols + x].traptile;
	std::string item_name = dungeon[y*maxcols + x].item_name;
	std::string shop_action = dungeon[y*maxcols + x].shop_action;

	//		WEAPONS
	if (item_name == RUSTY_CUTLASS) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(std::make_shared<RustyCutlass>(x, y));

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup2.mp3", false, 1.0f);

			//dungeonText.push_back("You pick up the Rusty Cutlass.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (item_name == BONE_AXE) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(std::make_shared<BoneAxe>(x, y)); //adds bone axe to inventory

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup2.mp3", false, 1.0f);

			//dungeonText.push_back("You pick up the Bone Axe.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (item_name == BRONZE_DAGGER) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(std::make_shared<BronzeDagger>());

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup.mp3", false, 1.0f);

			//dungeonText.push_back("You pick up the Bronze Dagger.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (item_name == IRON_LANCE) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(std::make_shared<IronLance>());

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup.mp3", false, 1.0f);

			//dungeonText.push_back("You pick up the Iron Lance.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}

	//		SHIELDS
	else if (item_name == WOOD_SHIELD) {
		// save shield at this spot, overwritten if shield is dropped
		Shield newShield = *(dynamic_pointer_cast<WoodShield>(dungeon[y*maxcols + x].object));
		std::string shieldimage = player.at(0).getShield().getImageName();

		// if player has a shield, then drop and replace with new one
		if (player.at(0).hasShield()) {
			// handles the placement of shield on the ground
			player.at(0).dropShield(dungeon, maxrows, maxcols);
			removeSprite(item_sprites, maxrows, x, y);

			player.at(0).equipShield(newShield);
			
			// if player bought shield, place shield image at y+1
			if (shop_action == "shop_item") {
				dungeon[y*maxcols + x].shop_action = "";
				y++;
			}
			addSprite(item_sprites, maxrows, x, y, -1, shieldimage); // adds sprite of player's old shield

			return;
		}
		// else just equip the new one
		player.at(0).equipShield(newShield);
		player.at(0).setShieldPossession(true);

		dungeon[y*maxcols + x].item_name = EMPTY;
		dungeon[y*maxcols + x].object = nullptr;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
	else if (item_name == IRON_SHIELD) {
		// save shield at this spot, overwritten if shield is dropped
		Shield newShield = *(dynamic_pointer_cast<IronShield>(dungeon[y*maxcols + x].object));
		std::string shieldimage = player.at(0).getShield().getImageName();

		// if player has a shield, then drop and replace with new one
		if (player.at(0).hasShield()) {
			// handles the placement of shield on the ground
			player.at(0).dropShield(dungeon, maxrows, maxcols);
			removeSprite(item_sprites, maxrows, x, y);

			player.at(0).equipShield(newShield);

			// if player bought shield, place shield image at y+1
			if (shop_action == "shop_item") {
				dungeon[y*maxcols + x].shop_action = "";
				y++;
			}
			addSprite(item_sprites, maxrows, x, y, -1, shieldimage);

			return;
		}
		// else just equip the new one
		player.at(0).equipShield(newShield);
		player.at(0).setShieldPossession(true);

		dungeon[y*maxcols + x].item_name = EMPTY;
		dungeon[y*maxcols + x].object = nullptr;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}

	//		DROPS
	else if (item_name == HEART_POD) {
		cocos2d::experimental::AudioEngine::play2d("Heart_Pod_Pickup2.mp3", false, 1.0f);

		HeartPod heart;
		heart.changeStats(heart, player.at(0));

		dungeon[y*maxcols + x].item_name = EMPTY;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
	else if (item_name == LIFEPOT) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<LifePotion>());

			cocos2d::experimental::AudioEngine::play2d("Potion_Pickup2.mp3", false, 1.0f);

			//dungeonText.push_back("You grab the Life Potion!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (item_name == ARMOR) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<ArmorDrop>());

			cocos2d::experimental::AudioEngine::play2d("Armor_Pickup.mp3", false, 1.0f);

			//dungeonText.push_back("You grab the Armor!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (item_name == SHIELD_REPAIR) {
		cocos2d::experimental::AudioEngine::play2d("Metal_Hit2.mp3", false, 1.0f);

		ShieldRepair repair;
		repair.changeStats(repair, player.at(0));

		dungeon[y*maxcols + x].item_name = EMPTY;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
	else if (item_name == STATPOT) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<StatPotion>());

			cocos2d::experimental::AudioEngine::play2d("Potion_Pickup2.mp3", false, 1.0f);

			//dungeonText.push_back("You grab the Stat Potion!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (item_name == BOMB) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(std::make_shared<Bomb>());

			cocos2d::experimental::AudioEngine::play2d("Bomb_Pickup2.mp3", false, 1.0f);

			//dungeonText.push_back("You grab the Bomb!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (item_name == BROWN_CHEST) {
		BrownChest c;

		cocos2d::experimental::AudioEngine::play2d("ChestOpening1.mp3", false, 1.0f);

		//dungeonText.push_back("You open the Brown Chest... ");

		// flag for when chest is purchased
		dungeon[(y+1)*maxcols + x].shop_action = "chest purchased";
		
		removeSprite(item_sprites, maxrows, x, y);

		// allow item to be grabbable from below the counter
		image = c.open(dungeon[(y+1)*maxcols + x], dungeonText);

		addSprite(item_sprites, maxrows, x, y, 2, image);
	}
	else if (item_name == SILVER_CHEST) {
		SilverChest c;

		cocos2d::experimental::AudioEngine::play2d("ChestOpening1.mp3", false, 1.0f);

		//dungeonText.push_back("You open the Silver Chest... ");

		// flag for when chest is purchased
		dungeon[(y + 1)*maxcols + x].shop_action = "chest purchased";

		removeSprite(item_sprites, maxrows, x, y);

		// allow item to be grabbable from one below
		image = c.open(dungeon[(y + 1)*maxcols + x], dungeonText);
		addSprite(item_sprites, maxrows, x, y, 2, image);
	}
	else if (item_name == GOLDEN_CHEST) {
		GoldenChest c;

		cocos2d::experimental::AudioEngine::play2d("ChestOpening1.mp3", false, 1.0f);

		//dungeonText.push_back("You open the Golden Chest... ");

		// flag for when chest is purchased
		dungeon[(y + 1)*maxcols + x].shop_action = "chest purchased";

		removeSprite(item_sprites, maxrows, x, y);

		// allow item to be grabbable from one below
		image = c.open(dungeon[(y + 1)*maxcols + x], dungeonText);
		addSprite(item_sprites, maxrows, x, y, 2, image);
	}
	

	//		MISC
	else if (bottom == IDOL) {
		player.at(0).setWin(true);
		dungeon[y*maxcols + x].bottom = SPACE;
	}
	else if (traptile == STAIRS) {
		setLevel(getLevel() + 1);
		//clearScreen();
	}
	else if (traptile == LOCKED_STAIRS) {
		cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);

		//dungeonText.push_back("The stairs are locked!\n");
	}
	else if (traptile == BUTTON) {
		dungeonText.push_back("It's a button.\n");
	}
	else {
		cocos2d::experimental::AudioEngine::play2d("Player_Movement.mp3", false, 1.0f);

		//dungeonText.push_back("There's nothing in the dirt... \n");
	}

	// if an item was added, remove the sprite that was taken and reset tile item status
	if (current_inv_size < player.at(0).getWeapons().size() + player.at(0).getItems().size()) {
		dungeon[y*maxcols + x].bottom = SPACE;
		dungeon[y*maxcols + x].object = nullptr;
		dungeon[y*maxcols + x].item_name = EMPTY;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
}

std::vector<char> FirstShop::generate() {
	char dv = DOOR_V;
	char s = STAIRS;
	char c = 'c'; // shop countertop, where items will sit
	char k = 'k'; // shopkeeper
	char d = 'd'; // random breakable object
	// 1, 2, 3 : hp, shield repair, bombs
	// 4, 5 : random items
	// 6 : random chest
	// 'p' : stand here to purchase
	// '$' : insert price here

	std::vector<char> shop = {
	//    0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26
/*0*/   '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
/*1*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
/*2*/	'#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#',
/*3*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', '#', '#',
/*4*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  dv, ' ', ' ', 'b', ' ', '#', '#',
/*5*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', '#', '#',
/*6*/	'#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ',  k , ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#',
/*7*/	'#', '#', '#', '#', '#', '#', '#',  c , '$', '$', '$', ' ', ' ', '$', ' ', '$', ' ', '$', ' ',  c , '#', '#', '#', '#', '#', '#', '#',
/*8*/	'#', '#', '#', '#', '#', '#', '#',  c , '1', '2', '3',  c ,  c , '4',  c , '5',  c , '6',  c ,  c , '#', '#', '#', '#', '#', '#', '#',
/*9*/	'#',  d ,  d , ' ', ' ',  d , '#', ' ', 'p', 'p', 'p', ' ', ' ', 'p', ' ', 'p', ' ', 'p', ' ', ' ', '#', ' ', ' ', ' ', ' ',  d , '#',
/*10*/	'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#',
/*11*/	'#', '@', ' ', ' ', ' ', ' ',  dv, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  dv, ' ', ' ',   s, ' ', ' ', '#',
/*12*/	'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ',  d , '#',
/*13*/	'#',  d , ' ', ' ',  d ,  d , '#',  d ,  d , ' ', ' ',  d , ' ', ' ',  d , ' ', ' ', ' ',  d ,  d , '#', ' ', ' ', ' ',  d ,  d , '#',
/*14*/	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'
		};

	return shop;
}

std::vector<_Tile>& FirstShop::getDungeon() {
	return m_shop1;
}
int FirstShop::getRows() const {
	return m_rows;
}
int FirstShop::getCols() const {
	return m_cols;
}
std::vector<std::shared_ptr<Monster>>& FirstShop::getMonsters() {
	return m_shopMonsters;
}
std::vector<std::shared_ptr<Objects>>& FirstShop::getActives() {
	return m_shopActives;
}
std::vector<std::shared_ptr<Objects>>& FirstShop::getTraps() {
	return m_traps;
}


//	SECOND FLOOR FUNCTIONS
SecondFloor::SecondFloor(Player p) : Dungeon(3), m_openexit(false), m_watersUsed(false), m_watersCleared(true), m_guardians(-1) {
	int rows = getRows();
	int cols = getCols();

	_Tile *tile;
	int i, j;

	//	initialize entire dungeon to blank space
	for (i = 0; i < MAXROWS2; i++) {
		for (j = 0; j < MAXCOLS2; j++) {
			tile = new _Tile;
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = tile->extra = SPACE;
			tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = false;
			tile->gold = 0;
			tile->item_name = "";
			tile->object = nullptr;
			tile->marked = false;

			m_maze2.push_back(*tile);
			delete tile;
		}
	}

	//	initialize top and bottom of dungeon to be unbreakable walls
	for (i = 0; i < MAXROWS2; i++) {
		for (j = 0; j < MAXCOLS2; j++) {
			m_maze2[i*MAXCOLS2 + j].top = UNBREAKABLE_WALL;
			m_maze2[i*MAXCOLS2 + j].bottom = UNBREAKABLE_WALL;
			m_maze2[i*MAXCOLS2 + j].traptile = UNBREAKABLE_WALL;
			m_maze2[i*MAXCOLS2 + j].wall = true;
		}
		i += (MAXROWS2 - 2);
	}

	//	initialize edges of dungeon to be walls
	for (i = 0; i < MAXROWS2 - 1; i++) {
		m_maze2[i*MAXCOLS2].top = m_maze2[i*MAXCOLS2].bottom = m_maze2[i*MAXCOLS2].traptile = UNBREAKABLE_WALL;
		m_maze2[i*MAXCOLS2 + MAXCOLS2 - 1].top = m_maze2[i*MAXCOLS2 + MAXCOLS2 - 1].bottom = m_maze2[i*MAXCOLS2 + MAXCOLS2 - 1].traptile = UNBREAKABLE_WALL;
	
		m_maze2[i*MAXCOLS2].wall = true;
		m_maze2[i*MAXCOLS2 + MAXCOLS2 - 1].wall = true;
	}



	// RANDOMLY GENERATES LEVEL CHUNKS
	std::vector<std::vector<std::vector<char>>> b1;
	std::vector<char> v1, finalvec;
	int count = 0;
	while (count < (MAXROWS2-2)/4) {
		if (count == 0 || count == 8) {
			v1 = outermostChunks(b1, MAXCOLS2);
		}
		else if (count == 1 || count == 7) {
			v1 = outerChunks(b1, MAXCOLS2);
		}
		else if (count == 2 || count == 6) {
			v1 = innerChunks1(b1, MAXCOLS2);
		}
		else if (count == 3) {
			v1 = innerChunksCT(b1, MAXCOLS2);
		}
		else if (count == 5) {
			v1 = innerChunksCB(b1, MAXCOLS2);
		}
		else if (count == 4) {
			v1 = centerChunks(b1, MAXCOLS2);
		}

		for (i = 0; i < v1.size(); i++)
			finalvec.push_back(v1.at(i) == '#' ? WALL : v1.at(i));

		b1.clear();
		count++;
	}

	count = 0;
	char object;
	std::string item_name = "";

	for (i = 1; i < MAXROWS2 - 1; i++) {
		for (j = 1; j < MAXCOLS2 - 1; j++) {
			m_maze2[i*MAXCOLS2 + j].top = finalvec.at(count);
			m_maze2[i*MAXCOLS2 + j].bottom = finalvec.at(count);

			if (m_maze2[i*MAXCOLS2 + j].top == WALL || m_maze2[i*MAXCOLS2 + j].top == UNBREAKABLE_WALL ||
				m_maze2[i*MAXCOLS2 + j].top == FOUNTAIN) {
				m_maze2[i*MAXCOLS2 + j].traptile = finalvec.at(count);
				m_maze2[i*MAXCOLS2 + j].wall = true;
			}
			if (finalvec.at(count) == DEVILS_WATER) {
				std::shared_ptr<Objects> devils_water = std::make_shared<Objects>(j, i, "Devil's water");
				m_traps.emplace_back(devils_water);
				devils_water.reset();

				m_maze2[i*MAXCOLS2 + j].top = SPACE;
				m_maze2[i*MAXCOLS2 + j].bottom = SPACE;
				m_maze2[i*MAXCOLS2 + j].traptile = finalvec.at(count);
				m_maze2[i*MAXCOLS2 + j].trap = true;
			}
			if (finalvec.at(count) == LAVA) {
				std::shared_ptr<Objects> lava = std::make_shared<Lava>(j, i);
				m_traps.emplace_back(lava);
				lava.reset();

				m_maze2[i*MAXCOLS2 + j].top = SPACE;
				m_maze2[i*MAXCOLS2 + j].bottom = SPACE;
				m_maze2[i*MAXCOLS2 + j].traptile = finalvec.at(count);
				m_maze2[i*MAXCOLS2 + j].trap = true;
			}

			//	if the object was an item or enemy
			if (m_maze2[i*MAXCOLS2 + j].bottom != WALL && m_maze2[i*MAXCOLS2 + j].bottom != SPACE) {

				object = m_maze2[i*MAXCOLS2 + j].bottom;

				switch (object) {
				case c_BROWN_CHEST:
					m_maze2[i*MAXCOLS2 + j].top = SPACE;
					m_maze2[i*MAXCOLS2 + j].item_name = BROWN_CHEST;
					m_maze2[i*MAXCOLS2 + j].item = true;
					break;
				case c_SILVER_CHEST:
					m_maze2[i*MAXCOLS2 + j].top = SPACE;
					m_maze2[i*MAXCOLS2 + j].item_name = SILVER_CHEST;
					m_maze2[i*MAXCOLS2 + j].item = true;
					break;
				case WANDERER: {
					m_maze2[i*MAXCOLS2 + j].bottom = SPACE;
					Wanderer wand;
					wand.setPosX(j);
					wand.setPosY(i);
					m_f2monsters.emplace_back(new Wanderer(wand));
					m_maze2[i*MAXCOLS2 + j].enemy = true;
					break;
				}
				case GOBLIN: {
					m_maze2[i*MAXCOLS2 + j].bottom = SPACE;
					Goblin goblin(12);
					goblin.setPosX(j);
					goblin.setPosY(i);
					m_f2monsters.emplace_back(new Goblin(goblin));
					m_maze2[i*MAXCOLS2 + j].enemy = true;
					break;
				}
				default:
					break;
				}
			}
			count++;
		}
	}
	
	// END LEVEL GENERATION

	char toptile, bottomtile, traptile;
	bool hero, wall, enemy, trap;

	//	spawn player
	player.at(0) = p;
	player.at(0).setrandPosX(MAXCOLS2);
	player.at(0).setrandPosY(MAXROWS2);

	wall = m_maze2[player.at(0).getPosY()*cols + player.at(0).getPosX()].wall;
	enemy = m_maze2[player.at(0).getPosY()*cols + player.at(0).getPosX()].enemy;
	trap = m_maze2[player.at(0).getPosY()*cols + player.at(0).getPosX()].trap;

	while (wall || enemy || trap || player.at(0).getPosX() > 4 && player.at(0).getPosX() < MAXCOLS2 - 5) {
		player.at(0).setrandPosX(MAXCOLS2);
		player.at(0).setrandPosY(MAXROWS2);

		// while player is not spawned along left side or right side, reroll
		while (player.at(0).getPosX() > 4 && player.at(0).getPosX() < MAXCOLS2 - 5) {
			player.at(0).setrandPosX(MAXCOLS2);
		}

		wall = m_maze2[player.at(0).getPosY()*cols + player.at(0).getPosX()].wall;
		enemy = m_maze2[player.at(0).getPosY()*cols + player.at(0).getPosX()].enemy;
		trap = m_maze2[player.at(0).getPosY()*cols + player.at(0).getPosX()].trap;
	}
	m_maze2[player.at(0).getPosY()*MAXCOLS2 + player.at(0).getPosX()].hero = true;


	std::shared_ptr<Objects> stairs = std::make_shared<Stairs>();
	traptile = m_maze2[stairs->getPosY()*MAXCOLS2 + stairs->getPosX()].traptile;

	///	while stairs does not spawn on the side opposite from the player
	while (traptile != SPACE || ((player.at(0).getPosX() < 5 && stairs->getPosX() < MAXCOLS2-5) || (player.at(0).getPosX() > MAXCOLS2-6 && stairs->getPosX() > 4))) {
	//while (traptile != SPACE || ((player.at(0).getPosX() < 5 && stairs->getPosX() >= 5) || (player.at(0).getPosX() > MAXCOLS2-6 && stairs->getPosX() < MAXCOLS2 - 6))) {
		stairs->setrandPosX(MAXCOLS2);
		stairs->setrandPosY(MAXROWS2);

		traptile = m_maze2[stairs->getPosY()*MAXCOLS2 + stairs->getPosX()].traptile;
	}
	m_maze2[stairs->getPosY()*MAXCOLS2 + stairs->getPosX()].traptile = LOCKED_STAIRS;
	m_maze2[stairs->getPosY()*MAXCOLS2 + stairs->getPosX()].trap = true;
	m_traps.emplace_back(stairs);
	m_stairsX = stairs->getPosX();
	m_stairsY = stairs->getPosY();


	//	BUTTON for unlocking exit
	//Button button;
	std::shared_ptr<Objects> button = std::make_shared<Button>();
	button->setPosX(MAXCOLS2/2-1);
	button->setPosY(MAXROWS2/2-4);
	m_maze2[button->getPosY()*MAXCOLS2 + button->getPosX()].traptile = BUTTON;
	m_maze2[button->getPosY()*MAXCOLS2 + button->getPosX()].trap = true;
	m_traps.emplace_back(button);

	int n;
	char monstertype;
	shared_ptr<Monster> monster(nullptr);
	for (int i = 0; i < 4; i++) {

		//	selects how many of a type of monster to place
		switch (i) {
		case 0: n = 15 + randInt(4); break; // Zapper
		case 1: n = 15 + randInt(5); break; // Wanderer
		case 2: n = 20 + randInt(8); break; // Spinner
		case 3: n = 25 + randInt(5); break; // Roundabout
		case 4: n = 3 + randInt(6); break; //
		case 5: n = 1; break; //
		case 6: n = 5 + randInt(3); break; // 
		case 7: n = 3 + randInt(3); break; // 
		case 8: n = 4 + randInt(4); break; //
		}

		while (n > 0) {

			switch (i) {
			case 0:
				monster = make_shared<Zapper>();
				monstertype = ZAPPER; break;
			case 1:
				monster = make_shared<Wanderer>();
				monstertype = WANDERER; break;
			case 2:
				monster = make_shared<Spinner>();
				monstertype = SPINNER; break;
			case 3:
				monster = make_shared<Roundabout>();
				monstertype = ROUNDABOUT; break;
			case 4:
				monster = make_shared<Spinner>();
				monstertype = SPINNER; break;
			case 5:
				monster = make_shared<Bombee>();
				monstertype = BOMBEE; break;
			case 6:
				monster = make_shared<Roundabout>();
				monstertype = ROUNDABOUT; break;
			case 7:
				monster = make_shared<MountedKnight>();
				monstertype = MOUNTED_KNIGHT; break;
			case 8:
				monster = make_shared<Seeker>(12);
				monstertype = SEEKER; break;
			}

			wall = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].wall;
			hero = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].hero;
			enemy = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].enemy;
			trap = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].trap;

			while (hero || wall || enemy || trap) { // while monster position clashes with wall, player, or idol, reroll its position
				monster->setrandPosX(MAXCOLS2);	
				monster->setrandPosY(MAXROWS2);

				wall = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].wall;
				hero = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].hero;
				enemy = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].enemy;
				trap = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].trap;
			}
			m_f2monsters.emplace_back(monster);
			m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].top = monstertype;
			m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].enemy = true;

			if (monstertype == SPINNER) {
				std::shared_ptr<Spinner> spinner = std::dynamic_pointer_cast<Spinner>(monster);
				spinner->setInitialFirePosition(monster->getPosX(), monster->getPosY());
				spinner.reset();
			}

			n--;
			monster.reset();
		}
	}
	
}
SecondFloor& SecondFloor::operator=(SecondFloor const &dungeon) {
	for (unsigned i = 0; i < dungeon.getRows()*dungeon.getCols(); i++) {
		m_maze2[i] = dungeon.m_maze2[i];
	}
	player = dungeon.player;

	m_f2monsters = dungeon.m_f2monsters;
	m_f2actives = dungeon.m_f2actives;
	m_f2guardians = dungeon.m_f2guardians;
	m_traps = dungeon.m_traps;

	m_openexit = dungeon.m_openexit;

	m_watersUsed = dungeon.m_watersUsed;
	m_watersCleared = dungeon.m_watersCleared;
	m_guardians = dungeon.m_guardians;

	m_stairsX = dungeon.m_stairsX;
	m_stairsY = dungeon.m_stairsY;

	m_level = dungeon.getLevel();

	layer = dungeon.layer;
	specialChunkLayer1 = dungeon.specialChunkLayer1;
	specialChunkLayer2 = dungeon.specialChunkLayer2;

	return *this;
}

void SecondFloor::peekDungeon(int x, int y, char move) {
	int rows = getRows();
	int cols = getCols();

	char top, bottom, traptile;
	bool wall, item, trap, enemy;

	int initHP = player.at(0).getHP();

	// reset player's blocking stance
	player.at(0).setBlock(false);

	// set player's facing direction
	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd':
		player.at(0).setFacingDirection(move); break;
	}

	// check for any afflictions
	player.at(0).checkAfflictions();

	// :::: Move player ::::
	int weprange = player.at(0).getWeapon()->getRange();
	bool move_used = false;
	bool far_enemy, close_enemy;

	// if player is not immobilized, move them
	if (!(player.at(0).isStunned() || player.at(0).isFrozen())) {

		// if weapon has a range of 2, check for monsters to attack
		if (weprange == 2) {
			if (move == 'l') {
				//top = m_maze2[y*cols + x - 1].top;
				wall = m_maze2[y*cols + x - 1].wall;
				close_enemy = m_maze2[y*cols + x - 1].enemy;
				far_enemy = m_maze2[y*cols + x - 2].enemy;

				if (close_enemy) {
					fight(x - 1, y);
					move_used = true;
				}
				else if (far_enemy && !wall && y*cols + x - 2 > 0) {	// monster encountered
					fight(x - 2, y);
					move_used = true;
				}
			}
			else if (move == 'r') {
				//top = m_maze2[y*cols + x + 1].top;
				wall = m_maze2[y*cols + x + 1].wall;
				close_enemy = m_maze2[y*cols + x + 1].enemy;
				far_enemy = m_maze2[y*cols + x + 2].enemy;

				if (close_enemy) {
					fight(x + 1, y);
					move_used = true;
				}
				else if (far_enemy && !wall && y*cols + x + 2 < rows * cols) {
					fight(x + 2, y);
					move_used = true;
				}
			}
			else if (move == 'u') {
				//top = m_maze2[(y - 1)*cols + x].top;
				wall = m_maze2[(y - 1)*cols + x].wall;
				close_enemy = m_maze2[(y - 1)*cols + x].enemy;

				// if player is along top edge, assign far_enemy to close_enemy instead
				far_enemy = (y == 1 ? close_enemy : m_maze2[(y - 2)*cols + x].enemy);

				if (close_enemy) {
					fight(x, y - 1);
					move_used = true;
				}
				else if (far_enemy && !wall && (y - 2)*cols + x > 0) {
					fight(x, y - 2);
					move_used = true;
				}
			}
			else if (move == 'd') { // moved down
				//top = m_maze2[(y + 1)*cols + x].top;
				wall = m_maze2[(y + 1)*cols + x].wall;
				close_enemy = m_maze2[(y + 1)*cols + x].enemy;

				// if player is along top edge, assign far_enemy to close_enemy instead
				far_enemy = (y == rows - 2 ? close_enemy : m_maze2[(y + 2)*cols + x].enemy);

				if (close_enemy) {
					fight(x, y + 1);
					move_used = true;
				}
				else if (far_enemy && !wall && (y + 2)*cols + x < rows * cols) {
					fight(x, y + 2);
					move_used = true;
				}
			}
		}

		if (!move_used) {

			if (move == 'l') { // moved to the left
				top = m_maze2[y*cols + x - 1].top;
				bottom = m_maze2[y*cols + x - 1].bottom;
				traptile = m_maze2[y*cols + x - 1].traptile;

				wall = m_maze2[y*cols + x - 1].wall;
				item = m_maze2[y*cols + x - 1].item;
				trap = m_maze2[y*cols + x - 1].trap;
				enemy = m_maze2[y*cols + x - 1].enemy;

				if (!(wall || enemy)) {
					// move character to the left
					m_maze2[y*cols + x].hero = false;
					m_maze2[y*cols + x - 1].hero = true;
					player.at(0).setPosX(x - 1);

					if (trap) {
						if (traptile == LAVA) {
							trapEncounter(*this, x - 1, y);
						}
						if (traptile == BUTTON) {
							if (!m_openexit) {
								cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
								cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

								// replace button with pressed button
								removeSprite(trap_sprites, rows, x - 1, y);
								addSprite(trap_sprites, rows, x - 1, y, -1, "Button_Pressed_48x48.png");

								// replace locked stairs with open staircase
								removeSprite(trap_sprites, rows, m_stairsX, m_stairsY);
								addSprite(trap_sprites, rows, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");


								m_openexit = true;
								m_maze2[m_stairsY*cols + m_stairsX].traptile = STAIRS;
								dungeonText.push_back("The stairs have been unlocked!\n");
							}
						}
						if (traptile == DEVILS_WATER) {
							if (!m_watersUsed) {
								devilsWaterPrompt();
							}
						}
					}
					if (item) {
						foundItem(m_maze2, cols, x - 1, y);
					}
				}
				else if (enemy) {
					fight(x - 1, y);
				}
			}

			else if (move == 'r') { // moved to the right
				top = m_maze2[y*cols + x + 1].top;
				bottom = m_maze2[y*cols + x + 1].bottom;
				traptile = m_maze2[y*cols + x + 1].traptile;

				wall = m_maze2[y*cols + x + 1].wall;
				item = m_maze2[y*cols + x + 1].item;
				trap = m_maze2[y*cols + x + 1].trap;
				enemy = m_maze2[y*cols + x + 1].enemy;

				if (!(wall || enemy)) {
					// move character to the left
					m_maze2[y*cols + x].hero = false;
					m_maze2[y*cols + x + 1].hero = true;
					player.at(0).setPosX(x + 1);

					if (trap) {
						if (traptile == LAVA) {
							trapEncounter(*this, x + 1, y);
						}
						if (traptile == BUTTON) {
							if (!m_openexit) {
								cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
								cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

								// replace button with pressed button
								removeSprite(trap_sprites, rows, x + 1, y);
								addSprite(trap_sprites, rows, x + 1, y, -1, "Button_Pressed_48x48.png");

								// replace locked stairs with open staircase
								removeSprite(trap_sprites, rows, m_stairsX, m_stairsY);
								addSprite(trap_sprites, rows, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");


								m_openexit = true;
								m_maze2[m_stairsY*cols + m_stairsX].traptile = STAIRS;
								dungeonText.push_back("The stairs have been unlocked!\n");
							}
						}
						if (traptile == DEVILS_WATER) {
							if (!m_watersUsed) {
								devilsWaterPrompt();
							}
						}
					}
					if (item) {
						foundItem(m_maze2, cols, x + 1, y);
					}
				}
				else if (enemy) {
					fight(x + 1, y);
				}
			}

			else if (move == 'u') { // moved up
				top = m_maze2[(y - 1)*cols + x].top;
				bottom = m_maze2[(y - 1)*cols + x].bottom;
				traptile = m_maze2[(y - 1)*cols + x].traptile;

				wall = m_maze2[(y - 1)*cols + x].wall;
				item = m_maze2[(y - 1)*cols + x].item;
				trap = m_maze2[(y - 1)*cols + x].trap;
				enemy = m_maze2[(y - 1)*cols + x].enemy;

				if (!(wall || enemy)) {
					m_maze2[y*cols + x].hero = false;
					m_maze2[(y - 1)*cols + x].hero = true;
					player.at(0).setPosY(y - 1);

					if (trap) {
						if (traptile == LAVA) {
							trapEncounter(*this, x, y - 1);
						}
						if (traptile == BUTTON) {
							if (!m_openexit) {
								cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
								cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

								// replace button with pressed button
								removeSprite(trap_sprites, rows, x, y - 1);
								addSprite(trap_sprites, rows, x, y - 1, -1, "Button_Pressed_48x48.png");

								// replace locked stairs with open staircase
								removeSprite(trap_sprites, rows, m_stairsX, m_stairsY);
								addSprite(trap_sprites, rows, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");


								m_openexit = true;
								m_maze2[m_stairsY*cols + m_stairsX].traptile = STAIRS;
								dungeonText.push_back("The stairs have been unlocked!\n");
							}
						}
						if (traptile == DEVILS_WATER) {
							if (!m_watersUsed) {
								devilsWaterPrompt();
							}
						}
					}
					if (item) {
						foundItem(m_maze2, cols, x, y - 1);
					}
				}
				else if (enemy) {
					fight(x, y - 1);
				}
			}

			else if (move == 'd') { // moved down
				top = m_maze2[(y + 1)*cols + x].top;
				bottom = m_maze2[(y + 1)*cols + x].bottom;
				traptile = m_maze2[(y + 1)*cols + x].traptile;

				wall = m_maze2[(y + 1)*cols + x].wall;
				item = m_maze2[(y + 1)*cols + x].item;
				trap = m_maze2[(y + 1)*cols + x].trap;
				enemy = m_maze2[(y + 1)*cols + x].enemy;

				if (!(wall || enemy)) {
					m_maze2[y*cols + x].hero = false;
					m_maze2[(y + 1)*cols + x].hero = true;
					player.at(0).setPosY(y + 1);

					if (trap) {
						if (traptile == LAVA) {
							trapEncounter(*this, x, y + 1);
						}
						if (traptile == BUTTON) {
							if (!m_openexit) {
								cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
								cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

								// replace button with pressed button
								removeSprite(trap_sprites, rows, x, y + 1);
								addSprite(trap_sprites, rows, x, y + 1, -1, "Button_Pressed_48x48.png");

								// replace locked stairs with open staircase
								removeSprite(trap_sprites, rows, m_stairsX, m_stairsY);
								addSprite(trap_sprites, rows, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");


								m_openexit = true;
								m_maze2[m_stairsY*cols + m_stairsX].traptile = STAIRS;
								dungeonText.push_back("The stairs have been unlocked!\n");
							}
						}
						if (traptile == DEVILS_WATER) {
							if (!m_watersUsed) {
								devilsWaterPrompt();
							}
						}
					}
					if (item) {
						foundItem(m_maze2, cols, x, y + 1);
					}
				}
				else if (enemy) {
					fight(x, y + 1);
				}
			}

			else if (move == 'e') {
				collectItem(m_maze2, rows, cols, x, y);

				if (getLevel() > 3)
					return;
			}

			else if (move == 'b') {
				if (player.at(0).hasShield()) {
					// play setting up shield stance sound effect
					cocos2d::experimental::AudioEngine::play2d("Shield_Stance.mp3", false, 1.0f);

					player.at(0).setBlock(true);
				}
			}

			else if (move == 'w') {
				//showDungeon();
				//player.at(0).wield();
			}

			else if (move == 'c') {
				//showDungeon();
				//player.at(0).use(m_f2actives, m_maze2[y*cols + x], dungeonText);
			}

			// check if player is idling over lava
			if (move == 'e' || move == 'b' || move == 'w' || move == 'c' || move == '-') {
				traptile = m_maze2[y*cols + x].traptile;
				if (traptile == LAVA) {
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						player.at(0).addAffliction(std::make_shared<Burn>());
						player.at(0).setBurned(true);
					}
				}
			}
		}

	}

	// queue player move
	if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
		queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// pick up any gold/money that was on the ground
	if (m_maze2[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
		goldPickup(m_maze2, rows, cols, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// update sprite lighting
	updateLighting(*this, getMonsters(), getTraps());

	// check if player entered devil's waters and killed all guardians
	if (m_guardians == 0 && m_watersCleared) {
		guardiansDefeated();
		// prevents if statement from being entered again
		m_guardians = -1;
	}

	// check active items in play
	checkActive(m_maze2, rows, cols, m_f2actives, m_f2monsters);

	//	if player is dead then return
	if (player.at(0).getHP() <= 0) {
		//m_maze2[y*cols + x].top = 'X'; // change player icon to X to indicate death

		//clearScreen();
		//showDungeon();
		//if (!dungeonText.empty())
			//showText();
		return;
	}

	// if all monsters are dead?
	if (m_f2monsters.empty()) {
		//clearScreen();
		//showDungeon();
		//if (!dungeonText.empty())
			//showText();

		return;
	}

	// :::: Move monsters ::::
	int mx, my, shortest, mInitHP;
	char first, best, prev;
	string monster;

	for (unsigned i = 0; i < m_f2monsters.size(); i++) {
		mInitHP = getMonsters().at(i)->getHP();
		monster = m_f2monsters.at(i)->getName();
		mx = m_f2monsters.at(i)->getPosX();
		my = m_f2monsters.at(i)->getPosY();
		first = best = prev = '0';
		shortest = 0;

		// check for any afflictions
		m_f2monsters.at(i)->checkAfflictions();

		// check if an affliction killed the monster
		if (m_f2monsters.at(i)->getHP() <= 0) {
			monsterDeath(m_maze2, rows, cols, m_f2actives, m_f2monsters, i);
			i--;
			continue;
		}

		// if not stunned, use turn to move
		if (!(getMonsters().at(i)->isStunned() || getMonsters().at(i)->isFrozen())) {

			if (monster == "goblin") {
				//	allows use of goblin's smell distance function
				shared_ptr<Goblin> goblin = dynamic_pointer_cast<Goblin>(m_f2monsters.at(i));

				if (playerInRange(mx, my, i, goblin->getSmellDistance())) {
					// if viable path is found
					if (attemptChase(m_maze2, cols, i, shortest, goblin->getSmellDistance(), goblin->getSmellDistance(), mx, my, first, best, prev)) {
						moveWithSuccessfulChase(m_maze2, rows, cols, mx, my, i, best, m_f2monsters);
					}
				}
			}

			else if (monster == "wanderer") {
				moveWanderers(m_maze2, rows, cols, mx, my, i, m_f2monsters);
			}

			else if (monster == "archer" || monster == "Guardian") {
				checkArchers(m_maze2, rows, cols, player.at(0).getPosX(), player.at(0).getPosY(), i, m_f2monsters);
			}

			else if (monster == "zapper") {
				moveZappers(m_maze2, cols, mx, my, i, m_f2monsters);
			}

			else if (monster == "spinner") {
				moveSpinners(m_maze2, rows, cols, mx, my, i, m_f2monsters);
			}

			else if (monster == "bombee") {
				shared_ptr<Bombee> bombee = dynamic_pointer_cast<Bombee>(m_f2monsters.at(i));
				if (playerInRange(mx, my, i, bombee->getRange())) {
					// if viable path is found
					if (attemptChase(m_maze2, cols, i, shortest, bombee->getRange(), bombee->getRange(), mx, my, first, best, prev)) {
						moveWithSuccessfulChase(m_maze2, rows, cols, mx, my, i, best, m_f2monsters);
					}
				}
			}

			else if (monster == "roundabout") {
				moveRoundabouts(m_maze2, rows, cols, mx, my, i, m_f2monsters);
			}

			else if (monster == "mounted knight") {
				checkMountedKnights(m_maze2, rows, cols, player.at(0).getPosX(), player.at(0).getPosY(), i, m_f2monsters);
			}

			else if (monster == "seeker") {
				shared_ptr<Seeker> seeker = dynamic_pointer_cast<Seeker>(m_f2monsters.at(i));

				if (playerInRange(mx, my, i, seeker->getRange())) {
					// if viable path is found
					if (attemptChase(m_maze2, cols, i, shortest, seeker->getRange(), seeker->getRange(), mx, my, first, best, prev)) {
						moveSeekers(m_maze2, rows, cols, mx, my, i, best, m_f2monsters);
					}
				}
			}
		}

		// flash monster sprite if damage was taken
		if (getMonsters().at(i)->getHP() < mInitHP) {
			runMonsterDamage(getMonsters().at(i)->getSprite());
		}

		//	if player is dead then break
		if (player.at(0).getHP() <= 0) {
			//m_maze2[y*cols + x].top = 'X'; // change player icon to X to indicate death
			player.at(0).setDeath(m_f2monsters.at(i)->getName());
			break;
		}
	}

	// run actions
	runSpawn();

	// flash player sprite if took damage
	if (player.at(0).getHP() < initHP) {
		runPlayerDamage(player.at(0).getSprite());
	}

	//clearScreen();
	//showDungeon();
	//if (!dungeonText.empty())
		//showText();
}

void SecondFloor::devilsWater(bool bathed) {
	if (bathed) {
		m_waterPrompt = false;
		devilsGift();
	}
	m_waterPrompt = false;
}
void SecondFloor::devilsGift() {
	m_watersUsed = true;
	player.at(0).setMaxHP(player.at(0).getMaxHP() + 5);
	player.at(0).addAffliction(std::make_shared<HealOverTime>());

	//cout << "You feel life force flow through you, but there is a price to pay for your actions... \n";

	Archer a1, a2, a3, a4;
	a1.setName("Guardian");
	a2.setName("Guardian");
	a3.setName("Guardian");
	a4.setName("Guardian");

	a1.setSprite(createSprite(monster_sprites, MAXROWS2, 33, 15, 1, "Archer_48x48.png"));
	a2.setSprite(createSprite(monster_sprites, MAXROWS2, 33, 22, 1, "Archer_48x48.png"));
	a3.setSprite(createSprite(monster_sprites, MAXROWS2, 44, 15, 1, "Archer_48x48.png"));
	a4.setSprite(createSprite(monster_sprites, MAXROWS2, 44, 22, 1, "Archer_48x48.png"));

	a1.setPosX(33); a1.setPosY(15); m_f2monsters.emplace_back(new Archer(a1)); m_maze2[a1.getPosY()*MAXCOLS2 + a1.getPosX()].top = ARCHER;
	a2.setPosX(33); a2.setPosY(22); m_f2monsters.emplace_back(new Archer(a2)); m_maze2[a2.getPosY()*MAXCOLS2 + a2.getPosX()].top = ARCHER;
	a3.setPosX(44); a3.setPosY(15); m_f2monsters.emplace_back(new Archer(a3)); m_maze2[a3.getPosY()*MAXCOLS2 + a3.getPosX()].top = ARCHER;
	a4.setPosX(44); a4.setPosY(22); m_f2monsters.emplace_back(new Archer(a4)); m_maze2[a4.getPosY()*MAXCOLS2 + a4.getPosX()].top = ARCHER;

	// close off the exits with walls after accepting the devils gift
	for (int i = 31; i <= 32; i++) {
		for (int j = 18; j <= 19; j++) {
			addSprite(wall_sprites, MAXROWS2, i, j, 1, "D_Wall_Terrain2_48x48.png");

			m_maze2[j*MAXCOLS2 + i].top = UNBREAKABLE_WALL;
			m_maze2[j*MAXCOLS2 + i].wall = true;
		}
	}
	for (int i = 45; i <= 46; i++) {
		for (int j = 18; j <= 19; j++) {
			addSprite(wall_sprites, MAXROWS2, i, j, 1, "D_Wall_Terrain2_48x48.png");

			m_maze2[j*MAXCOLS2 + i].top = UNBREAKABLE_WALL;
			m_maze2[j*MAXCOLS2 + i].wall = true;
		}
	}

	m_watersCleared = false;
	m_guardians = 4;
}
void SecondFloor::guardiansDefeated() {
	cocos2d::experimental::AudioEngine::play2d("Guardians_Defeated.mp3", false, 1.0f);

	// remove the blocked walls
	for (int i = 31; i <= 32; i++) {
		for (int j = 18; j <= 19; j++) {
			removeSprite(wall_sprites, MAXROWS2, i, j);

			m_maze2[j*MAXCOLS2 + i].top = SPACE;
			m_maze2[j*MAXCOLS2 + i].wall = false;
		}
	}
	for (int i = 45; i <= 46; i++) {
		for (int j = 18; j <= 19; j++) {
			removeSprite(wall_sprites, MAXROWS2, i, j);

			m_maze2[j*MAXCOLS2 + i].top = SPACE;
			m_maze2[j*MAXCOLS2 + i].wall = false;
		}
	}
}
void SecondFloor::devilsWaterPrompt() {
	auto scene = dynamic_cast<Level2Scene*>(m_scene);
	scene->m_hud->devilsWaters(scene->kbListener, *this);

	m_waterPrompt = true;
}
bool SecondFloor::getWaterPrompt() {
	return m_waterPrompt;
}

//void SecondFloor::explosion(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int x, int y, int pos) { // bomb's coordinates
//	int px = player.at(0).getPosX();
//	int py = player.at(0).getPosY();
//	int mx, my;
//
//	for (unsigned i = 0; i < monsters.size(); i++) {
//		mx = monsters.at(i)->getPosX();
//		my = monsters.at(i)->getPosY();
//
//		if (actives.at(pos)->getItem() == "Mega Bomb") {
//			// play explosion sound effect
//			cocos2d::experimental::AudioEngine::play2d("Mega_Explosion.mp3", false, 1.0);
//
//			// find any monsters caught in the explosion
//			if (abs(mx - x) <= 2 && abs(my - y) <= 2) {
//				monsters.at(i)->setHP(monsters.at(i)->getHP() - 10);
//				dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");
//
//				if (monsters.at(i)->getHP() <= 0) {
//					monsterDeath(dungeon, maxrows, maxcols, actives, monsters, i);
//				}
//			}
//		}
//		// else was a regular bomb
//		else if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
//			monsters.at(i)->setHP(monsters.at(i)->getHP() - 10);
//			dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");
//
//			string monster = monsters.at(i)->getName();
//			if (monsters.at(i)->getHP() <= 0) {
//				if (monster == "Guardian") {
//					m_guardians--;
//					if (m_guardians == 0)
//						m_watersCleared = true;
//				}
//				monsterDeath(dungeon, maxrows, maxcols, actives, monsters, i);
//			}
//		}
//	}
//
//	if (actives.at(pos)->getItem() == "Mega Bomb") {
//		//	if player is caught in the explosion
//		if (abs(px - x) <= 2 && abs(py - y) <= 2) {
//			player.at(0).setHP(player.at(0).getHP() - 10);
//			dungeonText.push_back("That's gotta hurt!\n");
//
//			if (player.at(0).getHP() <= 0)
//				player.at(0).setDeath(actives.at(pos)->getItem());
//		}
//		//	destroy any nearby walls
//		for (int j = y - 2; j < y + 3; j++) {
//			for (int k = x - 2; k < x + 3; k++) {
//				if (dungeon[j*maxcols + k].top == WALL) {
//					dungeon[j*maxcols + k].top = SPACE;
//					dungeon[j*maxcols + k].bottom = SPACE;
//					dungeon[j*maxcols + k].wall = false;
//
//					removeSprite(wall_sprites, maxrows, k, j);
//					removeSprite(wall_sprites, maxrows, k, j);
//				}
//			}
//		}
//	}
//	else {
//		//	if player is caught in the explosion
//		if (abs(px - x) <= 1 && abs(py - y) <= 1) {
//			player.at(0).setHP(player.at(0).getHP() - 10);
//			dungeonText.push_back("That's gotta hurt!\n");
//			
//			if (player.at(0).getHP() <= 0)
//				player.at(0).setDeath(actives.at(pos)->getItem());
//		}
//
//		//	destroy any nearby walls
//		for (int j = y - 1; j < y + 2; j++) {
//			for (int k = x - 1; k < x + 2; k++) {
//				if (dungeon[j*maxcols + k].top == WALL) {
//					dungeon[j*maxcols + k].top = SPACE;
//					dungeon[j*maxcols + k].bottom = SPACE;
//					dungeon[j*maxcols + k].wall = false;
//
//					// call twice to remove top and bottom walls
//					removeSprite(wall_sprites, maxrows, k, j);
//					removeSprite(wall_sprites, maxrows, k, j);
//				}
//			}
//		}
//	}
//}
void SecondFloor::explosion(Dungeon &dungeon, int x, int y) { // bomb's coordinates
	int maxrows = dungeon.getRows();
	int maxcols = dungeon.getCols();

	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int mx, my;

	// find any monsters caught in the blast
	int pos = dungeon.findTrap(x, y, dungeon.getActives()); // finds the bomb
	for (unsigned i = 0; i < dungeon.getMonsters().size(); i++) {
		mx = dungeon.getMonsters().at(i)->getPosX();
		my = dungeon.getMonsters().at(i)->getPosY();

		if (dungeon.getActives().at(pos)->getItem() == "Mega Bomb") {
			if (abs(mx - x) <= 2 && abs(my - y) <= 2) {
				dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - 20);
				//dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

				if (dungeon.getMonsters().at(i)->getHP() <= 0) {
					monsterDeath(dungeon.getDungeon(), maxrows, maxcols, dungeon.getActives(), dungeon.getMonsters(), i);
				}
			}
		}
		// else is a regular bomb
		else if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			dungeon.getMonsters().at(i)->setHP(dungeon.getMonsters().at(i)->getHP() - 10);
			//dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

			string monster = dungeon.getMonsters().at(i)->getName();
			if (dungeon.getMonsters().at(i)->getHP() <= 0) {
				if (monster == "Guardian") {
					m_guardians--;
					if (m_guardians == 0)
						m_watersCleared = true;
				}
				monsterDeath(dungeon.getDungeon(), maxrows, maxcols, dungeon.getActives(), dungeon.getMonsters(), i);
			}
		}
	}

	// check if player was hit or if there are any walls to destroy
	if (dungeon.getActives().at(pos)->getItem() == "Mega Bomb") {

		//	if player is caught in the explosion
		if (abs(px - x) <= 2 && abs(py - y) <= 2) {
			player.at(0).setHP(player.at(0).getHP() - 20);
			//dungeonText.push_back("That's gotta hurt!\n");
			if (player.at(0).getHP() <= 0)
				player.at(0).setDeath(dungeon.getActives().at(pos)->getItem());
		}

		//	destroy any nearby walls
		for (int j = y - 2; j < y + 3; j++) {
			for (int k = x - 2; k < x + 3; k++) {
				if (j != -1 && j != maxrows) { // boundary check
					if (dungeon.getDungeon()[j*maxcols + k].top == WALL) {
						dungeon.getDungeon()[j*maxcols + k].top = SPACE;
						dungeon.getDungeon()[j*maxcols + k].bottom = SPACE;
						dungeon.getDungeon()[j*maxcols + k].wall = false;

						// call remove sprite twice to remove top and bottom walls
						removeSprite(wall_sprites, maxrows, k, j);
						//removeSprite(wall_sprites, maxrows, k, j);
					}

					// destroy any gold in the way
					if (dungeon.getDungeon()[j*maxcols + k].gold != 0) {
						dungeon.getDungeon()[j*maxcols + k].gold = 0;

						removeSprite(money_sprites, maxrows, k, j);
					}

					// destroy any destructible traps
					if (dungeon[j*maxcols + k].trap) {
						int pos = dungeon.findTrap(k, j, dungeon.getTraps());
						int pos2 = dungeon.findTrap(k, j, dungeon.getActives());

						if (pos != -1) {
							if (dungeon.getTraps().at(pos)->isDestructible()) {
								dungeon.getDungeon()[j*maxcols + k].trap = false;
								dungeon.getDungeon()[j*maxcols + k].trap_name = "";

								queueRemoveSprite(k, j, dungeon.getTraps().at(pos)->getSprite());
								dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
							}
						}
						if (pos2 != -1) {
							if (dungeon.getActives().at(pos)->isDestructible()) {
								// push this trap into the trap removal vector for later
								// removing it now would cause an error since checkActives() is still iterating through
								m_trapIndexes.push_back(std::make_pair(k, j));
							}
						}
					}
				}
			}
		}
	}
	// else is regular bomb
	else {

		//	if player is caught in the explosion
		if (abs(px - x) <= 1 && abs(py - y) <= 1) {
			player.at(0).setHP(player.at(0).getHP() - 10);
			//dungeonText.push_back("That's gotta hurt!\n");
			if (player.at(0).getHP() <= 0)
				player.at(0).setDeath(dungeon.getActives().at(pos)->getItem());
		}
		//	destroy any nearby walls
		for (int j = y - 1; j < y + 2; j++) {
			for (int k = x - 1; k < x + 2; k++) {
				if (dungeon.getDungeon()[j*maxcols + k].top == WALL) {
					dungeon.getDungeon()[j*maxcols + k].top = SPACE;
					dungeon.getDungeon()[j*maxcols + k].bottom = SPACE;
					dungeon.getDungeon()[j*maxcols + k].wall = false;

					// call remove sprite twice to remove top and bottom walls
					removeSprite(wall_sprites, maxrows, k, j);
					//removeSprite(wall_sprites, maxrows, k, j);
				}

				// destroy any gold
				if (dungeon.getDungeon()[j*maxcols + k].gold != 0) {
					dungeon.getDungeon()[j*maxcols + k].gold = 0;

					removeSprite(money_sprites, maxrows, k, j);
				}

				// destroy any destructible traps
				if (dungeon[j*maxcols + k].trap) {
					int pos = dungeon.findTrap(k, j, dungeon.getTraps());
					int pos2 = dungeon.findTrap(k, j, dungeon.getActives());

					if (pos != -1) {
						if (dungeon.getTraps().at(pos)->isDestructible()) {
							dungeon.getDungeon()[j*maxcols + k].trap = false;
							dungeon.getDungeon()[j*maxcols + k].trap_name = "";

							queueRemoveSprite(k, j, dungeon.getTraps().at(pos)->getSprite());
							dungeon.getTraps().erase(dungeon.getTraps().begin() + pos);
						}
					}
					if (pos2 != -1) {
						if (dungeon.getActives().at(pos)->isDestructible()) {
							// push this trap into the trap removal vector for later
							// removing it now would cause an error since checkActives() is still iterating through
							m_trapIndexes.push_back(std::make_pair(k, j));
						}
					}
				}
			}
		}
	}
}

void SecondFloor::fight(int x, int y) { // monster's coordinates
	int i = findMonster(x, y, m_f2monsters);

	if (i == -1)
		return;

	//player.at(0).attack(m_f2monsters, m_f2actives, i, dungeonText);
	player.at(0).attack(*this, *getMonsters().at(i));

	std::string monster = m_f2monsters.at(i)->getName();
	if (m_f2monsters.at(i)->getHP() <= 0) {

		if (monster == "Guardian") {
			m_guardians--;
			if (m_guardians == 0)
				m_watersCleared = true;
		}
		monsterDeath(m_maze2, getRows(), getCols(), getActives(), getMonsters(), i);
	}
}

/*
void SecondFloor::showDungeon() {
	//	adjust window to be more centered
	cout << "\n\n\n\n";
	cout << "       ";

	_Tile *tile;
	for (int i = 0; i < MAXROWS2; i++) {
		for (int j = 0; j < MAXCOLS2; j++) {
			tile = &m_maze2[i*MAXCOLS2 + j];
			if (tile->top == SPACE && tile->projectile == SPACE && tile->traptile != SPACE)
				cout << tile->traptile;
			else if (tile->top == SPACE && tile->projectile != SPACE)
				cout << tile->projectile;
			else if (tile->top == SPACE)
				cout << tile->bottom;
			else
				cout << tile->top;
		}
		cout << endl;
		cout << "       ";
	}
	cout << endl;

	cout << "       ";
	cout << "HP: " << player.at(0).getHP() << ", Armor: " << player.at(0).getArmor() \
		<< ", Strength: " << player.at(0).getStr() << ", Dexterity: " << player.at(0).getDex() \
		<< " | Current Weapon: " << player.at(0).getWeapon().getAction() << endl;
	cout << "       ";
	cout << endl;
}
*/

std::vector<char> SecondFloor::outermostChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one = { {'#', '#', '#', '#'},			
								 {' ', '#', '#', ' '},
								 {' ', ' ', ' ', ' '},
								 {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> two = { {'#', '#', '#', '#'},
								 {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> three = { {' ', '#', '#', ' '},
								   {' ', '#', '#', ' '},
								   {' ', '#', '#', ' '},
								   {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> four = { {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> five = { {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> six = { {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> seven = { {' ', ' ', '#', ' '},
								   {'#', ' ', ' ', ' '},
								   {' ', ' ', '#', ' '},
								   {'#', ' ', '#', ' '} };

	std::vector<std::vector<char>> eight = { {'#', '#', '#', '#'},
								   {' ', ' ', ' ', '#'},
								   {'#', ' ', ' ', ' '},
								   {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> ten = { {' ', ' ', ' ', ' '},
							     {' ', '#', ' ', ' '},
							     {' ', ' ', '#', ' '},
							     {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#'},
									{' ', ' ', ' ', ' '},
									{' ', ' ', ' ', ' '},
									{'#', '#', '#', '#'} };

	std::vector<std::vector<char>> twelve = { {' ', '#', ' ', ' '},
									{' ', ' ', ' ', ' '},
									{' ', ' ', ' ', ' '},
									{' ', ' ', '#', '#'} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	c.push_back(eleven);
	c.push_back(twelve);

	c = mixOutermostChunks(c, maxcols);

	return combineChunks(c);
}
std::vector<char> SecondFloor::outerChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one = { {' ', ' ', ' ', '#'},
								 {' ', ' ', '#', '#'},
								 {' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', '#'} };

	std::vector<std::vector<char>> two = { {' ', ' ', '#', '#'},
								 {'#', ' ', ' ', '#'},
								 {'#', '#', ' ', ' '},
								 {'#', '#', '#', ' '} };

	std::vector<std::vector<char>> three = { {' ', '#', '#', ' '},
								   {' ', '#', '#', ' '},
								   {' ', '#', '#', ' '},
								   {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> four = { {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {'#', ' ', ' ', '#'} };

	std::vector<std::vector<char>> five = { {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> six = { {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> seven = { {' ', ' ', '#', ' '},
								   {'#', ' ', ' ', ' '},
								   {' ', ' ', '#', ' '},
								   {'#', ' ', '#', ' '} };

	std::vector<std::vector<char>> eight = { {'#', '#', '#', ' '},
								   {' ', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '},
								   {'#', ' ', '#', '#'} };

	std::vector<std::vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> ten = { {'#', ' ', ' ', '#'},
								 {'#', ' ', ' ', '#'},
								 {'#', ' ', ' ', '#'},
								 {'#', ' ', ' ', '#'} };

	std::vector<std::vector<char>> eleven = { {'#', ' ', ' ', '#'},
									{' ', '#', ' ', ' '},
									{' ', ' ', ' ', ' '},
									{'#', ' ', '#', '#'} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	c.push_back(eleven);

	c = mixOuterChunks(c, maxcols);

	return combineChunks(c);
}
std::vector<char> SecondFloor::innerChunks1(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one = { {' ', '#', '#', ' '},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', ' ', ' '},
								 {' ', '#', '#', ' '} };

	std::vector<std::vector<char>> two = { {'#', ' ', ' ', ' '},
								 {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', '#', ' '} };

	std::vector<std::vector<char>> three = { {' ', '#', '#', ' '},
								   {' ', ' ', '#', ' '},
								   {' ', ' ', '#', ' '},
								   {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> four = { {' ', '#', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> five = { {' ', ' ', ' ', '#'},
								  {' ', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> six = { {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> seven = { {' ', ' ', '#', ' '},
								   {'#', ' ', ' ', ' '},
								   {' ', ' ', '#', ' '},
								   {'#', ' ', '#', ' '} };

	std::vector<std::vector<char>> eight = { {'#', '#', ' ', '#'},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', '#', ' '},
								   {' ', ' ', '#', ' '} };

	std::vector<std::vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> ten = { {'#', ' ', ' ', ' '},
								 {' ', ' ', '#', ' '},
								 {' ', '#', ' ', ' '},
								 {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> eleven = { {' ', ' ', ' ', '#'},
									{' ', '#', ' ', ' '},
									{' ', ' ', '#', ' '},
									{'#', ' ', ' ', ' '} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	c.push_back(eleven);

	c = mixInnerChunks1(c, maxcols);

	return combineChunks(c);
}
std::vector<char> SecondFloor::innerChunksCT(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one = { {' ', '#', '#', ' '},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', ' ', ' '},
								 {' ', '#', '#', ' '} };

	std::vector<std::vector<char>> two = { {'#', ' ', ' ', ' '},
								 {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', '#', ' '} };

	std::vector<std::vector<char>> three = { {' ', '#', '#', ' '},
								   {' ', ' ', '#', ' '},
								   {' ', ' ', '#', ' '},
								   {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> four = { {' ', '#', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> five = { {' ', ' ', ' ', '#'},
								  {' ', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> six = { {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> seven = { {' ', ' ', '#', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> eight = { {'#', '#', ' ', '#'},
								   {' ', ' ', ' ', ' '},
								   {' ', '#', ' ', ' '},
								   {' ', ' ', '#', ' '} };

	std::vector<std::vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {'#', '#', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> ten = { {'#', '#', '#', ' '},
								 {' ', ' ', '#', ' '},
								 {' ', '#', ' ', ' '},
								 {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> eleven = { {' ', ' ', ' ', ' '},
									{' ', '#', ' ', ' '},
									{' ', ' ', '#', ' '},
									{'#', ' ', ' ', ' '} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	c.push_back(eleven);

	c = mixInnerChunksCT(c, maxcols);

	return combineChunks(c);
}
std::vector<char> SecondFloor::innerChunksCB(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one = { {' ', '#', '#', ' '},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', ' ', ' '},
								 {' ', '#', '#', ' '} };

	std::vector<std::vector<char>> two = { {'#', ' ', ' ', ' '},
								 {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', '#', ' '} };

	std::vector<std::vector<char>> three = { {' ', '#', '#', ' '},
								   {' ', ' ', '#', ' '},
								   {' ', ' ', '#', ' '},
								   {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> four = { {' ', '#', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> five = { {' ', ' ', ' ', '#'},
								  {' ', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> six = { {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> seven = { {' ', ' ', '#', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> eight = { {'#', '#', ' ', '#'},
								   {' ', ' ', ' ', ' '},
								   {' ', '#', ' ', ' '},
								   {' ', ' ', '#', ' '} };

	std::vector<std::vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {'#', '#', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> ten = { {'#', '#', '#', ' '},
								 {' ', ' ', '#', ' '},
								 {' ', '#', ' ', ' '},
								 {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> eleven = { {' ', ' ', ' ', ' '},
									{' ', '#', ' ', ' '},
									{' ', ' ', '#', ' '},
									{'#', ' ', ' ', ' '} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	//c.push_back();

	c = mixInnerChunksCB(c, maxcols);

	return combineChunks(c);
}
std::vector<char> SecondFloor::centerChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one = { {' ', '#', '#', ' '},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', ' ', ' '},
								 {' ', '#', '#', ' '} };

	std::vector<std::vector<char>> two = { {'#', ' ', ' ', ' '},
								 {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', '#', ' '} };

	std::vector<std::vector<char>> three = { {' ', '#', '#', ' '},
								   {' ', ' ', '#', ' '},
								   {' ', ' ', '#', ' '},
								   {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> four = { {' ', '#', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> five = { {' ', ' ', ' ', '#'},
								  {' ', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> six = { {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> seven = { {' ', ' ', '#', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '} };

	std::vector<std::vector<char>> eight = { {'#', '#', ' ', '#'},
								   {' ', ' ', ' ', ' '},
								   {' ', '#', ' ', ' '},
								   {' ', ' ', '#', ' '} };

	std::vector<std::vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {'#', '#', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> ten = { {'#', '#', '#', ' '},
								 {' ', ' ', '#', ' '},
								 {' ', '#', ' ', ' '},
								 {' ', ' ', ' ', '#'} };

	std::vector<std::vector<char>> eleven = { {' ', ' ', ' ', ' '},
									{' ', '#', ' ', ' '},
									{' ', ' ', '#', ' '},
									{'#', ' ', ' ', ' '} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);
	c.push_back(nine);
	c.push_back(ten);
	c.push_back(eleven);

	c = mixCenterChunks(c, maxcols);

	return combineChunks(c);
}

std::vector<std::vector<std::vector<char>>> SecondFloor::mixOutermostChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols) {
	std::vector<std::vector<std::vector<char>>> v;

	std::vector<std::vector<char>> empty = { {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '} };

	for (int i = 0; i < (maxcols - 2) / 4; i++) {
		if (i == 9)
			v.push_back(empty);
		else
			v.push_back(c[randInt(c.size())]);
	}

	return v;
}
std::vector<std::vector<std::vector<char>>> SecondFloor::mixOuterChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols) {
	std::vector<std::vector<std::vector<char>>> v;

	//	Special tiles
	std::vector<std::vector<char>> lava = { {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA} };

	std::vector<std::vector<char>> empty = { {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '} };

	for (int i = 0; i < (maxcols - 2) / 4; i++) {
		if ((i >= 2 && i <= 8) || (i >= 10 && i <= 16))
			v.push_back(lava);
		else if (i == 9)
			v.push_back(empty);
		else
			v.push_back(c[randInt(c.size())]);
	}

	return v;
}
std::vector<std::vector<std::vector<char>>> SecondFloor::mixInnerChunks1(std::vector<std::vector<std::vector<char>>> c, int maxcols) {
	std::vector<std::vector<std::vector<char>>> v;

	//	Special tiles
	std::vector<std::vector<char>> lava = { {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA} };

	for (int i = 0; i < (maxcols - 2) / 4; i++) {
		if ((i >= 2 && i <= 3) || (i >= 15 && i <= 16))
			v.push_back(lava);
		else
			v.push_back(c[randInt(c.size())]);
	}

	return v;
}
std::vector<std::vector<std::vector<char>>> SecondFloor::mixInnerChunksCT(std::vector<std::vector<std::vector<char>>> c, int maxcols) {
	std::vector<std::vector<std::vector<char>>> v;

	//	Special tiles
	std::vector<std::vector<char>> lava = { {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA} };

	std::vector<std::vector<char>> centertop = { {'#', '#', '#', '#'},
									   {'#', '#', '#', '#'},
									   {' ', ' ', ' ', ' '},
									   {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> wall = { {'#', '#', '#', '#'},
								  {'#', '#', '#', '#'},
								  {'#', '#', '#', '#'},
								  {'#', '#', '#', '#'} };

	for (int i = 0; i < (maxcols - 2) / 4; i++) {
		if ((i >= 2 && i <= 3) || (i >= 15 && i <= 16))
			v.push_back(lava);
		else if ((i >= 6 && i <= 7) || (i >= 11 && i <= 12))
			v.push_back(wall);
		else if (i >= 8 && i <= 10)
			v.push_back(centertop);
		else
			v.push_back(c[randInt(c.size())]);
	}

	return v;
}
std::vector<std::vector<std::vector<char>>> SecondFloor::mixInnerChunksCB(std::vector<std::vector<std::vector<char>>> c, int maxcols) {
	std::vector<std::vector<std::vector<char>>> v;

	//	Special tiles
	std::vector<std::vector<char>> lava = { {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA} };

	std::vector<std::vector<char>> wall = { {'#', '#', '#', '#'},
								  {'#', '#', '#', '#'},
								  {'#', '#', '#', '#'},
								  {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> centerbot = { {' ', ' ', ' ', ' '},
								       {' ', ' ', ' ', ' '},
								       {'#', '#', '#', '#'},
							   	       {'#', '#', '#', '#'} };
	
	std::vector<std::vector<char>> empty = { {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '} };

	for (int i = 0; i < (maxcols - 2) / 4; i++) {
		if ((i >= 2 && i <= 3) || (i >= 15 && i <= 16))
			v.push_back(lava);
		else if ((i >= 6 && i <= 7) || (i >= 11 && i <= 12))
			v.push_back(wall);
		else if (i >= 8 && i <= 10)
			v.push_back(centerbot);
		else
			v.push_back(c[randInt(c.size())]);
	}

	return v;
}
std::vector<std::vector<std::vector<char>>> SecondFloor::mixCenterChunks(std::vector<std::vector<std::vector<char>>> c, int maxcols) {
	std::vector<std::vector<std::vector<char>>> v;

	//	Special tiles
	std::vector<std::vector<char>> fountain = { {DEVILS_WATER, DEVILS_WATER, DEVILS_WATER, DEVILS_WATER},
									  {DEVILS_WATER, FOUNTAIN, FOUNTAIN, DEVILS_WATER},
									  {DEVILS_WATER, FOUNTAIN, FOUNTAIN, DEVILS_WATER},
									  {DEVILS_WATER, DEVILS_WATER, DEVILS_WATER, DEVILS_WATER} };

	std::vector<std::vector<char>> lava = { {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA},
								  {LAVA, LAVA, LAVA, LAVA} };

	std::vector<std::vector<char>> hpath = { {'#', '#', '#', '#'},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> empty = { {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '} };

	for (int i = 0; i < (maxcols - 2) / 4; i++) {
		if ((i >= 2 && i <= 3) || (i >= 15 && i <= 16))
			v.push_back(lava);
		else if ((i >= 6 && i <= 7) || (i >= 11 && i <= 12))
			v.push_back(hpath);
		else if (i == 5 || i == 8 || i == 10 || i == 13)
			v.push_back(empty);
		else if (i == 9)
			v.push_back(fountain);
		else
			v.push_back(c[randInt(c.size())]);
	}

	return v;
}

std::vector<_Tile>& SecondFloor::getDungeon() {
	return m_maze2;
}
int SecondFloor::getRows() const {
	return m_rows;
}
int SecondFloor::getCols() const {
	return m_cols;
}
std::vector<std::shared_ptr<Monster>>& SecondFloor::getMonsters() {
	return m_f2monsters;
}
std::vector<std::shared_ptr<Objects>>& SecondFloor::getActives() {
	return m_f2actives;
}
std::vector<std::shared_ptr<Objects>>& SecondFloor::getTraps() {
	return m_traps;
}


//	THIRD FLOOR FUNCTIONS
ThirdFloor::ThirdFloor(Player p) : Dungeon(4), m_locked(false) {
	int rows = getRows();
	int cols = getCols();

	_Tile *tile;
	int i, j;

	//	initialize entire dungeon to blank space
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			tile = new _Tile;
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = SPACE;
			tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = false;
			tile->gold = 0;
			tile->item_name = "";
			tile->object = nullptr;
			tile->marked = false;

			m_maze3.push_back(*tile);
			delete tile;
		}
	}

	//	initialize top and bottom of dungeon to be unbreakable walls
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			m_maze3[i*cols + j].top = UNBREAKABLE_WALL;
			m_maze3[i*cols + j].bottom = UNBREAKABLE_WALL;
			m_maze3[i*cols + j].traptile = UNBREAKABLE_WALL;
			m_maze3[i*cols + j].wall = true;
		}
		i += (rows - 2);
	}

	//	initialize edges of dungeon to be walls
	for (i = 0; i < rows - 1; i++) {
		m_maze3[i*cols].top = m_maze3[i*cols].bottom = m_maze3[i*cols + j].traptile = UNBREAKABLE_WALL;
		m_maze3[i*cols + cols - 1].top = m_maze3[i*cols + cols - 1].bottom = m_maze3[i*cols + cols - 1].traptile = UNBREAKABLE_WALL;
	
		m_maze3[i*cols].wall = true;
		m_maze3[i*cols + cols - 1].wall = true;
	}

	// selects layer for the player and exit chunks to be on
	m_layer = 1;
	m_playerchunk = 1 + randInt(4);
	m_playerplaced = m_exitplaced = false;

	switch (m_playerchunk) {
	case 1: m_exitchunk = 4; break;
	case 2: m_exitchunk = 3; break;
	case 3: m_exitchunk = 2; break;
	case 4: m_exitchunk = 1; break;
	default: break;
	}

	// RANDOMLY GENERATES LEVEL CHUNKS
	std::vector<std::vector<std::vector<char>>> c, pool;
	std::vector<char> v, finalvec;

	int count = 0;
	while (count < (rows - 2) / 9) {
		if (count == 0) {
			TLChunks(c, pool, cols); pool.clear(); m_layer++;
			topChunks(c, pool, cols); pool.clear();
			v = TRChunks(c, pool, cols); pool.clear();
		}
		else if (count == 1 || count == 2) {
			leftEdgeChunks(c, pool, cols); pool.clear();
			middleChunks(c, pool, cols); pool.clear();
			v = rightEdgeChunks(c, pool, cols); pool.clear();
		}
		else if (count == 3) {
			m_layer++;
			BLChunks(c, pool, cols); pool.clear(); m_layer++;
			bottomChunks(c, pool, cols); pool.clear();
			v = BRChunks(c, pool, cols); pool.clear();
		}

		for (i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i) == '#' ? WALL : v.at(i));

		c.clear();
		count++;
	}

	count = 0;
	char object;
	for (i = 0; i < rows - 1; i++) { // changed to i = 0 from i = 1

		// Originally j < cols - 1
		for (j = 1; j < cols; j++) {
			m_maze3[i*cols + j].top = finalvec.at(count);
			m_maze3[i*cols + j].bottom = finalvec.at(count);
			if (m_maze3[i*cols + j].top == WALL || m_maze3[i*cols + j].top == UNBREAKABLE_WALL) {
				m_maze3[i*cols + j].traptile = finalvec.at(count);
				m_maze3[i*cols + j].wall = true;
			}
			if (finalvec.at(count) == LAVA) {
				std::shared_ptr<Objects> lava = std::make_shared<Lava>(j, i);
				m_traps.emplace_back(lava);
				lava.reset();

				m_maze3[i*cols + j].top = SPACE;
				m_maze3[i*cols + j].bottom = SPACE;
				m_maze3[i*cols + j].traptile = finalvec.at(count);
				m_maze3[i*cols + j].trap = true;
			}

			//	if the object was an item or enemy
			if (m_maze3[i*cols + j].bottom != WALL && m_maze3[i*cols + j].bottom != SPACE) {

				object = m_maze3[i*cols + j].bottom;

				switch (object) {
				case PLAYER:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].hero = true;
					player.at(0) = p;
					player.at(0).setPosX(j);
					player.at(0).setPosY(i);
					break;
				case DOOR_V:
				case DOOR_H: {
					m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].traptile = finalvec.at(count);
					m_maze3[i*cols + j].wall = true;
					//m_maze3[i*cols + j].trap = true;
					Door door;
					door.setPosX(j);
					door.setPosY(i);
					m_f3actives.emplace_back(new Door(door));
					break;
				}
				case STAIRS: {
					Stairs stairs;
					stairs.setPosX(j);
					stairs.setPosY(i);
					m_traps.emplace_back(new Stairs(stairs));

					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].traptile = STAIRS;
					m_maze3[i*cols + j].trap = true;
					break;
				}
				case BUTTON: {
					Button button;
					button.setPosX(j);
					button.setPosY(i);
					m_traps.emplace_back(new Button(button));

					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].traptile = BUTTON;
					m_maze3[i*cols + j].trap = true;
					break;
				}
				case c_BOMB:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = BOMB;
					m_maze3[i*cols + j].item = true;
					break;
				case c_ARMOR:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = ARMOR;
					m_maze3[i*cols + j].item = true;
					break;
				case c_LIFEPOT:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = LIFEPOT;
					m_maze3[i*cols + j].item = true;
					break;
				case c_STATPOT:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = STATPOT;
					m_maze3[i*cols + j].item = true;
					break;
				case c_HEART_POD:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = HEART_POD;
					m_maze3[i*cols + j].item = true;
					break;
				case c_BROWN_CHEST:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = BROWN_CHEST;
					m_maze3[i*cols + j].item = true;
					break;
				case c_SILVER_CHEST:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = SILVER_CHEST;
					m_maze3[i*cols + j].item = true;
					break;
				case c_GOLDEN_CHEST:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = GOLDEN_CHEST;
					m_maze3[i*cols + j].item = true;
					break;
				case c_INFINITY_BOX:
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = INFINITY_BOX;
					m_maze3[i*cols + j].item = true;
					break;
				case SPIKETRAP_DEACTIVE: {
					SpikeTrap st(j, i, 4);
					m_f3actives.emplace_back(new SpikeTrap(st));
					m_traps.emplace_back(new SpikeTrap(st)); //

					m_maze3[i*cols + j].top = m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].traptile = SPIKETRAP_DEACTIVE;
					m_maze3[i*cols + j].trap = true;
					break;
				}
				case TRIGGER_SPIKE_DEACTIVE: {
					TriggerSpike ts(j, i);
					m_f3actives.emplace_back(new TriggerSpike(ts));
					m_traps.emplace_back(new TriggerSpike(ts)); //

					m_maze3[i*cols + j].top = m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].traptile = TRIGGER_SPIKE_DEACTIVE;
					m_maze3[i*cols + j].trap = true;
					break;
				}
				case WANDERER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Wanderer wand;
					wand.setPosX(j);
					wand.setPosY(i);
					m_f3monsters.emplace_back(new Wanderer(wand));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case GOBLIN: {
					m_maze3[i*cols + j].bottom = SPACE;
					Goblin goblin(12);
					goblin.setPosX(j);
					goblin.setPosY(i);
					m_f3monsters.emplace_back(new Goblin(goblin));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case ARCHER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Archer archer;
					archer.setPosX(j);
					archer.setPosY(i);
					m_f3monsters.emplace_back(new Archer(archer));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case SEEKER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Seeker seeker(10);
					seeker.setPosX(j);
					seeker.setPosY(i);
					m_f3monsters.emplace_back(new Seeker(seeker));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case MOUNTED_KNIGHT: {
					m_maze3[i*cols + j].bottom = SPACE;
					MountedKnight knight;
					knight.setPosX(j);
					knight.setPosY(i);
					m_f3monsters.emplace_back(new MountedKnight(knight));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case ZAPPER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Zapper zapper;
					zapper.setPosX(j);
					zapper.setPosY(i);
					m_f3monsters.emplace_back(new Zapper(zapper));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case SPINNER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Spinner spinner;
					spinner.setPosX(j);
					spinner.setPosY(i);
					spinner.setInitialFirePosition(spinner.getPosX(), spinner.getPosY());
					m_f3monsters.emplace_back(new Spinner(spinner));

					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case ROUNDABOUT: {
					m_maze3[i*cols + j].bottom = SPACE;
					Roundabout ra;
					ra.setPosX(j);
					ra.setPosY(i);
					m_f3monsters.emplace_back(new Roundabout(ra));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case BOMBEE: {
					m_maze3[i*cols + j].bottom = SPACE;
					Bombee bombee;
					bombee.setPosX(j);
					bombee.setPosY(i);
					m_f3monsters.emplace_back(new Bombee(bombee));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				default: break;
				}
			}
			count++;
		}
	}
	// END LEVEL GENERATION
}
ThirdFloor& ThirdFloor::operator=(ThirdFloor const &dungeon) {
	for (unsigned i = 0; i < dungeon.getRows()*dungeon.getCols(); i++) {
		m_maze3[i] = dungeon.m_maze3[i];
	}
	player = dungeon.player;

	m_f3monsters = dungeon.m_f3monsters;
	m_f3actives = dungeon.m_f3actives;
	m_traps = dungeon.m_traps;

	m_locked = dungeon.m_locked;

	m_playerplaced = dungeon.m_playerplaced;
	m_exitplaced = dungeon.m_exitplaced;

	m_level = dungeon.getLevel();

	m_layer = dungeon.m_layer;
	m_playerchunk = dungeon.m_playerchunk;
	m_exitchunk = dungeon.m_exitchunk;

	return *this;
}

void ThirdFloor::peekDungeon(int x, int y, char move) {
	int rows = getRows();
	int cols = getCols();

	char top, bottom, traptile;
	bool wall, item, enemy, trap;

	int initHP = player.at(0).getHP();

	// reset player's blocking stance
	player.at(0).setBlock(false);

	// set player's facing direction
	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd':
		player.at(0).setFacingDirection(move); break;
	}

	// check for any afflictions
	player.at(0).checkAfflictions();

	// :::: Move player ::::
	int weprange = player.at(0).getWeapon()->getRange();
	bool move_used = false;
	bool far_enemy, close_enemy;

	// if player is not immobilized, move them
	if (!(player.at(0).isStunned() || player.at(0).isFrozen())) {

		// if weapon has a range of 2, check for monsters to attack
		if (weprange == 2) {
			if (move == 'l') { // moved to the left
				wall = m_maze3[y*cols + x - 1].wall;
				close_enemy = m_maze3[y*cols + x - 1].enemy;
				far_enemy = m_maze3[y*cols + x - 2].enemy;

				if (close_enemy) {
					fight(*this, x - 1, y);
					move_used = true;
				}
				else if (far_enemy && !wall && y*cols + x - 2 > 0) {	// monster encountered
					fight(*this, x - 2, y);
					move_used = true;
				}

			}
			else if (move == 'r') {
				//top = m_maze3[y*cols + x + 1].top;
				wall = m_maze3[y*cols + x + 1].wall;
				close_enemy = m_maze3[y*cols + x + 1].enemy;
				far_enemy = m_maze3[y*cols + x + 2].enemy;

				if (close_enemy) {
					fight(*this, x + 1, y);
					move_used = true;
				}
				else if (far_enemy && !wall && y*cols + x + 2 < rows * cols) {
					fight(*this, x + 2, y);
					move_used = true;
				}

			}
			else if (move == 'u') { // moved up
				//top = m_maze3[(y - 1)*cols + x].top;
				wall = m_maze3[(y - 1)*cols + x].wall;
				close_enemy = m_maze3[(y - 1)*cols + x].enemy;

				// if player is along top edge, assign far_enemy to close_enemy instead
				far_enemy = (y == 1 ? close_enemy : m_maze3[(y - 2)*cols + x].enemy);

				if (close_enemy) {
					fight(*this, x, y - 1);
					move_used = true;
				}
				// top != wall, since lava counts as free space
				else if (far_enemy && !wall && (y - 2)*cols + x > 0) {
					fight(*this, x, y - 2);
					move_used = true;
				}
			}
			else if (move == 'd') { // moved down
				//top = m_maze3[(y + 1)*cols + x].top;
				wall = m_maze3[(y + 1)*cols + x].wall;
				close_enemy = m_maze3[(y + 1)*cols + x].enemy;

				// if player is along top edge, assign far_enemy to close_enemy instead
				far_enemy = (y == rows - 2 ? close_enemy : m_maze3[(y + 2)*cols + x].enemy);

				if (close_enemy) {
					fight(*this, x, y + 1);
					move_used = true;
				}
				else if (far_enemy && !wall && (y + 2)*cols + x < rows * cols) {
					fight(*this, x, y + 2);
					move_used = true;
				}

			}
		}

		if (!move_used) {

			if (move == 'l') { // moved to the left
				top = m_maze3[y*cols + x - 1].top;
				bottom = m_maze3[y*cols + x - 1].bottom;
				traptile = m_maze3[y*cols + x - 1].traptile;

				wall = m_maze3[y*cols + x - 1].wall;
				item = m_maze3[y*cols + x - 1].item;
				trap = m_maze3[y*cols + x - 1].trap;
				enemy = m_maze3[y*cols + x - 1].enemy;

				if (!(wall || enemy)) {
					m_maze3[y*cols + x].hero = false;
					m_maze3[y*cols + x - 1].hero = true;
					player.at(0).setPosX(x - 1);

					if (trap) {
						if (traptile == LAVA) {
							trapEncounter(*this, x - 1, y);
						}
						if (traptile == SPIKE) {
							trapEncounter(*this, x - 1, y);

							if (player.at(0).getHP() <= 0) {
								player.at(0).setDeath("falling spike");
								//clearScreen();
								//showDungeon();
								if (!dungeonText.empty())
									showText();
								return;
							}
						}
						if (traptile == SPIKETRAP_ACTIVE) {
							trapEncounter(*this, x - 1, y);
						}
						if (traptile == TRIGGER_SPIKE_ACTIVE || traptile == TRIGGER_SPIKE_DEACTIVE) {
							trapEncounter(*this, x - 1, y);
						}
					}
					if (item) {
						foundItem(m_maze3, cols, x - 1, y);
					}
				}
				else if (wall) {
					if (top == DOOR_H || top == DOOR_V) {
						openDoor(*this, x - 1, y);
					}
					else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
						m_maze3[y*cols + x].hero = false;
						m_maze3[y*cols + x - 1].hero = true;
						player.at(0).setPosX(x - 1);
					}
					else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
						// play locked door sound
						cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
					}
				}
				else if (enemy) {
					fight(*this, x - 1, y);
				}
			}

			else if (move == 'r') { // moved to the right
				top = m_maze3[y*cols + x + 1].top;
				bottom = m_maze3[y*cols + x + 1].bottom;
				traptile = m_maze3[y*cols + x + 1].traptile;

				wall = m_maze3[y*cols + x + 1].wall;
				item = m_maze3[y*cols + x + 1].item;
				trap = m_maze3[y*cols + x + 1].trap;
				enemy = m_maze3[y*cols + x + 1].enemy;

				if (!(wall || enemy)) {
					m_maze3[y*cols + x].hero = false;
					m_maze3[y*cols + x + 1].hero = true;
					player.at(0).setPosX(x + 1);

					if (trap) {
						if (traptile == LAVA) {
							trapEncounter(*this, x + 1, y);
						}
						if (traptile == SPIKE) {
							trapEncounter(*this, x + 1, y);

							if (player.at(0).getHP() <= 0) {
								player.at(0).setDeath("falling spike");
								//clearScreen();
								//showDungeon();
								if (!dungeonText.empty())
									showText();
								return;
							}
						}
						if (traptile == SPIKETRAP_ACTIVE) {
							trapEncounter(*this, x + 1, y);
						}
						if (traptile == TRIGGER_SPIKE_ACTIVE || traptile == TRIGGER_SPIKE_DEACTIVE) {
							trapEncounter(*this, x + 1, y);
						}
					}
					if (item) {
						foundItem(m_maze3, cols, x + 1, y);
					}
				}
				else if (wall) {
					if (top == DOOR_H || top == DOOR_V) {
						openDoor(*this, x + 1, y);
					}
					else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
						m_maze3[y*cols + x].hero = false;
						m_maze3[y*cols + x + 1].hero = true;
						player.at(0).setPosX(x + 1);
					}
					else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
						// play locked door sound
						cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
					}
				}
				else if (enemy) {
					fight(*this, x + 1, y);
				}
			}

			else if (move == 'u') { // moved up
				top = m_maze3[(y - 1)*cols + x].top;
				bottom = m_maze3[(y - 1)*cols + x].bottom;
				traptile = m_maze3[(y - 1)*cols + x].traptile;

				wall = m_maze3[(y - 1)*cols + x].wall;
				item = m_maze3[(y - 1)*cols + x].item;
				trap = m_maze3[(y - 1)*cols + x].trap;
				enemy = m_maze3[(y - 1)*cols + x].enemy;

				if (!(wall || enemy)) {
					m_maze3[y*cols + x].hero = false;
					m_maze3[(y - 1)*cols + x].hero = true;
					player.at(0).setPosY(y - 1);

					if (trap) {
						if (traptile == LAVA) {
							trapEncounter(*this, x, y - 1);
						}
						if (traptile == SPIKE) {
							trapEncounter(*this, x, y - 1);

							if (player.at(0).getHP() <= 0) {
								player.at(0).setDeath("falling spike");
								//clearScreen();
								//showDungeon();
								if (!dungeonText.empty())
									showText();
								return;
							}
						}
						if (traptile == SPIKETRAP_ACTIVE) {
							trapEncounter(*this, x, y - 1);
						}
						if (traptile == TRIGGER_SPIKE_ACTIVE || traptile == TRIGGER_SPIKE_DEACTIVE) {
							trapEncounter(*this, x, y - 1);
						}
					}
					if (item) {
						foundItem(m_maze3, cols, x, y - 1);
					}
				}
				else if (wall) {
					if (top == DOOR_H || top == DOOR_V) {
						openDoor(*this, x, y - 1);
					}
					else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
						//m_maze3[y*cols + x].top = SPACE;
						//m_maze3[(y - 1)*cols + x].top = PLAYER;
						m_maze3[y*cols + x].hero = false;
						m_maze3[(y - 1)*cols + x].hero = true;
						player.at(0).setPosY(y - 1);
					}
					else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
						// play locked door sound
						cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
					}
				}
				else if (enemy) {
					fight(*this, x, y - 1);
				}
			}

			else if (move == 'd') { // moved down
				top = m_maze3[(y + 1)*cols + x].top;
				bottom = m_maze3[(y + 1)*cols + x].bottom;
				traptile = m_maze3[(y + 1)*cols + x].traptile;

				wall = m_maze3[(y + 1)*cols + x].wall;
				item = m_maze3[(y + 1)*cols + x].item;
				trap = m_maze3[(y + 1)*cols + x].trap;
				enemy = m_maze3[(y + 1)*cols + x].enemy;

				if (!(wall || enemy)) {
					m_maze3[y*cols + x].hero = false;
					m_maze3[(y + 1)*cols + x].hero = true;
					player.at(0).setPosY(y + 1);

					if (trap) {
						if (traptile == LAVA) {
							trapEncounter(*this, x, y + 1);
						}
						if (traptile == SPIKE) {
							trapEncounter(*this, x, y + 1);

							if (player.at(0).getHP() <= 0) {
								player.at(0).setDeath("falling spike");
								//clearScreen();
								//showDungeon();
								if (!dungeonText.empty())
									showText();
								return;
							}
						}
						if (traptile == SPIKETRAP_ACTIVE) {
							trapEncounter(*this, x, y + 1);
						}
						if (traptile == TRIGGER_SPIKE_ACTIVE || traptile == TRIGGER_SPIKE_DEACTIVE) {
							trapEncounter(*this, x, y + 1);
						}
					}
					if (item) {
						foundItem(m_maze3, cols, x, y + 1);
					}
				}
				else if (wall) {
					if (top == DOOR_H || top == DOOR_V) {
						openDoor(*this, x, y + 1);
					}
					else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
						m_maze3[y*cols + x].hero = false;
						m_maze3[(y + 1)*cols + x].hero = true;
						player.at(0).setPosY(y + 1);
					}
					else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
						// play locked door sound
						cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
					}
				}
				else if (enemy) {
					fight(*this, x, y + 1);
				}
			}

			else if (move == 'e') {
				collectItem(m_maze3, rows, cols, x, y);

				if (getLevel() > 4)
					return;
			}

			else if (move == 'b') {
				if (player.at(0).hasShield()) {
					// play setting up shield stance sound effect
					cocos2d::experimental::AudioEngine::play2d("Shield_Stance.mp3", false, 1.0f);

					player.at(0).setBlock(true);
				}
			}

			else if (move == 'w') {
				//showDungeon();
				//player.at(0).wield(dungeonText);
			}

			else if (move == 'c') {
				//showDungeon();
				//player.at(0).use(m_f3actives, m_maze3[y*cols + x], dungeonText);
			}

			// check if player is idling over lava
			if (move == 'e' || move == 'b' || move == 'w' || move == 'c' || move == '-') {
				traptile = m_maze3[y*cols + x].traptile;
				if (traptile == LAVA) {
					// lava sound
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						player.at(0).addAffliction(std::make_shared<Burn>());
						player.at(0).setBurned(true);
					}
				}
			}
		}

	}

	// queue player move
	if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
		queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// pick up any gold/money that was on the ground
	if (m_maze3[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
		goldPickup(m_maze3, rows, cols, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// check active items in play
	checkActive(m_maze3, rows, cols, m_f3actives, m_f3monsters);

	// update sprite lighting
	updateLighting(*this, getMonsters(), getTraps());

	//	if player is dead then return
	if (player.at(0).getHP() <= 0) {
		//m_maze3[y*cols + x].top = 'X'; // change player icon to X to indicate death

		//clearScreen();
		//showDungeon();
		//if (!dungeonText.empty())
			//showText();
		return;
	}

	// if all monsters are dead?
	if (m_f3monsters.empty()) {
		//clearScreen();
		//showDungeon();
		//if (!dungeonText.empty())
		//	showText();

		return;
	}

	// move monsters
	int mx, my, shortest, mInitHP;
	char first, best, prev;
	string monster;

	for (unsigned i = 0; i < m_f3monsters.size(); i++) {
		mInitHP = getMonsters().at(i)->getHP();
		monster = m_f3monsters.at(i)->getName();
		mx = m_f3monsters.at(i)->getPosX();
		my = m_f3monsters.at(i)->getPosY();
		first = best = prev = '0';
		shortest = 0;

		// check for any afflictions
		m_f3monsters.at(i)->checkAfflictions();

		// check if an affliction killed the monster
		if (m_f3monsters.at(i)->getHP() <= 0) {
			monsterDeath(m_maze3, rows, cols, m_f3actives, m_f3monsters, i);
			i--;
			continue;
		}

		// if not stunned, use turn to move
		if (!(getMonsters().at(i)->isStunned() || getMonsters().at(i)->isFrozen())) {

			if (monster == "goblin") {
				//	allows use of goblin's smell distance function
				std::shared_ptr<Goblin> goblin = std::dynamic_pointer_cast<Goblin>(m_f3monsters.at(i));

				if (playerInRange(mx, my, i, goblin->getSmellDistance())) {
					// if viable path is found
					if (attemptChase(m_maze3, cols, i, shortest, goblin->getSmellDistance(), goblin->getSmellDistance(), mx, my, first, best, prev)) {
						moveWithSuccessfulChase(m_maze3, rows, cols, mx, my, i, best, m_f3monsters);
					}
				}
				goblin.reset();
			}

			else if (monster == "wanderer") {
				moveWanderers(m_maze3, rows, cols, mx, my, i, m_f3monsters);
			}

			else if (monster == "archer") {
				checkArchers(m_maze3, rows, cols, player.at(0).getPosX(), player.at(0).getPosY(), i, m_f3monsters);
			}

			else if (monster == "zapper") {
				moveZappers(m_maze3, cols, mx, my, i, m_f3monsters);
			}

			else if (monster == "spinner") {
				moveSpinners(m_maze3, rows, cols, mx, my, i, m_f3monsters);
			}

			else if (monster == "bombee") {
				shared_ptr<Bombee> bombee = dynamic_pointer_cast<Bombee>(m_f3monsters.at(i));
				if (playerInRange(mx, my, i, bombee->getRange())) {
					// if viable path is found
					if (attemptChase(m_maze3, cols, i, shortest, bombee->getRange(), bombee->getRange(), mx, my, first, best, prev)) {
						moveWithSuccessfulChase(m_maze3, rows, cols, mx, my, i, best, m_f3monsters);
					}
				}
				bombee.reset();
			}

			else if (monster == "roundabout") {
				moveRoundabouts(m_maze3, rows, cols, mx, my, i, m_f3monsters);
			}

			else if (monster == "mounted knight") {
				checkMountedKnights(m_maze3, rows, cols, player.at(0).getPosX(), player.at(0).getPosY(), i, m_f3monsters);
			}

			else if (monster == "seeker") {
				shared_ptr<Seeker> seeker = dynamic_pointer_cast<Seeker>(m_f3monsters.at(i));

				if (playerInRange(mx, my, i, seeker->getRange())) {
					// if viable path is found
					if (attemptChase(m_maze3, cols, i, shortest, seeker->getRange(), seeker->getRange(), mx, my, first, best, prev)) {
						moveSeekers(m_maze3, rows, cols, mx, my, i, best, m_f3monsters);
					}
				}
				seeker.reset();
			}

			// flash monster sprite if damage was taken
			if (getMonsters().at(i)->getHP() < mInitHP) {
				runMonsterDamage(getMonsters().at(i)->getSprite());
			}

			//	if player is dead then break
			if (player.at(0).getHP() <= 0) {
				//m_maze3[y*cols + x].top = 'X'; // change player icon to X to indicate death
				player.at(0).setDeath(m_f3monsters.at(i)->getName());
				break;
			}
		}
	}

	// run actions
	runSpawn();

	// flash player sprite if took damage
	if (player.at(0).getHP() < initHP) {
		runPlayerDamage(player.at(0).getSprite());
	}

	//clearScreen();
	//showDungeon();
	if (!dungeonText.empty())
		showText();
}

void ThirdFloor::checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters) {

	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int x, y;
	int pos;

	if (!actives.empty()) {
		for (unsigned i = 0; i < actives.size(); i++) {
			if (actives.at(i)->getItem() == "Bomb") {
				shared_ptr<Bomb> bomb = dynamic_pointer_cast<Bomb>(actives.at(i));
				if (bomb->getFuse() > 0) {
					bomb->setFuse(bomb->getFuse() - 1);
					actives.at(i) = bomb;
				}
				else {
					cocos2d::experimental::AudioEngine::play2d("Explosion.mp3", false, 1.0f);

					// remove bomb sprite
					removeSprite(projectile_sprites, maxrows, bomb->getPosX(), bomb->getPosY());

					//dungeonText.push_back("A bomb exploded!\n");
					//explosion(dungeon, maxrows, maxcols, actives, monsters, bomb->getPosX(), bomb->getPosY(), i);
					explosion(*this, bomb->getPosX(), bomb->getPosY());
					dungeon[bomb->getPosY()*maxcols + bomb->getPosX()].extra = SPACE;

					// set i to pos because otherwise the iterator is destroyed
					pos = i;
					actives.erase(actives.begin() + pos);
					i--;
				}
				bomb.reset();
			}
			else if (actives.at(i)->getItem() == "Mega Bomb") {
				shared_ptr<MegaBomb> megabomb = dynamic_pointer_cast<MegaBomb>(actives.at(i));
				if (megabomb->getFuse() > 0) {
					megabomb->setFuse(megabomb->getFuse() - 1);
					actives.at(i) = megabomb;
				}
				else {
					cocos2d::experimental::AudioEngine::play2d("Mega_Explosion.mp3", false, 1.0f);

					// remove bomb sprite
					removeSprite(projectile_sprites, maxrows, megabomb->getPosX(), megabomb->getPosY());

					//dungeonText.push_back("BOOM!\n");
					//explosion(dungeon, maxrows, maxcols, actives, monsters, megabomb->getPosX(), megabomb->getPosY(), i);
					explosion(*this, megabomb->getPosX(), megabomb->getPosY());
					dungeon[megabomb->getPosY()*maxcols + megabomb->getPosX()].extra = SPACE;

					// set i to pos because otherwise the iterator is destroyed
					pos = i;
					actives.erase(actives.begin() + pos);
					i--;
				}
				megabomb.reset();
			}
			else if (actives.at(i)->getItem() == "falling spike") {
				shared_ptr<FallingSpike> spike = dynamic_pointer_cast<FallingSpike>(actives.at(i));
				x = spike->getPosX();
				y = spike->getPosY();

				switch (spike->getSpeed()) {
				case 1:
					//	if tile below spike is the player, hit them
					if (dungeon[(y + 1)*maxcols + x].top == PLAYER) {
						pos = i;
						actives.erase(actives.begin() + pos);

						dungeonText.push_back("A speed 1 spike smacked you on the head for " + to_string(spike->getDmg()) + " damage!\n");
						player.at(0).setHP(player.at(0).getHP() - spike->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							dungeon[py*maxcols + px].top = 'X';
						}
					}
					//	if tile below spike is not a wall
					else if (y + 1 < maxrows - 1) {
						spike->setPosY(y + 1);
						actives.at(i) = spike;
						dungeon[(y + 1)*maxcols + x].traptile = SPIKE;
						dungeon[(y + 1)*maxcols + x].trap = true;
					}
					//	else disappear
					else {
						pos = i;
						actives.erase(actives.begin() + pos);
					}
					break;
				case 2:
					//	if player is in the way, hit them
					if (dungeon[(y + 1)*maxcols + x].top == PLAYER || dungeon[(y + 2)*maxcols + x].top == PLAYER) {
						pos = i;
						actives.erase(actives.begin() + pos);

						dungeonText.push_back("A speed 2 spike smacked you on the head for " + to_string(spike->getDmg()) + "!\n");
						player.at(0).setHP(player.at(0).getHP() - spike->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							dungeon[py*maxcols + px].top = 'X';
						}
					}
					//	else if two tiles below spike are not walls
					else if (y + 2 < maxrows - 1) {
						spike->setPosY(y + 2);
						actives.at(i) = spike;
						dungeon[(y + 2)*maxcols + x].traptile = SPIKE;
						dungeon[(y + 2)*maxcols + x].trap = true;
					}
					//	else if there is only one tile of space underneath, set spike to bottom
					else if (y + 2 >= maxrows - 1 && y != maxrows - 2) {
						spike->setPosY(maxrows - 2);
						actives.at(i) = spike;
						dungeon[(maxrows - 2)*maxcols + x].traptile = SPIKE;
						dungeon[(maxrows - 2)*maxcols + x].trap = true;
					}
					//	else disappear
					else {
						pos = i;
						actives.erase(actives.begin() + pos);
					}
					break;
				case 3:
					//	if player is below, hit them
					if (dungeon[(y + 3)*maxcols + x].top == PLAYER || dungeon[(y + 2)*maxcols + x].top == PLAYER || dungeon[(y + 1)*maxcols + x].top == PLAYER) {
						pos = i;
						actives.erase(actives.begin() + pos);

						dungeonText.push_back("A speed 3 spike smacked you on the head for " + to_string(spike->getDmg()) + "!\n");
						player.at(0).setHP(player.at(0).getHP() - spike->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							dungeon[py*maxcols + px].top = 'X';
						}
					}
					//	if spike does not hit a wall
					else if (y + 3 < maxrows - 1) {
						spike->setPosY(y + 3);
						actives.at(i) = spike;
						dungeon[(y + 3)*maxcols + x].traptile = SPIKE;
						dungeon[(y + 3)*maxcols + x].trap = true;
					}
					//	if spike would overshoot, move it to the bottom
					else if (y + 3 >= maxrows - 1 && y != maxrows - 2) {
						spike->setPosY(maxrows - 2);
						actives.at(i) = spike;
						dungeon[(maxrows - 2)*maxcols + x].traptile = SPIKE;
						dungeon[(maxrows - 2)*maxcols + x].trap = true;
					}
					//	else disappear
					else {
						pos = i;
						actives.erase(actives.begin() + pos);
					}
					break;
				default:
					break;
				}
				dungeon[y*maxcols + x].traptile = SPACE;
				dungeon[y*maxcols + x].trap = false;
				spike.reset();
			}
			else if (actives.at(i)->getItem() == "spike trap") {
				shared_ptr<SpikeTrap> spiketrap = dynamic_pointer_cast<SpikeTrap>(actives.at(i));
				spiketrap->cycle(player.at(0));

				// assigns new sprites to the trap sprites vector for proper lighting updating
				x = spiketrap->getPosX();
				y = spiketrap->getPosY();
				int pos = findTrap(x, y, getTraps());
				getTraps().at(pos) = spiketrap;

				/*
				x = spiketrap->getPosX();
				y = spiketrap->getPosY();

				if (dungeon[y*maxcols + x].traptile == SPIKETRAP_ACTIVE) {
					// retract spiketrap
					removeSprite(trap_sprites, maxrows, x, y);
					//addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Deactive_48x48.png");
					actives.at(i)->setSprite(createSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Deactive_48x48.png"));

					spiketrap->setCountdown(spiketrap->getSpeed());
					actives.at(i) = spiketrap;
					dungeon[y*maxcols + x].traptile = SPIKETRAP_DEACTIVE;
				}

				if (spiketrap->getCountdown() == 1) {
					// spiketrap is primed

					// if player is too far, don't play the sound
					if (abs(px - x) + abs(py - y) < 12) {
						cocos2d::experimental::AudioEngine::play2d("Spiketrap_Primed.mp3", false, (float)exp(-(abs(px - x) + abs(py - y)) / 2));
					}
					removeSprite(trap_sprites, maxrows, x, y);
					//addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Primed_48x48.png");
					actives.at(i)->setSprite(createSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Primed_48x48.png"));

					spiketrap->setCountdown(spiketrap->getCountdown() - 1);
					actives.at(i) = spiketrap;
					dungeon[y*maxcols + x].traptile = SPIKETRAP_PRIMED;
				}
				else if (spiketrap->getCountdown() > 1) {
					spiketrap->setCountdown(spiketrap->getCountdown() - 1);
					actives.at(i) = spiketrap;
				}
				else {
					// spiketrap is launched

					// if player is too far, don't play the sound
					if (abs(px - x) + abs(py - y) < 12) {
						cocos2d::experimental::AudioEngine::play2d("Spiketrap_Active.mp3", false, (float)exp(-(abs(px - x) + abs(py - y)) / 2));
					}
					removeSprite(trap_sprites, maxrows, x, y);
					//addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Active_48x48.png");
					actives.at(i)->setSprite(createSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Active_48x48.png"));

					dungeon[y*maxcols + x].traptile = SPIKETRAP_ACTIVE;

					// check if player was hit
					if (px == x && py == y) {
						cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

						dungeonText.push_back("A spike trap bludgeons you for " + to_string(spiketrap->getDmg()) + " damage!\n");
						player.at(0).setHP(player.at(0).getHP() - spiketrap->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("spike trap");
							dungeon[py*maxcols + px].top = 'X';
						}
					}
				}
				*/

				spiketrap.reset();
			}
			else if (actives.at(i)->getItem() == "trigger spike trap") {
				shared_ptr<TriggerSpike> ts = dynamic_pointer_cast<TriggerSpike>(actives.at(i));
				ts->trigger(player.at(0));

				//x = ts->getPosX();
				//y = ts->getPosY();

				/*
				// if the spike trap was triggered
				if (ts->isTriggered()) {
					// spiketrap is launched
					if (abs(px - x) + abs(py - y) < 15) {
						cocos2d::experimental::AudioEngine::play2d("Spiketrap_Active.mp3", false, 1.0f);
					}
					removeSprite(trap_sprites, maxrows, x, y);
					//addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Active_48x48.png");
					actives.at(i)->setSprite(createSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Active_48x48.png"));

					dungeon[y*maxcols + x].traptile = TRIGGER_SPIKE_ACTIVE;

					//	check if player was still on top
					if (dungeon[y*maxcols + x].hero) {
						player.at(0).setHP(player.at(0).getHP() - ts->getDmg());
						dungeonText.push_back("You were bludgeoned by a spike for " + to_string(ts->getDmg()) + " damage!\n");

						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("trigger spike trap");
							dungeon[py*maxcols + px].top = 'X';
						}
					}

					ts->toggleTrigger();
					actives.at(i) = ts;
					ts.reset();
				}
				else {
					// else if the trap wasn't triggered, check if player is standing on top of it
					if (dungeon[y*maxcols + x].hero) {
						ts->toggleTrigger();
						actives.at(i) = ts;
						ts.reset();
						dungeonText.push_back("*tink*\n");

						// spiketrap is primed
						cocos2d::experimental::AudioEngine::play2d("Spiketrap_Primed.mp3", false, 1.0f);
						removeSprite(trap_sprites, maxrows, x, y);
						//addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Primed_48x48.png");
						actives.at(i)->setSprite(createSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Primed_48x48.png"));
					}
					else {
						// retract spiketrap
						cocos2d::experimental::AudioEngine::play2d("Spiketrap_Deactive.mp3", false, 1.0f);
						removeSprite(trap_sprites, maxrows, x, y);
						//addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Deactive_48x48.png");
						actives.at(i)->setSprite(createSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Deactive_48x48.png"));

						dungeon[y*maxcols + x].traptile = TRIGGER_SPIKE_DEACTIVE;
					}
				}
				*/
				ts.reset();

			}
			else if (actives.at(i)->getItem() == "door") {
				shared_ptr<Door> door = dynamic_pointer_cast<Door>(actives.at(i));
				x = door->getPosX();
				y = door->getPosY();

				bool enemy, hero;
				enemy = dungeon[y*maxcols + x].enemy;
				hero = dungeon[y*maxcols + x].hero;

				// if door is locked, check if room is clear to unlock
				if (door->isLocked()) {
					if (roomCleared(m_maze3, m_f3monsters, maxrows, maxcols)) {
						toggleDoorLocks(x, y, dungeon[y*maxcols + x].traptile);
					}
				}
				// if door is unlocked, open, isn't being held, and nothing is in the way
				else if (door->isOpen() && !door->isHeld() && !(enemy || hero) && (dungeon[y*maxcols + x].top == DOOR_H_OPEN || dungeon[y*maxcols + x].top == DOOR_V_OPEN)) {
					// close the door
					cocos2d::experimental::AudioEngine::play2d("Door_Closed.mp3", false, 1.0f);

					// show the room the player is in
					revealRoom();

					// replace sprite with closed door
					//removeSprite(wall_sprites, maxrows, x, y);
					if (dungeon[y*maxcols + x].traptile == DOOR_H)
						addSprite(door_sprites, maxrows, x, y, -2, "Door_Horizontal_Closed_48x48.png");
					else
						addSprite(door_sprites, maxrows, x, y, -2, "Door_Vertical_Closed_48x48.png");

					dungeon[y*maxcols + x].top = (dungeon[y*maxcols + x].traptile == DOOR_H ? DOOR_H : DOOR_V);
					door->toggleOpen();
					door->toggleHeld();
					actives.at(i) = door;
					
					// if the room the player is now in isn't cleared, lock all doors
					if (!roomCleared(m_maze3, m_f3monsters, maxrows, maxcols)) {
						toggleDoorLocks(x, y, dungeon[y*maxcols + x].traptile);
					}
				}
				else if (door->isOpen() && door->isHeld()) {
					// release hold on the door
					door->toggleHeld();
					actives.at(i) = door;
				}
				door.reset();
			}
		}
	}
}
bool ThirdFloor::roomCleared(std::vector<_Tile> &dungeon, std::vector<std::shared_ptr<Monster>> monsters, const int maxrows, const int maxcols) {
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	// find the chunk the player is in
	int xchunk = px / 14;
	int ychunk = py / 10;

	int mx, my, mxchunk, mychunk;
	for (int i = 0; i < monsters.size(); i++) {
		mx = monsters.at(i)->getPosX(); mxchunk = mx / 14;
		my = monsters.at(i)->getPosY(); mychunk = my / 10;

		if (mxchunk == xchunk && mychunk == ychunk) {
			return false;
		}
	}

	return true;
}
void ThirdFloor::toggleDoorLocks(int dx, int dy, char doortype) {
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	cocos2d::experimental::AudioEngine::play2d("Door_Locked.mp3", false, 1.0f);

	int x, y;
	std::string item;

	// door is oriented vertically
	if (doortype == DOOR_V || doortype == DOOR_V_LOCKED) {
		// if player is to the left of the given door
		if (px < dx) {
			// find the doors to lock/unlock
			for (int i = 0; i < m_f3actives.size(); i++) {
				item = m_f3actives.at(i)->getItem();
				if (item == "door") {
					shared_ptr<Door> door = dynamic_pointer_cast<Door>(m_f3actives.at(i));
					x = door->getPosX();
					y = door->getPosY();

					if ((x == dx - 14 && y == dy) || (x == dx && y == dy)) {
						door->toggleLock();
						m_f3actives.at(i) = door;
						m_maze3[y*MAXCOLS3 + x].top = (door->isLocked() ? DOOR_V_LOCKED : DOOR_V);
					}
					else if ((x == dx - 7 && y == dy - 5) || (x == dx - 7 && y == dy + 5)) {
						door->toggleLock();
						m_f3actives.at(i) = door;
						m_maze3[y*MAXCOLS3 + x].top = (door->isLocked() ? DOOR_H_LOCKED : DOOR_H);
					}
					door.reset();
				}
			}

		}
		// else if player is to the right of the given door
		else {
			for (int i = 0; i < m_f3actives.size(); i++) {
				item = m_f3actives.at(i)->getItem();
				if (item == "door") {
					shared_ptr<Door> door = dynamic_pointer_cast<Door>(m_f3actives.at(i));
					x = door->getPosX();
					y = door->getPosY();

					if ((x == dx + 14 && y == dy) || (x == dx && y == dy)) {
						door->toggleLock();
						m_f3actives.at(i) = door;
						m_maze3[y*MAXCOLS3 + x].top = (door->isLocked() ? DOOR_V_LOCKED : DOOR_V);
					}
					else if ((x == dx + 7 && y == dy - 5) || (x == dx + 7 && y == dy + 5)) {
						door->toggleLock();
						m_f3actives.at(i) = door;
						m_maze3[y*MAXCOLS3 + x].top = (door->isLocked() ? DOOR_H_LOCKED : DOOR_H);
					}
					door.reset();
				}
			}
		}
	}
	// door is oriented horizontally
	else {
		// if player is above the given door
		if (py < dy) {
			// find the doors to lock/unlock
			for (int i = 0; i < m_f3actives.size(); i++) {
				item = m_f3actives.at(i)->getItem();
				if (item == "door") {
					shared_ptr<Door> door = dynamic_pointer_cast<Door>(m_f3actives.at(i));
					x = door->getPosX();
					y = door->getPosY();

					if ((x == dx && y == dy - 10) || (x == dx && y == dy)) {
						door->toggleLock();
						m_f3actives.at(i) = door;
						m_maze3[y*MAXCOLS3 + x].top = (door->isLocked() ? DOOR_H_LOCKED : DOOR_H);
					}
					else if ((x == dx - 7 && y == dy - 5) || (x == dx + 7 && y == dy - 5)) {
						door->toggleLock();
						m_f3actives.at(i) = door;
						m_maze3[y*MAXCOLS3 + x].top = (door->isLocked() ? DOOR_V_LOCKED : DOOR_V);
					}
					door.reset();
				}
			}

		}
		// else if player is below the given door
		else {
			for (int i = 0; i < m_f3actives.size(); i++) {
				item = m_f3actives.at(i)->getItem();
				if (item == "door") {
					shared_ptr<Door> door = dynamic_pointer_cast<Door>(m_f3actives.at(i));
					x = door->getPosX();
					y = door->getPosY();

					if ((x == dx && y == dy + 10) || (x == dx && y == dy)) {
						door->toggleLock();
						m_f3actives.at(i) = door;
						m_maze3[y*MAXCOLS3 + x].top = (door->isLocked() ? DOOR_H_LOCKED : DOOR_H);
					}
					else if ((x == dx - 7 && y == dy + 5) || (x == dx + 7 && y == dy + 5)) {
						door->toggleLock();
						m_f3actives.at(i) = door;
						m_maze3[y*MAXCOLS3 + x].top = (door->isLocked() ? DOOR_V_LOCKED : DOOR_V);
					}
					door.reset();
				}
			}
		}
	}
}

void ThirdFloor::hideRooms() {
	// hide all elements
	for (int i = 0; i < wall_sprites.size(); i++) {
		wall_sprites[i]->setVisible(false);
	}
	for (int i = 0; i < monster_sprites.size(); i++) {
		monster_sprites[i]->setVisible(false);
	}
	for (int i = 0; i < item_sprites.size(); i++) {
		item_sprites[i]->setVisible(false);
	}
	for (int i = 0; i < getTraps().size(); i++) {
		getTraps()[i]->getSprite()->setVisible(false);
	}
	for (auto &it : spinner_sprites) {
		it.second->setVisible(false);
	}
	for (auto &it : zapper_sprites) {
		it.second->setVisible(false);
	}

	revealRoom();
}
void ThirdFloor::revealRoom() {
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	// find the chunk the player is in
	int xchunk = px / 14;
	int ychunk = py / 10;

	Vec2 spritePoint;
	int sx, sy, sxchunk, sychunk;
	/// x*SPACING_FACTOR - X_OFFSET, (maxrows - y)*SPACING_FACTOR - Y_OFFSET

	for (int i = 0; i < wall_sprites.size(); i++) {
		spritePoint = wall_sprites[i]->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			wall_sprites[i]->setVisible(true);
		}
	}
	/*for (int i = 0; i < monster_sprites.size(); i++) {
		spritePoint = monster_sprites[i]->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			monster_sprites[i]->setVisible(true);
		}
	}*/
	for (int i = 0; i < m_f3monsters.size(); i++) {
		spritePoint = m_f3monsters[i]->getSprite()->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			m_f3monsters[i]->getSprite()->setVisible(true);
		}
	}
	for (int i = 0; i < item_sprites.size(); i++) {
		spritePoint = item_sprites[i]->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			item_sprites[i]->setVisible(true);
		}
	}
	for (int i = 0; i < trap_sprites.size(); i++) {
		spritePoint = trap_sprites[i]->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			trap_sprites[i]->setVisible(true);
		}
	}
	for (auto &it : spinner_sprites) {
		spritePoint = it.second->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			it.second->setVisible(true);
		}
	}
	for (auto &it : zapper_sprites) {
		spritePoint = it.second->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			it.second->setVisible(true);
		}
	}
}

/*
void ThirdFloor::showDungeon() {
	//	adjust window to be more centered
	cout << "\n\n\n\n";
	cout << "       ";

	_Tile *tile;
	for (int i = 0; i < MAXROWS3; i++) {
		for (int j = 0; j < MAXCOLS3; j++) {
			tile = &m_maze3[i*MAXCOLS3 + j];
			if (tile->top == SPACE && tile->projectile == SPACE && tile->traptile != SPACE)
				cout << tile->traptile;
			else if (tile->top == SPACE && tile->projectile != SPACE)
				cout << tile->projectile;
			else if (tile->top == SPACE)
				cout << tile->bottom;
			else
				cout << tile->top;
		}
		cout << endl;
		cout << "       ";
	}
	cout << endl;

	cout << "       ";
	cout << "HP: " << player.at(0).getHP() << ", Armor: " << player.at(0).getArmor() \
		<< ", Strength: " << player.at(0).getStr() << ", Dexterity: " << player.at(0).getDex() \
		<< " | Current Weapon: " << player.at(0).getWeapon().getAction() << endl;
	cout << "       ";
	cout << endl;
}
*/

std::vector<_Tile>& ThirdFloor::getDungeon() {
	return m_maze3;
}
int ThirdFloor::getRows() const {
	return m_rows;
}
int ThirdFloor::getCols() const {
	return m_cols;
}
std::vector<std::shared_ptr<Monster>>& ThirdFloor::getMonsters() {
	return m_f3monsters;
}
std::vector<std::shared_ptr<Objects>>& ThirdFloor::getActives() {
	return m_f3actives;
}
std::vector<std::shared_ptr<Objects>>& ThirdFloor::getTraps() {
	return m_traps;
}


void ThirdFloor::TLChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	char dv = DOOR_V;
	char dh = DOOR_H;
	char unb = UNBREAKABLE_WALL;

	char c1 = SPIKETRAP_DEACTIVE;
	std::vector<std::vector<char>> one = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', 'C', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'G', ' ', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', 'C', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> two = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',  c1,  c1,  c1, ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',  c1,c_BOMB,  c1, ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ',  c1,  c1,  c1, ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> three = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								   {'#', 'K', '#', '#', ' ', ' ', '#', ' ', ' ', '#', ' ', ' ', '#', unb},
								   {'#', '#', '#', '#', '#', ' ',LAVA, ' ', ' ', ' ', '#', ' ', ' ',  dv},
								   {'#', 'K', '#', '#', ' ', ' ', '#', ' ', ' ', '#', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c5 = c_LIFEPOT;
	std::vector<std::vector<char>> four = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', ' ', ' ', ' ', '#', ' ', 'G', ' ', '#', ' ', ' ', ' ', '#', unb},
								  {'#', ' ', ' ', ' ', '#', ' ',  c5, ' ', '#', ' ', ' ', ' ', ' ',  dv},
								  {'#', ' ', ' ', ' ', '#', ' ', 'G', ' ', '#', ' ', ' ', ' ', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c2 = c_BROWN_CHEST;
	char c3 = c_SILVER_CHEST;
	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								  {'#',  c3, '#', '#', '#', '#', 'W', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#',  c2, ' ', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', ' ', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  dv},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c4 = c_BOMB;
	std::vector<std::vector<char>> six = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', unb},
								 {'#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', unb},
								 {'#', ' ', '#', '#', '#',  c2,  c4,  c2, '#', '#', '#', ' ', ' ',  dv},
								 {'#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', unb},
								 {'#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> seven = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
						           {'#', ' ', ' ', ' ',  c1, ' ', ' ', ' ',  c1, ' ', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', unb},
								   {'#', ' ', 'G', ' ', '#', ' ', ' ', ' ', '#', ' ', 'C', ' ', ' ',  dv},
								   {'#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', ' ',  c1, ' ', ' ', ' ',  c1, ' ', ' ', ' ', '#', unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eight = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> nine = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> ten = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> twelve = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> thirteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fourteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fifteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	// contains a single corner chunk
	pickSingleChunk(current, pool, maxcols);
}
void ThirdFloor::topChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	char dv = DOOR_V;
	char dh = DOOR_H;
	char unb = UNBREAKABLE_WALL;

	std::vector<std::vector<char>> one = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', 'Z', '#', 'Z', ' ', ' ', ' ', ' ', ' ',  dv},
							 	 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> two = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', 'S', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', 'S', '#', unb},
								 {'#',LAVA, '#', ' ', 'Z', ' ', '#', ' ', 'Z', ' ', '#',LAVA, '#', unb},
								 {'#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', unb},
								 {' ', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', ' ',  dv},
								 {'#', ' ', 'Z', ' ', '#', ' ', '#', ' ', '#', ' ', 'Z', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', '#', ' ', 'S', ' ', '#', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', 'S', '#', ' ', ' ', ' ', '#', 'S', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> three = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', unb},
								 {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', 'K', '#', 'G', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								 {'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c1 = TRIGGER_SPIKE_DEACTIVE;
	std::vector<std::vector<char>> four = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', 'C', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', 'C', '#', ' ', '#', unb},
								 {'#', ' ', ' ', '#', '#', '#',  c1,  c1, '#', 'C', '#', ' ', '#', unb},
								 {' ', ' ', ' ', '#', 'A', '#',  c1,  c1, '#', '#', '#', ' ', ' ',  dv},
								 {'#', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c2 = c_GOLDEN_CHEST;
	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#',  c2, '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', ' ',LAVA, ' ', '#', '#', '#', '#', '#', unb},
							 	  {'#', '#', '#', '#', ' ', '#', ' ', '#', ' ', '#', '#', '#', '#', unb},
								  {'#', '#', '#', ' ', '#', '#', ' ', '#', '#', ' ', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> six = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', 'G', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'K', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ',  c1, ' ', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', 'C', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'A', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c3 = SPIKETRAP_DEACTIVE;
	std::vector<std::vector<char>> seven = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#',  c3, ' ', ' ', 'C',  c3, '#',  c3, 'C', ' ', ' ',  c3, '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'C', ' ', 'C', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ',  c3, ' ', ' ',  c3, ' ', ' ',  c3, ' ', ' ', '#', unb},
								 {' ',  c3, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c3, ' ',  dv},
								 {'#', ' ', ' ',  c3, ' ', ' ', ' ', ' ', ' ',  c3, ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',  c3,  c3,  c3, ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eight = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'G', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', '#', ' ', 'A', ' ', '#', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', unb},
								 {' ', ' ', '#', ' ', 'S', ' ', '#', ' ', 'S', ' ', '#', ' ', ' ',  dv},
								 {'#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', '#', ' ', 'A', ' ', '#', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char bee = BOMBEE;
	std::vector<std::vector<char>> nine = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#',  c2, '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', unb},
								  {'#', '#', unb, '#', '#', '#', '#', '#', '#', '#', unb, '#', '#', unb},
								  {'#', ' ', ' ', unb, '#', '#', '#', '#', '#', unb, ' ', ' ', '#', unb},
								  {' ', ' ', ' ', ' ', unb, '#', bee, '#', unb, ' ', ' ', ' ', ' ',  dv},
								  {'#', ' ', ' ', ' ', '#', '#',LAVA, '#', '#', ' ', ' ', ' ', '#', unb},
								  {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', unb},
								  {'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> ten =  { {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> twelve = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> thirteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fourteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fifteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	pool.push_back(eight);
	pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	mixChunks(current, pool, maxcols);
}
std::vector<char> ThirdFloor::TRChunks(std::vector<std::vector<std::vector<char>>> current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	char dv = DOOR_V;
	char dh = DOOR_H;
	char unb = UNBREAKABLE_WALL;

	std::vector<std::vector<char>> one = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', '#', ' ', 'C', 'C', 'C', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'C', 'C', 'C', ' ', '#', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c1 = TRIGGER_SPIKE_DEACTIVE;
	char c2 = SPIKETRAP_DEACTIVE;
	std::vector<std::vector<char>> two = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',  c2,  c1,  c2, ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',  c1,  c2,  c1, ' ', ' ', 'Z', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',  c1,  c1,  c1, ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', 'K', ' ', ' ',  c1,  c2,  c1, ' ', ' ', ' ', 'Z', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',  c1,  c1,  c1, ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',  c1,  c2,  c1, ' ', ' ', 'Z', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',  c2,  c1,  c2, ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> three = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', 'K', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', ' ', '#', unb},
								   {'#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', 'A', '#', unb},
								   {'#', ' ', 'W', ' ', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', ' ', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c3 = c_ARMOR;
	std::vector<std::vector<char>> four = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#',  c3,LAVA, '#', '#', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#',LAVA, '#', '#', 'W', ' ', ' ', ' ', ' ', '#', '#', unb},
								  {'#', '#', '#', '#', unb, unb, 'W', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', '#', ' ', unb, unb, 'S', unb, unb, ' ', ' ', '#', unb},
								  {' ', ' ', ' ', ' ', ' ', ' ', unb, unb, unb, unb, '#', ' ', '#', unb},
								  {'#', '#', '#', '#', ' ', ' ', ' ', ' ', unb, ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', unb},
								  {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ',c_BOMB,LAVA, 'A', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> six = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> seven = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eight = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> nine = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> ten = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> twelve = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> thirteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fourteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fifteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool, maxcols);

	return combineChunks(current);
}

void ThirdFloor::leftEdgeChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	char dv = DOOR_V;
	char dh = DOOR_H;
	char unb = UNBREAKABLE_WALL;

	char c1 = c_HEART_POD;
	std::vector<std::vector<char>> one = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', 'C', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								 {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', unb},
								 {'#',  c1, 'S', ' ', ' ', ' ', '#', ' ', ' ', ' ', 'S', ' ', ' ',  dv},
								 {'#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', unb},
								 {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> two = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', '#',c_BOMB, '#', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c2 = TRIGGER_SPIKE_DEACTIVE;
	std::vector<std::vector<char>> three = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								   {'#', ' ', ' ', ' ', ' ',  c2, ' ',  c2, ' ', ' ', ' ', ' ', '#', unb},
								   {'#', 'C', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', 'G', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ',  dv},
								   {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', 'C', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', ' ', ' ', ' ', ' ',  c2, ' ',  c2, ' ', ' ', ' ', ' ', '#', unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> four ={ {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', 'A', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'A', '#', unb},
								 {'#', ' ', ' ', ' ',LAVA,LAVA,LAVA,LAVA,LAVA, ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ',LAVA,LAVA,LAVA, ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ',LAVA, ' ', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', '#', ' ', ' ',LAVA, ' ', ' ', '#', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', 'A', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'A', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	//char c3 = HEART_POD;
	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', 'W', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', 'W', 'C', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', 'C', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', ' ',  dv},
								  {'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								  {'#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', '#', '#', ' ', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', ' ', '#',  c1, '#', ' ', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> six = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> seven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> eight = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> nine = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> ten = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> twelve = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> thirteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fourteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fifteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool, maxcols);

}
void ThirdFloor::middleChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	char dv = DOOR_V;
	char dh = DOOR_H;
	char unb = UNBREAKABLE_WALL;

	char c1 = c_HEART_POD; // heart pod
	std::vector<std::vector<char>> one = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', 'C', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								 {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', unb},
								 {' ',  c1, 'S', ' ', ' ', ' ', '#', ' ', ' ', ' ', 'S', ' ', ' ',  dv},
								 {'#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', unb},
								 {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c2 = ROUNDABOUT;
	char c3 = TRIGGER_SPIKE_DEACTIVE;
	std::vector<std::vector<char>> two = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', 'S', ' ', '#', '#', '#', ' ', '#', '#', '#', ' ', 'S', '#', unb},
								 {'Z', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'Z', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', unb},
								 {'#', ' ', ' ', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ',  c3,  c2,c_BOMB,  c2,  c3, ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', unb},
								 {'Z', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'Z', unb},
								 {'#', 'S', ' ', '#', '#', '#', ' ', '#', '#', '#', ' ', 'S', '#', unb}, };

	std::vector<std::vector<char>> three = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								   {'#', ' ', ' ', ' ', ' ',  c3, ' ',  c3, ' ', ' ', ' ', ' ', '#', unb},
								   {'#', 'C', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', unb},
								   {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {' ', ' ', ' ', 'G', ' ', ' ', '#', ' ', ' ', ' ', 'Z', ' ', ' ',  dv},
								   {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', 'C', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', unb},
								   {'#', ' ', ' ', ' ', ' ',  c3, ' ',  c3, ' ', ' ', ' ', ' ', '#', unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c4 = SPIKETRAP_DEACTIVE;
	std::vector<std::vector<char>> four = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#',  c4,  c3,  c3,  c3,  c4,  c4,  c4,  c3,  c3,  c3,  c4, '#', unb},
								  {'#',  c3,  c3,  c3,  c4, '#', '#', '#',  c4,  c3,  c3,  c3, '#', unb},
								  {'#',  c3,  c3,  c4, '#', ' ', '#', '#', '#',  c4,  c3,  c3, '#', unb},
								  {' ',  c4,  c3,  c4, '#', '#', 'W', '#', '#',  c4,  c3,  c3, ' ',  dv},
								  {'#',  c3,  c3,  c4, '#', '#', '#', '#', '#',  c4,  c3,  c3, '#', unb},
								  {'#',  c3,  c3,  c3,  c4, '#', '#', '#',  c4,  c3,  c3,  c3, '#', unb},
								  {'#',  c4,  c3,  c3,  c3,  c4,  c4,  c4,  c3,  c3,  c3,  c4, '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#',  c3,  c3, ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c3,  c3, '#', unb},
								  {'#',  c3, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c3, '#', unb},
							 	  {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
							   	  {' ', ' ', ' ', ' ', 'C', ' ', 'S', ' ', 'C', ' ', ' ', ' ', ' ',  dv},
								  {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#',  c3, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c3, '#', unb},
								  {'#',  c3,  c3, ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c3,  c3, '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> six = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', '#',c_BOMB, '#', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char bee = BOMBEE;
	std::vector<std::vector<char>> seven = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								   {'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', unb},
								   {'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
								   {' ', ' ', ' ', ' ', ' ', '#', 'S', '#', ' ', ' ', ' ', ' ', ' ',  dv},
								   {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', '#', '#', 'A', ' ', ' ', ' ', ' ', ' ', 'A', '#', '#', '#', unb},
								   {'#', '#', '#', '#', 'C', ' ', ' ', ' ', 'C', '#', '#', '#', '#', unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eight = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								   {'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', unb},
								   {'#', '#', '#', ' ', ' ', '#', '#', '#', ' ', ' ', '#', '#', '#', unb},
								   {'#', ' ', ' ', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', unb},
								   {' ', ' ', 'S', ' ', '#', '#', bee, '#', '#', ' ', 'S', ' ', ' ',  dv},
								   {'#', ' ', ' ', ' ', '#', '#', 'W', '#', '#', ' ', ' ', ' ', '#', unb},
							       {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', unb},
							       {'#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> nine = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c4,  c4, '#', unb},
								  {'#', 'Z', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c4, '#', unb},
								  {'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {' ', ' ', ' ', '#', 'A', '#', '#', '#', 'A', '#', ' ', ' ', ' ',  dv},
								  {'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#',  c4, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'Z', '#', unb},
								  {'#',  c4,  c4, ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> ten = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', ' ', '#', ' ', bee, '#', '#', '#', ' ', ' ', '#', ' ', '#', unb},
								 {' ', ' ', 'W', ' ', ' ', '#',c_BOMB, '#', ' ', ' ', 'W', ' ', ' ',  dv},
								 {'#', ' ', '#', ' ', ' ', '#', '#', '#', bee, ' ', '#', ' ', '#', unb},
								 {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> twelve = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> thirteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fourteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fifteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	pool.push_back(eight);
	pool.push_back(nine);
	pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	mixChunks(current, pool, maxcols);
}
std::vector<char> ThirdFloor::rightEdgeChunks(std::vector<std::vector<std::vector<char>>> current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	char dv = DOOR_V;
	char dh = DOOR_H;
	char unb = UNBREAKABLE_WALL;

	char c1 = ROUNDABOUT;
	char c2 = SPIKETRAP_DEACTIVE;
	std::vector<std::vector<char>> one = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', 'W', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ',  c2, ' ', '#', unb},
								 {'#', ' ', '#', '#', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', '#', '#', ' ', 'C', '#', ' ',  c1, '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', '#', 'W', '#', 'C', ' ', ' ', '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> two = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ',LAVA, '#', unb},
								 {'#', ' ',LAVA, ' ', ' ', ' ', '#', ' ', ' ', ' ',LAVA,LAVA, '#', unb},
								 {' ', ' ',LAVA, ' ', ' ', 'K', 'W', 'K', ' ', ' ',LAVA,LAVA, '#', unb},
								 {'#', ' ',LAVA, ' ', ' ', ' ', '#', ' ', ' ', ' ',LAVA,LAVA, '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ',LAVA, '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	char c3 = TRIGGER_SPIKE_DEACTIVE;
	std::vector<std::vector<char>> three = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', 'Z', ' ', '#',  c2, '#', ' ', ' ', ' ', '#', '#', unb},
								 {' ', ' ', 'S', '#', ' ',  c2, 'S',  c2, ' ', ' ', ' ', 'C', '#', unb},
								 {'#', '#',  c3, 'Z', ' ', '#',  c2, '#', ' ', ' ', ' ', '#', '#', unb},
								 {'#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> four = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', 'C', '#', 'K', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ',  c1, ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ',  c3,  c3,  c3,  c1, '#',  c1,  c3,  c3,  c3, ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c1, ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> six = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> seven = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eight = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> nine = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
							 	  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
							 	  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
							 	  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> ten = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> twelve = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> thirteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fourteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fifteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool, maxcols);

	return combineChunks(current);
}

void ThirdFloor::BLChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	char dv = DOOR_V;
	char dh = DOOR_H;
	char unb = UNBREAKABLE_WALL;

	char c1 = ROUNDABOUT;
	std::vector<std::vector<char>> one = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', ' ', ' ', ' ', '#', ' ', '#', '#', 'C', ' ', ' ', '#', unb},
								 {'#', ' ',LAVA, ' ', ' ', '#', ' ', '#', '#', ' ', '#', ' ', '#', unb},
								 {'#', 'A',LAVA, ' ', ' ', ' ', 'Z', '#', '#', ' ', '#', ' ', '#', unb},
								 {'#', ' ',LAVA, ' ', ' ', '#', '#', 'Z', ' ', ' ', '#', ' ', ' ',  dv},
								 {'#', '#', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', '#', '#', ' ', '#', 'C', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> two = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#',  c1, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c1, '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'K', 'S', 'K', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#',  c1, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c1, '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	char c2 = SPIKETRAP_DEACTIVE;
	char c3 = ROUNDABOUT;
	std::vector<std::vector<char>> three = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'G', unb},
								 {'#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', ' ',  c2, ' ', '#', '#', '#',  c3, ' ', ' ', ' ', '#', unb},
								 {'#', '#', ' ', '#', ' ', ' ', ' ', '#', '#', ' ', ' ', '#', '#',  dv},
								 {'#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', ' ', ' ', '#', unb},
								 {'#', '#', '#', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#',c_ARMOR,'#', ' ', ' ', ' ', ' ', ' ', '#',c_BOMB, ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> four = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'Z', '#', 'Z', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'Z', '#', 'Z', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> six = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'Z', '#', 'Z', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> seven = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'Z', '#', 'Z', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eight = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'Z', '#', 'Z', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> nine = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'Z', '#', 'Z', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> ten = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'Z', '#', 'Z', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> twelve = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> thirteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fourteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fifteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	//pool.push_back(four);
	//pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool, maxcols);
}
void ThirdFloor::bottomChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	char dv = DOOR_V;
	char dh = DOOR_H;
	char unb = UNBREAKABLE_WALL;

	char c1 = SPIKETRAP_DEACTIVE;
	char c2 = c_STATPOT;
	std::vector<std::vector<char>> one = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#',  c1,  c1, ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c1,  c1, '#', unb},
								 {'#',  c1, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c1, '#', unb},
								 {'#', ' ', ' ', ' ', ' ', 'S', '#', 'S', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', 'W', '#', '#', '#', 'W', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', '#', unb},
								 {'#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#',  c2, '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> two = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#',  c1, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c1, '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', 'C', '#', 'C', ' ', ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#',  c1, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c1, '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	char ra = ROUNDABOUT;
	char ts = TRIGGER_SPIKE_DEACTIVE;
	std::vector<std::vector<char>> three = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', ' ', ' ', '#', ' ', ' ', ' ', '#', '#', unb},
								 {'#', ' ', ' ', ' ', '#', '#', ' ', '#', ' ', ' ', ' ', ' ',  ra, unb},
								 {'#', ' ', '#', ' ', ' ', '#', ' ', '#',  ra, '#', ' ', ' ', '#', unb},
								 {'#', ' ', '#', ' ', '#',  ra, ' ',  ra, '#', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ',  dv},
								 {'#',  ra, '#', ' ', '#',  ra, ' ',  ra, '#', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', '#', unb},
								 {'#',  ra, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',  ra, '#', unb}, };

	char bee = BOMBEE;
	std::vector<std::vector<char>> four = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, '#', unb},
								  {'#',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, '#', unb},
								  {'#',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, '#', unb},
								  {' ', ' ', ' ', 'S',LAVA,  c1,LAVA,  c1,LAVA, 'S', ' ', ' ', ' ',  dv},
								  {'#',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, '#', unb},
								  {'#',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, ' ',  c1, '#', unb},
								  {'#',  c1, ' ',  c1, ' ',  c1, bee,  c1, ' ',  c1, ' ',  c1, '#', unb},
								  {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	char c3 = c_SILVER_CHEST;
	char c4 = BUTTON;
	char c5 = c_LIFEPOT;
	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', ' ', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
								  {'#', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', ' ', '#', '#', unb},
								  {' ', ' ', '#', ' ', ' ', ' ', ' ',LAVA, 'A', '#', ' ', ' ', ' ',  dv},
								  {'#', '#', '#', '#', ' ', unb, unb, '#', '#', '#', ' ', '#', '#', unb},
								  {'#', ' ', ' ', ' ', ' ', unb, '#', ' ', ' ', ' ', ' ', '#', '#', unb},
								  {'#',  c3, '#', '#', '#', unb,  c5, ' ', '#', '#', '#', '#', '#', unb},
								  {'#',  c4, '#', '#', '#', unb, '#', '#', '#', 'S', '#', '#', '#', unb}, };

	char amr = c_ARMOR;
	std::vector<std::vector<char>> six = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', '#', '#', '#', ' ', ' ', ' ', '#', '#', '#', '#', '#', unb},
								 {'#', '#',  ts, ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ts, ' ', '#', unb},
								 {'#', ' ', ' ', '#', ' ', '#', '#', '#', ' ', '#', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ',  ts, '#', amr, '#',  ts, ' ', ' ', ' ', ' ',  dv},
								 {'#', ' ', ' ', ' ', ' ', 'C', '#', 'C', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', 'K', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'G', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> seven = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								   {'#', ' ', ' ', ' ',LAVA, ' ', ' ', ' ',LAVA, ' ', ' ', ' ', '#', unb},
								   {'#', ' ',LAVA, ' ', ' ', ' ', ' ', ' ', ' ', ' ',LAVA, ' ', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', 'W', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {' ', ' ', ' ', ' ', ' ', 'W', '#', 'W', ' ', ' ', ' ', ' ', ' ',  dv},
								   {'#', ' ', ' ', ' ', '#', ' ', 'W', ' ', '#', ' ', ' ', ' ', '#', unb},
								   {'#', ' ',LAVA, ' ', ' ', ' ', '#', ' ', ' ', ' ',LAVA, ' ', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> eight = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
							   	   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#',  c1, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c1, '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {' ', ' ', ' ', ' ', ' ', 'C', '#', 'C', ' ', ' ', ' ', ' ', ' ',  dv},
								   {'#', ' ', ' ', ' ', ' ', ' ', 'C', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#',  c1, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  c1, '#', unb},
								   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> nine = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> ten = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> twelve = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> thirteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fourteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fifteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(six);
	pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	mixChunks(current, pool, maxcols);

	//return combineChunks(c);
}
std::vector<char> ThirdFloor::BRChunks(std::vector<std::vector<std::vector<char>>> current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	char dv = DOOR_V;
	char dh = DOOR_H;
	char unb = UNBREAKABLE_WALL;

	std::vector<std::vector<char>> one = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', '#', '#', unb},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> two = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								 {'#', 'Z', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'K', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> three = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> four = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								 {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								 {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> six = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> seven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> eight = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> nine = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> ten = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> eleven = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> twelve = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> thirteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fourteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	std::vector<std::vector<char>> fifteen = { {'#', '#', '#', '#', ' ', '#', '#', '#', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
								 {'#', '#', '#', '#', ' ', '#', '#', '#', '#'}, };

	pool.push_back(one);
	pool.push_back(two);
	//pool.push_back(three);
	//pool.push_back(four);
	//pool.push_back(five);
	//pool.push_back(six);
	//pool.push_back(six);
	//pool.push_back(seven);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool, maxcols);

	return combineChunks(current);
}

std::vector<std::vector<char>> ThirdFloor::playerChunks() {
	char unb = UNBREAKABLE_WALL;
	char dh = DOOR_H;
	char dv = DOOR_V;
	char inf = c_INFINITY_BOX;
	char st = SPIKETRAP_DEACTIVE;

	std::vector<std::vector<char>> TLplayerchunk = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
										   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', '#', '@', ' ', ' ', ' ', ' ', ' ', ' ',  dv},
										   {'#', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> TRplayerchunk = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
										   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {' ', ' ', ' ', ' ', ' ', ' ', '@', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> BLplayerchunk = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
										   {'#', unb, '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', ' ', unb, ' ', ' ', ' ',  st, ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', unb},
										   {'#', ' ', '#', ' ', ' ', ' ', '@', ' ', ' ', ' ',  st, ' ', ' ',  dv},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', '#', unb},
										   {'#', ' ', unb, ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', '#', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', '#', '#', unb},
										   {'#', unb, '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> BRplayerchunk = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
										   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ',  st,  st,  st, ' ', ' ', ' ', ' ', '#', unb},
										   {' ', ' ', ' ', ' ', ' ',  st, '@',  st, ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ',  st,  st,  st, ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	switch (m_playerchunk) {
	case 1: return TLplayerchunk;
	case 2: return TRplayerchunk;
	case 3: return BLplayerchunk;
	case 4: return BRplayerchunk;
	default: return TLplayerchunk;
	}
}
std::vector<std::vector<char>> ThirdFloor::exitChunks() {
	char str = STAIRS;
	char unb = UNBREAKABLE_WALL;
	char dh = DOOR_H;
	char dv = DOOR_V;
	char inf = c_INFINITY_BOX;
	char st = SPIKETRAP_DEACTIVE;
	
	std::vector<std::vector<char>> TLexitchunk = {   {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
										   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', '#', str, ' ', ' ', ' ', ' ', ' ', ' ',  dv},
										   {'#', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> TRexitchunk = {   {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
										   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {' ', ' ', ' ', ' ', ' ', ' ', str, ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> BLexitchunk = {   {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
										   {'#', unb, '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
										   {'#', unb, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', ' ', unb, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', unb, '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', inf, ' ', '#', '#', ' ', str, ' ', ' ', ' ', ' ', ' ', ' ',  dv},
										   {'#', ' ', ' ', unb, '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', unb, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', unb, ' ', ' ', ' ', '#', '#', '#', ' ', ' ', ' ', '#', '#', unb},
										   {'#', unb, '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	std::vector<std::vector<char>> BRexitchunk = {   {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
										   {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ',  st, ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', '#',  st, '#', ' ', ' ', ' ', ' ', '#', unb},
										   {' ', ' ', ' ', ' ',  st,  st, str,  st,  st, ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', '#',  st, '#', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', ' ', ' ', ' ', ' ', ' ',  st, ' ', ' ', ' ', ' ', ' ', '#', unb},
										   {'#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', unb},
										   {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb}, };

	switch (m_exitchunk) {
	case 1: return TLexitchunk;
	case 2: return TRexitchunk;
	case 3: return BLexitchunk;
	case 4: return BRexitchunk;
	default: return TLexitchunk;
	}
}

void ThirdFloor::mixChunks(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	for (int i = 1; i < (maxcols - 2) / 13 - 1; i++) {
		current.push_back(pool[randInt(pool.size())]);
	}

	/*int s;
	if (layer == specialChunkLayer1) {
		s = randInt((maxcols - 2) / 4 - 1);
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom1);
	}
	if (layer == specialChunkLayer2) {
		s = randInt((maxcols - 2) / 4 - 1);
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom2);
	}*/
}
void ThirdFloor::pickSingleChunk(std::vector<std::vector<std::vector<char>>> &current, std::vector<std::vector<std::vector<char>>> pool, int maxcols) {
	// selects one random corner chunk and pushes it
	if (m_layer == m_playerchunk && !m_playerplaced) {
		current.push_back(playerChunks());
		m_playerplaced = true;
	}
	else if (m_layer == m_exitchunk && !m_exitplaced) {
		current.push_back(exitChunks());
		m_exitplaced = true;
	}
	else
		current.push_back(pool[randInt(pool.size())]);
}

std::vector<char> ThirdFloor::combineChunks(std::vector<std::vector<std::vector<char>>> c) {
	std::vector<char> v;
	for (int i = 0; i < 10; i++) {			// iterate thru rows of 2d vector
		for (int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 14; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}


//	BEGIN BOSS FIGHT FUNCTIONS
FirstBoss::FirstBoss(Player p) : Dungeon(5) {
	_Tile *tile;
	int i, j;

	for (i = 0; i < BOSSROWS; i++) { //		initialize entire dungeon to blank space
		for (j = 0; j < BOSSCOLS; j++) {
			tile = new _Tile;

			// all layers set to empty space
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = SPACE;
			tile->wall = tile->item = tile->enemy = tile->trap = tile->overlap = tile->enemy_overlap = tile->marked = false;
			tile->hero = false;
			tile->gold = 0;
			tile->overlap_count = tile->enemy_overlap_count = 0;

			m_boss.push_back(*tile);
			delete tile;
		}
	}

	for (i = 0; i < BOSSROWS; i++) { //	initialize top and bottom of dungeon to be unbreakable walls
		for (j = 0; j < BOSSCOLS; j++) {
			m_boss[i*BOSSCOLS + j].top = UNBREAKABLE_WALL;
			m_boss[i*BOSSCOLS + j].bottom = UNBREAKABLE_WALL;
			m_boss[i*BOSSCOLS + j].traptile = UNBREAKABLE_WALL;
			m_boss[i*BOSSCOLS + j].wall = true;
		}
		i += (BOSSROWS - 2);
	}

	for (i = 1; i < BOSSROWS - 1; i++) { //		initialize edges of dungeon to be unbreakable walls
		m_boss[i*BOSSCOLS].top = m_boss[i*BOSSCOLS].bottom = m_boss[i*BOSSCOLS].traptile = UNBREAKABLE_WALL;
		m_boss[i*BOSSCOLS + BOSSCOLS - 1].top = m_boss[i*BOSSCOLS + BOSSCOLS-1].bottom = m_boss[i*BOSSCOLS + BOSSCOLS - 1].traptile = UNBREAKABLE_WALL;
	
		m_boss[i*BOSSCOLS].wall = true;
		m_boss[i*BOSSCOLS + BOSSCOLS - 1].wall = true;
	}


	//	spawn player
	player.at(0) = p;
	player.at(0).setPosX(BOSSCOLS / 2);
	player.at(0).setPosY(BOSSROWS - 3);
	//m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;
	m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;


	//	spawn boss
	Smasher smasher;
	int col = smasher.getPosX();
	int row = smasher.getPosY();
	for (int i = row - 1; i < row + 2; i++) {
		for (int j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
	}
	m_firstbossMonsters.emplace_back(new Smasher(smasher));

	
	/*
	// test enemies
	int n = 60;
	int mx, my;
	char top, upper;
	while (n > 0) {
		Archer archer;
		mx = 1 + randInt(BOSSCOLS - 2);
		my = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
		archer.setPosX(mx);
		archer.setPosY(my);

		top = m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].top;
		upper = m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].upper;
		while (top != SPACE || upper == SMASHER) { 
			mx = 1 + randInt(BOSSCOLS - 2);
			my = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
			archer.setPosX(mx);
			archer.setPosY(my);

			top = m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].top;
			upper = m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].upper;
		}

		m_firstbossMonsters.emplace_back(new Archer(archer));
		m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].top = ARCHER;
		m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].enemy = true;
		n--;
	}
	*/

	//	spawn spike traps
	int x = 1 + randInt(BOSSCOLS - 2);
	int y = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
	int speed = 3 + randInt(3);
	char traptile;

	int m = 12 + randInt(5);
	while (m > 0) {
		SpikeTrap spiketrap(x, y, speed);

		traptile = m_boss[spiketrap.getPosY()*BOSSCOLS + spiketrap.getPosX()].traptile;
		while (traptile != SPACE) { // while spiketrap position clashes with wall, player, or idol
			x = 1 + randInt(BOSSCOLS - 2);
			y = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
			speed = 3 + randInt(3);
			spiketrap.setPosX(x);				// reroll it
			spiketrap.setPosY(y);
			spiketrap.setSpeed(speed);
			spiketrap.setCountdown(speed);

			traptile = m_boss[spiketrap.getPosY()*BOSSCOLS + spiketrap.getPosX()].traptile;
		}

		m_firstbossActives.emplace_back(new SpikeTrap(spiketrap));
		m_boss[spiketrap.getPosY()*BOSSCOLS + spiketrap.getPosX()].traptile = SPIKETRAP_DEACTIVE;
		m_boss[spiketrap.getPosY()*BOSSCOLS + spiketrap.getPosX()].trap = true;
		m--;
	}
}
FirstBoss& FirstBoss::operator=(FirstBoss const &dungeon) {
	for (unsigned i = 0; i < dungeon.getRows()*dungeon.getCols(); i++) {
		m_boss[i] = dungeon.m_boss[i];
	}
	player = dungeon.player;
	m_level = dungeon.getLevel();

	m_firstbossMonsters = dungeon.m_firstbossMonsters;
	m_firstbossActives = dungeon.m_firstbossActives;
	m_traps = dungeon.m_traps;

	return *this;
}

void FirstBoss::peekDungeon(int x, int y, char move) {
	int rows = getRows();
	int cols = getCols();

	char upper, top, bottom, traptile;
	bool wall, item, trap, enemy;

	int initHP = player.at(0).getHP();

	// reset player's blocking stance
	player.at(0).setBlock(false);

	// set player's facing direction
	switch (move) {
	case 'l':
	case 'r':
	case 'u':
	case 'd':
		player.at(0).setFacingDirection(move); break;
	}

	// check for any afflictions
	player.at(0).checkAfflictions();

	// :::: Move player ::::
	int weprange = player.at(0).getWeapon()->getRange();
	bool move_used = false;
	bool far_enemy, close_enemy;

	// if player is not immobilized, move them
	if (!(player.at(0).isStunned() || player.at(0).isFrozen())) {

		// if weapon has a range of 2, check for monsters to attack
		if (weprange == 2) {
			if (move == 'l') { // moved to the left
				top = m_boss[y*BOSSCOLS + x - 1].top;
				close_enemy = m_boss[y*BOSSCOLS + x - 1].enemy;
				far_enemy = m_boss[y*BOSSCOLS + x - 2].enemy;

				if (close_enemy) {
					fight(x - 1, y);
					move_used = true;
				}
				else if (far_enemy && top == SPACE && y*BOSSCOLS + x - 2 > 0) {	// monster encountered
					fight(x - 2, y);
					move_used = true;
				}
			}
			else if (move == 'r') {
				top = m_boss[y*BOSSCOLS + x + 1].top;
				close_enemy = m_boss[y*BOSSCOLS + x + 1].enemy;
				far_enemy = m_boss[y*BOSSCOLS + x + 2].enemy;

				if (close_enemy) {
					fight(x + 1, y);
					move_used = true;
				}
				else if (far_enemy && top == SPACE && y*BOSSCOLS + x + 2 < BOSSROWS * BOSSCOLS) {
					fight(x + 2, y);
					move_used = true;
				}
			}
			else if (move == 'u') { // moved up
				top = m_boss[(y - 1)*BOSSCOLS + x].top;
				close_enemy = m_boss[(y - 1)*BOSSCOLS + x].enemy;
				// if player is along top edge, assign far_enemy to close_enemy instead
				far_enemy = (y == 1 ? close_enemy : m_boss[(y - 2)*BOSSCOLS + x].enemy);
				//far_enemy = m_boss[(y - 2)*BOSSCOLS + x].enemy;

				if (close_enemy) {
					fight(x, y - 1);
					move_used = true;
				}
				else if (far_enemy && top == SPACE && (y - 2)*BOSSCOLS + x > 0) {
					fight(x, y - 2);
					move_used = true;
				}
			}
			else if (move == 'd') { // moved down
				top = m_boss[(y + 1)*BOSSCOLS + x].top;
				close_enemy = m_boss[(y + 1)*BOSSCOLS + x].enemy;
				// if player is along top edge, assign far_enemy to close_enemy instead
				far_enemy = (y == BOSSROWS - 2 ? close_enemy : m_boss[(y + 2)*BOSSCOLS + x].enemy);
				//far_enemy = m_boss[(y + 2)*BOSSCOLS + x].enemy;

				if (close_enemy) {
					fight(x, y + 1);
					move_used = true;
				}
				else if (far_enemy && top == SPACE && (y + 2)*BOSSCOLS + x < BOSSROWS * BOSSCOLS) {
					fight(x, y + 2);
					move_used = true;
				}
			}
		}

		if (!move_used) {

			if (move == 'l') { // moved to the left
				upper = m_boss[y*BOSSCOLS + x - 1].upper;
				top = m_boss[y*BOSSCOLS + x - 1].top;
				bottom = m_boss[y*BOSSCOLS + x - 1].bottom;
				traptile = m_boss[y*BOSSCOLS + x - 1].traptile;

				wall = m_boss[y*BOSSCOLS + x - 1].wall;
				item = m_boss[y*BOSSCOLS + x - 1].item;
				enemy = m_boss[y*BOSSCOLS + x - 1].enemy;
				trap = m_boss[y*BOSSCOLS + x - 1].trap;

				if (!(wall || enemy)) {
					m_boss[y*BOSSCOLS + x].hero = false;
					m_boss[y*BOSSCOLS + x - 1].hero = true;
					player.at(0).setPosX(x - 1);

					if (trap) {
						if (traptile == SPIKE) {
							trapEncounter(*this, x - 1, y);

							m_boss[y*BOSSCOLS + x - 1].traptile = SPACE;
							m_boss[y*BOSSCOLS + x - 1].trap = false;

							if (player.at(0).getHP() <= 0) {
								player.at(0).setDeath("falling spike");
								//clearScreen();
								//showDungeon();
								if (!dungeonText.empty())
									showText();
								return;
							}
						}
						if (traptile == SPIKETRAP_ACTIVE) {
							trapEncounter(*this, x - 1, y);
						}
					}
					if (item) {
						foundItem(m_boss, BOSSCOLS, x - 1, y);
					}
				}
				else if (enemy) {
					fight(x - 1, y);
				}
			}

			else if (move == 'r') { // moved to the right
				upper = m_boss[y*BOSSCOLS + x + 1].upper;
				top = m_boss[y*BOSSCOLS + x + 1].top;
				bottom = m_boss[y*BOSSCOLS + x + 1].bottom;
				traptile = m_boss[y*BOSSCOLS + x + 1].traptile;

				wall = m_boss[y*BOSSCOLS + x + 1].wall;
				item = m_boss[y*BOSSCOLS + x + 1].item;
				enemy = m_boss[y*BOSSCOLS + x + 1].enemy;
				trap = m_boss[y*BOSSCOLS + x + 1].trap;

				if (!(wall || enemy)) {
					m_boss[y*BOSSCOLS + x].hero = false;
					m_boss[y*BOSSCOLS + x + 1].hero = true;
					player.at(0).setPosX(x + 1);

					if (trap) {
						if (traptile == SPIKE) {
							trapEncounter(*this, x + 1, y);

							m_boss[y*BOSSCOLS + x + 1].traptile = SPACE;
							m_boss[y*BOSSCOLS + x + 1].trap = false;

							if (player.at(0).getHP() <= 0) {
								player.at(0).setDeath("falling spike");
								//clearScreen();
								//showDungeon();
								if (!dungeonText.empty())
									showText();
								return;
							}
						}
						if (traptile == SPIKETRAP_ACTIVE) {
							trapEncounter(*this, x + 1, y);
						}
					}
					if (item) {
						foundItem(m_boss, BOSSCOLS, x + 1, y);
					}
				}
				else if (enemy) {
					fight(x + 1, y);
				}
			}

			else if (move == 'u') { // moved up
				upper = m_boss[(y - 1)*BOSSCOLS + x].upper;
				top = m_boss[(y - 1)*BOSSCOLS + x].top;
				bottom = m_boss[(y - 1)*BOSSCOLS + x].bottom;
				traptile = m_boss[(y - 1)*BOSSCOLS + x].traptile;

				wall = m_boss[(y - 1)*BOSSCOLS + x].wall;
				item = m_boss[(y - 1)*BOSSCOLS + x].item;
				enemy = m_boss[(y - 1)*BOSSCOLS + x].enemy;
				trap = m_boss[(y - 1)*BOSSCOLS + x].trap;

				if (!(wall || enemy)) {
					m_boss[y*BOSSCOLS + x].hero = false;
					m_boss[(y - 1)*BOSSCOLS + x].hero = true;
					player.at(0).setPosY(y - 1);

					if (trap) {
						if (traptile == SPIKE) {
							trapEncounter(*this, x, y - 1);

							m_boss[(y - 1)*BOSSCOLS + x].traptile = SPACE;
							m_boss[(y - 1)*BOSSCOLS + x].trap = false;

							if (player.at(0).getHP() <= 0) {
								player.at(0).setDeath("falling spike");
								//clearScreen();
								//showDungeon();
								if (!dungeonText.empty())
									showText();
								return;
							}
						}
						if (traptile == SPIKETRAP_ACTIVE) {
							trapEncounter(*this, x, y - 1);
						}
					}
					if (item) {
						foundItem(m_boss, BOSSCOLS, x, y - 1);
					}
				}
				else if (enemy) {
					fight(x, y - 1);
				}
			}

			else if (move == 'd') { // moved down
				upper = m_boss[(y + 1)*BOSSCOLS + x].upper;
				top = m_boss[(y + 1)*BOSSCOLS + x].top;
				bottom = m_boss[(y + 1)*BOSSCOLS + x].bottom;
				traptile = m_boss[(y + 1)*BOSSCOLS + x].traptile;

				wall = m_boss[(y + 1)*BOSSCOLS + x].wall;
				item = m_boss[(y + 1)*BOSSCOLS + x].item;
				enemy = m_boss[(y + 1)*BOSSCOLS + x].enemy;
				trap = m_boss[(y + 1)*BOSSCOLS + x].trap;

				if (!(wall || enemy)) {
					m_boss[y*BOSSCOLS + x].hero = false;
					m_boss[(y + 1)*BOSSCOLS + x].hero = true;
					player.at(0).setPosY(y + 1);

					if (trap) {
						if (traptile == SPIKE) {
							trapEncounter(*this, x, y + 1);

							m_boss[(y + 1)*BOSSCOLS + x].traptile = SPACE;
							m_boss[(y + 1)*BOSSCOLS + x].trap = false;

							if (player.at(0).getHP() <= 0) {
								player.at(0).setDeath("falling spike");
								//clearScreen();
								//showDungeon();
								if (!dungeonText.empty())
									showText();
								return;
							}
						}
						if (traptile == SPIKETRAP_ACTIVE) {
							trapEncounter(*this, x, y + 1);
						}
					}
					if (item) {
						foundItem(m_boss, BOSSCOLS, x, y + 1);
					}
				}
				else if (enemy) {
					fight(x, y + 1);
				}
			}

			else if (move == 'e') {
				collectItem(m_boss, BOSSROWS, BOSSCOLS, x, y);
			}

			else if (move == 'b') {
				if (player.at(0).hasShield()) {
					// play setting up shield stance sound effect
					cocos2d::experimental::AudioEngine::play2d("Shield_Stance.mp3", false, 1.0f);

					player.at(0).setBlock(true);
				}
			}

			else if (move == 'w') {
				//showDungeon();
				//player.at(0).wield(dungeonText);
			}

			else if (move == 'c') {
				//showDungeon();
				//player.at(0).use(m_firstbossActives, m_boss[y*BOSSCOLS + x], dungeonText);
			}
		}

	}

	// check active items in play
	checkActive(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters);
	if (player.at(0).getHP() <= 0) {
		//clearScreen();
		//showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}

	// if all monsters are dead?
	if (m_firstbossMonsters.empty()) {
		//clearScreen();
		//showDungeon();
		if (!dungeonText.empty())
			showText();

		return;
	}

	// move any monsters
	int mInitHP;
	if (m_firstbossMonsters.size() > 1) {
		for (unsigned i = 1; i < m_firstbossMonsters.size(); i++) {
			mInitHP = getMonsters().at(i)->getHP();

			if (m_firstbossMonsters.at(i)->getName() == "archer") {
				checkArchers(m_boss, BOSSROWS, BOSSCOLS, player.at(0).getPosX(), player.at(0).getPosY(), i, m_firstbossMonsters);
			}

			// flash monster sprite if damage was taken
			if (getMonsters().at(i)->getHP() < mInitHP) {
				runMonsterDamage(getMonsters().at(i)->getSprite());
			}

			//	if player is dead then break
			if (player.at(0).getHP() <= 0) {
				m_boss[y*BOSSCOLS + x].top = 'X'; //change player icon to X to indicate death
				player.at(0).setDeath(m_firstbossMonsters.at(i)->getName());

				return;
			}
		}
	}

	//	check if smasher is dead
	if (m_firstbossMonsters.at(0)->getName() != "Smasher") {
		// queue player move to the front of the spawn
		if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
			queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY(), .1, true);
		}

		// run actions
		runSpawn();
		
		//if (!dungeonText.empty())
			//showText();
		return;
	}

	//	check if smasher is currently executing a move
	checkSmasher();

	// queue player move to the front of the spawn
	if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
		queueMoveSprite(player.at(0).getSprite(), rows, player.at(0).getPosX(), player.at(0).getPosY(), .1, true);
	}

	// run actions
	runSpawn();

	// flash player sprite if took damage
	if (player.at(0).getHP() < initHP) {
		runPlayerDamage(player.at(0).getSprite());
	}

	//	if killed by smasher then return
	if (player.at(0).getHP() <= 0) {
		//m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = 'X'; //change player icon to X to indicate death
		player.at(0).setDeath(m_firstbossMonsters.at(0)->getName());
		return;
	}

	//if (!dungeonText.empty())
		//showText();
}

void FirstBoss::monsterDeath(int pos) {
	int x = m_firstbossMonsters.at(pos)->getPosX();
	int y = m_firstbossMonsters.at(pos)->getPosY();

	// pause current music, play victory music, and then resume music once finished
	cocos2d::experimental::AudioEngine::pauseAll();
	int id = cocos2d::experimental::AudioEngine::play2d("Victory! All Clear.mp3", false, 1.0f);

	cocos2d::experimental::AudioEngine::setFinishCallback(id,
		[](int id, std::string music) {
		cocos2d::experimental::AudioEngine::resumeAll();
	});
	
	removeSprite(monster_sprites, BOSSROWS, x, y);

	string boss = m_firstbossMonsters.at(pos)->getName();
	dungeonText.push_back(boss + " was slain!\n");
	dungeonText.push_back("Well done!\n");

	Idol idol;
	idol.setPosX(BOSSCOLS / 2);
	idol.setPosY(3);
	m_boss[idol.getPosY()*BOSSCOLS + idol.getPosX()].bottom = IDOL;
	m_boss[idol.getPosY()*BOSSCOLS + idol.getPosX()].item = true;

	// sprite for idol
	addSprite(item_sprites, BOSSROWS, idol.getPosX(), idol.getPosY(), -1, "Freeze_Spell_48x48.png");

	for (int i = y - 1; i < y + 2; i++) {
		for (int j = x - 1; j < x + 2; j++) {
			m_boss[i*BOSSCOLS + j].upper = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
		}
	}

	m_firstbossMonsters.erase(m_firstbossMonsters.begin() + pos);
}

void FirstBoss::checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters) {
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int x, y;
	int pos;

	if (!actives.empty()) {
		for (unsigned i = 0; i < actives.size(); i++) {
			if (actives.at(i)->getItem() == "Bomb") {
				std::shared_ptr<Bomb> bomb = std::dynamic_pointer_cast<Bomb>(actives.at(i));
				if (bomb->getFuse() > 0) {
					bomb->setFuse(bomb->getFuse() - 1);
					actives.at(i) = bomb;
				}
				else {
					// play explosion sound effect
					cocos2d::experimental::AudioEngine::play2d("Explosion.mp3", false, 1.0f);

					//dungeonText.push_back("A bomb exploded!\n");
					//explosion(dungeon, maxrows, maxcols, actives, monsters, bomb->getPosX(), bomb->getPosY(), i);
					explosion(*this, bomb->getPosX(), bomb->getPosY());
					dungeon[bomb->getPosY()*maxcols + bomb->getPosX()].extra = SPACE;

					// remove bomb sprite
					removeSprite(projectile_sprites, maxrows, bomb->getPosX(), bomb->getPosY());

					// set i to pos because otherwise the iterator is destroyed
					pos = i;
					actives.erase(actives.begin() + pos);
					i--;
				}
				bomb.reset();
			}
			else if (actives.at(i)->getItem() == "Mega Bomb") {
				std::shared_ptr<MegaBomb> megabomb = std::dynamic_pointer_cast<MegaBomb>(actives.at(i));
				if (megabomb->getFuse() > 0) {
					megabomb->setFuse(megabomb->getFuse() - 1);
					actives.at(i) = megabomb;
				}
				else {
					// play explosion sound effect
					cocos2d::experimental::AudioEngine::play2d("Mega_Explosion.mp3", false, 1.0);

					//dungeonText.push_back("BOOM!\n");
					//explosion(dungeon, maxrows, maxcols, actives, monsters, megabomb->getPosX(), megabomb->getPosY(), i);
					explosion(*this, megabomb->getPosX(), megabomb->getPosY());
					dungeon[megabomb->getPosY()*maxcols + megabomb->getPosX()].extra = SPACE;
					removeSprite(projectile_sprites, maxrows, megabomb->getPosX(), megabomb->getPosY());

					// set i to pos because otherwise the iterator is destroyed
					pos = i;
					actives.erase(actives.begin() + pos);
					i--;
				}
				megabomb.reset();
			}
			else if (actives.at(i)->getItem() == "falling spike") {
				std::shared_ptr<FallingSpike> spike = std::dynamic_pointer_cast<FallingSpike>(actives.at(i));
				x = spike->getPosX();
				y = spike->getPosY();

				switch (spike->getSpeed()) {
				case 1:
					//	if tile below spike is the player, hit them
					if (dungeon[(y + 1)*maxcols + x].hero) {
						cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);

						//dungeonText.push_back("A speed 1 spike smacked you on the head for " + to_string(spike->getDmg()) + " damage!\n");
						player.at(0).setHP(player.at(0).getHP() - spike->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							dungeon[py*maxcols + px].top = 'X';
						}
						i--;
					}
					//	if tile below spike is not a wall
					else if (y + 1 < maxrows - 1) {
						moveSprite(spike_sprites, maxrows, x, y, 'd');

						spike->setPosY(y + 1);
						//actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(y + 1)*maxcols + x].traptile == SPIKE) {
							dungeon[(y + 1)*maxcols + x].overlap = true;
							dungeon[(y + 1)*maxcols + x].overlap_count++;
						}

						dungeon[(y + 1)*maxcols + x].traptile = SPIKE;
						dungeon[(y + 1)*maxcols + x].trap = true;
					}
					//	else disappear
					else {
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);
						// subtract one from the increment so as to not skip any spikes
						i--;
					}
					break;
				case 2:
					//	if player is in the way, hit them
					if (dungeon[(y + 1)*maxcols + x].hero || dungeon[(std::min(y + 2, maxrows-1))*maxcols + x].hero) {
						cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);

						//dungeonText.push_back("A speed 2 spike smacked you on the head for " + to_string(spike->getDmg()) + "!\n");
						player.at(0).setHP(player.at(0).getHP() - spike->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							dungeon[py*maxcols + px].top = 'X';
						}
						i--;
					}
					//	else if two tiles below spike are not walls
					else if (y + 2 < maxrows - 1) {
						moveSprite(spike_sprites, maxrows, x, y, 'D');

						spike->setPosY(y + 2);
						actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(y + 2)*maxcols + x].traptile == SPIKE) {
							dungeon[(y + 2)*maxcols + x].overlap = true;
							dungeon[(y + 2)*maxcols + x].overlap_count++;
						}

						dungeon[(y + 2)*maxcols + x].traptile = SPIKE;
						dungeon[(y + 2)*maxcols + x].trap = true;
					}
					//	else if there is only one tile of space underneath, set spike to bottom
					else if (y + 2 >= maxrows - 1 && y != maxrows - 2) {
						moveSprite(spike_sprites, maxrows, x, y, 'd');

						spike->setPosY(maxrows - 2);
						//actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(y + 1)*maxcols + x].traptile == SPIKE) {
							dungeon[(y + 1)*maxcols + x].overlap = true;
							dungeon[(y + 1)*maxcols + x].overlap_count++;
						}

						dungeon[(maxrows - 2)*maxcols + x].traptile = SPIKE;
						dungeon[(maxrows - 2)*maxcols + x].trap = true;
					}
					//	else disappear
					else {
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);
						// subtract one from the increment so as to not skip any spikes
						i--;
					}
					break;
				case 3:
					//	if player is below, hit them
					if (dungeon[(std::min(y + 3, maxrows-1))*maxcols + x].hero || dungeon[(std::min(y + 2, maxrows-1))*maxcols + x].hero || dungeon[(y + 1)*maxcols + x].hero) {
						cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);

						//dungeonText.push_back("A speed 3 spike smacked you on the head for " + to_string(spike->getDmg()) + "!\n");
						player.at(0).setHP(player.at(0).getHP() - spike->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							dungeon[py*maxcols + px].top = 'X';
						}
						i--;
					}
					//	if spike does not hit a wall
					else if (y + 3 < maxrows - 1) {
						moveSprite(spike_sprites, maxrows, x, y, 'V');

						spike->setPosY(y + 3);
						//actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(y + 3)*maxcols + x].traptile == SPIKE) {
							dungeon[(y + 3)*maxcols + x].overlap = true;
							dungeon[(y + 3)*maxcols + x].overlap_count++;
						}

						dungeon[(y + 3)*maxcols + x].traptile = SPIKE;
						dungeon[(y + 3)*maxcols + x].trap = true;
					}
					//	if spike would overshoot, move it to the bottom
					else if (y + 3 >= maxrows - 1 && y != maxrows - 2) {
						// if spike is one above the bottom
						if (y == maxrows - 3) {
							moveSprite(spike_sprites, maxrows, x, y, 'd');
						}
						// else spike was two above the bottom
						else {
							moveSprite(spike_sprites, maxrows, x, y, 'D');
						}

						spike->setPosY(maxrows - 2);
						actives.at(i) = spike;

						// check if a spike already exists in this spot, if so make note of it
						if (dungeon[(maxrows - 2)*maxcols + x].traptile == SPIKE) {
							dungeon[(maxrows - 2)*maxcols + x].overlap = true;
							dungeon[(maxrows - 2)*maxcols + x].overlap_count++;
						}

						dungeon[(maxrows - 2)*maxcols + x].traptile = SPIKE;
						dungeon[(maxrows - 2)*maxcols + x].trap = true;
					}
					//	else disappear
					else {
						removeSprite(spike_sprites, maxrows, x, y);

						pos = i;
						actives.erase(actives.begin() + pos);
						// subtract one from the increment so as to not skip any spikes
						i--;
					}
					break;
				default:
					break;
				}

				// if there are more than two spikes overlapping, just reduce the overlap count
				if (dungeon[y*maxcols + x].overlap && dungeon[y*maxcols + x].overlap_count > 1) {
					dungeon[y*maxcols + x].overlap_count--;
				}
				// if there is only one overlap, set overlap to false, reduce count to zero,
				// but do not replace the previous tile with a space
				else if (dungeon[y*maxcols + x].overlap && dungeon[y*maxcols + x].overlap_count == 1) {
					dungeon[y*maxcols + x].overlap = false;
					dungeon[y*maxcols + x].overlap_count--;
				}
				// else if there is no overlap, then reset the tile to empty projectile space
				else {
					dungeon[y*maxcols + x].traptile = SPACE;
					dungeon[y*maxcols + x].trap = false;
				}

				//dungeon[y*maxcols + x].traptile = SPACE;
				//dungeon[y*maxcols + x].trap = false;
				spike.reset();
			}
			else if (actives.at(i)->getItem() == "spike trap") {
				std::shared_ptr<SpikeTrap> spiketrap = std::dynamic_pointer_cast<SpikeTrap>(actives.at(i));
				x = spiketrap->getPosX();
				y = spiketrap->getPosY();

				// retract spiketrap
				if (dungeon[y*maxcols + x].traptile == SPIKETRAP_ACTIVE) {
					removeSprite(trap_sprites, maxrows, x, y);
					addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Deactive_48x48.png");

					spiketrap->setCountdown(spiketrap->getSpeed());
					actives.at(i) = spiketrap;
					dungeon[y*maxcols + x].traptile = SPIKETRAP_DEACTIVE;
				}
				// spiketrap is primed
				if (spiketrap->getCountdown() == 1) {
					cocos2d::experimental::AudioEngine::play2d("Spiketrap_Primed.mp3", false, (float)exp(-(abs(px - x) + abs(py - y)) / 2));
					removeSprite(trap_sprites, maxrows, x, y);
					addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Primed_48x48.png");

					spiketrap->setCountdown(spiketrap->getCountdown() - 1);
					actives.at(i) = spiketrap;
					dungeon[y*maxcols + x].traptile = SPIKETRAP_PRIMED;
				}
				else if (spiketrap->getCountdown() > 1) {
					spiketrap->setCountdown(spiketrap->getCountdown() - 1);
					actives.at(i) = spiketrap;
				}
				// spiketrap is out
				else {
					cocos2d::experimental::AudioEngine::play2d("Spiketrap_Active.mp3", false, (float)exp(-(abs(px - x) + abs(py - y)) / 2));
					removeSprite(trap_sprites, maxrows, x, y);
					addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Active_48x48.png");

					dungeon[y*maxcols + x].traptile = SPIKETRAP_ACTIVE;

					if (px == x && py == y) {
						cocos2d::experimental::AudioEngine::play2d("Spike_Hit.mp3", false, 1.0f);

						dungeonText.push_back("A spike trap bludgeons you for " + to_string(spiketrap->getDmg()) + " damage!\n");
						player.at(0).setHP(player.at(0).getHP() - spiketrap->getDmg());
						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("spike trap");
							dungeon[py*maxcols + px].top = 'X';
						}
					}
				}
				spiketrap.reset();
			}
			else if (actives.at(i)->getItem() == "trigger spike trap") {
				std::shared_ptr<TriggerSpike> ts = std::dynamic_pointer_cast<TriggerSpike>(actives.at(i));
				x = ts->getPosX();
				y = ts->getPosY();

				// if the spike trap was triggered
				if (ts->isTriggered()) {
					dungeon[y*maxcols + x].traptile = TRIGGER_SPIKE_ACTIVE;

					//	check if player was still on top
					if (dungeon[y*maxcols + x].hero) {
						player.at(0).setHP(player.at(0).getHP() - ts->getDmg());
						dungeonText.push_back("You were bludgeoned by a spike for " + to_string(ts->getDmg()) + " damage!\n");

						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("trigger spike trap");
							dungeon[py*maxcols + px].top = 'X';
						}
					}
					ts->toggleTrigger();
					actives.at(i) = ts;
					ts.reset();
				}
				else {
					// else if the trap wasn't triggered, check if player is standing on top of it
					if (dungeon[y*maxcols + x].hero) {
						ts->toggleTrigger();
						actives.at(i) = ts;
						ts.reset();
						dungeonText.push_back("*tink*\n");
					}
					else
						dungeon[y*maxcols + x].traptile = TRIGGER_SPIKE_DEACTIVE;
				}
			}
			else if (actives.at(i)->getItem() == "door") {
				shared_ptr<Door> door = dynamic_pointer_cast<Door>(actives.at(i));
				x = door->getPosX();
				y = door->getPosY();

				// if door is open, isn't being held, and nothing is in the way
				if (door->isOpen() && !door->isHeld() && (dungeon[y*maxcols + x].top == SPACE || dungeon[y*maxcols + x].top == DOOR_H_OPEN || dungeon[y*maxcols + x].top == DOOR_V_OPEN)) {
					// close the door
					dungeon[y*maxcols + x].top = (dungeon[y*maxcols + x].traptile == DOOR_H ? DOOR_H : DOOR_V);
					door->toggleOpen();
					door->toggleHeld();
					actives.at(i).reset();
					actives.at(i) = door;
				}
				else if (door->isOpen() && door->isHeld()) {
					// release hold on the door
					door->toggleHeld();
					actives.at(i).reset();
					actives.at(i) = door;
				}
				door.reset();
			}
		}
	}
}

void FirstBoss::checkSmasher() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));

	//	check if smasher is currently executing a move
	if (smasher->isActive()) {
		switch (smasher->getMove()) {
		case 1: move1(); break;
		case 2: move2(); break;
		case 3: move3(); break;
		case 4: move4(); break;
		case 5: move5(); break;
		default: break;
		}
	}
	// if smasher has finished a move and hasn't come off cooldown,
	// then continue the cooldown process
	else if (!smasher->isEnded()) {
		switch (smasher->getMove()) {
		case 1: resetUpward(); break;
		case 2: {
			switch (move) {
			case 'u': resetDownward(); break;
			case 'l': resetRightward(); break;
			case 'r': resetLeftward(); break;
			}

			break;
		}
		case 3: break;
		case 4: break;
		case 5: break;
		default: break;
		}
	}
	// smasher is not executing a move, so randomly select a new one
	else {
		pickMove();
	}

	smasher.reset();
}
void FirstBoss::pickMove() { // select a move to use
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int randmove = 1 + randInt(100);


	// 12% chance : smash attack
	if (randmove > 0 && randmove <= 12) {
		smasher->setActive(true);
		smasher->setMove(1);
		m_firstbossMonsters.at(0) = smasher;
		move1();
	}
	// 15% chance : avalanche attack
	else if (randmove > 12 && randmove <= 27) {
		smasher->setActive(true);
		smasher->setMove(2);
		m_firstbossMonsters.at(0) = smasher;
		move2();
	}
	// 3% chance : archer/enemy spawning, max 3 at once
	else if (randmove > 27 && randmove <= 30) {
		if (m_firstbossMonsters.size() < 3)
			;//move3();
		else
			moveSmasher();
	}
	else if (randmove == 0) {
		move4();
	}
	else if (randmove == 0) {
		move5();
	}
	// 70% chance : random movement
	else if (randmove > 30) {
		moveSmasher();
	}

	smasher.reset();
}
void FirstBoss::move1() { // smashes!
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();

	//	if wall immediately beneath smasher is an unbreakable wall, reset position to top
	if (m_boss[(row+2)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		smasher->setEnded(false);
		smasher->setActive(false);
		m_firstbossMonsters.at(0) = smasher;
		resetUpward();

		smasher.reset();
		return;
	}

	moveSmasherD();
	smasher.reset();
}
void FirstBoss::move2() { // starts avalanche
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	int n = randInt(3);

	// if this is the beginning of the move, then select a direction to move in
	if (move == '0') {
		switch (n) {
		case 0: move = 'u'; break;
		case 1: move = 'l'; break;
		case 2: move = 'r'; break;
		}
	}

	//	move is upward
	if (move == 'u') {
		//	if wall immediately above smasher is an unbreakable wall, make rocks fall
		if (m_boss[(row - 2)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
			cocos2d::experimental::AudioEngine::play2d("Rock_Slide.mp3", false, 1.0f);

			smasher->setEnded(false);
			smasher->setActive(false);
			m_firstbossMonsters.at(0) = smasher;
			avalanche();
			resetDownward();
			smasher.reset();

			return;
		}

		moveSmasherU();
	}
	//	move is to the left
	else if (move == 'l') {
		//	if wall immediately to the left of smasher is an unbreakable wall, make rocks fall
		if (m_boss[(row)*BOSSCOLS + col - 2].top == UNBREAKABLE_WALL) {
			cocos2d::experimental::AudioEngine::play2d("Rock_Slide.mp3", false, 1.0f);

			smasher->setEnded(false);
			smasher->setActive(false);
			m_firstbossMonsters.at(0) = smasher;
			avalanche();
			resetRightward();
			smasher.reset();

			return;
		}

		moveSmasherL();
	}
	//	move is to the right
	else if (move == 'r') {
		//	if wall immediately to the right of smasher is an unbreakable wall, make rocks fall
		if (m_boss[(row)*BOSSCOLS + col + 2].top == UNBREAKABLE_WALL) {
			cocos2d::experimental::AudioEngine::play2d("Rock_Slide.mp3", false, 1.0f);

			smasher->setEnded(false);
			smasher->setActive(false);
			m_firstbossMonsters.at(0) = smasher;
			avalanche();
			resetLeftward();
			smasher.reset();

			return;
		}

		moveSmasherR();
	}

	smasher.reset();
}
void FirstBoss::avalanche() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int x, y, speed, spikecount;
	char traptile;
	spikecount = 15 + randInt(8);
	
	for (int i = 0; i < spikecount; i++) {
		x = 1 + randInt(BOSSCOLS - 2);
		y = 1 + randInt(BOSSROWS / 3);
		speed = 1 + randInt(3);
		traptile = m_boss[y*BOSSCOLS + x].traptile;
		FallingSpike spike(x, y, speed);

		while (traptile != SPACE || x == col || x == col-1 || x == col+1) { // while spike position clashes with anything
			x = 1 + randInt(BOSSCOLS - 2);
			y = 1 + randInt(BOSSROWS / 3);
			spike.setPosX(x);	// reroll it
			spike.setPosY(y);
			
			traptile = m_boss[spike.getPosY()*BOSSCOLS + spike.getPosX()].traptile;
		}
		// add spike sprite
		addSprite(spike_sprites, BOSSROWS, spike.getPosX(), spike.getPosY(), 0, "CeilingSpike_48x48.png");

		m_firstbossActives.emplace_back(new FallingSpike(spike));
		m_boss[spike.getPosY()*BOSSCOLS + spike.getPosX()].trap = true;
		m_boss[spike.getPosY()*BOSSCOLS + spike.getPosX()].traptile = SPIKE;
	}
	smasher.reset();
}
void FirstBoss::move3() {	// spawn two archers
	int n = 2;
	int x, y;
	char top, hero;

	if (m_firstbossMonsters.size() <= 2) {
		while (n > 0) {
			Archer archer;
			x = 1 + randInt(BOSSCOLS - 2);
			y = BOSSROWS - 2 - randInt(10);
			archer.setPosX(x);
			archer.setPosY(y);

			top = m_boss[y*BOSSCOLS + x].top;
			hero = m_boss[y*BOSSCOLS + x].hero;
			while (top != SPACE || hero) {
				x = 1 + randInt(BOSSCOLS - 2);
				y = BOSSROWS - 2 - randInt(10);
				archer.setPosX(x);
				archer.setPosY(y);

				top = m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].top;
				hero = m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].hero;
			}
			//addSprite(monster_sprites, BOSSROWS, archer.getPosX(), archer.getPosY(), 1, "Archer_48x48.png");
			archer.setSprite(createSprite(monster_sprites, BOSSROWS, archer.getPosX(), archer.getPosY(), 1, "Archer_48x48.png"));

			m_firstbossMonsters.emplace_back(new Archer(archer));
			m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].top = ARCHER;
			m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].enemy = true;
			n--;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Spawns.mp3", false, 1.0f);
	}
}
void FirstBoss::move4() { // earthquake

}
void FirstBoss::move5() { // projectile bouncers

}

void FirstBoss::moveSmasher() { // pick a random direction to move in
	int n = randInt(4);

	switch(n) {
	case 0: moveSmasherL(); break;
	case 1: moveSmasherR(); break;
	case 2: moveSmasherU(); break;
	case 3: moveSmasherD(); break;
	default: break;
	}
}
void FirstBoss::moveSmasherL() {
	//shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	//	if wall immediately to left smasher is an unbreakable wall, do nothing
	if (m_boss[row*BOSSCOLS + col-2].top == UNBREAKABLE_WALL) {
		//smasher.reset();
		return;
	}

	//  if there is only one space to the left of the smasher
	else if (m_boss[row*BOSSCOLS + col-3].top == UNBREAKABLE_WALL) {
		// right side of smasher is set to spaces
		for (i = col + 1, j = row - 1; j < row + 2; j++) {
			m_boss[j*BOSSCOLS + i].upper = SPACE;
			m_boss[j*BOSSCOLS + i].enemy = false;
		}

		//	smasher is pushed to left wall
		for (i = col - 2, j = row - 1; j < row + 2; j++) {
			//	if player is in the way, crush them
			if (m_boss[j*BOSSCOLS + i].hero) {
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
			// if there are any monsters in the way, crush them
			if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {
				int pos = findMonster(i, j, m_firstbossMonsters);
				m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));

				// check if monster died
				if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
					monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
				}
			}

			m_boss[j*BOSSCOLS + i].upper = SMASHER;
			m_boss[j*BOSSCOLS + i].enemy = true;
		}

		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'l');
		queueMoveSprite(getMonsters().at(0), 'l');
		m_firstbossMonsters.at(0)->setPosX(col - 1);
	}

	//	if there are only two spaces to the left of the smasher
	else if (m_boss[row*BOSSCOLS + col-4].top == UNBREAKABLE_WALL) {
		// right 2 layers of smasher is set to spaces
		for (i = col + 1; i > col - 1; i--) {
			for (j = row - 1; j < row + 2; j++) {
				m_boss[j*BOSSCOLS + i].upper = SPACE;
				m_boss[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	smasher is pushed to left wall
		for (i = col - 2; i > col - 4; i--) {
			for (j = row - 1; j < row + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[j*BOSSCOLS + i].hero) {
					player.at(0).setPosX(col - 3);
					m_boss[j*BOSSCOLS + i].hero = false;
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
					//queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .03);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them
				if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {					
					int pos = findMonster(i, j, m_firstbossMonsters);
					m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
					
					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != col - 3 && m_boss[j*BOSSCOLS + (i - 1)].enemy) {
						int n = findMonster(i - 1, j, m_firstbossMonsters);
						
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, n);
					}

					// move the monster and sprite
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', m_firstbossMonsters, col - 3, 0);

					// check if monster died
					if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
					}
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;

			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'L');
		queueMoveSprite(getMonsters().at(0), 'L');
		m_firstbossMonsters.at(0)->setPosX(col - 2);
	}

	//	if there are at least 3 spaces to left of the smasher
	else if (m_boss[row*BOSSCOLS + col-4].top != UNBREAKABLE_WALL) {
		// right 2 layers of smasher is set to spaces
		for (i = col + 1; i > col - 1; i--) {
			for (j = row - 1; j < row + 2; j++) {
				//enemyOverlap(m_boss, BOSSROWS, BOSSCOLS, i, j);
				m_boss[j*BOSSCOLS + i].upper = SPACE;				
				m_boss[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	left two layers set to smasher
		for (i = col - 2; i > col - 4; i--) {
			for (j = row - 1; j < row + 2; j++) {
				// if player is in the way, move them to the side
				if (m_boss[j*BOSSCOLS + i].hero) {
					// if there's another enemy, push them
					if (m_boss[j*BOSSCOLS + i - 1].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i - 1, j, 'l', m_firstbossMonsters);
					}
					if (m_boss[j*BOSSCOLS + i - 2].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i - 2, j, 'l', m_firstbossMonsters);
					}

					player.at(0).setPosX(col - 4);
					m_boss[j*BOSSCOLS + i].hero = false;
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
					
					//queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .03);
				}
				
				// if there are any monsters in the way, move them
				if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {
					// if player is in the way, push them
					if (m_boss[j*BOSSCOLS + i - 1].hero || m_boss[j*BOSSCOLS + i - 2].hero) {
						pushPlayer(m_boss, BOSSROWS, BOSSCOLS, 'l');
					}
					// if there's another enemy, push them
					if (m_boss[j*BOSSCOLS + i - 1].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i - 1, j, 'l', m_firstbossMonsters);
					}
					if (i != col - 3 && m_boss[j*BOSSCOLS + i - 2].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i - 2, j, 'l', m_firstbossMonsters);
					}

					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', m_firstbossMonsters, col - 4, 0);
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;
			}
		}
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'L');
		queueMoveSprite(getMonsters().at(0), 'L');
		m_firstbossMonsters.at(0)->setPosX(col - 2);
	}

	//smasher.reset();
}
void FirstBoss::moveSmasherR() {
	//shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	//	if wall immediately to right of smasher is an unbreakable wall, do nothing
	if (m_boss[row*BOSSCOLS + col+2].top == UNBREAKABLE_WALL) {
		//smasher.reset();
		return;
	}

	//  if there is only one space to the right of the smasher
	else if (m_boss[row*BOSSCOLS + col+3].top == UNBREAKABLE_WALL) {
		// left side of smasher is set to spaces
		for (i = col - 1, j = row - 1; j < row + 2; j++) {
			m_boss[j*BOSSCOLS + i].upper = SPACE;
			m_boss[j*BOSSCOLS + i].enemy = false;
		}

		//	smasher is pushed to right wall
		for (i = col + 2, j = row - 1; j < row + 2; j++) {
			//	if player is in the way, crush them
			if (m_boss[j*BOSSCOLS + i].hero) {
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
			// if there are any monsters in the way, crush them
			if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {
				int pos = findMonster(i, j, m_firstbossMonsters);
				m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));

				// check if monster died
				if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
					monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
				}
			}

			m_boss[j*BOSSCOLS + i].upper = SMASHER;
			m_boss[j*BOSSCOLS + i].enemy = true;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'r');
		queueMoveSprite(getMonsters().at(0), 'r');
		m_firstbossMonsters.at(0)->setPosX(col + 1);
	}

	//	if there are only two spaces to the right of the smasher
	else if (m_boss[row*BOSSCOLS + col + 4].top == UNBREAKABLE_WALL) {
		// left two layers of smasher is set to spaces
		for (i = col - 1; i < col + 1; i++) {
			for (j = row - 1; j < row + 2; j++) {
				m_boss[j*BOSSCOLS + i].upper = SPACE;
				m_boss[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	smasher is pushed to right wall
		for (i = col + 2; i < col + 4; i++) {
			for (j = row - 1; j < row + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[j*BOSSCOLS + i].hero) {
					player.at(0).setPosX(col + 3);
					m_boss[j*BOSSCOLS + i].hero = false;
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
					//queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .03);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them
				if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {
					// crush the monster
					int pos = findMonster(i, j, m_firstbossMonsters);
					m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
					
					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != col + 3 && m_boss[j*BOSSCOLS + (i + 1)].enemy) {
						int n = findMonster(i + 1, j, m_firstbossMonsters);
						
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, n);
					}

					// move the sprite
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', m_firstbossMonsters, col + 3, 0);

					// check if monster died (should be dead)
					if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
					}
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;

			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'R');
		queueMoveSprite(getMonsters().at(0), 'R');
		m_firstbossMonsters.at(0)->setPosX(col + 2);
	}

	//	if there are at least 3 spaces to right of the smasher
	else if (m_boss[row*BOSSCOLS + col + 4].top != UNBREAKABLE_WALL) {
		// left two layers of smasher is set to spaces
		for (i = col - 1; i < col + 1; i++) {
			for (j = row - 1; j < row + 2; j++) {
				m_boss[j*BOSSCOLS + i].upper = SPACE;
				m_boss[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	right two layers set to smasher
		for (i = col + 2; i < col + 4; i++) {
			for (j = row - 1; j < row + 2; j++) {
				// if player is in the way, move them to the side
				if (m_boss[j*BOSSCOLS + i].hero) {
					// if there's another enemy, push them
					if (m_boss[j*BOSSCOLS + i + 1].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i + 1, j, 'r', m_firstbossMonsters);
					}
					if (m_boss[j*BOSSCOLS + i + 2].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i + 2, j, 'r', m_firstbossMonsters);
					}

					player.at(0).setPosX(col + 4);
					m_boss[j*BOSSCOLS + i].hero = false;
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
					//queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .03);
				}
				
				// if there are any monsters in the way, move them
				if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {
					// if player is in the way, push them
					if (m_boss[j*BOSSCOLS + i + 1].hero || m_boss[j*BOSSCOLS + i + 2].hero) {
						pushPlayer(m_boss, BOSSROWS, BOSSCOLS, 'r');
					}
					// if there's another enemy, push them
					if (m_boss[j*BOSSCOLS + i + 1].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i + 1, j, 'r', m_firstbossMonsters);
					}
					if (i != col + 3 && m_boss[j*BOSSCOLS + i + 2].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i + 2, j, 'r', m_firstbossMonsters);
					}
					
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', m_firstbossMonsters, col + 4, 0);
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;
			}
		}
		
		queueMoveSprite(getMonsters().at(0), 'R');
		m_firstbossMonsters.at(0)->setPosX(col + 2);
	}

	//smasher.reset();
}
void FirstBoss::moveSmasherU() {
	//shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	//	if wall immediately above smasher is an unbreakable wall, do nothing
	if (m_boss[(row - 2)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		//smasher.reset();
		return;
	}

	//  if there is only one space above the smasher
	else if (m_boss[(row - 3)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// bottom layer of smasher is set to spaces
		for (i = row + 1, j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].upper = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
		}

		//	smasher is pushed to upper layer
		for (i = row - 2, j = col - 1; j < col + 2; j++) {
			//	if player is in the way, crush them
			if (m_boss[i*BOSSCOLS + j].hero) {
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
			// if there are any monsters in the way, crush them
			if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
				// find and crush the monster
				int pos = findMonster(j, i, m_firstbossMonsters);
				m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));

				// check if monster died
				if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
					monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
				}
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'u');
		queueMoveSprite(getMonsters().at(0), 'u');
		m_firstbossMonsters.at(0)->setPosY(row - 1);
	}

	//	if there are only two spaces above the smasher
	else if (m_boss[(row - 4)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// bottom 2 layers of smasher is set to spaces
		for (i = row + 1; i > row - 1; i--) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].upper = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	smasher is pushed to ceiling
		for (i = row - 2; i > row - 4; i--) {
			for (j = col - 1; j < col + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[i*BOSSCOLS + j].hero) {
					player.at(0).setPosY(row - 3);
					m_boss[i*BOSSCOLS + j].hero = false;
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
					//queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .03);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them
				if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
					// find and crush the monster
					int pos = findMonster(j, i, m_firstbossMonsters);
					m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));

					// if there are two enemies stacked, then kill the enemy closest to the wall first
					// :::: This is for the purpose of looking more natural on screen.
					if (i != 1 && m_boss[(i - 1)*BOSSCOLS + j].enemy) {
						int n = findMonster(j, i - 1, m_firstbossMonsters);
						
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, n);
					}

					// move the monster
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', m_firstbossMonsters, 0, row - 3);

					// check if monster died
					if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
					}
					
				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;

			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'U');
		queueMoveSprite(getMonsters().at(0), 'U');
		m_firstbossMonsters.at(0)->setPosY(row - 2);
	}

	//	if there are at least 3 spaces above the smasher
	else if (m_boss[(row - 4)*BOSSCOLS + col].top != UNBREAKABLE_WALL) {
		// bottom 2 layers of smasher is set to spaces
		for (i = row + 1; i > row - 1; i--) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].upper = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	top two layers above smasher set to smasher
		for (i = row - 2; i > row - 4; i--) {
			for (j = col - 1; j < col + 2; j++) {
				// if player is in the way, move them above
				if (m_boss[i*BOSSCOLS + j].hero) {
					// if there's another enemy, push them
					if (m_boss[(i-1)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i-1, 'u', m_firstbossMonsters);
					}
					if (m_boss[(i - 2)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 2, 'u', m_firstbossMonsters);
					}

					player.at(0).setPosY(row - 4);
					m_boss[i*BOSSCOLS + j].hero = false;
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
					//queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .03);
				}
				
				// if there are any monsters in the way, move them
				if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {					
					// if player is in the way, push them
					if (m_boss[(i - 1)*BOSSCOLS + j].hero || m_boss[(i - 2)*BOSSCOLS + j].hero) {
						pushPlayer(m_boss, BOSSROWS, BOSSCOLS, 'u');
					}
					// if there's another enemy, push them
					if (m_boss[(i - 1)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 1, 'u', m_firstbossMonsters);
					}
					if (i != 1 && m_boss[(i - 2)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 2, 'u', m_firstbossMonsters);
					}
					
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', m_firstbossMonsters, 0, row - 4);
				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		queueMoveSprite(getMonsters().at(0), 'U');
		m_firstbossMonsters.at(0)->setPosY(row - 2);
	}
	
	//smasher.reset();
}
void FirstBoss::moveSmasherD() {
	//shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	//	if wall immediately beneath smasher is an unbreakable wall, do nothing
	if (m_boss[(row + 2)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		//smasher.reset();
		return;
	}

	// if there is only one space underneath the smasher
	else if (m_boss[(row + 3)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// top layer of smasher is set to spaces
		for (i = row - 1, j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].upper = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
		}

		//	smasher is pushed to bottom layer
		for (i = row + 2, j = col - 1; j < col + 2; j++) {
			//	if player is in the way, crush them
			if (m_boss[i*BOSSCOLS + j].hero) {
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
			
			// if there are any monsters in the way, crush them
			if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {

				int pos = findMonster(j, i, m_firstbossMonsters);
				m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
					
				// check if monster died
				if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
					monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
				}
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		
		queueMoveSprite(getMonsters().at(0), 'd');
		m_firstbossMonsters.at(0)->setPosY(row + 1);
	}

	//	if there are only two spaces underneath the smasher
	else if (m_boss[(row + 4)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// top 2 layers of smasher is set to spaces
		for (i = row - 1; i < row + 1; i++) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].upper = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	smasher is pushed to bottom layer
		for (i = row + 2; i < row + 4; i++) {
			for (j = col - 1; j < col + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[i*BOSSCOLS + j].hero) {
					player.at(0).setPosY(BOSSROWS - 2);
					m_boss[i*BOSSCOLS + j].hero = false;
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
					//queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .03);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them dead
				if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
					// smasher attacking a monster is a formality at this point
					int pos = findMonster(j, i, m_firstbossMonsters);
					m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
					
					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != row + 3 && m_boss[(i + 1)*BOSSCOLS + j].enemy) {
						int n = findMonster(j, i + 1, m_firstbossMonsters);
						
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, n);
					}

					// NOTE: this must be after the attack, otherwise pos is then incorrect
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', m_firstbossMonsters, 0, BOSSROWS - 2);

					// check if monster died (monster will be dead)
					if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
					}

				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		
		queueMoveSprite(getMonsters().at(0), 'D');
		m_firstbossMonsters.at(0)->setPosY(row + 2);
	}

	//	if there are at least 3 spaces underneath the smasher
	else if (m_boss[(row + 4)*BOSSCOLS + col].top != UNBREAKABLE_WALL) {
		// top 2 layers of smasher is set to spaces
		for (i = row - 1; i < row + 1; i++) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].upper = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	bottom two beneath smasher set to smasher
		for (i = row + 2; i < row + 4; i++) {
			for (j = col - 1; j < col + 2; j++) {
				// if player is in the way, move them below
				if (m_boss[i*BOSSCOLS + j].hero) {
					// if there's another enemy, push them
					if (m_boss[(i + 1)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 1, 'd', m_firstbossMonsters);
					}
					if (m_boss[(i + 2)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 2, 'd', m_firstbossMonsters);
					}

					player.at(0).setPosY(row + 4);
					m_boss[i*BOSSCOLS + j].hero = false;
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
					//queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .05);
				}

				// if there are any monsters in the way, move them
				if (m_boss[i*BOSSCOLS + j].enemy){// && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
					// if player is in the way, push them
					if (m_boss[(i + 1)*BOSSCOLS + j].hero || m_boss[(i + 2)*BOSSCOLS + j].hero) {
						pushPlayer(m_boss, BOSSROWS, BOSSCOLS, 'd');
					}
					
					// if there's another enemy, push them
					if (m_boss[(i + 1)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 1, 'd', m_firstbossMonsters);
					}
					if (i != row + 3 && m_boss[(i + 2)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 2, 'd', m_firstbossMonsters);
					}
					
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', m_firstbossMonsters, 0, row + 4);
				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		
		queueMoveSprite(getMonsters().at(0), 'D');
		m_firstbossMonsters.at(0)->setPosY(row + 2);
	}
	
	//smasher.reset();
}

void FirstBoss::resetLeftward() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;
	int rand = randInt(3);

	//	if smasher has reset far enough to the left, end cooldown
	if (col < BOSSCOLS / 1.5 - rand) {
		smasher->setEnded(true);
		m_firstbossMonsters.at(0) = smasher;
		smasher.reset();
		move = '0';

		return;
	}

	moveSmasherL();
	smasher.reset();
}
void FirstBoss::resetRightward() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;
	int rand = randInt(3);

	//	if smasher has reset far enough to the right, end cooldown
	if (col > BOSSCOLS / 3 + rand) {
		smasher->setEnded(true);
		m_firstbossMonsters.at(0) = smasher;
		smasher.reset();
		move = '0';

		return;
	}

	moveSmasherR();
	smasher.reset();
}
void FirstBoss::resetUpward() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;
	int rand = randInt(3);

	if (row < 6 + rand) {
		smasher->setEnded(true);
		m_firstbossMonsters.at(0) = smasher;
		smasher.reset();

		return;
	}

	//	bottom 3 layers set to space
	for (i = row + 1; i > row - 2; i--) {
		for (j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].upper = SPACE;			
			m_boss[i*BOSSCOLS + j].enemy = false;
		}
	}
	
	//  three layers above smasher set to smasher
	for (i = row - 2; i > row - 5; i--) {
		for (j = col - 1; j < col + 2; j++) {
			// if player is in the way, move them above
			if (m_boss[i*BOSSCOLS + j].hero) {
				// if there's another enemy, push them
				if (m_boss[(i - 1)*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 1, 'u', m_firstbossMonsters);
				}
				if (m_boss[(std::max(i - 2, 1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 2, 'u', m_firstbossMonsters);
				}
				if (m_boss[(std::max(i - 3, 1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 3, 'u', m_firstbossMonsters);
				}
				
				player.at(0).setPosY(row - 5);
				m_boss[i*BOSSCOLS + j].hero = false;
				m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
				//queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .05);
			}
			// if there are any monsters in the way, move them
			if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
				// if player is in the way, push them
				if (m_boss[(i - 1)*BOSSCOLS + j].hero || m_boss[(i - 2)*BOSSCOLS + j].hero) {
					pushPlayer(m_boss, BOSSROWS, BOSSCOLS, 'd');
				}
				// if there's another enemy, push them
				if (m_boss[(i - 1)*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 1, 'u', m_firstbossMonsters);
				}
				if (m_boss[(std::max(i - 2, 1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 2, 'u', m_firstbossMonsters);
				}
				if (m_boss[(std::max(i - 3, 1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 3, 'u', m_firstbossMonsters);
				}
				
				pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', m_firstbossMonsters, 0, row - 5);				
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
	}

	//moveSprite(monster_sprites, BOSSROWS, col, row, '^');
	queueMoveSprite(getMonsters().at(0), '^');
	m_firstbossMonsters.at(0)->setPosY(row - 3);
	smasher.reset();
}
void FirstBoss::resetDownward() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;
	int rand = randInt(3);

	if (row > 6 + rand) {
		smasher->setEnded(true);
		m_firstbossMonsters.at(0) = smasher;
		smasher.reset();
		move = '0';

		return;
	}

	//	top 2 layers set to space
	for (i = row - 1; i < row + 1; i++) {
		for (j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].upper = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
		}
	}

	moveSmasherD();
	smasher.reset();

	/*
	//  two layers below smasher set to smasher
	for (i = row + 2; i < row + 4; i++) {
		for (j = col - 1; j < col + 2; j++) {
			// if player is in the way, move them above
			if (m_boss[i*BOSSCOLS + j].hero) {
				// if there's another enemy, push them
				if (m_boss[(i + 1)*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 1, 'd', m_firstbossMonsters);
				}
				if (m_boss[(std::min(i + 2, BOSSROWS-1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 2, 'd', m_firstbossMonsters);
				}
				
				player.at(0).setPosY(row + 4);
				m_boss[i*BOSSCOLS + j].hero = false;
				m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].hero = true;
				queueMoveSprite(player.at(0).getSprite(), getRows(), player.at(0).getPosX(), player.at(0).getPosY(), .03);
			}
			// if there are any monsters in the way, move them
			if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
				// if player is in the way, push them
				if (m_boss[(i + 1)*BOSSCOLS + j].hero || m_boss[(i + 2)*BOSSCOLS + j].hero) {
					pushPlayer(m_boss, BOSSROWS, BOSSCOLS, 'd');
				}
				// if there's another enemy, push them
				if (m_boss[(i + 1)*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 1, 'd', m_firstbossMonsters);
				}
				if (m_boss[(std::min(i + 2, BOSSROWS - 1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 2, 'd', m_firstbossMonsters);
				}
				
				pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', m_firstbossMonsters, 0, row + 4);
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
	}
	
	queueMoveSprite(getMonsters().at(0), 'D');
	m_firstbossMonsters.at(0)->setPosY(row + 2);
	smasher.reset();
	*/
}

bool FirstBoss::monsterHash(int &x, int &y, bool &switched, char move) {
	// let starting x and y be the monster's initial position
	
	bool enemy = m_boss[y*BOSSCOLS + x].enemy;
	bool wall = m_boss[y*BOSSCOLS + x].wall;
	bool hero = m_boss[y*BOSSCOLS + x].hero;

	// if current tile is valid, return to set monster here
	if (!(wall || enemy || hero)) {
		return true;
	}

	switch (move) {
		// 1. if next tile is a wall then switch direction (one time)

		// 2. else if a switch has already been made and a wall is hit, return false

		// 3. otherwise keep looking in the same path

	case 'l':
		if (m_boss[y*BOSSCOLS + x - 1].wall && !switched) {
			move = (randInt(2) == 0 ? 'u' : 'd');
			switched = true;
		}
		else if (m_boss[y*BOSSCOLS + x - 1].wall && switched) {
			return false;
		}
		break;
	case 'r':
		if (m_boss[y*BOSSCOLS + x + 1].wall && !switched) {
			move = (randInt(2) == 0 ? 'u' : 'd');
			switched = true;
		}
		else if (m_boss[y*BOSSCOLS + x + 1].wall && switched) {
			return false;
		}
		break;
	case 'u':
		if (m_boss[(y - 1)*BOSSCOLS + x].wall && !switched) {
			move = (randInt(2) == 0 ? 'l' : 'r');
			switched = true;
		}
		else if (m_boss[(y - 1)*BOSSCOLS + x].wall && switched) {
			return false;
		}
		break;
	case 'd':
		if (m_boss[(y + 1)*BOSSCOLS + x].wall && !switched) {
			move = (randInt(2) == 0 ? 'l' : 'r');
			switched = true;
		}
		else if (m_boss[(y + 1)*BOSSCOLS + x].wall && switched) {
			return false;
		}
		break;
	}
	
	if (move == 'l') return monsterHash(--x, y, switched, move);
	if (move == 'r') return monsterHash(++x, y, switched, move);
	if (move == 'u') return monsterHash(x, --y, switched, move);
	if (move == 'd') return monsterHash(x, ++y, switched, move);

	return false;
}
void FirstBoss::pushMonster(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, char move, std::vector<std::shared_ptr<Monster>> &monsters, int cx, int cy) {
	bool b = false;
	int ox = mx;
	int oy = my;
	for (int i = 0; i < monsters.size(); i++) {
		if (monsters.at(i)->getPosX() == mx && monsters.at(i)->getPosY() == my) {
			switch (move) {
			case 'l': 
			case 'r':
			case 'u':
			case 'd': {
				if (monsterHash(mx, my, b, move)) {
					dungeon[oy*maxcols + ox].top = SPACE;
					dungeon[oy*maxcols + ox].enemy = false;
					dungeon[my*maxcols + mx].top = ARCHER;
					dungeon[my*maxcols + mx].enemy = true;
					monsters[i]->setPosX(mx);
					monsters[i]->setPosY(my);


					queueMoveSprite(monsters.at(i), getRows(), mx, my);
				}
				else {
					monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, i);
				}
				break;
			}
				//	CUSTOM MOVEMENT CASE in X direction
			case 'X': {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;

				dungeon[my*maxcols + cx].top = ARCHER;
				dungeon[my*maxcols + cx].enemy = true;
				
				monsters[i]->setPosX(cx);
				queueMoveSprite(monsters.at(i), getRows(), cx, my);
				break;
			}
				//	CUSTOM MOVEMENT CASE in Y direction
			case 'Y': {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;

				dungeon[(cy)*maxcols + mx].top = ARCHER;
				dungeon[(cy)*maxcols + mx].enemy = true;
				
				monsters[i]->setPosY(cy);
				queueMoveSprite(monsters.at(i), getRows(), mx, cy);
				break;
			}
			}
			return;
		}
	}
}
void FirstBoss::pushPlayer(std::vector<_Tile> &dungeon, int maxrows, int maxcols, char move) {
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	bool b = false;
	int ox = px;
	int oy = py;

	//dungeon[py*maxcols + px].hero = false;
	if (monsterHash(px, py, b, move)) {
		dungeon[oy*maxcols + ox].hero = false;
		dungeon[py*maxcols + px].hero = true;
		player.at(0).setPosX(px);
		player.at(0).setPosY(py);

		// queue player move
		queueMoveSprite(player.at(0).getSprite(), maxrows, player.at(0).getPosX(), player.at(0).getPosY());
	}
	else {
		m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
	}

	/*
	switch (move) {
	case 'l': {
		dungeon[py*maxcols + px].hero = false;
		if (monsterHash(px, py, b, 'l')) {
			dungeon[py*maxcols + px].hero = true;
			player.at(0).setPosX(px);
			player.at(0).setPosY(py);
		}
		else {
			m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
		}
		break;
	}
	case 'r': {
		dungeon[py*maxcols + px].hero = false;
		if (monsterHash(px, py, b, 'r')) {
			dungeon[py*maxcols + px].hero = true;
			player.at(0).setPosX(px);
			player.at(0).setPosY(py);
		}
		else {
			m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
		}
		break;
	}
	case 'u': {
		dungeon[py*maxcols + px].hero = false;
		if (monsterHash(px, py, b, 'u')) {
			dungeon[py*maxcols + px].hero = true;
			player.at(0).setPosX(px);
			player.at(0).setPosY(py);
		}
		else {
			m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
		}
		break;
	}
	case 'd': {
		dungeon[py*maxcols + px].hero = false;
		if (monsterHash(px, py, b, 'd')) {
			dungeon[py*maxcols + px].hero = true;
			player.at(0).setPosX(px);
			player.at(0).setPosY(py);
		}
		else {
			m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
		}
		break;
	}
	}
	*/
}

void FirstBoss::fight(int x, int y) { // monster's coordinates
	
	// Check if monster encountered was smasher
	if (m_boss[y*BOSSCOLS + x].upper == SMASHER) {
		//player.at(0).attack(m_firstbossMonsters, m_firstbossActives, 0, dungeonText);
		player.at(0).attack(*this, *getMonsters().at(0));

		if (m_firstbossMonsters.at(0)->getHP() <= 0)
			monsterDeath(0);

		return;
	}

	int i = findMonster(x, y, m_firstbossMonsters);

	// if monster wasn't found, return
	if (i == -1)
		return;
	
	//player.at(0).attack(m_firstbossMonsters, m_firstbossActives, i, dungeonText);
	player.at(0).attack(*this, *getMonsters().at(i));

	if (m_firstbossMonsters.at(i)->getHP() <= 0) {
		monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, i);
	}
}

/*
void FirstBoss::showDungeon() {
	cout << "\n\n\n";
	cout << "                           ";
	for (int i = 0; i < BOSSROWS; i++) {
		for (int j = 0; j < BOSSCOLS; j++) {
			if (m_boss[i*BOSSCOLS + j].top == SPACE && m_boss[i*BOSSCOLS + j].traptile != SPACE)
				cout << m_boss[i*BOSSCOLS + j].traptile;
			else if (m_boss[i*BOSSCOLS + j].top == SPACE && m_boss[i*BOSSCOLS + j].traptile == SPACE)
				cout << m_boss[i*BOSSCOLS + j].bottom;
			else
				cout << m_boss[i*BOSSCOLS + j].top;
		}
		cout << endl;
		cout << "                           ";
	}
	cout << endl;

	cout << "      ";
	cout << "HP: " << player.at(0).getHP() << ", Armor: " << player.at(0).getArmor() \
		<< ", Strength: " << player.at(0).getStr() << ", Dexterity: " << player.at(0).getDex() \
		<< " | Current Weapon: " << player.at(0).getWeapon().getAction() << endl;
	cout << endl;
}
*/

std::vector<_Tile>& FirstBoss::getDungeon() {
	return m_boss;
}
int FirstBoss::getRows() const {
	return m_rows;
}
int FirstBoss::getCols() const {
	return m_cols;
}
std::vector<std::shared_ptr<Monster>>& FirstBoss::getMonsters() {
	return m_firstbossMonsters;
}
std::vector<std::shared_ptr<Objects>>& FirstBoss::getActives() {
	return m_firstbossActives;
}
std::vector<std::shared_ptr<Objects>>& FirstBoss::getTraps() {
	return m_traps;
}


void FirstBoss::finishActions() {
	// if there are any lingering actions, finish them
	auto actions = m_scene->getActionManager();
	while (actions->getNumberOfRunningActions() > 0) { // > 0 because sprite::follow is not used right now
		actions->update(1.0);
	}
}


//		SOUND EFFECTS
void playMonsterDeathByPit(Player p, Monster m) {
	int px = p.getPosX();
	int py = p.getPosY();

	int mx = m.getPosX();
	int my = m.getPosY();

	std::string monster = m.getName();
	if (monster == "goblin") {
		cocos2d::experimental::AudioEngine::play2d("Goblin_Falling.mp3", false, (float)exp(-(abs(px - mx) + abs(py - my)) / 2));
	}
	else if (monster == "mounted knight") {
		cocos2d::experimental::AudioEngine::play2d("Knight_Falling.mp3", false, (float)exp(-(abs(px - mx) + abs(py - my)) / 2));
	}

	cocos2d::experimental::AudioEngine::play2d("Falling_In_A_Hole.mp3", false, (float)exp(-(abs(px - mx) + abs(py - my)) / 2));
}
void playArcherDaggerSwipe() {
	std::string sound;
	int n = randInt(6) + 1;

	switch (n) {
	case 1: sound = "Dagger_Swipe1.mp3"; break;
	case 2: sound = "Dagger_Swipe2.mp3"; break;
	case 3: sound = "Dagger_Swipe3.mp3"; break;
	}
	cocos2d::experimental::AudioEngine::play2d(sound, false, 0.6f);
}

//		SPRITE EFFECTS
void runPlayerDamage(cocos2d::Sprite* sprite) {
	auto tintRed = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 70, 70));
	auto tintNormal = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 255, 255));
	sprite->runAction(cocos2d::Blink::create(0.2, 4));
	sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintRed, tintNormal));
}
void runMonsterDamage(cocos2d::Sprite* sprite) {
	auto tintRed = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 60, 60));
	auto tintNormal = cocos2d::TintTo::create(0.1, cocos2d::Color3B(255, 255, 255));
	sprite->runAction(cocos2d::Blink::create(0.2, 5));
	sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintRed, tintNormal));
}
void runMonsterDamageBlink(cocos2d::Sprite* sprite) {
	sprite->runAction(cocos2d::Blink::create(0.2, 5));
}
void deathFade(cocos2d::Sprite* sprite) {
	auto scale = cocos2d::ScaleTo::create(0.5, 0);
	auto fade = cocos2d::FadeOut::create(0.5);
	sprite->runAction(scale);
	sprite->runAction(fade);
}