
/**
 * @file virtqueue.h
 * @brief virtqueue 头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-12-21
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_VIRTQUEUE_H
#define SIMPLEKERNEL_VIRTQUEUE_H

#include "cstddef"
#include "cstdint"

/**
 * @brief To mark a descriptor as available, the driver sets the
 * VIRTQ_DESC_F_AVAIL bit in Flags to match the internal Driver Ring Wrap
 * Counter. It also sets the VIRTQ_DESC_F_USED bit to match the inverse value
 * (i.e. to not match the internal Driver Ring Wrap Counter)
 * @see virtio-v1.1#2.7.1
 */
static constexpr const uint32_t VIRTQ_DESC_F_AVAIL         = (1 << 7);

/**
 * @brief To mark a descriptor as used, the device sets the VIRTQ_DESC_F_USED
 * bit in Flags to match the internal Device Ring Wrap Counter. It also sets the
 * VIRTQ_DESC_F_AVAIL bit to match the same value.
 * @see virtio-v1.1#2.7.1
 */
static constexpr const uint32_t VIRTQ_DESC_F_USED          = (1 << 15);

/**
 * @brief  This marks a descriptor as device write-only (otherwise device
 * read-only).
 * @see virtio-v1.1#2.7.3
 */
static constexpr const uint32_t VIRTQ_DESC_F_WRITE         = 2;

/**
 * @brief This marks a buffer as continuing.
 * @see virtio-v1.1#2.7.6
 */
static constexpr const uint32_t VIRTQ_DESC_F_NEXT          = 1;

/**
 * @brief This means the element contains a table of descriptors.
 * @see virtio-v1.1#2.7.7
 */
static constexpr const uint32_t VIRTQ_DESC_F_INDIRECT      = 4;

/**
 * @brief Enable events
 * @see virtio-v1.1#2.7.10
 */
static constexpr const uint32_t RING_EVENT_FLAGS_ENABLE    = 0x0;

/**
 * @brief Disable events
 * @see virtio-v1.1#2.7.10
 */
static constexpr const uint32_t RING_EVENT_FLAGS_DISABLE   = 0x1;

/**
 * @brief Enable events for a specific descriptor
 * (as specified by Descriptor Ring Change Event Offset/Wrap Counter).
 * Only valid if VIRTIO_F_RING_EVENT_IDX has been negotiated.
 * @see virtio-v1.1#2.7.10
 */
static constexpr const uint32_t RING_EVENT_FLAGS_DESC      = 0x2;

/**
 * @brief This feature indicates that the device accepts arbitrary descriptor
 * layouts
 * @see virtio-v1.1#6.3
 */
static constexpr const uint32_t VIRTIO_F_ANY_LAYOUT        = 27;

/**
 * @brief Support for indirect descriptors
 * @see virtio-v1.1#Appendix A. virtio_queue.h
 */
static constexpr const uint32_t VIRTIO_F_INDIRECT_DESC     = 28;

/**
 * @brief Support for avail_event and used_event fields
 * @see virtio-v1.1#Appendix A. virtio_queue.h
 */
static constexpr const uint32_t VIRTIO_F_EVENT_IDX         = 29;

/**
 * @brief The driver uses this in avail->flags to advise the device: don't
 * interrupt me when you consume a buffer. It's unreliable, so it's
 * simply an optimization.
 * @see virtio-v1.1#Appendix A. virtio_queue.h
 */
static constexpr const uint32_t VIRTQ_AVAIL_F_NO_INTERRUPT = 1;

/**
 * @brief The device uses this in used->flags to advise the driver: don't kick
 * me when you add a buffer. It's unreliable, so it's simply an optimization.
 * @see virtio-v1.1#Appendix A. virtio_queue.h
 */
static constexpr const uint32_t VIRTQ_USED_F_NO_NOTIFY     = 1;

/**
 * @brief virtio 队列，使用 Split Virtqueues
 * @see virtio-v1.1#2.6
 */
struct split_virtqueue_t {
    // 默认队列大小
    static constexpr const size_t DEFAULT_SIZE = 16;

    // virtio-v1.1#2.6.5
    struct virtq_desc_t {
        /// 对齐大小
        static constexpr const size_t ALIGN = 16;
        /// Address(guest - physical).
        uint64_t                      addr;
        /// Length.
        // 当描述符作为节点连接一个描述符表时，描述符项的个数为
        // len/sizeof(virtq_desc_t)
        uint32_t                      len;
        /// The flags as indicated above.
        uint16_t                      flags;
        /// Next field if flags & NEXT
        uint16_t                      next;
    } __attribute__((packed));

