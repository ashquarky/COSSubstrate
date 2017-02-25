/*	Cafe OS Substrate

	dynamic_linker.c - Manages modules.
	Paired with dynamic_linker.h.

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
#include <UDynLoad.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/mem_functions.h"
#include "utils/hash.h"
#include "loader/dynamic_linker.h"
#include <substrate/substrate.h>

COSSubstrate_Module** modules = 0;

void private_addToModuleHashTable(COSSubstrate_Module* module) {
	unsigned short hash = hashStr(module->name);
	hash >>= 2; //0x3FFF possibilities rather than 0xFFFF
	hash &= 0x3FFF;
	if (modules) {
		if (modules[hash]) {
			//hash collision!
			COSSubstrate_Module* p = modules[hash];
			while (p != 0) {
				p = (COSSubstrate_Module*)p->next;
			}
			p->next = (void*)module;
		} else {
			modules[hash] = module;
		}
	} else {
		modules = MEMAllocFromExpHeapEx(COSS_MAIN_HEAP, 0x3FFF * sizeof(COSSubstrate_Module*), 0x4); //oh man
		if (!modules) OSFatal("Couldn't allocate module hash table!");
		modules[hash] = module;
	}
}

COSSubstrate_Module* private_lookupFromModuleHashTable(char* name) {
	if (!modules) return 0;

	unsigned short hash = hashStr(name);
	hash >>= 2; //0x3FFF possibilities rather than 0xFFFF
	hash &= 0x3FFF;
	if (modules[hash]) {
		COSSubstrate_Module* p = modules[hash];
		while (p != 0) {
			if (!strcmp(p->name, name)) return modules[hash];
			p = (COSSubstrate_Module*)p->next;
		}
	}
	return 0;
}

/*	Function similar to OSDynLoad_Acquire, but for COSS modules. Not much to say about it, really.

	Worth noting that (unlike OSDynLoad) handles are *not* pointers to the module itself.
	Intead, they go to the COSSubstrate_Module struct for that module.
*/
int COSSDynLoad_Acquire(char* cosm, unsigned int* handle) {
	unsigned int module = (unsigned int)private_lookupFromModuleHashTable(cosm);

	if (module) {
		*handle = module;
		return 0;
	}
	return -1; //TODO #define this
}

/*	Wrapper for UDynLoad_FindExportDynamic so we can keep up this OSDynLoad aesthetic.
*/
int COSSDynLoad_FindExport(unsigned int handle, char* symbol, void* addr) {
	if (!handle || !symbol || !addr) return -1;
	COSSubstrate_Module* module = (COSSubstrate_Module*)handle;

	return UDynLoad_FindExportDynamic(module->file, module->dynamic, symbol, (void**)addr);
}
