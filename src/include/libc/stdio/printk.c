
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// printk.hpp for MRNIU/SimpleKernel.

#ifndef _PRINTK_HPP_
#define _PRINTK_HPP_

#include "stdio.h"
#include "stdarg.h"


static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);
extern void terminal_writestring(const char* data);
extern void terminal_setcolor(unsigned char color);
extern unsigned char terminal_getcolor(void);

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
		unsigned char old_color = terminal_getcolor();
		terminal_setcolor(color);
		terminal_writestring(buf);
		terminal_setcolor(old_color);
		return i;
}

// /* Format a string and print it on the screen, just like the libc
//    function printf. */
// void
// printf (const char *format, ...)
// {
//   char **arg = (char **) &format;
//   int c;
//   char buf[20];
//
//   arg++;
//
//   while ((c = *format++) != 0)
//   {
//     if (c != '%')
//       putchar (c);
//     else
//     {
//       char *p, *p2;
//       int pad0 = 0, pad = 0;
//
//       c = *format++;
//       if (c == '0')
//       {
//         pad0 = 1;
//         c = *format++;
//       }
//
//       if (c >= '0' && c <= '9')
//       {
//         pad = c - '0';
//         c = *format++;
//       }
//
//       switch (c)
//       {
//       case 'd':
//       case 'u':
//       case 'x':
//         itoa (buf, c, *((int *) arg++));
//         p = buf;
//         goto string;
//         break;
//
//       case 's':
//         p = *arg++;
//         if (!p)
//           p = "(null)";
//
// string:
//         for (p2 = p; *p2; p2++);
//         for (; p2 < p + pad; p2++)
//           putchar (pad0 ? '0' : ' ');
//         while (*p)
//           putchar (*p++);
//         break;
//
//       default:
//         putchar (*((int *) arg++));
//         break;
//       }
//     }
//   }
// }


#endif
