
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// intr.h for Simple-XX/SimpleKernel.

#ifndef _INTR_H_
#define _INTR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// 中断表最大值
#define INTERRUPT_MAX 256

// 定义IRQ
// 电脑系统计时器
#define IRQ0 32
// 键盘
#define IRQ1 33
// 与 IRQ9 相接，MPU-401 MD 使用
#define IRQ2 34
// 串口设备
#define IRQ3 35
// 串口设备
#define IRQ4 36
// 建议声卡使用
#define IRQ5 37
// 软驱传输控制使用
#define IRQ6 38
// 打印机传输控制使用
#define IRQ7 39
// 即时时钟
#define IRQ8 40
// 与 IRQ2 相接，可设定给其他硬件
#define IRQ9 41
// 建议网卡使用
#define IRQ10 42
// 建议 AGP 显卡使用
#define IRQ11 43
// 接 PS/2 鼠标，也可设定给其他硬件
#define IRQ12 44
// 协处理器使用
#define IRQ13 45
// IDE0 传输控制使用
#define IRQ14 46
// IDE1 传输控制使用
#define IRQ15 47

// 中断号定义
#define INT_DIVIDE_ERROR 0
#define INT_DEBUG 1
#define INT_NMI 2
#define INT_BREAKPOINT 3
#define INT_OVERFLOW 4
#define INT_BOUND 5
#define INT_INVALID_OPCODE 6
#define INT_DEVICE_NOT_AVAIL 7
#define INT_DOUBLE_FAULT 8
#define INT_COPROCESSOR 9
#define INT_INVALID_TSS 10
#define INT_SEGMENT 11
#define INT_STACK_FAULT 12
#define INT_GENERAL_PROTECT 13
#define INT_PAGE_FAULT 14

#define INT_X87_FPU 16
#define INT_ALIGNMENT 17
#define INT_MACHINE_CHECK 18
#define INT_SIMD_FLOAT 19
#define INT_VIRTUAL_EXCE 20

typedef struct pt_regs {
    // segment registers
    // 16 bits
    uint32_t gs;
    // 16 bits
    uint32_t fs;
    // 16 bits
    uint32_t es;
    // 16 bits
    uint32_t ds;

    // registers save by pusha
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t old_esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t int_no;
    // save by `int` instruction
    uint32_t err_code;
    // 以下指令由cpu压入，参见x86/x64 532页
    // 指向产生异常的指令
    uint32_t eip;
    // 16 bits
    uint32_t cs;
    uint32_t eflags;
    // 如果发生了特权级切换，CPU 会压入以下两个参数
    uint32_t user_esp;
    // 16 bits
    uint32_t user_ss;
} pt_regs_t;

// 中断描述符
typedef struct idt_entry_t {
    // 中断处理函数地址 15～0 位
    uint16_t base_low;
    // 目标代码段描述符选择子
    uint16_t selector;
    // 置 0 段
    uint8_t zero;
    // 一些标志，文档有解释
    uint8_t flags;
    // 中断处理函数地址 31～16 位
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

// IDTR
typedef struct idt_ptr_t {
    // 限长
    uint16_t limit;
    // 基址
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

// 声明中断处理函数 0 ~ 19 属于 CPU 的异常中断
// ISR:中断服务程序(interrupt service routine)
// 0 #DE 除 0 异常
extern void isr0();
// 1 #DB 调试异常
extern void isr1();
// 2 NMI
extern void isr2();
// 3 BP 断点异常
extern void isr3();
// 4 #OF 溢出
extern void isr4();
// 5 #BR 对数组的引用超出边界
extern void isr5();
// 6 #UD 无效或未定义的操作码
extern void isr6();
// 7 #NM 设备不可用(无数学协处理器)
extern void isr7();
// 8 #DF 双重故障(有错误代码)
extern void isr8();
// 9 协处理器跨段操作
extern void isr9();
// 10 #TS 无效TSS(有错误代码)
extern void isr10();
// 11 #NP 段不存在(有错误代码)
extern void isr11();
// 12 #SS 栈错误(有错误代码)
extern void isr12();
// 13 #GP 常规保护(有错误代码)
extern void isr13();
// 14 #PF 页故障(有错误代码)
extern void isr14();
// 15 CPU 保留
extern void isr15();
// 16 #MF 浮点处理单元错误
extern void isr16();
// 17 #AC 对齐检查
extern void isr17();
// 18 #MC 机器检查
extern void isr18();
// 19 #XM SIMD(单指令多数据)浮点异常
extern void isr19();

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
// 0x80 用于实现系统调用
extern void isr128();

// 声明 IRQ 函数
// IRQ:中断请求(Interrupt Request)
// 电脑系统计时器
extern void irq0();
// 键盘
extern void irq1();
// 与 IRQ9 相接，MPU-401 MD 使用
extern void irq2();
// 串口设备
extern void irq3();
// 串口设备
extern void irq4();
// 建议声卡使用
extern void irq5();
// 软驱传输控制使用
extern void irq6();
// 打印机传输控制使用
extern void irq7();
// 即时时钟
extern void irq8();
// 与 IRQ2 相接，可设定给其他硬件
extern void irq9();
// 建议网卡使用
extern void irq10();
// 建议 AGP 显卡使用
extern void irq11();
// 接 PS/2 鼠标，也可设定给其他硬件
extern void irq12();
// 协处理器使用
extern void irq13();
// IDE0 传输控制使用
extern void irq14();
// IDE1 传输控制使用
extern void irq15();

// IRQ 处理函数
void irq_handler(pt_regs_t *regs);

// 中断处理函数指针
typedef void (*interrupt_handler_t)(pt_regs_t *);

// 调用中断处理函数
void isr_handler(pt_regs_t *regs);

// 注册一个中断处理函数
void register_interrupt_handler(uint8_t n, interrupt_handler_t h);

// 声明加载 IDTR 的函数
extern void idt_load(uint32_t);

// 中断处理函数指针
typedef void (*isr_irq_func_t)();

// intr 初始化
void intr_init(void);
// 输出 pt_regs 信息
void show_pt_regs(pt_regs_t *pt_regs);
// 系统中断
void divide_error(pt_regs_t *regs);
void debug(pt_regs_t *regs);
void nmi(pt_regs_t *regs);
void breakpoint(pt_regs_t *regs);
void overflow(pt_regs_t *regs);
void bound(pt_regs_t *regs);
void invalid_opcode(pt_regs_t *regs);
void device_not_available(pt_regs_t *regs);
void double_fault(pt_regs_t *regs);
void coprocessor_error(pt_regs_t *regs);
void invalid_TSS(pt_regs_t *regs);
void segment_not_present(pt_regs_t *regs);
void stack_segment(pt_regs_t *regs);
void general_protection(pt_regs_t *regs);
void page_fault(pt_regs_t *regs);

void enable_irq(uint32_t irq_no);
void disable_irq(uint32_t irq_no);

#ifdef __cplusplus
}
#endif

#endif /* _INTR_H_ */
