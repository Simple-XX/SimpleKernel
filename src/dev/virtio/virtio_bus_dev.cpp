
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_bus_dev.cpp for Simple-XX/SimpleKernel.

#include "virtio_bus_dev.h"

virtio_bus_dev_t::virtio_bus_dev_t(const mystl::vector<resource_t *> &_res)
    : bus_t(_res) {
    printf("dev_name: %s\n", dev_name.c_str());
    // dev_name = mystl::string("virtio,mmio");
    dev_name = "virtio,mmio";
    printf("fine\n");
    drv_name = "virtio_mmio drv";
    bus_name = "";
    drv      = nullptr;
    dev_num  = 0;
    printf("virtio_bus_dev_t ctor.\n");
    return;
}

virtio_bus_dev_t::~virtio_bus_dev_t(void) {
    return;
}
