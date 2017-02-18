/*	Cafe OS Substrate Installer

	main.c - Main Installer logic. Does the actual installing.
	Paired with main.h.

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
#include "system/plt_resolve.h"

#include <substrate/substrate.h> /* /api/substrate/substrate.h */
#include <substrate/substrate_private.h>

#include <UDynLoad.h>
#include <elf_abi.h>

void relocateElf(void* elf, void* dynamic);

#define PATCH_VAL 3
void callback(COSSubstrate_FunctionContext* ctx) {
	ctx->args[0] = PATCH_VAL;
	log_printf("\nCallback! 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n\n", ctx->args[0], ctx->args[1], ctx->args[2], ctx->args[3], ctx->args[4]);
}

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

	log_init("192.168.192.37"); /* log_ functions in utils/logger.h */
	log_print("Hello World!\n");

	/* 	Install my improved kern_read/write.
		See kernel/kernel.h for more on that.
	*/
	InstallKernFunctions();

	/*	Setup memory mappings via BATs.
		Again, kernel/kernel.h has details.
	*/
	RunCodeAsKernel(&SetupBATs, 0);

	/*	Patch kernel to keep BATs safe.
	*/
	RunCodeAsKernel(&PatchKernelBATs, 0);

	/*	Set up memory heap.
		This is the heap that will be used for basically everything.
	*/
	memset((void*)COSS_MEM_BASE, 0, COSS_MEM_SIZE);
	int coss_heap = MEMCreateExpHeapEx((void*)COSS_MAIN_HEAP, COSS_MAIN_HEAP_SIZE, 0);

	/*	Load Substrate from SD */
	FILE* substrate_file = fopen("sd:/substrate.cosm", "rb"); //TODO change path

	/* Get Substrate filesize */
	fseek(substrate_file, 0L, SEEK_END);
	unsigned int substrate_file_size = ftell(substrate_file);
	fseek(substrate_file, 0L, SEEK_SET);

	/*	Allocate temporary Substrate location.
		This will be where the raw ELF is kept so we can do ELF loadery things.
	*/
	void* substrate_tmp = MEMAllocFromExpHeapEx(coss_heap, substrate_file_size, 0x4); //TODO try and allocate at end of heap
	if (!substrate_tmp) goto quit;
	fread(substrate_tmp, substrate_file_size, 1, substrate_file);
	log_printf("Read in Substrate! Allocated at 0x%08X.\n", substrate_tmp);

	/* Check Substrate validity */
	int res = UDynLoad_CheckELF(substrate_tmp);
	if (res != UDYNLOAD_ELF_OK) {
		log_printf("UDynLoad CheckELF error: %d\n", res);
		goto quit;
	}

	/* Get a pointer to the Substrate's program header table */
	Elf32_Phdr* substrate_phdrs = ((Elf32_Ehdr*)substrate_tmp)->e_phoff + substrate_tmp;

	/*	Try to calculate how much memory we'll need.
		This could take some serious love from someone who knows
		what they're doing.
	*/
	unsigned int substrate_size = 0;
	int tmp = 0;
	/* For each program header... */
	for (int i = 0; i < ((Elf32_Ehdr*)substrate_tmp)->e_phnum; i++) {
		if(substrate_phdrs[i].p_type == PT_LOAD) {
			/* Get the difference between the destination address and our current size estimate */
			tmp = substrate_phdrs[i].p_vaddr - substrate_size;
			/*	This is problematic. If the difference between offset and p_vaddr
				is too big, the result will wrap into negatives, the section
				will be skipped, and likely sections past it as well.
				Hopefully we don't get an ELF this large.
			*/
			if (tmp < 0) continue;
			/*	If the destination address is larger than the current size estimate...
				Update the size estimate.
			*/
			substrate_size += tmp;
			substrate_size += substrate_phdrs[i].p_memsz;
		}
	}

	/* Allocate space for the actual Substrate */
	void* substrate = MEMAllocFromExpHeapEx(coss_heap, substrate_size, 0x4);
	memset(substrate, 0, substrate_size);

	void* dynamic = 0;

	/* For each program header... */
	for (int i = 0; i < ((Elf32_Ehdr*)substrate_tmp)->e_phnum; i++) {
		if (substrate_phdrs[i].p_type == PT_LOAD) {
			/* Copy PT_LOAD headers into the destination */
			memcpy(substrate + substrate_phdrs[i].p_vaddr, substrate_tmp + substrate_phdrs[i].p_offset, substrate_phdrs[i].p_filesz);
		} else if (substrate_phdrs[i].p_type == PT_DYNAMIC) {
			/* Do the same for PT_DYNAMIC, but also take a note of where it ends up */
			memcpy(substrate + substrate_phdrs[i].p_vaddr, substrate_tmp + substrate_phdrs[i].p_offset, substrate_phdrs[i].p_filesz);
			dynamic = substrate + substrate_phdrs[i].p_vaddr;
		}
	}

	log_printf("Substrate loaded into 0x%08X\n\nRelocating...\n", substrate);

	/* Free the temporary file storage */
	MEMFreeToExpHeap(coss_heap, substrate_tmp);

	/* Apply everyone's favorite - ELF relocations! */
	relocateElf(substrate, dynamic);

	/* Flush everything to main memory and invalidate the instruction cache */
	DCFlushRange(substrate, substrate_size);
	ICInvalidateRange(substrate, substrate_size);

	/* Testing code from here on out */
	/*	Since we've followed the program headers, we have to use
		FindExportDynamic to get the function. The normal FindExport uses
		.symtab and .strtab, which didn't make it past the loading phase.
	*/
	res = UDynLoad_FindExportDynamic(substrate, dynamic, "_start", (void**)&_start);
	res = _start();
	log_printf("\n_start: 0x%08X\n", res);

	res = UDynLoad_FindExportDynamic(substrate, dynamic, "private_doSetup", (void**)&private_doSetup);
	private_doSetup(substrate, dynamic, OSDynLoad_Acquire, OSDynLoad_FindExport);
	log_printf("Did setup! substrate: 0x%08X\n", COSS_SPECIFICS->substrate);

	void (*COSSubstrate_PatchFunc)(void* func);
	res = UDynLoad_FindExportDynamic(substrate, dynamic, "COSSubstrate_PatchFunc", (void**)&COSSubstrate_PatchFunc);

	void (*COSSubstrate_RestoreFunc)(void* func);
	res = UDynLoad_FindExportDynamic(substrate, dynamic, "COSSubstrate_RestoreFunc", (void**)&COSSubstrate_RestoreFunc);

	UDynLoad_FindExportDynamic(substrate, dynamic, "debug_setCallback", (void**)&debug_setCallback);

	debug_setCallback(&callback);

	#define FUNC_TO_TRY ALongRoutine(1, 2);
	#define FUNC_TO_TRY_ADDR &ALongRoutine
	#define FUNC_TO_TRY_STR "ALongRoutine"

	unsigned int* t = (unsigned int*)FUNC_TO_TRY_ADDR;
	log_printf("pre-patch " FUNC_TO_TRY_STR ": %08X %08X %08X %08X %08X %08X\n", t[0], t[1], t[2], t[3], t[4], t[5]);
	COSSubstrate_PatchFunc(FUNC_TO_TRY_ADDR);
	log_printf(FUNC_TO_TRY_STR " = patched! %08X %08X %08X %08X %08X %08X\n", t[0], t[1], t[2], t[3], t[4], t[5]);

	DCFlushRange((FUNC_TO_TRY_ADDR - 0x100), 0x200);
	ICInvalidateRange((FUNC_TO_TRY_ADDR - 0x100), 0x200);

	int x = FUNC_TO_TRY;

	log_printf(FUNC_TO_TRY_STR " returned: 0x%08X\n", x);

	/* 	Funny story - if my Assembly properly followed calling convention, this
		function pointer would take us halfway through FUNC_TO_TRY.

		Good thing I stopped doing that.
	*/
	COSSubstrate_RestoreFunc(FUNC_TO_TRY_ADDR);
	log_printf(FUNC_TO_TRY_STR " = restored! %08X %08X %08X %08X %08X %08X\n", t[0], t[1], t[2], t[3], t[4], t[5]);
