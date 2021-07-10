
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// test.cpp for Simple-XX/SimpleKernel.

#include "common.h"
#include "stdio.h"
#include "iostream"
#include "assert.h"
#include "pmm.h"
#include "vmm.h"
#include "kernel.h"

int32_t test_pmm(void) {
    size_t free_pages = pmm.get_free_pages_count();
    auto   kernel_pages =
        ((uint8_t *)COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE) -
         (uint8_t *)COMMON::ALIGN(COMMON::KERNEL_START_ADDR,
                                  COMMON::PAGE_SIZE)) /
        COMMON::PAGE_SIZE;
    assert(free_pages == (COMMON::PMM_SIZE / COMMON::PAGE_SIZE) - kernel_pages);
    size_t used_pages = pmm.get_used_pages_count();
    assert(used_pages == kernel_pages);
    auto addr1 = pmm.alloc_pages(2);
    assert(pmm.get_used_pages_count() == 2 + kernel_pages);
    auto addr2 = pmm.alloc_pages(3);
    assert(pmm.get_used_pages_count() == 5 + kernel_pages);
    auto addr3 = pmm.alloc_pages(100);
    assert(pmm.get_used_pages_count() == 105 + kernel_pages);
    auto addr4 = pmm.alloc_pages(100);
    assert(pmm.get_used_pages_count() == 205 + kernel_pages);
    auto addr5 = pmm.alloc_pages(0xFFFFFFFF);
    assert(addr5 == nullptr);
    pmm.free_pages(addr1, 2);
    pmm.free_pages(addr2, 3);
    pmm.free_pages(addr3, 100);
    pmm.free_pages(addr4, 100);
    assert(pmm.get_free_pages_count() == free_pages);
    printf("pmm test done.\n");
    return 0;
}

int32_t test_vmm(void) {
    ptrdiff_t addr = (ptrdiff_t) nullptr;
    // 首先确认内核空间被映射了
    assert(vmm.get_pgd() != nullptr);
    // 0x00 留空
    assert(vmm.get_mmap(vmm.get_pgd(), 0x00, nullptr) == 0);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0x03, nullptr) == 0);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0xCD, nullptr) == 0);
    assert(vmm.get_mmap(vmm.get_pgd(), (void *)0xFFF, &addr) == 0);
    assert(addr == (ptrdiff_t) nullptr);
    assert(vmm.get_mmap(vmm.get_pgd(),
                        (void *)((ptrdiff_t)COMMON::KERNEL_START_ADDR + 0x1000),
                        &addr) == 1);
    assert(addr == (ptrdiff_t)COMMON::KERNEL_START_ADDR + 0x1000);
    addr = (ptrdiff_t) nullptr;
    assert(vmm.get_mmap(vmm.get_pgd(),
                        (void *)((ptrdiff_t)COMMON::KERNEL_START_ADDR +
                                 VMM_KERNEL_SIZE - 1),
                        &addr) == 1);
    assert(addr == (ptrdiff_t)(((ptrdiff_t)COMMON::KERNEL_START_ADDR +
                                VMM_KERNEL_SIZE - 1) &
                               COMMON::PAGE_MASK));
    addr = (ptrdiff_t) nullptr;
    assert(
        vmm.get_mmap(vmm.get_pgd(),
                     (void *)((ptrdiff_t)COMMON::ALIGN(
                                  COMMON::KERNEL_START_ADDR, 4 * COMMON::KB) +
                              VMM_KERNEL_SIZE),
                     &addr) == 0);
    assert(addr == (ptrdiff_t) nullptr);
    addr = (ptrdiff_t) nullptr;
    assert(
        vmm.get_mmap(vmm.get_pgd(),
                     (void *)((ptrdiff_t)COMMON::ALIGN(
                                  COMMON::KERNEL_START_ADDR, 4 * COMMON::KB) +
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
