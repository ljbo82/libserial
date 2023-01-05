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
#include "hal.hpp"
#include "hal/serial.hpp"
#include "serial.h"

#include <serial.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

static bool __toSerialCfg(const hal::serial::Config& cfg, serial_config_t* out) {
	#define mOut(assign) if (out) out->assign

	switch (cfg) {
	case hal::serial::Config::CONFIG_5N1:
		mOut(dataBits = SERIAL_DATA_BITS_5);
		mOut(parity = SERIAL_PARITY_NONE);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_6N1:
		mOut(dataBits = SERIAL_DATA_BITS_6);
		mOut(parity = SERIAL_PARITY_NONE);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_7N1:
		mOut(dataBits = SERIAL_DATA_BITS_7);
		mOut(parity = SERIAL_PARITY_NONE);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_8N1:
		mOut(dataBits = SERIAL_DATA_BITS_8);
		mOut(parity = SERIAL_PARITY_NONE);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_5N2:
		mOut(dataBits = SERIAL_DATA_BITS_5);
		mOut(parity = SERIAL_PARITY_NONE);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_6N2:
		mOut(dataBits = SERIAL_DATA_BITS_6);
		mOut(parity = SERIAL_PARITY_NONE);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_7N2:
		mOut(dataBits = SERIAL_DATA_BITS_7);
		mOut(parity = SERIAL_PARITY_NONE);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_8N2:
		mOut(dataBits = SERIAL_DATA_BITS_8);
		mOut(parity = SERIAL_PARITY_NONE);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_5E1:
		mOut(dataBits = SERIAL_DATA_BITS_5);
		mOut(parity = SERIAL_PARITY_EVEN);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_6E1:
		mOut(dataBits = SERIAL_DATA_BITS_6);
		mOut(parity = SERIAL_PARITY_EVEN);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_7E1:
		mOut(dataBits = SERIAL_DATA_BITS_7);
		mOut(parity = SERIAL_PARITY_EVEN);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_8E1:
		mOut(dataBits = SERIAL_DATA_BITS_8);
		mOut(parity = SERIAL_PARITY_EVEN);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_5E2:
		mOut(dataBits = SERIAL_DATA_BITS_5);
		mOut(parity = SERIAL_PARITY_EVEN);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_6E2:
		mOut(dataBits = SERIAL_DATA_BITS_6);
		mOut(parity = SERIAL_PARITY_EVEN);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_7E2:
		mOut(dataBits = SERIAL_DATA_BITS_7);
		mOut(parity = SERIAL_PARITY_EVEN);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_8E2:
		mOut(dataBits = SERIAL_DATA_BITS_8);
		mOut(parity = SERIAL_PARITY_EVEN);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_5O1:
		mOut(dataBits = SERIAL_DATA_BITS_5);
		mOut(parity = SERIAL_PARITY_ODD);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_6O1:
		mOut(dataBits = SERIAL_DATA_BITS_6);
		mOut(parity = SERIAL_PARITY_ODD);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_7O1:
		mOut(dataBits = SERIAL_DATA_BITS_7);
		mOut(parity = SERIAL_PARITY_ODD);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_8O1:
		mOut(dataBits = SERIAL_DATA_BITS_8);
		mOut(parity = SERIAL_PARITY_ODD);
		mOut(stopBits = SERIAL_STOP_BITS_1);
		break;

	case hal::serial::Config::CONFIG_5O2:
		mOut(dataBits = SERIAL_DATA_BITS_5);
		mOut(parity = SERIAL_PARITY_ODD);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_6O2:
		mOut(dataBits = SERIAL_DATA_BITS_6);
		mOut(parity = SERIAL_PARITY_ODD);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_7O2:
		mOut(dataBits = SERIAL_DATA_BITS_7);
		mOut(parity = SERIAL_PARITY_ODD);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	case hal::serial::Config::CONFIG_8O2:
		mOut(dataBits = SERIAL_DATA_BITS_8);
		mOut(parity = SERIAL_PARITY_ODD);
		mOut(stopBits = SERIAL_STOP_BITS_2);
		break;

	default:
		return false;
	}

	return true;
}

#if DEBUG_ENABLED
void hal::debug(const char* msg) {
	printf("%s\n", msg);
	fflush(stdout);
}
#endif // #if DEBUG_ENABLED

uint32_t hal::millis() {
	static uint64_t mStart = 0;

	if (mStart == 0) {
		struct timeval tvStart;
		gettimeofday(&tvStart, NULL);
		mStart = (tvStart.tv_sec * 1000000 + tvStart.tv_usec) / 1000;
	}

	struct timeval tvNow;
	gettimeofday(&tvNow, NULL);
	uint64_t now = (tvNow.tv_sec * 1000000 + tvNow.tv_usec) / 1000;

	return (uint32_t)now - mStart;
}

bool hal::serial::config(uint32_t baud, Config config) {
	serial_t* port = hal::serial::open();
	if (!port) {
		errno = SERIAL_ERROR_IO;
		return false;
	}
	serial_config_t serialCfg;
	if (!baud || !__toSerialCfg(config, &serialCfg)) {
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}
	serialCfg.baud = baud;
	return serial_config(port, &serialCfg);
}

bool hal::serial::setReadTimeout(uint32_t timeout) {
	serial_t* port = hal::serial::open();
	if (!port) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return serial_set_read_timeout(port, timeout);
}

uint32_t hal::serial::availableRead() {
	serial_t* port = hal::serial::open();
	if (!port) return 0;
	return serial_available(port);
}

uint32_t hal::serial::availableWrite() {
	serial_t* port = hal::serial::open();
	if (!port) return 0;
	return UINT32_MAX;
}

int32_t hal::serial::read(void* out, uint32_t len) {
	serial_t* port = hal::serial::open();
	if (!port) {
		errno = SERIAL_ERROR_IO;
		return -1;
	}

	return serial_read(port, out, len);
}

bool hal::serial::write(const void* in, uint32_t len) {
	serial_t* port = hal::serial::open();
	if (!port) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return serial_write(port, in, len);
}

bool hal::serial::flush() {
	serial_t* port = hal::serial::open();
	if (!port) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return serial_flush(port);
}

void hal::led::set(bool on) {}
