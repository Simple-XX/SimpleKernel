
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// 8259A.h for Simple-XX/SimpleKernel.

#ifndef _8259A_H_
#define _8259A_H_

#include "stdint.h"
#include "port.h"

// Master (IRQs 0-7)
const uint32_t IO_PIC1 = 0x20;
// Slave  (IRQs 8-15)
const uint32_t IO_PIC2  = 0xA0;
const uint32_t IO_PIC1C = IO_PIC1 + 1;
const uint32_t IO_PIC2C = IO_PIC2 + 1;
// End-of-interrupt command code
const uint32_t PIC_EOI = 0x20;

class A8259A : virtual PORT {
private:
protected:
public:
    A8259A(void);
    ~A8259A(void);
    // 设置 8259A 芯片
    static void init_interrupt_chip(void);
    // 重设 8259A 芯片
    static void clear_interrupt_chip(uint32_t intr_no);
};

#endif /* _8259A_H_ */
