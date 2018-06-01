
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// printk.c for MRNIU/SimpleKernel.

#include "../include/stdarg.h"

static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);
extern void terminal_writestring(const char* data);

int printk(const char * fmt, ...){
  va_list args;
  int i;
  va_start(args, fmt);
  i=vsprintf(buf, fmt, args);
  va_end(args);
  terminal_writestring(buf);
  return i;
}
