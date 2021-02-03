
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// uart.h for Simple-XX/SimpleKernel.

#ifndef _UART_H_
#define _UART_H_

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
    void                            delay(int32_t count);

protected:
public:
    UART(void);
    ~UART(void);
    int32_t init(void);
    void    put_char(const char c);
    // 写字符串
    void write_string(const char *s);
    // 写字符串
    void write(const char *s, size_t len);
    // 读字符
    uint8_t get_char(void);
    // 设置颜色
    void set_color(const color_t color);
    // 获取颜色
    color_t get_color(void);
};

#endif /* _UART_H_ */