    /**
     * @brief 可用队列
     * @see virtio-v1.1#2.6.6
     */
    struct virtq_avail_t {
        /// 对齐大小
        static constexpr const size_t ALIGN = 2;
        uint16_t                      flags;
        uint16_t                      idx;
        // queue size
        uint16_t                      ring[DEFAULT_SIZE];
        // Only if VIRTIO_F_EVENT_IDX
        uint16_t                      used_event;
    } __attribute__((packed));

    /**
     * @brief 已用队列数据
     * @see virtio-v1.1#2.6.8
     */
    struct virtq_used_elem_t {
        /// Index of start of used descriptor chain.
        uint32_t id;
        /// Total length of the descriptor chain which was used (written to)
        uint32_t len;
    } __attribute__((packed));

    /**
     * @brief 已用队列
     * @see virtio-v1.1#2.6.8
     */
    struct virtq_used_t {
        /// 对齐大小
        static constexpr const size_t ALIGN = 4;
        uint16_t                      flags;
        uint16_t                      idx;
        // queue size
        virtq_used_elem_t             ring[DEFAULT_SIZE];
        // Only if VIRTIO_F_EVENT_IDX
        // uint16_t avail_event;
    } __attribute__((packed));

    /**
     * @brief 完整的队列结构
     */
    struct virtq_t {
        // 此结构的物理地址
        uint64_t                phys;
        // 数组长度
        uint32_t                len;
        uint32_t                seen_used;
        uint32_t                free_desc;

        volatile virtq_desc_t*  desc;
        volatile virtq_avail_t* avail;
        volatile uint16_t*      used_event;
        volatile virtq_used_t*  used;
        volatile uint16_t*      avail_event;

        // 此结构的虚拟地址
        void**                  desc_virt;
    } __attribute__((packed));

    // virtq_t 各个结构体的偏移
    uint64_t off_desc;
    uint64_t off_driver;
    uint64_t off_device;
    uint64_t off_driver_event;
    uint64_t off_device_event;
    uint64_t off_desc_virt;

    /// 队列指针
    virtq_t* virtq;

    /**
     * @brief 构造函数
     * @param  _size            需要的队列数量，必须是 2 的幂，默认为
     * DEFAULT_SIZE
     * @see virtio-v1.1#2.5, virtio-v1.1#2.6
     */
    split_virtqueue_t(size_t _size = DEFAULT_SIZE);

    /**
     * @brief 析构函数
     */
    ~split_virtqueue_t(void);

    /**
     * @brief 分配 desc
     * @param  _addr            队列地址
     * @return uint32_t         可用的 desc 地址
     */
    uint32_t alloc_desc(void* _addr);

    /**
     * @brief 回收 desc
     * @param  _desc            要回收的 desc
     */
    void     free_desc(uint32_t _desc);
};

/**
 * @brief virtio 队列，使用 Packed Virtqueues
 * @see virtio-v1.1#2.7
 * @todo
 */
struct packed_virtqueue_t {
    /**
     * @brief The available descriptor refers to the buffers the driver is
     * sending to the device. addr is a physical address, and the descriptor is
     * identified with a buffer using the id field.
     * @note The descriptor ring is zero-initialized.
     * @see virtio-v1.1#2.7.13
     */
    struct pvirtq_desc_t {
        /// 对齐大小
        static constexpr const size_t ALIGN = 16;
        /// Buffer Address.
        uint64_t                      addr;
        /// Buffer Length.
        uint32_t                      len;
        /// Buffer ID.
        uint16_t                      id;
        /// The flags depending on descriptor type.
        uint16_t                      flags;
    };

    /**
     * @brief The following structure is used to reduce the number of
     * notifications sent between driver and device
     * @see virtio-v1.1#2.7.14
     */
    struct pvirtq_event_suppress_t {
        /// 对齐大小
        static constexpr const size_t ALIGN = 4;

        struct {
            /// Descriptor Ring Change Event Offset
            uint16_t desc_event_off  : 15;
            /// Descriptor Ring Change Event Wrap Counter
            uint16_t desc_event_wrap : 1;
        } desc;

        /// If desc_event_flags set to RING_EVENT_FLAGS_DESC
        struct {
            /// Descriptor Ring Change Event Flags
            uint16_t desc_event_flags : 2;
            /// Reserved, set to 0
            uint16_t reserved         : 14;
        } flags;
    };

    /**
     * @brief 构造函数
     * @param  _size            需要的队列数量，必须是 2 的幂，默认为
     * DEFAULT_SIZE
     * @see virtio-v1.1#2.5, virtio-v1.1#2.6
     */
    packed_virtqueue_t(size_t _size);

    /**
     * @brief 析构函数
     */
    ~packed_virtqueue_t(void);
};

#endif /* SIMPLEKERNEL_VIRTQUEUE_H */
