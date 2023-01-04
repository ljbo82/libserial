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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#if defined _WIN32 || defined __CYGWIN__
	#if defined(SERIAL_BUILD_SHARED_LIB)
		/** @internal */
		#define SERIAL_PUBLIC __declspec(dllexport)
	#else
		/** @internal */
		#define SERIAL_PUBLIC
	#endif

	/** @internal */
	#define SERIAL_CALL __cdecl
#else
	#if defined(SERIAL_BUILD_SHARED_LIB) && __GNUC__ >= 4
		/** @internal */
		#define SERIAL_PUBLIC __attribute__ ((visibility ("default")))
	#else
		/** @internal */
		#define SERIAL_PUBLIC
	#endif

	/** @internal */
	#define SERIAL_CALL
#endif

typedef struct __serial_list serial_list_t;

typedef struct __serial serial_t;

typedef enum serial_data_bits serial_data_bits_e;

typedef enum serial_parity serial_parity_e;

typedef enum serial_stop_bits serial_stop_bits_e;

typedef enum serial_purge_type serial_purge_type_e;

typedef enum serial_error serial_error_e;

typedef struct serial_config serial_config_t;

enum serial_data_bits {
	SERIAL_DATA_BITS_5 = 5,
	SERIAL_DATA_BITS_6,
	SERIAL_DATA_BITS_7,
	SERIAL_DATA_BITS_8
};

enum serial_parity {
	SERIAL_PARITY_NONE = 0,
	SERIAL_PARITY_EVEN,
	SERIAL_PARITY_ODD,
};

enum serial_stop_bits {
	SERIAL_STOP_BITS_1 = 1,
	SERIAL_STOP_BITS_2 = 2,
	SERIAL_STOP_BITS_1_5
};

enum serial_purge_type {
	SERIAL_PURGE_TYPE_RX,
	SERIAL_PURGE_TYPE_TX,
	SERIAL_PURGE_TYPE_RX_TX
};

enum serial_error {
	SERIAL_ERROR_OK            =  0,
	SERIAL_ERROR_UNKNOWN       = -1,
	SERIAL_ERROR_MEM           = -2,
	SERIAL_ERROR_IO            = -3,
	SERIAL_ERROR_ACCESS        = -4,
	SERIAL_ERROR_NOT_FOUND     = -5,
	SERIAL_ERROR_INVALID_PARAM = -6,
	SERIAL_ERROR_TIMEOUT       = -7
};

struct serial_config {
	uint32_t           baud;
	serial_data_bits_e dataBits;
	serial_parity_e    parity;
	serial_stop_bits_e stopBits;
};

#ifdef __cplusplus
extern "C" {
#endif

SERIAL_PUBLIC const char* SERIAL_CALL serial_error_to_str(serial_error_e error);

SERIAL_PUBLIC serial_list_t* SERIAL_CALL serial_list_new();

SERIAL_PUBLIC void SERIAL_CALL serial_list_del(serial_list_t* list);

SERIAL_PUBLIC size_t SERIAL_CALL serial_list_size(const serial_list_t* list);

SERIAL_PUBLIC const char* SERIAL_CALL serial_list_item(const serial_list_t* list, size_t index);

SERIAL_PUBLIC serial_list_t* SERIAL_CALL serial_list_ports(serial_list_t* list);

SERIAL_PUBLIC serial_t* SERIAL_CALL serial_open(const char* portName);

SERIAL_PUBLIC const char* SERIAL_CALL serial_get_name(const serial_t* port);

SERIAL_PUBLIC bool SERIAL_CALL serial_config(serial_t* port, const serial_config_t* config);

SERIAL_PUBLIC void serial_get_config(const serial_t* port, serial_config_t* out);

SERIAL_PUBLIC bool SERIAL_CALL serial_set_read_timeout(serial_t* port, uint32_t millis);

SERIAL_PUBLIC uint32_t SERIAL_CALL serial_get_read_timeout(const serial_t* port);

SERIAL_PUBLIC bool SERIAL_CALL serial_purge(serial_t* port, serial_purge_type_e type);

SERIAL_PUBLIC bool SERIAL_CALL serial_close(serial_t* port);

SERIAL_PUBLIC int32_t SERIAL_CALL serial_available(const serial_t* port);

SERIAL_PUBLIC int32_t SERIAL_CALL serial_read(serial_t* port, void* out, uint32_t len);

SERIAL_PUBLIC bool SERIAL_CALL serial_write(serial_t* port, const void* in, uint32_t len);

SERIAL_PUBLIC bool SERIAL_CALL serial_flush(serial_t* port);

SERIAL_PUBLIC const char* SERIAL_CALL serial_version();

#ifdef __cplusplus
} // extern "C"
#endif
