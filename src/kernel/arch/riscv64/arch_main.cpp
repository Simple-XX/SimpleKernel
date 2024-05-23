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

#include "cstdio"
#include "fdt_parser.hpp"
#include "opensbi_interface.h"

// printf_bare_metal 基本输出实现
extern "C" void _putchar(char _character) {
  sbi_debug_console_write_byte(_character);
}

// 在 riscv64 情景下，_argc 为启动核 id，_argv 为 dtb 地址
int32_t arch_init(uint32_t _argc, uint8_t **_argv) {
  printf("boot hart id: %d\n", _argc);
  printf("dtb info addr: %p\n", _argv);

  auto result = FDT_PARSER::fdt_parser((uintptr_t)_argv);

  printf("hello arch_init\n");

  return 0;
}
