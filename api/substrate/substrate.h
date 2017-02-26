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

#ifndef __ASSEMBLY //kinda like __CPLUSPLUS but for assembly; turns off everything but #defines
typedef struct _COSSubstrate_Specifics {
	/*	Very similar to native OSDynLoad functions. Must load any non-Substrate
		modules first.
	*/
	int (*COSSDynLoad_Acquire)(char* cosm, unsigned int* handle);
	int (*COSSDynLoad_FindExport)(unsigned int handle, char* symbol, void* addr);

	/* Average Wii U junk */
	int (*OSDynLoad_Acquire)(const char* rpl, unsigned int* handle);
	int (*OSDynLoad_FindExport)(unsigned int handle, int isdata, const char* symbol, void* address);
} COSSubstrate_Specifics;
#endif //__ASSEMBLY

#define COSS_MEM_BASE (void*)0x60000000
#define COSS_MEM_SIZE 0x00800000

#define COSS_MAIN_HEAP_OFFSET +0x200

/*	Main heap pointer; use MEMAllocFromExpHeapEx to allocate.
*/
#define COSS_MAIN_HEAP (int)(COSS_MEM_BASE COSS_MAIN_HEAP_OFFSET)
#define COSS_MAIN_HEAP_SIZE (COSS_MEM_SIZE - COSS_MAIN_HEAP_OFFSET)

/*	Pointer to COSSubstrate_Specifics struct.
	e.g. COSS_SPECIFICS->OSDynLoad_Acquire();
*/
#define COSS_SPECIFICS ((COSSubstrate_Specifics*)COSS_MEM_BASE)


/*******************************************************************************
Patching Functions - see patcher.c
*******************************************************************************/
#ifndef __ASSEMBLY

typedef struct _COSSubstrate_FunctionContext {
	/*	Address of function that triggered the callback.
		Should be equal to value from OSDynLoad.
	*/
	void* source;
	/*	Function arguments.
		If written, the new value will be passed into the original function
		appropriately.
		Does not work correctly with floats.
	*/
	unsigned int args[10];

	/*	TODO: Consider renaming to mc_hammer
		might get the point across better
	*/
	unsigned int substrate_internal;
} COSSubstrate_FunctionContext;

#ifndef COSS_NO_FUNC_POINTERS //This is super useful inside the Substrate code

/*	Hooks given callback into a function, patching if neccesary.
	Multiple callbacks are supported.
	Callbaks are dispatched BEFORE the patched function is ran.
*/
void (*COSSubstrate_PatchFunc)(void* func, void(*callback)(COSSubstrate_FunctionContext* ctx));
/*	Not ready for use at this point.
*/
void (*COSSubstrate_RestoreFunc)(void* func);

#endif //COSS_NO_FUNC_POINTERS
#endif //__ASSEMBLY

/*******************************************************************************
Module Loading - see module_loader.c
*******************************************************************************/
#ifndef __ASSEMBLY
#ifndef COSS_NO_FUNC_POINTERS

/*	Loads a module from a given memory address with a given identifier.

	Module is copied into internal Substrate heap, so module_tmp can safely
	be freed once this function returns.
	name is used with COSSDynLoad_Acquire; since there's not neccesarily a way
	of identifying an ELF after it's in memory. Can be on the stack, no biggie.

	Returns a COSS_LMR code from below.
*/
int (*COSSubstrate_LoadModuleRaw)(void* module_tmp, char* name);

#endif //COSS_NO_FUNC_POINTERS
#endif //__ASSEMBLY

#define COSS_LMR_BAD_MODULE 0xFFFFFFFE
#define COSS_LMR_OK 0

#endif //_COS_SUBSTRATE_H_
