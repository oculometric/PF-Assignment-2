#pragma once

#include <vector>
#include <string>
#include "ivector2.h"

class message_history
{
private:
	// screen offset where drawing should start
	ivector2 drawing_start;
	// internal buffer of messages
	string* lines;
	// maximum number of messages to show on the screen
	unsigned int maximum_lines;
	// maximum length of any one line, used to clear each line when redrawing
	unsigned int line_max_length;
	// number of messages actually being shown
	unsigned int lines_populated;
	// index of message being highlighted
	int highlight_line = -1;

public:
	// add a new message to the buffer, and highlight it
	void append_line(string);
	// clear the highlight
	void clear_highlight();
	// output the message history to the screen
	void draw(ostream&);
	// clear all messages in the history
	void clear();

	// initialiser
	message_history(ivector2, unsigned int, unsigned int);
};