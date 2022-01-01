
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// io.cpp for Simple-XX/SimpleKernel.
// IO 接口实现

#include "stddef.h"
#include "stdarg.h"
#include "string.h"
#ifndef __riscv
#include "port.h"
#endif
#include "io.h"
#include "stdio.h"

/// 输出缓冲区
/// @todo 用常量替换 128
static char buf[128];

IO &IO::get_instance(void) {
    /// 定义全局 IO 对象
    static IO io;
    return io;
}

// riscv 没有端口 IO
#ifndef __riscv
uint8_t IO::inb(const uint32_t port) {
    return PORT::inb(port);
}

uint16_t IO::inw(const uint32_t port) {
    return PORT::inw(port);
}

uint32_t IO::ind(const uint32_t port) {
    return PORT::ind(port);
}

void IO::outb(const uint32_t port, const uint8_t data) {
    PORT::outb(port, data);
    return;
}

void IO::outw(const uint32_t port, const uint16_t data) {
    PORT::outw(port, data);
    return;
}

void IO::outd(const uint32_t port, const uint32_t data) {
    PORT::outd(port, data);
    return;
}
#endif

// MMIO 实现
uint8_t IO::read8(void *_addr) {
    return *(uint8_t *)_addr;
}

void IO::write8(void *_addr, uint8_t _val) {
    *(uint8_t *)_addr = _val;
    return;
}

uint16_t IO::read16(void *_addr) {
    return *(uint16_t *)_addr;
}

void IO::write16(void *_addr, uint16_t _val) {
    *(uint16_t *)_addr = _val;
    return;
}

uint32_t IO::read32(void *_addr) {
    return *(uint32_t *)_addr;
}

void IO::write32(void *_addr, uint32_t _val) {
    *(uint32_t *)_addr = _val;
    return;
}

uint64_t IO::read64(void *_addr) {
    return *(uint64_t *)_addr;
}

void IO::write64(void *_addr, uint64_t _val) {
    *(uint64_t *)_addr = _val;
    return;
}

COLOR::color_t IO::get_color(void) {
    // 通过对应的 io 方式获取颜色
    return io.get_color();
}

void IO::set_color(const COLOR::color_t color) {
    // 通过对应的 io 方式设置颜色
    io.set_color(color);
    return;
}

void IO::put_char(char c) {
    // 通过对应的 io 方式输出字符
    io.put_char(c);
    return;
}

int32_t IO::write_string(const char *s) {
    io.write_string(s);
    return 0;
}

/**
 * @brief printf 定义
 * @param  _fmt           格式化字符串
 * @return int32_t        输出的长度
 */
extern "C" int32_t printf(const char *_fmt, ...) {
    va_list va;
    va_start(va, _fmt);
    // 交给 src/libc/src/stdio/vsprintf.c 中的 _vsnprintf
    // 处理格式，并将处理好的字符串保存到 buf 中
    const int ret = _vsnprintf(buf, 128, _fmt, va);
    va_end(va);
    // 输出 buf
    IO::get_instance().write_string(buf);
    // 清空数据
    bzero(buf, 128);
    return ret;
}

/**
 * @brief 与 printf 类似，只是颜色不同
 */
extern "C" int32_t info(const char *_fmt, ...) {
    COLOR::color_t curr_color = IO::get_instance().get_color();
    IO::get_instance().set_color(COLOR::CYAN);
    va_list va;
    int32_t i;
    va_start(va, _fmt);
    i = vsnprintf_(buf, (size_t)-1, _fmt, va);
    va_end(va);
    IO::get_instance().write_string(buf);
    bzero(buf, 128);
    IO::get_instance().set_color(curr_color);
    return i;
}

/**
 * @brief 与 printf 类似，只是颜色不同
 */
extern "C" int32_t warn(const char *_fmt, ...) {
    COLOR::color_t curr_color = IO::get_instance().get_color();
    IO::get_instance().set_color(COLOR::YELLOW);
    va_list va;
    int32_t i;
    va_start(va, _fmt);
    i = vsnprintf_(buf, (size_t)-1, _fmt, va);
    va_end(va);
    IO::get_instance().write_string(buf);
    bzero(buf, 128);
    IO::get_instance().set_color(curr_color);
    return i;
}

/**
 * @brief 与 printf 类似，只是颜色不同
 */
extern "C" int32_t err(const char *_fmt, ...) {
    COLOR::color_t curr_color = IO::get_instance().get_color();
    IO::get_instance().set_color(COLOR::LIGHT_RED);
    va_list va;
    int32_t i;
    va_start(va, _fmt);
    i = vsnprintf_(buf, (size_t)-1, _fmt, va);
    va_end(va);
    IO::get_instance().write_string(buf);
    bzero(buf, 128);
    IO::get_instance().set_color(curr_color);
    return i;
}
