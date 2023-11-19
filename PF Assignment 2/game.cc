#include "game.h"

#include <iostream>
#include <thread>
#include <Windows.h>
#include "terminal_utils.h"

using namespace std;

void game::game_main()
{
	system("chcp 65001");
	hide_cursor();

	// initial configuration
	rd_size			= ivector2{ 45, 21 };
	rd				= new render_data(rd_size);
	pd				= new player_data();
	room			= { 0,1 };
	rp				= new random_provider(get_seed(room));
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

	rd->clear_layer(layer::BACKGROUND);
	rd->clear_layer(layer::FOREGROUND);
	rd->clear_layer(layer::OVERLAY);
	rd->draw(cout);

	// center the room on the screen, in fullscreen terminal
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
			perform_player_attack();
			counts_as_turn = true;
		}
		else if (ks.alt_attack)
		{
			perform_player_bomb();
			counts_as_turn = true;
		}
		else if (ks.barrier_attack)
		{
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
	mh->append_line("GAME OVER");
	rd->set_tiles(layer::OVERLAY, ivector2{ rd_center.x - (int)(text_1.size() / 2), rd_center.y - 1 }, text_1, false);
	rd->set_tiles(layer::OVERLAY, ivector2{ rd_center.x - (int)(text_2.size() / 2), rd_center.y + 1 }, text_2, false);
	rd->draw(cout);
	mh->draw(cout);

	for (int i = 0; i < 2; i++)
	{
		for (int t = 0; t < rd->get_size().x * rd->get_size().y; t++)
		{
			uint32_t tile = rd->get_tile(layer::BACKGROUND, t);
			if (tile != BLOCK) rd->set_tile(layer::BACKGROUND, t, tile+1);
		}
		this_thread::sleep_for(chrono::seconds(1));
		rd->draw(cout);
	}

	while (true);
}

void game::draw_doorways(ivector2 room_position)
{
	unsigned char doors = get_room_doors(room_position);
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
	// top left: current room
	string top_left = " room : " + to_string(room.x) + " " + to_string(room.y);
	rd->set_tiles(layer::OVERLAY, 0, top_left, false);

	// top right: health
	unsigned int tile_pos = rd_size.x - 2;
	for (unsigned int i = 0; i < pd->max_health; i++)
	{
		rd->set_tile(layer::OVERLAY, tile_pos, (i < pd->health) ? FILLED_HEART : EMPTY_HEART);
		tile_pos--;
	}

	// bottom 
	string bottom = " d :   | b : " + to_string(pd->bombs) + " | r : " + to_string(pd->range) + " | i : " + to_string(pd->invincibility_timer) + " | q : " + to_string(pd->has_barrier ? 1 : 0);
	rd->set_tiles(layer::OVERLAY, rd_size.x * (rd_size.y - 1), bottom, false);
	rd->set_tile(layer::OVERLAY, (rd_size.x * (rd_size.y - 1)) + 5, direction_char(pd->direction));
}

void game::check_key_states()
{
	ks.move_up = GetAsyncKeyState(VK_UP) & 0x01;
	ks.move_down = GetAsyncKeyState(VK_DOWN) & 0x01;
	ks.move_left = GetAsyncKeyState(VK_LEFT) & 0x01;
	ks.move_right = GetAsyncKeyState(VK_RIGHT) & 0x01;

	ks.attack = GetAsyncKeyState('X') & 0x01;
	ks.alt_attack = GetAsyncKeyState('Z') & 0x01;
	ks.barrier_attack = GetAsyncKeyState('C') & 0x01;

	ks.any = ks.move_up || ks.move_down || ks.move_left || ks.move_right || ks.attack || ks.alt_attack || ks.barrier_attack;
}

