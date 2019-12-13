
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// pmm.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "mem/pmm.h"
#include "assert.h"
#include "string.h"

// 物理页帧数组长度
static uint32_t phy_pages_count;

// 物理内存页面管理的栈
static ptr_t pmm_stack[PAGE_MAX_SIZE + 1];

// 物理内存管理的栈指针
static ptr_t pmm_stack_top;

void pmm_init() {
	for ( ; (uint8_t *) mmap_entries < (uint8_t *) mmap_tag + mmap_tag->size;
	      mmap_entries = (multiboot_memory_map_entry_t *)
	                     ( (unsigned long) mmap_entries +
	                       ( (struct multiboot_tag_mmap *) mmap_tag )->entry_size ) ) {
		// 如果是可用内存
		if ( (unsigned) mmap_entries->type == MULTIBOOT_MEMORY_AVAILABLE
		     && (unsigned) ( mmap_entries->addr & 0xffffffff ) == 0x100000 ) {
			// 把内核位置到结束位置的内存段，按页存储到页管理栈里
			ptr_t page_addr = ( mmap_entries->addr );
			uint32_t length = ( mmap_entries->len );
			while (page_addr < length && page_addr <= PMM_MAX_SIZE) {
				pmm_free_page(page_addr);
				page_addr += PMM_PAGE_SIZE;
				phy_pages_count++;
			}
		}
	}
	printk_info("pmm_init\n");
	printk_info ("phy_pages_count: %d\n", phy_pages_count);
	return;
}

ptr_t pmm_alloc_page(void) {
	assert(pmm_stack_top != 0);
	ptr_t page = pmm_stack[pmm_stack_top--];
	memset( (void*)page, 0, PMM_PAGE_SIZE );
	return page;
}

ptr_t pmm_alloc_pages(uint32_t size) {
	ptr_t page;
	do {
		assert(pmm_stack_top != 0);
		page = pmm_stack[pmm_stack_top--];
		size--;
	} while ((size/PMM_PAGE_SIZE) > 0);
	memset( (void*)page, 0, PMM_PAGE_SIZE * (size/PMM_PAGE_SIZE) );
	return page;
}

void pmm_free_page(ptr_t page) {
	assert(pmm_stack_top != PAGE_MAX_SIZE);
	pmm_stack[++pmm_stack_top] = page;
	return;
}

#ifdef __cplusplus
}
#endif
