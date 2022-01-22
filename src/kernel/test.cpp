
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// test.cpp for Simple-XX/SimpleKernel.

#include "common.h"
#include "stdio.h"
#include "assert.h"
#include "pmm.h"
#include "vmm.h"
#include "stdlib.h"
#include "task.h"
#include "tmp_scheduler.h"
#include "kernel.h"

int32_t test_pmm(void) {
    // 保存现有 pmm 空闲页数量
    size_t free_pages = PMM::get_instance().get_free_pages_count();
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
           (PMM::get_instance().get_pmm_length() / COMMON::PAGE_SIZE) -
               kernel_pages);
    // 获取已使用页数
    size_t used_pages = PMM::get_instance().get_used_pages_count();
    // 已使用页数应该等于内核使用页数
    assert(used_pages == kernel_pages);
    // 分配
    auto addr1 = PMM::get_instance().alloc_pages(2);
    // 已使用应该会更新
    assert(PMM::get_instance().get_used_pages_count() == 2 + kernel_pages);
    // 同上
    auto addr2 = PMM::get_instance().alloc_pages(3);
    assert(PMM::get_instance().get_used_pages_count() == 5 + kernel_pages);
    // 同上
    auto addr3 = PMM::get_instance().alloc_pages(100);
    assert(PMM::get_instance().get_used_pages_count() == 105 + kernel_pages);
    // 同上
    auto addr4 = PMM::get_instance().alloc_pages(100);
    assert(PMM::get_instance().get_used_pages_count() == 205 + kernel_pages);
    // 分配超过限度的内存，应该返回 nullptr
    auto addr5 = PMM::get_instance().alloc_pages(0xFFFFFFFF);
    assert(addr5 == 0);
    // 全部释放
    PMM::get_instance().free_pages(addr1, 2);
    PMM::get_instance().free_pages(addr2, 3);
    PMM::get_instance().free_pages(addr3, 100);
    PMM::get_instance().free_pages(addr4, 100);
    // 现在内存使用情况应该与此函数开始时相同
    assert(PMM::get_instance().get_free_pages_count() == free_pages);
    info("pmm test done.\n");
    return 0;
}

int32_t test_vmm(void) {
    uintptr_t addr = 0;
    // 首先确认内核空间被映射了
    assert(VMM::get_instance().get_pgd() != nullptr);
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(),
                                        (COMMON::KERNEL_START_ADDR + 0x1000),
                                        &addr) == 1);
    assert(addr == COMMON::KERNEL_START_ADDR + 0x1000);
    addr = 0;
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(),
                                        COMMON::KERNEL_START_ADDR +
                                            VMM_KERNEL_SPACE_SIZE - 1,
                                        &addr) == 1);
    assert(addr == ((COMMON::KERNEL_START_ADDR + VMM_KERNEL_SPACE_SIZE - 1) &
                    COMMON::PAGE_MASK));
    addr = 0;
    assert(VMM::get_instance().get_mmap(
               VMM::get_instance().get_pgd(),
               (COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 4 * COMMON::KB) +
                VMM_KERNEL_SPACE_SIZE),
               &addr) == 0);
    assert(addr == 0);
    addr = 0;
    assert(VMM::get_instance().get_mmap(
               VMM::get_instance().get_pgd(),
               (COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 4 * COMMON::KB) +
                VMM_KERNEL_SPACE_SIZE + 0x1024),
               0) == 0);
    // 测试映射与取消映射
    addr = 0;
    // 准备映射的虚拟地址 3GB 处
    uintptr_t va = 0xC0000000;
    // 准备映射的物理地址 0.75GB 处
    uintptr_t pa = 0x30000000;
    // 确定一块未映射的内存
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(), va,
                                        nullptr) == 0);
    // 映射
    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), va, pa,
                             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(), va,
                                        &addr) == 1);
    assert(addr == pa);
    // 写测试
    *(uintptr_t *)va = 0xCD;
    //取消映射
    VMM::get_instance().unmmap(VMM::get_instance().get_pgd(), va);
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(), va,
                                        &addr) == 0);
    assert(addr == 0);
    info("vmm test done.\n");
    return 0;
}

