
/**
 * @file timer.h
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

#include "stdint.h"
#include "stdio.h"
#include "cpu.hpp"
#include "opensbi.h"
#include "intr.h"
#include "task.h"
#include "scheduler.h"

spinlock_t TIMER::spinlock;

/// timer interrupt interval
/// @todo 从 dts 读取
static constexpr const uint64_t INTERVAL = 390000000 / 20;

/**
 * @brief 设置下一次时钟
 */
void set_next(void) {
    // 调用 opensbi 提供的接口设置时钟
    OPENSBI::get_instance().set_timer(CPU::READ_TIME() + INTERVAL);
    return;
}

/**
 * @brief 时钟中断
 */
void timer_intr(void) {
    TIMER::spinlock.lock();
    // 每次执行中断时设置下一次中断的时间
    set_next();
    // TODO: 每次时钟中断更新当前任务的执行时间
    SCHEDULER::curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->slice +=
        INTERVAL;
    SCHEDULER::curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]
        ->slice_total += INTERVAL;
    TIMER::spinlock.unlock();
    return;
}

TIMER &TIMER::get_instance(void) {
    /// 定义全局 TIMER 对象
    static TIMER timer;
    return timer;
}

void TIMER::init(void) {
    // 初始化锁
    spinlock.init("TIMER");
    // 注册中断函数
    INTR::get_instance().register_interrupt_handler(INTR::INTR_S_TIMER,
                                                    timer_intr);
    // 设置初次中断
    OPENSBI::get_instance().set_timer(CPU::READ_TIME());
    // 开启时钟中断
    CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_STIE);
    info("timer init.\n");
    return;
}

void TIMER::init_other_core(void) {
    // 设置初次中断
    OPENSBI::get_instance().set_timer(CPU::READ_TIME());
    // 开启时钟中断
    CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_STIE);
    info("timer other init.\n");
    return;
}
