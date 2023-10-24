#include "room.h"
#include "spritesheet.h"
#include <string>

#include <chrono>

unsigned char generate_room(ivector2 room_offset, vector<object*>* objects_destination, vector<enemy*>* enemies_destination)
{
    // generate seeds for the room overall, and the doors, such that any two adjacent rooms will share the same door seed on their adjacent side
    huge_uint room_seed = get_seed(room_offset);
    huge_uint top_seed, bottom_seed, left_seed, right_seed;
    get_room_door_seeds(room_offset, top_seed, bottom_seed, left_seed, right_seed);
    bool door_top = top_seed % 2;
    bool door_bottom = bottom_seed % 2;
    bool door_left = left_seed % 2;
    bool door_right = right_seed % 2;
    // bitpack the door bools so they can be returned as a single value
    unsigned char door_configuration = (door_right << 3) | (door_left << 2) | (door_bottom << 1) | (door_top << 0);
    
    // use room seed to decide on a room layout
    const char* room_ptr = room_layout[room_seed % num_room_layouts];
    // unpack RLE encoded room layout
    int x_position = 2; // don't overwrite the walls
    int y_position = 2;

    size_t str_pos = 0;
    string current_block;
    while (room_ptr[str_pos] != NULL)
    {
        if ((room_ptr[str_pos + 1] == NULL || room_ptr[str_pos] != ' ') && room_ptr[str_pos] != '\n') current_block += room_ptr[str_pos];
        if ((room_ptr[str_pos] == ' ' || room_ptr[str_pos + 1] == NULL || room_ptr[str_pos] == '\n') && current_block.size() > 2)
        {
            string object_type = current_block.substr(0, 2);
            int length = stoi(current_block.substr(2));
            // process this RLE block
            for (int i = 0; i < length; i++)
            {
                object* obj = NULL;
                if (object_type == "sp") obj = new solid_pillar(x_position, y_position);
                else if (object_type == "tp") obj = new transparent_pillar(x_position, y_position);


                // TODO: Implement all the other command types
                if (obj) objects_destination->push_back(obj);
                x_position++;
            }

            current_block = "";
        }
        if (room_ptr[str_pos] == '\n')
        {
            y_position++;
            x_position = 2;
        }
        str_pos++;
    }
    return door_configuration;
}

void get_room_door_seeds(ivector2 room_offset, huge_uint& door_top, huge_uint& door_bottom, huge_uint& door_left, huge_uint& door_right)
{
    ivector2 door_offset = room_offset * 2;
    ivector2 bottom_offset = door_offset + ivector2{ 1, 0 };
    ivector2 top_offset = door_offset + ivector2{ 1, 2 };
    ivector2 left_offset = door_offset + ivector2{ 0,1 };
    ivector2 right_offset = door_offset + ivector2{ 2,1 };

    door_top = get_seed(top_offset);
    door_bottom = get_seed(bottom_offset);
    door_left = get_seed(left_offset);
    door_right = get_seed(right_offset);
}
