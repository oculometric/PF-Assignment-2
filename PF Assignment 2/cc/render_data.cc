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

inline void render_data::set_tiles(layer l, unsigned int i, string s)
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
	 // TODO
	read_multichars_to_buffer(s.c_str(), buffer)
}
