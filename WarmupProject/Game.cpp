// Game.cpp

#include "global.h"
#include "Game.h"
#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include "utilities.h"
#include <iostream>
using namespace std;

Game::Game() {

}

void Game::play()
{
	m_dungeon.showDungeon();

	cout << "       ";
	cout << "Press h for controls, q to exit game, or r to reset.\n" << endl;
	char c = getCharacter();
	clearScreen();

	Player p;
	int x, y;
	while (c != 'q') {
		//		FIRST LEVEL
		while (c != 'q') {
			p = m_dungeon.getPlayer();
			x = p.getPosX(); y = p.getPosY();

			if (c == ARROW_LEFT)
				m_dungeon.peekDungeon(x, y, 'l');
			if (c == ARROW_DOWN)
				m_dungeon.peekDungeon(x, y, 'd');
			if (c == ARROW_RIGHT)
				m_dungeon.peekDungeon(x, y, 'r');
			if (c == ARROW_UP)
				m_dungeon.peekDungeon(x, y, 'u');

			if (c == 'i') {
				m_dungeon.showDungeon();
				p.showInventory();
				m_dungeon.peekDungeon(0, 0, '-'); // calls move goblins
			}

			if (c == 'w') {
				//	wield command
				m_dungeon.peekDungeon(0, 0, 'w');
			}

			if (c == 'g') {
				//	pickup command
				m_dungeon.peekDungeon(x, y, 'g');
			}

			if (c == 'c') {
				//	items use menu
				m_dungeon.peekDungeon(x, y, 'c');
			}

			if (c == 'r') {
				reset();
			}

			if (c == 'h')
				help(m_dungeon);

			if (m_dungeon.getPlayer().getHP() <= 0) {		// check if player is dead
				if (!gameOver(m_dungeon.getPlayer(), m_dungeon)) {
					c = 'q';
					break;
				}
				clearScreen();
				reset();

				c = getCharacter();
				clearScreen();
				break;
			}
			else if (m_dungeon.getPlayer().getWin()) {		// check if player picked up the idol
				if (!win(m_dungeon)) {
					c = 'q';
					break;
				}
				clearScreen();
				reset();

				c = getCharacter();
				clearScreen();
				break;
			}
			else if (m_dungeon.getLevel() != 1) {
				p = m_dungeon.getPlayer();
				break;
			}
			else {
				c = getCharacter();
				clearScreen();
			}
		}

		//		BOSS LEVEL
		if (m_dungeon.getLevel() == 2 && c != 'q') {
			FirstBoss firstBoss(m_dungeon.getPlayer());

			firstBoss.showDungeon();
			c = getCharacter();
			clearScreen();

			while (c != 'q') {
				p = firstBoss.getPlayer();
				x = p.getPosX(); y = p.getPosY();

				if (c == ARROW_LEFT)
					firstBoss.peekFirstBossDungeon(x, y, 'l');
				if (c == ARROW_DOWN)
					firstBoss.peekFirstBossDungeon(x, y, 'd');
				if (c == ARROW_RIGHT)
					firstBoss.peekFirstBossDungeon(x, y, 'r');
				if (c == ARROW_UP)
					firstBoss.peekFirstBossDungeon(x, y, 'u');

				if (c == 'i') {
					firstBoss.showDungeon();
					p.showInventory();
					firstBoss.peekFirstBossDungeon(0, 0, '-'); // moves enemies
				}

				if (c == 'w') {
					//	wield command
					firstBoss.peekFirstBossDungeon(0, 0, 'w');
				}

				if (c == 'g') {
					//	pickup command
					firstBoss.peekFirstBossDungeon(x, y, 'g');
				}

				if (c == 'c') {
					//	items use menu
					firstBoss.peekFirstBossDungeon(x, y, 'c');
				}

				if (c == 'r') {
					reset();
					c = getCharacter();
					clearScreen();

					break;
				}

				if (c == 'h')
					help(firstBoss);

				if (firstBoss.getPlayer().getHP() <= 0) {		// check if player is dead
					if (!gameOver(firstBoss.getPlayer(), firstBoss)) {
						c = 'q';
						break;
					}
					clearScreen();
					reset();

					c = getCharacter();
					clearScreen();
					break;
				}
				else if (firstBoss.getPlayer().getWin()) {		// check if player picked up the idol
					if (!win(firstBoss)) {
						c = 'q';
						break;
					}
					clearScreen();
					reset();

					c = getCharacter();
					clearScreen();
					break;
				}
				else {
					c = getCharacter();
					clearScreen();
				}
			}
		}
		
	}

	cout << "Thanks for playing Super Mini Rogue.\n" << endl;
}
bool Game::gameOver(Player p, Dungeon &dungeon) {
	char c;

	cout << "You were slain by ";
	if (p.getDeath()[0] == 'a' || p.getDeath()[0] == 'e' || p.getDeath()[0] == 'i' || p.getDeath()[0] == 'o' || p.getDeath()[0] == 'u')
		cout << "an " << p.getDeath() << "!" << endl;
	else
		cout << "a " << p.getDeath() << "!" << endl;

	cout << "Game Over... " << endl;

	c = getCharacter();
	clearScreen();
	while (c != 'q' && c != 'r') {
		dungeon.showDungeon();
		cout << "You were slain by ";
		if (p.getDeath()[0] == 'a' || p.getDeath()[0] == 'e' || p.getDeath()[0] == 'i' || p.getDeath()[0] == 'o' || p.getDeath()[0] == 'u')
			cout << "an " << p.getDeath() << "!" << endl;
		else
			cout << "a " << p.getDeath() << "!" << endl;

		cout << "Game Over... " << endl;

		c = getCharacter();
		clearScreen();
	}
	if (c == 'r')
		return true;
	return false;
}
bool Game::win(Dungeon &dungeon) {
	char c;

	cout << "You found the golden idol!" << endl;
	cout << "Congratulations!" << endl;

	c = getCharacter();
	clearScreen();
	while (c != 'q' && c != 'r') {
		dungeon.showDungeon();
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
	Dungeon* d = new Dungeon();
	m_dungeon = *d;
	delete d;

	m_dungeon.showDungeon();
}
void Game::help(Dungeon dungeon) {
	cout << "How to play:\n" << endl;
	cout << "Move with the arrow keys.\n";
	cout << "i: Show inventory\n";
	cout << "g: Collect item\n";
	cout << "w: Equip weapon\n";
	cout << "c: Use item\n" << endl;

	cout << "q to quit and r to reset\n" << endl;

	char c;
	c = getCharacter();

	while (c != 'h') {
		cout << "How to play:\n" << endl;
		cout << "Move with the arrow keys.\n";
		cout << "i: Show inventory\n";
		cout << "g: Collect item\n";
		cout << "w: Equip weapon\n";
		cout << "c: Use item\n" << endl;

		cout << "q to quit and r to reset\n" << endl;

		c = getCharacter();
		clearScreen();
	}
	clearScreen();
	dungeon.showDungeon();
}
