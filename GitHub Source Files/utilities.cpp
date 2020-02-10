// utilities.cpp

// Note:  YOU MUST NOT MAKE ANY CHANGE TO THIS FILE!

#include "utilities.h"
#include <cstdlib>

  // Return a uniformly distributed random integer from 0 to limit-1 inclusive
int randInt(int limit)
{
    return std::rand() % limit;
}

  // return true with a given probability
bool trueWithProbability(double p)
{
    return rand() < p * RAND_MAX + p; 
}

#ifdef _MSC_VER  //  Microsoft Visual C++

#include <windows.h>
#include <conio.h>

char getCharacter()
{
    int c = _getch();
    if (c != 0xE0  &&  c != 0x00)  // first of the two sent by arrow keys
        return c;
    else
    {
        c = _getch();
        switch (c)
        {
          case 'K':  return ARROW_LEFT;
          case 'M':  return ARROW_RIGHT;
          case 'H':  return ARROW_UP;
          case 'P':  return ARROW_DOWN;
          default:   return '?';
        }
    }
}

bool getCharIfAny(char& ch)
{
	if (_kbhit())
	{
		int c = _getch();
		if (c == 0xE0 || c == 0x00)  // first of the two sent by arrow keys
		{
			c = _getch();
			switch (c)
			{
			case 'K':  ch = ARROW_LEFT;  break;
			case 'M':  ch = ARROW_RIGHT; break;
			case 'H':  ch = ARROW_UP;    break;
			case 'P':  ch = ARROW_DOWN;  break;
			case 'w':  ch = 'w';  break;
			case 'e':  ch = 'e';  break;
			case 'c':  ch = 'c';  break;
			case 'i':  ch = 'i';  break;
			default:   ch = '?';         break;
			}
		}
		else
		{
			/*	switch (c)
				{
				case 'a':  ch = ARROW_LEFT;  break;
				case 'd':  ch = ARROW_RIGHT; break;
				case 'w':  ch = ARROW_UP;    break;
				case 's':  ch = ARROW_DOWN;  break;
				default:   ch = c;           break;
				}*/
			switch (c)
			{
			case 'w':  ch = 'w';  break;
			case 'e':  ch = 'e';  break;
			case 'c':  ch = 'c';  break;
			case 'i':  ch = 'i';  break;
			default:   ch = c;           break;
			}
		}
		return true;
	}
	return false;
}

void waitForEnter()
{
	int ch;
	while (ch = _getch(), ch != '\n'  &&  ch != '\r' && ch != ' ')
		;
}

void waitForInverse(char c)
{
	int ch;
	while (ch = _getch(), ch != '\n'  &&  ch != '\r' && ch != '\x1B' && ch != c)
		;
}

void waitForPause(char &c)
{
	int ch;
	while (ch = _getch(), ch != '\n'  &&  ch != '\r' && ch != '\x1B' && ch != ' ' && ch != 27 && ch != 'p' &&  ch != 'q' && ch != 'r')
		;
	c = ch;
}

void discardPendingKeys()
{
	char ch;
	while (getCharIfAny(ch))
		;
}

void clearScreen()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD upperLeft = { 0, 0 };
    DWORD dwCharsWritten;
    FillConsoleOutputCharacter(hConsole, TCHAR(' '), dwConSize, upperLeft,
							&dwCharsWritten);
    SetConsoleCursorPosition(hConsole, upperLeft);
}


