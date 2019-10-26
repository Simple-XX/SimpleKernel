
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// bootinit.c for MRNIU/SimpleKernel.

#include "include/multiboot2.h"

/* This function gets called by the bootloader */
void bootinit(const void* multiboot_struct) {
	// Set up GDT

	// const multiboot_info_t* mb_info = multiboot_struct;            /* Make pointer to multiboot_info_t struct */
	// multiboot_uint32_t mb_flags = mb_info->flags;                  /* Get flags from mb_info */
	//
	// void* kentry = NULL;                                           /* Pointer to the kernel entry point */
	//
	// if (mb_flags & MULTIBOOT_INFO_MODS) {                          /* Check if modules are available */
	// 	multiboot_uint32_t mods_count = mb_info->mods_count;   /* Get the amount of modules available */
	// 	multiboot_uint32_t mods_addr = mb_info->mods_addr;     /* And the starting address of the modules */
	//
	// 	for (uint32_t mod = 0; mod < mods_count; mod++) {
	// 		multiboot_module_t* module = (multiboot_module_t*)(mods_addr + (mod * sizeof(multiboot_module_t)));     /* Loop through all modules */
	// 	}
	// }
}
