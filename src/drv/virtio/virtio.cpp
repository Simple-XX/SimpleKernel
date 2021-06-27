
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "stddef.h"
#include "stdio.h"
#include "assert.h"
#include "virtio.h"

uint32_t VIRTIO::read(uint32_t _off) {
    return *((uint32_t *)((ptrdiff_t)base_addr + _off));
}

void VIRTIO::write(uint32_t _off, uint32_t _val) {
    *((uint32_t *)((ptrdiff_t)base_addr + _off)) = _val;
    return;
}

bool VIRTIO::set_features(mystl::vector<feature_t> _features) {
    return true;
}

VIRTIO::VIRTIO(void *_addr, virt_device_type_t _type) : base_addr(_addr) {
    // 检查相关值
    assert(read(MMIO_REG_MAGIC_VALUE) == MAGIC_VALUE);
    assert(read(MMIO_REG_VERSION) == VERSION);
    // 检查类型是否符合
    assert(read(MMIO_REG_DEVICE_ID) == _type);
    // 初始化
    // virtio-v1.1#3.1.1
    // 重置设备
    write(MMIO_REG_STATUS, 0);
    // 置位 ACKNOWLEDGE
    VIRTIO::write(MMIO_REG_STATUS,
                  read(MMIO_REG_STATUS) | DEVICE_STATUS_ACKNOWLEDGE);
    // 置位 DRIVER
    VIRTIO::write(MMIO_REG_STATUS,
                  read(MMIO_REG_STATUS) | DEVICE_STATUS_DRIVER);
    // 接下来设置设备相关 feature，交给特定设备进行
    // 跳转到 virtio_blk.cpp 的构造函数
    return;
}

VIRTIO::~VIRTIO(void) {
    return;
}
