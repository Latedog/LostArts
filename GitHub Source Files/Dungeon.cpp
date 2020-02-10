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
			tile->npc = tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = tile->exit = false;
			tile->gold = 0;
			tile->marked = tile->noSpawn = false;
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
			m_maze[i*cols + j].boundary = true;
		}
		i += (rows - 2);
	}

	for (i = 0; i < rows - 1; i++){
		//	initialize edges of dungeon to be walls
		m_maze[i*cols].top = m_maze[i*cols].bottom = m_maze[i*cols].traptile = UNBREAKABLE_WALL;
		m_maze[i*cols + cols - 1].top = m_maze[i*cols + cols -1].bottom = m_maze[i*cols + cols - 1].traptile = UNBREAKABLE_WALL;
		m_maze[i*cols].wall = true;
		m_maze[i*cols + cols - 1].wall = true;
		m_maze[i*cols].boundary = true;
		m_maze[i*cols + cols - 1].boundary = true;
	}


	// GENERATES LEVEL
	Player play;
	std::vector<std::string> finalvec = generateLevel();
	fillLevel(finalvec, play);

	// END LEVEL GENERATION


	bool wall, enemy, _trap, _item, hero, noSpawn;
	char toptile, bottomtile, traptile;
	int n, x, y;


	// CREATURE LOVER
	x = 1 + randInt(cols - 2);
	y = 1 + randInt(rows - 2);

	noSpawn = m_maze[y*cols + x].noSpawn;
	wall = m_maze[y*cols + x].wall;
	enemy = m_maze[y*cols + x].enemy;
	hero = m_maze[y*cols + x].hero;
	_trap = m_maze[y*cols + x].trap;

	while (noSpawn || wall || enemy || hero || _trap) {
		x = 1 + randInt(cols - 2);
		y = 1 + randInt(rows - 2);

		noSpawn = m_maze[y*cols + x].noSpawn;
		wall = m_maze[y*cols + x].wall;
		enemy = m_maze[y*cols + x].enemy;
		hero = m_maze[y*cols + x].hero;
		_trap = m_maze[y*cols + x].trap;
	}

	std::shared_ptr<NPC> npc = std::make_shared<CreatureLover>(x, y);
	m_npcs.emplace_back(npc);
	m_maze[y*cols + x].npc = true;
	m_maze[y*cols + x].wall = true; // so they can't be moved


	// BEGIN MONSTER GENERATION
	char monstertype;
	std::shared_ptr<Monster> monster(nullptr);

	for (int i = 0; i < 11; i++) {

		// number of each enemy to spawn
		switch (i) {
		case 0: n = 1 + randInt(3); break;	// Goblin
		case 1: n = 12 + randInt(5); break; // Wanderer
		case 2: n = 2 + randInt(4); break;	// Archer
		case 3: n = 5 + randInt(4); break;	// Zapper
		case 4: n = 4 + randInt(4); break;	// Broundabout
		case 5: n = 1; break;				// Bombee
		case 6: n = 6 + randInt(5); break;	// Roundabout
		case 7: n = 1 + randInt(2); break;	// Mounted Knight
		case 8: n = 8 + randInt(4); break;	// Seeker
		case 9: n = 5 + randInt(3); break;	// Dead Seeker
		case 10: n = 1; break;				// Item Thief
		}

		while (n > 0) {

			x = 1 + randInt(cols - 2);
			y = 1 + randInt(rows - 2);

			noSpawn = m_maze[y*cols + x].noSpawn;
			wall = m_maze[y*cols + x].wall;
			enemy = m_maze[y*cols + x].enemy;
			hero = m_maze[y*cols + x].hero;

			while (noSpawn || wall || enemy || hero) { // while monster position clashes with wall, player, or idol, reroll its position
				x = 1 + randInt(cols - 2);
				y = 1 + randInt(rows - 2);

				noSpawn = m_maze[y*cols + x].noSpawn;
				wall = m_maze[y*cols + x].wall;
				enemy = m_maze[y*cols + x].enemy;
				hero = m_maze[y*cols + x].hero;
			}

			// the type of enemy
			switch (i) {
			case 0:
				monster = std::make_shared<Goblin>(x, y, 10); break;
			case 1:
				monster = std::make_shared<Wanderer>(x, y); break;
			case 2:
				monster = std::make_shared<Archer>(x, y); break;
			case 3:
				monster = std::make_shared<Zapper>(x, y, rows); break;
			case 4:
				monster = std::make_shared<Broundabout>(x, y); break;
			case 5:
				monster = std::make_shared<Bombee>(x, y); break;
			case 6:
				monster = std::make_shared<Roundabout>(x, y);break;
			case 7:
				monster = std::make_shared<MountedKnight>(x, y); break;
			case 8:
				monster = std::make_shared<Seeker>(x, y, 10); break;
			case 9:
				monster = std::make_shared<GooSack>(x, y); break;
			case 10:
				monster = std::make_shared<ItemThief>(x, y); break;
			}

			m_monsters.emplace_back(monster);

			m_maze[y*cols + x].monster_name = monster->getName();
			m_maze[y*cols + x].enemy = true;

			/*if (monstertype == SPINNER) {
				std::shared_ptr<Spinner> spinner = std::dynamic_pointer_cast<Spinner>(monster);
				spinner->setInitialFirePosition(x, y, rows);
				spinner.reset();
			}*/

			n--;
			monster.reset();
		}
	}
	/// END MONSTER GENERATION

	
	//	TRAP GENERATION
	char traptype;
	std::string trap_type = "";
	std::shared_ptr<Traps> trap(nullptr);
	
	for (int i = 0; i < 8; i++) { // i < number_of_different_trap_types

		// number of each trap to spawn
		switch (i) {
		case 0: n = 2 + randInt(5); break; // Pit
		case 1: n = 1 + randInt(3); break; // Firebars
		case 2: n = 0 + randInt(2); break; // Double Firebars
		case 3: n = 15 + randInt(5); break; // Puddles
		case 4: n = 5 + randInt(10); break; // Springs
		case 5: n = 3 + randInt(3); break; // Turrets
		case 6: n = 2 + randInt(3); break; // Moving Blocks
		case 7: n = 10 + randInt(3); break; // 
		case 8: n = 4 + randInt(4); break; // 
		default: n = 0; break;
		}

		while (n > 0) {
			
			x = 1 + randInt(cols - 2);
			y = 1 + randInt(rows - 2);

			noSpawn = m_maze[y*cols + x].noSpawn;
			_trap = m_maze[y*cols + x].trap;
			wall = m_maze[y*cols + x].wall;
			enemy = m_maze[y*cols + x].enemy;
			hero = m_maze[y*cols + x].hero;
			_item = m_maze[y*cols + x].item;

			// while item position clashes with wall, player, etc., then reroll its position
			while (noSpawn || _trap || wall || enemy || hero || _item) {
				x = 1 + randInt(cols - 2);
				y = 1 + randInt(rows - 2);

				noSpawn = m_maze[y*cols + x].noSpawn;
				_trap = m_maze[y*cols + x].trap;
				wall = m_maze[y*cols + x].wall;
				enemy = m_maze[y*cols + x].enemy;
				hero = m_maze[y*cols + x].hero;
				_item = m_maze[y*cols + x].item;
			}

			// type of item to spawn
			switch (i) {
			case 0:
				trap = std::make_shared<Pit>(x, y);
				trap_type = PIT; break;
			case 1:
				trap = std::make_shared<Firebar>(x, y, rows);
				trap_type = FIREBAR; break;
			case 2:
				trap = std::make_shared<DoubleFirebar>(x, y, rows);
				trap_type = DOUBLE_FIREBAR; break;
			case 3:
				trap = std::make_shared<Puddle>(x, y);
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
				trap = std::make_shared<Spring>(x, y, false);
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
				trap = std::make_shared<Turret>(x, y, move);
				trap_type = TURRET; break;
			case 6:
				trap = std::make_shared<MovingBlock>(x, y, (randInt(2) == 0 ? 'h' : 'v'));
				trap_type = MOVING_BLOCK; break;
			case 7:
				trap = std::make_shared<CrumbleFloor>(x, y, 4);
				trap_type = CRUMBLE_FLOOR; break;
			case 8:
				//item = std::make_shared<InfinityBox>();
				break;//itemtype = INFINITY_BOX; break;
			}

			m_traps.emplace_back(trap);

			m_maze[y*cols + x].trap_name = trap->getItem();
			m_maze[y*cols + x].trap = true;

			if (trap->actsAsWall()) {
				m_maze[y*cols + x].wall = true;
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

	for (int i = 0; i < 19; i++) { // i < number_of_different_items

		// number of each item to spawn
		switch (i) {
		case 0: n = 1; break; // Life Potion
		case 1: n = 1; break; // Bomb
		case 2: n = 1; break; // Brown Chest
		case 3: n = 1; break; // Rusty Cutlass
		case 4: n = 1; break; // Skeleton Key
		case 5: n = 1; break; // Wood Shield
		case 6: n = 2; break; // Freeze Spell
		case 7: n = 1 + randInt(3); break; // Earthquake Spell
		case 8: n = 2 + randInt(3); break; // 
		case 9: n = 2 + randInt(3); break; // 
		case 10: n = 2 + randInt(3); break; // 
		case 11: n = 1 + randInt(1); break; // 
		case 12: n = 1; break;
		case 13: n = 1; break;
		case 14: n = 1; break;
		case 15: n = 1; break;
		case 16: n = 1; break;
		case 17: n = 1; break;
		case 18: n = 1; break;
		case 19: n = 1; break;
		case 20: n = 1; break;
		case 21: n = 1; break;
		default: n = 0; break;
		}

		while (n > 0) {

			x = 1 + randInt(cols - 2);
			y = 1 + randInt(rows - 2);

			noSpawn = m_maze[y*cols + x].noSpawn;
			_item = m_maze[y*cols + x].item;
			wall = m_maze[y*cols + x].wall;
			_trap = m_maze[y*cols + x].trap;
			hero = m_maze[y*cols + x].hero;

			// while item position clashes with wall, player, etc., then reroll its position
			while (noSpawn || _item || wall || _trap || hero) {
				x = 1 + randInt(cols - 2);
				y = 1 + randInt(rows - 2);

				noSpawn = m_maze[y*cols + x].noSpawn;
				_item = m_maze[y*cols + x].item;
				wall = m_maze[y*cols + x].wall;
				_trap = m_maze[y*cols + x].trap;
				hero = m_maze[y*cols + x].hero;
			}

			// type of item to spawn
			switch (i) {
			case 0:
				item = std::make_shared<LifePotion>(x, y); break;
			case 1:
				item = std::make_shared<Bomb>(x, y); break;
			case 2:
				item = std::make_shared<BrownChest>(); break;
			case 3:
				item = std::make_shared<GoldPot>(x, y); break;
			case 4:
				item = std::make_shared<DarkStar>(x, y); break;
			case 5:
				item = std::make_shared<WoodShield>(x, y); break;
			case 6:
				item = std::make_shared<FreezeSpell>(x, y); break;
			case 7:
				item = std::make_shared<EarthquakeSpell>(x, y); break;
			case 8:
				item = std::make_shared<FireBlastSpell>(x, y); break;
			case 9:
				item = std::make_shared<WindSpell>(x, y); break;
			case 10:
				item = std::make_shared<InvisibilitySpell>(x, y); break;
			case 11:
				item = std::make_shared<BrightStar>(x, y); break;
			case 12:
				item = std::make_shared<Teleport>(x, y); break;
			case 13:
				item = std::make_shared<VulcanRune>(x, y); break;
			case 14:
				item = std::make_shared<EtherealSpell>(x, y); break;
			case 15:
				item = std::make_shared<LuckyPig>(x, y); break;
			case 16:
				item = std::make_shared<RingOfCasting>(x, y); break;
			case 17:
				item = std::make_shared<Bloodrite>(x, y); break;
			case 18:
				item = std::make_shared<Bloodlust>(x, y); break;
			case 19:
				item = std::make_shared<InvisibilitySpell>(x, y); break;
			case 20:
				item = std::make_shared<InvisibilitySpell>(x, y); break;
			}

			m_maze[y*cols + x].object = item;
			m_maze[y*cols + x].item_name = item->getItem();
			m_maze[y*cols + x].item = true;
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

	m_npcs = dungeon.m_npcs;
	m_monsters = dungeon.m_monsters;
	m_traps = dungeon.m_traps;
	m_doors = dungeon.m_doors;

	m_level = dungeon.m_level;
	layer = dungeon.layer;
	playerChunk = dungeon.playerChunk;
	specialChunk1 = dungeon.specialChunk1;
	specialChunk2 = dungeon.specialChunk2;
	specialChunkLayer1 = dungeon.specialChunkLayer1;
	specialChunkLayer2 = dungeon.specialChunkLayer2;

	//m_spawn.clear();
	m_seq.clear();

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
	bool boundary, wall, item, trap, enemy, npc;

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

	// set player's action key (for items that have casting time)
	player.at(0).setAction(move);

	// red tint if player has bloodlust
	if (player.at(0).hasBloodlust()) {
		bloodlustTint(player.at(0));
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
	if (!player.at(0).isEthereal() && m_maze[y * cols + x].wall && (m_maze[y*cols + x].top != DOOR_H_OPEN && m_maze[y*cols + x].top != DOOR_V_OPEN)) {
		cocos2d::experimental::AudioEngine::play2d("Female_Falling_Scream_License.mp3", false, 1.0f);
		deathFade(player.at(0).getSprite());
		player.at(0).setHP(0);
		return;
	}

	// if player is not immobilized, move them
	if (!(player.at(0).isStunned() || player.at(0).isFrozen())) {

		bool moveUsed = false;

		// check for special weapon attack patterns
		if (player.at(0).getWeapon()->hasAttackPattern()) {
			player.at(0).getWeapon()->usePattern(*this, moveUsed);
		}

		if (!moveUsed) {

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

				top = m_maze[(y + m)*cols + (x + n)].top;
				bottom = m_maze[(y + m)*cols + (x + n)].bottom;
				traptile = m_maze[(y + m)*cols + (x + n)].traptile;

				boundary = m_maze[(y + m)*cols + (x + n)].boundary;
				wall = (player.at(0).isEthereal() ? false : m_maze[(y + m)*cols + (x + n)].wall);
				item = m_maze[(y + m)*cols + (x + n)].item;
				trap = m_maze[(y + m)*cols + (x + n)].trap;
				enemy = m_maze[(y + m)*cols + (x + n)].enemy;
				npc = m_maze[(y + m)*cols + (x + n)].npc;

				if (npc) {
					interactWithNPC(x + n, y + m);
				}
				else if (!(wall || enemy || boundary)) {
					m_maze[y*cols + x].hero = false;
					m_maze[(y + m)*cols + (x + n)].hero = true;
					player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);

					// pick up any gold/money that was on the ground
					if (m_maze[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
						goldPickup(m_maze, player.at(0).getPosX(), player.at(0).getPosY());
					}

					if (trap) {
						// queue player move
						queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY());

						trapEncounter(x + n, y + m);
					}
					if (item) {
						foundItem(m_maze, x + n, y + m);
					}
				}
				else if (wall && !boundary) {
					if (top == DOOR_H || top == DOOR_V) {
						openDoor(x + n, y + m);
					}
					else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
						m_maze[y*cols + x].hero = false;
						m_maze[(y + m)*cols + (x + n)].hero = true;
						player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);
					}
					else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
						// play locked door sound
						cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
					}
				}
				else if (enemy) {	// monster encountered
					fight(x + n, y + m);
				}
			}

			else if (move == 'e') {
				collectItem(*this, x, y);

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

		}

	}

	// queue player move
	if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
		queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY());
	}

	// pick up any gold/money that was on the ground
	if (m_maze[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
		goldPickup(m_maze, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// check active items in play
	checkActive();

	// update sprite lighting
	updateLighting();

	// if there are any doors, check them
	if (!getDoors().empty()) {
		checkDoors();
	}

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
	if (getMonsters().empty()) {
		//if (!dungeonText.empty())
			//showText();

		return;
	}

	// :::: Move monsters ::::
	int mx, my, mInitHP;
	for (unsigned i = 0; i < getMonsters().size(); i++) {
		mInitHP = getMonsters().at(i)->getHP();
		mx = getMonsters().at(i)->getPosX();
		my = getMonsters().at(i)->getPosY();

		// check for any afflictions
		getMonsters().at(i)->checkAfflictions();

		// flash monster sprite if damage was taken
		if (getMonsters().at(i)->getHP() < mInitHP) {
			runMonsterDamageBlink(getMonsters().at(i)->getSprite());
			mInitHP = getMonsters().at(i)->getHP();
		}

		// check if an affliction killed the monster
		if (getMonsters().at(i)->getHP() <= 0) {
			monsterDeath(i);
			i--;
			continue;
		}

		// if monster is too far away, don't bother moving them
		if (abs(player.at(0).getPosX() - mx) + abs(player.at(0).getPosY() - my) > 25) {
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

			getMonsters().at(i)->move(*getMonsters().at(i), *this);

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
	monsterTrapEncounter();

	// check if player is holding the skeleton key
	if (player.at(0).hasSkeletonKey()) {
		player.at(0).checkKeyConditions(dungeonText);
	}

	// invulnerability check: if player is invulnerable and their hp is lower than initial, set hp back to original
	if (player.at(0).isInvulnerable() && player.at(0).getHP() < initHP) {
		player.at(0).setHP(initHP);
	}

	// flash player sprite if took damage
	if (player.at(0).getHP() < initHP) {

		// if player is invisible, they lose invisibility when damaged unless they had enough intellect
		if (player.at(0).isInvisible() && player.at(0).getInt() < 8) {
			player.at(0).removeAffliction("invisibility");
		}

		runPlayerDamage(player.at(0).getSprite());

		player.at(0).decreaseMoneyBonus();
	}

	// run actions
	runSpawn();

	//if (!dungeonText.empty())
		//showText();
}

void Dungeon::checkActive() {
	int rows = getRows();
	int cols = getCols();

	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int x, y;
	int pos;
	int oldSize;

	if (!getTraps().empty()) {
		for (unsigned i = 0; i < getTraps().size(); i++) {
			int oldSize = getTraps().size();

			getTraps().at(i)->activeTrapAction(*this, player.at(0));

			// if the current trap was removed from actives, decrement to not skip any actives
			if (getTraps().size() < oldSize)
				i--;
		}
	}

	// cleans up any destroyed traps
	trapRemoval();
}

int Dungeon::findDoor(int x, int y) {
	for (int i = 0; i < getDoors().size(); i++) {
		if (getDoors().at(i)->getPosX() == x && getDoors().at(i)->getPosY() == y) {
			return i;
		}
	}

	return -1;
}
void Dungeon::openDoor(int x, int y) {
	int i = findDoor(x, y);

	if (i == -1)
		return;

	int rows = getRows();
	int cols = getCols();
	shared_ptr<Door> door = dynamic_pointer_cast<Door>(getDoors().at(i));

	// if the door isn't locked, open it
	if (!door->isLocked()) {
		// door opening sound
		cocos2d::experimental::AudioEngine::play2d("Door_Opened.mp3", false, 1.0f);

		// remove door sprite
		removeSprite(door_sprites, rows, x, y);

		door->toggleOpen();
		getDungeon()[y*cols + x].top = (getDungeon()[y*cols + x].top == DOOR_H ? DOOR_H_OPEN : DOOR_V_OPEN);
	}

	door.reset();
}
void Dungeon::checkDoors() {
	if (!getDoors().empty()) {
		int rows = getRows();
		int cols = getCols();
		int x, y;

		for (int i = 0; i < getDoors().size(); i++) {
			shared_ptr<Door> door = getDoors().at(i);
			x = door->getPosX();
			y = door->getPosY();

			bool enemy, hero;
			enemy = getDungeon()[y*cols + x].enemy;
			hero = getDungeon()[y*cols + x].hero;

			// if door is open, isn't being held, and nothing is in the way
			if (door->isOpen() && !door->isHeld() && !(enemy || hero) && (getDungeon()[y*cols + x].top == DOOR_H_OPEN || getDungeon()[y*cols + x].top == DOOR_V_OPEN)) {
				// close the door
				cocos2d::experimental::AudioEngine::play2d("Door_Closed.mp3", false, 1.0f);

				// replace sprite with closed door
				if (getDungeon()[y*cols + x].traptile == DOOR_H)
					addSprite(door_sprites, rows, x, y, -2, "Door_Horizontal_Closed_48x48.png");
				else
					addSprite(door_sprites, rows, x, y, -2, "Door_Vertical_Closed_48x48.png");

				// close the door
				getDungeon()[y*cols + x].top = (getDungeon()[y*cols + x].traptile == DOOR_H ? DOOR_H : DOOR_V);
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

int Dungeon::findNPC(int x, int y) {
	for (int i = 0; i < getNPCs().size(); i++) {
		if (getNPCs().at(i)->getPosX() == x && getNPCs().at(i)->getPosY() == y) {
			return i;
		}
	}

	return -1;
}
void Dungeon::interactWithNPC(int x, int y) {
	int pos = findNPC(x, y);

	if (pos != -1) {
		getNPCs().at(pos)->talk(*this);
	}
}

//		Trap handling
void Dungeon::trapRemoval() {
	// removes any traps that were just destroyed after checkActive

	if (m_trapIndexes.empty())
		return;

	int pos;
	for (auto &it : m_trapIndexes) {
		pos = findTrap(it.first, it.second);
		if (pos != -1) {
			getTraps().erase(getTraps().begin() + pos);
		}
	}
	m_trapIndexes.clear();
}
int Dungeon::findTrap(int x, int y, bool endFirst) {

	// search forward
	if (!endFirst) {
		for (int i = 0; i < getTraps().size(); i++) {
			if (getTraps().at(i)->getPosX() == x && getTraps().at(i)->getPosY() == y) {
				return i;
			}
		}

		return -1;
	}

	// else search backward
	for (int i = getTraps().size() - 1; i >= 0; i--) {
		if (getTraps().at(i)->getPosX() == x && getTraps().at(i)->getPosY() == y) {
			return i;
		}
	}

	return -1;
}
void Dungeon::trapEncounter(int x, int y) { // trap's coordinates
	int rows = getRows();
	int cols = getCols();

	int i = findTrap(x, y);

	if (i == -1)
		return;

	getTraps().at(i)->trapAction(*getTraps().at(i), *this, player.at(0));

	trapRemoval();

	/*
	if (i != -1) {

		getTraps().at(i)->trapAction(*getTraps().at(i), dungeon, player.at(0));

		// if the trap is a one-time only trap, remove it
		if (dungeon.getTraps().at(i)->isTemporary() && !dungeon.getTraps().at(i)->isActive()) {
			dungeon[y*cols + x].trap = false;
			dungeon[y*cols + x].trap_name = "";
			queueRemoveSprite(x, y, dungeon.getTraps().at(i)->getSprite());
			dungeon.getTraps().erase(dungeon.getTraps().begin() + i);
		}
		// if trap is one-time only and is an active, remove it from the traps and actives pool
		else if (dungeon.getTraps().at(i)->isTemporary() && dungeon.getTraps().at(i)->isActive()) {
			dungeon[y*cols + x].trap = false;
			dungeon[y*cols + x].trap_name = "";
			dungeon.getTraps().erase(dungeon.getTraps().begin() + i);

			if (pos == -1)
				return;

			queueRemoveSprite(x, y, dungeon.getActives().at(pos)->getSprite());
			dungeon.getActives().erase(dungeon.getActives().begin() + pos);
		}
	}
	else if (pos != -1) {
		//if (dungeon.getTraps().at(i)->isTemporary() && dungeon.getTraps().at(i)->isActive()) {
			dungeon[y*cols + x].trap = false;
			dungeon[y*cols + x].trap_name = "";
			//dungeon.getTraps().erase(dungeon.getTraps().begin() + i);

			queueRemoveSprite(x, y, dungeon.getActives().at(pos)->getSprite());
			dungeon.getActives().erase(dungeon.getActives().begin() + pos);
		//}
	}
	*/

}

void Dungeon::monsterTrapEncounter() {
	int rows = getRows();
	int cols = getCols();
	int x, y;
	bool _trap;
	std::string trap_name;
	
	for (int i = 0; i < getMonsters().size(); i++) {
		x = getMonsters().at(i)->getPosX();
		y = getMonsters().at(i)->getPosY();
		_trap = getDungeon()[y*cols + x].trap;
		trap_name = getDungeon()[y*cols + x].trap_name;

		if (_trap) {
			std::shared_ptr<Traps> trap = std::dynamic_pointer_cast<Traps>(getTraps().at(findTrap(x, y)));
			trap->trapAction(*this, *getMonsters().at(i));
			trap.reset();
		}

		/*
		if (trap) {
			if (trap_name == PIT) {
				if (!getMonsters().at(i)->isFlying()) {
					// play falling sound effect
					playMonsterDeathByPit(player.at(0), *getMonsters().at(i));

					// death animation
					deathFade(getMonsters().at(i)->getSprite());

					destroyMonster(i);
				}
			}
			else if (trap_name == SPRING) {
				if (!getMonsters().at(i)->isFlying()) {
					std::shared_ptr<Spring> spring = std::dynamic_pointer_cast<Spring>(getTraps().at(findTrap(x, y)));
					spring->trapAction(*this, *getMonsters().at(i));
					spring.reset();
				}
			}
		}
		*/
	}
}
void Dungeon::singleMonsterTrapEncounter(int pos) {
	int rows = getRows();
	int cols = getCols();
	int x, y;
	bool trap;
	std::string trap_name;
	
	x = getMonsters().at(pos)->getPosX();
	y = getMonsters().at(pos)->getPosY();
	trap = getDungeon()[y*cols + x].trap;
	trap_name = getDungeon()[y*cols + x].trap_name;

	if (trap) {

		std::shared_ptr<Traps> trap = std::dynamic_pointer_cast<Traps>(getTraps().at(findTrap(x, y)));
		trap->trapAction(*this, *getMonsters().at(pos));
		trap.reset();

		/*
		if (trap_name == PIT) {
			if (!getMonsters().at(pos)->isFlying()) {
				// play falling sound effect
				playMonsterDeathByPit(player.at(0), *getMonsters().at(pos));

				// death animation
				deathFade(getMonsters().at(pos)->getSprite());

				destroyMonster(pos);
			}
		}
		else if (trap_name == SPRING) {
			if (!getMonsters().at(pos)->isFlying()) {
				std::shared_ptr<Spring> spring = std::dynamic_pointer_cast<Spring>(getTraps().at(findTrap(x, y)));
				spring->trapAction(*this, *getMonsters().at(pos));
				spring.reset();
			}
		}
		*/
	}
}

int Dungeon::findMonster(int mx, int my) {
	for (int i = 0; i < getMonsters().size(); i++) {
		if (getMonsters().at(i)->getPosX() == mx && getMonsters().at(i)->getPosY() == my) {
			return i;
		}
	}
	return -1;
}
void Dungeon::fight(int x, int y) { // monster's coordinates
	int i = findMonster(x, y);

	if (i == -1)
		return;

	player.at(0).attack(*this, *getMonsters().at(i));

	if (getMonsters().at(i)->getHP() <= 0) {
		monsterDeath(i);
	}
}
void Dungeon::destroyMonster(int pos) {
	// destroyMonster is used to eliminate monsters without having them drop anything

	int x = getMonsters().at(pos)->getPosX();
	int y = getMonsters().at(pos)->getPosY();

	getDungeon()[y*getCols() + x].top = SPACE;
	getDungeon()[y*getCols() + x].enemy = false;

	queueRemoveSprite(getMonsters().at(pos)->getSprite());
	getMonsters().erase(getMonsters().begin() + pos);
}
void Dungeon::monsterDeath(int pos) { // monster's coordinates
	// call monster's death function
	getMonsters().at(pos)->death(*getMonsters().at(pos), *this);

	// remove them from the monster vector
	getMonsters().erase(getMonsters().begin() + pos);
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
					
					dungeon[my*maxcols + mx].enemy = true;
					dungeon.getMonsters()[i]->setPosX(mx);
					dungeon.getMonsters()[i]->setPosY(my);


					queueMoveSprite(dungeon.getMonsters().at(i)->getSprite(), mx, my, 0.05f);

					if (dungeon.getMonsters().at(i)->getName() == SPINNER) {
						std::shared_ptr<Spinner> spinner = std::dynamic_pointer_cast<Spinner>(dungeon.getMonsters().at(i));
						spinner->setInitialFirePosition(mx, my, maxrows);
						spinner.reset();
					}
					else if (dungeon.getMonsters().at(i)->getName() == ZAPPER) {
						std::shared_ptr<Zapper> zapper = std::dynamic_pointer_cast<Zapper>(dungeon.getMonsters().at(i));
						zapper->moveSprites(mx, my, maxrows);
						zapper.reset();
					}
				}
				else {
					monsterDeath(i);
				}
				break;
			}
					  //	CUSTOM MOVEMENT CASE in X direction
			case 'X': {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;

				dungeon[my*maxcols + cx].enemy = true;

				dungeon.getMonsters()[i]->setPosX(cx);
				queueMoveSprite(dungeon.getMonsters().at(i)->getSprite(), cx, my);
				break;
			}
					  //	CUSTOM MOVEMENT CASE in Y direction
			case 'Y': {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;

				dungeon[(cy)*maxcols + mx].enemy = true;

				dungeon.getMonsters()[i]->setPosY(cy);
				queueMoveSprite(dungeon.getMonsters().at(i)->getSprite(), mx, cy);
				break;
			}
			}
			return;
		}
	}
}
void Dungeon::pushPlayer(char move) {
	int cols = getCols();
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	bool b = false;
	int ox = px;
	int oy = py;

	if (monsterHash(*this, px, py, b, move)) {
		getDungeon()[oy*cols + ox].hero = false;
		getDungeon()[py*cols + px].hero = true;
		player.at(0).setPosX(px);
		player.at(0).setPosY(py);

		// queue player move
		queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY());
	}
	else {
		; // could kill player here later
	}

}


//		Item collection
void Dungeon::itemHash(std::vector<_Tile> &dungeon, int &x, int &y) {
	bool item = dungeon[y*getCols() + x].item;
	bool wall = dungeon[y*getCols() + x].wall;
	bool trap = dungeon[y*getCols() + x].trap;

	// if current tile has no item and the position is valid, return to set item here
	if (!(item || wall || trap)) {
		return;
	}

	int n = -1 + randInt(3);
	int m = -1 + randInt(3);

	// while hash draws out of bounds, find a new position
	while (dungeon[(y + m)*getCols() + (x + n)].top == UNBREAKABLE_WALL) {
		n = -1 + randInt(3);
		m = -1 + randInt(3);
	}

	// hash until we find a valid spot
	itemHash(dungeon, x += n, y += m);
}
void Dungeon::foundItem(std::vector<_Tile> &dungeon, int x, int y) {
	//char c = dungeon[y*maxcols + x].bottom;

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
void Dungeon::collectItem(Dungeon &dungeon, int x, int y) {
	int maxrows = getRows();
	int maxcols = getCols();

	int current_inv_size = player.at(0).getWeapons().size() + player.at(0).getItems().size();
	int current_wep_inv_size = player.at(0).getWeapons().size();
	int current_item_inv_size = player.at(0).getItems().size();
	int max_wep_inv_size = player.at(0).getMaxWeaponInvSize();
	int max_item_inv_size = player.at(0).getMaxItemInvSize();
	
	char bottom = dungeon[y*maxcols + x].bottom;
	char traptile = dungeon[y*maxcols + x].traptile;
	std::string trap_name = dungeon[y*maxcols + x].trap_name;
	bool exit = dungeon[y*maxcols + x].exit;
	std::string item_name = dungeon[y*maxcols + x].item_name;
	std::shared_ptr<Objects> object = dungeon[y*maxcols + x].object;
	std::string image;

	// if it's the stairs, then advance level and return
	if (exit) {
		setLevel(getLevel() + 1);
		return;
	}

	// if stairs are locked, then play a sound effect
	if (trap_name == LOCKED_STAIRCASE) {
		// locked sound effect
		cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
	}

	if (bottom == IDOL) {
		player.at(0).setWin(true);
		dungeon[y*maxcols + x].bottom = SPACE;
		return;
	}

	// if object is null, then do nothing and return
	if (object == nullptr) {
		cocos2d::experimental::AudioEngine::play2d("Player_Movement.mp3", false, 1.0f);

		return;
	}

	// otherwise check if there's an item to interact with
	if (object->isAutoUse()) {
		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		drop->changeStats(*drop, player.at(0));

		cocos2d::experimental::AudioEngine::play2d(drop->getSoundName(), false, 1.0f);

		drop.reset();

		dungeon[y*maxcols + x].item_name = EMPTY;
		dungeon[y*maxcols + x].object = nullptr;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
	else if (object->isItem()) {
		if (current_item_inv_size < max_item_inv_size) {
			std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
			player.at(0).addItem(drop);

			cocos2d::experimental::AudioEngine::play2d(drop->getSoundName(), false, 1.0f);

			drop.reset();
		}
	}
	else if (object->isWeapon()) {
		if (current_wep_inv_size < max_wep_inv_size) {
			std::shared_ptr<Weapon> weapon = std::dynamic_pointer_cast<Weapon>(object);
			player.at(0).addWeapon(weapon);

			cocos2d::experimental::AudioEngine::play2d(weapon->getSoundName(), false, 1.0f);

			weapon.reset();
		}
	}
	else if (object->isChest()) {
		std::shared_ptr<Chests> chest = std::dynamic_pointer_cast<Chests>(object);

		removeSprite(item_sprites, maxrows, x, y);
		image = chest->open(*chest, dungeon[y*maxcols + x]);
		addSprite(item_sprites, maxrows, x, y, -1, image);

		cocos2d::experimental::AudioEngine::play2d(chest->getSoundName(), false, 1.0f);

		chest.reset();
	}
	else if (object->isShield()) {
		std::shared_ptr<Shield> shield = std::dynamic_pointer_cast<Shield>(object);
		player.at(0).equipShield(*this, shield);
		shield.reset();
	}
	else if (object->isTrinket()) {
		std::shared_ptr<Trinket> trinket = std::dynamic_pointer_cast<Trinket>(object);
		player.at(0).equipTrinket(*this, trinket);
		trinket.reset();
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
void Dungeon::goldPickup(std::vector<_Tile> &dungeon, int x, int y) {
	int rows = getRows();
	int cols = getCols();

	cocos2d::experimental::AudioEngine::play2d("Gold_Pickup2.mp3", false, 1.0f);

	player.at(0).setMoney(player.at(0).getMoney() + dungeon[y*cols + x].gold);
	dungeon[y*cols + x].gold = 0;
	removeSprite(money_sprites, rows, x, y);
}

//		Getters
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
std::vector<std::shared_ptr<NPC>>& Dungeon::getNPCs() {
	return m_npcs;
}
std::vector<std::shared_ptr<Monster>>& Dungeon::getMonsters() {
	return m_monsters;
}
std::vector<std::shared_ptr<Traps>>& Dungeon::getTraps() {
	return m_traps;
}
std::vector<std::shared_ptr<Door>>& Dungeon::getDoors() {
	return m_doors;
}
void Dungeon::callUse(std::vector<_Tile> &dungeon, int x, int y, int index) {
	player.at(0).use(*this, dungeon[y*getCols() + x], index);
}

//		:::: SPRITE SETTING ::::
void Dungeon::setPlayerSprite(cocos2d::Sprite* sprite) {
	player_sprite = sprite;
}
void Dungeon::setMoneySprites(std::vector<cocos2d::Sprite*> sprites) {
	money_sprites = sprites;
}
void Dungeon::setItemSprites(std::vector<cocos2d::Sprite*> sprites) {
	item_sprites = sprites;
}
void Dungeon::setWallSprites(std::vector<cocos2d::Sprite*> sprites) {
	wall_sprites = sprites;
}
void Dungeon::setDoorSprites(std::vector<cocos2d::Sprite*> sprites) {
	door_sprites = sprites;
}
void Dungeon::setSpikeProjectileSprites(std::multimap<cocos2d::Vec2, cocos2d::Sprite*> sprites) {
	spike_sprites = sprites;
}
void Dungeon::setScene(cocos2d::Scene* scene) {
	m_scene = scene;
}

cocos2d::Sprite* Dungeon::findSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y) {
	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (getRows() - y)*SPACING_FACTOR - Y_OFFSET;

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
void Dungeon::setSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y, char move) {
	int x_sprite = x * SPACING_FACTOR - X_OFFSET;
	int y_sprite = (getRows() - y)*SPACING_FACTOR - Y_OFFSET;

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
			case 'u': sprites.at(i)->setPosition(px, (getRows() - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;
			case 'd': sprites.at(i)->setPosition(px, (getRows() - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '1': sprites.at(i)->setPosition((x - 1)*SPACING_FACTOR - X_OFFSET, (getRows() - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '2': sprites.at(i)->setPosition((x + 1)*SPACING_FACTOR - X_OFFSET, (getRows() - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '3': sprites.at(i)->setPosition((x - 1)*SPACING_FACTOR - X_OFFSET, (getRows() - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '4': sprites.at(i)->setPosition((x + 1)*SPACING_FACTOR - X_OFFSET, (getRows() - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;
			}
			return;
		}
	}
}
void Dungeon::teleportSprite(cocos2d::Sprite* sprite, int x, int y) {
	sprite->setPosition(x * SPACING_FACTOR - X_OFFSET, (getRows() - y)*SPACING_FACTOR - Y_OFFSET);
}

// Move sprites
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, char move, float time) {

	// Key: (for moveSprite 'move' parameter)
	// 1 u 2
	// l _ r
	// 3 d 4

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
void Dungeon::queueMoveSprite(cocos2d::Sprite* sprite, int cx, int cy, float time, bool front) {
	auto move = cocos2d::MoveTo::create(time, cocos2d::Vec2(cx * SPACING_FACTOR - X_OFFSET, (getRows() - cy)*SPACING_FACTOR - Y_OFFSET));
	insertActionIntoSpawn(sprite, move, front);
}
void Dungeon::moveSprite(cocos2d::Sprite* sprite, char move, float time) {
	switch (move) {
	case 'l': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-SPACING_FACTOR, 0));
		sprite->runAction(move);
		break;
	}
	case 'L': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-2 * SPACING_FACTOR, 0));
		sprite->runAction(move);
		break;
	}
	case '<': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-3 * SPACING_FACTOR, 0));
		sprite->runAction(move);
		break;
	}
	case 'r': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(SPACING_FACTOR, 0));
		sprite->runAction(move);
		break;
	}
	case 'R': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(2 * SPACING_FACTOR, 0));
		sprite->runAction(move);
		break;
	}
	case '>': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(3 * SPACING_FACTOR, 0));
		sprite->runAction(move);
		break;
	}
	case 'u': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, SPACING_FACTOR));
		sprite->runAction(move);
		break;
	}
	case 'U': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, 2 * SPACING_FACTOR));
		sprite->runAction(move);
		break;
	}
	case '^': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, 3 * SPACING_FACTOR));
		sprite->runAction(move);
		break;
	}
	case 'd': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, -SPACING_FACTOR));
		sprite->runAction(move);
		break;
	}
	case 'D': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, -2 * SPACING_FACTOR));
		sprite->runAction(move);
		break;
	}
	case 'V': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(0, -3 * SPACING_FACTOR));
		sprite->runAction(move);
		break;
	}
	case '1': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-SPACING_FACTOR, SPACING_FACTOR));
		sprite->runAction(move);
		break;
	}
	case '2': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(SPACING_FACTOR, SPACING_FACTOR));
		sprite->runAction(move);
		break;
	}
	case '3': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(-SPACING_FACTOR, -SPACING_FACTOR));
		sprite->runAction(move);
		break;
	}
	case '4': {
		auto move = cocos2d::MoveBy::create(time, cocos2d::Vec2(SPACING_FACTOR, -SPACING_FACTOR));
		sprite->runAction(move);
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
cocos2d::Sprite* Dungeon::createSprite(int x, int y, int z, std::string image) {
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithSpriteFrameName(image);
	m_scene->addChild(sprite, z);
	sprite->setPosition(x*SPACING_FACTOR - X_OFFSET, (getRows() - y)*SPACING_FACTOR - Y_OFFSET);
	
	return sprite;
}
cocos2d::Sprite* Dungeon::createSprite(std::vector<cocos2d::Sprite*> &sprites, int x, int y, int z, std::string image) {
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithSpriteFrameName(image);
	m_scene->addChild(sprite, z);
	sprite->setPosition(x*SPACING_FACTOR - X_OFFSET, (getRows() - y)*SPACING_FACTOR - Y_OFFSET);
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
void Dungeon::addGoldSprite(std::vector<_Tile> &dungeon, int x, int y) {
	int rows = getRows();
	int cols = getCols();

	int gold = dungeon[y*cols + x].gold;

	// if there's gold here already, remove the sprite before we add a new one
	if (gold != 0) {
		removeSprite(money_sprites, rows, x, y);
	}

	std::string image;

	if (gold == 1) image = "Gold_Coin1_48x48.png";
	else if (gold == 2) image = "Gold_Coin2_48x48.png";
	else if (gold == 3) image = "Gold_Coin3_48x48.png";
	else if (gold >= 4 && gold <= 10) image = "Gold_Pile1_48x48.png";
	else if (gold > 10) image = "Gold_Pile2_48x48.png";

	cocos2d::Sprite* money = cocos2d::Sprite::createWithSpriteFrameName(image);
	m_scene->addChild(money, -3);
	money->setPosition(x*SPACING_FACTOR - X_OFFSET, (rows - y)*SPACING_FACTOR - Y_OFFSET);
	money_sprites.push_back(money);
}

// Remove sprites
void Dungeon::queueRemoveSprite(cocos2d::Sprite* sprite) {
	auto remove = cocos2d::RemoveSelf::create();
	insertActionIntoSpawn(sprite, remove);
}
void Dungeon::removeSprite(cocos2d::Sprite* sprite) {
	auto remove = cocos2d::RemoveSelf::create();
	sprite->runAction(remove);
	//if (sprite != nullptr)
		//sprite->removeFromParent();
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
void Dungeon::updateLighting() {
	int rows = getRows();
	int cols = getCols();

	//Vec2 pos = player_sprite->getPosition();
	//int x = pos.x;
	//int y = pos.y;
	int cutoff = 2400;

	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int p_cutoff = player.at(0).getVision();

	cocos2d::Sprite* sprite;
	int sx, sy, dist, color;

	//sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR;
	//sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR);


	// npc sprites
	for (int i = 0; i < getNPCs().size(); i++) {
		sx = getNPCs().at(i)->getPosX();
		sy = getNPCs().at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (dist > p_cutoff) {
			getNPCs().at(i)->getSprite()->setColor(cocos2d::Color3B(20, 20, 20));
		}
		else {
			color = (255 * formula);
			getNPCs().at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// monster sprites
	int mx, my;
	for (int i = 0; i < getMonsters().size(); i++) {
		mx = getMonsters().at(i)->getPosX();
		my = getMonsters().at(i)->getPosY();
		dist = abs(mx - px) + abs(my - py);

		float formula = (p_cutoff - dist) / (float)p_cutoff;

		// if trap is lava or other light-emitting source, emit light from their position
		if (getMonsters().at(i)->emitsLight()) {
			lightEmitters.push_back(std::make_pair(getMonsters().at(i)->getPosX(), getMonsters().at(i)->getPosY()));

			color = std::max(140, (int)(255 * formula));
			getMonsters().at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

			// if monster uses extra sprites, update their lighting too
			if (getMonsters().at(i)->hasExtraSprites()) {
				if (getMonsters().at(i)->getName() != ZAPPER)
					getMonsters().at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));
			}

			continue;
		}
		if (dist > p_cutoff) {
			getMonsters().at(i)->getSprite()->setColor(cocos2d::Color3B(0, 0, 0));

			// if monster uses extra sprites, update their lighting too
			if (getMonsters().at(i)->hasExtraSprites()) {
				getMonsters().at(i)->setSpriteColor(cocos2d::Color3B(0, 0, 0));
			}
		}
		else {
			color = (255 * formula);
			getMonsters().at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

			// if monster uses extra sprites, update their lighting too
			if (getMonsters().at(i)->hasExtraSprites()) {
				getMonsters().at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));
			}
		}

	}

	// trap sprites
	for (int i = 0; i < getTraps().size(); i++) {
		//sx = (getTraps().at(i)->getSprite()->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		//sy = rows - ((getTraps().at(i)->getSprite()->getPosition().y + Y_OFFSET) / SPACING_FACTOR);
		sx = getTraps().at(i)->getPosX();
		sy = getTraps().at(i)->getPosY();

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		// if trap is lava or other light-emitting source, emit light from their position
		if (getTraps().at(i)->emitsLight()) {
			lightEmitters.push_back(std::make_pair(getTraps().at(i)->getPosX(), getTraps().at(i)->getPosY()));

			getTraps().at(i)->getSprite()->setColor(cocos2d::Color3B(200, 200, 200));

			// if monster uses extra sprites, update their lighting too
			if (getTraps().at(i)->hasExtraSprites()) {
				getTraps().at(i)->setSpriteColor(cocos2d::Color3B(200, 200, 200));
			}
			continue;
		}

		if (getTraps().at(i)->getItem() == LAVA) {
			int cutoff = p_cutoff + 6;
			formula = (cutoff - dist) / (float)(cutoff);
			if (dist >= cutoff) {
				getTraps().at(i)->getSprite()->setColor(cocos2d::Color3B(19, 19, 19)); // color value calculated by taking (1/cutoff) * 255
			}
			else {
				color = (255 * formula);
				getTraps().at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
			}
		}
		else {
			if (dist > p_cutoff) {
				getTraps().at(i)->getSprite()->setColor(cocos2d::Color3B(0, 0, 0));

				// if trap uses extra sprites, update their lighting too
				if (getTraps().at(i)->hasExtraSprites()) {
					getTraps().at(i)->setSpriteColor(cocos2d::Color3B(40, 40, 40));
				}
			}
			else {
				color = (255 * formula);
				getTraps().at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));

				// if trap uses extra sprites, update their lighting too
				if (getTraps().at(i)->hasExtraSprites()) {
					getTraps().at(i)->setSpriteColor(cocos2d::Color3B(color, color, color));
				}
			}
		}
	}

	// floor sprites
	for (int i = px - std::max(10, p_cutoff); i < px + std::max(10, p_cutoff) + 1; i++) {
		for (int j = py - std::max(10, p_cutoff); j < py + std::max(10, p_cutoff) + 1; j++) {
			if (j > -1 && j < rows && i > -1 && i < cols) { // boundary check
				sx = i;
				sy = j;

				dist = abs(sx - px) + abs(sy - py);
				float formula = (p_cutoff - dist) / (float)p_cutoff;

				if (dist <= p_cutoff) {
					color = (255 * formula);
					getDungeon()[j*cols + i].floor->setColor(cocos2d::Color3B(color, color, color));
				}
				else {
					getDungeon()[j*cols + i].floor->setColor(cocos2d::Color3B(0, 0, 0));
				}
			}
		}
	}

	/*
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
	*/

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

	// door sprites
	for (int i = 0; i < door_sprites.size(); i++) {
		//sx = wall_sprites.at(i)->getPosition().x;
		//sy = wall_sprites.at(i)->getPosition().y;
		sx = (door_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((door_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - px) + abs(sy - py);
		float formula = (p_cutoff - dist) / (float)p_cutoff;

		if (dist > p_cutoff) {
			door_sprites.at(i)->setColor(cocos2d::Color3B(0, 0, 0));
		}
		else {
			color = (255 * formula);
			door_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}

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


	// if there other sprites that emit light, update lighting again
	if (!lightEmitters.empty()) {
		for (int i = 0; i < lightEmitters.size(); i++) {
			updateSecondaryLighting(lightEmitters[i].first, lightEmitters[i].second);
		}
		lightEmitters.clear();
	}

}
void Dungeon::updateSecondaryLighting(int x, int y) {
	int rows = getRows();
	int cols = getCols();

	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int vision = player.at(0).getVision();

	int sx, sy, dist, color;
	int lightCutoff = 2;


	// monster sprites
	int mx, my;
	for (int i = 0; i < getMonsters().size(); i++) {
		sx = getMonsters().at(i)->getPosX();
		sy = getMonsters().at(i)->getPosY();

		dist = abs(sx - x) + abs(sy - y);
		float formula = (lightCutoff - dist) / (float)lightCutoff;

		if (dist < lightCutoff && !(abs(px - sx) < vision && abs(py - sy) < vision)) {
			color = (40);
			getMonsters().at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// trap sprites
	for (int i = 0; i < getTraps().size(); i++) {
		sx = getTraps().at(i)->getPosX();
		sy = getTraps().at(i)->getPosY();

		dist = abs(sx - x) + abs(sy - y);
		float formula = (lightCutoff - dist) / (float)lightCutoff;

		if (dist < lightCutoff && !(abs(px - sx) <= vision && abs(py - sy) <= vision)) {
			color = (40);
			getTraps().at(i)->getSprite()->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// floor sprites
	for (int i = x - lightCutoff; i < x + lightCutoff + 1; i++) {
		for (int j = y - lightCutoff; j < y + lightCutoff + 1; j++) {
			if (j != -1 && j != rows && !(i == -1 && j <= 0) && !(i == cols && j >= rows - 1)) { // boundary check
				sx = i;// (getDungeon()[j*cols + i].floor->getPosition().x + X_OFFSET) / SPACING_FACTOR;
				sy = j;// rows - ((getDungeon()[j*cols + i].floor->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

				dist = abs(sx - x) + abs(sy - y);
				float formula = (lightCutoff - dist) / (float)lightCutoff;

				if (dist < lightCutoff && !(abs(px - sx) < vision && abs(py - sy) < vision)) {
					color = (40);
					getDungeon()[j*cols + i].floor->setColor(cocos2d::Color3B(color, color, color));
				}
			}
		}
	}
	
	// wall sprites
	for (int i = 0; i < wall_sprites.size(); i++) {
		//sx = wall_sprites.at(i)->getPosition().x;
		//sy = wall_sprites.at(i)->getPosition().y;
		sx = (wall_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((wall_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - x) + abs(sy - y);
		float formula = (lightCutoff - dist) / (float)lightCutoff;

		if (dist < lightCutoff && !(abs(px - sx) <= lightCutoff && abs(py - sy) <= lightCutoff)) {
			color = (40);
			wall_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// door sprites
	for (int i = 0; i < door_sprites.size(); i++) {
		//sx = wall_sprites.at(i)->getPosition().x;
		//sy = wall_sprites.at(i)->getPosition().y;
		sx = (door_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((door_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - x) + abs(sy - y);
		float formula = (lightCutoff - dist) / (float)lightCutoff;

		if (dist < lightCutoff && !(abs(px - sx) <= lightCutoff && abs(py - sy) <= lightCutoff)) {
			color = (50);
			door_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// item sprites
	for (int i = 0; i < item_sprites.size(); i++) {
		sx = (item_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((item_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - x) + abs(sy - y);
		float formula = (lightCutoff - dist) / (float)lightCutoff;

		if (dist < lightCutoff && !(abs(px - sx) <= lightCutoff && abs(py - sy) <= lightCutoff)) {
			color = (50);
			item_sprites.at(i)->setColor(cocos2d::Color3B(color, color, color));
		}
	}

	// money sprites
	for (int i = 0; i < money_sprites.size(); i++) {
		sx = (money_sprites.at(i)->getPosition().x + X_OFFSET) / SPACING_FACTOR;
		sy = rows - ((money_sprites.at(i)->getPosition().y + Y_OFFSET) / SPACING_FACTOR);

		dist = abs(sx - x) + abs(sy - y);
		float formula = (lightCutoff - dist) / (float)lightCutoff;

		if (dist < lightCutoff && !(abs(px - sx) <= lightCutoff && abs(py - sy) <= lightCutoff)) {
			color = (50);
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
			auto spriteObject = it.first->getTextureAtlas();
			int address = (int)&spriteObject;
			if (address != 0xDDDDDDDD) {
				// create a sequence for the given sprite
				cocos2d::Sequence* seq = cocos2d::Sequence::create(it.second);

				it.first->runAction(seq);
			}
		}
		m_seq.clear();
	}
}

/// unused
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
void Dungeon::unmarkTiles() {
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 70; j++) {
			m_maze[i*MAXCOLS + j].marked = false;
		}
	}
}
void Dungeon::showText() {
	for (unsigned i = 0; i < dungeonText.size(); i++) {
		cout << dungeonText.at(i);
	}
	dungeonText.clear();
}

// Level generation
/*
std::vector<char> Dungeon::topChunks(std::vector<std::vector<std::vector<char>>> &c, int maxcols) {
	std::vector<std::vector<char>> one =
								{ {'#', '#', '#', '#'},
								  {'#', '#', '#', '#'},
								  {' ', ' ', ' ', ' '},
								  {'#', '#', '#', '#'} };

	std::vector<std::vector<char>> two =
								 { {'#', '#', '#', '#'},
								 {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', ' ', ' '} };

	std::vector<std::vector<char>> three =
						{ {' ', '#', '#', ' '},
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
*/
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


std::vector<std::string> Dungeon::topChunks(std::vector<std::vector<std::vector<std::string>>> &c) {
	
	std::vector<std::vector<std::string>> one =
	{ {wll, wll, wll, wll, wll, wll},
	  {wll, wll,  i_bc,  sp, wll, wll},
	  {  sp, m_se,  t_f,  t_p, wll,   sp},
	  {  sp, m_se,  m_z,   sp,   _dv,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll,   sp,   sp,   sp, wll, wll} };

	std::vector<std::vector<std::string>> two =
	{ {wll, t_tr,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll, wll,   sp,  PIT,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> three =
	{ {wll,   sp,   sp, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll, wll, wll,   sp, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll,   sp, t_df,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp} };

	std::vector<std::vector<std::string>> four = 
	{ {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {wll,   sp,  m_k,   sp,   sp, wll},
	  {wll,   sp,   sp,   sp,   sp, wll},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {  sp,   sp, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> five = 
	{ {wll, i_ri, wll, wll, t_td,   sp},
	  {t_tr,   sp,   sp,   sp,   sp,   sp},
	  {wll,  PIT, wll,   sp,   sp, wll},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> six = 
	{ {  sp,   sp,  t_s,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,  t_s},
	  {  sp,   sp,  t_s,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,  t_s},
	  {  sp,   sp,  t_s,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,  t_s} };

	std::vector<std::vector<std::string>> seven = 
	{ {wll, wll, PIT, wll,  i_gd, wll},
	  {t_df, wll,   sp, wll,   sp, wll},
	  {wll, wll,   sp,   sp,   sp, wll},
	  {wll,   sp,   sp, wll,  PIT, wll},
	  {wll,   sp,   sp,   sp,   sp, wll},
	  {wll,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> eight = 
	{ {t_td,   sp,   sp,   sp,   sp, t_td},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp, t_tl, wll, wll, t_tr,   sp},
	  {  sp, wll, wll, wll, wll,   sp} };

	std::vector<std::vector<std::string>> nine = 
	{ {  sp,   sp,  t_f, wll,   sp,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp, wll, t_mb},
	  {  sp, t_mb,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll,  t_f,   sp,   sp},
	  {wll,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ten = 
	{ {wll, wll,   sp, wll, wll,   sp},
	  {  sp, wll, wll,   sp, wll, wll},
	  {  sp, wll, i_gd, wll,   sp, wll},
	  {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp, wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> eleven = 
	{ {  sp,   sp, r_wl, r_wl,   sp,   sp},
	  {wll,   sp, i_gd, r_wl,   sp, r_wl},
	  {  sp,   sp, wll,   sp,  t_s,   sp},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp, r_wl},
	  {wll,   sp, wll, wll,   sp,   sp} };

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

	c = mixChunks(c);

	return combineChunks(c);
}
std::vector<std::string> Dungeon::middleChunks(std::vector<std::vector<std::vector<std::string>>> &c) {

	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp, r_wl,   sp},
	  {r_wl,   sp, r_wl,   sp, r_wl,   sp},
	  {  sp,   sp, t_ts, t_ts,   sp,   sp},
	  {  sp, r_wl, t_ts, t_ts,   sp,   sp},
	  {  sp,   sp, r_wl,   sp,   sp, r_wl},
	  {  sp, r_wl,   sp,   sp, r_wl,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp, t_mb, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp, wll, wll, wll},
	  {  sp, t_mb,   sp,   sp,   sp,   sp},
	  {wll, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp, wll, wll,   sp,   sp},
	  {wll, r_wl,   sp,   sp,   sp,   sp},
	  {  sp, r_wl,   sp,   sp,   sp,   sp},
	  {t_td,   sp, r_wl,   sp, wll,   sp},
	  {  sp, wll,   sp, wll, t_df,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {wll,   sp,   sp,   sp,   sp, wll},
	  {  sp, r_wl,   sp,   sp, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp,   sp, r_wl,   sp},
	  {wll,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp, wll,   sp, i_gd, wll,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {wll, wll,   sp, wll, wll, wll},
	  {wll,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,   sp, wll, wll, wll},
	  {wll,   sp,   sp,   sp,   sp,   sp},
	  {wll,  t_p,   sp,   sp, wll,   sp},
	  {wll, wll, wll, wll, wll, wll} };

	std::vector<std::vector<std::string>> vii =
	{ {wll,   sp, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll,   sp, wll,   sp, wll},
	  {  sp, wll,   sp,   sp,   sp, wll},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp, t_tl},
	  { t_f, wll, wll, wll,   sp, wll},
	  {wll,   sp,   sp, r_wl,   sp, wll},
	  {  sp,   sp, r_wl, r_wl,   sp, wll},
	  {t_tr,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp, r_wl, t_f,   sp, r_wl},
	  {r_wl, r_wl,   sp, r_wl,   sp,   sp},
	  {  sp, r_wl, r_wl,   sp, r_wl,   sp},
	  {  sp, r_wl,   sp, r_wl, r_wl, r_wl},
	  {  sp,   sp, r_wl, r_wl,   sp,   sp},
	  {  sp, r_wl, r_wl, t_f, r_wl, r_wl} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,  PIT,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,  t_s,   sp},
	  {  sp,   sp,  PIT,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,  t_s,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,  t_s,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp, wll,   sp,   sp, r_wl, r_wl},
	  {  sp,   sp, wll,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp, wll, i_ri, wll, wll,   sp},
	  {wll, wll, t_ts, wll, wll,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,   sp, wll, i_bc, wll,   sp},
	  {  sp, wll, m_rm,   sp, wll,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {  sp, wll,   sp, t_tu,   sp, wll},
	  {  sp,   sp, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xiii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, r_wl,   sp,   sp,   sp,   sp},
	  {  sp,   sp, r_wl, r_wl, t_mb,   sp},
	  {  sp,  t_s, r_wl, r_wl,   sp,   sp},
	  {  sp, r_wl,   sp, r_wl, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xiv =
	{ {wll, r_wl,   sp,   sp,   sp, t_td},
	  {  sp,   sp, r_wl,   sp,   sp,   sp},
	  {  sp,   sp,   sp, r_wl,   sp,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp, t_df, wll, wll, wll, t_tu} };

	std::vector<std::vector<std::string>> xv =
	{ {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll, wll,   sp,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp, wll},
	  {  sp,   sp,   sp, wll,   sp, wll},
	  {wll,   sp,   sp,   sp,   sp, t_tl},
	  {  sp,   sp,   sp,   sp, wll,   sp} };

	std::vector<std::vector<std::string>> xvi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp, wll},
	  {  sp,   sp, t_ts, t_mb,   sp, wll},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {  sp, wll,  t_s, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> xvii =
	{ {wll, r_wl,   sp, wll, wll,   sp},
	  {  sp, wll, wll, wll, r_wl, wll},
	  {wll, wll, wll,   sp, wll, wll},
	  {wll, wll, wll, wll, wll, wll},
	  {  sp, wll, i_gd, wll, r_wl, wll},
	  {wll, wll, wll, wll, wll, wll} };

	std::vector<std::vector<std::string>> xviii =
	{ {wll, r_wl, wll, wll, wll,   sp},
	  {  sp, wll, wll, wll, r_wl, wll},
	  {wll, r_wl, wll, i_gd, wll, wll},
	  {wll, wll, r_wl, wll, wll, wll},
	  {  sp, wll, wll, wll, r_wl, wll},
	  {wll, wll, wll, wll,   sp, wll} };

	std::vector<std::vector<std::string>> xix =
	{ {wll,   sp, wll, wll, wll,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp},
	  {wll, r_wl,   sp,   sp,   sp, wll},
	  {wll, wll,   sp,   sp, wll, wll},
	  {  sp,   sp,   sp,   sp, r_wl, wll},
	  {wll, wll, wll,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> xx =
	{ {wll,   sp,   sp, r_wl,   sp,   sp},
	  {wll,   sp,   sp, r_wl, wll,   sp},
	  {  sp, wll,   sp,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp, wll,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp, wll,   sp,   sp, wll, wll} };

	std::vector<std::vector<std::string>> xxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp, r_wl,   sp,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,   sp,   sp, r_wl,   sp} };

	std::vector<std::vector<std::string>> xxii = // check for infinitely looping springs
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  t_s,   sp,  t_s,   sp},
	  {  sp,  t_s,  t_s,  t_s,  t_s,   sp},
	  {  sp,  t_s,  t_s,  t_s,  t_s,   sp},
	  {  sp,   sp,  t_s,   sp,   sp,   sp},
	  {  sp,  wll,   sp,   sp, r_wl,   sp} };

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);
	//c.push_back(xxii);

	c = mixChunks(c);

	return combineChunks(c);
}
std::vector<std::string> Dungeon::bottomChunks(std::vector<std::vector<std::vector<std::string>>> &c) {
	
	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, t_s,   sp},
	  {wll, wll, wll,   sp, wll,   sp},
	  {t_tr,   sp,   sp,   sp,   sp, wll},
	  {wll, i_bc, wll, wll,   sp, r_wl} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp, wll, wll, wll,   sp, wll},
	  {  sp, wll,   sp,  m_g, wll, r_wl},
	  {  sp, wll,   sp, r_wl,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp,   sp, wll,   sp},
	  {wll,  PIT,   sp, wll, wll,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp, wll, t_s, wll,   sp},
	  {  sp, wll, wll,   sp, wll, wll},
	  {wll, wll, wll, t_s, wll, wll},
	  {wll, wll,   sp,   sp, r_wl, wll},
	  {wll,   sp,   sp,   sp,   sp, wll},
	  {wll, i_gd, i_ri, i_gd,   sp, wll} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp, r_wl,   sp,   sp},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp, wll,   sp},
	  {wll,   sp,   sp, i_ri, wll, wll},
	  { m_z,   sp, t_tu, t_tu, wll, wll} };

	std::vector<std::vector<std::string>> v =
	{ {wll,   sp,   sp,   sp,   sp, wll},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp,  t_f, wll,   sp, r_wl, r_wl},
	  {  sp, r_wl,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp,   sp,   sp, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {wll,   sp,   sp,   sp,   sp,   sp},
	  {wll,   sp, wll,   sp,   sp,   sp},
	  {wll, wll,   sp, wll,   sp,   sp},
	  {wll, r_wl, wll, wll,   sp,   sp},
	  {wll, i_ri, wll, wll,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {wll,   sp,   sp,   sp,   sp,   sp},
	  {wll,   sp, wll,   sp, wll, wll},
	  {wll, wll,   sp,   sp,   sp,   sp},
	  {wll, i_gd,   sp, r_wl,   sp, wll},
	  {  sp, wll,   sp, t_mb,   sp, r_wl},
	  {  sp,   sp,   sp,   sp,   sp, wll} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp, wll,   _dh, wll,   sp,   sp},
	  {wll, wll,   sp, wll, wll,   sp},
	  {wll, wll,   sp, wll, wll, r_wl},
	  {wll,   sp,   sp,   sp, wll, wll},
	  {wll, wll,   sp, m_rm, m_rm, wll},
	  {wll, wll, m_rm, m_rm, m_rm, wll} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp, wll, wll, wll,   sp,   sp},
	  {  sp, t_ts, t_ts, wll,   sp,   sp},
	  {  sp, t_ts, t_ts, wll, wll,   sp},
	  {wll, wll, t_ts, wll, t_ts,   sp},
	  {wll,   sp, t_ts, t_ts, t_ts, wll},
	  {wll, t_ts, t_ts,   sp, t_ts, wll} };

	std::vector<std::vector<std::string>> x =
	{ {r_wl,   sp,   sp,   sp, wll,   sp},
	  {  sp,   sp, r_wl,   sp, t_mb, wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   sp,  t_s, wll, wll,   sp},
	  {wll,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp, wll,   sp,   sp,   sp},
	  {  sp, t_mb, wll, wll, t_mb,   sp},
	  {  sp,   sp, wll, wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl, wll, wll, wll, wll, r_wl} };

	std::vector<std::vector<std::string>> xii =
	{ {wll,   sp,   sp,   sp,   sp, r_wl},
	  {  sp, wll,   sp,   sp,   sp,   sp},
	  {  sp, i_gd, wll, t_f,   sp,   sp},
	  {  sp, wll, wll, wll,   sp,   sp},
	  {  sp,   sp,   sp, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xiii =
	{ {wll, wll, wll,   sp, wll,   sp},
	  {  sp,   sp, t_mb,   sp,   sp,   sp},
	  {  sp,   sp,   sp, t_mb,   sp,   sp},
	  {  sp,   sp,   sp,   sp, r_wl,   sp},
	  {  sp, wll,   sp, wll, wll,   sp},
	  {  sp,   sp, t_mb,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xiv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {wll, wll, m_g,  m_g, wll, wll},
	  {wll, wll, i_rc, i_sc, wll, wll} };

	std::vector<std::vector<std::string>> xv =
	{ {  sp,   sp, wll, wll,   _dh, wll},
	  {  sp, wll, m_rm, wll,   sp, wll},
	  {  sp, wll,   sp, wll,   sp,   sp},
	  {  sp,   _dv,   sp, wll, r_wl, wll},
	  {  sp, wll,   sp,   _dv,   sp,   sp},
	  { t_s, wll, wll, wll, wll,  t_f} };

	std::vector<std::vector<std::string>> xvi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xvii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xviii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xx =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xxi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };


	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	/*c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);*/

	c = mixChunks(c);

	return combineChunks(c);
}
std::vector<std::vector<std::vector<std::string>>> Dungeon::mixChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	int cols = getCols();

	std::string x = SPAWN_DISALLOWED;
	std::string play = PLAYER;
	std::string exit = STAIRCASE;


	std::vector<std::vector<std::vector<std::string>>> v;

	std::vector<std::vector<std::string>> pChunk =
	{ {   x,    x, wll, wll, r_wl,    x},
	  {wll,    x,    x,    x,    x,    x},
	  {   x,    x, play,    x,    x, wll},
	  {wll,    x,    x,    x,    x, wll},
	  {r_wl,    x,    x,    x,    x, wll},
	  {   x, wll, wll,    x, wll,    x} };

	std::vector<std::vector<std::string>> eChunk =
	{ {   x,    x, r_wl, r_wl, r_wl,    x},
	  {r_wl, m_rm, m_rm, m_rm,    x,    x},
	  {   x, m_rm, exit, m_rm,    x, r_wl},
	  { wll, m_rm, m_rm, m_rm, r_wl,  wll},
	  {r_wl,    x,    x,    x,    x,  wll},
	  {   x,    x, r_wl,    x,   wll,   x} };

	std::vector<std::vector<std::string>> specialRoom1 =
	{ {  sp, wll, wll, wll, wll,   sp},
	  {wll, wll, wll, wll, wll, wll},
	  {wll, wll,  m_g,   sp, wll, wll},
	  {wll, wll, i_sc, i_sc, wll, wll},
	  {wll, wll, wll, wll, wll, wll},
	  {  sp, wll, wll, wll, wll,   sp}, };

	std::vector<std::vector<std::string>> specialRoom2 =
	{ {  sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp, wll,  m_w,  m_w, wll,   sp},
	  {  sp, wll,  m_w,  m_w, wll,   sp},
	  {  sp, wll, wll, wll, wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp} };

	int s;

	for (int i = 0; i < (cols - 2) / 6 /*number_of_columns - 2 / length_of_each_chunk*/; i++) {
		if ((layer - 1) * 6 + i == playerChunk)
			v.push_back(pChunk);
		else if ((layer - 1) * 6 + i == exitChunk)
			v.push_back(eChunk);
		else if ((layer - 1) * 6 + i == specialChunk1)
			v.push_back(specialRoom1);
		else if ((layer - 1) * 6 + i == specialChunk2)
			v.push_back(specialRoom2);
		else
			v.push_back(c[randInt(c.size())]);
	}

	/*if (layer == specialChunkLayer1) {
		s = randInt(v.size());
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom1);
	}
	if (layer == specialChunkLayer2) {
		s = randInt(v.size());
		v.erase(v.begin() + s);
		v.emplace(v.begin() + s, specialroom2);
	}*/

	//// put in player chunk
	//v.erase(v.begin() + playerChunk);
	//v.emplace(v.begin() + playerChunk, pChunk);

	return v;
}
std::vector<std::string> Dungeon::combineChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	std::vector<std::string> v;
	for (int i = 0; i < 6; i++) {			// iterate thru rows of 2d vector
		for (int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 6; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}

std::vector<std::string> Dungeon::generateLevel() {
	int rows = getRows();
	int cols = getCols();

	std::vector<std::vector<std::vector<std::string>>> b;
	std::vector<std::string> v, finalvec;


	// player spawns in middle with special chunk
	int n = 1 + randInt(4);
	switch (n) {
	case 1: playerChunk = 14; break;
	case 2: playerChunk = 15; break;
	case 3: playerChunk = 20; break;
	case 4: playerChunk = 21; break;
	}

	// exit spawns in one of four corners
	n = 1 + randInt(4);
	switch (n) {
	case 1: exitChunk = 0; break;
	case 2: exitChunk = 5; break;
	case 3: exitChunk = 30; break;
	case 4: exitChunk = 35; break;
	}

	// chooses layer for the special chunks to be on
	specialChunk1 = randInt(36);
	while (specialChunk1 == playerChunk || specialChunk1 == exitChunk) {
		specialChunk1 = randInt(36);
	}

	specialChunk2 = randInt(36);
	while (specialChunk2 == playerChunk || specialChunk2 == exitChunk) {
		specialChunk2 = randInt(36);
	}

	specialChunkLayer1 = 1 + randInt(6);
	specialChunkLayer2 = 1 + randInt(6);
	layer = 1;

	int count = 0;
	while (count < (rows - 2) / 6 /* divided by length of chunk*/) {
		if (count == 0) {
			v = topChunks(b);
			layer++;
		}
		else if (count >= 1 && count <= 4) {
			v = middleChunks(b);
			layer++;
		}
		else if (count == 5) {
			v = bottomChunks(b);
			layer++;
		}

		for (int i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i));

		b.clear();
		count++;
	}

	return finalvec;
}
void Dungeon::fillLevel(std::vector<std::string> finalvec, Player p, int start, int end) {
	int rows = getRows();
	int cols = getCols();

	int count = 0;
	char object;
	std::shared_ptr<Monster> monster(nullptr);

	for (int i = start; i < rows - 1; i++) {
		for (int j = 1; j < cols + end; j++) {

			if (finalvec.at(count) == RANDOM_WALL) {
				// 50% chance to spawn wall
				if (randInt(2) == 0) {
					getDungeon()[i*cols + j].top = WALL;
					getDungeon()[i*cols + j].wall = true;
				}
			}
			else if (finalvec.at(count) == REG_WALL) {
				getDungeon()[i*cols + j].top = WALL;
				getDungeon()[i*cols + j].wall = true;
			}
			else if (finalvec.at(count) == UNB_WALL) {
				getDungeon()[i*cols + j].top = UNBREAKABLE_WALL;
				getDungeon()[i*cols + j].wall = true;
				getDungeon()[i*cols + j].boundary = true;
			}
			else if (finalvec.at(count) == DOOR_HORIZONTAL) {
				getDungeon()[i*cols + j].top = getDungeon()[i*cols + j].traptile = DOOR_H;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Door> door = std::make_shared<Door>(j, i);
				getDoors().emplace_back(door);
			}
			else if (finalvec.at(count) == DOOR_VERTICAL) {
				getDungeon()[i*cols + j].top = getDungeon()[i*cols + j].traptile = DOOR_V;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Door> door = std::make_shared<Door>(j, i);
				getDoors().emplace_back(door);
			}
			else if (finalvec.at(count) == SPAWN_DISALLOWED) {
				getDungeon()[i*cols + j].noSpawn = true;
			}
			else if (finalvec.at(count) == STAIRCASE) {
				std::shared_ptr<Traps> stairs = std::make_shared<Stairs>(j, i);
				getDungeon()[i*cols + j].trap_name = STAIRCASE;
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].exit = true;
				getTraps().emplace_back(stairs);
			}
			else if (finalvec.at(count) == LOCKED_STAIRCASE) {
				std::shared_ptr<Traps> stairs = std::make_shared<Stairs>(j, i);
				getDungeon()[i*cols + j].trap_name = LOCKED_STAIRCASE;
				getDungeon()[i*cols + j].trap = true;
				
				getTraps().emplace_back(stairs);
			}
			else if (finalvec.at(count) == DEVILS_WATER) {
				std::shared_ptr<Traps> devils_water = std::make_shared<Traps>(j, i, DEVILS_WATER, "Water_Tile1_48x48.png", 0, false);
				getTraps().emplace_back(devils_water);
				devils_water.reset();

				getDungeon()[i*cols + j].trap_name = finalvec.at(count);
				getDungeon()[i*cols + j].trap = true;
			}
			else if (finalvec.at(count) == FOUNTAIN) {
				getDungeon()[i*cols + j].top = c_FOUNTAIN;
				getDungeon()[i*cols + j].wall = true;
			}

			else if (finalvec.at(count) == PLAYER) {
				
				if (m_level != 1) {
					player.at(0) = p;
					player.at(0).setPosX(j);
					player.at(0).setPosY(i);
					//player.at(0).addItem(std::make_shared<Bomb>());
					//player.at(0).equipShield(*this, std::make_shared<WoodShield>());
					getDungeon()[i*cols + j].hero = true;
					count++;
					continue;
				}
				
				getDungeon()[i*cols + j].hero = true;

				Player p;
				player.push_back(p);
				player.at(0).setPosX(j); player.at(0).setPosY(i);

				player.at(0).addItem(std::make_shared<Bomb>());
				//player.at(0).addItem(std::make_shared<StatPotion>());
				//player.at(0).addItem(std::make_shared<PoisonCloud>());
				//player.at(0).addItem(std::make_shared<Teleport>());
				//player.at(0).addItem(std::make_shared<DizzyElixir>());
				//player.at(0).addItem(std::make_shared<EtherealSpell>());
				//player.at(0).addItem(std::make_shared<EtherealSpell>());
				//player.at(0).addWeapon(std::make_shared<ArcaneStaff>());
				//player.at(0).addWeapon(std::make_shared<VulcanHammer>());
				//player.at(0).addWeapon(std::make_shared<IronLongSword>());
				player.at(0).equipShield(*this, std::make_shared<WoodShield>());
				//player.at(0).equipShield(*this, std::make_shared<ReflectShield>());
				//player.at(0).equipTrinket(*this, std::make_shared<VulcanRune>());
			}
			else if (finalvec.at(count) == GOBLIN) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<Goblin>(j, i, 12);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == SEEKER) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<Seeker>(j, i, 10);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == DEAD_SEEKER) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<DeadSeeker>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == ITEM_THIEF) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<ItemThief>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == ARCHER) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<Archer>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == WANDERER) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<Wanderer>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == ZAPPER) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<Zapper>(j, i, rows);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == SPINNER) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<Spinner>(j, i, rows);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == GOO_SACK) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<GooSack>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == BOMBEE) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<Bombee>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == ROUNDABOUT) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<Roundabout>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == FIRE_ROUNDABOUT) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<FireRoundabout>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == BROUNDABOUT) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<Broundabout>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == MOUNTED_KNIGHT) {
				getDungeon()[i*cols + j].enemy = true;

				monster = std::make_shared<MountedKnight>(j, i);
				getMonsters().emplace_back(monster);
				getDungeon()[i*cols + j].monster_name = monster->getName();
			}
			else if (finalvec.at(count) == RANDOM_MONSTER) {
			
				// the type of enemy
				switch (randInt(8)) {
				case 0: monster = std::make_shared<Goblin>(j, i, 10); break;
				case 1:	monster = std::make_shared<Wanderer>(j, i);	break;
				case 2:	monster = std::make_shared<Archer>(j, i); break;
				case 3:	monster = std::make_shared<Zapper>(j, i, rows); break;
				case 4:	monster = std::make_shared<Spinner>(j, i, rows);break;
				case 5:	monster = std::make_shared<Roundabout>(j, i);	break;
				case 6:	monster = std::make_shared<MountedKnight>(j, i);	break;
				case 7:	monster = std::make_shared<Seeker>(j, i, 10);	break;
				//case 5:	monster = std::make_shared<Bombee>(j, i);	break;
				}

				getMonsters().emplace_back(monster);

				getDungeon()[i*cols + j].monster_name = monster->getName();
				getDungeon()[i*cols + j].enemy = true;
			}

			else if (finalvec.at(count) == PIT) {
				getDungeon()[i*cols + j].trap = true;

				std::shared_ptr<Traps> trap = std::make_shared<Pit>(j, i);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == FIREBAR) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Traps> trap = std::make_shared<Firebar>(j, i, rows);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == DOUBLE_FIREBAR) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Traps> trap = std::make_shared<DoubleFirebar>(j, i, rows);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == SPIKES) {
				getDungeon()[i*cols + j].trap = true;

				std::shared_ptr<Traps> trap = std::make_shared<Spikes>(j, i);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == AUTOSPIKE_DEACTIVE) {
				getDungeon()[i*cols + j].trap = true;

				std::shared_ptr<Traps> trap = std::make_shared<SpikeTrap>(j, i, 4);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == TRIGGERSPIKE_DEACTIVE) {
				getDungeon()[i*cols + j].trap = true;

				std::shared_ptr<Traps> trap = std::make_shared<TriggerSpike>(j, i);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == PUDDLE) {
				getDungeon()[i*cols + j].trap = true;

				std::shared_ptr<Traps> trap = std::make_shared<Puddle>(j, i);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == SPRING) {
				getDungeon()[i*cols + j].trap = true;

				std::shared_ptr<Traps> trap = std::make_shared<Spring>(j, i, false);

				// Begin check for if any spring traps are pointing toward each other
				std::shared_ptr<Spring> thisSpring = std::dynamic_pointer_cast<Spring>(trap);
				thisSpring->oppositeSprings(*this);


				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == LEFT_TURRET) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Traps> trap = std::make_shared<Turret>(j, i, 'l');
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == RIGHT_TURRET) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Traps> trap = std::make_shared<Turret>(j, i, 'r');
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == UP_TURRET) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Traps> trap = std::make_shared<Turret>(j, i, 'u');
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == DOWN_TURRET) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Traps> trap = std::make_shared<Turret>(j, i, 'd');
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == TURRET) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				char move;
				switch (1 + randInt(4)) {
				case 1: move = 'l'; break;	// L
				case 2: move = 'r'; break;	// R
				case 3: move = 'u'; break;	// U
				case 4: move = 'd'; break;	// D
				}

				std::shared_ptr<Traps> trap = std::make_shared<Turret>(j, i, move);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == MOVING_BLOCK) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Traps> trap = std::make_shared<MovingBlock>(j, i, (randInt(2) == 0 ? 'h' : 'v'));
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == MOVING_BLOCK_H) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Traps> trap = std::make_shared<MovingBlock>(j, i, 'h');
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == MOVING_BLOCK_V) {
				getDungeon()[i*cols + j].trap = true;
				getDungeon()[i*cols + j].wall = true;

				std::shared_ptr<Traps> trap = std::make_shared<MovingBlock>(j, i, 'v');
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == CRUMBLE_FLOOR) {
				getDungeon()[i*cols + j].trap = true;

				std::shared_ptr<Traps> trap = std::make_shared<CrumbleFloor>(j, i);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == CRUMBLE_LAVA) {
				getDungeon()[i*cols + j].trap = true;

				std::shared_ptr<Traps> trap = std::make_shared<CrumbleLava>(j, i);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}
			else if (finalvec.at(count) == LAVA) {
				getDungeon()[i*cols + j].trap = true;

				std::shared_ptr<Traps> trap = std::make_shared<Lava>(j, i);
				getTraps().emplace_back(trap);
				getDungeon()[i*cols + j].trap_name = trap->getItem();
			}

			else if (finalvec.at(count) == GOLD) {
				getDungeon()[i*cols + j].gold = 4 + randInt(9);
			}
			else if (finalvec.at(count) == HEART_POD) {
				std::shared_ptr<Objects> objects = std::make_shared<HeartPod>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == LIFEPOT) {
				std::shared_ptr<Objects> objects = std::make_shared<LifePotion>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == BIG_LIFEPOT) {
				std::shared_ptr<Objects> objects = std::make_shared<BigLifePotion>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == ARMOR) {
				std::shared_ptr<Objects> objects = std::make_shared<ArmorDrop>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == STATPOT) {
				std::shared_ptr<Objects> objects = std::make_shared<StatPotion>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == BOMB) {
				std::shared_ptr<Objects> objects = std::make_shared<Bomb>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == BROWN_CHEST) {
				std::shared_ptr<Objects> objects = std::make_shared<BrownChest>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == BROWN_CHEST) {
				std::shared_ptr<Objects> objects = std::make_shared<BrownChest>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == SILVER_CHEST) {
				std::shared_ptr<Objects> objects = std::make_shared<SilverChest>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == GOLDEN_CHEST) {
				std::shared_ptr<Objects> objects = std::make_shared<GoldenChest>();
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == RANDOM_CHEST) {
				std::shared_ptr<Objects> objects(nullptr);
				switch (randInt(3)) {
				case 0: objects = std::make_shared<BrownChest>(); break;
				case 1: objects = std::make_shared<SilverChest>(); break;
				case 2: objects = std::make_shared<GoldenChest>(); break;
				}
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == RANDOM_ITEM) {
				std::shared_ptr<Objects> objects(nullptr);
				switch (randInt(4)) {
				case 0: objects = std::make_shared<LifePotion>(); break;
				case 1: objects = std::make_shared<StatPotion>(); break;
				case 2: objects = std::make_shared<Bomb>(); break;
				case 3: objects = std::make_shared<ShieldRepair>(); break;
				}
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == RANDOM_WEAPON) {
				std::shared_ptr<Objects> objects(nullptr);
				switch (randInt(4)) {
				case 0: objects = std::make_shared<RustyCutlass>(); break;
				case 1: objects = std::make_shared<GoldenShortSword>(); break;
				case 2: objects = std::make_shared<Katana>(); break;
				case 3: objects = std::make_shared<RustyCutlass>(); break;
				}
				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == RANDOM_SHIELD) {
				std::shared_ptr<Objects> objects(nullptr);
				int n = 1 + randInt(100);
				if (n >= 1 && n < 50) objects = std::make_shared<WoodShield>();
				else if (n >= 50 && n < 70) objects = std::make_shared<IronShield>();
				else if (n >= 70 && n < 90) objects = std::make_shared<ReflectShield>();
				else if (n >= 90 && n <= 100) objects = std::make_shared<ThornedShield>();

				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}
			else if (finalvec.at(count) == RANDOM_TRINKET) {
				std::shared_ptr<Objects> objects(nullptr);
				int n = 1 + randInt(100);
				if (n >= 1 && n < 25) objects = std::make_shared<DarkStar>();
				else if (n >= 25 && n < 50) objects = std::make_shared<BrightStar>();
				else if (n >= 50 && n < 75) objects = std::make_shared<GoldPot>();
				else if (n >= 75 && n <= 100) objects = std::make_shared<LuckyPig>();

				getDungeon()[i*cols + j].object = objects;
				getDungeon()[i*cols + j].item = true;
			}

			count++;
		}
	}

	/*
	bool _trap, wall, noSpawn;
	int x = 1 + randInt(cols - 2);
	int y = 1 + randInt(rows - 2);

	noSpawn = m_maze[y*cols + x].noSpawn;
	_trap = m_maze[y*cols + x].trap;
	wall = m_maze[y*cols + x].wall;

	///	while stairs do not spawn on the side opposite from the player
	while (noSpawn || _trap || wall || (x >= 5 && x <= cols - 5 && !(y < 5 || y > rows - 5)) || (y >= 5 && y <= rows - 5 && !(x < 5 || x > cols - 5))){// ||
		//(x < 5 && (y > 5 || y < rows - 5)) || (x > cols - 5 && (y > 5 || y < rows - 5)) || (y < 5 && (x > 5 || x < cols - 5)) || (y > cols - 5 && (x > 5 || x < cols - 5))) {
	//while (_trap || wall || ((player.at(0).getPosX() < 5 && stairs->getPosX() >= 5) || (player.at(0).getPosX() > cols - 6 && stairs->getPosX() < cols - 6))) {
		x = 1 + randInt(cols - 2);
		y = 1 + randInt(rows - 2);

		noSpawn = m_maze[y*cols + x].noSpawn;
		_trap = m_maze[y*cols + x].trap;
		wall = m_maze[y*cols + x].wall;
	}
	std::shared_ptr<Traps> stairs = std::make_shared<Stairs>();
	m_maze[stairs->getPosY()*cols + stairs->getPosX()].exit = true;
	m_maze[stairs->getPosY()*cols + stairs->getPosX()].traptile = STAIRS;
	m_maze[stairs->getPosY()*cols + stairs->getPosX()].trap_name = STAIRCASE;
	m_maze[stairs->getPosY()*cols + stairs->getPosX()].trap = true;
	m_traps.emplace_back(stairs);*/
}


