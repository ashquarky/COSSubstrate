/*	Cafe OS Substrate
	This is the Installer.
	Licensed under MIT, visit https://github.com/QuarkTheAwesome/COSSubstrate for more details.
*/

#include "ppc_asm.h"

.set KERN_SYSCALL_TBL_1,	0xFFE84C70 // unknown
.set KERN_SYSCALL_TBL_2,	0xFFE85070 // works with games (us, Mii Maker)
.set KERN_SYSCALL_TBL_3,	0xFFE85470 // works with loader
.set KERN_SYSCALL_TBL_4,	0xFFEAAA60 // works with home menu
.set KERN_SYSCALL_TBL_5,	0xFFEAAE60 // works with browser (previously KERN_SYSCALL_TBL)

.set IMPROVED_KERN_READ,	0xFFF02044 //lwzx r3, r9, r10; rfi

.globl InstallKernFunctions
InstallKernFunctions:
	mflr r0
	stw r0, 4(r1) //TODO - is this 4 or 8? Does it matter?
	stwu r1, -16(r1)

	//Inject syscall 0x2300 (improved_kern_read)
	lis r3, (KERN_SYSCALL_TBL_2 + 0x8C)@ha //
	ori r3, r3, (KERN_SYSCALL_TBL_2 + 0x8C)@l

	lis r4, IMPROVED_KERN_READ@ha
	ori r4, r4, IMPROVED_KERN_READ@l

	bl old_kern_write

	addi r1, r1, 16
	lwz r0, 4(r1)
	mtlr r0

	blr

/*	All kern functions need somewhere to keep the stack pointer.
	This is it.
*/
stack_pointer_save:
	.int

/*	My take on an imporoved kern_read.
	I tried to make it superscalar where I could, which makes it tough to read.
	Sorry.
	unsigned int kern_read(const void* addr)
*/
.globl kern_read
kern_read:
	mflr r0
	mr r9, r3
	stw r0, 8(r1)
	lis r4, stack_pointer_save@ha
	li r10, 0
	stw r1, stack_pointer_save@l(r4)

	li r0, 0x2300
	sc
	nop

	lis r4, stack_pointer_save@ha
	lwz r1, stack_pointer_save@l(r4)

	lwz r0, 8(r1)
	mtlr r0

	blr

/*	Unoptimised, horrific kern_write. The syscall itself isn't much good either.
	void old_kern_write(const void* addr, unsigned int val)
*/
.globl old_kern_write
old_kern_write:
	mflr r0
	stw r0, 8(r1)

	mr r5, r4
	mr r9, r3

	li r3, 1
	li r4, 0
	li r6, 0
	li r7, 0
	lis r8, 1

	lis r10, stack_pointer_save@ha
	stw r1, stack_pointer_save@l(r10)

	li r0, 0x3500
	sc
	nop

	lis r10, stack_pointer_save@ha
	lwz r1, stack_pointer_save@l(r10)

	lwz r0, 8(r1)
	mtlr r0

	blr
