#include "player.h"
#include "ivector2.h"
#include <cmath>

#include <Windows.h>

void player::tick(float delta_time, game* _game)
{
	// update timers
	invincibility_timer -= delta_time;
	if (invincibility_timer < 0.0) invincibility_timer = 0.0;
	attack_timer -= delta_time;
	if (attack_timer < 0.0) attack_timer = 0.0;
	

	// query inputs
	ivector2 movement;
	if (GetAsyncKeyState(VK_RIGHT)) movement.x += 1;
	if (GetAsyncKeyState(VK_LEFT)) movement.x -= 1;
	if (GetAsyncKeyState(VK_UP)) movement.y -= 1;
	if (GetAsyncKeyState(VK_DOWN)) movement.y += 1;

	bool wants_to_attack = GetAsyncKeyState('X');

	// turn character
	if (magnitude_squared(movement) > 0)
	{
		if (movement.x > 0) facing = 1;
		else if (movement.x < 0) facing = 3;
		else if (movement.y > 0) facing = 2;
		else if (movement.y < 0) facing = 0;
	}

	// TODO: check collision for movement
	//if ()
	//{
		position = position + movement;
	//}

	if (wants_to_attack && invincibility_timer == 0.0 && attack_timer == 0.0)
	{
		perform_attack();// TODO: perform an attack
	}
}

player::player(int _x, int _y)
{
	health = max_health;
	position = { _x, _y };
}
