# Copyright (c) 2022 Leandro José Britto de Oliveira
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

export CPP_PROJECT_BUILDER ?= $(abspath ../libs/libcomm/make)

PROJ_NAME := serial-app
PROJ_TYPE := app

O ?= output

# libserial ====================================================================
LIBSERIAL_DIR  := ../..
PRE_BUILD_DEPS += $(O)/libs/libserial.marker
LDFLAGS        += -lserial0

--libserial:
	$(O_VERBOSE)$(MAKE) -C $(LIBSERIAL_DIR) O=$(call FN_REL_DIR,$(LIBSERIAL_DIR),$(O)/libs) BUILD_SUBDIR=libserial DIST_MARKER=libserial.marker LIB_TYPE=static

$(O)/libs/libserial.marker: --libserial ;
# ==============================================================================

# libcomm ======================================================================
LIBCOMM_DIR    := ../libs/libcomm
PRE_BUILD_DEPS += $(O)/libs/libcomm.marker
LDFLAGS        += -lcomm0

--libcomm:
	$(O_VERBOSE)$(MAKE) -C $(LIBCOMM_DIR) O=$(call FN_REL_DIR,$(LIBCOMM_DIR),$(O)/libs) BUILD_SUBDIR=libcomm DIST_MARKER=libcomm.marker LIB_TYPE=static

$(O)/libs/libcomm.marker: --libcomm ;
# ==============================================================================

INCLUDE_DIRS += $(O)/libs/dist/include
LDFLAGS      += -L$(O)/libs/dist/lib

ifeq ($(DEBUG),1)
    CFLAGS += -DDEBUG_ENABLED=1
endif

include $(CPP_PROJECT_BUILDER)/builder.mk
