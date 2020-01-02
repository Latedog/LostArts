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

//class Level2Scene;

Dungeon::Dungeon() : m_level(1) {
	_Tile *tile;
	int i, j;
	
	//	initialize entire dungeon to blank space
	for (i = 0; i < MAXROWS; i++) {
		for (j = 0; j < MAXCOLS; j++) {
			tile = new _Tile;
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = SPACE;
			tile->wall = tile->item = tile->trap = tile->enemy = false;

			tile->marked = false;

			//m_maze[i*MAXCOLS + j] = *tile;
			m_maze.push_back(*tile);
			delete tile;
		}
	}

	for (i = 0; i < MAXROWS; i++) { //	initialize top and bottom of dungeon to be unbreakable walls
		for (j = 0; j < MAXCOLS; j++) {
			m_maze[i*MAXCOLS + j].top = UNBREAKABLE_WALL;
			m_maze[i*MAXCOLS + j].bottom = UNBREAKABLE_WALL;
			m_maze[i*MAXCOLS + j].traptile = UNBREAKABLE_WALL;

			m_maze[i*MAXCOLS + j].wall = true;
		}
		i += (MAXROWS - 2);
	}

	for (i = 0; i < MAXROWS - 1; i++){
		//	initialize edges of dungeon to be walls
		m_maze[i*MAXCOLS].top = m_maze[i*MAXCOLS].bottom = m_maze[i*MAXCOLS].traptile = UNBREAKABLE_WALL;
		m_maze[i*MAXCOLS + MAXCOLS - 1].top = m_maze[i*MAXCOLS + MAXCOLS -1].bottom = m_maze[i*MAXCOLS + MAXCOLS - 1].traptile = UNBREAKABLE_WALL;
		m_maze[i*MAXCOLS].wall = true;
		m_maze[i*MAXCOLS + MAXCOLS - 1].wall = true;
	}

	// RANDOMLY GENERATES LEVEL CHUNKS
	std::vector<std::vector<std::vector<char>>> b;
	std::vector<char> v, finalvec;

	// chooses layer for the special chunks to be on
	specialChunkLayer1 = 1 + randInt(4);
	specialChunkLayer2 = 1 + randInt(4);
	layer = 1;

	int count = 0;
	while (count < (MAXROWS - 2) / 4) {
		if (count == 0) {
			v = topChunks(b, MAXCOLS);
			layer++;
		}
		else if (count == 1 || count == 2) {
			v = middleChunks(b, MAXCOLS);
			layer++;
		}
		else if (count == 3) {
			v = bottomChunks(b, MAXCOLS);
			layer++;
		}

		for (i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i) == '#' ? WALL : v.at(i));

		b.clear();
		count++;
	}

	/*v1 = topChunks(b1, MAXCOLS); layer++;
	v2 = middleChunks(b2, MAXCOLS); layer++;
	v3 = middleChunks(b3, MAXCOLS); layer++;
	v4 = bottomChunks(b4, MAXCOLS);

	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v1.at(i) == '#' ? WALL : v1.at(i));


	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v2.at(i) == '#' ? WALL : v2.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v3.at(i) == '#' ? WALL : v3.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v4.at(i) == '#' ? WALL : v4.at(i));*/
	
	count = 0;
	char object;
	std::shared_ptr<Monster> monster(nullptr);
	for (i = 1; i < MAXROWS - 1; i++) {
		for (j = 1; j < MAXCOLS - 1; j++) {
			m_maze[i*MAXCOLS + j].top = finalvec.at(count);
			m_maze[i*MAXCOLS + j].bottom = finalvec.at(count);
			if (m_maze[i*MAXCOLS + j].top == WALL || m_maze[i*MAXCOLS + j].top == UNBREAKABLE_WALL) {
				m_maze[i*MAXCOLS + j].traptile = finalvec.at(count);
				m_maze[i*MAXCOLS + j].wall = true;
			}

			//	if the object was an item or enemy
			if (m_maze[i*MAXCOLS + j].bottom != WALL && m_maze[i*MAXCOLS + j].bottom != SPACE) {

				object = m_maze[i*MAXCOLS + j].bottom;
				switch (object) {
				case BROWN_CHEST:
				case SILVER_CHEST:
					m_maze[i*MAXCOLS + j].top = SPACE;
					m_maze[i*MAXCOLS + j].item = true;
					break;
				case WANDERER: {
					m_maze[i*MAXCOLS + j].bottom = SPACE;
					monster = std::make_shared<Wanderer>();
					monster->setPosX(j);
					monster->setPosY(i);
					m_monsters.emplace_back(monster);
					//Wanderer wand;
					//wand.setPosX(j);
					//wand.setPosY(i);
					//m_monsters.emplace_back(new Wanderer(wand));
					m_maze[i*MAXCOLS + j].enemy = true;
					break;
				}
				case GOBLIN: {
					m_maze[i*MAXCOLS + j].bottom = SPACE;
					monster = std::make_shared<Goblin>(12);
					monster->setPosX(j);
					monster->setPosY(i);
					m_monsters.emplace_back(monster);
					//Goblin goblin(12);
					//goblin.setPosX(j);
					//goblin.setPosY(i);
					//m_monsters.emplace_back(new Goblin(goblin));
					m_maze[i*MAXCOLS + j].enemy = true;
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

	char toptile = m_maze[player.at(0).getPosY()*MAXCOLS + player.at(0).getPosX()].top;
	char bottomtile = m_maze[player.at(0).getPosY()*MAXCOLS + player.at(0).getPosX()].bottom;

	while (toptile != SPACE || bottomtile != SPACE || player.at(0).getPosX() > 4 && player.at(0).getPosX() < 65) {
		player.at(0).setrandPosX(MAXCOLS);
		player.at(0).setrandPosY(MAXROWS);

		// while player is not spawned along left side or right side, reroll
		while (player.at(0).getPosX() > 4 && player.at(0).getPosX() < 65) {
			player.at(0).setrandPosX(MAXCOLS);
		}

		toptile = m_maze[player.at(0).getPosY()*MAXCOLS + player.at(0).getPosX()].top;
		bottomtile = m_maze[player.at(0).getPosY()*MAXCOLS + player.at(0).getPosX()].bottom;
	}
	m_maze[player.at(0).getPosY()*MAXCOLS + player.at(0).getPosX()].top = PLAYER; // sets tile at this spot to be player position
	player.at(0).addItem(Bomb());	// adds bomb to player's inventory upon spawn
	//player.at(0).addItem(SkeletonKey());
	player.at(0).addWeapon(IronLance());


	Stairs stairs;
	char traptile = m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].traptile;
	//toptile = m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].top;
	//bottomtile = m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].bottom;
	
	///	while stairs do not spawn on the side opposite from the player
	//while (toptile != SPACE || bottomtile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() < 65) || (player.at(0).getPosX() > 64 && stairs.getPosX() > 4))) {
	while (traptile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() >= 5) || (player.at(0).getPosX() > 64 && stairs.getPosX() < 64))) {
		stairs.setrandPosX(MAXCOLS);
		stairs.setrandPosY(MAXROWS);

		traptile = m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].traptile;
		//toptile = m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].top;
		//bottomtile = m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].bottom;
	}
	//m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].bottom = STAIRS;
	m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].traptile = STAIRS;


	//	BETTER ITEM GENERATION?
	int n;
	char itemtype;
	std::shared_ptr<Objects> item(nullptr);
	for (int i = 0; i < 5; i++) { // i < number_of_different_items

		switch (i) {
		case 0: n = 1; break; // Life Potion
		case 1: n = 1; break; // Bomb
		case 2: n = 1; break; // Brown Chest
		case 3: n = 1; break; // Rusty Cutlass
		case 4: n = 1; break; // Skeleton Key
		case 5: n = 1; break; // 
		case 6: n = 5 + randInt(3); break; // 
		case 7: n = 3 + randInt(3); break; // 
		case 8: n = 4 + randInt(4); break; // 
		default: break;
		}

		while (n > 0) {

			switch (i) {
			case 0:
				item = std::make_shared<LifePotion>();
				itemtype = LIFEPOT; break;
			case 1:
				item = std::make_shared<Bomb>();
				itemtype = BOMB; break;
			case 2:
				item = std::make_shared<BrownChest>();
				itemtype = BROWN_CHEST; break;
			case 3:
				item = std::make_shared<RustyCutlass>();
				itemtype = CUTLASS; break;
			case 4:
				item = std::make_shared<SkeletonKey>();
				itemtype = SKELETON_KEY; break;
			case 5:
				item = std::make_shared<InfinityBox>();
				itemtype = INFINITY_BOX; break;
			case 6:
				//item = make_shared<Roundabout>();
				itemtype = ROUNDABOUT; break;
			case 7:
				//item = make_shared<MountedKnight>();
				itemtype = MOUNTED_KNIGHT; break;
			case 8:
				//item = make_shared<Seeker>(12);
				itemtype = SEEKER; break;
			}

			bottomtile = m_maze[item->getPosY()*MAXCOLS + item->getPosX()].bottom;
			// while item position clashes with wall, player, etc., then reroll its position
			while (bottomtile != SPACE) {
				item->setrandPosX(MAXCOLS);
				item->setrandPosY(MAXROWS);

				bottomtile = m_maze[item->getPosY()*MAXCOLS + item->getPosX()].bottom;
			}
			m_maze[item->getPosY()*MAXCOLS + item->getPosX()].bottom = itemtype;
			m_maze[item->getPosY()*MAXCOLS + item->getPosX()].item = true;
			n--;
			item.reset();
		}
	}

	//	END BETTER ITEM GENERATION?

	/*
	LifePotion lp;
	bottomtile = m_maze[lp.getPosY()*MAXCOLS + lp.getPosX()].bottom;
	while (bottomtile != SPACE) {
		lp.setrandPosX(MAXCOLS);
		lp.setrandPosY(MAXROWS);

		bottomtile = m_maze[lp.getPosY()*MAXCOLS + lp.getPosX()].bottom;
	}
	m_maze[lp.getPosY()*MAXCOLS + lp.getPosX()].bottom = LIFEPOT;


	Bomb bomb;
	bottomtile = m_maze[bomb.getPosY()*MAXCOLS + bomb.getPosX()].bottom;
	while (bottomtile != SPACE) {
		bomb.setrandPosX(MAXCOLS);
		bomb.setrandPosY(MAXROWS);

		bottomtile = m_maze[bomb.getPosY()*MAXCOLS + bomb.getPosX()].bottom;
	}
	m_maze[bomb.getPosY()*MAXCOLS + bomb.getPosX()].bottom = BOMB;


	BrownChest chest;
	bottomtile = m_maze[chest.getPosY()*MAXCOLS + chest.getPosX()].bottom;
	while (bottomtile != SPACE) {
		chest.setrandPosX(MAXCOLS);
		chest.setrandPosY(MAXROWS);

		bottomtile = m_maze[chest.getPosY()*MAXCOLS + chest.getPosX()].bottom;
	}
	m_maze[chest.getPosY()*MAXCOLS + chest.getPosX()].bottom = BROWN_CHEST;


	int numwep = 1 + randInt(2); // number of weapons to be placed
	while (numwep > 0) {
		RustyCutlass rc;
		
		bottomtile = m_maze[rc.getPosY()*MAXCOLS + rc.getPosX()].bottom;
		while (bottomtile != SPACE) { // while sword position clashes with anything
			rc.setrandPosX(MAXCOLS);	// reroll it
			rc.setrandPosY(MAXROWS);

			bottomtile = m_maze[rc.getPosY()*MAXCOLS + rc.getPosX()].bottom;
		}
		m_maze[rc.getPosY()*MAXCOLS + rc.getPosX()].bottom = CUTLASS;
		numwep--;
	}
	*/

	//	BETTER MONSTER GENERATION?
	char monstertype;
	for (int i = 0; i < 9; i++) {

		switch (i) {
		case 0: n = 3 + randInt(2); break; // Goblin
		case 1: n = 8 + randInt(2); break; // Wanderer
		case 2: n = 5 + randInt(5); break; // Archer
		case 3: n = 5 + randInt(4); break; // Zapper
		case 4: n = 7 + randInt(6); break; // Spinner
		case 5: n = 1; break; // Bombee
		case 6: n = 5 + randInt(3); break; // Roundabout
		case 7: n = 5 + randInt(3); break; // Mounted Knight
		case 8: n = 2 + randInt(4); break; // Seeker
		}

		while (n > 0) {

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

			toptile = m_maze[monster->getPosY()*MAXCOLS + monster->getPosX()].top;
			while (toptile != SPACE) { // while monster position clashes with wall, player, or idol
				monster->setrandPosX(MAXCOLS);				// reroll it
				monster->setrandPosY(MAXROWS);

				toptile = m_maze[monster->getPosY()*MAXCOLS + monster->getPosX()].top;
			}
			m_monsters.emplace_back(monster);

			m_maze[monster->getPosY()*MAXCOLS + monster->getPosX()].top = monstertype;
			m_maze[monster->getPosY()*MAXCOLS + monster->getPosX()].enemy = true;
			n--;
			monster.reset();
		}
	}
	//	END BETTER MONSTER GENERATION?

	/*
	//	generate goblins
	int m = 3 + randInt(3); // number of goblins to be placed
	while (m > 0) { // generate goblins
		Goblin g(12);

		toptile = m_maze[g.getPosY()*MAXCOLS + g.getPosX()].top;
		while (toptile != SPACE) { // while goblin position clashes with wall, player, or idol
			g.setrandPosX(MAXCOLS);				// reroll it
			g.setrandPosY(MAXROWS);

			toptile = m_maze[g.getPosY()*MAXCOLS + g.getPosX()].top;
		}
		m_monsters.emplace_back(new Goblin(g));
		m_maze[g.getPosY()*MAXCOLS + g.getPosX()].top = GOBLIN;
		m_maze[g.getPosY()*MAXCOLS + g.getPosX()].enemy = true;
		m--;
	}

	// generate wanderers
	m = 5 + randInt(7);
	while (m > 0) {
		Wanderer w;

		toptile = m_maze[w.getPosY()*MAXCOLS + w.getPosX()].top;
		while (toptile != SPACE) { // while enemy position clashes with wall, player, or idol
			w.setrandPosX(MAXCOLS);		// reroll it
			w.setrandPosY(MAXROWS);

			toptile = m_maze[w.getPosY()*MAXCOLS + w.getPosX()].top;
		}
		m_monsters.emplace_back(new Wanderer(w));
		m_maze[w.getPosY()*MAXCOLS + w.getPosX()].top = WANDERER;
		m_maze[w.getPosY()*MAXCOLS + w.getPosX()].enemy = true;
		m--;
	}

	// generate archers
	m = 5 + randInt(5);
	while (m > 0) {
		Archer a;

		toptile = m_maze[a.getPosY()*MAXCOLS + a.getPosX()].top;
		while (toptile != SPACE) {
			a.setrandPosX(MAXCOLS);				// reroll it
			a.setrandPosY(MAXROWS);

			toptile = m_maze[a.getPosY()*MAXCOLS + a.getPosX()].top;
		}
		m_monsters.emplace_back(new Archer(a));
		m_maze[a.getPosY()*MAXCOLS + a.getPosX()].top = ARCHER;
		m_maze[a.getPosY()*MAXCOLS + a.getPosX()].enemy = true;
		m--;
	}

	// generate zappers
	m = 5 + randInt(4);
	while (m > 0) {
		Zapper z;

		toptile = m_maze[z.getPosY()*MAXCOLS + z.getPosX()].top;
		while (toptile != SPACE) {
			z.setrandPosX(MAXCOLS);				// reroll it
			z.setrandPosY(MAXROWS);

			toptile = m_maze[z.getPosY()*MAXCOLS + z.getPosX()].top;
		}
		m_monsters.emplace_back(new Zapper(z));
		m_maze[z.getPosY()*MAXCOLS + z.getPosX()].top = ZAPPER;
		m_maze[z.getPosY()*MAXCOLS + z.getPosX()].enemy = true;
		m--;
	}

	// generate spinners
	m = 3 + randInt(6);
	while (m > 0) {
		Spinner s;

		toptile = m_maze[s.getPosY()*MAXCOLS + s.getPosX()].top;
		while (toptile != SPACE) {
			s.setrandPosX(MAXCOLS);				// reroll it
			s.setrandPosY(MAXROWS);

			toptile = m_maze[s.getPosY()*MAXCOLS + s.getPosX()].top;
		}
		m_monsters.emplace_back(new Spinner(s));
		m_maze[s.getPosY()*MAXCOLS + s.getPosX()].top = SPINNER;
		m_maze[s.getPosY()*MAXCOLS + s.getPosX()].enemy = true;
		m--;
	}

	// generate bombees
	Bombee bee;
	toptile = m_maze[bee.getPosY()*MAXCOLS + bee.getPosX()].top;
	while (toptile != SPACE) {
		bee.setrandPosX(MAXCOLS);				// reroll it
		bee.setrandPosY(MAXROWS);

		toptile = m_maze[bee.getPosY()*MAXCOLS + bee.getPosX()].top;
	}
	m_monsters.emplace_back(new Bombee(bee));
	m_maze[bee.getPosY()*MAXCOLS + bee.getPosX()].top = BOMBEE;
	m_maze[bee.getPosY()*MAXCOLS + bee.getPosX()].enemy = true;
	m--;

	// generate roundabouts
	m = 5 + randInt(3);
	while (m > 0) {
		Roundabout ra;

		toptile = m_maze[ra.getPosY()*MAXCOLS + ra.getPosX()].top;
		while (toptile != SPACE) {
			ra.setrandPosX(MAXCOLS);				// reroll it
			ra.setrandPosY(MAXROWS);

			toptile = m_maze[ra.getPosY()*MAXCOLS + ra.getPosX()].top;
		}
		m_monsters.emplace_back(new Roundabout(ra));
		m_maze[ra.getPosY()*MAXCOLS + ra.getPosX()].top = ROUNDABOUT;
		m_maze[ra.getPosY()*MAXCOLS + ra.getPosX()].enemy = true;
		m--;
	}

	// generate mounted knights
	m = 3 + randInt(3);
	while (m > 0) {
		MountedKnight k;

		toptile = m_maze[k.getPosY()*MAXCOLS + k.getPosX()].top;
		while (toptile != SPACE) {
			k.setrandPosX(MAXCOLS);				// reroll it
			k.setrandPosY(MAXROWS);

			toptile = m_maze[k.getPosY()*MAXCOLS + k.getPosX()].top;
		}
		m_monsters.emplace_back(new MountedKnight(k));
		m_maze[k.getPosY()*MAXCOLS + k.getPosX()].top = MOUNTED_KNIGHT;
		m_maze[k.getPosY()*MAXCOLS + k.getPosX()].enemy = true;
		m--;
	}

	// generate seekers
	m = 4 + randInt(4);
	while (m > 0) {
		Seeker s(10);

		toptile = m_maze[s.getPosY()*MAXCOLS + s.getPosX()].top;
		while (toptile != SPACE) {
			s.setrandPosX(MAXCOLS);				// reroll it
			s.setrandPosY(MAXROWS);

			toptile = m_maze[s.getPosY()*MAXCOLS + s.getPosX()].top;
		}
		m_monsters.emplace_back(new Seeker(s));
		m_maze[s.getPosY()*MAXCOLS + s.getPosX()].top = SEEKER;
		m_maze[s.getPosY()*MAXCOLS + s.getPosX()].enemy = true;
		m--;
	}

	*/
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

	m_level = dungeon.m_level;
	layer = dungeon.layer;
	specialChunkLayer1 = dungeon.specialChunkLayer1;
	specialChunkLayer2 = dungeon.specialChunkLayer2;

	return *this;
}
_Tile& Dungeon::operator[](int index) {
	return m_maze[index];
}
Dungeon::~Dungeon() {
	
}

