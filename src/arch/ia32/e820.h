
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// e820.h for Simple-XX/SimpleKernel.

#ifndef _E820_H_
#define _E820_H_

#include "stddef.h"
#include "stdint.h"

static constexpr const uint32_t E820_MAX      = 8;
static constexpr const uint32_t E820_RAM      = 1;
static constexpr const uint32_t E820_RESERVED = 2;
static constexpr const uint32_t E820_ACPI     = 3;
static constexpr const uint32_t E820_NVS      = 4;
static constexpr const uint32_t E820_UNUSABLE = 5;

struct e820entry_t {
    // 数据类型由位数决定
    uintptr_t addr;
    size_t    length;
    uint32_t  type;
};

struct e820map_t {
    e820map_t(void) {
        nr_map = 0;
        return;
    }
    ~e820map_t(void) {
        return;
    }
    uint32_t    nr_map;
    e820entry_t map[E820_MAX];
};

#endif /* _E820_H_ */