// TODO: 更多测试
int test_heap(void) {
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
    addr1 = malloc(0x10001);
    // 应该返回 nullptr
    assert(addr1 == nullptr);
    // 申请小块内存
    addr2 = malloc(0x1);
    assert(addr2 != nullptr);
    // 第一块被申请的内存，减去 chunk 大小后应该是 4k 对齐的
    assert(((uintptr_t)((uint8_t *)addr2 - chunk_size) & 0xFFF) == 0x0);
    // 在 LEN512 申请新的内存
    addr3 = malloc(0x200);
    assert(addr3 != nullptr);
    // 第一块被申请的内存，减去 chunk 大小后应该是 4k 对齐的
    assert(((uintptr_t)((uint8_t *)addr3 - chunk_size) & 0xFFF) == 0x0);
    // 加上 chunk 大小长度刚好是 LEN256
    addr4 = malloc(0x80);
    assert(addr4 != nullptr);
    // LEN256 区域第二块被申请的内存，地址可以计算出来
    // 前一个块的地址+chunk 长度+数据长度+对齐长度
    assert(addr4 == (uint8_t *)addr2 + chunk_size + 0x1 + 0x7);
    // 全部释放
    free(addr1);
    free(addr2);
    free(addr3);
    free(addr4);
    info("heap test done.\n");
    return 0;
}

#define taskxx(a, b)                                                           \
    static int tmp##a##b = 0;                                                  \
    void       task##a##b(void) {                                              \
        while (1) {                                                      \
            tmp##a##b += 1;                                              \
            if (tmp##a##b == 5) {                                        \
                exit(0);                                                 \
            }                                                            \
        }                                                                \
    }

#define taskxx_(a)                                                             \
    taskxx(a, 0);                                                              \
    taskxx(a, 1);                                                              \
    taskxx(a, 2);                                                              \
    taskxx(a, 3);                                                              \
    taskxx(a, 4);

taskxx_(0);
taskxx_(1);
taskxx_(2);
taskxx_(3);
taskxx_(4);

#define add_taskxx(a, b)                                                       \
    task_t *tmp_task##a##b = new task_t("task" #a "" #b "", &task##a##b);      \
    liner_scheduler::get_instance().add_task(tmp_task##a##b);

#define taskxx_cond(a, b) (tmp##a##b == 5)
#define taskxx_cond_(a)                                                        \
    taskxx_cond(a, 0) && taskxx_cond(a, 1) && taskxx_cond(a, 2) &&             \
        taskxx_cond(a, 3) && taskxx_cond(a, 4)
#define taskxx_cond_all                                                        \
    (taskxx_cond_(0) && taskxx_cond_(1) && taskxx_cond_(2) && taskxx_cond_(3))

int test_sched(void) {
    info("sched testing...\n");
    auto a = CPU::get_curr_core_id();
    if (a == 0) {
        add_taskxx(0, 0);
        add_taskxx(0, 1);
        add_taskxx(0, 2);
        add_taskxx(0, 3);
        add_taskxx(0, 4);
    }
    else if (a == 1) {
        add_taskxx(1, 0);
        add_taskxx(1, 1);
        add_taskxx(1, 2);
        add_taskxx(1, 3);
        add_taskxx(1, 4);
    }
    else if (a == 2) {
        add_taskxx(2, 0);
        add_taskxx(2, 1);
        add_taskxx(2, 2);
        add_taskxx(2, 3);
        add_taskxx(2, 4);
    }
    else if (a == 3) {
        add_taskxx(3, 0);
        add_taskxx(3, 1);
        add_taskxx(3, 2);
        add_taskxx(3, 3);
        add_taskxx(3, 4);
    }

    while (1) {
        if (taskxx_cond_all) {
            break;
        }
        liner_scheduler::get_instance().sched();
    }

    info("sched test done.\n");
    return 0;
}
