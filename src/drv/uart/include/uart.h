
/**
 * @file uart.h
 * @brief uart 驱动
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

#ifndef SIMPLEKERNEL_UART_H
#define SIMPLEKERNEL_UART_H

#include "stddef.h"
#include "stdint.h"
#include "color.h"
#include "hardware.h"

class UART {
private:
    // uart 地址
    // uart0
    static constexpr const uint32_t UART0_BASE   = HARDWARE::GPIO_BASE + 0x1000;
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
    void                            delay(int32_t count) const;

protected:
public:
    UART(void);
    ~UART(void);
    void put_char(const char _c) const;
    // 写字符串
    void write_string(const char *_s) const;
    // 写字符串
    void write(const char *_s, size_t _len) const;
    // 读字符 TODO
    uint8_t get_char(void) const;
    // 设置颜色 TODO
    void set_color(const COLOR::color_t _color) const;
    // 获取颜色 TODO
    COLOR::color_t get_color(void) const;
};

#endif /* SIMPLEKERNEL_UART_H */
