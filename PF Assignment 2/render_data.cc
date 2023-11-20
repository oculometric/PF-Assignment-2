#include "render_data.h"

#include "terminal_utils.h"

void render_data::set_draw_offset(ivector2 v)
{
	draw_offset = v;
	line_offset = "";
	for (int i = 0; i < draw_offset.x; i++) line_offset += ' ';
	whole_screen.reserve((canvas_size.x + draw_offset.x) * canvas_size.y * 4);
}

ivector2 render_data::get_size()
{
	return canvas_size;
}

uint32_t render_data::get_tile(layer l, ivector2 t)
{
	if (t.x < 0 || t.y < 0 || t.x >= canvas_size.x || t.y >= canvas_size.y) return 0;
	return get_tile(l, t.x + (t.y * canvas_size.x));
}

uint32_t render_data::get_tile(layer l, unsigned int i)
{
	if (i >= canvas_length) return 0;
	switch (l)
	{
	case layer::BACKGROUND:
		return background_layer[i];
	case layer::FOREGROUND:
		return foreground_layer[i];
	case layer::OVERLAY:
		return overlay_layer[i];
	default:
		return 0;
	}
}

void render_data::set_tile(layer l, ivector2 t, uint32_t v)
{
	if (t.x < 0 || t.y < 0 || t.x >= canvas_size.x || t.y >= canvas_size.y) return;
	set_tile(l, t.x + (t.y * canvas_size.x), v);
}

void render_data::set_tile(layer l, unsigned int i, uint32_t v)
{
	if (i >= canvas_length) return;
	switch (l)
	{
	case layer::BACKGROUND:
		background_layer[i] = v;
		break;
	case layer::FOREGROUND:
		foreground_layer[i] = v;
		break;
	case layer::OVERLAY:
		overlay_layer[i] = v;
		break;
	}
}

void render_data::set_tiles(layer l, ivector2 t, string s, bool b)
{
	if (t.x < 0 || t.y < 0 || t.x >= canvas_size.x || t.y >= canvas_size.y) return;
	set_tiles(l, t.x + (t.y * canvas_size.x), s, b);
}

void render_data::set_tiles(layer l, unsigned int i, string s, bool b)
{
	if (i + s.length() >= canvas_length) return;

	uint32_t* buffer = NULL;
	switch (l)
	{
	case layer::BACKGROUND:
		buffer = background_layer;
		break;
	case layer::FOREGROUND:
		buffer = foreground_layer;
		break;
	case layer::OVERLAY:
		buffer = overlay_layer;
		break;
	}

	buffer += i;
	read_multichars_to_buffer((char *)s.c_str(), buffer, b);
}

void render_data::write_buffer(layer l, uint32_t* d)
{
	uint32_t* buffer = NULL;
	switch (l)
	{
	case layer::BACKGROUND:
		buffer = background_layer;
		break;
	case layer::FOREGROUND:
		buffer = foreground_layer;
		break;
	case layer::OVERLAY:
		buffer = overlay_layer;
		break;
	}

	if (buffer == NULL) return;

	memcpy(d, buffer, canvas_length * sizeof(uint32_t));
}

void render_data::read_buffer(layer l, uint32_t* d)
{
	uint32_t* buffer = NULL;
	switch (l)
	{
	case layer::BACKGROUND:
		buffer = background_layer;
		break;
	case layer::FOREGROUND:
		buffer = foreground_layer;
		break;
	case layer::OVERLAY:
		buffer = overlay_layer;
		break;
	}

	if (buffer == NULL) return;

	memcpy(buffer, d, canvas_length * sizeof(uint32_t));
}

void render_data::clear_layer(layer l)
{
	uint32_t* buffer = NULL;
	switch (l)
	{
	case layer::BACKGROUND:
		buffer = background_layer;
		break;
	case layer::FOREGROUND:
		buffer = foreground_layer;
		break;
	case layer::OVERLAY:
		buffer = overlay_layer;
		break;
	}

	if (buffer == NULL) return;

	memset(buffer, 0, canvas_length * sizeof(uint32_t));
}

void render_data::draw(ostream& s)
{
	set_cursor_pos(draw_offset);
	uint32_t* buffer;
	uint32_t chr;
	whole_screen.clear();
	for (unsigned int i = 0; i < canvas_length; i++)
	{
		buffer = overlay_layer;
		if (overlay_layer[i] == ' ' || overlay_layer[i] == 0)
		{
			buffer = foreground_layer;
			if (foreground_layer[i] == ' ' || foreground_layer[i] == 0) buffer = background_layer;
		}

		chr = buffer[i];
		if (chr == ' ' || chr == 0)
		{
			whole_screen += ' ';
		}
		else
		{
			unsigned char c0 = chr & 0b11111111;
			unsigned char c1 = (chr >> 8) & 0b11111111;
			unsigned char c2 = (chr >> 16) & 0b11111111;
			unsigned char c3 = (chr >> 24) & 0b11111111;

			if (c3 != 0) whole_screen += c3;
			if (c2 != 0) whole_screen += c2;
			if (c1 != 0) whole_screen += c1;
			if (c0 != 0) whole_screen += c0;
			else whole_screen += ' ';
		}
		if (i % canvas_size.x == canvas_size.x - 1)
		{
			whole_screen += '\n';
			whole_screen += line_offset;
		}
	}
	cout << whole_screen;
}

render_data::render_data(ivector2 v)
{
	canvas_length = v.x * v.y;
	canvas_size = v;
	draw_offset = ivector2{ 0,0 };

	background_layer = new uint32_t[canvas_length];
	foreground_layer = new uint32_t[canvas_length];
	overlay_layer = new uint32_t[canvas_length];

	whole_screen.reserve(canvas_length*4);
}

render_data::~render_data()
{
	delete[] background_layer;
	delete[] foreground_layer;
	delete[] overlay_layer;
}
