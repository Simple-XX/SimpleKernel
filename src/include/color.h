
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vga.h for Simple-XX/SimpleKernel.

#ifndef _COLOR_H_
#define _COLOR_H_

#include "stdint.h"

// vga 颜色
typedef enum : uint8_t {
    BLACK         = 0,
    BLUE          = 1,
    GREEN         = 2,
    CYAN          = 3,
    RED           = 4,
    MAGENTA       = 5,
    BROWN         = 6,
    LIGHT_GREY    = 7,
    DARK_GREY     = 8,
    LIGHT_BLUE    = 9,
    LIGHT_GREEN   = 10,
    LIGHT_CYAN    = 11,
    LIGHT_RED     = 12,
    LIGHT_MAGENTA = 13,
    LIGHT_BROWN   = 14,
    WHITE         = 15,
} color_t;

#endif /* _COLOR_H_ */
