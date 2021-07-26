
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "common.h"
#include "multiboot2.h"
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

void PMM::helper(void) {
    // 物理地址开始 从 0 开始
    start = 0x0;
    MULTIBOOT2::multiboot2_iter(get_e820, (void *)&e820map);
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
    return;
}

void PMM::move_boot_info(void) {
    // 计算 multiboot2 信息需要多少页
    size_t pages = MULTIBOOT2::multiboot2_size / COMMON::PAGE_SIZE;
    if (MULTIBOOT2::multiboot2_size % COMMON::PAGE_SIZE != 0) {
        pages++;
    }
    // 申请空间
    void *new_addr = alloc_pages_kernel(pages);
    // 复制过来，完成后以前的内存就可以使用了
    memcpy(new_addr, MULTIBOOT2::multiboot2_addr, pages * COMMON::PAGE_SIZE);
    // 设置地址
    MULTIBOOT2::multiboot2_addr = new_addr;
    return;
}
