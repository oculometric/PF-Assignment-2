﻿#pragma once

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

// read a sequence of characters and put the result in a databuffer, combining multibyte characters into single data elements. make sure your destination buffer is big enough!
void read_multichars_to_buffer(char*, uint32_t*, bool);

// the overall room size is 45x21, however, with two chars on each side that makes it actually 41x17

//░
//▒
//▓
#define NUM_ROOM_LAYOUTS
static const char* room_layouts[NUM_ROOM_LAYOUTS] =
{
	u8"                                             "
	  "                                             "
	  "          ▐                       ▌          "
	  "          ▐                       ▌          "
	  "          ▐                       ▌          "
	  "          ▐                       ▌          "
	  "  ▄▄▄▄▄▄▄▄▟                       ▙▄▄▄▄▄▄▄▄  "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "  ▀▀▀▀▀▀▀▀▜                       ▛▀▀▀▀▀▀▀▀  "
	  "          ▐                       ▌          "
	  "          ▐                       ▌          "
	  "          ▐                       ▌          "
	  "          ▐                       ▌          "
	  "                                             "
	  "                                             ",

	u8"                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "      ▛▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▜      "
	  "      ▌                               ▐      "
	  "      ▌                               ▐      "
	  "      ▌                               ▐      "
	  "      ▌                               ▐      "
	  "      ▌                               ▐      "
	  "      ▙▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▟      "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             ",

	u8"                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "          ▛▀▀▀▀▀▀▜         ▛▀▀▀▀▀▀▜          "
	  "          ▌      ▐         ▌      ▐          "
	  "          ▌      ▐         ▌      ▐          "
	  "          ▌      ▐         ▌      ▐          "
	  "          ▌      ▐         ▌      ▐          "
	  "          ▌      ▐         ▌      ▐          "
	  "          ▌      ▐         ▌      ▐          "
	  "          ▌      ▐         ▌      ▐          "
	  "          ▌      ▐         ▌      ▐          "
	  "          ▌      ▐         ▌      ▐          "
	  "          ▙▄▄▄▄▄▄▟         ▙▄▄▄▄▄▄▟          "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
	  "                                             "
};

static const char* room_walls =
u8"                                             "
" +-----------------------------------------+ "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" |                                         | "
" +-----------------------------------------+ "
"                                             ";
