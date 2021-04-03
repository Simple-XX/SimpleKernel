
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// intr.h for Simple-XX/SimpleKernel.

#ifndef _INTR_H_
#define _INTR_H_

#include "stdint.h"

// TODO: 升级为 APIC

namespace INTR {
    // 中断表最大值
    static constexpr const uint32_t INTERRUPT_MAX = 256;

    // 定义IRQ
    // 电脑系统计时器
    static constexpr const uint32_t IRQ0 = 32;
    // 键盘
    static constexpr const uint32_t IRQ1 = 33;
    // 与 IRQ9 相接，MPU-401 MD 使用
    static constexpr const uint32_t IRQ2 = 34;
    // 串口设备
    static constexpr const uint32_t IRQ3 = 35;
    // 串口设备
    static constexpr const uint32_t IRQ4 = 36;
    // 建议声卡使用
    static constexpr const uint32_t IRQ5 = 37;
    // 软驱传输控制使用
    static constexpr const uint32_t IRQ6 = 38;
    // 打印机传输控制使用
    static constexpr const uint32_t IRQ7 = 39;
    // 即时时钟
    static constexpr const uint32_t IRQ8 = 40;
    // 与 IRQ2 相接，可设定给其他硬件
    static constexpr const uint32_t IRQ9 = 41;
    // 建议网卡使用
    static constexpr const uint32_t IRQ10 = 42;
    // 建议 AGP 显卡使用
    static constexpr const uint32_t IRQ11 = 43;
    // 接 PS/2 鼠标，也可设定给其他硬件
    static constexpr const uint32_t IRQ12 = 44;
    // 协处理器使用
    static constexpr const uint32_t IRQ13 = 45;
    // IDE0 传输控制使用
    static constexpr const uint32_t IRQ14 = 46;
    // IDE1 传输控制使用
    static constexpr const uint32_t IRQ15 = 47;

    // 中断号定义
    static constexpr const uint32_t INT_DIVIDE_ERROR     = 0;
    static constexpr const uint32_t INT_DEBUG            = 1;
    static constexpr const uint32_t INT_NMI              = 2;
    static constexpr const uint32_t INT_BREAKPOINT       = 3;
    static constexpr const uint32_t INT_OVERFLOW         = 4;
    static constexpr const uint32_t INT_BOUND            = 5;
    static constexpr const uint32_t INT_INVALID_OPCODE   = 6;
    static constexpr const uint32_t INT_DEVICE_NOT_AVAIL = 7;
    static constexpr const uint32_t INT_DOUBLE_FAULT     = 8;
    static constexpr const uint32_t INT_COPROCESSOR      = 9;
    static constexpr const uint32_t INT_INVALID_TSS      = 10;
    static constexpr const uint32_t INT_SEGMENT          = 11;
    static constexpr const uint32_t INT_STACK_FAULT      = 12;
    static constexpr const uint32_t INT_GENERAL_PROTECT  = 13;
    static constexpr const uint32_t INT_PAGE_FAULT       = 14;
    static constexpr const uint32_t INT_X87_FPU          = 16;
    static constexpr const uint32_t INT_ALIGNMENT        = 17;
    static constexpr const uint32_t INT_MACHINE_CHECK    = 18;
    static constexpr const uint32_t INT_SIMD_FLOAT       = 19;
    static constexpr const uint32_t INT_VIRTUAL_EXCE     = 20;

    // 8259A 相关定义
    // Master (IRQs 0-7)
    static constexpr const uint32_t IO_PIC1 = 0x20;
    // Slave  (IRQs 8-15)
    static constexpr const uint32_t IO_PIC2  = 0xA0;
    static constexpr const uint32_t IO_PIC1C = IO_PIC1 + 1;
    static constexpr const uint32_t IO_PIC2C = IO_PIC2 + 1;
    // End-of-interrupt command code
    static constexpr const uint32_t PIC_EOI = 0x20;

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
    typedef struct idt_entry {
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
    typedef struct idt_ptr {
        // 限长
        uint16_t limit;
        // 基址
        uint32_t base;
    } __attribute__((packed)) idt_ptr_t;

    // 定义中断处理函数指针
    typedef void (*interrupt_handler_t)(pt_regs_t *);

    // 中断处理函数指针类型
    typedef void (*isr_irq_func_t)();

    void die(const char *str, uint32_t oesp, uint32_t int_no);

    // 设置 8259A 芯片
    void init_interrupt_chip(void);
    // 重设 8259A 芯片
    void clear_interrupt_chip(uint32_t intr_no);

    // 设置中断描述符
    void set_idt(uint8_t num, uint32_t base, uint16_t target, uint8_t flags);
    // 中断名数组
    static constexpr const char *const intrnames[] = {
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
        "SIMD Floating-Point Exception",
    };
    int32_t init(void);
    // 注册一个中断处理函数
    void register_interrupt_handler(uint8_t n, interrupt_handler_t h);
    void enable_irq(uint32_t irq_no);
    void disable_irq(uint32_t irq_no);
    // 返回中断名
    const char *get_intr_name(uint32_t intr_no);
    // 执行中断
    int32_t call_irq(uint32_t intr_no, pt_regs_t *regs);
    int32_t call_isr(uint32_t intr_no, pt_regs_t *regs);

};

#endif /* _INTR_H_ */
