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

#include "cmd.h"

#include <serial.h>

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define __MSG_MAX_LEN 128

static bool __str_starts_with(const char* str, const char* prefix) {
	while (*prefix != '\0') {
		if (*str != *prefix) {
			return false;
		}
		str++;
		prefix++;
	}

	return true;
}

bool connection_cmd_ping(connection_t* connection, const char* msg) {
	char mMsg[__MSG_MAX_LEN + 1]; // __MSG_MAX_LEN + '\0'
	if (snprintf(mMsg, sizeof(mMsg) - 1, "PING;%s", msg) >= (sizeof(mMsg) - 1)) {
		// msg was truncated
		errno = SERIAL_ERROR_MEM;
		return false;
	}

	if (!connection_send_msg(connection, mMsg))
		return false;

	const char* rsp = connection_read_msg(connection);

	if (!rsp)
		return false;

	if (!__str_starts_with(rsp, "PING;")) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	rsp += 5;
	if (strcmp(msg, rsp) != 0) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

bool connection_cmd_protocol(connection_t* connection, uint32_t baud, connection_config_e cfg) {
	char msg[__MSG_MAX_LEN + 1]; // __MSG_MAX_LEN + '\0'
	if (snprintf(msg, sizeof(msg) - 1, "PROT;%" PRIu32 ";%s", baud, connection_config_to_str(cfg)) >= (sizeof(msg) - 1)) {
		// msg was truncated
		errno = SERIAL_ERROR_MEM;
		return false;
	}

	return connection_send_msg(connection, msg)
	       && connection_config(connection, baud, cfg);
}
