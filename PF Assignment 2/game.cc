#include "game.h"

#include <iostream>
#include <thread>
#include <Windows.h>

#pragma comment(lib,"XInput.lib")
#pragma comment(lib,"Xinput9_1_0.lib")
#include <Xinput.h>

#include "terminal_utils.h"
#include "profiling.h"

using namespace std;

bool game::game_main(bool show_tutorial)
{
	// set up the terminal
	system("chcp 65001");
	system("cls");
	hide_cursor();

	// initial configuration
	global_seed		= (unsigned int)chrono::high_resolution_clock::now().time_since_epoch().count();
	rd_size			= ivector2{ 45, 21 };
	rd				= new render_data(rd_size);
	pd				= new player_data();
	room			= ivector2{ 0, 0 }; cleared_rooms.insert(ivector2{ 0, 0 });
	rp				= new random_provider(global_seed);
	ivector2 rd_center = rd_size / 2;

	// set up player
	pd->max_health = PLAYER_INITIAL_HEALTH;
	pd->range = PLAYER_INITIAL_RANGE;
	pd->health = pd->max_health;
	pd->bombs = PLAYER_INITIAL_BOMBS;
	pd->invincibility_timer = 0;
	pd->position = rd_size/2;
	pd->has_barrier = true;
	pd->turns = 0;
	pd->goop_cleared = 0;

	// clear screen
	rd->clear_layer(layer::BACKGROUND);
	rd->clear_layer(layer::FOREGROUND);
	rd->clear_layer(layer::OVERLAY);
	rd->draw(cout);

	// center the room on the terminal
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	ivector2 terminal_size_characters = 
	{
		csbi.srWindow.Right - csbi.srWindow.Left + 1, 
		csbi.srWindow.Bottom - csbi.srWindow.Top + 1 
	};

	ivector2 graphics_offset = (terminal_size_characters / 2) - rd_center;
	rd->set_draw_offset(graphics_offset);

	mh = new message_history(ivector2{ graphics_offset.x + rd_size.x, graphics_offset.y + 1 }, rd_size.y - 5, 32);

	// set up controller
	user_controller_index = -1;
	XINPUT_STATE dump;
	for (int i = 0; i < 4; i++)
	{
		if (XInputGetState(i, &dump) == ERROR_SUCCESS)
		{
			user_controller_index = i;
			break;
		}
	}

	// load buffers of rooms
	for (int i = 0; i < NUM_ROOM_LAYOUTS; i++)
	{
		room_designs[i] = new uint32_t[rd_size.x * rd_size.y];
		read_multichars_to_buffer((char*)room_walls, room_designs[i], false);
		read_multichars_to_buffer((char*)room_layouts[i], room_designs[i], true);
	}

	// load buffers of transition frames
	for (int i = 0; i < NUM_TRANSITIONS; i++)
	{
		transition_frames[i] = new uint32_t[rd_size.x * rd_size.y];
		read_multichars_to_buffer((char*)transition_buffers[i], transition_frames[i], false);
	}

	// load tutorial room
	room_designs[NUM_ROOM_LAYOUTS] = new uint32_t[rd_size.x * rd_size.y];
	read_multichars_to_buffer((char*)room_tutorial, room_designs[NUM_ROOM_LAYOUTS], false);

	// if wanted, run the tutorial
	if (show_tutorial) perform_tutorial();

	// copy the tutorial room into the background buffer
	rd->read_buffer(layer::BACKGROUND, room_designs[NUM_ROOM_LAYOUTS]);
	draw_doorways(room);
	rd->set_tile(layer::FOREGROUND, ivector2{ 11,8 }, GOOP_INIT);
	rd->set_tile(layer::OVERLAY, pd->position, PLAYER);
	rd->draw(cout);

	ks.any = false;

	bool counts_as_turn = false;

	// main gameloop begins here:
	while (true)
	{
		// get user input
		while (!ks.any) check_key_states();

		counts_as_turn = false;
		mh->clear_highlight();

		// clear overlay
		rd->clear_layer(layer::OVERLAY);

		// check for player movement
		ivector2 move_vector;
		move_vector.x = ks.move_right - ks.move_left;
		move_vector.y = ks.move_down - ks.move_up;
		if (magnitude_squared(move_vector) > 0)
		{
			// turn the player
			int new_direction = direction(move_vector);
			if (new_direction == pd->direction)
			{

				// set the new player tile, clamped to the size of the room
				ivector2 raw_next_position = pd->position + move_vector;
				ivector2 old_position = pd->position;
				pd->position = raw_next_position;
				uint32_t bg = rd->get_tile(layer::BACKGROUND, pd->position);
				if (bg != 0 && bg != ' ' && bg != BARRIER) pd->position = old_position;
				else counts_as_turn = true;

				// handle door transitions
				room = handle_door_transition();
			}
			pd->direction = direction(move_vector);
		}
		else if (ks.attack)
		{
			// handle player attacking
			perform_player_attack();
			counts_as_turn = true;
		}
		else if (ks.alt_attack)
		{
			// handle player placing a bomb
			perform_player_bomb();
			counts_as_turn = true;
		}
		else if (ks.barrier_attack)
		{
			// handle player placing a barrier
			perform_player_barrier();
			counts_as_turn = true;
		}

		// draw player
		rd->set_tile(layer::OVERLAY, pd->position, PLAYER);

		// increment player turns, only if this was actually a turn
		if (counts_as_turn)
		{
			pd->turns++;

			// check for intersections
			check_intersection();

			// decrease invincibility timer
			if (pd->invincibility_timer > 0) pd->invincibility_timer--;

			// update the goop
			bool room_cleared = grow_goop_tiles();
			if (room_cleared && cleared_rooms.find(room) == cleared_rooms.end())
			{
				// room cleared, reward player and make a note
				pd->health = pd->max_health;
				pd->bombs += PLAYER_INITIAL_BOMBS;

				cleared_rooms.insert(room);

				// draw special message
				mh->append_line("ROOM CLEARED");
			}

			// update bombs
			update_bombs();
		}


		// update overlay text
		update_overlay_text();

		// reset cursor and draw screen
		rd->draw(cout);
		string score_text = " score : " + to_string(calculate_score()) + " (" + to_string(pd->turns) + " turns, " + to_string(cleared_rooms.size()) + " rooms cleared)";
		cout << score_text;
		for (int i = 0; i < rd_size.x - score_text.size(); i++) cout << ' ';
		mh->draw(cout);

		// break out if the player is dead
		if (pd->health == 0) break;

		// wait for there to be no keys pressed
		while (ks.any) check_key_states();
	}

	// show the game over screen
	room_designs[NUM_ROOM_LAYOUTS+1] = new uint32_t[rd_size.x * rd_size.y];
	read_multichars_to_buffer((char*)room_gameover, room_designs[NUM_ROOM_LAYOUTS+1], false);

	// copy the tutorial room into the background buffer
	rd->read_buffer(layer::BACKGROUND, room_designs[NUM_ROOM_LAYOUTS+1]);
	rd->clear_layer(layer::FOREGROUND);
	rd->clear_layer(layer::OVERLAY);
	
	string text_1 = "GAME OVER : SCORE " + to_string(calculate_score());
	string text_2 = "YOU ARE DEAD";
	string text_3 = "PRESS LEFT TO EXIT, RIGHT TO PLAY AGAIN";
	mh->append_line("GAME OVER");
	rd->set_tiles(layer::OVERLAY, ivector2{ rd_center.x - (int)(text_1.size() / 2), rd_center.y - 1 }, text_1, false);
	rd->set_tiles(layer::OVERLAY, ivector2{ rd_center.x - (int)(text_2.size() / 2), rd_center.y + 1 }, text_2, false);
	rd->set_tiles(layer::OVERLAY, ivector2{ rd_center.x - (int)(text_3.size() / 2), rd_center.y + 3 }, text_3, false);
	rd->draw(cout);
	mh->draw(cout);

	// gradually fade the gameover screen to be lighter, just as an effect
	for (int i = 0; i < 2; i++)
	{
		// make all the shaded blocks one shade stronger
		for (int t = 0; t < rd->get_size().x * rd->get_size().y; t++)
		{
			uint32_t tile = rd->get_tile(layer::BACKGROUND, t);
			if (tile != BLOCK) rd->set_tile(layer::BACKGROUND, t, tile+1);
		}

		this_thread::sleep_for(chrono::seconds(1));

		rd->draw(cout);
	}

	while (true)
	{
		// wait for player to press a key
		ks.any = false;
		while (!ks.any) check_key_states();

		// deallocate everything allocated in this function
		delete rd;
		delete pd;
		delete rp;
		delete mh;
		
		for (bomb* b : bombs) delete b;
		bombs.clear();
		for (int i = 0; i < NUM_ROOM_LAYOUTS+2; i++)
			delete room_designs[i];
		for (int i = 0; i < NUM_TRANSITIONS; i++)
			delete transition_frames[i];

		cleared_rooms.clear();

		// if the player pressed right, play again
		if (ks.move_right) return true;
		// if left, stop the program
		if (ks.move_left) return false;
	}
}

