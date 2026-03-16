/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>
#include <etl/flags.h>
#include <etl/intrusive_links.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include "file_descriptor.hpp"
#include "resource_id.hpp"
#include "task_fsm.hpp"

/// 进程 ID 类型
using Pid = size_t;

/// 线程入口函数类型
using ThreadEntry = void (*)(void*);

/**
 * @brief Clone 标志位常量 (用于 sys_clone 系统调用)
 * @note 参考 Linux clone flags
 */
namespace clone_flag {
/// 共享地址空间
inline constexpr uint64_t kVm = 0x00000100;
/// 共享文件系统信息
inline constexpr uint64_t kFs = 0x00000200;
/// 共享文件描述符表
inline constexpr uint64_t kFiles = 0x00000400;
/// 共享信号处理器
inline constexpr uint64_t kSighand = 0x00000800;
/// 保持相同父进程
inline constexpr uint64_t kParent = 0x00008000;
/// 同一线程组
inline constexpr uint64_t kThread = 0x00010000;
/// 全部标志掩码
inline constexpr uint64_t kAllMask =
    kVm | kFs | kFiles | kSighand | kParent | kThread;
}  // namespace clone_flag

/// 克隆标志位
using CloneFlags = etl::flags<uint64_t, clone_flag::kAllMask>;

/// CPU 亲和性位掩码
using CpuAffinity = etl::flags<uint64_t>;

/// Task status type alias — backed by FSM state IDs
/// @todo 为什么要定义 TaskStatusId 与 TaskStatus 两个命名空间？可以直接用
/// TaskStatus 吗？
namespace TaskStatus = TaskStatusId;

/**
 * @brief 调度策略
 */
enum class SchedPolicy : uint8_t {
  /// 实时任务 (最高优先级)
  kRealTime = 0,
  /// 普通任务
  kNormal = 1,
  /// 空闲任务 (最低优先级)
  kIdle = 2,
  /// 策略数量
  kPolicyCount
};

/// 线程组侵入式链表节点类型
using ThreadGroupLink = etl::bidirectional_link<0>;

/**
 * @brief 非调度热路径的任务辅助数据
 *
 * @details 包含进程/线程的标识、克隆标志、CPU 亲和性、文件描述符等
 *          仅在 clone/exit/wait/block 等慢路径中访问的字段。
 *          从 TaskControlBlock 分离以减少调度热路径的缓存压力。
 */
struct TaskAuxData {
  /// 父线程 ID
  Pid parent_pid{0};
  /// 进程组 ID
  Pid pgid{0};
  /// 会话 ID
  Pid sid{0};
  /// 线程组 ID (主线程的 PID)
  Pid tgid{0};

  /// 退出码
  int exit_code{0};

  /// 克隆标志位
  CloneFlags clone_flags{};

  /// CPU 亲和性位掩码
  CpuAffinity cpu_affinity{UINT64_MAX};

  /// 等待的资源 ID
  ResourceId blocked_on{};

  /// 是否为中断线程
  bool is_interrupt_thread{false};
  /// 关联的中断号
  uint64_t interrupt_number{0};

  /// @todo 优先级继承相关

  /// 文件描述符表
  filesystem::FileDescriptorTable* fd_table{nullptr};
};

/**
 * @brief 任务控制块，管理进程/线程的核心数据结构
 *
 * @details 仅保留调度热路径字段（状态机、优先级、上下文、页表等）。
 *          进程级辅助数据存储在 TaskAuxData 中，通过 aux 指针访问。
 */
struct TaskControlBlock : public ThreadGroupLink {
  /// 默认内核栈大小 (16 KB)
  static constexpr size_t kDefaultKernelStackSize = 16 * 1024;

  /**
   * @brief 任务优先级比较函数，优先级数值越小，优先级越高
   */
  struct PriorityCompare {
    /**
     * @brief 比较两个任务的优先级
     * @param a 第一个任务控制块指针
     * @param b 第二个任务控制块指针
     * @return bool 如果 a 的优先级低于 b 返回 true
     */
    auto operator()(TaskControlBlock* a, TaskControlBlock* b) -> bool {
      return a->sched_info.priority > b->sched_info.priority;
    }
  };

  /**
   * @brief 任务唤醒时间比较函数，时间越早优先级越高
   */
  struct WakeTickCompare {
    /**
     * @brief 比较两个任务的唤醒时间
     * @param a 第一个任务控制块指针
     * @param b 第二个任务控制块指针
     * @return bool 如果 a 的唤醒时间晚于 b 返回 true
     */
    auto operator()(TaskControlBlock* a, TaskControlBlock* b) -> bool {
      return a->sched_info.wake_tick > b->sched_info.wake_tick;
    }
  };

