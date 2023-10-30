#ifndef RENDERER_H
#define RENDERER_H

#include "room.h"
#include <string>


class renderer
{
private:
	ivector2 viewport_size;

public:
	void set_cursor_pos(ivector2);
	void hide_cursor();

	void clear_buffer();
	void draw_screen();
	void draw_walls();
	void draw_objects();

	void set_room(ivector2);
	void construct_room();

	void draw_pixel(ivector2, uint32_t);
	void draw_line(ivector2, string);
	void draw_sprite(ivector2, const unsigned char*);
	void blit();

	void set_player(player* _player);

	renderer (ivector2);

private:
	// current door information
	ivector2 room_position;
	bool door_top;
	bool door_bottom;
	bool door_left;
	bool door_right;

	vector<object*> objects;
	vector<enemy*> enemies;

	uint32_t* backbuffer;

	player* game_player;
};

#endif