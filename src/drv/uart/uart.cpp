
/**
 * @file uart.cpp
 * @brief uart 驱动实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * Based on https://wiki.osdev.org/Raspberry_Pi_Bare_Bones
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

#include "stddef.h"
#include "string.h"
#include "uart.h"
#include "port.h"

UART::UART(void) {
    // Disable UART0.
    PORT::outd(UART0_CR, 0x00000000);
    // Setup the GPIO pin 14 && 15.

    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    PORT::outd(HARDWARE::GPPUD, 0x00000000);
    delay(150);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    PORT::outd(HARDWARE::GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    PORT::outd(HARDWARE::GPPUDCLK0, 0x00000000);

    // Clear pending interrupts.
    PORT::outd(UART0_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // Baud = 115200.
    // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
    PORT::outd(UART0_IBRD, 1);
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    PORT::outd(UART0_FBRD, 40);

    // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
    PORT::outd(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // Mask all interrupts.
    PORT::outd(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
                               (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    // Enable UART0, receive & transfer part of UART.
    PORT::outd(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
    write_string("uart init\n");
    return;
}

UART::~UART(void) {
    return;
}

// Loop <delay> times in a way that the compiler won't optimize away
void UART::delay(int32_t count) const {
    asm("__delay_%=: subs %[count], %[count], #1"
        : "=r"(count)
        : [count] "0"(count)
        : "cc");
}

void UART::put_char(const char _c) const {
    // Wait for UART to become ready to transmit.
    while (PORT::ind(UART0_FR) & (1 << 5)) {
        ;
    }
    PORT::outd(UART0_DR, _c);
    return;
}

void UART::write_string(const char *_s) const {
    write(_s, strlen(_s));
    return;
}

void UART::write(const char *_s, size_t _len) const {
    for (size_t i = 0; i < _len; i++) {
        put_char(_s[i]);
    }
    return;
}

uint8_t UART::get_char(void) const {
    // Wait for UART to have received something.
    while (PORT::ind(UART0_FR) & (1 << 4)) {
        ;
    }
    return PORT::ind(UART0_DR);
}

// TODO
void UART::set_color(const COLOR::color_t _color __attribute((unused))) const {
    return;
}

// TODO
COLOR::color_t UART::get_color(void) const {
    return COLOR::BLACK;
}
