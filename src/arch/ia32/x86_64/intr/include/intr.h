
/**
 * @file intr.h
 * @brief 中断抽象头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef _INTR_H_
#define _INTR_H_

#include "stdint.h"

/// @todo 升级为 APIC
class INTR {
public:
    /**
     * @brief 错误码结构
     */
    struct error_code_t {
        uint32_t ext : 1;
        uint32_t idt : 1;
        uint32_t ti : 1;
        uint32_t sec_idx : 28;
    };

    /**
     * @brief 缺页错误码结构
     */
    struct page_fault_error_code_t {
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

    /**
     * @brief 中断上下文结构
     * @see 64-ia-32-architectures-software-developer-vol-3a-manual#6.12.1
     */
    struct intr_context_t {
        uint64_t r15;
        uint64_t r14;
        uint64_t r13;
        uint64_t r12;
        uint64_t r11;
        uint64_t r10;
        uint64_t r9;
        uint64_t r8;
        uint64_t rbp;
        uint64_t rdi;
        uint64_t rsi;
        uint64_t rdx;
        uint64_t rcx;
        uint64_t rbx;
        uint64_t rax;
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t rsp;
        uint64_t ss;
    };

    // 定义中断处理函数指针
    typedef void (*interrupt_handler_t)(intr_context_t *);

private:
    /// 中断表最大值
    static constexpr const uint32_t INTERRUPT_MAX = 256;
    /// 8259A 相关定义
    /// Master (IRQs 0-7)
    static constexpr const uint32_t IO_PIC1 = 0x20;
    /// Slave  (IRQs 8-15)
    static constexpr const uint32_t IO_PIC2  = 0xA0;
    static constexpr const uint32_t IO_PIC1C = IO_PIC1 + 1;
    static constexpr const uint32_t IO_PIC2C = IO_PIC2 + 1;
    /// End-of-interrupt command code
    static constexpr const uint32_t PIC_EOI = 0x20;

    /// 中断名数组
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

    /**
     * @brief 中断描述符结构
     * @see 64-ia-32-architectures-software-developer-vol-3a-manual#6.14.1
     */
    struct idt_entry64_t {
        // 低位地址
        uint64_t offset0 : 16;
        // 选择子
        uint64_t selector : 16;
        // 中断栈表
        // 64-ia-32-architectures-software-developer-vol-3a-manual#6.14.5
        uint64_t ist : 3;
        // 填充 0
        uint64_t zero0 : 5;
        // 类型
        uint64_t type : 4;
        // 填充 0
        uint64_t zero1 : 1;
        // 权限
        uint64_t dpl : 2;
        // 存在位
        uint64_t p : 1;
        // 中段地址
        uint64_t offset1 : 16;
        // 高位地址
        uint64_t offset2 : 32;
        // 保留
        uint64_t reserved : 32;
    } __attribute__((packed));

    /**
     * @brief 中断向量寄存器结构
     * @see 64-ia-32-architectures-software-developer-vol-3a-manual#6.10
     */
    struct idt_ptr_t {
        // 限长
        uint16_t limit;
        // 基址
        uint32_t base;
    } __attribute__((packed));

    /// 中断处理函数指针数组
    static interrupt_handler_t interrupt_handlers[INTERRUPT_MAX]
        __attribute__((aligned(4)));
    /// 中断描述符表
    static idt_entry64_t idt_entry64[INTERRUPT_MAX]
        __attribute__((aligned(16)));
    /// IDTR
    static idt_ptr_t idt_ptr;

    /**
     * @brief 设置中断描述符
     * @param  _num            中断号
     * @param  _base           基址
     * @param  _selector       ？
     * @param  _ist            ？
     * @param  _type           ？
     * @param  _dpl            ？
     * @param  _p              ？
     */
    static void set_idt(uint8_t _num, uintptr_t _base, uint16_t _selector,
                        uint8_t _ist, uint8_t _type, uint8_t _dpl, uint8_t _p);

    /**
     * @brief 8259A 芯片初始化
     */
    static void init_interrupt_chip(void);

    /**
     * @brief 重设 8259A 芯片
     * @param  _no             要重设的中断号
     */
    static void clear_interrupt_chip(uint8_t _no);

    /**
     * @brief 关闭 8259A 芯片的所有中断，为启动 APIC 作准备
     */
    static void disable_interrupt_chip(void);

public:
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

    /**
     * @brief 中断初始化
     * @return int32_t         desc
     */
    static int32_t init(void);

    /**
     * @brief 执行中断
     * @param  _no             中断号
     * @param  _intr_context   上下文
     * @return int32_t         保存中断处理后的返回值
     */
    static int32_t call_irq(uint8_t _no, intr_context_t *_intr_context);

    static int32_t call_isr(uint8_t _no, intr_context_t *_intr_context);

    /**
     * @brief 注册一个中断处理函数
     * @param  _no             中断号
     * @param  _handler        中断处理函数
     */
    static void register_interrupt_handler(uint8_t             _no,
                                           interrupt_handler_t _handler);

    /**
     * @brief 打开指定中断
     * @param  _no             要允许的中断号
     */
    static void enable_irq(uint8_t _no);

    /**
     * @brief 关闭指定中断
     * @param  _no             要允许的中断号
     */
    static void disable_irq(uint8_t _no);

    /**
     * @brief 返回中断名
     * @param  _no             中断号
     * @return const char*     对应的中断名
     */
    static const char *get_intr_name(uint8_t _no);
};

#endif /* _INTR_H_ */
