
// This file is a part of MRNIU/SimpleKernel
// (https://github.com/MRNIU/SimpleKernel).
//
// console.h for MRNIU/SimpleKernel.

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "vga.hpp"

void     console_init(void);
void     console_scroll(void);
uint16_t console_getcursorpos(void);
void     console_setcursorpos(size_t, size_t);
void     console_writestring(const char *);
void     console_write(const char *, size_t);
void     console_putchar(char);
void     console_escapeconv(char);
void     console_putentryat(char, uint8_t, size_t, size_t);
void     console_setcolor(uint8_t);
uint8_t  console_getcolor(void);

#ifdef __cplusplus
}
#endif

#endif /* _CONSOLE_H_ */
