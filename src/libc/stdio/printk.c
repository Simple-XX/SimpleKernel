
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
#include "debug.h"

static char buf[1024];

extern int           vsprintf(char *buf, const char *fmt, va_list args);
extern void          console_writestring(const char *data);
extern void          console_setcolor(unsigned char color);
extern unsigned char console_getcolor(void);

int32_t printk(const char *fmt, ...) {
    va_list args;
    int     i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console_writestring(buf);
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
    return i;
}

#ifdef __cplusplus
}
#endif
