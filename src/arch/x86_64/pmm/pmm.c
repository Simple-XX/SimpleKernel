
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "pmm.h"
#include "firstfit.h"

static const pmm_manage_t *pmm_manager = &firstfit_manage;

physical_page_t mem_page[PMM_PAGE_MAX_SIZE];

// TODO: 太难看了也
void pmm_get_ram_info(e820map_t *e820map) {
    for (; (uint8_t *)mmap_entries < (uint8_t *)mmap_tag + mmap_tag->size;
         mmap_entries =
             (multiboot_memory_map_entry_t
                  *)((uint32_t)mmap_entries +
                     ((struct multiboot_tag_mmap *)mmap_tag)->entry_size)) {
        // 如果是可用内存
        // printk("addr:%x%x,len:%x%x,type:%x:\n",mmap_entries->addr,mmap_entries->len,mmap_entries->type);
        if ((unsigned)mmap_entries->type == MULTIBOOT_MEMORY_AVAILABLE) {
            e820map->map[e820map->nr_map].addr   = mmap_entries->addr;
            e820map->map[e820map->nr_map].length = mmap_entries->len;
            e820map->map[e820map->nr_map].type   = mmap_entries->type;
            e820map->nr_map++;
        }
    }
    return;
}

void pmm_mamage_init(int pages) {
    // 因为只有一个可用内存区域，所以直接传递
    pmm_manager->pmm_manage_init(pages);
    return;
}

void pmm_init() {
    e820map_t e820map;
    bzero(&e820map, sizeof(e820map_t));
    pmm_get_ram_info(&e820map);
    bzero(mem_page, sizeof(physical_page_t) * PMM_PAGE_MAX_SIZE);
    // 用于记录可用内存总数
    int pages_count = 0;
    // 计算可用的内存
    for (size_t i = 0; i < e820map.nr_map; i++) {
        for (ptr_t addr = e820map.map[i].addr;
             addr < e820map.map[i].addr + e820map.map[i].length;
             addr += PMM_PAGE_SIZE) {
            // 初始化可用内存段的物理页数组
            // 地址对应的物理页数组下标
            size_t j         = (addr & PMM_PAGE_MASK) / PMM_PAGE_SIZE;
            mem_page[j].addr = addr;
            // 内核已占用部分
            if (addr >= (ptr_t)&kernel_start && addr <= ((ptr_t)&kernel_end)) {
                mem_page[j].ref = 1;
            }
            else {
                mem_page[j].ref = 0;
            }
            pages_count++;
        }
    }
    pmm_mamage_init(pages_count);
    printk_info("pmm_init\n");
    return;
}

ptr_t pmm_alloc(uint32_t byte) {
    ptr_t page;
    page = pmm_manager->pmm_manage_alloc(byte);
    return page;
}

ptr_t pmm_alloc_page(uint32_t pages) {
    ptr_t page;
    page = pmm_manager->pmm_manage_alloc(PMM_PAGE_SIZE * pages);
    return page;
}

void pmm_free(ptr_t addr, uint32_t byte) {
    pmm_manager->pmm_manage_free(addr, byte);
    return;
}

void pmm_free_page(ptr_t addr, uint32_t pages) {
    pmm_manager->pmm_manage_free(addr, pages * PMM_PAGE_SIZE);
    return;
}

uint32_t pmm_free_pages_count(void) {
    return pmm_manager->pmm_manage_free_pages_count();
}

#ifdef __cplusplus
}
#endif
