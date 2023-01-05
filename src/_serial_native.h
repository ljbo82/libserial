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

/**
 * @file
 * @brief [PRIVATE] Native serial interface
*/
#pragma once

#include "_serial.h"

/**
 * @brief Populates a list with the names of available serial platforms.
 *
 * @param list List to be filled with port names (it can be safely assumed
 *        it will be empty before the function is actually called).
 *
 * @return On success, returns \c list. Otherwise returns \n NULL.
 *
 * @see _serial_list_add
 *
*/
serial_list_t* _serial_native_list_ports(serial_list_t* list);

/**
 * @brief Opens a native port.
 *
 * @param portName Port to be open.
 *
 * @return On success returns a non-null value. Otherwise, returns \c NULL.
*/
void* _serial_native_open(const char* portName);

/**
 * @brief Configures the native port
 *
 * @param nativePort Native serial port.
 * @param config Serial port configuration.
*/
bool _serial_native_config(void* nativePort, const serial_config_t* config);

/**
 * @brief Sets the timeout used while reading data.
 *
 * @param nativePort Native serial port.
 * @param millis Number of milliseconds to wait for a single byte to be
 *               available for read.
 *
 * @return A boolean indicating if operation was successfull.
*/
bool _serial_native_set_read_timeout(void* nativePort, uint32_t millis);

/**
 * @brief Purges a port.
 *
 * @param nativePort Native serial port.
 * @param type Defines the buffers that shall be purged.
 *
 * @return A boolean indicating if operation was successfull.
*/
bool _serial_native_purge(void* nativePort, serial_purge_type_e type);

/**
 * @brief Closes a native port previously open using _serial_native_open().
 *
 * @param nativePort Native serial port.
 *
 * @return A boolean indicating if operation was successfull.
*/
bool _serial_native_close(void* nativePort);

/**
 * @brief Returns the number of bytes available for read on the port.
 *
 * @param nativePort Native serial port.
 *
 * @return On success, returns the number of bytes available for read
 *         (<code>&gt;= 0</code>). Otherwise, a negative value will be returned.
*/
int32_t _serial_native_available(const void* nativePort);

/**
 * @brief Reads data from a port.
 *
 * @param nativePort Native serial port.
 * @param out Buffer which will hold read data.
 * @param len Maximum number of bytes to read (it can be safely assumed that
 *        maximum value is \c INT32_MAX).
 *
 * @return On success, returns the number of bytes actually read. Otherwise,
 *         returns a negative value (zero is returned on timeout while
 *         reading data).
*/
int32_t _serial_native_read(void* nativePort, void* out, uint32_t len);

/**
 * @brief Writes data into a port.
 *
 * @param nativePort Native serial port.
 * @param in Buffer containing data to be written into the port.
 * @param len Maximum number of bytes to write (it can be safely assumed that
 *        maximum value is \c INT32_MAX).
 *
 * @return On success, returns the number of bytes actually read. Otherwise,
 *         returns a negative value (zero is returned on timeout while
 *         writing data).
*/
int32_t _serial_native_write(void* nativePort, const void* in, uint32_t len);

/**
 * @brief Flushes any pending data.
 *
 * @param nativePort Native serial port.
 *
 * @return A boolean indicating if operation was successful.
*/
bool _serial_native_flush(void* nativePort);
