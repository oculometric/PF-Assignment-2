#ifndef ROOM_H
#define ROOM_H

#include <vector>

#include "object.h"
#include "ivector2.h"
#include "enemy.h"

#define PRIME_0 131739228941
#define PRIME_1 773295369311
#define PRIME_2 523798441081
#define PRIME_3 686408054863

typedef unsigned long long int huge_uint;

using namespace std;

unsigned char generate_room(ivector2 room_offset, vector<object*>* objects_destination, vector<enemy*>* enemies_destination);

void get_room_door_seeds(ivector2 room_offset, huge_uint& door_top, huge_uint& door_bottom, huge_uint& door_left, huge_uint& door_right);

inline huge_uint get_seed(ivector2 position) { return (position.x * PRIME_0) + (position.y * PRIME_1); }

#endif