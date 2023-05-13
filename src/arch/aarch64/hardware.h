
/**
 * @file hardware.h
 * @brief 硬件定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_HARDWARE_H
#define SIMPLEKERNEL_HARDWARE_H

#include "cstdint"

namespace HARDWARE {
// for raspi2 & 3
static constexpr const uint32_t MMIO_BASE             = 0x3F000000;
// The offsets for reach register.
static constexpr const uint32_t GPIO_BASE             = MMIO_BASE + 0x200000;
// Controls actuation of pull up/down to ALL GPIO pins.
static constexpr const uint32_t GPPUD                 = GPIO_BASE + 0x94;
// Controls actuation of pull up/down for specific GPIO pin.
static constexpr const uint32_t GPPUDCLK0             = GPIO_BASE + 0x98;

// 设备 id
static constexpr const uint32_t DEVICE_ID_SD_CARD     = 0x00000000;
static constexpr const uint32_t DEVICE_ID_UART0       = 0x00000001;
static constexpr const uint32_t DEVICE_ID_UART1       = 0x00000002;
static constexpr const uint32_t DEVICE_ID_USB_HCD     = 0x00000003;
static constexpr const uint32_t DEVICE_ID_I2C0        = 0x00000004;
static constexpr const uint32_t DEVICE_ID_I2C1        = 0x00000005;
static constexpr const uint32_t DEVICE_ID_I2C2        = 0x00000006;
static constexpr const uint32_t DEVICE_ID_SPI         = 0x00000007;
static constexpr const uint32_t DEVICE_ID_CCP2TX      = 0x00000008;

// 时钟 id
static constexpr const uint32_t CLOCK_ID_RESERVED     = 0x000000000;
static constexpr const uint32_t CLOCK_ID_EMMC         = 0x000000001;
static constexpr const uint32_t CLOCK_ID_UART         = 0x000000002;
static constexpr const uint32_t CLOCK_ID_ARM          = 0x000000003;
static constexpr const uint32_t CLOCK_ID_CORE         = 0x000000004;
static constexpr const uint32_t CLOCK_ID_V3D          = 0x000000005;
static constexpr const uint32_t CLOCK_ID_H264         = 0x000000006;
static constexpr const uint32_t CLOCK_ID_ISP          = 0x000000007;
static constexpr const uint32_t CLOCK_ID_SDRAM        = 0x000000008;
static constexpr const uint32_t CLOCK_ID_PIXEL        = 0x000000009;
static constexpr const uint32_t CLOCK_ID_PWM          = 0x00000000a;
static constexpr const uint32_t CLOCK_ID_HEVC         = 0x00000000b;
static constexpr const uint32_t CLOCK_ID_EMMC2        = 0x00000000c;
static constexpr const uint32_t CLOCK_ID_M2MC         = 0x00000000d;
static constexpr const uint32_t CLOCK_ID_PIXEL_BVB    = 0x00000000e;

// 电压 id
static constexpr const uint32_t VOLTAGE_ID_RESERVED   = 0x000000000;
static constexpr const uint32_t VOLTAGE_ID_CORE       = 0x000000001;
static constexpr const uint32_t VOLTAGE_ID_SDRAM_C    = 0x000000002;
static constexpr const uint32_t VOLTAGE_ID_SDRAM_P    = 0x000000003;
static constexpr const uint32_t VOLTAGE_ID_SDRAM_I    = 0x000000004;

// mailbox 地址
static constexpr const uint32_t MAILBOX_OFFSET        = 0x0000B880;
static constexpr const uint32_t MAILBOX_READ_OFFSET   = 0x0;
static constexpr const uint32_t MAILBOX_POLL_OFFSET   = 0x10;
static constexpr const uint32_t MAILBOX_SENDER_OFFSET = 0x14;
static constexpr const uint32_t MAILBOX_STATUS_OFFSET = 0x18;
static constexpr const uint32_t MAILBOX_CONFIG_OFFSET = 0x1C;
static constexpr const uint32_t MAILBOX_WRITE_OFFSET  = 0x20;
static constexpr const uint32_t MAILBOX_VIDEOCORE = MMIO_BASE + MAILBOX_OFFSET;
static constexpr const uint32_t MAILBOX_READ
  = MAILBOX_VIDEOCORE + MAILBOX_READ_OFFSET;
static constexpr const uint32_t MAILBOX_POLL
  = MAILBOX_VIDEOCORE + MAILBOX_POLL_OFFSET;
static constexpr const uint32_t MAILBOX_SENDER
  = MAILBOX_VIDEOCORE + MAILBOX_SENDER_OFFSET;
static constexpr const uint32_t MAILBOX_STATUS
  = MAILBOX_VIDEOCORE + MAILBOX_STATUS_OFFSET;
static constexpr const uint32_t MAILBOX_CONFIG
  = MAILBOX_VIDEOCORE + MAILBOX_CONFIG_OFFSET;
static constexpr const uint32_t MAILBOX_WRITE
  = MAILBOX_VIDEOCORE + MAILBOX_WRITE_OFFSET;
static constexpr const uint32_t MAILBOX_RESPONSE_SUCCESSFUL = 0x80000000;
static constexpr const uint32_t MAILBOX_RESPONSE_ERROR      = 0x80000001;
// 1<<31
static constexpr const uint32_t MAILBOX_FULL                = 0x80000000;
// 1<<30
static constexpr const uint32_t MAILBOX_EMPTY               = 0x40000000;
};     // namespace HARDWARE

#endif /* SIMPLEKERNEL_HARDWARE_H */
