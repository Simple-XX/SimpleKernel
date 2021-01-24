
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// port.h for Simple-XX/SimpleKernel.

#ifndef _PORT_H_
#define _PORT_H_

#include "stdint.h"

namespace PORT {
    // 端口读一个字节
    uint8_t inb(const uint32_t port);
    // 端口读一个字
    uint16_t inw(const uint32_t port);
    // 端口读一个双字
    uint32_t ind(const uint32_t port);
    // 端口写一个字节
    void outb(const uint32_t port, const uint8_t data);
    // 端口写一个字
    void outw(const uint32_t port, const uint16_t data);
    // 端口写一个双字
    void outd(const uint32_t port, const uint32_t data);
};

#endif /* _PORT_H_ */
