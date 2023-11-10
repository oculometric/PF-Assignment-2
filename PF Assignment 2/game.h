#pragma once

#include <map>

#include "ivector2.h"
#include "bomb.h"
#include "render_data.h"
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

	bool any;
};

// where the mainloop of the game happens
void game_main();

// draw doorways based on room position
void draw_doorways(ivector2, render_data*);

#define TRANSITION_DELAY 300

// check for and handle the player exiting the current room
ivector2 handle_door_transition(player_data*, render_data*, random_provider*, map<ivector2, cached_room*>*, ivector2, vector<bomb*>*, uint32_t**, uint32_t**);

// check the state of all the keys and put the results into a struct
void check_key_states(key_states&);

// update all the bombs in a list (reducing their timers, drawing them to the render buffer)
void update_bombs(vector<bomb*>*, render_data*, random_provider*);

// create a bomb explosion centered on a particular tile, with a particular radius
void explode_bomb(ivector2, int, render_data*, random_provider*);

// probability definitions for different types of item dropped by goop when it dies
#define HEALTH_PICKUP_CHANCE 0.1
#define BOMB_PICKUP_CHANCE 0.1
#define HEALTH_UPGRADE_CHANCE 0.02
#define RANGE_UPGRADE_CHANCE 0.01

#define DROP_DIV_0 HEALTH_PICKUP_CHANCE
#define DROP_DIV_1 DROP_DIV_0 + BOMB_PICKUP_CHANCE
#define DROP_DIV_2 DROP_DIV_1 + HEALTH_UPGRADE_CHANCE
#define DROP_DIV_3 DROP_DIV_2 + RANGE_UPGRADE_CHANCE

// probability that a goop tile will grow on a particular update
#define GOOP_GROW_CHANCE 0.3

// try to drop a pickup (bomb, health, max health, range) at the targeted tile
void try_drop_pickup(ivector2, render_data*, random_provider*, uint32_t);

// check for and handle the player intersecting with different kinds of tiles, including goop, bomb pickups, health pickups, max health upgrades, and range upgrades
void check_intersection(player_data*, render_data*);

// iterate over all goop tiles and try to grow them into adjacent tiles
void grow_goop_tiles(render_data*, random_provider*);

#define UP_ATTACK 0xe2afad
#define DOWN_ATTACK 0xe2afaf
#define LEFT_ATTACK 0xe2afac
#define RIGHT_ATTACK 0xe2afae

// handle performing player input actions
void perform_player_attack(player_data*, render_data*, random_provider*);
void perform_player_bomb(vector<bomb*>*, player_data*, render_data*);

// update HUD text about the player's health etc
void update_overlay_text(player_data*, render_data*, ivector2);

// utility functions for check intersections
bool is_goop_tile(uint32_t);
bool is_bomb_pickup(uint32_t);
bool is_health_pickup(uint32_t);
bool is_health_upgrade(uint32_t);
bool is_range_upgrade(uint32_t);
uint32_t advance_goop_tile(uint32_t);

// definitions for the characters that should represent different gameplay elements
#define GOOP_0 0xe2b8ab //0xe29691
#define GOOP_1 0xe2b8ac //0xe29692
#define GOOP_2 0xe2b8ad //0xe29693
#define GOOP_INIT 0xe2b8aa

#define BOMB_EXPLOSION '%'
#define PLAYER '@'
#define HEALTH_PICKUP '#'
#define HEALTH_UPGRADE '+'
#define BOMB_PICKUP '*'
#define RANGE_UPGRADE '^'

#define FILLED_HEART 0xe299a5
#define EMPTY_HEART 0xe299a1

#define BLOCK 0xe29688

// set the text cursor position in the console
void set_cursor_pos(ivector2);

// hide the text cursor in the console
void hide_cursor();

// converting between direction formats
ivector2 direction(int);
int direction(ivector2);