
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// color.h for Simple-XX/SimpleKernel.

#ifndef _COLOR_H_
#define _COLOR_H_

#include "stdint.h"

namespace COLOR {
    // TUI 颜色
    typedef enum : uint8_t {
#if defined(__i386__) || defined(__x86_64__)
        // RGB: 0 0 0
        BLACK = 0,
        // RGB: 0 0 170
        BLUE = 1,
        // RGB: 0 170 0
        GREEN = 2,
        // RGB: 0 170 170
        CYAN = 3,
        // RGB: 170 0 0
        RED = 4,
        // RGB: 170 0 170
        PURPLE = 5,
        // RGB: 170 85 0
        BROWN = 6,
        // RGB: 170 170 170
        GREY = 7,
        // RGB: 85 85 85
        DARK_GREY = 8,
        // RGB: 85 85 255
        LIGHT_BLUE = 9,
        // RGB: 85 255 85
        LIGHT_GREEN = 10,
        // RGB: 85 255 255
        LIGHT_CYAN = 11,
        // RGB: 255 85 85
        LIGHT_RED = 12,
        // RGB: 255 85 255
        LIGHT_PURPLE = 13,
        // RGB: 255 255 85
        YELLOW = 14,
        // RGB: 255 255 255
        WHITE = 15,
#elif defined(__riscv)
        // See https://en.wikipedia.org/wiki/ANSI_escape_code
        BLACK        = 30,
        RED          = 31,
        GREEN        = 32,
        YELLOW       = 33,
        BLUE         = 34,
        PURPLE       = 35,
        CYAN         = 36,
        WHITE        = 37,
        GREY         = 90,
        LIGHT_RED    = 91,
        LIGHT_GREEN  = 92,
        LIGHT_YELLOW = 93,
        LIGHT_BLUE   = 94,
        LIGHT_PURPLE = 95,
        LIGHT_CYAN   = 96,
        LIGHT_WHITE  = 97,
#endif
    } color_t;
};

#endif /* _COLOR_H_ */
