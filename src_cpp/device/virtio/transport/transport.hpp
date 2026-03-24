/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <utility>

#include "expected.hpp"
#include "virtio/defs.h"

namespace virtio {

/**
 * @brief Virtio 传输层接口约束
 *
 * 编译期约束传输层实现必须提供的完整方法集。
 * DeviceInitializer 的 TransportImpl 参数必须满足此 concept。
 *
 * @note 部分方法（GetDeviceFeatures、GetQueueNumMax、GetQueueReady）
 *       在 MMIO 实现中非 const（需写选择器寄存器），因此 concept 对
 *       这些方法不要求 const 限定。
 *
 * @see virtio-v1.2#4 Virtio Transport Options
 */
template <typename T>
concept TransportConcept =
    requires(T t, const T ct, uint32_t u32, uint64_t u64) {
      // 设备有效性
      { ct.IsValid() } -> std::same_as<bool>;

      // 设备/供应商标识
      { ct.GetDeviceId() } -> std::same_as<uint32_t>;
      { ct.GetVendorId() } -> std::same_as<uint32_t>;

      // 设备状态
      { ct.GetStatus() } -> std::same_as<uint32_t>;
      { t.SetStatus(u32) } -> std::same_as<void>;

      // 特性协商（非 const：MMIO 需写选择器寄存器）
      { t.GetDeviceFeatures() } -> std::same_as<uint64_t>;
      { t.SetDriverFeatures(u64) } -> std::same_as<void>;

      // 队列配置（非 const：MMIO 需写 QueueSel）
      { t.GetQueueNumMax(u32) } -> std::same_as<uint32_t>;
      { t.SetQueueNum(u32, u32) } -> std::same_as<void>;
      { t.SetQueueDesc(u32, u64) } -> std::same_as<void>;
      { t.SetQueueAvail(u32, u64) } -> std::same_as<void>;
      { t.SetQueueUsed(u32, u64) } -> std::same_as<void>;
      { t.GetQueueReady(u32) } -> std::same_as<bool>;
      { t.SetQueueReady(u32, static_cast<bool>(true)) } -> std::same_as<void>;

      // 队列通知
      { t.NotifyQueue(u32) } -> std::same_as<void>;

      // 中断处理
      { ct.GetInterruptStatus() } -> std::same_as<uint32_t>;
      { t.AckInterrupt(u32) } -> std::same_as<void>;

      // 配置空间读取
      { ct.ReadConfigU8(u32) } -> std::same_as<uint8_t>;
      { ct.ReadConfigU16(u32) } -> std::same_as<uint16_t>;
      { ct.ReadConfigU32(u32) } -> std::same_as<uint32_t>;
      { ct.ReadConfigU64(u32) } -> std::same_as<uint64_t>;
      { ct.GetConfigGeneration() } -> std::same_as<uint32_t>;
    };

/**
 * @brief Virtio 传输层基类（零虚表开销，C++23 Deducing this）
 *
 * 利用 C++23 Deducing this（显式对象参数，P0847）实现编译期多态，
 * 消除虚表指针开销和 CRTP static_cast 样板代码，实现零开销抽象。
 * 子类（MmioTransport、PciTransport）继承此基类并提供具体的寄存器访问实现。
 *
 * 基类仅提供通用逻辑方法（Reset、NeedsReset、IsActive、AcknowledgeInterrupt），
 * 通过 Deducing this 在编译期静态分发到子类的具体实现。
 *
 * 子类应满足 TransportConcept 约束。
 *
 * @see TransportConcept
 * @see virtio-v1.2#4 Virtio Transport Options
 */
class Transport {
 public:
  /**
   * @brief 设备状态位定义
   * @see virtio-v1.2#2.1 Device Status Field
   */
  enum class DeviceStatus : uint32_t {
    /// 重置状态，驱动程序将此写入以重置设备
    kReset = 0,
    /// 表示客户操作系统已找到设备并识别为有效的 virtio 设备
    kAcknowledge = 1,
    /// 表示客户操作系统知道如何驱动该设备
    kDriver = 2,
    /// 表示驱动程序已准备好驱动设备（特性协商完成）
    kDriverOk = 4,
    /// 表示驱动程序已确认设备提供的所有功能
    kFeaturesOk = 8,
    /// 表示设备需要重置
    kDeviceNeedsReset = 64,
    /// 表示在客户机中出现问题，已放弃该设备
    kFailed = 128,
  };

  /**
   * @brief 重置设备
   * @see virtio-v1.2#2.1 Device Status Field
   */
  auto Reset(this auto&& self) -> void {
    self.SetStatus(std::to_underlying(DeviceStatus::kReset));
  }

  /**
   * @brief 检查设备是否需要重置
   *
   * @return true 表示设备需要重置
   * @see virtio-v1.2#2.1 Device Status Field
   */
  [[nodiscard]] auto NeedsReset(this auto const& self) -> bool {
    return (self.GetStatus() &
            std::to_underlying(DeviceStatus::kDeviceNeedsReset)) != 0;
  }

  /**
   * @brief 检查设备是否已激活（DRIVER_OK 已设置）
   */
  [[nodiscard]] auto IsActive(this auto const& self) -> bool {
    return (self.GetStatus() & std::to_underlying(DeviceStatus::kDriverOk)) !=
           0;
  }

  /**
   * @brief 确认并清除设备中断
   * @see virtio-v1.2#2.3 Notifications
   */
  auto AcknowledgeInterrupt(this auto&& self) -> void {
    auto status = self.GetInterruptStatus();
    if (status != 0) {
      self.AckInterrupt(status);
    }
  }

 protected:
  /// @name 构造/析构函数（仅允许派生类使用）
  /// @{
  Transport() = default;
  ~Transport() = default;
  Transport(Transport&&) noexcept = default;
  auto operator=(Transport&&) noexcept -> Transport& = default;
  Transport(const Transport&) = delete;
  auto operator=(const Transport&) -> Transport& = delete;
  /// @}
};

}  // namespace virtio
