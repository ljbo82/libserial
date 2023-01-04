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

#include "console.h"
#include "stack.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

static bool __char_equals_any(char c, const char* chars) {
	for (size_t i = 0; chars[i] != '\0'; i++) {
		if (c == chars[i]) {
			return true;
		}
	}
	return false;
}

static const char* __trim(char* buf) {
	// right trimm
	for (size_t i = strlen(buf); i > 0; i--) {
		if (!__char_equals_any(buf[i -1], " \t\n\r")) {
			buf[i] = '\0';
			break;
		}
	}

	// left trimm
	const char* str = NULL;
	for (size_t i = 0;;i++) {
		if (!str && !__char_equals_any(buf[i], " \t\n\r")) {
			str = buf + i;
			break;
		}
	}
	if (str == NULL)
		str = buf;

	return str;
}

static void __set_color(console_ansi_color_e color) {
	printf("\033[%dm", color);
}

const char* console_read(const char* prompt, char* buf, size_t szBuf) {
	printf("%s", prompt);
	fflush(stdout);

	if (fgets(buf, szBuf - 1, stdin)) {
		return __trim(buf);
	} else {
		return NULL;
	}
}

const char* console_get_option(const char* prompt, char* buf, size_t szBuf, size_t numOptions, ...) {
	const char** options = stack_malloc(sizeof(char*) * numOptions);
	va_list args;
	va_start(args, numOptions);
	for (size_t i = 0; i < numOptions; i++) {
		options[i] = va_arg(args, const char*);
	}
	va_end(args);

	const char* option;
	while(true) {
		if (!(option = console_read(prompt, buf, szBuf)))
			return NULL;

		for (size_t i = 0; i < numOptions; i++) {
			if (strcmp(option, options[i]) == 0) {
				return option;
			}
		}

		printf("Invalid option\n");
	}
}

int console_get_num_option(const char* prompt, int numOptions) {
	if (numOptions > UINT8_MAX)
		return -1;

	char buf[16];
	const char* option;

	while(true) {
		if (!(option = console_read(prompt, buf, sizeof(buf))))
			return -1;

		char* end;
		int i = strtol(option, &end, 10);
		if (*end != '\0') {
			goto invalid;
		}

		if (i < 0 || i >= (numOptions)) {
		invalid:
			printf("Invalid option!\n");
			continue;
		}

		return i;
	}
}

void console_color_printf(console_ansi_color_e fg, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	__set_color(fg);
	vprintf(fmt, args);
	__set_color(CONSOLE_ANSI_COLOR_RESET);
}
