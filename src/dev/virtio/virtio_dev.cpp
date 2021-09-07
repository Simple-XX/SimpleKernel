
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_dev.cpp for Simple-XX/SimpleKernel.

#include "virtio_dev.h"

virtio_dev_t::virtio_dev_t(const resource_t &_resource) : dev_t(_resource) {
    // 解析 dev_t 数据
    // TODO
    if (resource.type & resource_t::MEM) {
        addr_start = (void *)resource.mem.addr;
        addr_end   = (void *)(resource.mem.addr + resource.mem.len);
    }
    if (resource.type & resource_t::INTR_NO) {
        irq_no = resource.intr.intr_no;
    }

// #define DEBUG
#ifdef DEBUG
    printf("virtio_dev_t ctor.\n");
#undef DEBUG
#endif
    return;
}

virtio_dev_t::~virtio_dev_t(void) {
    return;
}
