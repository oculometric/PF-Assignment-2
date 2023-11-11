#pragma once

#include <string>
#include <iostream>

#include "ivector2.h"
#include "room.h"

using namespace std;

// enum to give identifiers to the layers
enum layer
{
	BACKGROUND,
	FOREGROUND,
	OVERLAY
};

// class for managing, modifying and displaying the visual data
class render_data
{
private:
	// backing data buffers for the canvas
	uint32_t* background_layer;
	uint32_t* foreground_layer;
	uint32_t* overlay_layer;

	// size of the canvas in width and height
	ivector2 canvas_size;
	// length of the buffer to save recalculating it
	unsigned int canvas_length;
	// terminal offset for drawing the room to the screen
	ivector2 draw_offset;

public:
	// set drawing offset
	void set_draw_offset(ivector2);

	// getter for canvas_size
	ivector2 get_size();

	// get the character at a tile, with overloads in case we want to do things quicker (precompute index)
	uint32_t get_tile(layer, ivector2);
	uint32_t get_tile(layer, unsigned int);

	// set the character at a tile, with overloads in case we want to do things quicker (precompute index)
	void set_tile(layer, ivector2, uint32_t);
	void set_tile(layer, unsigned int, uint32_t);

	// write a line of characters (multibyte chars are read into single uint32_t values), with overloads in case we want to do things quicker (precompute index). boolean for whether to jump over spaces
	void set_tiles(layer, ivector2, string, bool);
	void set_tiles(layer, unsigned int, string, bool);

	// read and write entire layers from another buffer, allows us to make backup copies of rooms. make sure your buffer is big enough!
	void write_buffer(layer, uint32_t*);
	void read_buffer(layer, uint32_t*);

	// utility functions
	void clear_layer(layer);

	// print out the entire buffer to the screen
	void draw(ostream&);

	// construct a render data buffer with a certain size
	render_data(ivector2);

	// destruct this render data buffer and all its internal buffers
	~render_data();
};
