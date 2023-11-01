#ifndef PLAYER_H
#define PLAYER_H

#include "ivector2.h"
#include "game.h"
#include "object.h"
#include "physics.h"

class player : object
{
public:
	unsigned int health;
	unsigned int max_health;

	unsigned char facing = 0;

	float invincibility_timer = 0.0;
	float attack_timer = 0.0;

	void tick(float, game*);

	bool receive_damage(unsigned int, object*);
	void perform_attack();

	player(int, int);
};

#endif