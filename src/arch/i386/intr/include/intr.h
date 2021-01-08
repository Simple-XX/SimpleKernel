
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// intr.h for Simple-XX/SimpleKernel.

#ifndef _INTR_H_
#define _INTR_H_

#include "stdint.h"
#include "console.h"
#include "port.h"
#include "8259A.h"

// 中断表最大值
const uint32_t INTERRUPT_MAX = 256;

// 定义IRQ
// 电脑系统计时器
const uint32_t IRQ0 = 32;
// 键盘
const uint32_t IRQ1 = 33;
// 与 IRQ9 相接，MPU-401 MD 使用
const uint32_t IRQ2 = 34;
// 串口设备
const uint32_t IRQ3 = 35;
// 串口设备
const uint32_t IRQ4 = 36;
// 建议声卡使用
const uint32_t IRQ5 = 37;
// 软驱传输控制使用
const uint32_t IRQ6 = 38;
// 打印机传输控制使用
const uint32_t IRQ7 = 39;
// 即时时钟
const uint32_t IRQ8 = 40;
// 与 IRQ2 相接，可设定给其他硬件
const uint32_t IRQ9 = 41;
// 建议网卡使用
const uint32_t IRQ10 = 42;
// 建议 AGP 显卡使用
const uint32_t IRQ11 = 43;
// 接 PS/2 鼠标，也可设定给其他硬件
const uint32_t IRQ12 = 44;
// 协处理器使用
const uint32_t IRQ13 = 45;
// IDE0 传输控制使用
const uint32_t IRQ14 = 46;
// IDE1 传输控制使用
const uint32_t IRQ15 = 47;

// 中断号定义
const uint32_t INT_DIVIDE_ERROR     = 0;
const uint32_t INT_DEBUG            = 1;
const uint32_t INT_NMI              = 2;
const uint32_t INT_BREAKPOINT       = 3;
const uint32_t INT_OVERFLOW         = 4;
const uint32_t INT_BOUND            = 5;
const uint32_t INT_INVALID_OPCODE   = 6;
const uint32_t INT_DEVICE_NOT_AVAIL = 7;
const uint32_t INT_DOUBLE_FAULT     = 8;
const uint32_t INT_COPROCESSOR      = 9;
const uint32_t INT_INVALID_TSS      = 10;
const uint32_t INT_SEGMENT          = 11;
const uint32_t INT_STACK_FAULT      = 12;
const uint32_t INT_GENERAL_PROTECT  = 13;
const uint32_t INT_PAGE_FAULT       = 14;

const uint32_t INT_X87_FPU       = 16;
const uint32_t INT_ALIGNMENT     = 17;
const uint32_t INT_MACHINE_CHECK = 18;
const uint32_t INT_SIMD_FLOAT    = 19;
const uint32_t INT_VIRTUAL_EXCE  = 20;

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
    uint32_t eip; // 指向产生异常的指令
    uint32_t cs;  // 16 bits
    uint32_t eflags;
    // 如果发生了特权级切换，CPU 会压入以下两个参数
    uint32_t user_esp;
    uint32_t user_ss; // 16 bits
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

// 声明中断处理函数 0 ~ 19 属于 CPU 的异常中断
// ISR:中断服务程序(interrupt service routine)
// 0 #DE 除 0 异常
extern "C" void isr0();
// 1 #DB 调试异常
extern "C" void isr1();
// 2 NMI
extern "C" void isr2();
// 3 BP 断点异常
extern "C" void isr3();
// 4 #OF 溢出
extern "C" void isr4();
// 5 #BR 对数组的引用超出边界
extern "C" void isr5();
// 6 #UD 无效或未定义的操作码
extern "C" void isr6();
// 7 #NM 设备不可用(无数学协处理器)
extern "C" void isr7();
// 8 #DF 双重故障(有错误代码)
extern "C" void isr8();
// 9 协处理器跨段操作
extern "C" void isr9();
// 10 #TS 无效TSS(有错误代码)
extern "C" void isr10();
// 11 #NP 段不存在(有错误代码)
extern "C" void isr11();
// 12 #SS 栈错误(有错误代码)
extern "C" void isr12();
// 13 #GP 常规保护(有错误代码)
extern "C" void isr13();
// 14 #PF 页故障(有错误代码)
extern "C" void isr14();
// 15 CPU 保留
extern "C" void isr15();
// 16 #MF 浮点处理单元错误
extern "C" void isr16();
// 17 #AC 对齐检查
extern "C" void isr17();
// 18 #MC 机器检查
extern "C" void isr18();
// 19 #XM SIMD(单指令多数据)浮点异常
extern "C" void isr19();

