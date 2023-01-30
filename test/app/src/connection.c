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
#include <comm.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define __MSG_MAX_LEN    128
#define __READ_TIMEOUT   3000
#define __DEFAULT_BAUD   9600
#define __DEFAULT_CONFIG CONNECTION_CONFIG_8N1

struct __connection {
	serial_t*             port;
	comm_stream_t*        serialStream;
	comm_line_stream_t*   lineStream;
	comm_packet_stream_t* packetStream;
};

static uint32_t COMM_CALL __available_read(const comm_stream_t* stream) {
	connection_t* connection = comm_obj_data(stream);
	return serial_available(connection->port);
}

static int32_t COMM_CALL __read(comm_stream_t* stream, void* out, uint32_t len) {
	connection_t* connection = comm_obj_data(stream);
	return serial_read(connection->port, out, len);
}

static uint32_t COMM_CALL __available_write(const comm_stream_t* stream) {
	return UINT32_MAX;
}

static int32_t COMM_CALL __write(comm_stream_t* stream, const void* in, uint32_t len) {
	connection_t* connection = comm_obj_data(stream);

	if (serial_write(connection->port, in, len))
		return len;

	return -1;
}

static bool COMM_CALL __flush(comm_stream_t* stream) {
	connection_t* connection = comm_obj_data(stream);
	return serial_flush(connection->port);
}

static int __convert_comm_error(int commError) {
	switch (commError) {
	case COMM_ERROR_NOMEM:
		return SERIAL_ERROR_MEM;

	case COMM_ERROR_IO:
		return SERIAL_ERROR_IO;

	case COMM_ERROR_INVPARAM:
		return SERIAL_ERROR_INVALID_PARAM;

	default:
		return SERIAL_ERROR_UNKNOWN;
	}
}

static bool __purge(serial_t* port, uint32_t timeout) {
	if (!port) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	uint32_t previousTimeout = serial_get_read_timeout(port);

	if (!serial_set_read_timeout(port, timeout))
		return false;

	int previousError = errno;
	errno = SERIAL_ERROR_OK;

	// Discards all data until timeout (silence)
	int32_t mRead;
	while ((mRead = serial_read(port, NULL, 1)) >= 0);

	bool result;
	if (mRead < 0 && errno == SERIAL_ERROR_TIMEOUT) {
		errno = previousError;
		result = true;
	} else {
		result = false;
	}

	result = serial_set_read_timeout(port, previousTimeout) && result;

	return result;
}

static bool __serial_config(serial_t* port, uint32_t baud, connection_config_e config) {
	serial_config_t oldConf;
	serial_get_config(port, &oldConf);

	serial_config_t newConf;
	newConf.baud     = baud;
	newConf.dataBits = CONNECTION_CONFIG_DATA_BITS(config);
	newConf.parity   = CONNECTION_CONFIG_PARITY(config);
	newConf.stopBits = CONNECTION_CONFIG_STOP_BITS(config);

	switch(newConf.dataBits) {
	case SERIAL_DATA_BITS_5:
	case SERIAL_DATA_BITS_6:
	case SERIAL_DATA_BITS_7:
	case SERIAL_DATA_BITS_8:
		break;

	default:
		goto error;
	}

	switch(newConf.parity) {
	case SERIAL_PARITY_NONE:
	case SERIAL_PARITY_EVEN:
	case SERIAL_PARITY_ODD:
		break;

	default:
		goto error;
	}

	switch(newConf.stopBits) {
	case SERIAL_STOP_BITS_1:
	case SERIAL_STOP_BITS_2:
	case SERIAL_STOP_BITS_1_5:
		break;

	default:
		goto error;
	}

	if (memcmp(&newConf, &oldConf, sizeof(serial_config_t))) {
		return serial_config(port, &newConf) && __purge(port, __READ_TIMEOUT);
	}

	return true;

error:
	errno = SERIAL_ERROR_INVALID_PARAM;
	return false;
}


connection_t* connection_open(const char* portName) {
	static const comm_stream_controller_t mSerialStreamController = {
		.available_read          = __available_read,
		.read                    = __read,
		.available_write         = __available_write,
		.write                   = __write,
		.flush                   = __flush
	};

	connection_t* connection = malloc(sizeof(connection_t));
	if (!connection) {
		errno = SERIAL_ERROR_MEM;
		goto error;
	} else {
		memset(connection, 0, sizeof(connection_t));
	}

	connection->port = serial_open(portName);
	if (!connection->port) {
		goto error;
	}

	if (!__serial_config(connection->port, __DEFAULT_BAUD, __DEFAULT_CONFIG)) {
		goto error;
	}

	if (!serial_set_read_timeout(connection->port, __READ_TIMEOUT)) {
		goto error;
	}

	connection->serialStream = comm_stream_new(&mSerialStreamController, connection);
	if (!connection->serialStream) {
		errno = __convert_comm_error(errno);
		goto error;
	}

	connection->lineStream = comm_line_stream_new(connection->serialStream, __MSG_MAX_LEN, true, NULL, connection);
	if (!connection->lineStream) {
		errno = __convert_comm_error(errno);
		goto error;
	}

	connection->packetStream = comm_packet_stream_new(connection->serialStream, true, NULL, connection);
	if (!connection->lineStream) {
		errno = __convert_comm_error(errno);
		goto error;
	}

	if (!__purge(connection->port, __READ_TIMEOUT))
		goto error;

	#if DEBUG_ENABLED
		if (!serial_set_read_timeout(connection->port, UINT32_MAX)) goto error;
	#endif

	return connection;

error:
	if (connection) {
		int previousError = errno;

		if (connection->packetStream) {
			comm_obj_del(connection->packetStream);
		}

		if (connection->lineStream) {
			comm_obj_del(connection->lineStream);
		}

		if (connection->serialStream) {
			comm_obj_del(connection->serialStream);
		}

		if (connection->port) {
			serial_close(connection->port);
		}

		free(connection);

		errno = previousError;
	}

	return NULL;
}

bool connection_config(connection_t* connection, uint32_t baud, connection_config_e config) {
	return __serial_config(connection->port, baud, config);
}

bool connection_close(connection_t* connection) {
	if (serial_close(connection->port)) {
		comm_obj_del(connection->serialStream);
		comm_obj_del(connection->lineStream);
		comm_obj_del(connection->packetStream);

		free(connection);
		return true;
	}

	return false;
}

char* connection_read_msg(connection_t* connection) {
	return comm_line_stream_read(connection->lineStream);
}

void* connection_read_packet(connection_t* connection, uint8_t* lenOut) {
	return comm_packet_stream_read(connection->packetStream, lenOut);
}

bool connection_write_msg(connection_t* connection, const char* msg) {
	return comm_line_stream_write(connection->lineStream, msg);
}

bool connection_write_packet(connection_t* connection, const void* packet, uint8_t szPacket) {
	return comm_packet_stream_write(connection->packetStream, packet, szPacket);
}
