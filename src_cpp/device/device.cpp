/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "device_manager.hpp"
#include "kernel.h"
#include "kernel_fdt.hpp"
#include "kernel_log.hpp"
#include "ns16550a/ns16550a_driver.hpp"
#include "pl011/pl011_driver.hpp"
#include "platform_bus.hpp"
#include "virtio/virtio_driver.hpp"

namespace {

/// 内建驱动表 — 编译期构建，运行期批量注册。
/// 添加新驱动只需在此追加一行。
using GetEntryFn = auto (*)() -> const DriverEntry&;
static constexpr GetEntryFn kBuiltinDrivers[] = {
    &Ns16550aDriver::GetEntry,
    &Pl011Driver::GetEntry,
    &VirtioDriver::GetEntry,
};

}  // namespace

/// 设备子系统初始化入口
auto DeviceInit() -> void {
  DeviceManagerSingleton::create();
  auto& dm = DeviceManagerSingleton::instance();

  // 创建驱动单例
  Ns16550aDriverSingleton::create();
  Pl011DriverSingleton::create();
  VirtioDriverSingleton::create();

  for (const auto& get_entry : kBuiltinDrivers) {
    const auto& entry = get_entry();
    if (auto r = dm.GetRegistry().Register(entry); !r) {
      klog::Err("DeviceInit: register driver '{}' failed: {}", entry.name,
                r.error().message());
      return;
    }
  }

  PlatformBus platform_bus(KernelFdtSingleton::instance());
  if (auto r = dm.RegisterBus(platform_bus); !r) {
    klog::Err("DeviceInit: PlatformBus enumeration failed: {}",
              r.error().message());
    return;
  }

  if (auto r = dm.ProbeAll(); !r) {
    klog::Err("DeviceInit: ProbeAll failed: {}", r.error().message());
    return;
  }

  klog::Info("DeviceInit: complete");
}
