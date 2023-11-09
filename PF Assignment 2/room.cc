#include "room.h"

void read_multichars_to_buffer(char* multichars, uint32_t* buffer, bool skip_spaces)
{
	uint32_t char_to_write = 0;
	int unicode_chars_expected = 0;
	int char_offset = 0;
	for (int i = 0; true; i++)
	{
		unsigned char c = multichars[i];
		if (c == '\0') break;
		if (c == '\n' && unicode_chars_expected == 0)
		{
			continue;
		}
		if (c == ' ' && skip_spaces && unicode_chars_expected == 0)
		{
			char_offset++;
			continue;
		}

		if (unicode_chars_expected == 0)
		{
			if ((c & 0b11110000) == 0b11110000)
			{
				unicode_chars_expected = 3;
			}
			else if ((c & 0b11100000) == 0b11100000)
			{
				unicode_chars_expected = 2;
			}
			else if ((c & 0b11000000) == 0b11000000)
			{
				unicode_chars_expected = 1;
			}
			else if ((c & 0b10000000) == 0b10000000)
			{
				unicode_chars_expected = 0;
			}
		}
		char_to_write |= (c << (unicode_chars_expected * 8));
		unicode_chars_expected--;
		if (unicode_chars_expected < 0)
		{
			unicode_chars_expected = 0;
			buffer[char_offset] = char_to_write;
			char_offset++;
			char_to_write = 0;
		}
	}
}

unsigned char get_room_doors(ivector2 position)
{
	ivector2 door_offset = position * 2;
	ivector2 bottom_offset = door_offset + ivector2{ 1, 0 };
	ivector2 top_offset = door_offset + ivector2{ 1, 2 };
	ivector2 left_offset = door_offset + ivector2{ 0,1 };
	ivector2 right_offset = door_offset + ivector2{ 2,1 };

	bool door_top = get_seed(top_offset) % 3;
	bool door_bottom = get_seed(bottom_offset) % 3;
	bool door_left = get_seed(left_offset) % 3;
	bool door_right = get_seed(right_offset) % 3;

	unsigned char door_configuration = (door_right << 3) | (door_left << 2) | (door_bottom << 1) | (door_top << 0);

	return door_configuration;
}