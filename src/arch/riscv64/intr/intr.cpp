
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "stdio.h"
#include "intr.h"
#include "cpu.hpp"

namespace INTR {
#ifdef __cplusplus
    extern "C" {
#endif

#ifdef __cplusplus
    }
#endif

    void enable_irq(uint32_t irq_no) {
        return;
    }

    void disable_irq(uint32_t irq_no) {
        return;
    }

    int32_t init(void) {
        CPU::WRITE_STVEC();
        w_stvec((uint64)kernelvec);
        w_sstatus(r_sstatus() | SSTATUS_SIE);
        w_sie(r_sie() | SIE_SEIE | SIE_SSIE);
        printf("intr init\n");
        return 0;
    }
};
