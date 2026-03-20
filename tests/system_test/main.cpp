/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <atomic>
#include <cstdint>
#include <new>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_cstdio"
#include "kstd_libcxx.h"
#include "kstd_memory"
#include "per_cpu.hpp"
#include "spinlock.hpp"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

namespace {

struct test_case {
  const char* name;
  bool (*func)(void);
  bool is_smp_test = false;
};

constexpr size_t kTestCount = 27;

std::array<test_case, kTestCount> test_cases = {
    test_case{"ctor_dtor_test", ctor_dtor_test, false},
    test_case{"spinlock_test", spinlock_test, true},
    test_case{"memory_test", memory_test, false},
    test_case{"virtual_memory_test", virtual_memory_test, false},
    test_case{"interrupt_test", interrupt_test, false},
    test_case{"kernel_task_test", kernel_task_test, false},
    test_case{"user_task_test", user_task_test, false},
    test_case{"fifo_scheduler_test", fifo_scheduler_test, false},
    test_case{"rr_scheduler_test", rr_scheduler_test, false},
    test_case{"cfs_scheduler_test", cfs_scheduler_test, false},
    test_case{"idle_scheduler_test", idle_scheduler_test, false},
    test_case{"thread_group_test", thread_group_test, false},
    test_case{"wait_test", wait_test, false},
    test_case{"clone_test", clone_test, false},
    test_case{"exit_test", exit_test, false},
    test_case{"cross_core_test", cross_core_test, false},
    test_case{"mutex_test", mutex_test, false},
    test_case{"yield_test", yield_test, false},
    test_case{"fork_test", fork_test, false},
    test_case{"signal_test", signal_test, false},
    test_case{"affinity_test", affinity_test, false},
    test_case{"tick_test", tick_test, false},
    test_case{"zombie_reap_test", zombie_reap_test, false},
    test_case{"stress_test", stress_test, false},
    test_case{"ramfs_test", ramfs_test, false},
    test_case{"fatfs_test", fatfs_test, false},
    test_case{"balance_test", balance_test, false},
};

std::array<TestResult, kTestCount> test_results{};

std::atomic<size_t> g_cores_ready{0};

void test_thread_entry(void* arg) {
  auto idx = reinterpret_cast<uint64_t>(arg);
  auto& test = test_cases[idx];

  klog::Info("[TEST] Starting: {}", test.name);

  bool passed = test.func();

  klog::Info("[TEST] Finished: {} — {}", test.name, passed ? "PASS" : "FAIL");

  sys_exit(passed ? 0 : 1);
}

void print_test_summary() {
  int passed = 0;
  int failed = 0;
  int timed_out = 0;

  klog::Info("========================================");
  klog::Info("     System Test Results");
  klog::Info("========================================");

  for (size_t i = 0; i < kTestCount; ++i) {
    auto& r = test_results[i];
    const char* tag = "???";

    switch (r.status) {
      case TestThreadStatus::kPassed:
        tag = "PASS";
        passed++;
        break;
      case TestThreadStatus::kFailed:
        tag = "FAIL";
        failed++;
        break;
      case TestThreadStatus::kTimeout:
        tag = "TIMEOUT";
        timed_out++;
        break;
      case TestThreadStatus::kRunning:
        tag = "TIMEOUT";
        timed_out++;
        break;
      default:
        tag = "SKIP";
        break;
    }

    if (test_cases[i].is_smp_test) {
      klog::Info("  [{}] {} (SMP, exit_code={})", tag, r.name, r.exit_code);
    } else {
      klog::Info("  [{}] {} (pid={}, exit_code={})", tag, r.name, r.pid,
                 r.exit_code);
    }
  }

  int total = static_cast<int>(kTestCount);

  klog::Info("========================================");
  klog::Info("  Total: {} | Passed: {} | Failed: {} | Timeout: {}", total,
             passed, failed, timed_out);
  klog::Info("========================================");
}

void test_runner_entry(void* /*arg*/) {
  auto& task_mgr = TaskManagerSingleton::instance();
  auto* runner = task_mgr.GetCurrentTask();

  // Phase 2: 为每个非 SMP 测试创建独立线程，建立父子关系
  int thread_test_count = 0;

  for (size_t i = 0; i < kTestCount; ++i) {
    if (test_cases[i].is_smp_test) {
      continue;
    }

    auto task = kstd::make_unique<TaskControlBlock>(
        test_cases[i].name, 10, test_thread_entry, reinterpret_cast<void*>(i));

    // Wait() requires parent-child relationship to collect exit status
    task->aux->parent_pid = runner->pid;
    task->aux->pgid = runner->aux->pgid;

    // Save raw pointer: pid is assigned inside AddTask() by AllocatePid(),
    // so we must read it *after* the call (unique_ptr is moved).
    auto* task_ptr = task.get();
    test_results[i].status = TestThreadStatus::kRunning;

    task_mgr.AddTask(std::move(task));
    test_results[i].pid = task_ptr->pid;
    thread_test_count++;
  }

  klog::Info("[RUNNER] Spawned {} test threads, collecting via Wait()...",
             thread_test_count);

  // Phase 3: 通过 Wait() 收集所有测试线程的退出状态
  int collected = 0;
  constexpr int kMaxWaitRetries = 1200;  // 1200 * 50ms = 60s 超时
  int retries = 0;

  while (collected < thread_test_count && retries < kMaxWaitRetries) {
    int status = 0;
    auto wait_result =
        task_mgr.Wait(static_cast<Pid>(-1), &status, true, false);

    if (wait_result.has_value() && wait_result.value() > 0) {
      Pid exited_pid = wait_result.value();

      bool is_test_thread = false;
      for (size_t i = 0; i < kTestCount; ++i) {
        if (test_results[i].pid == static_cast<int64_t>(exited_pid)) {
          test_results[i].exit_code = status;
          test_results[i].status = (status == 0) ? TestThreadStatus::kPassed
                                                 : TestThreadStatus::kFailed;

          klog::Info("[RUNNER] Collected: {} (pid={}, exit_code={}) — {}",
                     test_cases[i].name, exited_pid, status,
                     (status == 0) ? "PASS" : "FAIL");
          is_test_thread = true;
          break;
        }
      }

      if (is_test_thread) {
        collected++;
      } else {
        klog::Debug("[RUNNER] Reaped orphan pid={}, not a test thread",
                    exited_pid);
      }
    } else {
      (void)sys_sleep(50);
      retries++;
    }
  }

  for (size_t i = 0; i < kTestCount; ++i) {
    if (test_cases[i].is_smp_test) {
      continue;
    }
    if (test_results[i].status == TestThreadStatus::kPending ||
        test_results[i].status == TestThreadStatus::kRunning) {
      test_results[i].status = TestThreadStatus::kTimeout;
      klog::Err("[RUNNER] Timeout: {} (pid={})", test_cases[i].name,
                test_results[i].pid);
    }
  }

  // Phase 4: 汇总打印
  print_test_summary();

  bool all_passed = true;
  for (size_t i = 0; i < kTestCount; ++i) {
    if (test_results[i].status != TestThreadStatus::kPassed) {
      all_passed = false;
      break;
    }
  }

  QemuExit(all_passed);
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
  TimerInitSMP();
  klog::Info("Hello SimpleKernel SMP");

  g_cores_ready.fetch_add(1, std::memory_order_release);

  run_tests_smp();

  TaskManagerSingleton::instance().Schedule();

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
  per_cpu::PerCpuArraySingleton::create();
  per_cpu::GetCurrentCore() = per_cpu::PerCpu(cpu_io::GetCurrentCoreId());

  ArchInit(argc, argv);
  MemoryInit();
  InterruptInit(argc, argv);
  DeviceInit();
  FileSystemInit();

  TaskManagerSingleton::create();
  TaskManagerSingleton::instance().InitCurrentCore();

  TimerInit();

  WakeUpOtherCores();

  DumpStack();

  klog::Info("Hello SimpleKernel");

  for (size_t i = 0; i < kTestCount; ++i) {
    test_results[i].name = test_cases[i].name;
    test_results[i].status = TestThreadStatus::kPending;
    test_results[i].exit_code = -1;
    test_results[i].pid = 0;
  }

  // Phase 1: SMP 测试同步运行（需要跨核屏障协调，必须在调度器启动前完成）
  size_t expected_cores = BasicInfoSingleton::instance().core_count - 1;
  klog::Info("[RUNNER] Waiting for {} secondary core(s) to initialize...",
             expected_cores);
  while (g_cores_ready.load(std::memory_order_acquire) < expected_cores) {
    cpu_io::Pause();
  }
  klog::Info("[RUNNER] All cores ready, starting SMP tests");

  for (size_t i = 0; i < kTestCount; ++i) {
    if (!test_cases[i].is_smp_test) {
      continue;
    }
    auto& result = test_results[i];
    result.status = TestThreadStatus::kRunning;

    klog::Info("[SMP] Running: {}", test_cases[i].name);
    bool smp_passed = test_cases[i].func();

    result.exit_code = smp_passed ? 0 : 1;
    result.status =
        smp_passed ? TestThreadStatus::kPassed : TestThreadStatus::kFailed;

    klog::Info("[SMP] Finished: {} — {}", test_cases[i].name,
               smp_passed ? "PASS" : "FAIL");
  }

  auto runner = kstd::make_unique<TaskControlBlock>("test_runner", 10,
                                                    test_runner_entry, nullptr);
  TaskManagerSingleton::instance().AddTask(std::move(runner));

  TaskManagerSingleton::instance().Schedule();

  __builtin_unreachable();
}
