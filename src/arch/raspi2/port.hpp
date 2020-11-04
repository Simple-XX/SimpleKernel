
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// port.hpp for Simple-XX/SimpleKernel.

#ifndef _PORT_HPP_
#define _PORT_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// 指定地址读一个双字
uint32_t ind(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}

// 指定地址写一个双字
void outd(uint32_t addr, uint32_t data) {
    *(volatile uint32_t *)addr = data;
    return;
}

#ifdef __cplusplus
}
#endif

#endif /* _PORT_HPP_ */