void game::perform_tutorial()
{
	vector<string> text_0 =
	{
		"welcome adventurer!",
		"the labyrinth has been overrun with",
		"a strange and aggressive fungus",
		"",
		"it contains a deadly toxin, so avoid it!",
		"",
		"you have a few tools at your disposal:",
		"  your sword (keyboard X or controller A)",
		"  bombs (keyboard Z or controller B)",
		"  a barrier (keyboard C or controller X)",
		"",
		"you can move around with arrow keys or",
		"the D-pad.",
		"",
		"",
		"",
		"",
		"         - press RIGHT to continue -        ",
	};

	vector<string> text_1 =
	{
		"this is what you look like -> @",
		"",
		"when you destroy a fungus tile, it may",
		"drop a useful item, such as:",
		"  health refill ->       #",
		"  bomb pickup ->         *",
		"  max health upgrade ->  +",
		"  sword range upgrade -> ^",
		"  barrier pickup ->      $",
		"",
		"before you begin your journey, remember:",
		"  barriers will form a wall across the",
		"  room, blocking the path of the fungi,",
		"  however you can still walk across it",
		"",
		"  you can only hold one barrier",
		"  at a time",
		"",
		"         - press RIGHT to continue -        "
	};

	vector<string> text_2 = 
	{
		"  bomb explosions don't harm you, but",
		"  they explode in a large radius and",
		"  destroy a lot of fungi in one go",
		"",
		"  be aware of your health in the top right",
		"",
		"  each time you move or act, the fungi may",
		"  grow, but not when you turn on the spot",
		"",
		"the bottom of the screen also shows some",
		"useful information:",
		"  d - direction you are facing",
		"  b - bombs held",
		"  r - melee range",
		"  i - invincible turns",
		"  q - quarantine barriers held",
		"",
		"",
		"         - press RIGHT to continue -        "
	};

	vector<string> text_3 =
	{
		"when you clear a room, no more fungi will",
		"grow and the room will be safe from then on",
		"",
		"however if you leave a room uncleared, it",
		"will reset if you leave and re-enter it!",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"best of luck adventurer. you may need it.",
		"",
		"",
		"           - press RIGHT to begin -          "
	};

	// show screen 1
	for (int i = 0; i < text_0.size(); i++)
	{
		rd->set_tiles(layer::OVERLAY, 1 + (rd_size.x * (i+1)), text_0[i], false);
		rd->draw(cout);
		this_thread::sleep_for(chrono::milliseconds(50));
	}

	// wait for user confirm
	while (!ks.move_right) check_key_states();
	ks.move_right = false;
	rd->clear_layer(layer::OVERLAY);
	
	// show screen 2
	for (int i = 0; i < text_1.size(); i++)
	{
		rd->set_tiles(layer::OVERLAY, 1 + (rd_size.x * (i + 1)), text_1[i], false);
		rd->draw(cout);
		this_thread::sleep_for(chrono::milliseconds(50));
	}

	// wait for user confirm
	while (!ks.move_right) check_key_states();
	ks.move_right = false;
	rd->clear_layer(layer::OVERLAY);

	// show screen 3
	for (int i = 0; i < text_2.size(); i++)
	{
		rd->set_tiles(layer::OVERLAY, 1 + (rd_size.x * (i + 1)), text_2[i], false);
		rd->draw(cout);
		this_thread::sleep_for(chrono::milliseconds(50));
	}

	// wait for user confirm
	while (!ks.move_right) check_key_states();
	ks.move_right = false;
	rd->clear_layer(layer::OVERLAY);

	// show screen 4
	for (int i = 0; i < text_3.size(); i++)
	{
		rd->set_tiles(layer::OVERLAY, 1 + (rd_size.x * (i + 1)), text_3[i], false);
		rd->draw(cout);
		this_thread::sleep_for(chrono::milliseconds(50));
	}

	// wait for user confirm
	while (!ks.move_right) check_key_states();
	ks.move_right = false;
	rd->clear_layer(layer::OVERLAY);
	rd->draw(cout);
}

