#include "game.h"

int main()
{
	// transfer control to the game main function
	game g;
	// tutorial on the first time, off for all consecutive times
	bool show_tutorial = true;

	// repeatedly run the game until the player doesn't want to play again
	while (g.game_main(show_tutorial)) show_tutorial = false;

	return 0;
}