
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "common.h"
#include "endian.h"
#include "dtb.h"
#include "pmm.h"

// 保存物理地址信息
struct phy_mem_t {
    void * addr;
    size_t len;
};

static phy_mem_t phy_mem;

// TODO: 完善
static bool get_phy_mem(DTB::dtb_iter_t *_iter, void *_data) {
    if (_iter->tag != DTB::FDT_PROP) {
        return false;
    }
    if (strncmp((char *)_iter->node_name, "memory", sizeof("memory") - 1) ==
        0) {
        if (strcmp((char *)_iter->prop_name, "reg") == 0) {
            phy_mem_t *mem = (phy_mem_t *)_data;
            mem->addr      = (void *)((uintptr_t)be32toh(_iter->prop_addr[0]) +
                                 (uintptr_t)be32toh(_iter->prop_addr[1]));
            mem->len =
                be32toh(_iter->prop_addr[2]) + be32toh(_iter->prop_addr[3]);
        }
    }

    return false;
}

void PMM::helper(void) {
    DTB::dtb_iter(get_phy_mem, &phy_mem);
    // 物理地址开始 从 0 开始
    start  = phy_mem.addr;
    length = phy_mem.len;
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
    size_t pages = DTB::dtb_size / COMMON::PAGE_SIZE;
    if (DTB::dtb_size % COMMON::PAGE_SIZE != 0) {
        pages++;
    }
    // 申请空间
    void *new_addr = alloc_pages_kernel(pages);
    // 复制过来，完成后以前的内存就可以使用了
    memcpy(new_addr, DTB::dtb_addr, pages * COMMON::PAGE_SIZE);
    // 设置地址
    DTB::dtb_addr = (uint32_t *)new_addr;
    return;
}