void game::draw_doorways(ivector2 room_position)
{
	// get packed bools for whether or not to show particular doors
	unsigned char doors = get_room_doors(room_position, global_seed);
	ivector2 half = rd_size / 2;

	if (doors & 0b0001)
	{
		// draw top door
		rd->set_tile(layer::BACKGROUND, ivector2{ half.x - 1, 1 }, BLOCK);
		rd->set_tile(layer::BACKGROUND, ivector2{ half.x, 1 }, 0);
		rd->set_tile(layer::BACKGROUND, ivector2{ half.x + 1, 1 }, BLOCK);
	}
	if (doors & 0b0010)
	{
		// draw bottom door
		rd->set_tile(layer::BACKGROUND, ivector2{ half.x - 1, rd_size.y - 2 }, BLOCK);
		rd->set_tile(layer::BACKGROUND, ivector2{ half.x, rd_size.y - 2 }, 0);
		rd->set_tile(layer::BACKGROUND, ivector2{ half.x + 1, rd_size.y - 2 }, BLOCK);
	}
	if (doors & 0b0100)
	{
		// draw left door
		rd->set_tile(layer::BACKGROUND, ivector2{ 1, half.y - 1 }, BLOCK);
		rd->set_tile(layer::BACKGROUND, ivector2{ 1, half.y }, 0);
		rd->set_tile(layer::BACKGROUND, ivector2{ 1, half.y + 1 }, BLOCK);
	}
	if (doors & 0b1000)
	{
		// draw right door
		rd->set_tile(layer::BACKGROUND, ivector2{ rd_size.x - 2, half.y - 1 }, BLOCK);
		rd->set_tile(layer::BACKGROUND, ivector2{ rd_size.x - 2, half.y }, 0);
		rd->set_tile(layer::BACKGROUND, ivector2{ rd_size.x - 2, half.y + 1 }, BLOCK);
	}
}