// ===========================================
//				:::: SHOP ::::
// ===========================================
Shop::Shop(Player p, int level) : Dungeon(level) {

	_Tile *tile;

	// initialize tiles
	for (int i = 0; i < m_rows; i++) {
		for (int j = 0; j < m_cols; j++) {
			tile = new _Tile;
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = SPACE;
			tile->npc = tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = tile->exit = false;
			tile->price = tile->gold = 0;
			tile->marked = false;
			tile->object = nullptr;
			tile->item_name = tile->shop_action = "";

			m_shop1.push_back(*tile);
			delete tile;
		}
	}

	// price adjustment multiplier
	switch (level) {
	case 2: m_priceMultiplier = 1.0f; break;
	case 4: m_priceMultiplier = 2.2f; break;
	case 6: m_priceMultiplier = 3.6f; break;
	default: m_priceMultiplier = 1.0f;
	}
	

	// get the shop layout
	std::vector<char> v = generate();
	int rows = getRows();
	int cols = getCols();
	char top, bottom, traptile;

	// begin mapping shop
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			switch (v.at(i*cols + j)) {
			case c_PLAYER: {
				m_shop1[i*cols + j].hero = true;
				player.at(0) = p;
				player.at(0).setPosX(j);
				player.at(0).setPosY(i);
				//player.at(0).setMoney(1000);
				break;
			}
			case '#': {
				m_shop1[i*cols + j].top = m_shop1[i*cols + j].bottom = m_shop1[i*cols + j].traptile = UNBREAKABLE_WALL;
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].boundary = true;
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
				m_doors.emplace_back(new Door(door));
				break;
			}
			case STAIRS: {
				std::shared_ptr<Traps> stairs = std::make_shared<Stairs>(j, i);
				getTraps().emplace_back(stairs);
				m_shop1[i*cols + j].top = m_shop1[i*cols + j].bottom = SPACE;
				m_shop1[i*cols + j].exit = true;
				m_shop1[i*cols + j].trap_name = STAIRCASE;
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
				m_shop1[i*cols + j].boundary = true;
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
				case 0: m_shop1[i*cols + j].object = std::make_shared<LifePotion>(); m_shop1[i*cols + j].item_name = LIFEPOT; m_shop1[i*cols + j].price = 30 * m_priceMultiplier; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<ShieldRepair>(); m_shop1[i*cols + j].item_name = SHIELD_REPAIR; m_shop1[i*cols + j].price = 25 * m_priceMultiplier; break;
				}
				m_shop1[i*cols + j].shop_action = "shop_item";
				m_shop1[i*cols + j].boundary = true;
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				break;
			}
			case '2': {
				switch (randInt(3)) {
				case 0: m_shop1[i*cols + j].object = std::make_shared<ShieldRepair>(); m_shop1[i*cols + j].item_name = SHIELD_REPAIR; m_shop1[i*cols + j].price = 25 * m_priceMultiplier; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<Bomb>(); m_shop1[i*cols + j].item_name = BOMB; m_shop1[i*cols + j].price = 25 * m_priceMultiplier; break;
				case 2: m_shop1[i*cols + j].object = std::make_shared<StatPotion>(); m_shop1[i*cols + j].item_name = STATPOT; m_shop1[i*cols + j].price = 23 * m_priceMultiplier; break;
				}
				m_shop1[i*cols + j].boundary = true;
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
			case '3': {
				switch (randInt(2)) {
				case 0: m_shop1[i*cols + j].object = std::make_shared<LifePotion>(); m_shop1[i*cols + j].item_name = LIFEPOT; m_shop1[i*cols + j].price = 30 * m_priceMultiplier; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<Bomb>(); m_shop1[i*cols + j].item_name = BOMB; m_shop1[i*cols + j].price = 25 * m_priceMultiplier; break;
				}
				m_shop1[i*cols + j].boundary = true;
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
					  // 4, 5 : random items
			case '4': {
				switch (randInt(8)) {
				case 0: m_shop1[i*cols + j].object = std::make_shared<IronLongSword>(); m_shop1[i*cols + j].price = (100 + randInt(6)) * m_priceMultiplier; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<IronLance>(); m_shop1[i*cols + j].price = (80 + randInt(5)) * m_priceMultiplier; break;
				case 2: m_shop1[i*cols + j].object = std::make_shared<ArcaneStaff>(); m_shop1[i*cols + j].price = (55 + randInt(5)) * m_priceMultiplier; break;
				case 3: m_shop1[i*cols + j].object = std::make_shared<Katana>(); m_shop1[i*cols + j].price = (85 + randInt(4)) * m_priceMultiplier; break;
				case 4: m_shop1[i*cols + j].object = std::make_shared<WoodBow>(); m_shop1[i*cols + j].price = (120 + randInt(11)) * m_priceMultiplier; break;
				case 5: m_shop1[i*cols + j].object = std::make_shared<WoodShield>(j, i); m_shop1[i*cols + j].price = 38 * m_priceMultiplier; break;
				case 6: m_shop1[i*cols + j].object = std::make_shared<IronShield>(j, i); m_shop1[i*cols + j].price = 65 * m_priceMultiplier; break;
				case 7: m_shop1[i*cols + j].object = std::make_shared<BloodShortSword>(); m_shop1[i*cols + j].price = (70 + randInt(8)) * m_priceMultiplier; break;
				}
				m_shop1[i*cols + j].boundary = true;
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].item_name = m_shop1[i*cols + j].object->getItem();
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
			case '5': {
				switch (randInt(8)) {
				case 0: m_shop1[i*cols + j].object = std::make_shared<BigLifePotion>(); m_shop1[i*cols + j].price = (30) * m_priceMultiplier; break;
				case 1: m_shop1[i*cols + j].object = std::make_shared<FireBlastSpell>(); m_shop1[i*cols + j].price = (39 + randInt(3)) * m_priceMultiplier; break;
				case 2: m_shop1[i*cols + j].object = std::make_shared<Teleport>(); m_shop1[i*cols + j].price = (40 + randInt(3)) * m_priceMultiplier; break;
				case 3: m_shop1[i*cols + j].object = std::make_shared<ThornedShield>(); m_shop1[i*cols + j].price = (85 + randInt(9)) * m_priceMultiplier; break;
				case 4: m_shop1[i*cols + j].object = std::make_shared<DizzyElixir>(); m_shop1[i*cols + j].price = 25 * m_priceMultiplier; break;
				case 5: m_shop1[i*cols + j].object = std::make_shared<Bloodrite>(); m_shop1[i*cols + j].price = (68 + randInt(5)) * m_priceMultiplier; break;
				case 6: m_shop1[i*cols + j].object = std::make_shared<VulcanRune>(); m_shop1[i*cols + j].price = (68 + randInt(5)) * m_priceMultiplier; break;
				case 7: m_shop1[i*cols + j].object = std::make_shared<EtherealSpell>(); m_shop1[i*cols + j].price = (54) * m_priceMultiplier; break;
				}
				m_shop1[i*cols + j].boundary = true;
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].item_name = m_shop1[i*cols + j].object->getItem();
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
					  // 6 : random chest
			case '6': {
				int n = 1 + randInt(100);
				
				// 60% chance
				if (n <= 60) { m_shop1[i*cols + j].object = std::make_shared<BrownChest>(); m_shop1[i*cols + j].item_name = BROWN_CHEST; m_shop1[i*cols + j].price = 25 * m_priceMultiplier; }
				// 35% chance
				else if (n > 60 && n <= 95) { m_shop1[i*cols + j].object = std::make_shared<SilverChest>(); m_shop1[i*cols + j].item_name = SILVER_CHEST; m_shop1[i*cols + j].price = 45 * m_priceMultiplier; }
				// 5% chance
				else if (n > 95) { m_shop1[i*cols + j].object = std::make_shared<GoldenChest>(); m_shop1[i*cols + j].item_name = GOLDEN_CHEST; m_shop1[i*cols + j].price = 100 * m_priceMultiplier; }

				m_shop1[i*cols + j].boundary = true;
				m_shop1[i*cols + j].wall = true;
				m_shop1[i*cols + j].item = true;
				m_shop1[i*cols + j].shop_action = "shop_item";
				break;
			}
			}
		}
	}
}
Shop& Shop::operator=(Shop const &dungeon) {
	m_shop1 = dungeon.m_shop1;
	player = dungeon.player;
	m_level = dungeon.m_level;

	m_npcs = dungeon.m_npcs;
	m_monsters = dungeon.m_monsters;
	//m_shopMonsters = dungeon.m_shopMonsters;
	//m_shopActives = dungeon.m_shopActives;
	m_doors = dungeon.m_doors;
	m_traps = dungeon.m_traps;

	return *this;
}

