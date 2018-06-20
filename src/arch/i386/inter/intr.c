
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// intr.c for MRNIU/SimpleKernel.


#include "intr.h"

void idt_init(void){
  idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX -1;
  idt_ptr.base = (uint32_t)&idt_entries;

  // 0-32:  用于 CPU 的中断处理
  // GD_KTEXT: 内核代码段
  // 0x8E: 10001110

  for(uint32_t i=0; i<48; i++){
    idt_set_gate( i, (uint32_t)isr_irq_func[i], GD_KTEXT, 0x8E);
  }

  // 128 (0x80) 将来用于实现系统调用
  idt_set_gate(128, (uint32_t)isr128, GD_KTEXT, 0xEF);

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


void irq0(){}             // 电脑系统计时器
void irq1(){}             // 键盘
void irq2(){}            // 与 IRQ9 相接，MPU-401 MD 使用
void irq3(){}            // 串口设备
void irq4(){}            // 串口设备
void irq5(){}            // 建议声卡使用
void irq6(){}            // 软驱传输控制使用
void irq7(){}           // 打印机传输控制使用
void irq8(){}            // 即时时钟
void irq9(){}            // 与 IRQ2 相接，可设定给其他硬件
void irq10(){}           // 建议网卡使用
void irq11(){}          // 建议 AGP 显卡使用
void irq12(){}           // 接 PS/2 鼠标，也可设定给其他硬件
void irq13(){}           // 协处理器使用
void irq14(){}           // IDE0 传输控制使用
void irq15(){}           // IDE1 传输控制使用
