/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "spinlock.hpp"

#include <etl/format.h>

#include <atomic>
#include <cstdint>

#include "basic_info.hpp"
#include "cpu_io.h"
#include "system_test.h"

namespace {
// 测试辅助类：暴露 protected 成员用于测试验证
class TestSpinLock : public SpinLock {
 public:
  using SpinLock::IsLockedByCurrentCore;
  using SpinLock::SpinLock;
};

auto test_basic_lock() -> bool {
  klog::Info("Running test_basic_lock...");
  TestSpinLock lock("basic");
  EXPECT_TRUE(lock.Lock(), "Basic lock failed");
  EXPECT_TRUE(lock.IsLockedByCurrentCore(),
              "IsLockedByCurrentCore failed after lock");
  EXPECT_TRUE(lock.UnLock(), "Basic unlock failed");
  EXPECT_TRUE(!lock.IsLockedByCurrentCore(),
              "IsLockedByCurrentCore failed after unlock");
  klog::Info("test_basic_lock passed");
  return true;
}

auto test_recursive_lock() -> bool {
  klog::Info("Running test_recursive_lock...");
  TestSpinLock lock("recursive");
  EXPECT_TRUE(lock.Lock(), "Lock failed in recursive test");
  // Lock() 如果已经被当前核心锁定则返回 false
  if (lock.Lock()) {
    klog::Err("FAIL: Recursive lock should return false");
    (void)lock.UnLock();  // 尝试恢复
    (void)lock.UnLock();
    return false;
  }

  EXPECT_TRUE(lock.UnLock(), "Unlock failed in recursive test");
  // 再次解锁应该失败
  if (lock.UnLock()) {
    klog::Err("FAIL: Double unlock should return false");
    return false;
  }
  klog::Info("test_recursive_lock passed");
  return true;
}

auto test_lock_guard() -> bool {
  klog::Info("Running test_lock_guard...");
  TestSpinLock lock("guard");
  {
    LockGuard<TestSpinLock> guard(lock);
    EXPECT_TRUE(lock.IsLockedByCurrentCore(), "LockGuard failed to lock");
  }
  EXPECT_TRUE(!lock.IsLockedByCurrentCore(), "LockGuard failed to unlock");
  klog::Info("test_lock_guard passed");
  return true;
}

auto test_interrupt_restore() -> bool {
  klog::Info("Running test_interrupt_restore...");
  TestSpinLock lock("intr");

  // Case 1: Interrupts enabled
  cpu_io::EnableInterrupt();
  if (!cpu_io::GetInterruptStatus()) {
    klog::Err("FAIL: Failed to enable interrupts");
    return false;
  }

  (void)lock.Lock();
  if (cpu_io::GetInterruptStatus()) {
    klog::Err("FAIL: Lock didn't disable interrupts");
    (void)lock.UnLock();
    return false;
  }
  (void)lock.UnLock();

  if (!cpu_io::GetInterruptStatus()) {
    klog::Err("FAIL: Unlock didn't restore interrupts (expected enabled)");
    return false;
  }

  // Case 2: Interrupts disabled
  cpu_io::DisableInterrupt();
  // Ensure disabled
  if (cpu_io::GetInterruptStatus()) {
    klog::Err("FAIL: Failed to disable interrupts for test");
    return false;
  }

  (void)lock.Lock();
  if (cpu_io::GetInterruptStatus()) {
    klog::Err("FAIL: Lock enabled interrupts unexpectedly");
    (void)lock.UnLock();
    cpu_io::EnableInterrupt();
    return false;
  }
  (void)lock.UnLock();

  if (cpu_io::GetInterruptStatus()) {
    klog::Err("FAIL: Unlock enabled interrupts (expected disabled)");
    cpu_io::EnableInterrupt();
    return false;
  }

  cpu_io::EnableInterrupt();  // Cleanup
  klog::Info("test_interrupt_restore passed");
  return true;
}

SpinLock smp_lock("smp_lock");
int shared_counter = 0;
std::atomic<int> finished_cores = 0;

auto spinlock_smp_test() -> bool {
  for (int i = 0; i < 10000; ++i) {
    (void)smp_lock.Lock();
    shared_counter++;
    (void)smp_lock.UnLock();
  }

  int finished = finished_cores.fetch_add(1) + 1;
  int total_cores = BasicInfoSingleton::instance().core_count;

  if (finished == total_cores) {
    bool passed = (shared_counter == total_cores * 10000);
    if (passed) {
      klog::Info(" All cores finished. shared_counter = {}. OK.",
                 shared_counter);
    } else {
      klog::Err(" All cores finished. shared_counter = {}. EXPECTED {}. FAIL.",
                shared_counter, total_cores * 10000);
    }
    return passed;
  }

  return true;
}

constexpr int BUFFER_SIZE = 8192;
int shared_buffer[BUFFER_SIZE];
int buffer_index = 0;
SpinLock buffer_lock("buffer_lock");
std::atomic<int> buffer_test_finished_cores = 0;

auto spinlock_smp_buffer_test() -> bool {
  // 每个核心尝试写入一定次数
  int writes_per_core = 500;

  for (int i = 0; i < writes_per_core; ++i) {
    (void)buffer_lock.Lock();
    if (buffer_index < BUFFER_SIZE) {
      // 写入 Core ID
      shared_buffer[buffer_index++] = cpu_io::GetCurrentCoreId();
    }
    (void)buffer_lock.UnLock();
  }

  int finished = buffer_test_finished_cores.fetch_add(1) + 1;
  int total_cores = BasicInfoSingleton::instance().core_count;

  if (finished == total_cores) {
    // 最后一个完成的核心进行检查
    klog::Info("All cores finished buffer writes. Checking buffer...");

    // 检查写入总数
    int expected_writes = writes_per_core * total_cores;
    if (expected_writes > BUFFER_SIZE) expected_writes = BUFFER_SIZE;

    if (buffer_index != expected_writes) {
      klog::Err("FAIL: Buffer index {}, expected {}", buffer_index,
                expected_writes);
      return false;
    }

    klog::Info("Buffer test passed. Final index: {}", buffer_index);
    return true;
  }

  return true;
}

constexpr int STR_BUFFER_SIZE = 512 * 1024;
// 使用更大的缓冲区以容纳所有字符串
char shared_str_buffer[STR_BUFFER_SIZE];
int str_buffer_offset = 0;
// 单独的锁用于此测试
SpinLock str_lock("str_lock");
std::atomic<int> str_test_finished_cores = 0;
std::atomic<int> str_test_start_barrier = 0;

auto spinlock_smp_string_test() -> bool {
  size_t core_id = cpu_io::GetCurrentCoreId();
  size_t core_count = BasicInfoSingleton::instance().core_count;

  if (core_count < 2) {
    if (core_id == 0) {
      klog::Info("Skipping SMP string test: need more than 1 core.");
    }
    return true;
  }

  int arrived = str_test_start_barrier.fetch_add(1) + 1;

  constexpr int kBarrierSpinLimit = 100000000;
  int spins = 0;
  while (str_test_start_barrier.load(std::memory_order_acquire) <
         (int)core_count) {
    cpu_io::Pause();
    if (++spins > kBarrierSpinLimit) {
      if (core_id == 0) {
        klog::Err(
            "SMP string test barrier timeout: {}/{} cores arrived, skipping",
            str_test_start_barrier.load(), (int)core_count);
      }
      return false;
    }
  }

  int writes_per_core = 500;
  char local_buf[128];

  for (int i = 0; i < writes_per_core; ++i) {
    // 2. 先写入可区分的字符串到本地缓冲区
    // 增加数据长度以增加临界区持续时间
    auto* end = etl::format_to_n(local_buf, sizeof(local_buf) - 1,
                                 "[C:{}-{}|LongStringPaddingForContention]",
                                 (int)core_id, i);
    *end = '\0';
    int len = static_cast<int>(end - local_buf);

    (void)str_lock.Lock();
    if (str_buffer_offset + len < STR_BUFFER_SIZE - 1) {
      for (int k = 0; k < len; ++k) {
        shared_str_buffer[str_buffer_offset + k] = local_buf[k];
      }
      str_buffer_offset += len;
      shared_str_buffer[str_buffer_offset] = '\0';
    }
    (void)str_lock.UnLock();
  }

  int finished = str_test_finished_cores.fetch_add(1) + 1;
  if (finished == (int)core_count) {
    // 3. 验证
    klog::Info(
        "All cores finished string writes. Verifying string integrity...");
    bool failed = false;
    int current_idx = 0;
    int tokens_found = 0;

    while (current_idx < str_buffer_offset) {
      if (shared_str_buffer[current_idx] != '[') {
        failed = true;
        klog::Err("FAIL: Expected '[' at {}, got '{}'", current_idx,
                  shared_str_buffer[current_idx]);
        break;
      }

      // 应该找到匹配的 ']'
      int end_idx = current_idx + 1;
      bool closed = false;
      while (end_idx < str_buffer_offset) {
        if (shared_str_buffer[end_idx] == ']') {
          closed = true;
          break;
        }
        if (shared_str_buffer[end_idx] == '[') {
          // 在结束前遇到另一个开始 -> 数据损坏/交错
          break;
        }
        end_idx++;
      }

      if (!closed) {
        failed = true;
        klog::Err("FAIL: Broken token starting at {}", current_idx);
        break;
      }

      // 验证内容格式 C:ID-Seq
      if (shared_str_buffer[current_idx + 1] != 'C' ||
          shared_str_buffer[current_idx + 2] != ':') {
        failed = true;
        klog::Err("FAIL: Invalid content in token at {}", current_idx);
        break;
      }

      // 验证填充完整性
      const char* padding = "|LongStringPaddingForContention";
      int padding_len = 31;  // "|LongStringPaddingForContention" 的长度
      int token_content_len = end_idx - current_idx - 1;
      bool padding_ok = true;

      if (token_content_len < padding_len) {
        padding_ok = false;
      } else {
        int padding_start = end_idx - padding_len;
        for (int p = 0; p < padding_len; ++p) {
          if (shared_str_buffer[padding_start + p] != padding[p]) {
            padding_ok = false;
            break;
          }
        }
      }

      if (!padding_ok) {
        failed = true;
        klog::Err("FAIL: Broken padding in token at {}. Content len: {}",
                  current_idx, token_content_len);
        break;
      }

      tokens_found++;
      current_idx = end_idx + 1;
    }

    int expected_tokens = writes_per_core * core_count;
    // 如果缓冲区太小可能会耗尽，但通常我们应该期望完全匹配。

    if (tokens_found != expected_tokens) {
      failed = true;
      klog::Err("FAIL: Expected {} tokens, found {}", expected_tokens,
                tokens_found);
    }

    if (!failed) {
      klog::Info("String test passed. Length: {}, Tokens: {}",
                 str_buffer_offset, tokens_found);
    }
    return !failed;
  }
  return true;
}

}  // namespace

auto spinlock_test() -> bool {
  bool ret = true;
  size_t core_id = cpu_io::GetCurrentCoreId();

  // 单元测试仅在核心 0 上运行，以避免日志混乱
  if (core_id == 0) {
    klog::Info("Starting spinlock_test");
    ret = ret && test_basic_lock();
    ret = ret && test_recursive_lock();
    ret = ret && test_lock_guard();
    ret = ret && test_interrupt_restore();
  }

  // SMP (多核) 测试在所有核心上运行
  // 使用顺序执行以确保如果前一个测试失败，屏障不会死锁
  if (!spinlock_smp_test()) ret = false;
  if (!spinlock_smp_buffer_test()) ret = false;
  if (!spinlock_smp_string_test()) ret = false;

  if (core_id == 0) {
    if (ret) {
      klog::Info("spinlock_test passed");
    } else {
      klog::Err("spinlock_test failed");
    }
  }
  return ret;
}
