#include "room.h"

void read_multichars_to_buffer(char* multichars, uint32_t* buffer, int newline_skipahead)
{
	uint32_t char_to_write = 0;
	int unicode_chars_expected = 0;
	int char_offset = 0;
	for (int i = 0; true; i++)
	{
		char c = multichars[i];
		if (c == '\0') break;
		if (c == '\n')
		{
			i += newline_skipahead;
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
