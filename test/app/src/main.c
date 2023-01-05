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

#include <serial.h>

#include "console.h"
#include "connection/msg.h"
#include "connection/pkt.h"

#include <string.h>
#include <errno.h>

#define __ASSERT(cond) if (!(cond)) { console_color_printf(CONSOLE_ANSI_COLOR_BRIGHT_RED, "[ASSERTION ERROR][%s:%d] errno: %d (%s)\n", __FILE__, __LINE__, errno, serial_error_to_str(errno)); return 1; }

int main(int argc, char** argv) {
	serial_list_t* list = serial_list_new();

	if (!serial_list_ports(list)) {
		console_color_printf(CONSOLE_ANSI_COLOR_BRIGHT_RED, "[ERROR] Error listing serial ports\n");
		return 1;
	}

	if (serial_list_size(list) == 0) {
		console_color_printf(CONSOLE_ANSI_COLOR_BRIGHT_RED, "[ERROR] There is no serial ports\n");
		return 1;
	}

	const char* portName;
	if (serial_list_size(list) == 1) {
		portName = serial_list_item(list, 0);
	} else {
		console_printf("Available serial ports\n\n");
		for (size_t i = 0; i < serial_list_size(list); i++) {
			console_printf("%zu) %s\n", i, serial_list_item(list, i));
		}

		console_printf("\n");
		int option = console_get_num_option("Choose a port: ", serial_list_size(list));
		portName = serial_list_item(list, option);
	}

	connection_t* connection = connection_open(portName);
	if (!connection) {
		console_printf("[ERROR] Error opening serial port %s: %s\n", portName, serial_error_to_str(errno));
		return 1;
	}

	serial_list_del(list);
	__ASSERT(connection_config(connection, 9600, CONNECTION_CONFIG_8N1));
	__ASSERT(connection_msg_ping(connection, "hello"));
	__ASSERT(connection_msg_protocol(connection, 9600, CONNECTION_CONFIG_8N1, CONNECTION_MODE_PACKET));
	__ASSERT(connection_pkt_ping(connection, "world!", strlen("world!")));
	__ASSERT(connection_pkt_protocol(connection, 2400, CONNECTION_CONFIG_7E2, CONNECTION_MODE_MESSAGE));
	__ASSERT(connection_msg_ping(connection, "HELLO"));

	__ASSERT(connection_close(connection));

	console_color_printf(CONSOLE_ANSI_COLOR_BRIGHT_GREEN, "Success!\n");
	return 0;
}