void Shop::peekDungeon(int x, int y, char move) {
	char top, projectile, bottom, traptile;
	bool boundary, wall, item, trap, enemy;

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

	// set player's action key (for items that have casting time)
	player.at(0).setAction(move);

	// red tint if player has bloodlust
	if (player.at(0).hasBloodlust()) {
		bloodlustTint(player.at(0));
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

	// :::: Move player ::::
	int weprange = player.at(0).getWeapon()->getRange();
	bool moveUsed = false;
	bool far_enemy, close_enemy;

	// check for special weapon attack patterns
	if (player.at(0).getWeapon()->hasAttackPattern()) {
		player.at(0).getWeapon()->usePattern(*this, moveUsed);
	}

	// otherwise, check the action taken
	if (!moveUsed) {

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

			top = m_shop1[(y + m)*cols + (x + n)].top;
			bottom = m_shop1[(y + m)*cols + (x + n)].bottom;
			traptile = m_shop1[(y + m)*cols + (x + n)].traptile;

			boundary = m_shop1[(y + m)*cols + (x + n)].boundary;
			wall = (player.at(0).isEthereal() ? false : m_shop1[(y + m)*cols + (x + n)].wall);
			item = m_shop1[(y + m)*cols + (x + n)].item;
			trap = m_shop1[(y + m)*cols + (x + n)].trap;
			enemy = m_shop1[(y + m)*cols + (x + n)].enemy;

			if (!(wall || enemy || boundary)) {
				// move character to the left
				m_shop1[y*cols + x].hero = false;
				m_shop1[(y + m)*cols + (x + n)].hero = true;
				player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);

				// pick up any gold/money that was on the ground
				if (m_shop1[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
					goldPickup(m_shop1, player.at(0).getPosX(), player.at(0).getPosY());
				}

				if (trap) {
					; // check for traps
				}
				if (item) {
					foundItem(m_shop1, x + n, y + m);
				}
				if (m_shop1[(y + m)*cols + (x + n)].shop_action == PURCHASE) {
					// if there's a price already floating, remove before showing the new one
					if (scene->itemprice != nullptr)
						scene->deconstructShopHUD();

					scene->showShopHUD(*this, x + n, y + m);
				}
				else if (scene->itemprice != nullptr && m_shop1[(y + m)*cols + (x + n)].shop_action != PURCHASE) {
					scene->deconstructShopHUD();
				}
			}
			else if (wall && !boundary) {
				if (top == DOOR_H || top == DOOR_V) {
					openDoor(x + n, y + m);
				}
				else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
					m_shop1[y*cols + x].hero = false;
					m_shop1[(y + m)*cols + (x + n)].hero = true;
					player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);
				}
				else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
					// play locked door sound
					cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
				}
			}
			else if (enemy) {	// monster encountered
				fight(x + n, y + m);
			}
		}

		else if (move == 'e') {
			// do this if item needs to be purchased
			if (shop_action == "purchase") {
				purchaseItem(m_shop1, x, y);
			}
			// else if the item purchased was a chest, do this
			else if (shop_action == "chest purchased") {
				//m_shop1[y*cols + x].item_name = "";
				//removeSprite(item_sprites, rows, x, y - 1);
				collectItem(*this, x, y);
			}
			// else if there's just an item sitting on the ground, do this
			else {
				collectItem(*this, x, y);
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
	queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY());

	// pick up any gold/money that was on the ground
	if (m_shop1[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
		goldPickup(m_shop1, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// check active items in play
	checkActive();

	// if there are any doors, check them
	if (!getDoors().empty()) {
		checkDoors();
	}

	//	if player is dead then return
	if (player.at(0).getHP() <= 0) {
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

		player.at(0).decreaseMoneyBonus();
	}
}

void Shop::purchaseItem(std::vector<_Tile> &dungeon, int x, int y) {
	int cols = m_cols;
	int rows = m_rows;
	std::string item = dungeon[(y-1)*cols + x].item_name;
	std::shared_ptr<Objects> object = dungeon[(y - 1)*cols + x].object;
	int playerGold = player.at(0).getMoney();
	int price = dungeon[(y-1)*cols + x].price;

	int items = player.at(0).getItems().size();
	int maxItems = player.at(0).getMaxItemInvSize();
	int weapons = player.at(0).getWeapons().size();
	int maxWeapons = player.at(0).getMaxWeaponInvSize();

	// if player has enough money, let them buy it
	if (playerGold >= price && (!object->isItem() || (object->isItem() && items < maxItems) || (object->isWeapon() && weapons < maxWeapons))) {
		// purchase sound
		cocos2d::experimental::AudioEngine::play2d("Purchase_Item1.mp3", false, 1.0f);

		player.at(0).setMoney(playerGold - price);
		dungeon[y*cols + x].shop_action = ""; // removes purchase ability
		collectItem(*this, x, y - 1);

		// remove HUD after purchasing
		auto scene = dynamic_cast<Shop1Scene*>(m_scene);
		scene->deconstructShopHUD();
	}
	// else play insufficient funds sound
	else {
		cocos2d::experimental::AudioEngine::play2d("Insufficient_Funds.mp3", false, 1.0f);
	}
}
void Shop::collectItem(Dungeon &dungeon, int x, int y) {
	// :::: If coming from purchase item, y is equal to py-1 where py is the player's y position ::::

	// :::: If collectItem is called directly, then there is no y shift ::::

	int maxrows = getRows();
	int maxcols = getCols();

	int current_inv_size = player.at(0).getWeapons().size() + player.at(0).getItems().size();
	int current_wep_inv_size = player.at(0).getWeapons().size();
	int current_item_inv_size = player.at(0).getItems().size();
	int max_wep_inv_size = player.at(0).getMaxWeaponInvSize();
	int max_item_inv_size = player.at(0).getMaxItemInvSize();
	std::string image;

	char bottom = dungeon[y*maxcols + x].bottom;
	char traptile = dungeon[y*maxcols + x].traptile;
	bool exit = dungeon[y*maxcols + x].exit;
	
	std::string item_name = dungeon[y*maxcols + x].item_name;
	std::string shop_action = dungeon[y*maxcols + x].shop_action;
	std::shared_ptr<Objects> object = dungeon[y*maxcols + x].object;

	// if it's the stairs, then advance level and return
	if (exit) {
		setLevel(getLevel() + 1);
		return;
	}

	// if object is null, then do nothing and return
	if (object == nullptr) {
		cocos2d::experimental::AudioEngine::play2d("Player_Movement.mp3", false, 1.0f);

		return;
	}

	if (object->isAutoUse()) {
		std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
		drop->changeStats(*drop, player.at(0));

		cocos2d::experimental::AudioEngine::play2d(drop->getSoundName(), false, 1.0f);

		drop.reset();

		dungeon[y*maxcols + x].item_name = EMPTY;
		dungeon[y*maxcols + x].object = nullptr;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
	else if (object->isItem()) {
		if (current_item_inv_size < max_item_inv_size) {
			std::shared_ptr<Drops> drop = std::dynamic_pointer_cast<Drops>(object);
			player.at(0).addItem(drop);

			cocos2d::experimental::AudioEngine::play2d(drop->getSoundName(), false, 1.0f);

			if (shop_action == "chest purchased") {
				removeSprite(item_sprites, maxrows, x, y - 1);
			}

			drop.reset();
		}
		else {
			cocos2d::experimental::AudioEngine::play2d("Player_Movement.mp3", false, 1.0f);

			return;
		}
	}
	else if (object->isWeapon()) {
		if (current_wep_inv_size < max_wep_inv_size) {
			std::shared_ptr<Weapon> weapon = std::dynamic_pointer_cast<Weapon>(object);
			player.at(0).addWeapon(weapon);

			cocos2d::experimental::AudioEngine::play2d(weapon->getSoundName(), false, 1.0f);

			if (shop_action == "chest purchased") {
				removeSprite(item_sprites, maxrows, x, y - 1);
			}

			weapon.reset();
		}
		else {
			cocos2d::experimental::AudioEngine::play2d("Player_Movement.mp3", false, 1.0f);

			return;
		}
	}
	else if (object->isChest()) {
		std::shared_ptr<Chests> chest = std::dynamic_pointer_cast<Chests>(object);

		removeSprite(item_sprites, maxrows, x, y);
		image = chest->open(*chest, dungeon[(y + 1)*maxcols + x]);
		addSprite(item_sprites, maxrows, x, y, 2, image);

		cocos2d::experimental::AudioEngine::play2d(chest->getSoundName(), false, 1.0f);

		dungeon[(y + 1)*maxcols + x].shop_action = "chest purchased";

		chest.reset();
	}
	else if (object->isShield()) {
		removeSprite(item_sprites, maxrows, x, y);
		// save shield at this spot, overwritten if shield is dropped
		std::shared_ptr<Shield> shield = std::dynamic_pointer_cast<Shield>(object);
		player.at(0).equipShield(*this, shield, true);
		shield.reset();
	}
	else if (object->isTrinket()) {
		std::shared_ptr<Trinket> trinket = std::dynamic_pointer_cast<Trinket>(object);
		player.at(0).equipTrinket(*this, trinket, true);
		trinket.reset();
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

std::vector<char> Shop::generate() {
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

std::vector<_Tile>& Shop::getDungeon() {
	return m_shop1;
}
int Shop::getRows() const {
	return m_rows;
}
int Shop::getCols() const {
	return m_cols;
}


// ==============================================
//				:::: 2ND FLOOR ::::
// ==============================================
SecondFloor::SecondFloor(Player p) : Dungeon(3), m_openexit(false), m_watersUsed(false), m_watersCleared(true), m_guardians(-1) {
	int rows = getRows();
	int cols = getCols();

	_Tile *tile;
	int i, j;

	//	initialize entire dungeon to blank space
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			tile = new _Tile;
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = tile->extra = SPACE;
			tile->npc = tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = tile->exit = false;
			tile->gold = 0;
			tile->item_name = "";
			tile->object = nullptr;
			tile->marked = tile->noSpawn = false;

			m_maze2.push_back(*tile);
			delete tile;
		}
	}

	//	initialize top and bottom of dungeon to be unbreakable walls
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			m_maze2[i*cols + j].top = UNBREAKABLE_WALL;
			m_maze2[i*cols + j].bottom = UNBREAKABLE_WALL;
			m_maze2[i*cols + j].traptile = UNBREAKABLE_WALL;
			m_maze2[i*cols + j].wall = true;
			m_maze2[i*cols + j].boundary = true;
		}
		i += (rows - 2);
	}

	//	initialize edges of dungeon to be walls
	for (i = 0; i < rows - 1; i++) {
		m_maze2[i*cols].top = m_maze2[i*cols].bottom = m_maze2[i*cols].traptile = UNBREAKABLE_WALL;
		m_maze2[i*cols + cols - 1].top = m_maze2[i*cols + cols - 1].bottom = m_maze2[i*cols + cols - 1].traptile = UNBREAKABLE_WALL;
	
		m_maze2[i*cols].wall = true;
		m_maze2[i*cols + cols - 1].wall = true;
		m_maze2[i*cols].boundary = true;
		m_maze2[i*cols + cols - 1].boundary = true;
	}



	// :::: GENERATE LEVEL ::::

	std::vector<std::string> finalvec = generateLevel();
	fillLevel(finalvec, p);

	// set staircase coordinates
	for (int i = 0; i < getTraps().size(); i++) {
		if (getTraps()[i]->getItem() == STAIRCASE) {
			m_stairsX = getTraps()[i]->getPosX();
			m_stairsY = getTraps()[i]->getPosY();
		}
	}

	// ---- END LEVEL GENERATION ----


	char toptile, bottomtile, traptile;
	bool hero, wall, enemy, trap, noSpawn;


	//	BUTTON for unlocking exit
	std::shared_ptr<Traps> button = std::make_shared<Button>();
	button->setPosX(cols/2-1);
	button->setPosY(rows/2-6);
	//m_maze2[button->getPosY()*cols + button->getPosX()].traptile = BUTTON;
	m_maze2[button->getPosY()*cols + button->getPosX()].trap_name = BUTTON_UNPRESSED;
	m_maze2[button->getPosY()*cols + button->getPosX()].trap = true;
	m_traps.push_back(button);


	// GENERATE EXTRA MONSTERS
	int n, x, y;
	char monstertype;
	shared_ptr<Monster> monster(nullptr);
	for (int i = 0; i < 6; i++) {

		//	selects how many of a type of monster to place
		switch (i) {
		case 0: n = 15 + randInt(4); break; // Zapper
		case 1: n = 15 + randInt(5); break; // Wanderer
		case 2: n = 20 + randInt(8); break; // Spinner
		case 3: n = 25 + randInt(5); break; // Roundabout
		case 4: n = 3 + randInt(2); break; // Bombee
		case 5: n = 15 + randInt(10); break; // Fire Roundabout
		case 6: n = 5 + randInt(3); break; // 
		case 7: n = 3 + randInt(3); break; // 
		case 8: n = 4 + randInt(4); break; //
		}

		while (n > 0) {

			x = 1 + randInt(cols - 2);
			y = 1 + randInt(rows - 2);

			noSpawn = m_maze2[y*cols + x].noSpawn;
			wall = m_maze2[y*cols + x].wall;
			hero = m_maze2[y*cols + x].hero;
			enemy = m_maze2[y*cols + x].enemy;
			trap = m_maze2[y*cols + x].trap;

			while (noSpawn || hero || wall || enemy || trap) { // while monster position clashes with wall, player, or idol, reroll its position
				x = 1 + randInt(cols - 2);
				y = 1 + randInt(rows - 2);

				noSpawn = m_maze2[y*cols + x].noSpawn;
				wall = m_maze2[y*cols + x].wall;
				hero = m_maze2[y*cols + x].hero;
				enemy = m_maze2[y*cols + x].enemy;
				trap = m_maze2[y*cols + x].trap;
			}

			// the type of enemy
			switch (i) {
			case 0:
				monster = make_shared<Zapper>(x, y, rows); break;
			case 1:
				monster = make_shared<Wanderer>(x, y); break;
			case 2:
				monster = make_shared<Spinner>(x, y, rows); break;
			case 3:
				monster = make_shared<Roundabout>(x, y); break;
			case 4:
				monster = make_shared<Bombee>(x, y); break;
			case 5:
				monster = make_shared<FireRoundabout>(x, y); break;
			case 6:
				monster = make_shared<Roundabout>(x, y); break;
			case 7:
				monster = make_shared<MountedKnight>(x, y); break;
			case 8:
				monster = make_shared<Seeker>(x, y, 11); break;
			}

			getMonsters().emplace_back(monster);
			m_maze2[y*cols + x].monster_name = monster->getName();
			m_maze2[y*cols + x].enemy = true;

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

	m_npcs = dungeon.m_npcs;
	m_monsters = dungeon.m_monsters;
	m_f2guardians = dungeon.m_f2guardians;
	m_traps = dungeon.m_traps;
	m_doors = dungeon.m_doors;

	m_openexit = dungeon.m_openexit;

	m_watersUsed = dungeon.m_watersUsed;
	m_watersCleared = dungeon.m_watersCleared;
	m_guardians = dungeon.m_guardians;

	m_stairsX = dungeon.m_stairsX;
	m_stairsY = dungeon.m_stairsY;

	m_level = dungeon.getLevel();

	return *this;
}

void SecondFloor::peekDungeon(int x, int y, char move) {
	int rows = getRows();
	int cols = getCols();

	char top, bottom, traptile;
	bool boundary, wall, item, trap, enemy;
	std::string trap_name;

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

	// set player's action key (for items that have casting time)
	player.at(0).setAction(move);

	// red tint if player has bloodlust
	if (player.at(0).hasBloodlust()) {
		bloodlustTint(player.at(0));
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

	//			:::: MOVE PLAYER ::::
	int weprange = player.at(0).getWeapon()->getRange();
	bool moveUsed = false;

	// if player is not immobilized, move them
	if (!(player.at(0).isStunned() || player.at(0).isFrozen())) {

		// check for special weapon attack patterns
		if (player.at(0).getWeapon()->hasAttackPattern()) {
			player.at(0).getWeapon()->usePattern(*this, moveUsed);
		}

		if (!moveUsed) {

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

				top = m_maze2[(y + m)*cols + (x + n)].top;
				bottom = m_maze2[(y + m)*cols + (x + n)].bottom;
				trap_name = m_maze2[(y + m)*cols + (x + n)].trap_name;
				traptile = m_maze2[(y + m)*cols + (x + n)].traptile;

				boundary = m_maze2[(y + m)*cols + (x + n)].boundary;
				wall = (player.at(0).isEthereal() ? false : m_maze2[(y + m)*cols + (x + n)].wall);
				item = m_maze2[(y + m)*cols + (x + n)].item;
				trap = m_maze2[(y + m)*cols + (x + n)].trap;
				enemy = m_maze2[(y + m)*cols + (x + n)].enemy;

				if (!(wall || enemy || boundary)) {
					m_maze2[y*cols + x].hero = false;
					m_maze2[(y + m)*cols + (x + n)].hero = true;
					player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);

					// pick up any gold/money that was on the ground
					if (m_maze2[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
						goldPickup(m_maze2, player.at(0).getPosX(), player.at(0).getPosY());
					}

					if (trap) {
						// queue player move
						queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY());

						if (trap_name == BUTTON_UNPRESSED) {
							if (!m_openexit) {
								cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
								cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);
													
								// replace button with pressed button
								int pos = findTrap(x + n, y + m);
								if (pos != -1) {
									getTraps().at(pos)->getSprite()->removeFromParent();
									getTraps().at(pos)->setSprite(createSprite(x + n, y + m, -1, "Button_Pressed_48x48.png"));
								}

								// replace locked stairs with open staircase
								int stairsPos = findTrap(m_stairsX, m_stairsY);
								if (stairsPos != -1) {
									getTraps().at(stairsPos)->getSprite()->removeFromParent();
									getTraps().at(stairsPos)->setSprite(createSprite(m_stairsX, m_stairsY, -1, "Stairs_48x48.png"));
								}


								m_openexit = true;
								m_maze2[m_stairsY*cols + m_stairsX].exit = true;
								//dungeonText.push_back("The stairs have been unlocked!\n");
							}
						}
						else if (trap_name == DEVILS_WATER) {
							if (!m_watersUsed) {
								devilsWaterPrompt();
							}
						}
						else {
							trapEncounter(x + n, y + m);
						}
					}
				}
				else if (wall && !boundary) {
					if (top == DOOR_H || top == DOOR_V) {
						openDoor(x + n, y + m);
					}
					else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
						m_maze2[y*cols + x].hero = false;
						m_maze2[(y + m)*cols + (x + n)].hero = true;
						player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);
					}
					else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
						// play locked door sound
						cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
					}
				}
				else if (enemy) {	// monster encountered
					fight(x + n, y + m);
				}
			}

			else if (move == 'e') {
				collectItem(*this, x, y);

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

			// check if player is idling over lava
			if (!(move == 'l' || move == 'r' || move == 'u' || move == 'd')) {
				trap_name = m_maze2[y*cols + x].trap_name;
				if (trap_name == LAVA) {
					trapEncounter(x, y);
				}
			}
		}

	}

	// queue player move
	if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
		queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY());
	}

	// pick up any gold/money that was on the ground
	if (m_maze2[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
		goldPickup(m_maze2, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// check if player entered devil's waters and killed all guardians
	if (m_guardians == 0 && m_watersCleared) {
		guardiansDefeated();
		
		m_guardians = -1; // prevents if statement from being entered again
	}

	// check active items in play
	checkActive();

	// if there are any doors, check them
	if (!getDoors().empty()) {
		checkDoors();
	}

	// update sprite lighting
	updateLighting();

	//	if player is dead then return
	if (player.at(0).getHP() <= 0) {
		//if (!dungeonText.empty())
			//showText();
		return;
	}

	// if all monsters are dead?
	if (getMonsters().empty()) {
		//if (!dungeonText.empty())
			//showText();

		return;
	}

	// :::: Move monsters ::::
	int mx, my, mInitHP;

	for (unsigned i = 0; i < getMonsters().size(); i++) {
		mInitHP = getMonsters().at(i)->getHP();
		mx = getMonsters().at(i)->getPosX();
		my = getMonsters().at(i)->getPosY();

		// check for any afflictions
		getMonsters().at(i)->checkAfflictions();

		// check if an affliction killed the monster
		if (getMonsters().at(i)->getHP() <= 0) {
			monsterDeath(i);
			i--;
			continue;
		}

		// if monster is too far away, don't bother moving them
		if (abs(player.at(0).getPosX() - mx) + abs(player.at(0).getPosY() - my) > 25) {
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

			// move the monster
			getMonsters().at(i)->move(*getMonsters().at(i), *this);


			// flash monster sprite if damage was taken
			if (getMonsters().at(i)->getHP() < mInitHP) {
				runMonsterDamage(getMonsters().at(i)->getSprite());
			}

			//	if player is dead then break
			if (player.at(0).getHP() <= 0) {
				player.at(0).setDeath(getMonsters().at(i)->getName());
				break;
			}
		}

	}

	// check monsters on traps
	monsterTrapEncounter();

	// check if player is holding the skeleton key
	if (player.at(0).hasSkeletonKey()) {
		player.at(0).checkKeyConditions(dungeonText);
	}

	// invulnerability check: if player is invulnerable and their hp is lower than initial, set hp back to original
	if (player.at(0).isInvulnerable() && player.at(0).getHP() < initHP) {
		player.at(0).setHP(initHP);
	}

	// flash player sprite if took damage
	if (player.at(0).getHP() < initHP) {

		// if player is invisible, they lose invisibility when damaged unless they had enough intellect
		if (player.at(0).isInvisible() && player.at(0).getInt() < 8) {
			player.at(0).removeAffliction("invisibility");
		}

		runPlayerDamage(player.at(0).getSprite());

		player.at(0).decreaseMoneyBonus();
	}

	// run actions
	runSpawn();

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
	int rows = getRows();
	int cols = getCols();

	m_watersUsed = true;
	player.at(0).setMaxHP(player.at(0).getMaxHP() + 30);
	player.at(0).addAffliction(std::make_shared<HealOverTime>(30));

	//cout << "You feel life force flow through you, but there is a price to pay for your actions... \n";

	Archer a1(33, 15), a2(33, 22), a3(44, 15), a4(44,22);
	a1.setName("Guardian");
	a2.setName("Guardian");
	a3.setName("Guardian");
	a4.setName("Guardian");

	a1.setSprite(createSprite(31, 27, 1, "Archer_48x48.png"));
	a2.setSprite(createSprite(31, 28, 1, "Archer_48x48.png"));
	a3.setSprite(createSprite(42, 27, 1, "Archer_48x48.png"));
	a4.setSprite(createSprite(42, 28, 1, "Archer_48x48.png"));

	a1.setPosX(31); a1.setPosY(27); m_monsters.emplace_back(new Archer(a1)); m_maze2[a1.getPosY()*cols + a1.getPosX()].enemy = true;
	a2.setPosX(31); a2.setPosY(28); m_monsters.emplace_back(new Archer(a2)); m_maze2[a2.getPosY()*cols + a2.getPosX()].enemy = true;
	a3.setPosX(42); a3.setPosY(27); m_monsters.emplace_back(new Archer(a3)); m_maze2[a3.getPosY()*cols + a3.getPosX()].enemy = true;
	a4.setPosX(42); a4.setPosY(28); m_monsters.emplace_back(new Archer(a4)); m_maze2[a4.getPosY()*cols + a4.getPosX()].enemy = true;

	// close off the exits with walls after accepting the devils gift
	for (int i = 29; i <= 30; i++) {
		for (int j = 27; j <= 28; j++) {
			addSprite(wall_sprites, rows, i, j, 1, "D_Wall_Terrain2_48x48.png");

			m_maze2[j*cols + i].top = UNBREAKABLE_WALL;
			m_maze2[j*cols + i].wall = true;
		}
	}
	for (int i = 43; i <= 44; i++) {
		for (int j = 27; j <= 28; j++) {
			addSprite(wall_sprites, rows, i, j, 1, "D_Wall_Terrain2_48x48.png");

			m_maze2[j*cols + i].top = UNBREAKABLE_WALL;
			m_maze2[j*cols + i].wall = true;
		}
	}

	m_watersCleared = false;
	m_guardians = 4;
}
void SecondFloor::guardiansDefeated() {
	cocos2d::experimental::AudioEngine::play2d("Guardians_Defeated.mp3", false, 1.0f);

	// remove the blocked walls
	for (int i = 29; i <= 30; i++) {
		for (int j = 27; j <= 28; j++) {
			removeSprite(wall_sprites, getRows(), i, j);

			m_maze2[j*getCols() + i].top = SPACE;
			m_maze2[j*getCols() + i].wall = false;
		}
	}
	for (int i = 43; i <= 44; i++) {
		for (int j = 27; j <= 28; j++) {
			removeSprite(wall_sprites, getRows(), i, j);

			m_maze2[j*getCols() + i].top = SPACE;
			m_maze2[j*getCols() + i].wall = false;
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

void SecondFloor::monsterDeath(int pos) { // monster's coordinates
	// call monster's death function
	getMonsters().at(pos)->death(*getMonsters().at(pos), *this);

	// check if monster was a guardian
	if (getMonsters().at(pos)->getName() == "Guardian") {
		m_guardians--;
		if (m_guardians == 0)
			m_watersCleared = true;
	}

	// remove them from the monster vector
	getMonsters().erase(getMonsters().begin() + pos);
}
void SecondFloor::fight(int x, int y) { // monster's coordinates
	int i = findMonster(x, y);

	if (i == -1)
		return;

	//player.at(0).attack(m_f2monsters, m_f2actives, i, dungeonText);
	player.at(0).attack(*this, *getMonsters().at(i));

	std::string monster = getMonsters().at(i)->getName();
	if (getMonsters().at(i)->getHP() <= 0) {
		monsterDeath(i);
	}
}

std::vector<std::vector<std::string>> SecondFloor::outermostChunks() {

	std::vector<std::vector<std::string>> i = 
	{ { wll,  wll,  wll, i_gd,  wll,   sp,  wll,  wll},
	  { wll,   sp,  wll,   sp,   sp,   sp,  wll,   sp},
	  {  sp,   sp,  wll,   sp,   sp,   xx, t_cl,   xx},
	  {  sp,   xx,  t_f,   xx,   sp,   sp,  wll,   sp},
	  { wll,   xx,   xx,   xx,  wll, t_lv,  wll,   sp},
	  {  sp,  wll,   sp,  wll, i_gd,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> ii = 
	{ {r_wl,   sp, t_td,  wll,  wll,  wll, i_ri,  wll},
	  {  sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,  wll,  wll,   sp},
	  {  sp, t_lv, t_lv,  t_s, t_mb,   sp,   sp,   sp},
	  {  sp, t_lv, t_lv, t_lv,   sp,   sp,   sp,   sp},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,   sp,   sp} };

	std::vector<std::vector<std::string>> iii = 
	{ { wll,  wll,   sp,   sp, t_cl,   sp,  wll,   sp},
	  {  sp,  wll,   sp,  wll,   sp,   sp,   sp,  wll},
	  {  sp,   sp,   sp,  t_f,  wll,   sp,   sp, r_wl},
	  {  sp,   sp,  wll, i_gd,   sp,   sp,  wll,   sp},
	  {  sp,   sp,   sp, r_wl,   sp,   sp,   sp,   sp},
	  { wll,   sp,   sp, t_cl,   sp,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  { wll,   sp, i_gd,  wll,  wll,   sp,   sp,  wll},
	  { wll, i_gd,   sp,  wll,  wll, i_ri,   sp,  wll},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  { wll, t_tr,   sp, t_cl, t_cl,   sp,   sp,  wll},
	  { wll,   sp,   sp, t_cl, t_cl,   sp, t_tl,  wll},
	  {  sp,  wll,  wll,   sp,   sp,  wll,  wll,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi = 
	{ {  sp,  t_s,   sp,  wll,  wll,  m_z,  m_z,  wll},
	  { wll,  wll,   sp,  wll,  wll,   xx,   xx,   sp},
	  {m_sp, t_cl,   sp,  wll, t_cl,   xx,  wll,   sp},
	  { wll,  wll,   xx, t_cl,   xx,   xx,  wll,   sp},
	  {  sp,  t_s,   sp,   xx,  t_f,   sp, t_sk, r_wl},
	  {  sp,   sp,   sp,   sp,  wll,   sp, t_sk,   sp} };

	std::vector<std::vector<std::string>> vii = 
	{ { wll,   sp,   sp,   sp,  t_s,   sp,  m_z,   sp},
	  {  sp,  wll,  wll,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll,  wll,   xx,   sp,   sp},
	  {  sp,  t_s,   sp,   sp, r_wl, t_cl, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   xx,  wll,   sp},
	  {  sp,  m_z,  t_s, t_mb, t_cl, t_cl, t_cl,  wll} };

	std::vector<std::vector<std::string>> viii = 
	{ {t_td,   sp, t_td,   sp, t_td,   sp, t_td,   sp},
	  {  sp,   sp,   sp,  t_s,   sp,  t_s,   sp,   sp},
	  {  sp,   sp, r_wl,   sp,   sp,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,   sp,   sp, r_wl,   sp},
	  {  sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp},
	  {  sp, t_tu,   sp, t_tu,   sp, t_tu,   sp, t_tu} };

	std::vector<std::vector<std::string>> ix = 
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {  sp, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,   sp},
	  {  sp,   sp,   sp,   sp,   sp, m_sp,   sp,   sp},
	  {  sp,   sp, m_sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,   sp},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp, t_lv, t_lv, t_cl, t_lv,   sp,   sp},
	  {  sp, t_lv, t_lv, t_cl, t_cl, t_cl, t_lv,   sp},
	  {  sp, t_lv, t_cl, t_cl, t_lv, t_cl, t_cl,   sp},
	  {  sp, t_cl, t_cl, t_lv, t_cl, t_cl, t_lv,   sp},
	  {  sp, t_lv, t_lv, t_cl, t_cl, t_lv, t_lv,   sp},
	  {  sp,   sp, t_lv, t_cl, t_lv, t_lv,   sp,   sp} };

	std::vector<std::vector<std::string>> xi = 
	{ {  sp,   sp, t_lv, i_bc,   sp, t_lv,   sp,   sp},
	  {  sp,   sp, t_lv, t_lv,   sp, t_lv,   sp,   sp},
	  {  sp, m_sp,   sp, t_lv, t_sk, t_lv, m_sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp, t_tl, t_tr,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii = 
	{ {  sp, t_cl,   sp,t_mbv, t_cl, t_lv, t_cl,   sp},
	  {  sp,t_mbv, t_cl, t_cl,   sp, t_cl, t_cl,   sp},
	  {  sp,   sp, t_cl,   sp,t_mbv, t_cl,   sp,   sp},
	  {  sp,   sp,t_mbv,   sp, t_cl, t_cl,   sp,   sp},
	  {  sp, t_cl,   sp,   sp, t_cl, t_cl,t_mbv,   sp},
	  {  sp, t_cl,   sp,t_mbv,   sp,   sp, t_cl,   sp} };


	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);*/

	//c = mixOutermostChunks(c);

	return c.at(randInt(c.size()));
}
std::vector<std::vector<std::string>> SecondFloor::edgeChunks() {
	std::vector<std::vector<std::string>> i =
	{ { wll,   sp,  wll,  wll,  wll,   xx,   xx,   xx},
	  { wll,  wll,   sp,   sp,   sp,  wll,   xx,  wll},
	  {  sp,   sp,   xx,   sp,   sp,   sp,   xx,   sp},
	  { wll,   sp,   xx,  wll,  wll,   sp,   sp, r_wl},
	  { wll,  wll,   xx,   sp,   sp,  wll,  wll,   sp},
	  {r_wl,   sp,  wll,   sp,   sp,   sp,   sp,  wll} };

	std::vector<std::vector<std::string>> ii =
	{ { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  { wll,  m_g,   sp, t_cl, t_cl, m_sp,   sp, i_rc},
	  { wll,  m_g,   sp, t_cl, t_cl, m_sp,   sp, i_gd},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll} };

	std::vector<std::vector<std::string>> iii =
	{ { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  {i_gd,   sp, m_sp, t_cl, t_cl,   sp,  m_k,  wll},
	  {i_rc,   sp, m_sp, t_cl, t_cl,   sp,  m_a,  wll},
	  { wll,  wll,  wll, t_cl, t_cl,  wll,  wll,  wll},
	  { wll,  wll,   xx, t_cl, t_cl,   xx,  wll,  wll} };

	std::vector<std::vector<std::string>> iv =
	{ { wll,  wll,   sp,   sp,   sp,  wll,  wll,   sp},
	  {  sp,   sp,   sp,  wll, t_cl,   sp,  wll,   sp},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {r_wl,   sp,  wll, m_sp,  wll,   sp,  wll, m_sp},
	  { wll, t_lv,   sp,   sp,   sp, t_cl,   sp,   sp},
	  {  sp,  wll,   sp,   sp, t_cl,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ { wll,   sp,  wll,   sp,   sp,   sp,  wll,   sp},
	  { wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp},
	  {  sp,   sp,   sp,  wll,   sp,   sp,  wll,   sp},
	  {  sp,   sp,  wll,   sp,   sp,  wll,   sp,  t_s},
	  {  sp,  wll,   sp,   sp,  wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll,   sp,   sp, r_wl,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,  m_z,   sp,   sp, m_sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp},
	  {  sp,   sp,  wll,   xx,   xx,   sp,  wll,   sp},
	  {  xx,  wll,   xx,   xx,  wll,   sp,   sp,  wll},
	  {  xx,  wll, i_rt, t_lv,   sp,  wll,   sp,  wll},
	  {  xx, t_lv,  wll,  wll,   sp,  wll,   sp,  wll} };

	std::vector<std::vector<std::string>> vii =
	{ {  xx,   xx,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,  t_f, m_rm,  wll, i_ri,  wll},
	  {  sp,   sp,   sp,   sp,   sp, m_rm,  wll,  wll},
	  {t_lv, t_lv, t_lv,   sp,   sp,   sp, m_rm,  wll},
	  { wll, i_rs, t_lv,   sp,   sp,   sp, m_rm,  wll},
	  { wll,  wll, t_lv,   sp,   sp,   sp,  t_f,  wll} };

	std::vector<std::vector<std::string>> viii =
	{ { wll,   xx,   sp,  wll,  wll,  wll,   sp,  wll},
	  { wll,   xx, t_df,   xx,   xx,   xx,  wll,  wll},
	  { wll,   xx,   xx,   xx,  wll,   xx,   sp,  wll},
	  { t_f,  wll,  wll,  wll,   xx,   xx,   sp,  wll},
	  {  xx,   xx,   xx,   xx,   xx, t_df,   sp, i_gd},
	  {  xx,   xx,  wll, t_ts,  wll,   sp,  wll,  wll} };

	std::vector<std::vector<std::string>> ix =
	{ { wll,   sp,   sp,   sp,   sp,   sp,  wll, i_gd},
	  {  sp, t_sk,   sp, t_sk,   sp, t_sk,  wll,  wll},
	  {  sp,   sp, t_as,   sp, t_as,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp, m_rm,   sp, m_sp,   sp, m_rm,   sp, m_rm},
	  { wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  wll,  t_f, i_gd,   sp,   sp,   sp},
	  {  sp,   sp,   sp,  wll, m_sp, i_gd,   sp,   sp},
	  {  sp,   sp, i_ri, m_sp,  wll,   sp,   sp,   sp},
	  {  sp,   sp,   sp, i_gd,  t_f,  wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ { wll,  wll, r_wl,  m_z, m_sp,  wll,  wll,  wll},
	  { wll,   sp,   sp,   sp,   sp,  wll,  wll,   sp},
	  {  sp,  wll,   sp, m_rm,   sp,   sp,   sp, r_wl},
	  { wll, r_wl,   sp,   sp, m_rm,   sp,   sp,  wll},
	  {  sp,  wll,  wll,   sp,   sp,   sp,  wll,  wll},
	  { wll,  wll,  wll, m_sp,  m_z,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp, t_lv,  wll},
	  { wll, t_lv, t_lv,   sp, t_cl, t_lv, i_rc, t_lv},
	  {  sp, t_lv, t_lv, t_lv, t_cl,   sp, t_lv,   sp},
	  {  sp,   sp, t_lv, t_lv, t_cl,   sp,   sp,   sp},
	  {  sp,  t_s,   sp,   sp,   sp,   sp,  t_s,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };


	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);*/

	//c = mixEdgeChunks(c);

	return c.at(randInt(c.size()));
}

/*
std::vector<std::string> SecondFloor::innerChunksCT(std::vector<std::vector<std::vector<std::string>>> &c) {
	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);

	c = mixInnerChunksCT(c);

	return combineChunks(c);
}
std::vector<std::string> SecondFloor::innerChunksCB(std::vector<std::vector<std::vector<std::string>>> &c) {
	std::vector<std::vector<std::string>> i =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);

	c = mixInnerChunksCB(c);

	return combineChunks(c);
}
std::vector<std::string> SecondFloor::centerChunks(std::vector<std::vector<std::vector<std::string>>> &c) {
	std::vector<std::vector<std::string>> i =
	{ { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iv =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	c.push_back(x);
	c.push_back(xi);
	c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);

	c = mixCenterChunks(c);

	return combineChunks(c);
}
*/

std::vector<std::vector<std::string>> SecondFloor::lavaChunks() {
	//	 LAVA CHUNKS
	std::vector<std::vector<std::string>> i =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> ii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv,  m_a, t_tt, t_tt,  m_a, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> iii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv,  m_a,  m_a, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv,  m_a,  m_a, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> iv =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv,  t_f, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv,  t_f, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> v =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv,  t_f, t_lv},
	  {t_lv, t_lv, t_lv, t_df, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_df, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> vi =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_tr, t_lv, t_lv},
	  {t_lv, t_lv, t_tl, t_lv,  t_f, t_tr, t_lv, t_lv},
	  {t_lv, t_lv, t_tl, t_lv, t_lv, t_tr, t_lv, t_lv},
	  {t_lv, t_lv, t_tl, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> vii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> viii =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> ix =
	{ {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv},
	  {t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv, t_lv} };

	std::vector<std::vector<std::string>> x =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xi =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> xii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::vector<std::string>>> c;

	c.push_back(i);
	c.push_back(ii);
	c.push_back(iii);
	c.push_back(iv);
	c.push_back(v);
	c.push_back(vi);
	c.push_back(vii);
	c.push_back(viii);
	c.push_back(ix);
	//c.push_back(x);
	//c.push_back(xi);
	//c.push_back(xii);
	/*c.push_back(xiii);
	c.push_back(xiv);
	c.push_back(xv);
	c.push_back(xvi);
	c.push_back(xvii);
	c.push_back(xviii);
	c.push_back(xix);
	c.push_back(xx);
	c.push_back(xxi);*/

	//c = mixInnerChunks1(c);

	return c.at(randInt(c.size()));
}
std::vector<std::vector<std::string>> SecondFloor::fixedChunks(std::string chunk) {
	// the player's chunk where they spawn
	std::vector<std::vector<std::string>> pChunk =
	{ {  xx,   xx, r_wl,   xx,   xx, r_wl,   xx,   xx},
	  { wll,   xx,   xx,   xx,   xx,   xx, r_wl,   xx},
	  {  xx,   xx,   xx, _ply,   xx,  wll,   xx,   xx},
	  {  xx,   xx,   xx,   xx,   xx,   xx,   xx,   xx},
	  {  xx,  wll,   xx,   xx,   xx,   xx,   xx,   xx},
	  {  xx,   xx, r_wl,   xx,   xx, r_wl,   xx,  wll}, };


	// the chunk where the staircase will spawn
	std::vector<std::vector<std::string>> eChunk =
	{ { wll,  _dh,  wll,  wll,  wll,  wll,  _dh,  wll},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
	  { wll,   sp,   sp, m_rm,   sp, m_rm,   sp,  wll},
	  { wll,   sp,   sp, m_rm, lkex, m_rm,   sp,  wll},
	  { wll,   sp,   sp, m_rm, m_rm, m_rm,   sp,  wll},
	  { wll,  _dh,  wll,  wll,  wll,  wll,  _dh,  wll} };


	// the chunk where player's can enter the center ring
	std::vector<std::vector<std::string>> outerBridge =
	{ {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv},
	  {t_lv, t_lv,   sp,   sp,   sp,   sp, t_lv, t_lv} };

	// empty chunk
	std::vector<std::vector<std::string>> empty =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };


	// The 9 special chunks in the center

	std::vector<std::vector<std::string>> i =
	{ { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll, r_wl,  wll,  wll,  wll,  wll,  wll},
	  { wll, i_gd,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp} };

	std::vector<std::vector<std::string>> ii =
	{ { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> iii =
	{ { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll, i_gd,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll, i_gd,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll, r_wl,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> iv =
	{ { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp} };

	// center chunk with fountain
	std::vector<std::vector<std::string>> v =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  _dw,  _dw,  _dw,  _dw,   sp,   sp},
	  {  sp,   sp,  _dw,  ftn,  ftn,  _dw,   sp,   sp},
	  {  sp,   sp,  _dw,  ftn,  ftn,  _dw,   sp,   sp},
	  {  sp,   sp,  _dw,  _dw,  _dw,  _dw,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp} };

	std::vector<std::vector<std::string>> vi =
	{ {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> vii =
	{ { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp},
	  { wll,  wll, i_ri,  wll,  wll,  wll,   sp,   sp},
	  { wll, i_rw,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll, r_wl,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> viii =
	{ {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  {  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll} };

	std::vector<std::vector<std::string>> ix =
	{ {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll, r_wl,  wll},
	  {  sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll, r_wl,  wll,  wll},
	  { wll,  wll,  wll,  wll, i_gd,  wll,  wll,  wll},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll} };

	// get the correct chunk
	if (chunk == "pChunk") return pChunk;
	else if (chunk == "eChunk") return eChunk;
	else if (chunk == "outerBridge") return outerBridge;
	else if (chunk == "empty") return empty;
	else if (chunk == "i") return i;
	else if (chunk == "ii") return ii;
	else if (chunk == "iii") return iii;
	else if (chunk == "iv") return iv;
	else if (chunk == "v") return v;
	else if (chunk == "vi") return vi;
	else if (chunk == "vii") return vii;
	else if (chunk == "viii") return viii;
	else if (chunk == "ix") return ix;
}

std::vector<std::string> SecondFloor::generateLevel() {
	std::vector<std::string> finalvec, b;
	std::vector<std::vector<std::vector<std::string>>> v;

	for (layer; layer <= 9; layer++) {
		for (int i = 0; i < (getCols() - 2) / 8 /* divided by length of chunk */; i++) {

			if (!(layer == 1 || layer == 9 || layer == 5) && (i == 0 || i == 8)) {
				v.push_back(edgeChunks());
			}
			else if (layer == 1 || layer == 9) {
				if (i == 0 || i == 4 || i == 8)
					v.push_back(fixedChunks("empty"));
			}
			else if (layer == 2 || layer == 8) {

				// lava chunks
				if (i >= 1 && i <= 3 || i >= 5 && i <= 7) {
					v.push_back(lavaChunks());
				}
				else if (i == 4) {
					v.push_back(fixedChunks("outerBridge"));
				}
			}
			else if (layer == 3 || layer == 7) {

				// lava chunks
				if (i == 1 || i == 7) {
					v.push_back(lavaChunks());
				}
				else if (i == 4) {
					v.push_back(fixedChunks("empty"));
				}
			}
			// 4 - 6 are the special chunks in the center
			else if (layer == 4) {

				// special chunks
				if (i == 3) v.push_back(fixedChunks("i"));
				else if (i == 0 || i == 2 || i == 6 || i == 8) v.push_back(edgeChunks());
				else if (i == 4) v.push_back(fixedChunks("ii"));
				else if (i == 5) v.push_back(fixedChunks("iii"));
				else if (i == 1 || i == 7) v.push_back(lavaChunks());

			}
			else if (layer == 5) {

				// special chunks
				if (i == 0) v.push_back(fixedChunks("pChunk"));
				else if (i == 2 || i == 6) v.push_back(fixedChunks("empty"));
				else if (i == 3) v.push_back(fixedChunks("iv"));
				else if (i == 4) v.push_back(fixedChunks("v"));
				else if (i == 5) v.push_back(fixedChunks("vi"));
				else if (i == 1 || i == 7) v.push_back(lavaChunks());
				else if (i == 8) v.push_back(fixedChunks("eChunk"));

			}
			else if (layer == 6) {

				// special chunks
				if (i == 3) v.push_back(fixedChunks("vii"));
				else if (i == 0 || i == 2 || i == 6 || i == 8) v.push_back(edgeChunks());
				else if (i == 4) v.push_back(fixedChunks("viii"));
				else if (i == 5) v.push_back(fixedChunks("ix"));
				else if (i == 1 || i == 7) v.push_back(lavaChunks());

			}


			// if nothing was added in the intermediary, then add an "outer" chunk
			if (i == v.size())
				v.push_back(outermostChunks());
		}

		// transform 3d vector into 1d vector
		b = combineChunks(v);

		// add to the final dungeon map
		for (int i = 0; i < b.size(); i++)
			finalvec.push_back(b.at(i));

		b.clear();
		v.clear();
	}

	return finalvec;
}
std::vector<std::string> SecondFloor::combineChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	std::vector<std::string> v;
	for (int i = 0; i < 6; i++) {			// iterate thru rows of 2d vector
		for (int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 8; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
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


// ==============================================
//				:::: 3RD FLOOR ::::
// ==============================================
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
			tile->npc = tile->hero = tile->wall = tile->item = tile->trap = tile->enemy = tile->exit = false;
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
			m_maze3[i*cols + j].boundary = true;
		}
		i += (rows - 2);
	}

	//	initialize edges of dungeon to be walls
	for (i = 0; i < rows - 1; i++) {
		m_maze3[i*cols].top = m_maze3[i*cols].bottom = m_maze3[i*cols + j].traptile = UNBREAKABLE_WALL;
		m_maze3[i*cols + cols - 1].top = m_maze3[i*cols + cols - 1].bottom = m_maze3[i*cols + cols - 1].traptile = UNBREAKABLE_WALL;
	
		m_maze3[i*cols].wall = true;
		m_maze3[i*cols + cols - 1].wall = true;
		m_maze3[i*cols].boundary = true;
		m_maze3[i*cols + cols - 1].boundary = true;
	}

	/*
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
			TLChunks(c, pool); pool.clear(); m_layer++;
			topChunks(c, pool); pool.clear();
			v = TRChunks(c, pool); pool.clear();
		}
		else if (count == 1 || count == 2) {
			leftEdgeChunks(c, pool); pool.clear();
			middleChunks(c, pool); pool.clear();
			v = rightEdgeChunks(c, pool); pool.clear();
		}
		else if (count == 3) {
			m_layer++;
			BLChunks(c, pool); pool.clear(); m_layer++;
			bottomChunks(c, pool); pool.clear();
			v = BRChunks(c, pool); pool.clear();
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
			if (finalvec.at(count) == c_LAVA) {
				std::shared_ptr<Traps> lava = std::make_shared<Lava>(j, i);
				m_traps.emplace_back(lava);
				lava.reset();

				m_maze3[i*cols + j].top = SPACE;
				m_maze3[i*cols + j].bottom = SPACE;
				m_maze3[i*cols + j].traptile = finalvec.at(count);
				m_maze3[i*cols + j].trap_name = LAVA;
				m_maze3[i*cols + j].trap = true;
			}

			//	if the object was an item or enemy
			if (m_maze3[i*cols + j].bottom != WALL && m_maze3[i*cols + j].bottom != SPACE) {

				object = m_maze3[i*cols + j].bottom;

				switch (object) {
				case c_PLAYER:
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
					
					Door door;
					door.setPosX(j);
					door.setPosY(i);
					m_doors.emplace_back(new Door(door));
					break;
				}
				case STAIRS: {
					Stairs stairs;
					stairs.setPosX(j);
					stairs.setPosY(i);
					//m_traps.emplace_back(new Stairs(stairs));


					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].traptile = STAIRS;
					m_maze3[i*cols + j].exit = true;
					m_maze3[i*cols + j].trap = true;
					break;
				}
				case BUTTON: {
					Button button;
					button.setPosX(j);
					button.setPosY(i);
					//m_traps.emplace_back(new Button(button));

					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].traptile = BUTTON;
					m_maze3[i*cols + j].trap = true;
					break;
				}
				case c_BOMB: {
					m_maze3[i*cols + j].object = std::make_shared<Bomb>();

					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = BOMB;
					m_maze3[i*cols + j].item = true;
					break;
				}
				case c_ARMOR: {
					m_maze3[i*cols + j].object = std::make_shared<ArmorDrop>();
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = ARMOR;
					m_maze3[i*cols + j].item = true;
					break;
				}
				case c_LIFEPOT: {
					m_maze3[i*cols + j].object = std::make_shared<LifePotion>();
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = LIFEPOT;
					m_maze3[i*cols + j].item = true;
					break;
				}
				case c_STATPOT: {
					m_maze3[i*cols + j].object = std::make_shared<StatPotion>();
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = STATPOT;
					m_maze3[i*cols + j].item = true;
					break;
				}
				case c_HEART_POD: {
					m_maze3[i*cols + j].object = std::make_shared<HeartPod>();
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = HEART_POD;
					m_maze3[i*cols + j].item = true;
					break;
				}
				case c_BROWN_CHEST: {
					m_maze3[i*cols + j].object = std::make_shared<BrownChest>();
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = BROWN_CHEST;
					m_maze3[i*cols + j].item = true;
					break;
				}
				case c_SILVER_CHEST: {
					m_maze3[i*cols + j].object = std::make_shared<SilverChest>();
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = SILVER_CHEST;
					m_maze3[i*cols + j].item = true;
					break;
				}
				case c_GOLDEN_CHEST: {
					m_maze3[i*cols + j].object = std::make_shared<GoldenChest>();
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = GOLDEN_CHEST;
					m_maze3[i*cols + j].item = true;
					break;
				}
				case c_INFINITY_BOX: {
					m_maze3[i*cols + j].object = std::make_shared<InfinityBox>();
					m_maze3[i*cols + j].top = SPACE;
					m_maze3[i*cols + j].item_name = INFINITY_BOX;
					m_maze3[i*cols + j].item = true;
					break;
				}
				case SPIKETRAP_DEACTIVE: {
					SpikeTrap st(j, i, 4);
					m_traps.emplace_back(new SpikeTrap(st)); //

					m_maze3[i*cols + j].top = m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].traptile = SPIKETRAP_DEACTIVE;
					m_maze3[i*cols + j].trap = true;
					break;
				}
				case TRIGGER_SPIKE_DEACTIVE: {
					TriggerSpike ts(j, i);
					//m_f3actives.emplace_back(new TriggerSpike(ts));
					m_traps.emplace_back(new TriggerSpike(ts)); //

					m_maze3[i*cols + j].top = m_maze3[i*cols + j].bottom = SPACE;
					m_maze3[i*cols + j].traptile = TRIGGER_SPIKE_DEACTIVE;
					m_maze3[i*cols + j].trap = true;
					break;
				}
				case c_WANDERER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Wanderer wand(j, i);
					getMonsters().emplace_back(new Wanderer(wand));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case c_GOBLIN: {
					m_maze3[i*cols + j].bottom = SPACE;
					Goblin goblin(j, i, 12);
					getMonsters().emplace_back(new Goblin(goblin));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case c_ARCHER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Archer archer(j, i);
					getMonsters().emplace_back(new Archer(archer));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case c_SEEKER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Seeker seeker(j, i, 10);
					getMonsters().emplace_back(new Seeker(seeker));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case c_MOUNTED_KNIGHT: {
					m_maze3[i*cols + j].bottom = SPACE;
					MountedKnight knight(j, i);
					getMonsters().emplace_back(new MountedKnight(knight));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case c_ZAPPER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Zapper zapper(j, i, rows);
					//zapper.setPosX(j);
					//zapper.setPosY(i);
					getMonsters().emplace_back(new Zapper(zapper));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case c_SPINNER: {
					m_maze3[i*cols + j].bottom = SPACE;
					Spinner spinner(j, i, rows);
					getMonsters().emplace_back(new Spinner(spinner));

					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case c_ROUNDABOUT: {
					m_maze3[i*cols + j].bottom = SPACE;
					Roundabout ra(j, i);
					getMonsters().emplace_back(new Roundabout(ra));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				case c_BOMBEE: {
					m_maze3[i*cols + j].bottom = SPACE;
					Bombee bombee(j, i);
					getMonsters().emplace_back(new Bombee(bombee));
					m_maze3[i*cols + j].enemy = true;
					break;
				}
				default: break;
				}
			}
			count++;
		}
	}
	*/

	std::vector<std::string> finalvec = generateLevel();
	fillLevel(finalvec, p, 0, 0);

	// END LEVEL GENERATION
}
ThirdFloor& ThirdFloor::operator=(ThirdFloor const &dungeon) {
	for (unsigned i = 0; i < dungeon.getRows()*dungeon.getCols(); i++) {
		m_maze3[i] = dungeon.m_maze3[i];
	}
	player = dungeon.player;

	m_npcs = dungeon.m_npcs;
	m_monsters = dungeon.m_monsters;
	m_traps = dungeon.m_traps;
	m_doors = dungeon.m_doors;

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
	bool boundary, wall, item, enemy, trap;

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

	// set player's action key (for items that have casting time)
	player.at(0).setAction(move);

	// red tint if player has bloodlust
	if (player.at(0).hasBloodlust()) {
		bloodlustTint(player.at(0));
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

	// :::: Move player ::::
	int weprange = player.at(0).getWeapon()->getRange();
	bool moveUsed = false;

	// if player is not immobilized, move them
	if (!(player.at(0).isStunned() || player.at(0).isFrozen())) {

		// check for special weapon attack patterns
		if (player.at(0).getWeapon()->hasAttackPattern()) {
			player.at(0).getWeapon()->usePattern(*this, moveUsed);
		}

		if (!moveUsed) {

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

				top = m_maze3[(y + m)*cols + (x + n)].top;
				bottom = m_maze3[(y + m)*cols + (x + n)].bottom;
				traptile = m_maze3[(y + m)*cols + (x + n)].traptile;

				boundary = m_maze3[(y + m)*cols + (x + n)].boundary;
				wall = (player.at(0).isEthereal() ? false : m_maze3[(y + m)*cols + (x + n)].wall);
				item = m_maze3[(y + m)*cols + (x + n)].item;
				trap = m_maze3[(y + m)*cols + (x + n)].trap;
				enemy = m_maze3[(y + m)*cols + (x + n)].enemy;

				if (!(wall || enemy || boundary)) {
					m_maze3[y*cols + x].hero = false;
					m_maze3[(y + m)*cols + (x + n)].hero = true;
					player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);

					// pick up any gold/money that was on the ground
					if (m_maze3[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
						goldPickup(m_maze3, player.at(0).getPosX(), player.at(0).getPosY());
					}

					if (trap) {
						// queue player move
						queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY());

						trapEncounter(x + n, y + m);
					}
				}
				else if (wall && !boundary) {
					if (top == DOOR_H || top == DOOR_V) {
						openDoor(x + n, y + m);
					}
					else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
						m_maze3[y*cols + x].hero = false;
						m_maze3[(y + m)*cols + (x + n)].hero = true;
						player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);
					}
					else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
						// play locked door sound
						cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
					}
				}
				else if (enemy) {	// monster encountered
					fight(x + n, y + m);
				}
			}

			else if (move == 'e') {
				collectItem(*this, x, y);

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

			// check if player is idling over lava
			if (!(move == 'l' || move == 'r' || move == 'u' || move == 'd')) {
				traptile = m_maze3[y*cols + x].traptile;
				if (traptile == c_LAVA) {
					trapEncounter(x, y);
				}
			}
		}

	}

	// queue player move
	if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
		queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY());
	}

	// pick up any gold/money that was on the ground
	if (m_maze3[player[0].getPosY()*cols + player[0].getPosX()].gold > 0) {
		goldPickup(m_maze3, player.at(0).getPosX(), player.at(0).getPosY());
	}

	// check active items in play
	checkActive();

	// if there are any doors, check them
	if (!getDoors().empty()) {
		checkDoors();
	}

	// update sprite lighting
	updateLighting();

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
	if (getMonsters().empty()) {
		//if (!dungeonText.empty())
		//	showText();

		return;
	}

	// :::: Move monsters ::::
	int mx, my, mInitHP;
	for (unsigned i = 0; i < getMonsters().size(); i++) {
		mInitHP = getMonsters().at(i)->getHP();
		mx = getMonsters().at(i)->getPosX();
		my = getMonsters().at(i)->getPosY();

		// check for any afflictions
		getMonsters().at(i)->checkAfflictions();

		// check if an affliction killed the monster
		if (getMonsters().at(i)->getHP() <= 0) {
			monsterDeath(i);
			i--;
			continue;
		}

		// if monster is too far away, don't bother moving them
		if (abs(player.at(0).getPosX() - mx) + abs(player.at(0).getPosY() - my) > 20) {
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

			getMonsters().at(i)->move(*getMonsters().at(i), *this);

			// flash monster sprite if damage was taken
			if (getMonsters().at(i)->getHP() < mInitHP) {
				runMonsterDamage(getMonsters().at(i)->getSprite());
			}

			//	if player is dead then break
			if (player.at(0).getHP() <= 0) {
				player.at(0).setDeath(getMonsters().at(i)->getName());
				break;
			}
		}
	}

	// check monsters on traps
	monsterTrapEncounter();

	// check if player is holding the skeleton key
	if (player.at(0).hasSkeletonKey()) {
		player.at(0).checkKeyConditions(dungeonText);
	}

	// invulnerability check: if player is invulnerable and their hp is lower than initial, set hp back to original
	if (player.at(0).isInvulnerable() && player.at(0).getHP() < initHP) {
		player.at(0).setHP(initHP);
	}

	// flash player sprite if took damage
	if (player.at(0).getHP() < initHP) {

		// if player is invisible, they lose invisibility when damaged unless they had enough intellect
		if (player.at(0).isInvisible() && player.at(0).getInt() < 8) {
			player.at(0).removeAffliction("invisibility");
		}

		runPlayerDamage(player.at(0).getSprite());

		player.at(0).decreaseMoneyBonus();
	}

	// run actions
	runSpawn();

	//if (!dungeonText.empty())
		//showText();
}

void ThirdFloor::checkDoors() {
	int maxrows = getRows();
	int maxcols = getCols();

	if (!getDoors().empty()) {
		int x, y;

		for(int i = 0; i < getDoors().size(); i++) {
		
			shared_ptr<Door> door = dynamic_pointer_cast<Door>(getDoors().at(i));
			x = door->getPosX();
			y = door->getPosY();

			bool enemy, hero;
			enemy = getDungeon()[y*maxcols + x].enemy;
			hero = getDungeon()[y*maxcols + x].hero;

			// if door is locked, check if room is clear to unlock
			if (door->isLocked()) {
				if (roomCleared()) {
					toggleDoorLocks(x, y, getDungeon()[y*maxcols + x].traptile);
				}
			}
			// if door is unlocked, open, isn't being held, and nothing is in the way
			else if (door->isOpen() && !door->isHeld() && !(enemy || hero) && (getDungeon()[y*maxcols + x].top == DOOR_H_OPEN || getDungeon()[y*maxcols + x].top == DOOR_V_OPEN)) {
				// close the door
				cocos2d::experimental::AudioEngine::play2d("Door_Closed.mp3", false, 1.0f);

				// show the room the player is in
				revealRoom();

				// replace sprite with closed door
				//removeSprite(wall_sprites, maxrows, x, y);
				if (getDungeon()[y*maxcols + x].traptile == DOOR_H)
					addSprite(door_sprites, maxrows, x, y, -2, "Door_Horizontal_Closed_48x48.png");
				else
					addSprite(door_sprites, maxrows, x, y, -2, "Door_Vertical_Closed_48x48.png");

				getDungeon()[y*maxcols + x].top = (getDungeon()[y*maxcols + x].traptile == DOOR_H ? DOOR_H : DOOR_V);
				door->toggleOpen();
				door->toggleHeld();

				// if the room the player is now in isn't cleared, lock all doors
				if (!roomCleared()) {
					toggleDoorLocks(x, y, getDungeon()[y*maxcols + x].traptile);
				}
			}
			else if (door->isOpen() && door->isHeld()) {
				// release hold on the door
				door->toggleHeld();
			}
			door.reset();
		}
	}
}
bool ThirdFloor::roomCleared() {
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	// find the chunk the player is in
	int xchunk = px / 14;
	int ychunk = py / 10;

	int mx, my, mxchunk, mychunk;
	for (int i = 0; i < getMonsters().size(); i++) {
		mx = getMonsters().at(i)->getPosX(); mxchunk = mx / 14;
		my = getMonsters().at(i)->getPosY(); mychunk = my / 10;

		if (mxchunk == xchunk && mychunk == ychunk) {
			return false;
		}
	}

	return true;
}
void ThirdFloor::toggleDoorLocks(int dx, int dy, char doortype) {
	int rows = getRows();
	int cols = getCols();

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
			for (int i = 0; i < getDoors().size(); i++) {
				shared_ptr<Door> door = getDoors().at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx - 14 && y == dy) || (x == dx && y == dy)) {
					door->toggleLock();
					m_maze3[y*cols + x].top = (door->isLocked() ? DOOR_V_LOCKED : DOOR_V);
				}
				else if ((x == dx - 7 && y == dy - 5) || (x == dx - 7 && y == dy + 5)) {
					door->toggleLock();
					m_maze3[y*cols + x].top = (door->isLocked() ? DOOR_H_LOCKED : DOOR_H);
				}
				door.reset();
			}

		}
		// else if player is to the right of the given door
		else {
			for (int i = 0; i < getDoors().size(); i++) {
				shared_ptr<Door> door = getDoors().at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx + 14 && y == dy) || (x == dx && y == dy)) {
					door->toggleLock();
					m_maze3[y*cols + x].top = (door->isLocked() ? DOOR_V_LOCKED : DOOR_V);
				}
				else if ((x == dx + 7 && y == dy - 5) || (x == dx + 7 && y == dy + 5)) {
					door->toggleLock();
					m_maze3[y*cols + x].top = (door->isLocked() ? DOOR_H_LOCKED : DOOR_H);
				}
				door.reset();
			}
		}
	}
	// door is oriented horizontally
	else {
		// if player is above the given door
		if (py < dy) {
			// find the doors to lock/unlock
			for (int i = 0; i < getDoors().size(); i++) {
				shared_ptr<Door> door = getDoors().at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx && y == dy - 10) || (x == dx && y == dy)) {
					door->toggleLock();
					m_maze3[y*cols + x].top = (door->isLocked() ? DOOR_H_LOCKED : DOOR_H);
				}
				else if ((x == dx - 7 && y == dy - 5) || (x == dx + 7 && y == dy - 5)) {
					door->toggleLock();
					m_maze3[y*cols + x].top = (door->isLocked() ? DOOR_V_LOCKED : DOOR_V);
				}
				door.reset();
			}

		}
		// else if player is below the given door
		else {
			for (int i = 0; i < getDoors().size(); i++) {
				shared_ptr<Door> door = getDoors().at(i);
				x = door->getPosX();
				y = door->getPosY();

				if ((x == dx && y == dy + 10) || (x == dx && y == dy)) {
					door->toggleLock();
					m_maze3[y*cols + x].top = (door->isLocked() ? DOOR_H_LOCKED : DOOR_H);
				}
				else if ((x == dx - 7 && y == dy + 5) || (x == dx + 7 && y == dy + 5)) {
					door->toggleLock();
					m_maze3[y*cols + x].top = (door->isLocked() ? DOOR_V_LOCKED : DOOR_V);
				}
				door.reset();
			}
		}
	}
}

void ThirdFloor::hideRooms() {
	// hide all elements
	for (int i = 0; i < wall_sprites.size(); i++) {
		wall_sprites[i]->setVisible(false);
	}
	for (int i = 0; i < getMonsters().size(); i++) {
		getMonsters()[i]->getSprite()->setVisible(false);
	}
	for (int i = 0; i < item_sprites.size(); i++) {
		item_sprites[i]->setVisible(false);
	}
	for (int i = 0; i < getTraps().size(); i++) {
		getTraps()[i]->getSprite()->setVisible(false);
	}
	/*for (auto &it : spinner_sprites) {
		it.second->setVisible(false);
	}
	for (auto &it : zapper_sprites) {
		it.second->setVisible(false);
	}*/

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
	for (int i = 0; i < getMonsters().size(); i++) {
		spritePoint = getMonsters()[i]->getSprite()->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			getMonsters()[i]->getSprite()->setVisible(true);
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
	for (int i = 0; i < getTraps().size(); i++) {
		spritePoint = getTraps()[i]->getSprite()->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			getTraps()[i]->getSprite()->setVisible(true);
		}
	}
	/*for (auto &it : spinner_sprites) {
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
	}*/
}

std::vector<_Tile>& ThirdFloor::getDungeon() {
	return m_maze3;
}
int ThirdFloor::getRows() const {
	return m_rows;
}
int ThirdFloor::getCols() const {
	return m_cols;
}


void ThirdFloor::TLChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{ { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
	  { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp, m_ds,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,  m_g,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
	  { wll,   sp,   sp, m_ds,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
	  { wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{	{ unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as,  i_b, t_as,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  m_k,  wll,  wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,  _dv},
		{ wll,  m_k,  wll,  wll,   sp,   sp,  wll,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_g,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp, i_lp,   sp,  wll,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_g,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, i_sc,  wll,  wll,  wll,  wll,  m_w,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, i_bc,   sp,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll, i_bc,  i_b, i_bc,  wll,  wll,  wll,   sp,   sp,  _dv},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_as,   sp,   sp,   sp, t_as,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  m_g,   sp,  wll,   sp,   sp,   sp,  wll,   sp, m_ds,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_as,   sp,   sp,   sp, t_as,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


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
	pickSingleChunk(current, pool);
}
void ThirdFloor::topChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_tr,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  t_f,   sp,  m_z,  wll,  m_z,   sp,   sp,  t_f,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_tl,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  t_f,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  t_f,  wll,  unb},
		{ wll, t_lv,  wll,   sp,  m_z,   sp,  wll,   sp,  m_z,   sp,  wll, t_lv,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  unb},
		{  sp,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,   sp,  _dv},
		{ wll,   sp,  m_z,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  m_z,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp, m_sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, m_sp,  wll,   sp,   sp,   sp,  wll, m_sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,  m_k,  wll,  m_g,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll, m_ds,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,  wll,  wll, t_as, t_as,  wll, m_ds,  wll,   sp,  wll,  unb},
		{  sp,   sp,   sp,  wll,  m_a,  wll, t_as, t_as,  wll,  wll,  wll,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, i_gc,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp, t_lv,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,  wll,  wll,   sp,  wll,  wll,   sp,  wll,  wll,  wll,  unb},
		{  sp,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,   sp,  _dv},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_g,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_k,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, m_ds,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as,   sp,   sp, m_ds, t_as,  wll, t_as, m_ds,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, m_ds,   sp, m_ds,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, t_as,   sp,   sp, t_as,   sp,   sp, t_as,   sp,   sp,  wll,  unb},
		{  sp, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,   sp,  _dv},
		{ wll,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> eight = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_g,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_a,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  unb},
		{  sp,   sp,  wll,   sp, m_sp,   sp,  wll,   sp, m_sp,   sp,  wll,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_a,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> nine = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, i_gc,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  unb,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb,  wll,  unb},
		{ wll,  wll,  unb,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb,  wll,  wll,  unb},
		{ wll,   sp,   sp,  unb,  wll,  wll,   sp,  wll,  wll,  unb,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  unb,  wll, t_ts,  wll,  unb,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,  wll, t_lv,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> ten = 
	{   { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll}, };

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

	mixChunks(current, pool);
}
std::vector<std::string> ThirdFloor::TRChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp, m_ds, m_ds, m_ds,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, m_ds, m_ds, m_ds,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_ts, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_as, t_ts,   sp,   sp,  m_z,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_ts, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,  m_k,   sp,   sp, t_ts, t_as, t_ts,   sp,   sp,   sp,  m_z,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_ts, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts, t_as, t_ts,   sp,   sp,  m_z,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_ts, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_k,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  m_a,  wll,  unb},
		{ wll,   sp,  m_w,   sp,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll, i_ar, t_lv,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll, t_lv,  wll,  wll,  m_w,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  unb,  unb,  m_w,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,  unb,  unb, m_sp,  unb,  unb,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  unb,  unb,  unb,  unb,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  unb,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  i_b, t_lv, t_tl,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

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

	pickSingleChunk(current, pool);

	return combineChunks(current);
}

void ThirdFloor::leftEdgeChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,   sp,   sp, m_rm,  wll,  wll,  unb},
		{ wll,  t_f,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll, i_lp, t_ts,   sp,   sp,   sp,  wll,   sp,   sp,   sp, m_gs,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,  t_f,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, m_ds,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_sk, t_cf, t_sk,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_cf, t_cf, t_cf,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,t_mbv,  wll, i_ri,  wll,t_mbv,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_cf, t_cf, t_cf,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_sk, t_cf, t_sk,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  m_g,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  m_a,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_lv, t_lv, t_lv, t_lv, t_lv,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_lv, t_lv, t_lv,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, t_lv,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,   sp, t_lv,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_a,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  m_w,  wll,   sp,   sp,   sp,   sp, m_br,  t_f,   sp,  wll,  unb},
		{ wll,  wll,  wll,  m_w, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll, m_ds,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll, i_hp,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> vi =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_sk,  m_g,   sp,   sp,   sp,   sp,   sp,  m_g, t_sk,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  t_s, t_cf,  t_s,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_sk,  m_g,   sp,   sp,   sp,   sp,   sp,  m_g, t_sk,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> vii =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> viii =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> ix =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> x =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


	pool.push_back(one);
	pool.push_back(two);
	pool.push_back(three);
	pool.push_back(four);
	pool.push_back(five);
	pool.push_back(vi);
	//pool.push_back(vii);
	//pool.push_back(eight);
	//pool.push_back(nine);
	//pool.push_back(ten);
	//pool.push_back(eleven);
	//pool.push_back(twelve);

	pickSingleChunk(current, pool);

}
void ThirdFloor::middleChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {
	
	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{  sp,   sp, m_br,   sp,   sp,   sp,  wll,   sp,   sp,   sp, m_fr,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, m_ds,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,   sp,   sp,  wll,  wll,  wll,   sp,  wll,  wll,  wll,   sp,  t_f,  wll,  unb},
		{ m_z,  t_f,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, t_ts,  m_r,  i_b,  m_r, t_ts,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  unb},
		{ m_z,   sp,  t_f,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,  unb},
		{ wll,   sp,   sp,  wll,  wll,  wll,   sp,  wll,  wll,  wll,   sp,  t_f,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  m_g,   sp,   sp,  wll,   sp,   sp,   sp,  m_z,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, m_ds,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_ts,   sp, t_ts,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as, t_ts, t_ts, t_ts, t_as, t_as, t_as, t_ts, t_ts, t_ts, t_as,  wll,  unb},
		{ wll, t_ts, t_ts, t_ts, t_as,  wll,  wll,  wll, t_as, t_ts, t_ts, t_ts,  wll,  unb},
		{ wll, t_ts, t_ts, t_as,  wll,   sp,  wll,  wll,  wll, t_as, t_ts, t_ts,  wll,  unb},
		{  sp, t_ts, t_ts, t_as,  wll,  wll,  m_w,  wll,  wll, t_as, t_ts, t_ts,   sp,  _dv},
		{ wll, t_ts, t_ts, t_as,  wll,  wll,  wll,  wll,  wll, t_as, t_ts, t_ts,  wll,  unb},
		{ wll, t_ts, t_ts, t_ts, t_as,  wll,  wll,  wll, t_as, t_ts, t_ts, t_ts,  wll,  unb},
		{ wll, t_as, t_ts, t_ts, t_ts, t_as, t_as, t_as, t_ts, t_ts, t_ts, t_as,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_ts, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts, t_ts,  wll,  unb},
		{ wll, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, m_ds,   sp,  t_f,   sp, m_ds,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts,  wll,  unb},
		{ wll, t_ts, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts, t_ts,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, m_ds,   sp,   sp,  wll,   sp,   sp, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  wll,  wll, i_ri,  wll,  wll,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp, m_ds,   sp,   sp,  wll,   sp,   sp, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll, m_ds,   sp,   sp,   sp,   sp,   sp, m_ds,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,  wll, t_df,  wll,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  m_a,   sp,   sp,   sp,   sp,   sp,  m_a,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll, m_ds,   sp,   sp,   sp, m_ds,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> eight = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp, t_as,  wll,  wll,  wll, t_as,   sp,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp, m_sp,   sp,  wll,  wll,  m_b,  wll,  wll,   sp, m_sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,  wll, m_gs,  wll,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,  m_w,   sp,   sp,   sp,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> nine = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as, t_as,  wll,  unb},
		{ wll,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  wll,  m_a,  wll,  wll,  wll,  m_a,  wll,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,  wll,  unb},
		{ wll, t_as, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> ten = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,  m_b,  wll,  wll,  wll,   sp,   sp,  wll,   sp,  wll,  unb},
		{  sp,   sp,  m_w,   sp,   sp,  wll,  i_b,  wll,   sp,   sp,  m_w,   sp,   sp,  _dv},
		{ wll,   sp,  wll,   sp,   sp,  wll,  wll,  wll,  m_b,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


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

	mixChunks(current, pool);
}
std::vector<std::string> ThirdFloor::rightEdgeChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool) {
	
	std::vector<std::vector<std::string>> one =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  m_w,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,   sp,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp, t_as, t_as,  wll,  unb},
		{ wll,   sp,  wll,  wll,   sp,  m_g, m_ds,  wll,  wll,  wll,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  wll,  wll, m_br, m_ds,  wll,   sp,  m_r,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  m_w,  wll, m_ds,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp, t_lv,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,   sp, t_lv, t_lv,  wll,  unb},
		{  sp,   sp, t_lv,   sp,   sp,  m_k,  m_w,  m_k,   sp,   sp, t_lv, t_lv,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,   sp, t_lv, t_lv,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp, t_lv,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  m_z,   sp,  wll, t_as,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{  sp,   sp, m_sp,  wll,   sp, t_as, m_sp, t_as,   sp,   sp,   sp, m_ds,  wll,  unb},
		{ wll,  wll, t_ts,  m_z,   sp,  wll, t_as,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  t_s,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  t_s,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,t_mbh,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,  t_s,   sp,   sp, m_ds,  wll,  m_k,   sp,   sp,  t_s,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp,t_mbh,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  t_s,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  t_s,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, t_ts,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp, t_ts, t_ts, t_ts, t_ts,  wll,  t_s, t_ts, t_ts, t_ts,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  m_r,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, t_ts,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


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

	pickSingleChunk(current, pool);

	return combineChunks(current);
}

void ThirdFloor::BLChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   {  unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,  wll,   sp,  wll,  wll, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,  wll,   sp,  wll,  wll,   sp,  wll,   sp,  wll,  unb},
		{ wll, t_tr, t_lv,   sp,   sp,   sp,  m_z,  wll,  wll,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,  wll,  wll,  m_z,   sp,   sp,  wll,   sp,   sp,  _dv},
		{ wll,  wll,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,   sp,  wll, m_ds,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  m_r,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_r,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  m_k, t_df,  m_k,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_r,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_r,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_g,  unb},
		{ wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp, t_as,   sp,  wll,  wll,  wll,  m_r,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,  wll,   sp,   sp,   sp,  wll,  wll,   sp,   sp,  wll,  wll,  _dv},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, i_ar,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  i_b,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  m_z,  wll,  m_z,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


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

	pickSingleChunk(current, pool);
}
void ThirdFloor::bottomChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {

	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as, t_as,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, m_sp,  wll, m_sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,  m_w,  wll,  wll,  wll,  m_w,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll, i_ri,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> two = { { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
								 { wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
								 { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 { wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
								 { wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 {  sp,   sp,   sp,   sp,   sp, m_ds,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,  _dv},
								 { wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 { wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
								 { wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
								 { wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  m_r,  unb},
		{ wll,   sp,  wll,   sp,   sp,  wll,   sp,  wll,  m_r,  wll,   sp,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,  m_r,   sp,  m_r,  wll,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,  wll,   sp,  wll,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  _dv},
		{ wll,  m_r,  wll,   sp,  wll,  m_r,   sp,  m_r,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_r,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  m_r,  wll,  unb}, };

	std::vector<std::vector<std::string>> four = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{  sp,   sp,   sp,  t_f, t_lv, t_as, t_lv, t_as, t_lv,  t_f,   sp,   sp,   sp,  _dv},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll, t_as,   sp, t_as,   sp, t_as,  m_b, t_as,   sp, t_as,   sp, t_as,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> five =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  unb},
		{  sp,   sp,  wll,   sp,   sp,   sp,   sp, t_lv, t_tl,  wll,   sp,   sp,   sp,  _dv},
		{ wll,  wll,  wll,  wll,   sp,  unb,  unb,  wll,  wll,  wll,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  unb,  wll,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll, i_sc,  wll,  wll,  wll,  unb, i_lp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,  wll,  wll,  unb,  wll,  wll,  wll, t_df,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> six = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll, t_ts,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_ts,   sp,  wll,  unb},
		{ wll,   sp,   sp,  wll,   sp,  wll,  wll,  wll,   sp,  wll,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, t_ts,  wll, i_ar,  wll, t_ts,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp, m_ds,  wll, m_ds,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  m_k,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_g,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> seven =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp, t_lv,   sp,   sp,   sp, t_lv,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_lv,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  m_w,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,  m_w,  wll,  m_w,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,  wll,   sp,  m_w,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp, t_lv,   sp,   sp,   sp,  wll,   sp,   sp,   sp, t_lv,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> eight =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp, m_ds,  wll, m_ds,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp, m_ds,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, t_as,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp, t_as,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


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

	mixChunks(current, pool);

	//return combineChunks(c);
}
std::vector<std::string> ThirdFloor::BRChunks(std::vector<std::vector<std::vector<std::string>>> current, std::vector<std::vector<std::vector<std::string>>> pool) {
	
	std::vector<std::vector<std::string>> one = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_w,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,  wll,   sp,  wll,   sp,   sp,  wll,   sp, i_gd,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  wll, i_ri,  unb}, };

	std::vector<std::vector<std::string>> two = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  t_s,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  t_s,   sp,   sp,   sp,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,  m_z,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  m_k,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> three = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };


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

	pickSingleChunk(current, pool);

	return combineChunks(current);
}

std::vector<std::vector<std::string>> ThirdFloor::playerChunks() {

	std::vector<std::vector<std::string>> TLplayerchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, _ply,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> TRplayerchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp, _ply,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BLplayerchunk =
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  wll,   sp,   sp,   sp, _ply,   sp,   sp,   sp, t_as,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,  wll,   sp,   sp,  wll,   sp,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp,  wll,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BRplayerchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp, t_as, _ply, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp, t_as, t_as, t_as,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	switch (m_playerchunk) {
	case 1: return TLplayerchunk;
	case 2: return TRplayerchunk;
	case 3: return BLplayerchunk;
	case 4: return BRplayerchunk;
	default: return TLplayerchunk;
	}
}
std::vector<std::vector<std::string>> ThirdFloor::exitChunks() {

	std::vector<std::vector<std::string>> TLexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, _ext,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,   sp,   sp,  wll,   sp,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> TRexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp,   sp,   sp, _ext,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BLexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  unb,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,  unb,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll, i_rc,   sp,  wll,  wll,   sp, _ext,   sp,   sp,   sp,   sp,   sp,   sp,  _dv},
		{ wll,   sp,   sp,  unb,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,  unb,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  unb,   sp,   sp,   sp,  wll,  wll,  wll,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  unb,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	std::vector<std::vector<std::string>> BRexitchunk = 
	{   { unb,  unb,  unb,  unb,  unb,  unb,  _dh,  unb,  unb,  unb,  unb,  unb,  unb,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,   sp,  wll,  wll,  wll,  wll,  wll,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, t_as,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{  sp,   sp,   sp,   sp, t_as, t_as, _ext, t_as, t_as,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,  wll, t_as,  wll,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,   sp,   sp,   sp,   sp,   sp, t_as,   sp,   sp,   sp,   sp,   sp,  wll,  unb},
		{ wll,  wll,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,   sp,  wll,  wll,  unb},
		{ wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  wll,  unb}, };

	switch (m_exitchunk) {
	case 1: return TLexitchunk;
	case 2: return TRexitchunk;
	case 3: return BLexitchunk;
	case 4: return BRexitchunk;
	default: return TLexitchunk;
	}
}

void ThirdFloor::mixChunks(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {
	for (int i = 1; i < (getCols() - 2) / 13 - 1; i++) {
		current.push_back(pool[randInt(pool.size())]);
	}
}
void ThirdFloor::pickSingleChunk(std::vector<std::vector<std::vector<std::string>>> &current, std::vector<std::vector<std::vector<std::string>>> pool) {
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

std::vector<std::string> ThirdFloor::generateLevel() {
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
	std::vector<std::vector<std::vector<std::string>>> c, pool;
	std::vector<std::string> v, finalvec;

	int count = 0;
	while (count < (getRows() - 2) / 9) {
		if (count == 0) {
			TLChunks(c, pool); pool.clear(); m_layer++;
			topChunks(c, pool); pool.clear();
			v = TRChunks(c, pool); pool.clear();
		}
		else if (count == 1 || count == 2) {
			leftEdgeChunks(c, pool); pool.clear();
			middleChunks(c, pool); pool.clear();
			v = rightEdgeChunks(c, pool); pool.clear();
		}
		else if (count == 3) {
			m_layer++;
			BLChunks(c, pool); pool.clear(); m_layer++;
			bottomChunks(c, pool); pool.clear();
			v = BRChunks(c, pool); pool.clear();
		}

		for (int i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i));

		c.clear();
		count++;
	}

	return finalvec;
}
std::vector<std::string> ThirdFloor::combineChunks(std::vector<std::vector<std::vector<std::string>>> c) {
	std::vector<std::string> v;
	for (int i = 0; i < 10; i++) {			// iterate thru rows of 2d vector
		for (int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 14; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}


// ================================================
//				:::: BOSS FLOOR ::::
// ================================================
FirstBoss::FirstBoss(Player p) : Dungeon(5) {
	int rows = getRows();
	int cols = getCols();

	_Tile *tile;
	int i, j;

	for (i = 0; i < BOSSROWS; i++) { //		initialize entire dungeon to blank space
		for (j = 0; j < cols; j++) {
			tile = new _Tile;

			// all layers set to empty space
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = SPACE;
			tile->wall = tile->item = tile->enemy = tile->trap = tile->exit = tile->overlap = tile->enemy_overlap = tile->marked = false;
			tile->npc = tile->hero = false;
			tile->gold = 0;
			tile->overlap_count = tile->enemy_overlap_count = 0;

			m_boss.push_back(*tile);
			delete tile;
		}
	}

	for (i = 0; i < BOSSROWS; i++) { //	initialize top and bottom of dungeon to be unbreakable walls
		for (j = 0; j < cols; j++) {
			m_boss[i*cols + j].top = UNBREAKABLE_WALL;
			m_boss[i*cols + j].bottom = UNBREAKABLE_WALL;
			m_boss[i*cols + j].traptile = UNBREAKABLE_WALL;
			m_boss[i*cols + j].wall = true;
			m_boss[i*cols + j].boundary = true;
		}
		i += (BOSSROWS - 2);
	}

	for (i = 1; i < BOSSROWS - 1; i++) { //		initialize edges of dungeon to be unbreakable walls
		m_boss[i*cols].top = m_boss[i*cols].bottom = m_boss[i*cols].traptile = UNBREAKABLE_WALL;
		m_boss[i*cols + cols - 1].top = m_boss[i*cols + cols-1].bottom = m_boss[i*cols + cols - 1].traptile = UNBREAKABLE_WALL;
	
		m_boss[i*cols].wall = true;
		m_boss[i*cols + cols - 1].wall = true;
		m_boss[i*cols].boundary = true;
		m_boss[i*cols + cols - 1].boundary = true;
	}


	//	spawn player
	player.at(0) = p;
	player.at(0).setPosX(cols / 2);
	player.at(0).setPosY(BOSSROWS - 3);
	m_boss[player.at(0).getPosY()*cols + player.at(0).getPosX()].hero = true;


	//	spawn boss
	Smasher smasher;
	int col = smasher.getPosX();
	int row = smasher.getPosY();
	for (int i = row - 1; i < row + 2; i++) {
		for (int j = col - 1; j < col + 2; j++) {
			m_boss[i*cols + j].upper = SMASHER;
			m_boss[i*cols + j].enemy = true;
		}
	}
	getMonsters().emplace_back(new Smasher(smasher));

	
	/*
	// test enemies
	int n = 60;
	int mx, my;
	char top, upper;
	while (n > 0) {
		Archer archer;
		mx = 1 + randInt(cols - 2);
		my = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
		archer.setPosX(mx);
		archer.setPosY(my);

		top = m_boss[archer.getPosY()*cols + archer.getPosX()].top;
		upper = m_boss[archer.getPosY()*cols + archer.getPosX()].upper;
		while (top != SPACE || upper == SMASHER) { 
			mx = 1 + randInt(cols - 2);
			my = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
			archer.setPosX(mx);
			archer.setPosY(my);

			top = m_boss[archer.getPosY()*cols + archer.getPosX()].top;
			upper = m_boss[archer.getPosY()*cols + archer.getPosX()].upper;
		}

		m_firstbossMonsters.emplace_back(new Archer(archer));
		m_boss[archer.getPosY()*cols + archer.getPosX()].top = ARCHER;
		m_boss[archer.getPosY()*cols + archer.getPosX()].enemy = true;
		n--;
	}
	*/

	//	spawn spike traps
	int x = 1 + randInt(cols - 2);
	int y = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
	int speed = 3 + randInt(3);
	char traptile;

	int m = 12 + randInt(5);
	while (m > 0) {
		SpikeTrap spiketrap(x, y, speed);

		traptile = m_boss[spiketrap.getPosY()*cols + spiketrap.getPosX()].traptile;
		while (traptile != SPACE) { // while spiketrap position clashes with wall, player, or idol
			x = 1 + randInt(cols - 2);
			y = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
			speed = 3 + randInt(3);
			spiketrap.setPosX(x);				// reroll it
			spiketrap.setPosY(y);
			spiketrap.setSpeed(speed);
			spiketrap.setCountdown(speed);

			traptile = m_boss[spiketrap.getPosY()*cols + spiketrap.getPosX()].traptile;
		}

		//m_firstbossActives.emplace_back(new SpikeTrap(spiketrap));
		m_traps.emplace_back(new SpikeTrap(spiketrap));

		m_boss[spiketrap.getPosY()*cols + spiketrap.getPosX()].traptile = SPIKETRAP_DEACTIVE;
		m_boss[spiketrap.getPosY()*cols + spiketrap.getPosX()].trap = true;
		m--;
	}
}
FirstBoss& FirstBoss::operator=(FirstBoss const &dungeon) {
	for (int i = 0; i < dungeon.getRows()*dungeon.getCols(); i++) {
		m_boss[i] = dungeon.m_boss[i];
	}
	player = dungeon.player;
	m_level = dungeon.getLevel();

	m_npcs = dungeon.m_npcs;
	m_monsters = dungeon.m_monsters;
	//m_firstbossMonsters = dungeon.m_firstbossMonsters;
	//m_firstbossActives = dungeon.m_firstbossActives;
	m_traps = dungeon.m_traps;

	return *this;
}

void FirstBoss::peekDungeon(int x, int y, char move) {
	int rows = getRows();
	int cols = getCols();

	char upper, top, bottom, traptile;
	bool boundary, wall, item, trap, enemy;

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

	// set player's action key (for items that have casting time)
	player.at(0).setAction(move);

	// red tint if player has bloodlust
	if (player.at(0).hasBloodlust()) {
		bloodlustTint(player.at(0));
	}

	// check for any afflictions
	player.at(0).checkAfflictions();

	// :::: Move player ::::
	int weprange = player.at(0).getWeapon()->getRange();
	bool moveUsed = false;

	// if player is not immobilized, move them
	if (!(player.at(0).isStunned() || player.at(0).isFrozen())) {

		// check for special weapon attack patterns
		if (player.at(0).getWeapon()->hasAttackPattern()) {
			player.at(0).getWeapon()->usePattern(*this, moveUsed);
		}

		if (!moveUsed) {

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

				top = m_boss[(y + m)*cols + (x + n)].top;
				bottom = m_boss[(y + m)*cols + (x + n)].bottom;
				traptile = m_boss[(y + m)*cols + (x + n)].traptile;

				boundary = m_boss[(y + m)*cols + (x + n)].boundary;
				wall = (player.at(0).isEthereal() ? false : m_boss[(y + m)*cols + (x + n)].wall);
				item = m_boss[(y + m)*cols + (x + n)].item;
				trap = m_boss[(y + m)*cols + (x + n)].trap;
				enemy = m_boss[(y + m)*cols + (x + n)].enemy;

				if (!(wall || enemy || boundary)) {
					m_boss[y*cols + x].hero = false;
					m_boss[(y + m)*cols + (x + n)].hero = true;
					player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);

					if (trap) {
						trapEncounter(x + n, y + m);
					}
				}
				else if (wall && !boundary) {
					if (top == DOOR_H || top == DOOR_V) {
						openDoor(x + n, y + m);
					}
					else if (top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
						m_boss[y*cols + x].hero = false;
						m_boss[(y + m)*cols + (x + n)].hero = true;
						player.at(0).setPosX(x + n); player.at(0).setPosY(y + m);
					}
					else if (top == DOOR_H_LOCKED || top == DOOR_V_LOCKED) {
						// play locked door sound
						cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
					}
				}
				else if (enemy) {	// monster encountered
					fight(x + n, y + m);
				}
			}

			/*
			if (move == 'l') { // moved to the left
				upper = m_boss[y*cols + x - 1].upper;
				top = m_boss[y*cols + x - 1].top;
				bottom = m_boss[y*cols + x - 1].bottom;
				traptile = m_boss[y*cols + x - 1].traptile;

				wall = m_boss[y*cols + x - 1].wall;
				item = m_boss[y*cols + x - 1].item;
				enemy = m_boss[y*cols + x - 1].enemy;
				trap = m_boss[y*cols + x - 1].trap;

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
			*/

			else if (move == 'e') {
				collectItem(*this, x, y);
			}

			else if (move == 'b') {
				if (player.at(0).hasShield()) {
					// play setting up shield stance sound effect
					cocos2d::experimental::AudioEngine::play2d("Shield_Stance.mp3", false, 1.0f);

					player.at(0).setBlock(true);
				}
			}

		}

	}

	// check active items in play
	checkActive();

	if (player.at(0).getHP() <= 0) {
		
		if (!dungeonText.empty())
			showText();
		return;
	}

	// if all monsters are dead?
	if (getMonsters().empty()) {
		// queue player move to the front of the spawn
		if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
			queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY(), .1f, true);
		}

		// run actions
		runSpawn();

		//if (!dungeonText.empty())
			//showText();

		return;
	}

	// move any monsters
	int mx, my, mInitHP;
	if (getMonsters().size() > 1) {
		for (unsigned i = 1; i < getMonsters().size(); i++) {
			mInitHP = getMonsters().at(i)->getHP();
			mx = getMonsters().at(i)->getPosX();
			my = getMonsters().at(i)->getPosY();

			// check for any afflictions
			getMonsters().at(i)->checkAfflictions();

			// check if an affliction killed the monster
			if (getMonsters().at(i)->getHP() <= 0) {
				monsterDeath(i);
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

				// move the monster
				getMonsters().at(i)->move(*getMonsters().at(i), *this);

				// flash monster sprite if damage was taken
				if (getMonsters().at(i)->getHP() < mInitHP) {
					runMonsterDamage(getMonsters().at(i)->getSprite());
				}

				//	if player is dead then break
				if (player.at(0).getHP() <= 0) {
					player.at(0).setDeath(getMonsters().at(i)->getName());
					break;
				}
			}
		}
	}

	//	check if smasher is dead
	if (getMonsters().at(0)->getName() != "Smasher") {
		// queue player move to the front of the spawn
		if (player.at(0).getPosX() != x || player.at(0).getPosY() != y) {
			queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY(), .1f, true);
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
		queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY(), .1, true);
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
		player.at(0).setDeath(getMonsters().at(0)->getName());
		return;
	}

	//if (!dungeonText.empty())
		//showText();
}

void FirstBoss::checkSmasher() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));

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
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
	int randmove = 1 + randInt(100);


	// 12% chance : smash attack
	if (randmove > 0 && randmove <= 12) {
		smasher->setActive(true);
		smasher->setMove(1);
		getMonsters().at(0) = smasher;
		move1();
	}
	// 15% chance : avalanche attack
	else if (randmove > 12 && randmove <= 27) {
		smasher->setActive(true);
		smasher->setMove(2);
		getMonsters().at(0) = smasher;
		move2();
	}
	// 3% chance : archer/enemy spawning, max 3 at once
	else if (randmove > 27 && randmove <= 30) {
		if (getMonsters().size() < 3)
			move3();
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
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();

	//	if wall immediately beneath smasher is an unbreakable wall, reset position to top
	if (m_boss[(row+2)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		smasher->setEnded(false);
		smasher->setActive(false);
		getMonsters().at(0) = smasher;
		resetUpward();

		smasher.reset();
		return;
	}

	moveSmasherD();
	smasher.reset();
}
void FirstBoss::move2() { // starts avalanche
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();

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
			getMonsters().at(0) = smasher;
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
			getMonsters().at(0) = smasher;
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
			getMonsters().at(0) = smasher;
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
	//shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
	int x, y, speed, spikecount;
	char traptile;
	spikecount = 15 + randInt(8);
	std::shared_ptr<FallingSpike> spike = nullptr;
	
	for (int i = 0; i < spikecount; i++) {
		x = 1 + randInt(BOSSCOLS - 2);
		y = 1 + randInt(BOSSROWS / 3);
		speed = 1 + randInt(3);
		traptile = m_boss[y*BOSSCOLS + x].traptile;
		spike = std::make_shared<FallingSpike>(x, y, speed);
		//FallingSpike spike(x, y, speed);

		while (traptile != SPACE || x == col || x == col-1 || x == col+1) { // while spike position clashes with anything
			x = 1 + randInt(BOSSCOLS - 2);
			y = 1 + randInt(BOSSROWS / 3);
			spike->setPosX(x);	// reroll it
			spike->setPosY(y);
			
			traptile = m_boss[spike->getPosY()*BOSSCOLS + spike->getPosX()].traptile;
		}
		// add spike sprite
		//spike->setSprite(createSprite(spike->getPosX(), spike->getPosY(), 0, "CeilingSpike_48x48.png"));
		addSprite(spike_sprites, getRows(), spike->getPosX(), spike->getPosY(), 0, "CeilingSpike_48x48.png");

		m_traps.emplace_back(spike);
		//m_traps.emplace_back(new FallingSpike(*spike));
		m_boss[spike->getPosY()*BOSSCOLS + spike->getPosX()].trap = true;
		m_boss[spike->getPosY()*BOSSCOLS + spike->getPosX()].traptile = c_SPIKE;
		spike.reset();
	}
	//smasher.reset();
}
void FirstBoss::move3() {	// spawn two archers
	int n = 2;
	int x, y;
	char top, hero;

	if (getMonsters().size() <= 2) {
		while (n > 0) {
			x = 1 + randInt(BOSSCOLS - 2);
			y = BOSSROWS - 2 - randInt(10);

			Archer archer(x, y);
			//archer.setPosX(x);
			//archer.setPosY(y);

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
			archer.setSprite(createSprite(archer.getPosX(), archer.getPosY(), 1, "Archer_48x48.png"));

			getMonsters().emplace_back(new Archer(archer));
			m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].top = c_ARCHER;
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
	//shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
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
				getMonsters().at(0)->encounter(player.at(0), *(getMonsters().at(0)), dungeonText);
			}
			// if there are any monsters in the way, crush them
			if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {
				int pos = findMonster(i, j);
				getMonsters().at(0)->encounter(*(getMonsters().at(0)), *(getMonsters().at(pos)));

				// check if monster died
				if (getMonsters().at(pos)->getHP() <= 0) {
					monsterDeath(pos);
				}
			}

			m_boss[j*BOSSCOLS + i].upper = SMASHER;
			m_boss[j*BOSSCOLS + i].enemy = true;
		}

		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'l');
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'l');
		getMonsters().at(0)->setPosX(col - 1);
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
					getMonsters().at(0)->encounter(player.at(0), *(getMonsters().at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them
				if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {					
					int pos = findMonster(i, j);
					getMonsters().at(0)->encounter(*(getMonsters().at(0)), *(getMonsters().at(pos)));
					
					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != col - 3 && m_boss[j*BOSSCOLS + (i - 1)].enemy) {
						int n = findMonster(i - 1, j);
						
						monsterDeath(n);
					}

					// move the monster and sprite
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', getMonsters(), col - 3, 0);

					// check if monster died
					if (getMonsters().at(pos)->getHP() <= 0) {
						monsterDeath(pos);
					}
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;

			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'L');
		getMonsters().at(0)->setPosX(col - 2);
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
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i - 1, j, 'l', getMonsters());
					}
					if (m_boss[j*BOSSCOLS + i - 2].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i - 2, j, 'l', getMonsters());
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
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i - 1, j, 'l', getMonsters());
					}
					if (i != col - 3 && m_boss[j*BOSSCOLS + i - 2].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i - 2, j, 'l', getMonsters());
					}

					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', getMonsters(), col - 4, 0);
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;
			}
		}
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'L');
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'L');
		getMonsters().at(0)->setPosX(col - 2);
	}

	//smasher.reset();
}
void FirstBoss::moveSmasherR() {
	//shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
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
				getMonsters().at(0)->encounter(player.at(0), *(getMonsters().at(0)), dungeonText);
			}
			// if there are any monsters in the way, crush them
			if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {
				int pos = findMonster(i, j);
				getMonsters().at(0)->encounter(*(getMonsters().at(0)), *(getMonsters().at(pos)));

				// check if monster died
				if (getMonsters().at(pos)->getHP() <= 0) {
					monsterDeath(pos);
				}
			}

			m_boss[j*BOSSCOLS + i].upper = SMASHER;
			m_boss[j*BOSSCOLS + i].enemy = true;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'r');
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'r');
		getMonsters().at(0)->setPosX(col + 1);
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
					getMonsters().at(0)->encounter(player.at(0), *(getMonsters().at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them
				if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {
					// crush the monster
					int pos = findMonster(i, j);
					getMonsters().at(0)->encounter(*(getMonsters().at(0)), *(getMonsters().at(pos)));
					
					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != col + 3 && m_boss[j*BOSSCOLS + (i + 1)].enemy) {
						int n = findMonster(i + 1, j);
						
						monsterDeath(n);
					}

					// move the sprite
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', getMonsters(), col + 3, 0);

					// check if monster died (should be dead)
					if (getMonsters().at(pos)->getHP() <= 0) {
						monsterDeath(pos);
					}
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;

			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);

		queueMoveSprite(getMonsters().at(0)->getSprite(), 'R');
		getMonsters().at(0)->setPosX(col + 2);
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
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i + 1, j, 'r', getMonsters());
					}
					if (m_boss[j*BOSSCOLS + i + 2].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i + 2, j, 'r', getMonsters());
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
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i + 1, j, 'r', getMonsters());
					}
					if (i != col + 3 && m_boss[j*BOSSCOLS + i + 2].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, i + 2, j, 'r', getMonsters());
					}
					
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', getMonsters(), col + 4, 0);
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;
			}
		}
		
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'R');
		getMonsters().at(0)->setPosX(col + 2);
	}

	//smasher.reset();
}
void FirstBoss::moveSmasherU() {
	//shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
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
				getMonsters().at(0)->encounter(player.at(0), *(getMonsters().at(0)), dungeonText);
			}
			// if there are any monsters in the way, crush them
			if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
				// find and crush the monster
				int pos = findMonster(j, i);
				getMonsters().at(0)->encounter(*(getMonsters().at(0)), *(getMonsters().at(pos)));

				// check if monster died
				if (getMonsters().at(pos)->getHP() <= 0) {
					monsterDeath(pos);
				}
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'u');
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'u');
		getMonsters().at(0)->setPosY(row - 1);
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
					getMonsters().at(0)->encounter(player.at(0), *(getMonsters().at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them
				if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
					// find and crush the monster
					int pos = findMonster(j, i);
					getMonsters().at(0)->encounter(*(getMonsters().at(0)), *(getMonsters().at(pos)));

					// if there are two enemies stacked, then kill the enemy closest to the wall first
					// :::: This is for the purpose of looking more natural on screen.
					if (i != 1 && m_boss[(i - 1)*BOSSCOLS + j].enemy) {
						int n = findMonster(j, i - 1);
						
						monsterDeath(n);
					}

					// move the monster
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', getMonsters(), 0, row - 3);

					// check if monster died
					if (getMonsters().at(pos)->getHP() <= 0) {
						monsterDeath(pos);
					}
					
				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;

			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		//moveSprite(monster_sprites, BOSSROWS, col, row, 'U');
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'U');
		getMonsters().at(0)->setPosY(row - 2);
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
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i-1, 'u', getMonsters());
					}
					if (m_boss[(i - 2)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 2, 'u', getMonsters());
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
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 1, 'u', getMonsters());
					}
					if (i != 1 && m_boss[(i - 2)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 2, 'u', getMonsters());
					}
					
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', getMonsters(), 0, row - 4);
				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'U');
		getMonsters().at(0)->setPosY(row - 2);
	}
	
	//smasher.reset();
}
void FirstBoss::moveSmasherD() {
	//shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
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
				getMonsters().at(0)->encounter(player.at(0), *(getMonsters().at(0)), dungeonText);
			}
			
			// if there are any monsters in the way, crush them
			if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {

				int pos = findMonster(j, i);
				getMonsters().at(0)->encounter(*(getMonsters().at(0)), *(getMonsters().at(pos)));
					
				// check if monster died
				if (getMonsters().at(pos)->getHP() <= 0) {
					monsterDeath(pos);
				}
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'd');
		getMonsters().at(0)->setPosY(row + 1);
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
					getMonsters().at(0)->encounter(player.at(0), *(getMonsters().at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them dead
				if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
					// smasher attacking a monster is a formality at this point
					int pos = findMonster(j, i);
					getMonsters().at(0)->encounter(*(getMonsters().at(0)), *(getMonsters().at(pos)));
					
					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != row + 3 && m_boss[(i + 1)*BOSSCOLS + j].enemy) {
						int n = findMonster(j, i + 1);
						
						monsterDeath(n);
					}

					// NOTE: this must be after the attack, otherwise pos is then incorrect
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', getMonsters(), 0, BOSSROWS - 2);

					// check if monster died (monster will be dead)
					if (getMonsters().at(pos)->getHP() <= 0) {
						monsterDeath(pos);
					}

				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'D');
		getMonsters().at(0)->setPosY(row + 2);
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
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 1, 'd', getMonsters());
					}
					if (m_boss[(i + 2)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 2, 'd', getMonsters());
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
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 1, 'd', getMonsters());
					}
					if (i != row + 3 && m_boss[(i + 2)*BOSSCOLS + j].enemy) {
						pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 2, 'd', getMonsters());
					}
					
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', getMonsters(), 0, row + 4);
				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		
		queueMoveSprite(getMonsters().at(0)->getSprite(), 'D');
		getMonsters().at(0)->setPosY(row + 2);
	}
	
	//smasher.reset();
}

