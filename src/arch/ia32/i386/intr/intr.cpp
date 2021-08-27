
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.cpp for Simple-XX/SimpleKernel.

#include "io.h"
#include "cpu.hpp"
#include "stdio.h"
#include "gdt.h"
#include "intr.h"
#include "apic.h"
#include "keyboard.h"
#include "vmm.h"

// 声明中断处理函数 0 ~ 19 属于 CPU 的异常中断
// ISR:中断服务程序(interrupt service routine)
// 0 #DE 除 0 异常
extern "C" void isr0(void);
// 1 #DB 调试异常
extern "C" void isr1(void);
// 2 NMI
extern "C" void isr2(void);
// 3 BP 断点异常
extern "C" void isr3(void);
// 4 #OF 溢出
extern "C" void isr4(void);
// 5 #BR 对数组的引用超出边界
extern "C" void isr5(void);
// 6 #UD 无效或未定义的操作码
extern "C" void isr6(void);
// 7 #NM 设备不可用(无数学协处理器)
extern "C" void isr7(void);
// 8 #DF 双重故障(有错误代码)
extern "C" void isr8(void);
// 9 协处理器跨段操作
extern "C" void isr9(void);
// 10 #TS 无效TSS(有错误代码)
extern "C" void isr10(void);
// 11 #NP 段不存在(有错误代码)
extern "C" void isr11(void);
// 12 #SS 栈错误(有错误代码)
extern "C" void isr12(void);
// 13 #GP 常规保护(有错误代码)
extern "C" void isr13(void);
// 14 #PF 页故障(有错误代码)
extern "C" void isr14(void);
// 15 没有使用
// 16 #MF 浮点处理单元错误
extern "C" void isr16(void);
// 17 #AC 对齐检查
extern "C" void isr17(void);
// 18 #MC 机器检查
extern "C" void isr18(void);
// 19 #XM SIMD(单指令多数据)浮点异常
extern "C" void isr19(void);
extern "C" void isr20(void);
// 21 ~ 31 Intel 保留
// 32 ~ 255 用户自定义异常
// 0x80 用于实现系统调用
extern "C" void isr128(void);

// IRQ:中断请求(Interrupt Request)
// 电脑系统计时器
extern "C" void irq0(void);
// 键盘
extern "C" void irq1(void);
// 与 IRQ9 相接，MPU-401 MD 使用
extern "C" void irq2(void);
// 串口设备
extern "C" void irq3(void);
// 串口设备
extern "C" void irq4(void);
// 建议声卡使用
extern "C" void irq5(void);
// 软驱传输控制使用
extern "C" void irq6(void);
// 打印机传输控制使用
extern "C" void irq7(void);
// 即时时钟
extern "C" void irq8(void);
// 与 IRQ2 相接，可设定给其他硬件
extern "C" void irq9(void);
// 建议网卡使用
extern "C" void irq10(void);
// 建议 AGP 显卡使用
extern "C" void irq11(void);
// 接 PS/2 鼠标，也可设定给其他硬件
extern "C" void irq12(void);
// 协处理器使用
extern "C" void irq13(void);
// IDE0 传输控制使用
extern "C" void irq14(void);
// IDE1 传输控制使用
extern "C" void irq15(void);
// 声明加载 IDTR 的函数
extern "C" void idt_load(uint32_t);

// IRQ 处理函数
extern "C" void irq_handler(uint8_t _no, INTR::intr_context_t *_intr_context) {
    INTR::call_irq(_no, _intr_context);
    return;
}

// ISR 处理函数
extern "C" void isr_handler(uint8_t _no, INTR::intr_context_t *_intr_context,
                            INTR::error_code_t *_err_code) {
    (void)_err_code;
    INTR::call_isr(_no, _intr_context);
    return;
}

// 默认处理函数
static void handler_default(INTR::intr_context_t *) {
    while (1) {
        ;
    }
    return;
}

// 中断处理函数指针数组
INTR::interrupt_handler_t INTR::interrupt_handlers[INTERRUPT_MAX];
// 中断描述符表
INTR::idt_entry32_t INTR::idt_entry32[INTERRUPT_MAX];
// IDTR
INTR::idt_ptr_t INTR::idt_ptr;

