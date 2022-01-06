
/**
 * @file timer.cpp
 * @brief 时钟中断实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-06<td>MRNIU<td>新增文件
 * </table>
 */

#include "stdint.h"
#include "stdio.h"
#include "cpu.hpp"
#include "intr.h"
#include "io.h"

/**
 * @brief 时钟中断
 */
void timer_intr(INTR::intr_context_t *) {
    printf("timer.\n");
    return;
}

TIMER &TIMER::get_instance(void) {
    /// 定义全局 TIMER 对象
    static TIMER timer;
    return timer;
}

void TIMER::init(void) {
    // 注册中断函数
    INTR::get_instance().register_interrupt_handler(INTR::IRQ0, timer_intr);
    // 开启时钟中断
    INTR::get_instance().enable_irq(INTR::IRQ0);
    info("timer init.\n");
    return;
}

void TIMER::init_other_core(void) {
    // 开启时钟中断
    INTR::get_instance().enable_irq(INTR::IRQ0);
    info("timer other init.\n");
    return;
}
