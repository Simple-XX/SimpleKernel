
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_bus_dev.cpp for Simple-XX/SimpleKernel.

#include "virtio_bus_dev.h"

virtio_bus_dev_t::virtio_bus_dev_t(const mystl::vector<resource_t *> &_res)
    : bus_t(_res) {
    dev_name = "virtio,mmio";
    drv_name = "virtio_mmio drv";
    bus_name = "";
    drv      = nullptr;
    dev_num  = 0;
    return;
}

virtio_bus_dev_t::~virtio_bus_dev_t(void) {
    return;
}
