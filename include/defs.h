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

#if defined _WIN32 || defined __CYGWIN__
	#if defined(BUILD_DLL) && defined(USE_STATIC)
		#error BUILD_DLL and USE_STATIC are both defined
	#endif

	#ifdef BUILD_DLL
		/** @internal */
		#define PUBLIC __declspec(dllexport)
	#else
		#ifndef USE_STATIC
			#define PUBLIC __declspec(dllimport)
		#else
			#define PUBLIC
		#endif
	#endif

	/** @internal */
	#define CALL __cdecl
#else
	#if __GNUC__ >= 4
		/** @internal */
		#define PUBLIC __attribute__ ((visibility ("default")))
	#else
		/** @internal */
		#define PUBLIC
	#endif

	/** @internal */
	#define CALL
#endif
