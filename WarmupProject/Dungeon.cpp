#include "global.h"
#include "Dungeon.h"
#include "GameObjects.h"
#include "Actors.h"
#include "utilities.h"
#include <iostream>

using namespace std;

Dungeon::Dungeon(){

}
Dungeon::Dungeon(int smelldist) {
	Tile *tile;
	int i, j;
	
	for (i = 0; i < MAXROWS; i++) { //		initialize entire dungeon to blank space
		for (j = 0; j < MAXCOLS; j++) {
			tile = new Tile;
			tile->top = SPACE;
			tile->bottom = SPACE;
			tile->marked = false;

			m_maze[i][j] = *tile;
		}
	}

	for (i = 0; i < MAXROWS; i++) { //	initialize top and bottom of dungeon to be #
		for (j = 0; j < MAXCOLS; j++) {
			m_maze[i][j].top = WALL;
			m_maze[i][j].bottom = WALL;
		}
		i += 16;
	}

	for (i = 1; i < MAXROWS - 1; i++){ //		initialize edges of dungeon to be #
		m_maze[i][0].top = m_maze[i][0].bottom = WALL;
		m_maze[i][69].top = m_maze[i][69].bottom = WALL;
	}

	// RANDOMLY GENERATES LEVEL CHUNKS
	vector<vector<vector<char>>> b1, b2, b3, b4;
	vector<char> v1, v2, v3, v4, finalvec;
	
	v1 = chunks(b1);
	v2 = chunks(b2);
	v3 = chunks(b3);
	v4 = chunks(b4);

	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v1.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v2.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v3.at(i));
	for (i = 0; i < v1.size(); i++)
		finalvec.push_back(v4.at(i));
	
	int count = 0;
	for (i = 1; i < MAXROWS - 1; i++) {
		for (j = 1; j < MAXCOLS - 1; j++) {
			m_maze[i][j].top = finalvec.at(count);
			m_maze[i][j].bottom = finalvec.at(count);
			count++;
		}
	}
	// END LEVEL GENERATION

	Player p;
	player.push_back(p);

	char toptile = m_maze[player.at(0).getPosY()][player.at(0).getPosX()].top;
	while (toptile == WALL) {
		player.at(0).setrandPosX();
		player.at(0).setrandPosY();

		toptile = m_maze[player.at(0).getPosY()][player.at(0).getPosX()].top;
	}
	m_maze[player.at(0).getPosY()][player.at(0).getPosX()].top = PLAYER; // sets tile at this spot to be player position


	Idol idol;
	toptile = m_maze[idol.getPosY()][idol.getPosX()].top;
	while (toptile == PLAYER || toptile == WALL) {
		idol.setrandPosX();
		idol.setrandPosY();

		toptile = m_maze[idol.getPosY()][idol.getPosX()].top;
	}
	m_maze[idol.getPosY()][idol.getPosX()].bottom = IDOL;


	LifePotion lp;
	char bottomtile = m_maze[lp.getPosY()][lp.getPosX()].bottom;
	while (bottomtile == WALL) {
		lp.setrandPosX();
		lp.setrandPosY();

		bottomtile = m_maze[lp.getPosY()][lp.getPosX()].bottom;
	}
	m_maze[lp.getPosY()][lp.getPosX()].bottom = LIFEPOT;


	Chest chest;
	bottomtile = m_maze[chest.getPosY()][chest.getPosX()].bottom;
	while (bottomtile == WALL) {
		chest.setrandPosX();
		chest.setrandPosY();

		bottomtile = m_maze[chest.getPosY()][chest.getPosX()].bottom;
	}
	m_maze[chest.getPosY()][chest.getPosX()].bottom = CHEST;


	int numwep = 1 + randInt(2); // number of weapons to be placed
	//char bottomtile;

	while (numwep > 0) {
		ShortSword s;
		
		bottomtile = m_maze[s.getPosY()][s.getPosX()].bottom;
		while (bottomtile != SPACE) { //while sword position clashes with idol
			s.setrandPosX();				// reroll it
			s.setrandPosY();

			bottomtile = m_maze[s.getPosY()][s.getPosX()].bottom;
		}
		m_maze[s.getPosY()][s.getPosX()].bottom = CUTLASS;
		numwep--;
	}


	int m = 0 + randInt(1); // number of goblins to be placed

	while (m > 0) { //generate goblins
		Goblin g(smelldist);

		toptile = m_maze[g.getPosY()][g.getPosX()].top;
		while (toptile != SPACE) { //while goblin position clashes with wall, player, or idol
			g.setrandPosX();				// reroll it
			g.setrandPosY();

			toptile = m_maze[g.getPosY()][g.getPosX()].top;
		}

		goblins.push_back(g);
		m_maze[g.getPosY()][g.getPosX()].top = GOBLIN;
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
	//delete[] m_maze; //WATCH FOR THIS!!!!!!
}

