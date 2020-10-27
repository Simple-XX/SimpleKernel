
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// hardware.h for Simple-XX/SimpleKernel.

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

enum {
    // for raspi2 & 3
    MMIO_BASE = 0x3F000000,
    // The offsets for reach register.
    GPIO_BASE = (MMIO_BASE + 0x200000),
    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD = (GPIO_BASE + 0x94),
    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0 = (GPIO_BASE + 0x98),
};

// uart 地址
enum {
    // uart0
    UART0_BASE   = (GPIO_BASE + 0x1000),
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C),
};

// mailbox 地址
enum {
    VIDEOCORE_MBOX = (MMIO_BASE + 0x0000B880),
    MBOX_READ      = (VIDEOCORE_MBOX + 0x0),
    MBOX_POLL      = (VIDEOCORE_MBOX + 0x10),
    MBOX_SENDER    = (VIDEOCORE_MBOX + 0x14),
    MBOX_STATUS    = (VIDEOCORE_MBOX + 0x18),
    MBOX_CONFIG    = (VIDEOCORE_MBOX + 0x1C),
    MBOX_WRITE     = (VIDEOCORE_MBOX + 0x20),
    MBOX_RESPONSE  = (0x80000000),
    MBOX_FULL      = (0x80000000),
    MBOX_EMPTY     = (0x40000000),
};

#ifdef __cplusplus
}
#endif

#endif /* _HARDWARE_H_ */