/*
#include <windows.h>
#include <conio.h>
#include <string>
using namespace std;

///////////////////////////////////////////////////////////
// Screen -- Visual Output
///////////////////////////////////////////////////////////

class ScreenImpl
{
public:
	ScreenImpl(int width, int height);
	~ScreenImpl();
	void clear();
	void gotoXY(int x, int y);
	void printChar(char ch);
	void printString(std::string s);
	void printStringClearLine(std::string s);
	void refresh();

private:
	HANDLE m_hConsole;
	int m_width;
	int m_height;
};

ScreenImpl::ScreenImpl(int width, int height)
	: m_width(width), m_height(height)
{
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(m_hConsole, &csbi))
		return;
	SMALL_RECT r = csbi.srWindow;
	r.Top = 0;
	if (r.Bottom < r.Top + m_height)
		r.Bottom = r.Top + m_height;
	SetConsoleWindowInfo(m_hConsole, TRUE, &r);
}

ScreenImpl::~ScreenImpl()
{
}

void ScreenImpl::clear()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(m_hConsole, &csbi))
		return;
	DWORD size = csbi.dwSize.X * csbi.dwSize.Y;
	COORD pos = { 0, 0 };
	DWORD num;
	FillConsoleOutputCharacter(m_hConsole, TCHAR(' '), size, pos, &num);
	FillConsoleOutputAttribute(m_hConsole, csbi.wAttributes, size, pos, &num);
	gotoXY(0, 0);
}

void ScreenImpl::gotoXY(int x, int y)
{
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		return;
	COORD pos = { short(x), short(y) };
	SetConsoleCursorPosition(m_hConsole, pos);
}

void ScreenImpl::printChar(char ch)
{
	DWORD num;
	WriteConsole(m_hConsole, &ch, 1, &num, NULL);
}

void ScreenImpl::printString(std::string s)
{
	DWORD num;
	WriteConsole(m_hConsole, s.data(), s.size(), &num, NULL);
}

void ScreenImpl::printStringClearLine(std::string s)
{
	if (s.size() > size_t(m_width))
		s.erase(m_width);
	DWORD num;
	WriteConsole(m_hConsole, s.data(), s.size(), &num, NULL);
	string blanks(m_width - s.size(), ' ');
	WriteConsole(m_hConsole, blanks.data(), blanks.size(), &num, NULL);
}

void ScreenImpl::refresh()
{
}

Screen::Screen(int width, int height)
{
	m_impl = new ScreenImpl(width, height);
}

Screen::~Screen()
{
	delete m_impl;
}

void Screen::clear()
{
	m_impl->clear();
}

void Screen::gotoXY(int x, int y)
{
	m_impl->gotoXY(x, y);
}

void Screen::printChar(char ch)
{
	m_impl->printChar(ch);
}

void Screen::printString(std::string s)
{
	m_impl->printString(s);
}

void Screen::printStringClearLine(std::string s)
{
	m_impl->printStringClearLine(s);
}

void Screen::refresh()
{
	m_impl->refresh();
}*/



#else  // not Microsoft Visual C++, so assume UNIX interface

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <termios.h>
using namespace std;

static const char* ESC_SEQ = "\x1B[";  // ANSI Terminal escape sequence:  ESC [
static const char* ESC_SEQ_X = "\xEF\x9C";  // Xcode Console arrow key escape sequence

class TerminalModeSetter
{
  public:
    TerminalModeSetter()
    {
        tcgetattr(STDIN_FILENO, &m_old);
        m_new = m_old;
        m_new.c_lflag &= ~(ICANON | ECHO);
        m_new.c_cc[VMIN] = 1;
        m_new.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &m_new);
    }
    ~TerminalModeSetter()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &m_old);
    }
  private:
    termios m_old;
    termios m_new;
};

char getCharacter()
{
    static TerminalModeSetter dummy;
    char ch;
    if (!cin.get(ch))
        return '?';
    if (ch != ESC_SEQ[0]  &&  ch != ESC_SEQ_X[0])
        return ch;
    if (!cin.get(ch)  ||  (ch != ESC_SEQ[1]  &&  ch != ESC_SEQ_X[1])  ||  !cin.get(ch))
        return '?';
    switch (ch)
    {
      case 'D': case '\x82':  return ARROW_LEFT;
      case 'C': case '\x83':  return ARROW_RIGHT;
      case 'A': case '\x80':  return ARROW_UP;
      case 'B': case '\x81':  return ARROW_DOWN;
      default:   return '?';
    }
}

void clearScreen()
{
    static const char* term = getenv("TERM");
    if (term == NULL  ||  strcmp(term, "dumb") == 0)
        cout << endl;
     else
        cout << ESC_SEQ << "2J" << ESC_SEQ << "H" << flush;
}

#endif
