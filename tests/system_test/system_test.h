/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>
#include <etl/vector.h>

#include <cstdint>
#include <type_traits>

#include "kernel.h"
#include "kernel_log.hpp"
#include "virtual_memory.hpp"

// ===========================================================================
// Assertion helpers
// ===========================================================================

template <typename T1, typename T2>
bool expect_eq_helper(const T1& val1, const T2& val2, const char* msg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
  if (val1 != val2) {
#pragma GCC diagnostic pop
    if constexpr (std::is_convertible_v<T1, uint64_t> &&
                  std::is_convertible_v<T2, uint64_t>) {
      klog::Err("FAIL: {}. Expected {}, got {}", msg,
                static_cast<uint64_t>(val2), static_cast<uint64_t>(val1));
    } else {
      klog::Err("FAIL: {}", msg);
    }
    return false;
  }
  return true;
}

template <typename T1, typename T2>
bool expect_ne_helper(const T1& val1, const T2& val2, const char* msg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
  if (val1 == val2) {
#pragma GCC diagnostic pop
    if constexpr (std::is_convertible_v<T1, uint64_t> &&
                  std::is_convertible_v<T2, uint64_t>) {
      klog::Err("FAIL: {}. Expected not {}, got {}", msg,
                static_cast<uint64_t>(val2), static_cast<uint64_t>(val1));
    } else {
      klog::Err("FAIL: {}", msg);
    }
    return false;
  }
  return true;
}

template <typename T1, typename T2>
bool expect_gt_helper(const T1& val1, const T2& val2, const char* msg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
  if (!(val1 > val2)) {
#pragma GCC diagnostic pop
    if constexpr (std::is_convertible_v<T1, uint64_t> &&
                  std::is_convertible_v<T2, uint64_t>) {
      klog::Err("FAIL: {}. Expected {} > {}", msg, static_cast<uint64_t>(val1),
                static_cast<uint64_t>(val2));
    } else {
      klog::Err("FAIL: {}", msg);
    }
    return false;
  }
  return true;
}

template <typename T1, typename T2>
bool expect_lt_helper(const T1& val1, const T2& val2, const char* msg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
  if (!(val1 < val2)) {
#pragma GCC diagnostic pop
    if constexpr (std::is_convertible_v<T1, uint64_t> &&
                  std::is_convertible_v<T2, uint64_t>) {
      klog::Err("FAIL: {}. Expected {} < {}", msg, static_cast<uint64_t>(val1),
                static_cast<uint64_t>(val2));
    } else {
      klog::Err("FAIL: {}", msg);
    }
    return false;
  }
  return true;
}

template <typename T1, typename T2>
bool expect_ge_helper(const T1& val1, const T2& val2, const char* msg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
  if (!(val1 >= val2)) {
#pragma GCC diagnostic pop
    if constexpr (std::is_convertible_v<T1, uint64_t> &&
                  std::is_convertible_v<T2, uint64_t>) {
      klog::Err("FAIL: {}. Expected {} >= {}", msg, static_cast<uint64_t>(val1),
                static_cast<uint64_t>(val2));
    } else {
      klog::Err("FAIL: {}", msg);
    }
    return false;
  }
  return true;
}

template <typename T1, typename T2>
bool expect_le_helper(const T1& val1, const T2& val2, const char* msg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
  if (!(val1 <= val2)) {
#pragma GCC diagnostic pop
    if constexpr (std::is_convertible_v<T1, uint64_t> &&
                  std::is_convertible_v<T2, uint64_t>) {
      klog::Err("FAIL: {}. Expected {} <= {}", msg, static_cast<uint64_t>(val1),
                static_cast<uint64_t>(val2));
    } else {
      klog::Err("FAIL: {}", msg);
    }
    return false;
  }
  return true;
}

#define EXPECT_EQ(val1, val2, msg)          \
  if (!expect_eq_helper(val1, val2, msg)) { \
    return false;                           \
  }

