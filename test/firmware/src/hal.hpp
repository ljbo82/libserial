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

namespace hal {

namespace serial {
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

	bool config(uint32_t baud, Config config);

	bool setReadTimeout(uint32_t timeout);

	uint32_t availableRead();

	uint32_t availableWrite();

	int32_t read(void* out, uint32_t len);

	bool write(const void* in, uint32_t len);

	bool flush();
} // namespace serial

namespace led {
	void set(bool on);

} // namespace led

uint32_t millis();

} // namespace hal
