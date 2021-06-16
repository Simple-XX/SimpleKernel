
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cxxabi.h"
#include "multiboot2.h"
#include "common.h"
#include "stdio.h"
#include "iostream"
#include "assert.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "intr.h"
#include "dev.h"
#include "cpu.hpp"
#include "kernel.h"
#include "string"

// 内核入口
void kernel_main(uint32_t, void *) {
    // 物理内存初始化
    pmm.init();
    // 测试物理内存
    test_pmm();
    // 虚拟内存初始化
    // TODO: 将vmm的初始化放在构造函数里，这里只做开启分页
    vmm.init();
    // 测试虚拟内存
    test_vmm();
    // 堆初始化
    heap.init();
    // 测试堆
    test_heap();
    // 中断初始化
    INTR::init();
    // 初始化设备
    DEV dev = DEV();

    show_info();
    CPU::ENABLE_INTR();
    while (1) {
        ;
    }
    assert(0);
    return;
}

void show_info(void) {
    info("kernel in memory start: 0x%X, end 0x%X\n", COMMON::KERNEL_START_ADDR,
         COMMON::KERNEL_END_ADDR);
    info("kernel in memory start4k: 0x%X, end4k 0x%X, KERNEL_SIZE:0x%X\n",
         COMMON::KERNEL_START_4K, COMMON::KERNEL_END_4K, COMMON::KERNEL_SIZE);
    info("kernel in memory size: %d KB, %d pages\n",
         ((uint8_t *)COMMON::KERNEL_END_ADDR -
          (uint8_t *)COMMON::KERNEL_START_ADDR) /
             1024,
         ((uint8_t *)COMMON::KERNEL_END_4K -
          (uint8_t *)COMMON::KERNEL_START_4K) /
             COMMON::PAGE_SIZE);
    info("Simple Kernel.\n");
    return;
}
