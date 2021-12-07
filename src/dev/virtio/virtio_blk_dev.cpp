
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk_dev.cpp for Simple-XX/SimpleKernel.

#include "virtio_blk_dev.h"

virtio_blk_dev_t::virtio_blk_dev_t(const resource_t &_resource)
    : virtio_dev_t(_resource) {
    dev_name        = "virtio blk dev";
    compatible_name = "virtio blk dev drv";
    bus_name        = "virtio mmio";
    drv             = nullptr;
    return;
}

virtio_blk_dev_t::~virtio_blk_dev_t(void) {
    return;
}
