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

#include "debug.hpp"

#include "Comm.hpp"
#include <stdlib.h>
#include <inttypes.h>

#define __READ_TIMEOUT_MILLIS 1000
#define __MSG_MAX_LEN         128
#define __PARAM_DELIMITER     ';'
#define __CMD_DELIMITER       '\r'
#define __ACK                 "ACK"
#define __NAK                 "NAK"

typedef void (*__CommandHandlerCb)(const char* params);

struct __CommandInfo {
	const char* id;
	__CommandHandlerCb cb;
};

struct __ConfigInfo {
	const char* id;
	uint8_t     cfg;
};

static bool __startsWith(const char* str, const char* prefix);
static void __PROT_handler(const char* params);
static void __PING_handler(const char* params);

static const __CommandInfo __supportedCommands[] = {
	{ "PROT;", __PROT_handler },
	{ "PING;", __PING_handler },
	{ nullptr, nullptr } // terminator
};

static const __ConfigInfo  __configMap[] = {
	{ "5N1", SERIAL_5N1 },
	{ "6N1", SERIAL_6N1 },
	{ "7N1", SERIAL_7N1 },
	{ "8N1", SERIAL_8N1 },
	{ "5N2", SERIAL_5N2 },
	{ "6N2", SERIAL_6N2 },
	{ "7N2", SERIAL_7N2 },
	{ "8N2", SERIAL_8N2 },
	{ "5E1", SERIAL_5E1 },
	{ "6E1", SERIAL_6E1 },
	{ "7E1", SERIAL_7E1 },
	{ "8E1", SERIAL_8E1 },
	{ "5E2", SERIAL_5E2 },
	{ "6E2", SERIAL_6E2 },
	{ "7E2", SERIAL_7E2 },
	{ "8E2", SERIAL_8E2 },
	{ "5O1", SERIAL_5O1 },
	{ "6O1", SERIAL_6O1 },
	{ "7O1", SERIAL_7O1 },
	{ "8O1", SERIAL_8O1 },
	{ "5O2", SERIAL_5O2 },
	{ "6O2", SERIAL_6O2 },
	{ "7O2", SERIAL_7O2 },
	{ "8O2", SERIAL_8O2 },
	{ nullptr, 0 }
};

static bool __startsWith(const char* str, const char* prefix) {
	while (*prefix != '\0') {
		if (*str != *prefix) {
			return false;
		}
		str++;
		prefix++;
	}

	return true;
}

static void __PROT_handler(const char* params) {
	DEBUG("[__PROT_handler] params: \"%s\"", params);
	char          buf[128];
	char*         end;
	unsigned long baud;
	int16_t cfg;

	// Baud
	if (!Comm::nextParam(&params, buf))
		goto invalid;

	DEBUG("[__PROT_handler] buf: \"%s\"", buf);
	baud = strtoul(buf, &end, 10);
	if (*end != '\0') goto invalid;
	DEBUG("[__PROT_handler] baud: %lu", baud);

	// Config
	if (!Comm::nextParam(&params, buf))
		goto invalid;

	DEBUG("[__PROT_handler] buf: \"%s\"", buf);
	cfg = -1;
	for (const __ConfigInfo* cfgInfo = __configMap; cfgInfo->id; cfgInfo++) {
		if (strcmp(buf, cfgInfo->id) == 0) {
			DEBUG("[__PROT_handler] cfg: \"%s\"", buf);
			cfg = cfgInfo->cfg;
			break;
		}
	}
	if (cfg < 0) {
		DEBUG("[__PROT_handler] invalid cfg: \"%s\"", buf);
		goto invalid;
	}

	// Unexpected param
	if (Comm::nextParam(&params, buf)) {
		goto invalid;
	}

	Comm::sendMsg(__ACK);
	Serial.begin(baud, (uint8_t)cfg);
	return;

invalid:
	Comm::sendMsg(__NAK);
	Comm::purge();
}

