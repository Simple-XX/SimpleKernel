
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

// 设备 id
enum {
    DEVICE_ID_SD_CARD = (0x00000000UL),
    DEVICE_ID_UART0   = (0x00000001UL),
    DEVICE_ID_UART1   = (0x00000002UL),
    DEVICE_ID_USB_HCD = (0x00000003UL),
    DEVICE_ID_I2C0    = (0x00000004UL),
    DEVICE_ID_I2C1    = (0x00000005UL),
    DEVICE_ID_I2C2    = (0x00000006UL),
    DEVICE_ID_SPI     = (0x00000007UL),
    DEVICE_ID_CCP2TX  = (0x00000008UL),
};

// 时钟 id
enum {
    CLOCK_ID_RESERVED  = (0x000000000UL),
    CLOCK_ID_EMMC      = (0x000000001UL),
    CLOCK_ID_UART      = (0x000000002UL),
    CLOCK_ID_ARM       = (0x000000003UL),
    CLOCK_ID_CORE      = (0x000000004UL),
    CLOCK_ID_V3D       = (0x000000005UL),
    CLOCK_ID_H264      = (0x000000006UL),
    CLOCK_ID_ISP       = (0x000000007UL),
    CLOCK_ID_SDRAM     = (0x000000008UL),
    CLOCK_ID_PIXEL     = (0x000000009UL),
    CLOCK_ID_PWM       = (0x00000000aUL),
    CLOCK_ID_HEVC      = (0x00000000bUL),
    CLOCK_ID_EMMC2     = (0x00000000cUL),
    CLOCK_ID_M2MC      = (0x00000000dUL),
    CLOCK_ID_PIXEL_BVB = (0x00000000eUL),
};

// 电压 id
enum {
    VOLTAGE_ID_RESERVED = (0x000000000UL),
    VOLTAGE_ID_CORE     = (0x000000001UL),
    VOLTAGE_ID_SDRAM_C  = (0x000000002UL),
    VOLTAGE_ID_SDRAM_P  = (0x000000003UL),
    VOLTAGE_ID_SDRAM_I  = (0x000000004UL),
};

// uart 地址
enum {
    // uart0
    UART0_BASE   = (GPIO_BASE + 0x1000UL),
    UART0_DR     = (UART0_BASE + 0x00UL),
    UART0_RSRECR = (UART0_BASE + 0x04UL),
    UART0_FR     = (UART0_BASE + 0x18UL),
    UART0_ILPR   = (UART0_BASE + 0x20UL),
    UART0_IBRD   = (UART0_BASE + 0x24UL),
    UART0_FBRD   = (UART0_BASE + 0x28UL),
    UART0_LCRH   = (UART0_BASE + 0x2CUL),
    UART0_CR     = (UART0_BASE + 0x30UL),
    UART0_IFLS   = (UART0_BASE + 0x34UL),
    UART0_IMSC   = (UART0_BASE + 0x38UL),
    UART0_RIS    = (UART0_BASE + 0x3CUL),
    UART0_MIS    = (UART0_BASE + 0x40UL),
    UART0_ICR    = (UART0_BASE + 0x44UL),
    UART0_DMACR  = (UART0_BASE + 0x48UL),
    UART0_ITCR   = (UART0_BASE + 0x80UL),
    UART0_ITIP   = (UART0_BASE + 0x84UL),
    UART0_ITOP   = (UART0_BASE + 0x88UL),
    UART0_TDR    = (UART0_BASE + 0x8CUL),
};

// mailbox 地址
enum {
    MAILBOX_OFFSET              = (0x0000B880UL),
    MAILBOX_READ_OFFSET         = (0x0UL),
    MAILBOX_POLL_OFFSET         = (0x10UL),
    MAILBOX_SENDER_OFFSET       = (0x14UL),
    MAILBOX_STATUS_OFFSET       = (0x18UL),
    MAILBOX_CONFIG_OFFSET       = (0x1CUL),
    MAILBOX_WRITE_OFFSET        = (0x20UL),
    MAILBOX_VIDEOCORE           = (MMIO_BASE + MAILBOX_OFFSET),
    MAILBOX_READ                = (MAILBOX_VIDEOCORE + MAILBOX_READ_OFFSET),
    MAILBOX_POLL                = (MAILBOX_VIDEOCORE + MAILBOX_POLL_OFFSET),
    MAILBOX_SENDER              = (MAILBOX_VIDEOCORE + MAILBOX_SENDER_OFFSET),
    MAILBOX_STATUS              = (MAILBOX_VIDEOCORE + MAILBOX_STATUS_OFFSET),
    MAILBOX_CONFIG              = (MAILBOX_VIDEOCORE + MAILBOX_CONFIG_OFFSET),
    MAILBOX_WRITE               = (MAILBOX_VIDEOCORE + MAILBOX_WRITE_OFFSET),
    MAILBOX_RESPONSE_SUCCESSFUL = (0x80000000UL),
    MAILBOX_RESPONSE_ERROR      = (0x80000001UL),
    // 1<<31
    MAILBOX_FULL = (0x80000000UL),
    // 1<<30
    MAILBOX_EMPTY = (0x40000000UL),
};

#ifdef __cplusplus
}
#endif

#endif /* _HARDWARE_H_ */
