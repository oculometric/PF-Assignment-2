#ifndef OBJECT_H
#define OBJECT_H

#include "ivector2.h"
#include "player.h"

class object
{
public:
	// current position of object
	ivector2 position;
	
	// should return the ID of the sprite to draw for this object
	virtual unsigned int spritesheet_id() { return -1; }
	// should return the size of this object's hitbox (likely to be the same size as its sprite)
	ivector2 hitbox_size() { return ivector2{}; }

	// should return whether or not the player can walk onto this tile, and handle the collision
	bool contact_player(player* player_pointer) { return false; }
	// called after the object is spawned in a room
	void after_spawn() {};
	// called before the object is unloaded
	void before_destroy() {};

	object(int _x, int _y)
	{
		position = { _x, _y };
	}
};

class solid_pillar : public object
{
	unsigned int spritesheet_id() { return 0; }
	ivector2 hitbox_size() { return ivector2{ 1, 1 }; }

	bool contact_player(player* player_pointer) { return true; }
	void after_spawn() {};
	void before_destroy() {};

	using object::object;
};

class transparent_pillar : public object
{
	unsigned int spritesheet_id() { return 2; }
	ivector2 hitbox_size() { return ivector2{ 1, 1 }; }

	bool contact_player(player* player_pointer) { return true; }
	void after_spawn() {};
	void before_destroy() {};

	using object::object;
};

class health_pickup : public object
{
	unsigned int spritesheet_id() { return 1; }
	ivector2 hitbox_size() { return ivector2{ 1, 1 }; }

	bool contact_player(player* player_pointer) { player_pointer->health = player_pointer->max_health; return false; }
	void after_spawn() {};
	void before_destroy() {};

	using object::object;
};

#endif