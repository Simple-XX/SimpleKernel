
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

// class page_table_entry_t {
// public:
//     uint64_t v : 1;
//     uint64_t r : 1;
//     uint64_t w : 1;
//     uint64_t x : 1;
//     uint64_t u : 1;
//     uint64_t g : 1;
//     uint64_t a : 1;
//     uint64_t d : 1;
//     uint64_t rsw : 2;
//     uint64_t ppn0 : 9;
//     uint64_t ppn1 : 9;
//     uint64_t ppn2 : 26;
//     uint64_t reserved : 10;
// };

// 获取一个地址的 63−39，9 位
// 63−39 位的值必须等于第 38
// 位的值，否则会认为该虚拟地址不合法，在访问时会产生异常。
#define VMM_VPN_RESERVED(x) (((x) >> 39) & 0x3FFFFFF)

// PTE idx 的位数
static constexpr const uint64_t PTE_BITS = 10;
// PTE idx 的偏移
static constexpr const uint64_t PTE_SHIFT = 0;
static constexpr const uint64_t PTE_MASK  = 0x3FF;
// PMD idx 的位数
static constexpr const uint64_t PMD_BITS = 9;
// PMD idx 的偏移
static constexpr const uint64_t PMD_SHIFT = 10;
static constexpr const uint64_t PMD_MASK  = 0x1FF;
// PUD idx 的位数
static constexpr const uint64_t PUD_BITS = 9;
// PUD idx 的偏移
static constexpr const uint64_t PUD_SHIFT = 19;
static constexpr const uint64_t PUD_MASK  = 0x1FF;
// PGD idx 的位数
static constexpr const uint64_t PGD_BITS = 26;
// PGD idx 的偏移
static constexpr const uint64_t PGD_SHIFT = 28;
static constexpr const uint64_t PGD_MASK  = 0x3FFFFFF;

static constexpr uint64_t GET_PTE(uint64_t addr) {
    return (addr >> PTE_SHIFT) & PTE_MASK;
}

static constexpr uint64_t GET_PMD(uint64_t addr) {
    return (addr >> PMD_SHIFT) & PMD_MASK;
}

static constexpr uint64_t GET_PUD(uint64_t addr) {
    return (addr >> PUD_SHIFT) & PUD_MASK;
}

static constexpr uint64_t GET_PGD(uint64_t addr) {
    return (addr >> PGD_SHIFT) & PGD_MASK;
}

// 页目录，最高级
static pgd_t pgd_kernel[VMM_PAGES_PRE_PAGE_TABLE]
    __attribute__((aligned(COMMON::PAGE_SIZE)));

VMM::VMM(void) {
    curr_dir = (pgd_t)CPU::READ_SATP();
    return;
}

VMM::~VMM(void) {
    return;
}

int32_t VMM::init(void) {
    // 映射物理地址前 64MB 到虚拟地址前 64MB
    for (uint64_t addr = COMMON::KERNEL_BASE;
         addr < COMMON::KERNEL_BASE + COMMON::KERNEL_SIZE;
         addr += COMMON::PAGE_SIZE) {
        mmap((pgd_t)pgd_kernel, (void *)addr, (void *)addr,
             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE | VMM_PAGE_EXECUTABLE);
    }
    // 虚拟地址 0x00 设为 nullptr
    unmmap((pgd_t)pgd_kernel, nullptr);
    // uint64_t va      = 0x80200000;
    // uint64_t pgd_idx = GET_PGD(reinterpret_cast<uint64_t>(va));
    // uint64_t pud_idx = GET_PUD(reinterpret_cast<uint64_t>(va));
    // uint64_t pmd_idx = GET_PMD(reinterpret_cast<uint64_t>(va));
    // printf("pgd[%d]: 0x%p\n", pgd_idx, pgd_kernel[pgd_idx]);
    // uint64_t *p1 = (uint64_t *)pgd_kernel[pgd_idx];
    // printf("pud[%d]: 0x%p\n", pud_idx, p1[pud_idx]);
    // uint64_t *p2 = (uint64_t *)p1[pud_idx];
    // printf("pmd[%d]: 0x%p\n", pmd_idx, p2[pmd_idx]);
    set_pgd((pgd_t)pgd_kernel);
    printf("SATP: 0x%p\n", pgd_kernel);
    printf("SATP: 0x%p\n", CPU::READ_SATP());
    CPU::SFENCE_VMA();
    printf("vmm_init\n");
    return 0;
}

pgd_t VMM::get_pgd(void) const {
    return curr_dir;
}

void VMM::set_pgd(const pgd_t pgd) {
    curr_dir = pgd;
    CPU::WRITE_SATP(CPU::SET_SV39(curr_dir));
    return;
}

