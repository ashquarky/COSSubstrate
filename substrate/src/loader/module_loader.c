/*	Cafe OS Substrate

	module_loader.c - Loads and manages modules.
	No partner file.

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

#include <UDynLoad.h>
#include <elf_abi.h>
#include <stdlib.h>
#include <string.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/mem_functions.h"
#include "loader/plt_resolve.h"
#include <substrate/substrate.h>

/*	TODO keep track of modules properly to allow removal and dynamic loading
*/

void relocateElf(void* elf, void* dynamic);

/*	Loads in a module from memory.
	I may or may not have copied this from the Installer.
*/
int COSSubstrate_LoadModuleRaw(void* module_tmp) {
	int res = UDynLoad_CheckELF(module_tmp);
	if (res != UDYNLOAD_ELF_OK) {
		return COSS_LMR_BAD_MODULE;
	}

	/* Get a pointer to the module's program header table */
	Elf32_Phdr* module_phdrs = ((Elf32_Ehdr*)module_tmp)->e_phoff + module_tmp;

	/*	Try to calculate how much memory we'll need.
		This could take some serious love from someone who knows
		what they're doing.
	*/
	unsigned int module_size = 0;
	int tmp = 0;
	/* For each program header... */
	for (int i = 0; i < ((Elf32_Ehdr*)module_tmp)->e_phnum; i++) {
		if(module_phdrs[i].p_type == PT_LOAD) {
			/* Get the difference between the destination address and our current size estimate */
			tmp = module_phdrs[i].p_vaddr - module_size;
			/*	This is problematic. If the difference between offset and p_vaddr
				is too big, the result will wrap into negatives, the section
				will be skipped, and likely sections past it as well.
				Hopefully we don't get an ELF this large.
			*/
			if (tmp < 0) continue;
			/*	If the destination address is larger than the current size estimate...
				Update the size estimate.
			*/
			module_size += tmp;
			module_size += module_phdrs[i].p_memsz;
		}
	}

	/* Allocate space for the actual module */
	void* module = MEMAllocFromExpHeapEx(COSS_MAIN_HEAP, module_size, 0x4);
	memset(module, 0, module_size);

	void* dynamic = 0;

	/* For each program header... */
	for (int i = 0; i < ((Elf32_Ehdr*)module_tmp)->e_phnum; i++) {
		if (module_phdrs[i].p_type == PT_LOAD) {
			/* Copy PT_LOAD headers into the destination */
			void* dst = (void*)(module + module_phdrs[i].p_vaddr);
			//return dst;
			memcpy(dst, module_tmp + module_phdrs[i].p_offset, module_phdrs[i].p_filesz);
		} else if (module_phdrs[i].p_type == PT_DYNAMIC) {
			/* Do the same for PT_DYNAMIC, but also take a note of where it ends up */
			memcpy(module + module_phdrs[i].p_vaddr, module_tmp + module_phdrs[i].p_offset, module_phdrs[i].p_filesz);
			dynamic = module + module_phdrs[i].p_vaddr;
		}
	}

	/* Apply everyone's favorite - ELF relocations! */
	relocateElf(module, dynamic);

	/* Flush everything to main memory and invalidate the instruction cache */
	DCFlushRange(module, module_size);
	ICInvalidateRange(module, module_size);

	int (*_start)();
	UDynLoad_FindExportDynamic(module, dynamic, "_start", (void**)&_start);
	_start();

	return COSS_LMR_OK;
}

unsigned int makeB(void* dst, void* src) {
	return (unsigned int)(((dst - src) & 0x03FFFFFC) | 0x48000000);
}

unsigned int makeLiR0(unsigned short val) {
	return (unsigned int)(0x38000000 | val);
}

/*	Relocates an elf. This took FOREVER to write.
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

	unsigned int plt_otable = 0;
	unsigned short cur_table_offset = 0;
	if (plt) {
		/* Get needed info about the PLT */
		plt_otable = (unsigned int)plt + 0x48; //Constant 0x48 bytes at start of PLT
		for (unsigned int i = 0; i < (jmprel_sz / sizeof(Elf32_Rela)); i++) {
			plt_otable += 8; //two instructions
		}
		/* Set up PLT patches... yay... */
		((unsigned int*)&plt_resolve)[2] |= (unsigned short)(((unsigned int)((unsigned int)plt + (PLT_TABLE_ADDR_INDEX * 4)) >> 16) & 0xFFFF);
		((unsigned int*)&plt_resolve)[3] |= (unsigned short)((unsigned int)((unsigned int)plt + (PLT_TABLE_ADDR_INDEX * 4)) & 0xFFFF);
		memcpy(plt, &plt_resolve, PLT_RESOLVE_SIZE);
		plt[PLT_TABLE_ADDR_INDEX] = (unsigned int)plt_otable;
	}

	/* 	Staggering hack; relocations with .text as a symbol seem to not work properly.
		Thus, I need to know the address of .text before relocating.
		Should be the second symbol; loop up to 10 just in case.
	*/
	unsigned int text_addr = 0;
	for (int i = 0; i < 10; i++) {
		if (ELF32_ST_TYPE(sym[i].st_info) == STT_SECTION) {
			text_addr = sym[i].st_value;
			break;
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
					break;
				}
				*((unsigned int*)(elf + rela[i].r_offset)) &= 0xFC000003;
				*((unsigned int*)(elf + rela[i].r_offset)) |= (unsigned int)(value & 0x03FFFFFC);
				break;
			}
			default: {
				break;
			}
		}
	}
}
