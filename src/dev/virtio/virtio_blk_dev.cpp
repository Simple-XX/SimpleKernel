
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk_dev.cpp for Simple-XX/SimpleKernel.

#include "virtio_blk_dev.h"

virtio_blk_dev_t::virtio_blk_dev_t(const mystl::vector<resource_t> &_res)
    : virtio_dev_t(_res) {
    dev_name = "virtio blk dev";
    drv_name = "virtio blk dev drv";
    bus_name = "virtio mmio";
    drv      = nullptr;
    return;
}

virtio_blk_dev_t::~virtio_blk_dev_t(void) {
    return;
}
