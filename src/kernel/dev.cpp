
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev.cpp for Simple-XX/SimpleKernel.

#include "dev.h"
#include "virtio_blk.h"
#include "dtb.h"

DEV::DEV(void) {
    // 根据 dtb 获取硬件信息
    DTB dtb = DTB();
    // 获取块设备
    auto blk_dev = dtb.find("virtio_mmio@10001000");
    // 初始化 virtio
    // TODO: 这里的参数应该从 blk_dev 获取
    VIRTIO_BLK blk = VIRTIO_BLK((void *)0x10001000);
    return;
}

DEV::~DEV(void) {
    return;
}
