/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/fsm.h>

#include <atomic>

#include "kernel_log.hpp"
#include "task_messages.hpp"

/// 任务状态 ID — 用作 etl::fsm 的状态 ID
namespace TaskStatusId {
inline constexpr etl::fsm_state_id_t kUnInit = 0;
inline constexpr etl::fsm_state_id_t kReady = 1;
inline constexpr etl::fsm_state_id_t kRunning = 2;
inline constexpr etl::fsm_state_id_t kSleeping = 3;
inline constexpr etl::fsm_state_id_t kBlocked = 4;
inline constexpr etl::fsm_state_id_t kExited = 5;
inline constexpr etl::fsm_state_id_t kZombie = 6;
}  // namespace TaskStatusId

// 前向声明所有状态类，以便在转换表中相互引用
struct StateUnInit;
struct StateReady;
struct StateRunning;
struct StateSleeping;
struct StateBlocked;
struct StateExited;
struct StateZombie;

/**
 * @brief UnInit 状态 — 任务尚未初始化
 */
struct StateUnInit : public etl::fsm_state<etl::fsm, StateUnInit,
                                           TaskStatusId::kUnInit, MsgSchedule> {
  auto on_event(const MsgSchedule&) -> etl::fsm_state_id_t {
    return TaskStatusId::kReady;
  }
  auto on_event_unknown(const etl::imessage& msg) -> etl::fsm_state_id_t {
    klog::Warn("TaskFsm: UnInit received unexpected message id={}",
               static_cast<int>(msg.get_message_id()));
    return STATE_ID;
  }
};

/**
 * @brief Ready 状态 — 任务已就绪，等待调度
 */
struct StateReady : public etl::fsm_state<etl::fsm, StateReady,
                                          TaskStatusId::kReady, MsgSchedule> {
  auto on_event(const MsgSchedule&) -> etl::fsm_state_id_t {
    return TaskStatusId::kRunning;
  }
  auto on_event_unknown(const etl::imessage& msg) -> etl::fsm_state_id_t {
    klog::Warn("TaskFsm: Ready received unexpected message id={}",
               static_cast<int>(msg.get_message_id()));
    return STATE_ID;
  }
};

/**
 * @brief Running 状态 — 任务正在执行
 */
struct StateRunning
    : public etl::fsm_state<etl::fsm, StateRunning, TaskStatusId::kRunning,
                            MsgYield, MsgSleep, MsgBlock, MsgExit> {
  auto on_event(const MsgYield&) -> etl::fsm_state_id_t {
    return TaskStatusId::kReady;
  }
  auto on_event(const MsgSleep&) -> etl::fsm_state_id_t {
    return TaskStatusId::kSleeping;
  }
  auto on_event(const MsgBlock&) -> etl::fsm_state_id_t {
    return TaskStatusId::kBlocked;
  }
  auto on_event(const MsgExit& msg) -> etl::fsm_state_id_t {
    if (msg.has_parent) {
      return TaskStatusId::kZombie;
    }
    return TaskStatusId::kExited;
  }
  auto on_event_unknown(const etl::imessage& msg) -> etl::fsm_state_id_t {
    klog::Warn("TaskFsm: Running received unexpected message id={}",
               static_cast<int>(msg.get_message_id()));
    return STATE_ID;
  }
};

/**
 * @brief Sleeping 状态 — 任务已挂起，等待唤醒时钟
 */
struct StateSleeping
    : public etl::fsm_state<etl::fsm, StateSleeping, TaskStatusId::kSleeping,
                            MsgWakeup> {
  auto on_event(const MsgWakeup&) -> etl::fsm_state_id_t {
    return TaskStatusId::kReady;
  }
  auto on_event_unknown(const etl::imessage& msg) -> etl::fsm_state_id_t {
    klog::Warn("TaskFsm: Sleeping received unexpected message id={}",
               static_cast<int>(msg.get_message_id()));
    return STATE_ID;
  }
};

/**
 * @brief Blocked 状态 — 任务阻塞，等待资源
 */
