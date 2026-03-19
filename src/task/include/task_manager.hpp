/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>
#include <etl/list.h>
#include <etl/memory.h>
#include <etl/priority_queue.h>
#include <etl/singleton.h>
#include <etl/unordered_map.h>
#include <etl/vector.h>

#include <MPMCQueue.hpp>
#include <array>
#include <cstddef>
#include <cstdint>

#include "expected.hpp"
#include "interrupt_base.h"
#include "kernel_config.hpp"
#include "kstd_memory"
#include "per_cpu.hpp"
#include "resource_id.hpp"
#include "scheduler_base.hpp"
#include "signal.hpp"
#include "spinlock.hpp"
#include "task_control_block.hpp"

/**
 * @brief 每个核心的调度数据 (RunQueue)
 */
struct CpuSchedData {
  SpinLock lock{"sched_lock"};

  /// 调度器数组 (按策略索引)
  std::array<etl::unique_ptr<SchedulerBase>,
             static_cast<uint8_t>(SchedPolicy::kPolicyCount)>
      schedulers{};

  /// 睡眠队列 (优先队列，按唤醒时间排序)
  etl::priority_queue<
      TaskControlBlock*, kernel::config::kMaxSleepingTasks,
      etl::vector<TaskControlBlock*, kernel::config::kMaxSleepingTasks>,
      TaskControlBlock::WakeTickCompare>
      sleeping_tasks;

  /// 阻塞队列 (按资源 ID 分组)
  etl::unordered_map<
      ResourceId,
      etl::list<TaskControlBlock*, kernel::config::kMaxBlockedPerGroup>,
      kernel::config::kMaxBlockedGroups,
      kernel::config::kMaxBlockedGroupsBuckets>
      blocked_tasks;

  /// Per-CPU tick 计数 (每个核心独立计时)
  uint64_t local_tick{0};

  /// 本核心的空闲时间 (单位: ticks)
  uint64_t idle_time{0};

  /// 本核心的总调度次数
  uint64_t total_schedules{0};

  /// Schedule() 是否已被显式调用
  bool scheduler_started{false};

  /// @name 构造/析构函数
  /// @{
  CpuSchedData() = default;
  CpuSchedData(const CpuSchedData&) = delete;
  CpuSchedData(CpuSchedData&&) = delete;
  auto operator=(const CpuSchedData&) -> CpuSchedData& = delete;
  auto operator=(CpuSchedData&&) -> CpuSchedData& = delete;
  ~CpuSchedData() = default;
  /// @}
};

/**
 * @brief 任务管理器
 *
 * 负责管理系统中的所有任务，包括任务的创建、调度、切换等。
 */
class TaskManager {
 public:
  /**
   * @brief 初始化 per cpu 的调度数据，创建 idle 线程
   */
  auto InitCurrentCore() -> void;

  /**
   * @brief 添加任务（接管所有权）
   *
   * 根据任务的调度策略，将其添加到对应的调度器中。
   *
   * @param task 任务控制块，所有权转移给 TaskManager
   * @pre task 非空，状态为 kUnInit
   */
  auto AddTask(etl::unique_ptr<TaskControlBlock> task) -> void;

  /**
   * @brief 调度函数
   * 选择下一个任务并切换上下文
   *
   * @note 被调用意味着需要调度决策，可能是
   * 时间片耗尽（TickUpdate 检测到需要抢占）
   * 主动让出 CPU (yield)
   * 任务阻塞、睡眠或退出
   */
  auto Schedule() -> void;

  /**
   * @brief 获取当前任务
   * @return TaskControlBlock* 当前正在运行的任务
   */
  [[nodiscard]] auto GetCurrentTask() const -> TaskControlBlock* {
    return per_cpu::GetCurrentCore().running_task;
  }

  /**
   * @brief 更新系统 tick
   */
  auto TickUpdate() -> void;

  /**
   * @brief 线程睡眠
   * @param ms 睡眠毫秒数
   */
  auto Sleep(uint64_t ms) -> void;

  /**
   * @brief 退出当前线程
   * @param exit_code 退出码
   */
  [[noreturn]] auto Exit(int exit_code = 0) -> void;

  /**
   * @brief 阻塞当前任务
   * @param resource_id 等待的资源 ID
   */
  auto Block(ResourceId resource_id) -> void;

