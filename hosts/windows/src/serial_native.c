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

#include <_serial_native.h>

#include <windows.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define __PREFIX "\\\\.\\"

#define __WIN_PORT(p) ((HANDLE)p)

static DCB* __get_cfg(HANDLE winPort, DCB* out) {
	if (!GetCommState(winPort, out)) {
		errno = SERIAL_ERROR_IO;
		return NULL;
	}

	return out;
}

static bool __set_baud(DCB* dcb, uint32_t baud) {
	dcb->BaudRate = baud;
	return true;
}

static bool __set_data_bits(DCB* dcb, serial_data_bits_e dataBits) {
	switch (dataBits) {
	case SERIAL_DATA_BITS_5:
	case SERIAL_DATA_BITS_6:
	case SERIAL_DATA_BITS_7:
	case SERIAL_DATA_BITS_8:
		dcb->ByteSize = (BYTE)dataBits;
		return true;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}
}

static bool __set_parity(DCB* dcb, serial_parity_e parity) {
	switch (parity) {
	case SERIAL_PARITY_NONE:
		dcb->Parity = NOPARITY;
		break;

	case SERIAL_PARITY_EVEN:
		dcb->Parity = EVENPARITY;
		break;

	case SERIAL_PARITY_ODD:
		dcb->Parity = ODDPARITY;
		break;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	return true;
}

static bool __set_stop_bits(DCB* dcb, serial_stop_bits_e stopBits) {
	switch(stopBits) {
	case SERIAL_STOP_BITS_1:
		dcb->StopBits = ONESTOPBIT;
		break;

	case SERIAL_STOP_BITS_1_5:
		dcb->StopBits = ONE5STOPBITS;
		break;

	case SERIAL_STOP_BITS_2:
		dcb->StopBits = TWOSTOPBITS;
		break;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	return true;
}

static bool __set_cfg(HANDLE winPort, DCB* dcb) {
	if (!SetCommState(winPort, dcb)) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

serial_list_t* _serial_native_list_ports(serial_list_t* list) {
	HKEY phkResult;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM\\", 0, KEY_READ, &phkResult) == ERROR_SUCCESS) {
		DWORD i = 0;
		char lpValueName[256];
		DWORD lpcchValueName;
		BYTE lpData[256];
		DWORD lpcbData;
		DWORD enumResult;

		while (true) {
			lpcchValueName = 256;
			enumResult = RegEnumValueA(phkResult, i, lpValueName, &lpcchValueName, NULL, NULL, lpData, &lpcbData);

			if (enumResult == ERROR_SUCCESS) {
				if (!_serial_list_add(list, (const char*)lpData)) {
					goto error;
				}

				i++;
 			} else if(enumResult == ERROR_NO_MORE_ITEMS) {
				goto cleanup;
			} else {
				goto error;
			}
		}
	} else {
		goto error;
	}

cleanup:
	if (!CloseHandle(phkResult))
		goto error;

	return list;

error:
	CloseHandle(phkResult);
	errno = SERIAL_ERROR_IO;
	return NULL;
}

serial_native_port_t _serial_native_open(const char* portName) {
	serial_native_port_t nativePort = INVALID_HANDLE_VALUE;

	int previousError;

	char portFullName[128];
	if (snprintf(portFullName, sizeof(portFullName) - 1, "%s%s", __PREFIX, portName) >= (sizeof(portFullName) - 1)) {
		errno = SERIAL_ERROR_MEM;
		goto error;
	}

	nativePort = (serial_native_port_t) CreateFile(
		portFullName,
		GENERIC_READ | GENERIC_WRITE, // Read / Write
		0,                            // No sharing
		0,                            // No security
		OPEN_EXISTING,                // Only existing port
		FILE_ATTRIBUTE_NORMAL,        // The file does not have other attributes set
		0                             // Null for Comm Devices
	);

	if (__WIN_PORT(nativePort) == INVALID_HANDLE_VALUE) {
		DWORD errorValue = GetLastError();

		if (errorValue == ERROR_ACCESS_DENIED) {
			errno = SERIAL_ERROR_ACCESS;
		} else if (errorValue == ERROR_FILE_NOT_FOUND) {
			errno = SERIAL_ERROR_NOT_FOUND;
		} else {
			errno = SERIAL_ERROR_IO;
		}

		goto error;
	}

	DCB dcb;
	if (!GetCommState(__WIN_PORT(nativePort), &dcb)) {
		errno = SERIAL_ERROR_IO;
		goto error;
	}

	return nativePort;

error:
	previousError = errno;

	if (__WIN_PORT(nativePort) != INVALID_HANDLE_VALUE)
		CloseHandle(__WIN_PORT(nativePort));

	errno = previousError; // Ignore any errors caused by CloseHandle()

	return NULL;
}

bool _serial_native_config(serial_native_port_t nativePort, const serial_config_t* config) {
	DCB dcb;

	if (!__get_cfg(__WIN_PORT(nativePort), &dcb))
		return false;

	bool result = __set_baud(&dcb, config->baud)
		&& __set_data_bits(&dcb, config->dataBits)
		&& __set_parity(&dcb, config->parity)
		&& __set_stop_bits(&dcb, config->stopBits);

	if (!result)
		return false;

	return __set_cfg(__WIN_PORT(nativePort), &dcb);
}

bool _serial_native_set_read_timeout(serial_native_port_t nativePort, uint32_t millis) {
	COMMTIMEOUTS commTimeouts = { 0 };

	commTimeouts.ReadIntervalTimeout = millis;
	commTimeouts.ReadTotalTimeoutConstant = millis;

	if (!SetCommTimeouts(__WIN_PORT(nativePort), &commTimeouts)) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

bool _serial_native_purge(serial_native_port_t nativePort, serial_purge_type_e type) {
	DWORD winPurgeType;
	switch(type) {
	case SERIAL_PURGE_TYPE_RX:
		winPurgeType = PURGE_RXCLEAR;
		break;

	case SERIAL_PURGE_TYPE_TX:
		winPurgeType = PURGE_TXCLEAR;
		break;

	case SERIAL_PURGE_TYPE_RX_TX:
		winPurgeType = PURGE_RXCLEAR | PURGE_TXCLEAR;
		break;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	if (!PurgeComm(__WIN_PORT(nativePort), winPurgeType)) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

bool _serial_native_close(serial_native_port_t nativePort) {
	if (!CloseHandle(__WIN_PORT(nativePort))) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

int32_t _serial_native_available(const serial_native_port_t nativePort) {
	COMSTAT comstat;
	comstat.cbInQue = 0;

	if (!ClearCommError(__WIN_PORT(nativePort), NULL, &comstat)) {
		errno = SERIAL_ERROR_IO;
		return -1;
	}

	if (comstat.cbInQue > (uint32_t)INT32_MAX) {
		return INT32_MAX;
	} else {
		return (int32_t)comstat.cbInQue;
	}
}

int32_t _serial_native_read(serial_native_port_t nativePort, void* out, uint32_t len) {
	DWORD mRead = 0;

	if (!ReadFile(__WIN_PORT(nativePort), out, len, &mRead, NULL)) {
		errno = SERIAL_ERROR_IO;
		return -1;
	}

	return (int32_t)mRead;
}

int32_t _serial_native_write(serial_native_port_t nativePort, const void* in, uint32_t len) {
	DWORD written;

	if (!WriteFile(__WIN_PORT(nativePort), in, len, &written, NULL)) {
		errno = SERIAL_ERROR_IO;
		return -1;
	}

	return (int32_t)written;
}

bool _serial_native_flush(serial_native_port_t nativePort) {
	if (!FlushFileBuffers(__WIN_PORT(nativePort))) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}
