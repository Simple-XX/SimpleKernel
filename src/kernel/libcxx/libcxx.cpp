
/**
 * @file libcxx.cpp
 * @brief libcxx cpp
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

#include "libcxx.h"

#include <algorithm>

typedef void (*function_t)(void);
// 在 link.ld 中定义
extern "C" function_t __init_array_start;
extern "C" function_t __init_array_end;
extern "C" function_t __fini_array_start;
extern "C" function_t __fini_array_end;

extern "C" void *__dso_handle __attribute__((weak)) = &__dso_handle;
extern "C" int __cxa_atexit(void (*func)(void *), void *arg, void *dso_handle) {
  func(arg);
  return 0;
}

void CppInit(void) {
  // 调用构造函数
  std::for_each(&__init_array_start, &__init_array_end,
                [](function_t func) { (func)(); });
}

void CppDeInit(void) {
  // 调用析构函数
  std::for_each(&__fini_array_start, &__fini_array_end,
                [](function_t func) { (func)(); });
}
