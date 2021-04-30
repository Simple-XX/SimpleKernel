
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
static constexpr const uint32_t PTE_BITS = 12;
// PTE idx 的偏移
static constexpr const uint32_t PTE_SHIFT = 0;
static constexpr const uint32_t PTE_MASK  = 0xFFF;
// PMD idx 的位数
static constexpr const uint32_t PMD_BITS = 9;
// PMD idx 的偏移
static constexpr const uint32_t PMD_SHIFT = 12;
static constexpr const uint32_t PMD_MASK  = 0x1FF;
// PUD idx 的位数
static constexpr const uint32_t PUD_BITS = 9;
// PUD idx 的偏移
static constexpr const uint32_t PUD_SHIFT = 21;
static constexpr const uint32_t PUD_MASK  = 0x1FF;
// PGD idx 的位数
static constexpr const uint32_t PGD_BITS = 9;
// PGD idx 的偏移
static constexpr const uint32_t PGD_SHIFT = 30;
static constexpr const uint32_t PGD_MASK  = 0x1FF;

static constexpr uint32_t GET_PTE(uint32_t addr) {
    return (addr >> PTE_SHIFT) & PTE_MASK;
}

static constexpr uint32_t GET_PMD(uint32_t addr) {
    return (addr >> PMD_SHIFT) & PMD_MASK;
}

static constexpr uint32_t GET_PUD(uint32_t addr) {
    return (addr >> PUD_SHIFT) & PUD_MASK;
}

static constexpr uint32_t GET_PGD(uint32_t addr) {
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

void VMM::init(void) {
    // 映射物理地址前 64MB 到虚拟地址前 64MB
    for (uint64_t addr = 0x80000000; addr < 0x80000000 + COMMON::KERNEL_SIZE;
         addr += COMMON::PAGE_SIZE) {
        mmap((pgd_t)pgd_kernel, (void *)addr, (void *)addr,
             VMM_PAGE_READABLE | VMM_PAGE_EXECUTABLE);
    }
    // 虚拟地址 0x00 设为 nullptr
    unmmap((pgd_t)pgd_kernel, nullptr);

    set_pgd((pgd_t)pgd_kernel);
    void *a = CPU::READ_SATP();
    printf("a: 0x%X, pgd_kernel: 0x%X\n", a, pgd_kernel);
    CPU::SFENCE_VMA();
    printf("vmm_init\n");
    return;
}

pgd_t VMM::get_pgd(void) const {
    return (pgd_t)CPU::MAKE_SATP(curr_dir);
}

void VMM::set_pgd(const pgd_t pgd) {
    curr_dir = pgd;
    CPU::WRITE_SATP(CPU::MAKE_SATP(curr_dir));
    return;
}

void VMM::mmap(const pgd_t pgd, const void *va, const void *pa,
               const uint32_t flag) {
    uint64_t pgd_idx = GET_PGD(reinterpret_cast<uint64_t>(va));
    uint64_t pud_idx = GET_PUD(reinterpret_cast<uint64_t>(va));
    uint64_t pmd_idx = GET_PMD(reinterpret_cast<uint64_t>(va));
// #define DEBUG
#ifdef DEBUG
    io.info("pgd_idx: 0x%X, pud_idx: 0x%X, pmd_idx: 0x%X, k: %d\n", pgd_idx,
            pud_idx, pmd_idx, k);
#undef DEBUG
#endif
    // 填充 pgd 内容
    pud_t pud = (pud_t)(pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pud == nullptr) {
        pud          = (pud_t)pmm.alloc_page(1, COMMON::NORMAL);
        pgd[pgd_idx] = (reinterpret_cast<uint64_t>(pud) | VMM_PAGE_VALID);
    }
    // 填充 pud 内容
    pmd_t pmd = (pmd_t)(pud[pud_idx] & COMMON::PAGE_MASK);
    if (pmd == nullptr) {
        pmd          = (pmd_t)pmm.alloc_page(1, COMMON::NORMAL);
        pud[pud_idx] = (reinterpret_cast<uint64_t>(pmd) | VMM_PAGE_VALID);
    }
    if (pa == nullptr) {
        pa = pmm.alloc_page(1, COMMON::HIGH);
    }
    // 填充 pmd 内容
    pmd[pmd_idx] = ((reinterpret_cast<uint64_t>(pa) & COMMON::PAGE_MASK) |
                    flag | VMM_PAGE_VALID);
    return;
}

void VMM::unmmap(const pgd_t pgd, const void *va) {
    uint64_t pgd_idx = GET_PGD(reinterpret_cast<uint64_t>(va));
    uint64_t pud_idx = GET_PUD(reinterpret_cast<uint64_t>(va));
    uint64_t pmd_idx = GET_PMD(reinterpret_cast<uint64_t>(va));
    pud_t    pud     = (pud_t)((uint64_t)pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pud == nullptr) {
        printf("pud == nullptr\n");
        return;
    }
    pmd_t pmd = (pmd_t)((uint64_t)pud[pud_idx] & COMMON::PAGE_MASK);
    if (pmd == nullptr) {
        printf("pmd == nullptr\n");
        return;
    }
    pmd[pmd_idx] = 0;
    // TODO: 如果一页表都被 unmap，释放占用的物理内存
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
