#pragma once

#include <list>

#include "bomb.h"
#include "ivector2.h"

using namespace std;

// struct to hold cached data about a room that has been loaded, so that the room persists (does not reset) after the player leaves it
struct cached_room
{
	// the offset of this room in the global room grid
	ivector2 room_offset;
	
	// the seed for this room, based on room_offset to minimise recalculation
	unsigned long long room_seed;

	// copy of the foreground layer for persistence
	uint32_t foreground_layer[];

	// list of bombs for persistence
	list<bomb*> bombs;
};

// read a room from the room definitions and put the result in a databuffer, which can then be copied into the render buffer. make sure your destination buffer is big enough!
void read_room_to_buffer(unsigned int, uint32_t*);

// TODO: room designs and a few more funcs