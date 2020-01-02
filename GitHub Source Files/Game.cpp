// Game.cpp

#include "cocos2d.h"
#include "AppDelegate.h"
#include "global.h"
#include "Game.h"
#include "Dungeon.h"
#include "Actors.h"
#include "GameObjects.h"
#include "utilities.h"
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::setw;

Dungeon DUNGEON;
//SecondFloor DUNGEON2(DUNGEON.getPlayer());

Game::Game() : quit(false) {

}

void Game::play() {
	m_dungeon.showDungeon();

	std::cout << "\n\n\n";
	std::cout << std::setw(63) << "Press H for controls, or P/ESC to pause\n" << endl;

	_Timer timer;
	char c;

	c = getCharacter();
	clearScreen();

	Player p;
	int x, y;

	while (!quit) {
		
		//		FIRST LEVEL
		while (!quit) {
			p = m_dungeon.getPlayer();
			x = p.getPosX(); y = p.getPosY();

			if (c == ARROW_LEFT)
				m_dungeon.peekDungeon(x, y, 'l');
			else if (c == ARROW_DOWN)
				m_dungeon.peekDungeon(x, y, 'd');
			else if (c == ARROW_RIGHT)
				m_dungeon.peekDungeon(x, y, 'r');
			else if (c == ARROW_UP)
				m_dungeon.peekDungeon(x, y, 'u');

			else if (c == 'i') {
				p.showInventory();
				waitForInverse(c);
				m_dungeon.peekDungeon(0, 0, '-'); // moves monsters
			}

			else if (c == 'w') {
				//	wield command
				m_dungeon.peekDungeon(0, 0, 'w');
			}

			else if (c == 'e') {
				//	pickup command
				m_dungeon.peekDungeon(x, y, 'e');
			}

			else if (c == 'c') {
				//	items use menu
				m_dungeon.peekDungeon(x, y, 'c');
			}

			/*else if (c == 'r') {
				reset();
				c = tolower(getCharacter());
				clearScreen();
				continue;
			}*/

			else if (c == 'h')
				help(m_dungeon, c);

			else if (c == ESC || c == PAUSE) {
				pause(c);
				if (c == 'r') {
					reset();
					c = getCharacter();
					clearScreen();
					continue;
				}
				else if (c == 'q') {
					quit = true;
					clearScreen();
					break;
				}
				m_dungeon.peekDungeon(x, y, '-');
			}

			// null move from no action
			else {
				m_dungeon.peekDungeon(x, y, '-');
			}

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
			else if (m_dungeon.getLevel() != 1) {
				p = m_dungeon.getPlayer();
				break;
			}
			else {
				timer.start();
				while (timer.elapsed() < 750) {
					if (getCharIfAny(c))
						break;
					else
						c = '-';
				}
				//c = getCharacter();
				clearScreen();
			}
		}

		//		SECOND LEVEL
		if (m_dungeon.getLevel() == 2 && !quit) {
			SecondFloor secondfloor(m_dungeon.getPlayer());

			secondfloor.showDungeon();
			c = getCharacter();
			clearScreen();

			while (!quit) {
				p = secondfloor.getPlayer();
				x = p.getPosX(); y = p.getPosY();

				if (c == ARROW_LEFT)
					secondfloor.peekSecondFloor(x, y, 'l');
				else if (c == ARROW_DOWN)
					secondfloor.peekSecondFloor(x, y, 'd');
				else if (c == ARROW_RIGHT)
					secondfloor.peekSecondFloor(x, y, 'r');
				else if (c == ARROW_UP)
					secondfloor.peekSecondFloor(x, y, 'u');

				else if (c == 'i') {
					p.showInventory();
					waitForInverse(c);
					secondfloor.peekSecondFloor(x, y, '-'); // moves enemies
				}

				else if (c == 'w') {
					//	wield command
					secondfloor.peekSecondFloor(x, y, 'w');
				}

				else if (c == 'e') {
					//	pickup command
					secondfloor.peekSecondFloor(x, y, 'e');
				}

				else if (c == 'c') {
					//	items use menu
					secondfloor.peekSecondFloor(x, y, 'c');
				}

				else if (c == 'r') {
					reset();
					c = getCharacter();
					clearScreen();

					break;
				}

				else if (c == 'h')
					help(secondfloor, c);

				else if (c == ESC || c == PAUSE) {
					pause(c);
					if (c == 'r') {
						reset();
						c = getCharacter();
						clearScreen();
						break;
					}
					else if (c == 'q') {
						quit = true;
						clearScreen();
						break;
					}
					secondfloor.peekSecondFloor(x, y, '-');
				}

				else {
					secondfloor.peekSecondFloor(x, y, '-');
				}


				if (secondfloor.getPlayer().getHP() <= 0) {		// check if player is dead
					if (!gameOver(secondfloor.getPlayer(), secondfloor)) {
						c = 'q';
						break;
					}
					clearScreen();
					reset();

					c = getCharacter();
					clearScreen();
					break;
				}
				else if (secondfloor.getLevel() != 2) {
					//p = secondfloor.getPlayer();
					m_dungeon.setPlayer(p);
					m_dungeon.setLevel(3);
					break;
				}
				else {
					timer.start();
					while (timer.elapsed() < 700) {
						if (getCharIfAny(c))
							break;
						else
							c = '-';
					}
					//c = getCharacter();
					clearScreen();
				}
			}
		}

		//		THIRD LEVEL
		if (m_dungeon.getLevel() == 3 && !quit) {
			ThirdFloor thirdfloor(m_dungeon.getPlayer());

			thirdfloor.showDungeon();
			c = getCharacter();
			clearScreen();

			while (!quit) {
				p = thirdfloor.getPlayer();
				x = p.getPosX(); y = p.getPosY();

				if (c == ARROW_LEFT)
					thirdfloor.peekThirdFloor(x, y, 'l');
				else if (c == ARROW_DOWN)
					thirdfloor.peekThirdFloor(x, y, 'd');
				else if (c == ARROW_RIGHT)
					thirdfloor.peekThirdFloor(x, y, 'r');
				else if (c == ARROW_UP)
					thirdfloor.peekThirdFloor(x, y, 'u');

				else if (c == 'i') {
					p.showInventory();
					waitForInverse(c);
					thirdfloor.peekThirdFloor(x, y, '-'); // moves enemies
				}

				else if (c == 'w') {
					//	wield command
					thirdfloor.peekThirdFloor(x, y, 'w');
				}

				else if (c == 'e') {
					//	pickup command
					thirdfloor.peekThirdFloor(x, y, 'e');
				}

				else if (c == 'c') {
					//	items use menu
					thirdfloor.peekThirdFloor(x, y, 'c');
				}

				else if (c == 'h') {
					help(thirdfloor, c);
				}
				else if (c == ESC || c == PAUSE) {
					pause(c);
					if (c == 'r') {
						reset();
						c = getCharacter();
						clearScreen();
						break;
					}
					else if (c == 'q') {
						quit = true;
						clearScreen();
						break;
					}
					thirdfloor.peekThirdFloor(x, y, '-');
				}

				else {
					thirdfloor.peekThirdFloor(x, y, '-');
				}

				if (thirdfloor.getPlayer().getHP() <= 0) {		// check if player is dead
					if (!gameOver(thirdfloor.getPlayer(), thirdfloor)) {
						c = 'q';
						break;
					}
					clearScreen();
					reset();

					c = getCharacter();
					clearScreen();
					break;
				}
				else if (thirdfloor.getLevel() != 3) {
					//p = thirdfloor.getPlayer();
					m_dungeon.setPlayer(p);
					m_dungeon.setLevel(4);
					break;
				}
				else {
					timer.start();
					while (timer.elapsed() < 750) {
						if (getCharIfAny(c))
							break;
						else
							c = '-';
					}
					//c = getCharacter();
					clearScreen();
				}
			}
		}

		//		BOSS LEVEL
		if (m_dungeon.getLevel() == 4 && !quit) {
			FirstBoss firstBoss(p);

			firstBoss.showDungeon();
			c = getCharacter();
			clearScreen();

			while (!quit) {
				p = firstBoss.getPlayer();
				x = p.getPosX(); y = p.getPosY();

				if (c == ARROW_LEFT)
					firstBoss.peekFirstBossDungeon(x, y, 'l');
				else if (c == ARROW_DOWN)
					firstBoss.peekFirstBossDungeon(x, y, 'd');
				else if (c == ARROW_RIGHT)
					firstBoss.peekFirstBossDungeon(x, y, 'r');
				else if (c == ARROW_UP)
					firstBoss.peekFirstBossDungeon(x, y, 'u');

				else if (c == 'i') {
					p.showInventory();
					waitForInverse(c);
					firstBoss.peekFirstBossDungeon(x, y, '-'); // moves enemies
				}

				else if (c == 'w') {
					//	wield command
					firstBoss.peekFirstBossDungeon(x, y, 'w');
				}

				else if (c == 'e') {
					//	pickup command
					firstBoss.peekFirstBossDungeon(x, y, 'e');
				}

				else if (c == 'c') {
					//	items use menu
					firstBoss.peekFirstBossDungeon(x, y, 'c');
				}

				else if (c == 'h') {
					help(firstBoss, c);
				}
				else if (c == ESC || c == PAUSE) {
					pause(c);
					if (c == 'r') {
						reset();
						c = getCharacter();
						clearScreen();
						break;
					}
					else if (c == 'q') {
						quit = true;
						clearScreen();
						break;
					}
					firstBoss.peekFirstBossDungeon(x, y, '-');
				}

				else {
					firstBoss.peekFirstBossDungeon(x, y, '-');
				}


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
					timer.start();
					while (timer.elapsed() < 250) {
						if (getCharIfAny(c))
							break;
						else
							c = '-';
					}
					//c = getCharacter();
					clearScreen();
				}
			}
		}
	}

	std::cout << "Thanks for playing Super Mini Rogue.\n" << endl;
}
bool Game::gameOver(Player p, Dungeon &dungeon) {
	char c;

	cout << "You were killed by ";
	if (p.getDeath()[0] == 'a' || p.getDeath()[0] == 'e' || p.getDeath()[0] == 'i' || p.getDeath()[0] == 'o' || p.getDeath()[0] == 'u')
		cout << "an " << p.getDeath() << "!" << endl;
	else
		cout << "a " << p.getDeath() << "!" << endl;

	cout << "Game Over... " << endl;

	c = getCharacter();
	clearScreen();
	while (c != 'q' && c != 'r') {
		dungeon.showDungeon();
		cout << "You were killed by ";
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
	//Dungeon* d = new Dungeon;
	Dungeon d;
	m_dungeon = d;
	//m_dungeon = *d;
	//delete d;

	m_dungeon.showDungeon();
}
void Game::help(Dungeon dungeon, char c) {
	cout << "\n\n\n\n\n\n\n";
	cout << setw(65) << "How to play:             \n" << endl;
	cout << setw(60) << "Move with the arrow keys.\n";
	cout << setw(60) << "I: Show inventory        \n";
	cout << setw(60) << "E: Collect item          \n";
	cout << setw(60) << "W: Equip weapon          \n";
	cout << setw(60) << "C: Use item              \n" << endl;

	cout << setw(67) << "P/ESC to pause               \n";

	waitForInverse(c);

	clearScreen();
	dungeon.showDungeon();
}
void Game::pause(char &c) {
	cout << "\n\n\n\n\n\n\n";
	cout << setw(50) << "PAUSED\n\n";

	cout << setw(65) << "Press Q to quit, R to reset, or unpause.\n";

	waitForPause(c);
	clearScreen();
}

