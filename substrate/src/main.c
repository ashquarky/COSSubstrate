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

#include <string.h>
#include "patches/patches.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/mem_functions.h"
#include "loader/dynamic_linker.h"
#include <substrate/substrate.h>

void _start() {}

/*	Takes in data from the Installer and arranges it in memory.
	This minimizes updates to the Installer.
*/
void private_doSetup(void* substrate, void* substrateDynamic, void* OSDynLoad_Acquire, void* OSDynLoad_FindExport) {
	COSS_SPECIFICS->COSSDynLoad_Acquire = &COSSDynLoad_Acquire;
	COSS_SPECIFICS->COSSDynLoad_FindExport = &COSSDynLoad_FindExport;
	COSS_SPECIFICS->OSDynLoad_Acquire = OSDynLoad_Acquire;
	COSS_SPECIFICS->OSDynLoad_FindExport = OSDynLoad_FindExport;
	InitOSFunctionPointers();
	COSSubstrate_Module* self = MEMAllocFromExpHeapEx(COSS_MAIN_HEAP, sizeof(COSSubstrate_Module), 0x4);
	self->file = substrate;
	self->dynamic = substrateDynamic;
	memcpy(&self->name, &"substrate.cosm", MODULE_NAME_SZ);
	private_addToModuleHashTable(self);
}
