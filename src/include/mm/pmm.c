
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pmm.c for MRNIU/SimpleKernel.

#include "mm/pmm.h"

// 物理页帧数组长度
static uint32_t phy_pages_count;

// 可用物理内存页起始地址
static uint32_t pmm_addr_start;

// 可用物理内存页结束地址
static uint32_t pmm_addr_end;


void pmm_init(struct multiboot_tag *tag) {
		multiboot_memory_map_entry_t * mmap;
		mmap = ((struct multiboot_tag_mmap *) tag)->entries;
		for (; (uint8_t *) mmap< (uint8_t *) tag + tag->size;
		     mmap = (multiboot_memory_map_entry_t *)((unsigned long) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size)) {
				// 如果是可用内存
				if ((unsigned) mmap->type == MULTIBOOT_MEMORY_AVAILABLE
				    && (unsigned) (mmap->addr & 0xffffffff) == 0x100000)
						// 把内核结束位置到结束位置的内存段，按页存储到页管理栈里
						// TODO
						;
		}
		printk_color(COL_INFO, "[INFO] ");
		printk ("pmm_init\n");
		return;
}
