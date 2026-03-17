/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <MPMCQueue.hpp>
#include <cerrno>
#include <cstdint>
#include <new>

#include "arch.h"
#include "basic_info.hpp"
#include "expected.hpp"
#include "interrupt.h"
#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_cstdio"
#include "kstd_cstring"
#include "kstd_libcxx.h"
#include "mutex.hpp"
#include "per_cpu.hpp"
#include "sk_stdlib.h"
#include "syscall.hpp"
#include "task_control_block.hpp"
#include "task_manager.hpp"
#include "virtual_memory.hpp"

namespace {

/// 全局变量，用于测试多核同步
std::atomic<uint64_t> global_counter{0};

/// Task1: 每 1s 打印一次，测试 sys_exit
auto task1_func(void* arg) -> void {
  klog::Info("Task1: arg = {:#x}",
             static_cast<uint64_t>(reinterpret_cast<uintptr_t>(arg)));
  for (int i = 0; i < 5; ++i) {
    klog::Info("Task1: iteration {}/5", i + 1);
    (void)sys_sleep(1000);
  }
  klog::Info("Task1: exiting with code 0");
  sys_exit(0);
}

/// Task2: 每 2s 打印一次，测试 sys_yield
auto task2_func(void* arg) -> void {
  klog::Info("Task2: arg = {:#x}",
             static_cast<uint64_t>(reinterpret_cast<uintptr_t>(arg)));
  uint64_t count = 0;
  while (1) {
    klog::Info("Task2: yield count={}", count++);
    (void)sys_sleep(2000);
    // 主动让出 CPU
    (void)sys_yield();
  }
}

/// Task3: 每 3s 打印一次，同时修改全局变量，测试多核同步
auto task3_func(void* arg) -> void {
  klog::Info("Task3: arg = {:#x}",
             static_cast<uint64_t>(reinterpret_cast<uintptr_t>(arg)));
  while (1) {
    uint64_t old_value = global_counter.fetch_add(1, std::memory_order_relaxed);
    klog::Info("Task3: global_counter {} -> {}", old_value, old_value + 1);
    (void)sys_sleep(3000);
  }
}

/// Task4: 每 4s 打印一次，测试 sys_sleep
auto task4_func(void* arg) -> void {
  klog::Info("Task4: arg = {:#x}",
             static_cast<uint64_t>(reinterpret_cast<uintptr_t>(arg)));
  uint64_t iteration = 0;
  while (1) {
    auto* cpu_sched = per_cpu::GetCurrentCore().sched_data;
    auto start_tick = cpu_sched->local_tick;
    klog::Info("Task4: sleeping for 4s (iteration {})", iteration++);
    (void)sys_sleep(4000);
    auto end_tick = cpu_sched->local_tick;
    klog::Info("Task4: woke up (slept ~{} ticks)", end_tick - start_tick);
  }
}

/// 为当前核心创建测试任务
auto create_test_tasks() -> void {
  size_t core_id = cpu_io::GetCurrentCoreId();
  auto& tm = TaskManagerSingleton::instance();

  auto task1 = kstd::make_unique<TaskControlBlock>(
      "Task1-Exit", 10, task1_func, reinterpret_cast<void*>(0x1111));
  auto task2 = kstd::make_unique<TaskControlBlock>(
      "Task2-Yield", 10, task2_func, reinterpret_cast<void*>(0x2222));
  auto task3 = kstd::make_unique<TaskControlBlock>(
      "Task3-Sync", 10, task3_func, reinterpret_cast<void*>(0x3333));
  auto task4 = kstd::make_unique<TaskControlBlock>(
      "Task4-Sleep", 10, task4_func, reinterpret_cast<void*>(0x4444));

  // 设置 CPU 亲和性，绑定到当前核心
  task1->aux->cpu_affinity = (1UL << core_id);
  task2->aux->cpu_affinity = (1UL << core_id);
  task3->aux->cpu_affinity = (1UL << core_id);
  task4->aux->cpu_affinity = (1UL << core_id);

  tm.AddTask(std::move(task1));
  tm.AddTask(std::move(task2));
  tm.AddTask(std::move(task3));
  tm.AddTask(std::move(task4));

  klog::Info("Created 4 test tasks");
}

/// 非启动核入口
auto main_smp(int argc, const char** argv) -> int {
  per_cpu::GetCurrentCore() = per_cpu::PerCpu(cpu_io::GetCurrentCoreId());
  ArchInitSMP(argc, argv);
  MemoryInitSMP();
  InterruptInitSMP(argc, argv);
  TaskManagerSingleton::instance().InitCurrentCore();
  TimerInitSMP();

  klog::Info("Hello SimpleKernel SMP");

  // 为当前核心创建测试任务
  create_test_tasks();

  // 启动调度器
  TaskManagerSingleton::instance().Schedule();

  // UNREACHABLE: Schedule() 不应返回
  __builtin_unreachable();
}

}  // namespace

auto _start(int argc, const char** argv) -> void {
  if (argv != nullptr) {
    CppInit();
    main(argc, argv);
  } else {
    main_smp(argc, argv);
  }

  while (true) {
    cpu_io::Pause();
  }
}

auto main(int argc, const char** argv) -> int {
  // 初始化当前核心的 per_cpu 数据
  per_cpu::PerCpuArraySingleton::create();
  per_cpu::GetCurrentCore() = per_cpu::PerCpu(cpu_io::GetCurrentCoreId());

  // 架构相关初始化
  ArchInit(argc, argv);
  // 内存相关初始化
  MemoryInit();
  // 中断相关初始化
  InterruptInit(argc, argv);
  // 设备管理器初始化
  DeviceInit();
  // 文件系统初始化
  FileSystemInit();
  // 初始化任务管理器 (设置主线程)
  TaskManagerSingleton::create();
  TaskManagerSingleton::instance().InitCurrentCore();

  TimerInit();

  // 唤醒其余 core
  WakeUpOtherCores();

  DumpStack();

  klog::Info("Hello SimpleKernel");

  klog::Info("Initializing test tasks...");

  // 为主核心创建测试任务
  create_test_tasks();

  klog::Info("Main: Starting scheduler...");

  // 启动调度器，不再返回
  TaskManagerSingleton::instance().Schedule();

  // UNREACHABLE: Schedule() 不应返回
  __builtin_unreachable();
}
