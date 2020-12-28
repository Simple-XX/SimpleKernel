
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
#include "multiboot2.h"
#include "pmm.h"
#include "firstfit.h"

addr_t kernel_start_align4k = 0x00;
addr_t kernel_end_align4k   = 0x00;

static const pmm_manage_t *pmm_manager = &firstfit_manage;

// TODO: 优化空间
// TODO: 换一种更灵活的方法
physical_page_t mem_page[PMM_PAGE_MAX_SIZE];

// TODO: 太难看了也
// 这里的 addr 与 len 4k 对齐
void pmm_get_ram_info(e820map_t *e820map) {
    for (; (uint8_t *)mmap_entries < (uint8_t *)mmap_tag + mmap_tag->size;
         mmap_entries =
             (multiboot_memory_map_entry_t
                  *)((uint32_t)mmap_entries +
                     ((struct multiboot_tag_mmap *)mmap_tag)->entry_size)) {
        // 如果是可用内存
// #define DEBUG
#ifdef DEBUG
        printk_debug("addr: 0x%X, len: 0x%X, type: 0x%X\n",
                     (addr_t)mmap_entries->addr, (addr_t)mmap_entries->len,
                     (addr_t)mmap_entries->type);
#endif

        if (mmap_entries->type == MULTIBOOT_MEMORY_AVAILABLE) {
            e820map->map[e820map->nr_map].addr   = mmap_entries->addr;
            e820map->map[e820map->nr_map].length = mmap_entries->len;
            e820map->map[e820map->nr_map].type   = mmap_entries->type;
            e820map->nr_map++;
        }
    }
    return;
}

void pmm_mamage_init(uint32_t pages) {
    // 因为只有一个可用内存区域，所以直接传递
    pmm_manager->pmm_manage_init(pages);
    return;
}

void pmm_init() {
    // 向下取整
    kernel_start_align4k = ALIGN4K((addr_t)&kernel_start);
    // 向上取整
    kernel_end_align4k = ALIGN4K((addr_t)&kernel_end);
// #define DEBUG
#ifdef DEBUG
    printk_debug("kernel_start_align4k: %X, kernel_end_align4k: %X\n",
                 kernel_start_align4k, kernel_end_align4k);
#endif
    e820map_t e820map;
    bzero(&e820map, sizeof(e820map_t));
    pmm_get_ram_info(&e820map);
    bzero(mem_page, sizeof(physical_page_t) * PMM_PAGE_MAX_SIZE);
    // 用于记录可用内存总数
    uint32_t pages_count = 0;
    // 计算可用的内存
    // 这里需要保证 addr 是按照 PMM_PAGE_MASK 对齐的
    for (size_t i = 0; i < e820map.nr_map; i++) {
        for (addr_t addr = (addr_t)e820map.map[i].addr;
             addr < e820map.map[i].addr + e820map.map[i].length;
             addr += PMM_PAGE_SIZE) {
            // 初始化可用内存段的物理页数组
            // 地址对应的物理页数组下标
            mem_page[pages_count].addr = addr;
            // 内核已占用部分
            if (addr >= kernel_start_align4k && addr < kernel_end_align4k) {
                mem_page[pages_count].ref = 1;
            }
            else {
                mem_page[pages_count].ref = 0;
            }
            pages_count++;
        }
    }
    pmm_mamage_init(pages_count);
    printk_info("pmm_init\n");
    return;
}

addr_t pmm_alloc_page(uint32_t pages) {
    addr_t page;
    page = pmm_manager->pmm_manage_alloc(pages);
    return page;
}

void pmm_free_page(addr_t addr, uint32_t pages) {
    pmm_manager->pmm_manage_free(addr, pages);
    return;
}

uint32_t pmm_free_pages_count(void) {
    return pmm_manager->pmm_manage_free_pages_count();
}

#ifdef __cplusplus
}
#endif
