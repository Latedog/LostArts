#include "global.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include "Actors.h"
#include "utilities.h"
#include <iostream>

using namespace std;

Dungeon::Dungeon(){

}
Dungeon::Dungeon(int smelldist) : m_smelldist(smelldist), m_level(1) {
	Tile *tile;
	int i, j;
	
	for (i = 0; i < MAXROWS; i++) { //		initialize entire dungeon to blank space
		for (j = 0; j < MAXCOLS; j++) {
			tile = new Tile;
			tile->top = SPACE;
			tile->bottom = SPACE;
			tile->enemy = false;
			tile->marked = false;

			m_maze[i][j] = *tile;
		}
	}

	for (i = 0; i < MAXROWS; i++) { //	initialize top and bottom of dungeon to be unbreakable walls
		for (j = 0; j < MAXCOLS; j++) {
			m_maze[i][j].top = UNBREAKABLE_WALL;
			m_maze[i][j].bottom = UNBREAKABLE_WALL;
		}
		i += 16;
	}

	for (i = 1; i < MAXROWS - 1; i++){ //		initialize edges of dungeon to be #
		m_maze[i][0].top = m_maze[i][0].bottom = UNBREAKABLE_WALL;
		m_maze[i][69].top = m_maze[i][69].bottom = UNBREAKABLE_WALL;
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
			m_maze[i][j].top = finalvec.at(count);
			m_maze[i][j].bottom = finalvec.at(count);

			//	if the object was an item or enemy
			if (m_maze[i][j].bottom != WALL && m_maze[i][j].bottom != SPACE) {

				object = m_maze[i][j].bottom;
				switch (object) {
				case BROWN_CHEST:
				case SILVER_CHEST:
					m_maze[i][j].top = SPACE;
					break;
				case WANDERER: {
					m_maze[i][j].bottom = SPACE;
					Wanderer wand;
					wand.setPosX(j);
					wand.setPosY(i);
					monsters.emplace_back(new Wanderer(wand));
					m_maze[i][j].enemy = true;
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

	char toptile = m_maze[player.at(0).getPosY()][player.at(0).getPosX()].top;
	while (toptile != SPACE || player.at(0).getPosX() > 4 && player.at(0).getPosX() < 65) {
		player.at(0).setrandPosX();
		player.at(0).setrandPosY();

		// while player is not spawned along left side or right side, reroll
		while (player.at(0).getPosX() > 4 && player.at(0).getPosX() < 65) {
			player.at(0).setrandPosX();
		}

		toptile = m_maze[player.at(0).getPosY()][player.at(0).getPosX()].top;
	}
	m_maze[player.at(0).getPosY()][player.at(0).getPosX()].top = PLAYER; // sets tile at this spot to be player position
	player.at(0).addItem(Bomb());	// adds bomb to player's inventory upon spawn


	Idol idol;
	toptile = m_maze[idol.getPosY()][idol.getPosX()].top;

	//	while idol does not spawn on the side opposite from the player
	while (toptile != SPACE || ((player.at(0).getPosX() < 5 && idol.getPosX() < 65) || (player.at(0).getPosX() > 64 && idol.getPosX() > 4))) {
		idol.setrandPosX();
		idol.setrandPosY();

		toptile = m_maze[idol.getPosY()][idol.getPosX()].top;
	}
	m_maze[idol.getPosY()][idol.getPosX()].bottom = IDOL;


	Stairs stairs;
	toptile = m_maze[stairs.getPosY()][stairs.getPosX()].top;

	//	while stairs does not spawn on the side opposite from the player
	//while (toptile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() < 65) || (player.at(0).getPosX() > 64 && stairs.getPosX() > 4))) {
	while (toptile != SPACE || ((player.at(0).getPosX() < 5 && stairs.getPosX() >= 5) || (player.at(0).getPosX() > 64 && stairs.getPosX() < 64))) {
		stairs.setrandPosX();
		stairs.setrandPosY();

		toptile = m_maze[stairs.getPosY()][stairs.getPosX()].top;
	}
	m_maze[stairs.getPosY()][stairs.getPosX()].bottom = STAIRS;


	LifePotion lp;
	char bottomtile = m_maze[lp.getPosY()][lp.getPosX()].bottom;
	while (bottomtile != SPACE) {
		lp.setrandPosX();
		lp.setrandPosY();

		bottomtile = m_maze[lp.getPosY()][lp.getPosX()].bottom;
	}
	m_maze[lp.getPosY()][lp.getPosX()].bottom = LIFEPOT;


	Bomb bomb;
	bottomtile = m_maze[bomb.getPosY()][bomb.getPosX()].bottom;
	while (bottomtile != SPACE) {
		bomb.setrandPosX();
		bomb.setrandPosY();

		bottomtile = m_maze[bomb.getPosY()][bomb.getPosX()].bottom;
	}
	m_maze[bomb.getPosY()][bomb.getPosX()].bottom = BOMB;


	BrownChest chest;
	bottomtile = m_maze[chest.getPosY()][chest.getPosX()].bottom;
	while (bottomtile != SPACE) {
		chest.setrandPosX();
		chest.setrandPosY();

		bottomtile = m_maze[chest.getPosY()][chest.getPosX()].bottom;
	}
	m_maze[chest.getPosY()][chest.getPosX()].bottom = BROWN_CHEST;


	int numwep = 1 + randInt(2); // number of weapons to be placed
	while (numwep > 0) {
		RustyCutlass rc;
		
		bottomtile = m_maze[rc.getPosY()][rc.getPosX()].bottom;
		while (bottomtile != SPACE) { // while sword position clashes with anything
			rc.setrandPosX();				// reroll it
			rc.setrandPosY();

			bottomtile = m_maze[rc.getPosY()][rc.getPosX()].bottom;
		}
		m_maze[rc.getPosY()][rc.getPosX()].bottom = CUTLASS;
		numwep--;
	}

	//	generate goblins
	int m = 5 + randInt(3); // number of goblins to be placed
	while (m > 0) { //generate goblins
		Goblin g(smelldist);

		toptile = m_maze[g.getPosY()][g.getPosX()].top;
		while (toptile != SPACE) { //while goblin position clashes with wall, player, or idol
			g.setrandPosX();				// reroll it
			g.setrandPosY();

			toptile = m_maze[g.getPosY()][g.getPosX()].top;
		}

		//goblins.push_back(g);
		//monsters.push_back(&g);
		monsters.emplace_back(new Goblin(g));
		m_maze[g.getPosY()][g.getPosX()].top = GOBLIN;
		m_maze[g.getPosY()][g.getPosX()].enemy = true;
		m--;
	}

	// generate wanderers
	m = 5 + randInt(7);
	while (m > 0) {
		Wanderer w;

		toptile = m_maze[w.getPosY()][w.getPosX()].top;
		while (toptile != SPACE) { // while enemy position clashes with wall, player, or idol
			w.setrandPosX();		// reroll it
			w.setrandPosY();

			toptile = m_maze[w.getPosY()][w.getPosX()].top;
		}
		monsters.emplace_back(new Wanderer(w));
		m_maze[w.getPosY()][w.getPosX()].top = WANDERER;
		m_maze[w.getPosY()][w.getPosX()].enemy = true;
		m--;
	}

	// generate archers
	m = 5 + randInt(5);
	while (m > 0) {
		Archer a;

		toptile = m_maze[a.getPosY()][a.getPosX()].top;
		while (toptile != SPACE) {
			a.setrandPosX();				// reroll it
			a.setrandPosY();

			toptile = m_maze[a.getPosY()][a.getPosX()].top;
		}
		monsters.emplace_back(new Archer(a));
		m_maze[a.getPosY()][a.getPosX()].top = ARCHER;
		m_maze[a.getPosY()][a.getPosX()].enemy = true;
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

void Dungeon::bossFight() {
	//FirstBoss firstboss;
}

void Dungeon::peekDungeon(int x, int y, char move) {
	char top, bottom;

	//move player
	if (move == 'l') { // moved to the left
		top = m_maze[y][x - 1].top;
		bottom = m_maze[y][x - 1].bottom;

		if (top == SPACE && bottom == ' ') {
			// move character to the left
			m_maze[y][x - 1].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosX(x-1);

			showDungeon();
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y][x - 1].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosX(x - 1);

			showDungeon();
			cout << "You've found ";
			foundItem(x - 1, y);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {	// monster encountered
			fight(x-1, y);
			/*
			showDungeon();

			unsigned i = 0;
			bool found = false;
			while (!found && i < goblins.size()) { // finds the goblin just encountered
				if (goblins.at(i).getPosX() == x - 1 && goblins.at(i).getPosY() == y)
					found = true;
				else
					i++;
			}

			player.at(0).attack(goblins.at(i));
			if (goblins.at(i).getHP() <= 0) {
				cout << "The goblin was slain.\n" << endl;

				if (randInt(3) + 1 == 3) {
					BoneAxe(x - 1, y);
					m_maze[y][x - 1].bottom = 'q';
				}
				m_maze[y][x - 1].top = SPACE;

				goblins.erase(goblins.begin() + i);
			}*/
		}
		else if (top == SPACE && bottom == IDOL) {
			m_maze[y][x - 1].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosX(x - 1);

			showDungeon();
			cout << "It's the idol!\n" << endl;
			return;
		}
		else
			showDungeon();
	}

	if (move == 'r') { // moved to the right
		top = m_maze[y][x + 1].top;
		bottom = m_maze[y][x + 1].bottom;

		if (top == SPACE && bottom == SPACE) {
			// move character to the left
			m_maze[y][x + 1].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosX(x + 1);

			showDungeon();
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y][x + 1].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosX(x + 1);

			showDungeon();
			cout << "You've found ";
			foundItem(x + 1, y);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			fight(x+1, y);
			/*
			showDungeon();

			unsigned i = 0;
			bool found = false;
			while (!found && i < goblins.size()) {
				if (goblins.at(i).getPosX() == x+1 && goblins.at(i).getPosY() == y)
					found = true;
				else
					i++;
			}

			player.at(0).attack(goblins.at(i));
			if (goblins.at(i).getHP() <= 0) {
				cout << "The goblin was slain." << endl;

				if (randInt(3) + 1 == 3) {
					BoneAxe(x + 1, y);
					m_maze[y][x + 1].bottom = 'q';
				}
				m_maze[y][x + 1].top = SPACE;
				goblins.erase(goblins.begin() + i);
			}*/
		}
		else if (top == SPACE && bottom == IDOL) {
			m_maze[y][x + 1].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosX(x + 1);

			showDungeon();
			cout << "It's the idol!\n" << endl;
			return;
		}
		else
			showDungeon();
	}

	if (move == 'u') { // moved up
		top = m_maze[y+1][x].top;
		bottom = m_maze[y+1][x].bottom;

		if (top == SPACE && bottom == SPACE) {
			m_maze[y + 1][x].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosY(y + 1);

			showDungeon();
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y + 1][x].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosY(y + 1);

			showDungeon();
			cout << "You've found ";
			foundItem(x, y + 1);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			fight(x, y+1);
			/*
			showDungeon();

			unsigned i = 0;
			bool found = false;
			while (!found && i < goblins.size()) {
				if (goblins.at(i).getPosX() == x && goblins.at(i).getPosY() == y+1)
					found = true;
				else
					i++;
			}

			player.at(0).attack(goblins.at(i));
			if (goblins.at(i).getHP() <= 0) {
				cout << "The goblin was slain." << endl;

				if (randInt(3) + 1 == 3) {
					BoneAxe(x, y + 1);
					m_maze[y + 1][x].bottom = 'q';
				}
				m_maze[y+1][x].top = SPACE;
				goblins.erase(goblins.begin() + i);
			}*/
		}
		else if (top == SPACE && bottom == IDOL) {
			m_maze[y + 1][x].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosY(y + 1);

			showDungeon();
			cout << "It's the idol!\n" << endl;
			return;
		}
		else
			showDungeon();
	}

	if (move == 'd') { // moved down
		top = m_maze[y - 1][x].top;
		bottom = m_maze[y - 1][x].bottom;

		if (top == SPACE && bottom == SPACE) {
			m_maze[y - 1][x].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosY(y - 1);

			showDungeon();
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y - 1][x].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosY(y - 1);

			showDungeon();
			cout << "You've found ";
			foundItem(x, y - 1);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			fight(x, y-1);
			/*
			showDungeon();

			unsigned i = 0;
			bool found = false;
			while (!found && i < goblins.size()) {
				if (goblins.at(i).getPosX() == x && goblins.at(i).getPosY() == y-1)
					found = true;
				else
					i++;
			}

			player.at(0).attack(goblins.at(i));
			if (goblins.at(i).getHP() <= 0) {
				cout << "The goblin was slain." << endl;

				if (randInt(3) + 1 == 3) {
					BoneAxe(x, y-1);
					m_maze[y-1][x].bottom = 'q';
				}
				m_maze[y-1][x].top = SPACE;
				goblins.erase(goblins.begin() + i);
			}
			*/
		}
		else if (top == SPACE && bottom == IDOL) {
			m_maze[y - 1][x].top = PLAYER;
			m_maze[y][x].top = SPACE;
			player.at(0).setPosY(y - 1);

			showDungeon();
			cout << "It's the idol!\n" << endl;
			return;
		}
		else
			showDungeon();
	}

	if (move == 'g') {
		collectItem(x, y);
	}

	if (move == 'w') {
		showDungeon();
		player.at(0).wield();
	}

	if (move == 'c') {
		showDungeon();
		player.at(0).use(active, m_maze[y][x]);
	}
	
	
	// check active items in play
	checkActive();
	

	// if all monsters are dead?


	// move monsters
	int g_x, g_y, shortest;
	char first, best, prev;

	for (unsigned i = 0; i < monsters.size(); i++) {
		g_x = monsters.at(i)->getPosX();
		g_y = monsters.at(i)->getPosY();
		first = '0'; best = '0'; prev = '0';
		shortest = 0;

		if (monsters.at(i)->getName() == "goblin") {
			if (goblinInRange(i)) {
				if (moveGoblins(i, shortest, getSmellDistance(), g_x, g_y, \
					first, best, prev)) { // viable path is found
					switch (best) {
					case 'l':
						m_maze[g_y][g_x - 1].top = GOBLIN;
						m_maze[g_y][g_x - 1].enemy = true;
						m_maze[g_y][g_x].top = SPACE;
						m_maze[g_y][g_x].enemy = false;
						monsters.at(i)->setPosX(g_x - 1);

						break;
					case 'r':
						m_maze[g_y][g_x + 1].top = GOBLIN;
						m_maze[g_y][g_x + 1].enemy = true;
						m_maze[g_y][g_x].top = SPACE;
						m_maze[g_y][g_x].enemy = false;
						monsters.at(i)->setPosX(g_x + 1);
						break;
					case 'u':
						m_maze[g_y + 1][g_x].top = GOBLIN;
						m_maze[g_y + 1][g_x].enemy = true;
						m_maze[g_y][g_x].top = SPACE;
						m_maze[g_y][g_x].enemy = false;
						monsters.at(i)->setPosY(g_y + 1);
						break;
					case 'd':
						m_maze[g_y - 1][g_x].top = GOBLIN;
						m_maze[g_y - 1][g_x].enemy = true;
						m_maze[g_y][g_x].top = SPACE;
						m_maze[g_y][g_x].enemy = false;
						monsters.at(i)->setPosY(g_y - 1);
						break;
					default:
						monsters.at(i)->encounter(player.at(0), *(monsters.at(i)));
						if (player.at(0).getHP() <= 0) {
							m_maze[y][x].top = 'X'; // change player icon to X to indicate death
							player.at(0).setDeath(monsters.at(i)->getName());
						}
						break;
					}
				}
			}
		}

		else if (monsters.at(i)->getName() == "wanderer") {
			int n = -1 + randInt(3);
			int m = -1 + randInt(3);

			if (m_maze[g_y + m][g_x + n].top != WALL && m_maze[g_y + m][g_x + n].top != UNBREAKABLE_WALL && !m_maze[g_y + m][g_x + n].enemy) {
				if (m_maze[g_y + m][g_x + n].top != PLAYER) {
					m_maze[g_y + m][g_x + n].top = WANDERER;
					m_maze[g_y + m][g_x + n].enemy = true;
					m_maze[g_y][g_x].top = SPACE;
					m_maze[g_y][g_x].enemy = false;
					monsters.at(i)->setPosX(g_x + n);
					monsters.at(i)->setPosY(g_y + m);
				}
				else {
					monsters.at(i)->encounter(player.at(0), *(monsters.at(i)));
					if (player.at(0).getHP() <= 0) {
						m_maze[y][x].top = 'X'; //change player icon to X to indicate death
						player.at(0).setDeath(monsters.at(i)->getName());
					}
				}
			}
		}

		else if (monsters.at(i)->getName() == "archer") {
			checkArchers(player.at(0).getPosX(), player.at(0).getPosY(), i);

			if (player.at(0).getHP() <= 0) {
				m_maze[y][x].top = 'X'; //change player icon to X to indicate death
				player.at(0).setDeath(monsters.at(i)->getName());
			}
		}

		//	if player is dead then break
		if (player.at(0).getDeath() != "")
			break;
	}


	// roll heal chance
	player.at(0).rollHeal();
}
void Dungeon::checkActive() {
	if (!active.empty()) {
		for (unsigned i = 0; i < active.size(); i++) {
			if (active.at(i)->getItem() == "Bomb") {
				shared_ptr<Bomb> bomb = dynamic_pointer_cast<Bomb>(active.at(i));
				if (bomb->getFuse() > 0) {
					bomb->setFuse(bomb->getFuse() - 1);
					active.at(i) = bomb;
				}
				else {
					int pos = i;
					active.erase(active.begin() + pos);

					cout << "A bomb exploded!\n";
					explosion(bomb->getPosX(), bomb->getPosY());
					m_maze[bomb->getPosY()][bomb->getPosX()].bottom = SPACE;
				}
			}
		}
	}
}
void Dungeon::explosion(int x, int y) { // bomb's coordinates
	int mx, my;
	for (unsigned i = 0; i < monsters.size(); i++) {
		mx = monsters.at(i)->getPosX();
		my = monsters.at(i)->getPosY();
		if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			monsters.at(i)->setHP(monsters.at(i)->getHP() - 10);
			cout << "The " << monsters.at(i)->getName() << " was blown up!\n";

			if (monsters.at(i)->getHP() <= 0) {
				monsterDeath(mx, my, i);
			}
		}
	}
	
	//	destroy any nearby walls
	for (int j = y - 1; j < y + 2; j++) {
		for (int k = x - 1; k < x + 2; k++) {
			if (m_maze[j][k].top == WALL) {
				m_maze[j][k].top = SPACE;
				m_maze[j][k].bottom = SPACE;
			}
		}
	}
}
void Dungeon::monsterDeath(int x, int y, int pos) {
	string monster = monsters.at(pos)->getName();
	cout << "The " << monster << " was slain.\n" << endl;

	if (monster == "goblin") {
		if (randInt(3) + 1 == 3) {
			BoneAxe(x, y);
			m_maze[y][x].bottom = BONEAXE;
		}
	}
	else if (monster == "wanderer") {
		if (randInt(1) + 1 == 1) {
			HeartPod(x, y);
			m_maze[y][x].bottom = HEART_POD;
		}
	}
	else if (monster == "archer") {

	}
	m_maze[y][x].top = SPACE;
	m_maze[y][x].enemy = false;

	//delete monsters.at(i);
	monsters.erase(monsters.begin() + pos);
}

int Dungeon::getSmellDistance() const {
	return m_smelldist;
}
bool Dungeon::moveGoblins(int pos, int &shortest, int smelldist, int x, int y, \
	char &first_move, char &optimal_move, char prev) {
	
	if (smelldist == getSmellDistance() && \
		(m_maze[y - 1][x].top == PLAYER || m_maze[y + 1][x].top == PLAYER || m_maze[y][x - 1].top == PLAYER || m_maze[y][x + 1].top == PLAYER)) {
		return true; //	if player is immediately adjacent to the goblin, return to attack
	}
	
	if (smelldist < 0) // player was not found within getSmellDistance() steps
		return false;

	if (m_maze[y][x].top == PLAYER) {
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

	if (prev != 'l' && (m_maze[y][x+1].top != WALL && m_maze[y][x+1].top != UNBREAKABLE_WALL) && !m_maze[y][x+1].enemy) {
		if (smelldist == getSmellDistance())
			first_move = 'r';
		moveGoblins(pos, shortest, smelldist - 1, x + 1, y, first_move, optimal_move, 'r');
	}
	
	if (prev != 'r' && (m_maze[y][x-1].top != WALL && m_maze[y][x-1].top != UNBREAKABLE_WALL) && !m_maze[y][x-1].enemy) {
		if (smelldist == getSmellDistance())
			first_move = 'l';
		moveGoblins(pos, shortest, smelldist - 1, x - 1, y, first_move, optimal_move, 'l');
	}

	if (prev != 'd' && (m_maze[y+1][x].top != WALL && m_maze[y+1][x].top != UNBREAKABLE_WALL) && !m_maze[y+1][x].enemy) {
		if (smelldist == getSmellDistance())
			first_move = 'u';
		moveGoblins(pos, shortest, smelldist - 1, x, y + 1, first_move, optimal_move, 'u');
	}

	if (prev != 'u' && (m_maze[y-1][x].top != WALL && m_maze[y-1][x].top != UNBREAKABLE_WALL) && !m_maze[y-1][x].enemy) {
		if (smelldist == getSmellDistance())
			first_move = 'd';
		moveGoblins(pos, shortest, smelldist - 1, x, y - 1, first_move, optimal_move, 'd');
	}

	if (optimal_move != '0')
		return true;
	return false;
}
bool Dungeon::goblinInRange(int pos) {
	int px = player.at(0).getPosX();
	int gx = monsters.at(pos)->getPosX();

	int py = player.at(0).getPosY();
	int gy = monsters.at(pos)->getPosY();

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
void Dungeon::checkArchers(int x, int y, int pos) {
	shared_ptr<Archer> archer = dynamic_pointer_cast<Archer>(monsters.at(pos));
	int mx = monsters.at(pos)->getPosX();
	int my = monsters.at(pos)->getPosY();

	//	if archer is primed, check to attack and return
	if (archer->isPrimed()) {
		if (x - mx == 0 || y - my == 0)
			monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)));
		else
			cout << "The archer eases their aim.\n";

		archer->prime(false);
		monsters.at(pos) = archer;

		return;
	}


	// if archer has dagger out and is adjacent to player
	if (monsters.at(pos)->getWeapon().getAction() == "Bronze Dagger" && \
		(m_maze[my - 1][mx].top == PLAYER || m_maze[my + 1][mx].top == PLAYER || m_maze[my][mx - 1].top == PLAYER || m_maze[my][mx + 1].top == PLAYER)) {
		monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)));
		return;
	}
	// else if dagger is out and player moved away
	else if (monsters.at(pos)->getWeapon().getAction() == "Bronze Dagger" && \
		(m_maze[my - 1][mx].top != PLAYER && m_maze[my + 1][mx].top != PLAYER && m_maze[my][mx - 1].top != PLAYER && m_maze[my][mx + 1].top != PLAYER)) {
		monsters.at(pos)->setWeapon(WoodBow());
		cout << "The archer switches back to their Wood Bow.\n";
		return;
	}

	//if player is not in the archer's sights, move archer
	if (x - mx != 0 && y - my != 0) {
		moveArchers(mx, my, pos);
		return;
	}

	// if player and archer are on the same column
	if (x - mx == 0) {		
		//	if player is within range and not adjacent, get primed
		if (abs(y - my) > 1 && abs(y - my) <= 15) {
			if (!wallCollision('y', y, my)) {
				archer->prime(true);
				monsters.at(pos) = archer;

				cout << "The archer is primed to shoot!\n";
			}
			else {
				moveArchers(mx, my, pos);
				return;
			}
		}
		else if (abs(y - my) == 1) {
			//swap weapon to dagger
			monsters.at(pos)->setWeapon(BronzeDagger());
			cout << "The archer switches to their Bronze Dagger.\n";
			//monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)));
		}
		//	just move archer
		else {
			moveArchers(mx, my, pos);
			return;
		}
	}
	// if player and archer are on the same row
	else if (y - my == 0) {
		//	if player is within range and not adjacent, get primed
		if (abs(x - mx) > 1 && abs(x - mx) <= 15) {
			if (!wallCollision('x', x, mx)) {
				archer->prime(true);
				monsters.at(pos) = archer;

				cout << "The archer is primed to shoot!\n";
			}
			else {
				moveArchers(mx, my, pos);
				return;
			}
		}
		else if (abs(x - mx) == 1) {
			// swap weapon to dagger
			monsters.at(pos)->setWeapon(BronzeDagger());
			cout << "The archer switches to their Bronze Dagger.\n";
			//monsters.at(pos)->encounter(player.at(0), *(monsters.at(pos)));
		}
		// just move archer
		else {
			moveArchers(mx, my, pos);
		}
	}

}
void Dungeon::moveArchers(int mx, int my, int pos) {
	int n = randInt(2);
	int m = 0;
	char move;

	if (m_maze[my][mx + 1].top != SPACE && m_maze[my][mx - 1].top != SPACE && m_maze[my + 1][mx].top != SPACE && m_maze[my - 1][mx].top != SPACE) {
		//  if archer is surrounded completely, do nothing (such as goblin or wanderer blocking an archer's path)
		return;
	}
	else if (m_maze[my][mx + 1].top != SPACE && m_maze[my][mx - 1].top != SPACE) {
		//	if archer is surrounded by walls on left and right
		move = 'v';
	}
	else if (m_maze[my + 1][mx].top != SPACE && m_maze[my - 1][mx].top != SPACE) {
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
		while (n == 0 || m_maze[my][mx + n].top != SPACE) {
			n = -1 + randInt(3);
		}

		m_maze[my][mx + n].top = ARCHER;
		m_maze[my][mx + n].enemy = true;
		m_maze[my][mx].top = SPACE;
		m_maze[my][mx].enemy = false;
		monsters.at(pos)->setPosX(mx + n);
	}
	else if (move == 'v') {
		//	move is vertical
		m = -1 + randInt(3);
		while (m == 0 || m_maze[my + m][mx].top != SPACE) {
			m = -1 + randInt(3);
		}

		m_maze[my + m][mx].top = ARCHER;
		m_maze[my + m][mx].enemy = true;
		m_maze[my][mx].top = SPACE;
		m_maze[my][mx].enemy = false;
		monsters.at(pos)->setPosY(my + m);
	}

}
bool Dungeon::wallCollision(char direction, int p_move, int m_move) {
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
				if (m_maze[otherdir][p_move - 1].top == WALL || m_maze[otherdir][p_move - 1].top == UNBREAKABLE_WALL)
					return true;
				else
					p_move--;
			}
			else if (direction == 'y') {
				if (m_maze[p_move - 1][otherdir].top == WALL || m_maze[p_move - 1][otherdir].top == UNBREAKABLE_WALL)
					return true;
				else
					p_move--;
			}
		}
	}
	else {
		while (m_move - p_move != 0) {
			if (direction == 'x') {
				if (m_maze[otherdir][m_move - 1].top == WALL || m_maze[otherdir][m_move - 1].top == UNBREAKABLE_WALL)
					return true;
				else
					m_move--;
			}
			else if (direction == 'y') {
				if (m_maze[m_move - 1][otherdir].top == WALL || m_maze[m_move - 1][otherdir].top == UNBREAKABLE_WALL)
					return true;
				else
					m_move--;
			}
		}
	}
	return false;
}
void Dungeon::foundItem(int x, int y) {
	char c = m_maze[y][x].bottom;
	switch (c) {
	case CUTLASS:
		cout << "a Rusty Cutlass.\n" << endl;
		break;
	case BONEAXE:
		cout << "a Bone Axe.\n" << endl;
		break;
	case HEART_POD:
		cout << "a Heart Pod.\n" << endl;
		break;
	case LIFEPOT:
		cout << "a Life Potion.\n" << endl;
		break;
	case ARMOR:
		cout << "some extra Armor.\n" << endl;
		break;
	case STATPOT:
		cout << "a Stat Potion.\n" << endl;
		break;
	case BOMB:
		cout << "a Bomb!\n" << endl;
		break;
	case BROWN_CHEST:
		cout << "a Brown Chest!\n" << endl;
		break;
	case SILVER_CHEST:
		cout << "a Silver Chest!\n" << endl;
		break;
	case GOLDEN_CHEST:
		cout << "a Golden Chest!\n" << endl;
		break;
	case STAIRS:
		cout << "stairs!\n" << endl;
		break;
	default:
		break;
	}
}
void Dungeon::collectItem(int x, int y) {
	if (m_maze[y][x].bottom == CUTLASS) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addWeapon(RustyCutlass(x, y)); //adds short sword to inventory
			m_maze[y][x].bottom = SPACE;

			showDungeon();
			cout << "You pick up the Rusty Cutlass.\n" << endl;
		}
		else {
			cout << "Your backpack is full!\n" << endl;
		}
	}
	else if (m_maze[y][x].bottom == BONEAXE) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addWeapon(BoneAxe(x,y)); //adds bone axe to inventory
			m_maze[y][x].bottom = SPACE;

			showDungeon();
			cout << "You pick up the Bone Axe.\n" << endl;
		}
		else {
			cout << "Your backpack is full!\n" << endl;
		}
	}
	else if (m_maze[y][x].bottom == IDOL) {
		player.at(0).setWin(true);
		m_maze[y][x].bottom = SPACE;
		showDungeon();
	}
	else if (m_maze[y][x].bottom == STAIRS) {
		setLevel(getLevel() + 1);
	}

	//		BEGIN DROPS IF STATEMENTS
	else if (m_maze[y][x].bottom == HEART_POD) {
		HeartPod heart;

		m_maze[y][x].bottom = SPACE;
		showDungeon();
		heart.changeStats(heart, player.at(0));
	}
	else if (m_maze[y][x].bottom == LIFEPOT) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addItem(LifePotion());
			m_maze[y][x].bottom = SPACE;

			showDungeon();
			cout << "You grab the Life Potion!" << endl;
		}
		else {
			cout << "You can't hold any more items." << endl;
		}
	}
	else if (m_maze[y][x].bottom == ARMOR) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addItem(ArmorDrop());
			m_maze[y][x].bottom = SPACE;

			showDungeon();
			cout << "You grab the Armor!" << endl;
		}
		else {
			cout << "You can't hold any more items." << endl;
		}
	}
	else if (m_maze[y][x].bottom == STATPOT) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addItem(StatPotion());
			m_maze[y][x].bottom = SPACE;

			showDungeon();
			cout << "You grab the Stat Potion!" << endl;
		}
		else {
			cout << "You can't hold any more items." << endl;
		}
	}
	else if (m_maze[y][x].bottom == BOMB) {
		if (player.at(0).getInventorySize() + player.at(0).getItemInvSize() < 25) {
			player.at(0).addItem(Bomb());
			m_maze[y][x].bottom = SPACE;

			showDungeon();
			cout << "You grab the Bomb!" << endl;
		}
		else {
			cout << "You can't hold any more items." << endl;
		}
	}
	else if (m_maze[y][x].bottom == BROWN_CHEST) {
		BrownChest c;

		showDungeon();
		cout << "You open the Brown Chest... ";
		c.open(m_maze[y][x]);
	}
	else if (m_maze[y][x].bottom == SILVER_CHEST) {
		SilverChest c;

		showDungeon();
		cout << "You open the Silver Chest... ";
		c.open(m_maze[y][x]);
	}
	else if (m_maze[y][x].bottom == GOLDEN_CHEST) {
		GoldenChest c;

		showDungeon();
		cout << "You open the Golden Chest... ";
		c.open(m_maze[y][x]);
	}
	else {
		showDungeon();
		cout << "There's nothing in the dirt... \n" << endl;
	}
}
void Dungeon::showDungeon() {
	//	adjust window to be more centered
	cout << "\n\n\n\n";
	cout << "       ";

	for (int i = 0; i < MAXROWS; i++) {
		for (int j = 0; j < MAXCOLS; j++) {
			if (m_maze[i][j].top == SPACE)
				cout << m_maze[i][j].bottom;
			else
				cout << m_maze[i][j].top;
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
	showDungeon();

	unsigned i = 0;
	bool found = false;
	while (!found && i < monsters.size()) { // finds the monster just encountered
		if (monsters.at(i)->getPosX() == x && monsters.at(i)->getPosY() == y)
			found = true;
		else
			i++;
	}
	string monster = monsters.at(i)->getName();
	player.at(0).attack(monsters, i);
	if (monsters.at(i)->getHP() <= 0) {
		monsterDeath(x, y, i);
	}
}


void Dungeon::unmarkTiles() {
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 70; j++) {
			m_maze[i][j].marked = false;
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
FirstBoss::FirstBoss(Player &p) {
	Tile *tile;
	int i, j;

	for (i = 0; i < BOSSROWS; i++) { //		initialize entire dungeon to blank space
		for (j = 0; j < BOSSCOLS; j++) {
			tile = new Tile;
			tile->top = SPACE;
			tile->bottom = SPACE;
			tile->enemy = false;
			tile->marked = false;

			m_boss[i][j] = *tile;
		}
	}

	for (i = 0; i < BOSSROWS; i++) { //	initialize top and bottom of dungeon to be unbreakable walls
		for (j = 0; j < BOSSCOLS; j++) {
			m_boss[i][j].top = UNBREAKABLE_WALL;
			m_boss[i][j].bottom = UNBREAKABLE_WALL;
		}
		i += (BOSSROWS - 2);
	}

	for (i = 1; i < BOSSROWS - 1; i++) { //		initialize edges of dungeon to be unbreakable walls
		m_boss[i][0].top = m_boss[i][0].bottom = UNBREAKABLE_WALL;
		m_boss[i][BOSSCOLS-1].top = m_boss[i][BOSSCOLS-1].bottom = UNBREAKABLE_WALL;
	}


	//	spawn player
	m_bossplayer = p;
	m_bossplayer.setPosX(BOSSCOLS / 2);
	m_bossplayer.setPosY(BOSSROWS - 3);
	m_boss[m_bossplayer.getPosY()][m_bossplayer.getPosX()].top = PLAYER;


	//	spawn boss
	Smasher smasher;
	bossMonsters.emplace_back(new Smasher(smasher));
	m_boss[smasher.getPosY()][smasher.getPosX()].top = SMASHER;
	m_boss[smasher.getPosY()][smasher.getPosX()].enemy = true;


}

void FirstBoss::peekFirstBossDungeon(int x, int y, char move) {
	char top, bottom;

	//move player
	if (move == 'l') { // moved to the left
		top = m_boss[y][x - 1].top;
		bottom = m_boss[y][x - 1].bottom;

		if (top == SPACE && bottom == ' ') {
			// move character to the left
			m_boss[y][x - 1].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosX(x - 1);

			showDungeon();
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_boss[y][x - 1].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosX(x - 1);

			showDungeon();
			cout << "You've found ";
			foundItem(x - 1, y);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {	// monster encountered
			fight(x - 1, y);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_boss[y][x - 1].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosX(x - 1);

			showDungeon();
			cout << "It's the idol!\n" << endl;
			return;
		}
		else
			showDungeon();
	}

	if (move == 'r') { // moved to the right
		top = m_boss[y][x + 1].top;
		bottom = m_boss[y][x + 1].bottom;

		if (top == SPACE && bottom == SPACE) {
			// move character to the left
			m_boss[y][x + 1].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosX(x + 1);

			showDungeon();
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_boss[y][x + 1].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosX(x + 1);

			showDungeon();
			cout << "You've found ";
			foundItem(x + 1, y);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			fight(x + 1, y);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_boss[y][x + 1].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosX(x + 1);

			showDungeon();
			cout << "It's the idol!\n" << endl;
			return;
		}
		else
			showDungeon();
	}

	if (move == 'u') { // moved up
		top = m_boss[y + 1][x].top;
		bottom = m_boss[y + 1][x].bottom;

		if (top == SPACE && bottom == SPACE) {
			m_boss[y + 1][x].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosY(y + 1);

			showDungeon();
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_boss[y + 1][x].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosY(y + 1);

			showDungeon();
			cout << "You've found ";
			foundItem(x, y + 1);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			fight(x, y + 1);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_boss[y + 1][x].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosY(y + 1);

			showDungeon();
			cout << "It's the idol!\n" << endl;
			return;
		}
		else
			showDungeon();
	}

	if (move == 'd') { // moved down
		top = m_boss[y - 1][x].top;
		bottom = m_boss[y - 1][x].bottom;

		if (top == SPACE && bottom == SPACE) {
			m_boss[y - 1][x].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosY(y - 1);

			showDungeon();
		}
		else if (top == SPACE && bottom != IDOL) {
			// move character on top of weapon, DON'T pickup yet
			m_boss[y - 1][x].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosY(y - 1);

			showDungeon();
			cout << "You've found ";
			foundItem(x, y - 1);
		}
		else if (top != SPACE && top != WALL && top != UNBREAKABLE_WALL) {
			fight(x, y - 1);
		}
		else if (top == SPACE && bottom == IDOL) {
			m_boss[y - 1][x].top = PLAYER;
			m_boss[y][x].top = SPACE;
			m_bossplayer.setPosY(y - 1);

			showDungeon();
			cout << "It's the idol!\n" << endl;
			return;
		}
		else
			showDungeon();
	}

	if (move == 'g') {
		collectItem(x, y);
	}

	if (move == 'w') {
		showDungeon();
		m_bossplayer.wield();
	}

	if (move == 'c') {
		showDungeon();
		m_bossplayer.use(active, m_boss[y][x]);
	}


	// check active items in play
	checkActive();

	if (bossMonsters.empty())
		return;

	moveSmasher(x, y);

	m_bossplayer.rollHeal();
}
void FirstBoss::checkActive() {
	if (!firstbossActive.empty()) {
		for (unsigned i = 0; i < active.size(); i++) {
			if (firstbossActive.at(i)->getItem() == "Bomb") {
				shared_ptr<Bomb> bomb = dynamic_pointer_cast<Bomb>(firstbossActive.at(i));
				if (bomb->getFuse() > 0) {
					bomb->setFuse(bomb->getFuse() - 1);
					firstbossActive.at(i) = bomb;
				}
				else {
					int pos = i;
					firstbossActive.erase(firstbossActive.begin() + pos);

					cout << "A bomb exploded!\n";
					explosion(bomb->getPosX(), bomb->getPosY());
					m_boss[bomb->getPosY()][bomb->getPosX()].bottom = SPACE;
				}
			}
		}
	}
}
void FirstBoss::explosion(int x, int y) { // bomb's coordinates
	int mx, my;
	for (unsigned i = 0; i < bossMonsters.size(); i++) {
		mx = bossMonsters.at(i)->getPosX();
		my = bossMonsters.at(i)->getPosY();
		if (abs(mx - x) <= 1 && abs(my - y) <= 1) {
			bossMonsters.at(i)->setHP(bossMonsters.at(i)->getHP() - 10);
			cout << "The " << bossMonsters.at(i)->getName() << " was blown up!\n";

			if (bossMonsters.at(i)->getHP() <= 0) {
				monsterDeath(mx, my, i);
			}
		}
	}

	//	destroy any nearby walls
	for (int j = y - 1; j < y + 2; j++) {
		for (int k = x - 1; k < x + 2; k++) {
			if (m_boss[j][k].top == WALL) {
				m_boss[j][k].top = SPACE;
				m_boss[j][k].bottom = SPACE;
			}
		}
	}
}
void FirstBoss::monsterDeath(int x, int y, int pos) {
	string boss = bossMonsters.at(pos)->getName();
	cout << boss << " was slain!\n";
	cout << "Well done!\n" << endl;

	Idol idol;
	idol.setPosX(BOSSCOLS / 2);
	idol.setPosY(3);
	m_boss[idol.getPosY()][idol.getPosX()].bottom = IDOL;

	m_boss[y][x].top = SPACE;
	m_boss[y][x].enemy = false;

	bossMonsters.erase(bossMonsters.begin());
}
void FirstBoss::moveSmasher(int x, int y) {
	int g_x = bossMonsters.at(0)->getPosX();
	int g_y = bossMonsters.at(0)->getPosY();
	int n = -1 + randInt(3);
	int m = -1 + randInt(3);

	if (m_boss[g_y + m][g_x + n].top != WALL && m_boss[g_y + m][g_x + n].top != UNBREAKABLE_WALL && !m_boss[g_y + m][g_x + n].enemy) {
		if (m_boss[g_y + m][g_x + n].top != PLAYER) {
			m_boss[g_y + m][g_x + n].top = SMASHER;
			m_boss[g_y + m][g_x + n].enemy = true;
			m_boss[g_y][g_x].top = SPACE;
			m_boss[g_y][g_x].enemy = false;
			bossMonsters.at(0)->setPosX(g_x + n);
			bossMonsters.at(0)->setPosY(g_y + m);
		}
		else {
			bossMonsters.at(0)->encounter(m_bossplayer, *(bossMonsters.at(0)));
			if (m_bossplayer.getHP() <= 0) {
				m_boss[y][x].top = 'X'; //change player icon to X to indicate death
				m_bossplayer.setDeath(bossMonsters.at(0)->getName());
			}
		}
	}
}
void FirstBoss::fight(int x, int y) { // monster's coordinates
	showDungeon();

	unsigned i = 0;
	bool found = false;
	while (!found && i < bossMonsters.size()) { // finds the monster just encountered
		if (bossMonsters.at(i)->getPosX() == x && bossMonsters.at(i)->getPosY() == y)
			found = true;
		else
			i++;
	}
	string monster = bossMonsters.at(i)->getName();
	m_bossplayer.attack(bossMonsters, i);
	if (bossMonsters.at(i)->getHP() <= 0) {
		monsterDeath(x, y, i);
	}
}

void FirstBoss::showDungeon() {
	for (int i = 0; i < BOSSROWS; i++) {
		for (int j = 0; j < BOSSCOLS; j++) {
			if (m_boss[i][j].top == SPACE)
				cout << m_boss[i][j].bottom;
			else
				cout << m_boss[i][j].top;
		}
		cout << endl;
	}
	cout << endl;

	cout << "HP: " << m_bossplayer.getHP() << ", Armor: " << m_bossplayer.getArmor() \
		<< ", Strength: " << m_bossplayer.getStr() << ", Dexterity: " << m_bossplayer.getDex() \
		<< " | Current Weapon: " << m_bossplayer.getWeapon().getAction() << endl;
	cout << endl;
}
Player FirstBoss::getPlayer() const {
	return m_bossplayer;
}