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

#include "basic_info.hpp"
#include "cpu/cpu.hpp"
#include "kernel_elf.hpp"
#include "kernel_fdt.hpp"
#include "ns16550a.h"
#include "sk_cstdio"
#include "sk_libc.h"

// printf_bare_metal 基本输出实现
extern "C" void _putchar(char character) {
  sbi_debug_console_write_byte(character);
}

// 引用链接脚本中的变量
/// @see http://wiki.osdev.org/Using_Linker_Script_Values
/// 内核开始
extern "C" void *__executable_start[];
/// 内核结束
extern "C" void *end[];
BasicInfo::BasicInfo(uint32_t argc, uint8_t *argv) {
  (void)argc;
  (void)argv;

  auto [memory_base, memory_size] = kKernelFdt.GetInstance().GetMemory();
  physical_memory_addr = memory_base;
  physical_memory_size = memory_size;

  kernel_addr = reinterpret_cast<uint64_t>(__executable_start);
  kernel_size = reinterpret_cast<uint64_t>(end) -
                reinterpret_cast<uint64_t>(__executable_start);
  elf_addr = 0;
  elf_size = 0;

  fdt_addr = reinterpret_cast<uint64_t>(argv);
}

uint32_t ArchInit(uint32_t argc, uint8_t *argv) {
  printf("boot hart id: %d\n", argc);
  printf("dtb info addr: %p\n", argv);

  kKernelFdt.GetInstance() = KernelFdt((uint64_t)argv);

  kBasicInfo.GetInstance() = BasicInfo(argc, argv);
  sk_std::cout << kBasicInfo.GetInstance();

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

  klog::Info("Hello riscv64 ArchInit\n");

  return 0;
}
