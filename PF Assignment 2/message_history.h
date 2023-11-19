#pragma once

#include <vector>
#include <string>
#include "ivector2.h"

class message_history
{
private:
	ivector2 drawing_start;
	unsigned int maximum_lines;
	string* lines;
	unsigned int lines_populated;
	int highlight_line = -1;
	unsigned int line_max_length;

public:
	void append_line(string);
	void clear_highlight();
	void draw(ostream&);
	void clear();

	message_history(ivector2, unsigned int, unsigned int);
};