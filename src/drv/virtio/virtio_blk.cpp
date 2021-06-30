
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "vector"
#include "intr.h"
#include "memlayout.h"
#include "virtio_blk.h"
#include "virtio_queue.h"

VIRTIO_BLK::VIRTIO_BLK(void *_addr) : VIRTIO(_addr, BLOCK_DEVICE) {
    // virtio-v1.1#3.1.1
    // 设置属性
    mystl::vector<feature_t> feat;
    for (auto i : base_features) {
        feat.push_back(i);
    }
    for (auto i : blk_features) {
        feat.push_back(i);
    }
    set_features(feat);
    // 属性设置完成
    // 置位 FEATURES_OK
    VIRTIO::write(&regs->status,
                  VIRTIO::read(&regs->status) | DEVICE_STATUS_FEATURES_OK);
    // 再次读取以确认设置成功
    if ((VIRTIO::read(&regs->status) & DEVICE_STATUS_FEATURES_OK) == false) {
        // 失败则报错
        assert(0);
        return;
    }
    // TODO: 初始化队列
    // 设置队列
    queue = new virtio_queue_t(8);
    add_to_device(0);
    config = (virtio_blk_config *)&regs->config;
    uint32_t i, j;
    do {
        i = VIRTIO::read(&regs->config_generation);
        // printf("config->capacity: 0x%X\n",
        // VIRTIO::read64(&config->capacity));
        // printf("config->blk_size: 0x%X\n",
        // VIRTIO::read64(&config->blk_size));
        j = VIRTIO::read(&regs->config_generation);
    } while (i != j);
    VIRTIO::write(&regs->status,
                  VIRTIO::read(&regs->status) | DEVICE_STATUS_DRIVER_OK);
    // 至此 virtio-blk 设备的设置就完成了
    // 允许中断
    PLIC::set(MEMLAYOUT::VIRTIO0_IRQ, true);
    printf("virtio blk init\n");
    return;
}

VIRTIO_BLK::~VIRTIO_BLK(void) {
    return;
}

size_t VIRTIO_BLK::rw(virtio_blk_req_t &_req, void *_buf) {
    uint32_t d1, d2, d3, datamode = 0;

    if (_req.type == virtio_blk_req_t::IN) {
        // mark page writeable
        datamode = VIRTIO::virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_WRITE;
    }

    d1                         = queue->alloc_desc(&_req);
    queue->virtq->desc[d1].len = sizeof(virtio_blk_req_t);
    queue->virtq->desc[d1].flags =
        VIRTIO::virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_NEXT;

    d2                         = queue->alloc_desc(_buf);
    queue->virtq->desc[d2].len = 512;
    queue->virtq->desc[d2].flags =
        datamode | VIRTIO::virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_NEXT;

    d3 = queue->alloc_desc((void *)(&_req + sizeof(virtio_blk_req_t)));
    queue->virtq->desc[d3].len = 1;
    queue->virtq->desc[d3].flags =
        VIRTIO::virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_WRITE;

    queue->virtq->desc[d1].next = d2;
    queue->virtq->desc[d2].next = d3;

    queue->virtq->avail->ring[queue->virtq->avail->idx % queue->virtq->len] =
        d1;
    queue->virtq->avail->idx += 1;
    VIRTIO::write(&regs->queue_notify, 0);
    return 0;
}
