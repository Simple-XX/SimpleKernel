
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// intr.h for Simple-XX/SimpleKernel.

#ifndef _INTR_H_
#define _INTR_H_

#include "stdint.h"

namespace INTR {
    int32_t init(void);
    // 注册一个中断处理函数
    void register_interrupt_handler(uint8_t n, interrupt_handler_t h);
    void enable_irq(uint32_t irq_no);
    void disable_irq(uint32_t irq_no);
};

#endif /* _INTR_H_ */
