
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "string.h"
#include "memlayout.h"
#include "stdio.h"
#include "pmm.h"

// TODO: 优化空间
// TODO: 换一种更灵活的方法
COMMON::physical_pages_t PMM::phy_pages[COMMON::PMM_PAGE_MAX_SIZE];
size_t                   PMM::normal_pages = 0;
FIRSTFIT                 PMM::normal(phy_pages);
// FIRSTFIT  PMM::normal((COMMON::physical_pages_t *)0x23);
size_t    PMM::high_pages = 0;
FIRSTFIT  PMM::high(&phy_pages[COMMON::KERNEL_PAGES]);
size_t    PMM::pages                    = 0;
FIRSTFIT *PMM::zone[COMMON::ZONE_COUNT] = {&normal, &high};

PMM::PMM(void) {
    return;
}

PMM::~PMM(void) {
    return;
}

int32_t PMM::init(void) {
    for (uint8_t *addr = (uint8_t *)COMMON::KERNEL_END_4K;
         addr < (uint8_t *)MEMLAYOUT::DRAM_END; addr += COMMON::PAGE_SIZE) {
        // 跳过 0x00 开始的一页，便于判断 nullptr
        if (addr == nullptr) {
            continue;
        }
        // 初始化可用内存段的物理页数组
        // 地址对应的物理页数组下标
        phy_pages[pages].addr = addr;
        // 内核已使用部分
        if (addr >= COMMON::ALIGN4K(COMMON::KERNEL_START_ADDR) &&
            addr < COMMON::ALIGN4K(COMMON::KERNEL_END_ADDR)) {
            phy_pages[pages].ref = 1;
        }
        else {
            phy_pages[pages].ref = 0;
        }
// #define DEBUG
#ifdef DEBUG
        printf("phy_pages[%d].addr = addr: 0x%X\n", pages,
               phy_pages[pages].addr);
#undef DEBUG
#endif
        pages++;
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
    void *addr = nullptr;
    if (_pages > 0) {
        addr = zone[_zone]->alloc(_pages);
    }
    if (addr == nullptr) {
        printf("No enough mem: 0x%X pages.\n", _pages);
    }
    return addr;
}

void PMM::free_page(void *_addr, uint32_t _pages, COMMON::zone_t _zone) {
    if (_addr == nullptr) {
        printf("addr is null.\n");
        return;
    }
    zone[_zone]->free(_addr, _pages);
    return;
}

uint32_t PMM::free_pages_count(COMMON::zone_t _zone) {
    return zone[_zone]->free_pages_count();
}

PMM pmm;
