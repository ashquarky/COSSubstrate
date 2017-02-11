/*	Cafe OS Substrate

	main.c - Main Substrate logic.
	No partner file.

	https://github.com/QuarkTheAwesome/COSSubstrate

	Copyright (c) 2016 Ash (QuarkTheAwesome)
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to
	deal in the Software without restriction, including without limitation the
	rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
	sell copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include "patches/patches.h"
#include "dynamic_libs/os_functions.h"
#include <substrate/substrate.h>

int testSubroutine() {
	return 0x69690000;
}

int _start() {
	return testSubroutine() | 0x6969;
}

/*	Takes in data from the Installer and arranges it in memory.
	This minimizes updates to the Installer.
*/
void private_doSetup(void* substrate, void* substrateDynamic, void* OSDynLoad_Acquire, void* OSDynLoad_FindExport) {
	COSS_SPECIFICS->substrate = substrate;
	COSS_SPECIFICS->substrateDynamic = substrateDynamic;
	COSS_SPECIFICS->OSDynLoad_Acquire = OSDynLoad_Acquire;
	COSS_SPECIFICS->OSDynLoad_FindExport = OSDynLoad_FindExport;
	InitOSFunctionPointers();
}
