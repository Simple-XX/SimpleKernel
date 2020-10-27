
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://wiki.osdev.org/Raspberry_Pi_Bare_Bones
// uart.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "uart.h"
#include "hardware.h"
#include "port.hpp"

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count) {
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
                 : "=r"(count)
                 : [ count ] "0"(count)
                 : "cc");
}

// A Mailbox message with set clock rate of PL011 to 3MHz tag
volatile unsigned int __attribute__((aligned(16)))
mbox[9] = {9 * 4, 0, 0x38002, 12, 8, 2, 3000000, 0, 0};

void uart_init(void) {
    // Disable UART0.
    outd(UART0_CR, 0x00000000);
    // Setup the GPIO pin 14 && 15.

    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    outd(GPPUD, 0x00000000);
    delay(150);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    outd(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    outd(GPPUDCLK0, 0x00000000);

    // Clear pending interrupts.
    outd(UART0_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // Baud = 115200.
    // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
    outd(UART0_IBRD, 1);
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    outd(UART0_FBRD, 40);

    // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
    outd(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // Mask all interrupts.
    outd(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) |
                         (1 << 8) | (1 << 9) | (1 << 10));

    // Enable UART0, receive & transfer part of UART.
    outd(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
    return;
}

void uart_putc(uint8_t c) {
    // Wait for UART to become ready to transmit.
    while (ind(UART0_FR) & (1 << 5)) {}
    outd(UART0_DR, c);
    return;
}

uint8_t uart_getc() {
    // Wait for UART to have received something.
    while (ind(UART0_FR) & (1 << 4)) {}
    return ind(UART0_DR);
}

void uart_puts(const char *str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        uart_putc((uint8_t)str[i]);
    }
    return;
}

#ifdef __cplusplus
}
#endif