// 20 ~ 31 Intel 保留
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();

// 32 ~ 255 用户自定义异常
extern "C" void isr128(); // 0x80 用于实现系统调用

// 声明 IRQ 函数
// IRQ:中断请求(Interrupt Request)
// 电脑系统计时器
extern "C" void irq0();
// 键盘
extern "C" void irq1();
// 与 IRQ9 相接，MPU-401 MD 使用
extern "C" void irq2();
// 串口设备
extern "C" void irq3();
// 串口设备
extern "C" void irq4();
// 建议声卡使用
extern "C" void irq5();
// 软驱传输控制使用
extern "C" void irq6();
// 打印机传输控制使用
extern "C" void irq7();
// 即时时钟
extern "C" void irq8();
// 与 IRQ2 相接，可设定给其他硬件
extern "C" void irq9();
// 建议网卡使用
extern "C" void irq10();
// 建议 AGP 显卡使用
extern "C" void irq11();
// 接 PS/2 鼠标，也可设定给其他硬件
extern "C" void irq12();
// 协处理器使用
extern "C" void irq13();
// IDE0 传输控制使用
extern "C" void irq14();
// IDE1 传输控制使用
extern "C" void irq15();

// 定义中断处理函数指针
typedef void (*interrupt_handler_t)(pt_regs_t *);

// 声明加载 IDTR 的函数
extern "C" void idt_load(uint32_t);

// 中断处理函数指针类型
typedef void (*isr_irq_func_t)();

void die(const char *str, uint32_t oesp, uint32_t int_no);

// IRQ 处理函数
extern "C" void irq_handler(pt_regs_t *regs);
// ISR 处理函数
extern "C" void isr_handler(pt_regs_t *regs);

class INTR : virtual A8259A, virtual PORT, virtual CONSOLE {
private:
    // 系统中断
    static void divide_error(pt_regs_t *regs);
    static void debug(pt_regs_t *regs);
    static void nmi(pt_regs_t *regs);
    static void breakpoint(pt_regs_t *regs);
    static void overflow(pt_regs_t *regs);
    static void bound(pt_regs_t *regs);
    static void invalid_opcode(pt_regs_t *regs);
    static void device_not_available(pt_regs_t *regs);
    static void double_fault(pt_regs_t *regs);
    static void coprocessor_error(pt_regs_t *regs);
    static void invalid_TSS(pt_regs_t *regs);
    static void segment_not_present(pt_regs_t *regs);
    static void stack_segment(pt_regs_t *regs);
    static void general_protection(pt_regs_t *regs);
    static void page_fault(pt_regs_t *regs);
    // 设置中断描述符
    static void set_idt(uint8_t num, uint32_t base, uint16_t target,
                        uint8_t flags);
    // 返回中断名
    static const char *intrname(uint32_t intrno);
    // 中断处理函数指针数组
    static isr_irq_func_t      isr_irq_func[INTERRUPT_MAX];
    static interrupt_handler_t interrupt_handlers[INTERRUPT_MAX]
        __attribute__((aligned(4)));
    // 中断描述符表
    static idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__((aligned(16)));
    // IDTR
    static idt_ptr_t idt_ptr;

protected:
public:
    INTR(void);
    ~INTR(void);
    // 注册一个中断处理函数
    static void register_interrupt_handler(uint8_t n, interrupt_handler_t h);
    static void enable_irq(uint32_t irq_no);
    static void disable_irq(uint32_t irq_no);
    friend void irq_handler(pt_regs_t *regs);
    friend void isr_handler(pt_regs_t *regs);
};

#endif /* _INTR_H_ */
