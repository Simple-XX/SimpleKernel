
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// uart.h for Simple-XX/SimpleKernel.

#ifndef _UART_H_
#define _UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// uart 初始化
void uart_init();

// 输出字符
void uart_putc(uint8_t c);

// 输入字符
uint8_t uart_getc();

// 输出字符串
void uart_puts(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* _UART_H_ */
