/*
Copyright (c) 2023 Leandro José Britto de Oliveira

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
#include "serial.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static serial_t* __port = NULL;

static bool __charEqualsAny(char c, const char* chars) {
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
		if (!__charEqualsAny(buf[i -1], " \t\n\r")) {
			buf[i] = '\0';
			break;
		}
	}

	// left trimm
	const char* str = NULL;
	for (size_t i = 0;;i++) {
		if (!str && !__charEqualsAny(buf[i], " \t\n\r")) {
			str = buf + i;
			break;
		}
	}
	if (str == NULL)
		str = buf;

	return str;
}

const char* __consoleRead(const char* prompt, char* buf, size_t szBuf) {
	printf("%s", prompt);
	fflush(stdout);

	if (fgets(buf, szBuf - 1, stdin)) {
		return __trim(buf);
	} else {
		return NULL;
	}
}

size_t __getOption(const char* prompt, size_t numOptions) {
	if (numOptions > UINT8_MAX)
		return -1;

	char buf[16];
	const char* option;

	while(true) {
		if (!(option = __consoleRead(prompt, buf, sizeof(buf))))
			return -1;

		char* end;
		size_t i = strtoul(option, &end, 10);
		if (*end != '\0') {
			goto invalid;
		}

		if (i >= (numOptions)) {
		invalid:
			printf("Invalid option!\n");
			continue;
		}

		return i;
	}
}

static const char* __choosePort(serial_list_t* portList) {
	printf("Available serial ports:\n\n");

	for (size_t i = 0; i < serial_list_size(portList); i++) {
		printf("%zu) %s\n", i, serial_list_item(portList, i));
	}

	size_t index = __getOption("\nChoose a port: ", serial_list_size(portList));
	return serial_list_item(portList, index);
}

serial_t* hal::serial::open() {
	serial_list_t* portList = NULL;

	if (!__port) {
		portList = serial_list_new();
		if (!portList) goto error;
		if (!serial_list_ports(portList)) goto error;
		switch (serial_list_size(portList)) {
		case 0:
			printf("No serial port found\n");
			goto error;

		case 1:
			__port = serial_open(serial_list_item(portList, 0));
			if (!__port) goto error;
			break;

		default:
			__port = serial_open(__choosePort(portList));
			if (!__port) goto error;
			break;
		}

		serial_list_del(portList);
		return __port;
	}

	return __port;

error:
	if (portList) serial_list_del(portList);
	return NULL;
}

bool hal::serial::close() {
	if (__port) {
		if (serial_close(__port)) {
			__port = NULL;
			return true;
		}

		return false;
	}

	return true;
}
