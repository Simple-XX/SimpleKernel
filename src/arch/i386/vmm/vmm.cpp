
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "cpu.hpp"
#include "vmm.h"

// TODO: 结合这几个结构给出更直观的写法
// Format of a 32-Bit Page-Table Entry that Maps a 4-KByte Page
class page_table32_entry_t {
private:
    IO io;

public:
    // Present; must be 1 to map a 4-KByte page
    uint32_t p : 1;
    // Read/write; if 0, writes may not be allowed to the 4-KByte page
    // referenced by this entry
    uint32_t rw : 1;
    // User/supervisor; if 0, user-mode accesses are not allowed to the 4-KByte
    // page referenced by this entry
    uint32_t us : 1;
    // Page-level write-through; indirectly determines the memory type used to
    // access the 4-KByte page referenced by this entry
    uint32_t pwt : 1;
    // Page-level cache disable; indirectly determines the memory type used to
    // access the 4-KByte page referenced by this entry
    uint32_t pcd : 1;
    // Accessed; indicates whether software has accessed the 4-KByte page
    // referenced by this entry
    uint32_t a : 1;
    // Dirty; indicates whether software has written to the 4-KByte page
    // referenced by this entry
    uint32_t d : 1;
    // If the PAT is supported, indirectly determines the memory type used to
    // access the 4-KByte page referenced by this entry;
    // otherwise, reserved (must be 0)
    uint32_t pat : 1;
    // Global; if CR4.PGE = 1, determines whether the translation is global (see
    // Section 4.10); ignored otherwise
    uint32_t g : 1;
    // Ignored
    uint32_t ignore : 3;
    // Physical address of the 4-KByte page referenced by this entry
    uint32_t addr : 20;
    void     printf(void) {
        io.printf("%05X %d %d%d%d%d%d%d %d%d%d", addr, ignore, g, pat, d, a,
                  pcd, pwt, us, rw, p);
    }
};

// Format of a 32-Bit Page-Directory Entry that References a Page Table
class page_dir32_entry_t {
private:
    IO io;

public:
    // Present; must be 1 to reference a page table
    uint32_t p : 1;
    // Read/write; if 0, writes may not be allowed to the 4-MByte region
    // controlled by this entry
    uint32_t rw : 1;
    // User/supervisor; if 0, user-mode accesses are not allowed to the 4-MByte
    // region controlled by this entry
    uint32_t us : 1;
    // Page-level write-through; indirectly determines the memory type used to
    // access the page table referenced by this entry
    uint32_t pwt : 1;
    // Page-level cache disable; indirectly determines the memory type used to
    // access the page table referenced by this entry
    uint32_t pcd : 1;
    // Accessed; indicates whether this entry has been used for linear-address
    // translation
    uint32_t a : 1;
    //  Ignored
    uint32_t ignore1 : 1;
    // If CR4.PSE = 1, must be 0 (otherwise, this entry maps a 4-MByte page)
    // otherwise, ignored
    uint32_t ps : 1;
    // Ignored
    uint32_t ignore2 : 4;
    // Physical address of 4-KByte aligned page table referenced by this entry
    uint32_t addr : 20;
    void     printf(void) {
        io.printf("%05X %d %d %d %d%d%d %d%d%d", addr, ignore2, ps, ignore1, a,
                  pcd, pwt, us, rw, p);
    }
};

// PTE idx 的位数
static constexpr const uint32_t PTE_BITS = 12;
// PTE idx 的偏移
static constexpr const uint32_t PTE_SHIFT = 0;
static constexpr const uint32_t PTE_MASK  = 0xFFF;
// PMD idx 的位数
static constexpr const uint32_t PMD_BITS = 0;
// PMD idx 的偏移
static constexpr const uint32_t PMD_SHIFT = 0;
static constexpr const uint32_t PMD_MASK  = 0;
// PUD idx 的位数
static constexpr const uint32_t PUD_BITS = 10;
// PUD idx 的偏移
static constexpr const uint32_t PUD_SHIFT = 12;
static constexpr const uint32_t PUD_MASK  = 0x3FF;
// PGD idx 的位数
static constexpr const uint32_t PGD_BITS = 10;
// PGD idx 的偏移
static constexpr const uint32_t PGD_SHIFT = 22;
static constexpr const uint32_t PGD_MASK  = 0x3FF;

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

IO         VMM::io;
PMM        VMM::pmm;
page_dir_t VMM::pgd_kernel[VMM_PAGE_TABLES_TOTAL];

