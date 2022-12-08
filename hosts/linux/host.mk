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

ifneq ($(HOST),linux-x64)
    ifneq ($(HOST),linux-x86)
        $(error Unsupported HOST: $(HOST))
    endif
endif

ifeq ($(LIB_TYPE),static)
    CFLAGS += -DSTATIC_LIB
endif

ifeq ($(NATIVE_HOST),windows-x64)
    ifeq ($(HOST),linux-x64)
        CROSS_COMPILE := cygwin-linux-x64-
    else # ifeq ($(HOST),linux-x86)
        CROSS_COMPILE := cygwin-linux-x86-
    endif
else ifeq ($(NATIVE_HOST),linux-x64)
    ifeq ($(HOST),linux-x86)
        ifeq ($(origin CROSS_COMPILE),undefined)
            CROSS_COMPILE :=
            CFLAGS        += -m32
            CXXFLAGS      += -m32
            LDFLAGS       += -m32
        endif
    endif
endif
