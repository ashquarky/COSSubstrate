/*	Cafe OS Substrate

	dynamic_linker.h - Header for dynamic_linker.c.
	Paired with dynamic_linker.c.

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

#ifndef _DYNAMIC_LINKER_H_
#define _DYNAMIC_LINKER_H_

#define MODULE_NAME_SZ 20

typedef struct _COSSubstrate_Module {
	char name[20];
	void* file;
	void* dynamic;

	void* next;
} COSSubstrate_Module;

void private_addToModuleHashTable(COSSubstrate_Module* module);
COSSubstrate_Module* private_lookupFromModuleHashTable(char* name);

int COSSDynLoad_Acquire(char* cosm, unsigned int* handle);
int COSSDynLoad_FindExport(unsigned int handle, char* symbol, void* addr);

#endif