quit:
	fclose(substrate_file);

	log_print("Quitting...\n");
	log_print("------------------------------------\n\n\n");

	unmount_sd_fat("sd");
	log_deinit();

	return EXIT_SUCCESS;
}

unsigned int makeB(void* dst, void* src) {
	return (unsigned int)(((dst - src) & 0x03FFFFFC) | 0x48000000);
}

unsigned int makeLiR0(unsigned short val) {
	return (unsigned int)(0x38000000 | val);
}

/*	Relocates an elf. This took FOREVER to write.
	TODO move into dedicated file.
*/
void relocateElf(void* elf, void* dynamic) {
	Elf32_Dyn* dynamic_r = (Elf32_Dyn*)dynamic;
	Elf32_Rela* rela = 0;
	Elf32_Sym* sym = 0;
	unsigned int rela_sz = 0;
	unsigned int jmprel_sz = 0;
	unsigned int* plt = 0;

	/* Go find .dynamic, .rela.dyn, .rela.plt, .plt and a symbol table */
	for (int i = 0; dynamic_r[i].d_tag != DT_NULL; i++) {
		if (dynamic_r[i].d_tag == DT_RELA) {
			/* relocation table */
			rela = (Elf32_Rela*)(dynamic_r[i].d_un.d_ptr + elf);
		} else if (dynamic_r[i].d_tag == DT_RELASZ) {
			/* relocation table size */
			rela_sz = (unsigned int)dynamic_r[i].d_un.d_val;
		} else if (dynamic_r[i].d_tag == DT_SYMTAB) {
			/* symbol table */
			sym = (Elf32_Sym*)(dynamic_r[i].d_un.d_ptr + elf);
		} else if (dynamic_r[i].d_tag == DT_PLTGOT) {
			plt = (unsigned int*)(dynamic_r[i].d_un.d_ptr + elf);
		} else if (dynamic_r[i].d_tag == DT_PLTRELSZ) {
			jmprel_sz = (unsigned int)dynamic_r[i].d_un.d_val;
		}

		if (rela && rela_sz && sym && plt && jmprel_sz) break;
	}

	/* Get needed info about the PLT */
	unsigned int plt_otable = (unsigned int)plt + 0x48; //Constant 0x48 bytes at start of PLT
	for (unsigned int i = 0; i < (jmprel_sz / sizeof(Elf32_Rela)); i++) {
		plt_otable += 8; //two instructions
	}
	/* Set up PLT patches... yay... */
	((unsigned int*)&plt_resolve)[2] |= (unsigned short)(((unsigned int)((unsigned int)plt + (PLT_TABLE_ADDR_INDEX * 4)) >> 16) & 0xFFFF);
	((unsigned int*)&plt_resolve)[3] |= (unsigned short)((unsigned int)((unsigned int)plt + (PLT_TABLE_ADDR_INDEX * 4)) & 0xFFFF);
	memcpy(plt, &plt_resolve, PLT_RESOLVE_SIZE);
	plt[PLT_TABLE_ADDR_INDEX] = (unsigned int)plt_otable;
	unsigned short cur_table_offset = 0;

	/* 	Staggering hack; relocations with .text as a symbol seem to not work properly.
		Thus, I need to know the address of .text before relocating.
		Should be the second symbol; loop up to 10 just in case.
	*/
	unsigned int text_addr = 0;
	for (int i = 0; i < 10; i++) {
		if (ELF32_ST_TYPE(sym[i].st_info) == STT_SECTION) {
			text_addr = sym[i].st_value;
		}
	}
	#define IF_NOT_TEXT(x) ((x == text_addr) ? 0 : x)

	/* rela_sz is in bytes, divide out for proper indexing */
	rela_sz /= sizeof(Elf32_Rela); //TODO bitshift this for speed

	/* For each relocation... */
	for (unsigned int i = 0; i < rela_sz; i++) {
		/* See http://refspecs.linuxbase.org/elf/elfspec_ppc.pdf for info on each relocation. */
		switch (ELF32_R_TYPE(rela[i].r_info)) {
			case R_PPC_RELATIVE: {
				/* R_PPC_RELATIVE: Change (offset) to (base address) + (addend). */
				*((unsigned int*)(elf + rela[i].r_offset)) = (unsigned int)elf + rela[i].r_addend;
				break;
			}
			case R_PPC_ADDR32: {
				/* R_PPC_ADDR32: Change (offset) to (address of symbol at index ELF32_R_SYM(rela[i].r_info)) + (addend) */
				*((unsigned int*)(elf + rela[i].r_offset)) = (unsigned int)(elf + IF_NOT_TEXT(sym[ELF32_R_SYM(rela[i].r_info)].st_value) + rela[i].r_addend);
				break;
			}
			case R_PPC_JMP_SLOT: {
				/* R_PPC_JMP_SLOT: Change (offset) to a branch instruction going to (address of symbol at index ELF32_R_SYM(rela[i].r_info)) + (offset) */
				//TODO handle func/notype jumps outside the usual 8MB
				if (ELF32_ST_TYPE(sym[ELF32_R_SYM(rela[i].r_info)].st_info) == STT_FUNC || ELF32_ST_TYPE(sym[ELF32_R_SYM(rela[i].r_info)].st_info) == STT_NOTYPE) {
					*((unsigned int*)(elf + rela[i].r_offset)) = makeB(elf + IF_NOT_TEXT(sym[ELF32_R_SYM(rela[i].r_info)].st_value) + rela[i].r_addend, elf + rela[i].r_offset);
				} else if (ELF32_ST_TYPE(sym[ELF32_R_SYM(rela[i].r_info)].st_info) == STT_OBJECT) {
					/* Function pointer, woo-hoo? */
					((unsigned int*)(elf + rela[i].r_offset))[0] = makeLiR0(cur_table_offset);
					((unsigned int*)(elf + rela[i].r_offset))[1] = makeB(plt, elf + rela[i].r_offset);
					*((unsigned int*)(plt_otable + cur_table_offset)) = (unsigned int)(elf + IF_NOT_TEXT(sym[ELF32_R_SYM(rela[i].r_info)].st_value) + rela[i].r_addend);
					cur_table_offset += 4;
				}
				break;
			}
			case R_PPC_ADDR16_LO: {
				*((unsigned short*)(elf + rela[i].r_offset)) = (unsigned short)((unsigned int)(elf + IF_NOT_TEXT(sym[ELF32_R_SYM(rela[i].r_info)].st_value) + rela[i].r_addend) & 0xFFFF);
				break;
			}
			case R_PPC_ADDR16_HA: {
				//This doesn't seem to work quite right, despite using code straight from the ABI docs
				unsigned int x = (unsigned int)(elf + IF_NOT_TEXT(sym[ELF32_R_SYM(rela[i].r_info)].st_value) + rela[i].r_addend);
				*((unsigned short*)(elf + rela[i].r_offset)) = (unsigned short)(((x >> 16) + ((x & 0x8000) ? 1 : 0)) & 0xFFFF);
				break;
			}
			case R_PPC_ADDR16_HI: {
				*((unsigned short*)(elf + rela[i].r_offset)) = (unsigned short)(((unsigned int)(elf + IF_NOT_TEXT(sym[ELF32_R_SYM(rela[i].r_info)].st_value) + rela[i].r_addend) & 0xFFFF0000) >> 16);
				break;
			}
			case R_PPC_REL24: {
				unsigned int value = (unsigned int)((IF_NOT_TEXT(sym[ELF32_R_SYM(rela[i].r_info)].st_value) + rela[i].r_addend) - rela[i].r_offset);
				if ((value & 0xFC000000) && !(value & 0x80000000)) {
					log_printf("R_PPC_REL24 relocation number %d out of range: 0x%08X (target: 0x%08X)\n", i, value, rela[i].r_offset);
					break;
				}
				*((unsigned int*)(elf + rela[i].r_offset)) &= 0xFC000003;
				*((unsigned int*)(elf + rela[i].r_offset)) |= (unsigned int)(value & 0x03FFFFFC);
				break;
			}
			default: {
				log_printf("Unknown relocation type %d!\n", ELF32_R_TYPE(rela[i].r_info));
				break;
			}
		}
	}
}
