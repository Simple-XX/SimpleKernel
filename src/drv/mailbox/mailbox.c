
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/bztsrc/raspi3-tutorial
// mailbox.c for Simple-XX/SimpleKernel.

#include "mailbox.h"
#include "hardware.h"
#include "stdint.h"

volatile uint32_t __attribute__((aligned(16))) mbox[36];

int32_t mbox_call(uint8_t ch) {
    uint32_t r = (((uint32_t)((uint32_t)&mbox) & ~0xF) | (ch & 0xF));
    // 等待直到 mailbox 空闲
    do {
        asm volatile("nop");
    } while (*((uint32_t *)MBOX_STATUS) & MBOX_FULL);
    // 向指定通道写数据
    *((uint32_t *)MBOX_WRITE) = r;
    // 等待答复
    while (1) {
        // 先看有没有答复
        do {
            asm volatile("nop");
        } while (*((uint32_t *)MBOX_STATUS) & MBOX_EMPTY);
        // 判断是不是需要的答复
        if (r == *((uint32_t *)MBOX_READ)) {
            // 判断是否有效
            return mbox[1] == MBOX_RESPONSE;
        }
    }
    return 0;
}