#define EXPECT_NE(val1, val2, msg)          \
  if (!expect_ne_helper(val1, val2, msg)) { \
    return false;                           \
  }

#define EXPECT_GT(val1, val2, msg)          \
  if (!expect_gt_helper(val1, val2, msg)) { \
    return false;                           \
  }

#define EXPECT_LT(val1, val2, msg)          \
  if (!expect_lt_helper(val1, val2, msg)) { \
    return false;                           \
  }

#define EXPECT_GE(val1, val2, msg)          \
  if (!expect_ge_helper(val1, val2, msg)) { \
    return false;                           \
  }

#define EXPECT_LE(val1, val2, msg)          \
  if (!expect_le_helper(val1, val2, msg)) { \
    return false;                           \
  }

#define EXPECT_TRUE(cond, msg)  \
  if (!(cond)) {                \
    klog::Err("FAIL: {}", msg); \
    return false;               \
  }

#define EXPECT_FALSE(cond, msg) \
  if (cond) {                   \
    klog::Err("FAIL: {}", msg); \
    return false;               \
  }

// ===========================================================================
// Test declarations
// ===========================================================================

auto ctor_dtor_test() -> bool;
auto spinlock_test() -> bool;
auto virtual_memory_test() -> bool;
auto interrupt_test() -> bool;
auto fifo_scheduler_test() -> bool;
auto rr_scheduler_test() -> bool;
auto cfs_scheduler_test() -> bool;
auto idle_scheduler_test() -> bool;
auto thread_group_test() -> bool;
auto wait_test() -> bool;
auto clone_test() -> bool;
auto exit_test() -> bool;
auto ramfs_test() -> bool;
auto fatfs_test() -> bool;
auto memory_test() -> bool;
auto kernel_task_test() -> bool;
auto user_task_test() -> bool;
auto mutex_test() -> bool;
auto cross_core_test() -> bool;
auto yield_test() -> bool;
auto fork_test() -> bool;
auto signal_test() -> bool;
auto affinity_test() -> bool;
auto tick_test() -> bool;
auto zombie_reap_test() -> bool;
auto stress_test() -> bool;

// ===========================================================================
// QEMU exit
// ===========================================================================

/// 测试结束后退出 QEMU，success=true 表示全部通过
inline void QemuExit([[maybe_unused]] bool success) {
#if defined(__riscv)
  // sifive_test device (virt machine 默认存在，地址 0x100000)
  // 0x5555 = FINISHER_PASS, 0x3333 = FINISHER_FAIL
  constexpr uint64_t kSifiveTestAddr = 0x100000;
  constexpr size_t kSifiveTestSize = 0x1000;
  (void)VirtualMemorySingleton::instance().MapMMIO(kSifiveTestAddr,
                                                   kSifiveTestSize);
  volatile auto* finisher =
      reinterpret_cast<volatile uint32_t*>(kSifiveTestAddr);
  *finisher = success ? 0x5555 : 0x3333;
#elif defined(__aarch64__)
  // PSCI SYSTEM_OFF，通过 SMC 调用 ATF (EL3)
  cpu_io::SecureMonitorCall(cpu_io::psci::kSYSTEM_OFF, 0, 0, 0, 0, 0, 0, 0);
#endif
  __builtin_unreachable();
}

// ===========================================================================
// Test thread result tracking
// ===========================================================================

/// 测试线程运行状态
enum class TestThreadStatus : uint8_t {
  kPending,  ///< 等待启动
  kRunning,  ///< 运行中
  kPassed,   ///< 测试通过
  kFailed,   ///< 测试失败
  kTimeout,  ///< 超时未完成
};

/// 单个测试线程的结果记录
struct TestResult {
  const char* name = nullptr;
  int64_t pid = 0;
  TestThreadStatus status = TestThreadStatus::kPending;
  int exit_code = -1;
};
