
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

/// timer interrupt interval
// Intel 8253/8254 PIT芯片 I/O端口地址范围是40h~43h
// 输入频率为 1193180，1 即每秒中断次数
static constexpr const uint64_t INTERVAL = 1193180 / 1;

/**
 * @brief 时钟中断
 */
void timer_intr(void) {
    static uint32_t tick = 0;
    err("Tick: %d\n", tick++);
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
    // D7 D6 D5 D4 D3 D2 D1 D0
    // 0  0  1  1  0  1  1  0
    // 即就是 36 H
    // 设置 8253/8254 芯片工作在模式 3 下
    IO::get_instance().outb(0x43, 0x36);

    // 拆分低字节和高字节
    uint8_t low  = (uint8_t)(INTERVAL & 0xFF);
    uint8_t high = (uint8_t)((INTERVAL >> 8) & 0xFF);

    // 分别写入低字节和高字节
    IO::get_instance().outb(0x40, low);
    IO::get_instance().outb(0x40, high);
    info("timer init.\n");
    return;
}
