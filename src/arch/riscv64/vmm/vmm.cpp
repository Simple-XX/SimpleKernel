
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "cpu.hpp"
#include "memlayout.h"
#include "pmm.h"
#include "vmm.h"

#define PA2PTE(pa) ((((uint64_t)pa) >> 12) << 10)
#define PTE2PA(pte) (((pte) >> 10) << 12)
#define PXSHIFT(level) (12 + (9 * (level)))
#define PX(level, va) ((((uint64_t)(va)) >> PXSHIFT(level)) & 0x1FF)

pte_t *walk(pt_t pgd, uint64_t va, bool alloc) {
    for (int level = 2; level > 0; level--) {
        pte_t *pte = (pte_t *)&pgd[PX(level, va)];
        // 如果有效，那么 pgd 指向下一级页表
        if ((*pte & VMM_PAGE_VALID) == 1) {
            pgd = (pt_t)PTE2PA(*pte);
        }
        // 如果无效
        else {
            if (alloc == true) {
                pgd = (pt_t)pmm.alloc_page(1, COMMON::NORMAL);
                if (pgd == nullptr) {
                    return nullptr;
                }
            }
            else {
                return nullptr;
            }
            bzero(pgd, COMMON::PAGE_SIZE);
            *pte = PA2PTE(pgd) | VMM_PAGE_VALID;
        }
    }
    // 0 最低级 pt
    return &pgd[PX(0, va)];
}

static pt_t pgd_kernel;

VMM::VMM(void) {
    curr_dir = (pgd_t)CPU::READ_SATP();
    return;
}

VMM::~VMM(void) {
    return;
}

int32_t VMM::init(void) {
    pgd_kernel = (pt_t)pmm.alloc_page(1, COMMON::NORMAL);
    for (uint64_t addr = (uint64_t)COMMON::KERNEL_START_ADDR;
         addr < (uint64_t)COMMON::KERNEL_START_ADDR + VMM_KERNEL_SIZE;
         addr += COMMON::PAGE_SIZE) {
        mmap(pgd_kernel, (void *)addr, (void *)addr,
             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE | VMM_PAGE_EXECUTABLE);
    }
    set_pgd(pgd_kernel);
    CPU::ENABLE_PG();
    printf("vmm_init.\n");
    return 0;
}

pgd_t VMM::get_pgd(void) const {
    return curr_dir;
}

void VMM::set_pgd(const pgd_t pgd) {
    curr_dir = pgd;
    CPU::WRITE_SATP(curr_dir);
    CPU::SFENCE_VMA();
    return;
}

void VMM::mmap(const pgd_t pgd, const void *va, const void *pa,
               const uint32_t flag) {
    pte_t *pte;
    if ((pte = walk(pgd, (uint64_t)va, true)) == 0) {
        return;
    }
    if (*pte & VMM_PAGE_VALID) {
        info("remap");
    }
    *pte = PA2PTE(pa) | flag | VMM_PAGE_VALID;
    CPU::SFENCE_VMA();
    return;
}

void VMM::unmmap(const pgd_t pgd, const void *va) {
    pte_t *pte;
    if ((pte = walk(pgd, (uint64_t)va, false)) == 0) {
        info("uvmunmap: walk");
    }
    if ((*pte & VMM_PAGE_VALID) == 0) {
        info("uvmunmap: not mapped");
    }
    *pte = 0x00;
    CPU::SFENCE_VMA();
    return;
}

uint32_t VMM::get_mmap(const pgd_t pgd, const void *va, const void *pa) {
    pte_t *pte;
    if ((pte = walk(pgd, (uint64_t)va, false)) == 0) {
        if (pa != nullptr) {
            *(uint64_t *)pa = (uint64_t) nullptr;
        }
        return 0;
    }
    if ((*pte & VMM_PAGE_VALID) == 0) {
        if (pa != nullptr) {
            *(uint64_t *)pa = (uint64_t) nullptr;
        }
        return 0;
    }
    if (pa != nullptr) {
        *(uint64_t *)pa = (uint64_t)(PTE2PA(*pte));
    }
    return 1;
}

VMM vmm;
