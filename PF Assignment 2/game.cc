#include "game.h"

#include <iostream>
#include <thread>
#include <Windows.h>

#include "render_data.h"

using namespace std;

void game_main()
{
	system("chcp 65001");
	hide_cursor();

	// initial configuration
	ivector2 c_size		= ivector2{ 45, 21 };
	render_data* rd		= new render_data(c_size);
	player_data* pd		= new player_data();
	ivector2 room		= { 0,1 };
	random_provider* rp = new random_provider(get_seed(room));
	vector<bomb*> bombs;

	// set up player
	pd->max_health = 5;
	pd->range = 2;
	pd->health = pd->max_health;
	pd->bombs = 3;
	pd->invincibility_timer = 0;
	pd->position = c_size/2;
	pd->has_barrier = true;

	rd->clear_layer(layer::BACKGROUND);
	rd->clear_layer(layer::FOREGROUND);
	rd->clear_layer(layer::OVERLAY);

	// load buffers of rooms
	uint32_t* room_designs[NUM_ROOM_LAYOUTS+2] = { NULL };
	for (int i = 0; i < NUM_ROOM_LAYOUTS; i++)
	{
		room_designs[i] = new uint32_t[c_size.x * c_size.y];
		read_multichars_to_buffer((char*)room_walls, room_designs[i], false);
		read_multichars_to_buffer((char*)room_layouts[i], room_designs[i], true);
	}

	// load buffers of transition frames
	uint32_t* transition_frames[NUM_TRANSITIONS] = { NULL };
	for (int i = 0; i < NUM_TRANSITIONS; i++)
	{
		transition_frames[i] = new uint32_t[c_size.x * c_size.y];
		read_multichars_to_buffer((char*)transition_buffers[i], transition_frames[i], false);
	}

	// load tutorial room
	room_designs[NUM_ROOM_LAYOUTS] = new uint32_t[c_size.x * c_size.y];
	read_multichars_to_buffer((char*)room_tutorial, room_designs[NUM_ROOM_LAYOUTS], false);

	// copy the tutorial room into the background buffer
	rd->read_buffer(layer::BACKGROUND, room_designs[NUM_ROOM_LAYOUTS]);
	draw_doorways(room, rd);
	rd->set_tile(layer::FOREGROUND, ivector2{ 11,8 }, GOOP_INIT);

	rd->set_tile(layer::OVERLAY, pd->position, PLAYER);

	rd->draw(cout);

	key_states ks;
	ks.any = false;

	// main gameloop begins here:
	while (true)
	{
		// get user input
		while (!ks.any) check_key_states(ks);

		// clear overlay
		rd->clear_layer(layer::OVERLAY);

		// check for player movement
		ivector2 move_vector;
		move_vector.x = ks.move_right - ks.move_left;
		move_vector.y = ks.move_down - ks.move_up;
		if (magnitude_squared(move_vector) > 0)
		{
			// set the new player tile, clamped to the size of the room
			ivector2 raw_next_position = pd->position + move_vector;
			ivector2 old_position = pd->position;
			pd->direction = direction(move_vector);
			pd->position = raw_next_position; //clamp(raw_next_position, ivector2{ 2,2 }, c_size - ivector2{ 3,3 });
			uint32_t bg = rd->get_tile(layer::BACKGROUND, pd->position);
			if (bg != 0 && bg != ' ' && bg != BARRIER) pd->position = old_position;

			// handle door transitions
			room = handle_door_transition(pd, rd, rp, room, &bombs, room_designs, transition_frames);
		}
		else if (ks.attack)
		{
			perform_player_attack(pd, rd, rp);
		}
		else if (ks.alt_attack)
		{
			perform_player_bomb(&bombs, pd, rd);
		}
		else if (ks.barrier_attack)
		{
			perform_player_barrier(pd, rd);
		}
		
		// draw player
		rd->set_tile(layer::OVERLAY, pd->position, PLAYER);

		// check for intersections
		check_intersection(pd, rd);

		// decrease invincibility timer
		if (pd->invincibility_timer > 0) pd->invincibility_timer--;

		// update the goop
		grow_goop_tiles(rd, rp);

		// update bombs
		update_bombs(&bombs, rd, rp);

		// update overlay text
		update_overlay_text(pd, rd, room);

		// reset cursor and draw screen
		rd->draw(cout);

		if (pd->health == 0) break;

		// wait for there to be no keys pressed
		while (ks.any) check_key_states(ks);
	}

	// show the game over screen
	room_designs[NUM_ROOM_LAYOUTS+1] = new uint32_t[c_size.x * c_size.y];
	read_multichars_to_buffer((char*)room_gameover, room_designs[NUM_ROOM_LAYOUTS+1], false);

	// copy the tutorial room into the background buffer
	rd->read_buffer(layer::BACKGROUND, room_designs[NUM_ROOM_LAYOUTS+1]);
	rd->clear_layer(layer::FOREGROUND);
	rd->clear_layer(layer::OVERLAY);
	
	ivector2 center = rd->get_size() / 2;
	string text_1 = "GAME OVER";
	string text_2 = "YOU ARE DEAD";
	rd->set_tiles(layer::OVERLAY, ivector2{ center.x - (int)(text_1.size() / 2), center.y - 1 }, text_1, false);
	rd->set_tiles(layer::OVERLAY, ivector2{ center.x - (int)(text_2.size() / 2), center.y + 1 }, text_2, false);
	rd->draw(cout);

	for (int i = 0; i < 2; i++)
	{
		for (unsigned int t = 0; t < rd->get_size().x * rd->get_size().y; t++)
		{
			uint32_t tile = rd->get_tile(layer::BACKGROUND, t);
			if (tile != BLOCK) rd->set_tile(layer::BACKGROUND, t, tile+1);
		}
		this_thread::sleep_for(chrono::seconds(1));
		rd->draw(cout);
	}

	while (true);
}

