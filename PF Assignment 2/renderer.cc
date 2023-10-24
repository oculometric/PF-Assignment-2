#include "renderer.h"

#include "spritesheet.h"

#include <iostream>
#include <Windows.h>
#include <bitset>

using namespace std;

// Based closely on this Gist: https://gist.github.com/karolisjan/643c77b56e555683d57d1fac6906c638
void renderer::set_cursor_pos(ivector2 pos)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = { (SHORT)pos.x, (SHORT)pos.y };
	cout.flush();
	SetConsoleCursorPosition(out, coord);
}

void renderer::hide_cursor()
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursor_info;
	GetConsoleCursorInfo(out, &cursor_info);
	cursor_info.bVisible = false;
	SetConsoleCursorInfo(out, &cursor_info);
}

void renderer::clear_buffer()
{
	memset(backbuffer, 0, viewport_size.x * viewport_size.y * sizeof(uint32_t));
}

void renderer::draw_screen()
{
	clear_buffer();
	draw_walls();
	draw_objects();
	blit();
}

void renderer::draw_walls()
{
	string horizontal_wall = "+";
	string horizontal_wall_door = "+";

	ivector2 center = viewport_size / 2;

	for (int i = 2; i < viewport_size.x - 2; i++)
	{
		horizontal_wall += "-";

		int offset_from_center = abs(i - center.x);

		if (offset_from_center == 0) horizontal_wall_door += " ";
		else if (offset_from_center == 1) horizontal_wall_door += "|";
		else horizontal_wall_door += "-";
	}

	horizontal_wall += "+";
	horizontal_wall_door += "+";

	string vertical_wall = "|";
	for (int j = 2; j < viewport_size.x - 2; j++)
	{
		vertical_wall += " ";
	}
	vertical_wall += "|";

	draw_line(ivector2{ 1,1 }, door_top ? horizontal_wall_door : horizontal_wall);
	
	for (int i = 2; i < viewport_size.y - 2; i++) draw_line(ivector2{ 1, i }, vertical_wall);
	
	if (door_left)
	{
		draw_line(ivector2{ 1, center.y - 1 }, "-");
		draw_line(ivector2{ 1, center.y }, " ");
		draw_line(ivector2{ 1, center.y + 1 }, "-");
	}

	if (door_right)
	{
		draw_line(ivector2{ viewport_size.x - 2, center.y - 1 }, "-");
		draw_line(ivector2{ viewport_size.x - 2, center.y }, " ");
		draw_line(ivector2{ viewport_size.x - 2, center.y + 1 }, "-");
	}

	draw_line(ivector2{ 1,viewport_size.y-2 }, door_bottom ? horizontal_wall_door : horizontal_wall);
}

void renderer::draw_objects()
{
	for (object * obj : objects)
	{
		unsigned int sprite_id = obj->spritesheet_id();
		const unsigned char* sprite = (const unsigned char*)spritesheet[sprite_id];
		draw_sprite(obj->position, sprite);
		
		// TODO: multiline sprite rendering
	}
	// TODO: render enemies
}

renderer::renderer(ivector2 viewport)
{
	viewport_size = viewport;
	backbuffer = new uint32_t[viewport_size.x * viewport_size.y];
	clear_buffer();
	hide_cursor();
	system("chcp 65001 1>nul");
}

void renderer::set_room(ivector2 room_offset)
{
	room_position = room_offset;
}

void renderer::construct_room()
{
	objects.clear();
	enemies.clear();

	unsigned char sides = generate_room(room_position, &objects, &enemies);

	door_top = sides & (1 << 0);
	door_bottom = sides & (1 << 1);
	door_left = sides & (1 << 2);
	door_right = sides & (1 << 3);
}

void renderer::draw_pixel(ivector2 pos, uint32_t c)
{
	backbuffer[(pos.y * viewport_size.x) + pos.x] = c;
}

void renderer::draw_line(ivector2 start, string str)
{
	uint32_t char_to_write = 0;
	int unicode_chars = 0;
	int char_offset = 0;
	for (int i = 0; i < str.length(); i++)
	{
		if (unicode_chars == 0)
		{
			if ((str[i] & 0b11110000) == 0b11110000)
			{
				unicode_chars = 3;
			}
			else if ((str[i] & 0b11100000) == 0b11100000)
			{
				unicode_chars = 2;
			}
			else if ((str[i] & 0b11000000) == 0b11000000)
			{
				unicode_chars = 1;
			}
			else if ((str[i] & 0b10000000) == 0b10000000)
			{
				unicode_chars = 0;
			}
		}
		char_to_write |= (str[i] << (unicode_chars*8));
		unicode_chars--;
		if (unicode_chars < 0)
		{
			unicode_chars = 0;
			backbuffer[((start.y * viewport_size.x) + start.x)+char_offset] = char_to_write;
			char_offset++;
			char_to_write = 0;
		}
	}
}

void renderer::draw_sprite(ivector2 pos, const unsigned char* sprite)
{
	uint32_t char_to_write = 0;
	int unicode_chars = 0;
	int char_offset = 0;
	for (int i = 0; sprite[i] != 0x0; i++)
	{
		if (unicode_chars == 0)
		{
			if ((sprite[i] & 0b11110000) == 0b11110000)
			{
				unicode_chars = 3;
			}
			else if ((sprite[i] & 0b11100000) == 0b11100000)
			{
				unicode_chars = 2;
			}
			else if ((sprite[i] & 0b11000000) == 0b11000000)
			{
				unicode_chars = 1;
			}
			else if ((sprite[i] & 0b10000000) == 0b10000000)
			{
				unicode_chars = 0;
			}
		}
		char_to_write |= (sprite[i] << (unicode_chars * 8));
		unicode_chars--;
		if (unicode_chars < 0)
		{
			unicode_chars = 0;
			backbuffer[((pos.y * viewport_size.x) + pos.x) + char_offset] = char_to_write;
			char_offset++;
			char_to_write = 0;
		}
	}
	// TODO Handle multiline thing
}

void renderer::blit()
{
	set_cursor_pos(ivector2{ 0,0 });
	for (int i = 0; i < viewport_size.x * viewport_size.y; i++)
	{
		uint32_t chr = backbuffer[i];
		char c0 = chr & 0b11111111;
		char c1 = (chr >> 8) & 0b11111111;
		char c2 = (chr >> 16) & 0b11111111;
		char c3 = (chr >> 24) & 0b11111111;

		if (c3 != 0) cout << c3;
		if (c2 != 0) cout << c2;
		if (c1 != 0) cout << c1;
		if (c0 != 0) cout << c0;
		else cout << ' ';
		if (i % viewport_size.x == viewport_size.x-1) cout << endl;
	}
}