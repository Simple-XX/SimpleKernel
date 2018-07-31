// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// time.c for MRNIU/SimpleKernel.

#include "clock.h"

#define FREQUENCY       100

// 定时中断由 8253/8254 芯片从 IRQ0 提供
#define IO_TIMER        0x40               // 8253 Timer #1

// 输入频率为 1193180，frequency 即每秒中断次数
#define TIMER_FREQ      1193180

// Intel 8253/8254 PIT芯片 I/O端口地址范围是40h~43h
#define TIMER_MODE      (IO_TIMER + 3)         // timer mode port

#define TIMER_SEL0      0x00                    // select counter 0
#define TIMER_RATEGEN   0x04                    // mode 2
#define TIMER_CLK       0x06                    // mode 3
#define TIMER_16BIT     0x30                    // r/w counter 16 bits, LSB first

void clock_callback(pt_regs_t *regs){
  static uint32_t tick = 0;
  printk("Tick: %d\n", tick++);
}

void clock_init(){
  printk("initialize timer");
  // 注册时间相关的处理函数
  register_interrupt_handler(IRQ0, clock_callback);

  uint32_t divisor = TIMER_FREQ / FREQUENCY;

  //outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
  outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);

  // 拆分低字节和高字节
  uint8_t low = (uint8_t)(divisor & 0xFF);
  uint8_t hign = (uint8_t)((divisor >> 8) & 0xFF);

  // 分别写入低字节和高字节
  outb(IO_TIMER, low);
  outb(IO_TIMER, hign);
}
