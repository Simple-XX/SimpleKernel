
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pic.h for Simple-XX/SimpleKernel.

#ifndef _8259A_H_
#define _8259A_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "port.hpp"
#include "cpu.hpp"
#include "intr.h"

// Master (IRQs 0-7)
#define IO_PIC1 (0x20)
// Slave  (IRQs 8-15)
#define IO_PIC2 (0xA0)
#define IO_PIC1C (IO_PIC1 + 1)
#define IO_PIC2C (IO_PIC2 + 1)
// End-of-interrupt command code
#define PIC_EOI 0x20

// 设置 8259A 芯片
void init_interrupt_chip(void);

// 重设 8259A 芯片
void clear_interrupt_chip(uint32_t intr_no);

#ifdef __cplusplus
}
#endif

#endif /* _8259A_H_ */
