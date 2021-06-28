
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

// See
// http://docs.oasis-open.org/virtio/virtio/v1.0/csprd01/virtio-v1.0-csprd01.html#x1-530002
// for more info

class VIRTIO {
private:
protected:
    static constexpr const uint64_t MAGIC_VALUE = 0x74726976;
    static constexpr const uint64_t VERSION     = 0x02;
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

    struct virtio_queue_t {
        // Arbitrary descriptor layouts.
        static constexpr const uint64_t VIRTIO_F_ANY_LAYOUT = 27;
        // Support for indirect descriptors
        static constexpr const uint64_t VIRTIO_F_INDIRECT_DESC = 28;
        // Support for avail_event and used_event fields
        static constexpr const uint64_t VIRTIO_F_EVENT_IDX = 29;

        // virtio-v1.1#2.6.5
        struct virtq_desc_t {
            // This marks a buffer as continuing via the next field.
            static constexpr const uint64_t VIRTQ_DESC_F_NEXT = 1;
            // This marks a buffer as write-only (otherwise read-only).
            static constexpr const uint64_t VIRTQ_DESC_F_WRITE = 2;
            // This means the buffer contains a list of buffer descriptors.
            static constexpr const uint64_t VIRTQ_DESC_F_INDIRECT = 4;
            static constexpr const uint64_t ALIGN                 = 16;
            // Address(guest - physical).
            uint64_t addr;
            // Length.
            // 当描述符作为节点连接一个描述符表时，
            // 描述符项的个数为 len/sizeof(virtq_desc_t)
            uint32_t len;
            // The flags as indicated above.
            uint16_t flags;
            // Next field if flags & NEXT
            uint16_t next;
        } __attribute__((packed));

        // virtio-v1.1#2.6.6
        struct virtq_avail_t {
            static constexpr const uint64_t VIRTQ_AVAIL_F_NO_INTERRUPT = 1;
            static constexpr const uint64_t ALIGN                      = 2;
            uint16_t                        flags;
            uint16_t                        idx;
            // queue size
            uint16_t ring[];
            // Only if VIRTIO_F_EVENT_IDX
            // uint16_t used_event;
        } __attribute__((packed));

        // virtio-v1.1#2.6.8
        struct virtq_used_elem_t {
            // Index of start of used descriptor chain.
            uint32_t id;
            // Total length of the descriptor chain which was used (written to)
            uint32_t len;
        } __attribute__((packed));

        // virtio-v1.1#2.6.8
        struct virtq_used_t {
            static constexpr const uint64_t VIRTQ_USED_F_NO_NOTIFY = 1;
            static constexpr const uint64_t ALIGN                  = 4;
            uint16_t                        flags;
            uint16_t                        idx;
            // queue size
            virtq_used_elem_t ring[];
            // Only if VIRTIO_F_EVENT_IDX
            // uint16_t avail_event;
        } __attribute__((packed));

        struct virtq_t {
            // 此结构的物理地址
            uint32_t phys;
            // 数组长度
            uint32_t              len;
            uint32_t              seen_used;
            uint32_t              free_desc;
            struct virtq_desc_t * desc;
            struct virtq_avail_t *avail;
            uint16_t *            used_event;
            struct virtq_used_t * used;
            uint16_t *            avail_event;
            // 此结构的虚拟地址
            void **desc_virt;
        } __attribute__((packed));
        // virtq_t 各个结构体的偏移
        uint64_t off_desc;
        uint64_t off_avail;
        uint64_t off_used_event;
        uint64_t off_used;
        uint64_t off_avail_event;
        uint64_t off_desc_virt;

        virtq_t *virtq;
        virtio_queue_t(size_t _size);
        ~virtio_queue_t(void);
        // 分配 desc
        // 返回: 可用的 desc
        uint32_t alloc_desc(void *_addr);
        // 回收 desc
        // _desc: 要回收的 desc
        void free_desc(uint32_t _desc);
    };

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

    // virtio mmio 寄存器基地址
    virtio_regs_t * regs;
    virtio_queue_t *queue;
    uint32_t        read(void *_off);
    uint64_t        read64(void *_off);
    void            write(void *_off, uint32_t _val);
    // 设置 features
    bool set_features(const mystl::vector<feature_t> &_features);
    // 设置队列
    void add_to_device(uint32_t _queue_sel);

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
};

#endif /* _VIRTIO_H_ */
