
/**
 * @file kernel_main.cpp
 * @brief 内核主要逻辑
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#include "boot_info.h"
#include "cassert"
#include "common.h"
#include "cpu.hpp"
#include "cstdio"
#include "cstdlib"
#include "dev_drv_manager.h"
#include "heap.h"
#include "intr.h"
#include "iostream"
#include "kernel.h"
#include "pmm.h"
#include "vmm.h"

/**
 * @brief 内核主要逻辑
 * @note 这个函数不会返回
 */
void kernel_main(void) {
    // 初始化
    BOOT_INFO::init();
    // 物理内存初始化
    PMM::get_instance().init();
    // 测试物理内存
    test_pmm();
    // 虚拟内存初始化
    /// @todo 将vmm的初始化放在构造函数里，这里只做开启分页
    VMM::get_instance().init();
    // 测试虚拟内存
    test_vmm();
    // 堆初始化
    HEAP::get_instance().init();
    // 测试堆
    test_heap();
    // 中断初始化
    INTR::get_instance().init();
    // 测试中断
    test_intr();
    // 时钟中断初始化
    TIMER::get_instance().init();
    // 初始化设备
    DEV_DRV_MANAGER::get_instance().init();
    // 测试设备
    test_device();
    // 允许中断
    CPU::ENABLE_INTR();

    // 显示基本信息
    show_info();
    // 进入死循环
    while (1) {
        ;
    }
    // 不应该执行到这里
    assert(0);
    return;
}

/**
 * @brief 输出系统信息
 */
void show_info(void) {
    // 内核实际大小
    auto kernel_size = COMMON::KERNEL_END_ADDR - COMMON::KERNEL_START_ADDR;
    // 内核实际占用页数
    auto kernel_pages
      = (COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE)
         - COMMON::ALIGN(COMMON::KERNEL_START_ADDR, COMMON::PAGE_SIZE))
      / COMMON::PAGE_SIZE;
    info("Kernel start: 0x%p, end 0x%p, size: 0x%X bytes, 0x%X pages.\n",
         COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR, kernel_size,
         kernel_pages);
    info("Kernel start4k: 0x%p, end4k: 0x%p.\n",
         COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 4 * COMMON::KB),
         COMMON::ALIGN(COMMON::KERNEL_END_ADDR, 4 * COMMON::KB));
    std::cout << "Simple Kernel." << std::endl;
    return;
}
