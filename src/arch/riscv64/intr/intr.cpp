
/**
 * @file intr.cpp
 * @brief 中断实现
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
#include "vmm.h"
#include "pmm.h"
#include "task.h"
#include "core.h"

extern "C" void switch_context(CPU::context_t *_old, CPU::context_t *_new);

/**
 * @brief 保存当前上下文并跳转到调度线程
 */
static void switch_sched(void) {
    task_t *old = core_t::get_curr_task();
#define DEBUG
#ifdef DEBUG
    info("switch_sched\n");
    std::cout << "old: \n" << *old << std::endl;
    std::cout << "core_t::cores[" << CPU::get_curr_core_id()
              << "].sched_task: \n"
              << *core_t::cores[CPU::get_curr_core_id()].sched_task
              << std::endl;
#undef DEBUG
#endif
    switch_context(&old->context,
                   &core_t::cores[CPU::get_curr_core_id()].sched_task->context);
    return;
}

/**
 * @brief 中断处理函数
 * @param  _scause         原因
 * @param  _sepc           值
 * @param  _stval          值
 * @param  _scause         值
 * @param  _all_regs       保存在栈上的所有寄存器，实际上是 sp
 * @param  _sstatus        值
 * @param  _sstatus        值
 */
extern "C" void trap_handler(uintptr_t _sepc, uintptr_t _stval,
                             uintptr_t _scause, CPU::all_regs_t *_all_regs,
                             uintptr_t _sie, uintptr_t _sstatus,
                             uintptr_t _sscratch) {
    CPU::DISABLE_INTR();
    // 消除 unused 警告
    (void)_sepc;
    (void)_stval;
    (void)_scause;
    (void)_all_regs;
    (void)_sie;
    (void)_sstatus;
    (void)_sscratch;
#define DEBUG
#ifdef DEBUG
    info("sepc: 0x%p, stval: 0x%p, scause: 0x%p, all_regs(sp): 0x%p, sie: "
         "0x%p, sstatus: 0x%p.\n",
         _sepc, _stval, _scause, _all_regs, _sie, _sstatus);
//    std::cout << *_all_regs << std::endl;
#undef DEBUG
#endif
    if (_scause & CPU::CAUSE_INTR_MASK) {
        // 中断
#define DEBUG
#ifdef DEBUG
        info("intr: %s.\n", INTR::get_instance().get_intr_name(
                                _scause & CPU::CAUSE_CODE_MASK));
#undef DEBUG
#endif
        // 跳转到对应的处理函数
        INTR::get_instance().do_interrupt(_scause & CPU::CAUSE_CODE_MASK, 0,
                                          nullptr);
        _all_regs->sstatus |= CPU::SSTATUS_SIE;
        _all_regs->sie |= CPU::SIE_STIE;
        // 如果是时钟中断
        //        if ((_scause & CPU::CAUSE_CODE_MASK) == INTR::INTR_S_TIMER) {
        //            // 设置 sepc，切换到内核线程
        ////            _all_regs->sepc =
        /// reinterpret_cast<uintptr_t>(&switch_sched);
        //        }
    }
    else {
        // 异常
        // 跳转到对应的处理函数
#define DEBUG
#ifdef DEBUG
        warn("excp: %s.\n", INTR::get_instance().get_excp_name(
                                _scause & CPU::CAUSE_CODE_MASK));
#undef DEBUG
#endif
        INTR::get_instance().do_excp(_scause & CPU::CAUSE_CODE_MASK, 0,
                                     nullptr);
    }
    return;
}

/// 中断处理入口 intr_s.S
extern "C" void trap_entry(void);

/**
 * @brief 默认使用的中断处理函数
 */
int32_t handler_default(int, char **) {
    while (1) {
        ;
    }
    return 0;
}

INTR &INTR::get_instance(void) {
    /// 定义全局 INTR 对象
    static INTR intr;
    return intr;
}

int32_t INTR::init(void) {
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
    // 内部中断初始化
    CLINT::get_instance().init();
    // 外部中断初始化
    PLIC::get_instance().init();
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
    info("intr other 0x%X init.\n", CPU::get_curr_core_id());
    return 0;
}

void INTR::register_interrupt_handler(
    uint8_t _no, INTR::interrupt_handler_t _interrupt_handler) {
    spinlock.lock();
    interrupt_handlers[_no] = _interrupt_handler;
    spinlock.unlock();
    return;
}

void INTR::register_excp_handler(uint8_t                   _no,
                                 INTR::interrupt_handler_t _interrupt_handler) {
    spinlock.lock();
    excp_handlers[_no] = _interrupt_handler;
    spinlock.unlock();
    return;
}

int32_t INTR::do_interrupt(uint8_t _no, int32_t _argc, char **_argv) {
    return interrupt_handlers[_no](_argc, _argv);
}

int32_t INTR::do_excp(uint8_t _no, int32_t _argc, char **_argv) {
    return excp_handlers[_no](_argc, _argv);
}

const char *INTR::get_intr_name(uint8_t _no) const {
    return intr_names[_no];
}

const char *INTR::get_excp_name(uint8_t _no) const {
    return excp_names[_no];
}
