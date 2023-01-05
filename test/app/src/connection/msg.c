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

#include "msg.h"

#include <serial.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define __MSG_MAX_LEN       128
#define __MSG_PING         "PING"
#define __MSG_PROT         "PROT"
#define __MSG_ACK          "ACK"
#define __MSG_NAK          "NAK"
#define __DEBUG_MSG_PREFIX "\033[90m"

static bool __startsWith(const char* str, const char* prefix) {
	while(*prefix != '\0') {
		if (*str != *prefix) {
			return false;
		}
		str++;
		prefix++;
	}

	return true;
}

static const char* __read_message(connection_t* connection) {
	while (true) {
		const char* msg = connection_read_msg(connection);
		if (!msg || !__startsWith(msg, __DEBUG_MSG_PREFIX)) {
			return msg;
		}
	}
}

bool connection_msg_ping(connection_t* connection, const char* msg) {
	char mMsg[__MSG_MAX_LEN + 1]; // __MSG_MAX_LEN + '\0'
	const char* rsp;

	if (snprintf(mMsg, sizeof(mMsg) - 1, "%s;%s", __MSG_PING, msg) >= (sizeof(mMsg) - 1)) {
		// msg was truncated
		errno = SERIAL_ERROR_MEM;
		return false;
	}

	if (!connection_write_msg(connection, mMsg)) return false;

	rsp = __read_message(connection);
	if (!rsp || strcmp(rsp, __MSG_ACK) != 0) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	rsp = __read_message(connection);
	if (!rsp || strcmp(rsp, msg) != 0) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

bool connection_msg_protocol(connection_t* connection, uint32_t baud, connection_config_e cfg, connection_mode_e mode) {
	char msg[__MSG_MAX_LEN + 1]; // __MSG_MAX_LEN + '\0'
	const char* rsp;

	if (snprintf(msg, sizeof(msg) - 1, "%s;%" PRIu32 ";%s;%d", __MSG_PROT, baud, connection_config_to_str(cfg), mode) >= (sizeof(msg) - 1)) {
		// msg was truncated
		errno = SERIAL_ERROR_MEM;
		return false;
	}

	if (!connection_write_msg(connection, msg)) return false;

	rsp = __read_message(connection);
	if (!rsp || strcmp(rsp, __MSG_ACK) != 0) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return connection_config(connection, baud, cfg);
}
