
/**
 * @file libc.cpp
 * @brief libc c
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

#ifdef __cplusplus
extern "C" {
#endif

#include "libc.h"

int32_t libc(uint32_t argc, uint8_t **argv) {
  (void)argc;
  (void)argv;

  // 进入死循环
  while (1) {
    ;
  }

  return 0;
}

#ifdef __cplusplus
}
#endif
