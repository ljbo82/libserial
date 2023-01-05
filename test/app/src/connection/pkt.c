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
#include "pkt.h"
#include <serial.h>
#include <errno.h>
#include <string.h>

#define __PACKET_ACK  0
#define __PACKET_NAK  1
#define __PACKET_PING 2
#define __PACKET_PROT 3

static uint8_t __buf[255];

static uint8_t __from_cfg(connection_config_e cfg) {
	switch (cfg) {
	case CONNECTION_CONFIG_5N1: return 0x00;
	case CONNECTION_CONFIG_6N1: return 0x02;
	case CONNECTION_CONFIG_7N1: return 0x04;
	case CONNECTION_CONFIG_8N1: return 0x06;
	case CONNECTION_CONFIG_5N2: return 0x08;
	case CONNECTION_CONFIG_6N2: return 0x0A;
	case CONNECTION_CONFIG_7N2: return 0x0C;
	case CONNECTION_CONFIG_8N2: return 0x0E;
	case CONNECTION_CONFIG_5E1: return 0x20;
	case CONNECTION_CONFIG_6E1: return 0x22;
	case CONNECTION_CONFIG_7E1: return 0x24;
	case CONNECTION_CONFIG_8E1: return 0x26;
	case CONNECTION_CONFIG_5E2: return 0x28;
	case CONNECTION_CONFIG_6E2: return 0x2A;
	case CONNECTION_CONFIG_7E2: return 0x2C;
	case CONNECTION_CONFIG_8E2: return 0x2E;
	case CONNECTION_CONFIG_5O1: return 0x30;
	case CONNECTION_CONFIG_6O1: return 0x32;
	case CONNECTION_CONFIG_7O1: return 0x34;
	case CONNECTION_CONFIG_8O1: return 0x36;
	case CONNECTION_CONFIG_5O2: return 0x38;
	case CONNECTION_CONFIG_6O2: return 0x3A;
	case CONNECTION_CONFIG_7O2: return 0x3C;
	case CONNECTION_CONFIG_8O2: return 0x3E;
	default: return 0x06;
	}
}

bool connection_pkt_ping(connection_t* connection, const void* in, uint8_t len) {
	if (len >= sizeof(__buf) - 1) {
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	__buf[0] = __PACKET_PING;
	for (size_t i = 0; i < len; i++) {
		__buf[i + 1] = ((uint8_t*)in)[i];
	}

	if (!connection_write_packet(connection, __buf, len + 1)) return false;

	uint8_t rspLen;
	uint8_t* rsp;

	rsp = connection_read_packet(connection, &rspLen);
	if (!rsp || rspLen != 1 || *rsp != __PACKET_ACK) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	rsp = connection_read_packet(connection, &rspLen);
	if (!rsp || rspLen != len || memcmp(in, rsp, len) != 0) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

bool connection_pkt_protocol(connection_t* connection, uint32_t baud, connection_config_e cfg, connection_mode_e mode) {
	uint8_t* buf = __buf;

	*buf = __PACKET_PROT;
	buf++;

	*((uint32_t*)buf) = baud;
	buf += sizeof(uint32_t);

	*buf = __from_cfg(cfg);
	buf++;

	*buf = (uint8_t)mode;
	buf++;

	if (!connection_write_packet(connection, __buf, buf - __buf)) return false;

	uint8_t rspLen;
	uint8_t* rsp;

	rsp = connection_read_packet(connection, &rspLen);
	if (!rsp || rspLen != 1 || *rsp != __PACKET_ACK) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return connection_config(connection, baud, cfg);
}
