
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// hardware.h for Simple-XX/SimpleKernel.

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include "stdint.h"

static constexpr const uint32_t MMIO_BASE = 0x3F000000;

// The offsets for reach register.
static constexpr const uint32_t GPIO_BASE = MMIO_BASE + 0x200000;

// Controls actuation of pull up/down to ALL GPIO pins.
static constexpr const uint32_t GPPUD = GPIO_BASE + 0x94;

// Controls actuation of pull up/down for specific GPIO
// pin.
static constexpr const uint32_t GPPUDCLK0 = GPIO_BASE + 0x98;

// The base address for UART.
static constexpr const uint32_t UART0_BASE = GPIO_BASE + 0x1000;

// The offsets for reach register for the UART.
static constexpr const uint32_t UART0_DR     = UART0_BASE + 0x00;
static constexpr const uint32_t UART0_RSRECR = UART0_BASE + 0x04;
static constexpr const uint32_t UART0_FR     = UART0_BASE + 0x18;
static constexpr const uint32_t UART0_ILPR   = UART0_BASE + 0x20;
static constexpr const uint32_t UART0_IBRD   = UART0_BASE + 0x24;
static constexpr const uint32_t UART0_FBRD   = UART0_BASE + 0x28;
static constexpr const uint32_t UART0_LCRH   = UART0_BASE + 0x2C;
static constexpr const uint32_t UART0_CR     = UART0_BASE + 0x30;
static constexpr const uint32_t UART0_IFLS   = UART0_BASE + 0x34;
static constexpr const uint32_t UART0_IMSC   = UART0_BASE + 0x38;
static constexpr const uint32_t UART0_RIS    = UART0_BASE + 0x3C;
static constexpr const uint32_t UART0_MIS    = UART0_BASE + 0x40;
static constexpr const uint32_t UART0_ICR    = UART0_BASE + 0x44;
static constexpr const uint32_t UART0_DMACR  = UART0_BASE + 0x48;
static constexpr const uint32_t UART0_ITCR   = UART0_BASE + 0x80;
static constexpr const uint32_t UART0_ITIP   = UART0_BASE + 0x84;
static constexpr const uint32_t UART0_ITOP   = UART0_BASE + 0x88;
static constexpr const uint32_t UART0_TDR    = UART0_BASE + 0x8C;

// The offsets for Mailbox registers
static constexpr const uint32_t MAILBOX_BASE   = MMIO_BASE + 0xB880;
static constexpr const uint32_t MAILBOX_READ   = MAILBOX_BASE + 0x00;
static constexpr const uint32_t MAILBOX_STATUS = MAILBOX_BASE + 0x18;
static constexpr const uint32_t MAILBOX_WRITE  = MAILBOX_BASE + 0x20;
static constexpr const uint32_t MAILBOX_FULL   = 1 << 31;
static constexpr const uint32_t MAILBOX_EMPTY  = 1 << 30;
static constexpr const uint32_t CODE_REQUEST   = 0x00000000;
static constexpr const uint32_t MAILBOX_RESPONSE_SUCCESSFUL = 0x80000000;
static constexpr const uint32_t MAILBOX_RESPONSE_FAILURE    = 0x80000001;

#endif /* _HARDWARE_H_ */