void game::update_overlay_text()
{
	// top left text: current room
	string top_left = " room : " + to_string(room.x) + " " + to_string(room.y);
	rd->set_tiles(layer::OVERLAY, 0, top_left, false);

	// top right text: health
	unsigned int tile_pos = rd_size.x - 2;
	for (unsigned int i = 0; i < pd->max_health; i++)
	{
		rd->set_tile(layer::OVERLAY, tile_pos, (i < pd->health) ? FILLED_HEART : EMPTY_HEART);
		tile_pos--;
	}

	// bottom text: various stats
	string bottom = " d :   | b : " + to_string(pd->bombs) + " | r : " + to_string(pd->range) + " | i : " + to_string(pd->invincibility_timer) + " | q : " + to_string(pd->has_barrier ? 1 : 0);
	rd->set_tiles(layer::OVERLAY, rd_size.x * (rd_size.y - 1), bottom, false);
	rd->set_tile(layer::OVERLAY, (rd_size.x * (rd_size.y - 1)) + 5, direction_char(pd->direction));
}

void game::check_key_states()
{
	// check states of keyboard keys, resetting values in the struct
	ks.move_up = GetAsyncKeyState(VK_UP) & 0xFFF0;
	ks.move_down = GetAsyncKeyState(VK_DOWN) & 0xFFF0;
	ks.move_left = GetAsyncKeyState(VK_LEFT) & 0xFFF0;
	ks.move_right = GetAsyncKeyState(VK_RIGHT) & 0xFFF0;

	ks.attack = GetAsyncKeyState('X') & 0xFFF0;
	ks.alt_attack = GetAsyncKeyState('Z') & 0xFFF0;
	ks.barrier_attack = GetAsyncKeyState('C') & 0xFFF0;

	// query controller state and OR those values onto the struct states
	XINPUT_STATE controller_state;
	DWORD result = XInputGetState(user_controller_index, &controller_state);
	if (result == ERROR_SUCCESS)
	{
		ks.move_up			|= (bool)(controller_state.Gamepad.wButtons & 0b0001);
		ks.move_down		|= (bool)(controller_state.Gamepad.wButtons & 0b0010);
		ks.move_left		|= (bool)(controller_state.Gamepad.wButtons & 0b0100);
		ks.move_right		|= (bool)(controller_state.Gamepad.wButtons & 0b1000);

		ks.attack			|= (bool)(controller_state.Gamepad.wButtons & 0b001000000000000);
		ks.alt_attack		|= (bool)(controller_state.Gamepad.wButtons & 0b010000000000000);
		ks.barrier_attack	|= (bool)(controller_state.Gamepad.wButtons & 0b100000000000000);
	}

	// set the flag for any key down if any of the scanned keys are pressed
	ks.any = ks.move_up || ks.move_down || ks.move_left || ks.move_right || ks.attack || ks.alt_attack || ks.barrier_attack;
}

