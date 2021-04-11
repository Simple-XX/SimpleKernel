
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

<<<<<<< HEAD
#include "io.h"
#include "assert.h"
#include "stdint.h"
#include "string.h"
#include "multiboot2.h"
#include "pmm.h"
#include "firstfit.h"

// TODO: 优化空间
// TODO: 换一种更灵活的方法
physical_page_t phy_pages[PMM_PAGE_MAX_SIZE];

// 管理范围为内核结束到物理内存结束
PMM::PMM(void)
    : ff(FIRSTFIT(reinterpret_cast<uint8_t *>(ALIGN4K(KERNEL_END_ADDR)))) {
    name = "FirstFit";
    kernel_start_align4k =
        reinterpret_cast<uint8_t *>(ALIGN4K(KERNEL_START_ADDR));
    kernel_end_align4k = reinterpret_cast<uint8_t *>(ALIGN4K(KERNEL_END_ADDR));
=======
#include "string.h"
#include "multiboot2.h"
#include "gdt.h"
#include "pmm.h"

extern MULTIBOOT2::multiboot_memory_map_entry_t *mmap_entries;
extern MULTIBOOT2::multiboot_mmap_tag_t *        mmap_tag;

// TODO: 优化空间
// TODO: 换一种更灵活的方法
IO                       PMM::io;
COMMON::physical_pages_t PMM::phy_pages[COMMON::PMM_PAGE_MAX_SIZE];
size_t                   PMM::normal_pages = 0;
FIRSTFIT                 PMM::normal(phy_pages);
size_t                   PMM::high_pages = 0;
FIRSTFIT                 PMM::high(&phy_pages[COMMON::KERNEL_PAGES]);
size_t                   PMM::pages                    = 0;
FIRSTFIT *               PMM::zone[COMMON::ZONE_COUNT] = {&normal, &high};

PMM::PMM(void) {
>>>>>>> intr
    return;
}

PMM::~PMM(void) {
    return;
}

<<<<<<< HEAD
int32_t PMM::init(void) {
// #define DEBUG
#ifdef DEBUG
    printk_debug("kernel_start_align4k: 0x%X, kernel_end_align4k: 0x%X\n",
                 kernel_start_align4k, kernel_end_align4k);
=======
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

void PMM::mamage_init(void) {
    normal.init(normal_pages);
    high.init(high_pages);
    return;
}

int32_t PMM::init(void) {
    // 因为 GDT 是 x86 遗毒，所以在这里处理
    GDT::init();
// #define DEBUG
#ifdef DEBUG
    io.printf("KERNEL_START_4K: 0x%X, KERNEL_END_4K: 0x%X\n", KERNEL_START_4K,
              KERNEL_END_4K);
>>>>>>> intr
#endif
    e820map_t e820map;
    bzero(&e820map, sizeof(e820map_t));
    get_ram_info(&e820map);
<<<<<<< HEAD
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
=======
    // 计算可用的内存
    // 这里需要保证 addr 是按照 PMM_PAGE_MASK 对齐的
    for (size_t i = 0; i < e820map.nr_map; i++) {
// #define DEBUG
#ifdef DEBUG
        io.printf("addr: 0x%X, len: 0x%X, type: 0x%X\n", e820map.map[i].addr,
                  e820map.map[i].length, e820map.map[i].type);
#undef DEBUG
#endif
        for (uint8_t *addr = e820map.map[i].addr;
             addr < (e820map.map[i].addr + e820map.map[i].length);
             addr += COMMON::PAGE_SIZE) {
            // 跳过 0x00 开始的一页，便于判断 nullptr
            if (addr == nullptr) {
                continue;
            }
            // 初始化可用内存段的物理页数组
            // 地址对应的物理页数组下标
            phy_pages[pages].addr = addr;
            // 内核已使用部分
            if (addr >= COMMON::KERNEL_START_4K &&
                addr < COMMON::KERNEL_END_4K) {
                phy_pages[pages].ref = 1;
            }
            else {
                phy_pages[pages].ref = 0;
            }
            pages++;
        }
    }
    // 计算各个分区大小
    normal_pages = COMMON::KERNEL_PAGES;
    high_pages   = pages - normal_pages;
    mamage_init();
>>>>>>> intr
    io.printf("pmm_init\n");
    return 0;
}

<<<<<<< HEAD
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
=======
void *PMM::alloc_page(uint32_t _pages, COMMON::zone_t _zone) {
    void *addr = zone[_zone]->alloc(_pages);
    if (addr == nullptr) {
        io.printf("No enough mem.\n");
    }
    return addr;
}

void PMM::free_page(void *_addr, uint32_t _pages, COMMON::zone_t _zone) {
    zone[_zone]->free(_addr, _pages);
    return;
}

uint32_t PMM::free_pages_count(COMMON::zone_t _zone) {
    return zone[_zone]->free_pages_count();
>>>>>>> intr
}
