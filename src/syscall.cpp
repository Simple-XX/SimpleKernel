/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "syscall.hpp"

#include "kernel.h"
#include "kernel_log.hpp"
#include "spinlock.hpp"
#include "task_manager.hpp"

namespace {
/// @brief SpinLock protecting futex check-and-block / wake atomicity
SpinLock futex_lock_;
}  // namespace

auto syscall_dispatcher(int64_t syscall_id, uint64_t args[6]) -> int {
  int64_t ret = 0;
  switch (syscall_id) {
    case kSyscallWrite:
      ret = sys_write(static_cast<int>(args[0]),
                      reinterpret_cast<const char*>(args[1]),
                      static_cast<size_t>(args[2]));
      break;
    case kSyscallExit:
      ret = sys_exit(static_cast<int>(args[0]));
      break;
    case kSyscallYield:
      ret = sys_yield();
      break;
    case kSyscallClone:
      ret = sys_clone(args[0], reinterpret_cast<void*>(args[1]),
                      reinterpret_cast<int*>(args[2]),
                      reinterpret_cast<int*>(args[3]),
                      reinterpret_cast<void*>(args[4]));
      break;
    case kSyscallFork:
      ret = sys_fork();
      break;
    case kSyscallGettid:
      ret = sys_gettid();
      break;
    case kSyscallSetTidAddress:
      ret = sys_set_tid_address(reinterpret_cast<int*>(args[0]));
      break;
    case kSyscallFutex:
      ret = sys_futex(
          reinterpret_cast<int*>(args[0]), static_cast<int>(args[1]),
          static_cast<int>(args[2]), reinterpret_cast<const void*>(args[3]),
          reinterpret_cast<int*>(args[4]), static_cast<int>(args[5]));
      break;
    case kSyscallSleep:
      ret = sys_sleep(args[0]);
      break;
    default:
      klog::Err("[Syscall] Unknown syscall id: {}", syscall_id);
      ret = -1;
      break;
  }
  return ret;
}

[[nodiscard]] auto sys_write(int fd, const char* buf, size_t len) -> int {
  // 简单实现：仅支持向标准输出(1)和错误输出(2)打印
  if (fd == 1 || fd == 2) {
    /// @todo应该检查 buf 是否在用户空间合法范围内
    for (size_t i = 0; i < len; ++i) {
      etl_putchar(buf[i]);
    }
    return static_cast<int>(len);
  }
  return -1;
}

auto sys_exit(int code) -> int {
  klog::Info("[Syscall] Process {} exited with code {}",
             TaskManagerSingleton::instance().GetCurrentTask()->pid, code);
  // 调用 TaskManager 的 Exit 方法处理线程退出
  TaskManagerSingleton::instance().Exit(code);
  // 不会执行到这里，因为 Exit 会触发调度切换
  klog::Err("[Syscall] sys_exit should not return!");
  return 0;
}

[[nodiscard]] auto sys_yield() -> int {
  TaskManagerSingleton::instance().Schedule();
  return 0;
}

[[nodiscard]] auto sys_sleep(uint64_t ms) -> int {
  TaskManagerSingleton::instance().Sleep(ms);
  return 0;
}

[[nodiscard]] auto sys_clone(uint64_t flags, void* stack, int* parent_tid,
                             int* child_tid, void* tls) -> int {
  auto& task_manager = TaskManagerSingleton::instance();
  auto current = task_manager.GetCurrentTask();

  if (!current || !current->trap_context_ptr) {
    klog::Err("[Syscall] sys_clone: Invalid current task or trap context");
    return -1;
  }

  // 调用 TaskManager 的 Clone 方法
  auto result = task_manager.Clone(flags, stack, parent_tid, child_tid, tls,
                                   *current->trap_context_ptr);

  if (!result.has_value()) {
    // 失败返回 -1
    klog::Err("[Syscall] sys_clone failed: {}", result.error().message());
    return -1;
  }

  Pid child_pid = result.value();
  if (child_pid == 0) {
    // 子进程/线程返回 0
    return 0;
  } else {
    // 父进程返回子进程 PID
    return static_cast<int>(child_pid);
  }
}

[[nodiscard]] auto sys_fork() -> int {
  auto& task_manager = TaskManagerSingleton::instance();
  auto current = task_manager.GetCurrentTask();

  if (!current || !current->trap_context_ptr) {
    klog::Err("[Syscall] sys_fork: Invalid current task or trap context");
    return -1;
  }

  // fork = clone with flags=0 (完全复制，不共享任何资源)
  auto result = task_manager.Clone(0, nullptr, nullptr, nullptr, nullptr,
                                   *current->trap_context_ptr);

  if (!result.has_value()) {
    // 失败返回 -1
    klog::Err("[Syscall] sys_fork failed: {}", result.error().message());
    return -1;
  }

  Pid child_pid = result.value();
  if (child_pid == 0) {
    // 子进程返回 0
    return 0;
  } else {
    // 父进程返回子进程 PID
    return static_cast<int>(child_pid);
  }
}

