
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// pmm.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "mem/pmm.h"
#include "assert.h"
#include "string.h"
#include "mem/firstfit.h"

// 物理页帧数组长度
static uint32_t phy_pages_count;

static const pmm_manage_t * pmm_manage  = &firstfit_manage;

// 从 GRUB 读取物理内存信息
static void pmm_get_ram_info(e820map_t * e820map);
void pmm_get_ram_info(e820map_t * e820map) {
	for( ; (uint8_t *)mmap_entries < (uint8_t *)mmap_tag + mmap_tag->size ;
	    mmap_entries = (multiboot_memory_map_entry_t *)( (uint32_t)mmap_entries
	    + ( (struct multiboot_tag_mmap *)mmap_tag)->entry_size) ) {
		// 如果是可用内存
		if( (unsigned)mmap_entries->type == MULTIBOOT_MEMORY_AVAILABLE
		    && (unsigned)(mmap_entries->addr & 0xFFFFFFFF) == 0x100000) {
			e820map->map[e820map->nr_map].addr = mmap_entries->addr;
			e820map->map[e820map->nr_map].length = mmap_entries->len;
			e820map->map[e820map->nr_map].type = mmap_entries->type;
			e820map->nr_map++;
		}
	}
	return;
}

void pmm_phy_init(e820map_t * e820map) {
	// 计算物理页总数
	for(uint32_t i = 0 ; i < e820map->nr_map ; i++) {
		for(uint64_t addr = e820map->map[i].addr ;
		    addr < e820map->map[i].addr + e820map->map[i].length ;
		    addr += PMM_PAGE_SIZE) {
			phy_pages_count++;
		}
	}
	return;
}

void pmm_mamage_init(e820map_t * e820map) {
	// 因为只有一个可用内存区域，所以直接传递
	pmm_manage->pmm_manage_init( (ptr_t)e820map->map[0].addr, phy_pages_count);
	return;
}

void pmm_init() {
	e820map_t e820map;
	bzero(&e820map, sizeof(e820map_t) );
	pmm_get_ram_info(&e820map);
	pmm_phy_init(&e820map);
	pmm_mamage_init(&e820map);

	printk_info("pmm_init\n");
	printk_info("phy_pages_count: %d\n", phy_pages_count);
	return;
}

ptr_t pmm_alloc(uint32_t byte) {
	ptr_t page;
	page = pmm_manage->pmm_manage_alloc(byte);
	return page;
}

void pmm_free_page(ptr_t addr UNUSED) {
	// pmm_manage->free(byte);
	return;
}

#ifdef __cplusplus
}
#endif
