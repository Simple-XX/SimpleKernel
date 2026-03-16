/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cstdint>
#include <new>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_cstdio"
#include "kstd_libcxx.h"
#include "per_cpu.hpp"
#include "spinlock.hpp"
#include "syscall.hpp"
#include "system_test.h"
#include "task_manager.hpp"

namespace {

struct test_case {
  const char* name;
  bool (*func)(void);
  // 是否为多核测试，需要所有核心参与
  bool is_smp_test = false;
};

std::array<test_case, 18> test_cases = {
    test_case{"ctor_dtor_test", ctor_dtor_test, false},
    test_case{"spinlock_test", spinlock_test, true},
    test_case{"memory_test", memory_test, false},
    test_case{"virtual_memory_test", virtual_memory_test, false},
    test_case{"interrupt_test", interrupt_test, false},
    test_case{"fifo_scheduler_test", fifo_scheduler_test, false},
    test_case{"rr_scheduler_test", rr_scheduler_test, false},
    test_case{"cfs_scheduler_test", cfs_scheduler_test, false},
    test_case{"idle_scheduler_test", idle_scheduler_test, false},
    test_case{"thread_group_system_test", thread_group_system_test, false},
    test_case{"wait_system_test", wait_system_test, false},
    test_case{"clone_system_test", clone_system_test, false},
    test_case{"exit_system_test", exit_system_test, false},
    test_case{"ramfs_system_test", ramfs_system_test, false},
    test_case{"fatfs_system_test", fatfs_system_test, false},
    test_case{"mutex_test", mutex_test, false},
    test_case{"kernel_task_test", kernel_task_test, false},
    test_case{"user_task_test", user_task_test, false}};

/// 主核运行所有测试
void run_tests_main() {
  for (auto test : test_cases) {
    klog::Info("----%s----", test.name);
    if (test.func()) {
      klog::Info("----%s passed----", test.name);
    } else {
      klog::Err("----%s failed----", test.name);
    }
  }
  klog::Info("All tests done.");
}

/// 从核只参与多核测试
void run_tests_smp() {
  for (auto test : test_cases) {
    if (test.is_smp_test) {
      // 从核静默参与多核测试，不输出日志
      test.func();
    }
  }
}

/// 非启动核入口
auto main_smp(int argc, const char** argv) -> int {
  per_cpu::GetCurrentCore() = per_cpu::PerCpu(cpu_io::GetCurrentCoreId());
  ArchInitSMP(argc, argv);
  MemoryInitSMP();
  InterruptInitSMP(argc, argv);
  TaskManagerSingleton::instance().InitCurrentCore();
  klog::Info("Hello SimpleKernel SMP");

  // 从核只参与多核测试
  run_tests_smp();

  __builtin_unreachable();
}

}  // namespace

void _start(int argc, const char** argv) {
  if (argv != nullptr) {
    CppInit();
    main(argc, argv);
    CppDeInit();
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

  // 唤醒其余 core
  // WakeUpOtherCores();

  DumpStack();

  klog::Info("Hello SimpleKernel");

  // 主核运行所有测试（包括多核测试）
  run_tests_main();

  __builtin_unreachable();
}
