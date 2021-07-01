
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// intr.h for Simple-XX/SimpleKernel.

#ifndef _INTR_H_
#define _INTR_H_

#include "stdint.h"
#include "memlayout.h"

namespace INTR {
    typedef void (*interrupt_handler_t)(void);
    int32_t         init(void);
    void            handler_default(void);
    extern "C" void trap_handler(void);
};

// core-local interrupt controller
// 本地核心中断控制器
// 用于控制 excp 与 intr
namespace CLINT {
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

    int32_t init(void);
    void    register_interrupt_handler(uint8_t n, INTR::interrupt_handler_t h);
    void    register_excp_handler(uint8_t n, INTR::interrupt_handler_t h);
    void    do_interrupt(uint8_t n);
    void    do_excp(uint8_t n);
};

namespace TIMER {
    void init(void);
};

// platform-level interrupt controller
// 平台级中断控制器
// 用于控制外部中断
namespace PLIC {
    // VIRTIO0 中断
    // TODO: 动态获取
    static constexpr const uint8_t VIRTIO0_INTR = MEMLAYOUT::VIRTIO0_IRQ;
    int32_t                        init(void);
    // 向 PLIC 询问中断
    // 返回发生的外部中断号
    uint8_t get(void);
    // 告知PLIC已经处理了当前IRQ
    void done(uint8_t _irq);
    void set(uint8_t irq_no, bool _status);
    // 注册外部中断处理函数()
    void register_externel_handler(uint8_t n, INTR::interrupt_handler_t h);
};

#endif /* _INTR_H_ */