void Dungeon::peekDungeon(int x, int y, char move) {
	char top, projectile, bottom, traptile;
	bool wall, item, trap, enemy;

	//move player
	int weprange = player.at(0).getWeapon().getRange();
	bool move_used = false;
	bool far_enemy, close_enemy;
	// if weapon has a range of 2, check for monsters to attack
	if (weprange == 2) {
		if (move == 'l') { // moved to the left
			top = m_maze[y*MAXCOLS + x - 1].top;
			close_enemy = m_maze[y*MAXCOLS + x - 1].enemy;
			far_enemy = m_maze[y*MAXCOLS + x - 2].enemy;

			if (close_enemy) {
				fight(x - 1, y);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && y*MAXCOLS + x-2 > 0) {	// monster encountered
				fight(x - 2, y);
				move_used = true;
			}
		}
		else if (move == 'r') {
			top = m_maze[y*MAXCOLS + x + 1].top;
			close_enemy = m_maze[y*MAXCOLS + x + 1].enemy;
			far_enemy = m_maze[y*MAXCOLS + x + 2].enemy;

			if (close_enemy) {
				fight(x + 1, y);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && y*MAXCOLS + x+2 < MAXROWS * MAXCOLS) {
				fight(x + 2, y);
				move_used = true;
			}
		}
		else if (move == 'u') { // moved up
			top = m_maze[(y - 1)*MAXCOLS + x].top;
			close_enemy = m_maze[(y-1)*MAXCOLS + x].enemy;
			// if player is along top edge, assign far_enemy to close_enemy instead
			far_enemy = (y == 1 ? close_enemy : m_maze[(y-2)*MAXCOLS + x].enemy);

			if (close_enemy) {
				fight(x, y - 1);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && (y - 2)*MAXCOLS + x > 0){
				fight(x, y - 2);
				move_used = true;
			}
		}
		else if (move == 'd') { // moved down
			top = m_maze[(y + 1)*MAXCOLS + x].top;
			close_enemy = m_maze[(y + 1)*MAXCOLS + x].enemy;
			far_enemy = (y == MAXROWS - 2 ? close_enemy : m_maze[(y + 2)*MAXCOLS + x].enemy);

			if (close_enemy) {
				fight(x, y + 1);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && (y+2)*MAXCOLS+x < MAXROWS * MAXCOLS) {
				fight(x, y + 2);
				move_used = true;
			}
		}
	}
	
	if (!move_used) {
		if (move == 'l') { // moved to the left
			top = m_maze[y*MAXCOLS + x - 1].top;
			bottom = m_maze[y*MAXCOLS + x - 1].bottom;
			traptile = m_maze[y*MAXCOLS + x - 1].trap;

			wall = m_maze[y*MAXCOLS + x - 1].wall;
			item = m_maze[y*MAXCOLS + x - 1].item;
			trap = m_maze[y*MAXCOLS + x - 1].trap;
			enemy = m_maze[y*MAXCOLS + x - 1].enemy;

			if (trap) {
				; // check for traps
			}
			if (!(wall || item || enemy)) {
				// move character to the left
				m_maze[y*MAXCOLS + x - 1].top = PLAYER;
				m_maze[y*MAXCOLS + x].top = SPACE;
				player.at(0).setPosX(x - 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of item
				m_maze[y*MAXCOLS + x - 1].top = PLAYER;
				m_maze[y*MAXCOLS + x].top = SPACE;
				player.at(0).setPosX(x - 1);

				foundItem(m_maze, MAXCOLS, x - 1, y);
			}
			else if (enemy) {	// monster encountered
				fight(x - 1, y);
			}
		}

		else if (move == 'r') { // moved to the right
			top = m_maze[y*MAXCOLS + x + 1].top;
			bottom = m_maze[y*MAXCOLS + x + 1].bottom;
			traptile = m_maze[y*MAXCOLS + x + 1].trap;

			wall = m_maze[y*MAXCOLS + x + 1].wall;
			item = m_maze[y*MAXCOLS + x + 1].item;
			trap = m_maze[y*MAXCOLS + x + 1].trap;
			enemy = m_maze[y*MAXCOLS + x + 1].enemy;

			if (trap) {
				; // check for traps
			}
			if (!(wall || item || enemy)) {
				// move character to the left
				m_maze[y*MAXCOLS + x + 1].top = PLAYER;
				m_maze[y*MAXCOLS + x].top = SPACE;
				player.at(0).setPosX(x + 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of item
				m_maze[y*MAXCOLS + x + 1].top = PLAYER;
				m_maze[y*MAXCOLS + x].top = SPACE;
				player.at(0).setPosX(x + 1);

				foundItem(m_maze, MAXCOLS, x + 1, y);
			}
			else if (enemy) {
				fight(x + 1, y);
			}
		}

		else if (move == 'u') { // moved up
			top = m_maze[(y - 1)*MAXCOLS + x].top;
			bottom = m_maze[(y - 1)*MAXCOLS + x].bottom;
			traptile = m_maze[(y - 1)*MAXCOLS + x].traptile;

			wall = m_maze[(y - 1)*MAXCOLS + x].wall;
			item = m_maze[(y - 1)*MAXCOLS + x].item;
			trap = m_maze[(y - 1)*MAXCOLS + x].trap;
			enemy = m_maze[(y - 1)*MAXCOLS + x].enemy;

			if (trap) {
				;
			}
			if (!(wall || item || enemy)) {
				m_maze[(y - 1)*MAXCOLS + x].top = PLAYER;
				m_maze[y*MAXCOLS + x].top = SPACE;
				player.at(0).setPosY(y - 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of item
				m_maze[(y - 1)*MAXCOLS + x].top = PLAYER;
				m_maze[y*MAXCOLS + x].top = SPACE;
				player.at(0).setPosY(y - 1);

				foundItem(m_maze, MAXCOLS, x, y - 1);
			}
			else if (enemy) {
				fight(x, y - 1);
			}
		}

		else if (move == 'd') { // moved down
			top = m_maze[(y + 1)*MAXCOLS + x].top;
			bottom = m_maze[(y + 1)*MAXCOLS + x].bottom;
			traptile = m_maze[(y + 1)*MAXCOLS + x].traptile;

			wall = m_maze[(y + 1)*MAXCOLS + x].wall;
			item = m_maze[(y + 1)*MAXCOLS + x].item;
			trap = m_maze[(y + 1)*MAXCOLS + x].trap;
			enemy = m_maze[(y + 1)*MAXCOLS + x].enemy;

			if (trap) {
				;
			}
			if (!(wall || item || enemy)) {
				m_maze[(y + 1)*MAXCOLS + x].top = PLAYER;
				m_maze[y*MAXCOLS + x].top = SPACE;
				player.at(0).setPosY(y + 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of item
				m_maze[(y + 1)*MAXCOLS + x].top = PLAYER;
				m_maze[y*MAXCOLS + x].top = SPACE;
				player.at(0).setPosY(y + 1);

				foundItem(m_maze, MAXCOLS, x, y + 1);
			}
			else if (enemy) {
				fight(x, y + 1);
			}
		}

		else if (move == 'e') {
			collectItem(m_maze, MAXROWS, MAXCOLS, x, y);

			if (m_level > 1)
				return;
		}

		else if (move == 'w') {
			//showDungeon();
			//player.at(0).wield(dungeonText);
			player.at(0).wield();
		}

		else if (move == 'c') {
			//showDungeon();
			//player.at(0).use(m_actives, m_maze[y*MAXCOLS + x], index);
		}
	}
	

	// check active items in play
	checkActive(m_maze, MAXROWS, MAXCOLS, m_actives, m_monsters);

	//	if player is dead then return
	if (player.at(0).getHP() <= 0) {
		m_maze[y*MAXCOLS + x].top = 'X'; // change player icon to X to indicate death

		//clearScreen();
		//showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}

	// if all monsters are dead?
	if (m_monsters.empty()) {
		//clearScreen();
		//showDungeon();
		if (!dungeonText.empty())
			showText();

		return;
	}

	// move monsters
	int mx, my, shortest;
	char first, best, prev;
	std::string monster;

	for (unsigned i = 0; i < m_monsters.size(); i++) {
		monster = m_monsters.at(i)->getName();
		mx = m_monsters.at(i)->getPosX();
		my = m_monsters.at(i)->getPosY();
		first = best = prev = '0';
		shortest = 0;

		if (monster == "goblin") {
			//	allows use of goblin's smell distance function
			std::shared_ptr<Goblin> goblin = std::dynamic_pointer_cast<Goblin>(m_monsters.at(i));

			if (playerInRange(mx, my, i, goblin->getSmellDistance())) {
				// if viable path is found
				if (checkGoblins(m_maze, MAXCOLS, i, shortest, goblin->getSmellDistance(), goblin->getSmellDistance(), mx, my, first, best, prev)) {
					moveGoblins(m_maze, MAXROWS, MAXCOLS, mx, my, i, best, m_monsters);
				}
			}
			goblin.reset();
		}

		else if (monster == "wanderer") {
			moveWanderers(m_maze, MAXROWS, MAXCOLS, mx, my, i, m_monsters);
		}

		else if (monster == "archer") {
			checkArchers(m_maze, MAXROWS, MAXCOLS, player.at(0).getPosX(), player.at(0).getPosY(), i, m_monsters);
		}

		else if (monster == "zapper") {
			moveZappers(m_maze, MAXCOLS, mx, my, i, m_monsters);
		}

		else if (monster == "spinner") {
			moveSpinners(m_maze, MAXROWS, MAXCOLS, mx, my, i, m_monsters);
		}

		else if (monster == "bombee") {
			std::shared_ptr<Bombee> bombee = std::dynamic_pointer_cast<Bombee>(m_monsters.at(i));
			if (playerInRange(mx, my, i, bombee->getRange())) {
				// if viable path is found
				if (checkGoblins(m_maze, MAXCOLS, i, shortest, bombee->getRange(), bombee->getRange(), mx, my, first, best, prev)) {
					moveGoblins(m_maze, MAXROWS, MAXCOLS, mx, my, i, best, m_monsters);
				}
			}
			bombee.reset();
		}

		else if (monster == "roundabout") {
			moveRoundabouts(m_maze, MAXROWS, MAXCOLS, mx, my, i, m_monsters);
		}

		else if (monster == "mounted knight") {
			checkMountedKnights(m_maze, MAXROWS, MAXCOLS, player.at(0).getPosX(), player.at(0).getPosY(), i, m_monsters);
		}

		else if (monster == "seeker") {
			std::shared_ptr<Seeker> seeker = std::dynamic_pointer_cast<Seeker>(m_monsters.at(i));

			if (playerInRange(mx, my, i, seeker->getRange())) {
				// if viable path is found
				if (checkGoblins(m_maze, MAXCOLS, i, shortest, seeker->getRange(), seeker->getRange(), mx, my, first, best, prev)) {
					moveSeekers(m_maze, MAXROWS, MAXCOLS, mx, my, i, best, m_monsters);
				}
			}
			seeker.reset();
		}

		//	if player is dead then break
		if (player.at(0).getHP() <= 0) {
			m_maze[y*MAXCOLS + x].top = 'X'; // change player icon to X to indicate death
			player.at(0).setDeath(m_monsters.at(i)->getName());
			break;
		}
	}

	// check if player is holding the skeleton key
	if (player.at(0).hasSkeletonKey()) {
		player.at(0).checkKeyConditions(dungeonText);
	}


	//clearScreen();
	//showDungeon();
	if (!dungeonText.empty())
		showText();
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
	int x_sprite = x*SPACING_FACTOR - X_OFFSET;
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
			case 'u': sprites.at(i)->setPosition(px, (maxrows - (y-1))*SPACING_FACTOR - Y_OFFSET); break;
			case 'd': sprites.at(i)->setPosition(px, (maxrows - (y+1))*SPACING_FACTOR - Y_OFFSET); break;
			case '1': sprites.at(i)->setPosition((x - 1)*SPACING_FACTOR - X_OFFSET, (maxrows - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '2': sprites.at(i)->setPosition((x + 1)*SPACING_FACTOR - X_OFFSET, (maxrows - (y - 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '3': sprites.at(i)->setPosition((x - 1)*SPACING_FACTOR - X_OFFSET, (maxrows - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;
			case '4': sprites.at(i)->setPosition((x + 1)*SPACING_FACTOR - X_OFFSET, (maxrows - (y + 1))*SPACING_FACTOR - Y_OFFSET); break;
			}
			return;
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
				//move->setTarget(sprites.at(i));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'L': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-2*SPACING_FACTOR, 0));
				//move->setTarget(sprites.at(i));
				sprites.at(i)->runAction(move);
				break;
			}
			case '<': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(-3 * SPACING_FACTOR, 0));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'r': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(SPACING_FACTOR, 0));
				move->setTarget(sprites.at(i));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'R': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(2*SPACING_FACTOR, 0));
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
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, SPACING_FACTOR));
				move->setTarget(sprites.at(i));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'U': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, 2*SPACING_FACTOR));
				move->setTarget(sprites.at(i));
				sprites.at(i)->runAction(move);
				break;
			}
			case '^': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, 3 * SPACING_FACTOR));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'd': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -SPACING_FACTOR));
				move->setTarget(sprites.at(i));
				sprites.at(i)->runAction(move);
				break;
			}
			case 'D': {
				cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -2*SPACING_FACTOR));
				move->setTarget(sprites.at(i));
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
		sprites.insert(std::make_pair(Vec2(x, y+1), it->second));
		break;
	}
	case 'D': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -2 * SPACING_FACTOR));
		it->second->runAction(move);
		sprites.insert(std::make_pair(Vec2(x, y+2), it->second));
		break;
	}
	case 'V': {
		cocos2d::Action* move = cocos2d::MoveBy::create(.1, cocos2d::Vec2(0, -3 * SPACING_FACTOR));
		it->second->runAction(move);
		sprites.insert(std::make_pair(Vec2(x, y+3), it->second));
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

					dungeonText.push_back("A bomb exploded!\n");
					explosion(dungeon, maxrows, maxcols, actives, monsters, bomb->getPosX(), bomb->getPosY(), i);
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

					dungeonText.push_back("BOOM!\n");
					explosion(dungeon, maxrows, maxcols, actives, monsters, megabomb->getPosX(), megabomb->getPosY(), i);
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
					if (dungeon[(y + 1)*maxcols + x].top == PLAYER) {
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
					if (dungeon[(y + 1)*maxcols + x].top == PLAYER || dungeon[(y + 2)*maxcols + x].top == PLAYER) {
						removeSprite(spike_sprites, maxrows, x, y);
						
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
					if (dungeon[(y + 3)*maxcols + x].top == PLAYER || dungeon[(y + 2)*maxcols + x].top == PLAYER || dungeon[(y + 1)*maxcols + x].top == PLAYER) {
						removeSprite(spike_sprites, maxrows, x, y);
						
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
					if (dungeon[y*maxcols + x].top == PLAYER) {
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
					if (dungeon[y*maxcols + x].top == PLAYER) {
						ts->toggleTrigger();
						actives.at(i) = ts;
						ts.reset();
						dungeonText.push_back("*tink*\n");
					}
					else
						dungeon[y*maxcols + x].traptile = TRIGGER_SPIKE_DEACTIVE;
				}
			}
			else if (actives.at(i)->getItem() == "burn") {
				std::shared_ptr<Burn> burn = std::dynamic_pointer_cast<Burn>(actives.at(i));
				if (burn->burnsLeft() == burn->maxBurns())
					burn->setBurn(burn->burnsLeft() - 1);
				else if (burn->burnsLeft() > 0)
					burn->burn(player.at(0), dungeonText);
				else {
					dungeonText.push_back("The burning subsides.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}

				if (player.at(0).getHP() <= 0) {
					player.at(0).setDeath("burn");
					dungeon[py*maxcols + px].top = 'X';
				}
				burn.reset();
			}
			else if (actives.at(i)->getItem() == "heal") {
				shared_ptr<HealOverTime> heal = dynamic_pointer_cast<HealOverTime>(actives.at(i));
				if (heal->healsLeft() > 0)
					heal->heal(player.at(0), dungeonText);
				else {
					dungeonText.push_back("The heal fades.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}
				heal.reset();
			}
			else if (actives.at(i)->getItem() == "bleed") {
				shared_ptr<Bleed> bleed = dynamic_pointer_cast<Bleed>(actives.at(i));

				if (bleed->bleedLeft() > 0) {
					// find the monster(s) that are bleeding
					bool found = false;
					for (int n = 0; n < monsters.size(); n++) {
						// if bleed count of object equals bleed count of the monster, bleed the monster
						if (monsters.at(n)->isBled() && bleed->bleedLeft() == monsters.at(n)->bleedLeft()) {
							bleed->bleed(*(monsters.at(n)), dungeonText);
							if (monsters.at(n)->getHP() <= 0) {
								monsterDeath(dungeon, maxrows, maxcols, actives, monsters, n);// monsters.at(n)->getPosX(), monsters.at(n)->getPosY(), n);
								pos = i;
								actives.erase(actives.begin() + pos);
							}
							found = true;
							break;
						}
					}

					// if monster was not found and player is not bled, remove the affliction from the active pool
					if (!(found || player.at(0).isBled())) {
						pos = i;
						actives.erase(actives.begin() + pos);
					}

					// if player is bleeding
					if (player.at(0).isBled()) {
						bleed->bleed(player.at(0), dungeonText);

						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("bleed");
							dungeon[py*maxcols + px].top = 'X';
						}
					}
				}
				else {
					dungeonText.push_back("The bleeding stops.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}

				bleed.reset();
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
				monsters.at(i)->setHP(monsters.at(i)->getHP() - 10);
				dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

				if (monsters.at(i)->getHP() <= 0) {
					monsterDeath(dungeon, maxrows, maxcols, actives, monsters, i);// mx, my, i);
				}
			}
		}
		else if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			monsters.at(i)->setHP(monsters.at(i)->getHP() - 10);
			dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

			if (monsters.at(i)->getHP() <= 0) {
				monsterDeath(dungeon, maxrows, maxcols, actives, monsters, i);// mx, my, i);
			}
		}
	}

	// check if player was hit or if there are any walls to destroy
	if (actives.at(pos)->getItem() == "Mega Bomb") {
		//	if player is caught in the explosion
		if (abs(px - x) <= 2 && abs(py - y) <= 2) {
			player.at(0).setHP(player.at(0).getHP() - 10);
			dungeonText.push_back("That's gotta hurt!\n");
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
						removeSprite(wall_sprites, maxrows, k, j);
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
			dungeonText.push_back("That's gotta hurt!\n");
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
					removeSprite(wall_sprites, maxrows, k, j);
				}
			}
		}
	}
}
void Dungeon::trapEncounter(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, int x, int y) { // trap's coordinates
	unsigned i = 0;
	bool found = false;
	while (!found && i < actives.size()) { // finds the trap just encountered
		if (actives.at(i)->getPosX() == x && actives.at(i)->getPosY() == y)
			found = true;
		else
			i++;
	}

	string trap = actives.at(i)->getItem();
	if (trap == "falling spike") {
		shared_ptr<FallingSpike> spike = dynamic_pointer_cast<FallingSpike>(actives.at(i));

		// remove the spike sprite
		removeSprite(spike_sprites, maxrows, x, y);

		dungeonText.push_back("A falling spike hit you for " + to_string(spike->getDmg()) + "!\n");
		player.at(0).setHP(player.at(0).getHP() - spike->getDmg());

		actives.erase(actives.begin() + i);
		spike.reset();
	}
	else if (trap == "spike trap") {
		shared_ptr<SpikeTrap> spiketrap = dynamic_pointer_cast<SpikeTrap>(actives.at(i));

		if (dungeon[y*maxcols + x].traptile == SPIKETRAP_ACTIVE) {
			dungeonText.push_back("You step on a spike trap and take " + to_string(spiketrap->getDmg()) + " damage!\n");
			player.at(0).setHP(player.at(0).getHP() - spiketrap->getDmg());
		}
		spiketrap.reset();
	}
	else if (trap == "trigger spike trap") {
		shared_ptr<TriggerSpike> ts = dynamic_pointer_cast<TriggerSpike>(actives.at(i));

		if (dungeon[y*maxcols + x].traptile == TRIGGER_SPIKE_ACTIVE) {
			dungeonText.push_back("You step on a spike trap and take " + to_string(ts->getDmg()) + " damage!\n");
			player.at(0).setHP(player.at(0).getHP() - ts->getDmg());
		}

		ts.reset();
	}
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
	removeSprite(monster_sprites, maxrows, x, y);

	if (monster == "bombee") {
		shared_ptr<Bombee> bombee = dynamic_pointer_cast<Bombee>(monsters.at(pos));
		dungeon[y*maxcols + x].extra = BOMBEE_ACTIVE;

		MegaBomb megabomb;
		megabomb.lightBomb();
		megabomb.setPosX(bombee->getPosX());
		megabomb.setPosY(bombee->getPosY());

		actives.emplace_back(new MegaBomb(megabomb));
		addSprite(projectile_sprites, maxrows, x, y, -1, "Bomb_48x48.png");

		dungeonText.push_back("*beep*\n");
		bombee.reset();
	}
	else {
		dungeonText.push_back("The " + monster + " was slain.\n");

		if (monster == "goblin") {
			// 20% drop chance
			if (randInt(10) + 1 > 8) {
				if (item) {
					itemHash(dungeon, maxrows, maxcols, x, y);
				}
				dungeon[y*maxcols + x].bottom = BONEAXE;
				dungeon[y*maxcols + x].item = true;
				addSprite(item_sprites, maxrows, x, y, -1, "Bone_Axe_48x48.png");
			}
		}
		else if (monster == "wanderer") {
			if (randInt(1) + 1 == 1) {
				if (item) {
					itemHash(dungeon, maxrows, maxcols, x, y);
				}
				dungeon[y*maxcols + x].bottom = HEART_POD;
				dungeon[y*maxcols + x].item = true;
				addSprite(item_sprites, maxrows, x, y, -1, "Heart_Pod_48x48.png");
			}
		}
		else if (monster == "archer") {
			// 15% drop chance
			if (1 + randInt(20) > 17) {
				if (item) {
					itemHash(dungeon, maxrows, maxcols, x, y);
				}
				dungeon[y*maxcols + x].bottom = BRONZE_DAGGER;
				dungeon[y*maxcols + x].item = true;
				addSprite(item_sprites, maxrows, x, y, -1, "Bronze_Dagger_48x48.png");
			}
		}
		else if (monster == "zapper") {
			for (int i = y - 1; i < y + 2; i++) {
				for (int j = x - 1; j < x + 2; j++) {
					dungeon[i*maxcols + j].projectile = SPACE;
					removeAssociatedSprite(zapper_sprites, maxrows, x, y, j, i);
				}
			}
		}
		else if (monster == "spinner") {
			for (int i = y - 2; i < y + 3; i++) {
				for (int j = x - 2; j < x + 3; j++) {
					if (y != 1 && y != maxrows - 2) {
						dungeon[i*maxcols + j].projectile = SPACE;
						removeAssociatedSprite(spinner_sprites, maxrows, x, y, j, i);
					}
				}
			}
			/*std::shared_ptr<Spinner> spinner = std::dynamic_pointer_cast<Spinner>(monsters.at(pos));
			spinner->getInner()->removeFromParent();
			spinner->getOuter()->removeFromParent();
			spinner.reset();
			*/
		}
		else if (monster == "mounted knight") {
			// 10% drop chance
			if (randInt(10) + 1 == 10) {
				if (item) {
					itemHash(dungeon, maxrows, maxcols, x, y);
				}
				dungeon[y*maxcols + x].bottom = IRON_LANCE;
				dungeon[y*maxcols + x].item = true;
				addSprite(item_sprites, maxrows, x, y, -1, "Iron_Lance_48x48.png");
			}
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

bool Dungeon::checkGoblins(std::vector<_Tile> &dungeon, int maxcols, int pos, int &shortest, int smelldist, int origdist, int x, int y, \
	char &first_move, char &optimal_move, char prev) {
	
	// if enemy is immediately adjacent to the player, return to attack
	if (smelldist == origdist && \
		(dungeon[(y - 1)*maxcols + x].top == PLAYER || dungeon[(y + 1)*maxcols + x].top == PLAYER || dungeon[y*maxcols + x - 1].top == PLAYER || dungeon[y*maxcols + x + 1].top == PLAYER)) {
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

	if (dungeon[y*maxcols + x].top == PLAYER) {
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

	if (!dungeon[y*maxcols + x+1].marked && prev != 'l' && (dungeon[y*maxcols + x + 1].top == SPACE || dungeon[y*maxcols + x + 1].top == PLAYER)) { //(dungeon[y*maxcols + x+1].top != WALL && dungeon[y*maxcols + x+1].top != UNBREAKABLE_WALL && dungeon[y*maxcols + x + 1].top != LAVA) && !dungeon[y*maxcols + x+1].enemy) {
		if (smelldist == origdist)
			first_move = 'r';
		checkGoblins(dungeon, maxcols, pos, shortest, smelldist - 1, origdist, x + 1, y, first_move, optimal_move, 'r');
	}
	
	if (!dungeon[y*maxcols + x-1].marked && prev != 'r' && (dungeon[y*maxcols + x - 1].top == SPACE || dungeon[y*maxcols + x - 1].top == PLAYER)) { //(dungeon[y*maxcols + x-1].top != WALL && dungeon[y*maxcols + x-1].top != UNBREAKABLE_WALL && dungeon[y*maxcols + x - 1].top != LAVA) && !dungeon[y*maxcols + x-1].enemy) {
		if (smelldist == origdist)
			first_move = 'l';
		checkGoblins(dungeon, maxcols, pos, shortest, smelldist - 1, origdist, x - 1, y, first_move, optimal_move, 'l');
	}

	if (!dungeon[(y+1)*maxcols + x].marked && prev != 'u' && (dungeon[(y + 1)*maxcols + x].top == SPACE || dungeon[(y+1)*maxcols + x].top == PLAYER)) { //(dungeon[(y+1)*maxcols + x].top != WALL && dungeon[(y+1)*maxcols + x].top != UNBREAKABLE_WALL && dungeon[(y + 1)*maxcols + x].top != LAVA) && !dungeon[(y+1)*maxcols + x].enemy) {
		if (smelldist == origdist)
			first_move = 'd';
		checkGoblins(dungeon, maxcols, pos, shortest, smelldist - 1, origdist, x, y + 1, first_move, optimal_move, 'd');
	}

	if (!dungeon[(y-1)*maxcols + x].marked && prev != 'd' && (dungeon[(y - 1)*maxcols + x].top == SPACE || dungeon[(y-1)*maxcols + x].top == PLAYER)) { //(dungeon[(y-1)*maxcols + x].top != WALL && dungeon[(y-1)*maxcols + x].top != UNBREAKABLE_WALL && dungeon[(y - 1)*maxcols + x].top != LAVA) && !dungeon[(y-1)*maxcols + x].enemy) {
		if (smelldist == origdist)
			first_move = 'u';
		checkGoblins(dungeon, maxcols, pos, shortest, smelldist - 1, origdist, x, y - 1, first_move, optimal_move, 'u');
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
void Dungeon::moveGoblins(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, char move, std::vector<std::shared_ptr<Monster>> &monsters) {
	std::string monster = monsters.at(pos)->getName();

	switch (move) {
	case 'l':
		dungeon[my*maxcols + mx - 1].top = GOBLIN;
		if (monster == "bombee")
			dungeon[my*maxcols + mx - 1].top = BOMBEE;
		else if (monster == "seeker")
			dungeon[my*maxcols + mx - 1].top = SEEKER;
		dungeon[my*maxcols + mx - 1].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		moveSprite(monster_sprites, maxrows, mx, my, move); // insert monster sprites vector
		monsters.at(pos)->setPosX(mx - 1);
		break;
	case 'r':
		dungeon[my*maxcols + mx + 1].top = GOBLIN;
		if (monster == "bombee")
			dungeon[my*maxcols + mx + 1].top = BOMBEE;
		else if (monster == "seeker")
			dungeon[my*maxcols + mx + 1].top = SEEKER;
		dungeon[my*maxcols + mx + 1].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		moveSprite(monster_sprites, maxrows, mx, my, move);
		monsters.at(pos)->setPosX(mx + 1);
		break;
	case 'd':
		dungeon[(my + 1)*maxcols + mx].top = GOBLIN;
		if (monster == "bombee")
			dungeon[(my+1)*maxcols + mx].top = BOMBEE;
		else if (monster == "seeker")
			dungeon[(my + 1)*maxcols + mx].top = SEEKER;
		dungeon[(my + 1)*maxcols + mx].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		moveSprite(monster_sprites, maxrows, mx, my, move);
		monsters.at(pos)->setPosY(my + 1);
		break;
	case 'u':
		dungeon[(my - 1)*maxcols + mx].top = GOBLIN;
		if (monster == "bombee")
			dungeon[(my - 1)*maxcols + mx].top = BOMBEE;
		else if (monster == "seeker")
			dungeon[(my - 1)*maxcols + mx].top = SEEKER;
		dungeon[(my - 1)*maxcols + mx].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		moveSprite(monster_sprites, maxrows, mx, my, move);
		monsters.at(pos)->setPosY(my - 1);
		break;
	default:
		monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		break;
	}
}

void Dungeon::checkArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	shared_ptr<Archer> archer = dynamic_pointer_cast<Archer>(monsters.at(pos));
	int mx = monsters.at(pos)->getPosX();
	int my = monsters.at(pos)->getPosY();

	//	if archer is primed, check to attack and return
	if (archer->isPrimed()) {
		if (x - mx == 0 || y - my == 0)
			monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		else
			dungeonText.push_back("The archer eases their aim.\n");

		archer->prime(false);
		monsters.at(pos).reset();
		monsters.at(pos) = archer;
		archer.reset();

		return;
	}


	// if archer has dagger out and is adjacent to player
	if (monsters.at(pos)->getWeapon().getAction() == "Bronze Dagger" && \
		(dungeon[(my - 1)*maxcols + mx].top == PLAYER || dungeon[(my + 1)*maxcols + mx].top == PLAYER || dungeon[my*maxcols + mx - 1].top == PLAYER || dungeon[my*maxcols + mx + 1].top == PLAYER)) {
		monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		return;
	}
	// else if dagger is out and player moved away
	else if (monsters.at(pos)->getWeapon().getAction() == "Bronze Dagger" && \
		(dungeon[(my - 1)*maxcols + mx].top != PLAYER && dungeon[(my + 1)*maxcols + mx].top != PLAYER && dungeon[my*maxcols + mx - 1].top != PLAYER && dungeon[my*maxcols + mx + 1].top != PLAYER)) {
		monsters.at(pos)->setWeapon(WoodBow());
		dungeonText.push_back("The archer switches back to their Wood Bow.\n");
		return;
	}

	//if player is not in the archer's sights, move archer
	if (x - mx != 0 && y - my != 0) {
		moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
		return;
	}

	// if player and archer are on the same column
	if (x - mx == 0) {		
		//	if player is within range and not adjacent, get primed
		if (abs(y - my) > 1 && abs(y - my) <= 15) {
			if (!wallCollision(dungeon, maxcols, 'y', y, my)) {
				archer->prime(true);
				monsters.at(pos).reset();
				monsters.at(pos) = archer;
				archer.reset();

				dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
				return;
			}
		}
		else if (abs(y - my) == 1) {
			//swap weapon to dagger
			monsters.at(pos)->setWeapon(BronzeDagger());
			dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
		}
		//	just move archer
		else {
			moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
			return;
		}
	}
	// if player and archer are on the same row
	else if (y - my == 0) {
		//	if player is within range and not adjacent, get primed
		if (abs(x - mx) > 1 && abs(x - mx) <= 15) {
			if (!wallCollision(dungeon, maxcols, 'x', x, mx)) {
				archer->prime(true);
				monsters.at(pos).reset();
				monsters.at(pos) = archer;
				archer.reset();

				dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
				return;
			}
		}
		else if (abs(x - mx) == 1) {
			// swap weapon to dagger
			monsters.at(pos)->setWeapon(BronzeDagger());
			dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
		}
		// just move archer
		else {
			moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
		}
	}

}
void Dungeon::moveArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	int n = randInt(2);
	int m = 0;
	char move;

	char leftwall, rightwall, upwall, downwall;
	leftwall = dungeon[my*maxcols + mx - 1].wall;
	rightwall = dungeon[my*maxcols + mx + 1].wall;
	upwall = dungeon[(my - 1)*maxcols + mx].wall;
	downwall = dungeon[(my + 1)*maxcols + mx].wall;

	char leftenemy, rightenemy, upenemy, downenemy;
	leftenemy = dungeon[my*maxcols + mx - 1].enemy;
	rightenemy = dungeon[my*maxcols + mx + 1].enemy;
	upenemy = dungeon[(my - 1)*maxcols + mx].enemy;
	downenemy = dungeon[(my + 1)*maxcols + mx].enemy;

	if ((leftwall || leftenemy) && (rightwall || rightenemy) &&
		(downwall || downenemy) && (upwall || upenemy)) {//dungeon[my*maxcols + mx + 1].top != SPACE && dungeon[my*maxcols + mx - 1].top != SPACE && dungeon[(my + 1)*maxcols + mx].top != SPACE && dungeon[(my - 1)*maxcols + mx].top != SPACE) {
		//  if archer is surrounded completely, do nothing (such as goblin or wanderer blocking an archer's path)
		return;
	}
	else if ((leftwall || leftenemy) && (rightwall || rightenemy)) {//dungeon[my*maxcols + mx + 1].top != SPACE && dungeon[my*maxcols + mx - 1].top != SPACE) {
		//	if archer is surrounded by walls on left and right
		move = 'v';
	}
	else if ((downwall || downenemy) && (upwall || upenemy)) {// dungeon[(my + 1)*maxcols + mx].top != SPACE && dungeon[(my - 1)*maxcols + mx].top != SPACE) {
		//	if archer is surrounded by walls on top and bottom
		move = 'h';
	}
	else {
		//	else pick a random direction
		move = (n == 0 ? 'h' : 'v');
	}

	if (move == 'h') {
		//	move is horizontal
		n = -1 + randInt(3);
		while (n == 0 || dungeon[my*maxcols + mx + n].top != SPACE || dungeon[my*maxcols + mx + n].upper != SPACE) {
			n = -1 + randInt(3);
		}

		dungeon[my*maxcols + mx + n].top = ARCHER;
		dungeon[my*maxcols + mx + n].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;

		moveSprite(monster_sprites, maxrows, mx, my, n == -1 ? 'l' : 'r');
		monsters.at(pos)->setPosX(mx + n);
	}
	else if (move == 'v') {
		//	move is vertical
		m = -1 + randInt(3);
		while (m == 0 || dungeon[(my + m)*maxcols + mx].top != SPACE || dungeon[(my + m)*maxcols + mx].upper != SPACE) {
			m = -1 + randInt(3);
		}

		dungeon[(my + m)*maxcols + mx].top = ARCHER;
		dungeon[(my + m)*maxcols + mx].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? 'u' : 'd');
		monsters.at(pos)->setPosY(my + m);
	}

}
bool Dungeon::wallCollision(std::vector<_Tile> &dungeon, int maxcols, char direction, int p_move, int m_move) {
	int otherdir;

	// Since player and monster are on the same row/column, set otherdir to the complement of p_move/m_move
	if (direction == 'x') {
		otherdir = player.at(0).getPosY();
	}
	else {
		otherdir = player.at(0).getPosX();
	}
	char top;
	if (p_move > m_move) {
		while (p_move - m_move != 0) {
			if (direction == 'x') {
				top = dungeon[otherdir*maxcols + p_move - 1].top;
				if (top == WALL || top == UNBREAKABLE_WALL || top == DOOR_V || top == DOOR_H)
					return true;
				else
					p_move--;
			}
			else if (direction == 'y') {
				top = dungeon[(p_move - 1)*maxcols + otherdir].top;
				if (top == WALL || top == UNBREAKABLE_WALL || top == DOOR_V || top == DOOR_H)
					return true;
				else
					p_move--;
			}
		}
	}
	else {
		while (m_move - p_move != 0) {
			if (direction == 'x') {
				top = dungeon[otherdir*maxcols + m_move - 1].top;
				if (top == WALL || top == UNBREAKABLE_WALL || top == DOOR_V || top == DOOR_H)
					return true;
				else
					m_move--;
			}
			else if (direction == 'y') {
				top = dungeon[(m_move - 1)*maxcols + otherdir].top;
				if (top == WALL || top == UNBREAKABLE_WALL || top == DOOR_V || top == DOOR_H)
					return true;
				else
					m_move--;
			}
		}
	}
	return false;
}

void Dungeon::moveWanderers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	int n = -1 + randInt(3);
	int m = -1 + randInt(3);
	
	char top = dungeon[(my + m)*maxcols + mx + n].top;
	if ((top == SPACE || top == PLAYER) && !dungeon[(my + m)*maxcols + mx + n].enemy) {
		if (dungeon[(my + m)*maxcols + mx + n].top != PLAYER) {
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
				moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? 'u' : 'd');
			}
			else if (n != 0 && m == 0) {
				moveSprite(monster_sprites, maxrows, mx, my, n == -1 ? 'l' : 'r');
			}
			// wanderer moved diagonally
			else if (n != 0 && m != 0) {
				// moved left and ?
				if (n == -1) {
					moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? '1' : '3');
				}
				// moved right and ?
				else if (n == 1) {
					moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? '2' : '4');
				}
			}

			monsters.at(pos)->setPosX(mx + n);
			monsters.at(pos)->setPosY(my + m);
		}
		else {
			monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		}
	}
}
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
			//moveSprite(spinner_sprites, maxrows, mx - 1, my - 1, 'r');
			//moveSprite(spinner_sprites, maxrows, mx - 2, my - 2, 'R');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my - 1, 'r');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my - 2, 'R');
			if (my != 1 && !(my == 2 && mx == 1)) {
				dungeon[(my - 2)*maxcols + mx - 2].projectile = dungeon[(my - 1)*maxcols + mx - 1].projectile = SPACE;
				dungeon[(my - 2)*maxcols + mx].projectile = dungeon[(my - 1)*maxcols + mx].projectile = CHAIN;
				
				//spinner->getInner()->setPosition(mx * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition(mx * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx-1, my-1, 'r');
				//moveSprite(spinner_sprites, maxrows, mx-2, my-2, 'R');
			}
			if (my == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 2:
			//moveSprite(spinner_sprites, maxrows, mx, my - 1, 'r');
			//moveSprite(spinner_sprites, maxrows, mx, my - 2, 'R');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my - 1, 'r');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my - 2, 'R');
			if (my != 1) {
				dungeon[(my - 2)*maxcols + mx].projectile = dungeon[(my - 1)*maxcols + mx].projectile = SPACE;
				dungeon[(my - 2)*maxcols + mx + 2].projectile = dungeon[(my - 1)*maxcols + mx + 1].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx + 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx + 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx, my - 1, 'r');
				//moveSprite(spinner_sprites, maxrows, mx, my - 2, 'R');
				
				//moveSprite(spinner_sprites, maxrows, mx, my - 2, 'r');
			}
			if (my == 1 || mx == maxcols - 2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			// if spinner is on the left or right edge boundary, hide the projectiles
			/*if (mx == maxcols - 2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}*/
			break;
		case 3:
			//moveSprite(spinner_sprites, maxrows, mx + 1, my - 1, 'd');
			//moveSprite(spinner_sprites, maxrows, mx + 2, my - 2, 'D');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my - 1, 'd');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my - 2, 'D');
			if (my != 1)
				dungeon[(my - 2)*maxcols + mx + 2].projectile = SPACE;
			dungeon[(my - 1)*maxcols + mx + 1].projectile = SPACE;
			dungeon[(my)*maxcols + mx+1].projectile = dungeon[(my)*maxcols + mx+2].projectile = CHAIN;
			//spinner->getInner()->setPosition((mx + 1) * SPACING_FACTOR - X_OFFSET, (maxrows - my)*SPACING_FACTOR - Y_OFFSET);
			//spinner->getOuter()->setPosition((mx + 2) * SPACING_FACTOR - X_OFFSET, (maxrows - my)*SPACING_FACTOR - Y_OFFSET);
		
		
			// if spinner is on the left or right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 4:
			//moveSprite(spinner_sprites, maxrows, mx + 1, my, 'd');
			//moveSprite(spinner_sprites, maxrows, mx + 2, my, 'D');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my, 'd');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my, 'D');

			dungeon[(my)*maxcols + mx + 1].projectile = dungeon[(my)*maxcols + mx + 2].projectile = SPACE;
			// bottom right corner case
			if (my != maxrows - 2 && !(my == maxrows-3 && mx == maxcols-2)) {
				dungeon[(my + 1)*maxcols + mx + 1].projectile = dungeon[(my + 2)*maxcols + mx + 2].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx + 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx + 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx + 1, my, 'd');
				//moveSprite(spinner_sprites, maxrows, mx + 2, my, 'D');
			}
			// if spinner is on the bottom or right edge boundary, hide the projectiles
			else if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 5:
			//moveSprite(spinner_sprites, maxrows, mx + 1, my + 1, 'l');
			//moveSprite(spinner_sprites, maxrows, mx + 2, my + 2, 'L');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my + 1, 'l');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my + 2, 'L');

			if (my != maxrows - 2 && !(my == maxrows-3 && mx == maxcols-2)) {
				dungeon[(my + 1)*maxcols + mx + 1].projectile = dungeon[(my + 2)*maxcols + mx + 2].projectile = SPACE;
				dungeon[(my + 1)*maxcols + mx].projectile = dungeon[(my + 2)*maxcols + mx].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx + 1, my + 1, 'l');
				//moveSprite(spinner_sprites, maxrows, mx + 2, my + 2, 'L');
			}
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 6:
			//moveSprite(spinner_sprites, maxrows, mx, my + 1, 'l');
			//moveSprite(spinner_sprites, maxrows, mx, my + 2, 'L');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my + 1, 'l');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my + 2, 'L');
			
			if (my != maxrows - 2) {
				dungeon[(my + 1)*maxcols + mx].projectile = dungeon[(my + 2)*maxcols + mx].projectile = SPACE;
				dungeon[(my + 1)*maxcols + mx - 1].projectile = dungeon[(my + 2)*maxcols + mx - 2].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx - 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx - 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx, my + 1, 'l');
				//moveSprite(spinner_sprites, maxrows, mx, my + 2, 'L');
			}
			// if spinner is on the left or bottom edge boundary, hide the projectiles
			else if (my == maxrows - 2 || mx == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 7:
			//moveSprite(spinner_sprites, maxrows, mx - 1, my + 1, 'u');
			//moveSprite(spinner_sprites, maxrows, mx - 2, my + 2, 'U');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my + 1, 'u');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my + 2, 'U');

			if (my != maxrows - 2)
				dungeon[(my + 2)*maxcols + mx - 2].projectile = SPACE;
			dungeon[(my + 1)*maxcols + mx - 1].projectile = SPACE;
			dungeon[(my)*maxcols + mx-1].projectile = dungeon[(my)*maxcols + mx-2].projectile = CHAIN;
			//spinner->getInner()->setPosition((mx - 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my))*SPACING_FACTOR - Y_OFFSET);
			//spinner->getOuter()->setPosition((mx - 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my))*SPACING_FACTOR - Y_OFFSET);
			
			//moveSprite(spinner_sprites, maxrows, mx - 1, my + 1, 'u');
			//moveSprite(spinner_sprites, maxrows, mx - 2, my + 2, 'U');
			
			// if spinner is on the left edge boundary, hide the projectiles
			if (mx == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 8:
			//moveSprite(spinner_sprites, maxrows, mx - 1, my, 'u');
			//moveSprite(spinner_sprites, maxrows, mx - 2, my, 'U');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my, 'u');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my, 'U');

			dungeon[(my)*maxcols + mx - 1].projectile = dungeon[(my)*maxcols + mx - 2].projectile = SPACE;
			if (my != 1 && !(my == 2 && mx == 1)) { // literally a corner case (top-left corner)
				dungeon[(my - 1)*maxcols + mx - 1].projectile = dungeon[(my - 2)*maxcols + mx - 2].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx - 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx - 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx - 1, my, 'u');
				//moveSprite(spinner_sprites, maxrows, mx - 2, my, 'U');
			}
			// if spinner is on the left or top edge boundary, hide the projectiles
			else if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
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
			//moveSprite(spinner_sprites, maxrows, mx + 1, my - 1, 'l');
			//moveSprite(spinner_sprites, maxrows, mx + 2, my - 2, 'L');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my - 1, 'l');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my - 2, 'L');

			if (my != 1) {
				dungeon[(my - 2)*maxcols + mx + 2].projectile = dungeon[(my - 1)*maxcols + mx + 1].projectile = SPACE;
				dungeon[(my - 2)*maxcols + mx].projectile = dungeon[(my - 1)*maxcols + mx].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 2))*SPACING_FACTOR - Y_OFFSET);
			}
			if (my == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 2:
			//moveSprite(spinner_sprites, maxrows, mx + 1, my, 'u');
			//moveSprite(spinner_sprites, maxrows, mx + 2, my, 'U');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my, 'u');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my, 'U');

			dungeon[(my)*maxcols + mx + 1].projectile = dungeon[(my)*maxcols + mx + 2].projectile = SPACE;
			if (my != 1) {
				dungeon[(my - 2)*maxcols + mx + 2].projectile = dungeon[(my - 1)*maxcols + mx + 1].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx + 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx + 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx + 1, my, 'u');
				//moveSprite(spinner_sprites, maxrows, mx + 2, my, 'U');
			}
			// if spinner is on the left or right edge boundary, hide the projectiles
			if (my == 1 || mx == maxcols-2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 3:
			//moveSprite(spinner_sprites, maxrows, mx + 1, my + 1, 'u');
			//moveSprite(spinner_sprites, maxrows, mx + 2, my + 2, 'U');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 1, my + 1, 'u');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx + 2, my + 2, 'U');

			if (my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))
				dungeon[(my + 2)*maxcols + mx + 2].projectile = SPACE;
			dungeon[(my + 1)*maxcols + mx + 1].projectile = SPACE;
			dungeon[(my)*maxcols + mx + 1].projectile = dungeon[(my)*maxcols + mx + 2].projectile = CHAIN;
			//spinner->getInner()->setPosition((mx + 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my))*SPACING_FACTOR - Y_OFFSET);
			//spinner->getOuter()->setPosition((mx + 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my))*SPACING_FACTOR - Y_OFFSET);
			
			//moveSprite(spinner_sprites, maxrows, mx + 1, my + 1, 'u');
			//moveSprite(spinner_sprites, maxrows, mx + 2, my + 2, 'U');

			// if spinner is on the right edge boundary, hide the projectiles
			if (mx == maxcols - 2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 4:
			//moveSprite(spinner_sprites, maxrows, mx, my + 1, 'r');
			//moveSprite(spinner_sprites, maxrows, mx, my + 2, 'R');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my + 1, 'r');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my + 2, 'R');

			if (my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) {
				dungeon[(my + 1)*maxcols + mx].projectile = dungeon[(my + 2)*maxcols + mx].projectile = SPACE;
				dungeon[(my + 1)*maxcols + mx + 1].projectile = dungeon[(my + 2)*maxcols + mx + 2].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx + 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx + 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx, my + 1, 'r');
				//moveSprite(spinner_sprites, maxrows, mx, my + 2, 'R');
			}
			// if spinner is on the bottom or right edge boundary, hide the projectiles
			else if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) || mx == maxcols - 2){
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 5:
			//moveSprite(spinner_sprites, maxrows, mx - 1, my + 1, 'r');
			//moveSprite(spinner_sprites, maxrows, mx - 2, my + 2, 'R');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my + 1, 'r');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my + 2, 'R');

			if (my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2)) {
				dungeon[(my + 1)*maxcols + mx - 1].projectile = dungeon[(my + 2)*maxcols + mx - 2].projectile = SPACE;
				dungeon[(my + 1)*maxcols + mx].projectile = dungeon[(my + 2)*maxcols + mx].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx - 1, my + 1, 'r');
				//moveSprite(spinner_sprites, maxrows, mx - 2, my + 2, 'R');
			}
			if (!(my != maxrows - 2 && !(my == maxrows - 3 && mx == maxcols - 2))) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 6:
			//moveSprite(spinner_sprites, maxrows, mx - 1, my, 'd');
			//moveSprite(spinner_sprites, maxrows, mx - 2, my, 'D');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my, 'd');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my, 'D');

			dungeon[(my)*maxcols + mx - 1].projectile = dungeon[(my)*maxcols + mx - 2].projectile = SPACE;
			if (my != maxrows - 2) {
				dungeon[(my + 1)*maxcols + mx - 1].projectile = dungeon[(my + 2)*maxcols + mx - 2].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx - 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx - 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my + 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx - 1, my, 'd');
				//moveSprite(spinner_sprites, maxrows, mx - 2, my, 'D');
			}
			// if spinner is on the left or bottom edge boundary, hide the projectiles
			if (mx == 1 || my == maxrows-2) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 7:
			//moveSprite(spinner_sprites, maxrows, mx - 1, my - 1, 'd');
			//moveSprite(spinner_sprites, maxrows, mx - 2, my - 2, 'D');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 1, my - 1, 'd');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx - 2, my - 2, 'D');

			if (my != 1 && !(my == 2 && mx == 1))
				dungeon[(my - 2)*maxcols + mx - 2].projectile = SPACE;
			dungeon[(my - 1)*maxcols + mx - 1].projectile = SPACE;
			dungeon[(my)*maxcols + mx - 1].projectile = dungeon[(my)*maxcols + mx - 2].projectile = CHAIN;
			//spinner->getInner()->setPosition((mx - 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my))*SPACING_FACTOR - Y_OFFSET);
			//spinner->getOuter()->setPosition((mx - 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my))*SPACING_FACTOR - Y_OFFSET);
			
			//moveSprite(spinner_sprites, maxrows, mx - 1, my - 1, 'd');
			//moveSprite(spinner_sprites, maxrows, mx - 2, my - 2, 'D');
			
			// if spinner is on the left or top edge boundary, hide the projectiles
			if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
				spinner->getInner()->setVisible(false);
				spinner->getOuter()->setVisible(false);
			}
			break;
		case 8:
			//moveSprite(spinner_sprites, maxrows, mx, my - 1, 'l');
			//moveSprite(spinner_sprites, maxrows, mx, my - 2, 'L');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my - 1, 'l');
			moveAssociatedSprite(spinner_sprites, maxrows, mx, my, mx, my - 2, 'L');

			if (my != 1 && !(my == 2 && mx == 1)) {
				dungeon[(my - 2)*maxcols + mx].projectile = dungeon[(my - 1)*maxcols + mx].projectile = SPACE;
				dungeon[(my - 1)*maxcols + mx - 1].projectile = dungeon[(my - 2)*maxcols + mx - 2].projectile = CHAIN;
				//spinner->getInner()->setPosition((mx - 1) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 1))*SPACING_FACTOR - Y_OFFSET);
				//spinner->getOuter()->setPosition((mx - 2) * SPACING_FACTOR - X_OFFSET, (maxrows - (my - 2))*SPACING_FACTOR - Y_OFFSET);
			
				//moveSprite(spinner_sprites, maxrows, mx, my - 1, 'l');
				//moveSprite(spinner_sprites, maxrows, mx, my - 2, 'L');
			}
			// if spinner is on the left or top edge boundary, hide the projectiles
			else if (!(my != 1 && !(my == 2 && mx == 1)) || mx == 1) {
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

	monsters.at(pos).reset();
	monsters.at(pos) = spinner;
	
	if (dungeon[py*maxcols + px].projectile == CHAIN && abs(px - mx) <= 2 && abs(py - my) <= 2) {
		monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
	}
	spinner.reset();
}

