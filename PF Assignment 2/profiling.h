#pragma once

#include <chrono>

struct profiling_timing_data
{
	std::chrono::steady_clock::duration setup_time, buffer_load_time, draw_time, player_actions_time, intersection_check_time, growth_time, misc_time, update_overlay_time;
};

inline std::chrono::steady_clock::duration get_now() { return std::chrono::high_resolution_clock::now().time_since_epoch(); }