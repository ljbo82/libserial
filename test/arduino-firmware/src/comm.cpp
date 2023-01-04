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
#include "comm.hpp"
#include "debug.hpp"
#include "dispatcher.hpp"
#include "message.hpp"
#include "packet.hpp"

#include <comm.h>
#include <stdlib.h>
#include <inttypes.h>
#include <Arduino.h>

#define __READ_TIMEOUT_MILLIS 1000
#define __MSG_MAX_LEN         128
#define __CASE(mNamespace, mCase) case mNamespace::mCase: return #mCase

static comm_stream_t *       __serialStream = NULL;
static comm_line_stream_t*   __lineStream   = NULL;
static comm_packet_stream_t* __packetStream = NULL;
static long                  __speed        = COMM_DEFAULT_SPEED;
static comm::Config          __cfg          = COMM_DEFAULT_CFG;
static comm::Mode            __mode         = COMM_DEFAULT_MODE;

static uint32_t COMM_CALL __available_read(const comm_stream_t* stream) {
	return Serial.available();
}

static int32_t COMM_CALL __read(comm_stream_t* stream, void* out, uint32_t len) {
	return Serial.readBytes((uint8_t*)out, len);
}

static uint32_t COMM_CALL __available_write(const comm_stream_t* stream) {
	return Serial.availableForWrite();
}

static int32_t COMM_CALL __write(comm_stream_t* stream, const void* in, uint32_t len) {
	return Serial.write((uint8_t*)in, len);
}

static bool COMM_CALL __flush(comm_stream_t* stream) {
	Serial.flush();
	return true;
}

static void __purge() {
	uint8_t b;
	while(Serial.readBytes(&b, 1));
}

static uint8_t __toArduinoCfg(const comm::Config& cfg) {
	switch (cfg) {
	case comm::Config::CONFIG_5N1: return SERIAL_5N1;
	case comm::Config::CONFIG_6N1: return SERIAL_6N1;
	case comm::Config::CONFIG_7N1: return SERIAL_7N1;
	case comm::Config::CONFIG_8N1: return SERIAL_8N1;
	case comm::Config::CONFIG_5N2: return SERIAL_5N2;
	case comm::Config::CONFIG_6N2: return SERIAL_6N2;
	case comm::Config::CONFIG_7N2: return SERIAL_7N2;
	case comm::Config::CONFIG_8N2: return SERIAL_8N2;
	case comm::Config::CONFIG_5E1: return SERIAL_5E1;
	case comm::Config::CONFIG_6E1: return SERIAL_6E1;
	case comm::Config::CONFIG_7E1: return SERIAL_7E1;
	case comm::Config::CONFIG_8E1: return SERIAL_8E1;
	case comm::Config::CONFIG_5E2: return SERIAL_5E2;
	case comm::Config::CONFIG_6E2: return SERIAL_6E2;
	case comm::Config::CONFIG_7E2: return SERIAL_7E2;
	case comm::Config::CONFIG_8E2: return SERIAL_8E2;
	case comm::Config::CONFIG_5O1: return SERIAL_5O1;
	case comm::Config::CONFIG_6O1: return SERIAL_6O1;
	case comm::Config::CONFIG_7O1: return SERIAL_7O1;
	case comm::Config::CONFIG_8O1: return SERIAL_8O1;
	case comm::Config::CONFIG_5O2: return SERIAL_5O2;
	case comm::Config::CONFIG_6O2: return SERIAL_6O2;
	case comm::Config::CONFIG_7O2: return SERIAL_7O2;
	case comm::Config::CONFIG_8O2: return SERIAL_8O2;
	default: return SERIAL_8E1;
	}
}

