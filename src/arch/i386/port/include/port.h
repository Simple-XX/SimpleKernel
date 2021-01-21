
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
    uint8_t inb(const uint32_t port);
    // 端口读一个字
    uint16_t inw(const uint32_t port);
    // 端口写一个字节
    void outb(const uint32_t port, const uint8_t data);
    // 端口写一个字
    void outw(const uint32_t port, const uint16_t data);
};

extern PORT portk;

#endif /* _PORT_H_ */
