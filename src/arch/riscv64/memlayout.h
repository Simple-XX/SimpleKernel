
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// memlayout.h for Simple-XX/SimpleKernel.

#ifndef _MEMLAYOUT_H_
#define _MEMLAYOUT_H_

#include "qemu-virt.h"

// 内存布局
// TODO: 设备地址应该由 dtb 传递，在系统启动时动态获取
namespace MEMLAYOUT {
    // 根据具体设备设置相关值
    static constexpr const uint64_t DRAM_START =
        MACHINE::virt_memmap[MACHINE::VIRT_DRAM].base;
    // 这两个值使用 qemu 的默认参数
    static constexpr const uint64_t DRAM_END  = 0x88000000;
    static constexpr const uint64_t DRAM_SIZE = DRAM_END - DRAM_START;

    // qemu puts UART registers here in physical memory.
    static constexpr const uint64_t UART0 =
        MACHINE::virt_memmap[MACHINE::VIRT_UART0].base;
    static constexpr const uint64_t UART0_IRQ = MACHINE::UART0_IRQ;

    // virtio mmio interface
    static constexpr const uint64_t VIRTIO0 =
        MACHINE::virt_memmap[MACHINE::VIRT_VIRTIO0].base;
    static constexpr const uint64_t VIRTIO1 =
        MACHINE::virt_memmap[MACHINE::VIRT_VIRTIO1].base;

    static constexpr const uint64_t VIRTIO0_IRQ = MACHINE::VIRTIO_IRQ;

    // local interrupt controller, which contains the timer.
    static constexpr const uint64_t CLINT =
        MACHINE::virt_memmap[MACHINE::VIRT_CLINT].base;

    static constexpr uint64_t CLINT_MTIMECMP(uint64_t _hartid) {
        return CLINT + 0x4000 + 8 * _hartid;
    }

    // cycles since boot
    static constexpr const uint64_t CLINT_MTIME = (CLINT + 0xBFF8);

    static constexpr const uint64_t PLIC =
        MACHINE::virt_memmap[MACHINE::VIRT_PLIC].base;

    static constexpr const uint64_t PLIC_PRIORITY = (PLIC + 0x0);
    static constexpr const uint64_t PLIC_PENDING  = (PLIC + 0x1000);
    static constexpr uint64_t       PLIC_MENABLE(uint64_t _hart) {
        return PLIC + 0x2000 + _hart * 0x100;
    }

    static constexpr uint64_t PLIC_SENABLE(uint64_t _hart) {
        return PLIC + 0x2080 + _hart * 0x100;
    }

    static constexpr uint64_t PLIC_MPRIORITY(uint64_t _hart) {
        return PLIC + 0x200000 + _hart * 0x2000;
    }

    static constexpr uint64_t PLIC_SPRIORITY(uint64_t _hart) {
        return PLIC + 0x201000 + _hart * 0x2000;
    }

    static constexpr uint64_t PLIC_MCLAIM(uint64_t _hart) {
        return PLIC + 0x200004 + _hart * 0x2000;
    }

    static constexpr uint64_t PLIC_SCLAIM(uint64_t _hart) {
        return PLIC + 0x201004 + _hart * 0x2000;
    }

    // the physical address of opensbi
    static constexpr uint64_t OPENSBI_BASE = 0x80000000;

};

#endif /* _MEMLAYOUT_HPP_ */
