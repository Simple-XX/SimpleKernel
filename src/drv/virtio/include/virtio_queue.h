
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_queue.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_QUEUE_H_
#define _VIRTIO_QUEUE_H_

#include "stdint.h"

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
        uint64_t phys;
        // 数组长度
        uint32_t                len;
        uint32_t                seen_used;
        uint32_t                free_desc;
        volatile virtq_desc_t  *desc;
        volatile virtq_avail_t *avail;
        volatile uint16_t      *used_event;
        volatile virtq_used_t  *used;
        volatile uint16_t      *avail_event;
        // 此结构的虚拟地址
        void **desc_virt;
    } __attribute__((packed));

    // virtq_t 各个结构体的偏移
    uint64_t off_desc;
    uint64_t off_driver;
    uint64_t off_device;
    uint64_t off_driver_event;
    uint64_t off_device_event;
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

#endif /* _VIRTIO_QUEUE_H_ */
