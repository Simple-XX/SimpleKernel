
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

void vmm_init(){



	return;
}



#ifdef __cplusplus
}
#endif