  /// 任务名称
  const char* name{"Unnamed Task"};

  /// 线程 ID (Task ID)
  Pid pid{0};

  /// 任务状态机
  TaskFsm fsm{};

  /// 调度策略
  SchedPolicy policy{SchedPolicy::kNormal};

  /**
   * @brief 基础调度信息
   */
  struct SchedInfo {
    /// 优先级 (数字越小优先级越高)
    int priority{10};
    /// 基础优先级 (静态，用于优先级继承)
    int base_priority{10};
    /// 继承的优先级
    int inherited_priority{0};
    /// 唤醒时间
    uint64_t wake_tick{0};
    /// 剩余时间片
    uint64_t time_slice_remaining{10};
    /// 默认时间片
    uint64_t time_slice_default{10};
    /// 总运行时间
    uint64_t total_runtime{0};
    /// 上下文切换次数
    uint64_t context_switches{0};
  } sched_info;

  /**
   * @brief 不同调度器的专用字段 (互斥使用)
   */
  union SchedData {
    /// CFS 调度器数据
    struct {
      /// 虚拟运行时间
      uint64_t vruntime;
      /// 任务权重 (1024 为默认)
      uint32_t weight;
    } cfs;

    /// MLFQ 调度器数据
    struct {
      /// 优先级级别 (0 = 最高)
      uint8_t level;
    } mlfq;
  } sched_data{};

  /// 内核栈
  uint8_t* kernel_stack{nullptr};

  /// Trap 上下文
  cpu_io::TrapContext* trap_context_ptr{nullptr};
  /// 任务上下文
  cpu_io::CalleeSavedContext task_context{};
  /// 页表
  uint64_t* page_table{nullptr};

  /// 非调度热路径的辅助数据
  TaskAuxData* aux{nullptr};

  /**
   * @brief 获取当前任务状态
   * @return etl::fsm_state_id_t 当前任务状态 ID
   */
  [[nodiscard]] auto GetStatus() const -> etl::fsm_state_id_t;

  /**
   * @brief 检查是否是线程组的主线程
   * @return true 如果是主线程 (pid == tgid)
   */
  [[nodiscard]] auto IsThreadGroupLeader() const -> bool {
    return aux && pid == aux->tgid;
  }

  /**
   * @brief 将线程添加到线程组
   * @param leader 线程组的主线程
   * @note 调用者需要确保加锁
   */
  auto JoinThreadGroup(TaskControlBlock* leader) -> void;

  /**
   * @brief 从线程组中移除自己
   * @note 调用者需要确保加锁
   */
  auto LeaveThreadGroup() -> void;

  /**
   * @brief 获取线程组中的线程数量
   * @return size_t 线程数量 (包括自己)
   */
  [[nodiscard]] auto GetThreadGroupSize() const -> size_t;

  /**
   * @brief 检查是否与另一个任务在同一线程组
   * @param other 另一个任务
   * @return true 如果在同一线程组
   */
  [[nodiscard]] auto InSameThreadGroup(const TaskControlBlock* other) const
      -> bool {
    return aux && other && other->aux && (aux->tgid == other->aux->tgid) &&
           (aux->tgid != 0);
  }

  /// @name 构造/析构函数
  /// @{
  /**
   * @brief 构造函数 (内核线程)
   * @param name 任务名称
   * @param priority 优先级 (数字越小优先级越高)
   * @param entry 线程入口函数
   * @param arg 线程参数
   */
  TaskControlBlock(const char* name, int priority, ThreadEntry entry,
                   void* arg);

  /**
   * @brief 构造函数 (用户线程)
   * @param name 任务名称
   * @param priority 优先级 (数字越小优先级越高)
   * @param elf 指向 ELF 镜像的指针
   * @param argc 参数个数
   * @param argv 参数数组
   */
  TaskControlBlock(const char* name, int priority, uint8_t* elf, int argc,
                   char** argv);

  TaskControlBlock() = default;
  TaskControlBlock(const TaskControlBlock&) = delete;
  TaskControlBlock(TaskControlBlock&&) = delete;
  auto operator=(const TaskControlBlock&) -> TaskControlBlock& = delete;
  auto operator=(TaskControlBlock&&) -> TaskControlBlock& = delete;
  ~TaskControlBlock();
  /// @}
};
