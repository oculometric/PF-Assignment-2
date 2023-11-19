#pragma once

#include <Windows.h>
#include <iostream>
#include "ivector2.h"

// set the text cursor position in the console
static void set_cursor_pos(ivector2 pos)
{
	// based closely on this Gist: https://gist.github.com/karolisjan/643c77b56e555683d57d1fac6906c638
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = { (SHORT)pos.x, (SHORT)pos.y };
	cout.flush();
	SetConsoleCursorPosition(out, coord);
}

// hide the text cursor in the console
static void hide_cursor()
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursor_info;
	GetConsoleCursorInfo(out, &cursor_info);
	cursor_info.bVisible = false;
	SetConsoleCursorInfo(out, &cursor_info);
}