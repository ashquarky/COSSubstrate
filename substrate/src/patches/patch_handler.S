/*	Cafe OS Substrate Installer

	patch_handler.S - Code branched to by main_patch.
	Paired with patches.h.

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

#define __ASSEMBLY
#include <substrate/substrate.h>
#undef __ASSEMBLY

.section ".text"
.align 4

/*	Main patch handler. All patched functions redirect here.
	Saves all registers and branches out to our C functions to generate contexts
	and dispatch the callbacks.
*/
.globl PatchHandler
PatchHandler:
	mflr r0
	/*	https://twitter.com/quarktheawesome/status/832508820605710336
		https://twitter.com/quarktheawesome/status/832509144741531649
		https://twitter.com/quarktheawesome/status/832516570412834816
		https://twitter.com/quarktheawesome/status/832516809811128322
		Thus, this is perfectly fine in my new calling convention, designed just
		to deal with GCC's weirdness.
	*/
	stw r0, -8(r1)
	stwu r1, -16(r1)

	/* main_patch sticks the original LR in r2 */
	stw r2, -0x80(r1)
	stw r3, -0x7C(r1)
	stw r4, -0x78(r1)
	stw r5, -0x74(r1)
	stw r6, -0x70(r1)
	stw r7, -0x6C(r1)
	stw r8, -0x68(r1)
	stw r9, -0x64(r1)
	stw r10, -0x60(r1)
	stw r11, -0x5C(r1)
	stw r12, -0x58(r1)
	stw r13, -0x54(r1)

	//Instruction right after main_patch
	mr r4, r0
	subi r1, r1, 124 //I heard you liked magic numbers
	//"pointer to registers" yeah let's go with that
	mr r3, r1
	subi r1, r1, 32 //Leave some breathing room

	bl private_generateFunctionContext
	bl private_dispatchCallbacksAndGetInstrunctions
	mr r0, r3
	//Instructions to execute are in r0

	/*	Reset stack and reload registers, which may or may not have been
		modified by our callbacks
	*/
	addi r1, r1, 156 //124+32

	lwz r2, -0x80(r1)
	lwz r3, -0x7C(r1)
	lwz r4, -0x78(r1)
	lwz r5, -0x74(r1)
	lwz r6, -0x70(r1)
	lwz r7, -0x6C(r1)
	lwz r8, -0x68(r1)
	lwz r9, -0x64(r1)
	lwz r10, -0x60(r1)
	lwz r11, -0x5C(r1)
	lwz r12, -0x58(r1)
	lwz r13, -0x54(r1)

	addi r1, r1, 16 //further wondorous trial/error

	//r2 has the original LR in it
	mtlr r2

	/*	patcher.c sticks a "mtctr r2" in front of the original instructions
		(along with a "bctr" at the end)

		This instruction is usually lwz r0, 8(r1) for all you stack frame nerds
	*/
	lwz r2, -8(r1)

	//Address returned by private_dispatchCallbacksAndGetInstrunctions
	mtctr r0
	bctr

//Just some randoms to help out PatchedFunction generation
.globl bctr
bctr:
	bctr

.globl mtctr_r2
mtctr_r2:
	mtctr r2