[[nodiscard]] auto sys_gettid() -> int {
  auto current = TaskManagerSingleton::instance().GetCurrentTask();
  if (!current) {
    klog::Err("[Syscall] sys_gettid: No current task");
    return -1;
  }
  return static_cast<int>(current->pid);
}

[[nodiscard]] auto sys_set_tid_address([[maybe_unused]] int* tidptr) -> int {
  auto current = TaskManagerSingleton::instance().GetCurrentTask();
  if (!current) {
    klog::Err("[Syscall] sys_set_tid_address: No current task");
    return -1;
  }

  // 保存 tidptr，在线程退出时会清零该地址并执行 futex wake
  /// @todo需要在 TaskControlBlock 中添加字段保存 tidptr
  // current->clear_child_tid = tidptr;

  // 返回当前线程的 TID
  return static_cast<int>(current->pid);
}

[[nodiscard]] auto sys_futex(int* uaddr, int op, int val,
                             [[maybe_unused]] const void* timeout,
                             [[maybe_unused]] int* uaddr2,
                             [[maybe_unused]] int val3) -> int {
  // Futex 常量定义
  static constexpr int kFutexWait = 0;
  static constexpr int kFutexWake = 1;
  static constexpr int kFutexRequeue = 3;

  // 提取操作类型（低位）
  int cmd = op & 0x7F;

  auto& task_manager = TaskManagerSingleton::instance();

  switch (cmd) {
    case kFutexWait: {
      klog::Debug("[Syscall] FUTEX_WAIT on {:#x} (val={})",
                  static_cast<uint64_t>(reinterpret_cast<uintptr_t>(uaddr)),
                  val);

      ResourceId futex_id(ResourceType::kFutex,
                          reinterpret_cast<uintptr_t>(uaddr));
      {
        LockGuard<SpinLock> guard(futex_lock_);
        if (*uaddr != val) {
          return 0;
        }
      }
      // futex_lock_ released before Block() — SpinLock must NOT be held
      // across context switch boundaries (Block → Schedule → switch_to).
      // A narrow race exists: FUTEX_WAKE between lock release and Block()
      // can miss this thread. Production kernels solve this with a two-phase
      // API (add-to-waitqueue under lock, schedule after release). This is
      // still far safer than the original completely-unprotected check.
      task_manager.Block(futex_id);
      return 0;
    }

    case kFutexWake: {
      klog::Debug("[Syscall] FUTEX_WAKE on {:#x} (count={})",
                  static_cast<uint64_t>(reinterpret_cast<uintptr_t>(uaddr)),
                  val);

      ResourceId futex_id(ResourceType::kFutex,
                          reinterpret_cast<uintptr_t>(uaddr));
      {
        LockGuard<SpinLock> guard(futex_lock_);
        task_manager.Wakeup(futex_id);
      }

      /// @todo应该返回实际唤醒的线程数
      return val;
    }

    case kFutexRequeue: {
      // 将等待 uaddr 的线程重新排队到 uaddr2
      /// @todo实现 FUTEX_REQUEUE
      klog::Warn("[Syscall] FUTEX_REQUEUE not implemented");
      return -1;
    }

    default:
      klog::Err("[Syscall] Unknown futex operation: {}", cmd);
      return -1;
  }
}

[[nodiscard]] auto sys_sched_getaffinity(int pid, size_t cpusetsize,
                                         uint64_t* mask) -> int {
  auto& task_manager = TaskManagerSingleton::instance();

  TaskControlBlock* target;
  if (pid == 0) {
    // pid=0 表示当前线程
    target = task_manager.GetCurrentTask();
  } else {
    // 查找指定 PID 的任务
    target = task_manager.FindTask(static_cast<Pid>(pid));
    if (!target) {
      klog::Err("[Syscall] sys_sched_getaffinity: Task {} not found", pid);
      return -1;
    }
  }

  if (!target) {
    return -1;
  }

  // 简单实现：将 cpu_affinity 复制到用户空间
  if (cpusetsize < sizeof(uint64_t)) {
    return -1;
  }

  *mask = target->aux->cpu_affinity;
  return 0;
}

[[nodiscard]] auto sys_sched_setaffinity(int pid, size_t cpusetsize,
                                         const uint64_t* mask) -> int {
  auto& task_manager = TaskManagerSingleton::instance();

  TaskControlBlock* target;
  if (pid == 0) {
    target = task_manager.GetCurrentTask();
  } else {
    // 查找指定 PID 的任务
    target = task_manager.FindTask(static_cast<Pid>(pid));
    if (!target) {
      klog::Err("[Syscall] sys_sched_setaffinity: Task {} not found", pid);
      return -1;
    }
  }

  if (!target) {
    return -1;
  }

  // 简单实现：更新 cpu_affinity
  if (cpusetsize < sizeof(uint64_t)) {
    return -1;
  }

  target->aux->cpu_affinity = *mask;

  klog::Debug("[Syscall] Set CPU affinity for task {} to {:#x}", target->pid,
              *mask);

  /// @todo 如果当前任务不在允许的 CPU 上运行，应该触发迁移

  return 0;
}
