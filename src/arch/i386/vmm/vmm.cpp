
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "cpu.hpp"
#include "intr.h"
#include "vmm.h"

// TODO: 完善缺页处理
static void page_fault(INTR::pt_regs_t *regs) {
    IO io;
#ifdef __x86_64__
    uint64_t cr2;
    asm volatile("movq %%cr2,%0" : "=r"(cr2));
#else
    uint32_t cr2;
    asm volatile("mov %%cr2,%0" : "=r"(cr2));
#endif
    io.printf("Page fault at 0x%08X, virtual faulting address 0x%08X, ",
              regs->eip, cr2);
    io.printf("Error code: 0x%08X\n", regs->err_code);

    // bit 0 为 0 指页面不存在内存里
    if (!(regs->err_code & 0x1)) {
        io.printf("Because the page wasn't present.\n");
    }
    // bit 1 为 0 表示读错误，为 1 为写错误
    if (regs->err_code & 0x2) {
        io.printf("Write error.\n");
    }
    else {
        io.printf("Read error.\n");
    }
    // bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
    if (regs->err_code & 0x4) {
        io.printf("In user mode.\n");
    }
    else {
        io.printf("In kernel mode.\n");
    }
    // bit 3 为 1 表示错误是由保留位覆盖造成的
    if (regs->err_code & 0x8) {
        io.printf("Reserved bits being overwritten.\n");
    }
    // bit 4 为 1 表示错误发生在取指令的时候
    if (regs->err_code & 0x10) {
        io.printf("The fault occurred during an instruction fetch.\n");
    }
    while (1) {
        ;
    }
    return;
}

IO                   VMM::io;
PMM                  VMM::pmm;
page_table32_entry_t VMM::pte_kernel[VMM_KERNEL_PAGES];
page_dir32_entry_t   VMM::pgde_kernel[VMM_KERNEL_PAGE_TABLES];
cr3_t                VMM::cr3_kernel;

VMM::VMM(void) {
    return;
}

VMM::~VMM(void) {
    return;
}

void VMM::init(void) {
    INTR::register_interrupt_handler(INTR::INT_PAGE_FAULT, &page_fault);
// #define DEBUG
#ifdef DEBUG
    io.printf("VMM_PAGES_TOTAL: 0x%08X, ", VMM_PAGES_TOTAL);
    io.printf("VMM_PAGE_TABLES_TOTAL: 0x%08X\n", VMM_PAGE_TABLES_TOTAL);
#undef DEBUG
#endif
    for (uint32_t i = 0; i < VMM_KERNEL_PAGES; i++) {
        pte_kernel[i].p    = 1;
        pte_kernel[i].rw   = 1;
        pte_kernel[i].us   = 0;
        pte_kernel[i].addr = i & 0x000FFFFF;
    }
    // 内核用一个页目录就够了，不需要多余处理
    for (uint32_t i = VMM_PTE_INDEX(COMMON::KERNEL_BASE);
         i < VMM_KERNEL_PAGE_TABLES; i++) {
        pgde_kernel[i].p  = 1;
        pgde_kernel[i].rw = 1;
        pgde_kernel[i].us = 0;
        pgde_kernel[i].addr =
            ((uint32_t)&pte_kernel[VMM_PAGES_PRE_PAGE_TABLE * i]) >> 12;
#define DEBUG
#ifdef DEBUG
        pte_kernel[VMM_PAGES_PRE_PAGE_TABLE * i].printf();
        io.printf("\t");
        pgde_kernel[i].printf();
        io.printf("\n");
#undef DEBUG
#endif
    }
// #define DEBUG
#ifdef DEBUG
    // asm("hlt");
    // pte_kernel[VMM_PAGES_PRE_PAGE_TABLE * i].printf();
    // pgde_kernel[i].printf();
#undef DEBUG
#endif
    // CPU::CR3_SET_PGD((uint32_t)cr3_kernel);
    // __asm__ volatile("mov %0, %%cr3" : : "r"(cr3_kernel));
    io.printf("pgde_kernel: 0x%08X\n", pgde_kernel);
    __asm__ volatile("mov %0, %%cr3" : : "r"(pgde_kernel));
    asm("hlt");
    CPU::CR0_SET_PG();
    io.printf("vmm_init\n");
    return;
}

// void VMM::set_pgd(page_dir_t pgd) {
//     page_dir = pgd;
//     CPU::CR3_SET_PGD((void *)pgd);
//     return;
// }

// void VMM::mmap(void *va, void *pa, uint32_t flag) {
//     uint32_t pgd_idx = VMM_PGD_INDEX(reinterpret_cast<uint32_t>(va));
//     uint32_t pte_idx = VMM_PTE_INDEX(reinterpret_cast<uint32_t>(va));

//     page_table_t pt = page_dir[pgd_idx];
//     if (pt == nullptr) {
//         pt                = (page_table_t)pmm.alloc_page(1);
//         page_dir[pgd_idx] = (page_table_t)(reinterpret_cast<uint32_t>(pt)
//         |
//                                            VMM_PAGE_PRESENT |
//                                            VMM_PAGE_RW);
//     }
//     pt          = (page_table_t)VMM_PA_LA(pt);
//     pt[pte_idx] = (page_table_entry_t)(reinterpret_cast<uint32_t>(pa) |
//     flag);

//     CPU::INVLPG(va);
//     return;
// }

// void VMM::unmmap(void *va) {
//     uint32_t     pgd_idx = VMM_PGD_INDEX(reinterpret_cast<uint32_t>(va));
//     uint32_t     pte_idx = VMM_PTE_INDEX(reinterpret_cast<uint32_t>(va));
//     page_table_t pt      = page_dir[pgd_idx];
//     if (pt == nullptr) {
//         io.printf("pt == nullptr\n");
//         return;
//     }
//     pt          = (page_table_t)VMM_PA_LA(pt);
//     pt[pte_idx] = nullptr;
//     CPU::INVLPG(va);
// }

// uint32_t VMM::get_mmap(void *va, void *pa) {
//     uint32_t pgd_idx = VMM_PGD_INDEX(reinterpret_cast<uint32_t>(va));
//     uint32_t pte_idx = VMM_PTE_INDEX(reinterpret_cast<uint32_t>(va));
//     io.printf("pgd_idx: 0x%08X, pte_idx: 0x%08X\n", pgd_idx, pte_idx);
//     io.printf("page_dir[0]: 0x%08X\n", page_dir[1]);
//     page_table_t pt = page_dir[pgd_idx];
//     io.printf("pt: 0x%08X\n", pt);
//     if (pt == nullptr) {
//         return 0;
//     }
//     pt = (page_table_t)(VMM_PA_LA(pt));
//     if (pt[pte_idx] != nullptr && pa != nullptr) {
//         io.printf("pt[pte_idx]: 0x%X\n", pt[pte_idx]);
//         *(uint32_t *)pa = reinterpret_cast<uint32_t>(pt[pte_idx]);
//         return 1;
//     }
//     return 0;
// }
