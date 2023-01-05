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
#include "message.hpp"
#include "debug.hpp"
#include "comm.hpp"
#include "led.hpp"
#include "hal.hpp"

#include <stdlib.h>
#include <string.h>

#define __PARAM_DELIMITER ';'
#define __ACK             "ACK"
#define __NAK             "NAK"

static bool __toConfig(const char* strCfg, hal::serial::Config* out) {
	struct ConfigInfo {
		const char*  id;
		hal::serial::Config cfg;
	};

	static const ConfigInfo  mConfigMap[] = {
		{ "5N1", hal::serial::Config::CONFIG_5N1 },
		{ "6N1", hal::serial::Config::CONFIG_6N1 },
		{ "7N1", hal::serial::Config::CONFIG_7N1 },
		{ "8N1", hal::serial::Config::CONFIG_8N1 },
		{ "5N2", hal::serial::Config::CONFIG_5N2 },
		{ "6N2", hal::serial::Config::CONFIG_6N2 },
		{ "7N2", hal::serial::Config::CONFIG_7N2 },
		{ "8N2", hal::serial::Config::CONFIG_8N2 },
		{ "5E1", hal::serial::Config::CONFIG_5E1 },
		{ "6E1", hal::serial::Config::CONFIG_6E1 },
		{ "7E1", hal::serial::Config::CONFIG_7E1 },
		{ "8E1", hal::serial::Config::CONFIG_8E1 },
		{ "5E2", hal::serial::Config::CONFIG_5E2 },
		{ "6E2", hal::serial::Config::CONFIG_6E2 },
		{ "7E2", hal::serial::Config::CONFIG_7E2 },
		{ "8E2", hal::serial::Config::CONFIG_8E2 },
		{ "5O1", hal::serial::Config::CONFIG_5O1 },
		{ "6O1", hal::serial::Config::CONFIG_6O1 },
		{ "7O1", hal::serial::Config::CONFIG_7O1 },
		{ "8O1", hal::serial::Config::CONFIG_8O1 },
		{ "5O2", hal::serial::Config::CONFIG_5O2 },
		{ "6O2", hal::serial::Config::CONFIG_6O2 },
		{ "7O2", hal::serial::Config::CONFIG_7O2 },
		{ "8O2", hal::serial::Config::CONFIG_8O2 },
		{ nullptr, hal::serial::Config::CONFIG_8E1 }
	};

	for (const ConfigInfo* cfgInfo = mConfigMap; cfgInfo->id; cfgInfo++) {
		if (strcmp(strCfg, cfgInfo->id) == 0) {
			if (out) *out = cfgInfo->cfg;
			return true;
		}
	}

	return false;
}

static bool __toMode(const char* strMode, comm::Mode* out) {
	switch (*strMode) {
	case '0':
		if (out) *out = comm::Mode::MESSAGE;
		break;

	case '1':
		if (out) *out = comm::Mode::PACKET;
		break;

	default:
		return false;
	}

	return true;
}

const char* message::nextToken(char** strPtr) {
	if (strPtr == nullptr || *strPtr == nullptr) {
		return nullptr;
	}

	const char* start = *strPtr;

	while (true) {
		if (**strPtr == __PARAM_DELIMITER) {
			**strPtr = '\0';
			(*strPtr)++;
			return start;
		}

		if (**strPtr == '\0')
			return start;

		(*strPtr)++;
	}
}

void message::PING(const char* id, char* data) {
	comm::write(__ACK);
	comm::write(data);
}

void message::PROT(const char* id, char* data) {
	const char* strSpeed;
	const char* strCfg;
	const char* strMode;

	strSpeed = nextToken(&data);
	strCfg   = nextToken(&data);
	strMode  = nextToken(&data);

	DEBUG("speed: %s, cfg: %s, mode: %s", strSpeed, strCfg, strMode);

	if (!strSpeed) goto nak;
	if (!strCfg)   goto nak;
	if (!strMode)  goto nak;

	char* end;

	unsigned long speed;
	hal::serial::Config  cfg;
	comm::Mode    mode;

	speed = strtoul(strSpeed, &end, 10);
	if (*end != '\0') goto nak;
	DEBUG("speed: ok");
	if (!__toConfig(strCfg, &cfg)) goto nak;
	DEBUG("cfg: ok");
	if (!__toMode(strMode, &mode)) goto nak;
	DEBUG("mode: ok");

	comm::write(__ACK);
	comm::init(speed, cfg, mode);
	return;

nak:
	comm::write(__NAK);
}

void message::BLINK(const char* id, char* data) {
	char* end;
	unsigned long blinkInterval = strtoul(data, &end, 10);
	if (*end != '\0') goto nak;

	comm::write(__ACK);
	led::init(blinkInterval);
	return;

nak:
	comm::write(__NAK);
}

void message::noHandler(const char* id, char* data) {
	comm::write(__NAK);
}