void Dungeon::moveRoundabouts(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	std::shared_ptr<Roundabout> ra = std::dynamic_pointer_cast<Roundabout>(monsters.at(pos));
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	if (ra->getDirection() == 'l') {
		//	if tile to left of the roundabout is a wall, check above
		if (dungeon[my*maxcols + mx - 1].top != SPACE) {
			//	if player is in the way, attack them
			if (dungeon[my*maxcols + mx - 1].top == PLAYER) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}

			//	if tile above is a wall, then set direction to right
			if (dungeon[(my - 1)*maxcols + mx].top != SPACE) {
				if (dungeon[(my - 1)*maxcols + mx].top == PLAYER) {
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
			moveSprite(monster_sprites, maxrows, mx, my, ra->getDirection());
			monsters.at(pos)->setPosX(mx - 1);
		}
	}
	else if (ra->getDirection() == 'r') {
		//	if tile to right of the roundabout is a wall, check below
		if (dungeon[my*maxcols + mx + 1].top != SPACE) {
			//	if player is in the way, attack them
			if (dungeon[my*maxcols + mx + 1].top == PLAYER) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}

			//	if tile below is a wall, then set direction to left
			if (dungeon[(my + 1)*maxcols + mx].top != SPACE) {
				if (dungeon[(my + 1)*maxcols + mx].top == PLAYER) {
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
			moveSprite(monster_sprites, maxrows, mx, my, ra->getDirection());
			monsters.at(pos)->setPosX(mx + 1);
		}
	}
	else if (ra->getDirection() == 'u') {
		//	if tile above the roundabout is a wall, check right
		if (dungeon[(my-1)*maxcols + mx].top != SPACE) {
			//	if player is in the way, attack them
			if (dungeon[(my-1)*maxcols + mx].top == PLAYER) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}

			//	if tile to the right is a wall, then set direction downward
			if (dungeon[my*maxcols + mx + 1].top != SPACE) {
				if (dungeon[my*maxcols + mx + 1].top == PLAYER) {
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
			dungeon[(my-1)*maxcols + mx].top = ROUNDABOUT;
			dungeon[(my-1)*maxcols + mx].enemy = true;
			moveSprite(monster_sprites, maxrows, mx, my, ra->getDirection());
			monsters.at(pos)->setPosY(my - 1);
		}
	}
	else if (ra->getDirection() == 'd') {
		//	if tile below the roundabout is a wall, check left
		if (dungeon[(my + 1)*maxcols + mx].top != SPACE) {
			//	if player is in the way, attack them
			if (dungeon[(my + 1)*maxcols + mx].top == PLAYER) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}

			//	if tile to the left is a wall, then set direction upward
			if (dungeon[my*maxcols + mx - 1].top != SPACE) {
				if (dungeon[my*maxcols + mx - 1].top == PLAYER) {
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
			moveSprite(monster_sprites, maxrows, mx, my, ra->getDirection());
			monsters.at(pos)->setPosY(my + 1);
		}
	}

	ra.reset();
}

void Dungeon::checkMountedKnights(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	std::shared_ptr<MountedKnight> knight = std::dynamic_pointer_cast<MountedKnight>(monsters.at(pos));
	int mx = monsters.at(pos)->getPosX();
	int my = monsters.at(pos)->getPosY();

	//	if knight was alerted, check to attack, stop, or advance and return
	if (knight->isAlerted()) {
		char direction = knight->getDirection();
		if (direction == 'l') {
			if (dungeon[my*maxcols + mx - 1].top == PLAYER) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
			else if (dungeon[my*maxcols + mx - 1].top != SPACE) {
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
				moveSprite(monster_sprites, maxrows, mx, my, direction);
				monsters.at(pos)->setPosX(mx - 1);
			}
		}
		else if (direction == 'r') {
			if (dungeon[my*maxcols + mx + 1].top == PLAYER) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
			else if (dungeon[my*maxcols + mx + 1].top != SPACE) {
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
				moveSprite(monster_sprites, maxrows, mx, my, direction);
				monsters.at(pos)->setPosX(mx + 1);
			}
		}
		else if (direction == 'u') {
			if (dungeon[(my-1)*maxcols + mx].top == PLAYER) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
			else if (dungeon[(my - 1)*maxcols + mx].top != SPACE) {
				knight->toggleAlert();
				monsters.at(pos) = knight;
				knight.reset();

				return;
			}
			else {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;
				dungeon[(my-1)*maxcols + mx].top = MOUNTED_KNIGHT;
				dungeon[(my-1)*maxcols + mx].enemy = true;
				moveSprite(monster_sprites, maxrows, mx, my, direction);
				monsters.at(pos)->setPosY(my - 1);
			}
		}
		else if (direction == 'd') {
			if (dungeon[(my + 1)*maxcols + mx].top == PLAYER) {
				monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
			}
			else if (dungeon[(my + 1)*maxcols + mx].top != SPACE) {
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
				moveSprite(monster_sprites, maxrows, mx, my, direction);
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

	int n = randInt(2);
	int m = 0;
	char move;

	if (dungeon[my*maxcols + mx + 1].top != SPACE && dungeon[my*maxcols + mx - 1].top != SPACE && dungeon[(my + 1)*maxcols + mx].top != SPACE && dungeon[(my - 1)*maxcols + mx].top != SPACE) {
		//  if knight is surrounded completely, do nothing (such as goblin or wanderer blocking an archer's path)
		return;
	}
	else if (dungeon[my*maxcols + mx + 1].top != SPACE && dungeon[my*maxcols + mx - 1].top != SPACE) {
		//	if knight is surrounded by walls on left and right
		move = 'v';
	}
	else if (dungeon[(my + 1)*maxcols + mx].top != SPACE && dungeon[(my - 1)*maxcols + mx].top != SPACE) {
		//	if knight is surrounded by walls on top and bottom
		move = 'h';
	}
	else {
		//	else pick a random direction
		move = (n == 0 ? 'h' : 'v');
	}

	if (move == 'h') {
		//	move is horizontal
		n = -1 + randInt(3);
		while (n == 0 || dungeon[my*maxcols + mx + n].top != SPACE) {
			n = -1 + randInt(3);
		}

		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		dungeon[my*maxcols + mx + n].top = MOUNTED_KNIGHT;
		dungeon[my*maxcols + mx + n].enemy = true;
		moveSprite(monster_sprites, maxrows, mx, my, n == -1 ? 'l' : 'r');
		monsters.at(pos)->setPosX(mx + n);
	}
	else if (move == 'v') {
		//	move is vertical
		m = -1 + randInt(3);
		while (m == 0 || dungeon[(my + m)*maxcols + mx].top != SPACE) {
			m = -1 + randInt(3);
		}

		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		dungeon[(my + m)*maxcols + mx].top = MOUNTED_KNIGHT;
		dungeon[(my + m)*maxcols + mx].enemy = true;
		moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? 'u' : 'd');
		monsters.at(pos)->setPosY(my + m);
	}
}

void Dungeon::moveSeekers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, char move, std::vector<std::shared_ptr<Monster>> &monsters) {
	std::shared_ptr<Seeker> seeker = std::dynamic_pointer_cast<Seeker>(monsters.at(pos));

	if (seeker->getStep())
		moveGoblins(dungeon, maxrows, maxcols, mx, my, pos, move, monsters);

	seeker->toggleStep();
	monsters.at(pos) = seeker;
	seeker.reset();
}

int Dungeon::findMonster(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, std::vector<std::shared_ptr<Monster>> monsters) {
	for (int i = 0; i < monsters.size(); i++) {
		if (monsters.at(i)->getPosX() == mx && monsters.at(i)->getPosY() == my) {
			return i;
		}
	}
	return -1;
}
void Dungeon::fight(int x, int y) { // monster's coordinates
	int i = findMonster(m_maze, MAXROWS, MAXCOLS, x, y, m_monsters);
	
	if (i == -1)
		return;

	player.at(0).attack(m_monsters, m_actives, i, dungeonText);
	if (m_monsters.at(i)->getHP() <= 0) {
		monsterDeath(m_maze, MAXROWS, MAXCOLS, m_actives, m_monsters, i);// x, y, i);
	}
}

void Dungeon::foundItem(std::vector<_Tile> &dungeon, int maxcols, int x, int y) {
	char c = dungeon[y*maxcols + x].bottom;

	dungeonText.push_back("You've found ");
	switch (c) {
		// WEAPONS
	case CUTLASS: dungeonText.push_back("a Rusty Cutlass.\n"); break;
	case BONEAXE: dungeonText.push_back("a Bone Axe.\n"); break;
	case BRONZE_DAGGER: dungeonText.push_back("a Bronze Dagger.\n"); break;
	case IRON_LANCE: dungeonText.push_back("an Iron Lance.\n"); break;
		// ITEMS
	case HEART_POD: dungeonText.push_back("a Heart Pod.\n"); break;
	case LIFEPOT: dungeonText.push_back("a Life Potion.\n"); break;
	case ARMOR: dungeonText.push_back("some extra Armor.\n"); break;
	case STATPOT: dungeonText.push_back("a Stat Potion.\n"); break;
	case BOMB: dungeonText.push_back("a Bomb!\n"); break;
	case BROWN_CHEST: dungeonText.push_back("a Brown Chest!\n"); break;
	case SILVER_CHEST: dungeonText.push_back("a Silver Chest!\n"); break;
	case GOLDEN_CHEST: dungeonText.push_back("a Golden Chest!\n"); break;
	case SKELETON_KEY: dungeonText.push_back("a Mysterious Trinket.\n"); break;
	case INFINITY_BOX: dungeonText.push_back("an ornate box, emanating with power.\n"); break;
		// OBJECTS
	case STAIRS: dungeonText.push_back("stairs!\n"); break;
	case LOCKED_STAIRS: dungeonText.push_back("stairs but it's blocked off...\n"); break;
	case BUTTON: dungeonText.push_back("a button!?\n"); break;
	default:
		break;
	}
}
void Dungeon::collectItem(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y) {
	int current_inv_size = player.at(0).getInventorySize() + player.at(0).getItemInvSize();
	int current_wep_inv_size = player.at(0).getInventorySize();
	int current_item_inv_size = player.at(0).getItemInvSize();
	int max_wep_inv_size = player.at(0).getMaxWeaponInvSize();
	int max_item_inv_size = player.at(0).getMaxItemInvSize();
	std::string image;

	char bottom = dungeon[y*maxcols + x].bottom;
	char traptile = dungeon[y*maxcols + x].traptile;

	//		WEAPONS
	if (bottom == CUTLASS) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(RustyCutlass(x, y));

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup2.mp3", false, 1.0f);

			dungeonText.push_back("You pick up the Rusty Cutlass.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (bottom == BONEAXE) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(BoneAxe(x,y)); //adds bone axe to inventory

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup2.mp3", false, 1.0f);

			dungeonText.push_back("You pick up the Bone Axe.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (bottom == BRONZE_DAGGER) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(BronzeDagger());

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup.mp3", false, 1.0f);

			dungeonText.push_back("You pick up the Bronze Dagger.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (bottom == IRON_LANCE) {
		if (current_wep_inv_size < max_wep_inv_size) {
			player.at(0).addWeapon(IronLance());

			cocos2d::experimental::AudioEngine::play2d("Weapon_Pickup.mp3", false, 1.0f);

			dungeonText.push_back("You pick up the Iron Lance.\n");
		}
		else {
			dungeonText.push_back("Your backpack is full!\n");
		}
	}

	//		BEGIN DROPS IF STATEMENTS
	else if (bottom == HEART_POD) {
		cocos2d::experimental::AudioEngine::play2d("Heart_Pod_Pickup2.mp3", false, 1.0f);
		
		HeartPod heart;
		heart.changeStats(heart, player.at(0));

		dungeon[y*maxcols + x].bottom = SPACE;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
	else if (bottom == LIFEPOT) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(LifePotion());

			cocos2d::experimental::AudioEngine::play2d("Potion_Pickup2.mp3", false, 1.0f);

			dungeonText.push_back("You grab the Life Potion!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (bottom == ARMOR) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(ArmorDrop());

			cocos2d::experimental::AudioEngine::play2d("Armor_Pickup.mp3", false, 1.0f);

			dungeonText.push_back("You grab the Armor!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (bottom == STATPOT) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(StatPotion());

			cocos2d::experimental::AudioEngine::play2d("Potion_Pickup2.mp3", false, 1.0f);

			dungeonText.push_back("You grab the Stat Potion!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (bottom == BOMB) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(Bomb());

			cocos2d::experimental::AudioEngine::play2d("Bomb_Pickup2.mp3", false, 1.0f);

			dungeonText.push_back("You grab the Bomb!\n");
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (bottom == SKELETON_KEY) {
		if (current_item_inv_size < max_item_inv_size) {
			player.at(0).addItem(SkeletonKey());

			cocos2d::experimental::AudioEngine::play2d("Key_Pickup.mp3", false, 1.0f);

			dungeonText.push_back("You pick up the Mysterious Trinket.\n");
			player.at(0).setKeyHP(player.at(0).getHP()); // set key hp (hp at time of pickup)
		}
		else {
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (bottom == BROWN_CHEST) {
		BrownChest c;

		cocos2d::experimental::AudioEngine::play2d("ChestOpening1.mp3", false, 1.0f);

		dungeonText.push_back("You open the Brown Chest... ");

		removeSprite(item_sprites, maxrows, x, y);
		image = c.open(dungeon[y*maxcols + x], dungeonText);
		addSprite(item_sprites, maxrows, x, y, -1, image);
	}
	else if (bottom == SILVER_CHEST) {
		SilverChest c;

		cocos2d::experimental::AudioEngine::play2d("ChestOpening1.mp3", false, 1.0f);

		dungeonText.push_back("You open the Silver Chest... ");

		removeSprite(item_sprites, maxrows, x, y);
		image = c.open(dungeon[y*maxcols + x], dungeonText);
		addSprite(item_sprites, maxrows, x, y, -1, image);
	}
	else if (bottom == GOLDEN_CHEST) {
		GoldenChest c;

		cocos2d::experimental::AudioEngine::play2d("ChestOpening1.mp3", false, 1.0f);

		dungeonText.push_back("You open the Golden Chest... ");

		removeSprite(item_sprites, maxrows, x, y);
		image = c.open(dungeon[y*maxcols + x], dungeonText);
		addSprite(item_sprites, maxrows, x, y, -1, image);
	}
	else if (bottom == INFINITY_BOX) {
		dungeonText.push_back("The ornate box is sealed shut.\n");
	}


	//		MISC
	else if (bottom == IDOL) {
		player.at(0).setWin(true);
		dungeon[y*maxcols + x].bottom = SPACE;
	}
	else if (traptile == STAIRS) {
		setLevel(getLevel() + 1);
		clearScreen();
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
	if (current_inv_size < player.at(0).getInventorySize() + player.at(0).getItemInvSize()) {
		dungeon[y*maxcols + x].bottom = SPACE;
		dungeon[y*maxcols + x].item = false;
		removeSprite(item_sprites, maxrows, x, y);
	}
}
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
void Dungeon::showText() {
	for (unsigned i = 0; i < dungeonText.size(); i++) {
		cout << dungeonText.at(i);
	}
	dungeonText.clear();
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
std::vector<_Tile> Dungeon::getDungeon() const {
	return m_maze;
}
std::vector<std::shared_ptr<Monster>> Dungeon::getMonsters() const {
	return m_monsters;
}
std::vector<std::shared_ptr<Objects>> Dungeon::getActives() const {
	return m_actives;
}

void Dungeon::setPlayerSprite(cocos2d::Sprite* sprite) {
	player_sprite = sprite;
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
void Dungeon::setScene(cocos2d::Scene* scene) {
	m_scene = scene;
}

void Dungeon::unmarkTiles() {
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 70; j++) {
			m_maze[i*MAXCOLS + j].marked = false;
		}
	}
}
int Dungeon::getRows() const {
	return m_rows;
}
int Dungeon::getCols() const {
	return m_cols;
}
void Dungeon::callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index) {
	string item = player.at(0).getItemInv().at(index).getItem(); // get item name
	std::string image = player.at(0).use(m_actives, dungeon[y*maxcols + x], index); // gets sprite, if any
	if (item == "Bomb")
		addSprite(projectile_sprites, maxrows, x, y, -1, image);
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
										  {'#', SILVER_CHEST, SILVER_CHEST, '#'},
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


//	SECOND FLOOR FUNCTIONS
SecondFloor::SecondFloor(Player p) : Dungeon(2), m_openexit(false), m_watersUsed(false), m_watersCleared(true), m_guardians(-1) {
	//setLevel(2);

	_Tile *tile;
	int i, j;

	//	initialize entire dungeon to blank space
	for (i = 0; i < MAXROWS2; i++) {
		for (j = 0; j < MAXCOLS2; j++) {
			tile = new _Tile;
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = tile->extra = SPACE;
			tile->wall = tile->item = tile->trap = tile->enemy = false;

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
	for (i = 1; i < MAXROWS2 - 1; i++) {
		for (j = 1; j < MAXCOLS2 - 1; j++) {
			m_maze2[i*MAXCOLS2 + j].top = finalvec.at(count);
			m_maze2[i*MAXCOLS2 + j].bottom = finalvec.at(count);
			if (m_maze2[i*MAXCOLS2 + j].top == WALL || m_maze2[i*MAXCOLS2 + j].top == UNBREAKABLE_WALL) {
				m_maze2[i*MAXCOLS2 + j].traptile = finalvec.at(count);
				m_maze2[i*MAXCOLS2 + j].wall = true;
			}
			if (finalvec.at(count) == DEVILS_WATER || finalvec.at(count) == LAVA) {
				m_maze2[i*MAXCOLS2 + j].top = SPACE;
				m_maze2[i*MAXCOLS2 + j].bottom = SPACE;
				m_maze2[i*MAXCOLS2 + j].traptile = finalvec.at(count);
			}

			//	if the object was an item or enemy
			if (m_maze2[i*MAXCOLS2 + j].bottom != WALL && m_maze2[i*MAXCOLS2 + j].bottom != SPACE) {

				object = m_maze2[i*MAXCOLS2 + j].bottom;
				switch (object) {
				/*case LAVA:
					m_maze2[i*MAXCOLS2 + j].bottom = SPACE;
					break;*/
				case BROWN_CHEST:
				case SILVER_CHEST:
					m_maze2[i*MAXCOLS2 + j].top = SPACE;
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


	//	spawn player
	player.at(0) = p;
	player.at(0).setrandPosX(MAXCOLS2);
	player.at(0).setrandPosY(MAXROWS2);

	char toptile = m_maze2[player.at(0).getPosY()*MAXCOLS2 + player.at(0).getPosX()].top;
	char bottomtile = m_maze2[player.at(0).getPosY()*MAXCOLS2 + player.at(0).getPosX()].bottom;

	while (toptile != SPACE || bottomtile != SPACE || player.at(0).getPosX() > 4 && player.at(0).getPosX() < MAXCOLS2 - 5) {
		player.at(0).setrandPosX(MAXCOLS2);
		player.at(0).setrandPosY(MAXROWS2);

		// while player is not spawned along left side or right side, reroll
		while (player.at(0).getPosX() > 4 && player.at(0).getPosX() < MAXCOLS2 - 5) {
			player.at(0).setrandPosX(MAXCOLS2);
		}

		toptile = m_maze2[player.at(0).getPosY()*MAXCOLS2 + player.at(0).getPosX()].top;
		bottomtile = m_maze2[player.at(0).getPosY()*MAXCOLS2 + player.at(0).getPosX()].bottom;
	}
	m_maze2[player.at(0).getPosY()*MAXCOLS2 + player.at(0).getPosX()].top = PLAYER; // sets tile at this spot to be player position


	Stairs stairs;
	char traptile = m_maze2[stairs.getPosY()*MAXCOLS2 + stairs.getPosX()].traptile;
	//toptile = m_maze2[stairs.getPosY()*MAXCOLS2 + stairs.getPosX()].top;
	//bottomtile = m_maze2[stairs.getPosY()*MAXCOLS2 + stairs.getPosX()].bottom;

	///	while stairs does not spawn on the side opposite from the player
	while (toptile != SPACE || bottomtile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() < MAXCOLS2-5) || (player.at(0).getPosX() > MAXCOLS2-6 && stairs.getPosX() > 4))) {
	//while (toptile != SPACE || bottomtile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() >= 5) || (player.at(0).getPosX() > MAXCOLS2-6 && stairs.getPosX() < MAXCOLS2 - 6))) {
		stairs.setrandPosX(MAXCOLS2);
		stairs.setrandPosY(MAXROWS2);

		traptile = m_maze2[stairs.getPosY()*MAXCOLS2 + stairs.getPosX()].traptile;
		//toptile = m_maze2[stairs.getPosY()*MAXCOLS2 + stairs.getPosX()].top;
		//bottomtile = m_maze2[stairs.getPosY()*MAXCOLS2 + stairs.getPosX()].bottom;
	}
	m_maze2[stairs.getPosY()*MAXCOLS2 + stairs.getPosX()].traptile = LOCKED_STAIRS;
	m_stairsX = stairs.getPosX();
	m_stairsY = stairs.getPosY();


	//	BUTTON for unlocking exit
	Button button;
	button.setPosX(MAXCOLS2/2-1);
	button.setPosY(MAXROWS2/2-4);
	m_maze2[button.getPosY()*MAXCOLS2 + button.getPosX()].traptile = BUTTON;


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

			toptile = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].top;
			bottomtile = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].bottom;
			while (toptile != SPACE || bottomtile == BUTTON) { // while monster position clashes with wall, player, or idol
				monster->setrandPosX(MAXCOLS2);				// reroll it
				monster->setrandPosY(MAXROWS2);

				toptile = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].top;
				bottomtile = m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].bottom;
			}
			m_f2monsters.emplace_back(monster);
			m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].top = monstertype;
			m_maze2[monster->getPosY()*MAXCOLS2 + monster->getPosX()].enemy = true;
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

void SecondFloor::peekSecondFloor(int x, int y, char move) {
	char top, bottom, traptile;
	bool wall, item, trap, enemy;

	//move player
	int weprange = player.at(0).getWeapon().getRange();
	bool move_used = false;
	bool far_enemy, close_enemy;
	// if weapon has a range of 2, check for monsters to attack
	if (weprange == 2) {
		if (move == 'l') { // moved to the left
			top = m_maze2[y*MAXCOLS2 + x - 1].top;
			close_enemy = m_maze2[y*MAXCOLS2 + x - 1].enemy;
			far_enemy = m_maze2[y*MAXCOLS2 + x - 2].enemy;

			if (close_enemy) {
				fight(x - 1, y);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && y*MAXCOLS2 + x - 2 > 0) {	// monster encountered
				fight(x - 2, y);
				move_used = true;
			}
		}
		else if (move == 'r') {
			top = m_maze2[y*MAXCOLS2 + x + 1].top;
			close_enemy = m_maze2[y*MAXCOLS2 + x + 1].enemy;
			far_enemy = m_maze2[y*MAXCOLS2 + x + 2].enemy;

			if (close_enemy) {
				fight(x + 1, y);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && y*MAXCOLS2 + x + 2 < MAXROWS2 * MAXCOLS2) {
				fight(x + 2, y);
				move_used = true;
			}
		}
		else if (move == 'u') { // moved up
			top = m_maze2[(y - 1)*MAXCOLS2 + x].top;
			close_enemy = m_maze2[(y - 1)*MAXCOLS2 + x].enemy;
			// if player is along top edge, assign far_enemy to close_enemy instead
			far_enemy = (y == 1 ? close_enemy : m_maze2[(y - 2)*MAXCOLS2 + x].enemy);
			//far_enemy = m_maze2[(y - 2)*MAXCOLS2 + x].enemy;

			if (close_enemy) {
				fight(x, y - 1);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && (y - 2)*MAXCOLS2 + x > 0) {
				fight(x, y - 2);
				move_used = true;
			}
		}
		else if (move == 'd') { // moved down
			top = m_maze2[(y + 1)*MAXCOLS2 + x].top;
			close_enemy = m_maze2[(y + 1)*MAXCOLS2 + x].enemy;
			// if player is along top edge, assign far_enemy to close_enemy instead
			far_enemy = (y == MAXROWS2-2 ? close_enemy : m_maze2[(y + 2)*MAXCOLS2 + x].enemy);
			//far_enemy = m_maze2[(y + 2)*MAXCOLS2 + x].enemy;

			if (close_enemy) {
				fight(x, y + 1);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && (y + 2)*MAXCOLS2 + x < MAXROWS2 * MAXCOLS2) {
				fight(x, y + 2);
				move_used = true;
			}
		}
	}

	if (!move_used) {
		if (move == 'l') { // moved to the left
			top = m_maze2[y*MAXCOLS2 + x - 1].top;
			bottom = m_maze2[y*MAXCOLS2 + x - 1].bottom;
			traptile = m_maze2[y*MAXCOLS2 + x - 1].traptile;
			
			wall = m_maze2[y*MAXCOLS2 + x - 1].wall;
			item = m_maze2[y*MAXCOLS2 + x - 1].item;
			trap = m_maze2[y*MAXCOLS2 + x - 1].trap;
			enemy = m_maze2[y*MAXCOLS2 + x - 1].enemy;

			if (trap) {
				if (traptile == LAVA) {
					// lava sound
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						m_f2actives.emplace_back(new Burn());
						player.at(0).toggleBurn();
					}
				}
				if (traptile == BUTTON) {
					if (!m_openexit) {
						cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
						cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

						// replace button with pressed button
						removeSprite(trap_sprites, MAXROWS2, x - 1, y);
						addSprite(trap_sprites, MAXROWS2, x - 1, y, -1, "Button_Pressed_48x48.png");

						// replace locked stairs with open staircase
						removeSprite(trap_sprites, MAXROWS2, m_stairsX, m_stairsY);
						addSprite(trap_sprites, MAXROWS2, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");


						m_openexit = true;
						m_maze2[m_stairsY*MAXCOLS2 + m_stairsX].traptile = STAIRS;
						dungeonText.push_back("The stairs have been unlocked!\n");
					}
				}
				if (traptile == DEVILS_WATER) {
					if (!m_watersUsed) {
						devilsWaterPrompt();
					}
				}
			}
			if (!(wall || item || enemy)) {
				// move character to the left
				m_maze2[y*MAXCOLS2 + x - 1].top = PLAYER;
				m_maze2[y*MAXCOLS2 + x].top = SPACE;
				player.at(0).setPosX(x - 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of item
				m_maze2[y*MAXCOLS2 + x - 1].top = PLAYER;
				m_maze2[y*MAXCOLS2 + x].top = SPACE;
				player.at(0).setPosX(x - 1);

				foundItem(m_maze2, MAXCOLS2, x - 1, y);
			}
			else if (top == SPACE && bottom == BUTTON) {
				/*m_maze2[y*MAXCOLS2 + x - 1].top = PLAYER;
				m_maze2[y*MAXCOLS2 + x].top = SPACE;
				player.at(0).setPosX(x - 1);*/
				//if (!m_openexit) {
				//	cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
				//	cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

				//	// replace button with pressed button
				//	removeSprite(item_sprites, MAXROWS2, x - 1, y);
				//	addSprite(item_sprites, MAXROWS2, x - 1, y, -1, "Button_Pressed_48x48.png");

				//	// replace locked stairs with open staircase
				//	removeSprite(wall_sprites, MAXROWS2, m_stairsX, m_stairsY);
				//	addSprite(item_sprites, MAXROWS2, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");


				//	m_openexit = true;
				//	m_maze2[m_stairsY*MAXCOLS2 + m_stairsX].traptile = STAIRS;
				//	dungeonText.push_back("The stairs have been unlocked!\n");
				//}
			}
			else if (top == LAVA || bottom == LAVA) {
				//m_maze2[y*MAXCOLS2 + x - 1].top = PLAYER;
				//player.at(0).setPosX(x - 1);
				//if (m_maze2[y*MAXCOLS2 + x].bottom == LAVA)
				//	m_maze2[y*MAXCOLS2 + x].top = LAVA;
				//else
				//	m_maze2[y*MAXCOLS2 + x].top = SPACE;

				//// lava sound
				//cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				//player.at(0).setHP(player.at(0).getHP() - 5);
				//dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				//if (!player.at(0).isBurned()) {
				//	m_f2actives.emplace_back(new Burn());
				//	player.at(0).toggleBurn();
				//}
			}
			else if (top == DEVILS_WATER || bottom == DEVILS_WATER) {
				//if (!m_watersUsed) {
				//	//showDungeon();
				//	devilsWaterPrompt();

				//	/*if (devilsWater()) {
				//		m_maze2[y*MAXCOLS2 + x - 1].top = PLAYER;
				//		player.at(0).setPosX(x - 1);
				//		if (m_maze2[y*MAXCOLS2 + x].bottom == DEVILS_WATER)
				//			m_maze2[y*MAXCOLS2 + x].top = DEVILS_WATER;
				//		else
				//			m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//	}*/
				//}
				//else {
				//	m_maze2[y*MAXCOLS2 + x - 1].top = PLAYER;
				//	player.at(0).setPosX(x - 1);
				//	if (m_maze2[y*MAXCOLS2 + x].bottom == DEVILS_WATER)
				//		m_maze2[y*MAXCOLS2 + x].top = DEVILS_WATER;
				//	else
				//		m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//}
			}
			else if (enemy) {	// monster encountered
				fight(x - 1, y);
			}
		}

		else if (move == 'r') { // moved to the right
			top = m_maze2[y*MAXCOLS2 + x + 1].top;
			bottom = m_maze2[y*MAXCOLS2 + x + 1].bottom;
			traptile = m_maze2[y*MAXCOLS2 + x + 1].traptile;

			wall = m_maze2[y*MAXCOLS2 + x + 1].wall;
			item = m_maze2[y*MAXCOLS2 + x + 1].item;
			trap = m_maze2[y*MAXCOLS2 + x + 1].trap;
			enemy = m_maze2[y*MAXCOLS2 + x + 1].enemy;

			if (trap) {
				if (traptile == LAVA) {
					// lava sound
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						m_f2actives.emplace_back(new Burn());
						player.at(0).toggleBurn();
					}
				}
				if (traptile == BUTTON) {
					if (!m_openexit) {
						cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
						cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

						// replace button with pressed button
						removeSprite(trap_sprites, MAXROWS2, x - 1, y);
						addSprite(trap_sprites, MAXROWS2, x - 1, y, -1, "Button_Pressed_48x48.png");

						// replace locked stairs with open staircase
						removeSprite(trap_sprites, MAXROWS2, m_stairsX, m_stairsY);
						addSprite(trap_sprites, MAXROWS2, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");


						m_openexit = true;
						m_maze2[m_stairsY*MAXCOLS2 + m_stairsX].traptile = STAIRS;
						dungeonText.push_back("The stairs have been unlocked!\n");
					}
				}
				if (traptile == DEVILS_WATER) {
					if (!m_watersUsed) {
						devilsWaterPrompt();
					}
				}
			}
			if (!(wall || item || enemy)) {
				// move character to the left
				m_maze2[y*MAXCOLS2 + x + 1].top = PLAYER;
				m_maze2[y*MAXCOLS2 + x].top = SPACE;
				player.at(0).setPosX(x + 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of weapon, DON'T pickup yet
				m_maze2[y*MAXCOLS2 + x + 1].top = PLAYER;
				m_maze2[y*MAXCOLS2 + x].top = SPACE;
				player.at(0).setPosX(x + 1);

				foundItem(m_maze2, MAXCOLS2, x + 1, y);
			}
			else if (top == SPACE && bottom == BUTTON) {
				//m_maze2[y*MAXCOLS2 + x + 1].top = PLAYER;
				//m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//player.at(0).setPosX(x + 1);
				//if (!m_openexit) {
				//	//auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
				//	cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
				//	cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

				//	// replace button with pressed button
				//	removeSprite(item_sprites, MAXROWS2, x+1, y);
				//	addSprite(item_sprites, MAXROWS2, x+1, y, -1, "Button_Pressed_48x48.png");

				//	// replace locked stairs with open staircase
				//	removeSprite(wall_sprites, MAXROWS2, m_stairsX, m_stairsY);
				//	addSprite(item_sprites, MAXROWS2, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");

				//	m_openexit = true;
				//	m_maze2[m_stairsY*MAXCOLS2 + m_stairsX].bottom = STAIRS;
				//	dungeonText.push_back("The stairs have been unlocked!\n");
				//}
			}
			else if (top == LAVA || bottom == LAVA) {
				//m_maze2[y*MAXCOLS2 + x + 1].top = PLAYER;
				//if (m_maze2[y*MAXCOLS2 + x].bottom == LAVA)
				//	m_maze2[y*MAXCOLS2 + x].top = LAVA;
				//else
				//	m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//player.at(0).setPosX(x + 1);

				//// lava sound
				////auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
				//cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				//player.at(0).setHP(player.at(0).getHP() - 5);
				//dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				//if (!player.at(0).isBurned()) {
				//	m_f2actives.emplace_back(new Burn());
				//	player.at(0).toggleBurn();
				//}
			}
			else if (top == DEVILS_WATER || bottom == DEVILS_WATER) {
				//if (!m_watersUsed) {
				//	devilsWaterPrompt();

				//	/*showDungeon();
				//	if (devilsWater()) {
				//		m_maze2[y*MAXCOLS2 + x + 1].top = PLAYER;
				//		player.at(0).setPosX(x + 1);
				//		if (m_maze2[y*MAXCOLS2 + x].bottom == DEVILS_WATER)
				//			m_maze2[y*MAXCOLS2 + x].top = DEVILS_WATER;
				//		else
				//			m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//	}*/
				//}
				//else {
				//	m_maze2[y*MAXCOLS2 + x + 1].top = PLAYER;
				//	player.at(0).setPosX(x + 1);
				//	if (m_maze2[y*MAXCOLS2 + x].bottom == DEVILS_WATER)
				//		m_maze2[y*MAXCOLS2 + x].top = DEVILS_WATER;
				//	else
				//		m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//}
			}
			else if (enemy) {
				fight(x + 1, y);
			}
		}

		else if (move == 'u') { // moved up
			top = m_maze2[(y - 1)*MAXCOLS2 + x].top;
			bottom = m_maze2[(y - 1)*MAXCOLS2 + x].bottom;
			traptile = m_maze2[(y - 1)*MAXCOLS2 + x].traptile;

			wall = m_maze2[(y - 1)*MAXCOLS2 + x].wall;
			item = m_maze2[(y - 1)*MAXCOLS2 + x].item;
			trap = m_maze2[(y - 1)*MAXCOLS2 + x].trap;
			enemy = m_maze2[(y - 1)*MAXCOLS2 + x].enemy;

			if (trap) {
				if (traptile == LAVA) {
					// lava sound
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						m_f2actives.emplace_back(new Burn());
						player.at(0).toggleBurn();
					}
				}
				if (traptile == BUTTON) {
					if (!m_openexit) {
						cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
						cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

						// replace button with pressed button
						removeSprite(trap_sprites, MAXROWS2, x - 1, y);
						addSprite(trap_sprites, MAXROWS2, x - 1, y, -1, "Button_Pressed_48x48.png");

						// replace locked stairs with open staircase
						removeSprite(trap_sprites, MAXROWS2, m_stairsX, m_stairsY);
						addSprite(trap_sprites, MAXROWS2, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");


						m_openexit = true;
						m_maze2[m_stairsY*MAXCOLS2 + m_stairsX].traptile = STAIRS;
						dungeonText.push_back("The stairs have been unlocked!\n");
					}
				}
				if (traptile == DEVILS_WATER) {
					if (!m_watersUsed) {
						devilsWaterPrompt();
					}
				}
			}
			if (!(wall || item || enemy)) {
				m_maze2[(y - 1)*MAXCOLS2 + x].top = PLAYER;
				m_maze2[y*MAXCOLS2 + x].top = SPACE;
				player.at(0).setPosY(y - 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of item
				m_maze2[(y - 1)*MAXCOLS2 + x].top = PLAYER;
				m_maze2[y*MAXCOLS2 + x].top = SPACE;
				player.at(0).setPosY(y - 1);

				foundItem(m_maze2, MAXCOLS2, x, y - 1);
			}
			else if (top == SPACE && bottom == BUTTON) {
				//m_maze2[(y - 1)*MAXCOLS2 + x].top = PLAYER;
				//m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//player.at(0).setPosY(y - 1);
				//if (!m_openexit) {
				//	//auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
				//	cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
				//	cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

				//	// replace button with pressed button
				//	removeSprite(item_sprites, MAXROWS2, x, y - 1);
				//	addSprite(item_sprites, MAXROWS2, x, y - 1, -1, "Button_Pressed_48x48.png");

				//	// replace locked stairs with open staircase
				//	removeSprite(wall_sprites, MAXROWS2, m_stairsX, m_stairsY);
				//	addSprite(item_sprites, MAXROWS2, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");

				//	m_openexit = true;
				//	m_maze2[m_stairsY*MAXCOLS2 + m_stairsX].bottom = STAIRS;
				//	dungeonText.push_back("The stairs have been unlocked!\n");
				//}
			}
			else if (top == LAVA || bottom == LAVA) {
				//m_maze2[(y - 1)*MAXCOLS2 + x].top = PLAYER;
				//player.at(0).setPosY(y - 1);
				//if (m_maze2[y*MAXCOLS2 + x].bottom == LAVA)
				//	m_maze2[y*MAXCOLS2 + x].top = LAVA;
				//else
				//	m_maze2[y*MAXCOLS2 + x].top = SPACE;

				//// lava sound
				////auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
				//cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				//player.at(0).setHP(player.at(0).getHP() - 5);
				//dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				//if (!player.at(0).isBurned()) {
				//	m_f2actives.emplace_back(new Burn());
				//	player.at(0).toggleBurn();
				//}
			}
			else if (top == DEVILS_WATER || bottom == DEVILS_WATER) {
				//if (!m_watersUsed) {
				//	devilsWaterPrompt();

				//	/*showDungeon();
				//	if (devilsWater()) {
				//		m_maze2[(y - 1)*MAXCOLS2 + x].top = PLAYER;
				//		player.at(0).setPosY(y - 1);
				//		if (m_maze2[y*MAXCOLS2 + x].bottom == DEVILS_WATER)
				//			m_maze2[y*MAXCOLS2 + x].top = DEVILS_WATER;
				//		else
				//			m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//	}*/
				//}
				//else {
				//	m_maze2[(y - 1)*MAXCOLS2 + x].top = PLAYER;
				//	player.at(0).setPosY(y - 1);
				//	if (m_maze2[y*MAXCOLS2 + x].bottom == DEVILS_WATER)
				//		m_maze2[y*MAXCOLS2 + x].top = DEVILS_WATER;
				//	else
				//		m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//}
			}
			else if (enemy) {
				fight(x, y - 1);
			}
		}

		else if (move == 'd') { // moved down
			top = m_maze2[(y + 1)*MAXCOLS2 + x].top;
			bottom = m_maze2[(y + 1)*MAXCOLS2 + x].bottom;
			traptile = m_maze2[(y + 1)*MAXCOLS2 + x].traptile;

			wall = m_maze2[(y + 1)*MAXCOLS2 + x].wall;
			item = m_maze2[(y + 1)*MAXCOLS2 + x].item;
			trap = m_maze2[(y + 1)*MAXCOLS2 + x].trap;
			enemy = m_maze2[(y + 1)*MAXCOLS2 + x].enemy;

			if (trap) {
				if (traptile == LAVA) {
					// lava sound
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						m_f2actives.emplace_back(new Burn());
						player.at(0).toggleBurn();
					}
				}
				if (traptile == BUTTON) {
					if (!m_openexit) {
						cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
						cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

						// replace button with pressed button
						removeSprite(trap_sprites, MAXROWS2, x - 1, y);
						addSprite(trap_sprites, MAXROWS2, x - 1, y, -1, "Button_Pressed_48x48.png");

						// replace locked stairs with open staircase
						removeSprite(trap_sprites, MAXROWS2, m_stairsX, m_stairsY);
						addSprite(trap_sprites, MAXROWS2, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");


						m_openexit = true;
						m_maze2[m_stairsY*MAXCOLS2 + m_stairsX].traptile = STAIRS;
						dungeonText.push_back("The stairs have been unlocked!\n");
					}
				}
				if (traptile == DEVILS_WATER) {
					if (!m_watersUsed) {
						devilsWaterPrompt();
					}
				}
			}
			if (!(wall || item || enemy)) {
				m_maze2[(y + 1)*MAXCOLS2 + x].top = PLAYER;
				m_maze2[y*MAXCOLS2 + x].top = SPACE;
				player.at(0).setPosY(y + 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of item
				m_maze2[(y + 1)*MAXCOLS2 + x].top = PLAYER;
				m_maze2[y*MAXCOLS2 + x].top = SPACE;
				player.at(0).setPosY(y + 1);

				foundItem(m_maze2, MAXCOLS2, x, y + 1);
			}
			else if (top == SPACE && bottom == BUTTON) {
				//m_maze2[(y + 1)*MAXCOLS2 + x].top = PLAYER;
				//m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//player.at(0).setPosY(y + 1);
				//if (!m_openexit) {
				//	cocos2d::experimental::AudioEngine::play2d("Button_Pressed.mp3", false, 1.0f);
				//	cocos2d::experimental::AudioEngine::play2d("Exit_Opening.mp3", false, 1.0f);

				//	// replace button with pressed button
				//	removeSprite(item_sprites, MAXROWS2, x, y+1);
				//	addSprite(item_sprites, MAXROWS2, x, y+1, -1, "Button_Pressed_48x48.png");

				//	// replace locked stairs with open staircase
				//	removeSprite(wall_sprites, MAXROWS2, m_stairsX, m_stairsY);
				//	addSprite(item_sprites, MAXROWS2, m_stairsX, m_stairsY, -1, "Stairs_48x48.png");

				//	m_openexit = true;
				//	m_maze2[m_stairsY*MAXCOLS2 + m_stairsX].bottom = STAIRS;
				//	dungeonText.push_back("The stairs have been unlocked!\n");
				//}
			}
			else if (top == LAVA || bottom == LAVA) {
				//m_maze2[(y + 1)*MAXCOLS2 + x].top = PLAYER;
				//if (m_maze2[y*MAXCOLS2 + x].bottom == LAVA)
				//	m_maze2[y*MAXCOLS2 + x].top = LAVA;
				//else
				//	m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//player.at(0).setPosY(y + 1);

				//// lava sound
				//cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				//player.at(0).setHP(player.at(0).getHP() - 5);
				//dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				//if (!player.at(0).isBurned()) {
				//	m_f2actives.emplace_back(new Burn());
				//	player.at(0).toggleBurn();
				//}
			}
			else if (top == DEVILS_WATER || bottom == DEVILS_WATER) {
				//if (!m_watersUsed) {
				//	devilsWaterPrompt();

				//	/*showDungeon();
				//	if (devilsWater()) {
				//		m_maze2[(y + 1)*MAXCOLS2 + x].top = PLAYER;
				//		player.at(0).setPosY(y + 1);
				//		if (m_maze2[y*MAXCOLS2 + x].bottom == DEVILS_WATER)
				//			m_maze2[y*MAXCOLS2 + x].top = DEVILS_WATER;
				//		else
				//			m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//	}*/
				//}
				//else {
				//	m_maze2[(y + 1)*MAXCOLS2 + x].top = PLAYER;
				//	player.at(0).setPosY(y + 1);
				//	if (m_maze2[y*MAXCOLS2 + x].bottom == DEVILS_WATER)
				//		m_maze2[y*MAXCOLS2 + x].top = DEVILS_WATER;
				//	else
				//		m_maze2[y*MAXCOLS2 + x].top = SPACE;
				//}
			}
			else if (enemy) {
				fight(x, y + 1);
			}
		}

		else if (move == 'e') {
			collectItem(m_maze2, MAXROWS2, MAXCOLS2, x, y);

			if (getLevel() > 2)
				return;
		}

		else if (move == 'w') {
			//showDungeon();
			player.at(0).wield();
		}

		else if (move == 'c') {
			//showDungeon();
			//player.at(0).use(m_f2actives, m_maze2[y*MAXCOLS2 + x], dungeonText);
		}

		// check if player is idling over lava
		if (move == 'e' || move == 'w' || move == 'c' || move == '-') {
			traptile = m_maze2[y*MAXCOLS2 + x].traptile;
			if (traptile == LAVA) {
				cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				player.at(0).setHP(player.at(0).getHP() - 5);
				dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				if (!player.at(0).isBurned()) {
					m_f2actives.emplace_back(new Burn());
					player.at(0).toggleBurn();
				}
			}
		}
	}

	// check if player entered devil's waters and killed all guardians
	if (m_guardians == 0 && m_watersCleared) {
		guardiansDefeated();
		// prevents if statement from being entered again
		m_guardians = -1;
	}

	// check active items in play
	checkActive(m_maze2, MAXROWS2, MAXCOLS2, m_f2actives, m_f2monsters);

	//	if player is dead then return
	if (player.at(0).getHP() <= 0) {
		m_maze2[y*MAXCOLS2 + x].top = 'X'; // change player icon to X to indicate death

		clearScreen();
		showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}

	// if all monsters are dead?
	if (m_f2monsters.empty()) {
		clearScreen();
		showDungeon();
		if (!dungeonText.empty())
			showText();

		return;
	}

	// move monsters
	int mx, my, shortest;
	char first, best, prev;
	string monster;

	for (unsigned i = 0; i < m_f2monsters.size(); i++) {
		monster = m_f2monsters.at(i)->getName();
		mx = m_f2monsters.at(i)->getPosX();
		my = m_f2monsters.at(i)->getPosY();
		first = best = prev = '0';
		shortest = 0;

		if (monster == "goblin") {
			//	allows use of goblin's smell distance function
			shared_ptr<Goblin> goblin = dynamic_pointer_cast<Goblin>(m_f2monsters.at(i));

			if (playerInRange(mx, my, i, goblin->getSmellDistance())) {
				// if viable path is found
				if (checkGoblins(m_maze2, MAXCOLS2, i, shortest, goblin->getSmellDistance(), goblin->getSmellDistance(), mx, my, first, best, prev)) {
					moveGoblins(m_maze2, MAXROWS2, MAXCOLS2, mx, my, i, best, m_f2monsters);
				}
			}
		}

		else if (monster == "wanderer") {
			moveWanderers(m_maze2, MAXROWS2, MAXCOLS2, mx, my, i, m_f2monsters);
		}

		else if (monster == "archer") {
			checkArchers(m_maze2, MAXROWS2, MAXCOLS2, player.at(0).getPosX(), player.at(0).getPosY(), i, m_f2monsters);
		}

		else if (monster == "zapper") {
			moveZappers(m_maze2, MAXCOLS2, mx, my, i, m_f2monsters);
		}

		else if (monster == "spinner") {
			moveSpinners(m_maze2, MAXROWS2, MAXCOLS2, mx, my, i, m_f2monsters);
		}

		else if (monster == "bombee") {
			shared_ptr<Bombee> bombee = dynamic_pointer_cast<Bombee>(m_f2monsters.at(i));
			if (playerInRange(mx, my, i, bombee->getRange())) {
				// if viable path is found
				if (checkGoblins(m_maze2, MAXCOLS2, i, shortest, bombee->getRange(), bombee->getRange(), mx, my, first, best, prev)) {
					moveGoblins(m_maze2, MAXROWS2, MAXCOLS2, mx, my, i, best, m_f2monsters);
				}
			}
		}

		else if (monster == "roundabout") {
			moveRoundabouts(m_maze2, MAXROWS2, MAXCOLS2, mx, my, i, m_f2monsters);
		}

		else if (monster == "mounted knight") {
			checkMountedKnights(m_maze2, MAXROWS2, MAXCOLS2, player.at(0).getPosX(), player.at(0).getPosY(), i, m_f2monsters);
		}

		else if (monster == "seeker") {
			shared_ptr<Seeker> seeker = dynamic_pointer_cast<Seeker>(m_f2monsters.at(i));

			if (playerInRange(mx, my, i, seeker->getRange())) {
				// if viable path is found
				if (checkGoblins(m_maze2, MAXCOLS2, i, shortest, seeker->getRange(), seeker->getRange(), mx, my, first, best, prev)) {
					moveSeekers(m_maze2, MAXROWS2, MAXCOLS2, mx, my, i, best, m_f2monsters);
				}
			}
		}

		//	if player is dead then break
		if (player.at(0).getHP() <= 0) {
			m_maze2[y*MAXCOLS2 + x].top = 'X'; // change player icon to X to indicate death
			player.at(0).setDeath(m_f2monsters.at(i)->getName());
			break;
		}
	}


	//clearScreen();
	//showDungeon();
	if (!dungeonText.empty())
		showText();
}
/*
bool SecondFloor::devilsWater() {
	cout << "Will you bathe in the Devil's waters?\n";

	char c;
	while (c = getCharacter(), c != 'y' && c != 'n') {
		;
	}

	if (c == 'y') {
		devilsGift();
		return true;
	}
	else if (c == 'n') {
		cout << "You choose not to enter the waters.\n";
		c = getCharacter();
		return false;
	}
}
*/
void SecondFloor::devilsWater(bool bathed) {
	if (bathed) {
		m_waterPrompt = false;
		devilsGift();
	}
	m_waterPrompt = false;
}
/*
void SecondFloor::devilsGift() {
	m_watersUsed = true;
	player.at(0).setMaxHP(player.at(0).getMaxHP() + 5);
	m_f2actives.emplace_back(new HealOverTime());

	cout << "You feel life force flow through you, but there is a price to pay for your actions... \n";
	//waitForEnter();
	char c = getCharacter();

	Archer a1, a2, a3, a4;
	a1.setPosX(33); a1.setPosY(15); m_f2monsters.emplace_back(new Archer(a1)); m_maze2[a1.getPosY()*MAXCOLS2 + a1.getPosX()].top = ARCHER;
	a2.setPosX(33); a2.setPosY(22); m_f2monsters.emplace_back(new Archer(a2)); m_maze2[a2.getPosY()*MAXCOLS2 + a2.getPosX()].top = ARCHER;
	a3.setPosX(44); a3.setPosY(15); m_f2monsters.emplace_back(new Archer(a3)); m_maze2[a3.getPosY()*MAXCOLS2 + a3.getPosX()].top = ARCHER;
	a4.setPosX(44); a4.setPosY(22); m_f2monsters.emplace_back(new Archer(a4)); m_maze2[a4.getPosY()*MAXCOLS2 + a4.getPosX()].top = ARCHER;

	// close off the exits with walls after accepting the devils gift
	for (int i = 31; i <= 32; i++) {
		for (int j = 18; j <= 19; j++) {
			m_maze2[j*MAXCOLS2 + i].top = UNBREAKABLE_WALL;
		}
	}
	for (int i = 45; i <= 46; i++) {
		for (int j = 18; j <= 19; j++) {
			m_maze2[j*MAXCOLS2 + i].top = UNBREAKABLE_WALL;
		}
	}

	m_watersCleared = false;
	m_guardians = 4;
}
*/
void SecondFloor::devilsGift() {
	m_watersUsed = true;
	player.at(0).setMaxHP(player.at(0).getMaxHP() + 5);
	m_f2actives.emplace_back(new HealOverTime());

	//cout << "You feel life force flow through you, but there is a price to pay for your actions... \n";

	Archer a1, a2, a3, a4;
	a1.setPosX(33); a1.setPosY(15); m_f2monsters.emplace_back(new Archer(a1)); m_maze2[a1.getPosY()*MAXCOLS2 + a1.getPosX()].top = ARCHER;
	a2.setPosX(33); a2.setPosY(22); m_f2monsters.emplace_back(new Archer(a2)); m_maze2[a2.getPosY()*MAXCOLS2 + a2.getPosX()].top = ARCHER;
	a3.setPosX(44); a3.setPosY(15); m_f2monsters.emplace_back(new Archer(a3)); m_maze2[a3.getPosY()*MAXCOLS2 + a3.getPosX()].top = ARCHER;
	a4.setPosX(44); a4.setPosY(22); m_f2monsters.emplace_back(new Archer(a4)); m_maze2[a4.getPosY()*MAXCOLS2 + a4.getPosX()].top = ARCHER;

	addSprite(monster_sprites, MAXROWS2, 33, 15, 1, "Archer_48x48.png");
	addSprite(monster_sprites, MAXROWS2, 33, 22, 1, "Archer_48x48.png");
	addSprite(monster_sprites, MAXROWS2, 44, 15, 1, "Archer_48x48.png");
	addSprite(monster_sprites, MAXROWS2, 44, 22, 1, "Archer_48x48.png");

	// close off the exits with walls after accepting the devils gift
	for (int i = 31; i <= 32; i++) {
		for (int j = 18; j <= 19; j++) {
			addSprite(wall_sprites, MAXROWS2, i, j, 1, "D_Wall_Terrain2_48x48.png");

			m_maze2[j*MAXCOLS2 + i].top = UNBREAKABLE_WALL;
		}
	}
	for (int i = 45; i <= 46; i++) {
		for (int j = 18; j <= 19; j++) {
			addSprite(wall_sprites, MAXROWS2, i, j, 1, "D_Wall_Terrain2_48x48.png");

			m_maze2[j*MAXCOLS2 + i].top = UNBREAKABLE_WALL;
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
		}
	}
	for (int i = 45; i <= 46; i++) {
		for (int j = 18; j <= 19; j++) {
			removeSprite(wall_sprites, MAXROWS2, i, j);

			m_maze2[j*MAXCOLS2 + i].top = SPACE;
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

void SecondFloor::checkActive(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters) {
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
					// play explosion sound effect
					cocos2d::experimental::AudioEngine::play2d("Explosion.mp3", false, 1.0f);

					dungeonText.push_back("A bomb exploded!\n");
					explosion(dungeon, maxrows, maxcols, actives, monsters, bomb->getPosX(), bomb->getPosY(), i);
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
				shared_ptr<MegaBomb> megabomb = dynamic_pointer_cast<MegaBomb>(actives.at(i));
				if (megabomb->getFuse() > 0) {
					megabomb->setFuse(megabomb->getFuse() - 1);
					actives.at(i) = megabomb;
				}
				else {
					// play explosion sound effect
					cocos2d::experimental::AudioEngine::play2d("Mega_Explosion.mp3", false, 1.0f);

					dungeonText.push_back("BOOM!\n");
					explosion(dungeon, maxrows, maxcols, actives, monsters, megabomb->getPosX(), megabomb->getPosY(), i);
					dungeon[megabomb->getPosY()*maxcols + megabomb->getPosX()].extra = SPACE;

					// remove bomb sprite
					removeSprite(projectile_sprites, maxrows, megabomb->getPosX(), megabomb->getPosY());

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
				x = spiketrap->getPosX();
				y = spiketrap->getPosY();

				if (dungeon[y*maxcols + x].traptile == SPIKETRAP_ACTIVE) {
					spiketrap->setCountdown(spiketrap->getSpeed());
					actives.at(i) = spiketrap;
					dungeon[y*maxcols + x].traptile = SPIKETRAP_DEACTIVE;
				}
				if (spiketrap->getCountdown() == 1) {
					spiketrap->setCountdown(spiketrap->getCountdown() - 1);
					actives.at(i) = spiketrap;
					dungeon[y*maxcols + x].traptile = SPIKETRAP_PRIMED;
				}
				else if (spiketrap->getCountdown() > 1) {
					spiketrap->setCountdown(spiketrap->getCountdown() - 1);
					actives.at(i) = spiketrap;
				}
				else {
					dungeon[y*maxcols + x].traptile = SPIKETRAP_ACTIVE;

					if (px == x && py == y) {
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
			else if (actives.at(i)->getItem() == "burn") {
				shared_ptr<Burn> burn = dynamic_pointer_cast<Burn>(actives.at(i));
				if (burn->burnsLeft() == burn->maxBurns())
					burn->setBurn(burn->burnsLeft() - 1);
				else if (burn->burnsLeft() > 0)
					burn->burn(player.at(0), dungeonText);
				else {
					dungeonText.push_back("The burning subsides.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}

				if (player.at(0).getHP() <= 0) {
					player.at(0).setDeath("burn");
					dungeon[py*maxcols + px].top = 'X';
				}
				burn.reset();
			}
			else if (actives.at(i)->getItem() == "heal") {
				shared_ptr<HealOverTime> heal = dynamic_pointer_cast<HealOverTime>(actives.at(i));
				if (heal->healsLeft() > 0)
					heal->heal(player.at(0), dungeonText);
				else {
					dungeonText.push_back("The heal fades.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}
				heal.reset();
			}
		}
	}
}
void SecondFloor::explosion(std::vector<_Tile> &dungeon, int maxrows, int maxcols, std::vector<std::shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters, int x, int y, int pos) { // bomb's coordinates
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();
	int mx, my;

	for (unsigned i = 0; i < monsters.size(); i++) {
		mx = monsters.at(i)->getPosX();
		my = monsters.at(i)->getPosY();

		if (actives.at(pos)->getItem() == "Mega Bomb") {
			// play explosion sound effect
			cocos2d::experimental::AudioEngine::play2d("Mega_Explosion.mp3", false, 1.0);

			// find any monsters caught in the explosion
			if (abs(mx - x) <= 2 && abs(my - y) <= 2) {
				monsters.at(i)->setHP(monsters.at(i)->getHP() - 10);
				dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

				if (monsters.at(i)->getHP() <= 0) {
					monsterDeath(dungeon, maxrows, maxcols, actives, monsters, i);// mx, my, i);
				}
			}
		}
		// else was a regular bomb
		else if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			monsters.at(i)->setHP(monsters.at(i)->getHP() - 10);
			dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

			string monster = monsters.at(i)->getName();
			if (monsters.at(i)->getHP() <= 0) {
				if (monster == "archer") {
					m_guardians--;
					if (m_guardians == 0)
						m_watersCleared = true;
				}
				monsterDeath(dungeon, maxrows, maxcols, actives, monsters, i);// mx, my, i);
			}
		}
	}

	if (actives.at(pos)->getItem() == "Mega Bomb") {
		//	if player is caught in the explosion
		if (abs(px - x) <= 2 && abs(py - y) <= 2) {
			player.at(0).setHP(player.at(0).getHP() - 10);
			dungeonText.push_back("That's gotta hurt!\n");

			if (player.at(0).getHP() <= 0)
				player.at(0).setDeath(actives.at(pos)->getItem());
		}
		//	destroy any nearby walls
		for (int j = y - 2; j < y + 3; j++) {
			for (int k = x - 2; k < x + 3; k++) {
				if (dungeon[j*maxcols + k].top == WALL) {
					dungeon[j*maxcols + k].top = SPACE;
					dungeon[j*maxcols + k].bottom = SPACE;
					dungeon[j*maxcols + k].wall = false;

					removeSprite(wall_sprites, maxrows, k, j);
					removeSprite(wall_sprites, maxrows, k, j);
				}
			}
		}
	}
	else {
		//	if player is caught in the explosion
		if (abs(px - x) <= 1 && abs(py - y) <= 1) {
			player.at(0).setHP(player.at(0).getHP() - 10);
			dungeonText.push_back("That's gotta hurt!\n");
			
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

					// call twice to remove top and bottom walls
					removeSprite(wall_sprites, maxrows, k, j);
					removeSprite(wall_sprites, maxrows, k, j);
				}
			}
		}
	}
}
void SecondFloor::findMonster(int x, int y) {
	unsigned i = 0;
	bool found = false;
	while (!found && i < m_f2monsters.size()) { // finds the monster just encountered
		if (m_f2monsters.at(i)->getPosX() == x && m_f2monsters.at(i)->getPosY() == y)
			found = true;
		else
			i++;
	}
	string monster = m_f2monsters.at(i)->getName();
}
void SecondFloor::fight(int x, int y) { // monster's coordinates
	unsigned i = 0;
	bool found = false;
	while (!found && i < m_f2monsters.size()) { // finds the monster just encountered
		if (m_f2monsters.at(i)->getPosX() == x && m_f2monsters.at(i)->getPosY() == y)
			found = true;
		else
			i++;
	}
	std::string monster = m_f2monsters.at(i)->getName();

	player.at(0).attack(m_f2monsters, m_f2actives, i, dungeonText);
	if (m_f2monsters.at(i)->getHP() <= 0) {
		if (monster == "archer") {
			m_guardians--;
			if (m_guardians == 0)
				m_watersCleared = true;
		}
		monsterDeath(m_maze2, MAXROWS2, MAXCOLS2, m_f2actives, m_f2monsters, i);// x, y, i);
	}
}

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

std::vector<_Tile> SecondFloor::getDungeon() const {
	return m_maze2;
}
int SecondFloor::getRows() const {
	return m_rows;
}
int SecondFloor::getCols() const {
	return m_cols;
}
std::vector<std::shared_ptr<Monster>> SecondFloor::getMonsters() const {
	return m_f2monsters;
}
std::vector<std::shared_ptr<Objects>> SecondFloor::getActives() const {
	return m_f2actives;
}
void SecondFloor::callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index) {
	string item = player.at(0).getItemInv().at(index).getItem(); // get item name
	std::string image = player.at(0).use(m_f2actives, dungeon[y*maxcols + x], index); // gets sprite, if any
	if (item == "Bomb")
		addSprite(projectile_sprites, maxrows, x, y, -1, image);
}


//	THIRD FLOOR FUNCTIONS
ThirdFloor::ThirdFloor(Player p) : Dungeon(3), m_locked(false) {
	_Tile *tile;
	int i, j;

	//	initialize entire dungeon to blank space
	for (i = 0; i < MAXROWS3; i++) {
		for (j = 0; j < MAXCOLS3; j++) {
			tile = new _Tile;
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = SPACE;
			tile->wall = tile->item = tile->trap = tile->enemy = false;

			tile->marked = false;

			m_maze3.push_back(*tile);
			delete tile;
		}
	}

	//	initialize top and bottom of dungeon to be unbreakable walls
	for (i = 0; i < MAXROWS3; i++) {
		for (j = 0; j < MAXCOLS3; j++) {
			m_maze3[i*MAXCOLS3 + j].top = UNBREAKABLE_WALL;
			m_maze3[i*MAXCOLS3 + j].bottom = UNBREAKABLE_WALL;
			m_maze3[i*MAXCOLS3 + j].traptile = UNBREAKABLE_WALL;
			m_maze3[i*MAXCOLS3 + j].wall = true;
		}
		i += (MAXROWS3 - 2);
	}

	//	initialize edges of dungeon to be walls
	for (i = 0; i < MAXROWS3 - 1; i++) {
		m_maze3[i*MAXCOLS3].top = m_maze3[i*MAXCOLS3].bottom = m_maze3[i*MAXCOLS3 + j].traptile = UNBREAKABLE_WALL;
		m_maze3[i*MAXCOLS3 + MAXCOLS3 - 1].top = m_maze3[i*MAXCOLS3 + MAXCOLS3 - 1].bottom = m_maze3[i*MAXCOLS3 + MAXCOLS3 - 1].traptile = UNBREAKABLE_WALL;
	
		m_maze3[i*MAXCOLS3].wall = true;
		m_maze3[i*MAXCOLS3 + MAXCOLS3 - 1].wall = true;
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
	while (count < (MAXROWS3 - 2) / 9) {
		if (count == 0) {
			TLChunks(c, pool, MAXCOLS3); pool.clear(); m_layer++;
			topChunks(c, pool, MAXCOLS3); pool.clear();
			v = TRChunks(c, pool, MAXCOLS3); pool.clear();
		}
		else if (count == 1 || count == 2) {
			leftEdgeChunks(c, pool, MAXCOLS3); pool.clear();
			middleChunks(c, pool, MAXCOLS3); pool.clear();
			v = rightEdgeChunks(c, pool, MAXCOLS3); pool.clear();
		}
		else if (count == 3) {
			m_layer++;
			BLChunks(c, pool, MAXCOLS3); pool.clear(); m_layer++;
			bottomChunks(c, pool, MAXCOLS3); pool.clear();
			v = BRChunks(c, pool, MAXCOLS3); pool.clear();
		}

		for (i = 0; i < v.size(); i++)
			finalvec.push_back(v.at(i) == '#' ? WALL : v.at(i));

		c.clear();
		count++;
	}

	count = 0;
	char object;
	for (i = 0; i < MAXROWS3 - 1; i++) { // changed to i = 0 from i =1
		//Originally j < MAXCOLS3 - 1
		for (j = 1; j < MAXCOLS3; j++) {
			m_maze3[i*MAXCOLS3 + j].top = finalvec.at(count);
			m_maze3[i*MAXCOLS3 + j].bottom = finalvec.at(count);
			if (m_maze3[i*MAXCOLS3 + j].top == WALL || m_maze3[i*MAXCOLS3 + j].top == UNBREAKABLE_WALL) {
				m_maze3[i*MAXCOLS3 + j].traptile = finalvec.at(count);
				m_maze3[i*MAXCOLS3 + j].wall = true;
			}
			if (finalvec.at(count) == LAVA) {
				m_maze3[i*MAXCOLS3 + j].top = SPACE;
				m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
				m_maze3[i*MAXCOLS3 + j].traptile = finalvec.at(count);
			}

			//	if the object was an item or enemy
			if (m_maze3[i*MAXCOLS3 + j].bottom != WALL && m_maze3[i*MAXCOLS3 + j].bottom != SPACE) {

				object = m_maze3[i*MAXCOLS3 + j].bottom;
				switch (object) {
				case PLAYER:
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					player.at(0) = p;
					player.at(0).setPosX(j);
					player.at(0).setPosY(i);
					break;
				case DOOR_V:
				case DOOR_H: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					m_maze3[i*MAXCOLS3 + j].traptile = finalvec.at(count);
					//m_maze3[i*MAXCOLS3 + j].trap = true;
					Door door;
					door.setPosX(j);
					door.setPosY(i);
					m_f3actives.emplace_back(new Door(door));
					break;
				}
				case STAIRS:
				case BUTTON:
				case BOMB:
				case ARMOR:
				case LIFEPOT:
				case STATPOT:
				case HEART_POD:
				case BROWN_CHEST:
				case SILVER_CHEST:
				case GOLDEN_CHEST:
				case INFINITY_BOX:
					m_maze3[i*MAXCOLS3 + j].top = SPACE;
					break;
				/*case SPIKE:
					m_maze3[i*MAXCOLS3 + j].top = m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					FallingSpike spike(;
					m_maze3[i*MAXCOLS3 + j].traptile = SPIKE;
					m_maze3[i*MAXCOLS3 + j].trap = true;
					break;*/
				case SPIKETRAP_DEACTIVE: {
					m_maze3[i*MAXCOLS3 + j].top = m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					SpikeTrap st(j, i, 4);
					m_f3actives.emplace_back(new SpikeTrap(st));
					m_maze3[i*MAXCOLS3 + j].traptile = SPIKETRAP_DEACTIVE;
					m_maze3[i*MAXCOLS3 + j].trap = true;
					break;
				}
				case TRIGGER_SPIKE_DEACTIVE: {
					m_maze3[i*MAXCOLS3 + j].top = m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					TriggerSpike ts(j, i);
					m_f3actives.emplace_back(new TriggerSpike(ts));
					m_maze3[i*MAXCOLS3 + j].traptile = TRIGGER_SPIKE_DEACTIVE;
					m_maze3[i*MAXCOLS3 + j].trap = true;
					break;
				}
				case WANDERER: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					Wanderer wand;
					wand.setPosX(j);
					wand.setPosY(i);
					m_f3monsters.emplace_back(new Wanderer(wand));
					m_maze3[i*MAXCOLS3 + j].enemy = true;
					break;
				}
				case GOBLIN: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					Goblin goblin(12);
					goblin.setPosX(j);
					goblin.setPosY(i);
					m_f3monsters.emplace_back(new Goblin(goblin));
					m_maze3[i*MAXCOLS3 + j].enemy = true;
					break;
				}
				case ARCHER: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					Archer archer;
					archer.setPosX(j);
					archer.setPosY(i);
					m_f3monsters.emplace_back(new Archer(archer));
					m_maze3[i*MAXCOLS3 + j].enemy = true;
					break;
				}
				case SEEKER: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					Seeker seeker(10);
					seeker.setPosX(j);
					seeker.setPosY(i);
					m_f3monsters.emplace_back(new Seeker(seeker));
					m_maze3[i*MAXCOLS3 + j].enemy = true;
					break;
				}
				case MOUNTED_KNIGHT: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					MountedKnight knight;
					knight.setPosX(j);
					knight.setPosY(i);
					m_f3monsters.emplace_back(new MountedKnight(knight));
					m_maze3[i*MAXCOLS3 + j].enemy = true;
					break;
				}
				case ZAPPER: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					Zapper zapper;
					zapper.setPosX(j);
					zapper.setPosY(i);
					m_f3monsters.emplace_back(new Zapper(zapper));
					m_maze3[i*MAXCOLS3 + j].enemy = true;
					break;
				}
				case SPINNER: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					Spinner spinner;
					spinner.setPosX(j);
					spinner.setPosY(i);
					m_f3monsters.emplace_back(new Spinner(spinner));
					m_maze3[i*MAXCOLS3 + j].enemy = true;
					break;
				}
				case ROUNDABOUT: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					Roundabout ra;
					ra.setPosX(j);
					ra.setPosY(i);
					m_f3monsters.emplace_back(new Roundabout(ra));
					m_maze3[i*MAXCOLS3 + j].enemy = true;
					break;
				}
				case BOMBEE: {
					m_maze3[i*MAXCOLS3 + j].bottom = SPACE;
					Bombee bombee;
					bombee.setPosX(j);
					bombee.setPosY(i);
					m_f3monsters.emplace_back(new Bombee(bombee));
					m_maze3[i*MAXCOLS3 + j].enemy = true;
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

	/*
	////	spawn player
	//player.at(0) = p;
	//char toptile = m_maze3[player.at(0).getPosY()*MAXCOLS3 + player.at(0).getPosX()].top;
	//char bottomtile = m_maze3[player.at(0).getPosY()*MAXCOLS3 + player.at(0).getPosX()].bottom;

	//while (toptile != SPACE || bottomtile != SPACE || player.at(0).getPosX() > 4 && player.at(0).getPosX() < 65) {
	//	player.at(0).setrandPosX(MAXCOLS3);
	//	player.at(0).setrandPosY(MAXROWS3);

	//	// while player is not spawned along left side or right side, reroll
	//	while (player.at(0).getPosX() > 4 && player.at(0).getPosX() < MAXCOLS3 - 5) {
	//		player.at(0).setrandPosX(MAXCOLS3);
	//	}

	//	toptile = m_maze3[player.at(0).getPosY()*MAXCOLS3 + player.at(0).getPosX()].top;
	//	bottomtile = m_maze3[player.at(0).getPosY()*MAXCOLS3 + player.at(0).getPosX()].bottom;
	//}
	//m_maze3[player.at(0).getPosY()*MAXCOLS3 + player.at(0).getPosX()].top = PLAYER; // sets tile at this spot to be player position


	//Stairs stairs;
	//toptile = m_maze3[stairs.getPosY()*MAXCOLS3 + stairs.getPosX()].top;
	//bottomtile = m_maze3[stairs.getPosY()*MAXCOLS3 + stairs.getPosX()].bottom;
	////	while stairs does not spawn on the side opposite from the player
	//while (toptile != SPACE || bottomtile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() < MAXCOLS3 - 5) || (player.at(0).getPosX() > MAXCOLS3 - 5 && stairs.getPosX() > 4))) {
	//	//while (toptile != SPACE || bottomtile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() >= 5) || (player.at(0).getPosX() > MAXCOLS3-6 && stairs.getPosX() < MAXCOLS3 - 6))) {
	//	stairs.setrandPosX(MAXCOLS3);
	//	stairs.setrandPosY(MAXROWS3);

	//	toptile = m_maze3[stairs.getPosY()*MAXCOLS3 + stairs.getPosX()].top;
	//	bottomtile = m_maze3[stairs.getPosY()*MAXCOLS3 + stairs.getPosX()].bottom;
	//}
	//m_maze3[stairs.getPosY()*MAXCOLS3 + stairs.getPosX()].bottom = STAIRS;
	*/
}
ThirdFloor& ThirdFloor::operator=(ThirdFloor const &dungeon) {
	for (unsigned i = 0; i < dungeon.getRows()*dungeon.getCols(); i++) {
		m_maze3[i] = dungeon.m_maze3[i];
	}
	player = dungeon.player;

	m_f3monsters = dungeon.m_f3monsters;
	m_f3actives = dungeon.m_f3actives;

	m_locked = dungeon.m_locked;

	m_playerplaced = dungeon.m_playerplaced;
	m_exitplaced = dungeon.m_exitplaced;

	m_level = dungeon.getLevel();

	m_layer = dungeon.m_layer;
	m_playerchunk = dungeon.m_playerchunk;
	m_exitchunk = dungeon.m_exitchunk;

	return *this;
}

void ThirdFloor::peekThirdFloor(int x, int y, char move) {
	char top, bottom, traptile;
	bool wall, item, enemy, trap;

	// move player
	int weprange = player.at(0).getWeapon().getRange();
	bool move_used = false;
	bool far_enemy, close_enemy;
	// if weapon has a range of 2, check for monsters to attack
	if (weprange == 2) {
		if (move == 'l') { // moved to the left
			top = m_maze3[y*MAXCOLS3 + x - 1].top;
			close_enemy = m_maze3[y*MAXCOLS3 + x - 1].enemy;
			far_enemy = m_maze3[y*MAXCOLS3 + x - 2].enemy;

			if (close_enemy) {
				fight(x - 1, y);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && y*MAXCOLS3 + x - 2 > 0) {	// monster encountered
				fight(x - 2, y);
				move_used = true;
			}
		}
		else if (move == 'r') {
			top = m_maze3[y*MAXCOLS3 + x + 1].top;
			close_enemy = m_maze3[y*MAXCOLS3 + x + 1].enemy;
			far_enemy = m_maze3[y*MAXCOLS3 + x + 2].enemy;

			if (close_enemy) {
				fight(x + 1, y);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && y*MAXCOLS3 + x + 2 < MAXROWS3 * MAXCOLS3) {
				fight(x + 2, y);
				move_used = true;
			}
		}
		else if (move == 'u') { // moved up
			top = m_maze3[(y - 1)*MAXCOLS3 + x].top;
			close_enemy = m_maze3[(y - 1)*MAXCOLS3 + x].enemy;
			// if player is along top edge, assign far_enemy to close_enemy instead
			far_enemy = (y == 1 ? close_enemy : m_maze3[(y - 2)*MAXCOLS3 + x].enemy);
			//far_enemy = m_maze3[(y - 2)*MAXCOLS3 + x].enemy;

			if (close_enemy) {
				fight(x, y - 1);
				move_used = true;
			}
			// top != wall, since lava counts as free space
			else if (far_enemy && top == SPACE && (y - 2)*MAXCOLS3 + x > 0) {
				fight(x, y - 2);
				move_used = true;
			}
		}
		else if (move == 'd') { // moved down
			top = m_maze3[(y + 1)*MAXCOLS3 + x].top;
			close_enemy = m_maze3[(y + 1)*MAXCOLS3 + x].enemy;
			// if player is along top edge, assign far_enemy to close_enemy instead
			far_enemy = (y == MAXROWS3 - 2 ? close_enemy : m_maze3[(y + 2)*MAXCOLS3 + x].enemy);

			if (close_enemy) {
				fight(x, y + 1);
				move_used = true;
			}
			else if (far_enemy && top == SPACE && (y + 2)*MAXCOLS3 + x < MAXROWS3 * MAXCOLS3) {
				fight(x, y + 2);
				move_used = true;
			}
		}
	}

	if (!move_used) {
		if (move == 'l') { // moved to the left
			top = m_maze3[y*MAXCOLS3 + x - 1].top;
			bottom = m_maze3[y*MAXCOLS3 + x - 1].bottom;
			traptile = m_maze3[y*MAXCOLS3 + x - 1].traptile;

			wall = m_maze3[y*MAXCOLS3 + x - 1].wall;
			item = m_maze3[y*MAXCOLS3 + x - 1].item;
			trap = m_maze3[y*MAXCOLS3 + x - 1].trap;
			enemy = m_maze3[y*MAXCOLS3 + x - 1].enemy;

			if (trap) {
				if (traptile == LAVA) {
					// lava sound
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						m_f3actives.emplace_back(new Burn());
						player.at(0).toggleBurn();
					}
				}
				if (traptile == SPIKE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x - 1, y);

					if (player.at(0).getHP() <= 0) {
						player.at(0).setDeath("falling spike");
						clearScreen();
						showDungeon();
						if (!dungeonText.empty())
							showText();
						return;
					}
				}
				if (traptile == SPIKETRAP_ACTIVE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x - 1, y);
				}
				if (traptile == TRIGGER_SPIKE_ACTIVE || traptile == TRIGGER_SPIKE_DEACTIVE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x - 1, y);
				}
			}
			if (!(wall || item || enemy)) {
				// move character to the left
				m_maze3[y*MAXCOLS3 + x - 1].top = PLAYER;
				m_maze3[y*MAXCOLS3 + x].top = SPACE;
				player.at(0).setPosX(x - 1);

				//if (trap) {
				//	if (traptile == SPIKE) {
				//		/*trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x - 1, y);
				//		m_maze3[y*MAXCOLS3 + x - 1].top = PLAYER;
				//		m_maze3[y*MAXCOLS3 + x - 1].traptile = SPACE;
				//		m_maze3[y*MAXCOLS3 + x - 1].trap = false;
				//		m_maze3[y*MAXCOLS3 + x].top = SPACE;
				//		player.at(0).setPosX(x - 1);*/

				//		if (player.at(0).getHP() <= 0) {
				//			player.at(0).setDeath("falling spike");
				//			clearScreen();
				//			showDungeon();
				//			if (!dungeonText.empty())
				//				showText();
				//			return;
				//		}
				//	}
				//	else if (traptile == SPIKETRAP_ACTIVE) {
				//		trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x - 1, y);
				//	}
				//	else if (traptile == TRIGGER_SPIKE_ACTIVE || traptile == TRIGGER_SPIKE_DEACTIVE) {
				//		trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x - 1, y);
				//	}
				//}
			}
			else if (!wall && !enemy && item) {
				// move character on top of weapon, DON'T pickup yet
				m_maze3[y*MAXCOLS3 + x - 1].top = PLAYER;
				m_maze3[y*MAXCOLS3 + x].top = SPACE;
				player.at(0).setPosX(x - 1);

				foundItem(m_maze3, MAXCOLS3, x - 1, y);
			}
			else if (top == LAVA || bottom == LAVA) {
				//m_maze3[y*MAXCOLS3 + x - 1].top = PLAYER;
				//player.at(0).setPosX(x - 1);
				//if (m_maze3[y*MAXCOLS3 + x].bottom == LAVA)
				//	m_maze3[y*MAXCOLS3 + x].top = LAVA;
				//else
				//	m_maze3[y*MAXCOLS3 + x].top = SPACE;

				//// lava sound
				//cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				//player.at(0).setHP(player.at(0).getHP() - 5);
				//dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				//if (!player.at(0).isBurned()) {
				//	m_f3actives.emplace_back(new Burn());
				//	player.at(0).toggleBurn();
				//}
			}
			else if (top == DOOR_H || top == DOOR_V || top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
				if (top == DOOR_H || top == DOOR_V) {
					openDoor(x - 1, y);
				}
				else {
					m_maze3[y*MAXCOLS3 + x - 1].top = PLAYER;
					player.at(0).setPosX(x - 1);
					m_maze3[y*MAXCOLS3 + x].top = SPACE;
				}
			}
			else if (enemy) {	// monster encountered
				fight(x - 1, y);
			}
		}

		else if (move == 'r') { // moved to the right
			top = m_maze3[y*MAXCOLS3 + x + 1].top;
			bottom = m_maze3[y*MAXCOLS3 + x + 1].bottom;
			traptile = m_maze3[y*MAXCOLS3 + x + 1].traptile;

			wall = m_maze3[y*MAXCOLS3 + x + 1].wall;
			item = m_maze3[y*MAXCOLS3 + x + 1].item;
			trap = m_maze3[y*MAXCOLS3 + x + 1].trap;
			enemy = m_maze3[y*MAXCOLS3 + x + 1].enemy;

			if (trap) {
				if (traptile == LAVA) {
					// lava sound
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						m_f3actives.emplace_back(new Burn());
						player.at(0).toggleBurn();
					}
				}
				if (traptile == SPIKE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x + 1, y);

					if (player.at(0).getHP() <= 0) {
						player.at(0).setDeath("falling spike");
						clearScreen();
						showDungeon();
						if (!dungeonText.empty())
							showText();
						return;
					}
				}
				if (traptile == SPIKETRAP_ACTIVE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x + 1, y);
				}
				if (traptile == TRIGGER_SPIKE_ACTIVE || traptile == TRIGGER_SPIKE_DEACTIVE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x + 1, y);
				}
			}
			if (!(wall || item || enemy)) {
				// move character to the left
				m_maze3[y*MAXCOLS3 + x + 1].top = PLAYER;
				m_maze3[y*MAXCOLS3 + x].top = SPACE;
				player.at(0).setPosX(x + 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of item
				m_maze3[y*MAXCOLS3 + x + 1].top = PLAYER;
				m_maze3[y*MAXCOLS3 + x].top = SPACE;
				player.at(0).setPosX(x + 1);

				foundItem(m_maze3, MAXCOLS3, x + 1, y);
			}
			else if (top == LAVA || bottom == LAVA) {
				//m_maze3[y*MAXCOLS3 + x + 1].top = PLAYER;
				//player.at(0).setPosX(x + 1);
				//if (m_maze3[y*MAXCOLS3 + x].bottom == LAVA)
				//	m_maze3[y*MAXCOLS3 + x].top = LAVA;
				//else
				//	m_maze3[y*MAXCOLS3 + x].top = SPACE;

				//// lava sound
				////auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
				//cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				//player.at(0).setHP(player.at(0).getHP() - 5);
				//dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				//if (!player.at(0).isBurned()) {
				//	m_f3actives.emplace_back(new Burn());
				//	player.at(0).toggleBurn();
				//}
			}
			else if (top == DOOR_H || top == DOOR_V || top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
				if (top == DOOR_H || top == DOOR_V) {
					openDoor(x + 1, y);
				}
				else {
					m_maze3[y*MAXCOLS3 + x + 1].top = PLAYER;
					player.at(0).setPosX(x + 1);
					m_maze3[y*MAXCOLS3 + x].top = SPACE;
				}
			}
			else if (enemy) {
				fight(x + 1, y);
			}
		}

		else if (move == 'u') { // moved up
			top = m_maze3[(y - 1)*MAXCOLS3 + x].top;
			bottom = m_maze3[(y - 1)*MAXCOLS3 + x].bottom;
			traptile = m_maze3[(y - 1)*MAXCOLS3 + x].traptile;

			wall = m_maze3[(y - 1)*MAXCOLS3 + x].wall;
			item = m_maze3[(y - 1)*MAXCOLS3 + x].item;
			trap = m_maze3[(y - 1)*MAXCOLS3 + x].trap;
			enemy = m_maze3[(y - 1)*MAXCOLS3 + x].enemy;

			if (trap) {
				if (traptile == LAVA) {
					// lava sound
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						m_f3actives.emplace_back(new Burn());
						player.at(0).toggleBurn();
					}
				}
				if (traptile == SPIKE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x, y - 1);

					if (player.at(0).getHP() <= 0) {
						player.at(0).setDeath("falling spike");
						clearScreen();
						showDungeon();
						if (!dungeonText.empty())
							showText();
						return;
					}
				}
				if (traptile == SPIKETRAP_ACTIVE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x, y - 1);
				}
				if (traptile == TRIGGER_SPIKE_ACTIVE || traptile == TRIGGER_SPIKE_DEACTIVE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x, y - 1);
				}
			}
			if (!(wall || item || enemy)) {
				m_maze3[(y - 1)*MAXCOLS3 + x].top = PLAYER;
				m_maze3[y*MAXCOLS3 + x].top = SPACE;
				player.at(0).setPosY(y - 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of item
				m_maze3[(y - 1)*MAXCOLS3 + x].top = PLAYER;
				m_maze3[y*MAXCOLS3 + x].top = SPACE;
				player.at(0).setPosY(y - 1);

				foundItem(m_maze3, MAXCOLS3, x, y - 1);
			}
			else if (top == LAVA || bottom == LAVA) {
				//m_maze3[(y - 1)*MAXCOLS3 + x].top = PLAYER;
				//player.at(0).setPosY(y - 1);
				//if (m_maze3[y*MAXCOLS3 + x].bottom == LAVA)
				//	m_maze3[y*MAXCOLS3 + x].top = LAVA;
				//else
				//	m_maze3[y*MAXCOLS3 + x].top = SPACE;

				//// lava sound
				////auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
				//cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				//player.at(0).setHP(player.at(0).getHP() - 5);
				//dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				//if (!player.at(0).isBurned()) {
				//	m_f3actives.emplace_back(new Burn());
				//	player.at(0).toggleBurn();
				//}
			}
			else if (top == DOOR_H || top == DOOR_V || top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
				if (top == DOOR_H || top == DOOR_V) {
					openDoor(x, y - 1);
				}
				else {
					m_maze3[(y - 1)*MAXCOLS3 + x].top = PLAYER;
					m_maze3[y*MAXCOLS3 + x].top = SPACE;
					player.at(0).setPosY(y - 1);
				}
			}
			else if (enemy) {
				fight(x, y - 1);
			}
		}

		else if (move == 'd') { // moved down
			top = m_maze3[(y + 1)*MAXCOLS3 + x].top;
			bottom = m_maze3[(y + 1)*MAXCOLS3 + x].bottom;
			traptile = m_maze3[(y + 1)*MAXCOLS3 + x].traptile;

			wall = m_maze3[(y + 1)*MAXCOLS3 + x].wall;
			item = m_maze3[(y + 1)*MAXCOLS3 + x].item;
			trap = m_maze3[(y + 1)*MAXCOLS3 + x].trap;
			enemy = m_maze3[(y + 1)*MAXCOLS3 + x].enemy;

			if (trap) {
				if (traptile == LAVA) {
					// lava sound
					cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

					player.at(0).setHP(player.at(0).getHP() - 5);
					dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

					if (!player.at(0).isBurned()) {
						m_f3actives.emplace_back(new Burn());
						player.at(0).toggleBurn();
					}
				}
				if (traptile == SPIKE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x, y + 1);

					if (player.at(0).getHP() <= 0) {
						player.at(0).setDeath("falling spike");
						clearScreen();
						showDungeon();
						if (!dungeonText.empty())
							showText();
						return;
					}
				}
				if (traptile == SPIKETRAP_ACTIVE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x, y + 1);
				}
				if (traptile == TRIGGER_SPIKE_ACTIVE || traptile == TRIGGER_SPIKE_DEACTIVE) {
					trapEncounter(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, x, y + 1);
				}
			}
			if (!(wall || item || enemy)) {
				m_maze3[(y + 1)*MAXCOLS3 + x].top = PLAYER;
				m_maze3[y*MAXCOLS3 + x].top = SPACE;
				player.at(0).setPosY(y + 1);
			}
			else if (!wall && !enemy && item) {
				// move character on top of weapon, DON'T pickup yet
				m_maze3[(y + 1)*MAXCOLS3 + x].top = PLAYER;
				m_maze3[y*MAXCOLS3 + x].top = SPACE;
				player.at(0).setPosY(y + 1);

				foundItem(m_maze3, MAXCOLS3, x, y + 1);
			}
			else if (top == LAVA || bottom == LAVA) {
				//m_maze3[(y + 1)*MAXCOLS3 + x].top = PLAYER;
				//player.at(0).setPosY(y + 1);
				//if (m_maze3[y*MAXCOLS3 + x].bottom == LAVA)
				//	m_maze3[y*MAXCOLS3 + x].top = LAVA;
				//else
				//	m_maze3[y*MAXCOLS3 + x].top = SPACE;

				//// lava sound
				////auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
				//cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				//player.at(0).setHP(player.at(0).getHP() - 5);
				//dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				//if (!player.at(0).isBurned()) {
				//	m_f3actives.emplace_back(new Burn());
				//	player.at(0).toggleBurn();
				//}
			}
			else if (top == DOOR_H || top == DOOR_V || top == DOOR_H_OPEN || top == DOOR_V_OPEN) {
				if (top == DOOR_H || top == DOOR_V) {
					openDoor(x, y + 1);
				}
				else {
					m_maze3[(y + 1)*MAXCOLS3 + x].top = PLAYER;
					player.at(0).setPosY(y + 1);
					m_maze3[y*MAXCOLS3 + x].top = SPACE;
				}
			}
			else if (enemy) {
				fight(x, y + 1);
			}
		}

		else if (move == 'e') {
			collectItem(m_maze3, MAXROWS3, MAXCOLS3, x, y);

			if (getLevel() > 3)
				return;
		}

		else if (move == 'w') {
			//showDungeon();
			//player.at(0).wield(dungeonText);
		}

		else if (move == 'c') {
			//showDungeon();
			//player.at(0).use(m_f3actives, m_maze3[y*MAXCOLS3 + x], dungeonText);
		}

		// check if player is idling over lava
		if (move == 'e' || move == 'w' || move == 'c' || move == '-') {
			traptile = m_maze3[y*MAXCOLS3 + x].traptile;
			if (traptile == LAVA) {
				// lava sound
				cocos2d::experimental::AudioEngine::play2d("Fire4.mp3", false, 1.0f);

				player.at(0).setHP(player.at(0).getHP() - 5);
				dungeonText.push_back("*sizzle* ... The lava scorches you!\n");

				if (!player.at(0).isBurned()) {
					m_f3actives.emplace_back(new Burn());
					player.at(0).toggleBurn();
				}
			}
		}
	}

	// check active items in play
	checkActive(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, m_f3monsters);

	//	if player is dead then return
	if (player.at(0).getHP() <= 0) {
		m_maze3[y*MAXCOLS3 + x].top = 'X'; // change player icon to X to indicate death

		clearScreen();
		showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}

	// if all monsters are dead?
	if (m_f3monsters.empty()) {
		clearScreen();
		showDungeon();
		if (!dungeonText.empty())
			showText();

		return;
	}

	// move monsters
	int mx, my, shortest;
	char first, best, prev;
	string monster;

	for (unsigned i = 0; i < m_f3monsters.size(); i++) {
		monster = m_f3monsters.at(i)->getName();
		mx = m_f3monsters.at(i)->getPosX();
		my = m_f3monsters.at(i)->getPosY();
		first = best = prev = '0';
		shortest = 0;

		if (monster == "goblin") {
			//	allows use of goblin's smell distance function
			std::shared_ptr<Goblin> goblin = std::dynamic_pointer_cast<Goblin>(m_f3monsters.at(i));

			if (playerInRange(mx, my, i, goblin->getSmellDistance())) {
				// if viable path is found
				if (checkGoblins(m_maze3, MAXCOLS3, i, shortest, goblin->getSmellDistance(), goblin->getSmellDistance(), mx, my, first, best, prev)) {
					moveGoblins(m_maze3, MAXROWS3, MAXCOLS3, mx, my, i, best, m_f3monsters);
				}
			}
			goblin.reset();
		}

		else if (monster == "wanderer") {
			moveWanderers(m_maze3, MAXROWS3, MAXCOLS3, mx, my, i, m_f3monsters);
		}

		else if (monster == "archer") {
			checkArchers(m_maze3, MAXROWS3, MAXCOLS3, player.at(0).getPosX(), player.at(0).getPosY(), i, m_f3monsters);
		}

		else if (monster == "zapper") {
			moveZappers(m_maze3, MAXCOLS3, mx, my, i, m_f3monsters);
		}

		else if (monster == "spinner") {
			moveSpinners(m_maze3, MAXROWS3, MAXCOLS3, mx, my, i, m_f3monsters);
		}

		else if (monster == "bombee") {
			shared_ptr<Bombee> bombee = dynamic_pointer_cast<Bombee>(m_f3monsters.at(i));
			if (playerInRange(mx, my, i, bombee->getRange())) {
				// if viable path is found
				if (checkGoblins(m_maze3, MAXCOLS3, i, shortest, bombee->getRange(), bombee->getRange(), mx, my, first, best, prev)) {
					moveGoblins(m_maze3, MAXROWS3, MAXCOLS3, mx, my, i, best, m_f3monsters);
				}
			}
			bombee.reset();
		}

		else if (monster == "roundabout") {
			moveRoundabouts(m_maze3, MAXROWS3, MAXCOLS3, mx, my, i, m_f3monsters);
		}

		else if (monster == "mounted knight") {
			checkMountedKnights(m_maze3, MAXROWS3, MAXCOLS3, player.at(0).getPosX(), player.at(0).getPosY(), i, m_f3monsters);
		}

		else if (monster == "seeker") {
			shared_ptr<Seeker> seeker = dynamic_pointer_cast<Seeker>(m_f3monsters.at(i));

			if (playerInRange(mx, my, i, seeker->getRange())) {
				// if viable path is found
				if (checkGoblins(m_maze3, MAXCOLS3, i, shortest, seeker->getRange(), seeker->getRange(), mx, my, first, best, prev)) {
					moveSeekers(m_maze3, MAXROWS3, MAXCOLS3, mx, my, i, best, m_f3monsters);
				}
			}
			seeker.reset();
		}

		//	if player is dead then break
		if (player.at(0).getHP() <= 0) {
			m_maze3[y*MAXCOLS3 + x].top = 'X'; // change player icon to X to indicate death
			player.at(0).setDeath(m_f3monsters.at(i)->getName());
			break;
		}
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

					dungeonText.push_back("A bomb exploded!\n");
					explosion(dungeon, maxrows, maxcols, actives, monsters, bomb->getPosX(), bomb->getPosY(), i);
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

					dungeonText.push_back("BOOM!\n");
					explosion(dungeon, maxrows, maxcols, actives, monsters, megabomb->getPosX(), megabomb->getPosY(), i);
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
				x = spiketrap->getPosX();
				y = spiketrap->getPosY();

				if (dungeon[y*maxcols + x].traptile == SPIKETRAP_ACTIVE) {
					// retract spiketrap
					removeSprite(trap_sprites, maxrows, x, y);
					addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Deactive_48x48.png");

					spiketrap->setCountdown(spiketrap->getSpeed());
					actives.at(i) = spiketrap;
					dungeon[y*maxcols + x].traptile = SPIKETRAP_DEACTIVE;
				}
				if (spiketrap->getCountdown() == 1) {
					// spiketrap is primed
					cocos2d::experimental::AudioEngine::play2d("Spiketrap_Primed.mp3", false, (float)exp(-(abs(px - x) + abs(py - y))/2));
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
				else {
					// spiketrap is launched
					cocos2d::experimental::AudioEngine::play2d("Spiketrap_Active.mp3", false, (float)exp(-(abs(px - x) + abs(py - y))/2));
					removeSprite(trap_sprites, maxrows, x, y);
					addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Active_48x48.png");

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
				spiketrap.reset();
			}
			else if (actives.at(i)->getItem() == "trigger spike trap") {
				shared_ptr<TriggerSpike> ts = dynamic_pointer_cast<TriggerSpike>(actives.at(i));
				x = ts->getPosX();
				y = ts->getPosY();

				// if the spike trap was triggered
				if (ts->isTriggered()) {
					// spiketrap is launched
					cocos2d::experimental::AudioEngine::play2d("Spiketrap_Active.mp3", false, 1.0f);
					removeSprite(trap_sprites, maxrows, x, y);
					addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Active_48x48.png");

					dungeon[y*maxcols + x].traptile = TRIGGER_SPIKE_ACTIVE;

					//	check if player was still on top
					if (dungeon[y*maxcols + x].top == PLAYER) {
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
					if (dungeon[y*maxcols + x].top == PLAYER) {
						ts->toggleTrigger();
						actives.at(i) = ts;
						ts.reset();
						dungeonText.push_back("*tink*\n");

						// spiketrap is primed
						cocos2d::experimental::AudioEngine::play2d("Spiketrap_Primed.mp3", false, 1.0f);
						removeSprite(trap_sprites, maxrows, x, y);
						addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Primed_48x48.png");
					}
					else {
						// retract spiketrap
						cocos2d::experimental::AudioEngine::play2d("Spiketrap_Deactive.mp3", false, 1.0f);
						removeSprite(trap_sprites, maxrows, x, y);
						addSprite(trap_sprites, maxrows, x, y, -2, "Spiketrap_Deactive_48x48.png");

						dungeon[y*maxcols + x].traptile = TRIGGER_SPIKE_DEACTIVE;
					}
				}
			}
			else if (actives.at(i)->getItem() == "burn") {
				shared_ptr<Burn> burn = dynamic_pointer_cast<Burn>(actives.at(i));
				if (burn->burnsLeft() == burn->maxBurns())
					burn->setBurn(burn->burnsLeft() - 1);
				else if (burn->burnsLeft() > 0)
					burn->burn(player.at(0), dungeonText);
				else {
					dungeonText.push_back("The burning subsides.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}

				if (player.at(0).getHP() <= 0) {
					player.at(0).setDeath("burn");
					dungeon[py*maxcols + px].top = 'X';
				}
				burn.reset();
			}
			else if (actives.at(i)->getItem() == "heal") {
				shared_ptr<HealOverTime> heal = dynamic_pointer_cast<HealOverTime>(actives.at(i));
				if (heal->healsLeft() > 0)
					heal->heal(player.at(0), dungeonText);
				else {
					dungeonText.push_back("The heal fades.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}
				heal.reset();
			}
			else if (actives.at(i)->getItem() == "door") {
				shared_ptr<Door> door = dynamic_pointer_cast<Door>(actives.at(i));
				x = door->getPosX();
				y = door->getPosY();

				// if door is locked, check if room is clear to unlock
				if (door->isLocked()) {
					if (roomCleared(m_maze3, m_f3monsters, maxrows, maxcols)) {
						toggleDoorLocks(x, y, dungeon[y*maxcols + x].traptile);
					}
				}
				// if door is unlocked, open, isn't being held, and nothing is in the way
				else if (door->isOpen() && !door->isHeld() && (dungeon[y*maxcols + x].top == SPACE || dungeon[y*maxcols + x].top == DOOR_H_OPEN || dungeon[y*maxcols + x].top == DOOR_V_OPEN)) {
					// close the door
					//auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
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
void ThirdFloor::openDoor(int dx, int dy) {
	unsigned i = 0;
	while (i < m_f3actives.size()) { // finds the door just encountered
		if (m_f3actives.at(i)->getPosX() == dx && m_f3actives.at(i)->getPosY() == dy)
			break;
		else
			i++;
	}

	shared_ptr<Door> door = dynamic_pointer_cast<Door>(m_f3actives.at(i));

	if (!door->isLocked()) {
		// door opening sound
		//auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
		cocos2d::experimental::AudioEngine::play2d("Door_Opened.mp3", false, 1.0f);

		// remove door sprite
		removeSprite(door_sprites, MAXROWS3, dx, dy);

		door->toggleOpen();
		m_f3actives.at(i) = door;
		m_maze3[dy*MAXCOLS3 + dx].top = (m_maze3[dy*MAXCOLS3 + dx].top == DOOR_H ? DOOR_H_OPEN : DOOR_V_OPEN);
	}
	else {
		// locked door sound
		cocos2d::experimental::AudioEngine::play2d("Locked.mp3", false, 1.0f);
	}

	door.reset();
}
void ThirdFloor::toggleDoorLocks(int dx, int dy, char doortype) {
	int px = player.at(0).getPosX();
	int py = player.at(0).getPosY();

	//auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
	cocos2d::experimental::AudioEngine::play2d("Door_Locked.mp3", false, 1.0f);

	int x, y;
	std::string item;
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

void ThirdFloor::fight(int x, int y) { // monster's coordinates
	unsigned i = 0;
	bool found = false;
	while (!found && i < m_f3monsters.size()) { // finds the monster just encountered
		if (m_f3monsters.at(i)->getPosX() == x && m_f3monsters.at(i)->getPosY() == y)
			found = true;
		else
			i++;
	}
	string monster = m_f3monsters.at(i)->getName();
	player.at(0).attack(m_f3monsters, m_f3actives, i, dungeonText);
	if (m_f3monsters.at(i)->getHP() <= 0) {
		monsterDeath(m_maze3, MAXROWS3, MAXCOLS3, m_f3actives, m_f3monsters, i);// x, y, i);
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
	for (int i = 0; i < trap_sprites.size(); i++) {
		trap_sprites[i]->setVisible(false);
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
	for (int i = 0; i < monster_sprites.size(); i++) {
		spritePoint = monster_sprites[i]->getPosition();
		sx = (spritePoint.x + X_OFFSET) / SPACING_FACTOR; sxchunk = sx / 14;
		sy = MAXROWS3 - ((spritePoint.y + Y_OFFSET) / SPACING_FACTOR); sychunk = sy / 10;

		if (sxchunk == xchunk && sychunk == ychunk) {
			monster_sprites[i]->setVisible(true);
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

std::vector<_Tile> ThirdFloor::getDungeon() const {
	return m_maze3;
}
int ThirdFloor::getRows() const {
	return m_rows;
}
int ThirdFloor::getCols() const {
	return m_cols;
}
std::vector<std::shared_ptr<Monster>> ThirdFloor::getMonsters() const {
	return m_f3monsters;
}
std::vector<std::shared_ptr<Objects>> ThirdFloor::getActives() const {
	return m_f3actives;
}
void ThirdFloor::callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index) {
	string item = player.at(0).getItemInv().at(index).getItem(); // get item name
	std::string image = player.at(0).use(m_f3actives, dungeon[y*maxcols + x], index); // gets sprite, if any
	if (item == "Bomb")
		addSprite(projectile_sprites, maxrows, x, y, -1, image);
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
								 {'#', ' ', ' ', ' ', ' ',  c1,BOMB,  c1, ' ', ' ', ' ', ' ', ' ',  dv},
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

	char c5 = LIFEPOT;
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

	char c2 = BROWN_CHEST;
	char c3 = SILVER_CHEST;
	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', unb},
								  {'#',  c3, '#', '#', '#', '#', 'W', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#',  c2, ' ', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', ' ', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  dv},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', '#', '#',  dh, '#', '#', '#', '#', '#', '#', unb}, };

	char c4 = BOMB;
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

	char c2 = GOLDEN_CHEST;
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

	char c3 = ARMOR;
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
								  {' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ',BOMB,LAVA, 'A', '#', unb},
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

	char c1 = HEART_POD;
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
								 {'#', ' ', ' ', ' ', ' ', '#',BOMB, '#', ' ', ' ', ' ', ' ', ' ',  dv},
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

	char c3 = HEART_POD;
	std::vector<std::vector<char>> five = { {unb, unb, unb, unb, unb, unb,  dh, unb, unb, unb, unb, unb, unb, unb},
								  {'#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', unb},
								  {'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', 'W', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', 'W', 'C', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', unb},
								  {'#', '#', '#', 'C', ' ', ' ', 'Z', ' ', ' ', ' ', ' ', ' ', ' ',  dv},
								  {'#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', unb},
								  {'#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', '#', '#', ' ', '#', unb},
								  {'#', '#', '#', '#', '#', '#', ' ', ' ', '#',  c3, '#', ' ', '#', unb},
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

	char c1 = HEART_POD;
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
								 {' ', ' ', ' ', ' ',  c3,  c2,BOMB,  c2,  c3, ' ', ' ', ' ', ' ',  dv},
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
								 {' ', ' ', ' ', ' ', ' ', '#',BOMB, '#', ' ', ' ', ' ', ' ', ' ',  dv},
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
								 {' ', ' ', 'W', ' ', ' ', '#',BOMB, '#', ' ', ' ', 'W', ' ', ' ',  dv},
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
								 {'#',ARMOR,'#', ' ', ' ', ' ', ' ', ' ', '#',BOMB, ' ', ' ', '#', unb},
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
	char c2 = STATPOT;
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

	char c3 = SILVER_CHEST;
	char c4 = BUTTON;
	char c5 = LIFEPOT;
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

	char amr = ARMOR;
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
	char inf = INFINITY_BOX;
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
	char inf = INFINITY_BOX;
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
FirstBoss::FirstBoss(Player p) : Dungeon(4) {
	_Tile *tile;
	int i, j;

	for (i = 0; i < BOSSROWS; i++) { //		initialize entire dungeon to blank space
		for (j = 0; j < BOSSCOLS; j++) {
			tile = new _Tile;

			// all layers set to empty space
			tile->upper = tile->top = tile->bottom = tile->projectile = tile->traptile = SPACE;
			tile->wall = tile->item = tile->enemy = tile->trap = tile->overlap = tile->enemy_overlap = tile->marked = false;
			tile->hero = false;
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

	return *this;
}

void FirstBoss::peekFirstBossDungeon(int x, int y, char move) {
	char upper, top, bottom, traptile;
	bool wall, item, trap, enemy;

	// BEGIN move player
	int weprange = player.at(0).getWeapon().getRange();
	bool move_used = false;
	bool far_enemy, close_enemy;

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
				//m_boss[y*BOSSCOLS + x].top = SPACE;
				m_boss[y*BOSSCOLS + x].hero = false;
				//m_boss[y*BOSSCOLS + x - 1].top = PLAYER;
				m_boss[y*BOSSCOLS + x - 1].hero = true;
				player.at(0).setPosX(x - 1);

				if (trap) {
					if (traptile == SPIKE) {
						trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x - 1, y);

						m_boss[y*BOSSCOLS + x - 1].traptile = SPACE;
						m_boss[y*BOSSCOLS + x - 1].trap = false;

						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							clearScreen();
							showDungeon();
							if (!dungeonText.empty())
								showText();
							return;
						}
					}
					if (traptile == SPIKETRAP_ACTIVE) {
						trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x - 1, y);
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
				//m_boss[y*BOSSCOLS + x].top = SPACE;
				m_boss[y*BOSSCOLS + x].hero = false;
				//m_boss[y*BOSSCOLS + x + 1].top = PLAYER;
				m_boss[y*BOSSCOLS + x + 1].hero = true;
				player.at(0).setPosX(x + 1);

				if (trap) {
					if (traptile == SPIKE) {
						trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x + 1, y);

						m_boss[y*BOSSCOLS + x + 1].traptile = SPACE;
						m_boss[y*BOSSCOLS + x + 1].trap = false;

						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							clearScreen();
							showDungeon();
							if (!dungeonText.empty())
								showText();
							return;
						}
					}
					if (traptile == SPIKETRAP_ACTIVE) {
						trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x + 1, y);
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
				//m_boss[y*BOSSCOLS + x].top = SPACE;
				m_boss[y*BOSSCOLS + x].hero = false;
				//m_boss[(y - 1)*BOSSCOLS + x].top = PLAYER;
				m_boss[(y - 1)*BOSSCOLS + x].hero = true;
				player.at(0).setPosY(y - 1);

				if (trap) {
					if (traptile == SPIKE) {
						trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x, y - 1);

						m_boss[(y - 1)*BOSSCOLS + x].traptile = SPACE;
						m_boss[(y - 1)*BOSSCOLS + x].trap = false;

						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							clearScreen();
							showDungeon();
							if (!dungeonText.empty())
								showText();
							return;
						}
					}
					if (traptile == SPIKETRAP_ACTIVE) {
						trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x, y - 1);
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
				//m_boss[y*BOSSCOLS + x].top = SPACE;
				m_boss[y*BOSSCOLS + x].hero = false;
				//m_boss[(y + 1)*BOSSCOLS + x].top = PLAYER;
				m_boss[(y + 1)*BOSSCOLS + x].hero = true;
				player.at(0).setPosY(y + 1);

				if (trap) {
					if (traptile == SPIKE) {
						trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x, y + 1);

						m_boss[(y + 1)*BOSSCOLS + x].traptile = SPACE;
						m_boss[(y + 1)*BOSSCOLS + x].trap = false;

						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("falling spike");
							clearScreen();
							showDungeon();
							if (!dungeonText.empty())
								showText();
							return;
						}
					}
					if (traptile == SPIKETRAP_ACTIVE) {
						trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x, y + 1);
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

		else if (move == 'w') {
			showDungeon();
			player.at(0).wield(dungeonText);
		}

		else if (move == 'c') {
			showDungeon();
			player.at(0).use(m_firstbossActives, m_boss[y*BOSSCOLS + x], dungeonText);
		}
	}

	// check active items in play
	checkActive(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters);
	if (player.at(0).getHP() <= 0) {
		clearScreen();
		//showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}

	// if all monsters are dead?
	if (m_firstbossMonsters.empty()) {
		clearScreen();
		//showDungeon();
		if (!dungeonText.empty())
			showText();

		return;
	}

	// move any monsters
	if (m_firstbossMonsters.size() > 1) {
		for (unsigned i = 1; i < m_firstbossMonsters.size(); i++) {
			if (m_firstbossMonsters.at(i)->getName() == "archer") {
				checkArchers(m_boss, BOSSROWS, BOSSCOLS, player.at(0).getPosX(), player.at(0).getPosY(), i, m_firstbossMonsters);
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

		clearScreen();
		//showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}

	//	check if smasher is currently executing a move
	checkSmasher();

	//	if killed by smasher then return
	if (player.at(0).getHP() <= 0) {
		//m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = 'X'; //change player icon to X to indicate death
		player.at(0).setDeath(m_firstbossMonsters.at(0)->getName());
		return;
	}

	clearScreen();
	//showDungeon();
	if (!dungeonText.empty())
		showText();
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

					dungeonText.push_back("A bomb exploded!\n");
					explosion(dungeon, maxrows, maxcols, actives, monsters, bomb->getPosX(), bomb->getPosY(), i);
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

					dungeonText.push_back("BOOM!\n");
					explosion(dungeon, maxrows, maxcols, actives, monsters, megabomb->getPosX(), megabomb->getPosY(), i);
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
			else if (actives.at(i)->getItem() == "burn") {
				std::shared_ptr<Burn> burn = std::dynamic_pointer_cast<Burn>(actives.at(i));
				if (burn->burnsLeft() == burn->maxBurns())
					burn->setBurn(burn->burnsLeft() - 1);
				else if (burn->burnsLeft() > 0)
					burn->burn(player.at(0), dungeonText);
				else {
					dungeonText.push_back("The burning subsides.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}

				if (player.at(0).getHP() <= 0) {
					player.at(0).setDeath("burn");
					dungeon[py*maxcols + px].top = 'X';
				}
				burn.reset();
			}
			else if (actives.at(i)->getItem() == "heal") {
				shared_ptr<HealOverTime> heal = dynamic_pointer_cast<HealOverTime>(actives.at(i));
				if (heal->healsLeft() > 0)
					heal->heal(player.at(0), dungeonText);
				else {
					dungeonText.push_back("The heal fades.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}
				heal.reset();
			}
			else if (actives.at(i)->getItem() == "bleed") {
				shared_ptr<Bleed> bleed = dynamic_pointer_cast<Bleed>(actives.at(i));

				if (bleed->bleedLeft() > 0) {
					// find the monster(s) that are bleeding
					bool found = false;
					for (int n = 0; n < monsters.size(); n++) {
						// if bleed count of object equals bleed count of the monster, bleed the monster
						if (monsters.at(n)->isBled() && bleed->bleedLeft() == monsters.at(n)->bleedLeft()) {
							bleed->bleed(*(monsters.at(n)), dungeonText);
							if (monsters.at(n)->getHP() <= 0) {
								monsterDeath(dungeon, maxrows, maxcols, actives, monsters, n);// monsters.at(n)->getPosX(), monsters.at(n)->getPosY(), n);
								pos = i;
								actives.erase(actives.begin() + pos);
							}
							found = true;
							break;
						}
					}

					// if monster was not found and player is not bled, remove the affliction from the active pool
					if (!(found || player.at(0).isBled())) {
						pos = i;
						actives.erase(actives.begin() + pos);
					}

					// if player is bleeding
					if (player.at(0).isBled()) {
						bleed->bleed(player.at(0), dungeonText);

						if (player.at(0).getHP() <= 0) {
							player.at(0).setDeath("bleed");
							dungeon[py*maxcols + px].top = 'X';
						}
					}
				}
				else {
					dungeonText.push_back("The bleeding stops.\n");
					pos = i;
					actives.erase(actives.begin() + pos);
				}

				bleed.reset();
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
			addSprite(monster_sprites, BOSSROWS, archer.getPosX(), archer.getPosY(), 1, "Archer_48x48.png");

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
				int pos = findMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, m_firstbossMonsters);
				m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
				
				//pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', m_firstbossMonsters, col - 2, 0);
				//finishActions();
				//moveSprite(monster_sprites, BOSSROWS, i, j, col - 2, j);

				// check if monster died
				if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
					finishActions();
					monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
				}
			}

			m_boss[j*BOSSCOLS + i].upper = SMASHER;
			m_boss[j*BOSSCOLS + i].enemy = true;
		}

		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		moveSprite(monster_sprites, BOSSROWS, col, row, 'l');
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
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them
				if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {					
					int pos = findMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, m_firstbossMonsters);
					m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
					
					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != col - 3 && m_boss[j*BOSSCOLS + (i - 1)].enemy) {
						int n = findMonster(m_boss, BOSSROWS, BOSSCOLS, i - 1, j, m_firstbossMonsters);
						finishActions();
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, n);
					}

					// move the monster and sprite
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', m_firstbossMonsters, col - 3, 0);
					finishActions();
					moveSprite(monster_sprites, BOSSROWS, i, j, col - 3, j);

					// check if monster died
					if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
						finishActions();
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);// i, j, pos);
					}
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;

			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		moveSprite(monster_sprites, BOSSROWS, col, row, 'L');
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
					finishActions();
					moveSprite(monster_sprites, BOSSROWS, i, j, col - 4, j);
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;
			}
		}
		moveSprite(monster_sprites, BOSSROWS, col, row, 'L');
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
			//enemyOverlap(m_boss, BOSSROWS, BOSSCOLS, i, j);
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
				int pos = findMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, m_firstbossMonsters);
				m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
				
				//pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', m_firstbossMonsters, col + 2, 0);
				//finishActions();
				//moveSprite(monster_sprites, BOSSROWS, i, j, col + 2, j);

				// check if monster died
				if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
					monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
				}
			}

			m_boss[j*BOSSCOLS + i].upper = SMASHER;
			m_boss[j*BOSSCOLS + i].enemy = true;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		moveSprite(monster_sprites, BOSSROWS, col, row, 'r');
		m_firstbossMonsters.at(0)->setPosX(col + 1);
	}

	//	if there are only two spaces to the right of the smasher
	else if (m_boss[row*BOSSCOLS + col + 4].top == UNBREAKABLE_WALL) {
		// left two layers of smasher is set to spaces
		for (i = col - 1; i < col + 1; i++) {
			for (j = row - 1; j < row + 2; j++) {
				//m_boss[j*BOSSCOLS + i].upper = SPACE;
				
				enemyOverlap(m_boss, BOSSROWS, BOSSCOLS, i, j);
				//m_boss[j*BOSSCOLS + i].enemy = false;
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
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them
				if (m_boss[j*BOSSCOLS + i].enemy && m_boss[j*BOSSCOLS + i].upper != SMASHER) {
					// crush the monster
					int pos = findMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, m_firstbossMonsters);
					m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
					
					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != col + 3 && m_boss[j*BOSSCOLS + (i + 1)].enemy) {
						int n = findMonster(m_boss, BOSSROWS, BOSSCOLS, i + 1, j, m_firstbossMonsters);
						finishActions();
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, n);
					}

					// move the sprite
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, i, j, 'X', m_firstbossMonsters, col + 3, 0);
					finishActions();
					moveSprite(monster_sprites, BOSSROWS, i, j, col + 3, j);

					// check if monster died (should be dead)
					if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
						finishActions();
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);// i, j, pos);
					}
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;

			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		moveSprite(monster_sprites, BOSSROWS, col, row, 'R');
		m_firstbossMonsters.at(0)->setPosX(col + 2);
	}

	//	if there are at least 3 spaces to right of the smasher
	else if (m_boss[row*BOSSCOLS + col + 4].top != UNBREAKABLE_WALL) {
		// left two layers of smasher is set to spaces
		for (i = col - 1; i < col + 1; i++) {
			for (j = row - 1; j < row + 2; j++) {
				//m_boss[j*BOSSCOLS + i].upper = SPACE;

				enemyOverlap(m_boss, BOSSROWS, BOSSCOLS, i, j);
				//m_boss[j*BOSSCOLS + i].enemy = false;
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
					finishActions();
					moveSprite(monster_sprites, BOSSROWS, i, j, col + 4, j);
				}

				m_boss[j*BOSSCOLS + i].upper = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;
			}
		}
		moveSprite(monster_sprites, BOSSROWS, col, row, 'R');
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
			//enemyOverlap(m_boss, BOSSROWS, BOSSCOLS, j, i);
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
				int pos = findMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, m_firstbossMonsters);
				m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));

				// check if monster died
				if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
					finishActions();
					monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
				}
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		moveSprite(monster_sprites, BOSSROWS, col, row, 'u');
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
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them
				if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
					// find and crush the monster
					int pos = findMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, m_firstbossMonsters);
					m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));

					// if there are two enemies stacked, then kill the enemy closest to the wall first
					// :::: This is for the purpose of looking more natural on screen.
					if (i != 1 && m_boss[(i - 1)*BOSSCOLS + j].enemy) {
						int n = findMonster(m_boss, BOSSROWS, BOSSCOLS, j, i - 1, m_firstbossMonsters);
						finishActions();
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, n);
					}

					// move the monster
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', m_firstbossMonsters, 0, row - 3);
					finishActions();
					moveSprite(monster_sprites, BOSSROWS, j, i, j, row - 3);

					// check if monster died
					if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
						finishActions();
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
					}
					
				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;

			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		moveSprite(monster_sprites, BOSSROWS, col, row, 'U');
		m_firstbossMonsters.at(0)->setPosY(row - 2);
	}

	//	if there are at least 3 spaces above the smasher
	else if (m_boss[(row - 4)*BOSSCOLS + col].top != UNBREAKABLE_WALL) {
		// bottom 2 layers of smasher is set to spaces
		for (i = row + 1; i > row - 1; i--) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].upper = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
				//enemyOverlap(m_boss, BOSSROWS, BOSSCOLS, j, i);
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
					finishActions();
					moveSprite(monster_sprites, BOSSROWS, j, i, j, row - 4);					
				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		moveSprite(monster_sprites, BOSSROWS, col, row, 'U');
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

				int pos = findMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, m_firstbossMonsters);
				m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
					
				// check if monster died
				if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
					finishActions();
					monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
				}
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		moveSprite(monster_sprites, BOSSROWS, col, row, 'd');
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
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				
				// if there are any monsters in the way, crush them dead
				if (m_boss[i*BOSSCOLS + j].enemy && m_boss[i*BOSSCOLS + j].upper != SMASHER) {
					// smasher attacking a monster is a formality at this point
					int pos = findMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, m_firstbossMonsters);
					m_firstbossMonsters.at(0)->encounter(*(m_firstbossMonsters.at(0)), *(m_firstbossMonsters.at(pos)));
					
					// if there are two enemies stacked, then kill the second enemy first.
					// :::: This is for the purpose of looking more natural on screen.
					if (i != row + 3 && m_boss[(i + 1)*BOSSCOLS + j].enemy) {
						int n = findMonster(m_boss, BOSSROWS, BOSSCOLS, j, i + 1, m_firstbossMonsters);
						finishActions();
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, n);
					}

					// NOTE: this must be after the attack, otherwise pos is then incorrect
					pushMonster(m_boss, BOSSROWS, BOSSCOLS, j, i, 'Y', m_firstbossMonsters, 0, BOSSROWS - 2);
					finishActions();
					moveSprite(monster_sprites, BOSSROWS, j, i, j, BOSSROWS - 2);

					// check if monster died (monster will be dead)
					if (m_firstbossMonsters.at(pos)->getHP() <= 0) {
						finishActions();
						monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, pos);
					}

				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		cocos2d::experimental::AudioEngine::play2d("Smasher_Smash.mp3", false, 1.0f);
		moveSprite(monster_sprites, BOSSROWS, col, row, 'D');
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
					finishActions();
					moveSprite(monster_sprites, BOSSROWS, j, i, j, row + 4);
				}

				m_boss[i*BOSSCOLS + j].upper = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		moveSprite(monster_sprites, BOSSROWS, col, row, 'D');
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
				finishActions();				
				moveSprite(monster_sprites, BOSSROWS, j, i, j, row - 5);
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
	}

	moveSprite(monster_sprites, BOSSROWS, col, row, '^');
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
			//enemyOverlap(m_boss, BOSSROWS, BOSSCOLS, j, i);
		}
	}

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
				finishActions();
				moveSprite(monster_sprites, BOSSROWS, j, i, j, row + 4);
			}

			m_boss[i*BOSSCOLS + j].upper = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
	}
	moveSprite(monster_sprites, BOSSROWS, col, row, 'D');

	m_firstbossMonsters.at(0)->setPosY(row + 2);
	smasher.reset();
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
					finishActions();
					moveSprite(monster_sprites, BOSSROWS, ox, oy, mx, my);
				}
				else {
					finishActions();
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
				break;
			}
				//	CUSTOM MOVEMENT CASE in Y direction
			case 'Y': {
				dungeon[my*maxcols + mx].top = SPACE;
				dungeon[my*maxcols + mx].enemy = false;

				dungeon[(cy)*maxcols + mx].top = ARCHER;
				dungeon[(cy)*maxcols + mx].enemy = true;
				
				monsters[i]->setPosY(cy);
				break;
			}
			}
			return;
		}
	}
}
int FirstBoss::findMonster(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, std::vector<std::shared_ptr<Monster>> monsters) {
	for (int i = 0; i < monsters.size(); i++) {
		if (monsters.at(i)->getPosX() == mx && monsters.at(i)->getPosY() == my) {
			return i;
		}
	}
	return -1;
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

void FirstBoss::checkArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int x, int y, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	shared_ptr<Archer> archer = dynamic_pointer_cast<Archer>(monsters.at(pos));
	int mx = monsters.at(pos)->getPosX();
	int my = monsters.at(pos)->getPosY();

	//	if archer is primed, check to attack and return
	if (archer->isPrimed()) {
		if (x - mx == 0 || y - my == 0)
			monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		else
			dungeonText.push_back("The archer eases their aim.\n");

		archer->prime(false);
		monsters.at(pos).reset();
		monsters.at(pos) = archer;
		archer.reset();

		return;
	}


	// if archer has dagger out and is adjacent to player
	if (monsters.at(pos)->getWeapon().getAction() == "Bronze Dagger" && \
		(dungeon[(my - 1)*maxcols + mx].hero || dungeon[(my + 1)*maxcols + mx].hero || dungeon[my*maxcols + mx - 1].hero || dungeon[my*maxcols + mx + 1].hero)) {
		monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		return;
	}
	
	// else if dagger is out and player moved away
	else if (monsters.at(pos)->getWeapon().getAction() == "Bronze Dagger" && \
		(!dungeon[(my - 1)*maxcols + mx].hero && !dungeon[(my + 1)*maxcols + mx].hero && !dungeon[my*maxcols + mx - 1].hero && !dungeon[my*maxcols + mx + 1].hero)) {
		monsters.at(pos)->setWeapon(WoodBow());
		//dungeonText.push_back("The archer switches back to their Wood Bow.\n");
		return;
	}

	//if player is not in the archer's sights, move archer
	if (x - mx != 0 && y - my != 0) {
		moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
		return;
	}

	// if player and archer are on the same column
	if (x - mx == 0) {
		//	if player is within range and not adjacent, get primed
		if (abs(y - my) > 1 && abs(y - my) <= 15) {
			if (!wallCollision(dungeon, maxcols, 'y', y, my)) {
				archer->prime(true);
				monsters.at(pos).reset();
				monsters.at(pos) = archer;
				archer.reset();

				//dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
				return;
			}
		}
		else if (abs(y - my) == 1) {
			//swap weapon to dagger
			monsters.at(pos)->setWeapon(BronzeDagger());
			dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
		}
		//	just move archer
		else {
			moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
			return;
		}
	}
	// if player and archer are on the same row
	else if (y - my == 0) {
		//	if player is within range and not adjacent, get primed
		if (abs(x - mx) > 1 && abs(x - mx) <= 15) {
			if (!wallCollision(dungeon, maxcols, 'x', x, mx)) {
				archer->prime(true);
				monsters.at(pos).reset();
				monsters.at(pos) = archer;
				archer.reset();

				dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
				return;
			}
		}
		else if (abs(x - mx) == 1) {
			// swap weapon to dagger
			monsters.at(pos)->setWeapon(BronzeDagger());
			dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
		}
		// just move archer
		else {
			moveArchers(dungeon, maxrows, maxcols, mx, my, pos, monsters);
		}
	}

}
void FirstBoss::moveArchers(std::vector<_Tile> &dungeon, int maxrows, int maxcols, int mx, int my, int pos, std::vector<std::shared_ptr<Monster>> &monsters) {
	int n = randInt(2);
	int m = 0;
	char move;

	char leftwall, rightwall, upwall, downwall;
	leftwall = dungeon[my*maxcols + mx - 1].wall;
	rightwall = dungeon[my*maxcols + mx + 1].wall;
	upwall = dungeon[(my - 1)*maxcols + mx].wall;
	downwall = dungeon[(my + 1)*maxcols + mx].wall;

	char leftenemy, rightenemy, upenemy, downenemy;
	leftenemy = dungeon[my*maxcols + mx - 1].enemy;
	rightenemy = dungeon[my*maxcols + mx + 1].enemy;
	upenemy = dungeon[(my - 1)*maxcols + mx].enemy;
	downenemy = dungeon[(my + 1)*maxcols + mx].enemy;

	//  if archer is surrounded completely, do nothing (such as goblin or wanderer blocking an archer's path)
	if ((leftwall || leftenemy) && (rightwall || rightenemy) &&
		(downwall || downenemy) && (upwall || upenemy)) {
		return;
	}
	//	else if archer is surrounded by walls on left and right
	else if ((leftwall || leftenemy) && (rightwall || rightenemy)) {
		move = 'v';
	}
	//	else if archer is surrounded by walls on top and bottom
	else if ((downwall || downenemy) && (upwall || upenemy)) {
		move = 'h';
	}
	//	else pick a random direction
	else {
		move = (n == 0 ? 'h' : 'v');
	}

	if (move == 'h') {
		//	move is horizontal
		n = -1 + randInt(3);
		while (n == 0 || dungeon[my*maxcols + mx + n].top != SPACE || dungeon[my*maxcols + mx + n].upper != SPACE) {
			n = -1 + randInt(3);
		}

		dungeon[my*maxcols + mx + n].top = ARCHER;
		dungeon[my*maxcols + mx + n].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		//enemyOverlap(dungeon, maxrows, maxcols, mx, my);

		moveSprite(monster_sprites, maxrows, mx, my, n == -1 ? 'l' : 'r');
		monsters.at(pos)->setPosX(mx + n);
	}
	else if (move == 'v') {
		//	move is vertical
		m = -1 + randInt(3);
		while (m == 0 || dungeon[(my + m)*maxcols + mx].top != SPACE || dungeon[(my + m)*maxcols + mx].upper != SPACE) {
			m = -1 + randInt(3);
		}

		dungeon[(my + m)*maxcols + mx].top = ARCHER;
		dungeon[(my + m)*maxcols + mx].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		//enemyOverlap(dungeon, maxrows, maxcols, mx, my);
		moveSprite(monster_sprites, maxrows, mx, my, m == -1 ? 'u' : 'd');
		monsters.at(pos)->setPosY(my + m);
	}

}