static void __PING_handler(const char* params) {
	DEBUG("[__PING_handler] params: \"%s\"", params);
	Comm::sendMsg(__ACK);
	char rsp[__MSG_MAX_LEN + 2];
	sprintf(rsp, "PING;%s",params);
	Comm::sendMsg(rsp);
}

void Comm::init(long speed, uint8_t cfg) {
	Serial.begin(speed, cfg);
	Serial.setTimeout(__READ_TIMEOUT_MILLIS);
	Serial.flush();
	Comm::purge();
	DEBUG("[Comm::init] speed: %ld, cfg: %d", speed, cfg);
}

void Comm::purge() {
	uint8_t b;
	while(Serial.readBytes(&b, 1));
	DEBUG("[Comm::purge] Done");
}

const char* Comm::readMsg() {
	static char   mBuf[__MSG_MAX_LEN + 2]; // __MSG_MAX_LEN + __CMD_DELIMITER + '\0'
	static size_t mTotalRead = 0;
	static char*  mCursor    = mBuf;

	size_t mRead;
	size_t readLen;

	while (mTotalRead < (__MSG_MAX_LEN + 1)) {
		if (Serial.available() == 0)
			return nullptr;

		// Since device will not receive two messages, it is safe to assume that
		// no more data will be read after the end of message
		readLen = min((size_t)Serial.available(), ((__MSG_MAX_LEN + 1) - mTotalRead));
		DEBUG("[Comm::readMsg] readLen: %d", readLen);

		mRead = Serial.readBytes(mCursor, readLen);
		DEBUG("Comm::readMsg] mRead: %d", mRead);

		// Check for __CMD_DELIMITER inside read chunk
		for (size_t i = 0; i < mRead; i++) {
			if (mCursor[i] == __CMD_DELIMITER) {
				mCursor[i] = '\0';

				mTotalRead = 0;
				mCursor = mBuf;

				DEBUG("[Comm::readMsg] mBuf: \"%s\"", mBuf);
				return mBuf;
			}
		}

		mTotalRead += mRead;
		mCursor    += mRead;
	}

	//  __CMD_DELIMITER was not found (message is too long)
	DEBUG("[Comm::readMsg] Long message");
	Comm::purge();

	mTotalRead = 0;
	mCursor = mBuf;

	return nullptr;
}

void Comm::sendMsg(const char* msg) {
	DEBUG("[Comm::sendMsg] msg: \"%s\"", msg);
	while(*msg != '\0' && *msg != __CMD_DELIMITER) {
		Serial.write((uint8_t) *msg);
		msg++;
	}
	Serial.write((uint8_t)__CMD_DELIMITER);
	Serial.flush();
}

void Comm::dispatch(const char* cmd) {
	if (!cmd || *cmd == '\0')
		return;

	for (const __CommandInfo* cmdInfo = __supportedCommands; cmdInfo->id; cmdInfo++) {
		if (__startsWith(cmd, cmdInfo->id)) {
			DEBUG("[Comm::dispatch] Found handler!");
			cmdInfo->cb(cmd += strlen(cmdInfo->id));
			return;
		}
	}

	// No command handler found
	DEBUG("[Comm::dispatch] No handler");
	Comm::sendMsg(__NAK);
}

const char* Comm::nextParam(const char** params, char* out) {
	if (params == nullptr || (*params)[0] == '\0') {
		DEBUG("[Comm::nextParam] No params");
		return nullptr;
	}

	size_t i = 0;
	while((*params)[i] != '\0' && (*params)[i] != __PARAM_DELIMITER) {
		out[i] = (*params)[i];
		i++;
	}

	out[i] = '\0';
	*params = (*params) + i + ((*params)[i] == __PARAM_DELIMITER ? 1 : 0);

	DEBUG("[Comm::nextParam] out: \"%s\", params: \"%s\"", out, (*params));
	return out;
}
