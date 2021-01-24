
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://wiki.osdev.org/Raspberry_Pi_Bare_Bones
// uart.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "uart.h"
#include "io.h"
#include "hardware.h"

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count) {
    asm volatile("__delay_%=: subs %[count], %[count], #1"
                 : "=r"(count)
                 : [count] "0"(count)
                 : "cc");
}

UART::UART(void) {
    return;
}

UART::~UART(void) {
    return;
}

int32_t UART::init(void) {
    // Disable UART0.
    io.outd(UART0_CR, 0x00000000);
    // Setup the GPIO pin 14 && 15.

    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    io.outd(GPPUD, 0x00000000);
    delay(150);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    io.outd(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    io.outd(GPPUDCLK0, 0x00000000);

    // Clear pending interrupts.
    io.outd(UART0_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // Baud = 115200.
    // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
    io.outd(UART0_IBRD, 1);
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    io.outd(UART0_FBRD, 40);

    // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
    io.outd(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // Mask all interrupts.
    io.outd(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) |
                            (1 << 8) | (1 << 9) | (1 << 10));

    // Enable UART0, receive & transfer part of UART.
    io.outd(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
    return 0;
}

void UART::putc(uint8_t c) {
    // Wait for UART to become ready to transmit.
    while (io.ind(UART0_FR) & (1 << 5)) {
        ;
    }
    io.outd(UART0_DR, c);
    return;
}

uint8_t UART::getc(void) {
    // Wait for UART to have received something.
    while (io.ind(UART0_FR) & (1 << 4)) {
        ;
    }
    return io.ind(UART0_DR);
}

void UART::puts(const char *str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        putc((uint8_t)str[i]);
    }
    return;
}

UART uart;
