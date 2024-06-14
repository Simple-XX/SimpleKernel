
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
#include "kernel_fdt.hpp"
#include "libc.h"

int backtrace(void **buffer, int size) {
  uint64_t *fp = (uint64_t *)cpu::ReadFp();
  uint64_t *ra = nullptr;

  int count = 0;
  while (fp && *fp && count < size) {
    ra = fp - 1;
    fp = (uint64_t *)*(fp - 2);
    buffer[count++] = (void *)*ra;
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