void check_key_states(key_states& ks)
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

void update_bombs(vector<bomb*>* bombs, render_data* rd, random_provider* rp)
{
	vector<bomb*>* new_list = new vector<bomb*>();
	new_list->reserve(bombs->size());
	for (bomb* b : *bombs)
	{
		// does modifying the list destroy the iterator? maybe!
		// note from future me, yes, yes it very much does
		b->timer--;
		rd->set_tile(layer::FOREGROUND, b->position, b->timer + 0x30);
		if (b->timer == 0) explode_bomb(b->position, 4, rd, rp);
		else new_list->push_back(b);
	}
	if (new_list->size() != bombs->size())
	{
		bombs->clear();
		for (bomb* b : *new_list) bombs->push_back(b);
	}
}

void explode_bomb(ivector2 center, int radius, render_data* rd, random_provider* rp)
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
					try_drop_pickup(ivector2{ i,j }, rd, rp, 0);
				}
				rd->set_tile(layer::OVERLAY, ivector2{ i,j }, BOMB_EXPLOSION);
			}
		}
	}
}

void try_drop_pickup(ivector2 tile, render_data* rd, random_provider* rp, uint32_t fallback)
{
	double r = rp->next();
	if (r < DROP_DIV_0) rd->set_tile(layer::FOREGROUND, tile, HEALTH_PICKUP);
	else if (r < DROP_DIV_1) rd->set_tile(layer::FOREGROUND, tile, BOMB_PICKUP);
	else if (r < DROP_DIV_2) rd->set_tile(layer::FOREGROUND, tile, HEALTH_UPGRADE);
	else if (r < DROP_DIV_3) rd->set_tile(layer::FOREGROUND, tile, RANGE_UPGRADE);
	else if (r < DROP_DIV_4) rd->set_tile(layer::FOREGROUND, tile, BARRIER_PICKUP);
	else rd->set_tile(layer::FOREGROUND, tile, fallback);
}

