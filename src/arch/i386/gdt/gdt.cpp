
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// gdt.cpp for Simple-XX/SimpleKernel.

#include "io.h"
#include "cpu.hpp"
#include "gdt.h"

namespace GDT {
    IO io;
    // 加载 GDTR
    extern "C" void gdt_load(uint32_t);
    // 刷新 TSS
    extern "C" void tss_load();
    // 全局 gdt 指针
    static gdt_ptr_t gdt_ptr;
    // 全局描述符表定义
    static gdt_entry_t gdt_entries[GDT_LENGTH] __attribute__((aligned(8)));
    // TSS 段定义
    tss_struct_t tss_entry __attribute__((aligned(8)));
    void set_gdt(int32_t num, uint32_t base, uint32_t limit, uint8_t access,
                 uint8_t gran) {
        gdt_entries[num].base_low    = (base & 0xFFFF);
        gdt_entries[num].base_middle = (base >> 16) & 0xFF;
        gdt_entries[num].base_high   = (base >> 24) & 0xFF;

        gdt_entries[num].limit_low   = (limit & 0xFFFF);
        gdt_entries[num].granularity = (limit >> 16) & 0x0F;

        gdt_entries[num].granularity |= gran & 0xF0;
        gdt_entries[num].access = access;
        return;
    }

    void set_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
        // 获取 TSS 描述符的位置和长度
        uint32_t base  = (uint32_t)&tss_entry;
        uint32_t limit = base + sizeof(tss_entry);

        // 在 GDT 表中增加 TSS 段描述
        set_gdt(num, base, limit, 0x89, 0x40);

        // 设置内核栈的地址
        tss_entry.ts_ss0  = ss0;
        tss_entry.ts_esp0 = esp0;
        tss_entry.ts_cs   = USER_CS;
        tss_entry.ts_ss   = USER_DS;
        tss_entry.ts_ds   = USER_DS;
        tss_entry.ts_es   = USER_DS;
        tss_entry.ts_fs   = USER_DS;
        tss_entry.ts_gs   = USER_DS;
        return;
    }

    int32_t init(void) {
        // 全局描述符表界限  从 0 开始，所以总长要 - 1
        gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_LENGTH - 1;
        gdt_ptr.base  = (uint32_t)&gdt_entries;
        // 采用 Intel 平坦模型
        // 0xC0: 粒度为 4096?
        // Intel 文档要求首个描述符全 0
        set_gdt(SEG_NULL, 0x0, 0x0, 0x0, 0x0);
        // 内核指令段
        set_gdt(SEG_KTEXT, 0x0, 0xFFFFFFFF, KREAD_EXEC, 0xC0);
        // 内核数据段
        set_gdt(SEG_KDATA, 0x0, 0xFFFFFFFF, KREAD_WRITE, 0xC0);
        // 用户模式代码段
        set_gdt(SEG_UTEXT, 0x0, 0xFFFFFFFF, UREAD_EXEC, 0xC0);
        // 用户模式数据段
        set_gdt(SEG_UDATA, 0x0, 0xFFFFFFFF, UREAD_WRITE, 0xC0);
        set_tss(SEG_TSS, KERNEL_DS, 0);

        // 加载全局描述符表地址到 GDTR 寄存器
        gdt_load((uint32_t)&gdt_ptr);
        // 加载任务寄存器
        tss_load();
        io.printf("gdt init\n");
        return 0;
    }
};