void VMM::mmap(const pgd_t pgd, const void *va, const void *pa,
               const uint32_t flag) {
    uint64_t pgd_idx = GET_PGD(reinterpret_cast<uint64_t>(va));
    uint64_t pud_idx = GET_PUD(reinterpret_cast<uint64_t>(va));
    uint64_t pmd_idx = GET_PMD(reinterpret_cast<uint64_t>(va));
// #define DEBUG
#ifdef DEBUG
    printf("pgd_idx: %p, pud_idx: %p, pmd_idx: %p\n", pgd_idx, pud_idx,
           pmd_idx);
#undef DEBUG
#endif
    // 填充 pgd 内容
    pud_t pud = (pud_t)(pgd[pgd_idx] & COMMON::PAGE_MASK);
// #define DEBUG
#ifdef DEBUG
    printf("pud: %p\n", pud);
#undef DEBUG
#endif
    if (pud == nullptr) {
        pud          = (pud_t)pmm.alloc_page(1, COMMON::NORMAL);
        pud          = (pud_t)(((uint64_t)pud >> 12) << 10);
        pgd[pgd_idx] = (reinterpret_cast<uint64_t>(pud) | VMM_PAGE_VALID);
// #define DEBUG
#ifdef DEBUG
        printf("pgd[%d]: %p\n", pgd_idx, pgd[pgd_idx]);
#undef DEBUG
#endif
    }
    pud = (pud_t)(((uint64_t)pud >> 10) << 12);
    // 填充 pud 内容
    pmd_t pmd = (pmd_t)(pud[pud_idx] & COMMON::PAGE_MASK);
// #define DEBUG
#ifdef DEBUG
    printf("pmd: %p\n", pmd);
#undef DEBUG
#endif
    if (pmd == nullptr) {
        pmd          = (pmd_t)pmm.alloc_page(1, COMMON::NORMAL);
        pmd          = (pmd_t)(((uint64_t)pmd >> 12) << 10);
        pud[pud_idx] = (reinterpret_cast<uint64_t>(pmd) | VMM_PAGE_VALID);
// #define DEBUG
#ifdef DEBUG
        printf("pud[%d]: %p\n", pud_idx, pud[pud_idx]);
#undef DEBUG
#endif
    }
    pmd = (pmd_t)(((uint64_t)pmd >> 10) << 12);
    if (pa == nullptr) {
        pa = pmm.alloc_page(1, COMMON::HIGH);
// #define DEBUG
#ifdef DEBUG
        printf("pa: 0x%p\n", pa);
#undef DEBUG
#endif
    }
    // 填充 pmd 内容
    pmd[pmd_idx] = ((reinterpret_cast<uint64_t>(pa) & COMMON::PAGE_MASK) |
                    flag | VMM_PAGE_VALID);
// #define DEBUG
#ifdef DEBUG
    printf("pmd[%d]: %p\n", pmd_idx, pmd[pmd_idx]);
#undef DEBUG
#endif
    CPU::SFENCE_VMA();
    return;
}

void VMM::unmmap(const pgd_t pgd, const void *va) {
    uint64_t pgd_idx = GET_PGD(reinterpret_cast<uint64_t>(va));
    uint64_t pud_idx = GET_PUD(reinterpret_cast<uint64_t>(va));
    uint64_t pmd_idx = GET_PMD(reinterpret_cast<uint64_t>(va));
    pud_t    pud     = (pud_t)(pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pud == nullptr) {
        printf("unmmap: pud == nullptr\n");
        return;
    }
    pmd_t pmd = (pmd_t)(pud[pud_idx] & COMMON::PAGE_MASK);
    if (pmd == nullptr) {
        printf("unmmap: pmd == nullptr\n");
        return;
    }
    pmd[pmd_idx] = 0;
    // TODO: 如果一页表都被 unmap，释放占用的物理内存
    CPU::SFENCE_VMA();
    return;
}

uint32_t VMM::get_mmap(const pgd_t pgd, const void *va, const void *pa) {
    uint64_t pgd_idx = GET_PGD(reinterpret_cast<uint64_t>(va));
    uint64_t pud_idx = GET_PUD(reinterpret_cast<uint64_t>(va));
    uint64_t pmd_idx = GET_PMD(reinterpret_cast<uint64_t>(va));

    pud_t pud = (pud_t)((uint64_t)pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pud == nullptr) {
        if (pa != nullptr) {
            *(uint64_t *)pa = (uint64_t) nullptr;
        }
        return 0;
    }
    pmd_t pmd = (pmd_t)((uint64_t)pud[pud_idx] & COMMON::PAGE_MASK);
    if (pmd == nullptr) {
        if (pa != nullptr) {
            *(uint64_t *)pa = (uint64_t) nullptr;
        }
        return 0;
    }
    if (pmd[pmd_idx] != 0) {
        if (pa != nullptr) {
            *(uint64_t *)pa = (uint64_t)pmd[pmd_idx] & COMMON::PAGE_MASK;
        }
        return 1;
    }
    else {
        if (pa != nullptr) {
            *(uint64_t *)pa = (uint64_t) nullptr;
        }
        return 0;
    }
}
