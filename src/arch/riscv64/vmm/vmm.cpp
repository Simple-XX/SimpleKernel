
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "assert.h"
#include "cpu.hpp"
#include "memlayout.h"
#include "pmm.h"
#include "vmm.h"

// 物理地址转换到页表项
#define PA2PTE(_pa) ((((uint64_t)_pa) >> 12) << 10)
// 页表项转换到物理地址
#define PTE2PA(pte) (((pte) >> 10) << 12)
// 计算 X 级页表的偏移
#define PXSHIFT(level) (12 + (9 * (level)))
// 获取 X 级页表索引
#define PX(level, _va) ((((uint64_t)(_va)) >> PXSHIFT(level)) & 0x1FF)

// 在 _pgd 中查找 _va 对应的页表项
// 如果未找到，_alloc 为真时会进行分配
pte_t *walk(pt_t _pgd, uint64_t _va, bool _alloc) {
    // sv39 共有三级页表，一级一级查找
    for (size_t level = 2; level > 0; level--) {
        pte_t *pte = (pte_t *)&_pgd[PX(level, _va)];
        // 如果有效，那么 _pgd 指向下一级页表
        if ((*pte & VMM_PAGE_VALID) == 1) {
            _pgd = (pt_t)PTE2PA(*pte);
        }
        // 如果无效
        else {
            // 分配
            if (_alloc == true) {
                // 申请新的物理页
                _pgd = (pt_t)PMM::alloc_page_kernel();
                // 申请失败则返回
                if (_pgd == nullptr) {
                    assert(0);
                    return nullptr;
                }
                // 清零
                bzero(_pgd, COMMON::PAGE_SIZE);
                // 填充页表项
                *pte = PA2PTE(_pgd) | VMM_PAGE_VALID;
            }
            // 不分配的话直接返回
            else {
                return nullptr;
            }
        }
    }
    // 0 最低级 pt
    return &_pgd[PX(0, _va)];
}

static pt_t pgd_kernel;
pgd_t       VMM::curr_dir;

VMM::VMM(void) {
    // 读取当前页目录
    curr_dir = (pgd_t)CPU::READ_SATP();
    return;
}

VMM::~VMM(void) {
    return;
}

bool VMM::init(void) {
    // 分配一页用于保存页目录
    pgd_kernel = (pt_t)PMM::alloc_page_kernel();
    // 映射内核空间
    for (uint64_t addr = (uint64_t)COMMON::KERNEL_START_ADDR;
         addr < (uint64_t)COMMON::KERNEL_START_ADDR + VMM_KERNEL_SPACE_SIZE;
         addr += COMMON::PAGE_SIZE) {
        // TODO: 区分代码/数据等段分别映射
        mmap(pgd_kernel, (void *)addr, (void *)addr,
             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE | VMM_PAGE_EXECUTABLE);
    }
    // 设置页目录
    set_pgd(pgd_kernel);
    // 开启分页
    CPU::ENABLE_PG();
    printf("vmm_init.\n");
    return 0;
}

pgd_t VMM::get_pgd(void) {
    return curr_dir;
}

void VMM::set_pgd(const pgd_t _pgd) {
    // 更新当前页表
    curr_dir = _pgd;
    // 设置页目录
    CPU::WRITE_SATP(curr_dir);
    // 刷新缓存
    CPU::SFENCE_VMA();
    return;
}

void VMM::mmap(const pgd_t _pgd, const void *_va, const void *_pa,
               const uint32_t _flag) {
    pte_t *pte = nullptr;
    // 如果未找到对应页，直接返回
    if ((pte = walk(_pgd, (uint64_t)_va, true)) == nullptr) {
        return;
    }
    // 已经映射过了
    if (*pte & VMM_PAGE_VALID) {
        info("remap.\n");
    }
    // 没有问题，那么设置 flags
    *pte = PA2PTE(_pa) | _flag | VMM_PAGE_VALID;
    // 刷新缓存
    CPU::SFENCE_VMA();
    return;
}

void VMM::unmmap(const pgd_t _pgd, const void *_va) {
    pte_t *pte = nullptr;
    // 找到页表项
    // 未找到
    if ((pte = walk(_pgd, (uint64_t)_va, false)) == nullptr) {
        warn("VMM::unmmap: walk.\n");
        return;
    }
    // 找到了，但是并没有被映射
    if ((*pte & VMM_PAGE_VALID) == 0) {
        warn("VMM::unmmap: not mapped.\n");
    }
    // 置零
    *pte = 0x00;
    // 刷新缓存
    CPU::SFENCE_VMA();
    return;
}

// TODO: 优化
bool VMM::get_mmap(const pgd_t _pgd, const void *_va, const void *_pa) {
    pte_t *pte = nullptr;
    // 如果没有找到对应的页表项
    if ((pte = walk(_pgd, (uint64_t)_va, false)) == nullptr) {
        // 如果 _pa 不为空
        if (_pa != nullptr) {
            // 设置 _pa
            *(uint64_t *)_pa = (uint64_t) nullptr;
        }
        return false;
    }
    // 找到了，但是没有被映射
    if ((*pte & VMM_PAGE_VALID) == 0) {
        // 如果 _pa 不为空
        if (_pa != nullptr) {
            // 设置 _pa
            *(uint64_t *)_pa = (uint64_t) nullptr;
        }
        return false;
    }
    // 这里说明找到了且有效
    // 如果 _pa 不为空
    if (_pa != nullptr) {
        // 设置 _pa
        *(uint64_t *)_pa = (uint64_t)(PTE2PA(*pte));
    }
    return true;
}
