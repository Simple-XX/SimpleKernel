
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

na16550a_t::na16550a_t(uintptr_t _dev_addr) : base_addr(_dev_addr) {
  // disable interrupt
  write(REG_IER, 0x00);
  // set baud rate
  write(REG_LCR, 0x80);
  write(UART_DLL, 0x03);
  write(UART_DLM, 0x00);
  // set word length to 8-bits
  write(REG_LCR, 0x03);
  // enable FIFOs
  write(REG_FCR, 0x07);
  // enable receiver interrupts
  write(REG_IER, 0x01);
}

void na16550a_t::putc(uint8_t _c) {
  while ((read(REG_LSR) & (1 << 5)) == 0);
  write(REG_THR, _c);
}

volatile uint8_t* na16550a_t::Reg(uint8_t _reg) {
  return (volatile uint8_t*)(base_addr + _reg);
}

uint8_t na16550a_t::read(uint8_t _reg) { return (*(Reg(_reg))); }

void na16550a_t::write(uint8_t _reg, uint8_t _c) { (*Reg(_reg)) = _c; }
