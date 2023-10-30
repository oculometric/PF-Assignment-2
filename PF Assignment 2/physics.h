#ifndef PHYSICS_H
#define PHYSICS_H

#define LAYER_OBSTACLE			0b00000001 // obstacles like walls, pillars, etc
#define LAYER_PICKUP			0b00000010 // items that will be picked up by the player, like heals etc
#define LAYER_PLAYER_HITBOX		0b00000100 // where the player can be damaged by things
#define LAYER_PLAYER_HURTBOX	0b00001000 // where the player can damage other things, i.e. the sword
#define LAYER_ENEMY_HITBOX		0b00010000 // where an enemy can be damaged by things
#define LAYER_ENEMY_HURTBOX		0b00100000 // where an enemy can damage other things like the player
#define LAYER_GENERAL_HURDBOX	0b01000000 // objects that will damage the player and enemies, like spikes, explosions, etc

// TODO: physics & collisions

struct collision_data
{
	ivector2 point;
	object* hit_object;
	unsigned char layer;
};

class physics
{
public:
	static bool check_collision(renderer*, unsigned int, collision_data&, unsigned char);
	static bool check_collision(renderer*, ivector2, collision_data&, unsigned char);
	static bool check_ray(renderer*, ivector2, ivector2, collision_data&, unsigned char);
	static bool check_box(renderer*, ivector2, ivector2, collision_data&, unsigned char);
};

#endif