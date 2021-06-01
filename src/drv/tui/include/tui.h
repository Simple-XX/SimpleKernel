
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// TUI.h for Simple-XX/SimpleKernel.

// Text User Interface(TUI), see https://wiki.osdev.org/Text_mode

#ifndef _TUI_H_
#define _TUI_H_

#include "stdint.h"
#include "stddef.h"
#include "color.h"

// 位置信息
class pos_t {
public:
    pos_t(const uint8_t col, const uint8_t row);
    ~pos_t(void);
    uint8_t col;
    uint8_t row;
};

// 颜色信息
class col_t {
public:
    col_t(const COLOR::color_t fore, const COLOR::color_t back);
    ~col_t(void);
    uint8_t fore : 4;
    uint8_t back : 4;
};

class char_t {
public:
    char_t(void);
    char_t(const uint8_t c, const col_t color);
    ~char_t(void);
    uint8_t c;
    col_t   color;
};

class TUI {
private:
    // CRT 控制寄存器-地址
    static constexpr const uint32_t TUI_ADDR = 0x3D4;
    // CRT 控制寄存器-数据
    static constexpr const uint32_t TUI_DATA = 0x3D5;
    // 光标高位
    static constexpr const uint32_t TUI_CURSOR_H = 0xE;
    // 光标低位
    static constexpr const uint32_t TUI_CURSOR_L = 0xF;
    // TUI 缓存基址
    static constexpr const uint32_t TUI_MEM_BASE = 0xB8000;
    // TUI 缓存大小
    static constexpr size_t TUI_MEM_SIZE = 0x8000;
    // 规定显示行数、列数
    static constexpr const size_t WIDTH  = 80;
    static constexpr const size_t HEIGHT = 25;
    // 缓存
    char_t *const buffer = (char_t *)TUI_MEM_BASE;
    // 当前位置
    static pos_t pos;
    // 当前命令行颜色
    static col_t color;
    // 指定位置输出
    void put_entry_at(const char c, const col_t color, const size_t x,
                      const size_t y);
    // 转义字符处理
    bool escapeconv(const char c);
    // 滚动显示
    void scroll(void);
    // 生成一个字符
    char_t gen_char(const uint8_t c, const col_t color) const;

protected:
public:
    TUI(void);
    ~TUI(void);
    // 设置颜色
    // TODO: 支持分别字体与背景色
    void set_color(const COLOR::color_t color);
    // 获取颜色
    COLOR::color_t get_color(void) const;
    // 设置光标位置
    void set_pos(const pos_t pos);
    void set_pos_row(const size_t row);
    void set_pos_col(const size_t col);
    // 获取光标位置
    pos_t get_pos(void) const;
    // 写缓存
    void write(const size_t idx, const char_t data);
    // 读缓存
    char_t read(const size_t idx) const;
    // 写字符
    void put_char(const char c);
    // 读字符
    char get_char(void) const;
    // 写字符串
    void write_string(const char *s);
    // 写字符串
    void write(const char *s, const size_t len);
    // 清屏
    void clear(void);
};

#endif /* _TUI_H_ */
