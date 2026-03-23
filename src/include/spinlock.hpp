/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>

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
#include "per_cpu.hpp"

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
    // 先 spin 获取锁——保证只有一个执行流通过此点。
    // 重入在 spin 中通过 owner_core_ 检测（原子操作，多核安全）。
    while (locked_.test_and_set(std::memory_order_acquire)) {
      if (owner_core_.load(std::memory_order_acquire) ==
          cpu_io::GetCurrentCoreId()) {
        return std::unexpected(Error{ErrorCode::kSpinLockRecursiveLock});
      }
      cpu_io::Pause();
    }
    owner_core_.store(cpu_io::GetCurrentCoreId(), std::memory_order_release);

    // 获取锁后操作 per-CPU lock_stack（此时中断已关，同核心无并发）
    auto& stack = per_cpu::GetCurrentCore().lock_stack;
    if (lock_level_ != lock_level::kUnclassified && stack.depth > 0) {
      uint8_t top_level = stack.entries[stack.depth - 1].level;
      if (top_level != lock_level::kUnclassified && lock_level_ < top_level) {
        klog::RawPut("LOCK ORDER VIOLATION: acquiring '");
        klog::RawPut(name);
        klog::RawPut("' while holding '");
        klog::RawPut(stack.entries[stack.depth - 1].lock->name);
        klog::RawPut("'\n");
        RawDumpStack();
        while (true) {
          cpu_io::Pause();
        }
      }
    }
    if (stack.depth < per_cpu::PerCpu::LockStack::kMaxDepth) {
      stack.entries[stack.depth] = {this, lock_level_};
      stack.depth++;
    }

    return {};
  }

  /**
   * @brief 释放锁
   * @return Expected<void> 成功返回空值，失败返回错误
   */
  [[nodiscard]] __always_inline auto UnLock() -> Expected<void> {
    if (owner_core_.load(std::memory_order_acquire) !=
        cpu_io::GetCurrentCoreId()) {
      return std::unexpected(Error{ErrorCode::kSpinLockNotOwned});
    }

    // 先弹栈（仍持有锁，per-CPU 数据安全），再释放锁
    auto& stack = per_cpu::GetCurrentCore().lock_stack;
    if (stack.depth > 0 && stack.entries[stack.depth - 1].lock == this) {
      stack.depth--;
    }

    owner_core_.store(std::numeric_limits<size_t>::max(),
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
  std::atomic<size_t> owner_core_{std::numeric_limits<size_t>::max()};
  uint8_t lock_level_{lock_level::kUnclassified};
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
