/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>
#include <etl/format.h>

#include <MPMCQueue.hpp>
#include <atomic>
#include <cstdint>

#include "kstd_cstdio"

namespace klog {
namespace detail {

/// @name ANSI 转义码
/// @{
inline constexpr auto kReset = "\033[0m";
inline constexpr auto kRed = "\033[31m";
inline constexpr auto kGreen = "\033[32m";
inline constexpr auto kYellow = "\033[33m";
inline constexpr auto kCyan = "\033[36m";
/// @}

/// 日志级别
enum class Level : uint8_t {
  kDebug = 0,
  kInfo = 1,
  kWarn = 2,
  kErr = 3,
};

/// 编译期最低日志级别
inline constexpr auto kMinLevel =
    static_cast<Level>(SIMPLEKERNEL_MIN_LOG_LEVEL);

/// 日志级别标签（定宽，对齐输出）
inline constexpr const char* kLevelLabel[] = {
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
};

/// 日志级别对应的颜色（按 Level 枚举索引）
inline constexpr const char* kLevelColor[] = {
    // 调试
    kGreen,
    // 信息
    kCyan,
    // 警告
    kYellow,
    // 错误
    kRed,
};

/// 存储于 MPMC 队列中的日志条目
struct LogEntry {
  uint64_t seq{0};
  uint64_t core_id{0};
  Level level{Level::kDebug};
  char msg[239]{};
};
static_assert(sizeof(LogEntry) == 256, "LogEntry must be 256 bytes");

/// 全局日志队列（64 KB 静态内存，constexpr 构造）
inline mpmc_queue::MPMCQueue<LogEntry, 256> log_queue;

/// 用于跨核心排序的单调递增序列计数器
inline std::atomic<uint64_t> log_seq{0};

/// 单消费者排空保护（非阻塞 try-lock）
inline std::atomic_flag drain_flag = ATOMIC_FLAG_INIT;

/// 队列满时丢弃的条目计数
inline std::atomic<uint64_t> dropped_count{0};

/// 通过 etl_putchar 输出字符串（空指针安全）
__always_inline auto PutStr(const char* s) -> void {
  if (!s) {
    s = "(null)";
  }
  while (*s) {
    etl_putchar(static_cast<unsigned char>(*s));
    ++s;
  }
}

/**
 * @brief 将队列中所有条目输出至串口
 *
 * 使用 atomic_flag 实现非阻塞 try-lock，同一时刻仅一个核心执行排空，
 * 其他核心直接返回，等待下次调用时再排空。
 */
inline auto TryDrain() -> void {
  // 非阻塞 try-lock：若其他核心正在排空则立即返回
  if (drain_flag.test_and_set(std::memory_order_acquire)) {
    return;
  }

  // 若有丢弃条目则上报
  auto dropped = dropped_count.exchange(0, std::memory_order_relaxed);
  if (dropped > 0) {
    char drop_buf[64];
    auto* end = etl::format_to_n(drop_buf, sizeof(drop_buf) - 1,
                                 "\033[31m[LOG] dropped {} entries\033[0m\n",
                                 static_cast<uint64_t>(dropped));
    *end = '\0';
    PutStr(drop_buf);
  }

  // 排空循环
  auto printer_core = cpu_io::GetCurrentCoreId();
  LogEntry entry{};
  while (log_queue.pop(entry)) {
    // 格式: [id][core_id1 core_id2 LEVEL] msg
    char hdr_buf[48];
    auto* hdr_end =
        etl::format_to_n(hdr_buf, sizeof(hdr_buf) - 1, "[{}][{} {} {}]",
                         entry.seq, entry.core_id, printer_core,
                         kLevelLabel[static_cast<uint8_t>(entry.level)]);
    *hdr_end = '\0';
    PutStr(kLevelColor[static_cast<uint8_t>(entry.level)]);
    PutStr(hdr_buf);
    PutStr(entry.msg);
    PutStr(kReset);
    PutStr("\n");
  }

  drain_flag.clear(std::memory_order_release);
}

/**
 * @brief 核心实现：格式化消息并入队，随后尝试排空
 *
 * @tparam Lvl 编译期日志级别，低于 kMinLevel 时整个函数被编译器消除
 * @tparam Args 可变格式化参数类型
 */
template <Level Lvl, typename... Args>
__always_inline auto Log(etl::format_string<Args...> fmt, Args&&... args)
    -> void {
  if constexpr (Lvl < kMinLevel) {
    return;
  }

  LogEntry entry{};
  entry.seq = log_seq.fetch_add(1, std::memory_order_relaxed);
  entry.core_id = cpu_io::GetCurrentCoreId();
  entry.level = Lvl;
  auto* end = etl::format_to_n(entry.msg, sizeof(entry.msg) - 1, fmt,
                               static_cast<Args&&>(args)...);
  *end = '\0';

  if (!log_queue.push(entry)) {
    // 队列满：尝试排空后重试一次
    TryDrain();
    if (!log_queue.push(entry)) {
      dropped_count.fetch_add(1, std::memory_order_relaxed);
      return;
    }
  }

  TryDrain();
}

}  // namespace detail

/// 以 DEBUG 级别记录日志（SIMPLEKERNEL_MIN_LOG_LEVEL > 0 时编译期消除）
template <typename... Args>
inline auto Debug(etl::format_string<Args...> fmt, Args&&... args) -> void {
  if constexpr (detail::Level::kDebug < detail::kMinLevel) {
    return;
  }
  detail::Log<detail::Level::kDebug>(fmt, static_cast<Args&&>(args)...);
}

/// 以 INFO 级别记录日志
template <typename... Args>
inline auto Info(etl::format_string<Args...> fmt, Args&&... args) -> void {
  if constexpr (detail::Level::kInfo < detail::kMinLevel) {
    return;
  }
  detail::Log<detail::Level::kInfo>(fmt, static_cast<Args&&>(args)...);
}

/// 以 WARN 级别记录日志
template <typename... Args>
inline auto Warn(etl::format_string<Args...> fmt, Args&&... args) -> void {
  if constexpr (detail::Level::kWarn < detail::kMinLevel) {
    return;
  }
  detail::Log<detail::Level::kWarn>(fmt, static_cast<Args&&>(args)...);
}

/// 以 ERROR 级别记录日志
template <typename... Args>
inline auto Err(etl::format_string<Args...> fmt, Args&&... args) -> void {
  if constexpr (detail::Level::kErr < detail::kMinLevel) {
    return;
  }
  detail::Log<detail::Level::kErr>(fmt, static_cast<Args&&>(args)...);
}

/// 强制将队列中所有日志条目输出至串口
__always_inline auto Flush() -> void { detail::TryDrain(); }

/// 绕过队列直接输出至串口（用于 panic 路径）
__always_inline auto RawPut(const char* msg) -> void { detail::PutStr(msg); }

}  // namespace klog
