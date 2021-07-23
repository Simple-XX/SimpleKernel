
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

    // External(hardware generated) interrupts.
    // 64-ia-32-architectures-software-developer-vol-3a-manual#6.3.1
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
    // 15 没有使用
    static constexpr const uint32_t INT_X87_FPU       = 16;
    static constexpr const uint32_t INT_ALIGNMENT     = 17;
    static constexpr const uint32_t INT_MACHINE_CHECK = 18;
    static constexpr const uint32_t INT_SIMD_FLOAT    = 19;
    static constexpr const uint32_t INT_VIRTUAL_EXCE  = 20;
    // 21~31 保留

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
    // SATA 主硬盘
    static constexpr const uint32_t IRQ14 = 46;
    // SATA 从硬盘
    static constexpr const uint32_t IRQ15 = 47;

    // 系统调用
    static constexpr const uint32_t IRQ128 = 128;

    // 8259A 相关定义
    // Master (IRQs 0-7)
    static constexpr const uint32_t IO_PIC1 = 0x20;
    // Slave  (IRQs 8-15)
    static constexpr const uint32_t IO_PIC2  = 0xA0;
    static constexpr const uint32_t IO_PIC1C = IO_PIC1 + 1;
    static constexpr const uint32_t IO_PIC2C = IO_PIC2 + 1;
    // End-of-interrupt command code
    static constexpr const uint32_t PIC_EOI = 0x20;

    class error_code_t {
    public:
        uint32_t ext : 1;
        uint32_t idt : 1;
        uint32_t ti : 1;
        uint32_t sec_idx : 28;
    };

    class page_fault_error_code_t {
    public:
        uint32_t p : 1;
        uint32_t wr : 1;
        uint32_t us : 1;
        uint32_t rsvd : 1;
        uint32_t id : 1;
        uint32_t pk : 1;
        uint32_t reserved1 : 9;
        uint32_t sgx : 1;
        uint32_t reserved2 : 16;
    };

    class pt_regs_t {
    public:
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
    };

    // 中断描述符
    // 64-ia-32-architectures-software-developer-vol-3a-manual#6.11
    struct idt_entry32_t {
        // 低位地址
        uint64_t offset0 : 16;
        // 选择子
        uint64_t selector : 16;
        // 保留
        uint64_t reserved : 4;
        // 填充 0
        uint64_t zero : 4;
        // 类型
        uint64_t type : 5;
        // 权限
        uint64_t dpl : 2;
        // 存在位
        uint64_t p : 1;
        // 中段地址
        uint64_t offset1 : 16;
    } __attribute__((packed));

    // IDTR
    // 64-ia-32-architectures-software-developer-vol-3a-manual#6.10
    struct idt_ptr_t {
        // 限长
        uint16_t limit;
        // 基址
        uint32_t base;
    } __attribute__((packed));

    // 定义中断处理函数指针
    typedef void (*interrupt_handler_t)(pt_regs_t *);

    // 中断处理函数指针类型
    typedef void (*isr_irq_func_t)();

    void die(const char *str, uintptr_t oesp, uint32_t int_no);

    // 设置 8259A 芯片
    void init_interrupt_chip(void);
    // 重设 8259A 芯片
    void clear_interrupt_chip(uint32_t intr_no);

    // 设置中断描述符
    void set_idt(uint8_t _num, uintptr_t _base, uint16_t _selector,
                 uint8_t _ist, uint8_t _type, uint8_t _dpl, uint8_t _p);
    // 中断名数组
    static constexpr const char *const intrnames[] = {
        "Divide Error",
        "Debug Exception",
        "NMI Interrupt",
        "Breakpoint",
        "Overflow",
        "BOUND Range Exceeded",
        "Invalid Opcode (Undefined Opcode)",
        "Device Not Available (No Math Coprocessor)",
        "Double Fault",
        "Coprocessor Segment Overrun (reserved)",
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection",
        "Page Fault",
        "(Intel reserved. Do not use.)",
        "x87 FPU Floating-Point Error (Math Fault)",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
    };
    int32_t init(void);
    // 注册一个中断处理函数
    void register_interrupt_handler(uint8_t n, interrupt_handler_t h);
    void enable_irq(uint8_t irq_no);
    void disable_irq(uint8_t irq_no);
    // 返回中断名
    const char *get_intr_name(uint8_t intr_no);
    // 执行中断
    int32_t call_irq(uint8_t intr_no, pt_regs_t *regs);
    int32_t call_isr(uint8_t intr_no, pt_regs_t *regs);
};

#endif /* _INTR_H_ */
