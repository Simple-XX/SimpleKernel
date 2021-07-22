
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// io.h for Simple-XX/SimpleKernel.

#ifndef _IO_H_
#define _IO_H_

#include "stdint.h"
#include "color.h"

#if defined(__i386__) || defined(__x86_64__)
#include "tui.h"
#elif defined(__arm__) || defined(__aarch64__)
#include "uart.h"
#elif defined(__riscv)
#include "sbi_console.h"
#endif

class IO {
private:
    // io 对象
#if defined(__i386__) || defined(__x86_64__)
    // x86 平台使用 TUI 接口
    TUI io;
#elif defined(__arm__) || defined(__aarch64__)
    // arn 平台使用 UART 接口
    UART io;
#elif defined(__riscv)
    // riscv 平台使用 opensbi 提供的接口
    SBI_CONSOLE io;
#endif

protected:
public:
    IO(void);
    ~IO(void);
    // 端口读字节
    uint8_t inb(const uint32_t port);
    // 端口读字
    uint16_t inw(const uint32_t port);
    // 端口读双字
    uint32_t ind(const uint32_t port);
    // 端口写字节
    void outb(const uint32_t port, const uint8_t data);
    // 端口写字
    void outw(const uint32_t port, const uint16_t data);
    // 端口写双字
    void outd(const uint32_t port, const uint32_t data);
    // MMIO 读写
    uint8_t  read8(void *_addr);
    void     write8(void *_addr, uint8_t _val);
    uint16_t read16(void *_addr);
    void     write16(void *_addr, uint16_t _val);
    uint32_t read32(void *_addr);
    void     write32(void *_addr, uint32_t _val);
    uint64_t read64(void *_addr);
    void     write64(void *_addr, uint64_t _val);
    // 获取当前颜色
    COLOR::color_t get_color(void);
    // 设置当前颜色
    void set_color(const COLOR::color_t color);
    // 输出字符
    void put_char(const char c);
    // 输入字符
    char get_char(void);
    // 输出字符串
    int32_t write_string(const char *s);
};

extern IO io;

#endif /* _IO_H_ */
