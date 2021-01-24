
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// uart.h for Simple-XX/SimpleKernel.

#ifndef _UART_H_
#define _UART_H_

#include "stdint.h"

class UART {
private:
protected:
public:
    UART(void);
    ~UART(void);
    // uart 初始化
    int32_t init();
    // 输出字符
    void putc(uint8_t c);
    // 输入字符
    uint8_t getc(void);
    // 输出字符串
    void puts(const char *str);
};

extern UART uart;

#endif /* _UART_H_ */
