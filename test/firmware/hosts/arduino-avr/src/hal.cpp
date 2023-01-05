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
#include "comm.hpp"

#include <Arduino.h>

static uint8_t __toArduinoCfg(const hal::serial::Config& cfg) {
	switch (cfg) {
	case hal::serial::Config::CONFIG_5N1: return SERIAL_5N1;
	case hal::serial::Config::CONFIG_6N1: return SERIAL_6N1;
	case hal::serial::Config::CONFIG_7N1: return SERIAL_7N1;
	case hal::serial::Config::CONFIG_8N1: return SERIAL_8N1;
	case hal::serial::Config::CONFIG_5N2: return SERIAL_5N2;
	case hal::serial::Config::CONFIG_6N2: return SERIAL_6N2;
	case hal::serial::Config::CONFIG_7N2: return SERIAL_7N2;
	case hal::serial::Config::CONFIG_8N2: return SERIAL_8N2;
	case hal::serial::Config::CONFIG_5E1: return SERIAL_5E1;
	case hal::serial::Config::CONFIG_6E1: return SERIAL_6E1;
	case hal::serial::Config::CONFIG_7E1: return SERIAL_7E1;
	case hal::serial::Config::CONFIG_8E1: return SERIAL_8E1;
	case hal::serial::Config::CONFIG_5E2: return SERIAL_5E2;
	case hal::serial::Config::CONFIG_6E2: return SERIAL_6E2;
	case hal::serial::Config::CONFIG_7E2: return SERIAL_7E2;
	case hal::serial::Config::CONFIG_8E2: return SERIAL_8E2;
	case hal::serial::Config::CONFIG_5O1: return SERIAL_5O1;
	case hal::serial::Config::CONFIG_6O1: return SERIAL_6O1;
	case hal::serial::Config::CONFIG_7O1: return SERIAL_7O1;
	case hal::serial::Config::CONFIG_8O1: return SERIAL_8O1;
	case hal::serial::Config::CONFIG_5O2: return SERIAL_5O2;
	case hal::serial::Config::CONFIG_6O2: return SERIAL_6O2;
	case hal::serial::Config::CONFIG_7O2: return SERIAL_7O2;
	case hal::serial::Config::CONFIG_8O2: return SERIAL_8O2;
	default: return SERIAL_8E1;
	}
}

#if DEBUG_ENABLED
#warning "DEBUG INFO IS BEING ADDED TO FIRMWARE"
void hal::debug(const char* msg) {
	comm::write(msg);
}
#endif // #if DEBUG_ENABLED

uint32_t hal::millis() {
	return ::millis();
}

bool hal::serial::config(uint32_t baud, Config config) {
	Serial.begin(baud, __toArduinoCfg(config));
	return true;
}

bool hal::serial::setReadTimeout(uint32_t timeout) {
	Serial.setTimeout(timeout);
	return true;
}

uint32_t hal::serial::availableRead() {
	return Serial.available();
}

uint32_t hal::serial::availableWrite() {
	return Serial.availableForWrite();
}

int32_t hal::serial::read(void* out, uint32_t len) {
	return Serial.readBytes((uint8_t*)out, len);
}

bool hal::serial::write(const void* in, uint32_t len) {
	Serial.write((uint8_t*)in, len);
	return true;
}

bool hal::serial::flush() {
	Serial.flush();
	return true;
}

void hal::led::set(bool on) {
	static bool inited = false;
	if (!inited) {
		pinMode(LED_BUILTIN, OUTPUT);
		inited = true;
	}

	digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
}
