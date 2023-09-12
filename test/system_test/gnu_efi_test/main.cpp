
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

#include "kernel.h"

extern "C" void _start() { main(0, nullptr); }

int main(int _argc, char **_argv) {
  (void)_argc;
  (void)_argv;
  // 进入死循环
  while (true) {
    ;
  }
  return 0;
}
