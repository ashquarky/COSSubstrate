/*	Cafe OS Substrate

	patches.h - Exposes Assembly patches to patcher.c.
	Paired with main_patch.S, patcher.c and patch_handler.S.

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

#ifndef _PATCHES_H_
#define _PATCHES_H_

//patcher.c



//main_patch.S
#ifndef __ASSEMBLY
void main_patch();
#endif //__ASSEMBLY

#define MAIN_PATCH main_patch
#define MAIN_PATCH_SIZE 0x4 * 5 /* 4 bytes to an instruction, 5 instructions */
#define MAIN_PATCH_INSTR_PATCH_LIS 0 /* Patch first instruction */
#define MAIN_PATCH_INSTR_PATCH_LIS_MASK 0x0000FFFF
#define MAIN_PATCH_INSTR_PATCH_ORI 1
#define MAIN_PATCH_INSTR_PATCH_ORI_MASK 0x0000FFFF

//patch_handler.S

#ifndef __ASSEMBLY
void PatchHandler(int r3, int r4, int r5, int r6, int r7, int r8, int r9, int r10, int r11, int r12);
void bctr();
void mtctr_r2();
#endif //__ASSEMBLY

#define PATCH_HANDLER PatchHandler

#endif //_PATCHES_H_
