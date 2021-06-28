
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "vector"
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
    VIRTIO::write(MMIO_REG_STATUS,
                  VIRTIO::read(MMIO_REG_STATUS) | DEVICE_STATUS_FEATURES_OK);
    // 再次读取以确认设置成功
    if ((VIRTIO::read(MMIO_REG_STATUS) & DEVICE_STATUS_FEATURES_OK) == false) {
        // 失败则报错
        assert(0);
        return;
    }
    // TODO: 初始化队列
    // 设置队列
    queue = new virtio_queue_t(8);
    add_to_device(0);
    //
    config = (virtio_blk_config *)((ptrdiff_t)base_addr + MMIO_REG_CONFIG);
    printf("config->capacity: 0x%p\n", (uint64_t)((uint64_t)&config->capacity -
                                                  (uint64_t)VIRTIO::base_addr));
    /* capacity is 64 bit, configuration reg read is not atomic */
    uint32_t i, j;
    do {
        i = VIRTIO::read(MMIO_REG_CONFIG_GEN);
        printf("config->capacity: 0x%X\n",
               VIRTIO::read64((uint64_t)((uint64_t)&config->capacity -
                                         (uint64_t)VIRTIO::base_addr)));
        j = VIRTIO::read(MMIO_REG_CONFIG_GEN);
    } while (i != j);
    // printf("config->blk_size: 0x%X\n", *(uint64_t *)&config->capacity);
    printf("virtio blk init\n");
    return;
}

VIRTIO_BLK::~VIRTIO_BLK(void) {
    return;
}
