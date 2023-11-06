#include "game.h"
#include "render_data.h"

void game_main()
{
	// initial configuration
	ivector2 c_size = ivector2{ 45, 21 };
	render_data* rd = new render_data(c_size);
	player_data* pd = new player_data();

	// set up player
	pd->max_health = 5;
	pd->range = 2;
	pd->health = pd->max_health;
	pd->bombs = 3;
	pd->invincibility_timer = 0;
	pd->position = c_size/2;

	rd->clear_layer(layer::BACKGROUND);
}
