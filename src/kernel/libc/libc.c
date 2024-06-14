
/**
 * @file libc.c
 * @brief c 运行时支持
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include "libc.h"

#include <stdint.h>

// 声明内核提供的函数
extern void Err(const char* format, ...);

/// 栈保护
uint64_t __stack_chk_guard = 0x595E9FBD94FDA766;

/// 栈保护检查失败后进入死循环
__attribute__((noreturn)) void __stack_chk_fail() {
  Err("Error: Stack smashing detected!\n");
  while (1);
}
