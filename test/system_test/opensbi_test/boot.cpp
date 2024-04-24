
/**
 * @file boot.cpp
 * @brief boot cpp
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

#include "opensbi_interface.h"

int main(int, char **) {
  sbi_debug_console_write_byte('H');
  sbi_debug_console_write_byte('e');
  sbi_debug_console_write_byte('l');
  sbi_debug_console_write_byte('l');
  sbi_debug_console_write_byte('o');
  sbi_debug_console_write_byte('W');
  sbi_debug_console_write_byte('o');
  sbi_debug_console_write_byte('r');
  sbi_debug_console_write_byte('l');
  sbi_debug_console_write_byte('d');
  sbi_debug_console_write_byte('!');
  sbi_debug_console_write_byte('\n');

  return 0;
}

extern "C" void _start(int _argc, char **_argv) {
  /// @todo c++ 全局对象初始化
  //  cpp_init();
  main(_argc, _argv);
}
