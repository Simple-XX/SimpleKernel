
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// vmm.c for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"
#include "cpu.hpp"
#include "debug.h"
#include "intr.h"
#include "vmm.h"

// 内核页目录区域，内容为页表
pmd_t pgd_kernel[VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY]
    __attribute__((aligned(VMM_PAGE_SIZE)));

// DMA 页表区域，内容为页表项
pte_t pte_DMA[VMM_PAGE_TABLES_DMA][VMM_PAGES_PRE_PAGE_TABLE]
    __attribute__((aligned(VMM_PAGE_SIZE)));

// NORMAL 页表区域
pte_t pte_NORMAL[VMM_PAGE_TABLES_NORMAL][VMM_PAGES_PRE_PAGE_TABLE]
    __attribute__((aligned(VMM_PAGE_SIZE)));

// HIGHMEM 页表区域
pte_t pte_HIGHMEM[VMM_PAGE_TABLES_HIGHMEM][VMM_PAGES_PRE_PAGE_TABLE]
    __attribute__((aligned(VMM_PAGE_SIZE)));

void page_fault(pt_regs_t *pt_regs) {
#ifdef __x86_64__
    uint64_t cr2;
    __asm__ volatile("movq %%cr2,%0" : "=r"(cr2));
#else
    uint32_t cr2;
    __asm__ volatile("mov %%cr2,%0" : "=r"(cr2));
#endif
    printk("Page fault at 0x%08X, virtual faulting address 0x%08X\n",
           pt_regs->eip, cr2);
    printk_err("Error code: 0x%08X\n", pt_regs->err_code);

    // bit 0 为 0 指页面不存在内存里
    if (!(pt_regs->err_code & 0x1)) {
        printk_color(red, "Because the page wasn't present.\n");
    }
    // bit 1 为 0 表示读错误，为 1 为写错误
    if (pt_regs->err_code & 0x2) {
        printk_err("Write error.\n");
    }
    else {
        printk_err("Read error.\n");
    }
    // bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
    if (pt_regs->err_code & 0x4) {
        printk_err("In user mode.\n");
    }
    else {
        printk_err("In kernel mode.\n");
    }
    // bit 3 为 1 表示错误是由保留位覆盖造成的
    if (pt_regs->err_code & 0x8) {
        printk_err("Reserved bits being overwritten.\n");
    }
    // bit 4 为 1 表示错误发生在取指令的时候
    if (pt_regs->err_code & 0x10) {
        printk_err("The fault occurred during an instruction fetch.\n");
    }
    while (1) {
        ;
    }
}

// 根据 zone 机制，内核可用区域为
void vmm_init(void) {
    register_interrupt_handler(INT_PAGE_FAULT, &page_fault);
    uint32_t pgd_idx = 0;
    ptr_t *  pte     = NULL;

    // 映射 DMA
    pgd_idx = VMM_PGD_INDEX(DMA_START_ADDR);
    for (uint32_t i = pgd_idx, j = 0; j < VMM_PAGE_TABLES_DMA; i++, j++) {
        pgd_kernel[i] = ((ptr_t)VMM_LA_PA((ptr_t)pte_DMA[j]) |
                         VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL);
    }
    pte = (ptr_t *)pte_DMA;
    for (uint32_t i = 0; i < VMM_PAGES_DMA; i++) {
        pte[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL;
    }

    // 映射 NORMAL
    pgd_idx = VMM_PGD_INDEX(NORMAL_START_ADDR);
    for (uint32_t i = pgd_idx, j = 0; j < VMM_PAGE_TABLES_NORMAL; i++, j++) {
        pgd_kernel[i] = ((ptr_t)VMM_LA_PA((ptr_t)pte_NORMAL[j]) |
                         VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL);
    }
    pte = (ptr_t *)pte_NORMAL;
    for (uint32_t i = 0; i < VMM_PAGES_NORMAL; i++) {
        pte[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL;
    }

    // 映射 HIGHMEM
    pgd_idx = VMM_PGD_INDEX(HIGHMEM_START_ADDR);
    for (uint32_t i = pgd_idx, j = 0; j < VMM_PAGE_TABLES_HIGHMEM; i++, j++) {
        pgd_kernel[i] = ((ptr_t)VMM_LA_PA((ptr_t)pte_HIGHMEM[j]) |
                         VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL);
    }
    pte = (ptr_t *)pte_HIGHMEM;
    for (uint32_t i = 0; i < VMM_PAGES_HIGHMEM; i++) {
        pte[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL;
    }

    enable_page(VMM_LA_PA(pgd_kernel));

    printk_info("vmm_init\n");
    return;
}

void enable_page(pmd_t *pgd) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(pgd));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    // 最高位 PG 位置 1，分页开启
    cr0 |= (1u << 31);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
    return;
}

void switch_pgd(pmd_t *pgd) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(pgd));
}

#ifdef __cplusplus
}
#endif
