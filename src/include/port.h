
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// port.h for Simple-XX/SimpleKernel.

#ifndef _PORT_H_
#define _PORT_H_

#include "stdint.h"

class PORT {
private:
protected:
public:
    PORT(void);
    ~PORT(void);
    // 端口读一个字节
    static uint8_t inb(const uint32_t port);
    // 端口读一个字
    static uint16_t inw(const uint32_t port);
    // 端口写一个字节
    static void outb(const uint32_t port, uint8_t data);
};

#endif /* _PORT_H_ */
