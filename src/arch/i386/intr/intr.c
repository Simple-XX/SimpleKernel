
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.c for MRNIU/SimpleKernel.


#include "intr.h"
#include "port.hpp"
#include "stdio.h"
#include "pic.hpp"
#include "cpu.hpp"


// 中断描述符表
static idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__ ((aligned(16)));

static idt_ptr_t idt_ptr; // IDTR

// 设置中断描述符
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags){
  idt_entries[num].base_low = (base & 0xFFFF);
  idt_entries[num].base_high = (base >> 16) & 0xFFFF;
  idt_entries[num].selector = selector;
  idt_entries[num].zero = 0;
  idt_entries[num].flags = flags;
}

// 中断处理函数指针数组
static interrupt_handler_t interrupt_handlers[INTERRUPT_MAX] __attribute__ ((aligned(4)));

static const char *intrname(uint32_t intrno){
  static const char *const intrnames[] = {
    "Divide error",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection",
    "Page Fault",
    "(unknown trap)",
    "x87 FPU Floating-Point Error",
    "Alignment Check",
    "Machine-Check",
    "SIMD Floating-Point Exception"
  };
  if(intrno < sizeof(intrnames)/sizeof(const char *const)){
    return intrnames[intrno];
  }
  return "(unknown trap)";
}

// 调用中断处理函数
void isr_handler(pt_regs_t *regs){
  if (interrupt_handlers[regs->int_no]) {
    interrupt_handlers[regs->int_no](regs);
  }
  else {
    printk("Unhandled interrupt: %d %s\n", regs->int_no, intrname(regs->int_no));
    cpu_hlt();
  }
}

// 注册一个中断处理函数
void register_interrupt_handler(uint8_t n, interrupt_handler_t h){
  interrupt_handlers[n] = h;
}

// IRQ 处理函数
void irq_handler(pt_regs_t *regs){
	clear_interrupt_chip(regs->int_no);  // 重设PIC芯片
  if (interrupt_handlers[regs->int_no]) {
    interrupt_handlers[regs->int_no](regs);
  }
}

// 中断处理函数指针数组
static isr_irq_func_t isr_irq_func[INTERRUPT_MAX] = {
  [0]  = &isr0,  [1]  = &isr1,  [2]  = &isr2,  [3]  = &isr3,
  [4]  = &isr4,  [5]  = &isr5,  [6]  = &isr6,  [7]  = &isr7,
  [8]  = &isr8,  [9]  = &isr9,  [10] = &isr10, [11] = &isr11,
  [12] = &isr12, [13] = &isr13, [14] = &isr14, [15] = &isr15,
  [16] = &isr16, [17] = &isr17, [18] = &isr18, [19] = &isr19,
  [20] = &isr20, [21] = &isr21, [22] = &isr22, [23] = &isr23,
  [24] = &isr24, [25] = &isr25, [26] = &isr26, [27] = &isr27,
  [28] = &isr28, [29] = &isr29, [30] = &isr30, [31] = &isr31,

  [32] = &irq0,  [33] = &irq1,  [34] = &irq2,  [35] = &irq3,
  [36] = &irq4,  [37] = &irq5,  [38] = &irq6,  [39] = &irq7,
  [40] = &irq8,  [41] = &irq9,  [42] = &irq10, [43] = &irq11,
  [44] = &irq12, [45] = &irq13, [46] = &irq14, [47] = &irq15,
};

// idt 初始化
void idt_init(void){
  init_interrupt_chip();
  idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX -1;
  idt_ptr.base = (uint32_t)&idt_entries;

  // 0-32:  用于 CPU 的中断处理
  // GD_KTEXT: 内核代码段
  // 0x8E: 10001110
  // 0x08: 0000 1000

  for(uint32_t i=0; i<48; ++i){
    idt_set_gate(i, (uint32_t)isr_irq_func[i], 0x08, 0x8E);
  }
  // 128 (0x80) 将来用于实现系统调用
  // 0xEF: 1110 1111
  idt_set_gate(128, (uint32_t)isr128, 0x08, 0xEF);
  idt_load((uint32_t)&idt_ptr);
}
