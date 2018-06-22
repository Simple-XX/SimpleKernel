
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// intr.h for MRNIU/SimpleKernel.


#ifndef _INTR_H_
#define _INTR_H_

#include "stdint.h"
#include "../../../include/kernel.h"


// 寄存器类型
typedef
struct pt_regs_t {
  uint32_t ds;        // 用于保存用户的数据段描述符
  uint32_t edi;       // 从 edi 到 eax 由 pusha 指令压入
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t int_no;    // 中断号
  uint32_t err_code;      // 错误代码(有中断错误代码的中断会由CPU压入)
  uint32_t eip;       // 以下由处理器自动压入
  uint32_t cs;
  uint32_t eflags;
  uint32_t useresp;
  uint32_t ss;
} pt_regs_t;

// 声明中断处理函数 0 ~ 19 属于 CPU 的异常中断
// ISR:中断服务程序(interrupt service routine)
extern void isr0();        // 0 #DE 除 0 异常
extern void isr1();        // 1 #DB 调试异常
extern void isr2();        // 2 NMI
extern void isr3();        // 3 BP 断点异常
extern void isr4();        // 4 #OF 溢出
extern void isr5();        // 5 #BR 对数组的引用超出边界
extern void isr6();        // 6 #UD 无效或未定义的操作码
extern void isr7();        // 7 #NM 设备不可用(无数学协处理器)
extern void isr8();        // 8 #DF 双重故障(有错误代码)
extern void isr9();        // 9 协处理器跨段操作
extern void isr10();       // 10 #TS 无效TSS(有错误代码)
extern void isr11();       // 11 #NP 段不存在(有错误代码)
extern void isr12();       // 12 #SS 栈错误(有错误代码)
extern void isr13();       // 13 #GP 常规保护(有错误代码)
extern void isr14();       // 14 #PF 页故障(有错误代码)
extern void isr15();       // 15 CPU 保留
extern void isr16();       // 16 #MF 浮点处理单元错误
extern void isr17();       // 17 #AC 对齐检查
extern void isr18();       // 18 #MC 机器检查
extern void isr19();       // 19 #XM SIMD(单指令多数据)浮点异常

// 20 ~ 31 Intel 保留
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// 32 ~ 255 用户自定义异常
extern void isr128();  // 0x80 用于实现系统调用

// 声明 IRQ 函数
// IRQ:中断请求(Interrupt Request)
extern void irq0();            // 电脑系统计时器
extern void irq1();            // 键盘
extern void irq2();            // 与 IRQ9 相接，MPU-401 MD 使用
extern void irq3();            // 串口设备
extern void irq4();            // 串口设备
extern void irq5();            // 建议声卡使用
extern void irq6();            // 软驱传输控制使用
extern void irq7();            // 打印机传输控制使用
extern void irq8();            // 即时时钟
extern void irq9();            // 与 IRQ2 相接，可设定给其他硬件
extern void irq10();           // 建议网卡使用
extern void irq11();           // 建议 AGP 显卡使用
extern void irq12();           // 接 PS/2 鼠标，也可设定给其他硬件
extern void irq13();           // 协处理器使用
extern void irq14();           // IDE0 传输控制使用
extern void irq15();           // IDE1 传输控制使用


void irq_handler(pt_regs_t *regs);  // IRQ 处理函数

typedef void (*interrupt_handler_t)(pt_regs_t *); // 定义中断处理函数指针

void isr_handler(pt_regs_t *regs);  // 调用中断处理函数

void register_interrupt_handler(uint8_t n, interrupt_handler_t h);  // 注册一个中断处理函数

// 中断处理函数指针数组
static interrupt_handler_t interrupt_handlers[INTERRUPT_MAX] __attribute__ ((aligned(4)));

void irq_handler(pt_regs_t *regs);  // IRQ 处理函数

extern void idt_load(uint32_t);  // 声明加载 IDTR 的函数

typedef void (*isr_irq_func_t)(); // 中断处理函数指针类型

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

// 中断描述符
typedef
struct idt_entry_t {
  uint16_t base_low;        // 中断处理函数地址 15～0 位
  uint16_t selector;            // 目标代码段描述符选择子
  uint8_t  zero;        // 置 0 段
  uint8_t  flags;          // 一些标志，文档有解释
  uint16_t base_high;        // 中断处理函数地址 31～16 位
} __attribute__((packed)) idt_entry_t;

// IDTR
typedef
struct idt_ptr_t {
  uint16_t limit;        // 限长
  uint32_t base;         // 基址
} __attribute__((packed)) idt_ptr_t;

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

#endif