ivector2 game::handle_door_transition()
{
	// check if the player has crossed out of the current room
	ivector2 transition_direction{ 0, 0 };
	if (pd->position.y <= 1) transition_direction.y = -1;
	if (pd->position.x <= 1) transition_direction.x = -1;
	if (pd->position.y >= rd_size.y - 2) transition_direction.y = 1;
	if (pd->position.x >= rd_size.x - 2) transition_direction.x = 1;

	// if the player is not going through a doorway, do nothing
	if (transition_direction.x == 0 && transition_direction.y == 0) return room;

	// otherwise calculate the base offset of the new room we are going
	// to transition into
	ivector2 new_room = room + transition_direction;

	// clear bombs list, since the room will be unloaded and destroyed
	bombs.clear();

	// clear foreground layer
	rd->clear_layer(layer::FOREGROUND);

	// play transition frames one by one
	// these are drawn in the overlay buffer which means they cover
	// everything else (including the player and text, since we 
	// do not update those during this)
	for (int i = 0; i < NUM_TRANSITIONS; i++)
	{
		rd->read_buffer(layer::OVERLAY, transition_frames[i]);
		// small delay between each frame to be fancy
		this_thread::sleep_for(chrono::milliseconds(TRANSITION_DELAY));
		rd->draw(cout);
	}

	// get the room layout id for the new room
	unsigned int room_layout_id = (global_seed + get_seed(new_room)) % NUM_ROOM_LAYOUTS;

	// check if this room is the starting room
	bool is_start = new_room.x == 0 && new_room.y == 0;

	// load room layout based on seed, and draw the doorways
	if (is_start)
		rd->read_buffer(layer::BACKGROUND, room_designs[NUM_ROOM_LAYOUTS]);
	else
		rd->read_buffer(layer::BACKGROUND, room_designs[room_layout_id]);
	draw_doorways(new_room);

	// populate the room with goop, if the room was not already cleared
	if (cleared_rooms.find(new_room) == cleared_rooms.end())
		generate_fresh_goop(room_layout_id, MAX_GOOP_INITIAL+(pd->goop_cleared/GOOP_CLEARING_RATIO));

	// transition out again to show the new room
	for (int i = NUM_TRANSITIONS-1; i >= 0; i--)
	{
		rd->read_buffer(layer::OVERLAY, transition_frames[i]);
		this_thread::sleep_for(chrono::milliseconds(TRANSITION_DELAY));
		rd->draw(cout);
	}

	// clear the overlay again to be ready for the rest of
	// the game to draw itself back out in game_main
	rd->clear_layer(layer::OVERLAY);
	this_thread::sleep_for(chrono::milliseconds(TRANSITION_DELAY));
	rd->draw(cout);

	// set the player's position for entering the 
	// room (i.e. move them to the other side)
	pd->position = (((transition_direction*2) + rd_size) % rd_size) + ((rd_size / 2) * (ivector2{ 1,1 } - abs(transition_direction)));
	pd->position = pd->position - ivector2{ transition_direction.x == -1, transition_direction.y == -1 };
	return new_room;
}

