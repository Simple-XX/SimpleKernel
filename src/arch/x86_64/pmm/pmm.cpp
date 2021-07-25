
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "common.h"
#include "multiboot2.h"
#include "gdt.h"
#include "e820.h"
#include "pmm.h"

// 用于保存物理内存信息
static e820map_t e820map;

// 读取 grub2 传递的物理内存信息，保存到 e820map_t 结构体中
// 一般而言是这样的
// 地址(长度) 类型
// 0x00(0x9F000) 0x1
// 0x9F000(0x1000) 0x2
// 0xE8000(0x18000) 0x2
// 0x100000(0x7EF0000) 0x1
// 0x7FF0000(0x10000) 0x3
// 0xFFFC0000(0x40000) 0x2
static bool get_e820(MULTIBOOT2::multiboot_tag_t *_tag, void *_data) {
    if (_tag->type != MULTIBOOT2::MULTIBOOT_TAG_TYPE_MMAP) {
        return false;
    }
    e820map_t *                         e820map = (e820map_t *)_data;
    MULTIBOOT2::multiboot_mmap_entry_t *mmap =
        ((MULTIBOOT2::multiboot_tag_mmap_t *)_tag)->entries;
    for (; (uint8_t *)mmap < (uint8_t *)_tag + _tag->size;
         mmap =
             (MULTIBOOT2::multiboot_mmap_entry_t
                  *)((uint8_t *)mmap +
                     ((MULTIBOOT2::multiboot_tag_mmap_t *)_tag)->entry_size)) {
        e820map->map[e820map->nr_map].addr   = (uintptr_t)mmap->addr;
        e820map->map[e820map->nr_map].length = (uintptr_t)mmap->len;
        e820map->map[e820map->nr_map].type   = mmap->type;
        e820map->nr_map++;
    }
    return true;
}

const void *PMM::start                   = nullptr;
size_t      PMM::length                  = 0;
size_t      PMM::total_pages             = 0;
const void *PMM::kernel_space_start      = nullptr;
size_t      PMM::kernel_space_length     = 0;
const void *PMM::non_kernel_space_start  = nullptr;
size_t      PMM::non_kernel_space_length = 0;
ALLOCATOR * PMM::allocator               = nullptr;
ALLOCATOR * PMM::kernel_space_allocator  = nullptr;

PMM::PMM(void) {
    return;
}

PMM::~PMM(void) {
    return;
}

bool PMM::init(void) {
    // 因为 GDT 是 x86 遗毒，所以在这里处理
    GDT::init();
    // TODO: 处理不能使用的内存，在虚拟内存映射时需要考虑
    MULTIBOOT2::multiboot2_iter(get_e820, (void *)&e820map);
    // 物理地址开始 从 0 开始
    start = 0x0;
    // TODO: 动态获取
    // 遍历 e820map
    for (size_t i = 0; i < e820map.nr_map; i++) {
        // 更新物理内存长度
        length += e820map.map[i].length;
    }
    // 计算页数
    total_pages = length / COMMON::PAGE_SIZE;
    // 内核空间地址开始
    kernel_space_start = COMMON::KERNEL_START_ADDR;
    // 长度手动指定
    kernel_space_length = COMMON::KERNEL_SPACE_SIZE;
    // 非内核空间在内核空间结束后
    non_kernel_space_start = (void *)((uint8_t *)COMMON::KERNEL_START_ADDR +
                                      COMMON::KERNEL_SPACE_SIZE);
    // 长度为总长度减去内核长度
    non_kernel_space_length = length - kernel_space_length;
    // 内核空间
    static FIRSTFIT first_fit_allocator_kernel(
        "First Fit Allocator(kernel space)", kernel_space_start,
        kernel_space_length / COMMON::PAGE_SIZE);
    kernel_space_allocator = (ALLOCATOR *)&first_fit_allocator_kernel;

    // 非内核空间
    static FIRSTFIT first_fit_allocator(
        "First Fit Allocator", non_kernel_space_start,
        non_kernel_space_length / COMMON::PAGE_SIZE);
    allocator = (ALLOCATOR *)&first_fit_allocator;
    // 内核实际占用页数 这里也算了 0～1M 的 reserved 内存
    size_t kernel_pages =
        ((uint8_t *)COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE) -
         (uint8_t *)COMMON::ALIGN(COMMON::KERNEL_START_ADDR,
                                  COMMON::PAGE_SIZE)) /
        COMMON::PAGE_SIZE;
    // 将内核已使用部分划分出来
    if (alloc_pages_kernel(const_cast<void *>(COMMON::KERNEL_START_ADDR),
                           kernel_pages) == true) {
        printf("pmm init.\n");
        return true;
    }
    else {
        return false;
    }
}

size_t PMM::get_pmm_length(void) {
    return length;
}

void *PMM::alloc_page(void) {
    return allocator->alloc(1);
}

void *PMM::alloc_pages(size_t _len) {
    return allocator->alloc(_len);
}

bool PMM::alloc_pages(void *_addr, size_t _len) {
    return allocator->alloc(_addr, _len);
}

void *PMM::alloc_page_kernel(void) {
    return kernel_space_allocator->alloc(1);
}

void *PMM::alloc_pages_kernel(size_t _len) {
    return kernel_space_allocator->alloc(_len);
}

bool PMM::alloc_pages_kernel(void *_addr, size_t _len) {
    return kernel_space_allocator->alloc(_addr, _len);
}

void PMM::free_page(void *_addr) {
    // 判断应该使用哪个分配器
    if (_addr >= kernel_space_start &&
        _addr < (uint8_t *)kernel_space_start + kernel_space_length) {
        kernel_space_allocator->free(_addr, 1);
    }
    else if (_addr >= non_kernel_space_start &&
             _addr <
                 (uint8_t *)non_kernel_space_start + non_kernel_space_length) {
        allocator->free(_addr, 1);
    }
    else {
        // 如果都不是说明有问题
        assert(0);
    }
    return;
}

void PMM::free_pages(void *_addr, size_t _len) {
    // 判断应该使用哪个分配器
    if (_addr >= kernel_space_start &&
        _addr < (uint8_t *)kernel_space_start + kernel_space_length) {
        kernel_space_allocator->free(_addr, _len);
    }
    else if (_addr >= non_kernel_space_start &&
             _addr <
                 (uint8_t *)non_kernel_space_start + non_kernel_space_length) {
        allocator->free(_addr, _len);
    }
    // 如果都不是说明有问题
    else {
        assert(0);
    }
    return;
}

uint64_t PMM::get_used_pages_count(void) {
    return kernel_space_allocator->get_used_count() +
           allocator->get_used_count();
}

uint64_t PMM::get_free_pages_count(void) {
    return kernel_space_allocator->get_free_count() +
           allocator->get_free_count();
}
