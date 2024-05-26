
/**
 * @file LibCxxInit.cpp
 * @brief LibCxxInit cpp
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

extern "C" {

typedef void (*ctor_t)(void);
// 在 link.ld 中定义
extern ctor_t __init_array_start[];
extern ctor_t __init_array_end[];

void CppInit(void) {
  for (auto ctor = __init_array_start; ctor < __init_array_end; ctor++) {
    /// @todo x86_64 crashed
    (*ctor)();
  }
}
};

int32_t LibCxxInit(uint32_t argc, uint8_t** argv) {
  (void)argc;
  (void)argv;

  // 进入死循环
  while (1) {
    ;
  }

  return 0;
}
