
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// gdt.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "debug.h"
#include "cpu.hpp"
#include "include/gdt.h"

static gdt_ptr_t gdt_ptr;
// 全局描述符表定义
static gdt_entry_t gdt_entries[GDT_LENGTH] __attribute__( (aligned(8) ) );

// TSS 段定义
static tss_entry_t tss_entry __attribute__( (aligned(8) ) );
static void tss_set_gate(int32_t num, uint16_t ss0, uint32_t esp0);

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt_entries[num].base_low     = (base & 0xFFFF);
	gdt_entries[num].base_middle  = (base >> 16) & 0xFF;
	gdt_entries[num].base_high    = (base >> 24) & 0xFF;

	gdt_entries[num].limit_low    = (limit & 0xFFFF);
	gdt_entries[num].granularity  = (limit >> 16) & 0x0F;

	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access       = access;
}

void tss_set_gate(int32_t num, uint16_t ss0, uint32_t esp0) {
	// 获取 TSS 描述符的位置和长度
	uint32_t base = (uint32_t)&tss_entry;
	uint32_t limit = base + sizeof(tss_entry);

	// 在 GDT 表中增加 TSS 段描述
	gdt_set_gate(num, base, limit, 0x89, 0x40);

	// 设置内核栈的地址
	tss_entry.ts_ss0  = ss0;
	tss_entry.ts_esp0 = esp0;
	tss_entry.ts_cs = USER_CS;
	tss_entry.ts_ss = USER_DS;
	tss_entry.ts_ds = USER_DS;
	tss_entry.ts_es = USER_DS;
	tss_entry.ts_fs = USER_DS;
	tss_entry.ts_gs = USER_DS;
}

// 初始化全局描述符表
void gdt_init(void) {
	cpu_cli();
	// 全局描述符表界限  从 0 开始，所以总长要 - 1
	gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_LENGTH - 1;
	gdt_ptr.base = (uint32_t)&gdt_entries;

	// 采用 Intel 平坦模型
	// 0xC0: 粒度为 4096?
	gdt_set_gate(SEG_NULL, 0x0, 0x0, 0x0, 0x0);      // Intel 文档要求首个描述符全 0
	gdt_set_gate(SEG_KTEXT, 0x0, 0xFFFFFFFF, KREAD_EXEC, 0xC0); // 内核指令段
	gdt_set_gate(SEG_KDATA, 0x0, 0xFFFFFFFF, KREAD_WRITE, 0xC0); // 内核数据段
	gdt_set_gate(SEG_UTEXT, 0x0, 0xFFFFFFFF, UREAD_EXEC, 0xC0); // 用户模式代码段
	gdt_set_gate(SEG_UDATA, 0x0, 0xFFFFFFFF, UREAD_WRITE, 0xC0); // 用户模式数据段
	tss_set_gate(SEG_TSS, KERNEL_DS, 0);

	// 加载全局描述符表地址到 GDTR 寄存器
	gdt_load( (uint32_t)&gdt_ptr);
	// 加载任务寄存器
	tss_load();

	printk_info("gdt_init\n");
	cpu_sti();
	return;
}

#ifdef __cplusplus
}
#endif
