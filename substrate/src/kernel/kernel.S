/*	Cafe OS Substrate

	kernel.S - Miscellaneous kernel-related functions, such as kern_read/write.
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

/*	All kern functions need somewhere to keep the stack pointer.
	This is it.
*/
stack_pointer_save:
	.int 0x00000000

.section ".text"
.align 4

/*	My take on an imporoved kern_read.
	I tried to make it superscalar where I could, which makes it tough to read.
	Sorry.
	unsigned int kern_read(const void* addr)
*/
.globl kern_read
kern_read:
	mflr r0 //C calling convention
	mr r9, r3 //Move addr into r9
	stw r0, 8(r1)
	lis r4, stack_pointer_save@ha
	li r10, 0
	stw r1, stack_pointer_save@l(r4)

	//Syscall 0x2300 points to lwzx r3, r9, r10; rfi
	li r0, 0x2300
	sc
	nop

	lis r4, stack_pointer_save@ha
	lwz r1, stack_pointer_save@l(r4)

	lwz r0, 8(r1)
	mtlr r0

	blr

/*	My take on an imporoved kern_write.
	void kern_write(const void* addr, unsigned int val)
*/
.globl kern_write
kern_write:
	mflr r0 //C calling convention
	mr r9, r3 //Move addr into r9
	stw r0, 8(r1)
	mr r5, r4
	lis r4, stack_pointer_save@ha
	li r10, 0
	stw r1, stack_pointer_save@l(r4)

	//Syscall 0x2400 points to stwx r5, r9, r10; rfi
	li r0, 0x2400
	sc
	nop

	lis r4, stack_pointer_save@ha
	lwz r1, stack_pointer_save@l(r4)

	lwz r0, 8(r1)
	mtlr r0

	blr
