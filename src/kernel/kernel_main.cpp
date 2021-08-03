
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cxxabi.h"
#include "common.h"
#include "stdio.h"
#include "stdlib.h"
#include "iostream"
#include "assert.h"
#include "boot_info.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "intr.h"
#include "dev_drv_manager.h"
#include "cpu.hpp"
#include "kernel.h"
#include "dtb.h"
// TODO: gdb 调试
// TODO: clion 环境
// 内核入口
void kernel_main(void) {
    // 初始化
    BOOT_INFO::init();
    // 物理内存初始化
    PMM::init();
    // 测试物理内存
    test_pmm();
    // 虚拟内存初始化
    // TODO: 将vmm的初始化放在构造函数里，这里只做开启分页
    VMM::init();
    // 测试虚拟内存
    test_vmm();
    // 堆初始化
    heap.init();
    // 测试堆
    test_heap();
    // 中断初始化
    INTR::init();
    CPU::ENABLE_INTR();
    // 初始化设备
    DEV_DRV_MANAGER manage = DEV_DRV_MANAGER();

    show_info();

    while (1) {
        ;
    }
    // 不应该执行到这里
    assert(0);
    return;
}

void show_info(void) {
    // 内核实际大小
    auto kernel_size = COMMON::KERNEL_END_ADDR - COMMON::KERNEL_START_ADDR;
    // 内核实际占用页数
    auto kernel_pages =
        (COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE) -
         COMMON::ALIGN(COMMON::KERNEL_START_ADDR, COMMON::PAGE_SIZE)) /
        COMMON::PAGE_SIZE;
    info("Kernel start: 0x%p, end 0x%p, size: 0x%X bytes, 0x%X pages.\n",
         COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR, kernel_size,
         kernel_pages);
    info("Kernel start4k: 0x%p, end4k: 0x%p.\n",
         COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 4 * COMMON::KB),
         COMMON::ALIGN(COMMON::KERNEL_END_ADDR, 4 * COMMON::KB));
    std::cout << "Simple Kernel." << std::endl;
    return;
}
