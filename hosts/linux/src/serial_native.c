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

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <unistd.h>
#include <regex.h>
#include <stdio.h>

#define __PORT_BASE "/dev"
#define __PORT_NAME_PATTERN "(ttyS|ttyUSB|ttyACM|ttyAMA|rfcomm|ttyO)[0-9]{1,3}"
#define __LINUX_PORT(nativePort) ((int)(intptr_t)nativePort)

static bool __regex_match(const regex_t* regex, const char* str) {
	return regexec(regex, str, 0, NULL, 0) == 0;
}

static bool __exists(const char* path, struct stat* out) {
	static struct stat mStat;

	bool errnoWasZero = errno == 0;
	if (stat(path, out == NULL ? &mStat : out) < 0) {
		if (errno == ENOENT) {
			if (errnoWasZero) {
				errno = 0;
			}
			return false;
		} else {
			errno = SERIAL_ERROR_IO;
			return false;
		}
	}

	return true;
}

static bool __path_is_dir(const char* path) {
	struct stat pathStat;

	if (!__exists(path, &pathStat))
		return false;

	return S_ISDIR(pathStat.st_mode);
}

static bool __path_is_char_file(const char* path) {
	struct stat pathStat;

	if (!__exists(path, &pathStat))
		return false;

	return S_ISCHR(pathStat.st_mode);
}

