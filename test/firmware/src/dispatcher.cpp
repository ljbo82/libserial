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
#include "dispatcher.hpp"
#include "debug.hpp"
#include "message.hpp"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static const dispatcher::MessageHandler* __msgHandlers;
static const dispatcher::PacketHandler*  __packetHandlers;
static dispatcher::MessageHandlerCb __msgNoHandler;
static dispatcher::PacketHandlerCb  __packetNoHandler;

void dispatcher::init(
	const MessageHandler msgHandlers[], MessageHandlerCb msgNoHandler,
	const PacketHandler packetHandlers[], PacketHandlerCb packetNoHandler
) {
	__msgHandlers     = msgHandlers;
	__msgNoHandler    = msgNoHandler;
	__packetHandlers  = packetHandlers;
	__packetNoHandler = packetNoHandler;
}

void dispatcher::dispatch(char* msg) {
	if (!msg)
		return;

	// NOTE: msg will be updated to point to data
	const char* id = message::nextToken(&msg);

	for (const MessageHandler* handler = __msgHandlers; handler->cb; handler++) {
		if (strcmp(id, handler->id) == 0) {
			DEBUG("[dispatcher::dispatch] Found handler!");
			handler->cb(id, msg);
			return;
		}
	}

	// No command handler found
	DEBUG("[comm::dispatch] No handler for id: %s", id);
	if (__msgNoHandler) {
		__msgNoHandler(id, msg);
	}
}

void dispatcher::dispatch(void* xPacket, uint8_t szPacket) {
	if (!xPacket || szPacket == 0)
		return;

	uint8_t* packet = (uint8_t*)xPacket;
	uint8_t id = packet[0];

	packet++;
	szPacket--;

	for (const PacketHandler* handler = __packetHandlers; handler->cb; handler++) {
		if (id == handler->id) {
			DEBUG("[dispatcher::dispatch] Found handler!");
			handler->cb(id, packet, szPacket);
			return;
		}
	}

	// No command handler found
	DEBUG("[comm::dispatch] No handler");
	if (__packetNoHandler) {
		__packetNoHandler(id, packet, szPacket);
	}
}
