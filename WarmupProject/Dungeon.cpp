#include "global.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include "Actors.h"
#include "utilities.h"
#include <iostream>

using namespace std;

Dungeon::Dungeon() : m_level(1) {
	Tile *tile;
	int i, j;
	
	for (i = 0; i < MAXROWS; i++) { //		initialize entire dungeon to blank space
		for (j = 0; j < MAXCOLS; j++) {
			tile = new Tile;
			tile->top = SPACE;
			tile->bottom = SPACE;
			tile->enemy = false;
			tile->marked = false;

			m_maze[i*MAXCOLS + j] = *tile;
		}
	}

	for (i = 0; i < MAXROWS; i++) { //	initialize top and bottom of dungeon to be unbreakable walls
		for (j = 0; j < MAXCOLS; j++) {
			m_maze[i*MAXCOLS + j].top = UNBREAKABLE_WALL;
			m_maze[i*MAXCOLS + j].bottom = UNBREAKABLE_WALL;
		}
		i += (MAXROWS - 2);
	}

	for (i = 0; i < MAXROWS - 1; i++){ //		initialize edges of dungeon to be #
		//m_maze[i][0].top = m_maze[i][0].bottom = UNBREAKABLE_WALL;
		//m_maze[i][69].top = m_maze[i][69].bottom = UNBREAKABLE_WALL;
		m_maze[i*MAXCOLS].top = m_maze[i*MAXCOLS].bottom = UNBREAKABLE_WALL;
		m_maze[i*MAXCOLS + MAXCOLS - 1].top = m_maze[i*MAXCOLS + MAXCOLS -1].bottom = UNBREAKABLE_WALL;
	}

	// RANDOMLY GENERATES LEVEL CHUNKS
	vector<vector<vector<char>>> b1, b2, b3, b4;
	vector<char> v1, v2, v3, v4, finalvec;
	
	v1 = topChunks(b1);
	v2 = middleChunks(b2);
	v3 = middleChunks(b3);
	v4 = bottomChunks(b4);

	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v1.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v2.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v3.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v4.at(i));
	
	int count = 0;
	char object;
	for (i = 1; i < MAXROWS - 1; i++) {
		for (j = 1; j < MAXCOLS - 1; j++) {
			//m_maze[i][j].top = finalvec.at(count);
			//m_maze[i][j].bottom = finalvec.at(count);
			m_maze[i*MAXCOLS + j].top = finalvec.at(count);
			m_maze[i*MAXCOLS + j].top = finalvec.at(count);

			//	if the object was an item or enemy
			if (m_maze[i*MAXCOLS + j].bottom != WALL && m_maze[i*MAXCOLS + j].bottom != SPACE) {

				object = m_maze[i*MAXCOLS + j].bottom;
				switch (object) {
				case BROWN_CHEST:
				case SILVER_CHEST:
					m_maze[i*MAXCOLS + j].top = SPACE;
					break;
				case WANDERER: {
					m_maze[i*MAXCOLS + j].bottom = SPACE;
					Wanderer wand;
					wand.setPosX(j);
					wand.setPosY(i);
					m_monsters.emplace_back(new Wanderer(wand));
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
	while (toptile != SPACE || player.at(0).getPosX() > 4 && player.at(0).getPosX() < 65) {
		player.at(0).setrandPosX();
		player.at(0).setrandPosY();

		// while player is not spawned along left side or right side, reroll
		while (player.at(0).getPosX() > 4 && player.at(0).getPosX() < 65) {
			player.at(0).setrandPosX();
		}

		toptile = m_maze[player.at(0).getPosY()*MAXCOLS + player.at(0).getPosX()].top;
	}
	m_maze[player.at(0).getPosY()*MAXCOLS + player.at(0).getPosX()].top = PLAYER; // sets tile at this spot to be player position
	player.at(0).addItem(Bomb());	// adds bomb to player's inventory upon spawn

	/*
	Idol idol;
	toptile = m_maze[idol.getPosY()*MAXCOLS + idol.getPosX()].top;

	//	while idol does not spawn on the side opposite from the player
	while (toptile != SPACE || ((player.at(0).getPosX() < 5 && idol.getPosX() < 65) || (player.at(0).getPosX() > 64 && idol.getPosX() > 4))) {
		idol.setrandPosX();
		idol.setrandPosY();

		toptile = m_maze[idol.getPosY()*MAXCOLS + idol.getPosX()].top;
	}
	m_maze[idol.getPosY()*MAXCOLS + idol.getPosX()].bottom = IDOL;*/


	Stairs stairs;
	toptile = m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].top;

	//	while stairs does not spawn on the side opposite from the player
	//while (toptile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() < 65) || (player.at(0).getPosX() > 64 && stairs.getPosX() > 4))) {
	while (toptile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() >= 5) || (player.at(0).getPosX() > 64 && stairs.getPosX() < 64))) {
		stairs.setrandPosX();
		stairs.setrandPosY();

		toptile = m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].top;
	}
	m_maze[stairs.getPosY()*MAXCOLS + stairs.getPosX()].bottom = STAIRS;


	LifePotion lp;
	char bottomtile = m_maze[lp.getPosY()*MAXCOLS + lp.getPosX()].bottom;
	while (bottomtile != SPACE) {
		lp.setrandPosX();
		lp.setrandPosY();

		bottomtile = m_maze[lp.getPosY()*MAXCOLS + lp.getPosX()].bottom;
	}
	m_maze[lp.getPosY()*MAXCOLS + lp.getPosX()].bottom = LIFEPOT;


	Bomb bomb;
	bottomtile = m_maze[bomb.getPosY()*MAXCOLS + bomb.getPosX()].bottom;
	while (bottomtile != SPACE) {
		bomb.setrandPosX();
		bomb.setrandPosY();

		bottomtile = m_maze[bomb.getPosY()*MAXCOLS + bomb.getPosX()].bottom;
	}
	m_maze[bomb.getPosY()*MAXCOLS + bomb.getPosX()].bottom = BOMB;


	BrownChest chest;
	bottomtile = m_maze[chest.getPosY()*MAXCOLS + chest.getPosX()].bottom;
	while (bottomtile != SPACE) {
		chest.setrandPosX();
		chest.setrandPosY();

		bottomtile = m_maze[chest.getPosY()*MAXCOLS + chest.getPosX()].bottom;
	}
	m_maze[chest.getPosY()*MAXCOLS + chest.getPosX()].bottom = BROWN_CHEST;


	int numwep = 1 + randInt(2); // number of weapons to be placed
	while (numwep > 0) {
		RustyCutlass rc;
		
		bottomtile = m_maze[rc.getPosY()*MAXCOLS + rc.getPosX()].bottom;
		while (bottomtile != SPACE) { // while sword position clashes with anything
			rc.setrandPosX();				// reroll it
			rc.setrandPosY();

			bottomtile = m_maze[rc.getPosY()*MAXCOLS + rc.getPosX()].bottom;
		}
		m_maze[rc.getPosY()*MAXCOLS + rc.getPosX()].bottom = CUTLASS;
		numwep--;
	}

	//	generate goblins
	int m = 5 + randInt(3); // number of goblins to be placed
	while (m > 0) { //generate goblins
		Goblin g(10);

		toptile = m_maze[g.getPosY()*MAXCOLS + g.getPosX()].top;
		while (toptile != SPACE) { //while goblin position clashes with wall, player, or idol
			g.setrandPosX();				// reroll it
			g.setrandPosY();

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
			w.setrandPosX();		// reroll it
			w.setrandPosY();

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
			a.setrandPosX();				// reroll it
			a.setrandPosY();

			toptile = m_maze[a.getPosY()*MAXCOLS + a.getPosX()].top;
		}
		m_monsters.emplace_back(new Archer(a));
		m_maze[a.getPosY()*MAXCOLS + a.getPosX()].top = ARCHER;
		m_maze[a.getPosY()*MAXCOLS + a.getPosX()].enemy = true;
		m--;
	}

	/*
	int numwall = 250 + randInt(100); // number of walls to generate on top
	int randX = randInt(68) + 1, randY = randInt(16) + 1;

	while (numwall > 0) {
		toptile = m_maze[randY][randX].top;
		bottomtile = m_maze[randY][randX].bottom;
		while (toptile != ' ' || bottomtile != ' ') {
			randX = randInt(68) + 1;
			randY = randInt(16) + 1;
			toptile = m_maze[randY][randX].top;
			bottomtile = m_maze[randY][randX].bottom;
		}
		m_maze[randY][randX].top = m_maze[randY][randX].bottom = '#';
		numwall--;
	}*/
}
Dungeon::~Dungeon() {
	
}

