
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// vmm.c for MRNIU/SimpleKernel.

#include "mm/include/vmm.h"
#include "include/debug.h"
#include "stdio.h"


#ifdef __cplusplus
extern "C" {
#endif

// When writing a higher-half kernel, the steps required are:
// 1. Reserve some pages for the initial mappings, so it is possible to parse GRUB structures before memory management is enabled.
// 2. Create page tables that contain the page frames of the kernel image.
// 3. Create the boot page directory that contains the aforementioned page tables both in lower half and higher half.
// 4. Enable paging.
// 5. Jump to higher half.
// 6. Remove the lower half kernel mapping.

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

void vmm_init() {

	for(int i = 0; i < 1024; i++) {
		page_directory[i] = PAGE_WRITE;
	}
	// holds the physical address where we want to start mapping these pages to.
	// in this case, we want to map these pages to the very beginning of memory.

	//we will fill all 1024 entries in the table, mapping 4 megabytes
	for(int i = 0; i < 1024; i++) {
		// As the address is page aligned, it will always leave 12 bits zeroed.
		// Those bits are used by the attributes ;)
		first_page_table[i] = (i * PAGE_SIZE) | PAGE_WRITE | PAGE_PRESENT; // attributes: supervisor level, read/write, present.
	}

	// attributes: supervisor level, read/write, present
	page_directory[0] = ((uint32_t)first_page_table) | PAGE_WRITE | PAGE_PRESENT;

	// enable_page(page_directory);


	printk_color(COL_INFO, "[INFO] ");
	printk("vmm_init\n");

	return;
}



#ifdef __cplusplus
}
#endif
