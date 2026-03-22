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

  assert(false && "_start should not return");
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
  // 初始化任务管理器，创建 init 进程 (pid 1) 和 idle 线程
  TaskManagerSingleton::create();
  TaskManagerSingleton::instance().InitCurrentCore(true);

  TimerInit();

  // 唤醒其余 core
  WakeUpOtherCores();

  DumpStack();

  klog::Info("Hello SimpleKernel");

  // init 循环：非阻塞收割孤儿僵尸，然后让出 CPU
  while (true) {
    while (TaskManagerSingleton::instance()
               .Wait(static_cast<Pid>(-1), nullptr, true, false)
               .value_or(0) > 0) {
    }
    TaskManagerSingleton::instance().Schedule();
  }
}
