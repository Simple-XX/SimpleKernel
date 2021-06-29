
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// sbi_console.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "string.h"
#include "opensbi.h"
#include "sbi_console.h"

SBI_CONSOLE::SBI_CONSOLE(void) {
    write_string("sbi console init.\n");
    return;
}

SBI_CONSOLE::~SBI_CONSOLE(void) {
    return;
}

void SBI_CONSOLE::put_char(const char _c) const {
    OPENSBI::put_char(_c);
    return;
}

void SBI_CONSOLE::write_string(const char *_s) const {
    write(_s, strlen(_s));
    return;
}

void SBI_CONSOLE::write(const char *_s, size_t _len) const {
    for (size_t i = 0; i < _len; i++) {
        put_char(_s[i]);
    }
    return;
}

uint8_t SBI_CONSOLE::get_char(void) const {
    return OPENSBI::get_char();
}

// TODO
void SBI_CONSOLE::set_color(const COLOR::color_t _color
                            __attribute((unused))) const {
    return;
}

// TODO
COLOR::color_t SBI_CONSOLE::get_color(void) const {
    return COLOR::BLACK;
}