void FirstBoss::resetLeftward() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
	int i, j;
	int rand = randInt(3);

	//	if smasher has reset far enough to the left, end cooldown
	if (col < BOSSCOLS / 1.5 - rand) {
		smasher->setEnded(true);
		getMonsters().at(0) = smasher;
		smasher.reset();
		move = '0';

		return;
	}

	moveSmasherL();
	smasher.reset();
}
void FirstBoss::resetRightward() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
	int i, j;
	int rand = randInt(3);

	//	if smasher has reset far enough to the right, end cooldown
	if (col > BOSSCOLS / 3 + rand) {
		smasher->setEnded(true);
		getMonsters().at(0) = smasher;
		smasher.reset();
		move = '0';

		return;
	}

	moveSmasherR();
	smasher.reset();
}
void FirstBoss::resetUpward() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
	int i, j;
	int rand = randInt(3);

	if (row < 6 + rand) {
		smasher->setEnded(true);
		getMonsters().at(0) = smasher;
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
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 1, 'u', getMonsters());
				}
				if (m_boss[(std::max(i - 2, 1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 2, 'u', getMonsters());
				}
				if (m_boss[(std::max(i - 3, 1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 3, 'u', getMonsters());
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
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 1, 'u', getMonsters());
				}
				if (m_boss[(std::max(i - 2, 1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 2, 'u', getMonsters());
				}
				if (m_boss[(std::max(i - 3, 1))*BOSSCOLS + j].enemy) {
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 3, 'u', getMonsters());
				}
				
				pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', getMonsters(), 0, row - 5);				
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
	}

	queueMoveSprite(getMonsters().at(0)->getSprite(), '^');
	getMonsters().at(0)->setPosY(row - 3);
	smasher.reset();
}
void FirstBoss::resetDownward() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(getMonsters().at(0));
	int col = getMonsters().at(0)->getPosX();
	int row = getMonsters().at(0)->getPosY();
	int i, j;
	int rand = randInt(3);

	if (row > 6 + rand) {
		smasher->setEnded(true);
		getMonsters().at(0) = smasher;
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
					dungeon[my*maxcols + mx].top = c_ARCHER;
					dungeon[my*maxcols + mx].enemy = true;
					monsters[i]->setPosX(mx);
					monsters[i]->setPosY(my);


					queueMoveSprite(monsters.at(i)->getSprite(), mx, my);
				}
				else {
					monsterDeath(i);
				}
				break;
			}
				//	CUSTOM MOVEMENT CASE in X direction
			case 'X': {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;

				dungeon[my*maxcols + cx].top = c_ARCHER;
				dungeon[my*maxcols + cx].enemy = true;
				
				monsters[i]->setPosX(cx);
				queueMoveSprite(monsters.at(i)->getSprite(), cx, my);
				break;
			}
				//	CUSTOM MOVEMENT CASE in Y direction
			case 'Y': {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;

				dungeon[(cy)*maxcols + mx].top = c_ARCHER;
				dungeon[(cy)*maxcols + mx].enemy = true;
				
				monsters[i]->setPosY(cy);
				queueMoveSprite(monsters.at(i)->getSprite(), mx, cy);
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
		queueMoveSprite(player.at(0).getSprite(), player.at(0).getPosX(), player.at(0).getPosY());
	}
	else {
		getMonsters().at(0)->encounter(player.at(0), *(getMonsters().at(0)), dungeonText);
	}
}

