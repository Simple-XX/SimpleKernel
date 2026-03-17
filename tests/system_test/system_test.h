/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/vector.h>

#include <cstdint>
#include <type_traits>

#include "kernel_log.hpp"

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
auto thread_group_system_test() -> bool;
auto wait_system_test() -> bool;
auto clone_system_test() -> bool;
auto exit_system_test() -> bool;
auto ramfs_system_test() -> bool;
auto fatfs_system_test() -> bool;
auto memory_test() -> bool;
auto kernel_task_test() -> bool;
auto user_task_test() -> bool;
auto mutex_test() -> bool;

// ===========================================================================
// QEMU exit
// ===========================================================================

/// 测试结束后退出 QEMU，success=true 表示全部通过
inline void QemuExit([[maybe_unused]] bool success) {
#if defined(__riscv)
  // sifive_test device (virt machine 默认存在，地址 0x100000)
  // 0x5555 = FINISHER_PASS, 0x3333 = FINISHER_FAIL
  volatile auto* finisher = reinterpret_cast<volatile uint32_t*>(0x100000);
  *finisher = success ? 0x5555 : 0x3333;
#elif defined(__x86_64__)
  // isa-debug-exit device (需要 QEMU 参数:
  //   -device isa-debug-exit,iobase=0xf4,iosize=0x04)
  // QEMU 退出码 = (val << 1) | 1，所以 0 -> 1(pass), 1 -> 3(fail)
  uint32_t code = success ? 0 : 1;
  asm volatile("outl %0, %1" ::"a"(code), "Nd"(static_cast<uint16_t>(0xf4)));
#elif defined(__aarch64__)
  // PSCI SYSTEM_OFF (function id = 0x84000008)
  // 需要 EL2/EL3 支持（ATF 已提供）
  register uint64_t x0 asm("x0") = 0x84000008;
  asm volatile("hvc #0" : "+r"(x0));
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
