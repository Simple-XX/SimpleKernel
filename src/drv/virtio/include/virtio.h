
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
    static constexpr const uint64_t MMIO_REG_STATUS            = 0x70;
    static constexpr const uint64_t MMIO_REG_QUEUE_DESC_LOW    = 0x80;
    static constexpr const uint64_t MMIO_REG_QUEUE_DESC_HIGH   = 0x84;
    static constexpr const uint64_t MMIO_REG_QUEUE_DRIVER_LOW  = 0x90;
    static constexpr const uint64_t MMIO_REG_QUEUE_DRIVER_HIGH = 0x94;
    static constexpr const uint64_t MMIO_REG_QUEUE_DEVICE_LOW  = 0xA0;
    static constexpr const uint64_t MMIO_REG_QUEUE_DEVICE_HIGH = 0xA4;
    static constexpr const uint64_t MMIO_REG_CONFIG_GEN        = 0xFC;
    static constexpr const uint64_t MMIO_REG_CONFIG            = 0x100;

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

    // virtio 基地址
    void *          base_addr;
    virtio_queue_t *queue;
    uint32_t        read(uint32_t _off);
    uint64_t        read64(uint32_t _off);
    void            write(uint32_t _off, uint32_t _val);
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
