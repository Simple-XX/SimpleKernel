/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>

#include <atomic>
#include <concepts>
#include <cstddef>
#include <limits>

#include "expected.hpp"
#include "kernel_log.hpp"
#include "kstd_cstdio"

/**
 * @brief 自旋锁
 * @note 使用限制：
 * 1. 不可重入：不支持同一核心递归获取锁，会导致返回失败
 * 2. 关中断：获取锁时会自动关闭中断，释放锁时恢复之前的状态
 * 3. 必须配对：必须在获取锁的同一个核心释放锁
 * 4. 不可休眠：持有自旋锁期间不可执行休眠或调度操作
 * 5. 副作用：修改当前 CPU 的中断状态
 */
class SpinLock {
 public:
  /// 自旋锁名称
  const char* name{"unnamed"};

  /**
   * @brief 获得锁
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] __always_inline auto Lock() -> Expected<void> {
    auto intr_enable = cpu_io::GetInterruptStatus();
    cpu_io::DisableInterrupt();

    // 先尝试获取锁
    while (locked_.test_and_set(std::memory_order_acquire)) {
      // 在等待时检查是否是当前核心持有锁（递归锁检测）
      if (core_id_.load(std::memory_order_acquire) ==
          cpu_io::GetCurrentCoreId()) {
        // 递归锁定，恢复中断状态并返回失败
        if (intr_enable) {
          cpu_io::EnableInterrupt();
        }
        // klog::Err("spinlock {}: {} recursive lock detected.",
        //           cpu_io::GetCurrentCoreId(), name);
        return std::unexpected(Error{ErrorCode::kSpinLockRecursiveLock});
      }
      cpu_io::Pause();
    }

    // 获取锁成功后立即设置 core_id_
    core_id_.store(cpu_io::GetCurrentCoreId(), std::memory_order_release);
    saved_intr_enable_ = intr_enable;
    return {};
  }

  /**
   * @brief 释放锁
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] __always_inline auto UnLock() -> Expected<void> {
    if (!IsLockedByCurrentCore()) {
      // klog::Err("spinlock {}: {} unlock by non-owner detected.",
      //           cpu_io::GetCurrentCoreId(), name);
      return std::unexpected(Error{ErrorCode::kSpinLockNotOwned});
    }

    // 先重置 core_id_，再释放锁
    core_id_.store(std::numeric_limits<size_t>::max(),
                   std::memory_order_release);
    locked_.clear(std::memory_order_release);

    if (saved_intr_enable_) {
      cpu_io::EnableInterrupt();
    }
    return {};
  }

  /// @name 构造/析构函数
  /// @{

  /**
   * @brief 构造函数
   * @param  _name            锁名
   * @note 需要堆初始化后可用
   */
  explicit SpinLock(const char* _name) : name(_name) {}

  SpinLock() = default;
  SpinLock(const SpinLock&) = delete;
  SpinLock(SpinLock&&) = default;
  auto operator=(const SpinLock&) -> SpinLock& = delete;
  auto operator=(SpinLock&&) -> SpinLock& = default;
  ~SpinLock() = default;
  /// @}

 protected:
  /// 是否 Lock
  std::atomic_flag locked_{ATOMIC_FLAG_INIT};
  /// 获得此锁的 core_id
  std::atomic<size_t> core_id_{std::numeric_limits<size_t>::max()};
  /// 保存的中断状态
  bool saved_intr_enable_{false};

  /**
   * @brief 检查当前 core 是否获得此锁
   * @return true             是
   * @return false            否
   */
  __always_inline auto IsLockedByCurrentCore() -> bool {
    return locked_.test(std::memory_order_acquire) &&
           (core_id_.load(std::memory_order_acquire) ==
            cpu_io::GetCurrentCoreId());
  }
};

/**
 * @brief RAII 风格的锁守卫模板类
 * @tparam Mutex 锁类型，必须有返回 Expected<void> 的 Lock() 和 UnLock() 方法
 */
template <typename Mutex>
  requires requires(Mutex& m) {
    { m.Lock() } -> std::same_as<Expected<void>>;
    { m.UnLock() } -> std::same_as<Expected<void>>;
  }
class LockGuard {
 public:
  using mutex_type = Mutex;

  /// @name 构造/析构函数
  /// @{

  /**
   * @brief 构造函数，自动获取锁
   * @param mutex 要保护的锁对象
   */
  explicit LockGuard(mutex_type& mutex) : mutex_(mutex) {
    mutex_.Lock().or_else([&](auto&& err) {
      char core_buf[4] = {};
      auto core_id = cpu_io::GetCurrentCoreId();
      size_t pos = 0;
      if (core_id == 0) {
        core_buf[pos++] = '0';
      } else {
        char tmp[4] = {};
        size_t tmp_pos = 0;
        while (core_id > 0 && tmp_pos < sizeof(tmp)) {
          tmp[tmp_pos++] = static_cast<char>('0' + (core_id % 10));
          core_id /= 10;
        }
        while (tmp_pos > 0) {
          core_buf[pos++] = tmp[--tmp_pos];
        }
      }
      core_buf[pos] = '\0';
      klog::RawPut("PANIC: LockGuard failed to acquire lock '");
      klog::RawPut(mutex_.name);
      klog::RawPut("' on core ");
      klog::RawPut(core_buf);
      klog::RawPut(": ");
      klog::RawPut(err.message());
      klog::RawPut("\n");
      while (true) {
        cpu_io::Pause();
      }
      return Expected<void>{};
    });
  }

  /**
   * @brief 析构函数，自动释放锁
   */
  ~LockGuard() {
    mutex_.UnLock().or_else([&](auto&& err) {
      char core_buf[4] = {};
      auto core_id = cpu_io::GetCurrentCoreId();
      size_t pos = 0;
      if (core_id == 0) {
        core_buf[pos++] = '0';
      } else {
        char tmp[4] = {};
        size_t tmp_pos = 0;
        while (core_id > 0 && tmp_pos < sizeof(tmp)) {
          tmp[tmp_pos++] = static_cast<char>('0' + (core_id % 10));
          core_id /= 10;
        }
        while (tmp_pos > 0) {
          core_buf[pos++] = tmp[--tmp_pos];
        }
      }
      core_buf[pos] = '\0';
      klog::RawPut("PANIC: LockGuard failed to release lock '");
      klog::RawPut(mutex_.name);
      klog::RawPut("' on core ");
      klog::RawPut(core_buf);
      klog::RawPut(": ");
      klog::RawPut(err.message());
      klog::RawPut("\n");
      while (true) {
        cpu_io::Pause();
      }
      return Expected<void>{};
    });
  }

  LockGuard() = delete;
  LockGuard(const LockGuard&) = delete;
  LockGuard(LockGuard&&) = delete;
  auto operator=(const LockGuard&) -> LockGuard& = delete;
  auto operator=(LockGuard&&) -> LockGuard& = delete;
  /// @}

 private:
  mutex_type& mutex_;
};
