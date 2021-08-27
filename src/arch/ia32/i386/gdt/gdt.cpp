
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// gdt.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "gdt.h"

namespace GDT {
    // 加载 GDTR
    extern "C" void gdt_load(uint32_t);
    // 全局 gdt 指针
    static gdt_ptr_t gdt_ptr;
    // 全局描述符表定义
    static gdt_entry_t gdt_entries[GDT_LENGTH] __attribute__((aligned(8)));

    void set_gdt(const uint8_t idx, const uint32_t base, const uint32_t limit,
                 const uint8_t type, const uint8_t s, const uint8_t dpl,
                 const uint8_t p, const uint8_t avl, const uint8_t l,
                 const uint8_t db, const uint8_t g) {
        gdt_entries[idx].limit1     = (limit & 0xFFFF);
        gdt_entries[idx].base_addr1 = (base & 0xFFFF);
        gdt_entries[idx].base_addr2 = (base >> 16) & 0xFF;
        gdt_entries[idx].type       = type;
        gdt_entries[idx].s          = s;
        gdt_entries[idx].dpl        = dpl;
        gdt_entries[idx].p          = p;
        gdt_entries[idx].limit2     = (limit >> 16) & 0x0F;
        gdt_entries[idx].avl        = avl;
        gdt_entries[idx].l          = l;
        gdt_entries[idx].db         = db;
        gdt_entries[idx].g          = g;
        gdt_entries[idx].base_addr3 = (base >> 24) & 0xFF;
        return;
    }

    int32_t init(void) {
        // 全局描述符表界限  从 0 开始，所以总长要 - 1
        gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_LENGTH - 1;
        gdt_ptr.base  = (uint32_t)&gdt_entries;
        // 采用 Intel 平坦模型
        // Intel 文档要求首个描述符全 0
        set_gdt(GDT_NULL, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);
        // 内核指令段
        set_gdt(GDT_KERNEL_CODE, BASE, LIMIT, TYPE_CODE_EXECUTE_READ,
                S_CODE_DATA, CPU::DPL0, SEGMENT_PRESENT, AVL_NOT_AVAILABLE,
                L_32BIT, DB_EXECUTABLE_CODE_SEGMENT_32, G_4KB);
        // 内核数据段
        set_gdt(GDT_KERNEL_DATA, BASE, LIMIT, TYPE_DATA_READ_WRITE, S_CODE_DATA,
                CPU::DPL0, SEGMENT_PRESENT, AVL_NOT_AVAILABLE, L_32BIT,
                DB_EXPAND_DOWN_DATA_SEGMENT_4GB, G_4KB);
        // 用户模式代码段
        set_gdt(GDT_USER_CODE, BASE, LIMIT, TYPE_CODE_EXECUTE_READ, S_CODE_DATA,
                CPU::DPL3, SEGMENT_PRESENT, AVL_NOT_AVAILABLE, L_32BIT,
                DB_EXECUTABLE_CODE_SEGMENT_32, G_4KB);
        // 用户模式数据段
        set_gdt(GDT_USER_DATA, BASE, LIMIT, TYPE_DATA_READ_WRITE, S_CODE_DATA,
                CPU::DPL3, SEGMENT_PRESENT, AVL_NOT_AVAILABLE, L_32BIT,
                DB_EXPAND_DOWN_DATA_SEGMENT_4GB, G_4KB);
        // 加载全局描述符表地址到 GDTR 寄存器
        gdt_load((uint32_t)&gdt_ptr);
        info("gdt init.\n");
        return 0;
    }
};
