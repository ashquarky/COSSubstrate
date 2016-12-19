/*	Cafe OS Substrate
	This is the Installer.
	Licensed under MIT, visit https://github.com/QuarkTheAwesome/COSSubstrate for more details.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/mem_functions.h"
#include "fs/sd_fat_devoptab.h"
#include "utils/logger.h"
#include "utils/exception.h"

#include "kernel/kernel.h"

/*
*/
int Menu_Main() {
	/* Init ALL THE THINGS */
	InitOSFunctionPointers(); /* dynamic_libs/os_functions.h */
	InitSocketFunctionPointers(); /* dynamic_libs/socket_functions.h */
	InitFSFunctionPointers(); /* dynamic_libs/fs_functions.h */
	InstallExceptionHandler(); /* utils/exception.h */

	/*	Allows me to be lazy and use C FILE functions. */
	mount_sd_fat("sd"); /* fs/sd_fat_devoptab.h */

	log_init("192.168.192.36"); /* log_ functions in utils/logger.h */
	log_print("Hello World!\n");

	/* 	Install my improved kern_read/write.
		See kernel/kernel.h for more on that.
	*/
	InstallKernFunctions();

	/*	Setup memory mappings via BATs.
		Again, kernel/kernel.h has details.
	*/
	RunCodeAsKernel(&SetupBATs, 0);

	/*	Set up memory heap.
		This is the heap that will be used for basically everything.
	*/
	memset((void*)COSS_MEM_BASE, 0, COSS_MEM_SIZE);
	//TODO move the following magic numbers into a standard API header
	int coss_heap = MEMCreateExpHeapEx((void*)COSS_MEM_BASE + 0x100 /*Leave 0x100 bytes for... stuff*/, COSS_MEM_SIZE - 0x100, 0);

	/*	Load Substrate from SD */
	FILE* substrate = fopen("sd:/substrate.cosm", "rb"); //TODO change path

	/* Get Substrate filesize */
	fseek(substrate, 0L, SEEK_END);
	unsigned int substrate_size = ftell(substrate);
	fseek(substrate, 0L, SEEK_SET);

	/* Allocate and load Substrate */
	void* substrate_mem = MEMAllocFromExpHeapEx(coss_heap, substrate_size, 0x10);
	log_printf("Substrate allocated at 0x%08X\n", substrate_mem);
	if (!substrate_mem) goto quit;
	fread(substrate_mem, substrate_size, 1, substrate);
	log_printf("Read in Substrate! ELF magic: 0x%02X %c%c%c\n", ((char*)substrate_mem)[0], ((char*)substrate_mem)[1], ((char*)substrate_mem)[2], ((char*)substrate_mem)[3]);

quit:
	fclose(substrate);

	log_print("Quitting...\n");
	log_print("------------------------------------\n\n\n");

	unmount_sd_fat("sd");
	log_deinit();

	return EXIT_SUCCESS;
}
