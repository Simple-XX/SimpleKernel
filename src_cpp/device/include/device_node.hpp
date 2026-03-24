/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

#include "expected.hpp"

/// 总线类型标识 — 为将来扩展 PCI/ACPI 总线预留的扩展点
enum class BusType : uint8_t { kPlatform, kPci, kAcpi };

/// 设备分类
enum class DeviceType : uint8_t {
  /// 字符设备（串口等）
  kChar,
  /// 块设备（磁盘等）
  kBlock,
  /// 网络设备
  kNet,
  /// 平台设备（中断控制器、定时器等）
  kPlatform,
};

namespace vfs {
class BlockDevice;
}  // namespace vfs

/**
 * @brief 单个设备的硬件资源描述。
 *
 * 纯数据结构 — 无生命周期管理、无 DMA 缓冲区、
 * 无并发原语。`bound` 受 DeviceManager::lock_ 保护
 *（在整个 ProbeAll() 循环期间持有）。
 */
struct DeviceNode {
  /// 可读的设备名称（来自 FDT 节点名）
  char name[32]{};

  BusType bus_type{BusType::kPlatform};
  DeviceType type{DeviceType::kPlatform};

  /// 第一个 MMIO 区域（需要多 BAR 支持时扩展为数组）
  uint64_t mmio_base{0};
  size_t mmio_size{0};

  /// 第一条中断线（需要多 IRQ 支持时扩展）
  uint32_t irq{0};

  /// FDT compatible 字符串列表（以 '\0' 分隔，如 "ns16550a\0ns16550\0"）
  char compatible[128]{};
  size_t compatible_len{0};

  /// 由 DeviceManager 分配的全局设备 ID
  uint32_t dev_id{0};

  /// 由 ProbeAll() 在 DeviceManager::lock_ 保护下设置 — 无需每节点锁。
  bool bound{false};

  /// 由驱动 Probe() 设置 — 指向内核生命周期的适配器。
  /// 若不是块设备或尚未探测则为 nullptr。
  vfs::BlockDevice* block_device{nullptr};
};

/// 总线 concept — 每种总线负责枚举自己管辖范围内的设备
template <typename B>
concept Bus = requires(B b, DeviceNode* out, size_t max) {
  /// 枚举该总线上所有设备，填充到 out[]，返回发现的设备数
  { b.Enumerate(out, max) } -> std::same_as<Expected<size_t>>;
  /// 返回总线名称（用于日志）
  { B::GetName() } -> std::same_as<const char*>;
};
