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

#include "config.h"

#include <serial.h>
#include <errno.h>

const char* connection_config_to_str(connection_config_e config) {
	serial_data_bits_e dataBits = CONNECTION_CONFIG_DATA_BITS(config);
	serial_parity_e    parity   = CONNECTION_CONFIG_PARITY(config);
	serial_stop_bits_e stopBits = CONNECTION_CONFIG_STOP_BITS(config);

	static char buf[4];

	switch (dataBits) {
	case SERIAL_DATA_BITS_5:
	case SERIAL_DATA_BITS_6:
	case SERIAL_DATA_BITS_7:
	case SERIAL_DATA_BITS_8:
		buf[0] = '0' + dataBits;
		break;

	default:
		goto error;
	}
	switch(parity) {
	case SERIAL_PARITY_NONE:
		buf[1] = 'N';
		break;

	case SERIAL_PARITY_EVEN:
		buf[1] = 'E';
		break;

	case SERIAL_PARITY_ODD:
		buf[1] = 'O';
		break;

	default:
		goto error;
	}
	switch(stopBits) {
	case SERIAL_STOP_BITS_1:
	case SERIAL_STOP_BITS_2:
		buf[2] = '0' + stopBits;
		break;

	default:
		goto error;
	}

	buf[3] = '\0';
	return buf;

error:
	errno = SERIAL_ERROR_INVALID_PARAM;
	return NULL;
}
