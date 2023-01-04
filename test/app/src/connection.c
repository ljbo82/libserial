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

#include "connection.h"

#include <serial.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define __MSG_MAX_LEN    128
#define __READ_TIMEOUT   3000
#define __DEFAULT_BAUD   9600
#define __DEFAULT_CONFIG CONNECTION_CONFIG_8N1
#define __CMD_DELIMITER  '\n'
#define __ACK            "ACK"
#define __NAK            "NAK"

struct __connection {
	serial_t* port;
	char msgBuffer[__MSG_MAX_LEN + 2]; // __MSG_MAX_LEN + __CMD_DELIMITER + '\0'
};

static const char* __find(const char* msg, char c) {
	const char* cursor = msg;
	while(*cursor != '\0') {
		if (*cursor == c) {
			return cursor;
		}
		cursor++;
	}

	return NULL;
}

connection_t* connection_open(const char* portName) {
	connection_t* connection = malloc(sizeof(connection_t));

	if (!connection) {
		errno = SERIAL_ERROR_MEM;
		goto error;
	}

	connection->port = serial_open(portName);
	if (!connection->port)
		goto error;

	if (!connection_config(connection, __DEFAULT_BAUD, __DEFAULT_CONFIG))
		goto error;

	if (!serial_set_read_timeout(connection->port, __READ_TIMEOUT))
		goto error;

	if (!connection_purge(connection))
		goto error;

	return connection;

error:
	if (connection) {
		if (connection->port) {
			int previousError = errno;
			serial_close(connection->port);
			errno = previousError; // Discards errors during serial_close()
		}

		free(connection);
	}
	return NULL;
}

bool connection_purge(connection_t* connection) {
	int previousError = errno;
	errno = SERIAL_ERROR_OK;

	// Discards all data until timeout (silence)
	int32_t mRead;
	while ((mRead = serial_read(connection->port, NULL, 1)) > 0);

	if (mRead < 0 && errno == SERIAL_ERROR_TIMEOUT) {
		errno = previousError;
		return serial_purge(connection->port, SERIAL_PURGE_TYPE_RX_TX);
	} else {
		return false;
	}
}

bool connection_config(connection_t* connection, uint32_t baud, connection_config_e config) {
	serial_config_t serialConfig;

	serialConfig.baud     = baud;
	serialConfig.dataBits = CONNECTION_CONFIG_DATA_BITS(config);
	serialConfig.parity   = CONNECTION_CONFIG_PARITY(config);
	serialConfig.stopBits = CONNECTION_CONFIG_STOP_BITS(config);

	switch(serialConfig.dataBits) {
	case SERIAL_DATA_BITS_5:
	case SERIAL_DATA_BITS_6:
	case SERIAL_DATA_BITS_7:
	case SERIAL_DATA_BITS_8:
		break;

	default:
		goto error;
	}
	switch(serialConfig.parity) {
	case SERIAL_PARITY_NONE:
	case SERIAL_PARITY_EVEN:
	case SERIAL_PARITY_ODD:
		break;

	default:
		goto error;
	}
	switch(serialConfig.stopBits) {
	case SERIAL_STOP_BITS_1:
	case SERIAL_STOP_BITS_2:
	case SERIAL_STOP_BITS_1_5:
		break;

	default:
		goto error;
	}

	return serial_config(connection->port, &serialConfig);

error:
	errno = SERIAL_ERROR_INVALID_PARAM;
	return false;
}

const char* connection_to_str(const connection_t* connection, char* buf, size_t szBuf) {
	serial_config_t config;
	serial_get_config(connection->port, &config);

	char chDataBits;
	switch (config.dataBits) {
	case SERIAL_DATA_BITS_5:
	case SERIAL_DATA_BITS_6:
	case SERIAL_DATA_BITS_7:
	case SERIAL_DATA_BITS_8:
		chDataBits = '5' + (config.dataBits - SERIAL_DATA_BITS_5);
		break;

	default:
		goto error;
	}

	char chParity;
	switch (config.parity) {
	case SERIAL_PARITY_NONE:
		chParity = 'N';
		break;

	case SERIAL_PARITY_EVEN:
		chParity = 'E';
		break;

	case SERIAL_PARITY_ODD:
		chParity = 'O';
		break;

	default:
		goto error;
	}

	const char* chStopBits;
	switch (config.stopBits) {
	case SERIAL_STOP_BITS_1:
		chStopBits = "1";
		break;

	case SERIAL_STOP_BITS_1_5:
		chStopBits = "1.5";
		break;

	case SERIAL_STOP_BITS_2:
		chStopBits = "2";
		break;

	default:
		goto error;
	}

	if (snprintf(buf, szBuf - 1, "%s (%" PRIu32 " %c%c%s)", serial_get_name(connection->port), config.baud, chDataBits, chParity, chStopBits) >= (szBuf - 1)) {
		errno = SERIAL_ERROR_MEM;
		return NULL;
	}

	return buf;

error:
	errno = SERIAL_ERROR_INVALID_PARAM;
	return NULL;
}

bool connection_close(connection_t* connection) {
	if (serial_close(connection->port)) {
		free(connection);
		return true;
	}

	return false;
}

bool connection_send_msg(connection_t* connection, const char* msg) {
	size_t msgLen = strlen(msg);
	if (msgLen > __MSG_MAX_LEN) {
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	if (__find(msg, __CMD_DELIMITER)) {
		// Malformed msg (it cannot contain __CMD_DELIMITER char in the payload)
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	if (!serial_write(connection->port, msg, msgLen))
		return false;

	char delimiter = __CMD_DELIMITER;
	if (!serial_write(connection->port, &delimiter, 1))
		return false;

	if (!serial_flush(connection->port))
		return false;

	// Check ACK
	const char* ack = connection_read_msg(connection);

	if (strcmp(ack, __ACK) == 0) {
		return true;
	} else if (strcmp(ack, __NAK) == 0) {
		return false;
	} else {
		// Invalid acknowledge
		connection_purge(connection);
		errno = SERIAL_ERROR_IO;
		return false;
	}
}

const char* connection_read_msg(connection_t* connection) {
	uint32_t totalRead    = 0;
	char*    buf          = connection->msgBuffer;
	bool     debugMessage = false;

	while (totalRead < sizeof(connection->msgBuffer) - 1) {
		if (serial_read(connection->port, buf, 1) < 0)
			return NULL;

		if (!debugMessage && totalRead == 0 && *buf == '\033')
			debugMessage = true;

		if (*buf == __CMD_DELIMITER) {
			*buf = '\0';

			if (debugMessage) {
				// Discards debug message
				debugMessage = false;
				continue;
			} else {
				return connection->msgBuffer;
			}
		}

		totalRead += debugMessage ? 0 : 1;
		buf += debugMessage ? 0 : 1;
	}

	//  __CMD_DELIMITER was not found (message is too big)
	connection_purge(connection);
	return NULL;
}
