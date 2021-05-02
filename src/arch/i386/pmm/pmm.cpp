
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "string.h"
#include "multiboot2.h"
#include "gdt.h"
#include "e820.h"
#include "stdio.h"
#include "pmm.h"

// TODO: 优化空间
// TODO: 换一种更灵活的方法
COMMON::physical_pages_t PMM::phy_pages[COMMON::PMM_PAGE_MAX_SIZE];
size_t                   PMM::normal_pages = 0;
FIRSTFIT                 PMM::normal(phy_pages);
size_t                   PMM::high_pages = 0;
FIRSTFIT                 PMM::high(&phy_pages[COMMON::KERNEL_PAGES]);
size_t                   PMM::pages                    = 0;
FIRSTFIT *               PMM::zone[COMMON::ZONE_COUNT] = {&normal, &high};

PMM::PMM(void) {
    return;
}

PMM::~PMM(void) {
    return;
}

int32_t PMM::init(void) {
    // 因为 GDT 是 x86 遗毒，所以在这里处理
    GDT::init();
// #define DEBUG
#ifdef DEBUG
    printf("KERNEL_START_4K: 0x%X, KERNEL_END_4K: 0x%X\n", KERNEL_START_4K,
           KERNEL_END_4K);
#endif
    e820map_t e820map;
    bzero(&e820map, sizeof(e820map_t));
    // TODO: 处理不能使用的内存，在虚拟内存映射时需要考虑
    MULTIBOOT2::get_e820(e820map);
    // 计算可用的内存
    // 这里需要保证 addr 是按照 PMM_PAGE_MASK 对齐的
    for (size_t i = 0; i < e820map.nr_map; i++) {
// #define DEBUG
#ifdef DEBUG
        printf("addr: 0x%X, len: 0x%X, type: 0x%X\n", e820map.map[i].addr,
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
    printf("pmm_init\n");
    return 0;
}

void PMM::mamage_init(void) {
    normal.init(normal_pages);
    high.init(high_pages);
    return;
}

void *PMM::alloc_page(uint32_t _pages, COMMON::zone_t _zone) {
    void *addr = zone[_zone]->alloc(_pages);
    if (addr == nullptr) {
        printf("No enough mem.\n");
    }
    return addr;
}

void PMM::free_page(void *_addr, uint32_t _pages, COMMON::zone_t _zone) {
    zone[_zone]->free(_addr, _pages);
    return;
}

uint32_t PMM::free_pages_count(COMMON::zone_t _zone) {
    return zone[_zone]->free_pages_count();
}
