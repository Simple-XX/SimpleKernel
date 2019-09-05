
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// printk.hpp for MRNIU/SimpleKernel.

#ifndef _PRINTK_HPP_
#define _PRINTK_HPP_

#include "stdio.h"
#include "stdarg.h"


static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);
extern void console_writestring(const char* data);
extern void console_setcolor(unsigned char color);
extern unsigned char console_getcolor(void);

int printk(const char * fmt, ...){
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console_writestring(buf);
    return i;
}

int printk_color(unsigned char color, const char *fmt, ...){
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    unsigned char old_color = console_getcolor();
    console_setcolor(color);
    console_writestring(buf);
    console_setcolor(old_color);
    return i;
}


#endif
