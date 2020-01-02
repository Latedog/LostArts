// utilities.h

// Note:  YOU MUST NOT MAKE ANY CHANGE TO THIS FILE!

#ifndef UTILITIES_INCLUDED
#define UTILITIES_INCLUDED

//const char ARROW_LEFT  = 'h';
//const char ARROW_RIGHT = 'l';
//const char ARROW_UP    = 'k';
//const char ARROW_DOWN  = 'j';

const char ARROW_LEFT = 75;
const char ARROW_RIGHT = 77;
const char ARROW_UP = 72;
const char ARROW_DOWN = 80;

char getCharacter();
void clearScreen();
int randInt(int limit);             // random int from 0 to limit-1
bool trueWithProbability(double p); // return true with probability p

bool getCharIfAny(char& ch);
void waitForEnter();
void waitForInverse(char c);
void waitForPause(char &c);
void discardPendingKeys();

#include <chrono>

class _Timer
{
public:
	_Timer() {
		start();
	}
	void start() {
		m_time = std::chrono::high_resolution_clock::now();
	}
	double elapsed() const {
		std::chrono::duration<double, std::milli> diff =
			std::chrono::high_resolution_clock::now() - m_time;
		return diff.count();
	}
private:
	std::chrono::high_resolution_clock::time_point m_time;
};



/*
#include <string>
class ScreenImpl;

class Screen
{
public:
	Screen(int width, int height);
	~Screen();
	void clear();
	void gotoXY(int x, int y);
	void printChar(char ch);
	void printString(std::string s);
	void printStringClearLine(std::string s);
	void refresh();

private:
	ScreenImpl* m_impl;
};*/

#endif // UTILITIES_INCLUDED
