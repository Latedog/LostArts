// Game.cpp

#include "global.h"
#include "Game.h"
#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include "utilities.h"
#include <iostream>
using namespace std;

Game::Game(int goblinSmellDistance) : m_dungeon(goblinSmellDistance), m_smelldist(goblinSmellDistance) {

}

int Game::getSmellDistance() const {
	return m_smelldist;
}

void Game::play()
{
	m_dungeon.showDungeon();
	cout << "Press q to exit game.\n" << endl;
	char c = getCharacter();
	clearScreen();

	Player p;
	int x, y;
	while (c != 'q') {
		p = m_dungeon.getPlayer();
		x = p.getPosX(); y = p.getPosY();

		if (c == ARROW_LEFT)
			m_dungeon.peekDungeon(x, y, 'l');
		if (c == ARROW_DOWN) 
			m_dungeon.peekDungeon(x, y, 'u');
		if (c == ARROW_RIGHT) 
			m_dungeon.peekDungeon(x, y, 'r');
		if (c == ARROW_UP) 
			m_dungeon.peekDungeon(x, y, 'd');
		
		if (c == 'i') {
			m_dungeon.showDungeon();
			p.showInventory();
			m_dungeon.peekDungeon(0, 0, '/0'); // calls move goblins
		}

		if (c == 'w') {
			//	wield command
			m_dungeon.peekDungeon(0, 0, 'w');
		}
		
		if (c == 'g'){
			m_dungeon.peekDungeon(x, y, 'g');
		}

		if (c == 'c') {
			m_dungeon.peekDungeon(x, y, 'c');
		}

		if (c == 'r') {
			reset();
		}

		if (m_dungeon.getPlayer().getHP() <= 0) {		// check if player is dead
			if (!gameOver())
				break;
			clearScreen();
			reset();

			c = getCharacter();
			clearScreen();
		}
		else if (m_dungeon.getPlayer().getWin()) {		// check if player picked up the idol
			if (!win())
				break;
			clearScreen();
			reset();
			
			c = getCharacter();
			clearScreen();
		}
		else {
			c = getCharacter();
			clearScreen();
		}
	}

	cout << "Thanks for playing Super Mini Rogue.\n" << endl;
}

bool Game::gameOver() {
	char c;

	cout << "You were slain by a Goblin!" << endl;
	cout << "Game Over... " << endl;

	c = getCharacter();
	clearScreen();
	while (c != 'q' && c != 'r') {
		m_dungeon.showDungeon();
		cout << "You were slain by a Goblin!" << endl;
		cout << "Game Over... " << endl;

		c = getCharacter();
		clearScreen();
	}
	if (c == 'r')
		return true;
	return false;
}
bool Game::win() {
	char c;

	cout << "You found the golden idol!" << endl;
	cout << "Congratulations!" << endl;

	c = getCharacter();
	clearScreen();
	while (c != 'q' && c != 'r') {
		m_dungeon.showDungeon();
		cout << "You found the golden idol!" << endl;
		cout << "Congratulations!" << endl;

		c = getCharacter();
		clearScreen();
	}
	if (c == 'r')
		return true;
	return false;
}
void Game::reset() {
	Dungeon* d = new Dungeon(getSmellDistance());
	m_dungeon = *d;
	delete d;

	m_dungeon.showDungeon();
}