static serial_list_t* __serial_native_list_unix_ports(serial_list_t* list, const char* namePattern) {
	regex_t regex;
	DIR* dir = NULL;
	char portName[512];
	serial_native_port_t nativePort;

	if (regcomp(&regex, namePattern, REG_EXTENDED)) {
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	if (!__exists(__PORT_BASE, NULL)) {
		errno = SERIAL_ERROR_NOT_FOUND;
		list = NULL;
		goto clean_up;
	}

	if (!__path_is_dir(__PORT_BASE)) {
		errno = SERIAL_ERROR_INVALID_PARAM;
		list = NULL;
		goto clean_up;
	}

	if (!(dir = opendir(__PORT_BASE))) {
		list = NULL;
		goto clean_up;
	}

	struct dirent *dirEntry;
	int previousError = errno;
	while(true) {
		dirEntry = readdir(dir);

		if (dirEntry == NULL) { // No more entries
			errno = previousError == 0 ? 0 : previousError;
			goto clean_up;
		}

		const char* filename = dirEntry->d_name;

		if (strcmp(".", filename) == 0 || strcmp("..", filename) == 0)
			continue;

		if (snprintf(portName, sizeof(portName) - 1, "%s/%s", __PORT_BASE, filename) > (sizeof(portName) - 1)) {
			// Truncated filename (buffer size is not enough)
			errno = SERIAL_ERROR_MEM;
			list = NULL;
			goto clean_up;
		}

		if (__path_is_char_file(portName) && __regex_match(&regex, filename)) {
			if ((nativePort =_serial_native_open(portName)) != NULL) {
				if (_serial_native_close(nativePort)) {
					if (!_serial_list_add(list, portName)) {
						list = NULL;
						goto clean_up;
					}
				} else {
					list = NULL;
					goto clean_up;
				}
			} else {
				if (previousError == 0) {
					errno = 0; // Ignore errors caused by open()
				}
			}
		}
	}

clean_up:
	previousError = errno;

	if (dir)
		closedir(dir);

	errno = previousError; // Ignore any new error caused by closedir()

	regfree(&regex);

	return list;
}

static struct termios* __get_cfg(int linuxNativePort, struct termios *out) {
	if (tcgetattr(linuxNativePort, out) < 0) {
		errno = SERIAL_ERROR_IO;
		return NULL;
	}

	// Enable the receiver and set local mode...
	out->c_cflag |= (CLOCAL | CREAD);
	out->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	out->c_iflag &= ~(ISTRIP | IGNCR | INLCR | ICRNL
		#ifdef IUCLC
			| IUCLC
		#endif
	);

	out->c_oflag &= ~(OPOST
		#ifdef ONLCR
			| ONLCR
		#endif
		#ifdef OCRNL
			| OCRNL
		#endif
		#ifdef ONOCR
			| ONOCR
		#endif
		#ifdef ONLRET
			| ONLRET
		#endif
	);

	return out;
}

static bool __set_baud(struct termios* termios, uint32_t baud) {
	int unixBaud;
	#define _case_unix_baud(baud) case baud: unixBaud = B##baud; break

	switch(baud) {
	_case_unix_baud(50);
	_case_unix_baud(75);
	_case_unix_baud(110);
	_case_unix_baud(134);
	_case_unix_baud(150);
	_case_unix_baud(200);
	_case_unix_baud(300);
	_case_unix_baud(600);
	_case_unix_baud(1200);
	_case_unix_baud(1800);
	_case_unix_baud(2400);
	_case_unix_baud(4800);
	_case_unix_baud(9600);
	_case_unix_baud(19200);
	_case_unix_baud(38400);
	_case_unix_baud(57600);
	_case_unix_baud(115200);
	_case_unix_baud(230400);
	_case_unix_baud(460800);
	_case_unix_baud(500000);
	_case_unix_baud(576000);
	_case_unix_baud(921600);
	_case_unix_baud(1000000);
	_case_unix_baud(1152000);
	_case_unix_baud(1500000);
	_case_unix_baud(2000000);
	_case_unix_baud(2500000);
	_case_unix_baud(3000000);
	_case_unix_baud(3500000);
	_case_unix_baud(4000000);
	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}
	#undef _case_unix_baud

	if (cfsetispeed(termios, unixBaud) < 0)
		goto error;

	if (cfsetospeed(termios, unixBaud) < 0)
		goto error;

	return true;

error:
	errno = SERIAL_ERROR_IO;
	return false;
}

static bool __set_data_bits(struct termios* termios, serial_data_bits_e dataBits) {
	int unixDataBits;
	switch (dataBits) {
	case SERIAL_DATA_BITS_5:
		unixDataBits = CS5;
		break;

	case SERIAL_DATA_BITS_6:
		unixDataBits = CS6;
		break;

	case SERIAL_DATA_BITS_7:
		unixDataBits = CS7;
		break;

	case SERIAL_DATA_BITS_8:
		unixDataBits = CS8;
		break;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	termios->c_cflag &= ~CSIZE; // Mask the character size bits
	termios->c_cflag |= unixDataBits;

	return true;
}

static bool __set_parity(struct termios* termios, serial_parity_e parity) {
	switch (parity) {
	case SERIAL_PARITY_NONE:
		termios->c_cflag &= ~PARENB;
		break;

	case SERIAL_PARITY_EVEN:
		termios->c_cflag |= PARENB;
		termios->c_cflag &= ~PARODD;
		break;

	case SERIAL_PARITY_ODD:
		termios->c_cflag |= PARENB;
		termios->c_cflag |= PARODD;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	return true;
}

static bool __set_stop_bits(struct termios* termios, serial_stop_bits_e stopBits) {
	switch (stopBits) {
	case SERIAL_STOP_BITS_1:
		termios->c_cflag &= ~CSTOPB;
		break;

	case SERIAL_STOP_BITS_1_5:
	case SERIAL_STOP_BITS_2:
		termios->c_cflag |= CSTOPB;
		break;

	default:
		errno = SERIAL_ERROR_INVALID_PARAM;
		return false;
	}

	return true;
}

static bool __set_cfg(int linuxNativePort, const struct termios* cfg) {
	if (tcsetattr(linuxNativePort, TCSANOW, cfg) < 0) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

serial_list_t* _serial_native_list_ports(serial_list_t* list) {
	return __serial_native_list_unix_ports(list, __PORT_NAME_PATTERN);
}

serial_native_port_t _serial_native_open(const char* portName) {
	int previousError;

	int linuxNativePort = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);

	if (linuxNativePort < 0)
		goto error;

	struct termios settings;
	if (tcgetattr(linuxNativePort, &settings) < 0)
		goto error;

	int flags;
	if ((flags = fcntl(linuxNativePort, F_GETFL, 0)) < 0)
		goto error;

	flags &= ~O_NDELAY; // Restores blocking mode after open
	if (fcntl(linuxNativePort, F_SETFL, flags) < 0)
		goto error;

	return (serial_native_port_t)(size_t) linuxNativePort;

error:
	previousError = errno;

	if (linuxNativePort > 0)
		close(linuxNativePort);

	errno = previousError; // Discards any error caused by close()

	switch(errno) {
	case EACCES:
		errno = SERIAL_ERROR_ACCESS;
		break;

	case ENOENT:
		errno = SERIAL_ERROR_NOT_FOUND;
		break;

	case ENOMEM:
		errno = SERIAL_ERROR_MEM;
		break;

	case EINVAL:
		errno = SERIAL_ERROR_INVALID_PARAM;
		break;

	default:
		errno = SERIAL_ERROR_IO;
		break;
	}

	return NULL;
}

bool _serial_native_config(serial_native_port_t nativePort, const serial_config_t* config) {
	struct termios termios;

	if (!__get_cfg(__LINUX_PORT(nativePort), &termios))
		return false;

	bool result = __set_baud(&termios, config->baud)
		&& __set_data_bits(&termios, config->dataBits)
		&& __set_parity(&termios, config->parity)
		&& __set_stop_bits(&termios, config->stopBits);

	if (!result)
		return false;

	return __set_cfg(__LINUX_PORT(nativePort), &termios);
}

bool _serial_native_set_read_timeout(serial_native_port_t nativePort, uint32_t millis) {
	struct termios termios;

	if (!__get_cfg(__LINUX_PORT(nativePort), &termios))
		return false;

	termios.c_cc[VTIME] = millis / 100;
	termios.c_cc[VMIN]  = 0;

	if (!__set_cfg(__LINUX_PORT(nativePort), &termios))
		return false;

	return true;
}

bool _serial_native_purge(serial_native_port_t nativePort, serial_purge_type_e type) {
	int unixPurgeType;
	switch(type) {
		case SERIAL_PURGE_TYPE_RX:
			unixPurgeType = TCIFLUSH;
			break;

		case SERIAL_PURGE_TYPE_TX:
			unixPurgeType = TCOFLUSH;
			break;

		case SERIAL_PURGE_TYPE_RX_TX:
			unixPurgeType = TCIOFLUSH;
			break;

		default:
			errno = SERIAL_ERROR_INVALID_PARAM;
			return false;
	}

	if (tcflush(__LINUX_PORT(nativePort), unixPurgeType) < 0) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

bool _serial_native_close(serial_native_port_t nativePort) {
	if (close(__LINUX_PORT(nativePort)) < 0) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}

int32_t _serial_native_available(serial_native_port_t nativePort) {
	int32_t bytes;

	if (ioctl(__LINUX_PORT(nativePort), FIONREAD, &bytes) < 0) {
		errno = SERIAL_ERROR_IO;
		return -1;
	}

	return bytes;
}

int32_t _serial_native_read(serial_native_port_t nativePort, void* out, uint32_t len) {
	uint32_t maxRef = (SIZE_MAX > INT32_MAX) ? INT32_MAX : SIZE_MAX;
	int32_t mRead = read(__LINUX_PORT(nativePort), out, len > maxRef ? maxRef : len);

	if (mRead < 0) {
		errno = SERIAL_ERROR_IO;
		return -1;
	}

	return mRead;
}

int32_t _serial_native_write(serial_native_port_t nativePort, const void* in, uint32_t len) {
	uint32_t maxRef = (SIZE_MAX > INT32_MAX) ? INT32_MAX : SIZE_MAX;
	ssize_t written = write(__LINUX_PORT(nativePort), in, len > maxRef ? maxRef : len);

	if (written < 0)
		goto error;

	return written;

error:
	errno = SERIAL_ERROR_IO;
	return -1;
}

bool _serial_native_flush(serial_native_port_t nativePort) {
	if (tcdrain(__LINUX_PORT(nativePort)) < 0) {
		errno = SERIAL_ERROR_IO;
		return false;
	}

	return true;
}
