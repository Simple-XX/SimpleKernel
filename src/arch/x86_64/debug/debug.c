
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// debug.c for MRNIU/SimpleKernel.

#include "include/debug.h"

void debug_init(uint32_t magic, uint32_t addr) {
    UNUSED(magic);
    UNUSED(addr);
    printk_color(COL_DEBUG, "[DEBUG] ");
    printk("debug_init\n");
    // multiboot2_init(magic, addr);
    return;
}

void print_cur_status() {
    static int round = 0;
    uint16_t reg1, reg2, reg3, reg4;
    asm volatile (  "mov %%cs, %0;"
                    "mov %%ds, %1;"
                    "mov %%es, %2;"
                    "mov %%ss, %3;"
                    : "=m" ( reg1 ), "=m" ( reg2 ), "=m" ( reg3 ), "=m" ( reg4 ) );

    // 打印当前的运行级别
    printk_color(COL_DEBUG, "[DEBUG] ");
    printk("%d:  @ring %d\n", round, reg1 & 0x3);
    printk_color(COL_DEBUG, "[DEBUG] ");
    printk("%d:  cs = %X\n", round, reg1);
    printk_color(COL_DEBUG, "[DEBUG] ");
    printk("%d:  ds = %X\n", round, reg2);
    printk_color(COL_DEBUG, "[DEBUG] ");
    printk("%d:  es = %X\n", round, reg3);
    printk_color(COL_DEBUG, "[DEBUG] ");
    printk("%d:  ss = %X\n", round, reg4);
    ++round;
}

void panic(const char *msg) {
    printk("*** System panic: %s\n", msg);
    print_stack_trace();
    printk("***\n");

    // 致命错误发生后打印栈信息后停止在这里
    while (1)
        cpu_hlt();
}

const char * elf_lookup_symbol(uint32_t addr, elf_t *elf) {
    printk("%x\n", elf->symtabsz);
    for (uint32_t i = 0; i < ( elf->symtabsz / sizeof( Elf32_Sym ) ); i++) {
        printk("66666666666\n");
        if (ELF32_ST_TYPE(elf->symtab[i].st_info) != 0x2) {
            continue;
        }
        // 通过函数调用地址查到函数的名字(地址在该函数的代码段地址区间之内)
        if ( ( addr >= elf->symtab[i].st_value ) && ( addr < ( elf->symtab[i].st_value + elf->symtab[i].st_size ) ) ) {
            return (const char *)( (uint32_t)elf->strtab + elf->symtab[i].st_name );
        }
    }
    return NULL;
}

void print_stack_trace(void) {
    uint32_t *ebp, *eip;
    __asm__ volatile ( "mov %%ebp, %0" : "=r" ( ebp ) );
    while (*ebp) {
        eip = ebp + 1;
        printk( "   [0x%x] %s\n", *eip, elf_lookup_symbol(*eip, &kernel_elf) );
        ebp = (uint32_t*)*ebp;
    }
}
