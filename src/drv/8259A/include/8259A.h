
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// pic.h for MRNIU/SimpleKernel.

#ifndef _8259A_H_
#define _8259A_H_

#include "stdint.h"
#include "port.hpp"
#include "cpu.hpp"
#include "intr/include/intr.h"

#define IO_PIC1   ( 0x20 )   // Master (IRQs 0-7)
#define IO_PIC2   ( 0xA0 )   // Slave  (IRQs 8-15)
#define IO_PIC1C  ( IO_PIC1 + 1 )
#define IO_PIC2C  ( IO_PIC2 + 1 )
#define PIC_EOI  0x20  /* End-of-interrupt command code */

// 设置 8259A 芯片
void init_interrupt_chip(void);

// 重设 8259A 芯片
void clear_interrupt_chip(uint32_t intr_no);

#endif /* _8259A_H_ */
