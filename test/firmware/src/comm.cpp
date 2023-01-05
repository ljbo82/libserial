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
#include "led.hpp"
#include "hal.hpp"

#include <comm.h>
#include <stdlib.h>
#include <inttypes.h>

#define __MSG_PING  "PING"
#define __MSG_PROT  "PROT"
#define __MSG_BLINK "BLINK"

#define __PACKET_PING 2
#define __PACKET_PROT 3

#define __READ_TIMEOUT_MILLIS 1000
#define __MSG_MAX_LEN         128
#define __CASE(mNamespace, mCase) case mNamespace::mCase: return #mCase

#define __MODE_MSG_BLINK 100
#define __MODE_PKT_BLINK 500

static comm_stream_t *       __serialStream = NULL;
static comm_line_stream_t*   __lineStream   = NULL;
static comm_packet_stream_t* __packetStream = NULL;
static long                  __speed        = COMM_DEFAULT_SPEED;
static hal::serial::Config   __cfg          = COMM_DEFAULT_CFG;
static comm::Mode            __mode         = COMM_DEFAULT_MODE;

static uint32_t COMM_CALL __available_read(const comm_stream_t* stream) {
	return hal::serial::availableRead();
}

static int32_t COMM_CALL __read(comm_stream_t* stream, void* out, uint32_t len) {
	return hal::serial::read(out, len);
}

static uint32_t COMM_CALL __available_write(const comm_stream_t* stream) {
	return hal::serial::availableWrite();
}

static int32_t COMM_CALL __write(comm_stream_t* stream, const void* in, uint32_t len) {
	return hal::serial::write(in, len) ? len : -1;
}

static bool COMM_CALL __flush(comm_stream_t* stream) {
	return hal::serial::flush();
}

static void __purge() {
	uint8_t b;
	while(hal::serial::read(&b, 1) > 0);
}

#if DEBUG_ENABLED
	static const char* __toString(const hal::serial::Config& config) {
		switch (config) {
		case hal::serial::Config::CONFIG_5N1: return "5N1";
		case hal::serial::Config::CONFIG_6N1: return "6N1";
		case hal::serial::Config::CONFIG_7N1: return "7N1";
		case hal::serial::Config::CONFIG_8N1: return "8N1";
		case hal::serial::Config::CONFIG_5N2: return "5N2";
		case hal::serial::Config::CONFIG_6N2: return "6N2";
		case hal::serial::Config::CONFIG_7N2: return "7N2";
		case hal::serial::Config::CONFIG_8N2: return "8N2";
		case hal::serial::Config::CONFIG_5E1: return "5E1";
		case hal::serial::Config::CONFIG_6E1: return "6E1";
		case hal::serial::Config::CONFIG_7E1: return "7E1";
		case hal::serial::Config::CONFIG_8E1: return "8E1";
		case hal::serial::Config::CONFIG_5E2: return "5E2";
		case hal::serial::Config::CONFIG_6E2: return "6E2";
		case hal::serial::Config::CONFIG_7E2: return "7E2";
		case hal::serial::Config::CONFIG_8E2: return "8E2";
		case hal::serial::Config::CONFIG_5O1: return "5O1";
		case hal::serial::Config::CONFIG_6O1: return "6O1";
		case hal::serial::Config::CONFIG_7O1: return "7O1";
		case hal::serial::Config::CONFIG_8O1: return "8O1";
		case hal::serial::Config::CONFIG_5O2: return "5O2";
		case hal::serial::Config::CONFIG_6O2: return "6O2";
		case hal::serial::Config::CONFIG_7O2: return "7O2";
		case hal::serial::Config::CONFIG_8O2: return "8O2";
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

void comm::init(unsigned long speed, const hal::serial::Config& cfg, Mode mode) {
	static comm_stream_controller_t mController;
	static bool inited = false;
	static const dispatcher::MessageHandler mMessageHandlers[] = {
		{ __MSG_PROT , message::PROT  },
		{ __MSG_PING , message::PING  },
		{ __MSG_BLINK, message::BLINK },
		{ nullptr, nullptr }
	};
	static const dispatcher::PacketHandler mPacketHandlers[] = {
		{ __PACKET_PROT, packet::PROT },
		{ __PACKET_PING, packet::PING },
		{ 0, nullptr }
	};

	hal::serial::config(speed, cfg);
	hal::serial::setReadTimeout(__READ_TIMEOUT_MILLIS);

	__speed = speed;
	__cfg   = cfg;
	__mode  = mode;

	if (!inited) {
		hal::serial::flush();

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
		__purge();
		DEBUG("[comm::init] static initialization");
	}

	switch (mode) {
	case Mode::MESSAGE:
		led::init(__MODE_MSG_BLINK);
		break;

	case Mode::PACKET:
		led::init(__MODE_PKT_BLINK);
		break;

	default:
		break;
	}
	DEBUG("[comm::init] speed: %ld, cfg: %s, mode: %s", speed, __TO_STR(cfg), __TO_STR(mode));
}

unsigned long comm::getSpeed() {
	return __speed;
}

const hal::serial::Config& comm::getCfg() {
	return __cfg;
}

const comm::Mode& comm::getMode() {
	return __mode;
}

char* comm::readMsg() {
	#if DEBUG_ENABLED
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
}

void comm::check() {
	switch (getMode()) {
	case Mode::MESSAGE:
		dispatcher::dispatch(readMsg());
		break;

	case Mode::PACKET: {
		uint8_t szPacket;
		void* packet = readPacket(&szPacket);
		dispatcher::dispatch(packet, szPacket);
		break;
	}

	default:
		break;
	}
}
