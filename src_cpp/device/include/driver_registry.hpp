/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/delegate.h>
#include <etl/flat_map.h>
#include <etl/span.h>
#include <etl/vector.h>

#include <cassert>
#include <cstddef>
#include <cstdint>

#include "device_node.hpp"
#include "expected.hpp"
#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "spinlock.hpp"
#include "virtual_memory.hpp"

/// 驱动静态匹配表中的一条记录
struct MatchEntry {
  BusType bus_type{BusType::kPlatform};
  /// FDT compatible 字符串（平台总线）
  /// 或 vendor/HID 字符串（PCI/ACPI — 未来扩展）
  const char* compatible{nullptr};
};

/**
 * @brief 类型擦除的驱动条目 — 每个已注册驱动对应一条。
 *
 * @pre  注册前 match/probe/remove 委托必须已绑定
 */
struct DriverEntry {
  const char* name{nullptr};
  etl::span<const MatchEntry> match_table{};
  /// 硬件检测
  etl::delegate<bool(DeviceNode&)> match{};
  /// 驱动初始化
  etl::delegate<Expected<void>(DeviceNode&)> probe{};
  /// 驱动卸载
  etl::delegate<Expected<void>(DeviceNode&)> remove{};
};

/// flat_map 中 const char* 键的比较器（使用 kstd::strcmp）。
struct CStrLess {
  auto operator()(const char* a, const char* b) const -> bool {
    return kstd::strcmp(a, b) < 0;
  }
};

/**
 * @brief 驱动注册表 — 以 ETL vector 存储 DriverEntry，并附带 flat_map
 * 兼容索引。
 *
 * 注册时构建 etl::flat_map（compatible 字符串 → 驱动索引），
 * 将 FindDriver 的复杂度从 O(N·M·K) 降至 O(Cn · log T)。
 */
class DriverRegistry {
 public:
  /**
   * @brief 注册一个驱动条目。
   *
   * @pre  entry.match/probe/remove 委托已绑定
   * @return Expected<void> 注册表已满时返回 kOutOfMemory
   */
  [[nodiscard]] auto Register(const DriverEntry& entry) -> Expected<void> {
    LockGuard guard(lock_);
    if (drivers_.full()) {
      return std::unexpected(Error(ErrorCode::kOutOfMemory));
    }
    const size_t idx = drivers_.size();
    for (const auto& me : entry.match_table) {
      if (compat_map_.full()) {
        return std::unexpected(Error(ErrorCode::kOutOfMemory));
      }
      // 重复键时 insert() 为空操作 — 先注册的驱动优先。
      compat_map_.insert({me.compatible, idx});
    }
    drivers_.push_back(entry);
    return {};
  }

  /**
   * @brief 查找 match_table 中含有 node.compatible 字符串的第一个驱动
   *        （flat_map 查找，O(Cn · log T)）。
   *
   * @return DriverEntry 指针，若无匹配则返回 nullptr
   */
  [[nodiscard]] auto FindDriver(const DeviceNode& node) -> const DriverEntry* {
    // 遍历节点的 compatible 字符串列表，对每个字符串执行 flat_map 查找。
    const char* p = node.compatible;
    const char* end = node.compatible + node.compatible_len;
    while (p < end) {
      const auto it = compat_map_.find(p);
      if (it != compat_map_.end()) {
        auto& entry = drivers_[it->second];
        if (entry.match(const_cast<DeviceNode&>(node))) {
          return &entry;
        }
      }
      p += kstd::strlen(p) + 1;
    }
    return nullptr;
  }

  /// @name 构造 / 析构
  /// @{
  DriverRegistry() = default;
  ~DriverRegistry() = default;
  DriverRegistry(const DriverRegistry&) = delete;
  DriverRegistry(DriverRegistry&&) = delete;
  auto operator=(const DriverRegistry&) -> DriverRegistry& = delete;
  auto operator=(DriverRegistry&&) -> DriverRegistry& = delete;
  /// @}

 private:
  static constexpr size_t kMaxDrivers = 32;
  /// 所有驱动 MatchEntry 行数上限（32 个驱动 × 约 3 条 compatible 字符串）
  static constexpr size_t kMaxCompatEntries = 96;

  etl::vector<DriverEntry, kMaxDrivers> drivers_;
  etl::flat_map<const char*, size_t, kMaxCompatEntries, CStrLess> compat_map_;
  SpinLock lock_{"driver_registry"};
};

namespace mmio_helper {

/// 映射完成后的 MMIO 区域信息
struct ProbeContext {
  uint64_t base{0};
  size_t size{0};
};

/**
 * @brief 从节点提取 MMIO base/size 并通过 VirtualMemory 映射该区域。
 *
 * 不设置 node.bound — 调用方（驱动的 Probe()）负责在
 * DeviceManager::lock_ 保护下将 node.bound 置为 true。
 *
 * @param  node         设备节点（mmio_base 必须非零）
 * @param  default_size 当 node.mmio_size == 0 时使用的默认大小
 * @return Expected<ProbeContext>
 */
[[nodiscard]] inline auto Prepare(const DeviceNode& node, size_t default_size)
    -> Expected<ProbeContext> {
  assert(
      node.mmio_base != 0 &&
      "mmio_helper::Prepare: node has no MMIO base; driver matched wrong node");

  size_t size = node.mmio_size > 0 ? node.mmio_size : default_size;
  auto map_result =
      VirtualMemorySingleton::instance().MapMMIO(node.mmio_base, size);
  if (!map_result.has_value()) {
    return std::unexpected(map_result.error());
  }

  return ProbeContext{node.mmio_base, size};
}

}  // namespace mmio_helper
