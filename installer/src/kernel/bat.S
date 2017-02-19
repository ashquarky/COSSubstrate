/*	Cafe OS Substrate Installer

	bat.S - Handles PowerPC memory mapping. See below for details.
	Paired with kernel.h.

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

#include "ppc_asm.h"

/*	Kernelmode code to set Data Block Address Translation (DBAT) registers.
	This essentially maps memory for us.
	|---------------------------------|
	|       |  Physical  | Effective  |
	| Start | 0x33800000 | 0x60000000 |
	| End   | 0x34000000 | 0x60800000 |
	|---------------------------------|

	ANY CHANGES MUST BE REFLECTED IN THE SUBSTRATE API HEADER

	_kernelmode_ void SetupBATs();
*/
.globl SetupBATs
SetupBATs:
	lis r3, 0x600000FF@h
	ori r3, r3, 0x600000FF@l
	mtspr 572, r3 //dbat6u
	mtspr 564, r3 //ibat6u

	lis r3, 0x33800012@h
	ori r3, r3, 0x33800012@l
	mtspr 573, r3 //dbat6l
	mtspr 565, r3 //ibat6l

	eieio
	isync

	rfi

/*	Clears our mappings, returning the console to the way it was before.

	_kernelmode_ void ClearBATs();
*/
.globl ClearBATs
ClearBATs:
	li r3, 0
	mtspr 572, r3 //dbat6u
	mtspr 564, r3 //ibat6u
	mtspr 573, r3 //dbat6l
	mtspr 565, r3 //ibat6l

	eieio
	isync

	rfi

/*	Patches the kernel so it won't delete all our BATs constantly.
	Saves original instructions somewhere not-really-safe.
	TODO move patch_nop_save

	_kernelmode_ void PatchKernelBATs();
*/
.globl PatchKernelBATs
PatchKernelBATs:
	lis r6, patch_nop_table@ha
	ori r6, r6, patch_nop_table@l
	lis r7, patch_nop_save@ha
	ori r7, r7, patch_nop_save@l

	lis r4, 0x6000 //NOP

	li r5, 0

	mfdbatu r9, 0
	mfdbatl r10, 0
	mfdbatu r11, 1
	mfdbatl r12, 1

	lis r3, 0xFFF0
	ori r3, r3, 0x0002
	mtdbatu 0, r3
	lis r3, 0xFFF0
	ori r3, r3, 0x0032
	mtdbatl 0, r3

	lis r3, 0xFFEE
    ori r3, r3, 0x0002
    mtdbatu 1, r3
    lis r3, 0xFFEE
    ori r3, r3, 0x0032
    mtdbatl 1, r3

	eieio
	isync

	PatchKernelBATs_loop:
		lwzx r3, r6, r5
		cmplwi r3, 0
		beq PatchKernelBATs_loopend

		lwz r8, 0(r3)
		stw r4, 0(r3)

		dcbf 0, r3
		icbi 0, r3

		stwx r8, r7, r5

		addi r5, r5, 4
		b PatchKernelBATs_loop
	PatchKernelBATs_loopend:

	mtdbatu 0, r9
	mtdbatl 0, r10
	mtdbatu 1, r11
	mtdbatl 1, r12

	eieio
	isync

	rfi

/*	Undoes the patching we did above.

	_kernelmode_ void RestoreKernelBATs();
*/
.globl RestoreKernelBATs
RestoreKernelBATs:
	lis r6, patch_nop_table@ha
	ori r6, r6, patch_nop_table@l
	lis r7, patch_nop_save@ha
	ori r7, r7, patch_nop_save@l

	li r5, 0

	mfdbatu r9, 0
	mfdbatl r10, 0
	mfdbatu r11, 1
	mfdbatl r12, 1

	lis r3, 0xFFF0
	ori r3, r3, 0x0002
	mtdbatu 0, r3
	lis r3, 0xFFF0
	ori r3, r3, 0x0032
	mtdbatl 0, r3

	lis r3, 0xFFEE
	ori r3, r3, 0x0002
	mtdbatu 1, r3
	lis r3, 0xFFEE
	ori r3, r3, 0x0032
	mtdbatl 1, r3

	eieio
	isync

	RestoreKernelBATs_loop:
		lwzx r3, r6, r5

		cmplwi r3, 0
		beq RestoreKernelBATs_loopend

		lwzx r4, r7, r5
		stw r4, 0(r3)

		dcbf 0, r3
		icbi 0, r3

		addi r5, r5, 4
		b PatchKernelBATs_loop
	RestoreKernelBATs_loopend:

	mtdbatu 0, r9
	mtdbatl 0, r10
	mtdbatu 1, r11
	mtdbatl 1, r12

	eieio
	isync

	rfi

.section ".rodata"
.align 4

#if (VER == 550)
patch_nop_table:
	.int 0xFFF1D5AC
	.int 0xFFF1D5B4
	.int 0xFFF06B74
	.int 0xFFF06B84
	.int 0xFFF06B90
	.int 0xFFF06C00
	.int 0xFFF06C10
	.int 0xFFF06C1C
	.int 0xFFF003D4
	.int 0xFFF003D8
	.int 0xFFF1D710
	.int 0xFFF1D830
	.int 0xFFEE11D4
	.int 0xFFEE11D8
	.int 0x00000000
#else
	#error Please set a firmware version.
#endif

.section ".data"
.align 4

#if (VER == 550)
patch_nop_save:
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
	.int 0
#else
	#error Please set a firmware version.
#endif

.section ".text"
.align 4
