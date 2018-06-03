
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-6.html
// gdt.h for MRNIU/SimpleKernel.

#ifndef _GDT_H_
#define _GDT_H_

#include "stdint.h"
#include "../../include/kernel.h"

// 全局描述符表长度
#define GDT_LENGTH 6

// 各个内存段所在全局描述符表下标
#define SEG_NULL    0
#define SEG_KTEXT   1
#define SEG_KDATA   2
#define SEG_UTEXT   3
#define SEG_UDATA   4
#define SEG_TSS     5

#define GD_KTEXT    ((SEG_KTEXT) << 3)      // 内核代码段
#define GD_KDATA    ((SEG_KDATA) << 3)      // 内核数据段
#define GD_UTEXT    ((SEG_UTEXT) << 3)      // 用户代码段
#define GD_UDATA    ((SEG_UDATA) << 3)      // 用户数据段
#define GD_TSS      ((SEG_TSS) << 3)        // 任务段

// 段描述符 DPL
#define DPL_KERNEL  (0)
#define DPL_USER    (3)

// 各个段的全局描述符表的选择子
#define KERNEL_CS   ((GD_KTEXT) | DPL_KERNEL)
#define KERNEL_DS   ((GD_KDATA) | DPL_KERNEL)
#define USER_CS     ((GD_UTEXT) | DPL_USER)
#define USER_DS     ((GD_UDATA) | DPL_USER)

// TSS 描述符
typedef struct tss_entry_t {
  uint32_t ts_link; // old ts selector
  uint32_t ts_esp0; // stack pointers and segment selectors
  uint16_t ts_ss0; // after an increase in privilege level
  uint16_t ts_padding1;
  uint32_t ts_esp1;
  uint16_t ts_ss1;
  uint16_t ts_padding2;
  uint32_t ts_esp2;
  uint16_t ts_ss2;
  uint16_t ts_padding3;
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
  uint16_t ts_padding4;
  uint16_t ts_cs;
  uint16_t ts_padding5;
  uint16_t ts_ss;
  uint16_t ts_padding6;
  uint16_t ts_ds;
  uint16_t ts_padding7;
  uint16_t ts_fs;
  uint16_t ts_padding8;
  uint16_t ts_gs;
  uint16_t ts_padding9;
  uint16_t ts_ldt;
  uint16_t ts_padding10;
  uint16_t ts_t; // trap on task switch
  uint16_t ts_iomb; // i/o map base address
} __attribute__((packed)) tss_entry_t;


#endif
