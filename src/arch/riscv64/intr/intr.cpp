
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
#include "scheduler.h"
#include "task.h"
#include "spinlock.h"

/**
 * @brief 中断处理函数
 * @param  _scause         原因
 * @param  _sepc           值
 * @param  _stval          值
 */
extern "C" void trap_handler(uintptr_t _sepc, uintptr_t _stval,
                             uintptr_t _scause, uintptr_t _sp,
                             uintptr_t _sstatus, CPU::context_t *_context) {
    // 消除 unused 警告
    (void)_sepc;
    (void)_stval;
    (void)_scause;
    (void)_sp;
    (void)_sstatus;
    (void)_context;
// #define DEBUG
#ifdef DEBUG
    printf("scause: 0x%p, sepc: 0x%p, stval: 0x%p, hartid: 0x%X.\n", _scause,
           _sepc, _stval, COMMON::get_curr_core_id(CPU::READ_SP()));
#undef DEBUG
#endif
    if (_scause & CPU::CAUSE_INTR_MASK) {
// 中断
// #define DEBUG
#ifdef DEBUG
        printf("intr: %s.\n", INTR::get_instance().get_intr_name(
                                  _scause & CPU::CAUSE_CODE_MASK));
#undef DEBUG
#endif
        // 跳转到对应的处理函数
        INTR::get_instance().do_interrupt(_scause & CPU::CAUSE_CODE_MASK);
        // 如果是时钟中断
        if ((_scause & CPU::CAUSE_CODE_MASK) == INTR::INTR_S_TIMER) {
            // 设置 sepc，切换到内核线程
            _context->sepc = (uintptr_t)&SCHEDULER::switch_to_kernel;
        }
    }
    else {
// 异常
// 跳转到对应的处理函数
#define DEBUG
#ifdef DEBUG
        printf("excp: %s.\n",
               INTR::get_instance().get_excp_name(_scause & CPU::CAUSE_CODE_MASK));
#undef DEBUG
#endif
        INTR::get_instance().do_excp(_scause & CPU::CAUSE_CODE_MASK);
    }
    return;
}

/// 中断处理入口 intr_s.S
extern "C" void trap_entry(void);

/**
 * @brief 缺页处理
 */
void pg_load_excp(void) {
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
void pg_store_excp(void) {
    uintptr_t addr = CPU::READ_STVAL();
    // 映射页
    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), addr, addr,
                             VMM_PAGE_WRITABLE | VMM_PAGE_READABLE);
    info("pg_store_excp done: 0x%p.\n", addr);
    return;
}

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

int32_t INTR::init(void) {
    // 初始化锁
    spinlock.init("INTR");
    // 设置 trap vector
    CPU::WRITE_STVEC((uintptr_t)trap_entry);
    // 直接跳转到处理函数
    CPU::STVEC_DIRECT();
    // 内部中断初始化
    CLINT::get_instance().init();
    // 外部中断初始化
    PLIC::get_instance().init();
    // 设置处理函数
    for (auto &i : interrupt_handlers) {
        i = handler_default;
    }
    for (auto &i : excp_handlers) {
        i = handler_default;
    }
    // 注册缺页中断
    register_excp_handler(EXCP_LOAD_PAGE_FAULT, pg_load_excp);
    // 注册缺页中断
    register_excp_handler(EXCP_STORE_PAGE_FAULT, pg_store_excp);
    info("intr init.\n");
    return 0;
}

int32_t INTR::init_other_core(void) {
    // 设置 trap vector
    CPU::WRITE_STVEC((uintptr_t)trap_entry);
    // 直接跳转到处理函数
    CPU::STVEC_DIRECT();
    // 内部中断初始化
    CLINT::get_instance().init_other_core();
    // 外部中断初始化
    PLIC::get_instance().init_other_core();
    info("intr other 0x%X init.\n", COMMON::get_curr_core_id(CPU::READ_SP()));
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

const char *INTR::get_intr_name(uint8_t _no) const {
    return intr_names[_no];
}

const char *INTR::get_excp_name(uint8_t _no) const {
    return excp_names[_no];
}