  /**
   * @brief 阻塞当前任务（调用者已持有 cpu_sched.lock，不调用 Schedule）
   * @param cpu_sched 当前核心的调度数据
   * @param resource_id 等待的资源 ID
   * @pre cpu_sched.lock 已被调用者持有
   * @post 当前任务已转移到 blocked_tasks，FSM 状态为 kBlocked
   * @note 调用者必须在释放锁后调用 Schedule()
   */
  auto Block(CpuSchedData& cpu_sched, ResourceId resource_id) -> void;

  /**
   * @brief 唤醒等待指定资源的所有任务
   * @param resource_id 资源 ID
   * @note 会唤醒所有阻塞在此资源上的任务
   */
  auto Wakeup(ResourceId resource_id) -> void;

  /**
   * @brief 唤醒等待指定资源的第一个任务（跨核搜索）
   * @param resource_id 资源 ID
   * @note 只唤醒一个任务，用于互斥锁等只需唤醒一个等待者的场景
   */
  auto WakeupOne(ResourceId resource_id) -> void;

  /**
   * @brief 唤醒等待指定资源的所有任务（调用者已持有 cpu_sched.lock）
   * @param cpu_sched 当前核心的调度数据
   * @param resource_id 资源 ID
   * @pre cpu_sched.lock 已被调用者持有
   */
  auto Wakeup(CpuSchedData& cpu_sched, ResourceId resource_id) -> void;

  /**
   * @brief 克隆当前任务 (fork/clone 系统调用)
   * @param flags 克隆标志位
   *        - kCloneVm: 共享地址空间
   *        - kCloneThread: 共享线程组
   *        - kCloneFiles: 共享文件描述符表
   *        - kCloneSighand: 共享信号处理器
   *        - 0: 完全复制 (fork)
   * @param user_stack 用户栈指针 (nullptr 表示复制父进程栈)
   * @param parent_tid 父进程 TID 存储地址
   * @param child_tid 子进程 TID 存储地址
   * @param tls 线程局部存储指针
   * @param parent_context 父进程的 trap 上下文 (用于复制寄存器)
   * @return Expected<Pid> 父进程返回子进程 PID，子进程返回 0，失败返回错误
   */
  [[nodiscard]] auto Clone(uint64_t flags, void* user_stack, int* parent_tid,
                           int* child_tid, void* tls,
                           cpu_io::TrapContext& parent_context)
      -> Expected<Pid>;

  /**
   * @brief 等待子进程退出
   * @param pid 子进程 PID (-1 表示任意子进程，0 表示同组，>0 表示指定进程)
   * @param status 退出状态存储位置 (可为 nullptr)
   * @param no_hang 非阻塞等待，立即返回 (类似 WNOHANG)
   * @param untraced 报告已停止的子进程 (类似 WUNTRACED)
   * @return Expected<Pid> 成功返回子进程 PID，无子进程或被中断返回错误
   */
  [[nodiscard]] auto Wait(Pid pid, int* status, bool no_hang = false,
                          bool untraced = false) -> Expected<Pid>;

  /**
   * @brief 按 PID 查找任务
   * @param pid 进程 ID
   * @return TaskControlBlock* 找到的任务，未找到返回 nullptr
   */
  [[nodiscard]] auto FindTask(Pid pid) -> TaskControlBlock*;

  /**
   * @brief 获取当前核心的调度数据
   * @return CpuSchedData& 当前核心的调度数据引用
   */
  [[nodiscard]] auto GetCurrentCpuSched() -> CpuSchedData& {
    return cpu_schedulers_[cpu_io::GetCurrentCoreId()];
  }

  /// @name 信号机制
  /// @{

  /**
   * @brief 向指定任务发送信号
   * @param pid 目标任务 PID
   * @param signum 信号编号
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] auto SendSignal(Pid pid, int signum) -> Expected<void>;

  /**
   * @brief 检查并处理当前任务的待处理信号
   * @return int 处理的信号编号，无信号返回 0
   */
  [[nodiscard]] auto CheckPendingSignals() -> int;

