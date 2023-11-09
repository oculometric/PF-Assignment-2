#pragma once

#include "ivector2.h"

// struct describing a placed bomb
struct bomb
{
	// it has a position in the game map
	ivector2 position;

	// and a countdown timer
	unsigned int timer;
};