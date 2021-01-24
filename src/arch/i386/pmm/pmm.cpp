
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "io.h"
#include "stdint.h"
#include "string.h"
#include "multiboot2.h"
#include "pmm.h"
#include "firstfit.h"
#include "assert.h"

// TODO: 优化空间
// TODO: 换一种更灵活的方法
physical_page_t phy_pages[PMM_PAGE_MAX_SIZE];

// 管理范围为内核结束到物理内存结束
PMM::PMM(void)
    : ff(FIRSTFIT(reinterpret_cast<uint8_t *>(ALIGN4K(KERNEL_END_ADDR)))) {
    // 向下取整
    kernel_start_align4k =
        reinterpret_cast<uint8_t *>(ALIGN4K(KERNEL_START_ADDR));
    // 向上取整
    kernel_end_align4k = reinterpret_cast<uint8_t *>(ALIGN4K(KERNEL_END_ADDR));
    return;
}

PMM::~PMM(void) {
    return;
}

int32_t PMM::init(void) {
// #define DEBUG
#ifdef DEBUG
    printk_debug("kernel_start_align4k: 0x%X, kernel_end_align4k: 0x%X\n",
                 kernel_start_align4k, kernel_end_align4k);
#endif
    e820map_t e820map;
    bzero(&e820map, sizeof(e820map_t));
    get_ram_info(&e820map);
    bzero(phy_pages, sizeof(physical_page_t) * PMM_PAGE_MAX_SIZE);
    // 用于记录可用内存总数
    uint32_t pages_count = 0;
    // 计算可用的内存
    // 这里需要保证 addr 是按照 PMM_PAGE_MASK 对齐的
    for (size_t i = 0; i < e820map.nr_map; i++) {
        for (uint8_t *addr = e820map.map[i].addr;
             addr < (e820map.map[i].addr + e820map.map[i].length);
             addr += PMM_PAGE_SIZE) {
            // 初始化可用内存段的物理页数组
            // 地址对应的物理页数组下标
            phy_pages[pages_count].addr = addr;
            // 内核已占用部分
            if (addr >= kernel_start_align4k && addr < kernel_end_align4k) {
                phy_pages[pages_count].ref = 1;
            }
            else {
                phy_pages[pages_count].ref = 0;
            }
            pages_count++;
        }
    }
    mamage_init(pages_count);
    io.printf("pmm_init\n");
    return 0;
}

// TODO: 太难看了也
// 这里的 addr 与 len 4k 对齐
void PMM::get_ram_info(e820map_t *e820map) {
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
            e820map->map[e820map->nr_map].addr =
                reinterpret_cast<uint8_t *>(mmap_entries->addr);
            e820map->map[e820map->nr_map].length = mmap_entries->len;
            e820map->map[e820map->nr_map].type   = mmap_entries->type;
            e820map->nr_map++;
        }
    }
    return;
}

void PMM::mamage_init(uint32_t pages) {
    ff.init(pages);
    return;
}

void *PMM::alloc_page(uint32_t pages) {
    void *page = ff.alloc(pages);
    return page;
}

void PMM::free_page(void *addr, uint32_t pages) {
    ff.free(addr, pages);
    return;
}

uint32_t PMM::free_pages_count(void) {
    return ff.free_pages_count();
}
