
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "common.h"
#include "pmm.h"

void PMM::helper(void) {
    // 物理地址开始 从 0 开始
    start  = 0x0;
    length = 0;
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
