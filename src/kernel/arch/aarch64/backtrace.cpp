/**
 * @file backtrace.cpp
 * @brief 栈回溯实现
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
#include "cpu.hpp"
#include "cstdio"
#include "kernel_elf.hpp"
#include "libc.h"

int backtrace(void **buffer, int size) {
  uint64_t *x29 = (uint64_t *)cpu::kAllXreg.x29.Read();

  int count = 0;
  while (x29 && *x29 && count < size) {
    uint64_t lr = x29[1];
    x29 = (uint64_t *)x29[0];
    buffer[count++] = (void *)lr;
  }

  return count;
}

void DumpStack() {
  void *buffer[kMaxFramesCount];

  // 获取调用栈中的地址
  auto num_frames = backtrace(buffer, kMaxFramesCount);

  // 打印地址
  /// @todo 打印函数名，需要 elf 支持
  for (auto i = 0; i < num_frames; i++) {
    printf("[0x%p]\n", buffer[i]);
  }
}
