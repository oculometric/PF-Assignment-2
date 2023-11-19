﻿#pragma once

#include <Windows.h>
#include <set>

#include "ivector2.h"
#include "bomb.h"
#include "render_data.h"
#include "message_history.h"
#include "random_provider.h"
#include "player_data.h"

// struct for passing data about the state of inputs
struct key_states
{
	bool move_up;
	bool move_down;
	bool move_left;
	bool move_right;

	bool attack;
	bool alt_attack;
	bool barrier_attack;

	bool any;
};

// player capability initial constants
#define PLAYER_INITIAL_HEALTH 5
#define PLAYER_INITIAL_RANGE 2
#define PLAYER_INITIAL_BOMBS 3

// time between transition frames for room transitions
#define TRANSITION_DELAY 150

// probability definitions for different types of item dropped by goop when it dies
#define HEALTH_PICKUP_CHANCE 0.02
#define BOMB_PICKUP_CHANCE 0.04
#define HEALTH_UPGRADE_CHANCE 0.004
#define RANGE_UPGRADE_CHANCE 0.002
#define BARRIER_PICKUP_CHANCE 0.02

#define DROP_DIV_0 HEALTH_PICKUP_CHANCE
#define DROP_DIV_1 DROP_DIV_0 + BOMB_PICKUP_CHANCE
#define DROP_DIV_2 DROP_DIV_1 + HEALTH_UPGRADE_CHANCE
#define DROP_DIV_3 DROP_DIV_2 + RANGE_UPGRADE_CHANCE
#define DROP_DIV_4 DROP_DIV_3 + BARRIER_PICKUP_CHANCE

// probability that a goop tile will grow on a particular update
#define GOOP_GROW_CHANCE 0.05
// maximum amount of goop tiles allowed to spawn in a room, which gets increased as the game progresses
#define MAX_GOOP_INITIAL 5
// inital goop spawns in a room increase by 1/this as a factor of the player's goop_cleared property
#define GOOP_CLEARING_RATIO 100
// how many times to grow the goop before the room is fully loaded
#define GOOP_GEN_ITERATIONS 50

// definitions for character symbols representing different game elements
#define UP_ATTACK 0xe2afad
#define DOWN_ATTACK 0xe2afaf
#define LEFT_ATTACK 0xe2afac
#define RIGHT_ATTACK 0xe2afae

#define GOOP_0 0xe2b8ab
#define GOOP_1 0xe2b8ac
#define GOOP_2 0xe2b8ad
#define GOOP_INIT 0xe2b8aa

#define BOMB_EXPLOSION '%'
#define BARRIER 0xe29692
#define PLAYER '@'
#define HEALTH_PICKUP '#'
#define HEALTH_UPGRADE '+'
#define BOMB_PICKUP '*'
#define RANGE_UPGRADE '^'
#define BARRIER_PICKUP '$'

#define FILLED_HEART 0xe299a5
#define EMPTY_HEART 0xe299a1

#define BLOCK 0xe29688

// constants for calculating the player's score
#define GOOP_SCORE_MULTIPLIER 25
#define TURNS_SCORE_MULTIPLIER -0.5
#define HEALTH_SCORE_MULTIPLIER 100
#define RANGE_SCORE_MULTIPLER 250
#define ROOMS_SCORE_MULTIPLIER 2000

// class encapsulating all of the actual gameplay code
class game
{
private:
	ivector2 rd_size;
	render_data* rd;
	player_data* pd;
	random_provider* rp;
	message_history* mh;
	ivector2 room;
	vector<bomb*> bombs;
	key_states ks;
	uint32_t* room_designs[NUM_ROOM_LAYOUTS + 2] = { NULL };
	uint32_t* transition_frames[NUM_TRANSITIONS] = { NULL };
	set<ivector2> cleared_rooms;

	// draw doorways based on room position
	void draw_doorways(ivector2);
	// update HUD text about the player's health etc
	void update_overlay_text();

	// check the state of all the keys and put the results into a struct
	void check_key_states();

	// check for and handle the player exiting the current room
	ivector2 handle_door_transition();

	// handle performing player input actions
	void perform_player_attack();
	void perform_player_bomb();
	void perform_player_barrier();
	// converting between direction formats
	static ivector2 direction(int);
	static int direction(ivector2);
	static uint32_t direction_char(int);

	// update all the bombs in a list (reducing their timers, drawing them to the render buffer)
	void update_bombs();
	// create a bomb explosion centered on a particular tile, with a particular radius
	void explode_bomb(ivector2, int);

	// try to drop a pickup (bomb, health, max health, range) at the targeted tile
	void try_drop_pickup(ivector2, uint32_t);
	// check for and handle the player intersecting with different kinds of tiles, including goop, bomb pickups, health pickups, max health upgrades, and range upgrades
	void check_intersection();
	// utility functions for check intersections
	static bool is_goop_tile(uint32_t);
	static bool is_bomb_pickup(uint32_t);
	static bool is_health_pickup(uint32_t);
	static bool is_health_upgrade(uint32_t);
	static bool is_range_upgrade(uint32_t);
	static bool is_barrier_pickup(uint32_t);

	// iterate over all goop tiles and try to grow them into adjacent tiles
	bool grow_goop_tiles();
	// generate a fresh room of goop for the player to fight. masked based on the room layout chosen
	void generate_fresh_goop(unsigned int, unsigned int);
	// iterate to the next graphical version of a goop tile
	static uint32_t advance_goop_tile(uint32_t);

	// calculate the player's score based on their number of turns and number of goop tiles destroyed
	unsigned int calculate_score();
public:
	// where the mainloop of the game happens
	void game_main();
};

// clamp an int between two other ints
static unsigned int clamp(unsigned int x, unsigned int a, unsigned int b) { return max(min(x, b), a); }