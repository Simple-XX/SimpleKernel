/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "expected.hpp"
#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "sk_stdlib.h"
#include "task_manager.hpp"
#include "virtual_memory.hpp"

auto TaskManager::Clone(uint64_t flags, void* user_stack, int* parent_tid,
                        int* child_tid, void* tls,
                        cpu_io::TrapContext& parent_context) -> Expected<Pid> {
  auto* parent = GetCurrentTask();
  if (!parent) {
    klog::Err("Clone: No current task");
    return std::unexpected(Error(ErrorCode::kTaskNoCurrentTask));
  }

  // 验证克隆标志的合法性
  // 如果设置了 kCloneThread，必须同时设置 kCloneVm, kCloneFiles, kCloneSighand
  if ((flags & clone_flag::kThread) &&
      (!(flags & clone_flag::kVm) || !(flags & clone_flag::kFiles) ||
       !(flags & clone_flag::kSighand))) {
    klog::Warn(
        "Clone: kCloneThread requires kCloneVm, kCloneFiles, kCloneSighand");
    // 自动补全必需的标志
    flags |= (clone_flag::kVm | clone_flag::kFiles | clone_flag::kSighand);
  }

  // 分配新的 PID
  Pid new_pid = AllocatePid();
  if (new_pid == 0) {
    klog::Err("Clone: Failed to allocate PID");
    return std::unexpected(Error(ErrorCode::kTaskPidAllocationFailed));
  }

  // 创建子任务控制块
  auto child_ptr = kstd::make_unique<TaskControlBlock>();
  if (!child_ptr) {
    klog::Err("Clone: Failed to allocate child task");
    return std::unexpected(Error(ErrorCode::kTaskAllocationFailed));
  }
  auto* child = child_ptr.get();
  // Default ctor leaves FSM in set_states-but-not-started state.
  // Start it so get_state_id() returns kUnInit instead of deref null.
  child->fsm.Start();

  // 基本字段设置
  child->pid = new_pid;
  child->name = parent->name;
  child->policy = parent->policy;
  child->sched_info = parent->sched_info;

  // 设置父进程 ID
  if (flags & clone_flag::kParent) {
    // 保持与父进程相同的父进程
    child->aux->parent_pid = parent->aux->parent_pid;
  } else {
    child->aux->parent_pid = parent->pid;
  }

  // 处理线程组 ID (TGID)
  if (flags & clone_flag::kThread) {
    // 创建线程: 共享线程组
    child->aux->tgid = parent->aux->tgid;
    child->aux->pgid = parent->aux->pgid;
    child->aux->sid = parent->aux->sid;

    // 将子线程加入父线程的线程组
    if (parent->IsThreadGroupLeader()) {
      child->JoinThreadGroup(parent);
    } else {
      // 找到线程组的主线程
      TaskControlBlock* leader = FindTask(parent->aux->tgid);
      if (leader) {
        child->JoinThreadGroup(leader);
      } else {
        klog::Warn("Clone: Thread group leader not found for tgid={}",
                   parent->aux->tgid);
      }
    }
  } else {
    // 创建进程: 新的线程组
    // 新进程的 tgid 是自己的 pid
    child->aux->tgid = new_pid;
    child->aux->pgid = parent->aux->pgid;
    child->aux->sid = parent->aux->sid;
  }

  // 克隆标志位
  child->aux->clone_flags = CloneFlags(flags);

  // 处理文件描述符表 (kCloneFiles)
  /// @todo 当前未实现文件系统，此标志暂时仅记录
  if (flags & clone_flag::kFiles) {
    klog::Debug("Clone: sharing file descriptor table (not implemented)");
  } else {
    klog::Debug("Clone: copying file descriptor table (not implemented)");
  }

  // 处理信号处理器 (kCloneSighand)
  /// @todo 当前未实现信号机制，此标志暂时仅记录
  if (flags & clone_flag::kSighand) {
    klog::Debug("Clone: sharing signal handlers (not implemented)");
  } else {
    klog::Debug("Clone: copying signal handlers (not implemented)");
  }

  // 处理文件系统信息 (kCloneFs)
  /// @todo 当前未实现文件系统，此标志暂时仅记录
  if (flags & clone_flag::kFs) {
    klog::Debug("Clone: sharing filesystem info (not implemented)");
  } else {
    klog::Debug("Clone: copying filesystem info (not implemented)");
  }

  // 处理地址空间
  if (flags & clone_flag::kVm) {
    // 共享地址空间（线程）
    child->page_table = parent->page_table;
    klog::Debug("Clone: sharing page table {:#x}",
                reinterpret_cast<uintptr_t>(child->page_table));
  } else {
    // 复制地址空间（进程）
    if (parent->page_table) {
      // copy_mappings=true 表示复制用户空间映射
      auto result = VirtualMemorySingleton::instance().ClonePageDirectory(
          parent->page_table, true);
      if (!result.has_value()) {
        klog::Err("Clone: Failed to clone page table: {}",
                  result.error().message());
        // 独立页表已由 unique_ptr 在作用域结束时自动释放
        // 仅需清理页表
        if (child->page_table && !(flags & clone_flag::kVm)) {
          VirtualMemorySingleton::instance().DestroyPageDirectory(
              child->page_table, false);
          child->page_table = nullptr;
        }
        return std::unexpected(Error(ErrorCode::kTaskAllocationFailed));
      }
      child->page_table = reinterpret_cast<uint64_t*>(result.value());
      klog::Debug("Clone: cloned page table from {:#x} to {:#x}",
                  reinterpret_cast<uintptr_t>(parent->page_table),
                  reinterpret_cast<uintptr_t>(child->page_table));
    } else {
      // 父进程没有页表（内核线程），子进程也不需要
      child->page_table = nullptr;
    }
  }

  // 分配内核栈
  child->kernel_stack = static_cast<uint8_t*>(
      aligned_alloc(cpu_io::virtual_memory::kPageSize,
                    TaskControlBlock::kDefaultKernelStackSize));
  if (!child->kernel_stack) {
    klog::Err("Clone: Failed to allocate kernel stack");
    // 清理已分配的资源
    if (child->page_table && !(flags & clone_flag::kVm)) {
      VirtualMemorySingleton::instance().DestroyPageDirectory(child->page_table,
                                                              false);
      child->page_table = nullptr;
    }
    return std::unexpected(Error(ErrorCode::kTaskKernelStackAllocationFailed));
  }

  // 初始化内核栈内容
  kstd::memset(child->kernel_stack, 0,
               TaskControlBlock::kDefaultKernelStackSize);

  // 复制 trap context
  child->trap_context_ptr = reinterpret_cast<cpu_io::TrapContext*>(
      child->kernel_stack + TaskControlBlock::kDefaultKernelStackSize -
      sizeof(cpu_io::TrapContext));
  kstd::memcpy(child->trap_context_ptr, &parent_context,
               sizeof(cpu_io::TrapContext));

  // 设置用户栈
  if (user_stack) {
    child->trap_context_ptr->UserStackPointer() =
        reinterpret_cast<uint64_t>(user_stack);
  }

  // 设置 TLS (Thread Local Storage)
  if (tls) {
    child->trap_context_ptr->ThreadPointer() = reinterpret_cast<uint64_t>(tls);
  }

  // 父进程返回子进程 PID
  parent_context.ReturnValue() = new_pid;
  // 子进程返回 0
  child->trap_context_ptr->ReturnValue() = 0;

  // 写入 TID
  if (parent_tid) {
    *parent_tid = new_pid;
  }
  if (child_tid) {
    *child_tid = new_pid;
  }

  // 将子任务添加到调度器
  AddTask(std::move(child_ptr));

  // 打印详细的 clone 信息
  const char* clone_type = (flags & clone_flag::kThread) ? "thread" : "process";
  const char* vm_type = (flags & clone_flag::kVm) ? "shared" : "copied";
  klog::Debug(
      "Clone: created {} - parent={}, child={}, tgid={}, vm={}, flags={:#x}",
      clone_type, parent->pid, new_pid, child->aux->tgid, vm_type,
      static_cast<uint64_t>(flags));
  return new_pid;
}