  /**
   * @brief 设置信号处理函数
   * @param signum 信号编号
   * @param action 新的信号动作
   * @param old_action 旧的信号动作（输出参数，可为 nullptr）
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] auto SetSignalAction(int signum, const SignalAction& action,
                                     SignalAction* old_action)
      -> Expected<void>;

  /**
   * @brief 修改当前任务的信号掩码
   * @param how 操作方式 (kSigBlock, kSigUnblock, kSigSetmask)
   * @param set 要操作的信号集
   * @param oldset 旧信号集输出（可为 nullptr）
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] auto SetSignalMask(int how, uint32_t set, uint32_t* oldset)
      -> Expected<void>;

  /// @}

  /// @name 构造/析构函数
  /// @{
  TaskManager() = default;
  TaskManager(const TaskManager&) = delete;
  TaskManager(TaskManager&&) = delete;
  auto operator=(const TaskManager&) -> TaskManager& = delete;
  auto operator=(TaskManager&&) -> TaskManager& = delete;
  ~TaskManager();
  /// @}

 private:
  /// 中断工作队列容量
  static constexpr size_t kInterruptQueueCapacity = 256;

  /**
   * @brief 中断线程处理结构体
   */
  struct InterruptWork {
    using WorkHandler = void (*)(InterruptWork*);

    /// 中断号
    uint64_t interrupt_no{0};
    /// 中断上下文
    cpu_io::TrapContext* data{nullptr};
    /// 时间戳
    uint64_t timestamp{0};

    /// 工作处理函数
    WorkHandler handler{nullptr};
  };

  /// 中断工作队列
  using InterruptWorkQueue =
      mpmc_queue::MPMCQueue<InterruptWork, kInterruptQueueCapacity>;

  /// 每个核心的调度数据
  std::array<CpuSchedData, SIMPLEKERNEL_MAX_CORE_COUNT> cpu_schedulers_{};

  /// 全局任务表 (PID -> TCB 映射)
  SpinLock task_table_lock_{"task_table_lock"};
  etl::unordered_map<Pid, etl::unique_ptr<TaskControlBlock>,
                     kernel::config::kMaxTasks,
                     kernel::config::kMaxTasksBuckets>
      task_table_;

  /// 中断线程相关数据保护锁
  SpinLock interrupt_threads_lock_{"interrupt_threads_lock"};
  /// 中断号 -> 中断线程映射
  etl::unordered_map<uint64_t, TaskControlBlock*,
                     kernel::config::kMaxInterruptThreads,
                     kernel::config::kMaxInterruptThreadsBuckets>
      interrupt_threads_;
  /// 中断号 -> 工作队列映射
  etl::unordered_map<uint64_t, InterruptWorkQueue*,
                     kernel::config::kMaxInterruptThreads,
                     kernel::config::kMaxInterruptThreadsBuckets>
      interrupt_work_queues_;

  /// PID 分配器
  std::atomic<size_t> pid_allocator_{1};

  /**
   * @brief 分配新的 PID
   * @return size_t 新的 PID
   */
  [[nodiscard]] auto AllocatePid() -> size_t;

  /**
   * @brief 负载均衡 (空闲 core 窃取任务)
   */
  auto Balance() -> void;

  /**
   * @brief 获取线程组的所有线程
   * @param tgid 线程组 ID
   * @return etl::vector<TaskControlBlock*, kernel::config::kMaxReadyTasks>
   * 线程组中的所有线程
   */
  auto GetThreadGroup(Pid tgid)
      -> etl::vector<TaskControlBlock*, kernel::config::kMaxReadyTasks>;

  /**
   * @brief 向线程组中的所有线程发送信号
   * @param tgid 线程组 ID
   * @param signal 信号编号
   * @note 暂未实现信号机制，预留接口
   */
  auto SignalThreadGroup(Pid tgid, int signal) -> void;

  /**
   * @brief 回收僵尸进程资源
   * @param task 要回收的任务 (必须处于 kZombie 状态)
   * @note 释放内核栈、页表、TCB，回收 PID
   */
  auto ReapTask(TaskControlBlock* task) -> void;

  /**
   * @brief 将孤儿进程过继给 init 进程
   * @param parent 退出的父进程
   * @note 在父进程退出时调用，防止子进程变成僵尸无人回收
   */
  auto ReparentChildren(TaskControlBlock* parent) -> void;
};

using TaskManagerSingleton = etl::singleton<TaskManager>;
