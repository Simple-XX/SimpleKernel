
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "stdarg.h"
#include "string.h"
#include "cxxabi.h"
#include "color.h"
#include "debug.h"
#include "assert.h"

#if defined(__i386__) || defined(__x86_64__)
#include "gdt.h"
#include "intr.h"
#include "clock.h"
#elif defined(__arm__) || defined(__aarch64__)
#endif

#include "io.h"
#include "keyboard.h"
#include "kernel.h"

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
    // BUG: raspi2 下不能正常输出链接脚本中的地址
    io.printf(LIGHT_GREEN, "kernel in memory start: 0x%08X, end 0x%08X\n",
              KERNEL_START_ADDR, KERNEL_END_ADDR);
    io.printf(LIGHT_GREEN, "kernel in memory size: %d KB, %d pages\n",
              (KERNEL_END_ADDR - KERNEL_START_ADDR) / 1024,
              (KERNEL_END_ADDR - KERNEL_START_ADDR + 4095) / 1024 / 4);
    io.printf(LIGHT_GREEN, "Simple Kernel.\n");
    return;
}

int32_t KERNEL::init(void) {
    // 全局对象的构造
    cpp_init();
    // 输入输出初始化
    io.init();
    // 架构相关初始化
    arch_init();
    // 读取 grub2 传递的信息
    multiboot2_init(magic, addr);
    // 时钟初始化
    clock.init();
    // 键盘输入初始化
    keyboard.init();
    // debug 程序初始化
    debug.init(magic, addr);
    // 物理内存管理初始化
    pmm.init();
    // 显示内核信息
    show_info();
    return 0;
}

int32_t KERNEL::test(void) {
    test_pmm();
    return 0;
}