// 64-ia-32-architectures-software-developer-vol-3a-manual#6.11
void INTR::set_idt(uint8_t _num, uint32_t _base, uint16_t _selector,
                   uint8_t _type, uint8_t _dpl, uint8_t _p) {
    idt_entry32[_num].offset0  = _base & 0xFFFF;
    idt_entry32[_num].selector = _selector;
    idt_entry32[_num].reserved = 0;
    idt_entry32[_num].zero     = 0;
    idt_entry32[_num].type     = _type;
    idt_entry32[_num].dpl      = _dpl;
    idt_entry32[_num].p        = _p;
    idt_entry32[_num].offset1  = (_base >> 16) & 0xFFFF;
    return;
}

void INTR::init_interrupt_chip(void) {
    // 重新映射 IRQ 表
    // 两片级联的 Intel 8259A 芯片
    // 主片端口 0x20 0x21
    // 从片端口 0xA0 0xA1

    // 初始化主片、从片
    // 0001 0001
    io.outb(IO_PIC1, 0x11);
    // 设置主片 IRQ 从 0x20(32) 号中断开始
    io.outb(IO_PIC1C, IRQ0);
    // 设置主片 IR2 引脚连接从片
    io.outb(IO_PIC1C, 0x04);
    // 设置主片按照 EOI 的方式工作
    // 在这种模式下，中断处理完后需要通知 8259A 重置 ISR 寄存器
    // 即调用 clear_interrupt_chip()
    io.outb(IO_PIC1C, 0x01);

    io.outb(IO_PIC2, 0x11);
    // 设置从片 IRQ 从 0x28(40) 号中断开始
    io.outb(IO_PIC2C, IRQ8);
    // 告诉从片输出引脚和主片 IR2 号相连
    io.outb(IO_PIC2C, 0x02);
    // 设置从片按照 EOI 的方式工作
    io.outb(IO_PIC2C, 0x01);

    // 默认关闭所有中断
    io.outb(IO_PIC1C, 0xFF);
    io.outb(IO_PIC2C, 0xFF);
    return;
}

void INTR::clear_interrupt_chip(uint8_t _no) {
    // 发送中断结束信号给 PICs
    // 按照我们的设置，从 32 号中断起为用户自定义中断
    // 因为单片的 Intel 8259A 芯片只能处理 8 级中断
    // 故大于等于 40 的中断号是由从片处理的
    if (_no >= IRQ8) {
        // 发送重设信号给从片
        io.outb(IO_PIC2, PIC_EOI);
    }
    // 发送重设信号给主片
    io.outb(IO_PIC1, PIC_EOI);
    return;
}

void INTR::disable_interrupt_chip(void) {
    // 屏蔽所有中断
    io.outb(IO_PIC1C, 0xFF);
    io.outb(IO_PIC2C, 0xFF);
    return;
}

