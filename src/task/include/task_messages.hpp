/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/message.h>

#include "resource_id.hpp"

/// Task FSM 消息 ID
namespace task_msg_id {
inline constexpr etl::message_id_t kSchedule = 1;
inline constexpr etl::message_id_t kYield = 2;
inline constexpr etl::message_id_t kSleep = 3;
inline constexpr etl::message_id_t kBlock = 4;
inline constexpr etl::message_id_t kWakeup = 5;
inline constexpr etl::message_id_t kExit = 6;
inline constexpr etl::message_id_t kReap = 7;
inline constexpr etl::message_id_t kStop = 8;
inline constexpr etl::message_id_t kCont = 9;

[[nodiscard]] constexpr auto GetName(etl::message_id_t id) -> const char* {
  switch (id) {
    case kSchedule:
      return "MsgSchedule";
    case kYield:
      return "MsgYield";
    case kSleep:
      return "MsgSleep";
    case kBlock:
      return "MsgBlock";
    case kWakeup:
      return "MsgWakeup";
    case kExit:
      return "MsgExit";
    case kReap:
      return "MsgReap";
    case kStop:
      return "MsgStop";
    case kCont:
      return "MsgCont";
    default:
      return "Unknown";
  }
}

}  // namespace task_msg_id

/// 消息路由 ID
namespace router_id {
inline constexpr etl::message_router_id_t kTimerHandler = 0;
inline constexpr etl::message_router_id_t kTaskFsm = 1;
inline constexpr etl::message_router_id_t kVirtioBlk = 2;
inline constexpr etl::message_router_id_t kVirtioNet = 3;
}  // namespace router_id

/**
 * @brief 调度消息（无负载，用作事件）
 */
struct MsgSchedule : public etl::message<task_msg_id::kSchedule> {};

/**
 * @brief 让出消息（无负载，用作事件）
 */
struct MsgYield : public etl::message<task_msg_id::kYield> {};

/**
 * @brief 唤醒消息（无负载，用作事件）
 */
struct MsgWakeup : public etl::message<task_msg_id::kWakeup> {};

/**
 * @brief 回收消息（无负载，用作事件）
 */
struct MsgReap : public etl::message<task_msg_id::kReap> {};

/**
 * @brief 停止消息（SIGSTOP/SIGTSTP 触发）
 */
struct MsgStop : public etl::message<task_msg_id::kStop> {};

/**
 * @brief 继续消息（SIGCONT 触发）
 */
struct MsgCont : public etl::message<task_msg_id::kCont> {};

/**
 * @brief 睡眠消息，携带唤醒时钟
 */
struct MsgSleep : public etl::message<task_msg_id::kSleep> {
  /// 唤醒时钟
  uint64_t wake_tick{0};

  /// @name 构造/析构函数
  /// @{
  explicit MsgSleep(uint64_t _wake_tick) : wake_tick(_wake_tick) {}
  MsgSleep() = default;
  MsgSleep(const MsgSleep&) = default;
  MsgSleep(MsgSleep&&) = default;
  auto operator=(const MsgSleep&) -> MsgSleep& = default;
  auto operator=(MsgSleep&&) -> MsgSleep& = default;
  ~MsgSleep() = default;
  /// @}
};

/**
 * @brief 阻塞消息，携带资源 ID
 */
struct MsgBlock : public etl::message<task_msg_id::kBlock> {
  /// 资源 ID
  ResourceId resource_id{};

  /// @name 构造/析构函数
  /// @{
  explicit MsgBlock(ResourceId _resource_id) : resource_id(_resource_id) {}
  MsgBlock() = default;
  MsgBlock(const MsgBlock&) = default;
  MsgBlock(MsgBlock&&) = default;
  auto operator=(const MsgBlock&) -> MsgBlock& = default;
  auto operator=(MsgBlock&&) -> MsgBlock& = default;
  ~MsgBlock() = default;
  /// @}
};

/**
 * @brief 退出消息，携带退出码与父任务标志
 */
struct MsgExit : public etl::message<task_msg_id::kExit> {
  /// 退出码
  int exit_code{0};
  /// 是否有父任务
  bool has_parent{false};

  /// @name 构造/析构函数
  /// @{
  MsgExit(int _exit_code, bool _has_parent)
      : exit_code(_exit_code), has_parent(_has_parent) {}
  MsgExit() = default;
  MsgExit(const MsgExit&) = default;
  MsgExit(MsgExit&&) = default;
  auto operator=(const MsgExit&) -> MsgExit& = default;
  auto operator=(MsgExit&&) -> MsgExit& = default;
  ~MsgExit() = default;
  /// @}
};