#if DEBUG_ENABLED
 	#warning "DEBUG INFO IS BEING ADDED TO FIRMWARE"
	static const char* __toString(const comm::Config& config) {
		switch (config) {
		case comm::Config::CONFIG_5N1: return "5N1";
		case comm::Config::CONFIG_6N1: return "6N1";
		case comm::Config::CONFIG_7N1: return "7N1";
		case comm::Config::CONFIG_8N1: return "8N1";
		case comm::Config::CONFIG_5N2: return "5N2";
		case comm::Config::CONFIG_6N2: return "6N2";
		case comm::Config::CONFIG_7N2: return "7N2";
		case comm::Config::CONFIG_8N2: return "8N2";
		case comm::Config::CONFIG_5E1: return "5E1";
		case comm::Config::CONFIG_6E1: return "6E1";
		case comm::Config::CONFIG_7E1: return "7E1";
		case comm::Config::CONFIG_8E1: return "8E1";
		case comm::Config::CONFIG_5E2: return "5E2";
		case comm::Config::CONFIG_6E2: return "6E2";
		case comm::Config::CONFIG_7E2: return "7E2";
		case comm::Config::CONFIG_8E2: return "8E2";
		case comm::Config::CONFIG_5O1: return "5O1";
		case comm::Config::CONFIG_6O1: return "6O1";
		case comm::Config::CONFIG_7O1: return "7O1";
		case comm::Config::CONFIG_8O1: return "8O1";
		case comm::Config::CONFIG_5O2: return "5O2";
		case comm::Config::CONFIG_6O2: return "6O2";
		case comm::Config::CONFIG_7O2: return "7O2";
		case comm::Config::CONFIG_8O2: return "8O2";
		default: return nullptr;
		}
	}

	static const char* __toString(const comm::Mode& mode) {
		switch (mode) {
		case comm::Mode::MESSAGE: return "MESSAGE";
		case comm::Mode::PACKET: return "PACKET";
		default: return nullptr;
		}
	}

	#define __TO_STR(w) __toString(w)
#else
	#define __TO_STR(w)
#endif

void comm::init(unsigned long speed, const Config& cfg, Mode mode) {
	static comm_stream_controller_t mController;
	static bool inited = false;
	static const dispatcher::MessageHandler mMessageHandlers[] = {
		{ "PROT" , message::PROT  },
		{ "PING" , message::PING  },
		{ "BLINK", message::BLINK },
		{ nullptr, nullptr }
	};
	static const dispatcher::PacketHandler mPacketHandlers[] = {
		{ 2, packet::PROT },
		{ 3, packet::PING },
		{ 0, nullptr }
	};

	Serial.begin(speed, __toArduinoCfg(cfg));
	Serial.setTimeout(__READ_TIMEOUT_MILLIS);

	__speed = speed;
	__cfg   = cfg;
	__mode  = mode;

	Serial.flush();

	if (!inited) {
		mController.available_read  = __available_read;
		mController.read            = __read;
		mController.available_write = __available_write;
		mController.write           = __write;
		mController.flush           = __flush;

		__serialStream = comm_stream_new(&mController, 0);
		__lineStream   = comm_line_stream_new(__serialStream, __MSG_MAX_LEN, false, NULL, NULL);
		__packetStream = comm_packet_stream_new(__serialStream, false, NULL, NULL);

		inited = true;

		dispatcher::init(mMessageHandlers, message::noHandler, mPacketHandlers, packet::noHandler);
		DEBUG("[comm::init] static initialization");
	}

	__purge();
	DEBUG("[comm::init] speed: %ld, cfg: %s, mode: %s", speed, __TO_STR(cfg), __TO_STR(mode));
}

unsigned long comm::getSpeed() {
	return __speed;
}

const comm::Config& comm::getCfg() {
	return __cfg;
}

const comm::Mode& comm::getMode() {
	return __mode;
}

char* comm::readMsg() {
	#if DEBUG_ENABLED
	 	#warning "DEBUG INFO IS BEING ADDED TO FIRMWARE"
		char* line = comm_line_stream_read(__lineStream);
		if (line) {
			DEBUG("[comm::readLine] Line read: %s", line);
		}
		return line;
	#else
		return comm_line_stream_read(__lineStream);
	#endif
}

void* comm::readPacket(uint8_t* lenOut) {
	#if DEBUG_ENABLED
		uint8_t* packet = comm_packet_stream_read(__packetStream, lenOut);
		if (packet) {
			DEBUG("[comm::readPacket] packet read (size: %d)", *lenOut);
		}
		return packet;
	#else
		return comm_packet_stream_read(__packetStream, lenOut);
	#endif
}

void comm::write(const char* line) {
	comm_line_stream_write(__lineStream, line);
}

void comm::write(const void* packet, uint8_t szPacket) {
	comm_packet_stream_write(__packetStream, packet, szPacket);
	DEBUG("[comm::writePacket] packet written");
}

void comm::check() {
	switch (getMode()) {
	case Mode::MESSAGE:
		dispatcher::dispatch(readMsg());
		break;

	case Mode::PACKET: {
		uint8_t szPacket;
		dispatcher::dispatch(readPacket(&szPacket), szPacket);
		break;
	}

	default:
		break;
	}
}
