
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "stdarg.h"
#include "string.h"
#include "cxxabi.h"
#include "common.h"
#include "color.h"
#include "assert.h"
#include "keyboard.h"
#include "multiboot2.h"
#include "kernel.h"

#if defined(__i386__) || defined(__x86_64__)
#include "gdt.h"
#include "intr.h"
#include "clock.h"
#elif defined(__arm__) || defined(__aarch64__)
#endif

IO   KERNEL::io;
APIC KERNEL::apic;

KERNEL::KERNEL(uint32_t _magic, void *_addr)
    : pmm(PMM()), vmm(VMM()), magic(_magic), addr(_addr) {
    // 读取 grub2 传递的信息
    MULTIBOOT2::multiboot2_init(magic, addr);
    cpp_init();
    arch_init();
    drv_init();
    // 物理内存管理初始化
    pmm.init();
    // 测试物理内存
    test_pmm();
    // 虚拟内存初始化
    vmm.init();
    // 测试虚拟内存
    test_vmm();
    return;
}

KERNEL::~KERNEL(void) {
    return;
}

void KERNEL::arch_init(void) const {
#if defined(__i386__) || defined(__x86_64__)
    GDT::init();
    INTR::init();
#elif defined(__arm__) || defined(__aarch64__)
#endif
    return;
}

void KERNEL::drv_init(void) const {
#if defined(__i386__) || defined(__x86_64__)
    // 时钟初始化
    clock.init();
#elif defined(__arm__) || defined(__aarch64__)
#endif
    // 键盘输入初始化
    keyboard.init();
    return;
}

int32_t KERNEL::test_pmm(void) {
    // TODO: 分配的地址应该在相应区域内
    uint32_t cd         = 0xCD;
    uint8_t *addr1      = nullptr;
    uint8_t *addr2      = nullptr;
    uint8_t *addr3      = nullptr;
    uint8_t *addr4      = nullptr;
    uint32_t free_count = pmm.free_pages_count(COMMON::NORMAL);
    addr1               = (uint8_t *)pmm.alloc_page(0x9F, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 0x9F);
    *(uint32_t *)addr1 = cd;
    assert((*(uint32_t *)addr1 == cd));
    pmm.free_page(addr1, 0x9F, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    addr2 = (uint8_t *)pmm.alloc_page(1, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1);
    *(int *)addr2 = cd;
    assert((*(uint32_t *)addr2 == cd));
    pmm.free_page(addr2, 1, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    addr3 = (uint8_t *)pmm.alloc_page(1024, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1024);
    *(int *)addr3 = cd;
    assert((*(uint32_t *)addr3 == cd));
    pmm.free_page(addr3, 1024, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    addr4 = (uint8_t *)pmm.alloc_page(4096, COMMON::NORMAL);
    assert((*(uint32_t *)addr4 == cd));
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 4096);
    *(int *)addr4 = cd;
    pmm.free_page(addr4, 4096, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    io.printf("pmm test done.\n");
    return 0;
}

int32_t KERNEL::test_vmm(void) {
    uint32_t addr = (uint32_t) nullptr;
    // 首先确认内核空间被映射了
    assert(vmm.get_pgd() != nullptr);
    // 0x00 留空
    assert(vmm.get_mmap(vmm.get_pgd(), 0x00, nullptr) == 0);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0x03, nullptr) == 0);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0xCD, nullptr) == 0);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0xFFF, &addr) == 0);
    assert(addr == (uint32_t) nullptr);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0x1000, &addr) == 1);
    assert(addr == COMMON::KERNEL_BASE + 0x1000);
    addr = (uint32_t) nullptr;
    assert(vmm.get_mmap(vmm.get_pgd(),
                        (void *)(COMMON::KERNEL_BASE + VMM_KERNEL_SIZE - 1),
                        &addr) == 1);
    assert(addr ==
           ((COMMON::KERNEL_BASE + VMM_KERNEL_SIZE - 1) & COMMON::PAGE_MASK));
    addr = (uint32_t) nullptr;
    assert(vmm.get_mmap(
               vmm.get_pgd(),
               (void *)((uint32_t)COMMON::KERNEL_START_4K + VMM_KERNEL_SIZE),
               &addr) == 0);
    assert(addr == (uint32_t) nullptr);
    addr = (uint32_t) nullptr;
    assert(vmm.get_mmap(vmm.get_pgd(),
                        (void *)((uint32_t)COMMON::KERNEL_START_4K +
                                 VMM_KERNEL_SIZE + 0x1024),
                        nullptr) == 0);
    // 测试映射与取消映射

    addr = (uint32_t) nullptr;
    // 准备映射的虚拟地址 3GB 处
    uint32_t va = 0xC0000000;
    // 准备映射的物理地址 0.75GB 处
    uint32_t pa = 0x30000000;
    // 确定一块未映射的内存
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)va, nullptr) == 0);
    // 映射
    vmm.mmap(vmm.get_pgd(), (void *)va, (void *)pa,
             VMM_PAGE_PRESENT | VMM_PAGE_RW);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)va, &addr) == 1);
    assert(addr == pa);
    // 写测试
    *(uint32_t *)va = 0xCD;
    //取消映射
    vmm.unmmap(vmm.get_pgd(), (void *)va);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)va, &addr) == 0);
    assert(addr == (uint32_t) nullptr);
    io.printf("vmm test done.\n");
    return 0;
}

void KERNEL::show_info(void) {
    // BUG: raspi2 下不能正常输出链接脚本中的地址
    io.printf(COLOR::LIGHT_GREEN,
              "kernel in memory start: 0x%08X, end 0x%08X\n",
              COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR);
    io.printf(
        COLOR::LIGHT_GREEN,
        "kernel in memory start4k: 0x%08X, end4k 0x%08X, KERNEL_SIZE: 0x%08X\n",
        COMMON::KERNEL_START_4K, COMMON::KERNEL_END_4K, COMMON::KERNEL_SIZE);
    io.printf(COLOR::LIGHT_GREEN, "kernel in memory size: %d KB, %d pages\n",
              ((uint8_t *)COMMON::KERNEL_END_ADDR -
               (uint8_t *)COMMON::KERNEL_START_ADDR) /
                  1024,
              ((uint8_t *)COMMON::KERNEL_END_4K -
               (uint8_t *)COMMON::KERNEL_START_4K) /
                  COMMON::PAGE_SIZE);
    io.printf(COLOR::LIGHT_GREEN, "Simple Kernel.\n");
    return;
}

int32_t KERNEL::test(void) {
    io.printf("test done.\n");
    return 0;
}
