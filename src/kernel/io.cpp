
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
#include "common.h"

/// IO 自旋锁
/// @todo 这里需要看一下这么构造有没有问题
static spinlock_t spinlock("IO");

IO &IO::get_instance(void) {
    /// 定义全局 IO 对象
    static IO io;
    return io;
}

// riscv 没有端口 IO
#ifndef __riscv
uint8_t IO::inb(const uint32_t port) {
    spinlock.lock();
    uint8_t ret = PORT::inb(port);
    spinlock.unlock();
    return ret;
}

uint16_t IO::inw(const uint32_t port) {
    spinlock.lock();
    uint16_t ret = PORT::inw(port);
    spinlock.unlock();
    return ret;
}

uint32_t IO::ind(const uint32_t port) {
    spinlock.lock();
    uint32_t ret = PORT::ind(port);
    spinlock.unlock();
    return ret;
}

void IO::outb(const uint32_t port, const uint8_t data) {
    spinlock.lock();
    PORT::outb(port, data);
    spinlock.unlock();
    return;
}

void IO::outw(const uint32_t port, const uint16_t data) {
    spinlock.lock();
    PORT::outw(port, data);
    spinlock.unlock();
    return;
}

void IO::outd(const uint32_t port, const uint32_t data) {
    spinlock.lock();
    PORT::outd(port, data);
    spinlock.unlock();
    return;
}
#endif

// MMIO 实现
uint8_t IO::read8(void *_addr) {
    spinlock.lock();
    uint8_t ret = *(uint8_t *)_addr;
    spinlock.unlock();
    return ret;
}

void IO::write8(void *_addr, uint8_t _val) {
    spinlock.lock();
    *(uint8_t *)_addr = _val;
    spinlock.unlock();
    return;
}

uint16_t IO::read16(void *_addr) {
    spinlock.lock();
    uint16_t ret = *(uint16_t *)_addr;
    spinlock.unlock();
    return ret;
}

void IO::write16(void *_addr, uint16_t _val) {
    spinlock.lock();
    *(uint16_t *)_addr = _val;
    spinlock.unlock();
    return;
}

uint32_t IO::read32(void *_addr) {
    spinlock.lock();
    uint32_t ret = *(uint32_t *)_addr;
    spinlock.unlock();
    return ret;
}

void IO::write32(void *_addr, uint32_t _val) {
    spinlock.lock();
    *(uint32_t *)_addr = _val;
    spinlock.unlock();
    return;
}

uint64_t IO::read64(void *_addr) {
    spinlock.lock();
    uint64_t ret = *(uint64_t *)_addr;
    spinlock.unlock();
    return ret;
}

void IO::write64(void *_addr, uint64_t _val) {
    spinlock.lock();
    *(uint64_t *)_addr = _val;
    spinlock.unlock();
    return;
}

COLOR::color_t IO::get_color(void) {
    spinlock.lock();
    // 通过对应的 io 方式获取颜色
    COLOR::color_t ret = io.get_color();
    spinlock.unlock();
    return ret;
}

void IO::set_color(const COLOR::color_t color) {
    spinlock.lock();
    // 通过对应的 io 方式设置颜色
    io.set_color(color);
    spinlock.unlock();
    return;
}

void IO::put_char(char c) {
    spinlock.lock();
    // 通过对应的 io 方式输出字符
    io.put_char(c);
    spinlock.unlock();
    return;
}

int32_t IO::write_string(const char *s) {
    spinlock.lock();
    io.write_string(s);
    spinlock.unlock();
    return 0;
}

/// 输出缓冲区
char buf[IO::BUF_SIZE];
char buf_test[IO::BUF_SIZE];

/// 格式化输出自旋锁
/// @todo 这里需要看一下这么构造有没有问题
static spinlock_t spinlock_printf("printf");
static spinlock_t spinlock_test("printf test");

/**
 * @brief printf 定义
 * @param  _fmt           格式化字符串
 * @return int32_t        输出的长度
 */
extern "C" int32_t printf(const char *_fmt, ...) {
    spinlock_printf.lock();
    va_list va;
    va_start(va, _fmt);
    // 交给 src/libc/src/stdio/vsprintf.c 中的 _vsnprintf
    // 处理格式，并将处理好的字符串保存到 buf 中
    const int ret = _vsnprintf(buf, IO::BUF_SIZE, _fmt, va);
    va_end(va);
    // 输出 buf
    IO::get_instance().write_string(buf);
    // 清空数据
    bzero(buf, IO::BUF_SIZE);
    spinlock_printf.unlock();
    return ret;
}

/**
 * @brief 与 printf 类似，只是颜色不同
 */
extern "C" int32_t info(const char *_fmt, ...) {
    spinlock_test.lock();
    COLOR::color_t curr_color = IO::get_instance().get_color();
    IO::get_instance().set_color(COLOR::CYAN);
    va_list va;
    int32_t i;
    va_start(va, _fmt);
    i = vsnprintf_(buf, IO::BUF_SIZE, _fmt, va);
    va_end(va);
    // 输出 cpuid
    char tmp[5] = {0};
    itoa(CPU::get_curr_core_id(), &tmp[1], 1, 10);
    tmp[0] = '[';
    tmp[2] = ']';
    tmp[3] = ' ';
    tmp[4] = '\0';
    IO::get_instance().write_string(tmp);
    IO::get_instance().write_string(buf_test);
    bzero(buf_test, IO::BUF_SIZE);
    IO::get_instance().set_color(curr_color);
    spinlock_test.unlock();
    return i;
}

/**
 * @brief 与 printf 类似，只是颜色不同
 */
extern "C" int32_t warn(const char *_fmt, ...) {
    spinlock_printf.lock();
    COLOR::color_t curr_color = IO::get_instance().get_color();
    IO::get_instance().set_color(COLOR::YELLOW);
    va_list va;
    int32_t i;
    va_start(va, _fmt);
    i = vsnprintf_(buf, IO::BUF_SIZE, _fmt, va);
    va_end(va);
    // 输出 cpuid
    char tmp[5] = {0};
    itoa(CPU::get_curr_core_id(), &tmp[1], 1, 10);
    tmp[0] = '[';
    tmp[2] = ']';
    tmp[3] = ' ';
    tmp[4] = '\0';
    IO::get_instance().write_string(tmp);
    IO::get_instance().write_string(buf);
    bzero(buf, IO::BUF_SIZE);
    IO::get_instance().set_color(curr_color);
    spinlock_printf.unlock();
    return i;
}

/**
 * @brief 与 printf 类似，只是颜色不同
 */
extern "C" int32_t err(const char *_fmt, ...) {
    spinlock_printf.lock();
    COLOR::color_t curr_color = IO::get_instance().get_color();
    IO::get_instance().set_color(COLOR::LIGHT_RED);
    va_list va;
    int32_t i;
    va_start(va, _fmt);
    i = vsnprintf_(buf, IO::BUF_SIZE, _fmt, va);
    va_end(va);
    // 输出 cpuid
    char tmp[5] = {0};
    itoa(CPU::get_curr_core_id(), &tmp[1], 1, 10);
    tmp[0] = '[';
    tmp[2] = ']';
    tmp[3] = ' ';
    tmp[4] = '\0';
    IO::get_instance().write_string(tmp);
    IO::get_instance().write_string(buf);
    bzero(buf, IO::BUF_SIZE);
    IO::get_instance().set_color(curr_color);
    spinlock_printf.unlock();
    return i;
}
