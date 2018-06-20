
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// intr.c for MRNIU/SimpleKernel.


#include "intr.h"
#include "../../../include/pic.h"

// idt 初始化
void idt_init(void){
  init_interrupt_chip();
  idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX -1;
  idt_ptr.base = (uint32_t)&idt_entries;

  // 0-32:  用于 CPU 的中断处理
  // GD_KTEXT: 内核代码段
  // 0x8E: 10001110
  // 0x08: 0000 1000
  for(uint32_t i=0; i<48; i++){
    idt_set_gate( i, (uint32_t)isr_irq_func[i], 0x08, 0x8E);
  }
  // 128 (0x80) 将来用于实现系统调用
  // 0xEF: 1110 1111
  idt_set_gate(128, (uint32_t)isr128, 0x08, 0xEF);
  idt_load((uint32_t)&idt_ptr);
}

// 调用中断处理函数
void isr_handler(pt_regs_t *regs){
  if (interrupt_handlers[regs->int_no]) {
    interrupt_handlers[regs->int_no](regs);
  }
  else {
    printk("Unhandled interrupt: %d\n", regs->int_no);
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
