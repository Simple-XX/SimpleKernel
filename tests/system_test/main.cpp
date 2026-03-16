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

void run_tests_main() {
  int passed = 0;
  int failed = 0;

  for (const auto& test : test_cases) {
    klog::Info("----%s----", test.name);
    if (test.func()) {
      passed++;
      klog::Info("----%s PASS----", test.name);
    } else {
      failed++;
      klog::Err("----%s FAIL----", test.name);
    }
  }

  klog::Info("========================================");
  klog::Info("Result: %d passed, %d failed, %d total", passed, failed,
             passed + failed);
  klog::Info("========================================");

  QemuExit(failed == 0);
}

void run_tests_smp() {
  for (const auto& test : test_cases) {
    if (test.is_smp_test) {
      test.func();
    }
  }
}

auto main_smp(int argc, const char** argv) -> int {
  per_cpu::GetCurrentCore() = per_cpu::PerCpu(cpu_io::GetCurrentCoreId());
  ArchInitSMP(argc, argv);
  MemoryInitSMP();
  InterruptInitSMP(argc, argv);
  TaskManagerSingleton::instance().InitCurrentCore();
  klog::Info("Hello SimpleKernel SMP");

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
  per_cpu::PerCpuArraySingleton::create();
  per_cpu::GetCurrentCore() = per_cpu::PerCpu(cpu_io::GetCurrentCoreId());

  ArchInit(argc, argv);
  MemoryInit();
  InterruptInit(argc, argv);
  DeviceInit();
  FileSystemInit();

  TaskManagerSingleton::create();
  TaskManagerSingleton::instance().InitCurrentCore();

  DumpStack();

  klog::Info("Hello SimpleKernel");

  run_tests_main();

  __builtin_unreachable();
}
