
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "virtio_blk.h"

VIRTIO_BLK::VIRTIO_BLK(void *_addr) : VIRTIO(_addr, BLOCK_DEVICE) {
    // 初始化流程
    // 确认可以读到
    // 取消 VIRTIO_BLK_F_RO 以允许写入
    printf("blk %X\n", VIRTIO::read(MMIO_REG_DEVICE_FEATURES));
    printf("virtio blk init\n");
    return;
}

VIRTIO_BLK::~VIRTIO_BLK(void) {
    return;
}
