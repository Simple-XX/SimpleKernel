
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "cpu.hpp"
#include "pmm.h"
#include "vmm.h"

// 64-ia-32-architectures-software-developer-vol-3a-manual#4.3

// 物理地址转换到页表项
// 页表项结构：
// 0~11: pte 属性
// 12~31: 页表的物理页地址
static constexpr uintptr_t PA2PTE(const void *_pa) {
    return ((uintptr_t)_pa) & (~0x3FF);
}
// 页表项转换到物理地址
static constexpr uintptr_t PTE2PA(const pte_t _pte) {
    return ((uintptr_t)_pte) & (~0x3FF);
}
// 计算 X 级页表的偏移
// 9: 虚拟地址 VPN 的长度，均为 9 位
// 12: 页内偏移，12 位
static constexpr uintptr_t PXSHIFT(const size_t _level) {
    return 12 + (9 * _level);
}
// 获取 _va 的第 _level 级 VPN
// 虚拟地址右移 12+(10 * _level) 位，
// 得到的就是第 _level 级页表的 VPN
// 0x1FF: 9 位
static constexpr uintptr_t PX(size_t _level, const void *_va) {
    return (((uintptr_t)(_va)) >> PXSHIFT(_level)) & 0x1FF;
}

// x86_64 使用了四级页表
static constexpr const size_t PT_LEVEL = 4;

// 在 _pgd 中查找 _va 对应的页表项
// 如果未找到，_alloc 为真时会进行分配
pte_t *find(const pt_t _pgd, const void *_va, bool _alloc) {
    pt_t pgd = _pgd;
    // sv39 共有三级页表，一级一级查找
    // -1 是因为最后一级是具体的某一页，在函数最后直接返回
    for (size_t level = PT_LEVEL - 1; level > 0; level--) {
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
                pgd = (pt_t)PMM::alloc_page_kernel();
                // 申请失败则返回
                if (pgd == nullptr) {
                    // 如果出现这种情况，说明物理内存不够，一般不会出现
                    assert(0);
                    return nullptr;
                }
                // 清零
                bzero(pgd, COMMON::PAGE_SIZE);
                // 填充页表项
                *pte = PA2PTE(pgd) | VMM_PAGE_VALID;
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

static pt_t pgd_kernel;
pt_t        VMM::curr_dir;

VMM::VMM(void) {
    curr_dir = (pt_t)CPU::READ_CR3();
    return;
}

VMM::~VMM(void) {
    return;
}

// BUG: alloc_page_kernel 会分配 2m 内存
bool VMM::init(void) {
    // 分配一页用于保存页目录
    pgd_kernel = (pt_t)PMM::alloc_page_kernel();
    // 映射内核空间
    for (uintptr_t addr = (uintptr_t)COMMON::KERNEL_START_ADDR;
         addr < (uintptr_t)COMMON::KERNEL_START_ADDR + VMM_KERNEL_SPACE_SIZE;
         addr += COMMON::PAGE_SIZE) {
        // TODO: 区分代码/数据等段分别映射
        mmap(pgd_kernel, (void *)addr, (void *)addr,
             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE | VMM_PAGE_EXECUTABLE);
    }
    set_pgd(pgd_kernel);
    CPU::ENABLE_PG();
    printf("vmm init.\n");
    return 0;
}

pt_t VMM::get_pgd(void) {
    return curr_dir;
}

void VMM::set_pgd(const pt_t _pgd) {
    curr_dir = _pgd;
    CPU::CR3_SET_PGD((void *)curr_dir);
    return;
}

void VMM::mmap(const pt_t _pgd, const void *_va, const void *_pa,
               uint32_t _flag) {
    pte_t *pte = find(_pgd, _va, true);
    // 一般情况下不应该为空
    assert(pte != nullptr);
    // 已经映射过了
    if (*pte & VMM_PAGE_VALID) {
        warn("remap.\n");
    }
    // 没有映射，这是正常情况
    else {
        // 那么设置 *pte
        // pte 解引用后的值是页表项
        *pte = PA2PTE(_pa) | _flag | VMM_PAGE_VALID;
        // 刷新缓存
        CPU::INVLPG(_va);
    }
    return;
}

void VMM::unmmap(const pt_t _pgd, const void *_va) {
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
    CPU::INVLPG(_va);
    // TODO: 如果一页表都被 unmap，释放占用的物理内存
    return;
}

bool VMM::get_mmap(const pt_t _pgd, const void *_va, const void *_pa) {
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
    return res;
}
