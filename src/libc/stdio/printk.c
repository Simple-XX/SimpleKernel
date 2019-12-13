
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// printk.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdarg.h"
#include "stdint.h"

static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);
extern void console_writestring(const char * data);
extern void console_setcolor(unsigned char color);
extern unsigned char console_getcolor(void);

int32_t printk(const char * fmt, ...) {
	va_list args;
	int i;
	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	console_writestring(buf);
	return i;
}

#ifdef __cplusplus
}
#endif
