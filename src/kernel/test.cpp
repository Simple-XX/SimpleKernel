
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
