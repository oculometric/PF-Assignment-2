#pragma once

#include "ivector2.h"

// struct that holds some data about the current player state
struct player_data
{
	// player's current position in the room
	ivector2 position;

	// how much health the player has right now
	unsigned int health;

	// maximum amount of health the player can have
	unsigned int max_health;

	// number of bombs the player is holding
	unsigned int bombs;

	// whether the player has a barrier held
	bool has_barrier;

	// range of the player's melee attack in tiles
	unsigned int range;

	// countdown timer for the player being invincible. if zero, the player is not invincible
	unsigned int invincibility_timer;

	// direction the player is facing in. should only ever be 0-3, clockwise from up
	int direction;
};