
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// resource.h for Simple-XX/SimpleKernel.

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "stdint.h"
#include "iostream"
#include "assert.h"

struct resource_t {
    // 资源类型
    enum : uint8_t {
        // 内存
        MEM = 1 << 0,
    };
    uint8_t type;
    // 资源名称
    char *name;
    // 当资源类型为内存时，uinon 保存内存地址
    // 当资源类型为中断号，uinon 保存中断号
    struct {
        uintptr_t addr;
        size_t    len;
    } mem;

    resource_t(void) : type(0), name(nullptr) {
        mem.addr = 0;
        mem.len  = 0;
        return;
    }

    friend std::ostream &operator<<(std::ostream &_os, const resource_t &_res) {
        printf("%s: ", _res.name);
        if (_res.type & MEM) {
            printf("MEM(0x%p, 0x%p)", _res.mem.addr, _res.mem.len);
        }
        return _os;
    }
};

#endif /* _RESOURCE_H_ */
