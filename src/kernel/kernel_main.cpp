
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
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "intr.h"
#include "cpu.hpp"
#include "kernel.h"
#include "stdlib.h"

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
    //    test_heap();
    // -----------------

    // 根据字长不同 CHUNK_SIZE 是不一样的
    size_t chunk_size = 0;
    if (sizeof(void *) == 4) {
        chunk_size = 0x10;
    }
    else if (sizeof(void *) == 8) {
        chunk_size = 0x20;
    }
    void *addr1 = nullptr;
    void *addr2 = nullptr;
    void *addr3 = nullptr;
    void *addr4 = nullptr;
    // 申请超过最大允许的内存 65536B
    addr1 = kmalloc(0x10001);
    // 应该返回 nullptr
    assert(addr1 == nullptr);
    // 申请小块内存
    addr2 = kmalloc(0x1);
    assert(addr2 != nullptr);
    // 第一块被申请的内存，减去 chunk 大小后应该是 4k 对齐的
    assert(((uintptr_t)((uint8_t *)addr2 - chunk_size) & 0xFFF) == 0x0);
    // 在 LEN512 申请新的内存
    addr3 = kmalloc(0x200);
    assert(addr3 != nullptr);
    // 第一块被申请的内存，减去 chunk 大小后应该是 4k 对齐的
    assert(((uintptr_t)((uint8_t *)addr3 - chunk_size) & 0xFFF) == 0x0);
    // 加上 chunk 大小长度刚好是 LEN256
    addr4 = kmalloc(0x80);
    assert(addr4 != nullptr);
    // LEN256 区域第二块被申请的内存，地址可以计算出来
    // 前一个块的地址+chunk 长度+数据长度+对齐长度
    assert(addr4 == (uint8_t *)addr2 + chunk_size + 0x1 + 0x7);
    // 全部释放
    info("addr1: 0x%X\n", addr1);
    info("addr2: 0x%X\n", addr2);
    info("addr3: 0x%X\n", addr3);
    info("addr4: 0x%X\n", addr4);
    kfree(addr1);
    kfree(addr2);
//    kfree(addr3);
    kfree(addr4);
    info("heap test done.\n");
    // -------------------

    // 中断初始化
    INTR::get_instance().init();
    // 时钟中断初始化
    TIMER::get_instance().init();
    // 允许中断
    CPU::ENABLE_INTR();
    // 显示基本信息
    show_info();
//    kfree(addr3);
//    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), 0xCCCCC000,
//                             0xCCCCC000, VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
//    info("434242342\n");
    // 进入死循环
    while (1) {
        info("tetete\n");
        int a=0;
        while (a<9999999)
        {a++;}
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
