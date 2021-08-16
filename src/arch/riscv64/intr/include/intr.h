
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// intr.h for Simple-XX/SimpleKernel.

#ifndef _INTR_H_
#define _INTR_H_

#include "stdint.h"

void handler_default(void);

class INTR {
public:
    typedef void (*interrupt_handler_t)(void);
    INTR(void);
    ~INTR(void);
    static int32_t init(void);
};

// core-local interrupt controller
// 本地核心中断控制器
// 用于控制 excp 与 intr
class CLINT {
public:
    // 页读错误
    static constexpr const uint8_t EXCP_LOAD_PAGE_FAULT = 13;
    // 页写错误
    static constexpr const uint8_t     EXCP_STORE_PAGE_FAULT = 15;
    static constexpr const char *const excp_names[]          = {
        "Instruction Address Misaligned",
        "Instruction Access Fault",
        "Illegal Instruction",
        "Breakpoint",
        "Load Address Misaligned",
        "Load Access Fault",
        "Store/AMO Address Misaligned",
        "Store/AMO Access Fault",
        "Environment Call from U-mode",
        "Environment Call from S-mode",
        "Reserved",
        "Environment Call from M-mode",
        "Instruction Page Fault",
        "Load Page Fault",
        "Reserved",
        "Store/AMO Page Fault",
        "Reserved",
    };

    // S 态时钟中断
    static constexpr const uint8_t INTR_S_TIMER = 5;
    // S 态外部中断
    static constexpr const uint8_t INTR_S_EXTERNEL = 9;

    static constexpr const char *const intr_names[] = {
        "User Software Interrupt",
        "Supervisor Software Interrupt",
        "Reserved",
        "Machine Software Interrupt",
        "User Timer Interrupt",
        "Supervisor Timer Interrupt",
        "Reserved",
        "Machine Timer Interrupt",
        "User External Interrupt",
        "Supervisor External Interrupt",
        "Reserved",
        "Machine External Interrupt",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Local Interrupt X",
    };

    // 最大中断数
    static constexpr const uint32_t INTERRUPT_MAX = 16;
    // 最大异常数
    static constexpr const uint32_t EXCP_MAX = 16;
    // 中断处理函数数组
    static INTR::interrupt_handler_t interrupt_handlers[INTERRUPT_MAX]
        __attribute__((aligned(4)));
    // 异常处理函数数组
    static INTR::interrupt_handler_t excp_handlers[EXCP_MAX]
        __attribute__((aligned(4)));

    CLINT(void);
    ~CLINT(void);
    static int32_t init(void);
    static void
    register_interrupt_handler(uint8_t                   _no,
                               INTR::interrupt_handler_t _interrupt_handler);
    static void
                register_excp_handler(uint8_t                   _no,
                                      INTR::interrupt_handler_t _interrupt_handler);
    static void do_interrupt(uint8_t _no);
    static void do_excp(uint8_t _no);
};

// platform-level interrupt controller
// 平台级中断控制器
// 用于控制外部中断
class PLIC {
private:
    static uintptr_t      base_addr;
    static const uint64_t PLIC_PRIORITY;
    static const uint64_t PLIC_PENDING;
    static uint64_t       PLIC_SENABLE(uint64_t hart);
    static uint64_t       PLIC_SPRIORITY(uint64_t hart);
    static uint64_t       PLIC_SCLAIM(uint64_t hart);

protected:
public:
    PLIC(void);
    ~PLIC(void);
    static int32_t init(void);
    // 向 PLIC 询问中断
    // 返回发生的外部中断号
    static uint8_t get(void);
    // 告知 PLIC 已经处理了当前 IRQ
    static void done(uint8_t _no);
    static void set(uint8_t _no, bool _status);
};

class TIMER {
public:
    TIMER(void);
    ~TIMER(void);
    static void init(void);
};

#endif /* _INTR_H_ */
