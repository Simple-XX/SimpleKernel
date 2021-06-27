
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/qemu/qemu/blob/stable-5.0/hw/riscv/virt.c
// qemu-virt.h for Simple-XX/SimpleKernel.

#ifndef _QEMU_VIRT_H_
#define _QEMU_VIRT_H_

#include "stddef.h"
#include "stdint.h"

// qemu-virt 设备定义
namespace MACHINE {
    enum : uint8_t {
        VIRT_DEBUG,
        VIRT_MROM,
        VIRT_TEST,
        VIRT_RTC,
        VIRT_CLINT,
        VIRT_PLIC,
        VIRT_UART0,
        VIRT_VIRTIO0,
        VIRT_VIRTIO1,
        VIRT_FLASH,
        VIRT_DRAM,
        VIRT_PCIE_MMIO,
        VIRT_PCIE_PIO,
        VIRT_PCIE_ECAM
    };

    enum : uint8_t {
        UART0_IRQ = 10,
        RTC_IRQ   = 11,
        // 1 to 8
        VIRTIO_IRQ   = 1,
        VIRTIO_COUNT = 8,
        // 32 to 35
        PCIE_IRQ = 0x20,
        // Arbitrary maximum number of interrupts
        VIRTIO_NDEV = 0x35,
    };

    static constexpr const struct {
        uint64_t base;
        size_t   size;
    } virt_memmap[] = {
        [VIRT_DEBUG]     = {0x0, 0x100},
        [VIRT_MROM]      = {0x1000, 0x11000},
        [VIRT_TEST]      = {0x100000, 0x1000},
        [VIRT_RTC]       = {0x101000, 0x1000},
        [VIRT_CLINT]     = {0x2000000, 0x10000},
        [VIRT_PLIC]      = {0xC000000, 0x4000000},
        [VIRT_UART0]     = {0x10000000, 0x100},
        [VIRT_VIRTIO0]   = {0x10001000, 0x1000},
        [VIRT_VIRTIO1]   = {0x10002000, 0x1000},
        [VIRT_FLASH]     = {0x20000000, 0x4000000},
        [VIRT_DRAM]      = {0x80000000, 0x0},
        [VIRT_PCIE_MMIO] = {0x40000000, 0x40000000},
        [VIRT_PCIE_PIO]  = {0x03000000, 0x00010000},
        [VIRT_PCIE_ECAM] = {0x30000000, 0x10000000},
    };
};

#endif /* _QEMU_VIRT_H_ */
