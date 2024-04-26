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

void _start(int _argc, char** _argv) {
  cpp_init();
  main(_argc, _argv);
}

class aaa {
 public:
  int a = 233;

  aaa(void) : a(666) { printf("aaa init\n"); }

  aaa(int _a) : a(_a) { printf("aaa init %d\n", _a); }
};

auto class_a = aaa();
auto class_a2 = aaa(1);

int main(int _argc, char** _argv) {
  // 架构相关初始化
  auto arch_init_ret = arch_init(_argc, reinterpret_cast<uint8_t**>(_argv));

  printf("class_a.a: %d\n", class_a.a);
  printf("class_a.a: %d\n", class_a2.a);

  printf("------\n");

  // 进入死循环
  while (1) {
    ;
  }
  return 0;
}
