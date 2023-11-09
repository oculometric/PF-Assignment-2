﻿#pragma once

#include <list>

#include "bomb.h"
#include "ivector2.h"

#define PRIME_0 2600149679
#define PRIME_1 1880881
#define PRIME_2 3943267
#define PRIME_3 686408054863

using namespace std;

typedef unsigned long long int huge_uint;

// struct to hold cached data about a room that has been loaded, so that the room persists (does not reset) after the player leaves it
struct cached_room
{
	// the offset of this room in the global room grid
	ivector2 room_offset;

	// the seed for this room, based on room_offset to minimise recalculation
	unsigned long long room_seed;

	// copy of the foreground layer for persistence
	uint32_t* foreground_layer;

	// list of bombs for persistence
	list<bomb*> bombs;
};

// read a sequence of characters and put the result in a databuffer, combining multibyte characters into single data elements. make sure your destination buffer is big enough!
void read_multichars_to_buffer(char*, uint32_t*, bool);

// get the seed deterministically based on the room position
inline huge_uint get_seed(ivector2 position) { return (position.x * PRIME_0) + (position.y * PRIME_2); }

// get the door toggles deterministically for a room, packed as 0b0000RLBT
unsigned char get_room_doors(ivector2);

// the overall room size is 45x21, however, with two chars on each side that makes it actually 41x17

//░
//▒
//▓

#define NUM_ROOM_LAYOUTS 3
static const char* room_layouts[NUM_ROOM_LAYOUTS] =
{
	u8"                                             "
	u8"                                             "
	u8"          ▐                       ▌          "
	u8"          ▐                       ▌          "
	u8"          ▐                       ▌          "
	u8"          ▐                       ▌          "
	u8"  ▄▄▄▄▄▄▄▄▟                       ▙▄▄▄▄▄▄▄▄  "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"  ▀▀▀▀▀▀▀▀▜                       ▛▀▀▀▀▀▀▀▀  "
	u8"          ▐                       ▌          "
	u8"          ▐                       ▌          "
	u8"          ▐                       ▌          "
	u8"          ▐                       ▌          "
	u8"                                             "
	u8"                                             ",

	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"      ▛▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▜      "
	u8"      ▌                               ▐      "
	u8"      ▌                               ▐      "
	u8"      ▌                               ▐      "
	u8"      ▌                               ▐      "
	u8"      ▌                               ▐      "
	u8"      ▙▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▟      "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             ",
	
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"          ▛▀▀▀▀▀▀▜         ▛▀▀▀▀▀▀▜          "
	u8"          ▌      ▐         ▌      ▐          "
	u8"          ▌      ▐         ▌      ▐          "
	u8"          ▌      ▐         ▌      ▐          "
	u8"          ▌      ▐         ▌      ▐          "
	u8"          ▌      ▐         ▌      ▐          "
	u8"          ▌      ▐         ▌      ▐          "
	u8"          ▌      ▐         ▌      ▐          "
	u8"          ▌      ▐         ▌      ▐          "
	u8"          ▌      ▐         ▌      ▐          "
	u8"          ▙▄▄▄▄▄▄▟         ▙▄▄▄▄▄▄▟          "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
	u8"                                             "
};

static const char* room_walls =
u8"                                             "
u8" ▟▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▙ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▐                                         ▌ "
u8" ▜▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▛ "
u8"                                             ";

static const char* room_tutorial =
u8"                                             "
u8" ▟▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▙ "
u8" ▐                                         ▌ "
u8" ▐  welcome, player!                       ▌ "
u8" ▐  use the arrow keys to move             ▌ "
u8" ▐  X to melee attack                      ▌ "
u8" ▐  Z to place a bomb in front of you      ▌ "
u8" ▐                                         ▌ "
u8" ▐  this:  ░  is goop, it hurts you        ▌ "
u8" ▐  destroy it with melee or bombs         ▌ "
u8" ▐                                         ▌ "
u8" ▐  it also grows! but only when you move  ▌ "
u8" ▐  or act. beware, and good luck!         ▌ "
u8" ▐  use the doorways on the sides of the   ▌ "
u8" ▐  room to navigate the maze              ▌ "
u8" ▐                                         ▌ "
u8" ▐  your stats: b - number of bombs,       ▌ "
u8" ▐  r - melee range, i - invincible turns  ▌ "
u8" ▐                                         ▌ "
u8" ▜▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▛ "
u8"                                             ";

static const char* room_gameover =
u8"█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█"
u8"░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█░"
u8"░░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█░░"
u8"░░░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█░░░"
u8"░░░░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█░░░░"
u8"░░░░░███████████████████████████████████░░░░░"
u8"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
u8"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
u8"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
u8"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
u8"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
u8"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
u8"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
u8"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
u8"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
u8"░░░░░███████████████████████████████████░░░░░"
u8"░░░░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█░░░░"
u8"░░░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█░░░"
u8"░░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█░░"
u8"░█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█░"
u8"█░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█";