
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"
#include "cxxabi.h"
#include "color.h"
#include "stdarg.h"
#include "string.h"

extern "C" int32_t vsprintf(char *buf, const char *fmt, va_list args);

#if defined(__i386__) || defined(__x86_64__)
KERNEL::KERNEL(void) : vga(VGA()), console(CONSOLE(vga)), io(IO(console)) {
    return;
}
#endif

KERNEL::~KERNEL(void) {
    return;
}

void KERNEL::show_info(void) {
    printf(LIGHT_GREEN, "kernel in memory start: 0x%08X, end 0x%08X\n",
           kernel_start, kernel_end);
    printf(LIGHT_GREEN, "kernel in memory size: %d KB, %d pages\n",
           (kernel_end - kernel_start) / 1024,
           (kernel_end - kernel_start + 4095) / 1024 / 4);
    printf(LIGHT_GREEN, "Simple Kernel.\n");

    return;
}

int32_t KERNEL::init(void) {
    cpp_init();
#if defined(__i386__) || defined(__x86_64__)
    vga.init();
    console.init();
#endif
    io.init();
    show_info();
    return 0;
}

int32_t KERNEL::printf(const char *fmt, ...) {
    va_list args;
    int32_t i;
    char    buf[256];
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    io.write_string(buf);
    bzero(buf, 256);
    return i;
}

int32_t KERNEL::printf(color_t color, const char *fmt, ...) {
    color_t curr_color = io.get_color();
    io.set_color(color);
    va_list args;
    int32_t i;
    char    buf[256];
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    io.write_string(buf);
    bzero(buf, 256);
    io.set_color(curr_color);
    return i;
}