void game::perform_player_attack()
{
	// get the arrow direction character for the player's direction
	uint32_t c = direction_char(pd->direction);

	ivector2 pos = pd->position;
	ivector2 dir = direction(pd->direction);
	// draw an attack graphic proportional to the player's range in the
	// direction they are facing
	for (unsigned int i = 0; i < pd->range + 1; i++)
	{
		if (pos.x <= 0 || pos.x >= rd_size.x - 1 || pos.y <= 0 || pos.y >= rd_size.y - 1) break;
		rd->set_tile(layer::OVERLAY, pos, c);
		if (is_goop_tile(rd->get_tile(layer::FOREGROUND, pos)))
		{
			// if the melee attack encountered goop, drop pickups
			try_drop_pickup(pos, 0);
			pd->goop_cleared++;
		}
		// increment pos based on facing direction
		pos = pos + dir;
	}
}

void game::perform_player_bomb()
{
	// if the player has no bombs, do nothing
	if (pd->bombs <= 0) return;
	// create a new bomb at the target position
	bomb* b = new bomb();
	b->position = pd->position + direction(pd->direction);
	b->timer = 5;
	// add it to the list
	bombs.push_back(b);
	// remove a bomb from the player
	pd->bombs--;
}

void game::perform_player_barrier()
{
	// if the player doesn't have a barrier, do nothing
	if (!pd->has_barrier) return;
	ivector2 pos = pd->position;
	ivector2 dir = direction(pd->direction);
	while (pos.x < rd_size.x - 2 && pos.x > 1 && pos.y < rd_size.y - 2 && pos.y > 1)
	{
		// set the tile to be a barrier
		rd->set_tile(layer::BACKGROUND, pos, BARRIER);
		if (is_goop_tile(rd->get_tile(layer::FOREGROUND, pos)))
		{
			// destroy goop tiles if there are any on the foreground
			// if destroyed by a barrier, goop shouldn't drop pickups
			rd->set_tile(layer::FOREGROUND, pos, 0);
			pd->goop_cleared++;
		}
		// increment the position by the direction
		pos = pos + dir;
	}
	// remove the barrier from the player's inventory
	pd->has_barrier = false;
}

ivector2 game::direction(int dir)
{
	// modulus maths to convert a direction to a unit ivector2
	int ldir = dir % 4;
	int bdir = ldir % 2;
	int sdir = ((ldir / 2) * 2) - 1;
	return ivector2{ (-sdir) * bdir, sdir * (1 - bdir) };
}

int game::direction(ivector2 dir)
{
	ivector2 mdir = abs(dir);
	if (mdir.x > mdir.y)
	{
		// if X is dominant
		if (dir.x > 0) return 1;
		else return 3;
	}
	else if (mdir.x < mdir.y)
	{
		// if Y is dominant
		if (dir.y > 0) return 2;
		else return 0;
	}
	// otherwise assume up, since this function 
	// should never be passed a zero ivector2
	return 0;
}

uint32_t game::direction_char(int dir)
{
	switch (dir)
	{
	case 0:
		return UP_ATTACK;
	case 1:
		return RIGHT_ATTACK;
	case 2:
		return DOWN_ATTACK;
	case 3:
		return LEFT_ATTACK;
	default:
		return NULL;
	}
}

void game::update_bombs()
{
	// backup list of bombs
	vector<bomb*>* new_list = new vector<bomb*>();
	new_list->reserve(bombs.size());
	for (bomb* b : bombs)
	{
		// since modifying the list while iterating on it destroys the iterator
		// we have to have a backing list and copy the ones we want to keep to it
		// decrease the timer on each bomb
		b->timer--;
		// display timer
		rd->set_tile(layer::FOREGROUND, b->position, b->timer + 0x30);
		// if timer is zero, explode the bomb
		if (b->timer == 0) explode_bomb(b->position, 4);
		else new_list->push_back(b);
	}
	if (new_list->size() != bombs.size())
	{
		bombs.clear();
		for (bomb* b : *new_list) bombs.push_back(b);
	}
}

void game::explode_bomb(ivector2 center, int radius)
{
	// clear bomb timer label
	rd->set_tile(layer::FOREGROUND, center, 0);
	// loop over a square block of tiles
	for (int i = center.x - radius; i < center.x + radius; i++)
	{
		for (int j = center.y - radius; j < center.y + radius; j++)
		{
			ivector2 offset{ i - center.x, j - center.y };
			// if the distance from the center is less than the desired radius
			if (magnitude_squared(offset) < radius*radius)
			{
				// destroy goop, and drop pickups if goop was destroyed 
				// at any particular tile
				if (is_goop_tile(rd->get_tile(layer::FOREGROUND, ivector2{ i,j })))
				{
					try_drop_pickup(ivector2{ i,j }, 0);
					pd->goop_cleared++;
				}
				// set the tile in the overlay to bomb explosions
				rd->set_tile(layer::OVERLAY, ivector2{ i,j }, BOMB_EXPLOSION);
			}
		}
	}
}

