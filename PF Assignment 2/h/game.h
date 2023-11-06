#pragma once

#include <list>

#include "ivector2.h"
#include "bomb.h"
#include "render_data.h"

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

	// range of the player's melee attack in tiles
	unsigned int range;

	// countdown timer for the player being invincible. if zero, the player is not invincible
	unsigned int invincibility_timer;
};

// where the mainloop of the game happens
void game_main();

// load the tutorial level
void load_tutorial();

// update all the bombs in a list (reducing their timers, drawing them to the render buffer)
void update_bombs(list<bomb*>*, render_data*);

// create a bomb explosion centered on a particular tile, with a particular radius
void explode_bomb(ivector2, int, render_data*);

// handle destroying a goop tile, potentially spawning a powerup in its place
void destroy_goop_tile(ivector2, render_data*);

// try to drop a pickup (bomb, health, max health, range) at the targeted tile
void try_drop_pickup(ivector2, render_data*);

// check for and handle the player intersecting with different kinds of tiles, including goop, bomb pickups, health pickups, max health upgrades, and range upgrades
void check_intersection(player_data*, render_data*);

// iterate over all goop tiles and try to grow them into adjacent tiles
void grow_goop_tiles(render_data*);

// handle performing player input actions
void perform_player_attack(player_data*, render_data*);
void perform_player_bomb(player_data*, render_data*);