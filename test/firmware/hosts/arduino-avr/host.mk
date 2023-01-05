# Copyright (c) 2023 Leandro José Britto de Oliveira
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# arduino-core =================================================================
ARDUINO_CORE_AVR_DIR := arduino-core-avr
PRE_BUILD_DEPS       += $(O)/libs/arduino-core-avr.marker
LDFLAGS              += -larduino-core1

--arduino-core:
	$(O_VERBOSE)$(MAKE) -C $(ARDUINO_CORE_AVR_DIR) HOST=$(HOST) O=$(call FN_REL_DIR,$(ARDUINO_CORE_AVR_DIR),$(O)/libs) BUILD_SUBDIR=arduino-core DIST_MARKER=arduino-core-avr.marker

$(O)/libs/arduino-core-avr.marker: --arduino-core ;
# ==============================================================================
