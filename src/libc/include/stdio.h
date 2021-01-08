
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// stdio.h for Simple-XX/SimpleKernel.

#ifndef _STDIO_H_
#define _STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// 屏幕输出
int32_t printk(const char *fmt, ...);
// 可指定颜色的屏幕输出
int32_t printk_color(unsigned char color, const char *format, ...);
// 屏幕输出 INFO
int32_t printk_info(const char *fmt, ...);
// 屏幕输出 WARN
int32_t printk_warn(const char *fmt, ...);
// 屏幕输出 DEBUG
int32_t printk_debug(const char *fmt, ...);
// 屏幕输出 TEST
int32_t printk_test(const char *fmt, ...);
// 屏幕输出 ERROR
int32_t printk_err(const char *fmt, ...);

// 串口输出 INFO
int32_t log_info(const char *fmt, ...);
// 串口输出 WARN
int32_t log_warn(const char *fmt, ...);
// 串口输出 DEBUG
int32_t printk_debug(const char *fmt, ...);
// 串口输出 TEST
int32_t log_test(const char *fmt, ...);
// 串口输出 ERROR
int32_t log_error(const char *fmt, ...);

// 枚举颜色，与 vga_color 相同
enum color {
    black         = 0,
    blue          = 1,
    green         = 2,
    cyan          = 3,
    red           = 4,
    magenta       = 5,
    brown         = 6,
    light_grey    = 7,
    dark_gray     = 8,
    light_blue    = 9,
    light_green   = 10,
    light_cyan    = 11,
    light_red     = 12,
    light_magenta = 13,
    light_brown   = 14,
    white         = 15,
};

#define COL_DEBUG light_cyan
#define COL_WARN light_blue
#define COL_ERROR light_red
#define COL_INFO light_green
#define COL_TEST green

#ifdef __cplusplus
}
#endif

#endif /* _STDIO_H_ */
