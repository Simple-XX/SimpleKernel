
/**
 * @file vmm.cpp
 * @brief 虚拟内存头文件
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

#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "assert.h"
#include "cpu.hpp"
#if defined(__i386__) || defined(__x86_64__)
#include "gdt.h"
#endif
#include "pmm.h"
#include "vmm.h"

/// 内核页目录
static pt_t pgd_kernel[COMMON::CORES_COUNT];
pt_t        VMM::curr_dir[COMMON::CORES_COUNT];
spinlock_t  VMM::spinlock;

// 在 _pgd 中查找 _va 对应的页表项
// 如果未找到，_alloc 为真时会进行分配
pte_t *VMM::find(const pt_t _pgd, uintptr_t _va, bool _alloc) {
    pt_t pgd = _pgd;
    // sv39 共有三级页表，一级一级查找
    // -1 是因为最后一级是具体的某一页，在函数最后直接返回
    for (size_t level = VMM_PT_LEVEL - 1; level > 0; level--) {
        // 每次循环会找到 _va 的第 level 级页表 pgd
        // 相当于 pgd_level[VPN_level]，这样相当于得到了第 level 级页表的地址
        pte_t *pte = (pte_t *)&pgd[PX(level, _va)];
        // 解引用 pte，如果有效，获取 level+1 级页表，
        if ((*pte & VMM_PAGE_VALID) == 1) {
            // pgd 指向下一级页表
            // *pte 保存的是页表项，需要转换为对应的物理地址
            pgd = (pt_t)PTE2PA(*pte);
        }
        // 如果无效
        else {
            // 判断是否需要分配
            // 如果需要
            if (_alloc == true) {
                // 申请新的物理页
                pgd = (pt_t)PMM::get_instance().alloc_page_kernel();
                bzero(pgd, COMMON::PAGE_SIZE);
                // 申请失败则返回
                if (pgd == nullptr) {
                    // 如果出现这种情况，说明物理内存不够，一般不会出现
                    assert(0);
                    return nullptr;
                }
                // 清零
                bzero(pgd, COMMON::PAGE_SIZE);
                // 填充页表项
                *pte = PA2PTE((uintptr_t)pgd) | VMM_PAGE_VALID;
            }
            // 不分配的话直接返回
            else {
                return nullptr;
            }
        }
    }
    // 0 最低级 pt
    return &pgd[PX(0, _va)];
}

VMM &VMM::get_instance(void) {
    /// 定义全局 VMM 对象
    static VMM vmm;
    return vmm;
}

bool VMM::init(void) {
    // 初始化自旋锁
    spinlock.init("VMM");
#if defined(__i386__) || defined(__x86_64__)
    GDT::init();
#endif
    // 读取当前页目录
    curr_dir[COMMON::get_curr_core_id(CPU::READ_SP())] = (pt_t)CPU::GET_PGD();
    // 分配一页用于保存页目录
    pgd_kernel[COMMON::get_curr_core_id(CPU::READ_SP())] =
        (pt_t)PMM::get_instance().alloc_page_kernel();
    bzero(pgd_kernel[COMMON::get_curr_core_id(CPU::READ_SP())],
          COMMON::PAGE_SIZE);
    // 映射内核空间
    for (uintptr_t addr = (uintptr_t)COMMON::KERNEL_START_ADDR;
         addr < (uintptr_t)COMMON::KERNEL_START_ADDR + VMM_KERNEL_SPACE_SIZE;
         addr += COMMON::PAGE_SIZE) {
        // TODO: 区分代码/数据等段分别映射
        mmap(pgd_kernel[COMMON::get_curr_core_id(CPU::READ_SP())], addr, addr,
             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE | VMM_PAGE_EXECUTABLE);
    }
    // 设置页目录
    set_pgd(pgd_kernel[COMMON::get_curr_core_id(CPU::READ_SP())]);
    // 开启分页
    CPU::ENABLE_PG();
    info("vmm init.\n");
    return 0;
}

bool VMM::init_other_core(void) {
    // 读取当前页目录
    curr_dir[COMMON::get_curr_core_id(CPU::READ_SP())] = (pt_t)CPU::GET_PGD();
    // 分配一页用于保存页目录
    pgd_kernel[COMMON::get_curr_core_id(CPU::READ_SP())] =
        (pt_t)PMM::get_instance().alloc_page_kernel();
    bzero(pgd_kernel[COMMON::get_curr_core_id(CPU::READ_SP())],
          COMMON::PAGE_SIZE);
    // 映射内核空间
    for (uintptr_t addr = (uintptr_t)COMMON::KERNEL_START_ADDR;
         addr < (uintptr_t)COMMON::KERNEL_START_ADDR + VMM_KERNEL_SPACE_SIZE;
         addr += COMMON::PAGE_SIZE) {
        // TODO: 区分代码/数据等段分别映射
        mmap(pgd_kernel[COMMON::get_curr_core_id(CPU::READ_SP())], addr, addr,
             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE | VMM_PAGE_EXECUTABLE);
    }
    // 设置页目录
    set_pgd(pgd_kernel[COMMON::get_curr_core_id(CPU::READ_SP())]);
    // 开启分页
    CPU::ENABLE_PG();
    info("vmm other init: 0x%X.\n", COMMON::get_curr_core_id(CPU::READ_SP()));
    return 0;
}

pt_t VMM::get_pgd(void) {
    return curr_dir[COMMON::get_curr_core_id(CPU::READ_SP())];
}

void VMM::set_pgd(const pt_t _pgd) {
    spinlock.lock();
    // 更新当前页表
    curr_dir[COMMON::get_curr_core_id(CPU::READ_SP())] = _pgd;
    // 设置页目录
    CPU::SET_PGD((uintptr_t)curr_dir[COMMON::get_curr_core_id(CPU::READ_SP())]);
    // 刷新缓存
    CPU::VMM_FLUSH(0);
    spinlock.unlock();
    return;
}

void VMM::mmap(const pt_t _pgd, uintptr_t _va, uintptr_t _pa, uint32_t _flag) {
    spinlock.lock();
    pte_t *pte = find(_pgd, _va, true);
    // 一般情况下不应该为空
    assert(pte != nullptr);
    // 已经映射过了 且 flag 没有变化
    if (((*pte & VMM_PAGE_VALID) == VMM_PAGE_VALID) &&
        ((*pte & _flag) == _flag)) {
        warn("remap.\n");
    }
    // 没有映射，或更改了 flag
    else {
        // 那么设置 *pte
        // pte 解引用后的值是页表项
        *pte = PA2PTE(_pa) | _flag | VMM_PAGE_VALID;
        // 刷新缓存
        CPU::VMM_FLUSH((uintptr_t)_va);
    }
    spinlock.unlock();
    return;
}

void VMM::unmmap(const pt_t _pgd, uintptr_t _va) {
    spinlock.lock();
    pte_t *pte = find(_pgd, _va, false);
    // 找到页表项
    // 未找到
    if (pte == nullptr) {
        warn("VMM::unmmap: find.\n");
        return;
    }
    // 找到了，但是并没有被映射
    if ((*pte & VMM_PAGE_VALID) == 0) {
        warn("VMM::unmmap: not mapped.\n");
    }
    // 置零
    *pte = 0x00;
    // 刷新缓存
    CPU::VMM_FLUSH((uintptr_t)_va);
    // TODO: 如果一页表都被 unmap，释放占用的物理内存
    spinlock.unlock();
    return;
}

bool VMM::get_mmap(const pt_t _pgd, uintptr_t _va, const void *_pa) {
    spinlock.lock();
    pte_t *pte = find(_pgd, _va, false);
    bool   res = false;
    // pte 不为空且有效，说明映射了
    if ((pte != nullptr) && ((*pte & VMM_PAGE_VALID) == 1)) {
        // 如果 _pa 不为空
        if (_pa != nullptr) {
            // 设置 _pa
            // 将页表项转换为物理地址
            *(uintptr_t *)_pa = PTE2PA(*pte);
        }
        // 返回 true
        res = true;
    }
    // 否则说明没有映射
    else {
        // 如果 _pa 不为空
        if (_pa != nullptr) {
            // 设置 _pa
            *(uintptr_t *)_pa = (uintptr_t) nullptr;
        }
    }
    spinlock.unlock();
    return res;
}
