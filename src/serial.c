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

#include "_serial_native.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define __MIN_LIST_CAPACITY    5
#define __PORT_NAME_MAX_LEN    36
#define __DEFAULT_BAUD         9600
#define __DEFAULT_DATA_BITS    SERIAL_DATA_BITS_8
#define __DEFAULT_STOP_BITS    SERIAL_STOP_BITS_1
#define __DEFAULT_PARITY       SERIAL_PARITY_NONE
#define __DEFAULT_READ_TIMEOUT 0

// Expected to be passed during compilation
#ifndef LIB_VERSION
	#warning "LIB_VERSION is not defined"
	#define LIB_VERSION "unknown"
#endif

#define __SET_ERROR(err) errno = errno ? errno : err

struct __serial_list {
	size_t size;
	char** elements;
	size_t capacity;
};

struct __serial {
	void*              nativePort;
	char*              portName;
	serial_config_t    config;
	uint32_t           readTimeout;
};

static void __serial_list_clear(serial_list_t* list) {
	list->size = 0;
}

static void __serial_list_sort(serial_list_t* list) {
	if (list->size <= 1)
		return;

	bool hasChanges;
	char* current;
	char* next;
	char* buffer;
	size_t i = 0;

	do {
		i = 0;
		hasChanges = false;

		current = list->elements[i];
		next = i < (list->size - 1) ? list->elements[i + 1] : NULL;

		while (next != NULL) {
			if (strcmp(current, next) > 0) {
				hasChanges = true;

				buffer = list->elements[i];
				list->elements[i] = list->elements[i + 1];
				list->elements[i + 1] = buffer;

				break;
			}

			i++;
			current = next;
			next = i < (list->size - 1) ? list->elements[i + 1] : NULL;
		}
	} while (hasChanges);
}

const char* _serial_list_add(serial_list_t* list, const char* element) {
	if (strlen(element) > __PORT_NAME_MAX_LEN) {
		errno = SERIAL_ERROR_MEM;
		return NULL;
	}

	if (list->size + 1 > list->capacity) {
		// Capacity increase required
		size_t newCapacity = list->capacity == 0 ? __MIN_LIST_CAPACITY : list->capacity * 2;
		char** newElements = realloc(list->elements, sizeof(char*) * newCapacity);

		if (!newElements) {
			errno = SERIAL_ERROR_MEM;
			return NULL;
		}

		for (size_t i = list->size; i < newCapacity; i++) {
			newElements[i] = malloc(__PORT_NAME_MAX_LEN + 1);
			if (!newElements[i]) {
				errno = SERIAL_ERROR_MEM;
				return NULL;
			}
		}

		list->capacity = newCapacity;
		list->elements = newElements;
	}

	strcpy(list->elements[list->size], element);
	list->size++;

	return list->elements[list->size - 1];
}

PUBLIC const char* CALL serial_error_to_str(serial_error_e error) {
	#define __err_to_str(e) #e
	#define __err_case(e) case e: return __err_to_str(e)
	switch (error) {
	__err_case(SERIAL_ERROR_OK);
	__err_case(SERIAL_ERROR_MEM);
	__err_case(SERIAL_ERROR_IO);
	__err_case(SERIAL_ERROR_ACCESS);
	__err_case(SERIAL_ERROR_NOT_FOUND);
	__err_case(SERIAL_ERROR_INVALID_PARAM);
	__err_case(SERIAL_ERROR_TIMEOUT);

	default:
		return __err_to_str(SERIAL_ERROR_UNKNOWN);
	}

	#undef __err_to_str
	#undef __err_case
}

PUBLIC serial_list_t* CALL serial_list_new() {
	serial_list_t* list = malloc(sizeof(serial_list_t));

	if (!list) {
		errno = SERIAL_ERROR_MEM;
		goto error;
	}

	list->size = 0;
	list->elements = NULL;
	list->capacity = 0;
	return list;

error:
	return NULL;
}

PUBLIC void CALL serial_list_del(serial_list_t* list) {
	for (size_t i = 0; i < list->capacity; i++) {
		free(list->elements[i]);
	}
	free(list);
}

PUBLIC size_t CALL serial_list_size(const serial_list_t* list) {
	return list->size;
}

PUBLIC const char* CALL serial_list_item(const serial_list_t* list, size_t index) {
	if (index >= list->size) {
		errno = SERIAL_ERROR_INVALID_PARAM;
		goto error;
	}

	return list->elements[index];

error:
	return NULL;
}

PUBLIC serial_list_t* CALL serial_list_ports(serial_list_t* list) {
	__serial_list_clear(list);

	if (!_serial_native_list_ports(list))
		goto error;

	__serial_list_sort(list);
	return list;

error:
	__SET_ERROR(SERIAL_ERROR_IO);
	return NULL;
}

