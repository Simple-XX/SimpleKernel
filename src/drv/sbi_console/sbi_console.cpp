
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// sbi_console.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "string.h"
#include "stdlib.h"
#include "opensbi.h"
#include "sbi_console.h"

COLOR::color_t SBI_CONSOLE::color = COLOR::WHITE;

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
    uint8_t res = 0xFF;
    // TODO: 设置超时
    while (1) {
        res = OPENSBI::get_char();
        if (res != 0xFF) {
            break;
        }
    }
    return res;
}

void SBI_CONSOLE::set_color(const COLOR::color_t _color) const {
    char *tmp = nullptr;
    switch (_color) {
        case COLOR::BLACK: {
            tmp = (char *)"\033[30m";
            break;
        }
        case COLOR::RED: {
            tmp = (char *)"\033[31m";
            break;
        }
        case COLOR::GREEN: {
            tmp = (char *)"\033[32m";
            break;
        }
        case COLOR::YELLOW: {
            tmp = (char *)"\033[33m";
            break;
        }
        case COLOR::BLUE: {
            tmp = (char *)"\033[34m";
            break;
        }
        case COLOR::PURPLE: {
            tmp = (char *)"\033[35m";
            break;
        }
        case COLOR::CYAN: {
            tmp = (char *)"\033[36m";
            break;
        }
        case COLOR::WHITE: {
            tmp = (char *)"\033[37m";
            break;
        }
        case COLOR::GREY: {
            tmp = (char *)"\033[90m";
            break;
        }
        case COLOR::LIGHT_RED: {
            tmp = (char *)"\033[91m";
            break;
        }
        case COLOR::LIGHT_GREEN: {
            tmp = (char *)"\033[92m";
            break;
        }
        case COLOR::LIGHT_YELLOW: {
            tmp = (char *)"\033[93m";
            break;
        }
        case COLOR::LIGHT_BLUE: {
            tmp = (char *)"\033[94m";
            break;
        }
        case COLOR::LIGHT_PURPLE: {
            tmp = (char *)"\033[95m";
            break;
        }
        case COLOR::LIGHT_CYAN: {
            tmp = (char *)"\033[96m";
            break;
        }
        case COLOR::LIGHT_WHITE: {
            tmp = (char *)"\033[97m";
            break;
        }
    }
    write_string(tmp);
    color = _color;
    return;
}

COLOR::color_t SBI_CONSOLE::get_color(void) const {
    return color;
}
