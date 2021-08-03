
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_queue.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "common.h"
#include "assert.h"
#include "vmm.h"
#include "virtio_queue.h"

virtio_queue_t::virtio_queue_t(size_t _size) {
    // virtio-v1.1#2.5
    // virtio-v1.1#2.6
    // _size 应该为 2 的幂
    assert((_size & (_size - 1)) == 0);
    // TODO: 解释
    // 计算偏移
    // desc 偏移为 64
    off_desc = COMMON::ALIGN(sizeof(virtq_t), virtq_desc_t::ALIGN);
    // avail 偏移为在 desc 基地址+desc 个数*desc 长度，最后进行对齐
    off_driver = COMMON::ALIGN(off_desc + _size * sizeof(virtq_desc_t),
                               virtq_avail_t::ALIGN);
    off_device_event =
        (off_driver + sizeof(virtq_avail_t) + _size * sizeof(uint16_t));
    off_device =
        COMMON::ALIGN(off_device_event + sizeof(uint16_t), virtq_used_t::ALIGN);
    off_driver_event =
        (off_device + sizeof(virtq_used_t) + _size * sizeof(virtq_used_elem_t));
    off_desc_virt =
        COMMON::ALIGN(off_driver_event + sizeof(uint16_t), sizeof(void *));
    // 如果需要的内存大于1页则出错
    if (off_desc_virt + _size * sizeof(void *) > COMMON::PAGE_SIZE) {
        printf("virtq_create: error, too big for two pagess\n");
        return;
    }
    virtq              = (virtq_t *)malloc(COMMON::PAGE_SIZE);
    uint8_t *page_virt = (uint8_t *)virtq;
    virtq->phys        = VMM_VA2PA((uintptr_t)virtq);
    virtq->len         = _size;

    virtq->desc        = (virtq_desc_t *)(page_virt + off_desc);
    virtq->avail       = (virtq_avail_t *)(page_virt + off_driver);
    virtq->used_event  = (uint16_t *)(page_virt + off_device_event);
    virtq->used        = (virtq_used_t *)(page_virt + off_device);
    virtq->avail_event = (uint16_t *)(page_virt + off_driver_event);
    virtq->desc_virt   = (void **)(page_virt + off_desc_virt);

    virtq->avail->idx = 0;
    virtq->used->idx  = 0;
    virtq->seen_used  = virtq->used->idx;
    virtq->free_desc  = 0;

    for (size_t i = 0; i < _size; i++) {
        virtq->desc[i].next = i + 1;
    }

    return;
}

virtio_queue_t::~virtio_queue_t(void) {
    free(virtq);
    return;
}

uint32_t virtio_queue_t::alloc_desc(void *_addr) {
    // 获取空闲 desc 索引
    uint32_t desc = virtq->free_desc;
    // 获取下一个 desc 索引
    uint32_t next = virtq->desc[desc].next;
    if (desc == virtq->len) {
        printf("ERROR: ran out of virtqueue descriptors\n");
    }
    // 更新 free desc 索引
    virtq->free_desc = next;
    // 设置当前 desc addr 项
    virtq->desc[desc].addr = VMM_VA2PA((uintptr_t)_addr);
    // 设置当前 desc virt_addr 项
    virtq->desc_virt[desc] = _addr;
    return desc;
}

void virtio_queue_t::free_desc(uint32_t desc) {
    // 要释放 desc 的下一项指向另一个 free 的 desc
    virtq->desc[desc].next = virtq->free_desc;
    // free 索引设为当前索引
    virtq->free_desc = desc;
    // virt 地址置 0
    virtq->desc_virt[desc] = nullptr;
    return;
}
