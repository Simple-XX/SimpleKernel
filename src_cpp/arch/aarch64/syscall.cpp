/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "syscall.hpp"

#include <array>

#include "interrupt.h"
#include "kernel_log.hpp"

/**
 * @brief AArch64 系统调用处理
 * @param context_ptr 中断上下文，包含系统调用号和参数
 */
auto Syscall(uint64_t, cpu_io::TrapContext* context_ptr) -> void {
  // 获取系统调用号和参数
  uint64_t syscall_id = 0;
  std::array<uint64_t, 6> args{};

  syscall_id = context_ptr->x8;
  args[0] = context_ptr->x0;
  args[1] = context_ptr->x1;
  args[2] = context_ptr->x2;
  args[3] = context_ptr->x3;
  args[4] = context_ptr->x4;
  args[5] = context_ptr->x5;

  // 执行处理函数
  auto ret = syscall_dispatcher(syscall_id, args.data());

  // 设置返回值
  context_ptr->x0 = static_cast<uint64_t>(ret);

  // 跳过 svc 指令
  context_ptr->elr_el1 += 4;
}