void Dungeon::peekDungeon(int x, int y, char move) {
	char top, bottom;

	//move player
	if (move == 'l') { // moved to the left
		top = m_maze[y][x - 1].top;
		bottom = m_maze[y][x - 1].bottom;

		if (top == ' ' && bottom == ' ') {
			// move character to the left
			m_maze[y][x - 1].top = '@';
			m_maze[y][x].top = ' ';
			player.at(0).setPosX(x-1);

			showDungeon();
		}
		else if (top == ' ' && bottom != '&') {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y][x - 1].top = '@';
			m_maze[y][x].top = ' ';
			player.at(0).setPosX(x - 1);

			showDungeon();
			cout << "You've found a ";
			foundItem(x - 1, y);
			
			/*
			char c = m_maze[y][x - 1].bottom;
			switch (c) {
			case ')':
				cout << "Rusty Cutlass.\n" << endl;
				break;
			case 'q':
				cout << "Bone Axe.\n" << endl;
				break;
			default:
				break;
			}*/
		}
		else if (top == 'G') {
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
				m_maze[y][x - 1].top = ' ';

				goblins.erase(goblins.begin() + i);
			}
		}
		else if (top == ' ' && bottom == '&') {
			m_maze[y][x - 1].top = '@';
			m_maze[y][x].top = ' ';
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

		if (top == ' ' && bottom == ' ') {
			// move character to the left
			m_maze[y][x + 1].top = '@';
			m_maze[y][x].top = ' ';
			player.at(0).setPosX(x + 1);

			showDungeon();
		}
		else if (top == ' ' && bottom != '&') {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y][x + 1].top = '@';
			m_maze[y][x].top = ' ';
			player.at(0).setPosX(x + 1);

			showDungeon();
			cout << "You've found a ";
			foundItem(x + 1, y);
		}
		else if (top == 'G') {
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
				m_maze[y][x + 1].top = ' ';
				goblins.erase(goblins.begin() + i);
			}
		}
		else if (top == ' ' && bottom == '&') {
			m_maze[y][x + 1].top = '@';
			m_maze[y][x].top = ' ';
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

		if (top == ' ' && bottom == ' ') {
			m_maze[y + 1][x].top = '@';
			m_maze[y][x].top = ' ';
			player.at(0).setPosY(y + 1);

			showDungeon();
		}
		else if (top == ' ' && bottom != '&') {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y + 1][x].top = '@';
			m_maze[y][x].top = ' ';
			player.at(0).setPosY(y + 1);

			showDungeon();
			cout << "You've found a ";
			foundItem(x, y + 1);
		}
		else if (top == 'G') {
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
				m_maze[y+1][x].top = ' ';
				goblins.erase(goblins.begin() + i);
			}
		}
		else if (top == ' ' && bottom == '&') {
			m_maze[y + 1][x].top = '@';
			m_maze[y][x].top = ' ';
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

		if (top == ' ' && bottom == ' ') {
			m_maze[y - 1][x].top = '@';
			m_maze[y][x].top = ' ';
			player.at(0).setPosY(y - 1);

			showDungeon();
		}
		else if (top == ' ' && bottom != '&') {
			// move character on top of weapon, DON'T pickup yet
			m_maze[y - 1][x].top = '@';
			m_maze[y][x].top = ' ';
			player.at(0).setPosY(y - 1);

			showDungeon();
			cout << "You've found a ";
			foundItem(x, y - 1);
		}
		else if (top == 'G') {
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
				m_maze[y-1][x].top = ' ';
				goblins.erase(goblins.begin() + i);
			}
		}
		else if (top == ' ' && bottom == '&') {
			m_maze[y - 1][x].top = '@';
			m_maze[y][x].top = ' ';
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
		player.at(0).use();
	}
	

	// move goblins
	int g_x, g_y, shortest;
	char first, best, prev;

	for (unsigned i = 0; i < goblins.size(); i++) {
		g_x = goblins.at(i).getPosX();
		g_y = goblins.at(i).getPosY();
		first = '0'; best = '0'; prev = '0';
		shortest = 0;

		if (goblinInRange(i)) {
			if (moveGoblins(i, shortest, goblins.at(i).getSmellDistance(), g_x, g_y, \
				first, best, prev)) { // viable path is found
				switch (best) {
				case 'l':
					m_maze[g_y][g_x - 1].top = 'G';
					m_maze[g_y][g_x].top = ' ';
					goblins.at(i).setPosX(g_x - 1);
					break;
				case 'r':
					m_maze[g_y][g_x + 1].top = 'G';
					m_maze[g_y][g_x].top = ' ';
					goblins.at(i).setPosX(g_x + 1);
					break;
				case 'u':
					m_maze[g_y + 1][g_x].top = 'G';
					m_maze[g_y][g_x].top = ' ';
					goblins.at(i).setPosY(g_y + 1);
					break;
				case 'd':
					m_maze[g_y - 1][g_x].top = 'G';
					m_maze[g_y][g_x].top = ' ';
					goblins.at(i).setPosY(g_y - 1);
					break;
				default:
					goblins.at(i).attack(player.at(0));
					if (player.at(0).getHP() <= 0) {
						m_maze[y][x].top = 'X'; //change player icon to X to indicate death
					}
					break;
				}
			}
		}
	}
	// roll heal chance
	player.at(0).rollHeal();
}

