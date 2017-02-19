/*	Cafe OS Substrate

	substrate.h - Substrate API Header.
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

#ifndef _COS_SUBSTRATE_H_
#define _COS_SUBSTRATE_H_

/*	This API is still under active development.
	It could change at any time without warning, and without
	updating these numbers.
	The major version will be incremented to 1 when semantic versioning
	takes effect.
*/
#define COSS_API_MAJOR 0
#define COSS_API_MINOR 0
#define COSS_API_PATCH 1

#ifndef __ASSEMBLY //kinda like __CPLUSPLUS but for assembly

typedef struct _COSSubstrate_Specifics {
	/* Used for dynamic linking */
	void* substrate;
	void* substrateDynamic;

	/* Average Wii U junk */
	int (*OSDynLoad_Acquire)(const char* rpl, unsigned int* handle);
	int (*OSDynLoad_FindExport)(unsigned int handle, int isdata, const char* symbol, void* address);
} COSSubstrate_Specifics;

typedef struct _COSSubstrate_FunctionContext {
	void* source;
	unsigned int args[10];

	/*	TODO: Consider renaming to mc_hammer
		might get the point across better
	*/
	unsigned int substrate_internal;
} COSSubstrate_FunctionContext;

#endif //__ASSEMBLY

#define COSS_MEM_BASE (void*)0x60000000
#define COSS_MEM_SIZE 0x00800000

#define COSS_MAIN_HEAP_OFFSET +0x200
#define COSS_MAIN_HEAP_SIZE_OFFSET -0x200

#define COSS_MAIN_HEAP (int)(COSS_MEM_BASE COSS_MAIN_HEAP_OFFSET)
#define COSS_MAIN_HEAP_SIZE (COSS_MEM_SIZE COSS_MAIN_HEAP_SIZE_OFFSET)

#define COSS_SPECIFICS ((COSSubstrate_Specifics*)COSS_MEM_BASE)

//void (*COSSubstrate_PatchFunc)(void* func);

#define COSS_LMR_BAD_MODULE 0xFFFFFFFE
#define COSS_LMR_OK 0

#endif //_COS_SUBSTRATE_H_
