
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// timer.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "stdio.h"
#include "cpu.hpp"
#include "opensbi.h"
#include "intr.h"

namespace TIMER {
    // timer interrupt interval
    static constexpr const uint64_t INTERVAL = 390000000 / 200;

    void set_next(void) {
        // 调用 opensbi 提供的接口设置时钟
        opensbi.set_timer(CPU::READ_TIME() + INTERVAL);
    }

    void timer_intr(void) {
        // 每次执行中断时设置下一次中断的时间
        set_next();
    }

    void init(void) {
        // 注册中断函数
        CLINT::register_interrupt_handler(CLINT::INTR_S_TIMER, timer_intr);
        // 设置初次中断
        opensbi.set_timer(CPU::READ_TIME());
        // 开启时钟中断
        CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_STIE);
        printf("timer init\n");
    }
};
