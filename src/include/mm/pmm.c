
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pmm.c for MRNIU/SimpleKernel.

#include "mm/pmm.h"
#include "multiboot2.h"

void pmm_init() {
		uint64_t mmap_addr=mmap_entries->addr;
		uint64_t mmap_length=mmap_entries->len;
		uint64_t mmap_end=mmap_addr+mmap_length;
		for(uint64_t mmap = mmap_addr; mmap < mmap_end; mmap++) {
				// 如果是可用内存
				if (mmap_entries->type == MULTIBOOT_MEMORY_AVAILABLE
				    && (mmap_addr&0xffffffff) == 0x100000) {
						// 把内核结束位置到结束位置的内存段，按页存储到页管理栈里
						uint64_t page_addr = (mmap_addr&0xffffffff) + (uint64_t)(kernel_end - kernel_start);
						uint64_t length = (mmap_addr&0xffffffff) + (mmap_length&0xffffffff);

						// while (page_addr < length && page_addr <= PMM_MAX_SIZE) {
						//   pmm_free_page(page_addr);
						//   page_addr += PMM_PAGE_SIZE;
						//   phy_page_count++;
						// }
				}
		}
		printk("-----------------------\n");
		printk("%x, %x\n", phy_page_count, pmm_stack);
		printk("-----------------------\n");
}

uint64_t pmm_alloc_page(){
		if (pmm_alloc_page!=0)
				printk("out of memory!\n");
		uint64_t page = pmm_stack[pmm_stack_top--];
		return page;
}

void pmm_free_page(uint64_t p){
		pmm_stack[++pmm_stack_top] = p;
}