void draw_doorways(ivector2 room_position, render_data* rd)
{
	unsigned char doors = get_room_doors(room_position);
	ivector2 size = rd->get_size();
	ivector2 half = rd->get_size() / 2;
	
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
		rd->set_tile(layer::BACKGROUND, ivector2{ half.x - 1, size.y - 2 }, BLOCK);
		rd->set_tile(layer::BACKGROUND, ivector2{ half.x, size.y - 2 }, 0);
		rd->set_tile(layer::BACKGROUND, ivector2{ half.x + 1, size.y - 2 }, BLOCK);
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
		rd->set_tile(layer::BACKGROUND, ivector2{ size.x - 2, half.y - 1 }, BLOCK);
		rd->set_tile(layer::BACKGROUND, ivector2{ size.x - 2, half.y }, 0);
		rd->set_tile(layer::BACKGROUND, ivector2{ size.x - 2, half.y + 1 }, BLOCK);
	}
}

ivector2 handle_door_transition(player_data* pd, render_data* rd, random_provider* rp, ivector2 room, vector<bomb*>* bombs, uint32_t** room_designs, uint32_t** transition_frames)
{
	ivector2 rd_size = rd->get_size();
	ivector2 transition_direction{ 0, 0 };
	if (pd->position.y <= 1) transition_direction.y = -1;
	if (pd->position.x <= 1) transition_direction.x = -1;
	if (pd->position.y >= rd_size.y - 2) transition_direction.y = 1;
	if (pd->position.x >= rd_size.x - 2) transition_direction.x = 1;

	if (transition_direction.x == 0 && transition_direction.y == 0) return room;

	ivector2 new_room = room + transition_direction;

	// clear
	bombs->clear();

	// clear foreground
	rd->clear_layer(layer::FOREGROUND);

	// transition
	for (int i = 0; i < NUM_TRANSITIONS; i++)
	{
		rd->read_buffer(layer::OVERLAY, transition_frames[i]);
		this_thread::sleep_for(chrono::milliseconds(TRANSITION_DELAY));
		rd->draw(cout);
	}

	// TODO: Generate the room fresh

	// load room based on seed
	rd->read_buffer(layer::BACKGROUND, room_designs[get_seed(new_room)%NUM_ROOM_LAYOUTS]);
	draw_doorways(new_room, rd);

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

void check_intersection(player_data* pd, render_data* rd)
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
	if (is_bomb_pickup(tile)) { pd->bombs++; clear_tile = true; }

	// intersection with health pickup
	if (is_health_pickup(tile)) { pd->health = clamp(pd->health + 3, 0, pd->max_health); pd->invincibility_timer = 1; clear_tile = true; }

	// intersection with max health upgrade
	if (is_health_upgrade(tile)) { pd->max_health++; clear_tile = true; }

	// intersection with range upgrade
	if (is_range_upgrade(tile)) { pd->range++; clear_tile = true; }

	if (clear_tile) rd->set_tile(layer::FOREGROUND, pd->position, 0);
}

void grow_goop_tiles(render_data* rd, random_provider* rp)
{
	ivector2 rd_size = rd->get_size();
	unsigned int rd_length = rd_size.x * rd_size.y;
	for (unsigned int i = 2 + (rd_size.x*2); i < rd_length - (3 + (rd_size.x*2)); i++)
	{
		uint32_t tile_value = rd->get_tile(layer::FOREGROUND, i);
		if (!is_goop_tile(tile_value)) continue;

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

			if (!is_goop_tile(fg_udlr[0]) && (bg_udlr[0] == ' ' || bg_udlr[0] == 0))
				rd->set_tile(layer::FOREGROUND, i - rd_size.x, GOOP_INIT);
			if (!is_goop_tile(fg_udlr[1]) && (bg_udlr[1] == ' ' || bg_udlr[1] == 0))
				rd->set_tile(layer::FOREGROUND, i + rd_size.x, GOOP_INIT);
			if (!is_goop_tile(fg_udlr[2]) && (bg_udlr[2] == ' ' || bg_udlr[2] == 0))
				rd->set_tile(layer::FOREGROUND, i - 1, GOOP_INIT);
			if (!is_goop_tile(fg_udlr[3]) && (bg_udlr[3] == ' ' || bg_udlr[3] == 0))
				rd->set_tile(layer::FOREGROUND, i + 1, GOOP_INIT);
		}
	}
}

