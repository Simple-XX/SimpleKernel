
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
    static uint64_t tick = 0;
    // timer interrupt interval
    static constexpr const uint64_t INTERVAL = 390000000 / 200;

    void set_next(void) {
        opensbi.set_timer(CPU::READ_TIME() + INTERVAL);
    }

    void timer_intr(void) {
        set_next();
        tick++;
        if ((tick % 10) == 0) {
            printf("timer tick: %d\n", tick);
        }
    }

    void init(void) {
        // enable supervisor-mode timer interrupts.
        CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_STIE);
        INTR::register_interrupt_handler(INTR::INTR_S_TIMER, timer_intr);
        opensbi.set_timer(CPU::READ_TIME());
        printf("timerinit\n");
    }
};
