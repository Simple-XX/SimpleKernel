
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// port.h for Simple-XX/SimpleKernel.

#ifndef _PORT_H_
#define _PORT_H_

#include "stdint.h"

namespace PORT {
    // Memory-Mapped I/O input
    uint32_t ind(uint32_t reg);
    // Memory-Mapped I/O output
    void outd(uint32_t reg, uint32_t data);
};

#endif /* _PORT_H_ */
