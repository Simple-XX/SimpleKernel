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
#include "cstdio"
#include "kernel.h"
#include "libcxx.h"

[[maybe_unused]] void _start(int argc, char** argv) {
  CppInit();
  main(argc, argv);
}

int main(int argc, char** argv) {
  // 架构相关初始化
  [[maybe_unused]] auto arch_init_ret =
      ArchInit(argc, reinterpret_cast<uint8_t**>(argv));

  // 进入死循环
  while (1) {
    ;
  }
  return 0;
}
