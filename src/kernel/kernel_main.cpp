
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
#include "keyboard.h"
#include "cpu.hpp"
#include "kernel.h"

#if defined(__i386__) || defined(__x86_64__)
#include "apic.h"
#include "multiboot2.h"
#endif

// 内核入口
void kernel_main(uint32_t size __attribute__((unused)),
                 void *   addr __attribute__((unused))) {
#if defined(__i386__) || defined(__x86_64__)
    // __x86_64__ 时用于传递 grub 信息
    MULTIBOOT2::multiboot2_init(size, addr);
#endif
    cpp_init();
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
#if defined(__i386__) || defined(__x86_64__)
    // APIC 初始化
    apic.init();
#endif
    // 键盘初始化
    // keyboard.init();
    show_info();
    CPU::ENABLE_INTR();
    while (1) {
        ;
    }
    assert(0);
    return;
}

int test_pmm(void) {
    // TODO: 分配的地址应该在相应区域内
    uint32_t cd         = 0xCD;
    uint8_t *addr1      = nullptr;
    uint8_t *addr2      = nullptr;
    uint8_t *addr3      = nullptr;
    uint8_t *addr4      = nullptr;
    uint32_t free_count = pmm.free_pages_count(COMMON::NORMAL);
    addr1               = (uint8_t *)pmm.alloc_page(0x9F, COMMON::NORMAL);
    if (addr1 != nullptr) {
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 0x9F);
        *(uint32_t *)addr1 = cd;
        assert((*(uint32_t *)addr1 == cd));
        pmm.free_page(addr1, 0x9F, COMMON::NORMAL);
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    }
    addr2 = (uint8_t *)pmm.alloc_page(1, COMMON::NORMAL);
    if (addr2 != nullptr) {
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1);
        *(int *)addr2 = cd;
        assert((*(uint32_t *)addr2 == cd));
        pmm.free_page(addr2, 1, COMMON::NORMAL);
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    }
    addr3 = (uint8_t *)pmm.alloc_page(1024, COMMON::NORMAL);
    if (addr3 != nullptr) {
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1024);
        *(int *)addr3 = cd;
        assert((*(uint32_t *)addr3 == cd));
        pmm.free_page(addr3, 1024, COMMON::NORMAL);
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    }
    addr4 = (uint8_t *)pmm.alloc_page(4096, COMMON::NORMAL);
    if (addr4 != nullptr) {
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 4096);
        *(int *)addr4 = cd;
        assert((*(uint32_t *)addr4 == cd));
        pmm.free_page(addr4, 4096, COMMON::NORMAL);
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    }
    printf("pmm test done.\n");
    return 0;
}

int test_vmm(void) {
    ptrdiff_t addr = (ptrdiff_t) nullptr;
    // 首先确认内核空间被映射了
    assert(vmm.get_pgd() != nullptr);
    // 0x00 留空
    assert(vmm.get_mmap(vmm.get_pgd(), 0x00, nullptr) == 0);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0x03, nullptr) == 0);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0xCD, nullptr) == 0);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0xFFF, &addr) == 0);
    assert(addr == (ptrdiff_t) nullptr);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)(COMMON::KERNEL_BASE + 0x1000),
                        &addr) == 1);
    assert(addr == COMMON::KERNEL_BASE + 0x1000);
    addr = (ptrdiff_t) nullptr;
    assert(vmm.get_mmap(vmm.get_pgd(),
                        (void *)(COMMON::KERNEL_BASE + VMM_KERNEL_SIZE - 1),
                        &addr) == 1);
    assert(addr ==
           ((COMMON::KERNEL_BASE + VMM_KERNEL_SIZE - 1) & COMMON::PAGE_MASK));
    addr = (ptrdiff_t) nullptr;
    assert(vmm.get_mmap(
               vmm.get_pgd(),
               (void *)((ptrdiff_t)COMMON::KERNEL_START_4K + VMM_KERNEL_SIZE),
               &addr) == 0);
    assert(addr == (ptrdiff_t) nullptr);
    addr = (ptrdiff_t) nullptr;
    assert(vmm.get_mmap(vmm.get_pgd(),
                        (void *)((ptrdiff_t)COMMON::KERNEL_START_4K +
                                 VMM_KERNEL_SIZE + 0x1024),
                        nullptr) == 0);
    // 测试映射与取消映射
    addr = (ptrdiff_t) nullptr;
    // 准备映射的虚拟地址 3GB 处
    ptrdiff_t va = 0xC0000000;
    // 准备映射的物理地址 0.75GB 处
    ptrdiff_t pa = 0x30000000;
    // 确定一块未映射的内存
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)va, nullptr) == 0);
    // 映射
    vmm.mmap(vmm.get_pgd(), (void *)va, (void *)pa,
             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)va, &addr) == 1);
    assert(addr == pa);
    // 写测试
    *(ptrdiff_t *)va = 0xCD;
    //取消映射
    vmm.unmmap(vmm.get_pgd(), (void *)va);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)va, &addr) == 0);
    assert(addr == (ptrdiff_t) nullptr);
    printf("vmm test done.\n");
    return 0;
}

int test_heap(void) {
    void * addr1  = nullptr;
    void * addr2  = nullptr;
    void * addr3  = nullptr;
    void * addr4  = nullptr;
    size_t free0  = heap.get_free();
    size_t total0 = heap.get_total();
    size_t block0 = heap.get_block();
    addr1         = heap.malloc(0x1);
    size_t free1  = heap.get_free();
    size_t total1 = heap.get_total();
    size_t block1 = heap.get_block();
    addr2         = heap.malloc(0x1000);
    size_t free2  = heap.get_free();
    size_t total2 = heap.get_total();
    size_t block2 = heap.get_block();
    addr3         = heap.malloc(0x3FF);
    size_t free3  = heap.get_free();
    size_t total3 = heap.get_total();
    size_t block3 = heap.get_block();
    addr4         = heap.malloc(0xE);
    heap.free(addr4);
    assert(heap.get_free() == free3);
    assert(heap.get_total() == total3);
    assert(heap.get_block() == block3);
    heap.free(addr3);
    assert(heap.get_free() == free2);
    assert(heap.get_total() == total2);
    assert(heap.get_block() == block2);
    heap.free(addr2);
    assert(heap.get_free() == free1);
    assert(heap.get_total() == total1);
    assert(heap.get_block() == block1);
    heap.free(addr1);
    assert(heap.get_free() == free0);
    assert(heap.get_total() == total0);
    assert(heap.get_block() == block0);
    printf("heap test done.\n");
    return 0;
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
