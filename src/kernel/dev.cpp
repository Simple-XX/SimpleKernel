
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev.cpp for Simple-XX/SimpleKernel.

#include "dev.h"
#include "virtio.h"
#include "virtio_blk.h"
#include "dtb.h"

DEV::DEV(void) {
    DTB dtb = DTB();
    // 初始化 virtio
    VIRTIO virtio = VIRTIO();
    return;
}

DEV::~DEV(void) {
    return;
}
