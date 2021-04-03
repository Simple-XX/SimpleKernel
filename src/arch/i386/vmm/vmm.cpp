
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.cpp for Simple-XX/SimpleKernel.

#include "io.h"
#include "stdint.h"
#include "string.h"
#include "cpu.hpp"
#include "intr.h"
#include "vmm.h"

// TODO: 完善缺页处理
static void page_fault(INTR::pt_regs_t *regs) {
#ifdef __x86_64__
    uint64_t cr2;
    asm volatile("movq %%cr2,%0" : "=r"(cr2));
#else
    uint32_t cr2;
    asm volatile("mov %%cr2,%0" : "=r"(cr2));
#endif
    io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
              "Page fault at 0x%08X, virtual faulting address 0x%08X\n",
              regs->eip, cr2);
    io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
              "Error code: 0x%08X\n", regs->err_code);

    // bit 0 为 0 指页面不存在内存里
    if (!(regs->err_code & 0x1)) {
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "Because the page wasn't present.\n");
    }
    // bit 1 为 0 表示读错误，为 1 为写错误
    if (regs->err_code & 0x2) {
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "Write error.\n");
    }
    else {
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "Read error.\n");
    }
    // bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
    if (regs->err_code & 0x4) {
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "In user mode.\n");
    }
    else {
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "In kernel mode.\n");
    }
    // bit 3 为 1 表示错误是由保留位覆盖造成的
    if (regs->err_code & 0x8) {
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "Reserved bits being overwritten.\n");
    }
    // bit 4 为 1 表示错误发生在取指令的时候
    if (regs->err_code & 0x10) {
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "The fault occurred during an instruction fetch.\n");
    }
    while (1) {
        ;
    }
    return;
}

VMM::VMM(PMM &_pmm) : pmm(_pmm) {
    return;
}

VMM::~VMM(void) {
    return;
}

void VMM::init(void) {
    INTR::register_interrupt_handler(INTR::INT_PAGE_FAULT, &page_fault);
    page_dir = (page_dir_t)pmm.alloc_page(VMM_PAGE_DIRECTORIES_KERNEL);
    bzero((void *)page_dir, VMM_PAGE_SIZE * VMM_PAGE_DIRECTORIES_KERNEL);
    page_table = (page_table_t)pmm.alloc_page(VMM_PAGE_TABLES_KERNEL);
    bzero((void *)page_table, VMM_PAGE_SIZE * VMM_PAGE_TABLES_KERNEL);
// #define DEBUG
#ifdef DEBUG
    io.printf("VMM_PAGE_DIRECTORIES_KERNEL: 0x%X\n",
              VMM_PAGE_DIRECTORIES_KERNEL);
    io.printf("VMM_PAGE_TABLES_KERNEL: 0x%X\n", VMM_PAGE_TABLES_KERNEL);
    io.printf("pgd_krnel: 0x%X\n", page_dir);
    io.printf("page_table: 0x%X\n", page_table);
#endif
    for (uint32_t i = VMM_PGD_INDEX(KERNEL_BASE), j = 0;
         j < VMM_PAGE_TABLES_KERNEL; i++, j++) {
        page_dir[i] = reinterpret_cast<page_dir_entry_t>(
            reinterpret_cast<uint32_t>(
                &page_table[VMM_PAGES_PRE_PAGE_TABLE * j]) |
            VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL);
// #define DEBUG
#ifdef DEBUG
        io.printf("page_dir[%d] = 0x%X\n", i, page_dir[i]);
#endif
    }
    for (uint32_t i = 0; i < VMM_PAGES_KERNEL; i++) {
        page_table[i] = (page_table_t *)((i << 12) | VMM_PAGE_PRESENT |
                                         VMM_PAGE_RW | VMM_PAGE_KERNEL);
    }
    set_pgd(page_dir);
    enable_page();

    io.printf("vmm_init\n");
    return;
    return;
}

void VMM::enable_page() {
    void *cr3 = 0;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    if (cr3 == nullptr) {
        io.printf("cr3 not set!\n");
    }
    uint32_t cr0 = 0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    // 最高位 PG 位置 1，分页开启
    cr0 |= (1u << 31);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
    return;
}

void VMM::set_pgd(page_dir_t pgd) {
    page_dir = pgd;
    __asm__ volatile("mov %0, %%cr3" : : "r"(pgd));
    return;
}

void VMM::mmap(page_dir_t pgd, void *va, void *pa, uint32_t flag) {
    uint32_t pgd_idx = VMM_PGD_INDEX(reinterpret_cast<uint32_t>(va));
    uint32_t pte_idx = VMM_PTE_INDEX(reinterpret_cast<uint32_t>(va));

    page_table_t pt = pgd[pgd_idx];
    if (pt == nullptr) {
        pt           = (page_table_t)pmm.alloc_page(1);
        pgd[pgd_idx] = (page_table_t)(reinterpret_cast<uint32_t>(pt) |
                                      VMM_PAGE_PRESENT | VMM_PAGE_RW);
    }
    pt          = (page_table_t)VMM_PA_LA(pt);
    pt[pte_idx] = (page_table_entry_t)(reinterpret_cast<uint32_t>(pa) | flag);

    CPU::INVLPG(va);
    return;
}

void VMM::unmmap(page_dir_t pgd, void *va) {
    uint32_t     pgd_idx = VMM_PGD_INDEX(reinterpret_cast<uint32_t>(va));
    uint32_t     pte_idx = VMM_PTE_INDEX(reinterpret_cast<uint32_t>(va));
    page_table_t pt      = pgd[pgd_idx];
    if (pt == nullptr) {
        io.printf("pt == nullptr\n");
        return;
    }
    pt          = (page_table_t)VMM_PA_LA(pt);
    pt[pte_idx] = nullptr;
    CPU::INVLPG(va);
}

uint32_t VMM::get_mmap(page_dir_t pgd, void *va, void *pa) {
    uint32_t     pgd_idx = VMM_PGD_INDEX(reinterpret_cast<uint32_t>(va));
    uint32_t     pte_idx = VMM_PTE_INDEX(reinterpret_cast<uint32_t>(va));
    page_table_t pt      = pgd[pgd_idx];
    if (pt == nullptr) {
        return 0;
    }
    pt = (page_table_t)(VMM_PA_LA(pt));
    if (pt[pte_idx] != nullptr && pa != nullptr) {
        io.printf("pt[pte_idx]: 0x%X\n", pt[pte_idx]);
        *(uint32_t *)pa = reinterpret_cast<uint32_t>(pt[pte_idx]);
        return 1;
    }
    return 0;
}

void VMM::vmm_kernel_init(page_dir_t pgd) {
    return;
}
