
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "virtio_blk.h"
#include "virtio_queue.h"

VIRTIO_BLK::VIRTIO_BLK(void *_addr) : VIRTIO(_addr, BLOCK_DEVICE) {
    // 设置属性
    // set_features();
    // TODO: 初始化队列

    printf("virtio blk init\n");
    return;
}

VIRTIO_BLK::~VIRTIO_BLK(void) {
    return;
}
