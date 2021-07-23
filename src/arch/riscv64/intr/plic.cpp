
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
    // 最大外部中断数
    static constexpr const uint32_t EXTERNEL_INTERRUPT_MAX = 256;
    // 中断处理函数数组
    static INTR::interrupt_handler_t
        enternel_interrupt_handlers[EXTERNEL_INTERRUPT_MAX];
    // 这个值在启动时由 opensbi 传递，暂时写死
    static constexpr const uint64_t hart = 0;
    void                            externel_intr(void) {
        // 读取中断号
        // 根据中断号判断设备
        auto no = get();
        // 执行中断
        enternel_interrupt_handlers[no]();
        return;
    }
    int32_t init(void) {
        // 映射 plic
        for (uint64_t a = MEMLAYOUT::PLIC; a < MEMLAYOUT::PLIC + 0x210000;
             a += 0x1000) {
            VMM::mmap(VMM::get_pgd(), (void *)a, (void *)a,
                      VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
        }
        // TODO: 多核情况下设置所有 hart
        // 将当前 hart 的 S 模式优先级阈值设置为 0
        io.write32((void *)MEMLAYOUT::PLIC_SPRIORITY(hart), 0);
        // 注册外部中断处理函数
        CLINT::register_interrupt_handler(CLINT::INTR_S_EXTERNEL,
                                          externel_intr);
        // 开启外部中断
        CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_SEIE);
        printf("plic init.\n");
        return 0;
    }

    void set(uint8_t irq_no, bool _status) {
        // 设置 IRQ 的属性为非零，即启用 plic
        io.write32((void *)(MEMLAYOUT::PLIC + irq_no * 4), _status);
        // TODO: 多核情况下设置所有 hart
        // 为当前 hart 的 S 模式设置 uart 的 enable
        if (_status) {
            io.write32((void *)MEMLAYOUT::PLIC_SENABLE(hart),
                       io.read32((void *)MEMLAYOUT::PLIC_SENABLE(hart)) |
                           (1 << irq_no));
        }
        else {
            io.write32((void *)MEMLAYOUT::PLIC_SENABLE(hart),
                       io.read32((void *)MEMLAYOUT::PLIC_SENABLE(hart)) &
                           ~(1 << irq_no));
        }
        return;
    }

    uint8_t get(void) {
        return io.read32((void *)MEMLAYOUT::PLIC_SCLAIM(hart));
    }

    void done(uint8_t _irq) {
        io.write32((void *)MEMLAYOUT::PLIC_SCLAIM(hart), _irq);
        return;
    }
    void register_externel_handler(uint8_t n, INTR::interrupt_handler_t h) {
        enternel_interrupt_handlers[n] = h;
        return;
    }

};
