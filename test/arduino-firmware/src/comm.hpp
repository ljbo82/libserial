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
#pragma once

#include <stdint.h>

#define COMM_DEFAULT_SPEED 9600
#define COMM_DEFAULT_CFG   comm::Config::CONFIG_8N1
#define COMM_DEFAULT_MODE  comm::Mode::MESSAGE

namespace comm {

enum class Config {
	CONFIG_5N1,
	CONFIG_6N1,
	CONFIG_7N1,
	CONFIG_8N1,
	CONFIG_5N2,
	CONFIG_6N2,
	CONFIG_7N2,
	CONFIG_8N2,
	CONFIG_5E1,
	CONFIG_6E1,
	CONFIG_7E1,
	CONFIG_8E1,
	CONFIG_5E2,
	CONFIG_6E2,
	CONFIG_7E2,
	CONFIG_8E2,
	CONFIG_5O1,
	CONFIG_6O1,
	CONFIG_7O1,
	CONFIG_8O1,
	CONFIG_5O2,
	CONFIG_6O2,
	CONFIG_7O2,
	CONFIG_8O2
};

enum class Mode {
	MESSAGE,
	PACKET
};

void init(unsigned long speed = COMM_DEFAULT_SPEED, const Config& cfg = COMM_DEFAULT_CFG, Mode mode = COMM_DEFAULT_MODE);

unsigned long getSpeed();

const comm::Config& getCfg();

const comm::Mode& getMode();

char* readMsg();

void* readPacket(uint8_t* lenOut);

void write(const char* msg);

void write(const void* packet, uint8_t szPacket);

void check();

} // namespace comm
