
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// test.cpp for Simple-XX/SimpleKernel.

#include "common.h"
#include "stdio.h"
#include "iostream"
#include "assert.h"
#include "pmm.h"
#include "kernel.h"

int32_t test_pmm(void) {
    // 保存现有 pmm 空闲页数量
    size_t free_pages = PMM::get_free_pages_count();
    // 计算内核实际占用页数
    auto kernel_pages =
        (COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE) -
         COMMON::ALIGN(COMMON::KERNEL_START_ADDR, COMMON::PAGE_SIZE)) /
        COMMON::PAGE_SIZE;
    // 再加上启动信息使用的页，一般为一页
    kernel_pages++;
    // TODO: 替换宏
    // riscv 的 dtb 占用空间比较大，两页
#ifdef __riscv
    kernel_pages++;
#endif
    // 空闲页数应该等于物理内存大小-内核使用
    assert(free_pages ==
           (PMM::get_pmm_length() / COMMON::PAGE_SIZE) - kernel_pages);
    // 获取已使用页数
    size_t used_pages = PMM::get_used_pages_count();
    // 已使用页数应该等于内核使用页数
    assert(used_pages == kernel_pages);
    // 分配
    auto addr1 = PMM::alloc_pages(2);
    // 已使用应该会更新
    assert(PMM::get_used_pages_count() == 2 + kernel_pages);
    // 同上
    auto addr2 = PMM::alloc_pages(3);
    assert(PMM::get_used_pages_count() == 5 + kernel_pages);
    // 同上
    auto addr3 = PMM::alloc_pages(100);
    assert(PMM::get_used_pages_count() == 105 + kernel_pages);
    // 同上
    auto addr4 = PMM::alloc_pages(100);
    assert(PMM::get_used_pages_count() == 205 + kernel_pages);
    // 分配超过限度的内存，应该返回 nullptr
    auto addr5 = PMM::alloc_pages(0xFFFFFFFF);
    assert(addr5 == 0);
    // 全部释放
    PMM::free_pages(addr1, 2);
    PMM::free_pages(addr2, 3);
    PMM::free_pages(addr3, 100);
    PMM::free_pages(addr4, 100);
    // 现在内存使用情况应该与此函数开始时相同
    assert(PMM::get_free_pages_count() == free_pages);
    info("pmm test done.\n");
    return 0;
}