void perform_player_attack(player_data* pd, render_data* rd, random_provider* rp)
{
	uint32_t c = 0;
	if (pd->direction == 0) c = UP_ATTACK;
	if (pd->direction == 1) c = RIGHT_ATTACK;
	if (pd->direction == 2) c = DOWN_ATTACK;
	if (pd->direction == 3) c = LEFT_ATTACK;

	ivector2 pos = pd->position;
	for (unsigned int i = 0; i < pd->range+1; i++)
	{	
		rd->set_tile(layer::OVERLAY, pos, c);
		if (is_goop_tile(rd->get_tile(layer::FOREGROUND, pos)))
			try_drop_pickup(pos, rd, rp, 0);
		pos = pos + direction(pd->direction);
	}
}

void perform_player_bomb(vector<bomb*>* bombs, player_data* pd, render_data* rd)
{
	if (pd->bombs <= 0) return;
	bomb* b = new bomb();
	b->position = pd->position + direction(pd->direction);
	b->timer = 5;
	bombs->push_back(b);
	pd->bombs--;
}

void perform_player_barrier(player_data* pd, render_data* rd)
{
	if (!pd->has_barrier) return;
	ivector2 pos = pd->position;
	ivector2 rd_size = rd->get_size();
	while (pos.x < rd_size.x-1 && pos.x >= 0 && pos.y < rd_size.y-1 && pos.y >= 0)
	{
		rd->set_tile(layer::BACKGROUND, pos, BARRIER);
		pos = pos + direction(pd->direction);
	}
	pd->has_barrier = false;
}

void update_overlay_text(player_data* pd, render_data* rd, ivector2 room)
{
	ivector2 rd_size = rd->get_size();
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
	string bottom = " b : " + to_string(pd->bombs) + " | r : " + to_string(pd->range) + " | i : " + to_string(pd->invincibility_timer) + " | q : " + to_string(pd->has_barrier ? 1 : 0);
	rd->set_tiles(layer::OVERLAY, rd_size.x * (rd_size.y - 1), bottom, false);
}

bool is_goop_tile(uint32_t tile)
{
	return tile == GOOP_0 || tile == GOOP_1 || tile == GOOP_2 || tile == GOOP_INIT;
}

bool is_bomb_pickup(uint32_t tile)
{
	return tile == BOMB_PICKUP;
}

bool is_health_pickup(uint32_t tile)
{
	return tile == HEALTH_PICKUP;
}

bool is_health_upgrade(uint32_t tile)
{
	return tile == HEALTH_UPGRADE;
}

bool is_range_upgrade(uint32_t tile)
{
	return tile == RANGE_UPGRADE;
}

bool is_barrier_pickup(uint32_t tile)
{
	return tile == BARRIER_PICKUP;
}

uint32_t advance_goop_tile(uint32_t current)
{
	// ░ -> ▒
	if (current == GOOP_0) return GOOP_1;
	// ▒ -> ▓
	if (current == GOOP_1) return GOOP_2;
	// ▓ -> ░
	if (current == GOOP_2 || current == GOOP_INIT) return GOOP_0;

	return 0;
}

void set_cursor_pos(ivector2 pos)
{
	// based closely on this Gist: https://gist.github.com/karolisjan/643c77b56e555683d57d1fac6906c638
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = { (SHORT)pos.x, (SHORT)pos.y };
	cout.flush();
	SetConsoleCursorPosition(out, coord);
}

void hide_cursor()
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursor_info;
	GetConsoleCursorInfo(out, &cursor_info);
	cursor_info.bVisible = false;
	SetConsoleCursorInfo(out, &cursor_info);
}

ivector2 direction(int dir)
{
	int ldir = dir % 4;
	int bdir = ldir % 2;
	int sdir = ((ldir / 2)*2)-1;
	return ivector2{(-sdir)*bdir, sdir*(1-bdir)};
}

int direction(ivector2 dir)
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

unsigned int clamp(unsigned int x, unsigned int a, unsigned int b)
{
	return max(min(x, b), a);
}