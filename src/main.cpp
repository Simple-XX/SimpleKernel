/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <atomic>
#include <cstdint>

#include "arch.h"
#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_libcxx.h"
#include "per_cpu.hpp"
#include "task_manager.hpp"
#include "virtual_memory.hpp"

namespace {

/// 非启动核入口
auto main_smp(int argc, const char** argv) -> int {
  per_cpu::GetCurrentCore() = per_cpu::PerCpu(cpu_io::GetCurrentCoreId());
  ArchInitSMP(argc, argv);
  MemoryInitSMP();
  InterruptInitSMP(argc, argv);
  TaskManagerSingleton::instance().InitCurrentCore();
  TimerInitSMP();

  klog::Info("Hello SimpleKernel SMP");

  // 启动调度器
  TaskManagerSingleton::instance().Schedule();

  // UNREACHABLE: Schedule() 不应返回
  __builtin_unreachable();
}

}  // namespace

std::atomic_flag primary_booted_ = ATOMIC_FLAG_INIT;

auto _start(int argc, const char** argv) -> void {
  if (!primary_booted_.test_and_set(std::memory_order_acquire)) {
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

  // 启动调度器，不再返回
  TaskManagerSingleton::instance().Schedule();

  // UNREACHABLE: Schedule() 不应返回
  __builtin_unreachable();
}