void Dungeon::peekDungeon(int x, int y, char move) {
	char top, bottom;

	//move player
	if (move == 'l') { // moved to the left
		top = m_maze[y*MAXCOLS + x - 1].top;
		bottom = m_maze[y*MAXCOLS + x - 1].bottom;

		if (top == SPACE && bottom == ' ') {
			// move character to the left
			m_maze[y*MAXCOLS + x - 1].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosX(x-1);
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y*MAXCOLS + x - 1].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosX(x - 1);

			foundItem(m_maze, MAXCOLS, x - 1, y);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {	// monster encountered
			fight(x-1, y);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_maze[y*MAXCOLS + x - 1].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosX(x - 1);

			dungeonText.push_back("It's the idol!\n");
		}
	}

	if (move == 'r') { // moved to the right
		top = m_maze[y*MAXCOLS + x + 1].top;
		bottom = m_maze[y*MAXCOLS + x + 1].bottom;

		if (top == SPACE && bottom == SPACE) {
			// move character to the left
			m_maze[y*MAXCOLS + x + 1].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosX(x + 1);
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y*MAXCOLS + x + 1].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosX(x + 1);

			foundItem(m_maze, MAXCOLS, x + 1, y);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			fight(x+1, y);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_maze[y*MAXCOLS + x + 1].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosX(x + 1);

			dungeonText.push_back("It's the idol!\n");
		}
	}

	if (move == 'u') { // moved up
		top = m_maze[(y-1)*MAXCOLS + x].top;
		bottom = m_maze[(y-1)*MAXCOLS + x].bottom;

		if (top == SPACE && bottom == SPACE) {
			m_maze[(y - 1)*MAXCOLS + x].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosY(y - 1);
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_maze[(y-1)*MAXCOLS + x].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosY(y - 1);

			foundItem(m_maze, MAXCOLS, x, y - 1);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			fight(x, y-1);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_maze[(y-1)*MAXCOLS + x].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosY(y - 1);

			dungeonText.push_back("It's the idol!\n");
		}
	}

	if (move == 'd') { // moved down
		top = m_maze[(y+1)*MAXCOLS + x].top;
		bottom = m_maze[(y+1)*MAXCOLS + x].bottom;

		if (top == SPACE && bottom == SPACE) {
			m_maze[(y+1)*MAXCOLS + x].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosY(y + 1);
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_maze[(y+1)*MAXCOLS + x].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosY(y + 1);

			foundItem(m_maze, MAXCOLS, x, y + 1);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			fight(x, y+1);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_maze[(y+1)*MAXCOLS + x].top = PLAYER;
			m_maze[y*MAXCOLS + x].top = SPACE;
			player.at(0).setPosY(y + 1);

			dungeonText.push_back("It's the idol!\n");
		}
	}

	if (move == 'g') {
		collectItem(m_maze, MAXCOLS, x, y);

		if (m_level > 1)
			return;
	}

	if (move == 'w') {
		showDungeon();
		player.at(0).wield(dungeonText);
	}

	if (move == 'c') {
		showDungeon();
		player.at(0).use(m_actives, m_maze[y*MAXCOLS + x], dungeonText);
	}
	
	
	// check active items in play
	checkActive(m_maze, MAXROWS, MAXCOLS, m_actives, m_monsters);
	

	// if all monsters are dead?


	// move monsters
	int g_x, g_y, shortest;
	char first, best, prev;

	for (unsigned i = 0; i < m_monsters.size(); i++) {
		g_x = m_monsters.at(i)->getPosX();
		g_y = m_monsters.at(i)->getPosY();
		first = '0'; best = '0'; prev = '0';
		shortest = 0;

		if (m_monsters.at(i)->getName() == "goblin") {
			//	allows use of goblin's smell distance function
			shared_ptr<Goblin> goblin = dynamic_pointer_cast<Goblin>(m_monsters.at(i));

			if (goblinInRange(i)) {
				if (moveGoblins(i, shortest, goblin->getSmellDistance(), goblin->getSmellDistance(), g_x, g_y, first, best, prev)) { // viable path is found
					switch (best) {
					case 'l':
						m_maze[g_y*MAXCOLS + g_x - 1].top = GOBLIN;
						m_maze[g_y*MAXCOLS + g_x - 1].enemy = true;
						m_maze[g_y*MAXCOLS + g_x].top = SPACE;
						m_maze[g_y*MAXCOLS + g_x].enemy = false;
						m_monsters.at(i)->setPosX(g_x - 1);
						break;
					case 'r':
						m_maze[g_y*MAXCOLS + g_x + 1].top = GOBLIN;
						m_maze[g_y*MAXCOLS + g_x + 1].enemy = true;
						m_maze[g_y*MAXCOLS + g_x].top = SPACE;
						m_maze[g_y*MAXCOLS + g_x].enemy = false;
						m_monsters.at(i)->setPosX(g_x + 1);
						break;
					case 'd':
						m_maze[(g_y + 1)*MAXCOLS + g_x].top = GOBLIN;
						m_maze[(g_y + 1)*MAXCOLS + g_x].enemy = true;
						m_maze[g_y*MAXCOLS + g_x].top = SPACE;
						m_maze[g_y*MAXCOLS + g_x].enemy = false;
						m_monsters.at(i)->setPosY(g_y + 1);
						break;
					case 'u':
						m_maze[(g_y - 1)*MAXCOLS + g_x].top = GOBLIN;
						m_maze[(g_y - 1)*MAXCOLS + g_x].enemy = true;
						m_maze[g_y*MAXCOLS + g_x].top = SPACE;
						m_maze[g_y*MAXCOLS + g_x].enemy = false;
						m_monsters.at(i)->setPosY(g_y - 1);
						break;
					default:
						m_monsters.at(i)->encounter(player.at(0), *(m_monsters.at(i)), dungeonText);
						if (player.at(0).getHP() <= 0) {
							m_maze[y*MAXCOLS + x].top = 'X'; // change player icon to X to indicate death
							player.at(0).setDeath(m_monsters.at(i)->getName());
						}
						break;
					}
				}
			}
		}

		else if (m_monsters.at(i)->getName() == "wanderer") {
			int n = -1 + randInt(3);
			int m = -1 + randInt(3);

			if (m_maze[(g_y + m)*MAXCOLS + g_x + n].top != WALL && m_maze[(g_y + m)*MAXCOLS + g_x + n].top != UNBREAKABLE_WALL && !m_maze[(g_y + m)*MAXCOLS + g_x + n].enemy) {
				if (m_maze[(g_y + m)*MAXCOLS + g_x + n].top != PLAYER) {
					m_maze[(g_y + m)*MAXCOLS + g_x + n].top = WANDERER;
					m_maze[(g_y + m)*MAXCOLS + g_x + n].enemy = true;
					m_maze[g_y*MAXCOLS + g_x].top = SPACE;
					m_maze[g_y*MAXCOLS + g_x].enemy = false;
					m_monsters.at(i)->setPosX(g_x + n);
					m_monsters.at(i)->setPosY(g_y + m);
				}
				else {
					m_monsters.at(i)->encounter(player.at(0), *(m_monsters.at(i)), dungeonText);
					if (player.at(0).getHP() <= 0) {
						m_maze[y*MAXCOLS + x].top = 'X'; //change player icon to X to indicate death
						player.at(0).setDeath(m_monsters.at(i)->getName());
					}
				}
			}
		}

		else if (m_monsters.at(i)->getName() == "archer") {
			checkArchers(m_maze, MAXCOLS, player.at(0).getPosX(), player.at(0).getPosY(), i, m_monsters);

			if (player.at(0).getHP() <= 0) {
				m_maze[y*MAXCOLS + x].top = 'X'; //change player icon to X to indicate death
				player.at(0).setDeath(m_monsters.at(i)->getName());
			}
		}

		//	if player is dead then break
		if (player.at(0).getDeath() != "")
			break;
	}


	// roll heal chance
	player.at(0).rollHeal(dungeonText);


	clearScreen();
	showDungeon();
	if (!dungeonText.empty())
		showText();
}
void Dungeon::checkActive(Tile dungeon[], int maxrows, int maxcols, vector<shared_ptr<Objects>> &actives, std::vector<std::shared_ptr<Monster>> &monsters) {
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
					// set i to pos because otherwise the iterator is destroyed
					pos = i;
					actives.erase(actives.begin() + pos);

					dungeonText.push_back("A bomb exploded!\n");
					explosion(dungeon, MAXCOLS, monsters, bomb->getPosX(), bomb->getPosY());
					dungeon[bomb->getPosY()*maxcols + bomb->getPosX()].bottom = SPACE;
				}
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
					//dungeon[y*maxcols + x].top = SPACE;
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
					//dungeon[y*maxcols + x].top = SPACE;
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
					//dungeon[y*maxcols + x].top = SPACE;
					//dungeon[y*maxcols + x].trap = false;
					break;
				default:
					break;
				}
				dungeon[y*maxcols + x].traptile = SPACE;
				dungeon[y*maxcols + x].trap = false;
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
					}
				}
			}
		}
	}
}
void Dungeon::explosion(Tile dungeon[], int maxcols, vector<shared_ptr<Monster>> &monsters, int x, int y) { // bomb's coordinates
	int mx, my;
	for (unsigned i = 0; i < monsters.size(); i++) {
		mx = monsters.at(i)->getPosX();
		my = monsters.at(i)->getPosY();
		if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			monsters.at(i)->setHP(monsters.at(i)->getHP() - 10);
			//cout << "The " << monsters.at(i)->getName() << " was blown up!\n";
			dungeonText.push_back("The " + monsters.at(i)->getName() + " was blown up!\n");

			if (monsters.at(i)->getHP() <= 0) {
				monsterDeath(m_maze, MAXCOLS, m_monsters, mx, my, i);
			}
		}
	}
	
	//	destroy any nearby walls
	for (int j = y - 1; j < y + 2; j++) {
		for (int k = x - 1; k < x + 2; k++) {
			if (dungeon[j*maxcols + k].top == WALL) {
				dungeon[j*maxcols + k].top = SPACE;
				dungeon[j*maxcols + k].bottom = SPACE;
			}
		}
	}
}
void Dungeon::trapEncounter(Tile dungeon[], int maxrows, int maxcols, vector<shared_ptr<Objects>> &actives, int x, int y) { // trap's coordinates
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

		dungeonText.push_back("A falling spike hit you for " + to_string(spike->getDmg()) + "!\n");
		player.at(0).setHP(player.at(0).getHP() - spike->getDmg());

		actives.erase(actives.begin() + i);
	}
	else if (trap == "spike trap") {
		shared_ptr<SpikeTrap> spiketrap = dynamic_pointer_cast<SpikeTrap>(actives.at(i));

		if (dungeon[y*maxcols + x].traptile == SPIKETRAP_ACTIVE) {
			dungeonText.push_back("You step on a spike trap and take " + to_string(spiketrap->getDmg()) + " damage!\n");
			player.at(0).setHP(player.at(0).getHP() - spiketrap->getDmg());
		}
	}
}
void Dungeon::monsterDeath(Tile dungeon[], int maxcols, vector<shared_ptr<Monster>> &monsters, int x, int y, int pos) {
	string monster = monsters.at(pos)->getName();
	//cout << "The " << monster << " was slain.\n" << endl;
	dungeonText.push_back("The " + monster + " was slain.\n");

	if (monster == "goblin") {
		if (randInt(3) + 1 == 3) {
			BoneAxe(x, y);
			dungeon[y*maxcols + x].bottom = BONEAXE;
		}
	}
	else if (monster == "wanderer") {
		if (randInt(1) + 1 == 1) {
			HeartPod(x, y);
			dungeon[y*maxcols + x].bottom = HEART_POD;
		}
	}
	else if (monster == "archer") {

	}
	dungeon[y*maxcols + x].top = SPACE;
	dungeon[y*maxcols + x].enemy = false;

	//delete monsters.at(i);
	monsters.erase(monsters.begin() + pos);
}