int32_t INTR::init(void) {
    // 填充中断描述符
    idt_ptr.limit = sizeof(idt_entry32_t) * INTERRUPT_MAX - 1;
    idt_ptr.base  = (uintptr_t)&idt_entry32;

    // 先全部填充
    for (uint32_t i = 0; i < INTERRUPT_MAX; i++) {
        set_idt(i, 0x0, GDT::SEG_KERNEL_CODE,
                GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
                GDT::SEGMENT_PRESENT);
    }

    // 设置软中断
    set_idt(INT_DIVIDE_ERROR, (uintptr_t)isr0, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_DEBUG, (uintptr_t)isr1, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_NMI, (uintptr_t)isr2, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_BREAKPOINT, (uintptr_t)isr3, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_OVERFLOW, (uintptr_t)isr4, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_BOUND, (uintptr_t)isr5, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_INVALID_OPCODE, (uintptr_t)isr6, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_DEVICE_NOT_AVAIL, (uintptr_t)isr7, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_DOUBLE_FAULT, (uintptr_t)isr8, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_COPROCESSOR, (uintptr_t)isr9, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_INVALID_TSS, (uintptr_t)isr10, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_SEGMENT, (uintptr_t)isr11, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_STACK_FAULT, (uintptr_t)isr12, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_GENERAL_PROTECT, (uintptr_t)isr13, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_PAGE_FAULT, (uintptr_t)isr14, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_X87_FPU, (uintptr_t)isr16, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_ALIGNMENT, (uintptr_t)isr17, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_MACHINE_CHECK, (uintptr_t)isr18, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_SIMD_FLOAT, (uintptr_t)isr19, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(INT_VIRTUAL_EXCE, (uintptr_t)isr20, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);

    // 设置外部中断
    set_idt(IRQ0, (uintptr_t)irq0, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ1, (uintptr_t)irq1, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ2, (uintptr_t)irq2, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ3, (uintptr_t)irq3, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ4, (uintptr_t)irq4, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ5, (uintptr_t)irq5, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ6, (uintptr_t)irq6, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ7, (uintptr_t)irq7, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ8, (uintptr_t)irq8, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ9, (uintptr_t)irq9, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ10, (uintptr_t)irq10, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ11, (uintptr_t)irq11, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ12, (uintptr_t)irq12, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ13, (uintptr_t)irq13, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ14, (uintptr_t)irq14, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    set_idt(IRQ15, (uintptr_t)irq15, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL0,
            GDT::SEGMENT_PRESENT);
    // 填充系统调用中断
    set_idt(IRQ128, (uintptr_t)isr128, GDT::SEG_KERNEL_CODE,
            GDT::TYPE_SYSTEM_32_INTERRUPT_GATE, CPU::DPL3,
            GDT::SEGMENT_PRESENT);
    // 所有中断设为默认
    for (uint32_t i = 0; i < INTERRUPT_MAX; i++) {
        register_interrupt_handler(i, handler_default);
    }
    // idt 初始化
    init_interrupt_chip();
    // 加载 idt
    idt_load((uintptr_t)&idt_ptr);
    // APIC 初始化
    apic.init();
    // 键盘初始化
    keyboard.init();
    info("intr init.\n");
    return 0;
}

int32_t INTR::call_irq(uint8_t _no, intr_context_t *_intr_context) {
    // 重设PIC芯片
    clear_interrupt_chip(_no);
    if (interrupt_handlers[_no] != nullptr) {
        interrupt_handlers[_no](_intr_context);
    }
    return 0;
}

int32_t INTR::call_isr(uint8_t _no, intr_context_t *_intr_context) {
    if (interrupt_handlers[_no] != nullptr) {
        interrupt_handlers[_no](_intr_context);
    }
    else {
        warn("Unhandled interrupt: %d %s\n", _no, get_intr_name(_no));
        CPU::hlt();
    }
    return 0;
}

void INTR::register_interrupt_handler(uint8_t             _no,
                                      interrupt_handler_t _handler) {
    interrupt_handlers[_no] = _handler;
    return;
}

void INTR::enable_irq(uint8_t _no) {
    uint8_t mask = 0;
    // printk_color(green, "enable_irq mask: %X", mask);
    if (_no >= IRQ8) {
        mask = ((io.inb(IO_PIC2C)) & (~(1 << (_no % 8))));
        io.outb(IO_PIC2C, mask);
    }
    else {
        mask = ((io.inb(IO_PIC1C)) & (~(1 << (_no % 8))));
        io.outb(IO_PIC1C, mask);
    }
    return;
}

void INTR::disable_irq(uint8_t _no) {
    uint8_t mask = 0;
    // printk_color(green, "disable_irq mask: %X", mask);
    if (_no >= IRQ8) {
        mask = ((io.inb(IO_PIC2C)) | (1 << (_no % 8)));
        io.outb(IO_PIC2C, mask);
    }
    else {
        mask = ((io.inb(IO_PIC1C)) | (1 << (_no % 8)));
        io.outb(IO_PIC1C, mask);
    }
    return;
}

const char *INTR::get_intr_name(uint8_t _no) {
    if (_no < sizeof(intrnames) / sizeof(const char *const)) {
        return intrnames[_no];
    }
    return "(unknown trap)";
}
