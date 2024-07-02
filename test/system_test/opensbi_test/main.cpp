
/**
 * @file main.cpp
 * @brief 用于测试 opensbi 的启动
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

#include <cstdint>

#include "cstdio"

// printf_bare_metal 基本输出实现
extern "C" void _putchar(char character) {
  sbi_debug_console_write_byte(character);
}

uint32_t main(uint32_t, uint8_t *) {
  _putchar('H');
  _putchar('e');
  _putchar('l');
  _putchar('l');
  _putchar('o');
  _putchar('W');
  _putchar('o');
  _putchar('r');
  _putchar('l');
  _putchar('d');
  _putchar('!');
  _putchar('\n');

  return 0;
}

extern "C" void _start(uint32_t argc, uint8_t *argv) {
  main(argc, argv);

  // 进入死循环
  while (1) {
    ;
  }
}
