
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

#include "arch.h"
#include "kernel.h"

extern "C" void _start(int _argc, char **_argv) { main(_argc, _argv); }

int main(int _argc, char **_argv) {
  /// @todo c++ 全局对象初始化

  // 架构相关初始化
  auto arch_init_ret = arch_init(_argc, reinterpret_cast<uint8_t **>(_argv));

  // 进入死循环
  while (1) {
    ;
  }
  return 0;
}