void FirstBoss::fight(int x, int y) { // monster's coordinates
	
	// Check if monster encountered was smasher
	if (m_boss[y*BOSSCOLS + x].upper == SMASHER) {
		//player.at(0).attack(getMonsters(), m_firstbossActives, 0, dungeonText);
		player.at(0).attack(*this, *getMonsters().at(0));

		if (getMonsters().at(0)->getHP() <= 0)
			monsterDeath(0);

		return;
	}

	int i = findMonster(x, y);

	// if monster wasn't found, return
	if (i == -1)
		return;
	
	//player.at(0).attack(getMonsters(), m_firstbossActives, i, dungeonText);
	player.at(0).attack(*this, *getMonsters().at(i));

	if (getMonsters().at(i)->getHP() <= 0) {
		monsterDeath(i);
	}
}
void FirstBoss::monsterDeath(int pos) {
	if (pos != 0) {
		getMonsters().at(pos)->death(*getMonsters().at(pos), *this);

		// remove them from the monster vector
		getMonsters().erase(getMonsters().begin() + pos);

		return;
	}


	int rows = getRows();
	int cols = getCols();

	int x = getMonsters().at(pos)->getPosX();
	int y = getMonsters().at(pos)->getPosY();

	// pause current music, play victory music, and then resume music once finished
	cocos2d::experimental::AudioEngine::pauseAll();
	int id = cocos2d::experimental::AudioEngine::play2d("Victory! All Clear.mp3", false, 1.0f);

	cocos2d::experimental::AudioEngine::setFinishCallback(id,
		[](int id, std::string music) {
		cocos2d::experimental::AudioEngine::resumeAll();
	});

	removeSprite(monster_sprites, BOSSROWS, x, y);

	string boss = getMonsters().at(pos)->getName();
	//dungeonText.push_back(boss + " was slain!\n");
	//dungeonText.push_back("Well done!\n");

	Idol idol;
	idol.setPosX(cols / 2);
	idol.setPosY(3);
	m_boss[idol.getPosY()*cols + idol.getPosX()].bottom = IDOL;
	m_boss[idol.getPosY()*cols + idol.getPosX()].item = true;

	// sprite for idol
	addSprite(item_sprites, BOSSROWS, idol.getPosX(), idol.getPosY(), -1, "Freeze_Spell_48x48.png");

	for (int i = y - 1; i < y + 2; i++) {
		for (int j = x - 1; j < x + 2; j++) {
			m_boss[i*cols + j].upper = SPACE;
			m_boss[i*cols + j].enemy = false;
		}
	}

	getMonsters().erase(getMonsters().begin() + pos);
}

