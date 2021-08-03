
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include "stdint.h"
#include "common.h"
#include "vector"
#include "string"
#include "virtio_queue.h"
#include "dtb.h"
#include "dev.h"

// See
// http://docs.oasis-open.org/virtio/virtio/v1.0/csprd01/virtio-v1.0-csprd01.html#x1-530002
// for more info

// virtio mmio 设备抽象
class virtio_mmio_dev_t {
private:
protected:
public:
    // virtio mmio 控制寄存器
    // virtio-v1.1#4.2.2
    struct virtio_regs_t {
        // a Little Endian equivalent of the “virt” string: 0x74726976
        uint32_t magic;
        // Device version number
        // 0x2, Legacy devices(see 4.2.4 Legacy interface) used 0x1.
        uint32_t version;
        // Virtio Subsystem Device ID
        uint32_t device_id;
        uint32_t Vendor_id;
        uint32_t device_features;
        uint32_t device_features_sel;
        uint32_t _reserved0[2];
        uint32_t driver_features;
        uint32_t driver_features_sel;
        uint32_t _reserved1[2];
        uint32_t queue_sel;
        uint32_t queue_num_max;
        uint32_t queue_num;
        uint32_t _reserved2[2];
        uint32_t queue_ready;
        uint32_t _reserved3[2];
        uint32_t queue_notify;
        uint32_t _reserved4[3];
        uint32_t interrupt_status;
        uint32_t interrupt_ack;
        uint32_t _reserved5[2];
        uint32_t status;
        uint32_t _reserved6[3];
        uint32_t queue_desc_low;
        uint32_t queue_desc_high;
        uint32_t _reserved7[2];
        uint32_t queue_driver_low;
        uint32_t queue_driver_high;
        uint32_t _reserved8[2];
        uint32_t queue_device_low;
        uint32_t queue_device_high;
        uint32_t _reserved9[21];
        uint32_t config_generation;
        uint32_t config[0];
    } __attribute__((packed));

    // virtio mmio 寄存器基地址
    virtio_regs_t *regs;
    // virtio queue，有些设备使用多个队列
    mystl::vector<virtio_queue_t *> queues;
    // 设备名
    // 设备类别
    virtio_mmio_dev_t(void *_addr);
    virtual ~virtio_mmio_dev_t(void);
    // 设备是否有效
    virtual bool is_valid(void);
    // 读写操作
    virtual void read(void);
    virtual void write(void);
};

class VIRTIO {
private:
protected:
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

    static constexpr const uint64_t MAGIC_VALUE = 0x74726976;
    static constexpr const uint64_t VERSION     = 0x02;

    // Device Status Field
    // virtio-v1.1#2.1
    static constexpr const uint64_t DEVICE_STATUS_ACKNOWLEDGE        = 0x1;
    static constexpr const uint64_t DEVICE_STATUS_DRIVER             = 0x2;
    static constexpr const uint64_t DEVICE_STATUS_DRIVER_OK          = 0x4;
    static constexpr const uint64_t DEVICE_STATUS_FEATURES_OK        = 0x8;
    static constexpr const uint64_t DEVICE_STATUS_DEVICE_NEEDS_RESET = 0x40;
    static constexpr const uint64_t DEVICE_STATUS_FAILED             = 0x80;

    // Device Status Field
    // virtio-v1.1#6
    static constexpr const uint64_t VIRTIO_F_RING_INDIRECT_DESC = 0x1C;
    static constexpr const uint64_t VIRTIO_F_RING_EVENT_IDX     = 0x1D;
    static constexpr const uint64_t VIRTIO_F_VERSION_1          = 0x20;

    // feature 信息
    struct feature_t {
        // feature 名称
        mystl::string name;
        // 第几位
        uint32_t bit;
        // 状态
        bool status;
    };

    feature_t base_features[3] = {
        {NAME2STR(VIRTIO_F_RING_INDIRECT_DESC), VIRTIO_F_RING_INDIRECT_DESC,
         false},
        {NAME2STR(VIRTIO_F_RING_EVENT_IDX), VIRTIO_F_RING_EVENT_IDX, false},
        {NAME2STR(VIRTIO_F_VERSION_1), VIRTIO_F_VERSION_1, false},
    };

    virtio_mmio_dev_t::virtio_regs_t *regs;
    // 队列向量，有些设备使用多个队列
    mystl::vector<virtio_queue_t *> queues;

    // 设置 features
    bool set_features(const mystl::vector<feature_t> &_features);
    // 将队列设置传递到相应寄存器
    // _queue_sel: 第几个队列，从 0 开始
    void add_to_device(uint32_t _queue_sel);

public:
    // virtio 设备的数量
    uint32_t size;
    // virtio 设备 mmio 信息
    struct virtio_mmio_t {
        // 设备地址
        uint64_t addr;
        // 内存长度
        uint64_t len;
        // 中断号
        uint32_t intrno;
    };
    mystl::vector<virtio_mmio_t> mmio;
    // 有效设备
    mystl::vector<virtio_mmio_dev_t> devs;

    // _addr: 设备地址
    // _type: 设备类型
    VIRTIO(void *_addr, virt_device_type_t _type);
    // VIRTIO(const mystl::vector<dtb_prop_node_t *> &_props);
    ~VIRTIO(void);
};

#endif /* _VIRTIO_H_ */
