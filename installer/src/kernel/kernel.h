/*	Cafe OS Substrate
	This is the Installer.
	Licensed under MIT, visit https://github.com/QuarkTheAwesome/COSSubstrate for more details.
*/

#ifndef KERNEL_H
#define KERNEL_H

//misc_kern.S

/*	Installs some of the various kernelmode functions used by the installer and the Substrate.
*/
extern void InstallKernFunctions();

/*	My take on a better kern_read. Makes use of a slightly different gadget.
	Two instructions instead of many.
*/
extern unsigned int kern_read(const void* addr);

/*	The kern_write used by most homebrew, rewritten slightly into raw Assembly.
	Unoptimised, slow, and all round not very good, but needed as an entrypoint.
*/
extern void old_kern_write(const void* addr, unsigned int val);

#endif //KERNEL_H
