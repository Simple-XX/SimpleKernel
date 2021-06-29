
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// sbi_console.h for Simple-XX/SimpleKernel.

#ifndef _SBI_CONSOLE_H_
#define _SBI_CONSOLE_H_

#include "stdint.h"
#include "color.h"

class SBI_CONSOLE {
private:
protected:
public:
    SBI_CONSOLE(void);
    ~SBI_CONSOLE(void);
    void put_char(const char _c) const;
    // 写字符串
    void write_string(const char *_s) const;
    // 写字符串
    void write(const char *_s, size_t _len) const;
    // 读字符 TODO
    uint8_t get_char(void) const;
    // 设置颜色 TODO
    void set_color(const COLOR::color_t _color) const;
    // 获取颜色 TODO
    COLOR::color_t get_color(void) const;
};

#endif /* _SBI_CONSOLE_H_ */
