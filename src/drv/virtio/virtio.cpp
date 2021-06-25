
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
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

VIRTIO::VIRTIO(void *_addr, virt_device_type_t _type) : base_addr(_addr) {
    // 检查相关值
    assert(read(MMIO_REG_MAGIC_VALUE) == 0x74726976);
    assert(read(MMIO_REG_VENDOR_ID) == 0x554D4551);
    // 判断是否为旧型设备
    // TODO: 这里不需要 assert，记录一下即可
    assert(read(MMIO_REG_VERSION) == 0x1);
    // 检查类型是否符合
    assert(read(MMIO_REG_DEVICE_ID) == _type);
    return;
}

VIRTIO::~VIRTIO(void) {
    return;
}