void game::try_drop_pickup(ivector2 tile, uint32_t fallback)
{
	// drop a random pickup, or nothing, based on PRNG
	double r = rp->next();
	if (r < DROP_DIV_0) rd->set_tile(layer::FOREGROUND, tile, HEALTH_PICKUP);
	else if (r < DROP_DIV_1) rd->set_tile(layer::FOREGROUND, tile, BOMB_PICKUP);
	else if (r < DROP_DIV_2) rd->set_tile(layer::FOREGROUND, tile, HEALTH_UPGRADE);
	else if (r < DROP_DIV_3) rd->set_tile(layer::FOREGROUND, tile, RANGE_UPGRADE);
	else if (r < DROP_DIV_4) rd->set_tile(layer::FOREGROUND, tile, BARRIER_PICKUP);
	else rd->set_tile(layer::FOREGROUND, tile, fallback);
}

void game::check_intersection()
{
	// get the tile under the player in the foreground
	uint32_t tile = rd->get_tile(layer::FOREGROUND, pd->position);

	// check intersection with goop
	if (is_goop_tile(tile))
	{
		if (pd->invincibility_timer == 0)
		{
			pd->health--;
			pd->invincibility_timer = 3;
		}
	}

	bool clear_tile = false;

	// check intersection with bomb pickup
	if (is_bomb_pickup(tile)) 
	{
		pd->bombs++; 
		clear_tile = true; 
		mh->append_line("BOMB COLLECTED");
	}

	// check intersection with health pickup
	if (is_health_pickup(tile)) 
	{ 
		pd->health = clamp(pd->health + 3, 0, pd->max_health); 
		pd->invincibility_timer = 1; 
		clear_tile = true;
		mh->append_line("HEALTH REFILLED");
	}

	// check intersection with max health upgrade
	if (is_health_upgrade(tile)) 
	{ 
		pd->max_health++; 
		pd->health++; 
		clear_tile = true; 
		mh->append_line("MAX HEALTH INCREASED");
	}

	// check intersection with range upgrade
	if (is_range_upgrade(tile)) 
	{ 
		pd->range++; 
		clear_tile = true; 
		mh->append_line("SWORD RANGE INCREASED");
	}

	// check intersection with barrier pickup
	if (is_barrier_pickup(tile)) 
	{ 
		pd->has_barrier = true; 
		clear_tile = true; 
		mh->append_line("BARRIER COLLECTED");
	}

	if (clear_tile) rd->set_tile(layer::FOREGROUND, pd->position, 0);
}

bool game::is_goop_tile(uint32_t tile)
{
	return tile == GOOP_0 || tile == GOOP_1 || tile == GOOP_2 || tile == GOOP_INIT;
}

bool game::is_bomb_pickup(uint32_t tile)
{
	return tile == BOMB_PICKUP;
}

bool game::is_health_pickup(uint32_t tile)
{
	return tile == HEALTH_PICKUP;
}

bool game::is_health_upgrade(uint32_t tile)
{
	return tile == HEALTH_UPGRADE;
}

bool game::is_range_upgrade(uint32_t tile)
{
	return tile == RANGE_UPGRADE;
}

bool game::is_barrier_pickup(uint32_t tile)
{
	return tile == BARRIER_PICKUP;
}

