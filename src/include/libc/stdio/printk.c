
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// printk.hpp for MRNIU/SimpleKernel.

#ifndef _PRINTK_HPP_
#define _PRINTK_HPP_


#include "stdarg.h"

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

int printk_color(unsigned char color, const char *fmt, ...){
  va_list args;
  int i;
  va_start(args, fmt);
  i=vsprintf(buf, fmt, args);
  va_end(args);
  terminal_setcolor(color);
  terminal_writestring(buf);
  return i;
}


#endif
