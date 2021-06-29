
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// plic.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "stdio.h"
#include "cpu.hpp"
#include "memlayout.h"
#include "io.h"
#include "vmm.h"
#include "intr.h"

namespace PLIC {
    int32_t init(void) {
        // 映射 plic 地址
        vmm.mmap();
        // 允许 UART 中断
        *(uint32_t *)(MEMLAYOUT::PLIC + MEMLAYOUT::UART0_IRQ * 4) = 1;
        // set(MEMLAYOUT::UART0_IRQ, true);
        // int hart = CPU::READ_TP();
        // 为当前hart的S模式设置uart的enable
        // io.write32((void *)MEMLAYOUT::PLIC_SENABLE(hart),
        //            1 << MEMLAYOUT::UART0_IRQ);
        // 将当前hart的S模式优先级阈值设置为0
        // io.write32((void *)MEMLAYOUT::PLIC_SPRIORITY(hart), 0);
        printf("plic init\n");
        return 0;
    }

    void set(uint8_t irq_no, bool _status) {
        // 设置 IRQ 的属性为非零，即启用 plic
        // io.write32((void *)(MEMLAYOUT::PLIC + irq_no * 4), _status);
        return;
    }

    uint8_t get(void) {
        return io.read32((void *)MEMLAYOUT::PLIC_SCLAIM(CPU::READ_TP()));
    }

    void done(uint8_t _irq) {
        io.write32((void *)MEMLAYOUT::PLIC_SCLAIM(CPU::READ_TP()), _irq);
        return;
    }
};