void FirstBoss::fight(int x, int y) { // monster's coordinates
	
	// Check if monster encountered was smasher
	if (m_boss[y*BOSSCOLS + x].upper == SMASHER) {
		player.at(0).attack(m_firstbossMonsters, m_firstbossActives, 0, dungeonText);

		if (m_firstbossMonsters.at(0)->getHP() <= 0)
			monsterDeath(0);

		return;
	}

	int i = findMonster(m_boss, BOSSROWS, BOSSCOLS, x, y, m_firstbossMonsters);

	// if monster wasn't found, return
	if (i == -1)
		return;
	
	player.at(0).attack(m_firstbossMonsters, m_firstbossActives, i, dungeonText);

	if (m_firstbossMonsters.at(i)->getHP() <= 0) {
		monsterDeath(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters, i);// x, y, i);
	}
}

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

std::vector<_Tile> FirstBoss::getDungeon() const {
	return m_boss;
}
int FirstBoss::getRows() const {
	return m_rows;
}
int FirstBoss::getCols() const {
	return m_cols;
}
std::vector<std::shared_ptr<Monster>> FirstBoss::getMonsters() const {
	return m_firstbossMonsters;
}
std::vector<std::shared_ptr<Objects>> FirstBoss::getActives() const {
	return m_firstbossActives;
}
void FirstBoss::callUse(std::vector<_Tile> &dungeon, const int maxrows, const int maxcols, int x, int y, int index) {
	string item = player.at(0).getItemInv().at(index).getItem(); // get item name
	std::string image = player.at(0).use(m_firstbossActives, dungeon[y*maxcols + x], index); // gets sprite, if any
	if (item == "Bomb")
		addSprite(projectile_sprites, maxrows, x, y, -1, image);
}

void FirstBoss::finishActions() {
	// if there are any lingering actions, finish them
	auto actions = m_scene->getActionManager();
	while (actions->getNumberOfRunningActions() > 1) {
		actions->update(1.0);
	}
}