PUBLIC serial_t* CALL serial_open(const char* portName) {
	serial_t* port = malloc(sizeof(serial_t));

	if (!port) {
		errno = SERIAL_ERROR_MEM;
		goto error;
	}

	port->nativePort = _serial_native_open(portName);

	if (port->nativePort < 0)
		goto error;

	port->config.baud     = __DEFAULT_BAUD;
	port->config.dataBits = __DEFAULT_DATA_BITS;
	port->config.parity   = __DEFAULT_PARITY;
	port->config.stopBits = __DEFAULT_STOP_BITS;

	port->readTimeout = __DEFAULT_READ_TIMEOUT;

	if (!_serial_native_config(port->nativePort, &port->config))
		goto error;

	if (!_serial_native_set_read_timeout(port->nativePort, __DEFAULT_READ_TIMEOUT))
		goto error;

	port->portName = malloc(strlen(portName) + 1);

	if (!port->portName) {
		errno = SERIAL_ERROR_MEM;
		goto error;
	}

	strcpy(port->portName, portName);

	return port;

error:
	__SET_ERROR(SERIAL_ERROR_IO);
	int currentError = errno;

	if (port) {
		if (port->nativePort != NULL) {
			_serial_native_close(port->nativePort);
		}

		free(port);
	}

	errno = currentError; // Discards any error that could happen during _serial_native_close
	return NULL;
}

PUBLIC const char* CALL serial_get_name(const serial_t* port) {
	return port->portName;
}

PUBLIC bool CALL serial_config(serial_t* port, const serial_config_t* config) {
	if (memcmp(&port->config, config, sizeof(serial_config_t)) == 0)
		return true;

	switch (config->dataBits) {
	case SERIAL_DATA_BITS_5:
	case SERIAL_DATA_BITS_6:
	case SERIAL_DATA_BITS_7:
	case SERIAL_DATA_BITS_8:
		break;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}
	switch (config->parity) {
	case SERIAL_PARITY_NONE:
	case SERIAL_PARITY_EVEN:
	case SERIAL_PARITY_ODD:
		break;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}
	switch (config->stopBits) {
	case SERIAL_STOP_BITS_1:
	case SERIAL_STOP_BITS_1_5:
	case SERIAL_STOP_BITS_2:
		break;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	if (!_serial_native_config(port->nativePort, config)) {
		__SET_ERROR(SERIAL_ERROR_IO);
		return false;
	}

	port->config = *config;
	return true;
}

PUBLIC void serial_get_config(const serial_t* port, serial_config_t* out) {
	*out = port->config;
}

PUBLIC bool CALL serial_set_read_timeout(serial_t* port, uint32_t millis) {
	if (port->readTimeout == millis)
		return true;

	if (!_serial_native_set_read_timeout(port->nativePort, millis)) {
		__SET_ERROR(SERIAL_ERROR_IO);
		return false;
	}

	port->readTimeout = millis;
	return true;
}

PUBLIC uint32_t CALL serial_get_read_timeout(const serial_t* port) {
	return port->readTimeout;
}

PUBLIC bool CALL serial_purge(serial_t* port, serial_purge_type_e type) {
	if (!_serial_native_purge(port->nativePort, type)) {
		__SET_ERROR(SERIAL_ERROR_IO);
		return false;
	}

	return true;
}

PUBLIC bool CALL serial_close(serial_t* port) {
	if (
		serial_set_read_timeout(port, 0)
		&& serial_flush(port)
		&& _serial_native_close(port->nativePort)
	) {
		free(port->portName);
		free(port);
		return true;
	}

	__SET_ERROR(SERIAL_ERROR_IO);
	return false;
}

PUBLIC int32_t CALL serial_available(serial_t* port) {
	return _serial_native_available(port->nativePort);
}

PUBLIC int32_t CALL serial_read(serial_t* port, void* out, uint32_t len) {
	static uint8_t nullBuffer;

	len = len > (uint32_t) INT32_MAX ? INT32_MAX : len;

	uint32_t remaining = len;
	int32_t  totalRead = 0;
	int32_t  mRead;
	bool     errnoWasZero;

	while (remaining > 0) {
		errnoWasZero = errno == 0;
		mRead = _serial_native_read(port->nativePort, (out ? out : &nullBuffer), (out ? remaining : 1));

		if (mRead > 0) {
			remaining -= mRead;
			totalRead += mRead;
			if (out)
				out += mRead;
		} else { // mRead <= 0 (error or timeout)
			if (totalRead > 0) {
				// Ignore errors because some data was already read.
				// Error is raised only when no data was read.
				errno = errnoWasZero ? 0 : errno;
				return totalRead;
			} else {
				if (mRead < 0) { // Error
					__SET_ERROR(SERIAL_ERROR_IO);
					return -1;
				} else { // Timeout
					if (port->readTimeout > 0) {
						errno = SERIAL_ERROR_TIMEOUT;
						return -1;
					} else {
						return 0;
					}
				}
			}
		}
	}

	return totalRead;
}

PUBLIC bool CALL serial_write(serial_t* port, const void* in, uint32_t len) {
	uint32_t remaining = len;
	int32_t  written;

	while (remaining > 0) {
		written = _serial_native_write(port->nativePort, in, remaining > INT32_MAX ? INT32_MAX : remaining);

		// NOTE: function will return only when all data was written or an
		//       error occurred (timeout on write is considered an error).

		if (written <= 0) { // Error or timeout on while writting.
			__SET_ERROR(SERIAL_ERROR_IO);
			return false;
		} else {
			remaining -= written;
			in += written;
		}
	}

	return true;
}

PUBLIC bool CALL serial_flush(serial_t* port) {
	bool result = _serial_native_flush(port->nativePort);

	if (!result)
		__SET_ERROR(SERIAL_ERROR_IO);

	return result;
}

PUBLIC const char* CALL serial_version() {
	return LIB_VERSION;
}
