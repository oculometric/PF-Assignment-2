#include "render_data.h"

#include "terminal_utils.h"

void render_data::set_draw_offset(ivector2 v)
{
	// set the offset for drawing
	draw_offset = v;

	// create a string representing this offset, makes printing much easier
	line_offset = "";
	for (int i = 0; i < draw_offset.x; i++) line_offset += ' ';
	// ensure the reserved whole-screen string buffer is big enough with the new offset
	whole_screen.reserve((canvas_size.x + draw_offset.x) * canvas_size.y * 4);
}

ivector2 render_data::get_size()
{
	return canvas_size;
}

uint32_t render_data::get_tile(layer l, ivector2 t)
{
	// verify the requested tile is in bounds
	if (t.x < 0 || t.y < 0 || t.x >= canvas_size.x || t.y >= canvas_size.y) return 0;
	return get_tile(l, t.x + (t.y * canvas_size.x));
}

uint32_t render_data::get_tile(layer l, unsigned int i)
{
	// verify the requested tile is in bounds
	if (i >= canvas_length) return 0;
	// return value depending on buffer selection
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
	// verify the requested tile is in bounds
	if (t.x < 0 || t.y < 0 || t.x >= canvas_size.x || t.y >= canvas_size.y) return;
	set_tile(l, t.x + (t.y * canvas_size.x), v);
}

void render_data::set_tile(layer l, unsigned int i, uint32_t v)
{
	// verify the requested tile is in bounds
	if (i >= canvas_length) return;
	// set value depending on buffer selection
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
	// verify offset is in bounds
	if (t.x < 0 || t.y < 0 || t.x >= canvas_size.x || t.y >= canvas_size.y) return;
	set_tiles(l, t.x + (t.y * canvas_size.x), s, b);
}

void render_data::set_tiles(layer l, unsigned int i, string s, bool b)
{
	// verify the whole string will fit
	if (i + s.length() >= canvas_length) return;

	// select buffer based on layer selection
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

	// offset in buffer by i tiles
	buffer += i;
	// read string into buffer, accounting for UTF-8 multibyte characters
	read_multichars_to_buffer((char *)s.c_str(), buffer, b);
}

void render_data::write_buffer(layer l, uint32_t* d)
{
	// select buffer based on layer selection
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
	// copy contents of selected buffer to target memory address
	// hopefully the caller allocated enough memory
	memcpy(d, buffer, canvas_length * sizeof(uint32_t));
}

void render_data::read_buffer(layer l, uint32_t* d)
{
	// select buffer based on layer selection
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
	// copy contents of target memory address to selected buffer
	// hopefully the caller allocated enough memory
	memcpy(buffer, d, canvas_length * sizeof(uint32_t));
}

void render_data::clear_layer(layer l)
{
	// select buffer based on layer selection
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
	// fill buffer with zeros to clear it
	memset(buffer, 0, canvas_length * sizeof(uint32_t));
}

void render_data::draw(ostream& s)
{
	// position the cursor in the terminal, set up some variables we will reuse a lot
	set_cursor_pos(draw_offset);
	uint32_t chr;
	// clear the whole-screen buffer. this preserves capacity
	whole_screen.clear();
	// iterate over every tile
	for (unsigned int i = 0; i < canvas_length; i++)
	{
		// select the relevant tile from a buffer based on the contents of the layers
		// if the overlay is empty, draw the foreground
		// if the foreground is also empty, draw the background
		// otherwise draw the overlay
		chr = overlay_layer[i];
		if (chr == ' ' || chr == 0)
		{
			chr = foreground_layer[i];
			if (chr == ' ' || chr == 0) chr = background_layer[i];
		}

		if (chr == ' ' || chr == 0)
		{ 
			// append a blank character to the whole-screen buffer
			whole_screen += ' ';
		}
		else
		{
			// append multiple characters by splitting out the 
			// uint32_t-encoded UTF-8 character
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
			// if we've reached the end of a line, skip to the 
			// next and also append the line offset
			whole_screen += '\n';
			whole_screen += line_offset;
		}
	}
	
	// print out the whole-screen buffer
	// from testing it's much, much better (9x faster according to my test)
	// to push all the characters into the screen buffer as a (pre-reserved) string
	// and then print the whole thing to the screen, due to significant overhead
	// with 'cout <<'
	cout << whole_screen;
}

render_data::render_data(ivector2 v)
{
	canvas_length = v.x * v.y;
	canvas_size = v;
	draw_offset = ivector2{ 0,0 };

	// allocate buffers
	background_layer = new uint32_t[canvas_length];
	foreground_layer = new uint32_t[canvas_length];
	overlay_layer = new uint32_t[canvas_length];

	// reserve size of whole-screen buffer string to prevent repeatedly reallocating it
	whole_screen.reserve(canvas_length*4);
}

render_data::~render_data()
{
	// delete all buffers
	delete[] background_layer;
	delete[] foreground_layer;
	delete[] overlay_layer;
}
