/*	Cafe OS Substrate Installer

	kernel.h - Header for kernel-related Assembly functions.
	Paired with bat.S, get_exec.S and misc_kern.S.

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

#ifndef KERNEL_H
#define KERNEL_H
/*	Some functions end with an "rfi" or do other kernel-ly things.
	This means you absolutely cannot run them in userspace.
	the _kernelmode_ prefix should help ID them.
*/
#define _kernelmode_

//get_exec.S

/*	A simple system to run any code as kernel, via syscall 0x36.
	r3 can be used as an argument if you want, and the code can return a value.
*/
extern unsigned int RunCodeAsKernel(void(*codeToRun), unsigned int r3);

//misc_kern.S

/*	Installs some of the various kernelmode functions used by the installer and the Substrate.
*/
extern void InstallKernFunctions();

/*	My take on a better kern_read/kern_write combo. Makes use of a slightly different gadget.
	Two instructions instead of many.
*/
extern unsigned int kern_read(const void* addr);
extern void kern_write(const void* addr, unsigned int val);

/*	The kern_write used by most homebrew, rewritten slightly into raw Assembly.
	Unoptimised, slow, and all round not very good, but needed as an entrypoint.
*/
extern void old_kern_write(const void* addr, unsigned int val);

//bat.S

/*	Kernelmode code to set Block Address Translation (BAT) registers.
	This essentially maps memory for us.
	See bat.S for the mapping table.
	ANY CHANGES MUST BE REFLECTED IN THE SUBSTRATE API HEADER
*/
extern _kernelmode_ void SetupBATs();

/*	Clears our mappings, returning the console to the way it was before.
*/
extern _kernelmode_ void ClearBATs();

#endif //KERNEL_H
