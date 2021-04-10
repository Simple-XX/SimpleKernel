
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "string.h"
#include "multiboot2.h"
#include "pmm.h"

extern MULTIBOOT2::multiboot_memory_map_entry_t *mmap_entries;
extern MULTIBOOT2::multiboot_mmap_tag_t *        mmap_tag;

// TODO: 优化空间
// TODO: 换一种更灵活的方法
IO                       PMM::io;
COMMON::physical_pages_t PMM::phy_pages[COMMON::PMM_PAGE_MAX_SIZE];
FIRSTFIT                 PMM::manage(phy_pages);
const char *             PMM::name = "FirstFit";

PMM::PMM(void) {
    return;
}

PMM::~PMM(void) {
    return;
}

// TODO: 太难看了也
// 这里的 addr 与 len 4k 对齐
void PMM::get_ram_info(e820map_t *e820map) {
    for (; (uint8_t *)MULTIBOOT2::mmap_entries <
           (uint8_t *)MULTIBOOT2::mmap_tag + MULTIBOOT2::mmap_tag->size;
         MULTIBOOT2::mmap_entries =
             (MULTIBOOT2::multiboot_memory_map_entry_t
                  *)((uint32_t)MULTIBOOT2::mmap_entries +
                     ((struct MULTIBOOT2::multiboot_tag_mmap *)
                          MULTIBOOT2::mmap_tag)
                         ->entry_size)) {
        // 如果是可用内存
// #define DEBUG
#ifdef DEBUG
        printk_debug("addr: 0x%X, len: 0x%X, type: 0x%X\n", mmap_entries->addr,
                     mmap_entries->len, mmap_entries->type);
#endif

        if (MULTIBOOT2::mmap_entries->type ==
            MULTIBOOT2::MULTIBOOT_MEMORY_AVAILABLE) {
            e820map->map[e820map->nr_map].addr =
                reinterpret_cast<uint8_t *>(MULTIBOOT2::mmap_entries->addr);
            e820map->map[e820map->nr_map].length =
                MULTIBOOT2::mmap_entries->len;
            e820map->map[e820map->nr_map].type = MULTIBOOT2::mmap_entries->type;
            e820map->nr_map++;
        }
    }
    return;
}

void PMM::mamage_init(uint32_t pages) {
    manage.init(pages);
    return;
}

int32_t PMM::init(void) {
// #define DEBUG
#ifdef DEBUG
    io.printf("KERNEL_START_4K: 0x%X, KERNEL_END_4K: 0x%X\n", KERNEL_START_4K,
              KERNEL_END_4K);
#endif
    e820map_t e820map;
    bzero(&e820map, sizeof(e820map_t));
    get_ram_info(&e820map);
    // 用于记录可用内存总数
    uint32_t pages_count = 0;
    // 计算可用的内存
    // 这里需要保证 addr 是按照 PMM_PAGE_MASK 对齐的
    for (size_t i = 0; i < e820map.nr_map; i++) {
        for (uint8_t *addr = e820map.map[i].addr;
             addr < (e820map.map[i].addr + e820map.map[i].length);
             addr += COMMON::PAGE_SIZE) {
            // 初始化可用内存段的物理页数组
            // 地址对应的物理页数组下标
            // 跳过 0x00 开始的一页，便于判断 nullptr
            if (addr == nullptr) {
                continue;
            }
            phy_pages[pages_count].addr = addr;
            phy_pages[pages_count].ref  = 0;
            pages_count++;
        }
    }
    mamage_init(pages_count);
    io.printf("pmm_init\n");
    return 0;
}

void *PMM::alloc_page(uint32_t _pages, COMMON::zone_t _zone) {
    return manage.alloc(_pages);
}

void PMM::free_page(void *_addr, uint32_t _pages) {
    manage.free(_addr, _pages);
    return;
}

uint32_t PMM::free_pages_count(void) {
    return manage.free_pages_count();
}
