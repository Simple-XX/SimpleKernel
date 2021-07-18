
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_bus_drv.cpp for Simple-XX/SimpleKernel.

#include "virtio_bus_drv.h"

virtio_bus_drv_t::virtio_bus_drv_t(void) {
    name = "virtio_mmio drv";
    return;
}

virtio_bus_drv_t ::~virtio_bus_drv_t(void) {
    return;
}

bool virtio_bus_drv_t::init(void) {
    printf("virtio_bus_drv_t init.\n");
    return true;
}

void virtio_bus_drv_t::read(void) {
    return;
}

void virtio_bus_drv_t::write(void) {
    return;
}
