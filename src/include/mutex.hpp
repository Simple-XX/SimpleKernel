/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "expected.hpp"
#include "resource_id.hpp"
#include "task_control_block.hpp"

/**
 * @brief 互斥锁（Mutex）
 *
 * 实现基于任务调度的互斥锁：
 * - 当锁被占用时，请求线程会被阻塞并进入等待队列
 * - 当锁释放时，会唤醒一个等待线程
 * - 支持所有者跟踪和递归检测
 *
 * @note 使用限制：
 * 1. 不可重入：同一线程不能递归获取同一锁
 * 2. 所有权：必须由获取锁的线程释放锁
 * 3. 阻塞语义：获取锁失败时会阻塞当前任务
 * 4. 必须在任务上下文中使用：不能在中断处理程序中使用
 */
class Mutex {
 public:
  /// 锁的名称
  const char* name{"unnamed_mutex"};

  /**
   * @brief 获取锁（阻塞）
   *
   * 如果锁已被其他线程持有，当前线程将被阻塞直到锁可用。
   * 如果当前线程已持有锁，返回错误（不支持递归）。
   *
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] auto Lock() -> Expected<void>;

  /**
   * @brief 释放锁
   *
   * 释放锁并唤醒一个等待线程（如果有）。
   * 只能由持有锁的线程调用。
   *
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] auto UnLock() -> Expected<void>;

  /**
   * @brief 尝试获取锁（非阻塞）
   *
   * 尝试获取锁，如果锁不可用则立即返回。
   *
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] auto TryLock() -> Expected<void>;

  /**
   * @brief 检查锁是否被当前线程持有
   * @return true  当前线程持有锁
   * @return false 当前线程未持有锁
   */
  [[nodiscard]] auto IsLockedByCurrentTask() const -> bool;

  /// @name 构造/析构函数
  /// @{
  explicit Mutex(const char* _name)
      : name(_name),
        resource_id_(ResourceType::kMutex, reinterpret_cast<uint64_t>(this)) {}
  Mutex() = default;

  Mutex(const Mutex&) = delete;
  Mutex(Mutex&&) = delete;
  auto operator=(const Mutex&) -> Mutex& = delete;
  auto operator=(Mutex&&) -> Mutex& = delete;
  ~Mutex() = default;
  /// @}

 protected:
  /// 锁状态
  std::atomic<bool> locked_{false};

  /// 持有锁的任务 ID，max() 表示未被持有
  std::atomic<Pid> owner_{std::numeric_limits<Pid>::max()};

  /// 资源 ID，用于任务阻塞队列
  ResourceId resource_id_{};
};
