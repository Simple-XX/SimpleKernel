
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// memlayout.h for Simple-XX/SimpleKernel.

#ifndef _MEMLAYOUT_H_
#define _MEMLAYOUT_H_

#include "stdint.h"

// 内存布局
namespace MEMLAYOUT {
    static constexpr const uint64_t DRAM_START = 0x80000000;
    static constexpr const uint64_t DRAM_END   = 0x88000000;
    static constexpr const uint64_t DRAM_SIZE  = DRAM_END - DRAM_START;

    // qemu puts UART registers here in physical memory.
    static constexpr const uint64_t UART0     = 0x10000000;
    static constexpr const uint64_t UART0_IRQ = 10;

    // virtio mmio interface
    static constexpr const uint64_t VIRTIO0     = 0x10001000;
    static constexpr const uint64_t VIRTIO0_IRQ = 1;

    // local interrupt controller, which contains the timer.
    static constexpr const uint64_t CLINT = 0x02000000L;

    static constexpr uint64_t CLINT_MTIMECMP(uint64_t hartid) {
        return CLINT + 0x4000 + 8 * hartid;
    }
    // cycles since boot
    static constexpr const uint64_t CLINT_MTIME = (CLINT + 0xBFF8);

    static constexpr const uint64_t PLIC = 0xC000000L;

    static constexpr const uint64_t PLIC_PRIORITY = (PLIC + 0x0);
    static constexpr const uint64_t PLIC_PENDING  = (PLIC + 0x1000);
    static constexpr uint64_t       PLIC_MENABLE(uint64_t hart) {
        return PLIC + 0x2000 + hart * 0x100;
    }

    static constexpr uint64_t PLIC_SENABLE(uint64_t hart) {
        return PLIC + 0x2080 + hart * 0x100;
    }

    static constexpr uint64_t PLIC_MPRIORITY(uint64_t hart) {
        return PLIC + 0x200000 + hart * 0x2000;
    }

    static constexpr uint64_t PLIC_SPRIORITY(uint64_t hart) {
        return PLIC + 0x201000 + hart * 0x2000;
    }

    static constexpr uint64_t PLIC_MCLAIM(uint64_t hart) {
        return PLIC + 0x200004 + hart * 0x2000;
    }

    static constexpr uint64_t PLIC_SCLAIM(uint64_t hart) {
        return PLIC + 0x201004 + hart * 0x2000;
    }

    // the physical address of opensbi
    static constexpr uint64_t OPENSBI_BASE = 0x80000000;

};

#endif /* _MEMLAYOUT_HPP_ */
