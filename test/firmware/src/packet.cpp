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
#include "packet.hpp"
#include "comm.hpp"
#include "debug.hpp"

#include <comm.h>

#define __ACK        0
#define __NAK        1
#define __SERIAL_5N1 0x00
#define __SERIAL_6N1 0x02
#define __SERIAL_7N1 0x04
#define __SERIAL_8N1 0x06
#define __SERIAL_5N2 0x08
#define __SERIAL_6N2 0x0A
#define __SERIAL_7N2 0x0C
#define __SERIAL_8N2 0x0E
#define __SERIAL_5E1 0x20
#define __SERIAL_6E1 0x22
#define __SERIAL_7E1 0x24
#define __SERIAL_8E1 0x26
#define __SERIAL_5E2 0x28
#define __SERIAL_6E2 0x2A
#define __SERIAL_7E2 0x2C
#define __SERIAL_8E2 0x2E
#define __SERIAL_5O1 0x30
#define __SERIAL_6O1 0x32
#define __SERIAL_7O1 0x34
#define __SERIAL_8O1 0x36
#define __SERIAL_5O2 0x38
#define __SERIAL_6O2 0x3A
#define __SERIAL_7O2 0x3C
#define __SERIAL_8O2 0x3E

#define READ_DATA(type, data) **((type**)(data)); (*(type**)(data))++

static void __sendAck(bool accepted) {
	uint8_t packet = accepted ? __ACK : __NAK;
	comm::write(&packet, 1);
}

static bool __toConfig(uint8_t rawCfg, hal::serial::Config* out) {
	switch (rawCfg) {
	case __SERIAL_5N1:
		if (out) *out = hal::serial::Config::CONFIG_5N1;
		break;
	case __SERIAL_6N1:
		if (out) *out = hal::serial::Config::CONFIG_6N1;
		break;
	case __SERIAL_7N1:
		if (out) *out = hal::serial::Config::CONFIG_7N1;
		break;
	case __SERIAL_8N1:
		if (out) *out = hal::serial::Config::CONFIG_8N1;
		break;
	case __SERIAL_5N2:
		if (out) *out = hal::serial::Config::CONFIG_5N2;
		break;
	case __SERIAL_6N2:
		if (out) *out = hal::serial::Config::CONFIG_6N2;
		break;
	case __SERIAL_7N2:
		if (out) *out = hal::serial::Config::CONFIG_7N2;
		break;
	case __SERIAL_8N2:
		if (out) *out = hal::serial::Config::CONFIG_8N2;
		break;
	case __SERIAL_5E1:
		if (out) *out = hal::serial::Config::CONFIG_5E1;
		break;
	case __SERIAL_6E1:
		if (out) *out = hal::serial::Config::CONFIG_6E1;
		break;
	case __SERIAL_7E1:
		if (out) *out = hal::serial::Config::CONFIG_7E1;
		break;
	case __SERIAL_8E1:
		if (out) *out = hal::serial::Config::CONFIG_8E1;
		break;
	case __SERIAL_5E2:
		if (out) *out = hal::serial::Config::CONFIG_5E2;
		break;
	case __SERIAL_6E2:
		if (out) *out = hal::serial::Config::CONFIG_6E2;
		break;
	case __SERIAL_7E2:
		if (out) *out = hal::serial::Config::CONFIG_7E2;
		break;
	case __SERIAL_8E2:
		if (out) *out = hal::serial::Config::CONFIG_8E2;
		break;
	case __SERIAL_5O1:
		if (out) *out = hal::serial::Config::CONFIG_5O1;
		break;
	case __SERIAL_6O1:
		if (out) *out = hal::serial::Config::CONFIG_6O1;
		break;
	case __SERIAL_7O1:
		if (out) *out = hal::serial::Config::CONFIG_7O1;
		break;
	case __SERIAL_8O1:
		if (out) *out = hal::serial::Config::CONFIG_8O1;
		break;
	case __SERIAL_5O2:
		if (out) *out = hal::serial::Config::CONFIG_5O2;
		break;
	case __SERIAL_6O2:
		if (out) *out = hal::serial::Config::CONFIG_6O2;
		break;
	case __SERIAL_7O2:
		if (out) *out = hal::serial::Config::CONFIG_7O2;
		break;
	case __SERIAL_8O2:
		if (out) *out = hal::serial::Config::CONFIG_8O2;
		break;
	default:
		return false;
	}

	return true;
}

static bool __toMode(uint8_t rawMode, comm::Mode* out) {
	switch (rawMode) {
	case 0:
		if (out) *out = comm::Mode::MESSAGE;
		break;

	case 1:
		if (out) *out = comm::Mode::PACKET;
		break;

	default:
		return false;
	}

	return true;
}

void packet::PING(uint8_t id, void* data, uint8_t szData) {
	__sendAck(true);
	comm::write(data, szData);
	return;
}

void packet::PROT(uint8_t id, void* data, uint8_t szData) {
	uint32_t   speed;
	uint8_t    rawCfg;
	uint8_t    rawMode;

	hal::serial::Config cfg;
	comm::Mode   mode;

	if (szData != 6) goto nak;

	speed   = READ_DATA(uint32_t, &data);
	rawCfg  = READ_DATA(uint8_t, &data);
	rawMode = READ_DATA(uint8_t, &data);

	if (speed == 0)                goto nak;
	if (!__toConfig(rawCfg, &cfg)) goto nak;
	if (!__toMode(rawMode, &mode)) goto nak;

	__sendAck(true);
	comm::init(speed, cfg, mode);
	return;

nak:
	__sendAck(false);
}

void packet::noHandler(uint8_t id, void* data, uint8_t szData) {
	__sendAck(false);
}
