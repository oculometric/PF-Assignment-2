#ifndef ENEMY_H
#define ENEMY_H

#include "object.h"

class enemy : object
{
	virtual void tick(float, renderer*);
	unsigned char hitbox_layer() { return LAYER_ENEMY_HITBOX | LAYER_ENEMY_HURTBOX; }

	using object::object;
};

#endif