bool Dungeon::moveGoblins(int pos, int &shortest, int smelldist, int origdist, int x, int y, \
	char &first_move, char &optimal_move, char prev) {
	
	if (smelldist == origdist && \
		(m_maze[(y - 1)*MAXCOLS + x].top == PLAYER || m_maze[(y + 1)*MAXCOLS + x].top == PLAYER || m_maze[y*MAXCOLS + x - 1].top == PLAYER || m_maze[y*MAXCOLS + x + 1].top == PLAYER)) {
		return true; //	if player is immediately adjacent to the goblin, return to attack
	}
	
	if (smelldist < 0) // player was not found within origdist # of steps
		return false;

	if (m_maze[y*MAXCOLS + x].top == PLAYER) {
		if (smelldist > shortest) {
			shortest = smelldist;
			optimal_move = first_move;
		}
		if (smelldist == shortest) {
			// randomly select optimal path if multiple optimal paths found
			if (randInt(2) == 1)
				optimal_move = first_move;
		}
		return true; // if player is found, tell goblin to advance
	}

	if (prev != 'l' && (m_maze[y*MAXCOLS + x+1].top != WALL && m_maze[y*MAXCOLS + x+1].top != UNBREAKABLE_WALL) && !m_maze[y*MAXCOLS + x+1].enemy) {
		if (smelldist == origdist)
			first_move = 'r';
		moveGoblins(pos, shortest, smelldist - 1, origdist, x + 1, y, first_move, optimal_move, 'r');
	}
	
	if (prev != 'r' && (m_maze[y*MAXCOLS + x-1].top != WALL && m_maze[y*MAXCOLS + x-1].top != UNBREAKABLE_WALL) && !m_maze[y*MAXCOLS + x-1].enemy) {
		if (smelldist == origdist)
			first_move = 'l';
		moveGoblins(pos, shortest, smelldist - 1, origdist, x - 1, y, first_move, optimal_move, 'l');
	}

	if (prev != 'u' && (m_maze[(y+1)*MAXCOLS + x].top != WALL && m_maze[(y+1)*MAXCOLS + x].top != UNBREAKABLE_WALL) && !m_maze[(y+1)*MAXCOLS + x].enemy) {
		if (smelldist == origdist)
			first_move = 'd';
		moveGoblins(pos, shortest, smelldist - 1, origdist, x, y + 1, first_move, optimal_move, 'd');
	}

	if (prev != 'd' && (m_maze[(y-1)*MAXCOLS + x].top != WALL && m_maze[(y-1)*MAXCOLS + x].top != UNBREAKABLE_WALL) && !m_maze[(y-1)*MAXCOLS + x].enemy) {
		if (smelldist == origdist)
			first_move = 'u';
		moveGoblins(pos, shortest, smelldist - 1, origdist, x, y - 1, first_move, optimal_move, 'u');
	}

	if (optimal_move != '0')
		return true;
	return false;
}
bool Dungeon::goblinInRange(int pos) {
	int px = player.at(0).getPosX();
	int gx = m_monsters.at(pos)->getPosX();

	int py = player.at(0).getPosY();
	int gy = m_monsters.at(pos)->getPosY();

	if (abs(px - gx) > 15 || abs(py - gy) > 15) // if goblin and player are too far separated, then do not attempt to recurse
		return false;

	int steps = 0;

	while (px != gx) {
		if (px > gx)
			gx++;
		else
			gx--;

		steps++;
	}

	while (py != gy) {
		if (py > gy)
			gy++;
		else
			gy--;

		steps++;
	}

	return (steps <= 15 ? true : false);
}

void Dungeon::checkArchers(Tile dungeon[], int maxcols, int x, int y, int pos, vector<shared_ptr<Monster>> &monsters) {
	shared_ptr<Archer> archer = dynamic_pointer_cast<Archer>(monsters.at(pos));
	int mx = monsters.at(pos)->getPosX();
	int my = monsters.at(pos)->getPosY();

	//	if archer is primed, check to attack and return
	if (archer->isPrimed()) {
		if (x - mx == 0 || y - my == 0)
			monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)), dungeonText);
		else
			//cout << "The archer eases their aim.\n";
			dungeonText.push_back("The archer eases their aim.\n");

		archer->prime(false);
		monsters.at(pos) = archer;

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
		//cout << "The archer switches back to their Wood Bow.\n";
		dungeonText.push_back("The archer switches back to their Wood Bow.\n");
		return;
	}

	//if player is not in the archer's sights, move archer
	if (x - mx != 0 && y - my != 0) {
		moveArchers(dungeon, maxcols, mx, my, pos, monsters);
		return;
	}

	// if player and archer are on the same column
	if (x - mx == 0) {		
		//	if player is within range and not adjacent, get primed
		if (abs(y - my) > 1 && abs(y - my) <= 15) {
			if (!wallCollision(dungeon, maxcols, 'y', y, my)) {
				archer->prime(true);
				monsters.at(pos) = archer;

				dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveArchers(dungeon, maxcols, mx, my, pos, monsters);
				return;
			}
		}
		else if (abs(y - my) == 1) {
			//swap weapon to dagger
			monsters.at(pos)->setWeapon(BronzeDagger());
			dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
			//monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)));
		}
		//	just move archer
		else {
			moveArchers(dungeon, maxcols, mx, my, pos, monsters);
			return;
		}
	}
	// if player and archer are on the same row
	else if (y - my == 0) {
		//	if player is within range and not adjacent, get primed
		if (abs(x - mx) > 1 && abs(x - mx) <= 15) {
			if (!wallCollision(dungeon, maxcols, 'x', x, mx)) {
				archer->prime(true);
				monsters.at(pos) = archer;

				dungeonText.push_back("The archer is primed to shoot!\n");
			}
			else {
				moveArchers(dungeon, maxcols, mx, my, pos, monsters);
				return;
			}
		}
		else if (abs(x - mx) == 1) {
			// swap weapon to dagger
			monsters.at(pos)->setWeapon(BronzeDagger());
			dungeonText.push_back("The archer switches to their Bronze Dagger.\n");
			//monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)));
		}
		// just move archer
		else {
			moveArchers(dungeon, maxcols, mx, my, pos, monsters);
		}
	}

}
void Dungeon::moveArchers(Tile dungeon[], int maxcols, int mx, int my, int pos, vector<shared_ptr<Monster>> &monsters) {
	int n = randInt(2);
	int m = 0;
	char move;

	if (dungeon[my*maxcols + mx + 1].top != SPACE && dungeon[my*maxcols + mx - 1].top != SPACE && dungeon[(my + 1)*maxcols + mx].top != SPACE && dungeon[(my - 1)*maxcols + mx].top != SPACE) {
		//  if archer is surrounded completely, do nothing (such as goblin or wanderer blocking an archer's path)
		return;
	}
	else if (dungeon[my*maxcols + mx + 1].top != SPACE && dungeon[my*maxcols + mx - 1].top != SPACE) {
		//	if archer is surrounded by walls on left and right
		move = 'v';
	}
	else if (dungeon[(my + 1)*maxcols + mx].top != SPACE && dungeon[(my - 1)*maxcols + mx].top != SPACE) {
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
		while (n == 0 || dungeon[my*maxcols + mx + n].top != SPACE) {
			n = -1 + randInt(3);
		}

		dungeon[my*maxcols + mx + n].top = ARCHER;
		dungeon[my*maxcols + mx + n].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		monsters.at(pos)->setPosX(mx + n);
	}
	else if (move == 'v') {
		//	move is vertical
		m = -1 + randInt(3);
		while (m == 0 || dungeon[(my + m)*maxcols + mx].top != SPACE) {
			m = -1 + randInt(3);
		}

		dungeon[(my + m)*maxcols + mx].top = ARCHER;
		dungeon[(my + m)*maxcols + mx].enemy = true;
		dungeon[my*maxcols + mx].top = SPACE;
		dungeon[my*maxcols + mx].enemy = false;
		monsters.at(pos)->setPosY(my + m);
	}

}
bool Dungeon::wallCollision(Tile dungeon[], int maxcols, char direction, int p_move, int m_move) {
	int otherdir;

	// Since player and monster are on the same row/column, set otherdir to the complement of p_move/m_move
	if (direction == 'x') {
		otherdir = player.at(0).getPosY();
	}
	else {
		otherdir = player.at(0).getPosX();
	}

	if (p_move > m_move) {
		while (p_move - m_move != 0) {
			if (direction == 'x') {
				if (dungeon[otherdir*maxcols + p_move - 1].top == WALL || dungeon[otherdir*maxcols + p_move - 1].top == UNBREAKABLE_WALL)
					return true;
				else
					p_move--;
			}
			else if (direction == 'y') {
				if (dungeon[(p_move - 1)*maxcols + otherdir].top == WALL || dungeon[(p_move - 1)*maxcols + otherdir].top == UNBREAKABLE_WALL)
					return true;
				else
					p_move--;
			}
		}
	}
	else {
		while (m_move - p_move != 0) {
			if (direction == 'x') {
				if (dungeon[otherdir*maxcols + m_move - 1].top == WALL || dungeon[otherdir*maxcols + m_move - 1].top == UNBREAKABLE_WALL)
					return true;
				else
					m_move--;
			}
			else if (direction == 'y') {
				if (dungeon[(m_move - 1)*maxcols + otherdir].top == WALL || dungeon[(m_move - 1)*maxcols + otherdir].top == UNBREAKABLE_WALL)
					return true;
				else
					m_move--;
			}
		}
	}
	return false;
}

