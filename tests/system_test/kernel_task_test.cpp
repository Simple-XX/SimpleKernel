/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kstd_cstring"
#include "kstd_libcxx.h"
#include "kstd_memory"
#include "sk_stdlib.h"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

namespace {

std::atomic<int> g_task_a_counter{0};
std::atomic<int> g_task_b_counter{0};

void thread_func_a(void* arg) {
  uint64_t id = (uint64_t)arg;
  for (int i = 0; i < 5; ++i) {
    klog::Info("Thread A: running, arg={}, iter={}", id, i);
    g_task_a_counter++;
    (void)sys_sleep(50);
  }
  klog::Info("Thread A: exit");
  sys_exit(0);
}

void thread_func_b(void* arg) {
  uint64_t id = (uint64_t)arg;
  for (int i = 0; i < 5; ++i) {
    klog::Info("Thread B: running, arg={}, iter={}", id, i);
    g_task_b_counter++;
    (void)sys_sleep(50);
  }
  klog::Info("Thread B: exit");
  sys_exit(0);
}
}  // namespace

auto kernel_task_test() -> bool {
  klog::Info("kernel_task_test: start");
  g_task_a_counter = 0;
  g_task_b_counter = 0;

  // 创建线程 A
  auto task_a = kstd::make_unique<TaskControlBlock>("Task A", 10, thread_func_a,
                                                    (void*)100);
  TaskManagerSingleton::instance().AddTask(std::move(task_a));

  // 创建线程 B
  auto task_b = kstd::make_unique<TaskControlBlock>("Task B", 10, thread_func_b,
                                                    (void*)200);
  TaskManagerSingleton::instance().AddTask(std::move(task_b));

  klog::Info("Main: Waiting for tasks...");

  // Wait for tasks to finish (or reach expected count)
  int timeout = 200;  // 200 * 50ms = 10s roughly
  while (timeout > 0) {
    (void)sys_sleep(50);
    if (g_task_a_counter >= 5 && g_task_b_counter >= 5) {
      break;
    }
    timeout--;
  }

  EXPECT_GT(timeout, 0, "Tasks should complete before timeout");
  EXPECT_EQ(g_task_a_counter, 5, "Task A count");
  EXPECT_EQ(g_task_b_counter, 5, "Task B count");

  klog::Info("kernel_task_test: PASS");
  return true;
}
