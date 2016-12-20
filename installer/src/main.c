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

#include <UDynLoad.h>
#include <elf_abi.h>

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
	int coss_heap = MEMCreateExpHeapEx((void*)COSS_MEM_BASE + 0x200 /*Leave 0x200 bytes for... stuff*/, COSS_MEM_SIZE - 0x200, 0);

	/*	Load Substrate from SD */
	FILE* substrate_file = fopen("sd:/substrate.cosm", "rb"); //TODO change path

	/* Get Substrate filesize */
	fseek(substrate_file, 0L, SEEK_END);
	unsigned int substrate_file_size = ftell(substrate_file);
	fseek(substrate_file, 0L, SEEK_SET);

	/* Allocate and load Substrate */
	void* substrate_tmp = MEMAllocFromExpHeapEx(coss_heap, substrate_file_size, 0x10);
	log_printf("Substrate allocated at 0x%08X\n", substrate_tmp);
	if (!substrate_tmp) goto quit;
	fread(substrate_tmp, substrate_file_size, 1, substrate_file);
	log_printf("Read in Substrate! ELF magic: 0x%02X %c%c%c\n", ((char*)substrate_tmp)[0], ((char*)substrate_tmp)[1], ((char*)substrate_tmp)[2], ((char*)substrate_tmp)[3]);

	/* Check Substrate validity */
	int res = UDynLoad_CheckELF(substrate_tmp);
	if (res != UDYNLOAD_ELF_OK) {
		log_printf("UDynLoad CheckELF error: %d\n", res);
		goto quit;
	}

	/* Do program headery things */
	Elf32_Phdr* substrate_phdrs = ((Elf32_Ehdr*)substrate_tmp)->e_phoff + substrate_tmp;
	log_printf("Substrate program header table at 0x%08X\n", substrate_phdrs);
	/*	Try to calculate how much memory we'll need
		This could take some serious love from someone who knows
		what they're doing.
	*/
	unsigned int substrate_size = 0;
	int tmp = 0;
	for (int i = 0; i < ((Elf32_Ehdr*)substrate_tmp)->e_phnum; i++) {
		if(substrate_phdrs[i].p_type == PT_LOAD) {
			tmp = substrate_phdrs[i].p_vaddr - substrate_size;
			/*	This is problematic. If the difference between offset and p_vaddr
				is too big, the section will be skipped, and likely sections past
				it as well.
				Hopefully we don't get an ELF this large.
			*/
			if (tmp < 0) continue;

			substrate_size += tmp;
			substrate_size += substrate_phdrs[i].p_memsz;
		}
	}

	log_printf("Size of loaded application: 0x%08X\n", substrate_size);

	void* substrate = MEMAllocFromExpHeapEx(coss_heap, substrate_size, 0x10);
	memset(substrate, 0, substrate_size);

	void* dynamic = 0;

	for (int i = 0; i < ((Elf32_Ehdr*)substrate_tmp)->e_phnum; i++) {
		if (substrate_phdrs[i].p_type == PT_LOAD) {
			memcpy(substrate + substrate_phdrs[i].p_vaddr, substrate_tmp + substrate_phdrs[i].p_offset, substrate_phdrs[i].p_filesz);
		} else if (substrate_phdrs[i].p_type == PT_DYNAMIC) {
			memcpy(substrate + substrate_phdrs[i].p_vaddr, substrate_tmp + substrate_phdrs[i].p_offset, substrate_phdrs[i].p_filesz);
			dynamic = substrate + substrate_phdrs[i].p_vaddr;
		}
	}

	log_printf("Substrate loaded into 0x%08X\n", substrate);

	DCFlushRange(substrate, substrate_size);
	ICInvalidateRange(substrate, substrate_size);

	/* Testing code from here on out */
	int (*_start)();
	res = UDynLoad_FindExportDynamic(substrate, dynamic, "_start", (void**)&_start);
	log_printf("FindExport: 0x%08X, %d\n", _start, res);
	res = _start();
	log_printf("_start: 0x%08X\n", res);

quit:
	fclose(substrate_file);

	log_print("Quitting...\n");
	log_print("------------------------------------\n\n\n");

	unmount_sd_fat("sd");
	log_deinit();

	return EXIT_SUCCESS;
}
