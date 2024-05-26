
/**
 * @file ns16550a.h
 * @brief ns16550a 头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2024-05-24
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2024-05-24<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#include "ns16550a.h"

Ns16550a::Ns16550a(uintptr_t dev_addr) : base_addr_(dev_addr) {
  // disable interrupt
  write(kRegIER, 0x00);
  // set baud rate
  write(kRegLCR, 0x80);
  write(kUartDLL, 0x03);
  write(kUartDLM, 0x00);
  // set word length to 8-bits
  write(kRegLCR, 0x03);
  // enable FIFOs
  write(kRegFCR, 0x07);
  // enable receiver interrupts
  write(kRegIER, 0x01);
}

void Ns16550a::putc(uint8_t c) {
  while ((read(kRegLSR) & (1 << 5)) == 0)
    ;
  write(kRegTHR, c);
}

volatile uint8_t* Ns16550a::Reg(uint8_t reg) {
  return (volatile uint8_t*)(base_addr_ + reg);
}

uint8_t Ns16550a::read(uint8_t reg) { return (*(Reg(reg))); }

void Ns16550a::write(uint8_t reg, uint8_t c) { (*Reg(reg)) = c; }