bool game::grow_goop_tiles()
{
	// precompute to save time
	int rd_length = rd_size.x * rd_size.y;
	int room_start = 2 + (rd_size.x * 2);
	int room_end = rd_length - (2 + (rd_size.x * 2));
	bool goop_seen = false;
	uint32_t tile_value;

	// iterate over tiles in the buffer
	for (int i = room_start; i < room_end; i++)
	{
		tile_value = rd->get_tile(layer::FOREGROUND, i);
		// if the tile isn't goop, skip over it
		if (!is_goop_tile(tile_value)) continue;
		goop_seen = true;
		// advance the goop tile
		rd->set_tile(layer::FOREGROUND, i, advance_goop_tile(tile_value));
		
		// if we're on a certain goop state, and we get lucky, grow
		if (tile_value == GOOP_2 && rp->next() < GOOP_GROW_CHANCE)
		{
			// get data about surrounding tiles
			uint32_t bg_udlr[4] =
			{
				rd->get_tile(layer::BACKGROUND, i - rd_size.x),
				rd->get_tile(layer::BACKGROUND, i + rd_size.x),
				rd->get_tile(layer::BACKGROUND, i - 1),
				rd->get_tile(layer::BACKGROUND, i + 1)
			};
			uint32_t fg_udlr[4] =
			{
				rd->get_tile(layer::FOREGROUND, i - rd_size.x),
				rd->get_tile(layer::FOREGROUND, i + rd_size.x),
				rd->get_tile(layer::FOREGROUND, i - 1),
				rd->get_tile(layer::FOREGROUND, i + 1)
			};

			// grow into all adjacent tiles, as long as the background 
			// is empty, and the foreground isn't a goop tile, and the 
			// targeted tile is in the range of the buffer
			if (!is_goop_tile(fg_udlr[0]) && (bg_udlr[0] == ' ' || bg_udlr[0] == 0) && (i-rd_size.x >= rd_size.x*2))
				rd->set_tile(layer::FOREGROUND, i - rd_size.x, GOOP_INIT);
			if (!is_goop_tile(fg_udlr[1]) && (bg_udlr[1] == ' ' || bg_udlr[1] == 0) && (i+rd_size.x <= rd_length-(rd_size.x*2)))
				rd->set_tile(layer::FOREGROUND, i + rd_size.x, GOOP_INIT);
			if (!is_goop_tile(fg_udlr[2]) && (bg_udlr[2] == ' ' || bg_udlr[2] == 0) && ((i-1) % rd_size.x >= 2))
				rd->set_tile(layer::FOREGROUND, i - 1, GOOP_INIT);
			if (!is_goop_tile(fg_udlr[3]) && (bg_udlr[3] == ' ' || bg_udlr[3] == 0) && ((i+1) % rd_size.x <= rd_size.x-3))
				rd->set_tile(layer::FOREGROUND, i + 1, GOOP_INIT);
		}
	}
	// return whether the room is completely clear of goop
	return !goop_seen;
}

void game::generate_fresh_goop(unsigned int room_layout_id, unsigned int max_goop_tiles)
{
	// calculate buffer length
	unsigned int buffer_length = rd_size.x * rd_size.y;

	// keeps a list of tiles which will be turned to goop
	set<unsigned int> tiles_to_goopify;
	while (tiles_to_goopify.size() < max_goop_tiles)
	{
		// pick a random tile
		unsigned int test_tile = (int)(rp->next() * buffer_length);
		// check that goop is allowed here
		if (goop_gen_masks[room_layout_id][test_tile] == '1') tiles_to_goopify.insert(test_tile);
	}

	// set the selected tiles to be goop
	for (unsigned int t : tiles_to_goopify) rd->set_tile(layer::FOREGROUND, t, GOOP_INIT);

	// iterate a few times so the goop looks like it's been here the whole time
	for (int i = 0; i < GOOP_GEN_ITERATIONS; i++) grow_goop_tiles();
}

uint32_t game::advance_goop_tile(uint32_t current)
{
	// advance the state of a goop tile
	// goop can have 4 different states, gives some variety
	// and something to happen onscreen each turn
	// ░ -> ▒
	if (current == GOOP_0) return GOOP_1;
	// ▒ -> ▓
	if (current == GOOP_1) return GOOP_2;
	// ▓ -> ░
	if (current == GOOP_2 || current == GOOP_INIT) return GOOP_0;

	return 0;
}

unsigned int game::calculate_score()
{
	// combine various player/game state attributes to compute the player's current score
	return (unsigned int)max(0, (pd->goop_cleared * GOOP_SCORE_MULTIPLIER) 
		 + (pd->turns * TURNS_SCORE_MULTIPLIER)
		 + ((pd->max_health - PLAYER_INITIAL_HEALTH) * HEALTH_SCORE_MULTIPLIER)
		 + ((pd->range - PLAYER_INITIAL_RANGE) * RANGE_SCORE_MULTIPLER)
		 + (cleared_rooms.size() * ROOMS_SCORE_MULTIPLIER));
}