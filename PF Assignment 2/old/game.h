#ifndef GAME_H
#define GAME_H

#include "renderer.h"
#include "physics.h"

class game
{
public:
	void main();

	void tick();

	renderer* game_renderer;
	physics* game_physics;
};

#endif