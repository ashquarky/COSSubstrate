/*	Cafe OS Substrate

	patcher.c - Arranging, generating and patching PowerPC code
	No partner file. See patches/ for other related files.

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

#include "dynamic_libs/mem_functions.h"
#include "patches/patches.h"

#include <substrate/substrate.h>
#include <uthash.h>

typedef struct _COSSubstrate_PatchedFunction {
	/* Make sure to keep this aligned to 0x4! */
	unsigned char origInstructions[MAIN_PATCH_SIZE];

	int id;
	UT_hash_handle hh;
} COSSubstrate_PatchedFunction;

COSSubstrate_PatchedFunction* patchedFunctions = NULL;


void* private_prepareMainPatch() {
	unsigned int* patch = (unsigned int*)&main_patch;
	unsigned int handler = (unsigned int)&PatchHandler;

	patch[MAIN_PATCH_INSTR_PATCH_LIS] &= MAIN_PATCH_INSTR_PATCH_LIS_MASK;
	patch[MAIN_PATCH_INSTR_PATCH_LIS] |= ((handler & 0xFFFF0000) >> 16);

	patch[MAIN_PATCH_INSTR_PATCH_ORI] &= MAIN_PATCH_INSTR_PATCH_ORI_MASK;
	patch[MAIN_PATCH_INSTR_PATCH_ORI] |= (handler & 0x0000FFFF);

	DCFlushRange(patch, MAIN_PATCH_SIZE);
	ICInvalidateRange(patch, MAIN_PATCH_SIZE);

	return patch;
}

/*	WIP function to see if my codegen works okay
	From this alone you can probably tell this is going to be a very
	different type of patcher to what we're used to

	I think "function patcher" and I think actually overwriting the function :3
*/
void* COSSubstrate_GeneratePatch(void* func) {



	/*COSSubstrate_PatchedFunction* patch = MEMAllocFromExpHeapEx(COSS_MAIN_HEAP, sizeof(COSSubstrate_PatchedFunction), 0x4);
	patch->id = (int)func;
	memcpy(patch->origInstructions, func, MAIN_PATCH_SIZE);
	HASH_ADD_INT(patchedFunctions, id, patch);*/
}
