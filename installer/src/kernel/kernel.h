/*	Cafe OS Substrate
	This is the Installer.
	Licensed under MIT, visit https://github.com/QuarkTheAwesome/COSSubstrate for more details.
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
*/
extern _kernelmode_ void SetupBATs();

#define COSS_MEM_BASE 0x60000000
#define COSS_MEM_SIZE 0x00800000

/*	Clears our mappings, returning the console to the way it was before.
*/
extern _kernelmode_ void ClearBATs();

#endif //KERNEL_H
