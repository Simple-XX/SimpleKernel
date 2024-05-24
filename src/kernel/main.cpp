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

[[maybe_unused]] void _start(int _argc, char** _argv) {
  cpp_init();
  main(_argc, _argv);
}

class aaa {
 public:
  int a = 233;

  aaa() : a(666) { printf("aaa init\n"); }

  aaa(int _a) : a(_a) { printf("aaa init %d\n", _a); }
};

auto class_a = aaa(2);
static aaa class_a2 = aaa(3);
int i32;
static int si32;

int main(int _argc, char** _argv) {
  // 架构相关初始化
  auto arch_init_ret = arch_init(_argc, reinterpret_cast<uint8_t**>(_argv));

  printf("class_a.a: %d\n", class_a.a);
  printf("&class_a: %p\n", &class_a);
  printf("&class_a2: %p\n", &class_a2);
  printf("&i32: %p\n", &i32);
  printf("&si32: %p\n", &si32);
  printf("*(&si32): %p\n", *(&si32));
  uintptr_t ccc = 0;
  asm("nop");
  ccc = (uintptr_t)&si32;
  asm("nop");

  printf("ccc: %X\n", ccc);

  printf("------\n");

  // 进入死循环
  while (1) {
    ;
  }
  return 0;
}
