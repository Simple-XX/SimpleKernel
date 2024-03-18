
/**
 * @file main.cpp
 * @brief 内核入口
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
#include "kernel.h"

extern "C" void _start() { main(0, nullptr); }

int main(int _argc, char **_argv) {
  (void)_argc;
  (void)_argv;

  auto serial = CPU::Serial(CPU::COM1);
  serial.write('H');
  serial.write('e');
  serial.write('l');
  serial.write('l');
  serial.write('o');
  serial.write(' ');
  serial.write('T');
  serial.write('e');
  serial.write('s');
  serial.write('t');
  serial.write('!');
  serial.write('\n');

  // 进入死循环
  while (true) {
    ;
  }
  return 0;
}
