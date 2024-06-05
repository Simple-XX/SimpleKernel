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

#include <cstdint>

#include "arch.h"
#include "cstdio"
#include "iostream"
#include "kernel.h"
#include "libcxx.h"

class aaa {
 public:
  aaa(uint32_t a) { ; }
  ~aaa() { ; }
};

static aaa AAA(0);

void _start(uint32_t argc, uint8_t* argv) {
  CppInit();
  main(argc, argv);
  CppDeInit();

  // 进入死循环
  while (1) {
    ;
  }
}

uint32_t main(uint32_t argc, uint8_t* argv) {
  // 架构相关初始化
  [[maybe_unused]] auto arch_init_ret = ArchInit(argc, argv);

  printf("Hello printf\n");
  std::cout << "Hello ostream" << std::endl;

  return 0;
}
