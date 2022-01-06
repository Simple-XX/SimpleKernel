
/**
 * @file timer.cpp
 * @brief 时钟中断实现
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
    // 每次执行中断时设置下一次中断的时间
    set_next();
    return;
}

TIMER &TIMER::get_instance(void) {
    /// 定义全局 TIMER 对象
    static TIMER timer;
    return timer;
}

void TIMER::init(void) {
    // 注册中断函数
    INTR::get_instance().register_interrupt_handler(INTR::INTR_S_TIMER, timer_intr);
    // 设置初次中断
    OPENSBI::get_instance().set_timer(CPU::READ_TIME());
    // 开启时钟中断
    CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_STIE);
    info("timer init.\n");
    return;
}