std::vector<_Tile>& FirstBoss::getDungeon() {
	return m_boss;
}
int FirstBoss::getRows() const {
	return m_rows;
}
int FirstBoss::getCols() const {
	return m_cols;
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
	auto tintRed = cocos2d::TintTo::create(0.1f, cocos2d::Color3B(255, 70, 70));
	auto tintNormal = cocos2d::TintTo::create(0.1f, cocos2d::Color3B(255, 255, 255));
	sprite->runAction(cocos2d::Blink::create(0.2f, 4));
	sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintRed, tintNormal));
}
void runMonsterDamage(cocos2d::Sprite* sprite) {
	auto tintRed = cocos2d::TintTo::create(0.1f, cocos2d::Color3B(255, 60, 60));
	auto tintNormal = cocos2d::TintTo::create(0.1f, cocos2d::Color3B(255, 255, 255));
	sprite->runAction(cocos2d::Blink::create(0.2f, 5));
	sprite->runAction(cocos2d::Sequence::createWithTwoActions(tintRed, tintNormal));
}
void runMonsterDamageBlink(cocos2d::Sprite* sprite) {
	sprite->runAction(cocos2d::Blink::create(0.08f, 1));
}
void deathFade(cocos2d::Sprite* sprite) {
	auto scale = cocos2d::ScaleTo::create(0.5, 0);
	auto fade = cocos2d::FadeOut::create(0.5);
	sprite->runAction(scale);
	sprite->runAction(fade);
}