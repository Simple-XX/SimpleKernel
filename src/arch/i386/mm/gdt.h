
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-6.html
// gdt.h for MRNIU/SimpleKernel.

#ifndef _GDT_H_
#define _GDT_H_

#include "stdint.h"
#include "../../../include/kernel.h"

// 全局描述符类型
typedef struct gdt_entry_t {
  uint16_t limit_low;     // 段界限   15～0
  uint16_t base_low;      // 段基地址 15～0
  uint8_t  base_middle;   // 段基地址 23～16
  uint8_t  access;        // 段存在位、描述符特权级、描述符类型、描述符子类别
  uint8_t  granularity;   // 其他标志、段界限 19～16
  uint8_t  base_high;     // 段基地址 31～24
} __attribute__((packed)) gdt_entry_t;

// 全局描述符表定义
static gdt_entry_t gdt_entries[GDT_LENGTH] __attribute__ ((aligned(8)));

// GDTR
typedef struct gdt_ptr_t {
  uint16_t limit; // 全局描述符表限长
  uint32_t base; // 全局描述符表 32位 基地址
} __attribute__((packed)) gdt_ptr_t;

// GDTR
static gdt_ptr_t gdt_ptr;

/*
  TSS 状态段由两部分组成：
    1、 动态部分(处理器在每次任务切换时会设置这些字段值)
        通用寄存器(EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
        段寄存器(ES，CS，SS，DS，FS，GS)
        状态寄存器(EFLAGS)
        指令指针(EIP)
        前一个执行的任务的TSS段的选择子(只有当要返回时才更新)
    2、 静态字段(处理器读取，但从不更改)
        任务的LDT选择子
        页目录基址寄存器(PDBR)(当启用分页时，只读)
        内层堆栈指针，特权级 0-2
        T-位，指示了处理器在任务切换时是否引发一个调试异常
        I/O 位图基址
*/
// TSS(任务状态段) 描述符
// TSS的使用是为了解决调用门中特权级变换时堆栈发生的变化.
typedef struct tss_entry_t {
  uint32_t ts_link; // old ts selector
  uint32_t ts_esp0; // stack pointers and segment selectors
  uint16_t ts_ss0; // after an increase in privilege level
  uint32_t ts_esp1;
  uint16_t ts_ss1;
  uint32_t ts_esp2;
  uint16_t ts_ss2;
  uint32_t ts_cr3; // page directory base
  uint32_t ts_eip; // saved state from last task switch
  uint32_t ts_eflags;
  uint32_t ts_eax; // more saved state (registers)
  uint32_t ts_ecx;
  uint32_t ts_edx;
  uint32_t ts_ebx;
  uint32_t ts_esp;
  uint32_t ts_ebp;
  uint32_t ts_esi;
  uint32_t ts_edi;
  uint16_t ts_es; // even more saved state (segment selectors)
  uint16_t ts_cs;
  uint16_t ts_ss;
  uint16_t ts_ds;
  uint16_t ts_fs;
  uint16_t ts_gs;
  uint16_t ts_ldt;
  uint16_t ts_t; // trap on task switch
  uint16_t ts_iomb; // i/o map base address
} __attribute__((packed)) tss_entry_t;

// TSS 段定义
static tss_entry_t tss_entry __attribute__ ((aligned(8)));

#endif
