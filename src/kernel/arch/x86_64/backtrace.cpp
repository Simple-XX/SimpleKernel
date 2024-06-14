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
  uint64_t *rbp = (uint64_t *)cpu::kAllXreg.rbp.Read();
  uint64_t *rip = nullptr;

  int count = 0;
  while (rbp && *rbp && count < size) {
    rip = rbp + 1;
    rbp = (uint64_t *)*rbp;
    buffer[count++] = (void *)*rip;
  }

  return count;
}

void DumpStack() {
  void *buffer[kMaxFramesCount];

  // 获取调用栈中的地址
  auto num_frames = backtrace(buffer, kMaxFramesCount);

  for (auto i = 0; i < num_frames; i++) {
    // 打印函数名
    for (auto j : kKernelElf.GetInstance().symtab_) {
      if ((ELF64_ST_TYPE(j.st_info) == STT_FUNC) &&
          ((uint64_t)buffer[i] >= j.st_value) &&
          ((uint64_t)buffer[i] <= j.st_value + j.st_size)) {
        printf("[%s] 0x%p\n", kKernelElf.GetInstance().strtab_ + j.st_name,
               (uint64_t)buffer[i]);
      }
    }
  }
}
