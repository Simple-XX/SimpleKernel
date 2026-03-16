/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cassert>
#include <cstring>

#include "cpu_io.h"
#include "per_cpu.hpp"
#include "task_manager.hpp"
#include "test_environment_state.hpp"

extern "C" {

void switch_to(cpu_io::CalleeSavedContext* prev_ctx,
               cpu_io::CalleeSavedContext* next_ctx) {
  auto* env_state =
      test_env::TestEnvironmentState::GetCurrentThreadEnvironment();
  assert(env_state &&
         "TestEnvironmentState not set for current thread. "
         "Did you forget to call SetCurrentThreadEnvironment()?");

  auto& core_env = env_state->GetCurrentCoreEnv();

  // 从上下文指针查找对应的任务
  auto* prev_task = env_state->FindTaskByContext(prev_ctx);
  auto* next_task = env_state->FindTaskByContext(next_ctx);

  // 获取 PerCpu 数据
  auto& per_cpu = per_cpu::GetCurrentCore();

  // 记录切换事件到环境层
  test_env::CoreEnvironment::SwitchEvent event;
  event.timestamp = (per_cpu.sched_data ? per_cpu.sched_data->local_tick : 0);
  event.from = prev_task;
  event.to = next_task;
  event.core_id = core_env.core_id;
  core_env.switch_history.push_back(event);

  // 更新 PerCpu 的当前线程
  per_cpu.running_task = next_task;
}

void kernel_thread_entry() {}
void trap_return(void*) {}
void trap_entry() {}

}  // extern "C"

void InitTaskContext(cpu_io::CalleeSavedContext* task_context,
                     void (*entry)(void*), void* arg, uint64_t stack_top) {
  // 清零上下文
  std::memset(task_context, 0, sizeof(cpu_io::CalleeSavedContext));

  task_context->ReturnAddress() =
      reinterpret_cast<uint64_t>(kernel_thread_entry);
  task_context->EntryFunction() = reinterpret_cast<uint64_t>(entry);
  task_context->EntryArgument() = reinterpret_cast<uint64_t>(arg);
  task_context->StackPointer() = stack_top;
}

void InitTaskContext(cpu_io::CalleeSavedContext* task_context,
                     cpu_io::TrapContext* trap_context_ptr,
                     uint64_t stack_top) {
  // 清零上下文
  std::memset(task_context, 0, sizeof(cpu_io::CalleeSavedContext));

  task_context->ReturnAddress() =
      reinterpret_cast<uint64_t>(kernel_thread_entry);
  task_context->EntryFunction() = reinterpret_cast<uint64_t>(trap_return);
  task_context->EntryArgument() = reinterpret_cast<uint64_t>(trap_context_ptr);
  task_context->StackPointer() = stack_top;
}

#include <stdio.h>

extern "C" void etl_putchar(int c) { putchar(c); }
