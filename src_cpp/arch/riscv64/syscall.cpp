/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "syscall.hpp"

#include "interrupt.h"
#include "kernel_log.hpp"

auto Syscall(uint64_t, cpu_io::TrapContext* context_ptr) -> void {
  // 获取系统调用号和参数
  uint64_t syscall_id = 0;
  uint64_t args[6] = {0};

  syscall_id = context_ptr->a7;
  args[0] = context_ptr->a0;
  args[1] = context_ptr->a1;
  args[2] = context_ptr->a2;
  args[3] = context_ptr->a3;
  args[4] = context_ptr->a4;
  args[5] = context_ptr->a5;

  // 执行处理函数
  auto ret = syscall_dispatcher(syscall_id, args);

  // 设置返回值
  context_ptr->a0 = static_cast<uint64_t>(ret);
  // 跳过 ecall 指令
  context_ptr->sepc += 4;
}
