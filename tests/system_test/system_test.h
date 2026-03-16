/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#ifndef SIMPLEKERNEL_TESTS_SYSTEM_TEST_SYSTEM_TEST_H_
#define SIMPLEKERNEL_TESTS_SYSTEM_TEST_SYSTEM_TEST_H_

#include <type_traits>

#include "kernel_log.hpp"

template <typename T1, typename T2>
bool expect_eq_helper(const T1& val1, const T2& val2, const char* msg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
  if (val1 != val2) {
#pragma GCC diagnostic pop
    if constexpr (std::is_convertible_v<T1, long> &&
                  std::is_convertible_v<T2, long>) {
      klog::Err("FAIL: {}. Expected {}, got {}", msg, (long)(val2),
                (long)(val1));
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
    if constexpr (std::is_convertible_v<T1, long> &&
                  std::is_convertible_v<T2, long>) {
      klog::Err("FAIL: {}. Expected not {}, got {}", msg, (long)(val2),
                (long)(val1));
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
    if constexpr (std::is_convertible_v<T1, long> &&
                  std::is_convertible_v<T2, long>) {
      klog::Err("FAIL: {}. Expected {} > {}", msg, (long)(val1), (long)(val2));
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
    if constexpr (std::is_convertible_v<T1, long> &&
                  std::is_convertible_v<T2, long>) {
      klog::Err("FAIL: {}. Expected {} < {}", msg, (long)(val1), (long)(val2));
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
    if constexpr (std::is_convertible_v<T1, long> &&
                  std::is_convertible_v<T2, long>) {
      klog::Err("FAIL: {}. Expected {} >= {}", msg, (long)(val1), (long)(val2));
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

#endif /* SIMPLEKERNEL_TESTS_SYSTEM_TEST_SYSTEM_TEST_H_ */
