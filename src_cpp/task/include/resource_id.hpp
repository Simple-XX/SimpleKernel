/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/hash.h>

#include <cstdint>
#include <functional>

/**
 * @brief 资源类型枚举
 */
enum class ResourceType : uint8_t {
  /// 无效资源
  kNone = 0x00,
  /// 互斥锁
  kMutex = 0x01,
  /// 信号量
  kSemaphore = 0x02,
  /// 条件变量
  kCondVar = 0x03,
  /// 等待子进程退出
  kChildExit = 0x04,
  /// IO 完成
  kIoComplete = 0x05,
  /// Futex (快速用户空间互斥锁)
  kFutex = 0x06,
  /// 信号
  kSignal = 0x07,
  /// 定时器
  kTimer = 0x08,
  /// 中断（用于中断线程化）
  kInterrupt = 0x09,
  // 可以继续扩展...
  kResourceTypeCount,
};

/**
 * @brief 获取资源类型的字符串表示（用于调试）
 * @param type 资源类型
 * @return 类型名称字符串
 */
[[nodiscard]] constexpr auto GetResourceTypeName(ResourceType type) -> const
    char* {
  switch (type) {
    case ResourceType::kNone:
      return "None";
    case ResourceType::kMutex:
      return "Mutex";
    case ResourceType::kSemaphore:
      return "Semaphore";
    case ResourceType::kCondVar:
      return "CondVar";
    case ResourceType::kChildExit:
      return "ChildExit";
    case ResourceType::kIoComplete:
      return "IoComplete";
    case ResourceType::kFutex:
      return "Futex";
    case ResourceType::kSignal:
      return "Signal";
    case ResourceType::kTimer:
      return "Timer";
    default:
      return "Unknown";
  }
}

/**
 * @brief 资源 ID
 *
 * [63:56] - 资源类型 (8 bits)
 * [55:0]  - 资源数据 (56 bits)
 */
class ResourceId {
 public:
  /**
   * @brief 获取资源类型
   * @return ResourceType 资源类型
   */
  [[nodiscard]] constexpr auto GetType() const -> ResourceType {
    return static_cast<ResourceType>((value_ >> kTypeShift) & 0xFF);
  }

  /**
   * @brief 获取资源数据
   * @return uint64_t 资源数据
   */
  [[nodiscard]] constexpr auto GetData() const -> uint64_t {
    return value_ & kDataMask;
  }

  /**
   * @brief 获取类型名称（用于调试）
   * @return const char* 资源类型名称
   */
  [[nodiscard]] constexpr auto GetTypeName() const -> const char* {
    return GetResourceTypeName(GetType());
  }

  /**
   * @brief 检查是否为有效资源
   * @return bool 有效返回 true，无效返回 false
   */
  constexpr explicit operator bool() const {
    return GetType() != ResourceType::kNone;
  }

  /**
   * @brief 隐式转换到 uint64_t（用于存储和比较）
   * @return uint64_t 内部存储值
   */
  constexpr operator uint64_t() const { return value_; }

  /**
   * @brief 相等比较操作符
   * @param other 另一个资源 ID
   * @return bool 相等返回 true
   */
  constexpr auto operator==(const ResourceId& other) const -> bool {
    return value_ == other.value_;
  }

  /**
   * @brief 不等比较操作符
   * @param other 另一个资源 ID
   * @return bool 不相等返回 true
   */
  constexpr auto operator!=(const ResourceId& other) const -> bool {
    return value_ != other.value_;
  }

  /// @name 构造/析构函数
  /// @{

  /**
   * @brief 构造资源 ID
   * @param type 资源类型
   * @param data 资源数据 (如地址、PID 等)
   */
  constexpr ResourceId(ResourceType type, uint64_t data)
      : value_((static_cast<uint64_t>(type) << kTypeShift) |
               (data & kDataMask)) {}

  ResourceId() = default;
  ResourceId(const ResourceId&) = default;
  ResourceId(ResourceId&&) = default;
  auto operator=(const ResourceId&) -> ResourceId& = default;
  auto operator=(ResourceId&&) -> ResourceId& = default;
  ~ResourceId() = default;
  /// @}

 private:
  static constexpr uint8_t kTypeShift = 56;
  static constexpr uint64_t kTypeMask = 0xFF00000000000000ULL;
  static constexpr uint64_t kDataMask = 0x00FFFFFFFFFFFFFFULL;

  /// 内部存储的原始值
  uint64_t value_{0};
};

// std::hash 特化，使 ResourceId 可以作为 unordered_map 的键
namespace std {
template <>
struct hash<ResourceId> {
  constexpr auto operator()(const ResourceId& id) const noexcept -> size_t {
    return hash<uint64_t>{}(static_cast<uint64_t>(id));
  }
};
}  // namespace std

// etl::hash 特化，使 ResourceId 可以作为 etl::unordered_map 的键

namespace etl {
template <>
struct hash<ResourceId> {
  auto operator()(const ResourceId& id) const -> size_t {
    return etl::hash<uint64_t>{}(static_cast<uint64_t>(id));
  }
};
}  // namespace etl
