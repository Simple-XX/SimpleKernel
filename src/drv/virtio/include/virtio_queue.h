
/**
 * @file virtio_queue.h
 * @brief virtio queue 头文件
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

#ifndef SIMPLEKERNEL_VIRTIO_QUEUE_H
#define SIMPLEKERNEL_VIRTIO_QUEUE_H

#include "cstddef"
#include "cstdint"

/**
 * @brief virtio 队列
 * @see virtio-v1.1#2.6
 */
struct virtio_queue_t {
    // Arbitrary descriptor layouts.
    static constexpr const uint32_t VIRTIO_F_ANY_LAYOUT    = 27;
    // Support for indirect descriptors
    static constexpr const uint32_t VIRTIO_F_INDIRECT_DESC = 28;
    // Support for avail_event and used_event fields
    static constexpr const uint32_t VIRTIO_F_EVENT_IDX     = 29;

    // virtio-v1.1#2.6.5
    struct virtq_desc_t {
        /// This marks a buffer as continuing via the next field.
        static constexpr const uint32_t VIRTQ_DESC_F_NEXT     = 1;
        /// This marks a buffer as write-only (otherwise read-only).
        static constexpr const uint32_t VIRTQ_DESC_F_WRITE    = 2;
        /// This means the buffer contains a list of buffer descriptors.
        static constexpr const uint32_t VIRTQ_DESC_F_INDIRECT = 4;
        /// 对齐大小
        static constexpr const size_t   ALIGN                 = 16;
        /// Address(guest - physical).
        uint64_t                        addr;
        /// Length.
        // 当描述符作为节点连接一个描述符表时，描述符项的个数为
        // len/sizeof(virtq_desc_t)
        uint32_t                        len;
        /// The flags as indicated above.
        uint16_t                        flags;
        /// Next field if flags & NEXT
        uint16_t                        next;
    } __attribute__((packed));

    /**
     * @brief 可用队列
     * @see virtio-v1.1#2.6.6
     */
    struct virtq_avail_t {
        static constexpr const uint32_t VIRTQ_AVAIL_F_NO_INTERRUPT = 1;
        /// 对齐大小
        static constexpr const size_t   ALIGN                      = 2;
        uint16_t                        flags;
        uint16_t                        idx;
        // queue size
        uint16_t                        ring[];
        // Only if VIRTIO_F_EVENT_IDX
        // uint16_t used_event;
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
        static constexpr const uint32_t VIRTQ_USED_F_NO_NOTIFY = 1;
        /// 对齐大小
        static constexpr const size_t   ALIGN                  = 4;
        uint16_t                        flags;
        uint16_t                        idx;
        // queue size
        virtq_used_elem_t               ring[];
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
        virtq_desc_t volatile*  desc;
        virtq_avail_t volatile* avail;
        uint16_t volatile*      used_event;
        virtq_used_t volatile*  used;
        uint16_t volatile*      avail_event;
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
     * @param  _size            需要的队列数量，必须是 2 的幂
     * @see virtio-v1.1#2.5, virtio-v1.1#2.6
     */
    virtio_queue_t(size_t _size);
    ~virtio_queue_t(void);

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

#endif /* SIMPLEKERNEL_VIRTIO_QUEUE_H */
