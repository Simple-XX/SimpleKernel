
/**
 * @file intr.cpp
 * @brief 中断抽象
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

#include "cpu.hpp"
#include "stdio.h"
#include "intr.h"
#include "cpu.hpp"
#include "vmm.h"

/**
 * @brief 缺页处理
 */
static void pg_load_excp(void) {
    uintptr_t addr = CPU::READ_STVAL();
    // 映射页
    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), addr, addr,
                             VMM_PAGE_READABLE);
    info("pg_load_excp done: 0x%p.\n", addr);
    return;
}

/**
 * @brief 缺页处理
 */
static void pg_store_excp(void) {
    uintptr_t addr = CPU::READ_STVAL();
    // 映射页
    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), addr, addr,
                             VMM_PAGE_WRITABLE | VMM_PAGE_READABLE);
    info("pg_store_excp done: 0x%p.\n", addr);
    return;
}

/**
 * @brief 外部中断处理 0x9 Supervisor External Interrupt.
 */
static void externel_intr(void) {
    auto isr = PLIC::get_instance().get();
    info("externel_intr: 0x%X\n", isr);
    PLIC::externel_interrupt_handlers[isr]();
    // 根据中断号进行处理
    PLIC::get_instance().done(isr);
    info("externel_intr done: 0x%X.\n", isr);
    return;
}

/**
 * @brief 中断处理函数
 * @param  _scause         原因
 * @param  _sepc           值
 * @param  _stval          值
 */
extern "C" void trap_handler(uint64_t _scause, uint64_t _sepc,
                             uint64_t _stval) {

    // 消除 unused 警告
    (void)_sepc;
    (void)_stval;
#define DEBUG
#ifdef DEBUG
    printf("scause: 0x%p, sepc: 0x%p, stval: 0x%p.\n", _scause, _sepc, _stval);
#undef DEBUG
#endif
    if (_scause & CPU::CAUSE_INTR_MASK) {
// 中断
// #define DEBUG
#ifdef DEBUG
        printf("intr: %s.\n",
               INTR::get_instance().intr_names[_scause & CPU::CAUSE_CODE_MASK]);
#undef DEBUG
#endif
        // 跳转到对应的处理函数
        INTR::get_instance().do_interrupt(_scause & CPU::CAUSE_CODE_MASK);
    }
    else {
// 异常
// 跳转到对应的处理函数
#define DEBUG
#ifdef DEBUG
        printf("excp: %s.\n",
               INTR::get_instance().excp_names[_scause & CPU::CAUSE_CODE_MASK]);
#undef DEBUG
#endif
        INTR::get_instance().do_excp(_scause & CPU::CAUSE_CODE_MASK);
    }
    return;
}

/// 中断处理入口 intr_s.S
extern "C" void trap_entry(void);

/**
 * @brief 默认使用的中断处理函数
 */
void handler_default(void) {
    while (1) {
        ;
    }
    return;
}

INTR &INTR::get_instance(void) {
    /// 定义全局 INTR 对象
    static INTR intr;
    return intr;
}

// 中断处理函数数组
INTR::interrupt_handler_t INTR::interrupt_handlers[INTR::INTERRUPT_MAX];
// 异常处理函数数组
INTR::interrupt_handler_t INTR::excp_handlers[INTR::EXCP_MAX];

int32_t INTR::init(void) {
    // 内部中断初始化
    CLINT::get_instance().init();
    // 外部中断初始化
    PLIC::get_instance().init();
    // 设置 trap vector
    CPU::WRITE_STVEC((uintptr_t)trap_entry);
    // 直接跳转到处理函数
    CPU::STVEC_DIRECT();
    // 设置处理函数
    for (auto &i : interrupt_handlers) {
        i = handler_default;
    }
    for (auto &i : excp_handlers) {
        i = handler_default;
    }
    // 缺页中断
    register_excp_handler(EXCP_LOAD_PAGE_FAULT, pg_load_excp);
    register_excp_handler(EXCP_STORE_PAGE_FAULT, pg_store_excp);
    // S 态外部中断
    register_interrupt_handler(INTR_S_EXTERNEL, externel_intr);
    // 设置时钟中断
    TIMER::get_instance().init();
    info("intr init.\n");
    return 0;
}

void INTR::register_interrupt_handler(
    uint8_t _no, INTR::interrupt_handler_t _interrupt_handler) {
    interrupt_handlers[_no] = _interrupt_handler;
    return;
}

void INTR::register_excp_handler(uint8_t                   _no,
                                 INTR::interrupt_handler_t _interrupt_handler) {
    excp_handlers[_no] = _interrupt_handler;
    return;
}

void INTR::do_interrupt(uint8_t _no) {
    interrupt_handlers[_no]();
    return;
}

void INTR::do_excp(uint8_t _no) {
    excp_handlers[_no]();
    return;
}
