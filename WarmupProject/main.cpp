// main.cpp

#include "Dungeon.h"

#include "Game.h"
#include <cstdlib>
#include <ctime>
using namespace std;

int main()
{
	  // The following call to srand ensures that each time you run the program,
	  // you'll get a different series of random numbers generated, and thus
	  // different randomly-generated levels, monsters, scrolls, etc.  If you
      // comment out this call, then each game you start will run the same up
	  // until you change the way you attack a monster or do something else that	  // will call randInt in a different circumstance than you did before.
	  // This repeatability could be useful for debugging.

	srand(static_cast<unsigned int>(time(0)));

	Game g(9);
	g.play();

	system("pause");
}
