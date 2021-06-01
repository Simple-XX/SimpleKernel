
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// intr.h for Simple-XX/SimpleKernel.

#ifndef _INTR_H_
#define _INTR_H_

#include "stdint.h"

namespace INTR {
    static constexpr const uint8_t     EXCP_PAGE_FAULT = 13;
    static constexpr const char *const excp_names[]    = {
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

    static constexpr const uint8_t INTR_S_TIMER = 5;

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

    typedef void (*interrupt_handler_t)(void);
    extern "C" void trap_entry(void);
    extern "C" void trap_handler(void);
    int32_t         init(void);
    // 注册一个中断处理函数
    void register_interrupt_handler(uint8_t n, interrupt_handler_t h);
    void register_excp_handler(uint8_t n, interrupt_handler_t h);
    void enable_irq(uint32_t irq_no);
    void disable_irq(uint32_t irq_no);
};

namespace TIMER {
    void init(void);
};

#endif /* _INTR_H_ */
