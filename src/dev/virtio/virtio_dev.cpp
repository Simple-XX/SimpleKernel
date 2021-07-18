
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_dev.cpp for Simple-XX/SimpleKernel.

#include "virtio_dev.h"

virtio_dev_t::virtio_dev_t(const mystl::vector<resource_t *> &_res)
    : dev_t(_res) {
    // 解析 dev_t 数据
    for (auto i : res) {
        if (i->type == resource_t::MEMORY) {
            addr_start = i->mem.start;
            addr_end   = i->mem.end;
        }
        else if (i->type == resource_t::INTR) {
            irq_no = i->irq_no;
        }
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