ivector2 game::handle_door_transition()
{
	ivector2 transition_direction{ 0, 0 };
	if (pd->position.y <= 1) transition_direction.y = -1;
	if (pd->position.x <= 1) transition_direction.x = -1;
	if (pd->position.y >= rd_size.y - 2) transition_direction.y = 1;
	if (pd->position.x >= rd_size.x - 2) transition_direction.x = 1;

	if (transition_direction.x == 0 && transition_direction.y == 0) return room;

	ivector2 new_room = room + transition_direction;

	// clear
	bombs.clear();

	// clear foreground
	rd->clear_layer(layer::FOREGROUND);

	// transition
	for (int i = 0; i < NUM_TRANSITIONS; i++)
	{
		rd->read_buffer(layer::OVERLAY, transition_frames[i]);
		this_thread::sleep_for(chrono::milliseconds(TRANSITION_DELAY));
		rd->draw(cout);
	}

	// get the room layout id
	unsigned int room_layout_id = get_seed(new_room) % NUM_ROOM_LAYOUTS;

	// load room based on seed
	rd->read_buffer(layer::BACKGROUND, room_designs[room_layout_id]);
	draw_doorways(new_room);

	// populate the room with goop, if not cleared
	if (cleared_rooms.find(new_room) == cleared_rooms.end())
		generate_fresh_goop(room_layout_id, MAX_GOOP_INITIAL+(pd->goop_cleared/GOOP_CLEARING_RATIO));

	// transition out again
	for (int i = NUM_TRANSITIONS-1; i >= 0; i--)
	{
		rd->read_buffer(layer::OVERLAY, transition_frames[i]);
		this_thread::sleep_for(chrono::milliseconds(TRANSITION_DELAY));
		rd->draw(cout);
	}

	rd->clear_layer(layer::OVERLAY);
	this_thread::sleep_for(chrono::milliseconds(TRANSITION_DELAY));
	rd->draw(cout);

	pd->position = (((transition_direction*2) + rd_size) % rd_size) + ((rd_size / 2) * (ivector2{ 1,1 } - abs(transition_direction)));
	pd->position = pd->position - ivector2{ transition_direction.x == -1, transition_direction.y == -1 };
	return new_room;
}

void game::perform_player_attack()
{
	uint32_t c = direction_char(pd->direction);

	ivector2 pos = pd->position;
	for (unsigned int i = 0; i < pd->range + 1; i++)
	{
		if (pos.x <= 0 || pos.x >= rd_size.x - 1 || pos.y <= 0 || pos.y >= rd_size.y - 1) break;
		rd->set_tile(layer::OVERLAY, pos, c);
		if (is_goop_tile(rd->get_tile(layer::FOREGROUND, pos)))
		{
			try_drop_pickup(pos, 0);
			pd->goop_cleared++;
		}
		pos = pos + direction(pd->direction);
	}
}

void game::perform_player_bomb()
{
	if (pd->bombs <= 0) return;
	bomb* b = new bomb();
	b->position = pd->position + direction(pd->direction);
	b->timer = 5;
	bombs.push_back(b);
	pd->bombs--;
}

void game::perform_player_barrier()
{
	if (!pd->has_barrier) return;
	ivector2 pos = pd->position;
	while (pos.x < rd_size.x - 2 && pos.x > 1 && pos.y < rd_size.y - 2 && pos.y > 1)
	{
		rd->set_tile(layer::BACKGROUND, pos, BARRIER);
		if (is_goop_tile(rd->get_tile(layer::FOREGROUND, pos)))
		{
			rd->set_tile(layer::FOREGROUND, pos, 0);
			pd->goop_cleared++;
		}
		pos = pos + direction(pd->direction);
	}
	pd->has_barrier = false;
}

ivector2 game::direction(int dir)
{
	int ldir = dir % 4;
	int bdir = ldir % 2;
	int sdir = ((ldir / 2) * 2) - 1;
	return ivector2{ (-sdir) * bdir, sdir * (1 - bdir) };
}

int game::direction(ivector2 dir)
{
	ivector2 mdir{ abs(dir.x), abs(dir.y) };
	if (mdir.x > mdir.y)
	{
		if (dir.x > 0) return 1;
		else return 3;
	}
	else if (mdir.x < mdir.y)
	{
		if (dir.y > 0) return 2;
		else return 0;
	}
	return 0;
}

uint32_t game::direction_char(int dir)
{
	uint32_t c = 0;
	if (dir == 0) c = UP_ATTACK;
	if (dir == 1) c = RIGHT_ATTACK;
	if (dir == 2) c = DOWN_ATTACK;
	if (dir == 3) c = LEFT_ATTACK;
	return c;
}

