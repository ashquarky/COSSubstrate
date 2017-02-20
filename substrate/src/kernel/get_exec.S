/*	Cafe OS Substrate Installer

	get_exec.S - Gains code execution in the PowerPC kernel via syscalls.
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

.set KERN_SYSCALL_TBL_1,	0xFFE84C70 // unknown
.set KERN_SYSCALL_TBL_2,	0xFFE85070 // works with games (us, Mii Maker)
.set KERN_SYSCALL_TBL_3,	0xFFE85470 // works with loader
.set KERN_SYSCALL_TBL_4,	0xFFEAAA60 // works with home menu
.set KERN_SYSCALL_TBL_5,	0xFFEAAE60 // works with browser (previously KERN_SYSCALL_TBL)

stack_pointer_save:
	.int 0x00000000

/*	A simple system to run any code as kernel, via syscall 0x36.
	r3 can be used as an argument if you want, and the code can return a value.
	unsigned int RunCodeAsKernel(void(*codeToRun), unsigned int r3, unsigned int r4)
*/
.globl RunCodeAsKernel
RunCodeAsKernel:
	mflr r0
	stw r0, 8(r1) //TODO - is this 4 or 8? Does it matter?
	stwu r1, -20(r1)

	stw r4, -4(r1)
	stw r5, -8(r1)

	mr r4, r3
	mr r11, r3
	lis r3, (KERN_SYSCALL_TBL_2 + 0xD8)@ha //0xD8 = 0x36 * 4
	ori r3, r3, (KERN_SYSCALL_TBL_2 + 0xD8)@l
	bl kern_write

	mr r4, r11
	lis r3, (KERN_SYSCALL_TBL_4 + 0xD8)@ha //0xD8 = 0x36 * 4
	ori r3, r3, (KERN_SYSCALL_TBL_4 + 0xD8)@l
	bl kern_write

	mr r4, r11
	lis r3, (KERN_SYSCALL_TBL_5 + 0xD8)@ha //0xD8 = 0x36 * 4
	ori r3, r3, (KERN_SYSCALL_TBL_5 + 0xD8)@l
	bl kern_write

	lis r4, stack_pointer_save@ha
	li r0, 0x3600
	stw r1, stack_pointer_save@l(r4)
	lwz r3, -4(r1)
	lwz r4, -8(r1)

	sc
	nop

	lis r4, stack_pointer_save@ha
	lwz r1, stack_pointer_save@l(r4)

	addi r1, r1, 20
	lwz r0, 8(r1)
	mtlr r0

	blr
