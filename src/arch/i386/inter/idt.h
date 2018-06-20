
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-6.html
// idt.h for MRNIU/SimpleKernel.


#ifndef _IDT_H_
#define _IDT_H_

#include "stdint.h"
#include "../../../include/kernel.h"

#define IDT_SIZE 256

extern void idt_load();

// 中断描述符
typedef
struct idt_entry_t {
    uint16_t base_low; // 中断处理函数地址 15 ～ 0 位
    uint16_t selector;  // 目标代码段描述符选择子
    uint8_t  zero;  // 置 0 段
    uint8_t  flags; // 一些标志，文档有解释
    uint16_t base_high; // 中断处理函数地址 31 ～ 16 位
} __attribute__((packed)) idt_entry_t;

// IDTR
typedef
struct idt_ptr_t {
  uint16_t limit;     // 限长
  uint32_t base;      // 基址
} __attribute__((packed)) idt_ptr_t;

idt_entry_t idt_entries[IDT_SIZE];
idt_ptr_t idt_ptr;


#endif
