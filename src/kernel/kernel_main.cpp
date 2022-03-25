
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

#include "common.h"
#include "stdio.h"
#include "iostream"
#include "assert.h"
#include "boot_info.h"
#include "cxxabi.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "intr.h"
#include "cpu.hpp"
#include "kernel.h"
#include "opensbi.h"
#include "smp_task.h"

/**
 * @brief 启动所有 core
 */
static void start_all_core(uintptr_t _dtb_addr) {
    (void)_dtb_addr;
#if defined(__riscv)
    for (size_t i = 0; i < COMMON::CORES_COUNT; i++) {
        OPENSBI::get_instance().hart_start(i, COMMON::KERNEL_TEXT_START_ADDR,
                                           _dtb_addr);
    }
#endif
    return;
}

volatile static bool started = false;

void kernel_main_smp(void) {
    while (started == false) {
        ;
    }
    info("smp Running...\n");
    VMM::get_instance().init_other_core();
    // 中断初始化
    INTR::get_instance().init_other_core();
    // 初始化任务调度
    SMP_TASK::get_instance().init_other_core();
    // 时钟中断初始化
    TIMER::get_instance().init_other_core();
    return;
}

/**
 * @brief 内核主要逻辑
 * @note 这个函数不会返回
 */
void kernel_main(uintptr_t _hartid, uintptr_t _dtb_addr) {
    CPU::set_curr_core_id(_hartid);
    if (_hartid == COMMON::BOOT_HART_ID) {
        // 初始化 C++
        cpp_init();
        // 初始化基本信息
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
        // 初始化任务调度
        SMP_TASK::get_instance().init();
        // 设置时钟中断
        TIMER::get_instance().init();
        // 显示基本信息
        show_info();
        // 唤醒其余 core
        start_all_core(_dtb_addr);
        started = true;
    }
    else {
        // 唤醒 core0
        start_all_core(_dtb_addr);
        while (1)
            ;
        // 执行其它 core 的初始化
        kernel_main_smp();
    }

    // 允许中断
    CPU::ENABLE_INTR();

     test_sched();
    // 开始调度
    while (1) {
        SMP_TASK::get_instance().sched();
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
