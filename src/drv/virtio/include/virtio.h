
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include "stdint.h"

// See
// http://docs.oasis-open.org/virtio/virtio/v1.0/csprd01/virtio-v1.0-csprd01.html#x1-530002
// for more info

class VIRTIO {
private:
protected:
    // virtio mmio 控制寄存器
    // virtio-v1.1#4.2.2
    // a Little Endian equivalent of the “virt” string: 0x74726976
    static constexpr const uint64_t MMIO_REG_MAGIC_VALUE = 0x0;
    // Device version number
    // 0x2, Legacy devices(see 4.2.4 Legacy interface) used 0x1.
    static constexpr const uint64_t MMIO_REG_VERSION = 0x4;
    // Virtio Subsystem Device ID
    static constexpr const uint64_t MMIO_REG_DEVICE_ID = 0x8;
    // 0x554D4551
    static constexpr const uint64_t MMIO_REG_VENDOR_ID           = 0xC;
    static constexpr const uint64_t MMIO_REG_DEVICE_FEATURES     = 0x10;
    static constexpr const uint64_t MMIO_REG_DEVICE_FEATURES_SEL = 0x14;
    static constexpr const uint64_t MMIO_REG_DRIVER_FEATURES     = 0x20;
    static constexpr const uint64_t MMIO_REG_DRIVER_FEATURES_SEL = 0x24;
    // select queue, write-only
    static constexpr const uint64_t MMIO_REG_QUEUE_SEL = 0x30;
    // max size of current queue, read-only
    static constexpr const uint64_t MMIO_REG_QUEUE_NUM_MAX = 0x34;
    // size of current queue, write-only
    static constexpr const uint64_t MMIO_REG_QUEUE_NUM = 0x38;
    // ready bit
    static constexpr const uint64_t MMIO_REG_QUEUE_READY = 0x44;
    // write-only
    static constexpr const uint64_t MMIO_REG_QUEUE_NOTIFY = 0x50;
    // read-only
    static constexpr const uint64_t MMIO_REG_INTERRUPT_STATUS = 0x60;
    // write-only
    static constexpr const uint64_t MMIO_REG_INTERRUPT_ACK = 0x64;
    // read/write
    static constexpr const uint64_t MMIO_REG_STATUS = 0x70;
    // Device Status Field
    // virtio-v1.1#2.1
    static constexpr const uint64_t DEVICE_STATUS_ACKNOWLEDGE        = 0x1;
    static constexpr const uint64_t DEVICE_STATUS_DRIVER             = 0x2;
    static constexpr const uint64_t DEVICE_STATUS_DRIVER_OK          = 0x4;
    static constexpr const uint64_t DEVICE_STATUS_FEATURES_OK        = 0x8;
    static constexpr const uint64_t DEVICE_STATUS_DEVICE_NEEDS_RESET = 0x40;
    static constexpr const uint64_t DEVICE_STATUS_FAILED             = 0x80;

    // virtio 基地址
    void *   base_addr;
    uint32_t read(uint32_t _off);
    void     write(uint32_t _off, uint32_t _val);

public:
    // virtio 设备类型
    // virtio-v1.1#5
    typedef enum : uint8_t {
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
    } virt_device_type_t;
    // virtio 设备类型名名称
    const char *const virtio_device_name[25] = {
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

    // _addr: 设备地址
    // _type: 设备类型
    VIRTIO(void *_addr, virt_device_type_t _type);
    ~VIRTIO(void);

    /**
     * @brief 初始化磁盘，配置一些参数，详细流程可以查看
     *        virtio文档5.2节
     *        https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf
     *
     */
    void VirtioInit();

    /**
     * @brief 读取磁盘的相应扇区到b->data
     */
    void VirtioRead(char *buf, int sectorno);

    /**
     *  @brief 将b->data写入到磁盘对应扇区
     */
    void VirtioWrite(char *buf, int sectorno);

    /**
     * @brief virtio中断处理函数
     *
     */
    void VirtioIntr();

    /**
     * @brief 读写磁盘
     *
     * @param buf   读写数据缓存区
     * @param write 0代表读，1代表写
     */
    void VirtioRw(char *buf, uint64_t sector, int write);
};

#endif /* _VIRTIO_H_ */