struct StateBlocked : public etl::fsm_state<etl::fsm, StateBlocked,
                                            TaskStatusId::kBlocked, MsgWakeup> {
  auto on_event(const MsgWakeup&) -> etl::fsm_state_id_t {
    return TaskStatusId::kReady;
  }
  auto on_event_unknown(const etl::imessage& msg) -> etl::fsm_state_id_t {
    klog::Warn("TaskFsm: Blocked received unexpected message id={}",
               static_cast<int>(msg.get_message_id()));
    return STATE_ID;
  }
};

/**
 * @brief Exited 状态 — 任务已退出，无父任务回收
 */
struct StateExited : public etl::fsm_state<etl::fsm, StateExited,
                                           TaskStatusId::kExited, MsgReap> {
  auto on_event(const MsgReap&) -> etl::fsm_state_id_t { return STATE_ID; }
  auto on_event_unknown(const etl::imessage& msg) -> etl::fsm_state_id_t {
    klog::Warn("TaskFsm: Exited received unexpected message id={}",
               static_cast<int>(msg.get_message_id()));
    return STATE_ID;
  }
};

/**
 * @brief Zombie 状态 — 任务已退出，等待父任务回收
 */
struct StateZombie : public etl::fsm_state<etl::fsm, StateZombie,
                                           TaskStatusId::kZombie, MsgReap> {
  auto on_event(const MsgReap&) -> etl::fsm_state_id_t {
    return TaskStatusId::kExited;
  }
  auto on_event_unknown(const etl::imessage& msg) -> etl::fsm_state_id_t {
    klog::Warn("TaskFsm: Zombie received unexpected message id={}",
               static_cast<int>(msg.get_message_id()));
    return STATE_ID;
  }
};

/**
 * @brief 任务有限状态机
 */
class TaskFsm {
 public:
  /**
   * @brief 启动 FSM（在 TCB 完全构造后调用）
   */
  auto Start() -> void {
    fsm_.start();
    cached_state_.store(fsm_.get_state_id(), std::memory_order_release);
  }

  /**
   * @brief 向 FSM 发送消息
   * @param msg 要发送的消息
   * @pre 调用者持有保护此 TCB 的锁（如 cpu_sched.lock）
   * @post cached_state_ 以 release 语义更新，跨核可见
   */
  auto Receive(const etl::imessage& msg) -> void {
    fsm_.receive(msg);
    cached_state_.store(fsm_.get_state_id(), std::memory_order_release);
  }

  /**
   * @brief 获取当前状态 ID（线程安全）
   * @return etl::fsm_state_id_t 当前状态 ID
   * @note 使用 acquire 语义从 atomic 缓存读取，
   *       无需持有与 Receive() 相同的锁即可安全调用
   */
  [[nodiscard]] auto GetStateId() const -> etl::fsm_state_id_t {
    return cached_state_.load(std::memory_order_acquire);
  }

  /// @name 构造/析构函数
  /// @{
  TaskFsm() : fsm_(router_id::kTaskFsm) {
    state_list_[0] = &state_uninit_;
    state_list_[1] = &state_ready_;
    state_list_[2] = &state_running_;
    state_list_[3] = &state_sleeping_;
    state_list_[4] = &state_blocked_;
    state_list_[5] = &state_exited_;
    state_list_[6] = &state_zombie_;
    fsm_.set_states(state_list_, 7);
  }

  TaskFsm(const TaskFsm&) = delete;
  TaskFsm(TaskFsm&&) = delete;
  auto operator=(const TaskFsm&) -> TaskFsm& = delete;
  auto operator=(TaskFsm&&) -> TaskFsm& = delete;
  ~TaskFsm() = default;
  /// @}

 private:
  StateUnInit state_uninit_;
  StateReady state_ready_;
  StateRunning state_running_;
  StateSleeping state_sleeping_;
  StateBlocked state_blocked_;
  StateExited state_exited_;
  StateZombie state_zombie_;

  etl::ifsm_state* state_list_[7];

  etl::fsm fsm_;

  /// 跨核可见的状态缓存，解决 etl::fsm 非线程安全的问题
  std::atomic<etl::fsm_state_id_t> cached_state_{TaskStatusId::kUnInit};
};
