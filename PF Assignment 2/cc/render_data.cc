#include "render_data.h"

inline ivector2 render_data::get_size()
{
	return canvas_size;
}

inline uint32_t render_data::get_tile(layer l, ivector2 t)
{
	if (t.x < 0 || t.y < 0 || t.x >= canvas_size.x || t.y >= canvas_size.y) return 0;
	return get_tile(l, t.x + (t.y * canvas_size.x));
}

inline uint32_t render_data::get_tile(layer l, unsigned int i)
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
	}
}

inline void render_data::set_tile(layer l, ivector2 t, uint32_t v)
{
	if (t.x < 0 || t.y < 0 || t.x >= canvas_size.x || t.y >= canvas_size.y) return;
	set_tile(l, t.x + (t.y * canvas_size.x), v);
}

inline void render_data::set_tile(layer l, unsigned int i, uint32_t v)
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

inline void render_data::set_tiles(layer l, ivector2 t, string s, bool b)
{
	if (t.x < 0 || t.y < 0 || t.x >= canvas_size.x || t.y >= canvas_size.y) return;
	set_tiles(l, t.x + (t.y * canvas_size.x), s, b);
}

inline void render_data::set_tiles(layer l, unsigned int i, string s, bool b)
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
	read_multichars_to_buffer(s.c_str(), buffer, b);
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

	memcpy(d, buffer, canvas_length);
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

	memcpy(buffer, d, canvas_length);
}

void render_data::draw(ostream& s)
{// TODO: small optimisations
	for (int i = 0; i < canvas_length; i++)
	{
		uint32_t chr = backbuffer[i];
		char c0 = chr & 0b11111111;
		char c1 = (chr >> 8) & 0b11111111;
		char c2 = (chr >> 16) & 0b11111111;
		char c3 = (chr >> 24) & 0b11111111;

		if (c3 != 0) s << c3;
		if (c2 != 0) s << c2;
		if (c1 != 0) s << c1;
		if (c0 != 0) s << c0;
		else s << ' ';
		if (i % canvas_size.x == canvas_size.x - 1) s << endl;
	}
}

render_data::render_data(ivector2 v)
{
	canvas_length = v.x * v.y;
	canvas_size = v;

	background_layer = new uint32_t[canvas_length];
	foreground_layer = new uint32_t[canvas_length];
	overlay_layer = new uint32_t[canvas_length];
}

render_data::~render_data()
{
	delete[] background_layer;
	delete[] foreground_layer;
	delete[] overlay_layer;
}
