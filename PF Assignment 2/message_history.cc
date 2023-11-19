#include "message_history.h"

#include "terminal_utils.h"

void message_history::append_line(string new_line)
{
	if (lines_populated < maximum_lines)
	{
		// append a new line to the list
		lines[lines_populated] = new_line;
		lines_populated++;
	}
	else
	{
		// copy every line back by one
		for (int i = 0; i < maximum_lines - 1; i++)
		{
			lines[i] = lines[i + 1];
		}
		lines[maximum_lines - 1] = new_line;
	}
	// set the line highlighted
	highlight_line = lines_populated-1;
}

void message_history::clear_highlight()
{
	highlight_line = -1;
}

void message_history::draw(ostream& out_stream)
{
	set_cursor_pos(drawing_start);
	for (int j = 0; j < line_max_length; j++)
		out_stream << '_';

	for (int i = 0; i < maximum_lines; i++)
	{
		set_cursor_pos(drawing_start + ivector2{ 0, i + 1 });
		unsigned int line_length = 0;
		if (i == highlight_line) out_stream << "> ";
		else out_stream << "  ";
		if (i < lines_populated)
		{
			out_stream << lines[i];
			line_length = lines[i].length();
		}
		for (int j = line_length; j < line_max_length; j++)
			out_stream << ' ';
	}
}

void message_history::clear()
{
	lines_populated = 0;
	highlight_line = -1;
}

message_history::message_history(ivector2 offset, unsigned int capacity, unsigned int max_line_length)
{
	drawing_start = offset;
	maximum_lines = capacity;
	line_max_length = max_line_length;
	lines_populated = 0;
	highlight_line = -1;
	lines = new string[capacity];
}
