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

// class aaa {
//  public:
//   int a = 233;
//
//   aaa() { a = 666; }
// };
//
// class bbb {
//  public:
//   bbb() {}
// };
//
// typedef void (*ctor_t)(void);
//
//// 在 link.ld 中定义
// extern ctor_t __init_array_start[];
// extern ctor_t __init_array_end[];
//
// void cpp_init(void) {
//   ctor_t *f;
//   for (f = __init_array_start; f < __init_array_end; f++) {
//     (*f)();
//   }
//   return;
// }

extern "C" void _start(int _argc, char **_argv) {
  /// @todo c++ 全局对象初始化
  //  cpp_init();
  main(_argc, _argv);
}
// auto class_a = aaa();
// auto class_b = bbb();

int main(int _argc, char **_argv) {
  // 架构相关初始化
  auto arch_init_ret = arch_init(_argc, reinterpret_cast<uint8_t **>(_argv));
  //  printf("class_a.a: %d", class_a.a);
  //
  //  printf("__init_array_start: %p\n", __init_array_start);
  //  printf("&__init_array_start: %p\n", &__init_array_start);
  //  printf("*__init_array_start: %p\n", *__init_array_start);
  //
  //  printf("__init_array_end: %p\n", __init_array_end);
  //  printf("&__init_array_end: %p\n", &__init_array_end);
  //  printf("*__init_array_end: %p\n", *__init_array_end);

  // 进入死循环
  while (1) {
    ;
  }
  return 0;
}
