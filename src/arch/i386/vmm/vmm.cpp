
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

// TODO: 结合这几个结构给出更直观的写法
// Format of a 32-Bit Page-Table Entry that Maps a 4-KByte Page
class page_table32_entry_t {
private:
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
};

// Format of a 32-Bit Page-Directory Entry that References a Page Table
class page_dir32_entry_t {
private:
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

static pt_t pgd_kernel;
pgd_t       VMM::curr_dir;

VMM::VMM(void) {
    curr_dir = (pgd_t)CPU::READ_CR3();
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
    set_pgd((pgd_t)pgd_kernel);
    CPU::ENABLE_PG();
    printf("vmm init.\n");
    return 0;
}

pgd_t VMM::get_pgd(void) {
    return curr_dir;
}

void VMM::set_pgd(const pgd_t pgd) {
    curr_dir = pgd;
    CPU::CR3_SET_PGD((void *)curr_dir);
    return;
}

void VMM::mmap(const pgd_t pgd, const void *va, const void *pa,
               const uint32_t flag) {
    uint32_t pgd_idx = GET_PGD(reinterpret_cast<uint32_t>(va));
    uint32_t pud_idx = GET_PUD(reinterpret_cast<uint32_t>(va));
    pt_t     pud     = (pt_t)(pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pud == nullptr) {
        pud          = (pt_t)PMM::alloc_page_kernel();
        pgd[pgd_idx] = (reinterpret_cast<uint32_t>(pud) | VMM_PAGE_VALID |
                        VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    }
    pud = (pt_t)VMM_PA_LA(pud);
    if (pa == nullptr) {
        pa = PMM::alloc_page_kernel();
    }
    pud[pud_idx] = ((reinterpret_cast<uint32_t>(pa) & COMMON::PAGE_MASK) |
                    VMM_PAGE_VALID | flag);
// #define DEBUG
#ifdef DEBUG
    info("pud[pud_idx]: 0x%X, &: 0x%X, pa: 0x%X\n", pud[pud_idx], &pud[pud_idx],
         pa);
#undef DEBUG
#endif
    CPU::INVLPG(va);
    return;
}

void VMM::unmmap(const pgd_t pgd, const void *va) {
    uint32_t pgd_idx = GET_PGD(reinterpret_cast<uint32_t>(va));
    uint32_t pud_idx = GET_PUD(reinterpret_cast<uint32_t>(va));
    pt_t     pud     = (pt_t)(pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pud == nullptr) {
        printf("pud == nullptr\n");
        return;
    }
    pud          = (pt_t)VMM_PA_LA(pud);
    pud[pud_idx] = 0x00;
    // TODO: 如果一页表都被 unmap，释放占用的物理内存
    CPU::INVLPG(va);
}

bool VMM::get_mmap(const pgd_t pgd, const void *va, const void *pa) {
    uint32_t pgd_idx = GET_PGD(reinterpret_cast<uint32_t>(va));
    uint32_t pud_idx = GET_PUD(reinterpret_cast<uint32_t>(va));
    pt_t     pud     = (pt_t)(pgd[pgd_idx] & COMMON::PAGE_MASK);
    if (pud == nullptr) {
        if (pa != nullptr) {
            *(uint32_t *)pa = (uint32_t) nullptr;
        }
        return 0;
    }
    pud = (pt_t)(VMM_PA_LA(pud));
    if (pud[pud_idx] != 0) {
        if (pa != nullptr) {
            *(uint32_t *)pa = pud[pud_idx] & COMMON::PAGE_MASK;
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
