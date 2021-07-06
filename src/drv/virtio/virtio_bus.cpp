
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_bus.cpp for Simple-XX/SimpleKernel.

#include "virtio_bus.h"

virtio_bus_t::virtio_bus_t(mystl::vector<resource_t *> _res) {
    dev_name = "virtio,mmio";
    drv_name = "virtio_mmio drv";
    drv      = nullptr;
    bus      = nullptr;
    res      = _res;
    return;
}

virtio_bus_t::~virtio_bus_t(void) {
    return;
}
