/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstdint>

namespace virtio {

/**
 * @brief Virtio 设备 ID 定义
 * @see virtio-v1.2#5 Device Types
 */
enum class DeviceId : uint32_t {
  kReserved = 0,
  kNetwork = 1,
  kBlock = 2,
  kConsole = 3,
  kEntropy = 4,
  kMemoryBalloonTraditional = 5,
  kIoMemory = 6,
  kRpmsg = 7,
  kScsiHost = 8,
  kNinepTransport = 9,
  kMac80211Wlan = 10,
  kRprocSerial = 11,
  kVirtioCaif = 12,
  kMemoryBalloon = 13,
  // 14-15 保留
  kGpu = 16,
  kTimerClock = 17,
  kInput = 18,
  kSocket = 19,
  kCrypto = 20,
  kSignalDist = 21,
  kPstore = 22,
  kIommu = 23,
  kMemory = 24,
  kSound = 25,
  kFilesystem = 26,
  kPmem = 27,
  kRpmb = 28,
  // 29-31 保留
  kScmi = 32,
  // 33 保留
  kI2cAdapter = 34,
  // 35 保留
  kCan = 36,
  // 37 保留
  kParameterServer = 38,
  kAudioPolicy = 39,
  kBluetooth = 40,
  kGpio = 41,
  kRdma = 42,
};

/**
 * @brief 保留特性位定义 (Reserved Feature Bits)
 * @see virtio-v1.2#6
 */
enum class ReservedFeature : uint64_t {
  /// 设备支持间接描述符 (VIRTIO_F_INDIRECT_DESC) [1 << 28]
  kIndirectDesc = 0x10000000,
  /// 设备支持 avail_event 和 used_event 字段 (VIRTIO_F_EVENT_IDX) [1 << 29]
  kEventIdx = 0x20000000,
  /// 设备符合 virtio 1.0+ 规范 (VIRTIO_F_VERSION_1) [1 << 32]
  kVersion1 = 0x100000000,
  /// 设备可被 IOMMU 限定的平台访问 (VIRTIO_F_ACCESS_PLATFORM) [1 << 33]
  kAccessPlatform = 0x200000000,
  /// 支持 Packed Virtqueue 布局 (VIRTIO_F_RING_PACKED) [1 << 34]
  kRingPacked = 0x400000000,
  /// 按顺序使用缓冲区 (VIRTIO_F_IN_ORDER) [1 << 35]
  kInOrder = 0x800000000,
  /// 平台提供内存排序保证 (VIRTIO_F_ORDER_PLATFORM) [1 << 36]
  kOrderPlatform = 0x1000000000,
  /// 支持 Single Root I/O Virtualization (VIRTIO_F_SR_IOV) [1 << 37]
  kSrIov = 0x2000000000,
  /// 驱动在通知中传递额外数据 (VIRTIO_F_NOTIFICATION_DATA) [1 << 38]
  kNotificationData = 0x4000000000,
  /// 驱动使用设备提供的数据作为可用缓冲区通知的 virtqueue 标识符
  /// (VIRTIO_F_NOTIF_CONFIG_DATA) [1 << 39]
  kNotifConfigData = 0x8000000000,
  /// 驱动可以单独重置队列 (VIRTIO_F_RING_RESET) [1 << 40]
  kRingReset = 0x10000000000,
};

}  // namespace virtio
