/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/message.h>

#include <cstddef>

namespace lifecycle_msg_id {
inline constexpr etl::message_id_t kThreadCreate = 100;
inline constexpr etl::message_id_t kThreadExit = 101;
}  // namespace lifecycle_msg_id

/**
 * @brief 线程创建消息
 */
struct ThreadCreateMsg : public etl::message<lifecycle_msg_id::kThreadCreate> {
  /// 进程 ID
  size_t pid{0};

  /// @name 构造/析构函数
  /// @{
  explicit ThreadCreateMsg(size_t _pid) : pid(_pid) {}
  ThreadCreateMsg() = default;
  ThreadCreateMsg(const ThreadCreateMsg&) = default;
  ThreadCreateMsg(ThreadCreateMsg&&) = default;
  auto operator=(const ThreadCreateMsg&) -> ThreadCreateMsg& = default;
  auto operator=(ThreadCreateMsg&&) -> ThreadCreateMsg& = default;
  ~ThreadCreateMsg() = default;
  /// @}
};

/**
 * @brief 线程退出消息
 */
struct ThreadExitMsg : public etl::message<lifecycle_msg_id::kThreadExit> {
  /// 进程 ID
  size_t pid{0};
  /// 退出码
  int exit_code{0};

  /// @name 构造/析构函数
  /// @{
  ThreadExitMsg(size_t _pid, int _exit_code)
      : pid(_pid), exit_code(_exit_code) {}
  ThreadExitMsg() = default;
  ThreadExitMsg(const ThreadExitMsg&) = default;
  ThreadExitMsg(ThreadExitMsg&&) = default;
  auto operator=(const ThreadExitMsg&) -> ThreadExitMsg& = default;
  auto operator=(ThreadExitMsg&&) -> ThreadExitMsg& = default;
  ~ThreadExitMsg() = default;
  /// @}
};
