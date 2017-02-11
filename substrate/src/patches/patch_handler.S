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
	stw r0, 8(r1)
	//r1 = 105DC1B0
	//lr = 00807EB0
	stwu r1, -16(r1)

	/*	main_patch sticks the original LR in r2

		TODO this stores data from (-128 + r1) to (-8 + r1); yes?
		How fast is this? Is it better to store each register individually (r2-r13)?
	*/
	stmw r2, -128(r1)

	//Instruction right after main_patch
	mr r4, r0
	//r1 = 105DC1A0
	subi r1, r1, 124 //I heard you liked magic numbers
	//"pointer to registers" yeah let's go with that
	mr r3, r1
	subi r1, r1, 32 //Leave some breathing room

	bl private_generateFunctionContext
	bl private_dispatchCallbacksAndGetInstrunctions
	mr r0, r3
	//Instructions to execute are in r0

	/* TODO update registers based on callbacks */
	//r1 = 105DC108
	//Reset stack and reload registers
	addi r1, r1, 156 //124+32
	//r1 = 105DC1A0
	lmw r2, -128(r1)
	addi r1, r1, 16 //further wondorous trial/error

	//r2 has the original LR in it (thanks lmw!)
	mtlr r2

	/*	patcher.c sticks a "mtctr r2" in front of the original instructions
		(along with a "bctr" at the end)

		This instruction is usually lwz r0, 8(r1) for all you stack frame nerds
	*/
	//r1 = 0x105DC1B0
	lwz r2, 8(r1)
	//r2 = 0x00807EB0

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