void Dungeon::foundItem(Tile dungeon[], int maxcols, int x, int y) {
	char c = dungeon[y*maxcols + x].bottom;

	//cout << "You've found ";
	dungeonText.push_back("You've found ");
	switch (c) {
	case CUTLASS:
		//cout << "a Rusty Cutlass.\n" << endl;
		dungeonText.push_back("a Rusty Cutlass.\n");
		break;
	case BONEAXE:
		//cout << "a Bone Axe.\n" << endl;
		dungeonText.push_back("a Bone Axe.\n");
		break;
	case HEART_POD:
		//cout << "a Heart Pod.\n" << endl;
		dungeonText.push_back("a Heart Pod.\n");
		break;
	case LIFEPOT:
		//cout << "a Life Potion.\n" << endl;
		dungeonText.push_back("a Life Potion.\n");
		break;
	case ARMOR:
		//cout << "some extra Armor.\n" << endl;
		dungeonText.push_back("some extra Armor.\n");
		break;
	case STATPOT:
		//cout << "a Stat Potion.\n" << endl;
		dungeonText.push_back("a Stat Potion.\n");
		break;
	case BOMB:
		//cout << "a Bomb!\n" << endl;
		dungeonText.push_back("a Bomb!\n");
		break;
	case BROWN_CHEST:
		//cout << "a Brown Chest!\n" << endl;
		dungeonText.push_back("a Brown Chest!\n");
		break;
	case SILVER_CHEST:
		//cout << "a Silver Chest!\n" << endl;
		dungeonText.push_back("a Silver Chest!\n");
		break;
	case GOLDEN_CHEST:
		//cout << "a Golden Chest!\n" << endl;
		dungeonText.push_back("a Golden Chest!\n");
		break;
	case STAIRS:
		//cout << "stairs!\n" << endl;
		dungeonText.push_back("stairs!\n");
		break;
	default:
		break;
	}
}
void Dungeon::collectItem(Tile dungeon[], int maxcols, int x, int y) {
	if (dungeon[y*maxcols + x].bottom == CUTLASS) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addWeapon(RustyCutlass(x, y));
			dungeon[y*maxcols + x].bottom = SPACE;

			//showDungeon();
			//cout << "You pick up the Rusty Cutlass.\n" << endl;
			dungeonText.push_back("You pick up the Rusty Cutlass.\n");
		}
		else {
			//cout << "Your backpack is full!\n" << endl;
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (dungeon[y*maxcols + x].bottom == BONEAXE) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addWeapon(BoneAxe(x,y)); //adds bone axe to inventory
			dungeon[y*maxcols + x].bottom = SPACE;

			//showDungeon();
			//cout << "You pick up the Bone Axe.\n" << endl;
			dungeonText.push_back("You pick up the Bone Axe.\n");
		}
		else {
			//cout << "Your backpack is full!\n" << endl;
			dungeonText.push_back("Your backpack is full!\n");
		}
	}
	else if (dungeon[y*maxcols + x].bottom == IDOL) {
		player.at(0).setWin(true);
		dungeon[y*maxcols + x].bottom = SPACE;
		//showDungeon();
	}
	else if (dungeon[y*maxcols + x].bottom == STAIRS) {
		setLevel(getLevel() + 1);
		clearScreen();
	}

	//		BEGIN DROPS IF STATEMENTS
	else if (dungeon[y*maxcols + x].bottom == HEART_POD) {
		HeartPod heart;

		dungeon[y*maxcols + x].bottom = SPACE;
		//showDungeon();
		heart.changeStats(heart, player.at(0));
	}
	else if (dungeon[y*maxcols + x].bottom == LIFEPOT) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addItem(LifePotion());
			dungeon[y*maxcols + x].bottom = SPACE;

			//showDungeon();
			//cout << "You grab the Life Potion!" << endl;
			dungeonText.push_back("You grab the Life Potion!\n");
		}
		else {
			//cout << "You can't hold any more items." << endl;
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (dungeon[y*maxcols + x].bottom == ARMOR) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addItem(ArmorDrop());
			dungeon[y*maxcols + x].bottom = SPACE;

			//showDungeon();
			//cout << "You grab the Armor!" << endl;
			dungeonText.push_back("You grab the Armor!\n");
		}
		else {
			//cout << "You can't hold any more items." << endl;
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (dungeon[y*maxcols + x].bottom == STATPOT) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addItem(StatPotion());
			dungeon[y*maxcols + x].bottom = SPACE;

			//showDungeon();
			//cout << "You grab the Stat Potion!" << endl;
			dungeonText.push_back("You grab the Stat Potion!\n");
		}
		else {
			//cout << "You can't hold any more items." << endl;
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (dungeon[y*maxcols + x].bottom == BOMB) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addItem(Bomb());
			dungeon[y*maxcols + x].bottom = SPACE;

			//showDungeon();
			//cout << "You grab the Bomb!" << endl;
			dungeonText.push_back("You grab the Bomb!\n");
		}
		else {
			//cout << "You can't hold any more items." << endl;
			dungeonText.push_back("You can't hold any more items.\n");
		}
	}
	else if (dungeon[y*maxcols + x].bottom == BROWN_CHEST) {
		BrownChest c;

		//showDungeon();
		//cout << "You open the Brown Chest... ";
		dungeonText.push_back("You open the Brown Chest... ");
		c.open(dungeon[y*maxcols + x], dungeonText);
	}
	else if (dungeon[y*maxcols + x].bottom == SILVER_CHEST) {
		SilverChest c;

		//showDungeon();
		//cout << "You open the Silver Chest... ";
		dungeonText.push_back("You open the Silver Chest... ");
		c.open(dungeon[y*maxcols + x], dungeonText);
	}
	else if (dungeon[y*maxcols + x].bottom == GOLDEN_CHEST) {
		GoldenChest c;

		//showDungeon();
		//cout << "You open the Golden Chest... ";
		dungeonText.push_back("You open the Golden Chest... ");
		c.open(dungeon[y*maxcols + x], dungeonText);
	}
	else {
		//showDungeon();
		//cout << "There's nothing in the dirt... \n" << endl;
		dungeonText.push_back("There's nothing in the dirt... \n");
	}
}
void Dungeon::showDungeon() {
	//	adjust window to be more centered
	cout << "\n\n\n\n";
	cout << "       ";

	for (int i = 0; i < MAXROWS; i++) {
		for (int j = 0; j < MAXCOLS; j++) {
			if (m_maze[i*MAXCOLS + j].top == SPACE)
				cout << m_maze[i*MAXCOLS + j].bottom;
			else
				cout << m_maze[i*MAXCOLS + j].top;
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
int Dungeon::getLevel() const {
	return m_level;
}
void Dungeon::setLevel(int level) {
	m_level = level;
}

void Dungeon::fight(int x, int y) { // monster's coordinates
	unsigned i = 0;
	bool found = false;
	while (!found && i < m_monsters.size()) { // finds the monster just encountered
		if (m_monsters.at(i)->getPosX() == x && m_monsters.at(i)->getPosY() == y)
			found = true;
		else
			i++;
	}
	string monster = m_monsters.at(i)->getName();
	player.at(0).attack(m_monsters, i, dungeonText);
	if (m_monsters.at(i)->getHP() <= 0) {
		monsterDeath(m_maze, MAXCOLS, m_monsters, x, y, i);
	}
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

vector<char> Dungeon::topChunks(vector<vector<vector<char>>> &c) {
	vector<vector<char>> one = { {'#', '#', '#', '#'},
								 {'#', '#', '#', '#'},
								 {' ', ' ', ' ', ' '},
								 {'#', '#', '#', '#'} };

	vector<vector<char>> two = { {'#', '#', '#', '#'},
								 {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', ' '},
								 {' ', ' ', ' ', ' '} };

	vector<vector<char>> three = { {' ', '#', '#', ' '},
								   {' ', '#', '#', ' '},
								   {' ', '#', '#', ' '},
								   {' ', ' ', ' ', ' '} };

	vector<vector<char>> four = { {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {'#', ' ', ' ', '#'} };

	vector<vector<char>> five = { {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', '#'} };

	vector<vector<char>> six = {  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	vector<vector<char>> seven = { {' ', ' ', '#', ' '},
								   {'#', ' ', ' ', ' '},
								   {' ', ' ', '#', ' '},
								   {'#', ' ', '#', ' '} };

	vector<vector<char>> eight = { {'#', '#', '#', '#'},
								   {' ', ' ', ' ', '#'},
								   {'#', ' ', ' ', ' '},
								   {'#', '#', '#', '#'} };

	vector<vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', ' '} };

	vector<vector<char>> ten = {  {'#', ' ', ' ', '#'},
								  {'#', ' ', ' ', '#'},
								  {'#', ' ', ' ', '#'},
								  {'#', ' ', ' ', '#'} };

	vector<vector<char>> eleven = { {'#', '#', '#', '#'},
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
	
	c = mixChunks(c);

	return combineChunks(c);
}
vector<char> Dungeon::middleChunks(vector<vector<vector<char>>> &c) {
	vector<vector<char>> one = { {'#', '#', '#', '#'},
								 {'#', ' ', ' ', ' '},
								 {' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', '#'} };

	vector<vector<char>> two = { {'#', ' ', ' ', '#'},
								 {' ', ' ', ' ', '#'},
								 {'#', ' ', ' ', ' '},
								 {'#', '#', '#', '#'} };

	vector<vector<char>> three = { {'#', ' ', '#', '#'},
								   {' ', ' ', ' ', '#'},
								   {' ', ' ', ' ', '#'},
								   {'#', ' ', '#', '#'} };

	vector<vector<char>> four = { {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {' ', ' ', ' ', '#'} };

	vector<vector<char>> five = { {'#', ' ', ' ', '#'},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {'#', ' ', ' ', '#'} };

	vector<vector<char>> six = { {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	vector<vector<char>> seven = { {' ', ' ', '#', '#'},
								   {' ', ' ', ' ', '#'},
								   {'#', '#', ' ', ' '},
								   {'#', '#', ' ', ' '} };

	vector<vector<char>> eight = { {'#', ' ', '#', '#'},
								  {'#', ' ', '#', '#'},
								  {' ', ' ', ' ', ' '},
								  {'#', ' ', '#', '#'} };

	vector<vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', ' '} };

	vector<vector<char>> ten =  { {' ', ' ', '#', ' '},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '} };

	vector<vector<char>> specialroom1 = { {'#', '#', '#', '#'},
										  {'#', 'c', 'c', '#'},
										  {'#', '+', '+', '#'},
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
	//c.push_back(specialroom1);
	//c.push_back();

	c = mixChunks(c);

	return combineChunks(c);
}
vector<char> Dungeon::bottomChunks(vector<vector<vector<char>>> &c) {
	vector<vector<char>> one = { {' ', ' ', ' ', ' '},
								 {' ', '#', '#', ' '},
								 {' ', '#', ' ', ' '},
								 {'#', '#', ' ', '#'} };

	vector<vector<char>> two = { {' ', ' ', ' ', ' '},
								 {'#', ' ', ' ', '#'},
								 {'#', ' ', ' ', '#'},
								 {'#', '#', '#', '#'} };

	vector<vector<char>> three = { {'#', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '},
								   {'#', '#', '#', '#'} };

	vector<vector<char>> four = { {'#', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', '#', ' '},
								  {' ', ' ', ' ', '#'} };

	vector<vector<char>> five = { {'#', ' ', ' ', '#'},
								  {'#', ' ', ' ', '#'},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', ' ', '#'} };

	vector<vector<char>> six = {  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	vector<vector<char>> seven = { {' ', ' ', ' ', ' '},
								   {' ', ' ', ' ', ' '},
								   {'#', ' ', ' ', ' '},
								   {'#', '#', ' ', ' '} };

	vector<vector<char>> eight = { {' ', ' ', '#', '#'},
								   {' ', ' ', '#', '#'},
								   {' ', '#', '#', '#'},
								   {' ', '#', '#', '#'} };

	vector<vector<char>> nine = { {' ', ' ', ' ', ' '},
								  {' ', '#', '#', ' '},
								  {' ', '#', '#', ' '},
								  {' ', ' ', ' ', ' '} };

	vector<vector<char>> ten = { {' ', ' ', '#', ' '},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', '#', ' '},
								  {'#', ' ', ' ', ' '} };

	vector<vector<char>> eleven = { {' ', ' ', ' ', '#'},
								    {' ', ' ', ' ', '#'},
								    {' ', '#', ' ', '#'},
								    {' ', '#', '#', '#'} };

	vector<vector<char>> specialroom1 = { {'#', '#', '#', '#'},
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
	c.push_back(specialroom1);

	c = mixChunks(c);

	return combineChunks(c);
}
vector<vector<vector<char>>> Dungeon::mixChunks(vector<vector<vector<char>>> c) {
	vector<vector<vector<char>>> v;

	for (int i = 0; i < 17; i++)
		v.push_back(c[randInt(c.size())]);

	return v;
}
vector<char> Dungeon::combineChunks(vector<vector<vector<char>>> &c) {
	vector<char> v;
	for (int i = 0; i < 4; i++) {			// iterate thru rows of 2d vector
		for (int j = 0; j < c.size(); j++) {	// iterate thru vector of 2d vectors
			for (int k = 0; k < 4; k++) {			// iterate thru columns of 2d vector
				v.push_back(c.at(j)[i][k]);
			}
		}
	}

	return v;
}


//	BEGIN BOSS FIGHT FUNCTIONS
FirstBoss::FirstBoss(Player p) {
	Tile *tile;
	int i, j;

	for (i = 0; i < BOSSROWS; i++) { //		initialize entire dungeon to blank space
		for (j = 0; j < BOSSCOLS; j++) {
			tile = new Tile;

			tile->top = tile->bottom = tile->traptile = SPACE;
			tile->enemy = tile->trap = false;

			m_boss[i*BOSSCOLS + j] = *tile;
		}
	}

	for (i = 0; i < BOSSROWS; i++) { //	initialize top and bottom of dungeon to be unbreakable walls
		for (j = 0; j < BOSSCOLS; j++) {
			m_boss[i*BOSSCOLS + j].top = UNBREAKABLE_WALL;
			m_boss[i*BOSSCOLS + j].bottom = UNBREAKABLE_WALL;
			m_boss[i*BOSSCOLS + j].traptile = UNBREAKABLE_WALL;
		}
		i += (BOSSROWS - 2);
	}

	for (i = 1; i < BOSSROWS - 1; i++) { //		initialize edges of dungeon to be unbreakable walls
		m_boss[i*BOSSCOLS].top = m_boss[i*BOSSCOLS].bottom = m_boss[i*BOSSCOLS].traptile = UNBREAKABLE_WALL;
		m_boss[i*BOSSCOLS + BOSSCOLS - 1].top = m_boss[i*BOSSCOLS + BOSSCOLS-1].bottom = m_boss[i*BOSSCOLS + BOSSCOLS - 1].traptile = UNBREAKABLE_WALL;
	}


	//	spawn player
	player.at(0) = p;
	player.at(0).setPosX(BOSSCOLS / 2);
	player.at(0).setPosY(BOSSROWS - 3);
	m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;


	//	spawn boss
	Smasher smasher;
	int col = smasher.getPosX();
	int row = smasher.getPosY();
	for (unsigned i = row - 1; i < row + 2; i++) {
		for (unsigned j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].top = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
	}
	m_firstbossMonsters.emplace_back(new Smasher(smasher));


	//	spawn spike traps
	int x = 1 + randInt(BOSSCOLS - 2);
	int y = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
	int speed = 3 + randInt(3);
	char traptile;

	int m = 12 + randInt(5);
	while (m > 0) {
		SpikeTrap spiketrap;

		//top = m_boss[spiketrap.getPosY()*MAXCOLS + spiketrap.getPosX()].top;
		traptile = m_boss[spiketrap.getPosY()*BOSSCOLS + spiketrap.getPosX()].traptile;
		while (traptile != SPACE) { // while spiketrap position clashes with wall, player, or idol
			x = 1 + randInt(BOSSCOLS - 2);
			y = BOSSROWS / 2 + randInt(BOSSROWS / 2 - 2);
			spiketrap.setPosX(x);				// reroll it
			spiketrap.setPosY(y);

			traptile = m_boss[spiketrap.getPosY()*BOSSCOLS + spiketrap.getPosX()].traptile;
		}
		m_firstbossActives.emplace_back(new SpikeTrap(spiketrap));
		m_boss[spiketrap.getPosY()*BOSSCOLS + spiketrap.getPosX()].traptile = SPIKETRAP_DEACTIVE;
		m_boss[spiketrap.getPosY()*BOSSCOLS + spiketrap.getPosX()].trap = true;
		m--;
	}
}

void FirstBoss::peekFirstBossDungeon(int x, int y, char move) {
	char top, bottom, traptile;
	bool trap, enemy;

	//move player
	if (move == 'l') { // moved to the left
		top = m_boss[y*BOSSCOLS + x - 1].top;
		bottom = m_boss[y*BOSSCOLS + x - 1].bottom;
		enemy = m_boss[y*BOSSCOLS + x - 1].enemy;
		trap = m_boss[y*BOSSCOLS + x - 1].trap;
		traptile = m_boss[y*BOSSCOLS + x - 1].traptile;

		
		if (trap) {
			if (traptile == SPIKE) {
				trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x - 1, y);
				m_boss[y*BOSSCOLS + x - 1].top = PLAYER;
				m_boss[y*BOSSCOLS + x - 1].traptile = SPACE;
				m_boss[y*BOSSCOLS + x - 1].trap = false;
				m_boss[y*BOSSCOLS + x].top = SPACE;
				player.at(0).setPosX(x - 1);

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
		if (top == SPACE && bottom == SPACE) {
			// move character to the left
			m_boss[y*BOSSCOLS + x - 1].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosX(x - 1);
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_boss[y*BOSSCOLS + x - 1].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosX(x - 1);

			foundItem(m_boss, BOSSCOLS, x - 1, y);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {	// monster encountered
			if (enemy) {
				if (top == SMASHER) {
					player.at(0).attack(m_firstbossMonsters, 0, dungeonText);
					if (m_firstbossMonsters.at(0)->getHP() <= 0)
						monsterDeath(x - 1, y, 0);
				}
				else
					fight(x - 1, y);
			}
			
			//else
				//fight(x - 1, y);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_boss[y*BOSSCOLS + x - 1].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosX(x - 1);

			dungeonText.push_back("It's the idol!\n");
		}
	}

	if (move == 'r') { // moved to the right
		top = m_boss[y*BOSSCOLS + x + 1].top;
		bottom = m_boss[y*BOSSCOLS + x + 1].bottom;
		enemy = m_boss[y*BOSSCOLS + x + 1].enemy;
		trap = m_boss[y*BOSSCOLS + x + 1].trap;
		traptile = m_boss[y*BOSSCOLS + x + 1].traptile;

		if (trap) {
			if (traptile == SPIKE) {
				trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x + 1, y);
				m_boss[y*BOSSCOLS + x + 1].top = PLAYER;
				m_boss[y*BOSSCOLS + x+1].traptile = SPACE;
				m_boss[y*BOSSCOLS + x+1].trap = false;
				m_boss[y*BOSSCOLS + x].top = SPACE;
				player.at(0).setPosX(x + 1);

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
		if (top == SPACE && bottom == SPACE) {
			// move character to the left
			m_boss[y*BOSSCOLS + x + 1].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosX(x + 1);
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_boss[y*BOSSCOLS + x + 1].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosX(x + 1);

			foundItem(m_boss, BOSSCOLS, x + 1, y);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			if (enemy) {
				if (top == SMASHER) {
					player.at(0).attack(m_firstbossMonsters, 0, dungeonText);
					if (m_firstbossMonsters.at(0)->getHP() <= 0)
						monsterDeath(x + 1, y, 0);
				}
				else
					fight(x + 1, y);
			}
			
			//else
				//fight(x + 1, y);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_boss[y*BOSSCOLS + x + 1].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosX(x + 1);

			dungeonText.push_back("It's the idol!\n");
		}
	}

	if (move == 'u') { // moved up
		top = m_boss[(y - 1)*BOSSCOLS + x].top;
		bottom = m_boss[(y - 1)*BOSSCOLS + x].bottom;
		enemy = m_boss[(y-1)*BOSSCOLS + x].enemy;
		trap = m_boss[(y - 1)*BOSSCOLS + x].trap;
		traptile = m_boss[(y - 1)*BOSSCOLS + x].traptile;

		if (trap) {
			if (traptile == SPIKE) {
				trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x, y - 1);
				m_boss[(y - 1)*BOSSCOLS + x].top = PLAYER;
				m_boss[(y - 1)*BOSSCOLS + x].traptile = SPACE;
				m_boss[(y - 1)*BOSSCOLS + x].trap = false;
				m_boss[y*BOSSCOLS + x].top = SPACE;
				player.at(0).setPosY(y - 1);

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
		if (top == SPACE && bottom == SPACE) {
			m_boss[(y - 1)*BOSSCOLS + x].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosY(y - 1);
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_boss[(y - 1)*BOSSCOLS + x].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosY(y - 1);

			foundItem(m_boss, BOSSCOLS, x, y - 1);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			if (enemy) {
				if (top == SMASHER) {
					player.at(0).attack(m_firstbossMonsters, 0, dungeonText);
					if (m_firstbossMonsters.at(0)->getHP() <= 0)
						monsterDeath(x, y - 1, 0);
				}
				else
					fight(x, y - 1);
			}
			
			//else
				//fight(x, y - 1);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_boss[(y - 1)*BOSSCOLS + x].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosY(y - 1);

			dungeonText.push_back("It's the idol!\n");
		}
	}

	if (move == 'd') { // moved down
		top = m_boss[(y + 1)*BOSSCOLS + x].top;
		bottom = m_boss[(y + 1)*BOSSCOLS + x].bottom;
		enemy = m_boss[(y + 1)*BOSSCOLS + x].enemy;
		trap = m_boss[(y + 1)*BOSSCOLS + x].trap;
		traptile = m_boss[(y + 1)*BOSSCOLS + x].traptile;

		if (trap) {
			if (traptile == SPIKE) {
				trapEncounter(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, x, y + 1);
				m_boss[(y + 1)*BOSSCOLS + x].top = PLAYER;
				m_boss[(y + 1)*BOSSCOLS + x].traptile = SPACE;
				m_boss[(y + 1)*BOSSCOLS + x].trap = false;
				m_boss[y*BOSSCOLS + x].top = SPACE;
				player.at(0).setPosY(y + 1);

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
		if (top == SPACE && bottom == SPACE) {
			m_boss[(y + 1)*BOSSCOLS + x].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosY(y + 1);
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_boss[(y + 1)*BOSSCOLS + x].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosY(y + 1);

			foundItem(m_boss, BOSSCOLS, x, y + 1);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			if (enemy) {
				if (top == SMASHER) {
					player.at(0).attack(m_firstbossMonsters, 0, dungeonText);
					if (m_firstbossMonsters.at(0)->getHP() <= 0)
						monsterDeath(x, y + 1, 0);
				}
				else
					fight(x, y + 1);
			}
			
			//else
				//fight(x, y + 1);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_boss[(y + 1)*BOSSCOLS + x].top = PLAYER;
			m_boss[y*BOSSCOLS + x].top = SPACE;
			player.at(0).setPosY(y + 1);

			dungeonText.push_back("It's the idol!\n");
		}
	}

	if (move == 'g') {
		collectItem(m_boss, BOSSCOLS, x, y);
	}

	if (move == 'w') {
		showDungeon();
		player.at(0).wield(dungeonText);
	}

	if (move == 'c') {
		showDungeon();
		player.at(0).use(m_firstbossActives, m_boss[y*BOSSCOLS + x], dungeonText);
	}


	// check active items in play
	checkActive(m_boss, BOSSROWS, BOSSCOLS, m_firstbossActives, m_firstbossMonsters);
	if (player.at(0).getHP() <= 0) {
		clearScreen();
		showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}

	//	check if smasher is dead
	if (m_firstbossMonsters.at(0)->getName() != "Smasher") {
		clearScreen();
		showDungeon();
		if (!dungeonText.empty())
			showText();
		return;
	}

	//	check if smasher is currently executing a move
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	if (smasher->isActive()) {
		switch (smasher->getMove()) {
		case 1:
			move1();
			break;
		case 2:
			move2();
			break;
		case 3:
			move3();
			break;
		case 4:
			move4();
			break;
		case 5:
			move5();
			break;
		default:
			break;
		}
	}
	else if (!smasher->isEnded()) {
		switch (smasher->getMove()) {
		case 1:
			resetUpward();
			break;
		case 2:
			resetDownward();
			break;
		case 3:
			
			break;
		case 4:
			
			break;
		case 5:
			
			break;
		default:
			break;
		}
	}
	else {
		checkSmasher(x, y);
	}

	if (player.at(0).getHP() <= 0) {
		//m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = 'X'; //change player icon to X to indicate death
		player.at(0).setDeath(m_firstbossMonsters.at(0)->getName());
		return;
	}

	// move any monsters
	if (m_firstbossMonsters.size() > 1) {
		for (unsigned i = 1; i < m_firstbossMonsters.size(); i++) {
			if (m_firstbossMonsters.at(i)->getName() == "archer") {
				checkArchers(m_boss, BOSSCOLS, player.at(0).getPosX(), player.at(0).getPosY(), i, m_firstbossMonsters);

				if (player.at(0).getHP() <= 0) {
					m_boss[y*BOSSCOLS + x].top = 'X'; //change player icon to X to indicate death
					player.at(0).setDeath(m_firstbossMonsters.at(i)->getName());
				}
			}

			//	if player is dead then break
			if (player.at(0).getDeath() != "")
				break;
		}
	}


	clearScreen();
	showDungeon();
	if (!dungeonText.empty())
		showText();
}
void FirstBoss::monsterDeath(int x, int y, int pos) {
	string boss = m_firstbossMonsters.at(pos)->getName();
	dungeonText.push_back(boss + " was slain!\n");
	dungeonText.push_back("Well done!\n");

	Idol idol;
	idol.setPosX(BOSSCOLS / 2);
	idol.setPosY(3);
	m_boss[idol.getPosY()*BOSSCOLS + idol.getPosX()].bottom = IDOL;

	x = m_firstbossMonsters.at(pos)->getPosX();
	y = m_firstbossMonsters.at(pos)->getPosY();
	for (unsigned i = y - 1; i < y + 2; i++) {
		for (unsigned j = x - 1; j < x + 2; j++) {
			m_boss[i*BOSSCOLS + j].top = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
		}
	}

	m_firstbossMonsters.erase(m_firstbossMonsters.begin() + pos);
}

void FirstBoss::checkSmasher(int x, int y) {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int randmove = 1+randInt(6);
	
	
	if (randmove == 1) {
		smasher->setActive(true);
		smasher->setMove(1);
		m_firstbossMonsters.at(0) = smasher;
		move1();
	}
	else if (randmove == 2) {
		smasher->setActive(true);
		smasher->setMove(2);
		m_firstbossMonsters.at(0) = smasher;
		move2();
	}
	else if (randmove == 3) {
		//smasher->setActive(true);
		//smasher->setMove(3);
		//m_firstbossMonsters.at(0) = smasher;
		move3();
	}
	else if (randmove == 0)
		move4();
	else if (randmove == 0)
		move5();
	else if (randmove > 3)
		moveSmasher();
}
void FirstBoss::move1() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	//	if wall immediately beneath smasher is an unbreakable wall, reset position to top
	if (m_boss[(row+2)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		smasher->setEnded(false);
		smasher->setActive(false);
		m_firstbossMonsters.at(0) = smasher;
		resetUpward();

		return;
	}

	// if there is only one space underneath the smasher
	if (m_boss[(row+3)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// top layer of smasher is set to spaces
		for (i = row - 1, j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].top = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
		}

		//	smasher is pushed to bottom layer
		for (i = row + 2, j = col - 1; j < col + 2; j++) {
			//	if player is in the way, crush them
			if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
				player.at(0).setPosY(BOSSROWS - 2);
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
			m_boss[i*BOSSCOLS + j].top = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
		m_firstbossMonsters.at(0)->setPosY(row + 1);

		//	if player is in the way, crush them
		/*for (i = BOSSROWS - 2, j = col - 1; j < col + 2; j++) {
			if (i*BOSSCOLS + j == player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()) {
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
		}*/

		return;
	}

	//	if there are only two spaces underneath the smasher
	if (m_boss[(row+4)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// top 2 layers of smasher is set to spaces
		for (i = row - 1; i < row + 1; i++) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].top = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	smasher is pushed to bottom layer
		for (i = row + 2; i < row + 4; i++) {
			for (j = col - 1; j < col + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
					player.at(0).setPosY(BOSSROWS - 2);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				m_boss[i*BOSSCOLS + j].top = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		m_firstbossMonsters.at(0)->setPosY(row + 2);

		//	if player is in the way, crush them
	/*	for (i = BOSSROWS - 2, j = col - 1; j < col + 2; j++) {
			if (i*BOSSCOLS + j == player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()) {
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
		}*/
		
		return;
	}

	//	if there are at least 3 spaces underneath the smasher
	if (m_boss[(row+4)*BOSSCOLS + col].top != UNBREAKABLE_WALL) {
		// top 2 layers of smasher is set to spaces
		for (i = row - 1; i < row + 1; i++) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].top = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	bottom two beneath smasher set to smasher
		for (i = row + 2; i < row + 4; i++) {
			for (j = col - 1; j < col + 2; j++) {
				// if player is in the way, move them below
				if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
					player.at(0).setPosY(row + 4);
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;
				}
				m_boss[i*BOSSCOLS + j].top = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		m_firstbossMonsters.at(0)->setPosY(row + 2);

	}
}
void FirstBoss::move2() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	int n = randInt(2);
	char move;

	move = 'v';
	//move = (n == 0 ? 'h' : 'v');

	//	move is vertical
	if (move == 'v') {
		//	if wall immediately above smasher is an unbreakable wall, make rocks fall
		if (m_boss[(row - 2)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
			smasher->setEnded(false);
			smasher->setActive(false);
			m_firstbossMonsters.at(0) = smasher;
			avalanche();
			resetDownward();

			return;
		}

		//  if there is only one space above the smasher
		else if (m_boss[(row - 3)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
			// bottom layer of smasher is set to spaces
			for (i = row + 1, j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].top = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
			
			//	smasher is pushed to upper layer
			for (i = row - 2, j = col - 1; j < col + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
					player.at(0).setPosY(row - 2);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				m_boss[i*BOSSCOLS + j].top = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
			m_firstbossMonsters.at(0)->setPosY(row - 1);

			return;
		}

		//	if there are only two spaces above the smasher
		else if (m_boss[(row - 4)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
			// bottom 2 layers of smasher is set to spaces
			for (i = row + 1; i > row - 1; i--) {
				for (j = col - 1; j < col + 2; j++) {
					m_boss[i*BOSSCOLS + j].top = SPACE;
					m_boss[i*BOSSCOLS + j].enemy = false;
				}
			}

			//	smasher is pushed to ceiling
			for (i = row - 2; i > row - 4; i--) {
				for (j = col - 1; j < col + 2; j++) {
					//	if player is in the way, crush them
					if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
						player.at(0).setPosY(row - 3);
						m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
					}
					m_boss[i*BOSSCOLS + j].top = SMASHER;
					m_boss[i*BOSSCOLS + j].enemy = true;

				}
			}
			m_firstbossMonsters.at(0)->setPosY(row - 2);

			return;
		}

		//	if there are at least 3 spaces above the smasher
		else if (m_boss[(row - 4)*BOSSCOLS + col].top != UNBREAKABLE_WALL) {
			// bottom 2 layers of smasher is set to spaces
			for (i = row + 1; i > row - 1; i--) {
				for (j = col - 1; j < col + 2; j++) {
					m_boss[i*BOSSCOLS + j].top = SPACE;
					m_boss[i*BOSSCOLS + j].enemy = false;
				}
			}

			//	top two layers above smasher set to smasher
			for (i = row - 2; i > row - 4; i--) {
				for (j = col - 1; j < col + 2; j++) {
					// if player is in the way, move them above
					if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
						player.at(0).setPosY(row - 4);
						m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;
					}
					m_boss[i*BOSSCOLS + j].top = SMASHER;
					m_boss[i*BOSSCOLS + j].enemy = true;
				}
			}
			m_firstbossMonsters.at(0)->setPosY(row - 2);

		}
	}
	else if (move == 'h') {
		//	move is horizontal

		/*m_boss[(y + m)*BOSSCOLS + x].top = ARCHER;
		m_boss[(y + m)*BOSSCOLS + x].enemy = true;
		m_boss[y*BOSSCOLS + x].top = SPACE;
		m_boss[y*BOSSCOLS + x].enemy = false;
		m_firstbossMonsters.at(0)->setPosY(y + m);*/
	}
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

		m_firstbossActives.emplace_back(new FallingSpike(spike));
		m_boss[spike.getPosY()*BOSSCOLS + spike.getPosX()].trap = true;
	}
}
void FirstBoss::move3() {	// spawn two archers
	int n = 2;
	int x, y;
	char top;

	if (m_firstbossMonsters.size() <= 2) {
		while (n > 0) {
			Archer archer;
			x = 1 + randInt(BOSSCOLS - 2);
			y = BOSSROWS - 2 - randInt(10);
			archer.setPosX(x);
			archer.setPosY(y);

			top = m_boss[y*BOSSCOLS + x].top;
			while (top != SPACE) {
				x = 1 + randInt(BOSSCOLS - 2);
				y = BOSSROWS - 2 - randInt(10);
				archer.setPosX(x);
				archer.setPosY(y);

				top = m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].top;
			}
			m_firstbossMonsters.emplace_back(new Archer(archer));
			m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].top = ARCHER;
			m_boss[archer.getPosY()*BOSSCOLS + archer.getPosX()].enemy = true;
			n--;
		}
	}

}
void FirstBoss::move4() {

}
void FirstBoss::move5() {

}

void FirstBoss::moveSmasher() {
	int n = randInt(4);

	switch(n) {
	case 0:
		moveSmasherL();
		break;
	case 1:
		moveSmasherR();
		break;
	case 2:
		moveSmasherU();
		break;
	case 3:
		moveSmasherD();
		break;
	default:
		break;
	}
}
void FirstBoss::moveSmasherL() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	//	if wall immediately to left smasher is an unbreakable wall, do nothing
	if (m_boss[row*BOSSCOLS + col-2].top == UNBREAKABLE_WALL) {
		return;
	}

	//  if there is only one space to the left of the smasher
	else if (m_boss[row*BOSSCOLS + col-3].top == UNBREAKABLE_WALL) {
		// right side of smasher is set to spaces
		for (i = col + 1, j = row - 1; j < row + 2; j++) {
			m_boss[j*BOSSCOLS + i].top = SPACE;
			m_boss[j*BOSSCOLS + i].enemy = false;
		}

		//	smasher is pushed to left wall
		for (i = col - 2, j = row - 1; j < row + 2; j++) {
			//	if player is in the way, crush them
			if (m_boss[j*BOSSCOLS + i].top == PLAYER) {
				player.at(0).setPosX(col - 2);
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
			m_boss[j*BOSSCOLS + i].top = SMASHER;
			m_boss[j*BOSSCOLS + i].enemy = true;
		}
		m_firstbossMonsters.at(0)->setPosX(col - 1);

		return;
	}

	//	if there are only two spaces to the left of the smasher
	else if (m_boss[row*BOSSCOLS + col-4].top == UNBREAKABLE_WALL) {
		// right 2 layers of smasher is set to spaces
		for (i = col + 1; i > col - 1; i--) {
			for (j = row - 1; j < row + 2; j++) {
				m_boss[j*BOSSCOLS + i].top = SPACE;
				m_boss[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	smasher is pushed to left wall
		for (i = col - 2; i > col - 4; i--) {
			for (j = row - 1; j < row + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[j*BOSSCOLS + i].top == PLAYER) {
					player.at(0).setPosX(col - 3);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				m_boss[j*BOSSCOLS + i].top = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;

			}
		}
		m_firstbossMonsters.at(0)->setPosX(col - 2);

		return;
	}

	//	if there are at least 3 spaces to left of the smasher
	else if (m_boss[row*BOSSCOLS + col-4].top != UNBREAKABLE_WALL) {
		// right 2 layers of smasher is set to spaces
		for (i = col + 1; i > col - 1; i--) {
			for (j = row - 1; j < row + 2; j++) {
				m_boss[j*BOSSCOLS + i].top = SPACE;
				m_boss[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	left two layers set to smasher
		for (i = col - 2; i > col - 4; i--) {
			for (j = row - 1; j < row + 2; j++) {
				// if player is in the way, move them to the side
				if (m_boss[j*BOSSCOLS + i].top == PLAYER) {
					player.at(0).setPosX(col - 4);
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;
				}
				m_boss[j*BOSSCOLS + i].top = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;
			}
		}
		m_firstbossMonsters.at(0)->setPosX(col - 2);
	}
}
void FirstBoss::moveSmasherR() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	//	if wall immediately to right of smasher is an unbreakable wall, do nothing
	if (m_boss[row*BOSSCOLS + col+2].top == UNBREAKABLE_WALL) {
		return;
	}

	//  if there is only one space to the right of the smasher
	else if (m_boss[row*BOSSCOLS + col+3].top == UNBREAKABLE_WALL) {
		// left side of smasher is set to spaces
		for (i = col - 1, j = row - 1; j < row + 2; j++) {
			m_boss[j*BOSSCOLS + i].top = SPACE;
			m_boss[j*BOSSCOLS + i].enemy = false;
		}

		//	smasher is pushed to right wall
		for (i = col + 2, j = row - 1; j < row + 2; j++) {
			//	if player is in the way, crush them
			if (m_boss[j*BOSSCOLS + i].top == PLAYER) {
				player.at(0).setPosX(col + 2);
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
			m_boss[j*BOSSCOLS + i].top = SMASHER;
			m_boss[j*BOSSCOLS + i].enemy = true;
		}
		m_firstbossMonsters.at(0)->setPosX(col + 1);

		return;
	}

	//	if there are only two spaces to the right of the smasher
	else if (m_boss[row*BOSSCOLS + col + 4].top == UNBREAKABLE_WALL) {
		// left two layers of smasher is set to spaces
		for (i = col - 1; i < col + 1; i++) {
			for (j = row - 1; j < row + 2; j++) {
				m_boss[j*BOSSCOLS + i].top = SPACE;
				m_boss[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	smasher is pushed to right wall
		for (i = col + 2; i < col + 4; i++) {
			for (j = row - 1; j < row + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[j*BOSSCOLS + i].top == PLAYER) {
					player.at(0).setPosX(col + 3);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				m_boss[j*BOSSCOLS + i].top = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;

			}
		}
		m_firstbossMonsters.at(0)->setPosX(col + 2);

		return;
	}

	//	if there are at least 3 spaces to left of the smasher
	else if (m_boss[row*BOSSCOLS + col + 4].top != UNBREAKABLE_WALL) {
		// left two layers of smasher is set to spaces
		for (i = col - 1; i < col + 1; i++) {
			for (j = row - 1; j < row + 2; j++) {
				m_boss[j*BOSSCOLS + i].top = SPACE;
				m_boss[j*BOSSCOLS + i].enemy = false;
			}
		}

		//	right two layers set to smasher
		for (i = col + 2; i < col + 4; i++) {
			for (j = row - 1; j < row + 2; j++) {
				// if player is in the way, move them to the side
				if (m_boss[j*BOSSCOLS + i].top == PLAYER) {
					player.at(0).setPosX(col + 4);
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;
				}
				m_boss[j*BOSSCOLS + i].top = SMASHER;
				m_boss[j*BOSSCOLS + i].enemy = true;
			}
		}
		m_firstbossMonsters.at(0)->setPosX(col + 2);
	}
}
void FirstBoss::moveSmasherU() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	//	if wall immediately above smasher is an unbreakable wall, do nothing
	if (m_boss[(row - 2)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		return;
	}

	//  if there is only one space above the smasher
	else if (m_boss[(row - 3)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// bottom layer of smasher is set to spaces
		for (i = row + 1, j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].top = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
		}

		//	smasher is pushed to upper layer
		for (i = row - 2, j = col - 1; j < col + 2; j++) {
			//	if player is in the way, crush them
			if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
				player.at(0).setPosY(row - 2);
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
			m_boss[i*BOSSCOLS + j].top = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
		m_firstbossMonsters.at(0)->setPosY(row - 1);

		return;
	}

	//	if there are only two spaces above the smasher
	else if (m_boss[(row - 4)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// bottom 2 layers of smasher is set to spaces
		for (i = row + 1; i > row - 1; i--) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].top = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	smasher is pushed to ceiling
		for (i = row - 2; i > row - 4; i--) {
			for (j = col - 1; j < col + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
					player.at(0).setPosY(row - 3);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				m_boss[i*BOSSCOLS + j].top = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;

			}
		}
		m_firstbossMonsters.at(0)->setPosY(row - 2);

		return;
	}

	//	if there are at least 3 spaces above the smasher
	else if (m_boss[(row - 4)*BOSSCOLS + col].top != UNBREAKABLE_WALL) {
		// bottom 2 layers of smasher is set to spaces
		for (i = row + 1; i > row - 1; i--) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].top = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	top two layers above smasher set to smasher
		for (i = row - 2; i > row - 4; i--) {
			for (j = col - 1; j < col + 2; j++) {
				// if player is in the way, move them above
				if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
					player.at(0).setPosY(row - 4);
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;
				}
				m_boss[i*BOSSCOLS + j].top = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		m_firstbossMonsters.at(0)->setPosY(row - 2);
	}
}
void FirstBoss::moveSmasherD() {
	shared_ptr<Smasher> smasher = dynamic_pointer_cast<Smasher>(m_firstbossMonsters.at(0));
	int col = m_firstbossMonsters.at(0)->getPosX();
	int row = m_firstbossMonsters.at(0)->getPosY();
	int i, j;

	//	if wall immediately beneath smasher is an unbreakable wall, do nothing
	if (m_boss[(row + 2)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		return;
	}

	// if there is only one space underneath the smasher
	if (m_boss[(row + 3)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// top layer of smasher is set to spaces
		for (i = row - 1, j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].top = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
		}

		//	smasher is pushed to bottom layer
		for (i = row + 2, j = col - 1; j < col + 2; j++) {
			//	if player is in the way, crush them
			if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
				player.at(0).setPosY(BOSSROWS - 2);
				m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
			}
			m_boss[i*BOSSCOLS + j].top = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
		m_firstbossMonsters.at(0)->setPosY(row + 1);

		return;
	}

	//	if there are only two spaces underneath the smasher
	if (m_boss[(row + 4)*BOSSCOLS + col].top == UNBREAKABLE_WALL) {
		// top 2 layers of smasher is set to spaces
		for (i = row - 1; i < row + 1; i++) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].top = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	smasher is pushed to bottom layer
		for (i = row + 2; i < row + 4; i++) {
			for (j = col - 1; j < col + 2; j++) {
				//	if player is in the way, crush them
				if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
					player.at(0).setPosY(BOSSROWS - 2);
					m_firstbossMonsters.at(0)->encounter(player.at(0), *(m_firstbossMonsters.at(0)), dungeonText);
				}
				m_boss[i*BOSSCOLS + j].top = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		m_firstbossMonsters.at(0)->setPosY(row + 2);

		return;
	}

	//	if there are at least 3 spaces underneath the smasher
	if (m_boss[(row + 4)*BOSSCOLS + col].top != UNBREAKABLE_WALL) {
		// top 2 layers of smasher is set to spaces
		for (i = row - 1; i < row + 1; i++) {
			for (j = col - 1; j < col + 2; j++) {
				m_boss[i*BOSSCOLS + j].top = SPACE;
				m_boss[i*BOSSCOLS + j].enemy = false;
			}
		}

		//	bottom two beneath smasher set to smasher
		for (i = row + 2; i < row + 4; i++) {
			for (j = col - 1; j < col + 2; j++) {
				// if player is in the way, move them below
				if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
					player.at(0).setPosY(row + 4);
					m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;
				}
				m_boss[i*BOSSCOLS + j].top = SMASHER;
				m_boss[i*BOSSCOLS + j].enemy = true;
			}
		}
		m_firstbossMonsters.at(0)->setPosY(row + 2);

	}
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
		return;
	}

	//	bottom 3 layers set to space
	for (i = row + 1; i > row - 2; i--) {
		for (j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].top = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
			//	if player was smashed, replace with player icon
			if (i == player.at(0).getPosY() && j == player.at(0).getPosX())
				m_boss[i*BOSSCOLS + j].top = PLAYER;
		}
	}
	
	//  three layers above smasher set to smasher
	for (i = row - 2; i > row - 5; i--) {
		for (j = col - 1; j < col + 2; j++) {
			// if player is in the way, move them above
			if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
				player.at(0).setPosY(row - 5);
				m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;
			}
			m_boss[i*BOSSCOLS + j].top = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
	}
	m_firstbossMonsters.at(0)->setPosY(row - 3);
	
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
		return;
	}

	//	top 2 layers set to space
	for (i = row - 1; i < row + 1; i++) {
		for (j = col - 1; j < col + 2; j++) {
			m_boss[i*BOSSCOLS + j].top = SPACE;
			m_boss[i*BOSSCOLS + j].enemy = false;
			//	if player was smashed, replace with player icon
			if (i == player.at(0).getPosY() && j == player.at(0).getPosX())
				m_boss[i*BOSSCOLS + j].top = PLAYER;
		}
	}

	//  two layers below smasher set to smasher
	for (i = row + 2; i < row + 4; i++) {
		for (j = col - 1; j < col + 2; j++) {
			// if player is in the way, move them above
			if (m_boss[i*BOSSCOLS + j].top == PLAYER) {
				player.at(0).setPosY(row + 4);
				m_boss[player.at(0).getPosY()*BOSSCOLS + player.at(0).getPosX()].top = PLAYER;
			}
			m_boss[i*BOSSCOLS + j].top = SMASHER;
			m_boss[i*BOSSCOLS + j].enemy = true;
		}
	}
	m_firstbossMonsters.at(0)->setPosY(row + 2);
}

void FirstBoss::fight(int x, int y) { // monster's coordinates
	unsigned i = 0;
	bool found = false;
	while (!found && i < m_firstbossMonsters.size()) { // finds the monster just encountered
		if (m_firstbossMonsters.at(i)->getPosX() == x && m_firstbossMonsters.at(i)->getPosY() == y)
			found = true;
		else
			i++;
	}
	
	player.at(0).attack(m_firstbossMonsters, i, dungeonText);
	string monster = m_firstbossMonsters.at(i)->getName();

	if (m_firstbossMonsters.at(i)->getHP() <= 0) {
		if (monster == "Smasher") {
			monsterDeath(x, y, i);
		}
		else {
			monsterDeath(m_boss, BOSSCOLS, m_firstbossMonsters, x, y, i);
		}
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
