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
#include "debug.hpp"
#include "comm.hpp"
#include "led.hpp"
#include "hal/serial.hpp"

#include <signal.h>
#include <unistd.h>

volatile bool __stop = false;

static void __onSignal(int sgn) {
	switch(sgn) {
	case SIGTERM:
	case SIGABRT:
	case SIGSEGV:
	case SIGINT:
		__stop = true;
		break;
	}
}

int main() {
	signal(SIGTERM, __onSignal);
	signal(SIGABRT, __onSignal);
	signal(SIGSEGV, __onSignal);
	signal(SIGINT, __onSignal);

	if (!hal::serial::open()) return 1;

	led::init();
	comm::init();

	DEBUG("Application ready");

	__stop = false;
	while(!__stop) {
		led::check();
		comm::check();
		usleep(5000);
	}

	return hal::serial::close() ? 0 : 1;
}
