
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"
#include "cxxabi.h"
#include "color.h"
#include "io.h"
#include "cpu.hpp"
#include "gdt.h"
#include "intr.h"
#include "clock.h"
#include "keyboard.h"
#include "pmm.h"
#include "multiboot2.h"
#include "debug.h"
#include "assert.h"

KERNEL::KERNEL(void) {
    pmm = PMM();
    return;
}

KERNEL::KERNEL(addr_t magic, addr_t addr) {
    this->magic = magic;
    this->addr  = addr;
    return;
}

KERNEL::~KERNEL(void) {
    return;
}

void KERNEL::arch_init(void) const {
    GDT::init();
    INTR::init();
    return;
}

int32_t KERNEL::test_pmm(void) {
    uint32_t cd         = 0xCD;
    uint8_t *addr1      = nullptr;
    uint8_t *addr2      = nullptr;
    uint8_t *addr3      = nullptr;
    uint8_t *addr4      = nullptr;
    uint32_t free_count = pmm.free_pages_count();
    addr1               = (uint8_t *)pmm.alloc_page(0x9F);
    assert(pmm.free_pages_count() == free_count - 0x9F);
    *(uint32_t *)addr1 = cd;
    assert((*(uint32_t *)addr1 == cd));
    pmm.free_page(addr1, 0x9F);
    assert(pmm.free_pages_count() == free_count);
    addr2 = (uint8_t *)pmm.alloc_page(1);
    assert(pmm.free_pages_count() == free_count - 1);
    *(int *)addr2 = cd;
    assert((*(uint32_t *)addr2 == cd));
    pmm.free_page(addr2, 1);
    assert(pmm.free_pages_count() == free_count);
    addr3 = (uint8_t *)pmm.alloc_page(1024);
    assert(pmm.free_pages_count() == free_count - 1024);
    *(int *)addr3 = cd;
    assert((*(uint32_t *)addr3 == cd));
    pmm.free_page(addr3, 1024);
    assert(pmm.free_pages_count() == free_count);
    addr4 = (uint8_t *)pmm.alloc_page(4096);
    assert((*(uint32_t *)addr4 == cd));
    assert(pmm.free_pages_count() == free_count - 4096);
    *(int *)addr4 = cd;
    pmm.free_page(addr4, 4096);
    assert(pmm.free_pages_count() == free_count);
    io.printf("pmm test done.\n");
    return 0;
}

void KERNEL::show_info(void) {
    // 输出一些基本信息
    io.printf(LIGHT_GREEN, "kernel in memory start: 0x%08X, end 0x%08X\n",
              kernel_start, kernel_end);
    io.printf(LIGHT_GREEN, "kernel in memory size: %d KB, %d pages\n",
              (kernel_end - kernel_start) / 1024,
              (kernel_end - kernel_start + 4095) / 1024 / 4);
    io.printf(LIGHT_GREEN, "Simple Kernel.\n");

    return;
}

int32_t KERNEL::init(void) {
    // 全局对象的构造
    cpp_init();
    // 架构相关初始化
    arch_init();
    // 读取 grub2 传递的信息
    multiboot2_init(magic, addr);
    // 时钟初始化
    clockk.init();
    // 键盘输入初始化
    keyboardk.init();
    // debug 程序初始化
    debug.init(magic, addr);
    // 物理内存管理初始化
    pmm.init();
    // 显示内核信息
    this->show_info();
    return 0;
}

int32_t KERNEL::test(void) {
    test_pmm();
    return 0;
}
