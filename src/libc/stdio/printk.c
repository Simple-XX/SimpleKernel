
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// printk.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdarg.h"
#include "stdint.h"
#include "string.h"
#ifdef x86_64
#include "console.h"
#elif RASPI2
#include "uart.h"
#endif

static char buf[1024];

extern int vsprintf(char *buf, const char *fmt, va_list args);

#ifdef x86_64
int32_t printk(const char *fmt, ...) {
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console_writestring(buf);
    bzero(buf, 1024);
    return i;
}

int32_t printk_color(uint8_t color, const char *fmt, ...) {
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    unsigned char old_color = console_getcolor();
    console_setcolor(color);
    console_writestring(buf);
    console_setcolor(old_color);
    bzero(buf, 1024);
    return i;
}

// TODO:
// 将一下几个函数精简成调用 printk() 的形式，注意参数传递方式

int32_t printk_info(const char *fmt, ...) {
    printk_color(COL_INFO, "[INFO] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console_writestring(buf);
    bzero(buf, 1024);
    return i;
}

int32_t printk_warn(const char *fmt, ...) {
    printk_color(COL_WARN, "[WARN] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console_writestring(buf);
    bzero(buf, 1024);
    return i;
}

int32_t printk_debug(const char *fmt, ...) {
    printk_color(COL_DEBUG, "[DEBUG] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console_writestring(buf);
    bzero(buf, 1024);
    return i;
}

int32_t printk_test(const char *fmt, ...) {
    printk_color(COL_TEST, "[TEST] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console_writestring(buf);
    bzero(buf, 1024);
    return i;
}

int printk_err(const char *fmt, ...) {
    printk_color(COL_ERROR, "[ERROR] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console_writestring(buf);
    bzero(buf, 1024);
    asm("hlt");
    return i;
}
#elif RASPI2
// TODO: 添加颜色
int32_t log_info(const char *fmt, ...) {
    uart_puts("[INFO] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    uart_puts(buf);
    bzero(buf, 1024);
    return i;
}

int32_t log_debug(const char *fmt, ...) {
    uart_puts("[DEBUG] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    uart_puts(buf);
    bzero(buf, 1024);
    return i;
}

int32_t log_warn(const char *fmt, ...) {
    uart_puts("[WARN] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    uart_puts(buf);
    bzero(buf, 1024);
    asm("hlt");
    return i;
}

int32_t log_test(const char *fmt, ...) {
    uart_puts("[TEST] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    uart_puts(buf);
    bzero(buf, 1024);
    return i;
}

int32_t log_error(const char *fmt, ...) {
    uart_puts("[ERROR] ");
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    uart_puts(buf);
    bzero(buf, 1024);
    return i;
}
#endif /* x86_64 */

#ifdef __cplusplus
}
#endif
