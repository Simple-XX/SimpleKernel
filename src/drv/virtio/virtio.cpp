
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "assert.h"
#include "virtio.h"
#include "memlayout.h"

VIRTIO_DEVICE::VIRTIO_DEVICE(void) {
    return;
}

VIRTIO_DEVICE::~VIRTIO_DEVICE(void) {
    return;
}

uint32_t VIRTIO::read(uint32_t _off) {
    return *((uint32_t *)(MEMLAYOUT::VIRTIO0 + _off));
}

void VIRTIO::write(uint32_t _off, uint32_t _val) {
    *((uint32_t *)(MEMLAYOUT::VIRTIO0 + _off)) = _val;
    return;
}

VIRTIO::VIRTIO(void) {
    printf("magic: 0x%X\n", read(MAGIC_VALUE));
    printf("ver: 0x%X\n", read(VERSION));
    printf("device id: 0x%X\n", read(DEVICE_ID));
    printf("VEN: 0x%X\n", read(VENDOR_ID));
    assert(read(MAGIC_VALUE) == 0x74726976);
    assert(read(VERSION) == 0x1);
    assert(read(VENDOR_ID) == 0x554D4551);
    printf("virtio init\n");
    return;
}

VIRTIO::~VIRTIO(void) {
    return;
}