bool Dungeon::moveGoblins(int pos, int &shortest, int smelldist, int x, int y, \
	char &first_move, char &optimal_move, char prev) {
	
	if (smelldist == goblins.at(pos).getSmellDistance() && \
		(m_maze[y - 1][x].top == '@' || m_maze[y + 1][x].top == '@' || m_maze[y][x - 1].top == '@' || m_maze[y][x + 1].top == '@')) {
		return true; //	if player is immediately adjacent to the goblin, return to attack
	}
	
	if (smelldist < 0) // player was not found within getSmellDistance() steps
		return false;

	if (m_maze[y][x].top == '@') {
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

	if (prev != 'l' && m_maze[y][x+1].top != '#' && m_maze[y][x+1].top != 'G') {
		if (smelldist == goblins.at(pos).getSmellDistance())
			first_move = 'r';
		moveGoblins(pos, shortest, smelldist - 1, x + 1, y, first_move, optimal_move, 'r');
	}
	
	if (prev != 'r' && m_maze[y][x-1].top != '#' && m_maze[y][x-1].top != 'G') {
		if (smelldist == goblins.at(pos).getSmellDistance())
			first_move = 'l';
		moveGoblins(pos, shortest, smelldist - 1, x - 1, y, first_move, optimal_move, 'l');
	}

	if (prev != 'd' && m_maze[y+1][x].top != '#' && m_maze[y+1][x].top != 'G') {
		if (smelldist == goblins.at(pos).getSmellDistance())
			first_move = 'u';
		moveGoblins(pos, shortest, smelldist - 1, x, y + 1, first_move, optimal_move, 'u');
	}

	if (prev != 'u' && m_maze[y-1][x].top != '#' && m_maze[y-1][x].top != 'G') {
		if (smelldist == goblins.at(pos).getSmellDistance())
			first_move = 'd';
		moveGoblins(pos, shortest, smelldist - 1, x, y - 1, first_move, optimal_move, 'd');
	}

	if (optimal_move != '0')
		return true;
	return false;
}
bool Dungeon::goblinInRange(int pos) {
	int px = player.at(0).getPosX();
	int gx = goblins.at(pos).getPosX();

	int py = player.at(0).getPosY();
	int gy = goblins.at(pos).getPosY();

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
void Dungeon::foundItem(int x, int y) {
	char c = m_maze[y][x].bottom;
	switch (c) {
	case CUTLASS:
		cout << "Rusty Cutlass.\n" << endl;
		break;
	case BONEAXE:
		cout << "Bone Axe.\n" << endl;
		break;
	case LIFEPOT:
		cout << "Life Potion.\n" << endl;
		break;
	case ARMOR:
		cout << "Extra Armor.\n" << endl;
		break;
	case STATPOT:
		cout << "Stat Potion.\n" << endl;
		break;
	case CHEST:
		cout << "Chest!\n" << endl;
		break;
	default:
		break;
	}
}
void Dungeon::collectItem(int x, int y) {
	if (m_maze[y][x].bottom == CUTLASS) {
		if (player.at(0).getInventorySize() <= 25) {
			player.at(0).addInventory(RustyCutlass(x, y)); //adds short sword to inventory
			m_maze[y][x].bottom = SPACE;

			showDungeon();
			cout << "You pick up the Rusty Cutlass.\n" << endl;
		}
		else {
			cout << "Your backpack is full!\n" << endl;
		}
	}
	else if (m_maze[y][x].bottom == BONEAXE) {
		if (player.at(0).getInventorySize() <= 25) {
			player.at(0).addInventory(BoneAxe(x,y)); //adds bone axe to inventory
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

		cout << "Victory is yours!" << endl;
	}
	else if (m_maze[y][x].bottom == LIFEPOT) {
		if (player.at(0).getItemInvSize() <= 10) {
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
		if (player.at(0).getItemInvSize() <= 10) {
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
		if (player.at(0).getItemInvSize() <= 10) {
			player.at(0).addItem(StatPotion());
			m_maze[y][x].bottom = SPACE;

			showDungeon();
			cout << "You grab the Stat Potion!" << endl;
		}
		else {
			cout << "You can't hold any more items." << endl;
		}
	}
	else if (m_maze[y][x].bottom == CHEST) {
		Chest c;

		showDungeon();
		cout << "You open the chest... ";
		c.open(m_maze[y][x]);
	}
	else {
		showDungeon();
		cout << "There's nothing in the dirt... \n" << endl;
	}
}
void Dungeon::showDungeon() {
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 70; j++) {
			if (m_maze[i][j].top == ' ')
				cout << m_maze[i][j].bottom;
			else
				cout << m_maze[i][j].top;
		}
		cout << endl;
	}
	cout << endl;

	cout << "HP: " << player.at(0).getHP() << ", Armor: " << player.at(0).getArmor() \
		<< ", Strength: " << player.at(0).getStr() << ", Dexterity: " << player.at(0).getDex() \
		<< " | Current Weapon: " << player.at(0).getWeapon().getAction() << endl;
	cout << endl;
}

void Dungeon::unmarkTiles() {
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 70; j++) {
			m_maze[i][j].marked = false;
		}
	}
}


vector<char> chunks(vector<vector<vector<char>>> &c) {
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

	vector<vector<char>> seven = { {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '},
								  {' ', ' ', ' ', ' '} };

	vector<vector<char>> eight = { {' ', ' ', '#', '#'},
								   {' ', ' ', ' ', '#'},
								   {'#', '#', ' ', ' '},
								   {'#', '#', ' ', ' '} };

	c.push_back(one);
	c.push_back(two);
	c.push_back(three);
	c.push_back(four);
	c.push_back(five);
	c.push_back(six);
	c.push_back(seven);
	c.push_back(eight);

	//vector<vector<vector<char>>> v;
	c = mixChunks(c);

	//vector<char> combinedChunks = combineChunks(c);
	return combineChunks(c);
}
vector<vector<vector<char>>> mixChunks(vector<vector<vector<char>>> c) {
	vector<vector<vector<char>>> v;

	for (int i = 0; i < 17; i++)
		v.push_back(c[randInt(8)]);

	return v;
}
vector<char> combineChunks(vector<vector<vector<char>>> &c) {
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
