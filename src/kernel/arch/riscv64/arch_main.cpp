/**
 * @file arch_main.cpp
 * @brief arch_main cpp
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#include "cpu.hpp"
#include "cstdio"
#include "fdt_parser.hpp"
#include "ns16550a.h"
#include "opensbi_interface.h"

// printf_bare_metal 基本输出实现
extern "C" void _putchar(char character) {
  sbi_debug_console_write_byte(character);
}

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
uint32_t ArchInit(uint32_t argc, uint8_t *argv) {
  printf("boot hart id: %d\n", argc);
  printf("dtb info addr: %p\n", argv);

  auto dtb_info = FDT_PARSER::fdt_parser((uintptr_t)argv);

  auto resource_mem = FDT_PARSER::resource_t();
  dtb_info.find_via_prefix("serial@", &resource_mem);
  auto uart = Ns16550a(resource_mem.mem.addr);
  uart.PutChar('H');
  uart.PutChar('e');
  uart.PutChar('l');
  uart.PutChar('l');
  uart.PutChar('o');
  uart.PutChar(' ');
  uart.PutChar('u');
  uart.PutChar('a');
  uart.PutChar('r');
  uart.PutChar('t');
  uart.PutChar('!');
  uart.PutChar('\n');

  printf("hello ArchInit\n");

  return 0;
}

void DumpStack() {
  uint64_t *fp = (uint64_t *)cpu::ReadFp();
  uint64_t *ra = nullptr;

  printf("------DumpStack------\n");
  while (fp && *fp) {
    ra = fp - 1;
    fp = (uint64_t *)*(fp - 2);
    printf("[0x%p]\n", *ra);
  }
  printf("----DumpStack End----\n");
}
