
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_mmio_bus_drv.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "vector"
#include "string"
#include "virtio_queue.h"
#include "virtio_mmio_bus_drv.h"
#include "drv.h"

// 设置 features
bool virtio_mmio_bus_drv_t::set_features(
    const mystl::vector<feature_t> &_features) {
    (void)_features;
    return true;
}

// 将队列设置传递到相应寄存器
// _queue_sel: 第几个队列，从 0 开始
void virtio_mmio_bus_drv_t::add_to_device(uint32_t _queue_sel) {
    (void)_queue_sel;
    return;
}

virtio_mmio_bus_drv_t::virtio_mmio_bus_drv_t(void) {
    name = "virtio,mmio";
    return;
}

virtio_mmio_bus_drv_t::virtio_mmio_bus_drv_t(const void *_addr) {
    (void)_addr;
    return;
}

virtio_mmio_bus_drv_t::~virtio_mmio_bus_drv_t(void) {
    return;
}

// 驱动操作
bool virtio_mmio_bus_drv_t::init(void) {
    return true;
}

void virtio_mmio_bus_drv_t::read(void) {
    return;
}

void virtio_mmio_bus_drv_t::write(void) {
    return;
}
