/*
Copyright (c) 2022 Leandro José Britto de Oliveira

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <stddef.h>

typedef enum console_ansi_color console_ansi_color_e;

enum console_ansi_color {
	CONSOLE_ANSI_COLOR_RESET          = 0,
	CONSOLE_ANSI_COLOR_BLACK          = 30,
	CONSOLE_ANSI_COLOR_RED            = 31,
	CONSOLE_ANSI_COLOR_GREEN          = 32,
	CONSOLE_ANSI_COLOR_YELLOW         = 33,
	CONSOLE_ANSI_COLOR_BLUE           = 34,
	CONSOLE_ANSI_COLOR_MAGENTA        = 35,
	CONSOLE_ANSI_COLOR_CYAN           = 36,
	CONSOLE_ANSI_COLOR_WHITE          = 37,
	CONSOLE_ANSI_COLOR_BRIGHT_BLACK   = 90,
	CONSOLE_ANSI_COLOR_BRIGHT_RED     = 91,
	CONSOLE_ANSI_COLOR_BRIGHT_GREEN   = 92,
	CONSOLE_ANSI_COLOR_BRIGHT_YELLOW  = 93,
	CONSOLE_ANSI_COLOR_BRIGHT_BLUE    = 94,
	CONSOLE_ANSI_COLOR_BRIGHT_MAGENTA = 95,
	CONSOLE_ANSI_COLOR_BRIGHT_CYAN    = 96,
	CONSOLE_ANSI_COLOR_BRIGHT_WHITE   = 97
};

const char* console_read(const char* prompt, char* buf, size_t szBuf);

const char* console_get_option(const char* prompt, char* buf, size_t szBuf, size_t numOptions, ...);

int console_get_num_option(const char* prompt, int numOptions);

void console_color_printf(console_ansi_color_e fg, const char* fmt, ...);