void game::update_bombs()
{
	vector<bomb*>* new_list = new vector<bomb*>();
	new_list->reserve(bombs.size());
	for (bomb* b : bombs)
	{
		// does modifying the list destroy the iterator? maybe!
		// note from future me, yes, yes it very much does
		b->timer--;
		rd->set_tile(layer::FOREGROUND, b->position, b->timer + 0x30);
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
	rd->set_tile(layer::FOREGROUND, center, 0);
	for (int i = center.x - radius; i < center.x + radius; i++)
	{
		for (int j = center.y - radius; j < center.y + radius; j++)
		{
			ivector2 offset{ i - center.x, j - center.y };
			if (magnitude_squared(offset) < radius*radius)
			{
				if (is_goop_tile(rd->get_tile(layer::FOREGROUND, ivector2{ i,j })))
				{
					try_drop_pickup(ivector2{ i,j }, 0);
					pd->goop_cleared++;
				}
				rd->set_tile(layer::OVERLAY, ivector2{ i,j }, BOMB_EXPLOSION);
			}
		}
	}
}

void game::try_drop_pickup(ivector2 tile, uint32_t fallback)
{
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
	uint32_t tile = rd->get_tile(layer::FOREGROUND, pd->position);

	// intersection with goop
	if (is_goop_tile(tile))
	{
		if (pd->invincibility_timer == 0)
		{
			pd->health--;
			pd->invincibility_timer = 5;
		}
	}

	bool clear_tile = false;

	// intersection with bomb pickup
	if (is_bomb_pickup(tile)) 
	{
		pd->bombs++; 
		clear_tile = true; 
		mh->append_line("BOMB COLLECTED");
	}

	// intersection with health pickup
	if (is_health_pickup(tile)) 
	{ 
		pd->health = clamp(pd->health + 3, 0, pd->max_health); 
		pd->invincibility_timer = 1; 
		clear_tile = true;
		mh->append_line("HEALTH REFILLED");
	}

	// intersection with max health upgrade
	if (is_health_upgrade(tile)) 
	{ 
		pd->max_health++; 
		pd->health++; 
		clear_tile = true; 
		mh->append_line("MAX HEALTH INCREASED");
	}

	// intersection with range upgrade
	if (is_range_upgrade(tile)) 
	{ 
		pd->range++; 
		clear_tile = true; 
		mh->append_line("SWORD RANGE INCREASED");
	}

	// intersection with barrier pickup
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
	unsigned int rd_length = rd_size.x * rd_size.y;
	bool goop_seen = false;
	for (int i = 2 + (rd_size.x*2); i < rd_length - (3 + (rd_size.x*2)); i++)
	{
		uint32_t tile_value = rd->get_tile(layer::FOREGROUND, i);
		if (!is_goop_tile(tile_value)) continue;
		goop_seen = true;
		rd->set_tile(layer::FOREGROUND, i, advance_goop_tile(tile_value));
		
		if (tile_value == GOOP_2 && rp->next() < GOOP_GROW_CHANCE)
		{
			// grow into all adjacent tiles, as long as the background is empty
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

			if (!is_goop_tile(fg_udlr[0]) && (bg_udlr[0] == ' ' || bg_udlr[0] == 0) && (i-rd_size.x >= rd_size.x*2))
				rd->set_tile(layer::FOREGROUND, i - rd_size.x, GOOP_INIT);
			if (!is_goop_tile(fg_udlr[1]) && (bg_udlr[1] == ' ' || bg_udlr[1] == 0) && (i+rd_size.x <= rd_length-(rd_size.x*3)))
				rd->set_tile(layer::FOREGROUND, i + rd_size.x, GOOP_INIT);
			if (!is_goop_tile(fg_udlr[2]) && (bg_udlr[2] == ' ' || bg_udlr[2] == 0) && ((i-1) % rd_size.x >= 2))
				rd->set_tile(layer::FOREGROUND, i - 1, GOOP_INIT);
			if (!is_goop_tile(fg_udlr[3]) && (bg_udlr[3] == ' ' || bg_udlr[3] == 0) && ((i+1) % rd_size.x <= rd_size.x-3))
				rd->set_tile(layer::FOREGROUND, i + 1, GOOP_INIT);
		}
	}
	return !goop_seen;
}

void game::generate_fresh_goop(unsigned int room_layout_id, unsigned int max_goop_tiles)
{
	ivector2 rd_size = rd->get_size();
	unsigned int buffer_length = rd_size.x * rd_size.y;

	vector<unsigned int> tiles_to_goopify;
	while (tiles_to_goopify.size() < max_goop_tiles)
	{
		// pick a random tile
		unsigned int test_tile = (int)(rp->next() * buffer_length);
		// check that goop is allowed here
		if (goop_gen_masks[room_layout_id][test_tile] == '1') tiles_to_goopify.push_back(test_tile);
	}

	for (unsigned int t : tiles_to_goopify) rd->set_tile(layer::FOREGROUND, t, GOOP_0);

	for (int i = 0; i < GOOP_GEN_ITERATIONS; i++) grow_goop_tiles();
}

uint32_t game::advance_goop_tile(uint32_t current)
{
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
	return max(0, (pd->goop_cleared * GOOP_SCORE_MULTIPLIER) 
		 + (pd->turns * TURNS_SCORE_MULTIPLIER)
		 + ((pd->max_health - PLAYER_INITIAL_HEALTH) * HEALTH_SCORE_MULTIPLIER)
		 + ((pd->range - PLAYER_INITIAL_RANGE) * RANGE_SCORE_MULTIPLER)
		 + (cleared_rooms.size() * ROOMS_SCORE_MULTIPLIER));
}