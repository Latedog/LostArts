// Game.cpp

#include "Game.h"
#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include "utilities.h"
#include <iostream>
using namespace std;

// Implement these and other Game member functions you may have added.

Game::Game(int goblinSmellDistance) : m_dungeon(goblinSmellDistance) {
	
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
		p = m_dungeon.player.at(0);
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

		if (p.getHP() <= 0) {		// check if player is dead
			cout << "You were slain by a Goblin!" << endl;
			cout << "Game Over... " << endl;

			c = getCharacter();
			while (c != 'q') {
				m_dungeon.showDungeon();
				cout << "You were slain by a Goblin!" << endl;
				cout << "Game Over... " << endl;

				c = getCharacter();
				clearScreen();
			}
		}
		else if (p.getWin()) {		// check if player picked up the idol
			cout << "You found the golden idol!" << endl;
			cout << "Congratulations!" << endl;

			c = getCharacter();
			while (c != 'q') {
				m_dungeon.showDungeon();
				cout << "You found the golden idol!" << endl;
				cout << "Congratulations!" << endl;

				c = getCharacter();
				clearScreen();
			}
		}
		else {
			c = getCharacter();
			clearScreen();
		}
	}
	cout << "Thanks for playing MiniRogue.\n" << endl;
}

// You will presumably add to this project other .h/.cpp files for the
// various classes of objects you need to play the game:  player, monsters,
// weapons, etc.  You might have a separate .h/.cpp pair for each class
// (e.g., Player.h, Boegeyman.h, etc.), or you might put the class
// declarations for all actors in Actor.h, all game objects in GameObject.h,
// etc.
