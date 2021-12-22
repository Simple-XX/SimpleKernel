
/**
 * @file virtio_dev.h
 * @brief virtio 设备头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-12-21
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#ifndef _VIRTIO_DEV_H_
#define _VIRTIO_DEV_H_

#include "dev.h"
#include "virtio_queue.h"

/**
 * @brief 所有 virtio 设备的公有属性，标准文档的 #4 之外的部分
 */
class virtio_dev_t : public dev_t {
private:
protected:
public:
    /**
     * @brief virtio 设备类型
     * @see virtio-v1.1#5
     */
    enum : uint8_t {
        RESERVED = 0x00,
        NETWORK_CARD,
        BLOCK_DEVICE,
        CONSOLE,
        ENTROPY_SOURCE,
        MEMORY_BALLOONING,
        IOMEMORY,
        RPMSG,
        SCSI_HOST,
        NINEP_TRANSPORT,
        MAC_80211_WLAN,
        RPROC_SERIAL,
        VIRTIO_CAIF,
        MEMORY_BALLOON,
        GPU_DEVICE = 0x10,
        TIMER_CLOCK_DEVICE,
        INPUT_DEVICE,
        SOCKET_DEVICE,
        CRYPTO_DEVICE,
        SIGNAL_DISTRIBUTION_MODULE,
        PSTORE_DEVICE,
        IOMMU_DEVICE,
        MEMORY_DEVICE,
    };

    /**
     * @brief virtio 设备类型名名称
     * @see virtio-v1.1#5
     */
    static constexpr const char *const virtio_device_name[25] = {
        "reserved (invalid)",
        "network card",
        "block device",
        "console",
        "entropy source",
        "memory ballooning(traditional)",
        "ioMemory",
        "rpmsg",
        "SCSI host",
        "9P transport",
        "mac80211 wlan",
        "rproc serial",
        "virtio CAIF",
        "memory balloon",
        "null",
        "null",
        "GPU device",
        "Timer / Clock device",
        "Input device",
        "Socket device",
        "Crypto device",
        "Signal Distribution Module",
        "pstore device",
        "IOMMU device",
        "Memory device",
    };

    /**
     * @brief 设备状态标志 Device Status Field
     * @see virtio-v1.1#2.1
     */
    /// OS 已识别设备
    static constexpr const uint32_t DEVICE_STATUS_ACKNOWLEDGE = 0x1;
    /// OS 已匹配驱动
    static constexpr const uint32_t DEVICE_STATUS_DRIVER = 0x2;
    /// 驱动准备就绪
    static constexpr const uint32_t DEVICE_STATUS_DRIVER_OK = 0x4;
    /// 设备特性设置就绪
    static constexpr const uint32_t DEVICE_STATUS_FEATURES_OK = 0x8;
    /// 设备设置错误
    static constexpr const uint32_t DEVICE_STATUS_DEVICE_NEEDS_RESET = 0x40;
    /// 设备出错
    static constexpr const uint32_t DEVICE_STATUS_FAILED = 0x80;

    virtio_dev_t(void);
    virtio_dev_t(const resource_t &_resource);
    virtual ~virtio_dev_t(void);
};

#endif /* _VIRTIO_DEV_H_ */