VMM::VMM(void) {
    curr_dir = (page_dir_t)CPU::READ_CR3();
    return;
}

VMM::~VMM(void) {
    return;
}

void VMM::init(void) {
    // 映射物理地址前 32MB 到虚拟地址前 32MB
    for (uint32_t addr = 0; addr < VMM_KERNEL_SIZE; addr += COMMON::PAGE_SIZE) {
        mmap((page_dir_t)pgd_kernel, (void *)addr, (void *)addr,
             VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL);
    }
    // 虚拟地址 0x00 设为 nullptr
    unmmap((page_dir_t)pgd_kernel, nullptr);

// #define DEBUG
#ifdef DEBUG
    io.printf("&pte_kernel[0]: 0x%X, pte_kernel[0]: %X\n", &pte_kernel[0],
              pte_kernel[0]);
    io.printf("&pgd_kernel[0]: 0x%X, pgd_kernel[0]: 0x%X\n", &pgd_kernel[0],
              pgd_kernel[0]);
#undef DEBUG
#endif
    set_pgd((page_dir_t)pgd_kernel);
    CPU::CR0_SET_PG();
    io.printf("vmm_init\n");
    return;
}

page_dir_t VMM::get_pgd(void) const {
    return curr_dir;
}

void VMM::set_pgd(const page_dir_t pgd) {
    curr_dir = pgd;
    CPU::CR3_SET_PGD((void *)curr_dir);
    return;
}

void VMM::mmap(const page_dir_t pgd, const void *va, const void *pa,
               const uint32_t flag) {
    uint32_t     pgd_idx = GET_PGD(reinterpret_cast<uint32_t>(va));
    uint32_t     pte_idx = GET_PUD(reinterpret_cast<uint32_t>(va));
    page_table_t pt =
        (page_table_t)((uint32_t)pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pt == nullptr) {
        pt           = (page_table_t)pmm.alloc_page(1, COMMON::NORMAL);
        pgd[pgd_idx] = (page_dir_entry_t)(reinterpret_cast<uint32_t>(pt) |
                                          VMM_PAGE_PRESENT | VMM_PAGE_RW);
    }
    pt = (page_table_t)VMM_PA_LA(pt);
    if (pa == nullptr) {
        pa = pmm.alloc_page(1, COMMON::HIGH);
    }
    pt[pte_idx] = (page_table_entry_t)(
        (reinterpret_cast<uint32_t>(pa) & COMMON::PAGE_MASK) | flag);
// #define DEBUG
#ifdef DEBUG
    io.info("pt[pte_idx]: 0x%X, &: 0x%X, pa: 0x%X\n", pt[pte_idx], &pt[pte_idx],
            pa);
#undef DEBUG
#endif
    CPU::INVLPG(va);
    return;
}

void VMM::unmmap(const page_dir_t pgd, const void *va) {
    uint32_t     pgd_idx = GET_PGD(reinterpret_cast<uint32_t>(va));
    uint32_t     pte_idx = GET_PUD(reinterpret_cast<uint32_t>(va));
    page_table_t pt =
        (page_table_t)((uint32_t)pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pt == nullptr) {
        io.printf("pt == nullptr\n");
        return;
    }
    pt          = (page_table_t)VMM_PA_LA(pt);
    pt[pte_idx] = nullptr;
    // TODO: 如果一页表都被 unmap，释放占用的物理内存
    CPU::INVLPG(va);
}

uint32_t VMM::get_mmap(const page_dir_t pgd, const void *va, const void *pa) {
    uint32_t     pgd_idx = GET_PGD(reinterpret_cast<uint32_t>(va));
    uint32_t     pte_idx = GET_PUD(reinterpret_cast<uint32_t>(va));
    page_table_t pt =
        (page_table_t)((uint32_t)pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pt == nullptr) {
        if (pa != nullptr) {
            *(uint32_t *)pa = (uint32_t) nullptr;
        }
        return 0;
    }
    pt = (page_table_t)(VMM_PA_LA(pt));
    if (pt[pte_idx] != nullptr) {
        if (pa != nullptr) {
            *(uint32_t *)pa = (uint32_t)pt[pte_idx] & COMMON::PAGE_MASK;
        }
        return 1;
    }
    else {
        if (pa != nullptr) {
            *(uint32_t *)pa = (uint32_t) nullptr;
        }
        return 0;
    }
}
