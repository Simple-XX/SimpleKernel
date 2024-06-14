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

#include <opensbi_interface.h>

#include "cpu.hpp"
#include "cstdio"
#include "kernel_elf.hpp"
#include "kernel_fdt.hpp"
#include "libc.h"
#include "ns16550a.h"

// printf_bare_metal 基本输出实现
extern "C" void _putchar(char character) {
  sbi_debug_console_write_byte(character);
}

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
uint32_t ArchInit(uint32_t argc, uint8_t *argv) {
  printf("boot hart id: %d\n", argc);
  printf("dtb info addr: %p\n", argv);

  kKernelFdt.GetInstance() = KernelFdt((uint64_t)argv);

  auto [memory_base, memory_size] = kKernelFdt.GetInstance().GetMemory();
  printf("Memory address = 0x%p, size = 0x%X\n", memory_base, memory_size);

  auto [serial_base, serial_size] = kKernelFdt.GetInstance().GetSerial();
  auto uart = Ns16550a(serial_base);
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

  // 解析内核 elf 信息
  kKernelElf.GetInstance() = KernelElf();

  Info("Hello riscv64 ArchInit\n");

  return 0;
}
