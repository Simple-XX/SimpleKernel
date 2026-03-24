/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include "device_node.hpp"
#include "expected.hpp"
#include "kernel_fdt.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"

/// 平台总线 — 从扁平设备树（FDT）枚举设备
class PlatformBus {
 public:
  explicit PlatformBus(KernelFdt& fdt) : fdt_(fdt) {}

  [[nodiscard]] static auto GetName() -> const char* { return "platform"; }

  /**
   * @brief 枚举所有含 compatible 字符串的 FDT 设备节点。
   *
   * @param  out  输出的 DeviceNode 数组
   * @param  max  最多写入的节点数
   * @return Expected<size_t> 已写入的节点数
   */
  auto Enumerate(DeviceNode* out, size_t max) -> Expected<size_t> {
    size_t count = 0;

    auto result = fdt_.ForEachDeviceNode(
        [&out, &count, max](const char* node_name, const char* compatible_data,
                            size_t compatible_len, uint64_t mmio_base,
                            size_t mmio_size, uint32_t irq) -> bool {
          if (count >= max) return false;
          if (compatible_data == nullptr || compatible_len == 0) return true;

          auto& node = out[count];

          kstd::strncpy(node.name, node_name, sizeof(node.name) - 1);
          node.name[sizeof(node.name) - 1] = '\0';

          node.bus_type = BusType::kPlatform;
          node.type = DeviceType::kPlatform;
          node.mmio_base = mmio_base;
          node.mmio_size = mmio_size;
          node.irq = irq;

          size_t copy_len = compatible_len < sizeof(node.compatible)
                                ? compatible_len
                                : sizeof(node.compatible);
          if (compatible_len > sizeof(node.compatible)) {
            klog::Warn("PlatformBus: compatible truncated {}\u2192{} for '{}'",
                       compatible_len, sizeof(node.compatible), node_name);
          }
          kstd::memcpy(node.compatible, compatible_data, copy_len);
          node.compatible_len = copy_len;

          klog::Info(
              "PlatformBus: found '{}' compatible='{}' mmio={:#x} size={:#x} "
              "irq={}",
              node_name, compatible_data, mmio_base, mmio_size, irq);

          ++count;
          return true;
        });

    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
    return count;
  }

  /// @name 构造 / 析构
  /// @{
  PlatformBus() = delete;
  ~PlatformBus() = default;
  PlatformBus(const PlatformBus&) = delete;
  PlatformBus(PlatformBus&&) = delete;
  auto operator=(const PlatformBus&) -> PlatformBus& = delete;
  auto operator=(PlatformBus&&) -> PlatformBus& = delete;
  /// @}

 private:
  KernelFdt& fdt_;
};
