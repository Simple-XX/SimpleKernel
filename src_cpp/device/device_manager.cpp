/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "device_manager.hpp"

#include <cassert>

#include "kernel_log.hpp"

auto DeviceManager::ProbeAll() -> Expected<void> {
  LockGuard guard(lock_);

  size_t probed = 0;
  size_t no_driver_count = 0;
  for (size_t i = 0; i < device_count_; ++i) {
    auto& node = devices_[i];

    const auto* drv = registry_.FindDriver(node);
    if (drv == nullptr) {
      ++no_driver_count;
      continue;
    }

    if (!drv->match(node)) {
      continue;
    }

    if (node.bound) {
      continue;
    }
    node.bound = true;

    drv->probe(node).or_else([&](auto&& err) {
      klog::Err("DeviceManager: probe '{}' failed: {}", node.name,
                err.message());
      node.bound = false;
      return Expected<void>{};
    });

    ++probed;
  }

  klog::Info("DeviceManager: probed {} device(s), {} skipped (no driver)",
             probed, no_driver_count);
  return {};
}

auto DeviceManager::FindDevice(const char* name) -> Expected<DeviceNode*> {
  assert(name != nullptr && "FindDevice: name must not be null");
  LockGuard guard(lock_);
  const auto it = name_index_.find(name);
  if (it != name_index_.end()) {
    return &devices_[it->second];
  }
  return std::unexpected(Error(ErrorCode::kDeviceNotFound));
}

auto DeviceManager::FindDevicesByType(DeviceType type, DeviceNode** out,
                                      size_t max) -> size_t {
  assert((out != nullptr || max == 0) &&
         "FindDevicesByType: out must not be null when max > 0");
  LockGuard guard(lock_);
  size_t found = 0;
  for (size_t i = 0; i < device_count_ && found < max; ++i) {
    if (devices_[i].type == type) {
      out[found++] = &devices_[i];
    }
  }
  return found;
}
