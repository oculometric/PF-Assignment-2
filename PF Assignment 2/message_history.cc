#include "message_history.h"

#include "terminal_utils.h"

void message_history::append_line(string new_line)
{
	// check if the buffer is already fully populated
	if (lines_populated < maximum_lines)
	{
		// if not, append a new line to the list
		lines[lines_populated] = new_line;
		lines_populated++;
	}
	else
	{
		// if so, copy every line back by one
		for (unsigned int i = 0; i < maximum_lines - 1; i++)
		{
			lines[i] = lines[i + 1];
		}
		// then append the new line
		lines[maximum_lines - 1] = new_line;
	}
	// set the line highlighted
	highlight_line = lines_populated-1;
}

void message_history::clear_highlight()
{
	// clear the highlighted line
	highlight_line = -1;
}

void message_history::draw(ostream& out_stream)
{
	// set initial cursor position and draw header line
	set_cursor_pos(drawing_start);
	for (unsigned int j = 0; j < line_max_length; j++)
		out_stream << '_';

	// iterate over messages in buffer
	for (unsigned int i = 0; i < maximum_lines; i++)
	{
		// set cursor position to the start of the new line
		set_cursor_pos(drawing_start + ivector2{ 0, (int)i + 1 });
		unsigned int line_length = 0;

		// if the line should be highlighted, add a tag to it
		// otherwise just add spaces to keep messages aligned
		if (i == highlight_line) out_stream << "> ";
		else out_stream << "  ";

		// if the line is populated, print the text
		if (i < lines_populated)
		{
			out_stream << lines[i];
			line_length = (unsigned int)lines[i].length();
		}
		
		// fill the rest (or the whole, if the line was unpopulated) of
		// the line with spaces to clear any characters left over from
		// scrolling the message history up
		for (unsigned int j = line_length; j < line_max_length; j++)
			out_stream << ' ';
	}
}

void message_history::clear()
{
	// reset lines and highlight
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
