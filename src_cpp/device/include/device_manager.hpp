/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/singleton.h>

#include <cassert>

#include "device_node.hpp"
#include "driver_registry.hpp"
#include "expected.hpp"
#include "kernel_log.hpp"
#include "spinlock.hpp"

/**
 * @brief 设备管理器 — 管理所有设备节点和驱动程序。
 *
 * @pre  调用任何方法前，内存子系统必须已完成初始化
 * @post ProbeAll() 执行完成后，已绑定的设备可供使用
 */
class DeviceManager {
 public:
  /**
   * @brief 注册总线并立即枚举其设备。
   *
   * @tparam B              总线类型（必须满足 Bus 概念约束）
   * @param  bus            总线实例
   * @return Expected<void> 成功返回 void，失败返回错误
   */
  template <Bus B>
  auto RegisterBus(B& bus) -> Expected<void> {
    LockGuard guard(lock_);

    if (device_count_ >= kMaxDevices) {
      return std::unexpected(Error(ErrorCode::kOutOfMemory));
    }

    size_t remaining = kMaxDevices - device_count_;
    auto result = bus.Enumerate(devices_ + device_count_, remaining);
    if (!result.has_value()) {
      klog::Err("DeviceManager: bus '{}' enumeration failed: {}", B::GetName(),
                result.error().message());
      return std::unexpected(result.error());
    }

    size_t count = result.value();
    for (size_t i = 0; i < count; ++i) {
      devices_[device_count_ + i].dev_id = next_dev_id_++;
      if (!name_index_.full()) {
        name_index_.insert(
            {devices_[device_count_ + i].name, device_count_ + i});
      }
    }
    device_count_ += count;

    klog::Info("DeviceManager: bus '{}' enumerated {} device(s)", B::GetName(),
               count);
    return {};
  }

  /**
   * @brief 匹配已注册的驱动程序并探测所有未绑定的设备。
   *
   * @return Expected<void> 成功返回 void，失败返回错误
   */
  [[nodiscard]] auto ProbeAll() -> Expected<void>;

  /**
   * @brief 根据名称查找设备。
   *
   * @note  只读路径 — 枚举完成后 device_count_ 和 devices_ 稳定，
   *        并发调用者可安全使用。
   * @param  name           设备名称
   * @return Expected<DeviceNode*> 设备指针，或 kDeviceNotFound
   */
  [[nodiscard]] auto FindDevice(const char* name) -> Expected<DeviceNode*>;

  /**
   * @brief 按类型枚举设备。
   *
   * @param  type           设备类型
   * @param  out            输出的设备节点指针数组
   * @param  max            最大结果数量
   * @return size_t         找到的匹配设备数量
   */
  [[nodiscard]] auto FindDevicesByType(DeviceType type, DeviceNode** out,
                                       size_t max) -> size_t;

  /**
   * @brief 访问驱动注册表。
   *
   * @return DriverRegistry& 驱动注册表实例
   */
  [[nodiscard]] auto GetRegistry() -> DriverRegistry& { return registry_; }

  /// @name 构造 / 析构
  /// @{
  DeviceManager() = default;
  ~DeviceManager() = default;
  DeviceManager(const DeviceManager&) = delete;
  DeviceManager(DeviceManager&&) = delete;
  auto operator=(const DeviceManager&) -> DeviceManager& = delete;
  auto operator=(DeviceManager&&) -> DeviceManager& = delete;
  /// @}

 private:
  static constexpr size_t kMaxDevices = 64;
  DeviceNode devices_[kMaxDevices]{};
  size_t device_count_{0};
  uint32_t next_dev_id_{0};
  DriverRegistry registry_;
  SpinLock lock_{"device_manager"};
  etl::flat_map<const char*, size_t, kMaxDevices, CStrLess> name_index_;
};

using DeviceManagerSingleton = etl::singleton<DeviceManager>;
