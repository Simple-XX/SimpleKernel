/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>

#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "arch.h"
#include "expected.hpp"
#include "kernel_log.hpp"
#include "kstd_cstdio"

/// 锁级别常量 — 数值越小越先获取，相同级别禁止嵌套
namespace lock_level {
inline constexpr uint8_t kSchedLock = 0;
inline constexpr uint8_t kTaskTableLock = 1;
inline constexpr uint8_t kInterruptThreadsLock = 2;
inline constexpr uint8_t kUnclassified = 0xFF;
}  // namespace lock_level

/**
 * @brief 自旋锁（纯原子操作，不管理中断状态）
 */
class SpinLock {
 public:
  const char* name{"unnamed"};

  /**
   * @brief 获得锁
   * @pre 调用方已经关闭中断
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] __always_inline auto Lock() -> Expected<void> {
    CheckLockOrder(lock_level_, name);
    while (locked_.test_and_set(std::memory_order_acquire)) {
      if (core_id_.load(std::memory_order_acquire) ==
          cpu_io::GetCurrentCoreId()) {
        PopLockOrder(lock_level_);
        return std::unexpected(Error{ErrorCode::kSpinLockRecursiveLock});
      }
      cpu_io::Pause();
    }

    core_id_.store(cpu_io::GetCurrentCoreId(), std::memory_order_release);
    return {};
  }

  /**
   * @brief 释放锁
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] __always_inline auto UnLock() -> Expected<void> {
    if (!IsLockedByCurrentCore()) {
      return std::unexpected(Error{ErrorCode::kSpinLockNotOwned});
    }

    PopLockOrder(lock_level_);
    core_id_.store(std::numeric_limits<size_t>::max(),
                   std::memory_order_release);
    locked_.clear(std::memory_order_release);
    return {};
  }

  /// @name 构造/析构函数
  /// @{
  explicit SpinLock(const char* _name) : name(_name) {}

  SpinLock(const char* _name, uint8_t level)
      : name(_name), lock_level_(level) {}

  SpinLock() = default;
  SpinLock(const SpinLock&) = delete;
  SpinLock(SpinLock&&) = default;
  auto operator=(const SpinLock&) -> SpinLock& = delete;
  auto operator=(SpinLock&&) -> SpinLock& = default;
  ~SpinLock() = default;
  /// @}

 protected:
  std::atomic_flag locked_{ATOMIC_FLAG_INIT};
  std::atomic<size_t> core_id_{std::numeric_limits<size_t>::max()};
  uint8_t lock_level_{lock_level::kUnclassified};

  struct PerCpuLockStack {
    static constexpr size_t kMaxDepth = 4;
    uint8_t levels[kMaxDepth]{};
    const char* names[kMaxDepth]{};
    size_t depth{0};
  };

  static inline std::array<PerCpuLockStack, SIMPLEKERNEL_MAX_CORE_COUNT>
      lock_stacks_{};

  static __always_inline void CheckLockOrder(uint8_t new_level,
                                             const char* new_name) {
    if (new_level == lock_level::kUnclassified) {
      return;
    }
    auto& stack = lock_stacks_[cpu_io::GetCurrentCoreId()];
    if (stack.depth > 0) {
      uint8_t top_level = stack.levels[stack.depth - 1];
      if (top_level != lock_level::kUnclassified && new_level <= top_level) {
        klog::RawPut("LOCK ORDER VIOLATION: acquiring '");
        klog::RawPut(new_name);
        klog::RawPut("' while holding '");
        klog::RawPut(stack.names[stack.depth - 1]);
        klog::RawPut("'\n");
        RawDumpStack();
        while (true) {
          cpu_io::Pause();
        }
      }
    }
    if (stack.depth < PerCpuLockStack::kMaxDepth) {
      stack.levels[stack.depth] = new_level;
      stack.names[stack.depth] = new_name;
      stack.depth++;
    }
  }

  static __always_inline void PopLockOrder(uint8_t level) {
    if (level == lock_level::kUnclassified) {
      return;
    }
    auto& stack = lock_stacks_[cpu_io::GetCurrentCoreId()];
    if (stack.depth > 0 && stack.levels[stack.depth - 1] == level) {
      stack.depth--;
    }
  }

  __always_inline auto IsLockedByCurrentCore() -> bool {
    return locked_.test(std::memory_order_acquire) &&
           (core_id_.load(std::memory_order_acquire) ==
            cpu_io::GetCurrentCoreId());
  }
};

/**
 * @brief RAII 锁守卫，中断状态保存在本对象中
 */
template <typename T>
  requires std::derived_from<T, SpinLock>
class LockGuard {
 public:
  using mutex_type = T;

  /// @name 构造/析构函数
  /// @{
  explicit LockGuard(mutex_type& mutex) : mutex_(mutex) {
    saved_intr_ = cpu_io::GetInterruptStatus();
    cpu_io::DisableInterrupt();

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

      RawDumpStack();

      while (true) {
        cpu_io::Pause();
      }
      return Expected<void>{};
    });
  }

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

      RawDumpStack();

      while (true) {
        cpu_io::Pause();
      }
      return Expected<void>{};
    });

    if (saved_intr_) {
      cpu_io::EnableInterrupt();
    }
  }

  LockGuard() = delete;
  LockGuard(const LockGuard&) = delete;
  LockGuard(LockGuard&&) = delete;
  auto operator=(const LockGuard&) -> LockGuard& = delete;
  auto operator=(LockGuard&&) -> LockGuard& = delete;
  /// @}

 private:
  mutex_type& mutex_;
  bool saved_intr_;
};
